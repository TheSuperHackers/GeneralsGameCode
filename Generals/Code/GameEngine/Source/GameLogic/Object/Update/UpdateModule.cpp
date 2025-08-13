/*
**	Command & Conquer Generals(tm)
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

// FILE: UpdateModule.cpp /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2002
// Desc:   Update module base class
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/UpdateModule.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime UpdateModule::frameToSleepTime(
	UnsignedInt frame1,
	UnsignedInt frame2,
	UnsignedInt frame3,
	UnsignedInt frame4
)
{
	DEBUG_ASSERTCRASH(frame1 != 0 && frame2 != 0 && frame3 != 0 && frame4 != 0, ("probably should not pass zero to frameToSleepTime."));
	if (frame1 > frame2) frame1 = frame2;
	if (frame1 > frame3) frame1 = frame3;
	if (frame1 > frame4) frame1 = frame4;
	UnsignedInt now = TheGameLogic->getFrame();
	if (frame1 > now)
	{
		return UPDATE_SLEEP(frame1 - now);
	}
	else if (frame1 == now)
	{
		return UPDATE_SLEEP_NONE;
	}
	else
	{
		DEBUG_CRASH(("frameToSleepTime: frame is in the past. oops."));
		return UPDATE_SLEEP_NONE;
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime UpdateModule::getWakeFrame() const
{
	UnsignedInt now = TheGameLogic->getFrame();
	UnsignedInt nextCallFrame = m_nextCallFrameAndPhase >> 2;
	if (nextCallFrame > now)
		return UPDATE_SLEEP(nextCallFrame - now);
	else
		return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UpdateModule::setWakeFrame(Object* obj, UpdateSleepTime wakeDelay)
{
	UnsignedInt now = TheGameLogic->getFrame();
	TheGameLogic->friend_awakenUpdateModule(obj, this, now + (UnsignedInt)wakeDelay);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void UpdateModule::crc( Xfer *xfer )
{

	// extend base class
	BehaviorModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info;
	* 1: Initial version
	*/
// ------------------------------------------------------------------------------------------------
void UpdateModule::xfer( Xfer *xfer )
{

	// version
	const XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	BehaviorModule::xfer( xfer );

#if defined(RTS_DEBUG)
	/*
		this is a fix for the following scenario:

		save a game that has an object that uses module "FOO"
		now change the code for module "FOO" from being nonsleepy to being sleepy
		now reload that saved game
		as soon as "FOO" attempts to wake itself up, you're dead.

		this fix simply looks to see if the module in question is now sleepy in code,
		but was saved in sleepy form, and if so, quietly nudges a reasonable
		value into m_nextCallFrameAndPhase.
	*/
	#define FIX_OLD_SAVES
#endif

#ifdef FIX_OLD_SAVES
	#ifdef ALLOW_NONSLEEPY_UPDATES
		Bool thisModuleIsNowSleepy = (m_nextCallFrameAndPhase != 0);
	#else
		const Bool thisModuleIsNowSleepy = true;
	#endif
#endif

	// next call frame and phase
	xfer->xferUnsignedInt( &m_nextCallFrameAndPhase );

	if (xfer->getXferMode() == XFER_LOAD)
	{
		/*
			ensure the phase matches.

			pre-Jan 2, 2003:
				PHASE_INITIAL				= 0,
				PHASE_NORMAL				= 1,
				PHASE_FINAL					= 2

			post-Jan 2, 2003:
				PHASE_INITIAL				= 0,
				PHASE_PHYSICS				= 1,
				PHASE_NORMAL				= 2,
				PHASE_FINAL					= 3
		*/
		m_nextCallFrameAndPhase &= ~3;
		m_nextCallFrameAndPhase |= getUpdatePhase();
	}

#ifdef FIX_OLD_SAVES
	if (xfer->getXferMode() == XFER_LOAD
			&& thisModuleIsNowSleepy
			&& m_nextCallFrameAndPhase == 0)
	{
	#ifdef ALLOW_NONSLEEPY_UPDATES
		// when the file was saved, this module was nonsleepy, but now it is sleepy.
	#else
		// note that 'm_nextCallFrameAndPhase' should only be zero for legacy save files.
	#endif
		friend_setNextCallFrame(TheGameLogic->getFrame());
	}
#endif

	// m_indexInLogic is not saved -- it's restored in gamelogic::postprocess.
	if( xfer->getXferMode() == XFER_LOAD )
	{
		m_indexInLogic = -1;
	}

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void UpdateModule::loadPostProcess( void )
{

	// extned base class
	BehaviorModule::loadPostProcess();

}  // end loadPostProcess





