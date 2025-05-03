
#pragma once

#ifndef __SCATTER_SHOT_UPDATE_H_
#define __SCATTER_SHOT_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"

class FXList;
enum DeathType;

//-------------------------------------------------------------------------------------------------
class ScatterShotUpdateModuleData : public UpdateModuleData
{
public:
	const WeaponTemplate* m_weaponTemplate;
	UnsignedInt m_numShots;
	Real m_targetSearchRadius;
	Real m_targetMinRadius;
	UnsignedInt m_maxShotsPerTarget;
	Bool m_preferSimilarTargets;
	Bool m_preferNearestTargets;
	Real m_noTargetsScatterRadius;
	Bool m_attackGroundWhenNoTargets;

	Real m_triggerDistanceToTarget;
	Real m_triggerDistanceFromSource;
	Real m_triggerDistancePercent;
	UnsignedInt m_triggerLifetime;
	Bool m_triggerOnImpact;
	Bool m_triggerInstantly;

	Bool m_stayAliveAfterTrigger;

	DeathType m_triggerDeathType;
	const FXList* m_scatterFX;


	ScatterShotUpdateModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class ScatterShotUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ScatterShotUpdate, "ScatterShotUpdate")
		MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(ScatterShotUpdate, ScatterShotUpdateModuleData)

public:

	ScatterShotUpdate(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

protected:

	Weapon* m_weapon;
	UnsignedInt m_initialDelayFrame;
	Bool m_hasTriggered;
	Bool m_isInitialized; // are goal pos/obj set

	Coord3D m_goalPos;
	Object* m_goalObj;

	Real m_totalTargetDistance;

	void triggerScatterShot(void);
	Bool isValidTarget(const Object* victim) const;

private:
	Real getTargetDistance() const;

};

#endif

