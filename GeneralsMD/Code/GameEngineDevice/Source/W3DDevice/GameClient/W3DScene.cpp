import Graphics.Frame.RenderSettings;
import Graphics.Scene.OrderedDraws;
#include "W3DDevice/GameClient/W3DRenderServices.h"
#include "W3DDevice/GameClient/W3DMeshDrawing.h"
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

// FILE: W3DScene.cpp /////////////////////////////////////////////////////////
//
// Scene manger for display using W3DDispaly.  A scene manager can customize
// the rendering process, culling, material passes ...
//
// Author: Colin Day, April 2001
//
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
import Graphics.Frame.AttachmentBindings;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Props.Submission;
#include <stdlib.h>
#include "rts/profile.h"
#include "../../../../../../engine/graphics/profiling/Tracy.h"
#include "W3DDevice/GameClient/W3DObjectGraphics.h"
import Assets.Math;
import Graphics.Frame.Runtime;
import Graphics.Scene.Shadows.StencilVolumes;

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "Common/GameUtility.h"
#include "Common/GlobalData.h"
#include "Common/PerfTimer.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameLogic/Object.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/Drawable.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Color.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DSceneQueryMask.h"
#include "W3DDevice/GameClient/W3DLight.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DDirectionalShadows.h"
#include "W3DDevice/GameClient/W3DStatusCircle.h"
#include "W3DDevice/GameClient/W3DCustomScene.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"


import Graphics.Materials.MeshMaterial;
import Graphics.Materials.ProceduralPass;
import Graphics.Materials.State;

import Graphics.Diagnostics.Render;

///////////////////////////////////////////////////////////////////////////////
// DEFINITIONS ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///@todo: Remove these globals since we no longer need W3D to call them for us.
extern void PrepareShadows();
extern void DoTrees(W3DRenderContext & rinfo);
extern void DoShadows(W3DRenderContext & rinfo, Bool stencilPass);
extern void DoParticles(W3DRenderContext & rinfo);

// No texturing, no zbuffer reading/writing, primary gradient, no
// blending, no fogging - mostly for use in solid-colored opaque objects.
#define SC_PLAYER_COLOR ( Graphics::MaterialState::Make_Bits(Graphics::MaterialState::PASS_ALWAYS, Graphics::MaterialState::DEPTH_WRITE_DISABLE, Graphics::MaterialState::COLOR_WRITE_ENABLE, \
	Graphics::MaterialState::SRCBLEND_ONE, Graphics::MaterialState::DSTBLEND_ZERO, Graphics::MaterialState::FOG_DISABLE, Graphics::MaterialState::GRADIENT_MODULATE, Graphics::MaterialState::SECONDARY_GRADIENT_DISABLE, \
	Graphics::MaterialState::TEXTURING_DISABLE, Graphics::MaterialState::ALPHATEST_DISABLE, Graphics::MaterialState::CULL_MODE_ENABLE, \
	Graphics::MaterialState::DETAILCOLOR_DISABLE, Graphics::MaterialState::DETAILALPHA_DISABLE) )
static Graphics::MaterialState PlayerColorShader(SC_PLAYER_COLOR);

namespace
{
Graphics::MaterialLightSource Get_Light_Source(const W3DLight &light)
{
	Graphics::MaterialLightSource source;
	light.Get_Light_Description(source);
	return source;
}
}

//=============================================================================
// RTS3DScene::RTS3DScene
//=============================================================================
/** */
//=============================================================================
RTS3DScene::RTS3DScene()
{
	setName("RTS3DScene");
	m_drawTerrainOnly = false;
	m_numGlobalLights=0;
	Int i=0;
	for (; i<Graphics::Material_Light_Count; i++)
	{
		m_globalLight[i]=nullptr;
		m_infantryLight[i]=NEW_REF( W3DLight, (W3DLight::DIRECTIONAL) );
	}

	m_scratchLight = NEW_REF( W3DLight, (W3DLight::DIRECTIONAL) );
//	REF_PTR_SET(m_globalLight[lightIndex], pLight);

#if ENABLE_CONFIGURABLE_SHROUD
	if (TheGlobalData->m_shroudOn)
		m_shroudMaterialPass = Create_W3D_Shroud_Material_Pass();
	else
		m_shroudMaterialPass = nullptr;
#else
	m_shroudMaterialPass = Create_W3D_Shroud_Material_Pass();
#endif

	m_maskMaterialPass = Create_W3D_Mask_Material_Pass();
	m_customPassMode = SCENE_PASS_DEFAULT;

	m_heatVisionMaterialPass = std::make_shared<NativeMaterialPass>();
	m_heatVisionOnlyPass = std::make_shared<NativeMaterialPass>();
	auto heatVisionMtl = std::make_shared<Graphics::MeshMaterial>();
	heatVisionMtl->parameters.lighting = true;
	heatVisionMtl->parameters.ambient = {0,0,0};
	heatVisionMtl->parameters.diffuse = {0.02f,0.01f,0.00f};
	heatVisionMtl->parameters.emissive = {0.5f,0.2f,0.0f};
	m_heatVisionMaterialPass->material = heatVisionMtl;
	Graphics::MaterialState heatVisionShader=Graphics::MaterialState::AdditiveSolid();
	heatVisionShader.Set_Depth_Compare(Graphics::MaterialState::PASS_EQUAL);
	heatVisionShader.Enable_Fog_For_Blend();
	m_heatVisionMaterialPass->shader = heatVisionShader;
	heatVisionShader.Set_Depth_Compare(Graphics::MaterialState::PASS_LEQUAL);
	heatVisionShader.Set_Depth_Mask(Graphics::MaterialState::DEPTH_WRITE_DISABLE);
	heatVisionShader.Enable_Fog_For_Blend();
	m_heatVisionOnlyPass->material = heatVisionMtl;
	m_heatVisionOnlyPass->shader = heatVisionShader;




	//Allocate memory to hold queue of visible render objects that need to be drawn last
	//because they are forced translucent.
	m_translucentObjectsCount = 0;
	if (TheGlobalData->m_maxVisibleTranslucentObjects > 0)
		m_translucentObjectsBuffer = NEW W3DRenderObject* [TheGlobalData->m_maxVisibleTranslucentObjects];
	else
		m_translucentObjectsBuffer = nullptr;

	m_numPotentialOccluders=0;
	m_numPotentialOccludees=0;
	m_numNonOccluderOrOccludee=0;
	m_occludedObjectsCount=0;

	if (TheGlobalData->m_maxVisibleOccluderObjects > 0)
		m_potentialOccluders = NEW W3DRenderObject* [TheGlobalData->m_maxVisibleOccluderObjects];
	else
		m_potentialOccluders = nullptr;

	if (TheGlobalData->m_maxVisibleOccludeeObjects > 0)
		m_potentialOccludees = NEW W3DRenderObject* [TheGlobalData->m_maxVisibleOccludeeObjects];
	else
		m_potentialOccludees = nullptr;

	if (TheGlobalData->m_maxVisibleNonOccluderOrOccludeeObjects > 0)
		m_nonOccludersOrOccludees = NEW W3DRenderObject* [TheGlobalData->m_maxVisibleNonOccluderOrOccludeeObjects];
	else
		m_nonOccludersOrOccludees = nullptr;

	//Modify the shader to make occlusion transparent
	Graphics::MaterialState shader = PlayerColorShader;
	shader.Set_Src_Blend_Func(Graphics::MaterialState::SRCBLEND_SRC_ALPHA);
	shader.Set_Dst_Blend_Func(Graphics::MaterialState::DSTBLEND_ONE_MINUS_SRC_ALPHA);

#ifdef USE_NON_STENCIL_OCCLUSION
	for (i=0; i<MAX_PLAYER_COUNT; i++)
	{
		m_occludedMaterialPass[i]=std::make_shared<NativeMaterialPass>();
		auto vmtl = std::make_shared<Graphics::MeshMaterial>();
		vmtl->parameters.lighting = true;
		vmtl->parameters.ambient = {0,0,0};	//we're only using emissive so kill all other lights.
		vmtl->parameters.diffuse = {0,0,0};
		m_occludedMaterialPass[i]->material = vmtl;
		shader.Enable_Fog_For_Blend();
		m_occludedMaterialPass[i]->shader = shader;
	}
#else
	for (i=0; i<MAX_PLAYER_COUNT; i++)
		m_occludedMaterialPass[i]=nullptr;
#endif

}

//=============================================================================
// RTS3DScene::~RTS3DScene
//=============================================================================
/** */
//=============================================================================
RTS3DScene::~RTS3DScene()
{
	Int i=0;
	for (; i<Graphics::Material_Light_Count; i++)
	{
		REF_PTR_RELEASE(m_globalLight[i]);
		REF_PTR_RELEASE(m_infantryLight[i]);
	}

	REF_PTR_RELEASE(m_scratchLight);

	m_shroudMaterialPass.reset();

	m_maskMaterialPass.reset();

	m_heatVisionMaterialPass.reset();

	m_heatVisionOnlyPass.reset();

	delete [] m_translucentObjectsBuffer;
	delete [] m_nonOccludersOrOccludees;
	delete [] m_potentialOccludees;
	delete [] m_potentialOccluders;

	for (i=0; i<MAX_PLAYER_COUNT; i++)
	{
		m_occludedMaterialPass[i].reset();
	}
}


void	RTS3DScene::setGlobalLight(W3DLight *pLight, Int lightIndex)
{
	if (m_numGlobalLights < (lightIndex+1))
		m_numGlobalLights=(lightIndex+1);
	REF_PTR_SET(m_globalLight[lightIndex], pLight);
}

/**Find all objects which need to be drawn in a special way because they are occluded by other
objects.
@todo:  Need some kind of scene subdivision or find way to use Partition manger to speed up the ray
intersection tests.  Maybe truncate the ray to terrain length before using it?
*/
void RTS3DScene::flagOccludedObjects(W3DCamera * camera)
{
	Vector3 camPosition=camera->Get_Position();

	//Find which objects are actually occluded
	W3DRenderObject **occludee=m_potentialOccludees;
	LineSegClass lineseg;
	CastResultStruct result;
	Bool hit=FALSE;
	Vector3 newEndPoint;
	result.ComputeContactPoint=false;
	W3DRayCastQuery raytest(lineseg,&result,SCENE_QUERY_ALL,false,false);
	raytest.CollisionType=SCENE_QUERY_ALL;

	m_occludedObjectsCount=0;

	for (Int i=0; i<m_numPotentialOccludees; i++,occludee++)
	{
		raytest.Ray.Set(camPosition,(*occludee)->Get_Position());

		W3DRenderObject **occluder=m_potentialOccluders;

		//Check this object against all other possible blocking objects
		for (Int j=0; j<m_numPotentialOccluders; j++,occluder++)
		{
			// Do a quick ray-sphere test (Graphics Gems I,  p388)
			W3DRenderObject *robj=*occluder;

			const SphereClass *sphere = &robj->Get_Bounding_Sphere();

			// make a vector from the ray origin to the sphere center
			Vector3 sphere_vector(sphere->Center - raytest.Ray.Get_P0());

			// get the dot product between the sphere_vector and the ray vector
			Real Alpha = Vector3::Dot_Product(sphere_vector, raytest.Ray.Get_Dir());

			Real Beta = sphere->Radius * sphere->Radius - (Vector3::Dot_Product(sphere_vector, sphere_vector) - Alpha * Alpha);

			if(Beta < 0.0f)
				continue;	//no intersection

			//Do a more accurate test against object geometry
			if (robj->Cast_Ray(raytest))
			{
				//Found an object closer than last closest object
				//Adjust our results and refine search
				raytest.CollidedRenderObj = robj;
				hit=TRUE;
				//reset the result space for next test
				result.StartBad = false;
				result.Fraction = 1.0f;
				break;
			}
		}
		if (hit)
		{
			//occludee was blocked by something so flag it for custom rendering
			DrawableInfo *drawInfo=(DrawableInfo *)(*occludee)->Get_User_Data();
			drawInfo->m_flags |= DrawableInfo::ERF_IS_OCCLUDED;
			m_potentialOccludees[m_occludedObjectsCount++] = *occludee;
		}
	}
}

//=============================================================================
// RTS3DScene::castRay
//=============================================================================
/** Does a ray intersection test against objects in our scene.
	By default, it only tests objects determined to be visible to the user.
	Setting testAll forces it to test all objects in the scene.
	CollisionType is used as a mask to ignore certain types of objects.
 */
//=============================================================================
Bool RTS3DScene::castRay(W3DRayCastQuery & raytest, Bool testAll, Int collisionType)
{
// this shouldn't be necessary here, and would be an undesirable performance hit.
// if you ever add or modify code here, it MIGHT become necessary... so do so with caution. (srj)
//#ifdef DIRTY_CONDITION_FLAGS
//	StDrawableDirtyStuffLocker lockDirtyStuff;
//#endif

	//temporary results for each object tested
	CastResultStruct result;
	W3DRayCastQuery tempRayTest(raytest.Ray,&result);
	Vector3 newEndPoint;
	Bool hit=FALSE;

	tempRayTest.CollisionType = SCENE_QUERY_ALL;
	//check if a mesh is translucent before colliding with it. Skips headlights, etc.
	tempRayTest.CheckTranslucent = true;

	Graphics::SceneObjectList<W3DRenderObject>::Cursor it(&RenderList);

	// select the first object
	it.First();

	while (!it.Is_Done())
	{
		// get the render object
		W3DRenderObject * robj = it.Peek_Obj();
		it.Next();

		// only intersect if it was visible or if we must test all
		if(robj->Get_Collision_Type() & collisionType && (testAll || robj->Is_Really_Visible()))
		{
			// Do a quick ray-sphere test (Graphics Gems I,  p388)
			const SphereClass *sphere = &robj->Get_Bounding_Sphere();

			// make a vector from the ray origin to the sphere center
			Vector3 sphere_vector(sphere->Center - tempRayTest.Ray.Get_P0());

			// get the dot product between the sphere_vector and the ray vector
			Real Alpha = Vector3::Dot_Product(sphere_vector, tempRayTest.Ray.Get_Dir());

			Real Beta = sphere->Radius * sphere->Radius - (Vector3::Dot_Product(sphere_vector, sphere_vector) - Alpha * Alpha);

			if(Beta < 0.0f)
				continue;	//no intersection

			//Do a more accurate test against object geometry
			if (robj->Cast_Ray(tempRayTest))
			{
				//Found an object closer than last closest object
				//Adjust our results and refine search
				raytest.CollidedRenderObj = robj;
				hit=TRUE;
				//Refine search by making ray shorter
				tempRayTest.Ray.Compute_Point(tempRayTest.Result->Fraction,&newEndPoint);
				tempRayTest.Ray.Set(raytest.Ray.Get_P0(),newEndPoint);
				//intersection point is at the end of shortened ray, so adjust fraction to 1.0
				tempRayTest.Result->Fraction=1.0f;
			}
		}
	}

	//Store results of ray intersection test including a clipped ray that ends on object.
	raytest.Ray=tempRayTest.Ray;
	return hit;
}

//=============================================================================
// RTS3DScene::Visibility_Check
//=============================================================================
/** Custom visibility check method for the RTS3DScene, we can put optimized
  * culling methods in here */
//=============================================================================
void RTS3DScene::Visibility_Check(W3DCamera * camera)
{
    GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.Scene.Visibility");
#ifdef DIRTY_CONDITION_FLAGS
	StDrawableDirtyStuffLocker lockDirtyStuff;
#endif

	Graphics::SceneObjectList<W3DRenderObject>::Cursor it(&RenderList);
	DrawableInfo *drawInfo = nullptr;
	Drawable	*draw = nullptr;
	W3DRenderObject * robj;

	m_numPotentialOccluders=0;
	m_numPotentialOccludees=0;
	m_translucentObjectsCount=0;
	m_numNonOccluderOrOccludee=0;

	Int currentFrame = TheGameLogic ? TheGameLogic->getFrame() : 0;
	if (currentFrame <= TheGlobalData->m_defaultOcclusionDelay)
		currentFrame = TheGlobalData->m_defaultOcclusionDelay+1;	//make sure occlusion is enabled when game starts (frame 0).

	if (Get_W3D_Render_Services().Is_Reflection_Render_Pass())
	{
		//we are rendering reflections
		///@todo: Have better flag to detect reflection pass

		// Loop over all top-level RenderObjects in this scene. If the bounding sphere is not in front
		// of all the frustum planes, it is invisible.
		for (it.First(); !it.Is_Done(); it.Next()) {

			robj = it.Peek_Obj();

			draw=nullptr;
			drawInfo = (DrawableInfo *)robj->Get_User_Data();
			if (drawInfo)
				draw=drawInfo->m_drawable;

			if( draw )
			{
				if (robj->Is_Force_Visible()) {
					robj->Set_Visible(true);
				} else {
					robj->Set_Visible(draw->getDrawsInMirror() && !camera->Cull_Sphere(robj->Get_Bounding_Sphere()));
				}
			}
			else
			{
				//perform normal culling on non-drawables
				if (robj->Is_Force_Visible()) {
					robj->Set_Visible(true);
				} else {
					robj->Set_Visible(!camera->Cull_Sphere(robj->Get_Bounding_Sphere()));
				}
			}
		}
	}
	else
	{

		// Loop over all top-level RenderObjects in this scene. If the bounding sphere is not in front
		// of all the frustum planes, it is invisible.
		for (it.First(); !it.Is_Done(); it.Next()) {

			robj = it.Peek_Obj();

			if (robj->Is_Force_Visible()) {
				robj->Set_Visible(true);
			} else if (robj->Is_Hidden()) {
				robj->Set_Visible(false);
			} else {

				bool isVisible=!camera->Cull_Sphere(robj->Get_Bounding_Sphere());

				if (isVisible)
				{
					//need to keep track of occluders and occludees for subsequent code.
					drawInfo = (DrawableInfo *)robj->Get_User_Data();
					if (drawInfo && (draw=drawInfo->m_drawable) != nullptr)
					{
						if (draw->isDrawableEffectivelyHidden() || draw->getFullyObscuredByShroud())
						{
							isVisible = FALSE;
						  robj->Set_Visible(isVisible);
            }
						//assume normal rendering.
						drawInfo->m_flags = DrawableInfo::ERF_IS_NORMAL;	//clear any rendering flags that may be in effect.

            if ( ! isVisible )
              continue;

						if (draw->getEffectiveOpacity() != 1.0f && m_translucentObjectsCount < TheGlobalData->m_maxVisibleTranslucentObjects)
						{
							drawInfo->m_flags |= DrawableInfo::ERF_IS_TRANSLUCENT;
							m_translucentObjectsBuffer[m_translucentObjectsCount++] = robj;
						}
						if (TheGlobalData->m_enableBehindBuildingMarkers && TheGameLogic && TheGameLogic->getShowBehindBuildingMarkers())
						{
							const Bool isTranslucent = (drawInfo->m_flags & DrawableInfo::ERF_IS_TRANSLUCENT) != 0;
							//visible drawable. Check if it's either an occluder or occludee
							if (draw->isKindOf(KINDOF_STRUCTURE) && m_numPotentialOccluders < TheGlobalData->m_maxVisibleOccluderObjects)
							{
								//object which could occlude other objects that need to be visible.
								//Make sure this object is not translucent so it's not rendered twice (from m_potentialOccluders and m_translucentObjectsBuffer)
								if (!isTranslucent)
									m_potentialOccluders[m_numPotentialOccluders++]=robj;
								drawInfo->m_flags |= DrawableInfo::ERF_POTENTIAL_OCCLUDER;
							}
							else if (draw->getObject() &&
									(draw->isKindOf(KINDOF_SCORE) || draw->isKindOf(KINDOF_SCORE_CREATE) || draw->isKindOf(KINDOF_SCORE_DESTROY) || draw->isKindOf(KINDOF_MP_COUNT_FOR_VICTORY)) &&
									(draw->getObject()->getSafeOcclusionFrame()) <= currentFrame && m_numPotentialOccludees < TheGlobalData->m_maxVisibleOccludeeObjects)
							{
								//object which could be occluded but still needs to be visible.
								//We process translucent units twice (also in m_translucentObjectsBuffer) because we need to see them when occluded.
								m_potentialOccludees[m_numPotentialOccludees++]=robj;
								drawInfo->m_flags |= DrawableInfo::ERF_POTENTIAL_OCCLUDEE;
							}
							else if (drawInfo->m_flags == DrawableInfo::ERF_IS_NORMAL && m_numNonOccluderOrOccludee < TheGlobalData->m_maxVisibleNonOccluderOrOccludeeObjects)
							{
								//regular object with no custom effects but still needs to be delayed to get the occlusion feature to work correctly.
								//Make sure this object is not translucent so it's not rendered twice (from m_nonOccludersOrOccludees and m_translucentObjectsBuffer)
								if (!isTranslucent)
									m_nonOccludersOrOccludees[m_numNonOccluderOrOccludee++]=robj;
								drawInfo->m_flags |= DrawableInfo::ERF_IS_NON_OCCLUDER_OR_OCCLUDEE;
							}
						}
					}
				}

				robj->Set_Visible(isVisible);
			}

			///@todo: We're not using LOD yet so I disabled this code. MW
			// Also, should check how multiple passes (reflections) get along
			// with the LOD manager - we're rendering double the load it thinks we are.
			// Prepare visible objects for LOD:
			//	if (robj->Is_Really_Visible()) {
			//			robj->Prepare_LOD(*camera);
			//		}
		}
	}

	Traversal.Adopt_Visibility(*camera);
}

//============================================================================
// RTS3DScene::renderSingleDrawable
//=============================================================================
/** Renders a single drawable entity. */
//=============================================================================
void RTS3DScene::renderSpecificDrawables(W3DRenderContext &rinfo, Int numDrawable, Drawable **theDrawables)
{
    Graphics::PropBatchScope prop_batches(Graphics::Get_Prop_Submission(),Graphics::Get_Attachment_Bindings().Current().viewport);
#ifdef DIRTY_CONDITION_FLAGS
	StDrawableDirtyStuffLocker lockDirtyStuff;
#endif
	const Int localPlayerIndex = rts::getObservedOrLocalPlayerIndex_Safe();
	Graphics::SceneObjectList<W3DRenderObject>::Cursor it(&UpdateList);
	// loop through all render objects in the list:
	for (it.First(&RenderList); !it.Is_Done();)
	{
		W3DRenderObject *robj;
		// get the render object
		robj = it.Peek_Obj();

		it.Next();	//advance to next object in case this one gets deleted during renderOneObject().

		DrawableInfo *drawInfo = (DrawableInfo *)robj->Get_User_Data();
		Drawable *draw=nullptr;
		if (drawInfo)
			draw = drawInfo->m_drawable;
		if (!draw) continue;
		Bool match = false;
		for (Int i = 0; i<numDrawable; i++) {
			if (theDrawables[i] == draw) {
				match = true;
				break;
			}
		}
		if (match) {
			renderOneObject(rinfo, robj, localPlayerIndex);
		}
	}
}

//============================================================================
// RTS3DScene::renderOneObject
//=============================================================================
/** Renders a single drawable entity. */
//=============================================================================
void RTS3DScene::renderOneObject(W3DRenderContext &rinfo, W3DRenderObject *robj, Int localPlayerIndex)
{
    Flush_Before_W3D_Object_Draw(*robj);
    GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.Scene.RenderObject");
	Drawable *draw = nullptr;
	DrawableInfo *drawInfo = nullptr;
	Bool drawableHidden=FALSE;
	Object* obj = nullptr;
	ObjectShroudStatus ss=OBJECTSHROUD_INVALID;
	Bool doExtraMaterialPop=FALSE;
	Bool doExtraFlagsPop=FALSE;
	W3DLight **sceneLights=m_globalLight;

	if (robj->Class_ID() == W3DRenderObject::CLASSID_IMAGE3D	)
	{
		robj->Render(rinfo);	//notify decals system that this track is visible
		return;	//decals are not lit by this system yet so skip rest of lighting
	}

	Graphics::LocalLighting lightEnv;
	SphereClass sph = robj->Get_Bounding_Sphere();
	drawInfo = (DrawableInfo *)robj->Get_User_Data();
	if (drawInfo)
	{
		draw = drawInfo->m_drawable;
		//If we have a drawInfo but not drawable, we must be dealing with
		//a ghost object which is always fogged.
		if (!draw)
			ss = OBJECTSHROUD_FOGGED;
	}

	// all this ambient business no longer handles the tinting and flashing stuff,
	// but it does still light the drawable explicitly, and can be fudged like this
	// infantry test does, here...
	// the tint has been delegated to the getTint() stuff, below... MLorenzen
	Vector3 ambient = Get_Ambient_Light();
	if (draw && (drawableHidden=draw->isDrawableEffectivelyHidden()) != TRUE)
	{
#ifdef NOT_IN_USE
		const Vector3* drawAmbient = draw->getAmbientLight();
		if (drawAmbient)
			ambient.Add(ambient, *drawAmbient, &ambient);
#endif
		obj = draw->getObject();
		if (obj) {
			ss = obj->getShroudedStatus(localPlayerIndex);
			// For objects like planes, that pop out of the shroud, fire, then head back,
			// we keep drawing them for 2 seconds after they return to the fogged area,
			// so the player can see them and missiles chasing them.  jba.
			if (ss == OBJECTSHROUD_CLEAR) {
				draw->setShroudClearFrame(TheGameLogic->getFrame());
			}	else if (ss >= OBJECTSHROUD_FOGGED && draw->getShroudClearFrame() != InvalidShroudClearFrame) {
				UnsignedInt limit = 2*LOGICFRAMES_PER_SECOND;
				if (obj->isEffectivelyDead()) {
					limit += 3*LOGICFRAMES_PER_SECOND;
				}
				if (TheGameLogic->getFrame() < limit + draw->getShroudClearFrame()) {
					// It's been less than 2 seconds since we could see them clear, so keep showing them.
					ss = OBJECTSHROUD_PARTIAL_CLEAR;
				}
			}
 			if (!robj->Peek_Scene())
 				return;	//this object was removed by the getShroudedStatus() call.
		}
		else
		{
			//drawable with no object so no way to know if it's shrouded.
			ss = OBJECTSHROUD_CLEAR;	//assume not shrouded/fogged.
			//Check to see if there is another unrelated object which controls the shroud status
			//(Hack for prison camps which contain enemy prisoner drawables)
			if (drawInfo->m_shroudStatusObjectID != INVALID_ID)
			{
				Object *shroudObject=TheGameLogic->findObjectByID(drawInfo->m_shroudStatusObjectID);
				if (shroudObject && shroudObject->getShroudedStatus(localPlayerIndex) >= OBJECTSHROUD_FOGGED)
					ss = OBJECTSHROUD_SHROUDED;	//we will assume that drawables without objects are 'particle' like and therefore don't need drawing if fogged/shrouded.
			}
		}

		if (draw->isKindOf(KINDOF_INFANTRY))
		{
			//ambient = m_infantryAmbient;  //has no effect - see comment on m_infantryAmbient
			sceneLights = m_infantryLight;
		}

		lightEnv.Reset({(sph.Center).X,(sph.Center).Y,(sph.Center).Z}, {(ambient).X,(ambient).Y,(ambient).Z});


		// HANDLE THE SPECIAL DRAWABLE-LEVEL COLORING SETTINGS FIRST

		const Vector3 *tintColor = nullptr;
		const Vector3 *selectionColor = nullptr;

		tintColor			 = draw->getTintColor();
		selectionColor = draw->getSelectionColor();

		if ( tintColor || selectionColor )
		{
			Vector3 sumTint, temp, restore;

			sumTint.Set(0,0,0);

			if (tintColor)
				Vector3::Add(sumTint, *tintColor, &sumTint);
			if (selectionColor)
				Vector3::Add(sumTint, *selectionColor, &sumTint);

			for (Int globalLightIndex = 0; globalLightIndex < m_numGlobalLights; globalLightIndex++)
			{
				sceneLights[globalLightIndex]->Get_Diffuse( &temp );
				restore = temp;

				Vector3::Add(temp, sumTint, &temp);

				sceneLights[globalLightIndex]->Set_Diffuse( temp );
				lightEnv.Add(Get_Light_Source(*sceneLights[globalLightIndex]));
				sceneLights[globalLightIndex]->Set_Diffuse( restore );

			}

			temp.Set(lightEnv.ambient[0],lightEnv.ambient[1],lightEnv.ambient[2]);
			Vector3::Add(sumTint, temp, &temp );

			lightEnv.ambient = {temp.X,temp.Y,temp.Z};

		}
		else // no funny coloring going on, so just add the lights normally
		{
			for (Int globalLightIndex = 0; globalLightIndex < m_numGlobalLights; globalLightIndex++)
			{
				lightEnv.Add(Get_Light_Source(*sceneLights[globalLightIndex]));
			}
		}

		//Apply custom render pass for any drawables with heatvision enabled
		if (draw->getSecondMaterialPassOpacity() != 0 )
		{
			rinfo.pass_emissive_override = draw->getSecondMaterialPassOpacity();

      //if ( draw->testTintStatus( TINT_STATUS_FRENZY ) )
      //{
			//	rinfo.Push_Material_Pass(m_heatVisionMaterialPass);
      //}
			//else
      if (draw->getStealthLook() == STEALTHLOOK_VISIBLE_DETECTED )
			{
			  rinfo.pass_emissive_override = draw->getSecondMaterialPassOpacity();
				// THIS WILL EXPLICITLY SKIP THE FIRST PASS SO THAT HEATVISION ONLY WILL RENDER
				rinfo.Push_Override_Flags(Graphics::DrawOverride::AdditionalPassesOnly);
				rinfo.Push_Material_Pass(m_heatVisionOnlyPass);
        doExtraFlagsPop = TRUE;
			}
			else
			{
				//THIS CALLS FOR THE HEATVISION TO RENDER
			  rinfo.pass_emissive_override = draw->getSecondMaterialPassOpacity();
				rinfo.Push_Material_Pass(m_heatVisionMaterialPass);
			}

			doExtraMaterialPop = TRUE;
		}
	}
	else
	{
		//either no drawable or it is hidden
		if (drawableHidden)
			return;	//don't bother with anything else

		//Render object without a drawable.  Must be either some fluff/debug object or a ghostObject.
		if (ss == OBJECTSHROUD_FOGGED)
		{
			//Must be ghost object because we don't fog normal things.  Fogged objects always have a predefined
			//lighting environment applied which emulates the look of fog.
			rinfo.light_environment = &m_foggedLightEnv;
			robj->Render(rinfo);
			rinfo.light_environment = nullptr;
			return;
		}
		else
		{
			lightEnv.Reset({(sph.Center).X,(sph.Center).Y,(sph.Center).Z}, {(ambient).X,(ambient).Y,(ambient).Z});
			for (Int globalLightIndex = 0; globalLightIndex < m_numGlobalLights; globalLightIndex++)
				lightEnv.Add(Get_Light_Source(*m_globalLight[globalLightIndex]));
		}
	}

	if (!drawableHidden)
	{
		//standard scene lights
		Graphics::SceneObjectList<W3DRenderObject>::Cursor it2(&LightList);
		for (it2.First(); !it2.Is_Done(); it2.Next())
		{
			W3DLight *pLight = static_cast<W3DLight *>(it2.Peek_Obj());
			SphereClass lSph = pLight->Get_Bounding_Sphere();
			Bool cull = (pLight->Get_Type() == W3DLight::POINT && !Spheres_Intersect(sph, lSph));
			if (!cull) {
				lightEnv.Add(Get_Light_Source(*pLight));
			}
		}

    if( draw && draw->getReceivesDynamicLights() )
    {
		  // dynamic lights
		  Graphics::SceneObjectList<W3DRenderObject>::Cursor dynaLightIt(&m_dynamicLightList);
		  for (dynaLightIt.First(); !dynaLightIt.Is_Done(); dynaLightIt.Next())
		  {
			  W3DDynamicLight* pDyna = (W3DDynamicLight*)dynaLightIt.Peek_Obj();
			  if (!pDyna->isEnabled()) {
				  continue;
			  }
			  SphereClass lSph = pDyna->Get_Bounding_Sphere();
			  if (pDyna->Get_Type() == W3DLight::POINT && !Spheres_Intersect(sph, lSph)) {
				  continue;
			  }
			  lightEnv.Add(Get_Light_Source(*pDyna));
		  }
    }

		lightEnv.Finalize();
		rinfo.light_environment = &lightEnv;

		if (drawInfo)
		{
#if ENABLE_CONFIGURABLE_SHROUD
			if (!TheGlobalData->m_shroudOn)
				ss = OBJECTSHROUD_CLEAR;
#endif

			if (m_customPassMode == SCENE_PASS_DEFAULT)
			{
				if (ss <= OBJECTSHROUD_CLEAR)
				{
					robj->Render(rinfo);
				}
				else
				{
					rinfo.Push_Material_Pass(m_shroudMaterialPass);
					robj->Render(rinfo);
					rinfo.Pop_Material_Pass();
				}
			}
			else if (m_maskMaterialPass)
			{
				rinfo.Push_Material_Pass(m_maskMaterialPass);
				rinfo.Push_Override_Flags(Graphics::DrawOverride::AdditionalPassesOnly);
				robj->Render(rinfo);
				rinfo.Pop_Override_Flags();
				rinfo.Pop_Material_Pass();
			}
		}
		else
		{
			robj->Render(rinfo);
		}
	}

	rinfo.light_environment = nullptr;
	if (doExtraMaterialPop)	//check if there is an extra material on the stack from the added material effect.
		rinfo.Pop_Material_Pass();
	if (doExtraFlagsPop)
		rinfo.Pop_Override_Flags();	//flags used to disable base pass and only render custom heat vision pass.
}

//DECLARE_PERF_TIMER(translucentRender)

void RTS3DScene::Render_Water_Reflection(W3DCamera *camera,
	const Graphics::RHIViewport &viewport)
{
	if (camera == nullptr)
		return;

	Graphics::RHIViewport pass_viewport = viewport;
	Get_W3D_Render_Services().Render_Scene_Pass(this, camera, &pass_viewport);
}

/**Draw everything that was submitted from this scene*/
void RTS3DScene::Flush(W3DRenderContext & rinfo)
{
    PROFILER_SECTION_NAME("Graphics.Scene.Flush");
	// TheSuperHackers @bugfix Now always prepares shadows to guarantee correct state before doing any
	// shadow draw calls. Originally just drawing shadows for trees would not properly prepare shadows.
	PrepareShadows();

	//don't draw shadows in this mode because they interfere with destination alpha or are invisible (wireframe)
	if (m_customPassMode == SCENE_PASS_DEFAULT && Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE)
	{
		PROFILER_SECTION_NAME("Graphics.Scene.DecalShadows");
		DoShadows(rinfo, false);	//draw all non-stencil shadows (decals) since they fall under other objects.
	}

	{
		PROFILER_SECTION_NAME("Graphics.Scene.MaterialQueue");
		Graphics::Get_Prop_Submission().Flush_Materials();	//draw all non-translucent objects.
	}


    {
    Graphics::PropBatchScope prop_batches(Graphics::Get_Prop_Submission(),Graphics::Get_Attachment_Bindings().Current().viewport);
	//draw all non-translucent objects which were separated because they are hidden and need custom rendering.
#ifdef USE_NON_STENCIL_OCCLUSION
	flushOccludedObjects(rinfo);
#else
	if (Graphics::Get_Attachment_Bindings().Default().depth.Is_Valid())
		flushOccludedObjectsIntoStencil(rinfo);
#endif

    }

	// (gth) CNC3 Flush the shader meshes

	// Draw the trees last so they alpha blend onto everything correctly.
	DoTrees(rinfo);

	//don't draw shadows in this mode because they interfere with destination alpha
	if (m_customPassMode == SCENE_PASS_DEFAULT && Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE)
	{
		PROFILER_SECTION_NAME("Graphics.Scene.StencilShadows");
		DoShadows(rinfo, true);	//draw all stencil shadows
	}

	if (TheWaterRenderSystem != nullptr &&
		m_customPassMode != SCENE_PASS_ALPHA_MASK &&
		Get_Extra_Pass_Polygon_Mode() != EXTRA_PASS_CLEAR_LINE)
	{
		PROFILER_SECTION_NAME("Graphics.Scene.WaterDraw");
		TheWaterRenderSystem->Capture_Refraction_Texture();
		TheWaterRenderSystem->Render(rinfo);
	}

	{
		PROFILER_SECTION_NAME("Graphics.Scene.StaticQueue");
		Graphics::Get_Scene_Draw_Queue().Drain(&rinfo, [] { Graphics::Get_Prop_Submission().Flush_Materials(); });	//draw remaining static-sort submissions
	}

	if (m_customPassMode == SCENE_PASS_DEFAULT && Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE)
	{
		PROFILER_SECTION_NAME("Graphics.Scene.TransparentObjects");
		flushTranslucentObjects(rinfo);	//draw all translucent meshes which don't need per-polygon sorting.
	}

	{
		//USE_PERF_TIMER(translucentRender)

		//don't draw transparent in this mode because they interfere with destination alpha
		if (m_customPassMode == SCENE_PASS_DEFAULT && Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE)
			DoParticles(rinfo);	//queue up particles for rendering.

		Graphics::Get_Prop_Submission().Flush_Transparent();	//draw sorted translucent polygons like particles.

	}
}

/**Generate a predefined light environment(s) that will be applied to many objects.  Useful for things like totally fogged
objects and most generic map objects that are not lit by dynamic lights.*/
void RTS3DScene::updateFixedLightEnvironments(W3DRenderContext & rinfo)
{
	//Figure out how dimly lit fogged objects should be compared to fully lit.
	Real foggedLightFrac = (Real)TheGlobalData->m_fogAlpha/(Real)TheGlobalData->m_clearAlpha;
	Real infantryLightScale;
	if( TheGlobalData->m_scriptOverrideInfantryLightScale != -1.0f )
		infantryLightScale = TheGlobalData->m_scriptOverrideInfantryLightScale;
	else
		infantryLightScale = TheGlobalData->m_infantryLightScale[TheGlobalData->m_timeOfDay];

	//Generate the default light environment
	m_defaultLightEnv.Reset({0,0,0}, {(Get_Ambient_Light()).X,(Get_Ambient_Light()).Y,(Get_Ambient_Light()).Z});
	m_foggedLightEnv.Reset({0,0,0}, {(Get_Ambient_Light()*foggedLightFrac).X,(Get_Ambient_Light()*foggedLightFrac).Y,(Get_Ambient_Light()*foggedLightFrac).Z});

	Vector3 oldDiffuse, oldAmbient;
	for (Int globalLightIndex = 0; globalLightIndex < m_numGlobalLights; globalLightIndex++)
	{
		m_defaultLightEnv.Add(Get_Light_Source(*m_globalLight[globalLightIndex]));
		//copy default lighting for infantry so we can tweak it.
		*m_infantryLight[globalLightIndex]=*m_globalLight[globalLightIndex];
		m_infantryLight[globalLightIndex]->Set_Transform(m_globalLight[globalLightIndex]->Get_Transform());

		m_globalLight[globalLightIndex]->Get_Diffuse(&oldDiffuse);
		m_globalLight[globalLightIndex]->Get_Ambient(&oldAmbient);
    oldDiffuse *= infantryLightScale;
    oldAmbient *= infantryLightScale;
    static Vector3 id (1.0f, 1.0f, 1.0f);
    oldDiffuse.Cap_Absolute_To(id);
    oldAmbient.Cap_Absolute_To(id);
		m_infantryLight[globalLightIndex]->Set_Ambient(oldAmbient);//CLAMPED
		m_infantryLight[globalLightIndex]->Set_Diffuse(oldDiffuse);//CLAMPED

		//copy the normal light for fog so we can modify it
		m_scratchLight->Set_Transform(m_globalLight[globalLightIndex]->Get_Transform());
		//modify light with attenuated value to adjust for fog.
		m_globalLight[globalLightIndex]->Get_Diffuse(&oldDiffuse);
		m_scratchLight->Set_Diffuse(oldDiffuse*foggedLightFrac);
		m_globalLight[globalLightIndex]->Get_Ambient(&oldAmbient);
		m_scratchLight->Set_Ambient(oldAmbient*foggedLightFrac);
		m_foggedLightEnv.Add(Get_Light_Source(*m_scratchLight));
	}

	m_defaultLightEnv.Finalize();
	m_foggedLightEnv.Finalize();

	m_infantryAmbient = Get_Ambient_Light();// * infantryLightScale;	//for now don't adjust ambient so that we don't lose directional lighting.
}

/**Generate custom rendering passes for each potential player color.  This is currently only used
to render occluded objects using the color of the player*/
void RTS3DScene::updatePlayerColorPasses()
{
#ifdef USE_NON_STENCIL_OCCLUSION
	Vector3 hsv,rgb;

	if (TheGlobalData->m_enableBehindBuildingMarkers && TheGameLogic->getShowBehindBuildingMarkers())
	{
		Int numPlayers=ThePlayerList->getPlayerCount();

		for (Int i=0; i<numPlayers; i++)
		{	Player *player=ThePlayerList->getNthPlayer(i);
			Int playerIndex=player->getPlayerIndex();
			Real red,green,blue,alpha;
			GameGetColorComponentsReal(player->getPlayerColor(),&red,&green,&blue,&alpha);
			const auto source_hsv = Assets::RGB_To_HSV({red, green, blue});
            hsv.Set(source_hsv.x, source_hsv.y, source_hsv.z);
			hsv.Z*=TheGlobalData->m_occludedLuminanceScale;
			const auto converted = Assets::HSV_To_RGB({hsv.X, hsv.Y, hsv.Z});
                    rgb.Set(converted.x, converted.y, converted.z);
			Graphics::MeshMaterial *vmat=m_occludedMaterialPass[playerIndex]->material.get();
			vmat->parameters.emissive = {rgb.X,rgb.Y,rgb.Z};
		}
	}
#endif
}

#define ZBias 0.0001f

//DECLARE_PERF_TIMER(NonTerrainRender)
void RTS3DScene::Render(W3DRenderContext & rinfo)
{
    auto& draw_parameters = Graphics::Get_Scene_Draw_Parameters();
	//USE_PERF_TIMER(NonTerrainRender)
	draw_parameters.fog = {FogEnabled, FogStart, FogEnd, {FogColor.X, FogColor.Y, FogColor.Z, 1}};

	//Override the behind building selection if it's not available on current hardware (needs stencil).
	TheWritableGlobalData->m_enableBehindBuildingMarkers = TheWritableGlobalData->m_enableBehindBuildingMarkers &&
		Graphics::Get_Attachment_Bindings().Default().depth.Is_Valid();

	if (Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE)
	{
		if (m_customPassMode == SCENE_PASS_DEFAULT)
		{
			//Regular rendering pass with no effects
			updatePlayerColorPasses();///@todo: this probably doesn't need to be done each frame.
			updateFixedLightEnvironments(rinfo);
			Customized_Render(rinfo);
			Flush(rinfo);
		}
		else if (m_customPassMode == SCENE_PASS_ALPHA_MASK)
		{
			//a projected alpha texture which will later be used to determine where
			//wireframe should be visible.
			///@todo: Clearing to black may not be needed if the scene already did the clear.
			draw_parameters.color_write_mask = 0x08;
			draw_parameters.depth_bias = 0;

			Customized_Render(rinfo);	//render mask into alpha channel and fill z-buffer with depth values.
			Flush(rinfo);

			draw_parameters.color_write_mask = 0x07;

		}
	}
	else
	{
		Bool old_enable=Graphics::Get_Render_Settings().Is_Texturing_Enabled();
		if (Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_CLEAR_LINE)
		{
			//render scene with solid black color but have destination alpha store
			//a projected alpha texture which will later be used to determine where
			//wireframe should be visible.
			///@todo: Clearing to black may not be needed if the scene already did the clear.
			Graphics::Get_Attachment_Bindings().Clear(true, false, {0,0,0,1.0f});	// Clear color but not z
			draw_parameters.color_write_mask = 0x08;
			draw_parameters.depth_bias = 0;

			//We're only filling the z-buffer so ignore normal textures and state changes to speed things up.
			m_customPassMode = SCENE_PASS_ALPHA_MASK;

			Customized_Render(rinfo);	//render mask into alpha channel and fill z-buffer with depth values.
			Flush(rinfo);


			draw_parameters.color_write_mask = 0x07;
			Graphics::Get_Render_Settings().Set_Coloring_Enabled(true);
			Graphics::Get_Render_Settings().Set_Texturing_Enabled(false);
			draw_parameters.wireframe = true;

			//Move maximum z-buffer value in a little to shift all z-values closer
			//and thus forcing line to appear on top of previous pass.
			Real nearZ,farZ;
			rinfo.Camera.Get_Zbuffer_Range(nearZ, farZ);
			rinfo.Camera.Set_Zbuffer_Range(nearZ, farZ-ZBias);
			rinfo.Camera.Apply();

			// Material submission captures the pass depth bias.
			Customized_Render(rinfo);	//render wireframe where z-test passes
			Flush(rinfo);
			draw_parameters.wireframe = false;

			rinfo.Camera.Set_Zbuffer_Range(nearZ, farZ);
			rinfo.Camera.Apply();

			// Restore camera depth before the following pass.
			Graphics::Get_Render_Settings().Set_Texturing_Enabled(old_enable);
			Graphics::Get_Render_Settings().Set_Coloring_Enabled(false);

		}
		else
		{
			//old W3D custom rendering code.

			//Disable writes to color buffer to save memory bandwidth - we only need Z.
			draw_parameters.color_write_mask = 0;
			draw_parameters.depth_bias = 0;
			Customized_Render(rinfo);
			Flush(rinfo);
			//Re-enable writes to color buffer.
			draw_parameters.color_write_mask = 0x07;

			switch (Get_Extra_Pass_Polygon_Mode()) {
			case EXTRA_PASS_LINE:
				Graphics::Get_Render_Settings().Set_Texturing_Enabled(false);
				draw_parameters.wireframe = true;
				draw_parameters.depth_bias = 7;
				Customized_Render(rinfo);
				break;
			case EXTRA_PASS_CLEAR_LINE:
				Graphics::Get_Attachment_Bindings().Clear(true, false, {0,0,0,0.0f});	// Clear color but not z
				Graphics::Get_Render_Settings().Set_Texturing_Enabled(false);
				Graphics::Get_Render_Settings().Set_Coloring_Enabled(true);
				draw_parameters.wireframe = true;
				draw_parameters.depth_bias = 7;
				Customized_Render(rinfo);
				break;
			}
			Flush(rinfo);
			draw_parameters.wireframe = false;
			draw_parameters.depth_bias = 0;
			Graphics::Get_Render_Settings().Set_Texturing_Enabled(old_enable);
			Graphics::Get_Render_Settings().Set_Coloring_Enabled(false);
		}
	}
}

//=============================================================================
// RTS3DScene::Customized_Renderer
//=============================================================================
/** Custom render method for the RTS3DScene, custom render properties for our
  * particular game go here */
//=============================================================================
void RTS3DScene::Customized_Render( W3DRenderContext &rinfo )
{
    PROFILER_SECTION_NAME("Graphics.Scene.Traverse");
#ifdef DIRTY_CONDITION_FLAGS
	StDrawableDirtyStuffLocker lockDirtyStuff;
#endif

	W3DRenderObject *terrainObject=nullptr,*robj;
	m_translucentObjectsCount = 0;	//start of new frame so no translucent objects
	m_occludedObjectsCount = 0;

	const Int localPlayerIndex = rts::getObservedOrLocalPlayerIndex_Safe();

#define USE_LIGHT_ENV 1

   if (!Traversal.Is_Visibility_Checked_For(rinfo.Camera)) {
      // set the visibility bit in all render objects in all layers.
	   Visibility_Check(&rinfo.Camera);
#ifdef USE_NON_STENCIL_OCCLUSION
	   flagOccludedObjects(&rinfo.Camera);
#endif
   }
	Traversal.Clear_Visibility();


	Graphics::SceneObjectList<W3DRenderObject>::Cursor it(&UpdateList);
	// allow all objects in the update list to do their "every frame" processing
	{
		PROFILER_SECTION_NAME("Graphics.Scene.FrameUpdates");
		for (it.First(); !it.Is_Done(); it.Next()) {
			W3DRenderObject * robj = it.Peek_Obj();
			if (robj->Class_ID() == W3DRenderObject::CLASSID_TILEMAP)
				terrainObject=robj;	//found terrain object, store for later.
			if (!Get_W3D_Render_Services().Is_Reflection_Render_Pass()) {
				// If we are doing water mirror, we draw with backface culling inverted.  In this case,
				// we only want to call On_Frame_Update if we aren't drawing water, as otherwise
				// we get 2 frame updates per frame, and it screws up the particle emitters.
				it.Peek_Obj()->On_Frame_Update();
			}
		}
	}

	// Terrain objects are render objects first; some terrain implementations
	// register only for rendering and therefore are not present in UpdateList.
	// Keep the update-list scan above for per-frame processing, but discover the
	// tilemap from RenderList as a fallback so terrain is rendered in either
	// registration mode.
	if (terrainObject == nullptr)
	{
		Graphics::SceneObjectList<W3DRenderObject>::Cursor renderIt(&RenderList);
		for (renderIt.First(); !renderIt.Is_Done(); renderIt.Next())
		{
			W3DRenderObject *renderObj = renderIt.Peek_Obj();
			if (renderObj->Class_ID() == W3DRenderObject::CLASSID_TILEMAP)
			{
				terrainObject = renderObj;
				break;
			}
		}
	}

	if (terrainObject != nullptr && !Get_W3D_Render_Services().Is_Reflection_Render_Pass()
        && m_customPassMode == SCENE_PASS_DEFAULT
        && Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE) {
        if (!Collect_Directional_Shadow_Casters(rinfo)
            || !static_cast<BaseHeightMapRenderObjClass*>(terrainObject)->collectShadowCasters()
            || !Render_Directional_Shadow_Maps(rinfo)) {
            DEBUG_LOG(("Directional shadow submission failed.\n"));
        }
    }
	//terrain needs to be rendered first
	if (terrainObject)	// Don't check visibility - terrain is always visible. jba.
	{
		PROFILER_SECTION_NAME("Graphics.Scene.TerrainDraw");
		robj=terrainObject;
		rinfo.light_environment = nullptr;		// Terrain is self lit.
		rinfo.Camera.Set_User_Data(this);	//pass the scene to terrain via user data.
		// Terrain owns its complete terrain material, including the shroud
		// overlay. Do not install the legacy W3D shroud material pass around it;
		// that pass is still used for ordinary shrouded drawables below.
		if (m_customPassMode == SCENE_PASS_ALPHA_MASK && m_maskMaterialPass)
		{
			rinfo.Push_Material_Pass(m_maskMaterialPass);
			robj->Render(rinfo);
			rinfo.Pop_Material_Pass();
		}
		else
		robj->Render(rinfo);
	}

	if (m_drawTerrainOnly) {
		return;
	}
#ifdef EXTENDED_STATS
	if (Graphics::Get_Render_Diagnostics().disable_objects) {
		return;
	}
#endif

    Graphics::PropBatchScope prop_batches(Graphics::Get_Prop_Submission(),Graphics::Get_Attachment_Bindings().Current().viewport);
	// loop through all render objects in the list:
	for (it.First(&RenderList); !it.Is_Done();)
	{
		// get the render object
		robj = it.Peek_Obj();
 		it.Next();	//advance to next object in case this one gets deleted during renderOneObject().

		if (robj->Class_ID() == W3DRenderObject::CLASSID_TILEMAP)
			continue;	//we already rendered terrain

		if (robj->Is_Really_Visible()) {
			DrawableInfo *drawInfo = (DrawableInfo *)robj->Get_User_Data();
			Drawable *draw=nullptr;
			if (drawInfo)
				draw = drawInfo->m_drawable;
#ifdef USE_NON_STENCIL_OCCLUSION
			if (!(draw && drawInfo->m_flags & DrawableInfo::ERF_DELAYED_RENDER))	//model rendering is delayed for some reason until end of normal scene
#else
			if (!(draw && drawInfo->m_flags & (DrawableInfo::ERF_DELAYED_RENDER|DrawableInfo::ERF_POTENTIAL_OCCLUDER|DrawableInfo::ERF_IS_NON_OCCLUDER_OR_OCCLUDEE)))	//in this mode we delay almost all objects in order to do correct sorting with stencil.
#endif
				renderOneObject(rinfo, robj, localPlayerIndex);
		}
	}

	//Tell shadow manager to render shadows at the end of this frame
	//Don't draw shadows if there is no terrain present.
	if (TheW3DShadowManager && terrainObject && !Get_W3D_Render_Services().Is_Reflection_Render_Pass() &&
		Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE)
	{
		TheW3DShadowManager->queueShadows(TRUE);
	}

	// only render particles once per frame
	if (TheParticleSystemManager != nullptr &&
		Get_Extra_Pass_Polygon_Mode() == EXTRA_PASS_DISABLE)
	{
		TheParticleSystemManager->queueParticleRender();
	}
}

/**Convert a player index to a color index, we use this because color indices are
assigned in left-right binary flipped fashion so as not to occupy lower bits unless
necessary.  This is used because lower bits are free for use by stencil shadow
rendering*/
#define NUMBER_PLAYER_COLOR_BITS 4	//need 4 bits to encode 8 players because indices start at 1 (not 0).
Int playerIndexToColorIndex(Int playerIndex)
{
	Int tmp=playerIndex;
	Int result=0;
	Int flippedPosition;

	//Player index is stored in 4 bits
	for (Int i=0; i<NUMBER_PLAYER_COLOR_BITS; i++)
	{
		flippedPosition = NUMBER_PLAYER_COLOR_BITS-1-i;	//correct position of bit after it's flipped left/right

		if (flippedPosition > i)
		{
			//shifting left
			result |= (tmp & (1<<i))<<(flippedPosition-i);
		}
		else
		{
			//shifting right
			result |= (tmp & (1<<i))>>(i-flippedPosition);
		}
	}
	return result;
}

/**Utility function used to render a full screen quad with the specified color and
stencil mask*/
void renderStenciledPlayerColor(UnsignedInt color, UnsignedInt stencilRef, Bool clear=FALSE)
{
    auto* device=Graphics::Shared_Frame_Device();
    if (!device) return;
    Graphics::RHIViewport viewport{};
    viewport = Graphics::Get_Attachment_Bindings().Current().viewport;
    if (!viewport.width || !viewport.height) return;
    Int x,y; TheTacticalView->getOrigin(&x,&y);
    const float left=2.0f*(float(x)-viewport.x)/viewport.width-1;
    const float right=2.0f*(float(x+TheTacticalView->getWidth())-viewport.x)/viewport.width-1;
    const float top=1+2.0f*viewport.y/viewport.height;
    const float bottom=1-2.0f*(float(y+TheTacticalView->getHeight())-viewport.y)/viewport.height;
    Graphics::Draw_Player_Occlusion(Graphics::Get_Surface_Renderer(),device->Immediate_Command_List(),
        {left,top,right,bottom},{float((color>>16)&255)/255,float((color>>8)&255)/255,
        float(color&255)/255,float(color>>24)/255},static_cast<std::uint8_t>(stencilRef),clear,
        static_cast<std::uint8_t>(TheW3DShadowManager->getStencilShadowMask()));
}

#define MAX_VISIBLE_OCCLUDED_PLAYER_OBJECTS	512 //maximum number of occluded objects permitted per player
void RTS3DScene::flushOccludedObjectsIntoStencil(W3DRenderContext & rinfo)
{
    auto& draw_parameters = Graphics::Get_Scene_Draw_Parameters();
	W3DRenderObject *robj;
	Drawable *draw;
	W3DRenderObject *playerObjects[MAX_PLAYER_COUNT][MAX_VISIBLE_OCCLUDED_PLAYER_OBJECTS];
	W3DRenderObject **lastPlayerObject[MAX_PLAYER_COUNT];
	Int playerColorIndex[MAX_PLAYER_COUNT];
	Int visiblePlayerColors[MAX_PLAYER_COUNT];	///<color assigned to each of the visible players
	Int numObjects;
	Vector3 hsv,rgb;

	Int usedPlayerColorIndex=1;
	Int numVisiblePlayerColors=0;

	//Clear pointers into temporary arrays where each player's objects will be stored.
	//We do this so that all objects are sorted by color which reduces the number of
	//state changes needed when drawing them.
	for (Int i=0; i<MAX_PLAYER_COUNT; i++)
	{
		lastPlayerObject[i]=&playerObjects[i][0];
		playerColorIndex[i]=-1;
	}

	//Assume no player colors are visible and all stencil bits are free for use by shadows.
	TheW3DShadowManager->setStencilShadowMask(0);

	const Int localPlayerIndex = rts::getObservedOrLocalPlayerIndex_Safe();

	if (m_numPotentialOccludees && m_numPotentialOccluders)
	{
		//bucket sort all possibly occluded objects by player index/color.
		Int k=0;
		for (; k<m_numPotentialOccludees; k++)
		{
			robj=m_potentialOccludees[k];

			draw = ((DrawableInfo *)robj->Get_User_Data())->m_drawable;
			Object *object=draw->getObject();

			Int index=object->getControllingPlayer()->getPlayerIndex();

			if ((lastPlayerObject[index]-&playerObjects[index][0]) >= MAX_VISIBLE_OCCLUDED_PLAYER_OBJECTS)
			{
				DEBUG_CRASH(("Exceeded Maximum Number of potentially occluded models"));
				continue;
			}

			*lastPlayerObject[index] = robj;
			lastPlayerObject[index]++;	//increment to next object
		}

		draw_parameters.stencil.enabled = true;
		draw_parameters.stencil.read_mask = 0xff;
		draw_parameters.stencil.write_mask = 0xff;
		//Always store player index into stencil unless it is occluded by another
		//player's potentially occluded objects.
		draw_parameters.stencil.front.comparison = draw_parameters.stencil.back.comparison = Graphics::RHIComparison::Always;
		draw_parameters.stencil.front.depth_fail = draw_parameters.stencil.back.depth_fail = Graphics::RHIStencilOperation::Keep;
		draw_parameters.stencil.front.fail = draw_parameters.stencil.back.fail = Graphics::RHIStencilOperation::Keep;
		draw_parameters.stencil.front.pass = draw_parameters.stencil.back.pass = Graphics::RHIStencilOperation::Replace;

		//Find out which player indices are actually used and remap them to
		//a color index.  Render all objects using the same color index at once.
		//We render potential occludees first because this allows them to z-sort correctly
		//when they are behind an occluder.
		for (k=0; k<MAX_PLAYER_COUNT; k++)
		{
			if ((numObjects=lastPlayerObject[k]-&playerObjects[k][0]) != 0)
			{
				//this player has some objects so draw them using his color index.
				if (playerColorIndex[k]==-1)	//color index not assigned yet?
				{
					//assign a new color index to this player
					playerColorIndex[k]=playerIndexToColorIndex(usedPlayerColorIndex++);
					//assign a color to this index by copying it from the controlling player
					//of all objects in this list.
					draw = ((DrawableInfo *)playerObjects[k][0]->Get_User_Data())->m_drawable;
					Object *object=draw->getObject();

					Int color=object->getControllingPlayer()->getPlayerColor();
					const auto source_hsv = Assets::RGB_To_HSV({((color >> 16) & 255) / 255.0f,
                        ((color >> 8) & 255) / 255.0f, (color & 255) / 255.0f});
                    hsv.Set(source_hsv.x, source_hsv.y, source_hsv.z);
					hsv.Z*=TheGlobalData->m_occludedLuminanceScale;
					const auto converted = Assets::HSV_To_RGB({hsv.X, hsv.Y, hsv.Z});
                    rgb.Set(converted.x, converted.y, converted.z);
					visiblePlayerColors[numVisiblePlayerColors++]=Assets::Color_To_ARGB({rgb.X,rgb.Y,rgb.Z,0.5f});
				}

				Int thisPlayerColorIndex=playerColorIndex[k];

				//Store this object's color index into bits 3-6 of stencil buffer
				draw_parameters.stencil.reference = static_cast<std::uint8_t>(thisPlayerColorIndex<<3);

				//Render all of this player's objects for which we care when they are occluded.
				W3DRenderObject **renderList=&playerObjects[k][0];
				for (Int j=0; j<numObjects; j++)
				{
					DrawableInfo *drawInfo=((DrawableInfo *)(*renderList)->Get_User_Data());
					if (drawInfo->m_flags & DrawableInfo::ERF_IS_TRANSLUCENT)
					{
						// TheSuperHackers @info This only draws the occlusion of translucent objects.
						Graphics::Get_Prop_Submission().Flush_Materials();	//render all the submitted meshes using current stencil function
						//Disable writing to color buffer since translucent objects are rendered at end of frame.
						draw_parameters.stencil.front.comparison = draw_parameters.stencil.back.comparison = Graphics::RHIComparison::Never;	//never allow frame buffer writes.
						draw_parameters.stencil.front.fail = draw_parameters.stencil.back.fail = Graphics::RHIStencilOperation::Replace;	//always replace existing stencil value
						renderOneObject(rinfo, (*renderList), localPlayerIndex);
						Graphics::Get_Prop_Submission().Flush_Materials();	//render all the submitted meshes using current stencil function
						draw_parameters.stencil.front.fail = draw_parameters.stencil.back.fail = Graphics::RHIStencilOperation::Keep;
						draw_parameters.stencil.front.comparison = draw_parameters.stencil.back.comparison = Graphics::RHIComparison::Always;
					}
					else
					{
						renderOneObject(rinfo, (*renderList), localPlayerIndex);
					}
					renderList++;	//advance to next object
				}

					Graphics::Get_Prop_Submission().Flush_Materials();	//render all the submitted meshes using current stencil function

			}
		}
		//Stencil buffer is now filled with color indices of potentially occluded objects.  We now draw
		//non-occluder or occludee objects such as small rocks, shrubs, etc. which we don't care about
		//but need to render here so that they don't interfere with building occlusion.
		draw_parameters.stencil.enabled = false;	//these objects are not stored in stencil
		W3DRenderObject **nonOccluderOrOccludeeList=m_nonOccludersOrOccludees;
		for (k=0; k<m_numNonOccluderOrOccludee; k++)
		{
			renderOneObject(rinfo, (*nonOccluderOrOccludeeList), localPlayerIndex);
			nonOccluderOrOccludeeList++;	//advance to next one
		}
					Graphics::Get_Prop_Submission().Flush_Materials();	//render all the submitted meshes using current stencil function

		//Stencil buffer is now filled with color indices of potentially occluded objects.  We now draw
		//occluder objects so they cover up and modify stencil MSB wherever they are in front of other objects.
		draw_parameters.stencil.enabled = true;
		draw_parameters.stencil.reference = 0xff;
		draw_parameters.stencil.read_mask = 0xff;	//isolate lowest player color
		draw_parameters.stencil.write_mask = static_cast<std::uint8_t>(0x80);	//only write to MSB
		draw_parameters.stencil.front.comparison = draw_parameters.stencil.back.comparison = Graphics::RHIComparison::Always;	//check if player colors stored in pixel
		draw_parameters.stencil.front.depth_fail = draw_parameters.stencil.back.depth_fail = Graphics::RHIStencilOperation::Keep;
		draw_parameters.stencil.front.fail = draw_parameters.stencil.back.fail = Graphics::RHIStencilOperation::Keep;
		draw_parameters.stencil.front.pass = draw_parameters.stencil.back.pass = Graphics::RHIStencilOperation::Replace;

		//Render all potential occluders on top of already rendered potential occludees.
		W3DRenderObject **occluderList=m_potentialOccluders;
		for (k=0; k<m_numPotentialOccluders; k++)
		{
			renderOneObject(rinfo, (*occluderList), localPlayerIndex);
			occluderList++;	//advance to next one
		}

					Graphics::Get_Prop_Submission().Flush_Materials();	//render all the submitted meshes using current stencil function


		//We now have a stencil buffer where pixels that are occluded have a bit pattern of 1INDX000.
		//INDX contains the occluded player's color index.  We walk through all the player colors and
		//draw them wherever the stencil matches the color's index.
		Int usedPlayerColorBits=0;
		for (k=0; k<numVisiblePlayerColors; k++)
		{
			Int color=visiblePlayerColors[k];
			Int stencilRef=(playerIndexToColorIndex(k+1)<<3)|0x80;
			renderStenciledPlayerColor(color,stencilRef);
			usedPlayerColorBits |= stencilRef;	//keep track of all bits used for occlusion/player colors.
		}

		TheW3DShadowManager->setStencilShadowMask(usedPlayerColorBits);
		if (numVisiblePlayerColors >= 8 && TheGlobalData->m_useShadowVolumes)
		{
			//for cases where we have 8 or more visible players, we're only left with 3 bits to store
			//stencil shadows.  That's probably not enough since it will only allow 7 overlapping shadows.
			//So we clear the stencil buffer, leaving only the MSB set on any occluded player pixels so that
			//shadow code knows not to overwrite these pixels.
			renderStenciledPlayerColor(0,0, TRUE);
			TheW3DShadowManager->setStencilShadowMask(0x80808080);	//msb indicates occluded player pixels so ignore it when filling screen with shadow
		}

		draw_parameters.stencil.enabled = false;
	}
	else
	if (m_numNonOccluderOrOccludee || m_numPotentialOccluders || m_numPotentialOccludees)
	{
		//no occluded objects so don't need to render anything special.  Just draw the queued up
		//objects like normal because they were skipped in the main scene traversal.

		W3DRenderObject **occludeeList=m_potentialOccludees;
		Int k=0;
		for (; k<m_numPotentialOccludees; k++)
		{
			// TheSuperHackers @bugfix xezon 18/10/2025 No longer draws translucent objects
			// as non-translucent ones here. They are drawn in another pass.
			DrawableInfo *drawInfo = static_cast<DrawableInfo *>((*occludeeList)->Get_User_Data());
			if ((drawInfo->m_flags & DrawableInfo::ERF_IS_TRANSLUCENT) == 0)
			{
				renderOneObject(rinfo, (*occludeeList), localPlayerIndex);
			}
			occludeeList++;	//advance to next one
		}

		W3DRenderObject **occluderList=m_potentialOccluders;
		for (k=0; k<m_numPotentialOccluders; k++)
		{
			renderOneObject(rinfo, (*occluderList), localPlayerIndex);
			occluderList++;	//advance to next one
		}

		W3DRenderObject **nonOccluderOrOccludeeList=m_nonOccludersOrOccludees;
		for (k=0; k<m_numNonOccluderOrOccludee; k++)
		{
			renderOneObject(rinfo, (*nonOccluderOrOccludeeList), localPlayerIndex);
			nonOccluderOrOccludeeList++;	//advance to next one
		}
					Graphics::Get_Prop_Submission().Flush_Materials();	//render all the submitted meshes using current stencil function
	}

	//Reset scene ambient because we sometimes mess around with it to make objects
	//glow, etc. when processing drawables.  This is a good place to do it because this
	//function gets called right after we flush regular render objects.
}

/*Version which does not require stencil buffer*/
void RTS3DScene::flushOccludedObjects(W3DRenderContext & rinfo)
{
    auto& draw_parameters = Graphics::Get_Scene_Draw_Parameters();
	W3DRenderObject *robj;
	Drawable *draw;

	TheW3DShadowManager->setStencilShadowMask(0);

	if (m_occludedObjectsCount)
	{
		const Int localPlayerIndex = rts::getObservedOrLocalPlayerIndex_Safe();

		if (Graphics::Get_Attachment_Bindings().Default().depth.Is_Valid())	//just in case we have shadows, disable them over occluded pixels.
		{
			//Set all stencil pixels of potentially occluded objects to 128.
			draw_parameters.stencil.enabled = true;
			draw_parameters.stencil.reference = 128;
			draw_parameters.stencil.read_mask = 0xff;
			draw_parameters.stencil.write_mask = 0xff;
			draw_parameters.stencil.front.depth_fail = draw_parameters.stencil.back.depth_fail = Graphics::RHIStencilOperation::Keep;
			draw_parameters.stencil.front.fail = draw_parameters.stencil.back.fail = Graphics::RHIStencilOperation::Keep;
			draw_parameters.stencil.front.pass = draw_parameters.stencil.back.pass = Graphics::RHIStencilOperation::Replace;
			draw_parameters.stencil.front.comparison = draw_parameters.stencil.back.comparison = Graphics::RHIComparison::Always;
		}

		//First draw all the solid colored models
		///@todo: Optimize this so that the extra passes don't actually install the material since it's all the same.
		rinfo.Push_Override_Flags(Graphics::DrawOverride::AdditionalPassesOnly);	//disable textures
		Int i=0;
		for (; i<m_occludedObjectsCount; i++)
		{
			robj=m_potentialOccludees[i];

			draw = ((DrawableInfo *)robj->Get_User_Data())->m_drawable;
			Object *object=draw->getObject();

			Int index=object->getControllingPlayer()->getPlayerIndex();

			rinfo.Push_Material_Pass(m_occludedMaterialPass[index]);
			robj->Render(rinfo);
			rinfo.Pop_Material_Pass();
		}
		rinfo.Pop_Override_Flags();
				Graphics::Get_Prop_Submission().Flush_Materials();

		//Now draw the normal models so they cover up the colored models on any pixels that

		//Now draw the normal models so they cover up the colored models on any pixels that
		//Normal models will clear stencil value from 128 back to 0 where the object pixels are
		//not occluded but will leave  128 in stencil where still occluded.
		if (Graphics::Get_Attachment_Bindings().Default().depth.Is_Valid())
			draw_parameters.stencil.reference = 0;

		for (i=0; i<m_occludedObjectsCount; i++)
		{
			robj=m_potentialOccludees[i];
			renderOneObject(rinfo, robj, localPlayerIndex);//Get_W3D_Render_Services().Render(*robj,rinfo);
		}

		//Flush all the submitted translucent objects.
				Graphics::Get_Prop_Submission().Flush_Materials();
		m_occludedObjectsCount = 0;
		draw_parameters.stencil.enabled = false;
		TheW3DShadowManager->setStencilShadowMask(0x80808080);	//upper MSB always contains flag indicating occluded player color.
	}

	//Reset scene ambient because we sometimes mess around with it to make objects
	//glow, etc. when processing drawables.  This is a good place to do it because this
	//function gets called right after we flush regular render objects.
}

void RTS3DScene::flushTranslucentObjects(W3DRenderContext & rinfo)
{
	W3DRenderObject *robj;
	Drawable *draw;

	if (m_translucentObjectsCount)
	{
		const Int localPlayerIndex = rts::getObservedOrLocalPlayerIndex_Safe();

		for (Int i=0; i<m_translucentObjectsCount; i++)
		{
			robj=m_translucentObjectsBuffer[i];
			draw = ((DrawableInfo *)robj->Get_User_Data())->m_drawable;

			rinfo.alpha_override = draw->getEffectiveOpacity();

			renderOneObject(rinfo, robj, localPlayerIndex);//Get_W3D_Render_Services().Render(*robj,rinfo);
		}

		//Flush all the submitted translucent objects.
				Graphics::Get_Prop_Submission().Flush_Materials();
		Graphics::Get_Scene_Draw_Queue().Drain(&rinfo, [] { Graphics::Get_Prop_Submission().Flush_Materials(); });	//draws things like water
		rinfo.alpha_override = 1.0f;	//disable forced alpha
		m_translucentObjectsCount = 0;
	}

	//Reset scene ambient because we sometimes mess around with it to make objects
	//glow, etc. when processing drawables.  This is a good place to do it because this
	//function gets called right after we flush regular render objects.
}

//=============================================================================
// RTS3DScene::createLightsIterator
//=============================================================================
/** Returns an iterator of the lights in the scene. */
//=============================================================================
Graphics::SceneObjectList<W3DRenderObject>::Cursor * RTS3DScene::createLightsIterator()
{
	Graphics::SceneObjectList<W3DRenderObject>::Cursor * it = NEW Graphics::SceneObjectList<W3DRenderObject>::Cursor(&LightList);	// poolify
	return it;
}


//=============================================================================
// RTS3DScene::destroyLightsIterator
//=============================================================================
/** Destroys the iterator returned by createLightsIterator. */
//=============================================================================
void RTS3DScene::destroyLightsIterator(Graphics::SceneObjectList<W3DRenderObject>::Cursor * it)
{
	delete it;
}


//=============================================================================
// RTS3DScene::addDynamicLight
//=============================================================================
/** Adds a dynamic light. */
//=============================================================================
void RTS3DScene::addDynamicLight(W3DDynamicLight * obj)
{
	m_dynamicLightList.Add(obj);
	UpdateList.Add(obj);
}

//=============================================================================
// RTS3DScene::addDynamicLight
//=============================================================================
/** Adds a dynamic light. */
//=============================================================================
W3DDynamicLight * RTS3DScene::getADynamicLight()
{
	Graphics::SceneObjectList<W3DRenderObject>::Cursor dynaLightIt(&m_dynamicLightList);
	W3DDynamicLight *pLight;
	for (dynaLightIt.First(); !dynaLightIt.Is_Done(); dynaLightIt.Next())
	{
		pLight = (W3DDynamicLight*)dynaLightIt.Peek_Obj();
		if (!pLight->isEnabled()) {
			pLight->setEnabled(true);
			return(pLight);
		}
	}
	pLight = NEW_REF(W3DDynamicLight, ());
	addDynamicLight( pLight );
	pLight->Release_Ref();
	pLight->setEnabled(true);
	return(pLight);
}

//=============================================================================
// RTS3DScene::removeDynamicLight
//=============================================================================
/** Removes a dynamic light. */
//=============================================================================
void RTS3DScene::removeDynamicLight(W3DDynamicLight * obj)
{
	m_dynamicLightList.Remove(obj);
}

//=============================================================================
// RTS3DScene::doRender
//=============================================================================
/** Render the scene */
//=============================================================================
void RTS3DScene::doRender( W3DCamera * cam )
{
	m_camera = cam;
	DRAW();
	m_camera = nullptr;

}

//=============================================================================
// RTS3DScene::draw
//=============================================================================
/** Customized render for the 2d scene management */
//=============================================================================
void RTS3DScene::draw()
{
	if (m_camera == nullptr) {
		DEBUG_CRASH(("Null m_camera in RTS3DScene::draw"));
		return;
	}
	Get_W3D_Render_Services().Render( this, m_camera );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// RTS2DScene::RTS2DScene
//=============================================================================
/** */
//=============================================================================
RTS2DScene::RTS2DScene()
{
	setName("RTS2DScene");
	m_status = NEW_REF( W3DStatusCircle, () );
	Add_Render_Object( m_status );
}

//=============================================================================
// RTS2DScene::~RTS2DScene
//=============================================================================
/** */
//=============================================================================
RTS2DScene::~RTS2DScene()
{
	this->Remove_Render_Object(m_status);
	REF_PTR_RELEASE(m_status);
}

//=============================================================================
// RTS2DScene::Custimized_Render
//=============================================================================
/** Customized render for the 2d scene management */
//=============================================================================
void RTS2DScene::Customized_Render( W3DRenderContext &rinfo )
{
	// call simple scene class renderer
	W3DSimpleScene::Customized_Render( rinfo );
}

//=============================================================================
// RTS2DScene::doRender
//=============================================================================
/** Render the scene */
//=============================================================================
void RTS2DScene::doRender( W3DCamera * cam )
{
	m_camera = cam;
	DRAW();
	m_camera = nullptr;
}

//=============================================================================
// RTS2DScene::draw
//=============================================================================
/** Customized render for the 2d scene management */
//=============================================================================
void RTS2DScene::draw()
{
	if (m_camera == nullptr) {
		DEBUG_CRASH(("Null m_camera in RTS2DScene::draw"));
		return;
	}
	Get_W3D_Render_Services().Render( this, m_camera );
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// RTS3DInterfaceScene::RTS3DInterfaceScene
//=============================================================================
/** */
//=============================================================================
RTS3DInterfaceScene::RTS3DInterfaceScene()
{
}

//=============================================================================
// RTS3DInterfaceScene::~RTS3DInterfaceScene
//=============================================================================
/** */
//=============================================================================
RTS3DInterfaceScene::~RTS3DInterfaceScene()
{
}

//=============================================================================
// RTS3DInterfaceScene::Custimized_Render
//=============================================================================
/** Customized render for the 3d interface scene management */
//=============================================================================
void RTS3DInterfaceScene::Customized_Render( W3DRenderContext &rinfo )
{
	if (!Traversal.Is_Visibility_Checked_For(rinfo.Camera))
		Visibility_Check(&rinfo.Camera);
	Traversal.Clear_Visibility();
    Graphics::SceneObjectList<W3DRenderObject>::Cursor it(&UpdateList);
    for (it.First(); !it.Is_Done(); it.Next()) it.Peek_Obj()->On_Frame_Update();
    Graphics::LocalLighting environment;
    environment.Reset({0,0,0}, {(AmbientLight).X,(AmbientLight).Y,(AmbientLight).Z});
    for (it.First(&LightList); !it.Is_Done(); it.Next())
        environment.Add(Get_Light_Source(*static_cast<W3DLight *>(it.Peek_Obj())));
    environment.Finalize();
    Graphics::PropLighting lighting;
    const auto& ambient=environment.ambient;
    lighting.ambient=ambient;
    for (int i=0;i<environment.count && i<4;++i) {
        const auto& direction=environment.lights[i].direction;
        const auto& diffuse=environment.lights[i].diffuse;
        lighting.lights[i].direction=direction;
        lighting.lights[i].diffuse=diffuse;
    }
    for (it.First(&RenderList); !it.Is_Done(); it.Next()) {
        W3DRenderObject* object=it.Peek_Obj();
        if (!object->Is_Really_Visible()) continue;
        auto* hook=object->Get_Render_Hook();
        if (!hook || hook->Pre_Render(object,rinfo)) {
            W3DObjectGraphics graphics;
            graphics.Render(*object,rinfo,lighting,nullptr);
        }
        if (hook) hook->Post_Render(object,rinfo);
    }
}



/// The following is an :archive" of a partial attempt at detecting the mapshroud hack
/*
 *

void RTS3DScene::Visibility_Check(W3DCamera * camera)
{
    GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.Scene.Visibility");
#ifdef DIRTY_CONDITION_FLAGS
	StDrawableDirtyStuffLocker lockDirtyStuff;
#endif

	Graphics::SceneObjectList<W3DRenderObject>::Cursor it(&RenderList);
	DrawableInfo *drawInfo = nullptr;
	Drawable	*draw = nullptr;
	W3DRenderObject * robj;

	m_numPotentialOccluders=0;
	m_numPotentialOccludees=0;
	m_translucentObjectsCount=0;
	m_numNonOccluderOrOccludee=0;

	Int currentFrame=0;
	if (TheGameLogic) currentFrame = TheGameLogic->getFrame();
	if (currentFrame <= TheGlobalData->m_defaultOcclusionDelay)
		currentFrame = TheGlobalData->m_defaultOcclusionDelay+1;	//make sure occlusion is enabled when game starts (frame 0).


	if (Get_W3D_Render_Services().Is_Reflection_Render_Pass())
	{	//we are rendering reflections
		///@todo: Have better flag to detect reflection pass

		// Loop over all top-level RenderObjects in this scene. If the bounding sphere is not in front
		// of all the frustum planes, it is invisible.
		for (it.First(); !it.Is_Done(); it.Next()) {

			robj = it.Peek_Obj();

			draw=nullptr;
			drawInfo = (DrawableInfo *)robj->Get_User_Data();
			if (drawInfo)
				draw=drawInfo->m_drawable;

			if( draw )
			{
				if (robj->Is_Force_Visible()) {
					robj->Set_Visible(true);
				} else {
					robj->Set_Visible(draw->getDrawsInMirror() && !camera->Cull_Sphere(robj->Get_Bounding_Sphere()));
				}
			}
			else
			{	//perform normal culling on non-drawables
				if (robj->Is_Force_Visible()) {
					robj->Set_Visible(true);
				} else {
					robj->Set_Visible(!camera->Cull_Sphere(robj->Get_Bounding_Sphere()));
				}
			}
		}
	}
	else
	{

		// Loop over all top-level RenderObjects in this scene. If the bounding sphere is not in front
		// of all the frustum planes, it is invisible.
		for (it.First(); !it.Is_Done(); it.Next()) {

			robj = it.Peek_Obj();

			if (robj->Is_Force_Visible()) {
				robj->Set_Visible(true);
			} else if (robj->Is_Hidden()) {
				robj->Set_Visible(false);
			} else {

				UnsignedByte isVisible = 0;


        //Cheater Foil
        isVisible |= (UnsignedByte)(camera->Cull_Sphere(robj->Get_Bounding_Sphere()) == FALSE);
        isVisible |= (UnsignedByte)(draw->isDrawableEffectivelyHidden());
        isVisible |= (draw->getFullyObscuredByShroudWithCheatSpy());
				robj->Set_VisibleWithCheatSpy(isVisible);
				if (robj->Is_VisibleWithCheatSpy())//this will clear for the bit set above

				{	//need to keep track of occluders and ocludees for subsequent code.
					drawInfo = (DrawableInfo *)robj->Get_User_Data();
					if (drawInfo && (draw=drawInfo->m_drawable) != nullptr)
					{

//            now handled above in the cheater foil <<<<<<<<<<<<
//						if (draw->isDrawableEffectivelyHidden() || draw->getFullyObscuredByShroud())
//						{	robj->Set_Visible(false);
//							continue;
//						}
						//assume normal rendering.
						drawInfo->m_flags = DrawableInfo::ERF_IS_NORMAL;	//clear any rendering flags that may be in effect.


						if (draw->getEffectiveOpacity() != 1.0f && m_translucentObjectsCount < TheGlobalData->m_maxVisibleTranslucentObjects)
						{	drawInfo->m_flags |= DrawableInfo::ERF_IS_TRANSLUCENT;	//object is translucent
							m_translucentObjectsBuffer[m_translucentObjectsCount++] = robj;
						}
						if (TheGlobalData->m_enableBehindBuildingMarkers && TheGameLogic->getShowBehindBuildingMarkers())
						{
							//visible drawable. Check if it's either an occluder or occludee
							if (draw->isKindOf(KINDOF_STRUCTURE) && m_numPotentialOccluders < TheGlobalData->m_maxVisibleOccluderObjects)
							{	//object which could occlude other objects that need to be visible.
								//Make sure this object is not translucent so it's not rendered twice (from m_potentialOccluders and m_translucentObjectsBuffer)
								if (drawInfo->m_flags ^ DrawableInfo::ERF_IS_TRANSLUCENT)
									m_potentialOccluders[m_numPotentialOccluders++]=robj;
								drawInfo->m_flags |= DrawableInfo::ERF_POTENTIAL_OCCLUDER;
							}
							else
							if (draw->getObject() &&
									(draw->isKindOf(KINDOF_SCORE) || draw->isKindOf(KINDOF_SCORE_CREATE) || draw->isKindOf(KINDOF_SCORE_DESTROY) || draw->isKindOf(KINDOF_MP_COUNT_FOR_VICTORY)) &&
									(draw->getObject()->getSafeOcclusionFrame()) <= currentFrame && m_numPotentialOccludees < TheGlobalData->m_maxVisibleOccludeeObjects)
							{	//object which could be occluded but still needs to be visible.
								//We process transucent units twice (also in m_translucentObjectsBuffer) because we need to see them when occluded.
								m_potentialOccludees[m_numPotentialOccludees++]=robj;
								drawInfo->m_flags |= DrawableInfo::ERF_POTENTIAL_OCCLUDEE;
							}
							else
							if (drawInfo->m_flags == DrawableInfo::ERF_IS_NORMAL && m_numNonOccluderOrOccludee < TheGlobalData->m_maxVisibleNonOccluderOrOccludeeObjects)
							{	//regular object with no custom effects but still needs to be delayed to get the occlusion feature to work correctly.
								//Make sure this object is not translucent so it's not rendered twice (from m_potentialOccluders and m_translucentObjectsBuffer)
								if (drawInfo->m_flags ^ DrawableInfo::ERF_IS_TRANSLUCENT)	//make sure not translucent
									m_nonOccludersOrOccludees[m_numNonOccluderOrOccludee++]=robj;
								drawInfo->m_flags |= DrawableInfo::ERF_IS_NON_OCCLUDER_OR_OCCLUDEE;
							}
						}


					}
				}

//				robj->Set_Visible(isVisible); //now handled above in the cheater foil
			}

			///@todo: We're not using LOD yet so I disabled this code. MW
			// Also, should check how multiple passes (reflections) get along
			// with the LOD manager - we're rendering double the load it thinks we are.
			// Prepare visible objects for LOD:
			//	if (robj->Is_Really_Visible()) {
			//			robj->Prepare_LOD(*camera);
			//		}
		}
	}

	Traversal.Adopt_Visibility(*camera);
}


 *
 */
