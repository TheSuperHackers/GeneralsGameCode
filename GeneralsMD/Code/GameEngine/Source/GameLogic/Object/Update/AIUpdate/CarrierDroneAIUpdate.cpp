// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_LOCOMOTORSET_NAMES

#include "Common/CRCDebug.h"
#include "Common/Player.h"
#include "Common/Team.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameClient/Drawable.h"
//#include "GameClient/ParticleSys.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/PhysicsUpdate.h"
//#include "GameLogic/TerrainLogic.h"
//#include "GameLogic/Weapon.h"

#include "GameLogic/Module/CarrierDroneAIUpdate.h"
// #include "GameLogic/Module/JetAIUpdate.h"
//#include "GameLogic/Module/ProductionUpdate.h"
//#include "GameLogic/Module/ContainModule.h"

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

CarrierDroneAIUpdateModuleData::CarrierDroneAIUpdateModuleData()
{
}

//-------------------------------------------------------------------------------------------------
/*static*/ void CarrierDroneAIUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	AIUpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "DockingDistance",							INI::parseReal, NULL, offsetof(CarrierDroneAIUpdateModuleData, m_dockingDistance) },
		{ "DockingLocomotorType",					INI::parseIndexList, TheLocomotorSetNames, offsetof(CarrierDroneAIUpdateModuleData, m_dockingLoco) },
		//{ "DockingStartSound",					INI::parseIndexList, TheLocomotorSetNames, offsetof(CarrierDroneAIUpdateModuleData, m_dockingLoco) },
		{ "LaunchTime",							INI::parseDurationUnsignedInt, NULL, offsetof(CarrierDroneAIUpdateModuleData, m_launchTime) },
		{ "LaunchingLocomotorType",					INI::parseIndexList, TheLocomotorSetNames, offsetof(CarrierDroneAIUpdateModuleData, m_launchingLoco) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------


CarrierDroneAIUpdate::CarrierDroneAIUpdate(Thing* thing, const ModuleData* moduleData) : AIUpdateInterface(thing, moduleData)
{
	m_dockingSound = *(getObject()->getTemplate()->getPerUnitSound("Docking"));
	m_dockingSound.setObjectID(getObject()->getID());

	m_launchingSound = *(getObject()->getTemplate()->getPerUnitSound("Launching"));
	m_launchingSound.setObjectID(getObject()->getID());

	m_isContained = getObject()->isContained();
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

void CarrierDroneAIUpdate::privateAttackPosition(const Coord3D* pos, Int maxShotsToFire, CommandSourceType cmdSource)
{
	if (getObject() && !getObject()->isContained()) {
		AIUpdateInterface::privateAttackPosition(pos, maxShotsToFire, cmdSource);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CarrierDroneAIUpdate::~CarrierDroneAIUpdate(void)
{
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime CarrierDroneAIUpdate::update()
{
	Object* obj = getObject();
	const CarrierDroneAIUpdateModuleData* data = getCarrierDroneAIUpdateModuleData();

  // -------------
  // LAUNCH STATE (Not a real AI State)
  // -------------

	// Init launch state (check contained status)
	if (!obj->isContained() && m_isContained && m_launchFrame == 0) {
		DEBUG_LOG((">>> CarrierDroneAIUpdate::update() - Start Launch State %d", obj->getID()));
		obj->setModelConditionState(MODELCONDITION_TAKEOFF);
		chooseLocomotorSet(data->m_launchingLoco);
		m_launchFrame = TheGameLogic->getFrame();

		if (!m_launchingSound.isCurrentlyPlaying())
		{
			m_launchingSound.setObjectID(obj->getID());
			m_launchingSound.setPlayingHandle(TheAudio->addAudioEvent(&m_launchingSound));
		}
	}
	m_isContained = obj->isContained();

	// Exit launch state (check timer)
	if (m_launchFrame != 0) {
		UnsignedInt now = TheGameLogic->getFrame();
		if (m_launchFrame + data->m_launchTime <= now) {
			DEBUG_LOG((">>> CarrierDroneAIUpdate::update() - Stop Launch State %d", obj->getID()));
			obj->clearModelConditionState(MODELCONDITION_TAKEOFF);
			chooseLocomotorSet(LOCOMOTORSET_NORMAL);
			m_launchFrame = 0;

			if (m_launchingSound.isCurrentlyPlaying())
			{
				TheAudio->removeAudioEvent(m_launchingSound.getPlayingHandle());
			}
		}
	}

	


	// -------------
	// DOCKING STATE (Not a real AI State)
	// -------------

	Bool isDocking = false;
	Real distanceToTargetSquared = 0;
	Real dockingDistSquared = data->m_dockingDistance * data->m_dockingDistance;
	if (getStateMachine()->getCurrentStateID() == AI_ENTER && getGoalObject() != NULL) {
		distanceToTargetSquared = ThePartitionManager->getDistanceSquared(obj, getGoalObject(), FROM_CENTER_2D);
		DEBUG_LOG(("CarrierDroneAIUpdate::update() - distance = %f", sqrt(distanceToTargetSquared)));
		if (distanceToTargetSquared < dockingDistSquared) {
			isDocking = true;
		}
	}

	Locomotor* loco = getCurLocomotor();

	if (isDocking && !m_isDocking) {
		DEBUG_LOG((">>> CarrierDroneAIUpdate::update() - ENTER LANDING STATE %d", obj->getID()));

		//loco->setMaxSpeed(DOCKING_SPEED);
		chooseLocomotorSet(data->m_dockingLoco);

		Real speedMulti = (distanceToTargetSquared / dockingDistSquared) + 0.5f;
		loco->setMaxSpeed(loco->getMaxSpeedForCondition(BODY_PRISTINE) * speedMulti);

		loco->setUsePreciseZPos(true);
		obj->setModelConditionState(MODELCONDITION_LANDING);

		if (!m_dockingSound.isCurrentlyPlaying())
		{
			m_dockingSound.setObjectID(obj->getID());
			m_dockingSound.setPlayingHandle(TheAudio->addAudioEvent(&m_dockingSound));
		}

		m_isDocking = true;
	}
	else if (!isDocking && m_isDocking) {
		DEBUG_LOG(("<<< CarrierDroneAIUpdate::update() - EXIT LANDING STATE %d", obj->getID()));

		obj->clearModelConditionState(MODELCONDITION_LANDING);
		//loco->setMaxSpeed(loco->getMaxSpeedForCondition(getObject()->getBodyModule()->getDamageState()));
		chooseLocomotorSet(LOCOMOTORSET_NORMAL);
		loco->setUsePreciseZPos(false);

		if (m_dockingSound.isCurrentlyPlaying())
		{
			TheAudio->removeAudioEvent(m_dockingSound.getPlayingHandle());
		}

		m_isDocking = false;
	}

  // -------------
  // FLYING STATE (not a real AI State)
  // -------------
	PhysicsBehavior* physics = obj->getPhysics();
	if (!isDocking && physics->getVelocityMagnitude() > 0 && !obj->isContained()) {
		obj->setModelConditionState(MODELCONDITION_JETEXHAUST);
	}
	else {
		obj->clearModelConditionState(MODELCONDITION_JETEXHAUST);
	}

	return AIUpdateInterface::update();
}


// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void CarrierDroneAIUpdate::crc(Xfer* xfer)
{
	// extend base class
	AIUpdateInterface::crc(xfer);
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void CarrierDroneAIUpdate::xfer(Xfer* xfer)
{

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion(&version, currentVersion);

	// extend base class
	AIUpdateInterface::xfer(xfer);

	// docking state
	xfer->xferBool(&m_isDocking);

	// contained/launched tracker
	xfer->xferBool(&m_isContained);
	xfer->xferUnsignedInt(&m_launchFrame);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void CarrierDroneAIUpdate::loadPostProcess(void)
{
	// extend base class
	AIUpdateInterface::loadPostProcess();
}  // end loadPostProcess
