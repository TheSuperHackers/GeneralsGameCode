
// FILE: DroneCarrierSlavedUpdate.cpp /////////////////////////////////////////////////////////////////////////
// Desc:  expanded Slaved update to work with drone carrier
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _DRONE_CARRIER_SLAVED_UPDATE_H_
#define _DRONE_CARRIER_SLAVED_UPDATE_H_
#include "Common/INI.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/SlavedUpdate.h"

//-------------------------------------------------------------------------------------------------
class DroneCarrierSlavedUpdateModuleData : public SlavedUpdateModuleData
{
public:
	Real m_leashRange;

	DroneCarrierSlavedUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

class DroneCarrierSlavedUpdate : public SlavedUpdate
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(DroneCarrierSlavedUpdate, "DroneCarrierSlavedUpdate")
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(DroneCarrierSlavedUpdate, DroneCarrierSlavedUpdateModuleData)

public:

	DroneCarrierSlavedUpdate(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();	///< Deciding whether or not to make new guys

	virtual SlavedUpdateInterface* getSlavedUpdateInterface() { return this; }

};

#endif
