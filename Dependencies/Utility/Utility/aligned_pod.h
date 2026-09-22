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

// This file contains a POD type with a given alignment, which can align raw storage in a union
// instead of alignas, which VC6 does not have. For example:
//
//   union { char bytes[sizeof(T)]; aligned_pod<alignof(T)> alignment; } storage;
//
// Alignments above 8 are not supported, because VC6 cannot express them.

#pragma once

#include <stddef.h>

template <size_t Alignment> struct aligned_pod; // Is undefined for unsupported alignments
template <> struct aligned_pod<1> { char value; };
template <> struct aligned_pod<2> { short value; };
template <> struct aligned_pod<4> { long value; };
template <> struct aligned_pod<8> { double value; };
