/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

// FILE: W3DSmudge.h /////////////////////////////////////////////////////////

#pragma once

#include <span>

#include "GameClient/Smudge.h"
#include "WWLib/sharebuf.h"

class SmudgeGroupClass;	//forward reference.
class Vector3;
class Vector4;
class W3DTextureHandle;
class W3DRenderContext;

//#define USE_COPY_RECTS	1	//this was the old method that didn't render to texture. Just copied backbuffer into texture. Slow on Nvidia.

class W3DSmudgeManager final : public SmudgeManager
{
public:
	W3DSmudgeManager();
	virtual ~W3DSmudgeManager() override;

	virtual void init() override;
	virtual void reset () override;

	virtual void ReleaseResources() override;
	virtual void ReAcquireResources() override;

	std::size_t Collect_Graphics_Smudges(std::span<float> position_x, std::span<float> position_y,
		std::span<float> position_z, std::span<float> offset_x, std::span<float> offset_y,
		std::span<float> sizes, std::span<float> opacities) const noexcept;

private:
};
