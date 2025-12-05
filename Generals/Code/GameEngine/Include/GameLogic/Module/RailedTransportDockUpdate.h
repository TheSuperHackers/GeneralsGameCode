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

// FILE: RailedTransportDockUpdate.h //////////////////////////////////////////////////////////////
// Author: Colin Day, August 2002
// Desc:   Railed transport dock update
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/DockUpdate.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class RailedTransportDockUpdateModuleData : public DockUpdateModuleData
{

public:

	RailedTransportDockUpdateModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

	UnsignedInt m_pullInsideDurationInFrames;		/**< how long it takes to pull object inside
																									 once they're at the dock action point */
	UnsignedInt m_pushOutsideDurationInFrames;	/**< how long it takes to push object outside
																									 when we're unloading */

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class RailedTransportDockUpdateInterface
{

public:

	virtual Bool isLoadingOrUnloading( void ) = 0;
	virtual void unloadAll( void ) = 0;
	virtual void unloadSingleObject( Object *obj ) = 0;

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class RailedTransportDockUpdate : public DockUpdate,
																	public RailedTransportDockUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( RailedTransportDockUpdate, "RailedTransportDockUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( RailedTransportDockUpdate, RailedTransportDockUpdateModuleData )

public:

	RailedTransportDockUpdate( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// module interfaces
	virtual RailedTransportDockUpdateInterface *getRailedTransportDockUpdateInterface( void ) { return this; }

	// update module methods
	virtual UpdateSleepTime update( void );

	// dock methods
	virtual DockUpdateInterface* getDockUpdateInterface() { return this; }
	virtual Bool action( Object* docker, Object *drone = nullptr );
	virtual Bool isClearToEnter( Object const* docker ) const;

	// our own methods
	virtual Bool isLoadingOrUnloading( void );
	virtual void unloadAll( void );
	virtual void unloadSingleObject( Object *obj );

protected:

	void doPullInDocking( void );							///< pull docking objects into us
	void doPushOutDocking( void );						///< push unloading objects out of us
	void unloadNext( void );									///< start the "next" object we have inside us coming out

	ObjectID m_dockingObjectID;								///< object docking with us
	Real m_pullInsideDistancePerFrame;				///< when docking, pull object inside this much each frame

	ObjectID m_unloadingObjectID;							///< object that is currently unloading
	Real m_pushOutsideDistancePerFrame;				///< when unloading, push object outside this much frame

	Int m_unloadCount;												///< count used to govern unloading 1 or all objects

};
