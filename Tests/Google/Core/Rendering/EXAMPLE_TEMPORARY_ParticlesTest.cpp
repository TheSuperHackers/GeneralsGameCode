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
#include "WW3D2/camera.h"
#include "WW3D2/pointgr.h"
#include "WW3D2/rinfo.h"
#include "WW3D2/texture.h"
#include "WW3D2/ww3d.h"
#include "WWLib/sharebuf.h"

// Particles are camera facing quads with a soft texture, which W3DParticleSystemManager draws in batches with
// PointGroupClass and additive blending.
TEST_F(RenderingTest, Particles)
{
	// A round puff that fades out towards its edge.
	TextureClass *puff = createTexture(32, 32, [](int x, int y) -> unsigned int
	{
		const float dx = (x - 15.5f) / 16.0f;
		const float dy = (y - 15.5f) / 16.0f;
		const unsigned int brightness = static_cast<unsigned int>(255.0f * max(0.0f, 1.0f - sqrtf(dx * dx + dy * dy)));
		return 0xFF000000 | brightness << 16 | brightness << 8 | brightness;
	});

	// A ring of particles on the ground that turn from red to yellow and grow, around one in the middle.
	constexpr int Count = 13;
	ShareBufferClass<Vector3> *locations = NEW_REF(ShareBufferClass<Vector3>, (Count, "ParticlesTest"));
	ShareBufferClass<Vector4> *colors = NEW_REF(ShareBufferClass<Vector4>, (Count, "ParticlesTest"));
	ShareBufferClass<float> *sizes = NEW_REF(ShareBufferClass<float>, (Count, "ParticlesTest"));
	for (int i = 0; i < Count; ++i)
	{
		const float angle = i * WWMATH_PI * 2.0f / (Count - 1);
		const float radius = i == 0 ? 0.0f : 4.0f;
		locations->Get_Array()[i] = Vector3(radius * cosf(angle), radius * sinf(angle), 0.0f);
		colors->Get_Array()[i] = Vector4(1.0f, static_cast<float>(i) / Count, 0.1f, 1.0f);
		sizes->Get_Array()[i] = 1.0f + i * 0.1f;
	}

	// Set up as W3DParticleSystemManager sets up its batches.
	PointGroupClass particles;
	particles.Set_Texture(puff);
	particles.Set_Shader(ShaderClass::_PresetAdditiveSpriteShader);
	particles.Set_Flag(PointGroupClass::TRANSFORM, true);
	particles.Set_Point_Mode(PointGroupClass::QUADS);
	particles.Set_Billboard(true);
	particles.Set_Arrays(locations, colors, nullptr, sizes, nullptr, nullptr, Count);

	// The game draws particles from its scene, so this renders them as WW3D::Render renders a scene.
	RenderInfoClass rinfo(*s_camera);
	s_camera->Apply();
	particles.Render(rinfo);
	WW3D::Flush(rinfo);
	REF_PTR_RELEASE(locations);
	REF_PTR_RELEASE(colors);
	REF_PTR_RELEASE(sizes);
	REF_PTR_RELEASE(puff);

	capture();
	EXPECT_EQ(pixel(8, 8), 0x000000);
	EXPECT_NE(pixel(Width / 2, Height / 2), 0x000000);
}
