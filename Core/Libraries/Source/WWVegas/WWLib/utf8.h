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

// UTF-8 <-> wide-character conversion backed by ICU4C. Windows dynamically uses the ICU4C
// implementation shipped with the operating system and falls back to the native text APIs when
// ICU is unavailable. Other platforms link ICU4C's common library.

// Returned when UTF-8 input is malformed. Zero is reserved for a successful empty conversion.
const size_t UTF8_INVALID = (size_t)-1;

// Return the required destination length without counting a null terminator.
size_t Wide_To_Utf8_Len(const wchar_t* src, size_t srcLen);
size_t Utf8_To_Wide_Len(const char* src, size_t srcLen);

// Convert exactly srcLen source units. The destination is null-terminated when it has spare
// capacity. Wide input that cannot be represented as Unicode is replaced with U+FFFD. Malformed
// UTF-8 returns UTF8_INVALID and clears dest when destLen is nonzero.
size_t Wide_To_Utf8(char* dest, size_t destLen, const wchar_t* src, size_t srcLen);
size_t Utf8_To_Wide(wchar_t* dest, size_t destLen, const char* src, size_t srcLen);
