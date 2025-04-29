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

// FILE: DrawModule.h /////////////////////////////////////////////////////////////////////////////////
// Author: 
// Desc:	 
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DRAWMODULE_H_
#define __DRAWMODULE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameType.h"
#include "Common/Module.h"
#include "Common/ModelState.h"
#include "GameClient/Color.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Matrix3D;
class RenderCost;
class OBBoxClass;

// TYPES //////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** DRAWABLE DRAW MODULE base class */
//-------------------------------------------------------------------------------------------------

class ObjectDrawInterface;
class DebrisDrawInterface;
class TracerDrawInterface;
class RopeDrawInterface;
class LaserDrawInterface;
class FXList;
enum TerrainDecalType CPP_11(: Int);
enum ShadowType CPP_11(: Int);

//class ModelConditionFlags;

class DrawModule : public DrawableModule
{

	MEMORY_POOL_GLUE_ABC( DrawModule )
	MAKE_STANDARD_MODULE_MACRO_ABC( DrawModule )

public:
	
	DrawModule( Thing *thing, const ModuleData* moduleData );
	static ModuleType getModuleType() { return MODULETYPE_DRAW; }
	static Int getInterfaceMask() { return MODULEINTERFACE_DRAW; }
	
	virtual void doDrawModule(const Matrix3D* transformMtx) = 0;

	virtual void setShadowsEnabled(Bool enable) = 0;
	virtual void releaseShadows(void) = 0;	///< frees all shadow resources used by this module - used by Options screen.
	virtual void allocateShadows(void) = 0; ///< create shadow resources if not already present. Used by Options screen.

#if defined(RTS_DEBUG) || defined(_INTERNAL)	
	virtual void getRenderCost(RenderCost & rc) const { };  ///< estimates the render cost of this draw module
#endif

	virtual void setTerrainDecal(TerrainDecalType type) {};
	virtual void setTerrainDecalSize(Real x, Real y) {};
	virtual void setTerrainDecalOpacity(Real o) {};

	virtual void setFullyObscuredByShroud(Bool fullyObscured) = 0;
	
	virtual Bool isVisible() const { return true; }	///< for limiting tree sway, etc to visible objects

	virtual void reactToTransformChange(const Matrix3D* oldMtx, const Coord3D* oldPos, Real oldAngle) = 0;
	virtual void reactToGeometryChange() = 0;
	
	virtual Bool isLaser() const { return false; }

	// interface acquisition
	virtual ObjectDrawInterface* getObjectDrawInterface() { return NULL; }
	virtual const ObjectDrawInterface* getObjectDrawInterface() const { return NULL; }

	virtual DebrisDrawInterface* getDebrisDrawInterface() { return NULL; }
	virtual const DebrisDrawInterface* getDebrisDrawInterface() const { return NULL; }

	virtual TracerDrawInterface* getTracerDrawInterface() { return NULL; }
	virtual const TracerDrawInterface* getTracerDrawInterface() const { return NULL; }

	virtual RopeDrawInterface* getRopeDrawInterface() { return NULL; }
	virtual const RopeDrawInterface* getRopeDrawInterface() const { return NULL; }

	virtual LaserDrawInterface* getLaserDrawInterface() { return NULL; }
	virtual const LaserDrawInterface* getLaserDrawInterface() const { return NULL; }

};
inline DrawModule::DrawModule( Thing *thing, const ModuleData* moduleData ) : DrawableModule( thing, moduleData ) { }
inline DrawModule::~DrawModule() { }
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** VARIOUS MODULE INTERFACES */
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class DebrisDrawInterface
{
public:
	virtual void setModelName(AsciiString name, Color color, ShadowType t) = 0;
	virtual void setAnimNames(AsciiString initial, AsciiString flying, AsciiString final, const FXList* finalFX) = 0;
};

//-------------------------------------------------------------------------------------------------
class TracerDrawInterface
{
public:
	virtual void setTracerParms(Real speed, Real length, Real width, const RGBColor& color, Real initialOpacity) = 0;
};

//-------------------------------------------------------------------------------------------------
class RopeDrawInterface
{
public:
	virtual void initRopeParms(Real length, Real width, const RGBColor& color, Real wobbleLen, Real wobbleAmp, Real wobbleRate) = 0;
	virtual void setRopeCurLen(Real length) = 0;
	virtual void setRopeSpeed(Real curSpeed, Real maxSpeed, Real accel) = 0;
};

class LaserDrawInterface
{
public:
	virtual Real getLaserTemplateWidth() const = 0;
};

//-------------------------------------------------------------------------------------------------
class ObjectDrawInterface
{
public:

	// this method must ONLY be called from the client, NEVER From the logic, not even indirectly.
	virtual Bool clientOnly_getRenderObjInfo(Coord3D* pos, Real* boundingSphereRadius, Matrix3D* transform) const = 0;

	// (gth) C&C3 adding these accessors to render object properties
	virtual Bool clientOnly_getRenderObjBoundBox(OBBoxClass * boundbox) const = 0;
	virtual Bool clientOnly_getRenderObjBoneTransform(const AsciiString & boneName,Matrix3D * set_tm) const = 0;
	/**
		Find the bone(s) with the given name and return their positions and/or transforms in the given arrays.
		We look for a bone named "boneNamePrefixQQ", where QQ is 01, 02, 03, etc, starting at the
		value of "startIndex". Want to look for just a specific boneName with no numeric suffix?
		just pass zero (0) for startIndex. (no, we never look for "boneNamePrefix00".) 
		We copy up to 'maxBones' into the array(s), and return the total count found. 

		NOTE: this returns the positions and transform for the "ideal" model... that is,
		at its default rotation and scale, located at (0,0,0). You'll have to concatenate
		an Object's position and transform onto these in order to move 'em into "world space"!

		NOTE: this isn't very fast. Please call it sparingly and cache the result.
	*/
	virtual Int getPristineBonePositionsForConditionState(const ModelConditionFlags& condition, const char* boneNamePrefix, Int startIndex, Coord3D* positions, Matrix3D* transforms, Int maxBones) const = 0;
	virtual Int getCurrentBonePositions(const char* boneNamePrefix, Int startIndex, Coord3D* positions, Matrix3D* transforms, Int maxBones) const = 0;
	virtual Bool getCurrentWorldspaceClientBonePositions(const char* boneName, Matrix3D& transform) const = 0;
	virtual Bool getProjectileLaunchOffset(const ModelConditionFlags& condition, WeaponSlotType wslot, Int specificBarrelToUse, Matrix3D* launchPos, WhichTurretType tur, Coord3D* turretRotPos, Coord3D* turretPitchPos) const = 0;
	virtual void updateProjectileClipStatus( UnsignedInt shotsRemaining, UnsignedInt maxShots, WeaponSlotType slot ) = 0; ///< This will do the show/hide work if ProjectileBoneFeedbackEnabled is set.
	virtual void updateDrawModuleSupplyStatus( Int maxSupply, Int currentSupply ) = 0; ///< This will do visual feedback on Supplies carried
	virtual void notifyDrawModuleDependencyCleared( ) = 0; ///< if you were waiting for something before you drew, it's ready now

	virtual void setHidden(Bool h) = 0;
	virtual void replaceModelConditionState(const ModelConditionFlags& a) = 0;
	virtual void replaceIndicatorColor(Color color) = 0;
	virtual Bool handleWeaponFireFX(WeaponSlotType wslot, Int specificBarrelToUse, const FXList* fxl, Real weaponSpeed, const Coord3D* victimPos, Real damageRadius) = 0;
	virtual Int getBarrelCount(WeaponSlotType wslot) const = 0;

	virtual void setSelectable(Bool selectable) = 0;

	/**
		This call says, "I want the current animation (if any) to take n frames to complete a single cycle". 
		If it's a looping anim, each loop will take n frames. someday, we may want to add the option to insert
		"pad" frames at the start and/or end, but for now, we always just "stretch" the animation to fit.
		Note that you must call this AFTER setting the condition codes.
	*/
	virtual void setAnimationLoopDuration(UnsignedInt numFrames) = 0;

	/**
		similar to the above, but assumes that the current state is a "ONCE",
		and is smart about transition states... if there is a transition state 
		"inbetween", it is included in the completion time.
	*/
	virtual void setAnimationCompletionTime(UnsignedInt numFrames) = 0;
	virtual Bool updateBonesForClientParticleSystems( void ) = 0;///< this will reposition particle systems on the fly ML

	//Kris: Manually set a drawable's current animation to specific frame.
	virtual void setAnimationFrame( int frame ) = 0;
	
	/**
	  This call is used to pause or resume an animation.
	*/
	virtual void setPauseAnimation(Bool pauseAnim) = 0;

	virtual void updateSubObjects() = 0;
	virtual void showSubObject( const AsciiString& name, Bool show ) = 0;

	/**
		This call asks, "In the current animation (if any) how far along are you, from 0.0f to 1.0f". 
	*/
#ifdef ALLOW_ANIM_INQUIRIES
// srj sez: not sure if this is a good idea, for net sync reasons...
	virtual Real getAnimationScrubScalar( void ) const { return 0.0f;};
#endif
};

//-------------------------------------------------------------------------------------------------

class RenderCost
{
public:
	RenderCost(void) { clear(); }
	~RenderCost(void) { }

	void	clear(void) { m_drawCallCount = m_sortedMeshCount = m_boneCount = m_skinMeshCount = m_shadowDrawCount = 0; }
	void	addDrawCalls(int count) { m_drawCallCount += count; }
	void	addSortedMeshes(int count) { m_sortedMeshCount += count; }
	void	addSkinMeshes(int count) { m_skinMeshCount += count; }
	void	addBones(int count) { m_boneCount += count; }
	void	addShadowDrawCalls(int count) { m_shadowDrawCount += count; }

	int		getDrawCallCount(void) { return m_drawCallCount; }
	int		getSortedMeshCount(void) { return m_sortedMeshCount; }
	int		getSkinMeshCount(void) { return m_skinMeshCount; }
	int		getBoneCount(void) { return m_boneCount; }
	int		getShadowDrawCount(void) { return m_shadowDrawCount; }

protected:

	int		m_drawCallCount;
	int		m_sortedMeshCount;
	int		m_skinMeshCount;
	int		m_boneCount;
	int		m_shadowDrawCount;
};


#endif // __DRAWMODULE_H_

