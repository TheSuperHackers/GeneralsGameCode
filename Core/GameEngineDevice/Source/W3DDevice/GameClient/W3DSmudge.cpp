
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// W3DSmudge.cpp ////////////////////////////////////////////////////////////////////////////////
// Smudge System implementation
// Author: Mark Wilczynski, June 2003
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Lib/BaseType.h"
#include "WWLib/always.h"
#include "W3DDevice/GameClient/W3DSmudge.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "Common/GameMemory.h"
#include "GameClient/View.h"
#include "GameClient/Display.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DCamera.h"

#include <cstddef>
#include <span>


SmudgeManager *TheSmudgeManager=nullptr;

W3DSmudgeManager::W3DSmudgeManager()
{
}

W3DSmudgeManager::~W3DSmudgeManager()
{
	ReleaseResources();
}

void W3DSmudgeManager::init()
{
	SmudgeManager::init();
	ReAcquireResources();
}

void W3DSmudgeManager::reset ()
{
	SmudgeManager::reset();	//base
}

void W3DSmudgeManager::ReleaseResources()
{
}


#define SMUDGE_DRAW_SIZE	500	//draw at most 50 smudges per call. Tweak value to improve CPU/GPU parallelism.

static_assert(SMUDGE_DRAW_SIZE * 5 < 0x10000, "Vertex index exceeds 16-bit limit");


void W3DSmudgeManager::ReAcquireResources()
{
}

std::size_t W3DSmudgeManager::Collect_Graphics_Smudges(std::span<float> position_x, std::span<float> position_y,
	std::span<float> position_z, std::span<float> offset_x, std::span<float> offset_y,
	std::span<float> sizes, std::span<float> opacities) const noexcept
{
	const std::size_t capacity = position_x.size();
	if (position_y.size() != capacity || position_z.size() != capacity || offset_x.size() != capacity
		|| offset_y.size() != capacity || sizes.size() != capacity || opacities.size() != capacity)
		return 0;

	std::size_t count = 0;
	for (SmudgeSet *set : m_usedSmudgeSetList) {
		if (set == nullptr)
			continue;
		for (Smudge *smudge : set->getUsedSmudgeList()) {
			if (smudge == nullptr || !smudge->m_draw || count >= capacity)
				continue;
			position_x[count] = smudge->m_pos.X;
			position_y[count] = smudge->m_pos.Y;
			position_z[count] = smudge->m_pos.Z;
			offset_x[count] = smudge->m_offset.X;
			offset_y[count] = smudge->m_offset.Y;
			sizes[count] = smudge->m_size*0.5f;
			opacities[count] = smudge->m_opacity;
			++count;
		}
	}
	return count;
}

