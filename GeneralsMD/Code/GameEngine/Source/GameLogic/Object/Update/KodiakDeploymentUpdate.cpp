// FILE: KodiakDeploymentUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Desc:   Update module to handle deployment of the Kodiak Generals special power.from command center
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_DEATH_NAMES

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Xfer.h"
#include "Common/ClientUpdateModule.h"

#include "GameClient/ControlBar.h"
#include "GameClient/GameClient.h"
#include "GameClient/Drawable.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/FXList.h"
#include "GameClient/ParticleSys.h"

#include "GameLogic/Locomotor.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectIter.h"
#include "GameLogic/WeaponSet.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameLogic/Module/KodiakDeploymentUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/LaserUpdate.h"
#include "GameLogic/Module/ActiveBody.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"




//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
KodiakDeploymentUpdateModuleData::KodiakDeploymentUpdateModuleData()
{
	m_specialPowerTemplate			   = NULL;
	m_extraRequiredScience				 = SCIENCE_INVALID;
/******BOTH*******//*BOTH*//******BOTH*******//******BOTH*******/  m_attackAreaRadius             = 200.0f;
	m_createLoc = CREATE_GUNSHIP_AT_EDGE_FARTHEST_FROM_TARGET;

}


static const char* TheGunshipCreateLocTypeNames[] =
{
	"CREATE_AT_EDGE_NEAR_SOURCE",
  "CREATE_AT_EDGE_FARTHEST_FROM_SOURCE",
	"CREATE_AT_EDGE_NEAR_TARGET",
	"CREATE_AT_EDGE_FARTHEST_FROM_TARGET",
	NULL
};



static Real zero = 0.0f;
//-------------------------------------------------------------------------------------------------
/*static*/ void KodiakDeploymentUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "GunshipTemplateName",	    INI::parseAsciiString,				    NULL, offsetof( KodiakDeploymentUpdateModuleData, m_gunshipTemplateName ) },
		{ "RequiredScience",					INI::parseScience,								NULL, offsetof( KodiakDeploymentUpdateModuleData, m_extraRequiredScience ) },
/******BOTH*******/   { "SpecialPowerTemplate",     INI::parseSpecialPowerTemplate,   NULL, offsetof( KodiakDeploymentUpdateModuleData, m_specialPowerTemplate ) },
/*******BOTH******/		{ "AttackAreaRadius",	        INI::parseReal,				            NULL, offsetof( KodiakDeploymentUpdateModuleData, m_attackAreaRadius ) },
		{ "CreateLocation", INI::parseIndexList, TheGunshipCreateLocTypeNames, offsetof( KodiakDeploymentUpdateModuleData, m_createLoc ) },

    { 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
KodiakDeploymentUpdate::KodiakDeploymentUpdate( Thing *thing, const ModuleData* moduleData ) : SpecialPowerUpdateModule( thing, moduleData )
{
	m_specialPowerModule = NULL;
  m_gunshipID  = INVALID_ID;
}

//-------------------------------------------------------------------------------------------------
KodiakDeploymentUpdate::~KodiakDeploymentUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
// Validate that we have the necessary data from the ini file.
//-------------------------------------------------------------------------------------------------
void KodiakDeploymentUpdate::onObjectCreated()
{
	const KodiakDeploymentUpdateModuleData *data = getKodiakDeploymentUpdateModuleData();
	Object *obj = getObject();

	if( !data->m_specialPowerTemplate )
	{
		DEBUG_CRASH( ("%s object's KodiakDeploymentUpdate lacks access to the SpecialPowerTemplate. Needs to be specified in ini.", obj->getTemplate()->getName().str() ) );
		return;
	}

	m_specialPowerModule = obj->getSpecialPowerModule( data->m_specialPowerTemplate );
}

//-------------------------------------------------------------------------------------------------
Bool KodiakDeploymentUpdate::initiateIntentToDoSpecialPower(const SpecialPowerTemplate* specialPowerTemplate, const Object* targetObj, const Coord3D* targetPos, const Waypoint* way, UnsignedInt commandOptions)
{
	const KodiakDeploymentUpdateModuleData* data = getKodiakDeploymentUpdateModuleData();

	if (m_specialPowerModule->getSpecialPowerTemplate() != specialPowerTemplate)
	{
		//Check to make sure our modules are connected.
		return FALSE;
	}

	//	getObject()->getControllingPlayer()->getAcademyStats()->recordSpecialPowerUsed( specialPowerTemplate );

	if (!BitIsSet(commandOptions, COMMAND_FIRED_BY_SCRIPT))
	{
		/******CHANGE*******/		m_initialTargetPosition.set(targetPos);
	}
	else
	{
		UnsignedInt now = TheGameLogic->getFrame();
		m_specialPowerModule->setReadyFrame(now);
		/******CHANGE*******/   	m_initialTargetPosition.set(targetPos);
		//		setLogicalStatus( GUNSHIPDEPLOY_STATUS_INSERTING );
	}

	Object* newGunship = TheGameLogic->findObjectByID(m_gunshipID);
	const ThingTemplate* gunshipTemplate = TheThingFactory->findTemplate(data->m_gunshipTemplateName);
	if (newGunship != NULL)
	{
		//    disengageAndDepartAO( newGunship );
		m_gunshipID = INVALID_ID;
		newGunship = NULL;
	}


	// Lets make a gunship, since we have none.
	{
		newGunship = TheThingFactory->newObject(gunshipTemplate, getObject()->getTeam());
	}

	DEBUG_ASSERTCRASH(newGunship, ("KodiakUpdate failed to find or create a gunship object"));
	if (newGunship)
	{
		//PRODUCER
		newGunship->setProducer(getObject());

		//POSITION
		Coord3D creationCoord;
		switch (data->m_createLoc)
		{
		case CREATE_GUNSHIP_AT_EDGE_NEAR_SOURCE:
			creationCoord = TheTerrainLogic->findClosestEdgePoint(getObject()->getPosition());
			break;
		case CREATE_GUNSHIP_AT_EDGE_FARTHEST_FROM_SOURCE:
			creationCoord = TheTerrainLogic->findFarthestEdgePoint(getObject()->getPosition());
			break;
		case CREATE_GUNSHIP_AT_EDGE_NEAR_TARGET:
			creationCoord = TheTerrainLogic->findClosestEdgePoint(targetPos);
			break;
		case CREATE_GUNSHIP_AT_EDGE_FARTHEST_FROM_TARGET:
		default:
			creationCoord = TheTerrainLogic->findFarthestEdgePoint(targetPos);
			break;
		}

		// HERE WE NEED TO CREATE THE POINT FURTHER OFF THE MAP SO WE CANT SEE THE LAME HOVER AND ACCELLERATE BEHAVIOR
		Coord3D deltaToCreationPoint = m_initialTargetPosition;
		deltaToCreationPoint.sub(&creationCoord);
		Real distanceFromTarget = deltaToCreationPoint.length();
		deltaToCreationPoint.normalize();
		deltaToCreationPoint.x *= (distanceFromTarget + data->m_gunshipOrbitRadius);
		deltaToCreationPoint.y *= (distanceFromTarget + data->m_gunshipOrbitRadius);
		creationCoord.x = m_initialTargetPosition.x - deltaToCreationPoint.x;
		creationCoord.y = m_initialTargetPosition.y - deltaToCreationPoint.y;

		Real preferredElevation = newGunship->getAI()->getCurLocomotor()->getPreferredHeight();
		creationCoord.z = preferredElevation;
		newGunship->setPosition(&creationCoord);

		//ORIENTATION
		Real orient = atan2(m_initialTargetPosition.y - creationCoord.y, m_initialTargetPosition.x - creationCoord.x);
		newGunship->setOrientation(orient);

		// ID
		m_gunshipID = newGunship->getID();

		// FIRE THE SPECIAL POWER OF THE GUNSHIP
		SpecialPowerModuleInterface* shipSPMInterface = newGunship->getSpecialPowerModule(specialPowerTemplate);
		if (shipSPMInterface)
		{
			SpecialPowerModule* spModule = (SpecialPowerModule*)shipSPMInterface;
			spModule->markSpecialPowerTriggered(&m_initialTargetPosition);
			spModule->doSpecialPowerAtLocation(&m_initialTargetPosition, INVALID_ANGLE, commandOptions);

		}

		// MAKE THE GUNSHIP SELECTED (Update: Now only for the local player)

		// TheGameLogic->selectObject(newGunship, TRUE, getObject()->getControllingPlayer()->getPlayerMask(), TRUE);
		TheGameLogic->selectObject(newGunship, TRUE, getObject()->getControllingPlayer()->getPlayerMask(), getObject()->isLocallyControlled());


	}


	SpecialPowerModuleInterface* spmInterface = getObject()->getSpecialPowerModule(specialPowerTemplate);
	if (spmInterface)
	{
		SpecialPowerModule* spModule = (SpecialPowerModule*)spmInterface;
		spModule->markSpecialPowerTriggered(&m_initialTargetPosition);
	}

	return TRUE;
}




//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime KodiakDeploymentUpdate::update()
{
//	const SpectreGunshipDeploymentUpdateModuleData *data = getSpectreGunshipDeploymentUpdateModuleData();


	Object *me = getObject();
	// Abort conditions.
	if( me->testStatus(OBJECT_STATUS_SOLD)
    || me->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION)
    || me->isEffectivelyDead() )
	{

		return UPDATE_SLEEP_FOREVER;
	}

	return UPDATE_SLEEP_NONE;

}










// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void KodiakDeploymentUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void KodiakDeploymentUpdate::xfer( Xfer *xfer )
{
//	const SpectreGunshipDeploymentUpdateModuleData *data = getSpectreGunshipDeploymentUpdateModuleData();

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );
  xfer->xferObjectID( &m_gunshipID );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void KodiakDeploymentUpdate::loadPostProcess( void )
{
	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
