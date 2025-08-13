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

// FILE: CrushDie.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   Crush Die module
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/RandomValue.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/CrushDie.h"

//-------------------------------------------------------------------------------------------------
// Figure out which crush point was hit so the correct crushed object can be swapped in

// Figure out which crush point was hit so the correct crushed object can be swapped in
static CrushEnum crushLocationCheck( Object* crusherObject, Object* victimObject )
{
	if( (crusherObject == NULL)  ||  (victimObject == NULL) )
		return NO_CRUSH;

	Bool frontCrushed = victimObject->getBodyModule()->getFrontCrushed();
	Bool backCrushed = victimObject->getBodyModule()->getBackCrushed();

//	const Coord3D *dir = crusherObject->getUnitDirectionVector2D();
	const Coord3D *otherDir = victimObject->getUnitDirectionVector2D();
	const Coord3D *pos = crusherObject->getPosition();
	const Coord3D *otherPos = victimObject->getPosition();

	Real crushPointOffsetDistance = victimObject->getGeometryInfo().getMajorRadius() * 0.5;

	Coord3D crushPointOffset;
	crushPointOffset.x = otherDir->x * crushPointOffsetDistance;
	crushPointOffset.y = otherDir->y * crushPointOffsetDistance;
	crushPointOffset.z = 0;

	Coord3D comparisonCoord;
	Real dx, dy;

	CrushEnum retval = NO_CRUSH;
	Real bestDist = 99999;

	// PhysicsCollide has already done the logic of which point to smoosh and waited until we crossed that point
	// so at this point we just need to know which crush point is closest.

	if( !frontCrushed && !backCrushed )
	{
		// Check the middle crush point
		comparisonCoord = *otherPos;//copy so can move to each crush point

		dx = comparisonCoord.x - pos->x;
		dy = comparisonCoord.y - pos->y;
		Real dist = (Real)( dx*dx + dy*dy );

		//otherwise we want to make sure we get the closest valid crush point
		retval = TOTAL_CRUSH;
		bestDist = dist;
	}

	if( !frontCrushed )
	{
		// Check the front point.
		comparisonCoord = *otherPos;
		comparisonCoord.x += crushPointOffset.x;
		comparisonCoord.y += crushPointOffset.y;

		dx = comparisonCoord.x - pos->x;
		dy = comparisonCoord.y - pos->y;
		Real dist = (Real)( dx*dx + dy*dy );

		if( dist < bestDist )//closer
		{
			if( backCrushed )
			{
				retval = TOTAL_CRUSH;
				bestDist = dist;
			}
			else
			{
				retval = FRONT_END_CRUSH;
				bestDist = dist;
			}
		}
	}

	if( !backCrushed )
	{
		// Check back point
		comparisonCoord = *otherPos;
		comparisonCoord.x -= crushPointOffset.x;
		comparisonCoord.y -= crushPointOffset.y;

		dx = comparisonCoord.x - pos->x;
		dy = comparisonCoord.y - pos->y;
		Real dist = (Real)( dx*dx + dy*dy );

		if( dist < bestDist )//closer
		{
			if( frontCrushed )
			{
				retval = TOTAL_CRUSH;
				bestDist = dist;
			}
			else
			{
				retval = BACK_END_CRUSH;
				bestDist = dist;
			}
		}
	}

	return retval;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CrushDie::CrushDie( Thing *thing, const ModuleData* moduleData ) : DieModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CrushDie::~CrushDie( void )
{

}

//-------------------------------------------------------------------------------------------------
/** The die callback. */
//-------------------------------------------------------------------------------------------------
void CrushDie::onDie( const DamageInfo * damageInfo )
{
	if (!isDieApplicable(damageInfo))
		return;

	DEBUG_ASSERTCRASH(damageInfo->in.m_damageType == DAMAGE_CRUSH, ("this should only be used for crush damage"));
	if (damageInfo->in.m_damageType != DAMAGE_CRUSH)
		return;

	Object *damageDealer = TheGameLogic->findObjectByID( damageInfo->in.m_sourceID );
	DEBUG_ASSERTCRASH(damageDealer,("You must have a damageDealer source for this effect"));

	CrushEnum crushType = damageDealer ? crushLocationCheck(damageDealer, getObject()) : TOTAL_CRUSH;

	if (crushType != NO_CRUSH)
	{
		if (getCrushDieModuleData()->m_crushSounds[crushType].getEventName().isEmpty() == false)
		{
			// be sure that 0==never, 100==always
			// MDC: moving to GameLogicRandomValue.  This does not need to be synced, but having it so makes searches *so* much nicer.
			if (GameLogicRandomValue(0, 99) < getCrushDieModuleData()->m_crushSoundPercent[crushType])
			{
				AudioEventRTS crushSound(getCrushDieModuleData()->m_crushSounds[crushType]);
				crushSound.setObjectID(getObject()->getID());
				TheAudio->addAudioEvent(&crushSound);
			}
		}
		{
			Object *me = getObject();

			if (me)
			{
				me->getBodyModule()->setFrontCrushed(crushType == TOTAL_CRUSH || crushType == FRONT_END_CRUSH);
				me->getBodyModule()->setBackCrushed(crushType == TOTAL_CRUSH || crushType == BACK_END_CRUSH);

				ModelConditionFlags newCrushed;
				newCrushed.set(MODELCONDITION_FRONTCRUSHED, (crushType == TOTAL_CRUSH || crushType == FRONT_END_CRUSH));
				newCrushed.set(MODELCONDITION_BACKCRUSHED, crushType == TOTAL_CRUSH || crushType == BACK_END_CRUSH);

				me->getDrawable()->clearAndSetModelConditionFlags(
					MAKE_MODELCONDITION_MASK2(MODELCONDITION_BACKCRUSHED, MODELCONDITION_FRONTCRUSHED),
					newCrushed);
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void CrushDie::crc( Xfer *xfer )
{

	// extend base class
	DieModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void CrushDie::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DieModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void CrushDie::loadPostProcess( void )
{

	// extend base class
	DieModule::loadPostProcess();

}  // end loadPostProcess
