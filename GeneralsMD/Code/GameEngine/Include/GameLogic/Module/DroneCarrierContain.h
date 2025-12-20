// FILE: DroneCarrierContain.h ////////////////////////////////////////////////////////////////////////
// Desc:   expanded transport contain to work with drone carrier
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DRONE_CARRIER_CONTAIN_H_
#define __DRONE_CARRIER_CONTAIN_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/OpenContain.h"
#include "GameLogic/Module/TransportContain.h"
#include "GameLogic/Module/GarrisonContain.h"


enum DeathType CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
class DroneCarrierContainModuleData: public TransportContainModuleData
{
public:
	Real							m_launchVelocityBoost;
	DeathType					m_deathTypeToContained;

	DroneCarrierContainModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

};

//-------------------------------------------------------------------------------------------------
class DroneCarrierContain: public TransportContain
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(DroneCarrierContain, "DroneCarrierContain")
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(DroneCarrierContain, DroneCarrierContainModuleData)

public:

	DroneCarrierContain(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	//Only allow slaved units in
	virtual Bool isValidContainerFor(const Object* obj, Bool checkCapacity) const;

	virtual Bool isEnclosingContainerFor(const Object* obj) const { return true; } //TODO param in module

	virtual Bool isPassengerAllowedToFire(ObjectID id = INVALID_ID) const override;	///< Hey, can I shoot out of this container?

	//support for specific exit bones
	virtual void onRemoving(Object* obj) override;
	virtual void onContaining(Object* obj, Bool wasSelected) override;		///< object now contains 'obj'

	virtual short getRiderSlot(ObjectID riderID) const override;
	virtual short getPortableSlot(ObjectID portableID) const override;
	virtual const ContainedItemsList* getAddOnList() const override;
	virtual ContainedItemsList* getAddOnList() override;

	virtual void onDie(const DamageInfo* damageInfo) override;

	// Called from the AI update to reload the contained drones
	void updateContainedReloadingStatus();

protected:

	// Saves slot assignement and frame when entered
	std::vector<std::tuple<ObjectID, UnsignedInt>> m_contained_units;
};

#endif // __TransportContain_H_
