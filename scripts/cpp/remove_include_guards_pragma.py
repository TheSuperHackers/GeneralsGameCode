#!/usr/bin/env python3
"""
remove_include_guards.py

Recursively scan a directory for .h files and remove classic include guards
ONLY if:
  1) the file contains `#pragma once`, AND
  2) there is substantive content between the guard's #define and its matching #endif
     (i.e., not just blank/comment lines).

Safely handles both:
- #ifndef MACRO ... #define MACRO ... #endif
- #if !defined(MACRO) ... #define MACRO ... #endif

Leaves #pragma once intact. Prints totals and files skipped (with reasons).
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
    # Treat single-line comments and block-comment-only lines as non-substantive.
    return t.startswith('//') or t.startswith('/*') or t.endswith('*/')

def has_substantive_content(lines: list[str], start_idx: int, end_idx: int) -> bool:
    """
    Return True if there's at least one 'substantive' line between start_idx (inclusive)
    and end_idx (exclusive). Substantive means not just blank/comment-only.
    Any preprocessor directive like #include, #if, #define (for other macros),
    code, etc. counts as content. (#endif is outside this range by design.)
    """
    for k in range(start_idx, end_idx):
        s = lines[k]
        if not is_comment_or_blank(s):
            return True
    return False

def find_guard(lines: list[str], search_window: int = 150):
    """
    Locate a classic include guard near the file start.

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

        # Find #define with the same macro within a few lines, skipping blanks/comments
        j = i + 1
        max_lookahead = min(i + 10, n - 1)
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

        # Find the matching #endif for the opening #if/#ifndef at i
        i_endif = match_endif(lines, start_index=found_define_index + 1, initial_depth=1)
        if i_endif is None:
            continue

        return (i, found_define_index, macro, i_endif)

    return None

def match_endif(lines: list[str], start_index: int, initial_depth: int = 1):
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

def remove_guard_from_text(text: str) -> tuple[str, bool, str | None]:
    """
    Remove include guard if detected AND:
      - file has #pragma once
      - there is substantive content between #define and #endif
    Returns (new_text, changed_bool, reason_if_not_changed).
    """
    text = _strip_bom(text)
    lines = text.splitlines()

    if not has_pragma_once(lines):
        return text, False, "no #pragma once"

    found = find_guard(lines)
    if not found:
        return text, False, "no classic guard detected"

    i_if, i_define, macro, i_endif = found

    # NEW RULE: ensure the guarded region contains more than just the macro define.
    if not has_substantive_content(lines, i_define + 1, i_endif):
        # Likely a utility macro wrapper like:
        #   #ifndef X
        #   #define X ...
        #   #endif
        return text, False, "guard region has no content (macro wrapper)"

    # Proceed to remove the guard
    to_remove = {i_if, i_define}
    if i_define + 1 < len(lines) and lines[i_define + 1].strip() == '':
        to_remove.add(i_define + 1)
    if i_if - 1 >= 0 and lines[i_if - 1].strip() == '':
        to_remove.add(i_if - 1)

    to_remove.add(i_endif)
    if i_endif - 1 >= 0 and lines[i_endif - 1].strip() == '':
        to_remove.add(i_endif - 1)

    new_lines = [ln for idx, ln in enumerate(lines) if idx not in to_remove]

    # Tidy leading/trailing blanks
    while new_lines and new_lines[0].strip() == '':
        new_lines.pop(0)
    while new_lines and new_lines[-1].strip() == '':
        new_lines.pop()

    new_text = "\n".join(new_lines) + ("\n" if text.endswith("\n") else "")
    return new_text, True, None

def main():
    ap = argparse.ArgumentParser(description="Remove classic include guards from .h files that already use #pragma once, skipping macro-only wrappers.")
    ap.add_argument("directory", type=Path, help="Root directory to scan recursively")
    args = ap.parse_args()

    root: Path = args.directory
    if not root.exists() or not root.is_dir():
        print(f"Error: {root} is not a directory")
        raise SystemExit(2)

    all_headers = [p for p in root.rglob("*.h") if p.is_file()]
    changed = 0
    failed: list[str] = []

    for hdr in all_headers:
        try:
            text = hdr.read_text(encoding="utf-8", errors="replace")
        except Exception as e:
            failed.append(f"{hdr} (read-error: {e})")
            continue

        new_text, did_change, reason = remove_guard_from_text(text)
        if did_change:
            try:
                hdr.write_text(new_text, encoding="utf-8")
                changed += 1
            except Exception as e:
                failed.append(f"{hdr} (write-error: {e})")
        else:
            failed.append(f"{hdr} ({reason or 'unknown reason'})")

    print(f"Total .h files found: {len(all_headers)}")
    print(f"Files changed:        {changed}")
    if failed:
        print("Guards not removed from:")
        for path in failed:
            print(f"  - {path}")

if __name__ == "__main__":
    main()
