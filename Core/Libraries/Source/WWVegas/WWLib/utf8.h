/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <stddef.h>
#include <wchar.h>

// UTF-8 <-> wide-character transcoding, hand-rolled per RFC 3629, using no platform text APIs.
// The wide side is wchar_t, whose width is platform-dependent: on Windows it is a 16-bit UTF-16
// code unit (astral codepoints use surrogate pairs); on most other platforms it is a 32-bit
// UTF-32 codepoint. Both are handled transparently based on the width of wchar_t.

// Returned by the decoding functions when the source is not well-formed UTF-8. A return of 0 means
// an empty result, which is a success and must not be confused with a decoding failure.
const size_t UTF8_INVALID = (size_t)-1;

// Returns the number of UTF-8 bytes needed for the UTF-8 representation of srcLen wide characters
// from src, not counting a null terminator. Returns 0 if srcLen is 0. Wide values that have no
// UTF-8 representation are counted as U+FFFD.
size_t Wide_To_Utf8_Len(const wchar_t* src, size_t srcLen);

// Returns the number of wide characters needed for the wide representation of srcLen bytes from the
// UTF-8 string src, not counting a null terminator. Returns 0 if srcLen is 0, or UTF8_INVALID if
// src is not well-formed UTF-8.
size_t Utf8_To_Wide_Len(const char* src, size_t srcLen);

// Converts srcLen wide characters from src to UTF-8. destLen is the destination buffer capacity in
// bytes. Writes a null terminator if room remains, otherwise not. Wide values that have no UTF-8
// representation are written as U+FFFD, so the output always decodes back through Utf8_To_Wide.
// Returns the number of bytes the whole conversion needs, not counting a null terminator. A return
// greater than destLen means the output was truncated on a codepoint boundary; retry with that many
// bytes plus one for the terminator. Pass destLen 0 to measure without writing.
size_t Wide_To_Utf8(char* dest, size_t destLen, const wchar_t* src, size_t srcLen);

// Converts srcLen bytes from the UTF-8 string src to wide characters. destLen is the destination
// buffer capacity in wide characters. Writes a null terminator if room remains, otherwise not.
// Returns the number of wide characters the whole conversion needs, not counting a null terminator.
// A return greater than destLen means the output was truncated on a codepoint boundary; retry with
// that many wide characters plus one for the terminator. Pass destLen 0 to measure without writing.
// Returns UTF8_INVALID if src is not well-formed UTF-8, setting dest[0] to L'\0' if destLen > 0.
size_t Utf8_To_Wide(wchar_t* dest, size_t destLen, const char* src, size_t srcLen);
