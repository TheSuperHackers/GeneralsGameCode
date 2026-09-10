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

#include <memory>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "W3DDevice/GameClient/W3DSceneClass.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
import Graphics.RHI;
import Graphics.Scene.Lighting.Local;
#include "W3DDevice/GameClient/WaterReflectionRenderer.h"

///////////////////////////////////////////////////////////////////////////////
// PROTOTYPES /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class W3DDynamicLight;
class W3DLight;
class Drawable;
enum CustomScenePassModes CPP_11(: Int);
//-----------------------------------------------------------------------------
// RTS3DScene
//-----------------------------------------------------------------------------
/** Scene management for 3D RTS game */
//-----------------------------------------------------------------------------
class RTS3DScene : public W3DSimpleScene, public SubsystemInterface,
	public WaterReflectionRenderer
{

public:

	RTS3DScene();  ///< RTSScene constructor
	virtual ~RTS3DScene() override;  ///< RTSScene destructor

	/// ray picking against objects in scene
	Bool castRay(W3DRayCastQuery & raytest, Bool testAll, Int collisionType);

	/// customizable renderer for the RTS3DScene
	virtual void	Customized_Render( W3DRenderContext &rinfo ) override;
	virtual void	Visibility_Check(W3DCamera * camera) override;
	virtual void  Render(W3DRenderContext & rinfo) override;

	void setCustomPassMode (CustomScenePassModes mode) {m_customPassMode = mode;}
	CustomScenePassModes getCustomPassMode ()	{return m_customPassMode;}

	void Flush(W3DRenderContext & rinfo);	//draw queued up models.
	void Render_Water_Reflection(W3DCamera *camera,
		const Graphics::RHIViewport &viewport) override;
	/// Drawing control method
	void drawTerrainOnly(Bool draw) {m_drawTerrainOnly = draw;};

	/// Drawing control method
	void renderSpecificDrawables(W3DRenderContext &rinfo, Int numDrawables, Drawable **theDrawables) ;

	/// Lighting methods
	void addDynamicLight(W3DDynamicLight * obj);
	void removeDynamicLight(W3DDynamicLight * obj);
	Graphics::SceneObjectList<W3DRenderObject>::Cursor *createLightsIterator();
	void destroyLightsIterator(Graphics::SceneObjectList<W3DRenderObject>::Cursor * it);
	Graphics::SceneObjectList<W3DRenderObject> *getDynamicLights() {return &m_dynamicLightList;};
	W3DDynamicLight *getADynamicLight();
	void setGlobalLight(W3DLight *pLight,Int lightIndex=0);
	Graphics::LocalLighting &getDefaultLightEnv() {return m_defaultLightEnv;}

	virtual void init() override {}
	virtual void update() override {}
	virtual void draw() override;
	virtual void reset() override {}
	void doRender(W3DCamera * cam);

protected:
	void renderOneObject(W3DRenderContext &rinfo, W3DRenderObject *robj, Int localPlayerIndex);
	void updateFixedLightEnvironments(W3DRenderContext & rinfo);
	void flushTranslucentObjects(W3DRenderContext & rinfo);
	void flushOccludedObjects(W3DRenderContext & rinfo);
	void flagOccludedObjects(W3DCamera * camera);
	void flushOccludedObjectsIntoStencil(W3DRenderContext & rinfo);
	void updatePlayerColorPasses();

protected:
	Graphics::SceneObjectList<W3DRenderObject>	m_dynamicLightList;
	Bool									m_drawTerrainOnly;
	W3DLight						*m_globalLight[Graphics::Material_Light_Count];				///< The global directional light (sun, moon) Applies to objects.
	W3DLight						*m_scratchLight; ///< a workspace for copying global lights and modifying // MLorenzen
	Vector3 m_infantryAmbient;	///<scene ambient modified to make infantry easier to see
	W3DLight						*m_infantryLight[Graphics::Material_Light_Count];	///< The global direction light modified to make infantry easier to see.
	Int m_numGlobalLights;			///<number of global lights
	Graphics::LocalLighting	m_defaultLightEnv;		///<default light environment applied to objects without custom/dynamic lighting.
	Graphics::LocalLighting	m_foggedLightEnv;		///<default light environment applied to objects without custom/dynamic lighting.

	std::shared_ptr<NativeMaterialPass>	m_shroudMaterialPass;	///< Custom render pass which applies shrouds to objects
	std::shared_ptr<NativeMaterialPass> m_maskMaterialPass;			///< Custom render pass applied to entire scene used to mask out pixels.
	std::shared_ptr<NativeMaterialPass> m_heatVisionMaterialPass;			///< Custom render passed applied on top of objects with heatvision effect.
	std::shared_ptr<NativeMaterialPass> m_heatVisionOnlyPass;					///< Custom render pass applied in place of regular pass on objects with heat vision effect.
	std::shared_ptr<NativeMaterialPass> m_frenzyMaterialPass;					///< Custom render pass applied in place of regular pass on objects with FRENZY effect.
	///Custom rendering passes for each possible player color on the map
	std::shared_ptr<NativeMaterialPass> m_occludedMaterialPass[MAX_PLAYER_COUNT];
	CustomScenePassModes m_customPassMode;					///< flag used to force a non-standard rendering of scene.
	Int m_translucentObjectsCount;	///< number of translucent objects to render this frame.
	W3DRenderObject **m_translucentObjectsBuffer;	///< queue of current frame's translucent objects.
	Int m_occludedObjectsCount;	///<number of objects in current frame that need special rendering because occluded.
	W3DRenderObject **m_potentialOccluders;	///<objects which may block other objects from being visible
	W3DRenderObject **m_potentialOccludees;	///<objects which may be blocked from visibility by other objects.
	W3DRenderObject **m_nonOccludersOrOccludees;	///<objects which are neither bockers or blockees (small rocks, shrubs, etc.).
	Int m_numPotentialOccluders;
	Int m_numPotentialOccludees;
	Int m_numNonOccluderOrOccludee;

	W3DCamera *m_camera;
};

//-----------------------------------------------------------------------------
// RTS2DScene
//-----------------------------------------------------------------------------
/** Scene management for 2D overlay on top of 3D scene */
//-----------------------------------------------------------------------------
class RTS2DScene : public W3DSimpleScene, public SubsystemInterface
{
public:

	RTS2DScene();
	virtual ~RTS2DScene() override;

	/// customizable renderer for the RTS2DScene
	virtual void Customized_Render( W3DRenderContext &rinfo ) override;
	virtual void init() override {}
	virtual void update() override {}
	virtual void draw() override;
	virtual void reset() override {}
	void doRender(W3DCamera * cam);

protected:

	W3DRenderObject *m_status;
	W3DCamera *m_camera;
};

//-----------------------------------------------------------------------------
// RTS3DInterfaceScene
//-----------------------------------------------------------------------------
/** Scene management for 3D interface overlay on top of 3D scene */
//-----------------------------------------------------------------------------
class RTS3DInterfaceScene : public W3DSimpleScene
{
public:

	RTS3DInterfaceScene();
	virtual ~RTS3DInterfaceScene() override;

	/// customizable renderer for the RTS3DInterfaceScene
	virtual void Customized_Render( W3DRenderContext &rinfo ) override;
};
