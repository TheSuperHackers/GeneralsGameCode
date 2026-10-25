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
#include "WW3D2/line3d.h"

// Tracers are 3D lines, which W3DTracerDraw fades out by lowering their opacity.
TEST_F(RenderingTest, Tracer)
{
	// A solid yellow tracer, crossed by a red one at half opacity.
	Line3DClass *solid = NEW_REF(Line3DClass, (Vector3(-6.0f, -2.0f, 0.0f), Vector3(6.0f, 2.0f, 0.0f), 0.2f, 1.0f, 0.9f, 0.2f));
	Line3DClass *faded = NEW_REF(Line3DClass, (Vector3(-6.0f, 2.0f, 0.0f), Vector3(6.0f, -2.0f, 0.0f), 0.2f, 1.0f, 0.3f, 0.2f, 0.5f));

	render({solid, faded});
	REF_PTR_RELEASE(solid);
	REF_PTR_RELEASE(faded);

	capture();
	EXPECT_EQ(pixel(8, 8), 0x000000);
	EXPECT_NE(pixel(Width / 2, Height / 2), 0x000000);
}
