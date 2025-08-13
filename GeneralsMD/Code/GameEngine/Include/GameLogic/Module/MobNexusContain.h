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

// FILE: MobNexusContain.h ////////////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen, August 2002
// Desc:   Contain module for mob units. Acts as proxy for UI and AI
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __MOBNEXUSCONTAIN_H_
#define __MOBNEXUSCONTAIN_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/OpenContain.h"

//-------------------------------------------------------------------------------------------------
class MobNexusContainModuleData : public OpenContainModuleData
{
public:
	struct InitialPayload
	{
		AsciiString name;
		Int count;
	};

	Int								m_slotCapacity;								///< max units that can be inside us
	Real							m_exitPitchRate;
	Bool							m_scatterNearbyOnExit;
	Bool							m_orientLikeContainerOnExit;
	Bool							m_keepContainerVelocityOnExit;
	AsciiString				m_exitBone;
	InitialPayload		m_initialPayload;
	Real							m_healthRegen;

	MobNexusContainModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);
	static void parseInitialPayload( INI* ini, void *instance, void *store, const void* /*userData*/ );

};

//-------------------------------------------------------------------------------------------------
class MobNexusContain : public OpenContain,
												public TransportPassengerInterface // lorenzen add a MobMemberInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( MobNexusContain, "MobNexusContain" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( MobNexusContain, MobNexusContainModuleData )

public:

	MobNexusContain( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration
																																								// lorenzen add a MobMemberInterface
																																								// lorenzen add a MobMemberInterface
																																								// lorenzen add a MobMemberInterface
	virtual TransportPassengerInterface* getTransportPassengerInterface() { return this; }// lorenzen add a MobMemberInterface
																																								// lorenzen add a MobMemberInterface
																																								// lorenzen add a MobMemberInterface
																																								// lorenzen add a MobMemberInterface
	virtual Bool isValidContainerFor( const Object* obj, Bool checkCapacity) const;

	virtual void onContaining( Object *obj, Bool wasSelected );		///< object now contains 'obj'
	virtual void onRemoving( Object *obj );			///< object no longer contains 'obj'
	virtual UpdateSleepTime update();							///< called once per frame

	virtual Int getContainMax( void ) const;

	virtual void onObjectCreated();
	virtual Int getExtraSlotsInUse( void ) { return m_extraSlotsInUse; }

	virtual ExitDoorType reserveDoorForExit( const ThingTemplate* objType, Object *specificObject );					///< All types can answer if they are free to exit or not, and you can ask about a specific guy or just exit anything in general
	virtual void unreserveDoorForExit( ExitDoorType exitDoor );

	virtual Bool tryToEvacuate( Bool exposeStealthedUnits ); ///< Will try to kick everybody out with game checks, and will return whether anyone made it

protected:

private:

	Int m_extraSlotsInUse;

};

#endif // __MOBNEXUSCONTAIN_H_

