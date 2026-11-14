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

// The UI and the effects blend over what is below them: alpha fades windows in, additive blending makes glows and
// multiplicative blending darkens for shadows.
TEST_F(RenderingTest, Render2DBlending)
{
	const unsigned int background = 0x808080;
	clear(background);
	Render2DClass renderer;
	renderer.Set_Coordinate_Range(RectClass(0.0f, 0.0f, Width, Height));
	renderer.Enable_Texturing(false);

	// Half transparent red.
	renderer.Enable_Alpha(true);
	renderer.Add_Quad(RectClass(16.0f, 16.0f, 112.0f, 240.0f), 0x80FF0000);
	renderer.Render();

	// Green added on top, partly over the red.
	renderer.Reset();
	renderer.Enable_Additive(true);
	renderer.Add_Quad(RectClass(80.0f, 16.0f, 176.0f, 240.0f), 0xFF006000);
	renderer.Render();

	// Blue multiplied with what is below, partly over the green.
	renderer.Reset();
	*renderer.Get_Shader() = ShaderClass::_PresetMultiplicative2DShader;
	renderer.Enable_Texturing(false);
	renderer.Add_Quad(RectClass(144.0f, 16.0f, 240.0f, 240.0f), 0xFF4080FF);
	renderer.Render();

	capture();
	// The exact blended colors depend on how the GPU rounds, so they are left to the image comparison of CI.
	EXPECT_EQ(pixel(8, 8), background);
	EXPECT_NE(pixel(48, 128), background);
	EXPECT_NE(pixel(128, 128), background);
	EXPECT_NE(pixel(208, 128), background);
}
