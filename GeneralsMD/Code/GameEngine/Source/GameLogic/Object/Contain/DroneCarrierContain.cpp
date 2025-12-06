// FILE: DroneCarrierContain.cpp //////////////////////////////////////////////////////////////////////
// Desc:   Contain module for drone carrier
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/Module/TransportContain.h"
#include "GameLogic/Module/DroneCarrierContain.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/WeaponSetType.h"


DroneCarrierContainModuleData::DroneCarrierContainModuleData() : TransportContainModuleData()
{
	m_launchVelocityBoost = 0.0f;
}

void DroneCarrierContainModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	TransportContainModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "LaunchVelocityBoost", INI::parseReal, NULL, offsetof(DroneCarrierContainModuleData, m_launchVelocityBoost) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DroneCarrierContain::DroneCarrierContain(Thing* thing, const ModuleData* moduleData) :
	TransportContain(thing, moduleData)
{
	const TransportContainModuleData* data = dynamic_cast<const TransportContainModuleData*>(moduleData);
	if (data != nullptr) {
		m_contained_units.reserve(data->m_slotCapacity);
		for (size_t i = 0; i < data->m_slotCapacity; i++) {
			m_contained_units.emplace_back(INVALID_ID, 0);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DroneCarrierContain::~DroneCarrierContain(void)
{
}

Bool DroneCarrierContain::isValidContainerFor(const Object* rider, Bool checkCapacity) const
{
	// sanity
	if (!rider)
		return false;

	// no... actually, only OUR OWN units can be transported.
	if (rider->getControllingPlayer() != getObject()->getControllingPlayer())
		return false;

	Int transportSlotCount = rider->getTransportSlotCount();

	// if 0, this object isn't transportable.
	if (transportSlotCount == 0)
		return false;

	if (checkCapacity)
	{
		Int containMax = getContainMax();
		Int containCount = getContainCount();

		/*if (!(m_extraSlotsInUse + containCount + transportSlotCount <= containMax)) {
			return false;
		}*/
		if (containCount >= containMax) {
			return false;
		}
	}

	// Check if this is actually a slaved unit
	for (BehaviorModule** update = rider->getBehaviorModules(); *update; ++update)
	{
		SlavedUpdateInterface* sdu = (*update)->getSlavedUpdateInterface();
		if (sdu != nullptr)
		{
			return sdu->getSlaverID() == getObject()->getID();
		}
	}

	return false;
}

Bool DroneCarrierContain::isPassengerAllowedToFire(ObjectID id) const
{
	return false;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void DroneCarrierContain::onRemoving(Object* rider)
{
	//Intentionally not use TransportContain::onRemoving
	OpenContain::onRemoving(rider);

	// object is no longer held inside a transport
	rider->clearDisabled(DISABLED_HELD);

	const DroneCarrierContainModuleData* d = getDroneCarrierContainModuleData();

	// give the object back a regular weapon
	rider->clearWeaponBonusCondition(WEAPONBONUSCONDITION_CONTAINED);
	rider->clearWeaponSetFlag(WEAPONSET_CONTAINED);

	Drawable* rider_draw = rider->getDrawable();
	if (rider_draw != nullptr) {
		rider_draw->enableAmbientSound(true);
	}

	// If we have a bone and no exit paths, put each rider at the numbered bone position
	if (!d->m_exitBone.isEmpty() && d->m_numberOfExitPaths <= 0)
	{
		Drawable* draw = getObject()->getDrawable();
		if (draw)
		{
			Coord3D bonePos;
			Matrix3D boneMat;
			AsciiString boneName;

			//Int slot = static_cast<Int>(getContainCount());
			Int slot = getRiderSlot(rider->getID());
			if (slot >= 0) {
				boneName.format("%s%02d", d->m_exitBone.str(), slot + 1);

				Int foundBones = draw->getPristineBonePositions(boneName.str(), 0, &bonePos, &boneMat, 1);

				if (foundBones > 0) {
					Coord3D worldPos;
					Matrix3D worldMat;
					getObject()->convertBonePosToWorldPos(&bonePos, &boneMat, &worldPos, &worldMat);
					rider->setPosition(&worldPos);
					rider->setTransformMatrix(&worldMat);
				}
			}
		}
	}

	if (d->m_orientLikeContainerOnExit)
	{
		rider->setOrientation(getObject()->getOrientation());
	}

	if (d->m_keepContainerVelocityOnExit)
	{
		PhysicsBehavior* parent = getObject()->getPhysics();
		PhysicsBehavior* child = rider->getPhysics();
		if (parent && child)
		{
			Coord3D startingForce = *parent->getVelocity();
			Real mass = child->getMass();
			startingForce.x *= mass;
			startingForce.y *= mass;
			startingForce.z *= mass;
			child->applyMotiveForce(&startingForce);

			Real pitchRate = child->getCenterOfMassOffset() * d->m_exitPitchRate;
			child->setPitchRate(pitchRate);
		}
	}

	if (d->m_launchVelocityBoost > 0.0f) {
		PhysicsBehavior* phys = rider->getPhysics();
		if (phys != nullptr) {
			Coord3D dir;
			rider->getUnitDirectionVector3D(dir);
			Real mass = phys->getMass() * d->m_launchVelocityBoost;
			dir.x *= mass;
			dir.y *= mass;
			dir.z *= mass;
			phys->applyMotiveForce(&dir);
		}
	}

	Int transportSlotCount = rider->getTransportSlotCount();
	DEBUG_ASSERTCRASH(transportSlotCount > 0, ("Hmm, this object isnt transportable"));
	m_extraSlotsInUse -= transportSlotCount - 1;
	DEBUG_ASSERTCRASH(m_extraSlotsInUse >= 0 && m_extraSlotsInUse + getContainCount() <= getContainMax(), ("Hmm, bad slot count"));

	// when we are empty again, clear the model condition for loaded
	if (getContainCount() == 0)
	{
		Drawable* draw = getObject()->getDrawable();

		if (draw)
			draw->clearModelConditionState(MODELCONDITION_LOADED);

	}  // end if

	if (getObject()->isAboveTerrain())
	{
		// temporarily mark the guy as being allowed to fall
		// (overriding his locomotor's stick-to-ground attribute).
		// this will be reset (by PhysicsBehavior) when he touches the ground.
		PhysicsBehavior* physics = rider->getPhysics();
		if (physics)
			physics->setAllowToFall(true);
	}

	// AI might need help using this transport in a good way.  Make the passengers aggressive.
	//There is no computer player check since Aggressive only means something for computer players anyway
	if (d->m_goAggressiveOnExit && rider->getAI())
	{
		rider->getAI()->setAttitude(ATTITUDE_AGGRESSIVE);
	}
	if (getObject()->isEffectivelyDead()) {
		scatterToNearbyPosition(rider);
	}
	if (d->m_resetMoodCheckTimeOnExit && rider->getAI())
	{
		rider->getAI()->wakeUpAndAttemptToTarget();
	}

	// Remove unit from slot assign vector
	for (size_t i = 0; i < m_contained_units.size(); i++) {
		if (std::get<0>(m_contained_units[i]) == rider->getID()){
			m_contained_units[i] = { INVALID_ID, 0 };
		}
	}



	m_frameExitNotBusy = TheGameLogic->getFrame() + d->m_exitDelay;
}

void DroneCarrierContain::onContaining(Object* obj, Bool wasSelected)
{
	TransportContain::onContaining(obj, wasSelected);

	Drawable* draw = obj->getDrawable();
	if (draw != nullptr) {
		draw->enableAmbientSound(false);
	}

	// Assing in first free slot
	for (size_t i = 0; i < m_contained_units.size(); i++) {
		if (std::get<0>(m_contained_units[i]) == INVALID_ID) {
			m_contained_units[i] = { obj->getID(), TheGameLogic->getFrame() };
			break;
		}
	}
}

short DroneCarrierContain::getRiderSlot(ObjectID riderID) const
{
	for (size_t i = 0; i < m_contained_units.size(); i++) {
		if (std::get<0>(m_contained_units[i]) == riderID) {
			return i;
		}
	}
	return -1;
}

short DroneCarrierContain::getPortableSlot(ObjectID portableID) const
{
	return getRiderSlot(portableID);
}

const ContainedItemsList* DroneCarrierContain::getAddOnList() const
{
	return &m_containList;
}

ContainedItemsList* DroneCarrierContain::getAddOnList()
{
	return &m_containList;
}

// Similar to jet ai when parking
void DroneCarrierContain::updateContainedReloadingStatus()
{
	UnsignedInt now = TheGameLogic->getFrame();

	for (size_t i = 0; i < m_contained_units.size(); i++) {
		const auto& [objectID, enteredFrame] = m_contained_units[i];
		if (objectID != INVALID_ID) {

			Object* drone = TheGameLogic->findObjectByID(objectID);
			if (drone != nullptr) {
				for (Int i = 0; i < WEAPONSLOT_COUNT; ++i)
				{
					Weapon* w = drone->getWeaponInWeaponSlot((WeaponSlotType)i);
					if (w == NULL)
						continue;

					Int reloadTime = w->getClipReloadTime(drone);
					UnsignedInt reloadDone = enteredFrame + reloadTime;

					if (now >= reloadDone)
						w->setClipPercentFull(1.0f, false);
					else
						w->setClipPercentFull((Real)(reloadTime - (reloadDone - now)) / reloadTime, false);
				}
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void DroneCarrierContain::crc(Xfer* xfer)
{

	// extend base class
	TransportContain::crc(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void DroneCarrierContain::xfer(Xfer* xfer)
{
	// extend base class
	TransportContain::xfer(xfer);

	//xfer the contain vector
	// xfer the count of the vector
	UnsignedShort listCount = m_contained_units.size();
	xfer->xferUnsignedShort(&listCount);

	// xfer vector data
	std::tuple<ObjectID, UnsignedInt> entry;
	if (xfer->getXferMode() == XFER_SAVE || xfer->getXferMode() == XFER_CRC)
	{
		// save all tuples
		std::vector< std::tuple<ObjectID, UnsignedInt> >::const_iterator it;
		for (it = m_contained_units.begin(); it != m_contained_units.end(); ++it)
		{

			entry = *it;
			xfer->xferObjectID(&std::get<0>(entry));
			xfer->xferUnsignedInt(&std::get<1>(entry));

		}  // end for

	}  // end if, save
	else if (xfer->getXferMode() == XFER_LOAD)
	{
		m_contained_units.clear();
		m_contained_units.reserve(listCount);

		// sanity, the list should be empty before we transfer more data into it
		if (m_contained_units.size() != 0)
		{

			DEBUG_CRASH(("DroneCarrierContain::xfer - object vector should be empty before loading"));
			throw XFER_LIST_NOT_EMPTY;

		}  // end if

		// read all ids
		for (UnsignedShort i = 0; i < listCount; ++i)
		{

			xfer->xferObjectID(&std::get<0>(entry));
			xfer->xferUnsignedInt(&std::get<1>(entry));
			m_contained_units.push_back(entry);

		}  // end for, i

	}  // end else if
	else
	{

		DEBUG_CRASH(("DroneCarrierContain::xfer - Unknown xfer mode '%d'", xfer->getXferMode()));
		throw XFER_MODE_UNKNOWN;

	}  // end else

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void DroneCarrierContain::loadPostProcess(void)
{

	// extend base class
	TransportContain::loadPostProcess();

}  // end loadPostProcess
