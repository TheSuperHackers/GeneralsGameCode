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

#ifndef __FRAMEDATA_H
#define __FRAMEDATA_H

#include "Lib/BaseType.h"
#include "GameNetwork/NetCommandList.h"

enum FrameDataReturnType CPP_11(: Int) {
	FRAMEDATA_NOTREADY,
	FRAMEDATA_RESEND,
	FRAMEDATA_READY
};

class FrameData {
public:
	FrameData();
	~FrameData();

	void init();
	void reset();
	void update();

	UnsignedInt getFrame();
	void setFrame(UnsignedInt frame);
	FrameDataReturnType allCommandsReady(Bool debugSpewage);
	NetCommandList * getCommandList();
	void setFrameCommandCount(UnsignedInt totalCommandCount);
	UnsignedInt getFrameCommandCount();
	void addCommand(NetCommandMsg *msg);
	UnsignedInt getCommandCount();
	void zeroFrame();
	void destroyGameMessages();

protected:
	UnsignedInt m_frame;
	UnsignedInt m_frameCommandCount;
	UnsignedInt m_commandCount;
	NetCommandList *m_commandList;
	UnsignedInt m_lastFailedCC;
	UnsignedInt m_lastFailedFrameCC;
};

#endif
