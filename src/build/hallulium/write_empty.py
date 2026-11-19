#!/usr/bin/env python3
import os, sys

OVERLAY_GRD = """<?xml version="1.0" encoding="utf-8"?>
<grit latest_public_release="0" current_release="1">
  <outputs>
    <output filename="grit/inspector_overlay_resources_map.h" type="resource_map_header" />
    <output filename="inspector_overlay_resources.pak" type="data_package" />
  </outputs>
  <release seq="1">
  </release>
</grit>
"""

MAP_H = """#ifndef THIRD_PARTY_BLINK_PUBLIC_RESOURCES_GRIT_INSPECTOR_OVERLAY_RESOURCES_MAP_H_
#define THIRD_PARTY_BLINK_PUBLIC_RESOURCES_GRIT_INSPECTOR_OVERLAY_RESOURCES_MAP_H_
#include "third_party/blink/public/resources/grit/inspector_overlay_resources.h"
#endif
"""

RES_H = """#ifndef THIRD_PARTY_BLINK_PUBLIC_RESOURCES_GRIT_INSPECTOR_OVERLAY_RESOURCES_H_
#define THIRD_PARTY_BLINK_PUBLIC_RESOURCES_GRIT_INSPECTOR_OVERLAY_RESOURCES_H_
#include <stddef.h>
enum {
  IDR_INSPECT_TOOL_MAIN_JS = 30000,
};
#endif
"""

DEVTOOLS_H = """#ifndef CONTENT_BROWSER_DEVTOOLS_GRIT_DEVTOOLS_RESOURCES_H_
#define CONTENT_BROWSER_DEVTOOLS_GRIT_DEVTOOLS_RESOURCES_H_
#include <stddef.h>
enum {
  COMPRESSED_PROTOCOL_JSON = 1,
};
#endif
"""

DEVTOOLS_MAP_H = """#ifndef CONTENT_BROWSER_DEVTOOLS_GRIT_DEVTOOLS_RESOURCES_MAP_H_
#define CONTENT_BROWSER_DEVTOOLS_GRIT_DEVTOOLS_RESOURCES_MAP_H_
#include "ui/base/webui/resource_path.h"
extern const webui::ResourcePath kDevtoolsResources[1];
#endif
"""

DEVTOOLS_MAP_CC = """#include "content/browser/devtools/grit/devtools_resources_map.h"
#include "content/browser/devtools/grit/devtools_resources.h"

const webui::ResourcePath kDevtoolsResources[1] = {
  {"__hallulium_empty__", COMPRESSED_PROTOCOL_JSON},
};
"""


def main():
  for p in sys.argv[1:]:
    os.makedirs(os.path.dirname(p) or ".", exist_ok=True)
    norm = p.replace("\\\\", "/").replace("\\", "/")
    base = os.path.basename(norm)
    if p.endswith(".grd"):
      open(p, "w", encoding="utf-8", newline="\n").write(OVERLAY_GRD)
    elif base == "devtools_resources.h":
      open(p, "w", encoding="utf-8", newline="\n").write(DEVTOOLS_H)
    elif base == "devtools_resources_map.h":
      open(p, "w", encoding="utf-8", newline="\n").write(DEVTOOLS_MAP_H)
    elif base == "devtools_resources_map.cc":
      open(p, "w", encoding="utf-8", newline="\n").write(DEVTOOLS_MAP_CC)
    elif p.endswith("inspector_overlay_resources_map.h"):
      open(p, "w", encoding="utf-8", newline="\n").write(MAP_H)
      sib = p.replace("inspector_overlay_resources_map.h", "inspector_overlay_resources.h")
      open(sib, "w", encoding="utf-8", newline="\n").write(RES_H)
    elif p.endswith("inspector_overlay_resources.h"):
      open(p, "w", encoding="utf-8", newline="\n").write(RES_H)
    elif p.endswith(".pak.info"):
      open(p, "wb").write(b"")
    elif p.endswith(".pak"):
      # Valid empty data_pack v5 (resource count 0) — bare b"" breaks pak_util repack.
      open(p, "wb").write(
          bytes([0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00])
      )
    elif p.endswith(".h"):
      open(p, "w", encoding="utf-8", newline="\n").write("#pragma once\n// Hallulium hollow\n")
    else:
      open(p, "wb").close()
  return 0

if __name__ == "__main__":
  sys.exit(main())
