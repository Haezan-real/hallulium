#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_EXPORTED_WEB_EMBEDDED_WORKER_IMPL_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_EXPORTED_WEB_EMBEDDED_WORKER_IMPL_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "base/task/single_thread_task_runner.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "third_party/blink/public/web/web_embedded_worker.h"
#include "third_party/blink/public/web/web_embedded_worker_start_data.h"
#include "third_party/blink/renderer/modules/modules_export.h"

namespace blink {

class WebServiceWorkerContextClient;

// Service worker threads were removed. This still satisfies the public
// WebEmbeddedWorker entry point so the renderer can construct one; starting it
// does not run a worker.
class MODULES_EXPORT WebEmbeddedWorkerImpl final : public WebEmbeddedWorker {
 public:
  explicit WebEmbeddedWorkerImpl(WebServiceWorkerContextClient*);

  WebEmbeddedWorkerImpl(const WebEmbeddedWorkerImpl&) = delete;
  WebEmbeddedWorkerImpl& operator=(const WebEmbeddedWorkerImpl&) = delete;

  ~WebEmbeddedWorkerImpl() override;

  void StartWorkerContext(
      std::unique_ptr<WebEmbeddedWorkerStartData>,
      std::unique_ptr<WebServiceWorkerInstalledScriptsManagerParams>,
      CrossVariantMojoRemote<mojom::WorkerContentSettingsProxyInterfaceBase>
          content_settings,
      CrossVariantMojoRemote<mojom::CacheStorageInterfaceBase> cache_storage,
      CrossVariantMojoRemote<mojom::BrowserInterfaceBrokerInterfaceBase>
          browser_interface_broker,
      InterfaceRegistry* interface_registry,
      scoped_refptr<base::SingleThreadTaskRunner> initiator_thread_task_runner,
      CrossVariantMojoReceiver<mojom::ReportingObserverInterfaceBase>
          coep_reporting_observer,
      CrossVariantMojoReceiver<mojom::ReportingObserverInterfaceBase>
          dip_reporting_observer) override;
  void TerminateWorkerContext() override;

  void WaitForShutdownForTesting();

 private:
  const raw_ptr<WebServiceWorkerContextClient> worker_context_client_;
  bool asked_to_terminate_ = false;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_EXPORTED_WEB_EMBEDDED_WORKER_IMPL_H_
