#!/usr/bin/env python3
"""
replace_guards_with_pragma_once.py

Recursively scan a directory for .h files that DO NOT contain `#pragma once`.
For those files, detect classic include guards and replace the guard with a
single `#pragma once`.

We replace only when:
  - A classic guard is found near the start:
      #ifndef MACRO            OR     #if !defined(MACRO)
      #define MACRO                   #define MACRO
      ... (substantive content) ...
      #endif
  - There is substantive content between the #define and its matching #endif
    (i.e., more than just blank lines or comment-only lines), to avoid replacing
    macro wrapper patterns like:
        #ifndef ARRAY_SIZE
        #define ARRAY_SIZE(x) ...
        #endif

Output:
- total .h files found
- files changed
- list of files not changed (with reason)
"""

from __future__ import annotations
import argparse
import re
from pathlib import Path

RE_PRAGMA_ONCE = re.compile(r'^\s*#\s*pragma\s+once\b', re.IGNORECASE)

RE_IFNDEF = re.compile(r'^\s*#\s*ifndef\s+([A-Za-z_][A-Za-z0-9_]*)\s*(?:\/\/.*|/\*.*\*/\s*)?$', re.ASCII)
RE_IF_NOT_DEFINED = re.compile(
    r'^\s*#\s*if\s*!+\s*defined\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)\s*(?:\/\/.*|/\*.*\*/\s*)?$',
    re.ASCII,
)
RE_DEFINE = re.compile(r'^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)\b.*$', re.ASCII)
RE_IF = re.compile(r'^\s*#\s*if(n?def)?\b', re.ASCII)   # #if/#ifdef/#ifndef
RE_ENDIF = re.compile(r'^\s*#\s*endif\b', re.ASCII)

def _strip_bom(text: str) -> str:
    return text.lstrip('\ufeff')

def has_pragma_once(lines: list[str]) -> bool:
    return any(RE_PRAGMA_ONCE.match(l) for l in lines)

def is_comment_or_blank(s: str) -> bool:
    t = s.strip()
    if not t:
        return True
    # treat single-line/comment-only lines as non-substantive
    return t.startswith('//') or t.startswith('/*') or t.endswith('*/')

def has_substantive_content(lines: list[str], start_idx: int, end_idx: int) -> bool:
    """
    Return True if there's at least one non-blank, non-comment-only line
    between start_idx (inclusive) and end_idx (exclusive).
    """
    for k in range(start_idx, end_idx):
        if not is_comment_or_blank(lines[k]):
            return True
    return False

def match_endif(lines: list[str], start_index: int, initial_depth: int = 1):
    """
    Find the index of the #endif that closes the #if/#ifndef starting before start_index.
    Handles nested preprocessor blocks.
    """
    depth = initial_depth
    for k in range(start_index, len(lines)):
        s = lines[k]
        if not s.lstrip().startswith('#'):
            continue
        if RE_IF.match(s):
            depth += 1
        elif RE_ENDIF.match(s):
            depth -= 1
            if depth == 0:
                return k
    return None

def find_guard(lines: list[str], search_window: int = 200):
    """
    Locate classic include guard near the beginning (after possible license header).
    Returns (i_if, i_define, macro, i_endif) or None.
    """
    n = len(lines)
    lim = min(search_window, n)

    for i in range(lim):
        line = lines[i]
        m1 = RE_IFNDEF.match(line)
        m2 = RE_IF_NOT_DEFINED.match(line)
        macro = None
        if m1:
            macro = m1.group(1)
        elif m2:
            macro = m2.group(1)
        else:
            continue

        # Find #define with the same macro in the next few lines, skipping blanks/comments
        j = i + 1
        max_lookahead = min(i + 12, n - 1)
        found_define_index = None
        while j <= max_lookahead:
            l2 = lines[j]
            if is_comment_or_blank(l2):
                j += 1
                continue
            mdef = RE_DEFINE.match(l2)
            if mdef and mdef.group(1) == macro:
                found_define_index = j
            break
        if found_define_index is None:
            continue

        # Find the closing #endif for this guard block
        i_endif = match_endif(lines, start_index=found_define_index + 1, initial_depth=1)
        if i_endif is None:
            continue

        return (i, found_define_index, macro, i_endif)

    return None

def replace_guard_with_pragma_once(text: str):
    """
    If file lacks #pragma once and has a classic include guard with substantive content,
    replace the guard with a single '#pragma once'.
    Returns (new_text, changed_bool, reason_if_not_changed).
    """
    text = _strip_bom(text)
    lines = text.splitlines()

    if has_pragma_once(lines):
        return text, False, "already has #pragma once"

    found = find_guard(lines)
    if not found:
        return text, False, "no classic guard detected"

    i_if, i_define, macro, i_endif = found

    # Ensure the region contains more than just the macro define
    if not has_substantive_content(lines, i_define + 1, i_endif):
        return text, False, "guard region has no content (macro wrapper)"

    # Build new content:
    # - Replace the opening guard pair with a single '#pragma once'
    # - Remove the closing '#endif'
    new_lines = list(lines)

    # Prepare a tidy insertion: remove surrounding blank lines near opening/closing
    remove_idxs = set()

    # Remove opening lines
    remove_idxs.add(i_if)
    remove_idxs.add(i_define)

    # Optional tidy: blank line immediately after the #define
    if i_define + 1 < len(new_lines) and new_lines[i_define + 1].strip() == '':
        remove_idxs.add(i_define + 1)

    # Optional tidy: blank line immediately before the #if
    if i_if - 1 >= 0 and new_lines[i_if - 1].strip() == '':
        remove_idxs.add(i_if - 1)

    # Remove closing '#endif' and an optional blank before it
    remove_idxs.add(i_endif)
    if i_endif - 1 >= 0 and new_lines[i_endif - 1].strip() == '':
        remove_idxs.add(i_endif - 1)

    # Apply removals
    new_lines = [ln for idx, ln in enumerate(new_lines) if idx not in remove_idxs]

    # Insert '#pragma once' at the original guard start position (adjusted for removals)
    # Compute the new insertion index as the smallest kept index >= i_if
    insertion_index = 0
    count = 0
    for idx in range(len(lines)):
        if idx in remove_idxs:
            continue
        if idx >= i_if:
            insertion_index = count
            break
        count += 1
    else:
        insertion_index = 0  # fallback (shouldn't happen)

    new_lines.insert(insertion_index, "#pragma once")

    # Tidy leading/trailing blank lines
    while new_lines and new_lines[0].strip() == '':
        new_lines.pop(0)
    while new_lines and new_lines[-1].strip() == '':
        new_lines.pop()

    new_text = "\n".join(new_lines) + ("\n" if text.endswith("\n") else "")
    return new_text, True, None

def main():
    ap = argparse.ArgumentParser(
        description="Replace classic include guards with #pragma once in .h files that don't already have it."
    )
    ap.add_argument("directory", type=Path, help="Root directory to scan recursively")
    args = ap.parse_args()

    root: Path = args.directory
    if not root.exists() or not root.is_dir():
        print(f"Error: {root} is not a directory")
        raise SystemExit(2)

    all_headers = [p for p in root.rglob("*.h") if p.is_file()]
    changed = 0
    skipped: list[str] = []

    for hdr in all_headers:
        try:
            text = hdr.read_text(encoding="utf-8", errors="replace")
        except Exception as e:
            skipped.append(f"{hdr} (read-error: {e})")
            continue

        new_text, did_change, reason = replace_guard_with_pragma_once(text)
        if did_change:
            try:
                hdr.write_text(new_text, encoding="utf-8")
                changed += 1
            except Exception as e:
                skipped.append(f"{hdr} (write-error: {e})")
        else:
            skipped.append(f"{hdr} ({reason or 'unknown reason'})")

    print(f"Total .h files found: {len(all_headers)}")
    print(f"Files changed:        {changed}")
    if skipped:
        print("Not changed:")
        for path in skipped:
            print(f"  - {path}")

if __name__ == "__main__":
    main()
