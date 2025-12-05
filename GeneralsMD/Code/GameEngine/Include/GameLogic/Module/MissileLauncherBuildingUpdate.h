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

// FILE: MissileLauncherBuildingUpdate.h /////////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, April 2002
// Desc:   Update will change model state conditions based on special power state
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/AudioEventRTS.h"
#include "Common/INI.h"
#include "GameLogic/Module/SpecialPowerUpdateModule.h"
class DamageInfo;
class SpecialPowerTemplate;
class SpecialPowerModule;
class FXList;

//-------------------------------------------------------------------------------------------------
class MissileLauncherBuildingUpdateModuleData : public UpdateModuleData
{
public:
	SpecialPowerTemplate *m_specialPowerTemplate;		///< pointer to the special power template
	UnsignedInt m_doorOpenTime;							///< in frames, time we should take to open the door
	UnsignedInt m_doorWaitOpenTime;					///< in frames, time we should leave the door open after firing the superweapon
	UnsignedInt m_doorClosingTime;					///< in frames, time it takes to close the door

	const FXList *m_openingFX;
	const FXList *m_openFX;
	const FXList *m_waitingToCloseFX;
	const FXList *m_closingFX;
	const FXList *m_closedFX;
	AudioEventRTS m_openIdleAudio;

	MissileLauncherBuildingUpdateModuleData()
	{
		m_specialPowerTemplate = nullptr;
		m_doorOpenTime = 0;
		m_doorWaitOpenTime = 0;
		m_doorClosingTime = 0;

		m_openingFX = m_openFX = m_waitingToCloseFX = m_closingFX = m_closedFX = nullptr;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "SpecialPowerTemplate",	INI::parseSpecialPowerTemplate,					nullptr, offsetof( MissileLauncherBuildingUpdateModuleData, m_specialPowerTemplate ) },
			{ "DoorOpenTime",					INI::parseDurationUnsignedInt,	nullptr, offsetof( MissileLauncherBuildingUpdateModuleData, m_doorOpenTime ) },
			{ "DoorWaitOpenTime",			INI::parseDurationUnsignedInt,	nullptr, offsetof( MissileLauncherBuildingUpdateModuleData, m_doorWaitOpenTime ) },
			{ "DoorCloseTime",				INI::parseDurationUnsignedInt,	nullptr, offsetof( MissileLauncherBuildingUpdateModuleData, m_doorClosingTime ) },
			{ "DoorOpeningFX",				INI::parseFXList,								nullptr, offsetof( MissileLauncherBuildingUpdateModuleData, m_openingFX ) },
			{ "DoorOpenFX",						INI::parseFXList,								nullptr, offsetof( MissileLauncherBuildingUpdateModuleData, m_openFX ) },
			{ "DoorWaitingToCloseFX",	INI::parseFXList,								nullptr, offsetof( MissileLauncherBuildingUpdateModuleData, m_waitingToCloseFX ) },
			{ "DoorClosingFX",				INI::parseFXList,								nullptr, offsetof( MissileLauncherBuildingUpdateModuleData, m_closingFX ) },
			{ "DoorClosedFX",					INI::parseFXList,								nullptr, offsetof( MissileLauncherBuildingUpdateModuleData, m_closedFX ) },
			{ "DoorOpenIdleAudio",		INI::parseAudioEventRTS,				nullptr,	offsetof( MissileLauncherBuildingUpdateModuleData, m_openIdleAudio ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
class MissileLauncherBuildingUpdate : public SpecialPowerUpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( MissileLauncherBuildingUpdate, "MissileLauncherBuildingUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( MissileLauncherBuildingUpdate, MissileLauncherBuildingUpdateModuleData )

public:

	MissileLauncherBuildingUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	//SpecialPowerUpdateInterface pure virtual implementations
	virtual Bool initiateIntentToDoSpecialPower(const SpecialPowerTemplate *specialPowerTemplate, const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions );
	virtual Bool isSpecialAbility() const { return false; }
	virtual Bool isSpecialPower() const { return true; }
	virtual Bool isActive() const { return m_doorState != m_timeoutState; }
	SpecialPowerTemplate* getTemplate() const;
	virtual Bool doesSpecialPowerHaveOverridableDestinationActive() const { return false; } //Is it active now?
	virtual Bool doesSpecialPowerHaveOverridableDestination() const { return false; }	//Does it have it, even if it's not active?
	virtual void setSpecialPowerOverridableDestination( const Coord3D *loc ) {}

	virtual SpecialPowerUpdateInterface* getSpecialPowerUpdateInterface() { return this; }
	virtual CommandOption getCommandOption() const { return (CommandOption)0; }

	virtual UpdateSleepTime update();	///< Deciding whether or not to make new guys
	virtual Bool isPowerCurrentlyInUse( const CommandButton *command = nullptr ) const;

private:
	enum DoorStateType
	{
		DOOR_CLOSED,
		DOOR_OPENING,
		DOOR_OPEN,
		DOOR_WAITING_TO_CLOSE,
		DOOR_CLOSING,
	};

	void switchToState(DoorStateType dst);

	const SpecialPowerModuleInterface*	m_specialPowerModule;
	DoorStateType												m_doorState;
	DoorStateType												m_timeoutState;
	UnsignedInt													m_timeoutFrame;
	AudioEventRTS												m_openIdleAudio;
};
