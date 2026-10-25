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
#include "WW3D2/render2d.h"

// The UI draws filled and outlined rectangles, lines and gradients with Render2DClass, as W3DDisplay::drawFillRect
// and its siblings do.
TEST_F(RenderingTest, Render2DShapes)
{
	Render2DClass renderer;
	renderer.Set_Coordinate_Range(RectClass(0.0f, 0.0f, Width, Height));
	renderer.Enable_Texturing(false);

	// A blue rectangle with a white border, an orange outline, a green line and a red to blue gradient.
	renderer.Add_Rect(RectClass(16.0f, 16.0f, 112.0f, 112.0f), 4.0f, 0xFFFFFFFF, 0xFF2040C0);
	renderer.Add_Outline(RectClass(144.0f, 16.0f, 240.0f, 112.0f), 2.0f, 0xFFFFC000);
	renderer.Add_Line(Vector2(16.0f, 136.0f), Vector2(240.0f, 184.0f), 3.0f, 0xFF40FF40);
	renderer.Add_Quad_HGradient(RectClass(16.0f, 208.0f, 240.0f, 240.0f), 0xFFFF0000, 0xFF0000FF);
	renderer.Render();

	capture();
	// The fill and the border of the rectangle, and the inside and the edge of the outline.
	EXPECT_EQ(pixel(64, 64), 0x2040C0);
	EXPECT_EQ(pixel(17, 64), 0xFFFFFF);
	EXPECT_EQ(pixel(192, 64), 0x000000);
	EXPECT_EQ(pixel(145, 64), 0xFFC000);
}
