// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/mediastream/media_stream_renderer_factory.h"

#include <utility>

#include "base/memory/scoped_refptr.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/single_thread_task_runner.h"
#include "third_party/blink/public/platform/modules/mediastream/web_media_stream.h"
#include "third_party/blink/public/platform/modules/webrtc/webrtc_logging.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/modules/mediastream/media_stream_video_renderer_sink.h"
#include "third_party/blink/renderer/modules/mediastream/media_stream_video_track.h"
#include "third_party/blink/renderer/modules/mediastream/track_audio_renderer.h"
#include "third_party/blink/renderer/platform/mediastream/media_stream_audio_track.h"
#include "third_party/blink/renderer/platform/mediastream/media_stream_descriptor.h"
#include "third_party/blink/renderer/platform/webrtc/peer_connection_remote_audio_source.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
namespace blink {

namespace {

void SendLogMessage(const String& message) {
  WebRtcLogMessage("MSRF::" + message.Utf8());
}

}  // namespace

MediaStreamRendererFactory::MediaStreamRendererFactory() {}

MediaStreamRendererFactory::~MediaStreamRendererFactory() {}

scoped_refptr<MediaStreamVideoRenderer>
MediaStreamRendererFactory::GetVideoRenderer(
    const WebMediaStream& web_stream,
    const MediaStreamVideoRenderer::RepaintCB& repaint_cb,
    scoped_refptr<base::SequencedTaskRunner> video_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> main_render_task_runner) {
  DCHECK(!web_stream.IsNull());

  DVLOG(1) << "MediaStreamRendererFactory::GetVideoRenderer stream:"
           << web_stream.Id().Utf8();

  MediaStreamDescriptor& descriptor = *web_stream;
  auto video_components = descriptor.VideoComponents();
  if (video_components.empty() ||
      !MediaStreamVideoTrack::GetTrack(
          WebMediaStreamTrack(video_components[0].Get()))) {
    return nullptr;
  }

  return base::MakeRefCounted<MediaStreamVideoRendererSink>(
      video_components[0].Get(), repaint_cb, std::move(video_task_runner),
      std::move(main_render_task_runner));
}

scoped_refptr<MediaStreamAudioRenderer>
MediaStreamRendererFactory::GetAudioRenderer(
    const WebMediaStream& web_stream,
    WebLocalFrame* web_frame,
    const WebString& device_id,
    base::RepeatingCallback<void()> on_render_error_callback) {
  DCHECK(!web_stream.IsNull());
  SendLogMessage(String::Format("%s({web_stream_id=%s}, {device_id=%s})",
                                __func__, web_stream.Id().Utf8().c_str(),
                                device_id.Utf8().c_str()));

  MediaStreamDescriptor& descriptor = *web_stream;
  auto audio_components = descriptor.AudioComponents();
  if (audio_components.empty()) {
    // The stream contains no audio tracks. Log error message if the stream
    // contains no video tracks either. Without this extra check, video-only
    // streams would generate error messages at this stage and we want to
    // avoid that.
    auto video_tracks = descriptor.VideoComponents();
    if (video_tracks.empty()) {
      SendLogMessage(String::Format(
          "%s => (ERROR: no audio tracks in media stream)", __func__));
    }
    return nullptr;
  }

  // TODO(crbug.com/400764478): We need to fix the data flow so that
  // it works the same way for all track implementations, local, remote or what
  // have you.
  // In this function, we should simply create a renderer object that receives
  // and mixes audio from all the tracks that belong to the media stream.
  // For now, we have separate renderers depending on if the first audio track
  // in the stream is local or remote.
  MediaStreamAudioTrack* audio_track =
      MediaStreamAudioTrack::From(audio_components[0].Get());
  if (!audio_track) {
    // This can happen if the track was cloned.
    // TODO(tommi, perkj): Fix cloning of tracks to handle extra data too.
    SendLogMessage(String::Format(
        "%s => (ERROR: no native track for WebMediaStreamTrack)", __func__));
    return nullptr;
  }

  auto* frame = To<LocalFrame>(WebLocalFrame::ToCoreFrame(*web_frame));
  DCHECK(frame);

  if (PeerConnectionRemoteAudioTrack::From(audio_track)) {
    SendLogMessage(String::Format(
        "%s => (remote WebRTC audio is not available)", __func__));
    return nullptr;
  }

  SendLogMessage(UNSAFE_TODO(String::Format(
      "%s => (creating TrackAudioRenderer for %s audio track)", __func__,
      audio_track->is_local_track() ? "local" : "remote")));

  return base::MakeRefCounted<TrackAudioRenderer>(
      audio_components[0].Get(), *frame, String(device_id),
      std::move(on_render_error_callback));
}

}  // namespace blink
