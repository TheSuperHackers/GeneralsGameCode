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

// FILE: MissileLauncherBuildingUpdate.cpp /////////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, April 2002
// Desc:   Update will change model state conditions based on special power state
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameAudio.h"
#include "Common/GlobalData.h"
#include "Common/SpecialPower.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/MissileLauncherBuildingUpdate.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"


//-------------------------------------------------------------------------------------------------
MissileLauncherBuildingUpdate::MissileLauncherBuildingUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_doorState = DOOR_CLOSED;
	m_timeoutState = DOOR_CLOSED;
	m_timeoutFrame = 0;
	m_openIdleAudio = getMissileLauncherBuildingUpdateModuleData()->m_openIdleAudio;
	m_openIdleAudio.setObjectID(getObject()->getID());
}

//-------------------------------------------------------------------------------------------------
MissileLauncherBuildingUpdate::~MissileLauncherBuildingUpdate( void )
{
	if (m_openIdleAudio.isCurrentlyPlaying())
	{
		TheAudio->removeAudioEvent(m_openIdleAudio.getPlayingHandle());
	}
}

//-------------------------------------------------------------------------------------------------
void MissileLauncherBuildingUpdate::switchToState(DoorStateType dst)
{
	if (m_doorState == dst)
		return;

	const MissileLauncherBuildingUpdateModuleData* d = getMissileLauncherBuildingUpdateModuleData();
	ModelConditionFlags clr, set;
	UnsignedInt now = TheGameLogic->getFrame();
	switch (dst)
	{
		case DOOR_CLOSED:
			DEBUG_LOG(("switch to state DOOR_CLOSED at %d",now));
			/// @todo srj -- for now, this assumes at most one door
			clr.set(MODELCONDITION_DOOR_1_WAITING_TO_CLOSE);
			clr.set(MODELCONDITION_DOOR_1_CLOSING);
			clr.set(MODELCONDITION_DOOR_1_OPENING);
			clr.set(MODELCONDITION_DOOR_1_WAITING_OPEN);
			m_timeoutFrame = 0;
			m_timeoutState = DOOR_CLOSED;
			if (d->m_closedFX)
			{
				const Coord3D *pos = getObject()->getPosition();
				FXList::doFXPos(d->m_closedFX, pos);
			}
			if (m_openIdleAudio.isCurrentlyPlaying())
			{
				TheAudio->removeAudioEvent(m_openIdleAudio.getPlayingHandle());
			}
			break;

		case DOOR_OPENING:
			DEBUG_LOG(("switch to state DOOR_OPENING at %d",now));
			/// @todo srj -- for now, this assumes at most one door
			clr.set(MODELCONDITION_DOOR_1_WAITING_TO_CLOSE);
			clr.set(MODELCONDITION_DOOR_1_CLOSING);
			clr.set(MODELCONDITION_DOOR_1_WAITING_OPEN);
			set.set(MODELCONDITION_DOOR_1_OPENING);
			// we want this to be done BEFORE the power is ready, so
			// end it one frame ahead.
			m_timeoutFrame = m_specialPowerModule->getReadyFrame() - 1;
			m_timeoutState = DOOR_OPEN;
			if (d->m_openingFX)
			{
				const Coord3D *pos = getObject()->getPosition();
				FXList::doFXPos(d->m_openingFX, pos);
			}
			if (m_openIdleAudio.isCurrentlyPlaying())
			{
				TheAudio->removeAudioEvent(m_openIdleAudio.getPlayingHandle());
			}
			break;

		case DOOR_OPEN:
			DEBUG_LOG(("switch to state DOOR_OPEN at %d",now));
			/// @todo srj -- for now, this assumes at most one door
			clr.set(MODELCONDITION_DOOR_1_WAITING_TO_CLOSE);
			clr.set(MODELCONDITION_DOOR_1_CLOSING);
			clr.set(MODELCONDITION_DOOR_1_OPENING);
			set.set(MODELCONDITION_DOOR_1_WAITING_OPEN);
			m_timeoutFrame = 0;
			m_timeoutState = DOOR_OPEN;
			if (d->m_openFX)
			{
				const Coord3D *pos = getObject()->getPosition();
				FXList::doFXPos(d->m_openFX, pos);
			}
			if (!m_openIdleAudio.isCurrentlyPlaying())
			{
				m_openIdleAudio.setPlayingHandle(TheAudio->addAudioEvent(&m_openIdleAudio));
			}
			break;

		case DOOR_WAITING_TO_CLOSE:
			DEBUG_LOG(("switch to state DOOR_WAITING_TO_CLOSE at %d",now));
			/// @todo srj -- for now, this assumes at most one door
			clr.set(MODELCONDITION_DOOR_1_CLOSING);
			clr.set(MODELCONDITION_DOOR_1_OPENING);
			clr.set(MODELCONDITION_DOOR_1_WAITING_OPEN);
			set.set(MODELCONDITION_DOOR_1_WAITING_TO_CLOSE);
			m_timeoutFrame = now + d->m_doorWaitOpenTime;
			m_timeoutState = DOOR_CLOSING;
			if (d->m_waitingToCloseFX)
			{
				const Coord3D *pos = getObject()->getPosition();
				FXList::doFXPos(d->m_waitingToCloseFX, pos);
			}
			if (m_openIdleAudio.isCurrentlyPlaying())
			{
				TheAudio->removeAudioEvent(m_openIdleAudio.getPlayingHandle());
			}
			break;

		case DOOR_CLOSING:
			DEBUG_LOG(("switch to state DOOR_CLOSING at %d",now));
			/// @todo srj -- for now, this assumes at most one door
			clr.set(MODELCONDITION_DOOR_1_WAITING_TO_CLOSE);
			clr.set(MODELCONDITION_DOOR_1_WAITING_OPEN);
			clr.set(MODELCONDITION_DOOR_1_OPENING);
			set.set(MODELCONDITION_DOOR_1_CLOSING);
			m_timeoutFrame = now + d->m_doorClosingTime;

			{
				Int delta = m_specialPowerModule->getReadyFrame() - now;
				if (m_timeoutFrame > now + delta/2)
					m_timeoutFrame = now + delta/2;
			}

			m_timeoutState = DOOR_CLOSED;
			if (d->m_closingFX)
			{
				const Coord3D *pos = getObject()->getPosition();
				FXList::doFXPos(d->m_closingFX, pos);
			}
			if (m_openIdleAudio.isCurrentlyPlaying())
			{
				TheAudio->removeAudioEvent(m_openIdleAudio.getPlayingHandle());
			}
			break;
#ifdef RTS_DEBUG
		default:
			DEBUG_CRASH(("unknown state"));
			break;
#endif
	}
	m_doorState = dst;
	getObject()->clearAndSetModelConditionFlags(clr, set);

	if (m_timeoutFrame > 0)
	{
		Drawable* draw = getObject()->getDrawable();
		if (draw)
			draw->setAnimationLoopDuration(m_timeoutFrame - now);
	}
}

//-------------------------------------------------------------------------------------------------
Bool MissileLauncherBuildingUpdate::initiateIntentToDoSpecialPower( const SpecialPowerTemplate *specialPowerTemplate, const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions )
{
#if defined(RTS_DEBUG)
	DEBUG_ASSERTCRASH(!TheGlobalData->m_specialPowerUsesDelay || m_doorState == DOOR_OPEN, ("door is not fully open when specialpower is fired!"));
#endif

	switchToState(DOOR_WAITING_TO_CLOSE);
	return TRUE;
}

//-------------------------------------------------------------------------------------------------
Bool MissileLauncherBuildingUpdate::isPowerCurrentlyInUse( const CommandButton *command ) const
{
	//@todo -- Implement me in such a way that it still works with Ctrl+s cheat key...
	//In fact, this code doesn't really care because in legit mode, the game will
	//never require this check to disable the cameo button.
	return false;
}

//-------------------------------------------------------------------------------------------------
UpdateSleepTime MissileLauncherBuildingUpdate::update( void )
{
	const MissileLauncherBuildingUpdateModuleData* d = getMissileLauncherBuildingUpdateModuleData();

	UnsignedInt now = TheGameLogic->getFrame();

	// If we are under construction, any decision we make about door status could be wrong.
	// Our special power module is randomly going to be initialized or not (which would result
	// in him reporting a 0 frame ready, which means we will start open).
	if( getObject()->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) )
		return UPDATE_SLEEP_NONE;

	if (!m_specialPowerModule)
	{
		m_specialPowerModule = getObject()->getSpecialPowerModule(d->m_specialPowerTemplate);
		DEBUG_ASSERTCRASH(m_specialPowerModule, ("Missing special power"));
	}

	if (m_specialPowerModule)
	{
		UnsignedInt readyFrame = m_specialPowerModule->getReadyFrame();
		UnsignedInt whenToStartOpening = (readyFrame >= d->m_doorOpenTime) ? (readyFrame - d->m_doorOpenTime) : 0;

		if (m_timeoutFrame != 0 && now > m_timeoutFrame)
		{
			switchToState(m_timeoutState);
		}

#if defined(RTS_DEBUG)
		DEBUG_ASSERTCRASH(!TheGlobalData->m_specialPowerUsesDelay || !(m_specialPowerModule->isReady() && m_doorState != DOOR_OPEN), ("door is not fully open when specialpower is ready!"));
#endif

		if (m_doorState != DOOR_OPEN && m_specialPowerModule->isReady())
		{
			DEBUG_LOG(("*** had to POP the door open!"));
			switchToState(DOOR_OPEN);
		}
		else if (m_doorState == DOOR_CLOSED && now >= whenToStartOpening)
		{
			switchToState(DOOR_OPENING);
		}
	}
	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
SpecialPowerTemplate* MissileLauncherBuildingUpdate::getTemplate() const
{
	const MissileLauncherBuildingUpdateModuleData* data = getMissileLauncherBuildingUpdateModuleData();
	return data->m_specialPowerTemplate;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void MissileLauncherBuildingUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void MissileLauncherBuildingUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// do not need to tie the m_specialPowerModule pointer cause it gets tied
	// SpecialPowerModuleInterface *m_specialPowerModule;

	// door state
	xfer->xferUser( &m_doorState, sizeof( DoorStateType ) );

	// timeout state
	xfer->xferUser( &m_timeoutState, sizeof( DoorStateType ) );

	// timeout frame
	xfer->xferUnsignedInt( &m_timeoutFrame );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void MissileLauncherBuildingUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
