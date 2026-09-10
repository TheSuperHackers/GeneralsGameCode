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

// FILE: W3DProjectedShadow.h ///////////////////////////////////////////////////////////
//
// Real time shadow projection through textures.
//
// Author: Mark Wilczynski, February 2002
//
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>

#include "GameClient/Shadow.h"
import Graphics.Scene.Lighting.Local;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.TextureMapping;
import Graphics.Materials.TextureProjector;

class W3DShadowTexture;	//forward reference
class W3DShadowTextureManager;	//forward reference
class Drawable;	//forward reference
class W3DProjectedShadow; //forward reference.

class W3DProjectedShadowManager	: public ProjectedShadowManager
{
/*	enum ShadowTextureType {
		STT_STATIC	=	0x0001,
		STT_DYNAMIC	= 0x0002,
		STT_SHARED
			};
*/
	public:
		W3DProjectedShadowManager();
		virtual ~W3DProjectedShadowManager() override;
		Bool init();					///<allocate one-time shadow assets for length of entire game.
		void reset();					///<free all existing shadows - ready for next map.
		void shutdown();			///<free all assets prior to shutdown of entire game.
		void prepareShadows();
		Int	 renderShadows(W3DRenderContext & rinfo);	///<iterate over each object and render its shadow onto affected objects.
		void ReleaseResources();	///<release device dependent D3D resources.
		Bool ReAcquireResources();	///<allocate device dependent D3D resources.
		void invalidateCachedLightPositions();	///<forces shadows to update regardless of last lightposition

		virtual Shadow	*addDecal(W3DRenderObject *robj, Shadow::ShadowTypeInfo *shadowInfo) override;	///<add a non-shadow decal
		virtual Shadow	*addDecal(Shadow::ShadowTypeInfo *shadowInfo) override;	///<add a non-shadow decal which does not follow an object.
		W3DProjectedShadow	*addShadow( W3DRenderObject *robj, Shadow::ShadowTypeInfo *shadowInfo, Drawable *draw);	///<add a new shadow with texture of given name or that of robj.
		W3DProjectedShadow	*createDecalShadow( Shadow::ShadowTypeInfo *shadowInfo);	///<add a new shadow with texture of given name or that of robj.
		void removeShadow (W3DProjectedShadow *shadow);
		void removeAllShadows(); ///< Remove all shadows.
		W3DTextureHandle *getRenderTarget()	{ return m_dynamicRenderTarget;}
		W3DRenderContext *getRenderContext()	{ return m_shadowContext;}
		void updateRenderTargetTextures();	///<render into any textures that need updating.
		void queueDecal(W3DProjectedShadow *shadow);	///<add shadow decal to render list - decal conforms to terrain.
		void flushDecals(W3DShadowTexture *texture, ShadowType type);	///<empty queue by rendering all decals with given texture

	private:
		Int renderProjectedTerrainShadow(W3DProjectedShadow *shadow, AABoxClass &box);	///<render shadow on map terrain.
		void updateShadowNumbers(ShadowType shadowType, Int addNum);

	private:
        struct GraphicsState;
        GraphicsState* m_graphics;
		W3DProjectedShadow *m_shadowList;
		W3DProjectedShadow *m_decalList;
		W3DTextureHandle	*m_dynamicRenderTarget;	///<offscreen video memory texture used to render all shadow textures.
		Bool m_renderTargetHasAlpha;					///<does render target have destination alpha support?
		W3DCamera		*m_shadowCamera;					///<camera used to render all shadow textures - configured by projector
		Graphics::LocalLighting m_shadowLightEnv;
		W3DRenderContext *m_shadowContext;
		W3DShadowTextureManager *m_W3DShadowTextureManager;
		Int m_numDecalShadows;							///< number of decal shadows in the system.
		Int m_numProjectionShadows;						///< number of projected shadows in the system.

		//Bounding rectangle around rendered portion of terrain.
		Int m_drawEdgeX;
		Int m_drawEdgeY;
		Int m_drawStartX;
		Int m_drawStartY;
};

extern W3DProjectedShadowManager *TheW3DProjectedShadowManager;

/** Object for maintaining and updating an object's shadow texture.
*/
class W3DProjectedShadow	: public Shadow
{
	friend class W3DProjectedShadowManager;

	public:
		W3DProjectedShadow();
		~W3DProjectedShadow();
		void setRenderObject( W3DRenderObject	*robj) {m_robj=robj;}
		void setObjPosHistory(const Vector3 &pos)	{m_lastObjPosition=pos;}	///<position of object when projection matrix was updated.
		void setTexture(Int lightIndex,W3DShadowTexture *texture)	{m_shadowTexture[lightIndex]=texture;}	///<texture with light's shadow
		void update();	///<updates the texture and/or projection parameters when the object or light moves.
		void init();		///<allocates local member variables used for projection
		void updateTexture(Vector3 &lightPos);	///<updates the shadow texture image using render object and given light position.
		void updateProjectionParameters(const Matrix3D &cameraXform);	///<recompute projection matrix - needed when light or object moves.
		Graphics::TextureMapping *getShadowMapping() const { return m_shadowMapping.get(); }
		Graphics::MeshMaterial *getShadowMaterial() const { return m_shadowMaterial.get(); }
		#if defined(RTS_DEBUG)
		virtual void getRenderCost(RenderCost & rc) const override;
		#endif
		W3DShadowTexture *getTexture(Int lightIndex) {return m_shadowTexture[lightIndex];}


	protected:
		W3DShadowTexture *m_shadowTexture[MAX_SHADOW_LIGHTS];		///<cached shadow data
		std::shared_ptr<Graphics::TextureMapping> m_shadowMapping;
		std::shared_ptr<Graphics::MeshMaterial> m_shadowMaterial;
		Graphics::TextureProjectorFit m_shadowFit;
		W3DRenderObject	*m_robj;						///<render object used to cast the shadow.
		Vector3		m_lastObjPosition;	///<position of  object at time of projection matrix update.
		W3DProjectedShadow *m_next;	/// for the shadow manager list
		Bool	m_allowWorldAlign;	/// wrap shadow around world geometry - else align perpendicular to local z-axis.
		Real	m_decalOffsetU;		/// texture coordinate offset so not centered at object origin.
		Real	m_decalOffsetV;		/// texture coordinate offset so not centered at object origin.
		Int		m_flags;			/// custom rendering flags
		virtual void release() override	{TheW3DProjectedShadowManager->removeShadow(this);}	///<release shadow from manager
};
