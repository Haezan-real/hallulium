#ifndef MEDIA_WEBRTC_HALLULIUM_TFLITE_SHIM_H_
#define MEDIA_WEBRTC_HALLULIUM_TFLITE_SHIM_H_

#include <memory>

namespace tflite {
class FlatBufferModel {
 public:
  FlatBufferModel() = default;
  static std::unique_ptr<FlatBufferModel> VerifyAndBuildFromBuffer(
      const char*,
      size_t) {
    return nullptr;
  }
};
class FlatBufferModelBase {};
}  // namespace tflite

namespace optimization_guide {
class TFLiteOpResolver {};
}

#endif  // MEDIA_WEBRTC_HALLULIUM_TFLITE_SHIM_H_
