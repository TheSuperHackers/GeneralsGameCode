/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

// This file includes a hash map that is compatible with vs6, STLPort and modern c++ for the most part.
// There are differences, for example std::hash_map::resize is the equivalent to std::unordered_map::reserve.

#pragma once

#if defined(USING_STLPORT) || (defined(_MSC_VER) && _MSC_VER < 1300)

#include <hash_map>

#else

#include <unordered_map>
namespace std
{
template <
	class _Kty,
	class _Ty,
	class _Hasher = hash<_Kty>,
	class _Keyeq = equal_to<_Kty>,
	class _Alloc = allocator<pair<const _Kty, _Ty>>>
using hash_map = unordered_map<_Kty, _Ty, _Hasher, _Keyeq, _Alloc>;
}

#endif
