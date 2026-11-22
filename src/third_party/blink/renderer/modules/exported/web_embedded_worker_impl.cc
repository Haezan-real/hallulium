#include "third_party/blink/renderer/modules/exported/web_embedded_worker_impl.h"

#include "base/check.h"
#include "third_party/blink/public/web/modules/service_worker/web_service_worker_context_client.h"

namespace blink {

WebServiceWorkerInstalledScriptsManagerParams::
    WebServiceWorkerInstalledScriptsManagerParams(
        std::vector<WebURL> installed_scripts_urls,
        CrossVariantMojoReceiver<
            mojom::ServiceWorkerInstalledScriptsManagerInterfaceBase>
            manager_receiver,
        CrossVariantMojoRemote<
            mojom::ServiceWorkerInstalledScriptsManagerHostInterfaceBase>
            manager_host_remote)
    : installed_scripts_urls(std::move(installed_scripts_urls)),
      manager_receiver(std::move(manager_receiver)),
      manager_host_remote(std::move(manager_host_remote)) {
  DCHECK(!this->installed_scripts_urls.empty());
  DCHECK(this->manager_receiver);
  DCHECK(this->manager_host_remote);
}

std::unique_ptr<WebEmbeddedWorker> WebEmbeddedWorker::Create(
    WebServiceWorkerContextClient* client) {
  return std::make_unique<WebEmbeddedWorkerImpl>(client);
}

WebEmbeddedWorkerImpl::WebEmbeddedWorkerImpl(
    WebServiceWorkerContextClient* client)
    : worker_context_client_(client) {}

WebEmbeddedWorkerImpl::~WebEmbeddedWorkerImpl() {
  (void)worker_context_client_;
}

void WebEmbeddedWorkerImpl::StartWorkerContext(
    std::unique_ptr<WebEmbeddedWorkerStartData>,
    std::unique_ptr<WebServiceWorkerInstalledScriptsManagerParams>,
    CrossVariantMojoRemote<mojom::WorkerContentSettingsProxyInterfaceBase>,
    CrossVariantMojoRemote<mojom::CacheStorageInterfaceBase>,
    CrossVariantMojoRemote<mojom::BrowserInterfaceBrokerInterfaceBase>,
    InterfaceRegistry*,
    scoped_refptr<base::SingleThreadTaskRunner>,
    CrossVariantMojoReceiver<mojom::ReportingObserverInterfaceBase>,
    CrossVariantMojoReceiver<mojom::ReportingObserverInterfaceBase>) {}

void WebEmbeddedWorkerImpl::TerminateWorkerContext() {
  asked_to_terminate_ = true;
}

void WebEmbeddedWorkerImpl::WaitForShutdownForTesting() {}

}  // namespace blink
