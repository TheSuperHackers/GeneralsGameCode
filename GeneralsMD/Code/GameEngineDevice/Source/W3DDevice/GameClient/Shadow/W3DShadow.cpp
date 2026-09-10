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


// FILE: W3DShadow.cpp ///////////////////////////////////////////////////////////
//
// Real time shadow representations
//
// Author: Mark Wilczynski, February 2002
//
//

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "WWLib/always.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DHierarchyRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshResource.h"
#include "Lib/BaseType.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/W3DProjectedShadow.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DDirectionalShadows.h"
#include "Common/Debug.h"
#include "Common/PerfTimer.h"

#define SUN_DISTANCE_FROM_GROUND	10000.0f	//distance of sun (our only light source).

// Global Variables and Functions /////////////////////////////////////////////
W3DShadowManager *TheW3DShadowManager=nullptr;
const FrustumClass *shadowCameraFrustum;

Vector3 LightPosWorld[ MAX_SHADOW_LIGHTS ] =
{

	Vector3( 94.0161f, 50.499f, 200.0f)
};

void PrepareShadows()
{
	if (TheW3DProjectedShadowManager)
		TheW3DProjectedShadowManager->prepareShadows();
}

//DECLARE_PERF_TIMER(shadowsRender)
void DoShadows(W3DRenderContext & rinfo, Bool stencilPass)
{
    shadowCameraFrustum = &rinfo.Camera.Get_Frustum();
    if (TheW3DShadowManager != nullptr && stencilPass)
        TheW3DShadowManager->queueShadows(FALSE);

}

W3DShadowManager::W3DShadowManager()
{
	DEBUG_ASSERTCRASH(TheW3DProjectedShadowManager == nullptr,
		("Creating new shadow managers without deleting old ones"));

	m_shadowColor = 0x7fa0a0a0;
	m_isShadowScene = FALSE;
	m_stencilShadowMask = 0;	//all bits can be used for storing shadows.

	Vector3 lightRay(-TheGlobalData->m_terrainLightPos[0].x,
		-TheGlobalData->m_terrainLightPos[0].y, -TheGlobalData->m_terrainLightPos[0].z);
	lightRay.Normalize();

	LightPosWorld[0]=lightRay*SUN_DISTANCE_FROM_GROUND;

	TheProjectedShadowManager = TheW3DProjectedShadowManager = NEW W3DProjectedShadowManager;
}

W3DShadowManager::~W3DShadowManager()
{
    Reset_Directional_Shadows();
	delete TheW3DProjectedShadowManager;
	TheProjectedShadowManager = TheW3DProjectedShadowManager = nullptr;
}

/** Do one-time initilalization of shadow systems that need to be
active for full duration of game*/
Bool W3DShadowManager::init()
{
	Bool result=TRUE;

	if ( TheW3DProjectedShadowManager && TheW3DProjectedShadowManager->init())
	{
		if (TheW3DProjectedShadowManager->ReAcquireResources())
			result = TRUE;
	}

	return result;
}

/** Do per-map reset.  This frees up shadows from all objects since
they may not exist on the next map*/
void W3DShadowManager::Reset()
{
    Reset_Directional_Shadows();

	if (TheW3DProjectedShadowManager)
		TheW3DProjectedShadowManager->reset();
}

Bool W3DShadowManager::ReAcquireResources()
{
	Bool result = TRUE;

	if (TheW3DProjectedShadowManager && !TheW3DProjectedShadowManager->ReAcquireResources())
		result = FALSE;

	return result;
}

void W3DShadowManager::ReleaseResources()
{
	if (TheW3DProjectedShadowManager)
		TheW3DProjectedShadowManager->ReleaseResources();
}

Shadow *W3DShadowManager::addShadow( W3DRenderObject *robj, Shadow::ShadowTypeInfo *shadowInfo, Drawable *draw)
{
	ShadowType type = SHADOW_VOLUME;

	if (shadowInfo)
		type = shadowInfo->m_type;

	switch(type)
	{
		case	SHADOW_VOLUME:
		case	SHADOW_PROJECTION:
		case	SHADOW_DECAL:
            return Create_Directional_Shadow(robj);
		default:
			return nullptr;
	}

	return nullptr;
}

void W3DShadowManager::removeShadow(Shadow *shadow)
{
	shadow->release();
}

void W3DShadowManager::removeAllShadows()
{
    Reset_Directional_Shadows();
	if (TheW3DProjectedShadowManager)
		TheW3DProjectedShadowManager->removeAllShadows();
}

/**Force update of all shadows even when light source and object have not moved*/
void W3DShadowManager::invalidateCachedLightPositions()
{
	if (TheW3DProjectedShadowManager)
		TheW3DProjectedShadowManager->invalidateCachedLightPositions();
}

Vector3 &W3DShadowManager::getLightPosWorld(Int lightIndex)
{
	return LightPosWorld[lightIndex];
}

void W3DShadowManager::setLightPosition(Int lightIndex, Real x, Real y, Real z)
{
	if (lightIndex != 0)
		return;	///@todo: Add support for multiple lights

	LightPosWorld[lightIndex]=Vector3(x,y,z);
}

void W3DShadowManager::setTimeOfDay(TimeOfDay tod)
{
	//Ray to light source
	const GlobalData::TerrainLighting *ol=&TheGlobalData->m_terrainObjectsLighting[tod][0];

	Vector3 lightRay(-ol->lightPos.x,-ol->lightPos.y,-ol->lightPos.z);

	lightRay.Normalize();
	lightRay *= SUN_DISTANCE_FROM_GROUND;

	setLightPosition(0, lightRay.X, lightRay.Y, lightRay.Z);
}
