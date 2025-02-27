/*
**	Command & Conquer Generals(tm)
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

// FILE: StructureToppleUpdate.cpp ///////////////////////////////////////////////////////////////////////
// Author:
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/INI.h"
#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "GameClient/FXList.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/BoneFXUpdate.h"
#include "GameLogic/Module/StructureToppleUpdate.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/Weapon.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"



const Int MAX_IDX = 32;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StructureToppleUpdate::StructureToppleUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{

	//Added By Sadullah Nader
	//Initialization(s) inserted
	m_delayBurstLocation.zero();
	m_structuralIntegrity = 0.0f;
	m_toppleDirection.x = m_toppleDirection.y = 0;
	//
	m_toppleFrame = 0;
	m_toppleState = TOPPLESTATE_STANDING;
	m_toppleVelocity = 0.0f;
	m_accumulatedAngle = 0.001f; // Need to give it a little nudge in the right direction
	m_lastCrushedLocation = 0.0f;
	m_nextBurstFrame = -1;
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);

	//Get the extent height here, rather than after it dies -- when it switches to dead state
	//the rubble state has a tiny height.
	Object *building = getObject();
	m_buildingHeight = building->getGeometryInfo().getMaxHeightAbovePosition();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StructureToppleUpdate::~StructureToppleUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
static void parseOCL( INI* ini, void *instance, void * /*store*/, const void* /*userData*/ )
{
	StructureToppleUpdateModuleData* self = (StructureToppleUpdateModuleData*)instance;
	StructureTopplePhaseType stphase = (StructureTopplePhaseType)INI::scanIndexList(ini->getNextToken(), TheStructureTopplePhaseNames);
	for (const char* token = ini->getNextToken(); token != NULL; token = ini->getNextTokenOrNull())
	{
		const ObjectCreationList *ocl = TheObjectCreationListStore->findObjectCreationList(token);	// could be null! this is OK!
		self->m_ocls[stphase].push_back(ocl);
	}
}

//-------------------------------------------------------------------------------------------------
static void parseAngleFX(INI* ini, void *instance, void * /* store */, const void * /*userData*/)
{
	StructureToppleUpdateModuleData* self = (StructureToppleUpdateModuleData*)instance;
	AngleFXInfo info;
	INI::parseReal(ini, instance, &(info.angle), NULL);
	info.angle = info.angle * PI / 180.0f; // convert from degrees to radians.
	INI::parseFXList(ini, instance, &(info.fxList), NULL);
	self->angleFX.push_back(info);
}

//-------------------------------------------------------------------------------------------------
/*static*/ void StructureToppleUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "MinToppleDelay",						INI::parseDurationUnsignedInt,		NULL, offsetof( StructureToppleUpdateModuleData, m_minToppleDelay ) },
		{ "MaxToppleDelay",						INI::parseDurationUnsignedInt,		NULL, offsetof( StructureToppleUpdateModuleData, m_maxToppleDelay ) },
		{ "MinToppleBurstDelay",			INI::parseDurationUnsignedInt,		NULL, offsetof( StructureToppleUpdateModuleData, m_minToppleBurstDelay ) },
		{ "MaxToppleBurstDelay",			INI::parseDurationUnsignedInt,		NULL, offsetof( StructureToppleUpdateModuleData, m_maxToppleBurstDelay ) },
		{ "StructuralIntegrity",			INI::parseReal,										NULL, offsetof( StructureToppleUpdateModuleData, m_structuralIntegrity ) },
		{ "StructuralDecay",					INI::parseReal,										NULL, offsetof( StructureToppleUpdateModuleData, m_structuralDecay ) },
		{ "DamageFXTypes",						INI::parseDamageTypeFlags,				NULL, offsetof( StructureToppleUpdateModuleData, m_damageFXTypes ) },
		{ "TopplingFX",								INI::parseFXList,									NULL, offsetof( StructureToppleUpdateModuleData, m_toppleFXList ) },
		{ "ToppleDelayFX",						INI::parseFXList,									NULL, offsetof( StructureToppleUpdateModuleData, m_toppleDelayFXList ) },
		{ "ToppleStartFX",						INI::parseFXList,									NULL, offsetof( StructureToppleUpdateModuleData, m_toppleStartFXList ) },
		{ "ToppleDoneFX",							INI::parseFXList,									NULL, offsetof( StructureToppleUpdateModuleData, m_toppleDoneFXList ) },
		{ "CrushingFX",								INI::parseFXList,									NULL, offsetof( StructureToppleUpdateModuleData, m_crushingFXList ) },
		{ "CrushingWeaponName",				INI::parseAsciiString,						NULL, offsetof( StructureToppleUpdateModuleData, m_crushingWeaponName ) },
		{ "OCL",											parseOCL,													NULL, 0 },
		{ "AngleFX",									parseAngleFX,											NULL, 0 },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
	p.add(DieMuxData::getFieldParse(), offsetof( StructureToppleUpdateModuleData, m_dieMuxData ));
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureToppleUpdate::beginStructureTopple(const DamageInfo *damageInfo)
{
	const StructureToppleUpdateModuleData *d = getStructureToppleUpdateModuleData();

	if (d)
	{
		UnsignedInt now = TheGameLogic->getFrame();
		m_toppleFrame = now + GameLogicRandomValue(d->m_minToppleDelay, d->m_maxToppleDelay);

		Object *attacker = TheGameLogic->findObjectByID(damageInfo->in.m_sourceID);
		Object *building = getObject();

		Real toppleAngle = 0.0;
		if (attacker == NULL) {
			toppleAngle = GameLogicRandomValueReal(0.0, 2*PI);
		} else {
			const Coord3D *attackerPos = attacker->getPosition();
			const Coord3D *buildingPos = building->getPosition();

			// Calculate the topple direction to be the opposite of the direction fired from.
			m_toppleDirection.x = buildingPos->x - attackerPos->x;
			m_toppleDirection.y = buildingPos->y - attackerPos->y;

			// Give it a little randomness...
			toppleAngle = m_toppleDirection.toAngle();
			toppleAngle += GameLogicRandomValueReal(-PI/8, PI/8);
		}
		m_toppleDirection.x = Cos(toppleAngle);
		m_toppleDirection.y = Sin(toppleAngle);
		TheScriptEngine->adjustToppleDirection(getObject(), &m_toppleDirection);

		Real averageRadius = (building->getGeometryInfo().getMajorRadius() + building->getGeometryInfo().getMinorRadius()) / 2;
		Real explosionRadius = averageRadius * 0.90;

		m_delayBurstLocation.x = building->getPosition()->x + explosionRadius * Cos(toppleAngle);
		m_delayBurstLocation.y = building->getPosition()->y + explosionRadius * Sin(toppleAngle);
		m_delayBurstLocation.z = TheTerrainLogic->getGroundHeight(m_delayBurstLocation.x, m_delayBurstLocation.y);

		doToppleStartFX(building, damageInfo);
		m_nextBurstFrame = now + GameClientRandomValue(d->m_minToppleBurstDelay, d->m_maxToppleBurstDelay);

		m_toppleState = TOPPLESTATE_WAITINGFORTOPPLESTART;

		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureToppleUpdate::onDie( const DamageInfo *damageInfo )
{
	const StructureToppleUpdateModuleData* d = getStructureToppleUpdateModuleData();
	if (!d->m_dieMuxData.isDieApplicable(getObject(), damageInfo))
		return;

	AIUpdateInterface *ai = getObject()->getAIUpdateInterface();
	if (ai)
		ai->markAsDead();

	// Deselect the object for all players.
	TheGameLogic->deselectObject(getObject(), PLAYERMASK_ALL, TRUE);

	beginStructureTopple(damageInfo);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime StructureToppleUpdate::update( void )
{
	static const Real TOPPLE_ACCELERATION_FACTOR = 0.02f;

	const StructureToppleUpdateModuleData *d = getStructureToppleUpdateModuleData();

	if (m_toppleState == TOPPLESTATE_STANDING)
	{
		DEBUG_CRASH(("hmm, what?"));
		return UPDATE_SLEEP_FOREVER;
	}

	// get last damage info
	const DamageInfo *lastDamageInfo = getObject()->getBodyModule()->getLastDamageInfo();

	// We are in the dramatic pause between when the building has lost all its hit points and
	// when it starts toppling over.
	if (m_toppleState == TOPPLESTATE_WAITINGFORTOPPLESTART) {
		UnsignedInt now = TheGameLogic->getFrame();
		if (now >= m_nextBurstFrame) {
			doToppleDelayBurstFX();
			// This uses a game client random value because the delay bursts are purely visual and aural effects.
			m_nextBurstFrame = now + GameClientRandomValue(d->m_minToppleBurstDelay, d->m_maxToppleBurstDelay);
		}
		if (now >= m_toppleFrame) {
			m_toppleState = TOPPLESTATE_TOPPLING;
			m_structuralIntegrity = d->m_structuralIntegrity;
		}
	}

	// The building is in the process of falling over.
	if (m_toppleState == TOPPLESTATE_TOPPLING) {
		UnsignedInt now = TheGameLogic->getFrame();
		Real toppleAcceleration = TOPPLE_ACCELERATION_FACTOR * (Sin(m_accumulatedAngle) * (1.0 - m_structuralIntegrity));
//		DEBUG_LOG(("toppleAcceleration = %f", toppleAcceleration));
		m_toppleVelocity += toppleAcceleration;
//		DEBUG_LOG(("m_toppleVelocity = %f", m_toppleVelocity));

		// doesn't make sense to have a structural integrity less than zero.
		if (m_structuralIntegrity > 0.0f) {
			m_structuralIntegrity *= d->m_structuralDecay;
			if (m_structuralIntegrity < 0.0f) {
				m_structuralIntegrity = 0.0f;
			}
		}
//		DEBUG_LOG(("m_structuralIntegrity = %f", m_structuralIntegrity));

		doAngleFX(m_accumulatedAngle, m_accumulatedAngle + m_toppleVelocity);

		m_accumulatedAngle += m_toppleVelocity;

		applyCrushingDamage(PI/2 - m_accumulatedAngle);

		if (m_accumulatedAngle >= PI/2) {
			m_toppleVelocity -= m_accumulatedAngle - PI/2;
			m_accumulatedAngle = PI/2;
			m_toppleState = TOPPLESTATE_WAITINGFORDONE;

			applyCrushingDamage(0.0f);
			doPhaseStuff(STPHASE_FINAL, getObject()->getPosition());

			if( lastDamageInfo == NULL || getDamageTypeFlag( d->m_damageFXTypes, lastDamageInfo->in.m_damageType ) )
				FXList::doFXObj(d->m_toppleDoneFXList, getObject());

			m_toppleFrame = TheGameLogic->getFrame();
		}

		if (now >= m_nextBurstFrame) {
			doToppleDelayBurstFX();
			// This uses a game client random value because the delay bursts are purely visual and aural effects.
			m_nextBurstFrame = now + GameClientRandomValue(d->m_minToppleBurstDelay, d->m_maxToppleBurstDelay);
		}

		Object *building = getObject();
		Matrix3D xfrm = *building->getTransformMatrix();
		xfrm.In_Place_Pre_Rotate_X(-m_toppleVelocity * m_toppleDirection.y);
		xfrm.In_Place_Pre_Rotate_Y(m_toppleVelocity * m_toppleDirection.x);
		building->setTransformMatrix(&xfrm);
	}

	// The building is now flat on the ground and done with all the crushing and all that.
	if (m_toppleState == TOPPLESTATE_WAITINGFORDONE)
	{
		if (m_toppleFrame <= TheGameLogic->getFrame())
		{
			Object *building = getObject();
			Drawable *drawable = building->getDrawable();
			drawable->clearModelConditionState(MODELCONDITION_RUBBLE);
			drawable->setModelConditionState(MODELCONDITION_POST_COLLAPSE);


			// Need to update body particle systems, now
			BodyModuleInterface *body = building->getBodyModule();
			body->updateBodyParticleSystems();

			doToppleDoneStuff();

			m_toppleState = TOPPLESTATE_DONE;

			return UPDATE_SLEEP_FOREVER;
		}
	}

	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureToppleUpdate::doToppleDoneStuff()
{
	static NameKeyType key_BoneFXUpdate = NAMEKEY("BoneFXUpdate");
	BoneFXUpdate *bfxu = (BoneFXUpdate *)getObject()->findUpdateModule(key_BoneFXUpdate);
	if (bfxu != NULL) {
		bfxu->stopAllBoneFX();
	}

	Object *building = getObject();

	Real origAngle = building->getOrientation();
	building->setOrientation(origAngle);

	Real toppleAngle = m_toppleDirection.toAngle();

	Matrix3D xfrm = *building->getTransformMatrix();
	xfrm.In_Place_Pre_Rotate_Z(toppleAngle-origAngle);
	building->setTransformMatrix(&xfrm);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureToppleUpdate::doAngleFX(Real curAngle, Real newAngle)
{
	const StructureToppleUpdateModuleData *d = getStructureToppleUpdateModuleData();
	const DamageInfo *lastDamageInfo = getObject()->getBodyModule()->getLastDamageInfo();

	for (std::vector<AngleFXInfo>::const_iterator it = d->angleFX.begin(); it != d->angleFX.end(); ++it)
	{
		if ((it->angle > curAngle) && (it->angle <= newAngle))
		{
			if( lastDamageInfo == NULL || getDamageTypeFlag( d->m_damageFXTypes, lastDamageInfo->in.m_damageType ) )
				FXList::doFXObj(it->fxList, getObject());
		}
	}

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// theta is the angle of the building with respect to the ground.
void StructureToppleUpdate::applyCrushingDamage(Real theta)
{
//	static const Real THETA_CEILING = PI/8; // This weapon won't do any damage until theta is less than this value.
	static const Real THETA_CEILING = PI/6; // This weapon won't do any damage until theta is less than this value.
	static const Real WEAPON_SPACING_PERPENDICULAR = 25;	// The spacing between weapon firing locations,
																												// distance is perpendicular to the direction the
																												// building is falling in.

	const StructureToppleUpdateModuleData *d = getStructureToppleUpdateModuleData();
	if (theta > THETA_CEILING) {
		return;
	}

	Object *building = getObject();
	Real orientationAngle = building->getOrientation();
	Real toppleAngle = m_toppleDirection.toAngle();

	// Figure out the width of the projection of the boundary of the building along the topple direction.
	// Do this because the amount of ground that is affected will be different if the building falls
	// in different orientations.
	Real angle = orientationAngle - toppleAngle;
	Real minorComponent = building->getGeometryInfo().getMinorRadius() * Cos(angle);
	Real majorComponent = building->getGeometryInfo().getMajorRadius() * Sin(angle);

	Coord3D temp3D;
	temp3D.x = majorComponent;
	temp3D.y = minorComponent;
	temp3D.z = 0.0f;

	Real facingWidth = temp3D.length() / 2;

	// Get the crushing weapon.
	const WeaponTemplate* wt = TheWeaponStore->findWeaponTemplate(d->m_crushingWeaponName);
	if (wt == NULL) {
		return;
	}

	// The furthest away from the base of the building to explode on.
	Real maxDistance = m_buildingHeight * (1.0 - Sin(theta));

	/*
	 * Fire explosions at regular intervals across the area that the building is currently
	 * crushing.  The explosions occur across the face and along the length of the building.
	 */
	Real jcos;
	Real jsin;
//	Coord3D target;
	Real j = m_lastCrushedLocation;
	for (; j < maxDistance; j += WEAPON_SPACING_PERPENDICULAR) {
		jcos = j * Cos(toppleAngle);
		jsin = j * Sin(toppleAngle);
		doDamageLine(building, wt, jcos, jsin, facingWidth, toppleAngle);
	}


	jcos = maxDistance * Cos(toppleAngle);
	jsin = maxDistance * Sin(toppleAngle);
	doDamageLine(building, wt, jcos, jsin, facingWidth, toppleAngle);

	m_lastCrushedLocation = j;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureToppleUpdate::doDamageLine(Object *building, const WeaponTemplate* wt, Real jcos, Real jsin, Real facingWidth, Real toppleAngle)
{
	const DamageInfo *lastDamageInfo = getObject()->getBodyModule()->getLastDamageInfo();
	static const Real WEAPON_SPACING_PARALLEL = 25;				// The spacing between weapon firing locations,
																												// distance is parallel to the direction the building
																												// is falling in.

	const StructureToppleUpdateModuleData *d = getStructureToppleUpdateModuleData();

	Coord3D target;

	for (Real i = -facingWidth; i < facingWidth; i += WEAPON_SPACING_PARALLEL)
	{
		target.x = building->getPosition()->x + jcos + (i * Sin(toppleAngle));
		target.y = building->getPosition()->y + jsin + (i * Cos(toppleAngle));
		target.z = TheTerrainLogic->getGroundHeight(target.x, target.y);

	  TheWeaponStore->createAndFireTempWeapon(wt, building, &target);

		// do the crushing particle effects
		if( lastDamageInfo == NULL || getDamageTypeFlag( d->m_damageFXTypes, lastDamageInfo->in.m_damageType ) )
			FXList::doFXPos(d->m_crushingFXList, &target);
	}

	// Make sure there are weapons fired and FX done on the edge of the building.
	target.x = building->getPosition()->x + jcos + (facingWidth * Sin(toppleAngle));
	target.y = building->getPosition()->y + jsin + (facingWidth * Cos(toppleAngle));
	target.z = TheTerrainLogic->getGroundHeight(target.x, target.y);

  TheWeaponStore->createAndFireTempWeapon(wt, building, &target);

	// do the crushing particle effects
	if( lastDamageInfo == NULL || getDamageTypeFlag( d->m_damageFXTypes, lastDamageInfo->in.m_damageType ) )
		FXList::doFXPos(d->m_crushingFXList, &target);

	// Do the flying debris for this line.
	target.x = building->getPosition()->x + jcos;
	target.y = building->getPosition()->y + jsin;
	target.z = TheTerrainLogic->getGroundHeight(target.x, target.y);

	doPhaseStuff(STPHASE_FINAL, &target);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureToppleUpdate::doToppleStartFX(Object *building, const DamageInfo *damageInfo)
{
	const StructureToppleUpdateModuleData *d = getStructureToppleUpdateModuleData();
	const DamageInfo *lastDamageInfo = getObject()->getBodyModule()->getLastDamageInfo();

	if( lastDamageInfo == NULL || getDamageTypeFlag( d->m_damageFXTypes, lastDamageInfo->in.m_damageType ) )
		FXList::doFXPos(d->m_toppleStartFXList, building->getPosition());

	doPhaseStuff(STPHASE_INITIAL, building->getPosition());
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureToppleUpdate::doToppleDelayBurstFX()
{
	const StructureToppleUpdateModuleData *d = getStructureToppleUpdateModuleData();
	const DamageInfo *lastDamageInfo = getObject()->getBodyModule()->getLastDamageInfo();

	DEBUG_LOG(("Doing topple delay burst on frame %d", TheGameLogic->getFrame()));
	if( lastDamageInfo == NULL || getDamageTypeFlag( d->m_damageFXTypes, lastDamageInfo->in.m_damageType ) )
		FXList::doFXPos(d->m_toppleDelayFXList, &m_delayBurstLocation);

	Object *building = getObject();
	Drawable *drawable = building->getDrawable();

	if( lastDamageInfo == NULL || getDamageTypeFlag( d->m_damageFXTypes, lastDamageInfo->in.m_damageType ) )
	{

		for (std::vector<FXBoneInfo>::const_iterator it = d->fxbones.begin(); it != d->fxbones.end(); ++it)
		{
			ParticleSystem *sys = TheParticleSystemManager->createParticleSystem(it->particleSystemTemplate);
			if (sys != NULL)
			{
				Coord3D pos;
				if (drawable->getPristineBonePositions(it->boneName.str(), 0, &pos, NULL, 1) == 1)
				{
					// got the bone position...
					sys->setPosition(&pos);

					// Attatch it to the object...
					sys->attachToDrawable(drawable);
				}
			}
		}
	}

	doPhaseStuff(STPHASE_DELAY, &m_delayBurstLocation);

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
inline Bool inList(Int value, Int count, const Int idxList[])
{
	for (Int j = 0; j < count; ++j)
	{
		if (idxList[j] == value)
			return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
static void buildNonDupRandomIndexList(Int range, Int count, Int idxList[])
{
	for (Int i = 0; i < count; ++i)
	{
		Int idx;
		do
		{
			idx = GameLogicRandomValue(0, range-1);
		}
		while (inList(idx, i, idxList));
		idxList[i] = idx;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void StructureToppleUpdate::doPhaseStuff(StructureTopplePhaseType stphase, const Coord3D *target)
{
	const StructureToppleUpdateModuleData* d = getStructureToppleUpdateModuleData();
	Int i, idx, count, listSize;
	Int idxList[MAX_IDX];

	listSize = d->m_ocls[stphase].size();
	if (listSize > 0)
	{
		count = d->m_oclCount[stphase];
		buildNonDupRandomIndexList(listSize, count, idxList);
		for (i = 0; i < count; ++i)
		{
			idx = idxList[i];
			const OCLVec& v = d->m_ocls[stphase];
			DEBUG_ASSERTCRASH(idx>=0&&idx<v.size(),("bad idx"));
			const ObjectCreationList* ocl = v[idx];
			ObjectCreationList::create(ocl, getObject(), target, NULL);
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void StructureToppleUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void StructureToppleUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// topple frame
	xfer->xferUnsignedInt( &m_toppleFrame );

	// topple direction
	xfer->xferCoord2D( &m_toppleDirection );

	// topple state
	xfer->xferUser( &m_toppleState, sizeof( StructureToppleStateType ) );

	// topple velocity
	xfer->xferReal( &m_toppleVelocity );

	// accumulated angle
	xfer->xferReal( &m_accumulatedAngle );

	// structural integrity
	xfer->xferReal( &m_structuralIntegrity );

	// last crushed location
	xfer->xferReal( &m_lastCrushedLocation );

	// next burst frame
	xfer->xferInt( &m_nextBurstFrame );

	// delay burst location
	xfer->xferCoord3D( &m_delayBurstLocation );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void StructureToppleUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
