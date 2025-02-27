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

// EA Pacific
// John McDonald, Jr
// Do not distribute

#pragma once

#include "Common/GameAudio.h"
#include "Common/GameMemory.h"

class AudioEventRTS;

enum RequestType CPP_11(: Int)
{
	AR_Play,
	AR_Pause,
	AR_Stop
};

struct AudioRequest : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AudioRequest, "AudioRequest" )

public:
	RequestType m_request;
	union
	{
		AudioEventRTS *m_pendingEvent;
		AudioHandle m_handleToInteractOn;
	};
	Bool m_usePendingEvent;
	Bool m_requiresCheckForSample;
};
