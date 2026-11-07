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

// SSE2 implementation of premultiply_alpha.
// Adapted from v1 of https://github.com/Wizermil/premultiply_alpha (MIT License, Copyright (c) 2019 Mathieu Garaud).
inline void premultiply_alpha(unsigned char* pixels, int pitch, int width, int height)
{
	const __m128i alphaMask = _mm_set1_epi32(static_cast<int>(0xFF000000u));
	const __m128i oddMask = _mm_set1_epi16(static_cast<short>(0xFF00));
	const __m128i half = _mm_set1_epi16(127);
	const __m128i div255 = _mm_set1_epi16(static_cast<short>(0x8081));
	for (int y = 0; y < height; ++y, pixels += pitch)
	{
		unsigned int* line = reinterpret_cast<unsigned int*>(pixels);
		int x = 0;
		for (; x + 4 <= width; x += 4)
		{
			__m128i* const p = reinterpret_cast<__m128i*>(line + x);
			const __m128i color = _mm_loadu_si128(p);

			// Alpha times 256 in both 16-bit halves of each pixel.
			__m128i alpha = _mm_and_si128(color, alphaMask);
			alpha = _mm_or_si128(alpha, _mm_srli_epi32(alpha, 16));

			// Blue and red sit in the even bytes, green and alpha in the odd bytes. Moving each to the top of its 16-bit lane
			// makes the high half of the product with alpha * 256 equal to c * a.
			__m128i even = _mm_mulhi_epu16(_mm_slli_epi16(color, 8), alpha);
			__m128i odd = _mm_mulhi_epu16(_mm_and_si128(color, oddMask), alpha);

			// (c * a + 127) / 255, as (v * 0x8081) >> 23, which is exact for every 16-bit v.
			even = _mm_srli_epi16(_mm_mulhi_epu16(_mm_add_epi16(even, half), div255), 7);
			odd = _mm_srli_epi16(_mm_mulhi_epu16(_mm_add_epi16(odd, half), div255), 7);

			const __m128i result = _mm_or_si128(even, _mm_slli_epi16(odd, 8));
			_mm_storeu_si128(p, _mm_or_si128(_mm_andnot_si128(alphaMask, result), _mm_and_si128(alphaMask, color)));
		}

		// The last 0 to 3 pixels of the line.
		for (; x < width; ++x)
		{
			const unsigned int alpha = line[x] >> 24;
			unsigned int result = line[x] & 0xFF000000u;
			result |= ((line[x] & 255) * alpha + 127) / 255;
			result |= ((((line[x] >> 8) & 255) * alpha + 127) / 255) << 8;
			result |= ((((line[x] >> 16) & 255) * alpha + 127) / 255) << 16;
			line[x] = result;
		}
	}
}
