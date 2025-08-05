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

// FILE: ResourceGatheringManager.cpp ///////////////////////////////////////////////////////////
// The part of a Player's brain that keeps track of all Resource type Objects and makes
// gathering type decisions based on them.
// Author: Graham Smallwood, January, 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ResourceGatheringManager.h"

#include "Common/ActionManager.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/SupplyTruckAIUpdate.h"
#include "GameLogic/Module/SupplyCenterDockUpdate.h"
#include "GameLogic/Module/SupplyWarehouseDockUpdate.h"
#include "GameLogic/Module/UpdateModule.h"

ResourceGatheringManager::ResourceGatheringManager()
{
}

ResourceGatheringManager::~ResourceGatheringManager()
{
	m_supplyWarehouses.erase( m_supplyWarehouses.begin(), m_supplyWarehouses.end() );
	m_supplyCenters.erase( m_supplyCenters.begin(), m_supplyCenters.end() );
}

void ResourceGatheringManager::addSupplyCenter( Object *newCenter )
{
	if( newCenter == NULL )
		return;

	m_supplyCenters.push_back( newCenter->getID() );
}

void ResourceGatheringManager::removeSupplyCenter( Object *oldCenter )
{
	if( oldCenter == NULL )
		return;

	ObjectID targetID = oldCenter->getID();

	objectIDListIterator iterator = m_supplyCenters.begin();
	while( iterator != m_supplyCenters.end() )
	{
		if( targetID == *iterator )
		{
			iterator = m_supplyCenters.erase( iterator );
		}
		else
			iterator++;
	}
}

void ResourceGatheringManager::addSupplyWarehouse( Object *newWarehouse )
{
	if( newWarehouse == NULL )
		return;

	m_supplyWarehouses.push_back( newWarehouse->getID() );
}

void ResourceGatheringManager::removeSupplyWarehouse( Object *oldWarehouse )
{
	if( oldWarehouse == NULL )
		return;

	ObjectID targetID = oldWarehouse->getID();

	objectIDListIterator iterator = m_supplyWarehouses.begin();
	while( iterator != m_supplyWarehouses.end() )
	{
		if( targetID == *iterator )
		{
			iterator = m_supplyWarehouses.erase( iterator );
		}
		else
			iterator++;
	}
}

static Real computeRelativeCost( Object *queryObject, Object *destObject, Real *pureDistanceSquared )
{
	/** @todo This gets filled with Pathfinding computations, analysis of Boxes remaining,
			Threat calculations, paths of other trucks, and other fancy stuff.
	*/

	//A good score is a very small number.

	if( queryObject == NULL  ||  destObject == NULL )
		return FLT_MAX;

	if( !TheActionManager->canTransferSuppliesAt(queryObject, destObject) )
		return FLT_MAX;// Handles emptyness and alliances

	DockUpdateInterface *dockInterface = destObject->getDockUpdateInterface();
	if( !dockInterface->isClearToApproach( queryObject ) )
		return FLT_MAX;

	// since we don't care about the distance as a distance per se, but rather as
	// a goodness-factor, save some time by getting the dist-sqr (srj)
	Real distSquared = ThePartitionManager->getDistanceSquared(queryObject, destObject, FROM_CENTER_3D);

	// I need the distance, but I don't want to count on the coincidence that
	// the abstract 'cost' this function returns happens to be just the distance, since it could
	// become more complicated
	if( pureDistanceSquared )
		*pureDistanceSquared = distSquared;

	return distSquared;
}

Object *ResourceGatheringManager::findBestSupplyWarehouse( Object *queryObject )
{
	Object *bestWarehouse = NULL;
	Real maxDistanceSquared = 100000;

	if( ( queryObject == NULL ) || ( queryObject->getAI() == NULL ) )
		return NULL;

	SupplyTruckAIInterface *supplyTruckAI = queryObject->getAI()->getSupplyTruckAIInterface();
	if( supplyTruckAI )
	{
		// Check for a dock override being set.
		ObjectID dockID = supplyTruckAI->getPreferredDockID();
		Object *dock = TheGameLogic->findObjectByID(dockID);
		if( dock )
		{
			static const NameKeyType key_warehouseUpdate = NAMEKEY("SupplyWarehouseDockUpdate");
			SupplyWarehouseDockUpdate *warehouseModule = (SupplyWarehouseDockUpdate*)dock->findUpdateModule( key_warehouseUpdate );
			//If remotely okay, let User win.
			if( warehouseModule && computeRelativeCost( queryObject, dock, NULL ) != FLT_MAX )
				return dock;
		}
		// Please note, there is not a separate Warehouse and Center memory by Design.  Because
		// we lack a UI way to click Warehouse and drag to center to set up a specific path, the
		// practical realization has been made that you do not want separate memory.

		// Design wants a harvester to give up and return to base if it is "too far" to the warehouse.
		// Note, the "PreferedDock" will override this, and there is no distance max on Centers.
		maxDistanceSquared = supplyTruckAI->getWarehouseScanDistance() * supplyTruckAI->getWarehouseScanDistance();
	}

	//Otherwise, search for a good one.
	Real bestCost = FLT_MAX;

	objectIDListIterator iterator = m_supplyWarehouses.begin();
	while( iterator != m_supplyWarehouses.end() )
	{
		ObjectID currentID = *iterator;
		Object *currentWarehouse =TheGameLogic->findObjectByID(currentID);

		if( currentWarehouse == NULL )
		{
			iterator = m_supplyWarehouses.erase( iterator );
		}
		else
		{
			Real distanceSquared;
			Real currentCost = computeRelativeCost( queryObject, currentWarehouse, &distanceSquared );
			if( (currentCost < bestCost) && (distanceSquared < maxDistanceSquared) )
			{
				bestWarehouse = currentWarehouse;
				bestCost = currentCost;
			}

			iterator++;
		}
	}

	return bestWarehouse;
}

Object *ResourceGatheringManager::findBestSupplyCenter( Object *queryObject )
{
	Object *bestCenter = NULL;

	if( ( queryObject == NULL ) || ( queryObject->getAI() == NULL ) )
		return NULL;

	SupplyTruckAIInterface *supplyTruckAI = queryObject->getAI()->getSupplyTruckAIInterface();
	if( supplyTruckAI )
	{
		// Check for a dock override being set.
		ObjectID dockID = supplyTruckAI->getPreferredDockID();
		Object *dock = TheGameLogic->findObjectByID(dockID);
		if( dock )
		{
			static const NameKeyType key_centerUpdate = NAMEKEY("SupplyCenterDockUpdate");
			SupplyCenterDockUpdate *centerModule = (SupplyCenterDockUpdate*)dock->findUpdateModule( key_centerUpdate );
			//If remotely okay, let User win.
			if( centerModule && computeRelativeCost( queryObject, dock, NULL ) != FLT_MAX )
				return dock;
		}
		// Please note, there is not a separate Warehouse and Center memory by Design.  Because
		// we lack a UI way to click Warehouse and drag to center to set up a specific path, the
		// practical realization has been made that you do not want separate memory.
	}

	//Otherwise, search for a good one.
	Real bestCost = FLT_MAX;

	objectIDListIterator iterator = m_supplyCenters.begin();
	while( iterator != m_supplyCenters.end() )
	{
		ObjectID currentID = *iterator;
		Object *currentCenter =TheGameLogic->findObjectByID(currentID);

		if( currentCenter == NULL )
		{
			iterator = m_supplyCenters.erase( iterator );
		}
		else
		{
			Real currentCost = computeRelativeCost( queryObject, currentCenter, NULL );
			if( currentCost < bestCost )
			{
				bestCenter = currentCenter;
				bestCost = currentCost;
			}

			iterator++;
		}
	}

	return bestCenter;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ResourceGatheringManager::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ResourceGatheringManager::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// supply warehouses
	xfer->xferSTLObjectIDList( &m_supplyWarehouses );

	// supply centers
	xfer->xferSTLObjectIDList( &m_supplyCenters );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ResourceGatheringManager::loadPostProcess( void )
{

}  // end loadPostProcess

