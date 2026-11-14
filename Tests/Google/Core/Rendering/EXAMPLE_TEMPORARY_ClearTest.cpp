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

#include "RenderingTestFixture.h"

// Clearing the render target fills it with the clear color from corner to corner.
TEST_F(RenderingTest, Clear)
{
	const unsigned int color = 0x102030;
	clear(color);
	capture();
	EXPECT_EQ(pixel(0, 0), color);
	EXPECT_EQ(pixel(Width - 1, Height - 1), color);
}
