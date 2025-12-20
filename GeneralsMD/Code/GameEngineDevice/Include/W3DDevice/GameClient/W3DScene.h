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

// FILE: W3DScene.h ///////////////////////////////////////////////////////////
//
// Scene manger for display using W3DDispaly.  A scene manager can customize
// the rendering process, culling, material passes ...
//
// Author: Colin Day, April 2001
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "WW3D2/scene.h"
#include "WW3D2/rinfo.h"
#include "WW3D2/coltest.h"
#include "WW3D2/lightenvironment.h"

///////////////////////////////////////////////////////////////////////////////
// PROTOTYPES /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class W3DDynamicLight;
class LightClass;
class Drawable;
enum CustomScenePassModes CPP_11(: Int);
class MaterialPassClass;
class W3DShroudMaterialPassClass;
class W3DMaskMaterialPassClass;
//-----------------------------------------------------------------------------
// RTS3DScene
//-----------------------------------------------------------------------------
/** Scene management for 3D RTS game */
//-----------------------------------------------------------------------------
class RTS3DScene : public SimpleSceneClass, public SubsystemInterface
{

public:

	RTS3DScene();  ///< RTSScene constructor
	~RTS3DScene();  ///< RTSScene destructor

	/// ray picking against objects in scene
	Bool castRay(RayCollisionTestClass & raytest, Bool testAll, Int collisionType);

	/// customizable renderer for the RTS3DScene
	virtual void	Customized_Render( RenderInfoClass &rinfo );
	virtual void	Visibility_Check(CameraClass * camera);
	virtual void  Render(RenderInfoClass & rinfo);

	void setCustomPassMode (CustomScenePassModes mode) {m_customPassMode = mode;}
	CustomScenePassModes getCustomPassMode (void)	{return m_customPassMode;}

	void Flush(RenderInfoClass & rinfo);	//draw queued up models.
	/// Drawing control method
	void drawTerrainOnly(Bool draw) {m_drawTerrainOnly = draw;};

	/// Drawing control method
	void renderSpecificDrawables(RenderInfoClass &rinfo, Int numDrawables, Drawable **theDrawables) ;

	/// Lighting methods
	void addDynamicLight(W3DDynamicLight * obj);
	void removeDynamicLight(W3DDynamicLight * obj);
	RefRenderObjListIterator *createLightsIterator(void);
	void destroyLightsIterator(RefRenderObjListIterator * it);
	RefRenderObjListClass *getDynamicLights(void) {return &m_dynamicLightList;};
	W3DDynamicLight *getADynamicLight(void);
	void setGlobalLight(LightClass *pLight,Int lightIndex=0);
	LightEnvironmentClass &getDefaultLightEnv(void) {return m_defaultLightEnv;}

	void init() {}
	void update() {}
	void draw();
	void reset(){}
	void doRender(CameraClass * cam);

protected:
	void renderOneObject(RenderInfoClass &rinfo, RenderObjClass *robj, Int localPlayerIndex);
	void updateFixedLightEnvironments(RenderInfoClass & rinfo);
	void flushTranslucentObjects(RenderInfoClass & rinfo);
	void flushOccludedObjects(RenderInfoClass & rinfo);
	void flagOccludedObjects(CameraClass * camera);
	void flushOccludedObjectsIntoStencil(RenderInfoClass & rinfo);
	void updatePlayerColorPasses(void);

protected:
	RefRenderObjListClass	m_dynamicLightList;
	Bool									m_drawTerrainOnly;
	LightClass						*m_globalLight[LightEnvironmentClass::MAX_LIGHTS];				///< The global directional light (sun, moon) Applies to objects.
	LightClass						*m_scratchLight; ///< a workspace for copying global lights and modifying // MLorenzen
	Vector3 m_infantryAmbient;	///<scene ambient modified to make infantry easier to see
	LightClass						*m_infantryLight[LightEnvironmentClass::MAX_LIGHTS];	///< The global direction light modified to make infantry easier to see.
	Int m_numGlobalLights;			///<number of global lights
	LightEnvironmentClass	m_defaultLightEnv;		///<default light environment applied to objects without custom/dynamic lighting.
	LightEnvironmentClass	m_foggedLightEnv;		///<default light environment applied to objects without custom/dynamic lighting.

	W3DShroudMaterialPassClass	*m_shroudMaterialPass;	///< Custom render pass which applies shrouds to objects
	W3DMaskMaterialPassClass *m_maskMaterialPass;			///< Custom render pass applied to entire scene used to mask out pixels.
	MaterialPassClass *m_heatVisionMaterialPass;			///< Custom render passed applied on top of objects with heatvision effect.
	MaterialPassClass *m_heatVisionOnlyPass;					///< Custom render pass applied in place of regular pass on objects with heat vision effect.
	MaterialPassClass *m_frenzyMaterialPass;					///< Custom render pass applied in place of regular pass on objects with FRENZY effect.
	///Custom rendering passes for each possible player color on the map
	MaterialPassClass *m_occludedMaterialPass[MAX_PLAYER_COUNT];
	CustomScenePassModes m_customPassMode;					///< flag used to force a non-standard rendering of scene.
	Int m_translucentObjectsCount;	///< number of translucent objects to render this frame.
	RenderObjClass **m_translucentObjectsBuffer;	///< queue of current frame's translucent objects.
	Int m_occludedObjectsCount;	///<number of objects in current frame that need special rendering because occluded.
	RenderObjClass **m_potentialOccluders;	///<objects which may block other objects from being visible
	RenderObjClass **m_potentialOccludees;	///<objects which may be blocked from visibility by other objects.
	RenderObjClass **m_nonOccludersOrOccludees;	///<objects which are neither bockers or blockees (small rocks, shrubs, etc.).
	Int m_numPotentialOccluders;
	Int m_numPotentialOccludees;
	Int m_numNonOccluderOrOccludee;

	CameraClass *m_camera;
};

//-----------------------------------------------------------------------------
// RTS2DScene
//-----------------------------------------------------------------------------
/** Scene management for 2D overlay on top of 3D scene */
//-----------------------------------------------------------------------------
class RTS2DScene : public SimpleSceneClass, public SubsystemInterface
{
public:

	RTS2DScene();
	~RTS2DScene();

	/// customizable renderer for the RTS2DScene
	virtual void Customized_Render( RenderInfoClass &rinfo );
	void init() {}
	void update() {}
	void draw();
	void reset(){}
	void doRender(CameraClass * cam);

protected:

	RenderObjClass *m_status;
	CameraClass *m_camera;
};

//-----------------------------------------------------------------------------
// RTS3DInterfaceScene
//-----------------------------------------------------------------------------
/** Scene management for 3D interface overlay on top of 3D scene */
//-----------------------------------------------------------------------------
class RTS3DInterfaceScene : public SimpleSceneClass
{
public:

	RTS3DInterfaceScene();
	~RTS3DInterfaceScene();

	/// customizable renderer for the RTS3DInterfaceScene
	virtual void Customized_Render( RenderInfoClass &rinfo );
};
