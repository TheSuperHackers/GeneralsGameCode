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

#include <emmintrin.h>
#if defined(_MSC_VER)
	#include <intrin.h>
#endif

// SSE2 implementation of memcchr.
namespace stringex_memcchr_detail
{
inline unsigned mismatch_mask(const unsigned char* p, __m128i needle)
{
	const __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
	return static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(v, needle))) ^ 0xffffu;
}

// The caller guarantees a nonzero mask.
inline unsigned first_set_bit(unsigned mask)
{
#if defined(_MSC_VER)
	unsigned long index;
	_BitScanForward(&index, mask);
	return static_cast<unsigned>(index);
#else
	return static_cast<unsigned>(__builtin_ctz(mask));
#endif
}

}    // namespace stringex_memcchr_detail

inline const void* memcchr(const void* data, int c, size_t n)
{
	using stringex_memcchr_detail::first_set_bit;
	using stringex_memcchr_detail::mismatch_mask;
	const unsigned char* p = static_cast<const unsigned char*>(data);
	unsigned index;
	if (n >= 16)
	{
		const __m128i needle = _mm_set1_epi8(static_cast<char>(c));
		if (n > 32)
		{
			unsigned neq = mismatch_mask(p, needle);
			if (neq)
			{
				index = first_set_bit(neq);
				return p + index;
			}
			const unsigned char* const last = p + (n - 16);
			p += 16;
			// 64 bytes per iteration; combine equality masks before branching.
			for (size_t blocks = (n - 16) / 64; blocks; --blocks)
			{
				const __m128i eq0 = _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(p)), needle);
				const __m128i eq1 = _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(p + 16)), needle);
				const __m128i eq2 = _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(p + 32)), needle);
				const __m128i eq3 = _mm_cmpeq_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(p + 48)), needle);
				const __m128i all = _mm_and_si128(_mm_and_si128(_mm_and_si128(eq0, eq1), eq2), eq3);
				if (_mm_movemask_epi8(all) != 0xffff)
				{
					const unsigned neq0 = static_cast<unsigned>(_mm_movemask_epi8(eq0)) ^ 0xffffu;
					const unsigned neq1 = static_cast<unsigned>(_mm_movemask_epi8(eq1)) ^ 0xffffu;
					const unsigned neq2 = static_cast<unsigned>(_mm_movemask_epi8(eq2)) ^ 0xffffu;
					const unsigned neq3 = static_cast<unsigned>(_mm_movemask_epi8(eq3)) ^ 0xffffu;
					neq = (neq1 << 16) | neq0;
					if (neq == 0)
					{
						neq = (neq3 << 16) | neq2;
						p += 32;
					}
					index = first_set_bit(neq);
					return p + index;
				}
				p += 64;
			}
			while (p < last)
			{
				neq = mismatch_mask(p, needle);
				if (neq)
				{
					index = first_set_bit(neq);
					return p + index;
				}
				p += 16;
			}
			neq = mismatch_mask(last, needle);
			if (neq)
			{
				index = first_set_bit(neq);
				return last + index;
			}
			return nullptr;
		}
		const unsigned char* const last = p + (n - 16);
		const unsigned neq = mismatch_mask(p, needle) | (mismatch_mask(last, needle) << 16);
		if (neq == 0)
			return nullptr;
		index = first_set_bit(neq);
		return index < 16 ? p + index : last + (index - 16);
	}
	if (n >= 4)
	{
		if (n >= 8)
		{
			const __m128i needle = _mm_set1_epi8(static_cast<char>(c));
			const __m128i head = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(p));
			const __m128i tail = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(p + (n - 8)));
			const __m128i eq = _mm_cmpeq_epi8(_mm_unpacklo_epi64(head, tail), needle);
			const unsigned neq = static_cast<unsigned>(_mm_movemask_epi8(eq)) ^ 0xffffu;
			if (neq == 0)
				return nullptr;
			index = first_set_bit(neq);
			return index < 8 ? p + index : p + (n - 8) + (index - 8);
		}
		const uint32_t repeated32 = static_cast<unsigned char>(c) * 0x01010101u;
		uint32_t head;
		uint32_t tail;
		memcpy(&head, p, sizeof(head));
		memcpy(&tail, p + (n - 4), sizeof(tail));
		head ^= repeated32;
		tail ^= repeated32;
		if ((head | tail) == 0)
			return nullptr;
		if (head)
		{
			index = first_set_bit(head);
			return p + index / 8;
		}
		index = first_set_bit(tail);
		return p + (n - 4) + index / 8;
	}
	while (n)
	{
		if (*p != static_cast<unsigned char>(c))
			return p;
		++p;
		--n;
	}
	return nullptr;
}
