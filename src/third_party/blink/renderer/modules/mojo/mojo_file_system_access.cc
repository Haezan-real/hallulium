// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/mojo/mojo_file_system_access.h"

namespace blink {

const char MojoFileSystemAccess::kSupplementName[] = "MojoFileSystemAccess";

MojoFileSystemAccess::MojoFileSystemAccess(Mojo& mojo)
    : Supplement<Mojo>(mojo) {}

// static
MojoFileSystemAccess& MojoFileSystemAccess::From(Mojo& mojo) {
  MojoFileSystemAccess* supplement =
      Supplement<Mojo>::From<MojoFileSystemAccess>(mojo);
  if (!supplement) {
    supplement = MakeGarbageCollected<MojoFileSystemAccess>(mojo);
    ProvideTo(mojo, supplement);
  }
  return *supplement;
}

void MojoFileSystemAccess::Trace(Visitor* visitor) const {
  Supplement<Mojo>::Trace(visitor);
}

}  // namespace blink
