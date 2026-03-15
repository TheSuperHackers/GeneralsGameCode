#!/usr/bin/env python3
"""
format_pragma_once_spacing.py

Ensure consistent spacing around '#pragma once' in .h files:

- Exactly one blank line AFTER '#pragma once'.
- Exactly one blank line BEFORE '#pragma once' IF there is any non-blank content above it.
  (If '#pragma once' is already the first non-blank content in the file, we do NOT insert
   a leading blank line.)

We leave everything else untouched.

Outputs:
- total .h files found
- files changed
- files without '#pragma once'
"""

from __future__ import annotations
import argparse
import re
from pathlib import Path

RE_PRAGMA_ONCE = re.compile(r'^\s*#\s*pragma\s+once\s*$', re.IGNORECASE)

def is_blank(s: str) -> bool:
    return s.strip() == ""

def normalize_pragma_once_spacing(text: str) -> tuple[str, bool]:
    """
    For each '#pragma once' line in the file, enforce spacing rules.
    Returns (new_text, changed).
    """
    # Preserve final newline behaviour
    had_trailing_nl = text.endswith("\n")
    lines = text.splitlines()

    # Find all pragma indices
    pragma_idxs = [i for i, ln in enumerate(lines) if RE_PRAGMA_ONCE.match(ln)]
    if not pragma_idxs:
        return text, False

    changed = False
    # We'll operate on a mutable list; keep track of index shifts when inserting/removing
    i = 0
    while i < len(lines):
        if i in pragma_idxs:
            # ----- BEFORE: ensure exactly one blank line IF there is any non-blank above -----
            # Count run of blank lines immediately above
            j = i - 1
            blanks_before = 0
            while j >= 0 and is_blank(lines[j]):
                blanks_before += 1
                j -= 1

            has_nonblank_above = j >= 0  # there's some non-blank content above

            # If there is content above, enforce exactly one blank line before
            if has_nonblank_above:
                # Remove all blank lines immediately above
                if blanks_before > 1:
                    # delete the extra blanks, keep none for now
                    del lines[i - blanks_before: i - 1]  # keep one slot for later insert
                    i -= (blanks_before - 1)
                    changed = True
                elif blanks_before == 0:
                    # none present now; we'll insert one below
                    pass
                # After removals, ensure exactly one blank line
                if i - 1 < 0 or not is_blank(lines[i - 1]):
                    lines.insert(i, "")
                    i += 1  # pragma shifts down by one
                    # Update pragma index list too
                    pragma_idxs = [p + 1 if p >= i - 1 else p for p in pragma_idxs]
                    changed = True
            else:
                # No nonblank content above; ensure there are zero blank lines above
                if blanks_before > 0:
                    del lines[i - blanks_before:i]
                    i -= blanks_before
                    # Adjust pragma indices after deletion
                    removed = blanks_before
                    pragma_idxs = [p - removed if p >= i else p for p in pragma_idxs]
                    changed = True

            # Recompute blanks_after starting from (possibly) updated i
            # ----- AFTER: ensure exactly one blank line after -----
            k = i + 1
            blanks_after = 0
            while k < len(lines) and is_blank(lines[k]):
                blanks_after += 1
                k += 1

            # Remove all blank lines after, then insert exactly one
            if blanks_after != 1:
                # remove all current blanks after
                if blanks_after > 0:
                    del lines[i + 1 : i + 1 + blanks_after]
                    # adjust pragma indices after deletion (those after k shift left)
                    # Not strictly needed for correctness here
                    changed = True
                # insert exactly one blank line
                lines.insert(i + 1, "")
                changed = True

            # Advance past the pragma and the single enforced blank after it
            i = i + 2
            continue

        i += 1

    new_text = "\n".join(lines) + ("\n" if had_trailing_nl else "")
    return new_text, changed

def main():
    ap = argparse.ArgumentParser(description="Ensure exactly one empty line before/after '#pragma once' in .h files.")
    ap.add_argument("directory", type=Path, help="Root directory to scan recursively")
    args = ap.parse_args()

    root: Path = args.directory
    if not root.exists() or not root.is_dir():
        print(f"Error: {root} is not a directory")
        raise SystemExit(2)

    headers = [p for p in root.rglob("*.h") if p.is_file()]
    changed = 0
    no_pragma: list[str] = []

    for hdr in headers:
        try:
            text = hdr.read_text(encoding="utf-8", errors="replace")
        except Exception as e:
            print(f"Read error: {hdr} ({e})")
            continue

        new_text, did_change = normalize_pragma_once_spacing(text)
        if did_change:
            try:
                hdr.write_text(new_text, encoding="utf-8")
                changed += 1
            except Exception as e:
                print(f"Write error: {hdr} ({e})")
        else:
            # Determine if it lacked pragma
            if not RE_PRAGMA_ONCE.search(text):
                no_pragma.append(str(hdr))

    print(f"Total .h files found: {len(headers)}")
    print(f"Files changed:        {changed}")
    if no_pragma:
        print("Files without '#pragma once':")
        for p in no_pragma:
            print(f"  - {p}")

if __name__ == "__main__":
    main()
