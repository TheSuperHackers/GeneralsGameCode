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

// FILE: SpectreGunshipDeploymentUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen, April 2003
// Desc:   Update module to handle deployment of the SpectreGunship Generals special power.from command center
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SPECTRE_GUNSHIP_DEPLOYMENT_UPDATE_H_
#define __SPECTRE_GUNSHIP_DEPLOYMENT_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/KindOf.h"
#include "GameLogic/Module/SpecialPowerUpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class SpecialPowerModule;
class ParticleSystem;
class FXList;
class AudioEventRTS;
enum ParticleSystemID CPP_11(: Int);
enum ScienceType CPP_11(: Int);

//#define MAX_OUTER_NODES 16

//#define PUCK


enum GunshipCreateLocType CPP_11(: Int)
{
	CREATE_GUNSHIP_AT_EDGE_NEAR_SOURCE,
  CREATE_GUNSHIP_AT_EDGE_FARTHEST_FROM_SOURCE,
	CREATE_GUNSHIP_AT_EDGE_NEAR_TARGET,
	CREATE_GUNSHIP_AT_EDGE_FARTHEST_FROM_TARGET,
};



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpectreGunshipDeploymentUpdateModuleData : public ModuleData
{
public:
	SpecialPowerTemplate *m_specialPowerTemplate;
	ScienceType						m_extraRequiredScience;		///< science required (if any) to actually execute this power
	WeaponTemplate	      *m_howitzerWeaponTemplate;
  AsciiString           m_gunshipTemplateName;
  AsciiString           m_gattlingTemplateName;
//  AsciiString           m_howitzerTemplateName;
  RadiusDecalTemplate   m_attackAreaDecalTemplate;
  RadiusDecalTemplate   m_targetingReticleDecalTemplate;
  UnsignedInt           m_orbitFrames;
  Real                  m_attackAreaRadius;
  Real                  m_targetingReticleRadius;
  Real                  m_gunshipOrbitRadius;
	GunshipCreateLocType	m_createLoc;


	const ParticleSystemTemplate * m_gattlingStrafeFXParticleSystem;

	SpectreGunshipDeploymentUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

enum GunshipDeployStatus CPP_11(: Int)
{
   GUNSHIPDEPLOY_STATUS_INSERTING,
   GUNSHIPDEPLOY_STATUS_ORBITING,
   GUNSHIPDEPLOY_STATUS_DEPARTING,
   GUNSHIPDEPLOY_STATUS_IDLE,
};


//-------------------------------------------------------------------------------------------------
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class SpectreGunshipDeploymentUpdate : public SpecialPowerUpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpectreGunshipDeploymentUpdate, "SpectreGunshipDeploymentUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpectreGunshipDeploymentUpdate, SpectreGunshipDeploymentUpdateModuleData );

public:

	SpectreGunshipDeploymentUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// SpecialPowerUpdateInterface
	virtual Bool initiateIntentToDoSpecialPower(const SpecialPowerTemplate *specialPowerTemplate, const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions );
	virtual Bool isSpecialAbility() const { return false; }
	virtual Bool isSpecialPower() const { return true; }
	virtual Bool isActive() const {return FALSE;}
	virtual SpecialPowerUpdateInterface* getSpecialPowerUpdateInterface() { return this; }
	virtual CommandOption getCommandOption() const { return (CommandOption)0; }
  virtual Bool isPowerCurrentlyInUse( const CommandButton *command = NULL ) const { return FALSE; };
	virtual ScienceType getExtraRequiredScience() const { return getSpectreGunshipDeploymentUpdateModuleData()->m_extraRequiredScience; } //Does this object have more than one special power module with the same spTemplate?

	virtual void onObjectCreated();
	virtual UpdateSleepTime update();

	void cleanUp();



  virtual Bool doesSpecialPowerHaveOverridableDestinationActive() const { return FALSE; };
	virtual Bool doesSpecialPowerHaveOverridableDestination() const { return FALSE; }	//Does it have it, even if it's not active?
  virtual void setSpecialPowerOverridableDestination( const Coord3D *loc ) {};

	// Disabled conditions to process (termination conditions!)
	virtual DisabledMaskType getDisabledTypesToProcess() const { return MAKE_DISABLED_MASK4( DISABLED_SUBDUED, DISABLED_UNDERPOWERED, DISABLED_EMP, DISABLED_HACKED ); }

protected:



	SpecialPowerModuleInterface* m_specialPowerModule;
  Coord3D				m_initialTargetPosition;
  ObjectID        m_gunshipID;


};


#endif // __SPECTRE_GUNSHIP_DEPLOYMENT_UPDATE_H_

