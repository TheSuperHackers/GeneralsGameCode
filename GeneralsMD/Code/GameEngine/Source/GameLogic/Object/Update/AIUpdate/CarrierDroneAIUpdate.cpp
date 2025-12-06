// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/CRCDebug.h"
#include "Common/Player.h"
#include "Common/Team.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameClient/Drawable.h"
#include "GameClient/ParticleSys.h"

#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Weapon.h"

#include "GameLogic/Module/CarrierDroneAIUpdate.h"
#include "GameLogic/Module/JetAIUpdate.h"
#include "GameLogic/Module/ProductionUpdate.h"
#include "GameLogic/Module/ContainModule.h"

CarrierDroneAIUpdate::CarrierDroneAIUpdate(Thing* thing, const ModuleData* moduleData) : AIUpdateInterface(thing, moduleData)
{
}

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

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void CarrierDroneAIUpdate::loadPostProcess(void)
{
	// extend base class
	AIUpdateInterface::loadPostProcess();
}  // end loadPostProcess
