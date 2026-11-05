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

#include <gtest/gtest.h>
#include <vector>

#include "WW3D2/textureatlas.h"

namespace
{
// A cell of 2x2 texels with 1 texel of padding: 4x4 pixels, interior values 1 2 / 3 4.
std::vector<unsigned> makeCell()
{
	std::vector<unsigned> cell(16, 0);
	cell[1 * 4 + 1] = 1;
	cell[1 * 4 + 2] = 2;
	cell[2 * 4 + 1] = 3;
	cell[2 * 4 + 2] = 4;
	return cell;
}

const int CELL_PITCH = 4 * sizeof(unsigned);
}    // namespace

TEST(TextureAtlas, WrapOrClampClampsToNearestEdge)
{
	EXPECT_EQ(TextureAtlasClass::Wrap_Or_Clamp(-1, 8, false), 0);
	EXPECT_EQ(TextureAtlasClass::Wrap_Or_Clamp(3, 8, false), 3);
	EXPECT_EQ(TextureAtlasClass::Wrap_Or_Clamp(8, 8, false), 7);
}

TEST(TextureAtlas, WrapOrClampWrapsToOppositeEdge)
{
	EXPECT_EQ(TextureAtlasClass::Wrap_Or_Clamp(-1, 8, true), 7);
	EXPECT_EQ(TextureAtlasClass::Wrap_Or_Clamp(8, 8, true), 0);
	EXPECT_EQ(TextureAtlasClass::Wrap_Or_Clamp(-9, 8, true), 7);
}

TEST(TextureAtlas, FillPaddingRepeatsNearestEdgeWhenClamping)
{
	std::vector<unsigned> cell = makeCell();
	TextureAtlasClass::Fill_Padding((unsigned char*)cell.data(), CELL_PITCH, 2, 2, 1, false, false);
	const unsigned expected[16] = { 1, 1, 2, 2, 1, 1, 2, 2, 3, 3, 4, 4, 3, 3, 4, 4 };
	EXPECT_EQ(cell, std::vector<unsigned>(expected, expected + 16));
}

TEST(TextureAtlas, FillPaddingRepeatsOppositeEdgeWhenWrapping)
{
	std::vector<unsigned> cell = makeCell();
	TextureAtlasClass::Fill_Padding((unsigned char*)cell.data(), CELL_PITCH, 2, 2, 1, true, true);
	const unsigned expected[16] = { 4, 3, 4, 3, 2, 1, 2, 1, 4, 3, 4, 3, 2, 1, 2, 1 };
	EXPECT_EQ(cell, std::vector<unsigned>(expected, expected + 16));
}
