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

#include "always.h"

// Single-character functions

// Returns the number of bytes in a UTF-8 character based on the lead byte. Returns 0 if invalid lead byte.
size_t utf8_num_bytes(char lead);
// Returns number of bytes to truncate if str contains incomplete UTF-8 character at the end.
// 0 if no truncation needed. Assumes correct UTF-8 up to length - 1.
int utf8_truncate_if_incomplete(const char* str, size_t length);

// Validation functions

// Validates whether the given string is valid UTF-8.
bool utf8_validate_string(const char* str);
// Validates whether the given string is valid UTF-8 up to the specified length.
bool utf8_validate_string(const char* str, const size_t length);

// Conversion functions

// Gets the size in bytes required to hold the UTF-8 representation of the given widechar string, including null terminator.
size_t get_size_as_utf8(const wchar_t* s);
// Gets the size in bytes required to hold the widechar representation of the given UTF-8 string, including null terminator.
size_t get_size_as_widechar(const char* s);
// Converts a widechar string to UTF-8. Assumes tgt has enough space (use get_size_as_utf8 to determine size).
size_t convert_widechar_to_utf8(const wchar_t* orig, char* tgt, size_t tgtsize);
// Converts a UTF-8 string to widechar. Assumes tgt has enough space (use get_size_as_widechar to determine size).
size_t convert_utf8_to_widechar(const char* orig, wchar_t* tgt, size_t tgtsize);
