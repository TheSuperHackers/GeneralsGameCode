// FILE: ResetSpecialPowerTimerWhileAliveUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: pWn3d, 2025
// Desc:   Update module to reset a player global special power timer while this object is alive
//         Example use: airdrop special power that starts with the cooldown when the dropped unit is dead, add this module to the 
//						dropped unit
///////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#ifndef __RESET_SPECIAL_POWER_TIMER_WHILE_ALIVE_UPDATE_H_
#define __RESET_SPECIAL_POWER_TIMER_WHILE_ALIVE_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/KindOf.h"
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class ThingTemplate;
class SpecialPowerTemplate;


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class ResetSpecialPowerTimerWhileAliveUpdateModuleData : public ModuleData
{
public:
	SpecialPowerTemplate* m_specialPowerTemplate;

	ResetSpecialPowerTimerWhileAliveUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class ResetSpecialPowerTimerWhileAliveUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ResetSpecialPowerTimerWhileAliveUpdate, "ResetSpecialPowerTimerWhileAliveUpdate")
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(ResetSpecialPowerTimerWhileAliveUpdate, ResetSpecialPowerTimerWhileAliveUpdateModuleData);

public:

	ResetSpecialPowerTimerWhileAliveUpdate(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	virtual void onObjectCreated();
	virtual UpdateSleepTime update();

protected:
	// no members needed
};


#endif