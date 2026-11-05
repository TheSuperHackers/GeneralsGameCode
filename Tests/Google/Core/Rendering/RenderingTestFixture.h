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
#include <windows.h>
#include <functional>
#include <initializer_list>
#include <vector>

class CameraClass;
class RenderObjClass;
class TextureClass;

// Each test renders with WW3D into a black frame and captures it for the rendering check of CI.
class RenderingTest : public ::testing::Test
{
protected:
	static constexpr int Width = 256;
	static constexpr int Height = 256;

	static void SetUpTestSuite();
	static void TearDownTestSuite();
	virtual void SetUp() override;
	virtual void TearDown() override;

	static void clear(unsigned int color);
	static void render(std::initializer_list<RenderObjClass *> objects);
	static TextureClass *createTexture(int width, int height, const std::function<unsigned int(int x, int y)> &color); // color(x, y) returns 0xAARRGGBB.
	void capture(); // Saves the frame as <test>_<n>.png and starts a new one.
	unsigned int pixel(int x, int y) const { return m_pixels[y * Width + x]; } // 0xRRGGBB of the last capture.

	static CameraClass *s_camera; // Looks at the origin from the south, with z up.

private:
	std::vector<unsigned int> m_pixels = std::vector<unsigned int>(Width * Height);
	int m_captures = 0;
	bool m_rendering = false;
	static HWND s_window;
};
