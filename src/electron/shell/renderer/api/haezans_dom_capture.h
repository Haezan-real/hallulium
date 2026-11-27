// Copyright (c) 2026 Haezan.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ELECTRON_SHELL_RENDERER_API_HAEZANS_DOM_CAPTURE_H_
#define ELECTRON_SHELL_RENDERER_API_HAEZANS_DOM_CAPTURE_H_

#include "v8/include/v8-local-handle.h"

namespace content {
class RenderFrame;
}

namespace electron {

// Installs globalThis.haezansDomCapture on the main frame's page. The creation
// callback arrives for the preload's isolated world; the page is the main world.
// below(maxZ, callback) asks the compositor for quads at or below maxZ.
// The callback receives an ImageBitmap of each new rectangle. stop() drops it.
void InstallHaezansDomCapture(content::RenderFrame* render_frame,
                              v8::Isolate* isolate,
                              v8::Local<v8::Context> context);

}  // namespace electron

#endif  // ELECTRON_SHELL_RENDERER_API_HAEZANS_DOM_CAPTURE_H_
