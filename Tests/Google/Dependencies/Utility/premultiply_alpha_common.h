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

#include <gtest/gtest.h>
#include <vector>

using PremultiplyAlphaFunction = void (*)(unsigned char* pixels, Int pitch, Int width, Int height);

// Checks every color and alpha pair against rounded c * a / 255. Each row holds one alpha with all 256 colors plus
// 3 fixed pixels, so the width is not a multiple of 4 and remainders run too. The rest of each row must stay untouched.
inline void testPremultiplyAlpha(PremultiplyAlphaFunction premultiplyAlphaFunction)
{
	constexpr Int width = 259;
	constexpr Int stride = width + 5;
	constexpr Int height = 256;
	constexpr UnsignedInt untouched = 0x80FF8000u;
	std::vector<UnsignedInt> pixels(stride * height, untouched);
	for (Int y = 0; y < height; ++y)
	{
		const UnsignedInt alpha = y;
		UnsignedInt* row = &pixels[y * stride];
		for (UnsignedInt color = 0; color < 256; ++color)
		{
			row[color] = (alpha << 24) | (color << 16) | ((255 - color) << 8) | ((color * 7) & 255);
		}
		row[256] = 0x80FF8000u;
		row[257] = 0xFF102030u;
		row[258] = 0x00FFFFFFu;
	}
	const std::vector<UnsignedInt> source = pixels;

	premultiplyAlphaFunction(reinterpret_cast<unsigned char*>(pixels.data()), stride * Int(sizeof(UnsignedInt)), width, height);

	Int mismatches = 0;
	for (Int i = 0; i < stride * height; ++i)
	{
		UnsignedInt expected = source[i];
		if (i % stride < width)
		{
			const UnsignedInt alpha = source[i] >> 24;
			expected = source[i] & 0xFF000000u;
			for (UnsignedInt shift = 0; shift < 24; shift += 8)
			{
				expected |= ((((source[i] >> shift) & 255) * alpha + 127) / 255) << shift;
			}
		}
		mismatches += pixels[i] != expected;
	}
	EXPECT_EQ(mismatches, 0);
	EXPECT_EQ(pixels[256], 0x80804000u);
	EXPECT_EQ(pixels[257], 0xFF102030u);
	EXPECT_EQ(pixels[258], 0x00000000u);
	EXPECT_EQ(pixels[width], untouched);
}
