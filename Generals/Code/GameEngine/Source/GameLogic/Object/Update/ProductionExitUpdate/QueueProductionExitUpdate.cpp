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

// FILE: QueueProductionExitUpdate.cpp /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, January, 2002
// Desc:		Hand off produced Units to me so I can Exit them into the world with my specific style
//					This instance refuses to spit a second out until the first is clear
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "Lib/BaseType.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/QueueProductionExitUpdate.h"

//-------------------------------------------------------------------------------------------------
QueueProductionExitUpdate::QueueProductionExitUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_currentDelay = 0;

	//Added By Sadullah Nader
	//Initializations inserted
	m_creationClearDistance = 0;
	m_rallyPoint.zero();
	//

	// no rally point has been set
	m_rallyPointExists = false;
	m_currentBurstCount = 0;
	const QueueProductionExitUpdateModuleData* md = getQueueProductionExitUpdateModuleData();
	if (md) //sanity
	{
		m_currentBurstCount = md->m_initialBurst;
	}
}

//-------------------------------------------------------------------------------------------------
QueueProductionExitUpdate::~QueueProductionExitUpdate()
{
}

//-------------------------------------------------------------------------------------------------
void QueueProductionExitUpdate::exitObjectViaDoor( Object *newObj, ExitDoorType exitDoor )
{
	DEBUG_ASSERTCRASH(exitDoor == DOOR_1, ("multiple exit doors not supported here"));

	Object *creationObject = getObject();
	if (creationObject)
	{
		const QueueProductionExitUpdateModuleData* md = getQueueProductionExitUpdateModuleData();

		Real exitAngle = creationObject->getOrientation();
		const Matrix3D *transform = creationObject->getTransformMatrix();
		Vector3 loc;
		Coord3D createPoint;

		//
		// calculate the position to create the object at, we take the coord specified
		// in INI which is in model space, rotate it to match the building angle
		// and translate for building location via a transform call
		//
		loc.Set( md->m_unitCreatePoint.x, md->m_unitCreatePoint.y, md->m_unitCreatePoint.z );
		transform->Transform_Vector( *transform, loc, &loc );

		Bool creationInAir = FALSE;
		if( TheTerrainLogic )
		{
			if( loc.Z != TheTerrainLogic->getGroundHeight( loc.X, loc.Y ) )
				creationInAir = TRUE;
		}
		// make sure the point is on the terrain
		if( creationInAir && !md->m_allowAirborneCreationData )
			loc.Z = TheTerrainLogic ? TheTerrainLogic->getGroundHeight( loc.X, loc.Y ) : 0.0f;

		// we need it in Coord3D form
		createPoint.x = loc.X;
		createPoint.y = loc.Y;
		createPoint.z = loc.Z;

		newObj->setPosition( &createPoint );
		newObj->setOrientation( exitAngle );

		//
		// Objects that are created in the air from producers that are in the air get
		// a kick that will make their starting speed equal mine.
		//
		PhysicsBehavior *newObjectPhysics = newObj->getPhysics();
		PhysicsBehavior *myPhysics = creationObject->getPhysics();

		if( (myPhysics != NULL) && creationInAir && (newObjectPhysics != NULL) )
		{
			Coord3D startingForce = *myPhysics->getVelocity();
			startingForce.x *= newObjectPhysics->getMass();
			startingForce.y *= newObjectPhysics->getMass();
			startingForce.z *= newObjectPhysics->getMass();
			newObjectPhysics->applyMotiveForce( &startingForce );

			const Real STARTING_PITCH_COEFF = 0.04f;
			Real pitchRate = newObjectPhysics->getCenterOfMassOffset() * STARTING_PITCH_COEFF;
			newObjectPhysics->setPitchRate( pitchRate );
		}


		/** @todo This really should be automatically wrapped up in an actication sequence
		for objects in general */
		// tell the AI about it
		TheAI->pathfinder()->addObjectToPathfindMap( newObj );
		Coord3D tmp;
		getNaturalRallyPoint(tmp);
		// Grid it.
		TheAI->pathfinder()->snapPosition(newObj, &tmp);
		std::vector<Coord3D> exitPath;
		exitPath.push_back(tmp);

		AIUpdateInterface  *ai = newObj->getAIUpdateInterface();
		if (m_rallyPointExists)
		{
			tmp = m_rallyPoint;
			if (ai && ai->isDoingGroundMovement())
			{
				if (TheAI->pathfinder()->adjustDestination(newObj, ai->getLocomotorSet(), &tmp))
					exitPath.push_back(tmp);

			}
		} else {
			// Double the destination to keep redguards from stacking.
			exitPath.push_back(tmp);
		}
		if (ai) {
			ai->aiFollowExitProductionPath( &exitPath, creationObject, CMD_FROM_AI );
		}
		m_currentDelay = md->m_exitDelayData;

		if (m_currentBurstCount)
			m_currentBurstCount--; // fewer and fewer units to burst

	}

}


Bool QueueProductionExitUpdate::getExitPosition( Coord3D& exitPosition ) const
{
	const Object *obj = getObject();
	if (!obj)
		return FALSE;

	const Matrix3D *transform = obj->getTransformMatrix();

	const QueueProductionExitUpdateModuleData *md = getQueueProductionExitUpdateModuleData();

	Vector3 loc;
	loc.Set( md->m_unitCreatePoint.x, md->m_unitCreatePoint.y, md->m_unitCreatePoint.z );
	transform->Transform_Vector( *transform, loc, &loc );

	exitPosition.x = loc.X;
	exitPosition.y = loc.Y;
	exitPosition.z = loc.Z;

	return TRUE;

}


//-------------------------------------------------------------------------------------------------
ExitDoorType QueueProductionExitUpdate::reserveDoorForExit( const ThingTemplate* objType, Object *specificObject )
{
	return isFreeToExit() ? DOOR_1 : DOOR_NONE_AVAILABLE;
}

//-------------------------------------------------------------------------------------------------
void QueueProductionExitUpdate::unreserveDoorForExit( ExitDoorType exitDoor )
{
	/* nothing */
}

//-------------------------------------------------------------------------------------------------
Bool QueueProductionExitUpdate::isFreeToExit() const
{
	Bool stillBursting = (m_currentBurstCount > 0);
	Bool stillDelaying = (m_currentDelay != 0);

	if ( stillBursting )
	{
		return TRUE;
	}

	return ( ! stillDelaying );
}

//-------------------------------------------------------------------------------------------------
UpdateSleepTime QueueProductionExitUpdate::update()
{
	// This update needs to decide when we are again free to exit something
	if (isFreeToExit())
	{
		m_currentDelay = 0;
		return UPDATE_SLEEP_NONE;
	}

	m_currentDelay--;
	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
void QueueProductionExitUpdate::exitObjectByBudding( Object *newObj, Object *budHost )
{
	if (budHost) // object has provided a host for us
	{
		const Coord3D *newPos = budHost->getPosition();
		Real     newAngle = budHost->getOrientation();
		PathfindLayerEnum newLayer = budHost->getLayer();

		newObj->setPosition( newPos );
		newObj->setOrientation( newAngle );
		newObj->setLayer(newLayer);

	}
	else // we need to just copy the host object
	{
		newObj->setPosition( getObject()->getPosition() );
		newObj->setOrientation( getObject()->getOrientation() );
	}

	//This causes the formation to break like billard balls
	AIUpdateInterface *ai = newObj->getAI();
	if (ai)
	{
		ai->aiMoveToPosition( newObj->getPosition(), CMD_FROM_AI );// IOW, I cant stay here, if somebody else is here
	}

	const QueueProductionExitUpdateModuleData* data = getQueueProductionExitUpdateModuleData();
	m_currentDelay = data->m_exitDelayData;

	if (m_currentBurstCount)
		m_currentBurstCount--; // fewer and fewer units to burst

}





Bool QueueProductionExitUpdate::getNaturalRallyPoint( Coord3D& rallyPoint, Bool offset )  const
{
	const QueueProductionExitUpdateModuleData *data = getQueueProductionExitUpdateModuleData();
	Vector3 p;

	//
	// get the natural rally point from the INI definition, this coord is in model space relative
	// to the model (0,0,0)
	//
	p.X = data->m_naturalRallyPoint.x;
	p.Y = data->m_naturalRallyPoint.y;
	p.Z = data->m_naturalRallyPoint.z;

	if ( offset )
	{
		Vector3 offset = p;
		offset.Normalize();
		offset *= (2*PATHFIND_CELL_SIZE_F);
		p+=offset;
	}

	// transform the point into world space
	const Matrix3D *transform = getObject()->getTransformMatrix();
	transform->Transform_Vector( *transform, p, &p );
	rallyPoint.x = p.X; rallyPoint.y = p.Y; rallyPoint.z = p.Z;
	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void QueueProductionExitUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void QueueProductionExitUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// current delay
	xfer->xferUnsignedInt( &m_currentDelay );

	// rally point
	xfer->xferCoord3D( &m_rallyPoint );

	// rally point exists
	xfer->xferBool( &m_rallyPointExists );

	// creation clear distance
	xfer->xferReal( &m_creationClearDistance );

	// current burst count
	xfer->xferUnsignedInt( &m_currentBurstCount );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void QueueProductionExitUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
