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

// A triangle with a red, a green and a blue corner blends the corner colors across its area.
TEST_F(RenderingTest, Triangle)
{
	const unsigned int background = 0x102030;
	clear(background);
	Render2DClass renderer;
	renderer.Set_Coordinate_Range(RectClass(0.0f, 0.0f, Width, Height));
	renderer.Enable_Texturing(false);
	// Draws the triangle whichever way round its corners go.
	renderer.Get_Shader()->Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
	const Vector2 uv(0.0f, 0.0f);
	renderer.Add_Tri(Vector2(128.0f, 32.0f), Vector2(224.0f, 224.0f), Vector2(32.0f, 224.0f), uv, uv, uv);
	// Add_Tri gives all corners one color. The color array is a DynamicVectorClass in Zero Hour and a
	// SimpleDynVecClass in Generals.
	auto &colors = renderer.Get_Color_Array();
	colors[0] = 0xFFFF0000;
	colors[1] = 0xFF00FF00;
	colors[2] = 0xFF0000FF;
	renderer.Render();
	capture();
	// GPUs round the blended colors differently, so the exact colors are left to the image comparison of CI.
	EXPECT_EQ(pixel(8, 8), background);
	EXPECT_NE(pixel(128, 160), background);
}
