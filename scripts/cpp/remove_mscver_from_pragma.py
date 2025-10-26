#!/usr/bin/env python3
"""
Remove MSVC-only guards around '#pragma once' in .h files, preserving surrounding blank lines.

Recognised guard forms:
  #if defined(_MSC_VER)
  #ifdef _MSC_VER
  #if _MSC_VER >= 1000

Removes only:
  - the '#if...' line,
  - any blank lines immediately after it,
  - any blank lines immediately after '#pragma once',
  - the matching '#endif' line (optionally with trailing comment).

Usage:
  python unguard_pragma_once_universal.py /path/to/dir
"""

import sys
import re
from pathlib import Path

# Match any of the IF guard forms. No VERBOSE flag; escape literal '#'.
RE_IF = re.compile(
    r'^\s*\#\s*'
    r'(?:'
      r'if\s+defined\s*\(\s*_MSC_VER\s*\)'         # #if defined(_MSC_VER)
      r'|ifdef\s+_MSC_VER'                         # #ifdef _MSC_VER
      r'|if\s+_MSC_VER(?:\s*[<>!=]=?\s*\d+)?'      # #if _MSC_VER [op num]
    r')'
    r'\s*(?://.*)?\r?\n?$',                        # optional end-of-line comment
    re.IGNORECASE
)

RE_PRAGMA = re.compile(r'^\s*\#\s*pragma\s+once\s*\r?\n?$', re.IGNORECASE)
RE_ENDIF  = re.compile(r'^\s*\#\s*endif\b.*\r?\n?$', re.IGNORECASE)
RE_BLANK  = re.compile(r'^[ \t]*\r?\n?$')

def read_text_with_fallback(p: Path) -> str:
    for enc in ("utf-8", "utf-8-sig", "cp1252", "latin-1"):
        try:
            return p.read_text(encoding=enc)
        except UnicodeDecodeError:
            continue
    return p.read_bytes().decode("latin-1", errors="replace")

def unguard_msc_pragma_once(text: str) -> tuple[str, bool]:
    lines = text.splitlines(keepends=True)
    i = 0
    changed = False

    while i < len(lines):
        if not RE_IF.match(lines[i]):
            i += 1
            continue

        if_idx = i
        j = i + 1

        # remove blanks immediately after #if
        while j < len(lines) and RE_BLANK.match(lines[j]):
            j += 1

        # require #pragma once next
        if j >= len(lines) or not RE_PRAGMA.match(lines[j]):
            i += 1
            continue

        pragma_idx = j
        j += 1

        # remove blanks immediately after pragma
        while j < len(lines) and RE_BLANK.match(lines[j]):
            j += 1

        # require #endif next
        if j >= len(lines) or not RE_ENDIF.match(lines[j]):
            i += 1
            continue

        endif_idx = j

        # Keep: everything before IF, the pragma line itself, everything after ENDIF
        new_lines = []
        new_lines.extend(lines[:if_idx])            # preserves any blank lines before the block
        new_lines.append(lines[pragma_idx])         # keep '#pragma once'
        new_lines.extend(lines[endif_idx + 1:])     # preserves any blank lines after the block

        lines = new_lines
        changed = True

        # Resume scanning near the pragma location in the new list
        i = max(0, if_idx - 1)

    return "".join(lines), changed

def main():
    if len(sys.argv) != 2:
        print("Usage: python unguard_pragma_once_universal.py /path/to/dir", file=sys.stderr)
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
            updated, changed = unguard_msc_pragma_once(original)
            if changed:
                p.write_text(updated, encoding="utf-8", newline="")
                changed_count += 1
        except Exception as ex:
            print(f"[ERROR] Failed to process {p}: {ex}", file=sys.stderr)

    print(f"Scanned {total} .h file(s); changed {changed_count} file(s).")

if __name__ == "__main__":
    main()
