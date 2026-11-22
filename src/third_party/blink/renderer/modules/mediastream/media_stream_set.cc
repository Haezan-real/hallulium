// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/mediastream/media_stream_set.h"

#include "base/functional/bind.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/modules/mediastream/media_stream.h"
#include "third_party/blink/renderer/modules/mediastream/screen_capture_media_stream_track.h"
#include "third_party/blink/renderer/modules/mediastream/user_media_request.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "third_party/blink/renderer/platform/wtf/vector.h"
#include "third_party/blink/renderer/platform/wtf/wtf_size_t.h"

namespace blink {

MediaStreamSet* MediaStreamSet::Create(
    ExecutionContext* context,
    const GCedMediaStreamDescriptorVector& stream_descriptors,
    UserMediaRequestType request_type,
    MediaStreamSetInitializedCallback callback) {
  DCHECK(IsMainThread());

  return MakeGarbageCollected<MediaStreamSet>(
      context, stream_descriptors, request_type, std::move(callback));
}

MediaStreamSet::MediaStreamSet(
    ExecutionContext* context,
    const GCedMediaStreamDescriptorVector& stream_descriptors,
    UserMediaRequestType request_type,
    MediaStreamSetInitializedCallback callback)
    : ExecutionContextClient(context),
      media_streams_to_initialize_count_(stream_descriptors.size()),
      media_streams_initialized_callback_(std::move(callback)) {
  DCHECK(IsMainThread());

  if (request_type == UserMediaRequestType::kAllScreensMedia) {
    InitializeGetAllScreensMediaStreams(context, stream_descriptors);
    return;
  }

  if (stream_descriptors.empty()) {
    // No streams -> all streams are initialized, meaning the set
    // itself is fully initialized.
    context->GetTaskRunner(TaskType::kInternalMedia)
        ->PostTask(FROM_HERE,
                   BindOnce(&MediaStreamSet::OnMediaStreamSetInitialized,
                            WrapPersistent(this)));
    return;
  }

  // The set will be initialized when all of its streams are initialized.
  // When the last stream is initialized, its callback will trigger
  // a call to OnMediaStreamSetInitialized.
  for (wtf_size_t stream_index = 0; stream_index < stream_descriptors.size();
       ++stream_index) {
    MediaStream::Create(context, stream_descriptors[stream_index],
                        /*track=*/nullptr,
                        BindOnce(&MediaStreamSet::OnMediaStreamInitialized,
                                 WrapPersistent(this)));
  }
}

void MediaStreamSet::Trace(Visitor* visitor) const {
  visitor->Trace(initialized_media_streams_);
  ExecutionContextClient::Trace(visitor);
}

void MediaStreamSet::InitializeGetAllScreensMediaStreams(
    ExecutionContext* context,
    const GCedMediaStreamDescriptorVector& stream_descriptors) {
  DCHECK(IsMainThread());

  for (wtf_size_t stream_index = 0; stream_index < stream_descriptors.size();
       ++stream_index) {
    MediaStreamDescriptor* const descriptor = stream_descriptors[stream_index];
    DCHECK_EQ(1u, descriptor->NumberOfVideoComponents());

    MediaStreamTrack* video_track =
        MakeGarbageCollected<ScreenCaptureMediaStreamTrack>(
            context, descriptor->VideoComponent(0u));
    initialized_media_streams_.push_back(
        MediaStream::Create(context, descriptor, {}, {video_track}));
  }
  context->GetTaskRunner(TaskType::kInternalMedia)
      ->PostTask(FROM_HERE,
                 BindOnce(&MediaStreamSet::OnMediaStreamSetInitialized,
                          WrapPersistent(this)));
}

void MediaStreamSet::OnMediaStreamSetInitialized() {
  DCHECK(IsMainThread());

  std::move(std::move(media_streams_initialized_callback_))
      .Run(initialized_media_streams_);
}

// TODO(crbug.com/1300883): Clean up other streams if one stream capture
// results in an error. This is only required for getAllScreensMedia.
// Currently existing functionality generates only one stream which is not
// affected by this change.
void MediaStreamSet::OnMediaStreamInitialized(
    MediaStream* initialized_media_stream) {
  DCHECK(IsMainThread());
  DCHECK_LT(initialized_media_streams_.size(),
            media_streams_to_initialize_count_);

  initialized_media_streams_.push_back(initialized_media_stream);
  if (initialized_media_streams_.size() == media_streams_to_initialize_count_) {
    OnMediaStreamSetInitialized();
  }
}

}  // namespace blink
