#!/usr/bin/env python3
"""
Remove the MSC_VER guard around '#pragma once', specifically deleting:
  - the '#if defined(_MSC_VER)' line,
  - any blank lines immediately after that '#if',
  - any blank lines immediately after '#pragma once',
  - the matching '#endif' line.

Critically, this *does not* remove blank lines that precede the guard or follow the '#endif'.

Usage:
  python unguard_pragma_once_exact.py /path/to/dir
"""

import sys
import re
from pathlib import Path

RE_IF     = re.compile(r'^\s*#\s*if\s+defined\s*\(\s*_MSC_VER\s*\)\s*(?://.*)?\r?\n?$')
RE_PRAGMA = re.compile(r'^\s*#\s*pragma\s+once\s*\r?\n?$')
RE_ENDIF  = re.compile(r'^\s*#\s*endif\b.*\r?\n?$')
RE_BLANK  = re.compile(r'^[ \t]*\r?\n?$')

def read_text_with_fallback(p: Path) -> str:
    for enc in ("utf-8", "utf-8-sig", "cp1252", "latin-1"):
        try:
            return p.read_text(encoding=enc)
        except UnicodeDecodeError:
            continue
    return p.read_bytes().decode("latin-1", errors="replace")

def unguard_one(text: str) -> tuple[str, bool]:
    """
    Remove:
      IF line,
      blanks immediately after IF,
      blanks immediately after PRAGMA,
      ENDIF line.
    Preserve everything else.
    """
    lines = text.splitlines(keepends=True)
    i = 0
    changed = False

    while i < len(lines):
        if not RE_IF.match(lines[i]):
            i += 1
            continue

        if_idx = i
        j = i + 1

        # blanks immediately after #if (remove these)
        while j < len(lines) and RE_BLANK.match(lines[j]):
            j += 1
        blanks_after_if_start = i + 1
        blanks_after_if_end   = j  # [start, end) will be removed

        # must have #pragma once next
        if j >= len(lines) or not RE_PRAGMA.match(lines[j]):
            i += 1
            continue
        pragma_idx = j
        j += 1

        # blanks immediately after pragma (remove these if ENDIF follows)
        blanks_after_pragma_start = j
        while j < len(lines) and RE_BLANK.match(lines[j]):
            j += 1
        blanks_after_pragma_end = j  # [start, end) will be removed if ENDIF matches

        # must have #endif next
        if j >= len(lines) or not RE_ENDIF.match(lines[j]):
            i += 1
            continue
        endif_idx = j

        # Build new slice: keep everything except:
        # - IF line
        # - blanks after IF
        # - blanks after PRAGMA
        # - ENDIF line
        keep = []
        keep.extend(lines[:if_idx])                                # before IF (preserves "line 1")
        keep.extend(lines[pragma_idx:pragma_idx+1])                # keep the pragma line itself
        keep.extend(lines[endif_idx+1:])                           # after ENDIF (preserves "line 7")

        lines = keep
        changed = True

        # Restart scan near where we spliced (safe to restart from max(0, pragma_idx-1) in new array)
        i = max(0, (pragma_idx - 1))  # conservative restart

    return "".join(lines), changed

def main():
    if len(sys.argv) != 2:
        print("Usage: python unguard_pragma_once_exact.py /path/to/dir", file=sys.stderr)
        sys.exit(2)

    root = Path(sys.argv[1])
    if not root.is_dir():
        print(f"Error: {root} is not a directory", file=sys.stderr)
        sys.exit(2)

    total = 0
    changed_count = 0
    for p in root.rglob("*.h"):
        total += 1
        try:
            original = read_text_with_fallback(p)
            updated, changed = unguard_one(original)
            if changed:
                # Write back; per-line original EOLs are preserved in 'updated'
                p.write_text(updated, encoding="utf-8", newline="")
                changed_count += 1
        except Exception as ex:
            print(f"[ERROR] Failed to process {p}: {ex}", file=sys.stderr)

    print(f"Scanned {total} .h file(s); changed {changed_count} file(s).")

if __name__ == "__main__":
    main()
