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

// Included by stringex.h

#pragma once

// Portable implementation of memcchr.
inline const void* memcchr(const void* data, int c, size_t n)
{
	const unsigned char* p = static_cast<const unsigned char*>(data);
	const unsigned char b = static_cast<unsigned char>(c);
	const uint32_t repeated32 = b * 0x01010101u;
	const uint64_t repeated64 = (uint64_t(repeated32) << 32) | repeated32;
	while (n >= 8) {
		uint64_t v;
		memcpy(&v, p, sizeof(v));
		if (v != repeated64) break;
		p += 8;
		n -= 8;
	}
	if (n >= 4) {
		uint32_t v;
		memcpy(&v, p, sizeof(v));
		if (v == repeated32) {
			p += 4;
			n -= 4;
		}
	}
	while (n) {
		if (*p != b) return p;
		++p;
		--n;
	}
	return nullptr;
}
