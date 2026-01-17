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

// FILE: SupplyCenterProductionExitUpdate.cpp /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, June, 2002
// Desc:		Hand off produced Units to me so I can Exit them into the world with my specific style
//					This instance kicks things it outputs into SupplyTruck autopilot after exiting.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "Lib/BaseType.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/SupplyCenterProductionExitUpdate.h"
#include "GameLogic/Module/SupplyTruckAIUpdate.h"
#include "GameLogic/Object.h"
//#include "GameLogic/PartitionManager.h"

//-------------------------------------------------------------------------------------------------
SupplyCenterProductionExitUpdate::SupplyCenterProductionExitUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	// no rally point has been set
	m_rallyPointExists = false;
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
SupplyCenterProductionExitUpdate::~SupplyCenterProductionExitUpdate()
{
}

//-------------------------------------------------------------------------------------------------
void SupplyCenterProductionExitUpdate::exitObjectViaDoor( Object *newObj, ExitDoorType exitDoor )
{
	DEBUG_ASSERTCRASH(exitDoor == DOOR_1, ("multiple exit doors not supported here"));

	Object *creationObject = getObject();
	if (creationObject)
	{
		const SupplyCenterProductionExitUpdateModuleData* md = getSupplyCenterProductionExitUpdateModuleData();

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

		// make sure the point is on the terrain
		loc.Z = TheTerrainLogic ? TheTerrainLogic->getGroundHeight( loc.X, loc.Y ) : 0.0f;

		// we need it in Coord3D form
		createPoint.x = loc.X;
		createPoint.y = loc.Y;
		createPoint.z = loc.Z;

		newObj->setPosition( &createPoint );
		newObj->setOrientation( exitAngle );

		/** @todo This really should be automatically wrapped up in an activation sequence
		for objects in general */
		// tell the AI about it
		TheAI->pathfinder()->addObjectToPathfindMap( newObj );

		Vector3 p;

		//
		// get the natural rally point from the INI definition, this coord is in model space relative
		// to the model (0,0,0)
		//
		p.X = md->m_naturalRallyPoint.x;
		p.Y = md->m_naturalRallyPoint.y;
		p.Z = md->m_naturalRallyPoint.z;

		// transform the point into world space
		transform->Transform_Vector( *transform, p, &p );

		std::vector<Coord3D> exitPath;

		Coord3D tmp; tmp.x = p.X; tmp.y = p.Y; tmp.z = p.Z;
		exitPath.push_back(tmp);

		if (m_rallyPointExists)
		{
			exitPath.push_back(m_rallyPoint);
		}

		AIUpdateInterface  *ai = newObj->getAIUpdateInterface();
		if( ai )
		{
			ai->aiFollowExitProductionPath( &exitPath, creationObject, CMD_FROM_AI );

			// Here is the special bit for this exit style, force wanting on SupplyTruck types
			SupplyTruckAIInterface* supplyTruckAI = ai->getSupplyTruckAIInterface();

			if( supplyTruckAI )
				supplyTruckAI->setForceWantingState(true);
		}

	}

}


Bool SupplyCenterProductionExitUpdate::getExitPosition( Coord3D& exitPosition ) const
{
	const Object *obj = getObject();
	if (!obj)
		return FALSE;

	const Matrix3D *transform = obj->getTransformMatrix();

	const SupplyCenterProductionExitUpdateModuleData *md = getSupplyCenterProductionExitUpdateModuleData();

	Vector3 loc;
	loc.Set( md->m_unitCreatePoint.x, md->m_unitCreatePoint.y, md->m_unitCreatePoint.z );
	transform->Transform_Vector( *transform, loc, &loc );

	exitPosition.x = loc.X;
	exitPosition.y = loc.Y;
	exitPosition.z = loc.Z;

	return TRUE;

}


//-------------------------------------------------------------------------------------------------
Bool SupplyCenterProductionExitUpdate::getNaturalRallyPoint( Coord3D& rallyPoint, Bool offset ) const
{
	const SupplyCenterProductionExitUpdateModuleData *data = getSupplyCenterProductionExitUpdateModuleData();
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
void SupplyCenterProductionExitUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SupplyCenterProductionExitUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// rally point
	xfer->xferCoord3D( &m_rallyPoint );

	// rally point exists
	xfer->xferBool( &m_rallyPointExists );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SupplyCenterProductionExitUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
