#pragma once
// Hallulium hollow
#include <string>
#include <vector>
namespace tflite_support {
class Status {};
template<typename T> class StatusOr { public: bool ok() const { return false; } T value() const { return T(); } };
}
namespace tensorflow_lite_support { namespace cc {
using Status = tflite_support::Status;
template<typename T> using StatusOr = tflite_support::StatusOr<T>;
}}
#define RETURN_IF_ERROR(x) do { auto _s=(x); (void)_s; } while(0)