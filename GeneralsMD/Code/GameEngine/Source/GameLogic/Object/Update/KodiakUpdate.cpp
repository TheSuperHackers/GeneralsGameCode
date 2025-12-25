// FILE: KodiakUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Desc:   Update module to handle weapon firing of the Kodiak Generals special power.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_DEATH_NAMES

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameAudio.h"
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
#include "GameLogic/Module/KodiakUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/LaserUpdate.h"
#include "GameLogic/Module/ActiveBody.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"


#define ONE (1.0f)
#define ZERO (0.0f)
#define LOTS_OF_SHOTS (9999)


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
KodiakUpdateModuleData::KodiakUpdateModuleData()
{
	m_specialPowerTemplate			   = NULL;
/******BOTH*******//*BOTH*//******BOTH*******//******BOTH*******/  m_attackAreaRadius             = 200.0f;
/*************/  //m_gattlingStrafeFXParticleSystem = NULL;
/*************/  //m_howitzerWeaponTemplate = NULL;
/*************/  m_orbitFrames                  = 0;
/*************/  m_targetingReticleRadius       = 25.0f;
/*************/  m_gunshipOrbitRadius           = 250.0f;
  m_missileLockRadius = 50.0f;
  m_mainTargetingRate = 100;

  m_sideTargetingRate = 100;
  m_aaTargetingRate = 100;

  m_attackAreaRadius = 250.0f;
  m_sideAttackAreaRadius = 250.0f;
  m_aaAttackAreaRadius = 250.0f;
  m_missileScatterRadius = 250.0f;

  m_numMainTurrets = 1;
  m_numSideTurrets = 0;
  m_numAATurrets = 0;
  m_turretRecenterFramesBeforeExit = 0;
  m_initialAttackDelayFrames = 0;
}

static Real zero = 0.0f;
//-------------------------------------------------------------------------------------------------
/*static*/ void KodiakUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
    { "SpecialPowerTemplate",           INI::parseSpecialPowerTemplate,   NULL, offsetof( KodiakUpdateModuleData, m_specialPowerTemplate ) },
		{ "MainTargetingRate",	            INI::parseDurationUnsignedInt,    NULL, offsetof( KodiakUpdateModuleData, m_mainTargetingRate ) },
		{ "SideTargetingRate",	            INI::parseDurationUnsignedInt,    NULL, offsetof( KodiakUpdateModuleData, m_sideTargetingRate ) },
		{ "AATargetingRate",	            INI::parseDurationUnsignedInt,    NULL, offsetof( KodiakUpdateModuleData, m_aaTargetingRate ) },
		{ "OrbitTime",	                    INI::parseDurationUnsignedInt,		NULL, offsetof( KodiakUpdateModuleData, m_orbitFrames ) },
    { "AttackAreaRadius",	              INI::parseReal,				            NULL, offsetof( KodiakUpdateModuleData, m_attackAreaRadius ) },
    { "SideAttackAreaRadius",	              INI::parseReal,				            NULL, offsetof(KodiakUpdateModuleData, m_sideAttackAreaRadius) },
    { "AAAttackAreaRadius",	              INI::parseReal,				            NULL, offsetof(KodiakUpdateModuleData, m_aaAttackAreaRadius) },
		{ "TargetingReticleRadius",	        INI::parseReal,				            NULL, offsetof( KodiakUpdateModuleData, m_targetingReticleRadius ) },
		{ "GunshipOrbitRadius",	            INI::parseReal,				            NULL, offsetof( KodiakUpdateModuleData, m_gunshipOrbitRadius ) },
    { "MissileLockRadius",	            INI::parseReal,				            NULL, offsetof( KodiakUpdateModuleData, m_missileLockRadius) },
		{ "AttackAreaDecal",		            RadiusDecalTemplate::parseRadiusDecalTemplate,	NULL, offsetof( KodiakUpdateModuleData, m_attackAreaDecalTemplate ) },
		{ "TargetingReticleDecal",		      RadiusDecalTemplate::parseRadiusDecalTemplate,	NULL, offsetof( KodiakUpdateModuleData, m_targetingReticleDecalTemplate ) },
    { "ScatterTarget",						KodiakUpdateModuleData::parseScatterTarget,			NULL,							0 },
    { "MainTurrets",	                    INI::parseUnsignedInt,		NULL, offsetof(KodiakUpdateModuleData, m_numMainTurrets) },
    { "SideTurrets",	                    INI::parseUnsignedInt,		NULL, offsetof(KodiakUpdateModuleData, m_numSideTurrets) },
    { "AATurrets",	                    INI::parseUnsignedInt,		NULL, offsetof(KodiakUpdateModuleData, m_numAATurrets) },
    { "MissileScatterRadius",	        INI::parseReal,		NULL, offsetof(KodiakUpdateModuleData, m_missileScatterRadius) },
    { "TurretRecenterTimeBeforeExit", INI::parseDurationUnsignedInt,		NULL, offsetof(KodiakUpdateModuleData, m_turretRecenterFramesBeforeExit) },
    { "InitialAttackDelay",           INI::parseDurationUnsignedInt,		NULL, offsetof(KodiakUpdateModuleData, m_initialAttackDelayFrames) },


    { 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

/*static*/ void KodiakUpdateModuleData::parseScatterTarget(INI* ini, void* instance, void* /*store*/, const void* /*userData*/)
{
  // Accept multiple listings of Coord2D's.
  KodiakUpdateModuleData* self = (KodiakUpdateModuleData*)instance;

  Coord2D target;
  target.x = 0;
  target.y = 0;
  INI::parseCoord2D(ini, NULL, &target, NULL);

  self->m_scatterTargets.push_back(target);
}

//-------------------------------------------------------------------------------------------------
KodiakUpdate::KodiakUpdate( Thing *thing, const ModuleData* moduleData ) : SpecialPowerUpdateModule( thing, moduleData )
{
	m_specialPowerModule = NULL;
	m_status = GUNSHIP_STATUS_IDLE;
	m_initialTargetPosition.zero();
	m_overrideTargetDestination.zero();
  m_positionToShootAt.zero();
	m_attackAreaDecal.clear();
	m_targetingReticleDecal.clear();
  m_orbitEscapeFrame = 0;
	m_afterburnerSound = *(getObject()->getTemplate()->getPerUnitSound("Afterburner"));

#if defined TRACKERS
m_howitzerTrackerDecal.clear();
#endif

}

//-------------------------------------------------------------------------------------------------
KodiakUpdate::~KodiakUpdate( void )
{
	m_attackAreaDecal.clear();
	m_targetingReticleDecal.clear();

#if defined TRACKERS
m_howitzerTrackerDecal.clear();
#endif

}

//-------------------------------------------------------------------------------------------------
// Validate that we have the necessary data from the ini file.
//-------------------------------------------------------------------------------------------------
void KodiakUpdate::onObjectCreated()
{
	const KodiakUpdateModuleData *data = getKodiakUpdateModuleData();
	Object *obj = getObject();

	if( !data->m_specialPowerTemplate )
	{
		DEBUG_CRASH( ("%s object's KodiakUpdate lacks access to the SpecialPowerTemplate. Needs to be specified in ini.", obj->getTemplate()->getName().str() ) );
		return;
	}

	m_specialPowerModule = obj->getSpecialPowerModule( data->m_specialPowerTemplate );
  m_movementPosition = Coord3D(0, 0, 0);
}

//-------------------------------------------------------------------------------------------------
Bool KodiakUpdate::initiateIntentToDoSpecialPower(const SpecialPowerTemplate *specialPowerTemplate, const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions )
{
	const KodiakUpdateModuleData *data = getKodiakUpdateModuleData();

	if( m_specialPowerModule->getSpecialPowerTemplate() != specialPowerTemplate )
	{
		//Check to make sure our modules are connected.
		return FALSE;
	}

	if( !BitIsSet( commandOptions, COMMAND_FIRED_BY_SCRIPT ) )
	{
		m_initialTargetPosition.set( targetPos );
		m_overrideTargetDestination.set( targetPos );
	}
	else
	{
		UnsignedInt now = TheGameLogic->getFrame();
		m_specialPowerModule->setReadyFrame( now );
   	m_initialTargetPosition.set( targetPos );
		setLogicalStatus( GUNSHIP_STATUS_INSERTING );
	}


  Object * gunShip = getObject();


  if ( gunShip )
  {
    // MAKE TWEAKS TO AI SO SHIP MOVES PRETTY
    AIUpdateInterface *shipAI = gunShip->getAIUpdateInterface();
    if ( shipAI)
    {
      // shipAI->chooseLocomotorSet( LOCOMOTORSET_PANIC );  // Use normal for transit
	    shipAI->getCurLocomotor()->setAllowInvalidPosition(TRUE);
	    shipAI->getCurLocomotor()->setUltraAccurate(TRUE);	// set ultra-accurate just so AI won't try to adjust our dest
    }

    //Drawable *draw = gunShip->getDrawable();
    //if ( draw )
    //  draw->clearAndSetModelConditionState( MODELCONDITION_DOOR_1_OPENING, MODELCONDITION_DOOR_1_CLOSING );

    //ContainModuleInterface* cmi = getObject()->getContain();
    //if (cmi) {
    //  ContainedItemsList* addOns = cmi->getAddOnList();
    //  if (addOns) {
    //    for (ContainedItemsList::iterator it = addOns->begin(); it != addOns->end(); it++) {
    //      draw = (*it)->getDrawable();
    //      if (draw) {
    //        draw->clearAndSetModelConditionState(MODELCONDITION_DOOR_1_OPENING, MODELCONDITION_DOOR_1_CLOSING);
    //      }
    //    }
    //  }
    //}

    friend_enableAfterburners(TRUE);

    setLogicalStatus( GUNSHIP_STATUS_INSERTING ); // The gunship is en route to the tharget area, from map-edge

  }


	data->m_attackAreaDecalTemplate.createRadiusDecal( *getObject()->getPosition(), data->m_attackAreaRadius, getObject()->getControllingPlayer(), m_attackAreaDecal);
	data->m_targetingReticleDecalTemplate.createRadiusDecal( *getObject()->getPosition(), data->m_targetingReticleRadius, getObject()->getControllingPlayer(), m_targetingReticleDecal);


#if defined TRACKERS
	data->m_targetingReticleDecalTemplate.createRadiusDecal( *getObject()->getPosition(), data->m_targetingReticleRadius, getObject()->getControllingPlayer(), m_howitzerTrackerDecal);
#endif


	SpecialPowerModuleInterface *spmInterface = getObject()->getSpecialPowerModule( specialPowerTemplate );
	if( spmInterface )
	{
		SpecialPowerModule *spModule = (SpecialPowerModule*)spmInterface;
		spModule->markSpecialPowerTriggered( &m_initialTargetPosition );
	}

	return TRUE;
}

//-------------------------------------------------------------------------------------------------
Bool KodiakUpdate::isPowerCurrentlyInUse( const CommandButton *command ) const
{
	return false;
}

//-------------------------------------------------------------------------------------------------
void KodiakUpdate::setSpecialPowerOverridableDestination( const Coord3D *loc )
{
	Object *me = getObject();
	if( !me->isDisabled() )
	{
		m_overrideTargetDestination = *loc;

		if( me->getControllingPlayer()  &&  me->getControllingPlayer()->isLocalPlayer() )
		{
			AudioEventRTS soundToPlay = *me->getTemplate()->getVoiceAttack();
			soundToPlay.setObjectID( me->getID() );
			TheAudio->addAudioEvent(&soundToPlay);
		}
	}
}

//-------------------------------------------------------------------------------------------------
Bool KodiakUpdate::isPointOffMap( const Coord3D& testPos ) const
{
	Region3D mapRegion;
	TheTerrainLogic->getExtentIncludingBorder( &mapRegion );

	if (!mapRegion.isInRegionNoZ( &testPos ))
		return true;

	return false;
}


// PARTITION FILTERS!
//-----------------------------------------------------------------------------
class PartitionFilterLiveMapEnemies : public PartitionFilter
{
private:
	const Object *m_obj;
public:
	PartitionFilterLiveMapEnemies(const Object *obj) : m_obj(obj) { }

	virtual Bool allow(Object *objOther)
	{
		// this is way fast (bit test) so do it first.
		if (objOther->isEffectivelyDead())
			return false;

		// this is also way fast (bit test) so do it next.
		if (objOther->isOffMap() != m_obj->isOffMap())
			return false;

		Relationship r = m_obj->getRelationship(objOther);
		if (r != ENEMIES)
			return false;

		return true;
	}

#if defined(RTS_DEBUG)
	virtual const char* debugGetName() { return "PartitionFilterLiveMapEnemies"; }
#endif
};

class PartitionFilterLiveMapAirEnemies : public PartitionFilter
{
private:
  const Object* m_obj;
public:
  PartitionFilterLiveMapAirEnemies(const Object* obj) : m_obj(obj) {}

  virtual Bool allow(Object* objOther)
  {
    // this is way fast (bit test) so do it first.
    if (objOther->isEffectivelyDead())
      return false;

    // this is also way fast (bit test) so do it next.
    if (objOther->isOffMap() != m_obj->isOffMap())
      return false;

    Relationship r = m_obj->getRelationship(objOther);
    if (r != ENEMIES)
      return false;

    if (!objOther->isAirborneTarget()) {
      return false;
    }
    return true;
  }

#if defined(RTS_DEBUG)
  virtual const char* debugGetName() { return "PartitionFilterLiveMapAirEnemies"; }
#endif
};
//-----------------------------------------------------------------------------





//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime KodiakUpdate::update()
{
	const KodiakUpdateModuleData *data = getKodiakUpdateModuleData();

  int total_turrets = data->m_numMainTurrets + data->m_numSideTurrets + data->m_numAATurrets;

   Object *gunship = getObject();
   if ( gunship )
   {
      if ( gunship->isEffectivelyDead() )
        return UPDATE_SLEEP_FOREVER;

      m_attackAreaDecal.update();
      m_targetingReticleDecal.update();
#if defined TRACKERS
	    m_howitzerTrackerDecal.update();
#endif

      AIUpdateInterface *shipAI = gunship->getAIUpdateInterface();

      // get the turrets
      std::vector<Object*> turrets;
      turrets.reserve(total_turrets);

      ContainModuleInterface* cmi = getObject()->getContain();
      if (cmi != nullptr) {
        ContainedItemsList* addOns = cmi->getAddOnList();
        for (ContainedItemsList::iterator it = addOns->begin(); it != addOns->end(); it++) {
          turrets.push_back(*it);
        }
      }



      if ( m_status == GUNSHIP_STATUS_INSERTING || m_status == GUNSHIP_STATUS_ORBITING )
      {
        Coord3D pos = *gunship->getPosition();
        pos.sub( &m_initialTargetPosition );
        pos.z = zero;
        Real distanceToTarget = pos.length();

        Real orbitalRadius = data->m_gunshipOrbitRadius;

        Coord3D zero_pos;
        zero_pos.zero();
        if (m_movementPosition.equals(zero_pos)) {
          // set the direction
          Coord3D direction = m_initialTargetPosition;
          direction.sub(gunship->getPosition());
          direction.z = zero;
          direction.normalize();

          m_movementPosition = direction;
          m_movementPosition.scale(200.0f);
        }

        Coord3D target_move_location = *gunship->getPosition();
        target_move_location.add(&m_movementPosition);
        
        if ( shipAI)
        {
           shipAI->aiMoveToPosition( &target_move_location, CMD_FROM_AI );
        }

        Real constraintRadius = data->m_attackAreaRadius - data->m_targetingReticleRadius;

        //Constrain Target Override to the targeting radius
        Coord3D overrideTargetDelta = m_initialTargetPosition;
        overrideTargetDelta.sub( &m_overrideTargetDestination );
        if ( overrideTargetDelta.length() > constraintRadius )
        {
          overrideTargetDelta.normalize();
          overrideTargetDelta.x *= constraintRadius;
          overrideTargetDelta.y *= constraintRadius;

          m_overrideTargetDestination.x = m_initialTargetPosition.x - overrideTargetDelta.x;
          m_overrideTargetDestination.y = m_initialTargetPosition.y - overrideTargetDelta.y;

        }

        m_attackAreaDecal.setPosition( m_initialTargetPosition );
        m_targetingReticleDecal.setPosition( m_overrideTargetDestination );


#if defined TRACKERS
	 m_howitzerTrackerDecal.setPosition( m_gattlingTargetPosition );
#endif


        if ( (m_status == GUNSHIP_STATUS_INSERTING) && (distanceToTarget < orbitalRadius ) )// close enough to shoot
        {
          setLogicalStatus( GUNSHIP_STATUS_ORBITING );
          m_orbitEscapeFrame = TheGameLogic->getFrame() + data->m_orbitFrames;

          AIUpdateInterface *shipAI = gunship->getAIUpdateInterface();
          if ( shipAI)
          {
            shipAI->chooseLocomotorSet( LOCOMOTORSET_PANIC );  // Transit locomotor should be default
	          shipAI->getCurLocomotor()->setAllowInvalidPosition(TRUE);
	          shipAI->getCurLocomotor()->setUltraAccurate(TRUE);	// set ultra-accurate just so AI won't try to adjust our dest
          }

          Drawable *draw = gunship->getDrawable();
          if ( draw )
            draw->clearAndSetModelConditionState( MODELCONDITION_DOOR_1_CLOSING, MODELCONDITION_DOOR_1_OPENING );

          for (Object* turret : turrets) {
            draw = turret->getDrawable();
            if (draw) {
              draw->clearAndSetModelConditionState(MODELCONDITION_DOOR_1_CLOSING, MODELCONDITION_DOOR_1_OPENING);
            }
          }

          friend_enableAfterburners(FALSE);

          AudioEventRTS soundEventDescend = *(gunship->getTemplate()->getPerUnitSound("Descend"));
          soundEventDescend.setObjectID(gunship->getID());

          if (!soundEventDescend.getEventName().isEmpty())
          {
            TheAudio->addAudioEvent(&soundEventDescend);
          }

        }

      } // endif status == ORBITING || INSERTING


      if ( m_status == GUNSHIP_STATUS_ORBITING && TheGameLogic->getFrame() >= (m_orbitEscapeFrame - data->m_orbitFrames + data->m_initialAttackDelayFrames) ) // delay until attack
      {
        Object *validTargetObject = NULL;


        if ( TheGameLogic->getFrame() >= m_orbitEscapeFrame )
        {
          cleanUp();
          setLogicalStatus( GUNSHIP_STATUS_DEPARTING );

          // CEASE FIRE, RETURN TO BASE
          disengageAndDepartAO( gunship );


        }//endif escapeframe
        else if (TheGameLogic->getFrame() >= m_orbitEscapeFrame - data->m_turretRecenterFramesBeforeExit) { 
          for (auto& turret : turrets) {
            AIUpdateInterface* ai = turret->getAI();
            if (ai != nullptr) {
              //ai->setTurretEnabled(WhichTurretType::TURRET_MAIN, false);
              ai->aiAttackPosition(&m_initialTargetPosition, 0, CMD_FROM_AI);
              ai->recenterTurret(WhichTurretType::TURRET_MAIN);
            }
          }
        }
        else
        {

          // ONLY EVERY FEW FRAMES DO WE RE_EVALUATE THE TARGET OBJECT
          if ( TheGameLogic->getFrame() %data->m_mainTargetingRate < ONE )
          {

            m_positionToShootAt = m_overrideTargetDestination; // unless we get a hit, below

	          PartitionFilterLiveMapEnemies filterObvious( gunship );
	          PartitionFilterStealthedAndUndetected filterStealth( gunship, false );
	          PartitionFilterPossibleToAttack filterAttack(ATTACK_NEW_TARGET, gunship, CMD_FROM_AI);
	          PartitionFilterFreeOfFog filterFogged( gunship->getControllingPlayer()->getPlayerIndex() );
	          PartitionFilter *filters[6];
	          Int numFilters = 0;
	          filters[numFilters++] = &filterObvious;
	          filters[numFilters++] = &filterStealth;
	          filters[numFilters++] = &filterAttack;
	          filters[numFilters++] = &filterFogged;
	          filters[numFilters] = NULL;


            // THIS WILL FIND A VALID TARGET WITHIN THE TARGETING RETICLE
	          ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange(&m_overrideTargetDestination,
              data->m_targetingReticleRadius,
              FROM_BOUNDINGSPHERE_2D,
              filters,
              ITER_SORTED_NEAR_TO_FAR);
	          MemoryPoolObjectHolder holder(iter);
	          for (Object *theEnemy = iter->first(); theEnemy; theEnemy = iter->next())
	          {
              if ( theEnemy && isFairDistanceFromShip( theEnemy ) )
              {
                validTargetObject = theEnemy;
                break;
              }
	          }



            // WE WANT THE WIDE_RANGE AUTOACQUIRE POWER DISABLED FOR HUMAN PLAYERS
            // SO THAT THE SPECTREGUNSHIP REQUIRES BABYSITTING AT ALL TIMES
            if (gunship->getControllingPlayer()->getPlayerType() != PLAYER_HUMAN )
            {
              if ( ! validTargetObject )
              {
                // set a flag to start the targeting decal fading, since there is nothing to kill there
                // THIS WILL FIND A VALID TARGET ANYWHERE INSIDE THE TARGETING AREA (THE BIG CIRCLE)
	              ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange(&m_initialTargetPosition,
                  data->m_attackAreaRadius,
                  FROM_BOUNDINGSPHERE_2D,
                  filters,
                  ITER_SORTED_NEAR_TO_FAR);
	              MemoryPoolObjectHolder holder(iter);
	              for (Object *theEnemy = iter->first(); theEnemy; theEnemy = iter->next())
	              {
                  if ( theEnemy && isFairDistanceFromShip( theEnemy ) )
                  {
                    // WE GOT A HIT!!!! SHOOT HIM!
                    validTargetObject = theEnemy;
                    m_positionToShootAt = *validTargetObject->getPosition();

                    break;
                  }
	              }
              }
            }

            // Order Main turrets to attack location
            for (UnsignedInt i = 0; i < data->m_numMainTurrets; i++) {
              AIUpdateInterface* ai = turrets.at(i)->getAI();
              if (ai != nullptr) {
                ai->aiAttackPosition(&m_positionToShootAt, LOTS_OF_SHOTS, CMD_FROM_AI);
              }
            }

          }//endif frame modulator

          /* Auto Acquire Targets with side Turrets */
          if (data->m_numSideTurrets > 0 && TheGameLogic->getFrame() % data->m_sideTargetingRate < ONE)
          {
            PartitionFilterLiveMapEnemies filterObvious(gunship);
            PartitionFilterStealthedAndUndetected filterStealth(gunship, false);
            PartitionFilterPossibleToAttack filterAttack(ATTACK_NEW_TARGET, gunship, CMD_FROM_AI);
            PartitionFilterFreeOfFog filterFogged(gunship->getControllingPlayer()->getPlayerIndex());
            PartitionFilter* filters[6];
            Int numFilters = 0;
            filters[numFilters++] = &filterObvious;
            filters[numFilters++] = &filterStealth;
            filters[numFilters++] = &filterAttack;
            filters[numFilters++] = &filterFogged;
            filters[numFilters] = NULL;

            std::vector<Object*> targets;
            targets.reserve(data->m_numSideTurrets);

            ObjectIterator* iter = ThePartitionManager->iterateObjectsInRange(gunship->getPosition(),
              data->m_sideAttackAreaRadius,
              FROM_BOUNDINGSPHERE_2D,
              filters,
              ITER_SORTED_CHEAP_TO_EXPENSIVE);
            MemoryPoolObjectHolder holder(iter);
            for (Object* theEnemy = iter->first(); theEnemy && (targets.size() < data->m_numSideTurrets); theEnemy = iter->next())
            {
              targets.push_back(theEnemy);
            }
            if (!targets.empty()) {
              for (UnsignedInt i = 0U; i < data->m_numSideTurrets; i++) {
                AIUpdateInterface* ai = turrets.at(i + data->m_numMainTurrets)->getAI();
                if (ai != nullptr) {
                  Object* target = nullptr;
                  if (targets.size() > i) {
                    target = targets.at(i);
                  }
                  else {
                    target = targets.at(targets.size() - 1);
                  }
                  ai->aiAttackObject(target, LOTS_OF_SHOTS, CMD_FROM_AI);
                }
              }
            }
          }

          //AA TURRET TARGETING
          if (data->m_numAATurrets > 0 && TheGameLogic->getFrame() % data->m_aaTargetingRate < ONE)
          {
            PartitionFilterLiveMapAirEnemies filterObvious(gunship);
            PartitionFilterStealthedAndUndetected filterStealth(gunship, false);
            PartitionFilterPossibleToAttack filterAttack(ATTACK_NEW_TARGET, turrets.at(data->m_numMainTurrets+data->m_numSideTurrets), CMD_FROM_AI);
            PartitionFilterFreeOfFog filterFogged(gunship->getControllingPlayer()->getPlayerIndex());
            PartitionFilter* filters[6];
            Int numFilters = 0;
            filters[numFilters++] = &filterObvious;
            filters[numFilters++] = &filterStealth;
            filters[numFilters++] = &filterAttack;
            filters[numFilters++] = &filterFogged;
            filters[numFilters] = NULL;

            std::vector<Object*> targets;
            targets.reserve(data->m_numAATurrets);

            ObjectIterator* iter = ThePartitionManager->iterateObjectsInRange(gunship->getPosition(),
              data->m_aaAttackAreaRadius,
              FROM_BOUNDINGSPHERE_2D,
              filters,
              ITER_SORTED_CHEAP_TO_EXPENSIVE);
            MemoryPoolObjectHolder holder(iter);
            for (Object* theEnemy = iter->first(); theEnemy && (targets.size() < data->m_numAATurrets); theEnemy = iter->next())
            {
              if (theEnemy)
              {
                targets.push_back(theEnemy);
              }
            }
            if (!targets.empty()) {
              for (UnsignedInt i = 0U; i < data->m_numAATurrets; i++) {
                AIUpdateInterface* ai = turrets.at(i + data->m_numMainTurrets + data->m_numSideTurrets)->getAI();
                if (ai != nullptr) {
                  Object* target = nullptr;
                  if (targets.size() > i) {
                    target = targets.at(i);
                  }
                  else {
                    target = targets.at(targets.size() - 1);
                  }
                  ai->aiAttackObject(target, LOTS_OF_SHOTS, CMD_FROM_AI);
                }
              }
            }
          }

          //Missile Barrage
          Weapon* weap = getObject()->getWeaponInWeaponSlot(WeaponSlotType::PRIMARY_WEAPON);
          if (weap != nullptr) {
            if (weap->getPossibleNextShotFrame() <= TheGameLogic->getFrame()) {

              int shot_index = weap->getClipSize() - weap->getRemainingAmmo();
              if (!data->m_scatterTargets.empty()) {
                size_t target_idx = shot_index % data->m_scatterTargets.size();
                Coord2D scatterOffset = data->m_scatterTargets.at(target_idx);
                Coord3D targetPos = m_initialTargetPosition;
                //Calculate Target from Scatter

                scatterOffset.x *= data->m_missileScatterRadius;
                scatterOffset.y *= data->m_missileScatterRadius;

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


                // Check for a valid target near impact
                PartitionFilterLiveMapEnemies filterObvious(gunship);
                PartitionFilterStealthedAndUndetected filterStealth(gunship, false);
                PartitionFilterPossibleToAttack filterAttack(ATTACK_NEW_TARGET, gunship, CMD_FROM_AI);
                PartitionFilterFreeOfFog filterFogged(gunship->getControllingPlayer()->getPlayerIndex());
                PartitionFilter* filters[6];
                Int numFilters = 0;
                filters[numFilters++] = &filterObvious;
                filters[numFilters++] = &filterStealth;
                filters[numFilters++] = &filterAttack;
                filters[numFilters++] = &filterFogged;
                filters[numFilters] = NULL;

                Object* targetLock = nullptr;
                // THIS WILL FIND A VALID TARGET WITHIN THE TARGETING RETICLE
                ObjectIterator* iter = ThePartitionManager->iterateObjectsInRange(&targetPos,
                  data->m_missileLockRadius,
                  FROM_BOUNDINGSPHERE_2D,
                  filters,
                  ITER_SORTED_EXPENSIVE_TO_CHEAP);
                MemoryPoolObjectHolder holder(iter);
                for (Object* theEnemy = iter->first(); theEnemy && targetLock == nullptr; theEnemy = iter->next())
                {
                  if (theEnemy)
                  {
                    targetLock = theEnemy;
                  }
                }

                if (targetLock != nullptr) {
                  weap->fireWeapon(getObject(), targetLock);
                }
                else {
                  weap->fireWeapon(getObject(), &targetPos);
                }
              }
              else {
                weap->fireWeapon(getObject(), getObject()->getPosition());
              }
            }

          }

        }// end else


      }//not orbiting
      else if ( m_status == GUNSHIP_STATUS_DEPARTING )
      {
        if ( isPointOffMap( *gunship->getPosition() ) )
        {

          TheGameLogic->destroyObject( gunship );
          setLogicalStatus( GUNSHIP_STATUS_IDLE );

          cleanUp();

          // HERE WE NEED TO CLEAN UP THE TERRAIN DECALS, AND ANYTHING ELSE THAT WAS CREATED IN INIT-INTENT[]

        }
      }

   } // endif gunship
   else if ( m_status != GUNSHIP_STATUS_IDLE )
   {
     //OH MY GOODNESS, THE GUNSHIP MUST HAVE GOTTEN SHOT DOWN!
      setLogicalStatus( GUNSHIP_STATUS_IDLE );

      cleanUp();
   }


	return UPDATE_SLEEP_NONE;

}


//-------------------------------------------------------------------------------------------------
void KodiakUpdate::friend_enableAfterburners(Bool v)
{
	Object* gunship = getObject();
	if (v)
	{
		gunship->setModelConditionState(MODELCONDITION_JETAFTERBURNER);
		if (!m_afterburnerSound.isCurrentlyPlaying())
		{
			m_afterburnerSound.setObjectID(gunship->getID());
			m_afterburnerSound.setPlayingHandle(TheAudio->addAudioEvent(&m_afterburnerSound));
		}
	}
	else
	{
		gunship->clearModelConditionState(MODELCONDITION_JETAFTERBURNER);
		if (m_afterburnerSound.isCurrentlyPlaying())
		{
			TheAudio->removeAudioEvent(m_afterburnerSound.getPlayingHandle());
		}
	}
}



Bool KodiakUpdate::isFairDistanceFromShip( Object *target )
{

  Object *gunship = getObject();
  if ( !gunship )
    return FALSE;

  if ( ! target )
    return FALSE;

  const Coord3D *targetPosition = target->getPosition();
  const Coord3D *gunshipPosition = gunship->getPosition();

  Coord3D shipToTargetDelta;
  shipToTargetDelta.x = gunshipPosition->x - targetPosition->x;
  shipToTargetDelta.y = gunshipPosition->y - targetPosition->y;
  shipToTargetDelta.z = 0.0f;

  return (shipToTargetDelta.length() > getKodiakUpdateModuleData()->m_gunshipOrbitRadius);

}







//-------------------------------------------------------------------------------------------------
void KodiakUpdate::cleanUp()
{
  m_attackAreaDecal.clear();
  m_targetingReticleDecal.clear();

#if defined TRACKERS
	 m_howitzerTrackerDecal.clear();
#endif
}





// --------------------------------------------------------------------------------------------------
void KodiakUpdate::disengageAndDepartAO( Object *gunship )
{

  if ( gunship == NULL )
    return;

  AIUpdateInterface *shipAI = gunship->getAIUpdateInterface();

  if ( shipAI)
  {
    Coord3D exitPoint;// head off the map in the direction you are facing
    gunship->getUnitDirectionVector3D( exitPoint );
    Real mapSize = 99999.0f;
    exitPoint.x *= mapSize;
    exitPoint.y *= mapSize;
    exitPoint.add( gunship->getPosition() );

    shipAI->aiMoveToPosition( &exitPoint, CMD_FROM_AI );



  }

    if ( shipAI)
    {
      shipAI->chooseLocomotorSet( LOCOMOTORSET_NORMAL );
	    shipAI->getCurLocomotor()->setAllowInvalidPosition(TRUE);
	    shipAI->getCurLocomotor()->setUltraAccurate(TRUE);	// set ultra-accurate just so AI won't try to adjust our dest
    }

    Drawable *draw = gunship->getDrawable();
    if ( draw )
      draw->clearAndSetModelConditionState( MODELCONDITION_DOOR_1_OPENING, MODELCONDITION_DOOR_1_CLOSING );

    ContainModuleInterface* cmi = getObject()->getContain();
    if (cmi) {
      ContainedItemsList* addOns = cmi->getAddOnList();
      if (addOns) {
        for (ContainedItemsList::iterator it = addOns->begin(); it != addOns->end(); it++) {
          draw = (*it)->getDrawable();
          if (draw) {
            draw->clearAndSetModelConditionState(MODELCONDITION_DOOR_1_OPENING, MODELCONDITION_DOOR_1_CLOSING);
          }
        }
      }
    }

    friend_enableAfterburners(TRUE);

    AudioEventRTS soundEventAscend = *(getObject()->getTemplate()->getPerUnitSound("Ascend"));
    soundEventAscend.setObjectID(getObject()->getID());

    if (!soundEventAscend.getEventName().isEmpty())
    {
      TheAudio->addAudioEvent(&soundEventAscend);
    }

  cleanUp();

  return;

}




//-------------------------------------------------------------------------------------------------
Bool KodiakUpdate::doesSpecialPowerHaveOverridableDestinationActive() const
{
	 return m_status < GUNSHIP_STATUS_DEPARTING;
}


// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void KodiakUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version
	* 2: Can't flat-save decals, that's a hella crash (they aren't saved (no memory) and they have a pointer in them).  And half the class wasn't saved.
*/
// ------------------------------------------------------------------------------------------------
void KodiakUpdate::xfer( Xfer *xfer )
{
//	const SpectreGunshipUpdateModuleData *data = getSpectreGunshipUpdateModuleData();

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );




  // The initial target destination.
	xfer->xferCoord3D( &m_initialTargetPosition );
	// The manual override target destination.
	xfer->xferCoord3D( &m_overrideTargetDestination );
	// The current move-to point of the gunship.
	xfer->xferCoord3D( &m_movementPosition );
	// status
	xfer->xferUser( &m_status, sizeof( GunshipStatus ) );

  xfer->xferUnsignedInt( &m_orbitEscapeFrame );

	if( version < 2 )
	{
		xfer->xferUser( &m_attackAreaDecal, sizeof( RadiusDecal ) );
		xfer->xferUser( &m_targetingReticleDecal, sizeof( RadiusDecal ) );

#if defined TRACKERS
		xfer->xferUser( &m_howitzerTrackerDecal, sizeof( RadiusDecal ) );
#endif
	}

	if( version >= 2 )
	{
		xfer->xferCoord3D( &m_positionToShootAt );
	}

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void KodiakUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
