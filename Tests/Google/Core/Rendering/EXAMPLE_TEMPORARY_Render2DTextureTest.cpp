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
#include "WW3D2/texture.h"

// The UI draws its images as textured quads with Render2DClass. Scaled up, point filtering keeps the texels sharp while
// linear filtering blends them, and transparent texels let the background show through.
TEST_F(RenderingTest, Render2DTexture)
{
	const unsigned int background = 0x204060;
	clear(background);

	// A red and white checkerboard with a transparent top left quarter.
	const auto checkerboard = [](int x, int y) -> unsigned int
	{
		const unsigned int alpha = (x < 4 && y < 4) ? 0x00000000 : 0xFF000000;
		return alpha | (((x ^ y) & 1) ? 0xFFFFFF : 0xC02020);
	};
	TextureClass *sharp = createTexture(8, 8, checkerboard);
	sharp->Get_Filter().Set_Min_Filter(TextureFilterClass::FILTER_TYPE_NONE);
	sharp->Get_Filter().Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_NONE);
	TextureClass *smooth = createTexture(8, 8, checkerboard);
	smooth->Get_Filter().Set_Min_Filter(TextureFilterClass::FILTER_TYPE_FAST);
	smooth->Get_Filter().Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_FAST);

	// The sharp image on the left and the smooth one on the right, both scaled from 8 to 96 pixels.
	Render2DClass renderer(sharp);
	renderer.Set_Coordinate_Range(RectClass(0.0f, 0.0f, Width, Height));
	renderer.Add_Quad(RectClass(16.0f, 64.0f, 112.0f, 160.0f));
	renderer.Render();
	renderer.Reset();
	renderer.Set_Texture(smooth);
	renderer.Add_Quad(RectClass(144.0f, 64.0f, 240.0f, 160.0f));
	renderer.Render();
	REF_PTR_RELEASE(sharp);
	REF_PTR_RELEASE(smooth);

	capture();
	// Each texel covers 12 by 12 pixels. These are the centers of texel (5, 5), which is red, and texel (1, 1),
	// which is transparent.
	EXPECT_EQ(pixel(16 + 5 * 12 + 6, 64 + 5 * 12 + 6), 0xC02020);
	EXPECT_EQ(pixel(16 + 1 * 12 + 6, 64 + 1 * 12 + 6), background);
}
