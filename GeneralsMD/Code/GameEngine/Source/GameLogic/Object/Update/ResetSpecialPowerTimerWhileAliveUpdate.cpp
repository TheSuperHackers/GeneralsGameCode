
// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/BitFlagsIO.h"
#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "Common/SpecialPower.h"
#include "Common/Player.h"

#include "GameClient/Drawable.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectIter.h"
#include "GameLogic/Module/ResetSpecialPowerTimerWhileAliveUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Weapon.h"



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ResetSpecialPowerTimerWhileAliveUpdateModuleData::ResetSpecialPowerTimerWhileAliveUpdateModuleData()
{
	m_specialPowerTemplate		= NULL;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void ResetSpecialPowerTimerWhileAliveUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] = 
	{
		{ "SpecialPowerTemplate",				INI::parseSpecialPowerTemplate,				NULL, offsetof(ResetSpecialPowerTimerWhileAliveUpdateModuleData, m_specialPowerTemplate ) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
ResetSpecialPowerTimerWhileAliveUpdate::ResetSpecialPowerTimerWhileAliveUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	setWakeFrame(getObject(), UPDATE_SLEEP_NONE);// No starting sleep, but we want to sleep later.
} 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ResetSpecialPowerTimerWhileAliveUpdate::~ResetSpecialPowerTimerWhileAliveUpdate( void )
{

}


//-------------------------------------------------------------------------------------------------
void ResetSpecialPowerTimerWhileAliveUpdate::onObjectCreated()
{
	const ResetSpecialPowerTimerWhileAliveUpdateModuleData *data = getResetSpecialPowerTimerWhileAliveUpdateModuleData();
	
	//Make sure we have a weapon template
	if( !data->m_specialPowerTemplate )
	{
		DEBUG_CRASH( ("ResetSpecialPowerTimerWhileAliveUpdateModuleData for %s doesn't have a valid Special Power Template", 
			getObject()->getTemplate()->getName().str() ) );
		return;
	}

	//Make sure the special power template 
	if (!data->m_specialPowerTemplate->isSharedNSync()) {
		DEBUG_CRASH(("ResetSpecialPowerTimerWhileAliveUpdateModuleData for %s only supports SpecialPowerTemplates with shared sync timers!",
			getObject()->getTemplate()->getName().str()));
		return;
	}
}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime ResetSpecialPowerTimerWhileAliveUpdate::update()
{		
	Object *me = getObject();
	if( me->isEffectivelyDead() )
		return UPDATE_SLEEP_FOREVER; //No more resetting when dead

	const ResetSpecialPowerTimerWhileAliveUpdateModuleData *data = getResetSpecialPowerTimerWhileAliveUpdateModuleData();

	Player* owner = me->getControllingPlayer();
	if (owner != nullptr) {
		owner->resetOrStartSpecialPowerReadyFrame(data->m_specialPowerTemplate);
	}

	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ResetSpecialPowerTimerWhileAliveUpdate::crc( Xfer *xfer )
{
	// extend base class
	UpdateModule::crc( xfer );
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ResetSpecialPowerTimerWhileAliveUpdate::xfer( Xfer *xfer )
{
	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ResetSpecialPowerTimerWhileAliveUpdate::loadPostProcess( void )
{
	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
