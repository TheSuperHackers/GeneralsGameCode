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

// FILE: FlammableUpdate.cpp /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, April 2002
// Desc:   Update that manages Aflame and Burned statuses and their effects
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/AudioEventRTS.h"
#include "Common/GameAudio.h"
#include "Common/Xfer.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/FlammableUpdate.h"
#include "GameLogic/Module/FireSpreadUpdate.h"

//-------------------------------------------------------------------------------------------------
FlammableUpdateModuleData::FlammableUpdateModuleData()
{
	m_burnedDelay = 0;
	m_aflameDuration = 0;
	m_aflameDamageDelay = 0;
	m_aflameDamageAmount = 0;
	// Enabled By Sadullah Nader
	// Initialization needed
	m_burningSoundName.clear();
	//
	m_flameDamageLimitData = 20.0f;
	m_flameDamageExpirationDelay = LOGICFRAMES_PER_SECOND * 2;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void FlammableUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "BurnedDelay",						INI::parseDurationUnsignedInt,	NULL, offsetof( FlammableUpdateModuleData, m_burnedDelay ) },
		{ "AflameDuration",					INI::parseDurationUnsignedInt,	NULL, offsetof( FlammableUpdateModuleData, m_aflameDuration ) },
		{ "AflameDamageDelay",			INI::parseDurationUnsignedInt,	NULL, offsetof( FlammableUpdateModuleData, m_aflameDamageDelay ) },
		{ "AflameDamageAmount",			INI::parseInt,									NULL, offsetof( FlammableUpdateModuleData, m_aflameDamageAmount ) },
		{ "BurningSoundName",				INI::parseAsciiString,					NULL,	offsetof( FlammableUpdateModuleData, m_burningSoundName) },
		{ "FlameDamageLimit",				INI::parseReal,									NULL,	offsetof( FlammableUpdateModuleData, m_flameDamageLimitData ) },
		{ "FlameDamageExpiration",	INI::parseDurationUnsignedInt,	NULL,	offsetof( FlammableUpdateModuleData, m_flameDamageExpirationDelay ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FlammableUpdate::FlammableUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_status = FS_NORMAL;
	m_aflameEndFrame = 0;
	m_burnedEndFrame = 0;
	m_damageEndFrame = 0;
	m_audioHandle = NULL;
	m_flameDamageLimit = getFlammableUpdateModuleData()->m_flameDamageLimitData;
	m_flameSource = INVALID_ID;
	m_lastFlameDamageDealt = 0;

	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FlammableUpdate::~FlammableUpdate( void )
{
	stopBurningSound();
}

//-------------------------------------------------------------------------------------------------
/** Damage has been dealt, this is an opportunity to reach to that damage */
//-------------------------------------------------------------------------------------------------
void FlammableUpdate::onDamage( DamageInfo *damageInfo )
{
	if( damageInfo->in.m_damageType == DAMAGE_FLAME || damageInfo->in.m_damageType == DAMAGE_PARTICLE_BEAM )
	{
		UnsignedInt now = TheGameLogic->getFrame();
		if( now - getFlammableUpdateModuleData()->m_flameDamageExpirationDelay > m_lastFlameDamageDealt )
		{
			// If it has been a long time since our last flame damage, reset the threshold
			m_flameDamageLimit = getFlammableUpdateModuleData()->m_flameDamageLimitData;
		}
		m_lastFlameDamageDealt = now;
#if RETAIL_COMPATIBLE_CRC
		m_flameSource = getObject()->getID();
#else
		// TheSuperHackers @bugfix Stubbjax 03/09/2025 Allow flame damage to award xp to the flame source.
		m_flameSource = damageInfo->in.m_sourceID;
#endif

		Object *me = getObject();
		if( !me->getStatusBits().test( OBJECT_STATUS_AFLAME ) && !me->getStatusBits().test( OBJECT_STATUS_BURNED ) )
		{
			// If I'm not on fire, and I haven't burned up, see if I should try to catch fire.
			m_flameDamageLimit -= damageInfo->out.m_actualDamageDealt;
			if( m_flameDamageLimit <= 0 )
			{
				tryToIgnite();
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime FlammableUpdate::update( void )
{
	Object *me = getObject();
	DEBUG_ASSERTCRASH(m_status == FS_AFLAME, ("hmm, should be aflame"));

	UnsignedInt now = TheGameLogic->getFrame();
	const FlammableUpdateModuleData *data = getFlammableUpdateModuleData();

	if( m_damageEndFrame != 0 && now >= m_damageEndFrame )
	{
		m_damageEndFrame = now + data->m_aflameDamageDelay;
		doAflameDamage();
	}

	if( m_burnedEndFrame != 0 && now >= m_burnedEndFrame )
	{
		// So this status is set, but I am still aflame on an independent timer.
		me->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_BURNED ) );
		me->setModelConditionState( MODELCONDITION_SMOLDERING );
	}

	if( m_aflameEndFrame != 0 && now >= m_aflameEndFrame )
	{
		// This is the important one.  I am no longer on fire.
		if( me->getStatusBits().test( OBJECT_STATUS_BURNED ) )
		{
			// If I am burned, then I will never catch fire again.
			m_status = FS_BURNED;
		}
		else
		{
			// otherwise I am free to burn again
			m_status = FS_NORMAL;
		}
		stopBurningSound();
		me->clearStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_AFLAME ) );
		me->getBodyModule()->setAflame( FALSE );
		me->clearModelConditionState( MODELCONDITION_AFLAME );
	}

	return calcSleepTime();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime FlammableUpdate::calcSleepTime()
{
	UnsignedInt now = TheGameLogic->getFrame();
	if (m_status == FS_AFLAME && m_aflameEndFrame != 0 && m_aflameEndFrame > now)
	{
		UnsignedInt soonest = m_aflameEndFrame;
		if (m_burnedEndFrame != 0 && m_burnedEndFrame < soonest && m_burnedEndFrame > now) soonest = m_burnedEndFrame;
		if (m_damageEndFrame != 0 && m_damageEndFrame < soonest && m_damageEndFrame > now) soonest = m_damageEndFrame;
		DEBUG_ASSERTCRASH(soonest - now > 0, ("hmm"));
		// UPDATE_SLEEP requires a count-of-frames, not an absolute-frame, so subtract 'now'
		return UPDATE_SLEEP(soonest - now);
	}
	else
	{
		return UPDATE_SLEEP_FOREVER;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void FlammableUpdate::tryToIgnite()
{
	if( m_status == FS_NORMAL )
	{
		Object *me = getObject();
		me->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_AFLAME ) );
		me->getBodyModule()->setAflame( TRUE );
		me->setModelConditionState( MODELCONDITION_AFLAME );
		startBurningSound();

		// bleah. this sucks. (srj)
		static const NameKeyType key_FireSpreadUpdate = NAMEKEY("FireSpreadUpdate");
		FireSpreadUpdate* fu = (FireSpreadUpdate*)getObject()->findUpdateModule(key_FireSpreadUpdate);
		if (fu != NULL)
		{
			fu->startFireSpreading();
		}

		m_status = FS_AFLAME;

		const FlammableUpdateModuleData *data = getFlammableUpdateModuleData();
		UnsignedInt now = TheGameLogic->getFrame();
		m_aflameEndFrame = now + data->m_aflameDuration;
		m_burnedEndFrame = data->m_burnedDelay ? now + data->m_burnedDelay : 0;
		m_damageEndFrame = data->m_aflameDamageDelay ? now + data->m_aflameDamageDelay : 0;

		setWakeFrame(getObject(), calcSleepTime());
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void FlammableUpdate::doAflameDamage()
{
	const FlammableUpdateModuleData *data = getFlammableUpdateModuleData();

	DamageInfo info;
	info.in.m_amount = data->m_aflameDamageAmount;
	info.in.m_sourceID = m_flameSource;
	info.in.m_damageType = DAMAGE_FLAME;
	info.in.m_deathType = DEATH_BURNED;

	getObject()->attemptDamage( &info );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void FlammableUpdate::startBurningSound()
{
	const FlammableUpdateModuleData *data = getFlammableUpdateModuleData();

	AudioEventRTS audio(data->m_burningSoundName, getObject()->getID());
	m_audioHandle = TheAudio->addAudioEvent( &audio );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void FlammableUpdate::stopBurningSound()
{
	if (m_audioHandle)
	{
		TheAudio->removeAudioEvent( m_audioHandle );
		m_audioHandle = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool FlammableUpdate::wouldIgnite()
{
	if( m_status == FS_NORMAL )
		return TRUE;

	return FALSE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void FlammableUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void FlammableUpdate::xfer( Xfer *xfer )
{

	// version
#if RETAIL_COMPATIBLE_XFER_SAVE
	XferVersion currentVersion = 1;
#else
	XferVersion currentVersion = 2;
#endif
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// flammability status
	xfer->xferUser( &m_status, sizeof( FlammabilityStatusType ) );

	// aflame end frame
	xfer->xferUnsignedInt( &m_aflameEndFrame );

	// burned end frame
	xfer->xferUnsignedInt( &m_burnedEndFrame );

	// damage end frame
	xfer->xferUnsignedInt( &m_damageEndFrame );

	// flame damage limit
	xfer->xferReal( &m_flameDamageLimit );

	// last flame damage dealt
	xfer->xferUnsignedInt( &m_lastFlameDamageDealt );

#if !RETAIL_COMPATIBLE_XFER_SAVE
	if (version >= 2)
	{
		xfer->xferObjectID(&m_flameSource);
	}
#endif
}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void FlammableUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
