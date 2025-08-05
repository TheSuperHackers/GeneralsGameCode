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


#pragma once

#ifndef __FRAMEDATAMANAGER_H
#define __FRAMEDATAMANAGER_H

#include "GameNetwork/NetworkDefs.h"
#include "GameNetwork/FrameData.h"

class FrameDataManager : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(FrameDataManager, "FrameDataManager")
public:
	FrameDataManager(Bool isLocal);
	//virtual ~FrameDataManager();

	void init();
	void reset();
	void update();

	void addNetCommandMsg(NetCommandMsg *msg);
	void setIsLocal(Bool isLocal);
	FrameDataReturnType allCommandsReady(UnsignedInt frame, Bool debugSpewage);
	NetCommandList * getFrameCommandList(UnsignedInt frame);
	UnsignedInt getCommandCount(UnsignedInt frame);
	void setFrameCommandCount(UnsignedInt frame, UnsignedInt commandCount);
	UnsignedInt getFrameCommandCount(UnsignedInt frame);
	void zeroFrames(UnsignedInt startingFrame, UnsignedInt numFrames);
	void destroyGameMessages();
	void resetFrame(UnsignedInt frame, Bool isAdvancing = TRUE);
	void setQuitFrame(UnsignedInt frame);
	UnsignedInt getQuitFrame();
	Bool getIsQuitting();

protected:
	FrameData *m_frameData;
	Bool m_isLocal;

	Bool m_isQuitting;
	UnsignedInt m_quitFrame;
};

#endif
