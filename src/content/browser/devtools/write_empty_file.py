#!/usr/bin/env python3
# Copyright 2026 Hallulium Authors
"""Write an empty file (used for stub .pak targets)."""

import pathlib
import sys


def main(argv: list[str]) -> int:
  # argv[1:] are output paths to create as empty files (.pak and optional .pak.info).
  for raw in argv[1:]:
    path = pathlib.Path(raw)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b"")
  return 0


if __name__ == "__main__":
  sys.exit(main(sys.argv))
