#include "third_party/blink/public/mojom/shared_storage/shared_storage_worklet_service.mojom.h"
#include "third_party/blink/public/mojom/worker/worklet_global_scope_creation_params.mojom.h"
#include "third_party/blink/renderer/modules/exported/web_shared_storage_worklet_thread_impl.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"

namespace blink {

void WebSharedStorageWorkletThread::Start(
    scoped_refptr<base::SingleThreadTaskRunner>,
    CrossVariantMojoReceiver<
        mojom::SharedStorageWorkletServiceInterfaceBase>,
    mojom::WorkletGlobalScopeCreationParamsPtr) {}

WebSharedStorageWorkletThreadImpl::WebSharedStorageWorkletThreadImpl(
    scoped_refptr<base::SingleThreadTaskRunner> main_thread_runner)
    : main_thread_runner_(std::move(main_thread_runner)) {}

WebSharedStorageWorkletThreadImpl::~WebSharedStorageWorkletThreadImpl() =
    default;

void WebSharedStorageWorkletThreadImpl::Trace(Visitor* visitor) const {
  (void)visitor;
}

}  // namespace blink
