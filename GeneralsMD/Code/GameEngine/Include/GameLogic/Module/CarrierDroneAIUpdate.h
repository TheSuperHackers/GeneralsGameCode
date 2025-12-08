#pragma once

#include "Common/STLTypedefs.h"
#include "Common/GameMemory.h"
#include "GameLogic/AIStateMachine.h"
#include "GameLogic/Module/AIUpdate.h"


//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class CarrierDroneAIUpdateModuleData : public AIUpdateModuleData
{
public:
	Real										m_dockingDistance;  //< Distance from carrier to iniate docking
	LocomotorSetType				m_dockingLoco;		//< custom docking loco
	LocomotorSetType				m_launchingLoco;		//< custom launching loco
	UnsignedInt             m_launchTime;     //< launch state duration


	CarrierDroneAIUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class CarrierDroneAIUpdate : public AIUpdateInterface
{

		MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(CarrierDroneAIUpdate, "CarrierDroneAIUpdate")
		MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(CarrierDroneAIUpdate, CarrierDroneAIUpdateModuleData)

		virtual UpdateSleepTime update();

public:

	CarrierDroneAIUpdate(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	//stop contained drones from attacking
	virtual void privateAttackPosition(const Coord3D* pos, Int maxShotsToFire, CommandSourceType cmdSource) override;						///< attack given spot

private:
	bool m_isContained;
	UnsignedInt m_launchFrame;
	bool m_isDocking;
	AudioEventRTS						m_dockingSound;		///< Sound when docking
	AudioEventRTS						m_launchingSound;		///< Sound when launching


};
