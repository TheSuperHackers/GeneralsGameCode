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
#include "WW3D2/segline.h"
#include "WW3D2/texture.h"

// Lasers are segmented lines with a glowing texture across their width, which W3DLaserDraw adds to the scene with
// additive blending, so crossing beams add up.
TEST_F(RenderingTest, Laser)
{
	// Brightest along the middle of the beam, dark at its edges.
	TextureClass *glow = createTexture(32, 32, [](int x, int) -> unsigned int
	{
		const unsigned int brightness = 255 - abs(2 * x - 31) * 255 / 31;
		return 0xFF000000 | brightness << 16 | brightness << 8 | brightness;
	});

	// A red beam from west to east and a blue one that crosses it on the way up.
	Vector3 eastWest[] = {Vector3(-6.0f, 0.0f, 0.0f), Vector3(6.0f, 0.0f, 0.0f)};
	Vector3 diagonal[] = {Vector3(-5.0f, -3.0f, 0.0f), Vector3(5.0f, 3.0f, 2.0f)};
	SegmentedLineClass *red = NEW_REF(SegmentedLineClass, ());
	red->Set_Points(2, eastWest);
	red->Set_Color(Vector3(1.0f, 0.2f, 0.1f));
	SegmentedLineClass *blue = NEW_REF(SegmentedLineClass, ());
	blue->Set_Points(2, diagonal);
	blue->Set_Color(Vector3(0.2f, 0.4f, 1.0f));
	for (SegmentedLineClass *laser : {red, blue})
	{
		laser->Set_Texture(glow);
		laser->Set_Shader(ShaderClass::_PresetAdditiveShader);
		laser->Set_Width(0.8f);
	}

	render({red, blue});
	REF_PTR_RELEASE(red);
	REF_PTR_RELEASE(blue);
	REF_PTR_RELEASE(glow);

	capture();
	EXPECT_EQ(pixel(8, 8), 0x000000);
	EXPECT_NE(pixel(Width / 2, Height / 2), 0x000000);
}
