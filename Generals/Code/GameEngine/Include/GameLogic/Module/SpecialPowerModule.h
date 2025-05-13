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

// FILE: SpecialPowerModule.h /////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2002
// Desc:	 Special power module interface
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SPECIALPOWERMODULE_H_
#define __SPECIALPOWERMODULE_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/AudioEventRTS.h"
#include "Common/Module.h"
#include "GameLogic/Module/BehaviorModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Object;
class SpecialPowerTemplate;
struct FieldParse;
enum ScienceType CPP_11(: Int);


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpecialPowerModuleInterface
{
public:

	virtual Bool isModuleForPower( const SpecialPowerTemplate *specialPowerTemplate ) const = 0;
	virtual Bool isReady( void ) const = 0;
//  This is the althernate way to one-at-a-time BlackLotus' specials; we'll keep it commented her until Dustin decides, or until 12/10/02
//	virtual Bool isBusy( void ) const = 0;
	virtual Real getPercentReady( void ) const = 0;
	virtual UnsignedInt getReadyFrame( void ) const = 0;
	virtual AsciiString getPowerName( void ) const = 0;
	virtual const SpecialPowerTemplate* getSpecialPowerTemplate( void ) const = 0;
	virtual ScienceType getRequiredScience( void ) const = 0;
	virtual void onSpecialPowerCreation( void ) = 0;
	virtual void setReadyFrame( UnsignedInt frame ) = 0;
	virtual void pauseCountdown( Bool pause ) = 0;
	virtual void doSpecialPower( UnsignedInt commandOptions ) = 0;
	virtual void doSpecialPowerAtObject( Object *obj, UnsignedInt commandOptions ) = 0;
	virtual void doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions ) = 0;
	virtual void doSpecialPowerUsingWaypoints( const Waypoint *way, UnsignedInt commandOptions ) = 0;
	virtual void markSpecialPowerTriggered( const Coord3D *location ) = 0;
	virtual void startPowerRecharge() = 0;	
	virtual const AudioEventRTS& getInitiateSound() const = 0;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpecialPowerModuleData : public BehaviorModuleData
{
public:

	SpecialPowerModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

	const SpecialPowerTemplate *m_specialPowerTemplate;		///< pointer to the special power template
	Bool	m_updateModuleStartsAttack;	///< update module determines when the special power actually starts! If true, update module is required.
	Bool m_startsPaused; ///< Paused on creation, someone else will have to unpause (like upgrade module, or script)
	AudioEventRTS					m_initiateSound;
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpecialPowerModule : public BehaviorModule, 
													 public SpecialPowerModuleInterface
{

	MEMORY_POOL_GLUE_ABC( SpecialPowerModule )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpecialPowerModule, SpecialPowerModuleData )

public:

	SpecialPowerModule( Thing* thing, const ModuleData* moduleData );

	static Int getInterfaceMask() { return MODULEINTERFACE_SPECIAL_POWER; }

	// BehaviorModule
	virtual SpecialPowerModuleInterface* getSpecialPower() { return this; }

	Bool isModuleForPower( const SpecialPowerTemplate *specialPowerTemplate ) const;	///< is this module for the specified special power
	Bool isReady( void ) const; 						///< is this special power available now
//  This is the althernate way to one-at-a-time BlackLotus' specials; we'll keep it commented her until Dustin decides, or until 12/10/02
//	Bool isBusy( void ) const { return FALSE; } 

	Real getPercentReady( void ) const;		///< get the percent ready (1.0 = ready now, 0.5 = half charged up etc.)

	UnsignedInt getReadyFrame( void ) const;		///< get the frame at which we are ready
	AsciiString getPowerName( void ) const;
	void syncReadyFrameToStatusQuo( void );

	const SpecialPowerTemplate* getSpecialPowerTemplate( void ) const;
	ScienceType getRequiredScience( void ) const;

	void onSpecialPowerCreation( void );	// called by a create module to start our countdown
	//
	// The following methods are for use by the scripting engine ONLY
	//

	void setReadyFrame( UnsignedInt frame ) { m_availableOnFrame = frame; }
	UnsignedInt getReadyFrame( void ) { return m_availableOnFrame; }// USED BY PLAYER TO KEEP RECHARGE TIMERS IN SYNC
	void pauseCountdown( Bool pause );

	//
	// the following methods should be *EXTENDED* for any special power module implementations
	// and carry out the special power executions
	//
	virtual void doSpecialPower( UnsignedInt commandOptions );
	virtual void doSpecialPowerAtObject( Object *obj, UnsignedInt commandOptions );
	virtual void doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions );
	virtual void doSpecialPowerUsingWaypoints( const Waypoint *way, UnsignedInt commandOptions );

	/**
	 Now, there are special powers that require some preliminary processing before the actual
	 special power triggers. When the ini setting "UpdateModuleStartsAttack" is true, then 
	 the update module will call the doSpecialPower a second time. This function then resets 
	 the power recharge, and tells the scriptengine that the attack has started.
	 
	 A good example of something that uses this is the Black Lotus - capture building hack attack. 
	 When the user initiates the attack, the doSpecialPower is called, which triggers the update 
	 module. The update module then orders the unit to move within range, and it isn't until the 
	 hacker start the physical attack, that the timer is reset and the attack technically begins.
	*/
	virtual void markSpecialPowerTriggered( const Coord3D *location );

	/** start the recharge process for this special power. public because some powers call it repeatedly.
	*/
	virtual void startPowerRecharge();
	virtual const AudioEventRTS& getInitiateSound() const;

protected:

	Bool initiateIntentToDoSpecialPower( const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions );
	void triggerSpecialPower( const Coord3D *location );
	void createViewObject( const Coord3D *location );
	void resolveSpecialPower( void );
	void aboutToDoSpecialPower( const Coord3D *location );

	UnsignedInt m_availableOnFrame;			///< on this frame, this special power is available
	Int m_pausedCount;									///< Reference count of sources pausing me
	UnsignedInt m_pausedOnFrame;
	Real m_pausedPercent;

};

#endif  // end __SPECIALPOWERMODULE_H_

