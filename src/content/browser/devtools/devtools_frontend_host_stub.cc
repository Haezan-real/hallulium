// Copyright 2026 Hallulium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Compiled only when enable_devtools_frontend is false so Electron/content
// still link after //third_party/devtools-frontend is deleted.

#include "content/public/browser/devtools_frontend_host.h"

#include "base/memory/ref_counted_memory.h"
#include "base/notreached.h"

namespace content {

// static
std::unique_ptr<DevToolsFrontendHost> DevToolsFrontendHost::Create(
    RenderFrameHost* frame_host,
    const HandleMessageCallback& handle_message_callback) {
  // Bundled DevTools UI is disabled in hallulium builds.
  return nullptr;
}

// static
void DevToolsFrontendHost::SetupExtensionsAPI(
    RenderFrameHost* frame_host,
    const std::string& extension_api) {
  // No-op: no bundled DevTools frontend.
}

// static
scoped_refptr<base::RefCountedMemory>
DevToolsFrontendHost::GetFrontendResourceBytes(std::string_view path) {
  return nullptr;
}

// static
std::string DevToolsFrontendHost::GetFrontendResource(std::string_view path) {
  return std::string();
}

}  // namespace content
