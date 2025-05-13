/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: SpectreGunshipUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen, April 2003
// Desc:   Update module to handle weapon firing of the SpectreGunship Generals special power.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SPECTRE_GUNSHIP_UPDATE_H_
#define __SPECTRE_GUNSHIP_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/KindOf.h"
#include "GameLogic/Module/SpecialPowerUpdateModule.h"

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
class SpectreGunshipUpdateModuleData : public ModuleData
{
public:
	SpecialPowerTemplate *m_specialPowerTemplate;
	WeaponTemplate	      *m_howitzerWeaponTemplate;
//  AsciiString           m_gunshipTemplateName;
  AsciiString           m_gattlingTemplateName;
//  AsciiString           m_howitzerTemplateName;
  RadiusDecalTemplate   m_attackAreaDecalTemplate;
  RadiusDecalTemplate   m_targetingReticleDecalTemplate;
  UnsignedInt           m_orbitFrames;
  UnsignedInt           m_howitzerFiringRate;
  UnsignedInt           m_howitzerFollowLag;
  Real                  m_attackAreaRadius;
  Real                  m_targetingReticleRadius;
  Real                  m_gunshipOrbitRadius;
  Real                  m_strafingIncrement;
  Real                  m_orbitInsertionSlope;
  Real                  m_randomOffsetForHowitzer;

	const ParticleSystemTemplate * m_gattlingStrafeFXParticleSystem;

	SpectreGunshipUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

private: 

};

enum GunshipStatus CPP_11(: Int)
{
   GUNSHIP_STATUS_INSERTING,
   GUNSHIP_STATUS_ORBITING,
   GUNSHIP_STATUS_DEPARTING,
   GUNSHIP_STATUS_IDLE,
};


//-------------------------------------------------------------------------------------------------
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class SpectreGunshipUpdate : public SpecialPowerUpdateModule
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpectreGunshipUpdate, "SpectreGunshipUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpectreGunshipUpdate, SpectreGunshipUpdateModuleData );

public:

	SpectreGunshipUpdate( Thing *thing, const ModuleData* moduleData );
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

protected:

  void setLogicalStatus( GunshipStatus newStatus ) { m_status = newStatus; }
  void disengageAndDepartAO( Object *gunship );

  Bool isPointOffMap( const Coord3D& testPos ) const;
  Bool isFairDistanceFromShip( Object *target );

	SpecialPowerModuleInterface* m_specialPowerModule;

  void friend_enableAfterburners(Bool v);

  


  Coord3D				m_initialTargetPosition;
	Coord3D				m_overrideTargetDestination;
  Coord3D       m_satellitePosition;
  Coord3D       m_gattlingTargetPosition;
  Coord3D       m_positionToShootAt;


	GunshipStatus		m_status;

  UnsignedInt     m_okToFireHowitzerCounter;
  UnsignedInt     m_orbitEscapeFrame;


//  ObjectID        m_howitzerID;
  ObjectID        m_gattlingID;


	RadiusDecal			m_attackAreaDecal;
	RadiusDecal			m_targetingReticleDecal;




#if defined TRACKERS
  RadiusDecal			m_howitzerTrackerDecal;
#endif

  AudioEventRTS m_afterburnerSound; 
  AudioEventRTS m_howitzerFireSound;

};


#endif // __SPECTRE_GUNSHIP_UPDATE_H_

