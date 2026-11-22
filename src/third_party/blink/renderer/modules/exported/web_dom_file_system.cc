#include "third_party/blink/public/web/web_dom_file_system.h"

#include "base/check.h"
#include "third_party/blink/renderer/core/frame/web_local_frame_impl.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "v8/include/v8.h"

namespace blink {

class DOMFileSystem : public GarbageCollected<DOMFileSystem> {
 public:
  void Trace(Visitor*) const {}
};

WebDOMFileSystem WebDOMFileSystem::FromV8Value(v8::Isolate*,
                                               v8::Local<v8::Value>) {
  return WebDOMFileSystem();
}

WebURL WebDOMFileSystem::CreateFileSystemURL(v8::Isolate*,
                                             v8::Local<v8::Value>) {
  return WebURL();
}

WebDOMFileSystem WebDOMFileSystem::Create(WebLocalFrame*,
                                          WebFileSystemType,
                                          const WebString&,
                                          const WebURL&,
                                          SerializableType) {
  return WebDOMFileSystem();
}

void WebDOMFileSystem::Reset() {
  private_.Reset();
}

void WebDOMFileSystem::Assign(const WebDOMFileSystem& other) {
  private_ = other.private_;
}

WebString WebDOMFileSystem::GetName() const {
  return WebString();
}

WebFileSystemType WebDOMFileSystem::GetType() const {
  return WebFileSystemType::kWebFileSystemTypeTemporary;
}

WebURL WebDOMFileSystem::RootURL() const {
  return WebURL();
}

v8::Local<v8::Value> WebDOMFileSystem::ToV8Value(v8::Isolate*) {
  return v8::Local<v8::Value>();
}

v8::Local<v8::Value> WebDOMFileSystem::CreateV8Entry(const WebString&,
                                                     EntryType,
                                                     v8::Isolate*) {
  return v8::Local<v8::Value>();
}

WebDOMFileSystem::WebDOMFileSystem(DOMFileSystem*) {}

WebDOMFileSystem& WebDOMFileSystem::operator=(DOMFileSystem*) {
  return *this;
}

}  // namespace blink
