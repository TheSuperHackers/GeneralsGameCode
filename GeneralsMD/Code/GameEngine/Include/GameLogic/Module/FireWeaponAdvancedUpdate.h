// FILE: FireWeaponAdvancedUpdate.h /////////////////////////////////////////////////////////////////////////
// Desc: expanded FireWeaponUpdate with more Parameters
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __FIRE_WEAPON_ADVANCED_UPDATE_H_
#define __FIRE_WEAPON_ADVANCED_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Weapon.h"
#include <GameClient/RadiusDecal.h>

//-------------------------------------------------------------------------------------------------
class FireWeaponAdvancedUpdateModuleData : public UpdateModuleData
{
public:
	const WeaponTemplate* m_weaponTemplate;
  UnsignedInt m_initialDelayFrames;
	UnsignedInt m_exclusiveWeaponDelay;	///< If non-zero, any other weapon having fired this recently will keep us from doing anything
	std::vector<Coord2D> m_scatterTargets;
	Real m_scatterTargetScalar;
	Real m_fireHeight;
	Real m_lockOnRadius;
	bool m_scatterOnLockedStructuresMajorRadius;
	Real m_fireOffsetXMin;
	Real m_fireOffsetXMax;
	Real m_fireOffsetYMin;
	Real m_fireOffsetYMax;
	RadiusDecalTemplate m_decalTemplate;
	Real m_decalRadius;
	UnsignedInt m_decalDuration;
	Real m_scatterRadius;

	FireWeaponAdvancedUpdateModuleData();

	static void parseScatterTarget(INI* ini, void* instance, void* /*store*/, const void* /*userData*/);
	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class FireWeaponAdvancedUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FireWeaponAdvancedUpdate, "FireWeaponAdvancedUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FireWeaponAdvancedUpdate, FireWeaponAdvancedUpdateModuleData )

public:

	FireWeaponAdvancedUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

protected:

	Bool isOkayToFire();
	Coord3D getNextTargetPos();
	void adjustFireHeight(Object* targetLock, Coord3D* targetPos);

	Weapon* m_weapon;
  UnsignedInt m_initialDelayFrame;
	Coord3D m_initialPosition;
	bool m_initialized;
	RadiusDecal										m_deliveryDecal;
	UnsignedInt m_radiusDecalRemoveFrame;
};

#endif

