#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_EXPORTED_WEB_SHARED_STORAGE_WORKLET_THREAD_IMPL_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_EXPORTED_WEB_SHARED_STORAGE_WORKLET_THREAD_IMPL_H_

#include "third_party/blink/public/web/web_shared_storage_worklet_thread.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/heap/self_keep_alive.h"

namespace blink {

class MODULES_EXPORT WebSharedStorageWorkletThreadImpl final
    : public GarbageCollected<WebSharedStorageWorkletThreadImpl>,
      public WebSharedStorageWorkletThread {
 public:
  explicit WebSharedStorageWorkletThreadImpl(
      scoped_refptr<base::SingleThreadTaskRunner> main_thread_runner);

  ~WebSharedStorageWorkletThreadImpl() override;

  void Trace(Visitor*) const;

 private:
  scoped_refptr<base::SingleThreadTaskRunner> main_thread_runner_;
  SelfKeepAlive<WebSharedStorageWorkletThreadImpl> keep_alive_{this};
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_EXPORTED_WEB_SHARED_STORAGE_WORKLET_THREAD_IMPL_H_
