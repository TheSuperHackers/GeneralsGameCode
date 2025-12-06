#pragma once

#include "Common/STLTypedefs.h"
#include "Common/GameMemory.h"
#include "GameLogic/AIStateMachine.h"
#include "GameLogic/Module/AIUpdate.h"


//-------------------------------------------------------------------------------------------------
class CarrierDroneAIUpdate : public AIUpdateInterface
{

		MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(CarrierDroneAIUpdate, "CarrierDroneAIUpdate")
		MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(CarrierDroneAIUpdate, AIUpdateModuleData)

		//virtual UpdateSleepTime update();

public:

	CarrierDroneAIUpdate(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	//stop contained drones from attacking
	virtual void privateAttackPosition(const Coord3D* pos, Int maxShotsToFire, CommandSourceType cmdSource) override;						///< attack given spot
};
