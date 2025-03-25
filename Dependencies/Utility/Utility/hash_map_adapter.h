// TheSuperHackers
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
