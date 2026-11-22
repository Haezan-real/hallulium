#include "third_party/blink/renderer/modules/mediastream/screen_capture_media_stream_track.h"

#include "base/functional/callback_helpers.h"

namespace blink {

ScreenCaptureMediaStreamTrack::ScreenCaptureMediaStreamTrack(
    ExecutionContext* context,
    MediaStreamComponent* component)
    : MediaStreamTrackImpl(context,
                           component,
                           component->Source()->GetReadyState(),
                           /*callback=*/base::DoNothing()) {}

}  // namespace blink
