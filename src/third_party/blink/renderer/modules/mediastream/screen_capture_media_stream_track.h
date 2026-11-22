// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIASTREAM_SCREEN_CAPTURE_MEDIA_STREAM_TRACK_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIASTREAM_SCREEN_CAPTURE_MEDIA_STREAM_TRACK_H_

#include "build/build_config.h"
#include "third_party/blink/renderer/bindings/core/v8/active_script_wrappable.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/modules/mediastream/media_stream_track_impl.h"
#include "third_party/blink/renderer/modules/mediastream/user_media_request.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/mediastream/media_stream_descriptor.h"

namespace blink {

// Screen-details metadata lived in the deleted screen_details module. Capture
// tracks still exist so getDisplayMedia / getAllScreensMedia can return a
// MediaStreamTrack.
class MODULES_EXPORT ScreenCaptureMediaStreamTrack final
    : public MediaStreamTrackImpl {
  DEFINE_WRAPPERTYPEINFO();

 public:
  ScreenCaptureMediaStreamTrack(ExecutionContext* context,
                                MediaStreamComponent* component);
  ~ScreenCaptureMediaStreamTrack() override = default;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_MEDIASTREAM_SCREEN_CAPTURE_MEDIA_STREAM_TRACK_H_
