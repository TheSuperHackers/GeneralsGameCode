#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/DroneCarrierSlavedUpdate.h"
#include "GameLogic/Module/DroneCarrierAIUpdate.h"
#include "GameLogic/Object.h"

#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "Common/Team.h"
#include "Common/MiscAudio.h"
#include "GameClient/Drawable.h"
#include "GameClient/ParticleSys.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/SlavedUpdate.h"
#include "GameLogic/Weapon.h"
#include "Common/ThingTemplate.h"

// copies from SlavedUpdate.cpp
#define STRAY_MULTIPLIER 2.0f // Multiplier from stating diestance from tunnel, to max distance from
const Real CLOSE_ENOUGH = 15;				// Our moveTo commands and pathfinding can't handle people in the way, so quit trying to hump someone on your spot
const Real CLOSE_ENOUGH_SQR = (CLOSE_ENOUGH * CLOSE_ENOUGH);


DroneCarrierSlavedUpdateModuleData::DroneCarrierSlavedUpdateModuleData()
{
	m_leashRange = 0.0f;
}

void DroneCarrierSlavedUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	SlavedUpdateModuleData::buildFieldParse(p);
	static const FieldParse dataFieldParse[] =
	{
		{ "LeashRange",			INI::parseReal,	NULL, offsetof(DroneCarrierSlavedUpdateModuleData, m_leashRange) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

DroneCarrierSlavedUpdate::DroneCarrierSlavedUpdate(Thing* thing, const ModuleData* moduleData) : SlavedUpdate(thing, moduleData)
{
}

//-------------------------------------------------------------------------------------------------
DroneCarrierSlavedUpdate::~DroneCarrierSlavedUpdate(void)
{
}

UpdateSleepTime DroneCarrierSlavedUpdate::update(void)
{
	if (m_framesToWait > 0)
	{
		m_framesToWait--;
	}
	if (m_repairState == REPAIRSTATE_NONE)
	{
		if (m_framesToWait > 0)
		{
			return UPDATE_SLEEP_NONE;
		}
		m_framesToWait = SLAVED_UPDATE_RATE;
	}

	if (m_slaver == INVALID_ID)
		return UPDATE_SLEEP_NONE;

	const DroneCarrierSlavedUpdateModuleData* data = getDroneCarrierSlavedUpdateModuleData();
	Object* me = getObject();
	if (!me)
	{
		return UPDATE_SLEEP_NONE;
	}
	AIUpdateInterface* myAI = me->getAIUpdateInterface();
	if (!myAI)
	{
		return UPDATE_SLEEP_NONE;
	}
	Locomotor* locomotor = myAI->getCurLocomotor();
	if (!locomotor)
	{
		return UPDATE_SLEEP_NONE;
	}

	Object* master = TheGameLogic->findObjectByID(m_slaver);
	if (!master || master->isEffectivelyDead() || master->isDisabledByType(DISABLED_UNMANNED))
	{
		stopSlavedEffects();

		//Let's disable the drone so it crashes instead!
		//Added special case code in physics falling to ensure death.
		me->setDisabled(DISABLED_UNMANNED);
		if (me->getAI())
			me->getAI()->aiIdle(CMD_FROM_AI);

		return UPDATE_SLEEP_NONE;
	}
	else
	{
		Team* masterTeam = master->getTeam();
		Team* myTeam = me->getTeam();
		if (masterTeam->getRelationship(myTeam) != ALLIES)
		{//slaver must have been hijacked or something..	// we will join his team
			me->defect(masterTeam, 0);
		}
	}

	if (data->m_stayOnSameLayerAsMaster)
		me->setLayer(master->getLayer());

	
	Object* target = NULL;
	AIUpdateInterface* masterAI = master->getAIUpdateInterface();
	if (masterAI && DroneCarrierAIUpdate::isDroneCombatReady(me))
	{
		target = masterAI->getCurrentVictim();
	}

	AIUpdateInterface* ai = me->getAI();
	if (ai == nullptr) return UPDATE_SLEEP_NONE;

	if (data->m_attackRange && target != nullptr)
	{
		//Check distance to master
		Real dist = ThePartitionManager->getDistanceSquared(me, master->getPosition(), FROM_CENTER_2D);
		if (data->m_leashRange > 0.0f && dist > sqr(data->m_leashRange))
		{
			//Call back when too far away
			ai->aiEnter(master, CMD_FROM_AI);
		}
		else {
			doAttackLogic(target);
		}
		return UPDATE_SLEEP_NONE;
	}

	// No Target, go home to carrier
	if (!me->isContained()) {
		if (ai->isIdle()) {
			ai->aiEnter(master, CMD_FROM_AI);
		}
	}
	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void DroneCarrierSlavedUpdate::crc(Xfer* xfer)
{

	// extend base class
	SlavedUpdate::crc(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void DroneCarrierSlavedUpdate::xfer(Xfer* xfer)
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion(&version, currentVersion);

	// extend base class
	SlavedUpdate::xfer(xfer);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void DroneCarrierSlavedUpdate::loadPostProcess(void)
{

	// extend base class
	SlavedUpdate::loadPostProcess();

}  // end loadPostProcess
