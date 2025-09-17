// FILE: KodiakUpdate.h //////////////////////////////////////////////////////////////////////////
// Desc:   Update module to handle weapon firing of the Kodiak Generals special power.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __KODIAK_UPDATE_H_
#define __KODIAK_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/KindOf.h"
#include "GameLogic/Module/SpecialPowerUpdateModule.h"
#include "SpecialPowerModule.h"
#include <GameClient/RadiusDecal.h>
#include "UpdateModule.h"
#include "SpectreGunshipUpdate.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class SpecialPowerModule;
class ParticleSystem;
class FXList;
class AudioEventRTS;
enum ParticleSystemID CPP_11(: Int);

//#define MAX_OUTER_NODES 16
//#define TRACKERS

//#define PUCK

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class KodiakUpdateModuleData : public ModuleData
{
public:
	SpecialPowerTemplate *m_specialPowerTemplate;

  RadiusDecalTemplate   m_attackAreaDecalTemplate;
  RadiusDecalTemplate   m_targetingReticleDecalTemplate;
  UnsignedInt           m_orbitFrames;
  UnsignedInt           m_mainTargetingRate;
	UnsignedInt           m_sideTargetingRate;
	UnsignedInt           m_aaTargetingRate;
	UnsignedInt						m_turretRecenterFramesBeforeExit;
	UnsignedInt						m_initialAttackDelayFrames;

  Real                  m_attackAreaRadius;
	Real                  m_sideAttackAreaRadius;
	Real                  m_aaAttackAreaRadius;
  Real                  m_targetingReticleRadius;
  Real                  m_gunshipOrbitRadius;
	Real									m_missileLockRadius;
	UnsignedInt           m_numMainTurrets;
	UnsignedInt           m_numSideTurrets;
	UnsignedInt           m_numAATurrets;

	Real								 m_missileScatterRadius;
	std::vector<Coord2D> m_scatterTargets;	///< targets for missile attack, amount of targets should match clip size of primary weapon

	//const ParticleSystemTemplate * m_gattlingStrafeFXParticleSystem;

	KodiakUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);


	static void parseScatterTarget(INI* ini, void* instance, void* /*store*/, const void* /*userData*/);
private:

};

//-------------------------------------------------------------------------------------------------
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class KodiakUpdate : public SpecialPowerUpdateModule
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( KodiakUpdate, "KodiakUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( KodiakUpdate, KodiakUpdateModuleData );

public:

	KodiakUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// SpecialPowerUpdateInterface
	virtual Bool initiateIntentToDoSpecialPower(const SpecialPowerTemplate *specialPowerTemplate, const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions );
	virtual Bool isSpecialAbility() const { return false; }
	virtual Bool isSpecialPower() const { return true; }
	virtual Bool isActive() const {return m_status < GUNSHIP_STATUS_DEPARTING;}
	virtual SpecialPowerUpdateInterface* getSpecialPowerUpdateInterface() { return this; }
	virtual CommandOption getCommandOption() const { return (CommandOption)0; }
	virtual Bool isPowerCurrentlyInUse( const CommandButton *command = NULL ) const;

	virtual void onObjectCreated();
	virtual UpdateSleepTime update();

	void cleanUp();



	virtual Bool doesSpecialPowerHaveOverridableDestinationActive() const;
	virtual Bool doesSpecialPowerHaveOverridableDestination() const { return true; }	//Does it have it, even if it's not active?
	virtual void setSpecialPowerOverridableDestination( const Coord3D *loc );

	// Disabled conditions to process (termination conditions!)
	virtual DisabledMaskType getDisabledTypesToProcess() const { return MAKE_DISABLED_MASK4( DISABLED_SUBDUED, DISABLED_UNDERPOWERED, DISABLED_EMP, DISABLED_HACKED ); }
	static void parseScatterTarget(INI* ini, void* instance, void* /*store*/, const void* /*userData*/);
protected:

  void setLogicalStatus( GunshipStatus newStatus ) { m_status = newStatus; }
  void disengageAndDepartAO( Object *gunship );

  Bool isPointOffMap( const Coord3D& testPos ) const;
  Bool isFairDistanceFromShip( Object *target );

	SpecialPowerModuleInterface* m_specialPowerModule;

  void friend_enableAfterburners(Bool v);




  Coord3D				m_initialTargetPosition;
	Coord3D				m_overrideTargetDestination;
  Coord3D       m_movementPosition;
  Coord3D       m_positionToShootAt;


	GunshipStatus		m_status;

  UnsignedInt     m_orbitEscapeFrame;

	RadiusDecal			m_attackAreaDecal;
	RadiusDecal			m_targetingReticleDecal;

#if defined TRACKERS
  RadiusDecal			m_howitzerTrackerDecal;
#endif

  AudioEventRTS m_afterburnerSound;

};


#endif // __KODIAK_UPDATE_H_

