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

// FILE: ObjectDefectionHelper.cpp ////////////////////////////////////////////////////////////////
// Author: Colin Day, Steven Johnson - September 2002
// Desc:   Object helper module
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/MiscAudio.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ObjectDefectionHelper.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ObjectDefectionHelper::~ObjectDefectionHelper( void )
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime ObjectDefectionHelper::update()
{
	Object* obj = getObject();
	Drawable* draw = obj->getDrawable();

	// if we are here, we should be an undetected defector, but
	// just in case someone has changed us behind our backs...
	if (!obj->getIsUndetectedDefector())
		return UPDATE_SLEEP_FOREVER; 

	UnsignedInt now = TheGameLogic->getFrame();
	if (now >= m_defectionDetectionEnd)
	{
		obj->friend_setUndetectedDefector(FALSE);

		// timer has reached zero, so we flash white once!!!! -- lorenzen
		if (draw && m_doDefectorFX)
		{
			RGBColor white = {1,1,1};
			if (draw)
				draw->flashAsSelected( &white ); //Whew! that's easier, now, isn't it!

		
			AudioEventRTS defectorVulnerableSound = TheAudio->getMiscAudio()->m_defectorTimerDingSound;
			defectorVulnerableSound.setObjectID( obj->getID() );
			TheAudio->addAudioEvent(&defectorVulnerableSound);
		}
		return UPDATE_SLEEP_FOREVER;	// hey, we're done. 
	}

	// dead or attacking... our cover is blown.
	if( obj->isEffectivelyDead() || obj->getStatusBits().test( OBJECT_STATUS_IS_FIRING_WEAPON ) )
	{
		// PLEASE NOTE:
		// checking the is_attacking statusbit above, only handles weapon related attacks...
		// I also set m_undetectedDefector = FALSE in the groupDoSpecialPower[...]( ) functions;
		obj->friend_setUndetectedDefector(FALSE);
		return UPDATE_SLEEP_FOREVER;	// hey, we're done. 
	}

	if (draw && m_doDefectorFX) // skip fx if merely 'invulnerable'
	{
		Bool lastPhase = ( ((Int)m_defectionDetectionFlashPhase) & 1 );// were we in a flashy phase last frame?
		UnsignedInt timeLeft = m_defectionDetectionEnd - now;
		m_defectionDetectionFlashPhase += 0.5f * ( 1.0f - ((Real)timeLeft / DEFECTION_DETECTION_TIME_MAX ) );
		Bool thisPhase = ( ((Int)m_defectionDetectionFlashPhase) & 1 );// are we in a flashy phase this frame?

		if ( lastPhase && ( ! thisPhase ) ) 
		{
			draw->flashAsSelected(); //Whew! that's easier, now, isn't it!

			AudioEventRTS defectorTimerSound = TheAudio->getMiscAudio()->m_defectorTimerTickSound;
			defectorTimerSound.setObjectID( obj->getID() );
			TheAudio->addAudioEvent(&defectorTimerSound);

		}
	}

	return UPDATE_SLEEP_NONE; 
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ObjectDefectionHelper::startDefectionTimer(UnsignedInt numFrames, Bool withDefectorFX)
{
	Object* obj = getObject();

	if (!obj->getIsUndetectedDefector())
	{
		setWakeFrame(obj, UPDATE_SLEEP_FOREVER);
		return;
	}

	UnsignedInt now = TheGameLogic->getFrame();
	m_defectionDetectionStart = now;
	m_defectionDetectionEnd = now + numFrames;
	m_defectionDetectionFlashPhase = 0.0f;
	m_doDefectorFX = withDefectorFX;

	setWakeFrame(obj, UPDATE_SLEEP_NONE);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ObjectDefectionHelper::crc( Xfer *xfer )
{

	// object helper crc
	ObjectHelper::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info;
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ObjectDefectionHelper::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// object helper base class
	ObjectHelper::xfer( xfer );

	// detection start
	xfer->xferUnsignedInt( &m_defectionDetectionStart );

	// detection end
	xfer->xferUnsignedInt( &m_defectionDetectionEnd );

	// flash phase
	xfer->xferReal( &m_defectionDetectionFlashPhase );

	// do defector fx
	xfer->xferBool( &m_doDefectorFX );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ObjectDefectionHelper::loadPostProcess( void )
{

	// object helper base class
	ObjectHelper::loadPostProcess();

}  // end loadPostProcess
