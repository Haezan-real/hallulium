// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/modules_initializer.h"

#include <memory>

#include "base/task/thread_pool.h"
#include "base/trace_event/trace_event.h"
#include "build/android_buildflags.h"
#include "build/build_config.h"
#include "mojo/public/cpp/bindings/binder_map.h"
#include "third_party/blink/public/mojom/dom_storage/session_storage_namespace.mojom-blink.h"
#include "third_party/blink/public/platform/interface_registry.h"
#include "third_party/blink/public/platform/web_security_origin.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/web/web_local_frame_client.h"
#include "third_party/blink/public/web/web_view_client.h"
#include "third_party/blink/renderer/bindings/modules/v8/module_bindings_initializer.h"
#include "third_party/blink/renderer/core/css/background_color_paint_image_generator.h"
#include "third_party/blink/renderer/core/css/clip_path_paint_image_generator.h"
#include "third_party/blink/renderer/core/css/css_paint_image_generator.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/editing/suggestion/text_suggestion_backend_impl.h"
#include "third_party/blink/renderer/core/event_type_names.h"
#include "third_party/blink/renderer/core/exported/web_shared_worker_impl.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/frame/settings.h"
#include "third_party/blink/renderer/core/frame/web_local_frame_impl.h"
#include "third_party/blink/renderer/core/html/canvas/html_canvas_element.h"
#include "third_party/blink/renderer/core/html/media/html_media_element.h"
#include "third_party/blink/renderer/core/inspector/devtools_session.h"
#include "third_party/blink/renderer/core/inspector/inspector_media_context_impl.h"
#include "third_party/blink/renderer/core/loader/document_loader.h"
#include "third_party/blink/renderer/core/offscreencanvas/offscreen_canvas.h"
#include "third_party/blink/renderer/core/page/chrome_client.h"
#include "third_party/blink/renderer/core/page/page.h"
#include "third_party/blink/renderer/core/scheduler/task_attribution_tracker_impl.h"
#include "third_party/blink/renderer/modules/accessibility/ax_object_cache_impl.h"
#include "third_party/blink/renderer/modules/accessibility/inspector_accessibility_agent.h"
#include "third_party/blink/renderer/modules/audio_output_devices/html_media_element_audio_output_device.h"
#include "third_party/blink/renderer/modules/canvas/canvas2d/canvas_rendering_context_2d.h"
#include "third_party/blink/renderer/modules/canvas/imagebitmap/image_bitmap_rendering_context.h"
#include "third_party/blink/renderer/modules/canvas/offscreencanvas2d/offscreen_canvas_rendering_context_2d.h"
#include "third_party/blink/renderer/modules/encryptedmedia/html_media_element_encrypted_media.h"
#include "third_party/blink/renderer/modules/event_interface_modules_names.h"
#include "third_party/blink/renderer/modules/event_modules_factory.h"
#include "third_party/blink/renderer/modules/event_target_modules_names.h"
#include "third_party/blink/renderer/modules/image_downloader/image_downloader_impl.h"
#include "third_party/blink/renderer/modules/indexed_db_names.h"
#include "third_party/blink/renderer/modules/indexeddb/inspector_indexed_db_agent.h"
#include "third_party/blink/renderer/modules/manifest/manifest_manager.h"
#include "third_party/blink/renderer/modules/media/audio/audio_renderer_sink_cache.h"
#include "third_party/blink/renderer/modules/media_controls/media_controls_impl.h"
#include "third_party/blink/renderer/modules/mediasource/media_source_registry_impl.h"
#include "third_party/blink/renderer/modules/mediastream/user_media_request_provider_impl.h"
#include "third_party/blink/renderer/modules/screen_orientation/screen_orientation_controller.h"
#include "third_party/blink/renderer/modules/storage/dom_window_storage.h"
#include "third_party/blink/renderer/modules/storage/dom_window_storage_controller.h"
#include "third_party/blink/renderer/modules/storage/inspector_dom_storage_agent.h"
#include "third_party/blink/renderer/modules/storage/storage_namespace.h"
#include "third_party/blink/renderer/modules/webaudio/audio_graph_tracer.h"
#include "third_party/blink/renderer/modules/webaudio/inspector_web_audio_agent.h"
#include "third_party/blink/renderer/modules/webgpu/gpu_canvas_context.h"
#include "third_party/blink/renderer/modules/worklet/animation_and_paint_worklet_thread.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "third_party/blink/renderer/platform/heap/persistent.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"
#include "third_party/blink/renderer/platform/scheduler/public/thread_scheduler.h"
#include "third_party/blink/renderer/platform/widget/compositing/blink_categorized_worker_pool_delegate.h"
#include "third_party/blink/renderer/platform/widget/frame_widget.h"
#include "third_party/blink/renderer/platform/wtf/cross_thread_functional.h"
#include "third_party/blink/renderer/platform/wtf/functional.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "ui/accessibility/accessibility_features.h"

#if BUILDFLAG(IS_ANDROID)
#include "third_party/blink/public/platform/modules/video_capture/web_video_capture_impl_manager.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/public/web/modules/mediastream/web_media_stream_device_observer.h"
#include "third_party/blink/renderer/core/page/page_visibility_observer.h"
#include "third_party/blink/renderer/modules/remote_objects/remote_object_gateway_impl.h"
#endif

namespace blink {
namespace {

#if BUILDFLAG(IS_ANDROID) && !BUILDFLAG(IS_DESKTOP_ANDROID)

class SuspendCaptureObserver : public GarbageCollected<SuspendCaptureObserver>,
                               public Supplement<Page>,
                               public PageVisibilityObserver {
 public:
  static const char kSupplementName[];

  explicit SuspendCaptureObserver(Page& page)
      : Supplement<Page>(page), PageVisibilityObserver(&page) {}

  // PageVisibilityObserver overrides:
  void PageVisibilityChanged() override {
    // TODO(crbug.com/487935): We don't yet suspend video capture devices for
    // OOPIFs.
    WebLocalFrameImpl* frame = WebLocalFrameImpl::FromFrame(
        DynamicTo<LocalFrame>(GetPage()->MainFrame()));
    if (!frame)
      return;
    WebMediaStreamDeviceObserver* media_stream_device_observer =
        frame->Client()->MediaStreamDeviceObserver();
    if (!media_stream_device_observer)
      return;
    // Don't suspend media capture devices if page visibility is
    // PageVisibilityState::kHiddenButPainting (e.g. Picture-in-Picture).
    // TODO(crbug.com/1339252): Add tests.
    bool suspend = (GetPage()->GetVisibilityState() ==
                    mojom::blink::PageVisibilityState::kHidden);
    MediaStreamDevices video_devices =
        media_stream_device_observer->GetNonScreenCaptureDevices();
    Platform::Current()->GetVideoCaptureImplManager()->SuspendDevices(
        video_devices, suspend);
  }

  void Trace(Visitor* visitor) const override {
    Supplement<Page>::Trace(visitor);
    PageVisibilityObserver::Trace(visitor);
  }
};

const char SuspendCaptureObserver::kSupplementName[] = "SuspendCaptureObserver";
#endif  // BUILDFLAG(IS_ANDROID) && !BUILDFLAG(IS_DESKTOP_ANDROID)

}  // namespace

void ModulesInitializer::Initialize() {
  // Strings must be initialized before calling CoreInitializer::init().
  const unsigned kModulesStaticStringsCount =
      event_interface_names::kModulesNamesCount +
      event_target_names::kModulesNamesCount + indexed_db_names::kNamesCount;
  StringImpl::ReserveStaticStringsCapacityForSize(
      kModulesStaticStringsCount + StringImpl::AllStaticStrings().size());

  event_interface_names::InitModules();
  event_target_names::InitModules();
  Document::RegisterEventFactory(EventModulesFactory::Create());
  ModuleBindingsInitializer::Init();
  indexed_db_names::Init();
  AXObjectCache::Init(AXObjectCacheImpl::Create);
  MediaSourceRegistryImpl::Init();

  CoreInitializer::Initialize();

  // Canvas context types must be registered with the HTMLCanvasElement.
  HTMLCanvasElement::RegisterRenderingContextFactory(
      std::make_unique<CanvasRenderingContext2D::Factory>());
  HTMLCanvasElement::RegisterRenderingContextFactory(
      std::make_unique<ImageBitmapRenderingContext::Factory>());
  HTMLCanvasElement::RegisterRenderingContextFactory(
      std::make_unique<GPUCanvasContext::Factory>());

  // OffscreenCanvas context types must be registered with the OffscreenCanvas.
  OffscreenCanvas::RegisterRenderingContextFactory(
      std::make_unique<OffscreenCanvasRenderingContext2D::Factory>());
  OffscreenCanvas::RegisterRenderingContextFactory(
      std::make_unique<ImageBitmapRenderingContext::Factory>());
  OffscreenCanvas::RegisterRenderingContextFactory(
      std::make_unique<GPUCanvasContext::Factory>());

  V8PerIsolateData::SetTaskAttributionTrackerFactory(
      &scheduler::TaskAttributionTrackerImpl::Create);
}

void ModulesInitializer::InitLocalFrame(LocalFrame& frame) const {
  frame.GetInterfaceRegistry()->AddInterface(BindRepeating(
      &TextSuggestionBackendImpl::Bind, WrapWeakPersistent(&frame)));
}

void ModulesInitializer::InstallSupplements(LocalFrame& frame) const {
  DCHECK(WebLocalFrameImpl::FromFrame(&frame)->Client());
  InspectorAccessibilityAgent::ProvideTo(&frame);
  ImageDownloaderImpl::ProvideTo(frame);
  AudioRendererSinkCache::InstallWindowObserver(*frame.DomWindow());
  UserMediaRequestProviderImpl::ProvideTo(*frame.DomWindow());
}

MediaControls* ModulesInitializer::CreateMediaControls(
    HTMLMediaElement& media_element,
    ShadowRoot& shadow_root) const {
  return MediaControlsImpl::Create(media_element, shadow_root);
}

PictureInPictureController*
ModulesInitializer::CreatePictureInPictureController(Document&) const {
  return nullptr;
}

void ModulesInitializer::InitInspectorAgentSession(
    DevToolsSession* session,
    InspectorDOMAgent* dom_agent,
    InspectedFrames* inspected_frames,
    Page* page) const {
  session->CreateAndAppend<InspectorIndexedDBAgent>(
      inspected_frames, /*worker_global_scope=*/nullptr, session->V8Session());
  session->CreateAndAppend<InspectorDOMStorageAgent>(inspected_frames);
  session->CreateAndAppend<InspectorAccessibilityAgent>(inspected_frames,
                                                        dom_agent);
  session->CreateAndAppend<InspectorWebAudioAgent>(page);
}

void ModulesInitializer::InitWorkerInspectorAgentSession(
    DevToolsSession* session,
    WorkerGlobalScope* worker_global_scope) const {
  session->CreateAndAppend<InspectorIndexedDBAgent>(
      /*inspected_frames=*/nullptr, worker_global_scope, session->V8Session());
}

void ModulesInitializer::OnClearWindowObjectInMainWorld(
    Document& document,
    const Settings& settings) const {
  LocalDOMWindow& window = *document.domWindow();
  DOMWindowStorageController::From(window);
  ManifestManager::From(window);
  (void)settings;

#if BUILDFLAG(IS_ANDROID)
  LocalFrame* frame = window.GetFrame();
  DCHECK(frame);
  if (auto* gateway = RemoteObjectGatewayImpl::From(*frame))
    gateway->OnClearWindowObjectInMainWorld();
#endif  // BUILDFLAG(IS_ANDROID)
}

std::unique_ptr<WebMediaPlayer> ModulesInitializer::CreateWebMediaPlayer(
    WebLocalFrameClient* web_frame_client,
    HTMLMediaElement& html_media_element,
    const WebMediaPlayerSource& source,
    WebMediaPlayerClient* media_player_client) const {
  HTMLMediaElementEncryptedMedia& encrypted_media =
      HTMLMediaElementEncryptedMedia::From(html_media_element);
  WebString sink_id(
      HTMLMediaElementAudioOutputDevice::sinkId(html_media_element));
  MediaInspectorContextImpl* context_impl = MediaInspectorContextImpl::From(
      *To<LocalDOMWindow>(html_media_element.GetExecutionContextForPlayer()));
  FrameWidget* frame_widget =
      html_media_element.GetDocument().GetFrame()->GetWidgetForLocalRoot();
  return web_frame_client->CreateMediaPlayer(
      source, media_player_client, context_impl, &encrypted_media,
      encrypted_media.ContentDecryptionModule(), sink_id,
      frame_widget->GetLayerTreeSettings(),
      base::ThreadPool::CreateTaskRunner(base::TaskTraits{
          base::InheritThreadType(),
          base::MaxThreadType(base::ThreadType::kPresentation)}));
}

RemotePlaybackClient* ModulesInitializer::CreateRemotePlaybackClient(
    HTMLMediaElement&) const {
  return nullptr;
}

void ModulesInitializer::ProvideModulesToPage(
    Page& page,
    const SessionStorageNamespaceId& namespace_id) const {
  StorageNamespace::ProvideSessionStorageNamespaceTo(page, namespace_id);
  AudioGraphTracer::ProvideAudioGraphTracerTo(page);
#if BUILDFLAG(IS_ANDROID) && !BUILDFLAG(IS_DESKTOP_ANDROID)
  page.ProvideSupplement(MakeGarbageCollected<SuspendCaptureObserver>(page));
#endif  // BUILDFLAG(IS_ANDROID)  && !BUILDFLAG(IS_DESKTOP_ANDROID)
}

void ModulesInitializer::ForceNextWebGLContextCreationToFail() const {}

void ModulesInitializer::
    CollectAllGarbageForAnimationAndPaintWorkletForTesting() const {
  AnimationAndPaintWorkletThread::CollectAllGarbageForTesting();
}

void ModulesInitializer::CloneSessionStorage(
    Page* clone_from_page,
    const SessionStorageNamespaceId& clone_to_namespace) {
  StorageNamespace* storage_namespace = StorageNamespace::From(clone_from_page);
  if (storage_namespace)
    storage_namespace->CloneTo(WebString::FromLatin1(clone_to_namespace));
}

void ModulesInitializer::EvictSessionStorageCachedData(Page* page) {
  StorageNamespace* storage_namespace = StorageNamespace::From(page);
  if (storage_namespace)
    storage_namespace->EvictSessionStorageCachedData();
}

void ModulesInitializer::DidChangeManifest(LocalFrame& frame) {
  ManifestManager::From(*frame.DomWindow())->DidChangeManifest();
}

void ModulesInitializer::NotifyOrientationChanged(LocalFrame& frame) {
  ScreenOrientationController::From(*frame.DomWindow())
      ->NotifyOrientationChanged();
}

void ModulesInitializer::DidUpdateScreens(LocalFrame&,
                                          const display::ScreenInfos&) {}

void ModulesInitializer::SetLocalStorageArea(
    LocalFrame& frame,
    mojo::PendingRemote<mojom::blink::StorageArea> local_storage_area) {
  if (!frame.DomWindow())
    return;
  DOMWindowStorage::From(*frame.DomWindow())
      .InitLocalStorage(std::move(local_storage_area));
}

void ModulesInitializer::SetSessionStorageArea(
    LocalFrame& frame,
    mojo::PendingRemote<mojom::blink::StorageArea> session_storage_area) {
  if (!frame.DomWindow())
    return;
  DOMWindowStorage::From(*frame.DomWindow())
      .InitSessionStorage(std::move(session_storage_area));
}

}  // namespace blink
