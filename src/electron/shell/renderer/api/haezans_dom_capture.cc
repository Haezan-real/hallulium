// Copyright (c) 2026 Haezan.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "shell/renderer/api/haezans_dom_capture.h"

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "base/no_destructor.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "gpu/command_buffer/client/client_shared_image.h"
#include "gpu/command_buffer/common/sync_token.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "shell/common/api/api.mojom.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/renderer/core/imagebitmap/image_bitmap.h"  // nogncheck
#include "third_party/blink/renderer/platform/bindings/script_state.h"  // nogncheck
#include "third_party/blink/renderer/platform/graphics/accelerated_static_bitmap_image.h"  // nogncheck
#include "third_party/blink/renderer/platform/graphics/static_bitmap_image.h"  // nogncheck
#include "third_party/skia/include/core/SkAlphaType.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "ui/gfx/geometry/rect.h"
#include "v8/include/v8-context.h"
#include "v8/include/v8-external.h"
#include "v8/include/v8-function.h"
#include "v8/include/v8-isolate.h"
#include "v8/include/v8-object.h"
#include "v8/include/v8-primitive.h"

namespace electron {
namespace {

class HaezansHost;
std::map<content::RenderFrame*, HaezansHost*>& HaezansHosts();

// The append call's pipe is dropped by the browser. The host keeps this until Stop().
class DroppedClient : public mojom::HaezansDomCaptureClient {
 public:
  mojo::PendingRemote<mojom::HaezansDomCaptureClient> Bind() {
    return receiver_.BindNewPipeAndPassRemote();
  }

  void OnRegion(uint32_t,
                const gfx::Rect&,
                gpu::ExportedSharedImage,
                const gpu::SyncToken&) override {}

  void OnRegionPixels(const gfx::Rect&, const std::vector<uint8_t>&) override {}

 private:
  mojo::Receiver<mojom::HaezansDomCaptureClient> receiver_{this};
};

class HaezansHost : public content::RenderFrameObserver,
                    public mojom::HaezansDomCaptureClient {
 public:
  explicit HaezansHost(content::RenderFrame* render_frame)
      : content::RenderFrameObserver(render_frame) {}

  void Attach(v8::Isolate* isolate, v8::Local<v8::Context> context) {
    v8::Context::Scope scope(context);
    v8::Local<v8::Object> api = v8::Object::New(isolate);
    v8::Local<v8::Function> below =
        v8::Function::New(context, &HaezansHost::Below,
                          v8::External::New(isolate, this,
                                            v8::kExternalPointerTypeTagDefault))
            .ToLocalChecked();
    v8::Local<v8::Function> stop =
        v8::Function::New(context, &HaezansHost::Stop,
                          v8::External::New(isolate, this,
                                            v8::kExternalPointerTypeTagDefault))
            .ToLocalChecked();
    api->Set(context, v8::String::NewFromUtf8Literal(isolate, "below"), below)
        .Check();
    api->Set(context, v8::String::NewFromUtf8Literal(isolate, "stop"), stop)
        .Check();
    api->Set(context, v8::String::NewFromUtf8Literal(isolate, "layers"),
             v8::True(isolate))
        .Check();
    api->Set(context, v8::String::NewFromUtf8Literal(isolate, "append"),
             v8::True(isolate))
        .Check();
    context->Global()
        ->Set(context,
              v8::String::NewFromUtf8Literal(isolate, "haezansDomCapture"),
              api)
        .Check();
  }

  void OnRegion(uint32_t token,
                const gfx::Rect& rect,
                gpu::ExportedSharedImage image,
                const gpu::SyncToken& sync_token) override {
    if (callback_.IsEmpty() || context_.IsEmpty()) {
      ReleaseCapture(token, gpu::SyncToken());
      return;
    }
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    if (!isolate) {
      ReleaseCapture(token, gpu::SyncToken());
      return;
    }
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = context_.Get(isolate);
    v8::Context::Scope context_scope(context);
    v8::TryCatch try_catch(isolate);

    std::shared_ptr<bool> alive = alive_;
    scoped_refptr<blink::AcceleratedStaticBitmapImage> accelerated =
        blink::AcceleratedStaticBitmapImage::CreateFromExternalSharedImage(
            std::move(image), sync_token, kPremul_SkAlphaType,
            base::BindOnce(
                [](std::shared_ptr<bool> alive, HaezansHost* host,
                   uint32_t token, const gpu::SyncToken& release_token) {
                  if (!alive || !*alive)
                    return;
                  host->ReleaseCapture(token, release_token);
                },
                alive, this, token),
            /*verify_release=*/false);
    if (!accelerated) {
      ReleaseCapture(token, gpu::SyncToken());
      return;
    }

    blink::ScriptState* script_state =
        blink::ScriptState::MaybeFrom(isolate, context);
    if (!script_state)
      return;
    auto* bitmap = blink::MakeGarbageCollected<blink::ImageBitmap>(
        scoped_refptr<blink::StaticBitmapImage>(std::move(accelerated)));
    v8::Local<v8::Value> wrapped = bitmap->ToV8(script_state);
    if (wrapped.IsEmpty())
      return;
    v8::Local<v8::Value> args[] = {
        v8::Integer::New(isolate, rect.x()),
        v8::Integer::New(isolate, rect.y()),
        v8::Integer::New(isolate, rect.width()),
        v8::Integer::New(isolate, rect.height()),
        wrapped,
        v8::Integer::New(isolate, static_cast<int32_t>(token >> 16)),
    };
    std::ignore =
        callback_.Get(isolate)->Call(context, context->Global(), 6, args);
  }

  void OnRegionPixels(const gfx::Rect& rect,
                      const std::vector<uint8_t>& pixels) override {
    const size_t need = static_cast<size_t>(rect.width()) * rect.height() * 4;
    if (rect.IsEmpty() || pixels.size() < need || callback_.IsEmpty() ||
        context_.IsEmpty()) {
      return;
    }
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    if (!isolate)
      return;
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = context_.Get(isolate);
    v8::Context::Scope context_scope(context);
    v8::TryCatch try_catch(isolate);

    SkImageInfo info =
        SkImageInfo::Make(rect.width(), rect.height(), kRGBA_8888_SkColorType,
                          kPremul_SkAlphaType);
    sk_sp<SkData> data = SkData::MakeWithCopy(pixels.data(), need);
    scoped_refptr<blink::StaticBitmapImage> image =
        blink::StaticBitmapImage::Create(std::move(data), info);
    if (!image)
      return;
    blink::ScriptState* script_state =
        blink::ScriptState::MaybeFrom(isolate, context);
    if (!script_state)
      return;
    auto* bitmap = blink::MakeGarbageCollected<blink::ImageBitmap>(
        std::move(image));
    v8::Local<v8::Value> wrapped = bitmap->ToV8(script_state);
    if (wrapped.IsEmpty())
      return;
    v8::Local<v8::Value> args[] = {
        v8::Integer::New(isolate, rect.x()),
        v8::Integer::New(isolate, rect.y()),
        v8::Integer::New(isolate, rect.width()),
        v8::Integer::New(isolate, rect.height()),
        wrapped,
        v8::Integer::New(isolate, 0),
    };
    std::ignore =
        callback_.Get(isolate)->Call(context, context->Global(), 6, args);
  }

  void ReleaseCapture(uint32_t token, const gpu::SyncToken& sync_token) {
    if (!ipc_.is_bound())
      return;
    // An unverified token fails mojo validation and kills the renderer.
    gpu::SyncToken verified = sync_token;
    if (verified.HasData() && !verified.verified_flush())
      verified.SetVerifyFlush();
    ipc_->ReleaseHaezansCapture(token, verified);
  }

 private:
  static void Below(const v8::FunctionCallbackInfo<v8::Value>& info) {
    v8::Isolate* isolate = info.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    auto* host = static_cast<HaezansHost*>(
        info.Data().As<v8::External>()->Value(
            v8::kExternalPointerTypeTagDefault));
    if (info.Length() < 2 || !info[1]->IsFunction())
      return;
    std::vector<int32_t> cutoffs;
    if (info[0]->IsArray()) {
      v8::Local<v8::Array> array = info[0].As<v8::Array>();
      for (uint32_t i = 0; i < array->Length(); ++i) {
        v8::Local<v8::Value> item;
        if (!array->Get(context, i).ToLocal(&item))
          continue;
        cutoffs.push_back(item->Int32Value(context).FromMaybe(0));
      }
    } else {
      cutoffs.push_back(info[0]->Int32Value(context).FromMaybe(0));
    }
    if (cutoffs.empty())
      return;
    host->callback_.Reset(isolate, info[1].As<v8::Function>());
    host->context_.Reset(isolate, context);
    if (!host->ipc_.is_bound()) {
      host->render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(
          &host->ipc_);
    }
    // One client. Extra cutoffs are negative z on that same pipe. A second
    // Bind() here used to replace the client and kill the renderer.
    if (!host->receiver_.is_bound()) {
      mojo::PendingRemote<mojom::HaezansDomCaptureClient> remote;
      host->receiver_.Bind(remote.InitWithNewPipeAndPassReceiver());
      host->sent_cutoffs_.clear();
      int32_t first = cutoffs[0] < 0 ? -cutoffs[0] : cutoffs[0];
      host->ipc_->SetHaezansDomCapture(true, first, std::move(remote));
      if (first != 0)
        host->sent_cutoffs_.insert(first);
      for (size_t i = 1; i < cutoffs.size(); ++i)
        host->AppendCutoff(cutoffs[i]);
      return;
    }
    for (int32_t z : cutoffs)
      host->AppendCutoff(z);
  }

  void AppendCutoff(int32_t z) {
    z = z < 0 ? -z : z;
    if (z == 0 || !sent_cutoffs_.insert(z).second || !ipc_.is_bound())
      return;
    // The browser ignores this client when max_z is negative and keeps the
    // first callback. The remote has to stay valid until that IPC is sent.
    auto dropped = std::make_unique<DroppedClient>();
    ipc_->SetHaezansDomCapture(true, -z, dropped->Bind());
    dropped_clients_.push_back(std::move(dropped));
  }

  static void Stop(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* host = static_cast<HaezansHost*>(
        info.Data().As<v8::External>()->Value(
            v8::kExternalPointerTypeTagDefault));
    host->callback_.Reset();
    host->context_.Reset();
    host->sent_cutoffs_.clear();
    host->dropped_clients_.clear();
    if (!host->ipc_.is_bound())
      return;
    host->receiver_.reset();
    mojo::PendingRemote<mojom::HaezansDomCaptureClient> remote;
    host->receiver_.Bind(remote.InitWithNewPipeAndPassReceiver());
    host->ipc_->SetHaezansDomCapture(false, 0, std::move(remote));
    host->receiver_.reset();
  }

  void WillReleaseScriptContext(v8::Isolate* isolate,
                                v8::Local<v8::Context> context,
                                int32_t world_id) override {
    if (!context_.IsEmpty() && context == context_.Get(isolate)) {
      callback_.Reset();
      context_.Reset();
    }
  }

  void OnDestruct() override {
    *alive_ = false;
    HaezansHosts().erase(render_frame());
    delete this;
  }

  std::shared_ptr<bool> alive_ = std::make_shared<bool>(true);
  mojo::AssociatedRemote<mojom::ElectronApiIPC> ipc_;
  mojo::Receiver<mojom::HaezansDomCaptureClient> receiver_{this};
  std::vector<std::unique_ptr<DroppedClient>> dropped_clients_;
  std::set<int32_t> sent_cutoffs_;
  v8::Global<v8::Function> callback_;
  v8::Global<v8::Context> context_;
};

std::map<content::RenderFrame*, HaezansHost*>& HaezansHosts() {
  static base::NoDestructor<std::map<content::RenderFrame*, HaezansHost*>>
      hosts;
  return *hosts;
}

}  // namespace

void InstallHaezansDomCapture(content::RenderFrame* render_frame,
                              v8::Isolate* isolate,
                              v8::Local<v8::Context> context) {
  if (!render_frame || !render_frame->IsMainFrame())
    return;
  blink::WebLocalFrame* web_frame = render_frame->GetWebFrame();
  if (!web_frame)
    return;
  // DidCreateScriptContext arrives for the isolated world when the page is
  // isolated from the preload. The page itself is the main world, and that
  // context already exists by the time the isolated one is created.
  v8::Local<v8::Context> page = web_frame->MainWorldScriptContext();
  if (page.IsEmpty())
    return;
  context = page;
  auto& hosts = HaezansHosts();
  HaezansHost* host = nullptr;
  auto it = hosts.find(render_frame);
  if (it == hosts.end()) {
    host = new HaezansHost(render_frame);
    hosts[render_frame] = host;
  } else {
    host = it->second;
  }
  host->Attach(isolate, context);
}

}  // namespace electron
