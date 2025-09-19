#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/FireWeaponAdvancedUpdate.h"
#include "GameLogic/WeaponStatus.h"
#include "GameLogic/GameLogic.h"
#include <GameLogic/TerrainLogic.h>



//-------------------------------------------------------------------------------------------------
FireWeaponAdvancedUpdateModuleData::FireWeaponAdvancedUpdateModuleData()
{
	m_weaponTemplate = NULL;
  m_initialDelayFrames = 0;
	m_exclusiveWeaponDelay = 0;
	m_scatterTargets = std::vector<Coord2D>();
	m_scatterTargetScalar = 1.0;
	m_fireHeight = -1.0; //if <0 we ignore
}

//-------------------------------------------------------------------------------------------------
/*static*/ void FireWeaponAdvancedUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Weapon",								INI::parseWeaponTemplate,	      NULL, offsetof( FireWeaponAdvancedUpdateModuleData, m_weaponTemplate ) },
		{ "InitialDelay",					INI::parseDurationUnsignedInt,	NULL, offsetof( FireWeaponAdvancedUpdateModuleData, m_initialDelayFrames ) },
		{ "ExclusiveWeaponDelay",	INI::parseDurationUnsignedInt,	NULL, offsetof( FireWeaponAdvancedUpdateModuleData, m_exclusiveWeaponDelay ) },
		{ "ScatterTarget",				FireWeaponAdvancedUpdateModuleData::parseScatterTarget,			NULL,							0 },
		{ "ScatterTargetScalar",	INI::parseReal,			NULL,							 offsetof(FireWeaponAdvancedUpdateModuleData, m_scatterTargetScalar)  },
		{ "FireHeight",	          INI::parseReal,			NULL,							 offsetof(FireWeaponAdvancedUpdateModuleData, m_fireHeight)  },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

/*static*/ void FireWeaponAdvancedUpdateModuleData::parseScatterTarget(INI* ini, void* instance, void* /*store*/, const void* /*userData*/)
{
	// Accept multiple listings of Coord2D's.
	FireWeaponAdvancedUpdateModuleData* self = (FireWeaponAdvancedUpdateModuleData*)instance;

	Coord2D target;
	target.x = 0;
	target.y = 0;
	INI::parseCoord2D(ini, NULL, &target, NULL);

	self->m_scatterTargets.push_back(target);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FireWeaponAdvancedUpdate::FireWeaponAdvancedUpdate( Thing *thing, const ModuleData* moduleData ) :
	UpdateModule( thing, moduleData ),
	m_weapon(NULL)
{
	const WeaponTemplate *tmpl = getFireWeaponAdvancedUpdateModuleData()->m_weaponTemplate;
	if (tmpl)
	{
		m_weapon = TheWeaponStore->allocateNewWeapon(tmpl, PRIMARY_WEAPON);
		m_weapon->loadAmmoNow( getObject() );
	}


  m_initialDelayFrame = TheGameLogic->getFrame() + getFireWeaponAdvancedUpdateModuleData()->m_initialDelayFrames;
	m_initialPosition = *getObject()->getPosition();

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FireWeaponAdvancedUpdate::~FireWeaponAdvancedUpdate( void )
{
	if (m_weapon)
		deleteInstance(m_weapon);
}

Coord3D FireWeaponAdvancedUpdate::getNextTargetPos() {
	const FireWeaponAdvancedUpdateModuleData* data = getFireWeaponAdvancedUpdateModuleData();
	if (!data->m_scatterTargets.empty() && m_weapon->getClipSize()>0) {

		//If we have scatter targets and a clipsize, use the targets
		int shot_index = m_weapon->getClipSize() - m_weapon->getRemainingAmmo();
		if (!data->m_scatterTargets.empty()) {
			size_t target_idx = shot_index % data->m_scatterTargets.size();
			Coord2D scatterOffset = data->m_scatterTargets.at(target_idx);
			Coord3D targetPos = m_initialPosition;
			//Calculate Target from Scatter

			scatterOffset.x *= data->m_scatterTargetScalar;
			scatterOffset.y *= data->m_scatterTargetScalar;

			const Coord3D srcPos = *getObject()->getPosition();
			Real angle = 0.0f; // getObject()->getOrientation();
			angle += atan2(targetPos.y - srcPos.y, targetPos.x - srcPos.x);

			Real cosA = Cos(angle);
			Real sinA = Sin(angle);
			Real scatterOffsetRotX = scatterOffset.x * cosA - scatterOffset.y * sinA;
			Real scatterOffsetRotY = scatterOffset.x * sinA + scatterOffset.y * cosA;
			scatterOffset.x = scatterOffsetRotX;
			scatterOffset.y = scatterOffsetRotY;

			targetPos.x += scatterOffset.x;
			targetPos.y += scatterOffset.y;
			targetPos.z = TheTerrainLogic->getGroundHeight(targetPos.x, targetPos.y);

			return targetPos;
		}
	}
	return *getObject()->getPosition();
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime FireWeaponAdvancedUpdate::update( void )
{

  if ( TheGameLogic->getFrame() < m_initialDelayFrame )
    return UPDATE_SLEEP_NONE;

	const FireWeaponAdvancedUpdateModuleData* data = getFireWeaponAdvancedUpdateModuleData();

	// If my weapon is ready, shoot it.
	if( isOkayToFire() )
	{
		Coord3D targetPos = getNextTargetPos();
		if (data->m_fireHeight >= 0.0) {
			Coord3D pos = targetPos;
			pos.z = TheTerrainLogic->getGroundHeight(pos.x, pos.y) + data->m_fireHeight;
			getObject()->setPosition(&pos);
		}

		m_weapon->forceFireWeapon( getObject(), &targetPos );
	}
	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool FireWeaponAdvancedUpdate::isOkayToFire()
{
	const Object *me = getObject();
	const FireWeaponAdvancedUpdateModuleData *data = getFireWeaponAdvancedUpdateModuleData();

	if( m_weapon == NULL )
		return FALSE;

	// Weapon is reloading
	if( m_weapon->getStatus() != READY_TO_FIRE )
		return FALSE;

	if( me->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) )
		return FALSE; // no hitting with a 0% building, cheater

	// Firing a real weapon surpresses this module
	if( data->m_exclusiveWeaponDelay > 0  &&  ( TheGameLogic->getFrame() < (me->getLastShotFiredFrame() + data->m_exclusiveWeaponDelay) ) )
		return FALSE;

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void FireWeaponAdvancedUpdate::crc( Xfer *xfer )
{
	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void FireWeaponAdvancedUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// weapon
	xfer->xferSnapshot( m_weapon );

  if ( version >= 2 )
    xfer->xferUnsignedInt( &m_initialDelayFrame );

	xfer->xferUser(&m_initialPosition, sizeof(Coord3D));

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void FireWeaponAdvancedUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
