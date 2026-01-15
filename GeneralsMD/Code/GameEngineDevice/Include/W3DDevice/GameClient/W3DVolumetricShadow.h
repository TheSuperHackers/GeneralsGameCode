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

#pragma once

#include "matrix4.h"
#include "W3DDevice/GameClient/W3DBufferManager.h"
#include "GameClient/Shadow.h"

///@todo Make the 100 below a 'better' number. Was 32, increased because of overcomplex models.
#define MAX_SHADOW_CASTER_MESHES	160	//number of meshes allowed in animated hierarchy (must be <256 since index is a byte).

class W3DShadowGeometry;	//forward reference
class W3DShadowGeometryManager;	//forward reference
struct Geometry;	//forward reference
struct PolyNeighbor;	//forward reference
class W3DVolumetricShadow;	//forward reference
class Drawable;	//forward reference

struct W3DVolumetricShadowRenderTask : public W3DBufferManager::W3DRenderTask
{
	W3DVolumetricShadow	*m_parentShadow;		///<main casting object to which this volume belongs.
	UnsignedByte		m_meshIndex;		///<mesh index of volume within parent to render.
	UnsignedByte		m_lightIndex;		///<light index of volume within parent to render.
};

// ShadowManager -------------------------------------------------------------
class W3DVolumetricShadowManager
{

public:

	W3DVolumetricShadowManager( void );
	~W3DVolumetricShadowManager( void );
	Bool init( void );	///<initialize resources used by manager, must have valid D3D device.
	// shadow list management
	void reset( void );
	W3DVolumetricShadow* addShadow( RenderObjClass *robj, Shadow::ShadowTypeInfo *shadowInfo, Drawable *draw);	///< adds shadow caster to rendering system.
	void removeShadow(W3DVolumetricShadow *shadow);	///< removed shadow from rendering system and frees its resources.
	void removeAllShadows(void); ///< Remove all shadows.
	/// queues up a dynamic shadow caster for rendering - only used internally by shadow system.
	void addDynamicShadowTask(W3DVolumetricShadowRenderTask *task)
	{	W3DBufferManager::W3DRenderTask *oldTask=m_dynamicShadowVolumesToRender;
		m_dynamicShadowVolumesToRender=task;
		m_dynamicShadowVolumesToRender->m_nextTask=oldTask;
	}
	void invalidateCachedLightPositions(void);	///<forces shadow volumes to update regardless of last lightposition
	void loadTerrainShadows(void);

	// rendering
	void renderShadows( Bool forceStencilFill );
	void ReleaseResources(void);
	Bool ReAcquireResources(void);

protected:

		// to render the stencil buffer polygon to the screen
		void renderStencilShadows( void );

		W3DVolumetricShadow *m_shadowList;
		W3DVolumetricShadowRenderTask *m_dynamicShadowVolumesToRender;
		W3DShadowGeometryManager *m_W3DShadowGeometryManager;
};

extern W3DVolumetricShadowManager *TheW3DVolumetricShadowManager;

// W3DVolumetricShadow ---------------------------------------------------------------------
class W3DVolumetricShadow	: public Shadow
{
	friend class W3DVolumetricShadowManager;

	enum
	{
		SHADOW_DYNAMIC	=	0x1			//flag indicating a dynamic shadow caster (animated mesh).
	};

	public:

		W3DVolumetricShadow( void );
		~W3DVolumetricShadow( void );

	protected:

		virtual void release(void)	{TheW3DVolumetricShadowManager->removeShadow(this);}	///<release shadow from manager

		#if defined(RTS_DEBUG)
		virtual void getRenderCost(RenderCost & rc) const;
		#endif

		// tie in geometry and transformation for this shadow
		void SetGeometry( W3DShadowGeometry *geometry );
		void setShadowLengthScale(Real value) {m_shadowLengthScale = value;}
		void updateOptimalExtrusionPadding(void);	///<for immobile objects, figure out the length of extrusion needed to hit ground - expensive!
		void setOptimalExtrusionPadding(Real value)	{m_extraExtrusionPadding=value;}
		const W3DShadowGeometry *getGeometry(void) {return m_geometry;}

		void setRenderObject( RenderObjClass	*robj) {assert(m_robj==nullptr); m_robj=robj;}
 		void setRenderObjExtent ( Real extent) { m_robjExtent = extent; }

		// called once per frame, updates shadow volume when necessary
		void Update();
		void updateVolumes(Real zoffset);	///<update shadow volumes of all meshes in this model
		void updateMeshVolume(Int meshIndex, Int lightIndex, const Matrix3D *meshXform, const AABoxClass &meshBox, float floorZ);///<update shadow volume of this mesh.

		// rendering interface
		void RenderVolume(Int meshIndex, Int lightIndex);	///<renders a specifc volume from the model hierarchy
		///render single mesh which could belong to a larger hierarchy (optimized for static meshes).
		void RenderMeshVolume(Int meshIndex, Int lightIndex, const Matrix3D *meshXform);
		///render single mesh which could belong to a larger hierarchy (optimized for animated meshes).
		void RenderDynamicMeshVolume(Int meshIndex, Int lightIndex, const Matrix3D *meshXform);
		void RenderMeshVolumeBounds(Int meshIndex, Int lightIndex, const Matrix3D *meshXform);	///<render bounding volume around shadow volume - for debug use.

		void setLightPosHistory(Int lightIndex, Int meshIndex, Vector3 &pos) {m_lightPosHistory[lightIndex][meshIndex]=pos;}	///<updates the last position of light
		W3DVolumetricShadow *m_next;	/// for the shadow manager list

		// silhouette tools
		void buildSilhouette(Int meshIndex, Vector3 *lightPosWorld);
		void addSilhouetteEdge(Int meshIndex, PolyNeighbor *visible, PolyNeighbor *hidden );
		void addNeighborlessEdges(Int meshIndex, PolyNeighbor *us );
		void addSilhouetteIndices(Int meshIndex, Short edgeStart, Short edgeEnd );
		Bool allocateSilhouette(Int meshIndex, Int numVertices );  // allocate memory for sil
		void deleteSilhouette(Int meshIndex );  // resets and frees silhouette memory
		void resetSilhouette( Int meshIndex );  // reset silhouette to empty

		// shadow volume access
		void constructVolume( Vector3 *lightPos, Real shadowExtrudeDistance, Int volumeIndex, Int meshIndex );
		void constructVolumeVB( Vector3 *lightPosObject,Real shadowExtrudeDistance, Int volumeIndex, Int meshIndex );
		Bool allocateShadowVolume( Int volumeIndex, Int meshIndex );  // allocate mem
		void deleteShadowVolume( Int volumeIndex );  // delete all volume data
		void resetShadowVolume( Int volumeIndex, Int meshIndex );  // reset shadow volume

		// this is our current geometry we will derive the shadow from
		W3DShadowGeometry *m_geometry;

		RenderObjClass	*m_robj;	///<render object belonging to model casting shadow

		Real	m_shadowLengthScale;	///<scale factor used to reduce/clamp shadow length
 		Real	m_robjExtent;			///<maximum possible horizontal reach of shadow from center of object.
		Real	m_extraExtrusionPadding;	///<amount to extrude shadow beyond where it would normally stop (ground level below object).

		//
		// this geometry is our actual shadow volume data, we have an array
		// of them so we can have separate shadow volume for each light source
		// casting shadows
		//

		static Geometry m_tempShadowVolume;	//scratch buffer to use by all shadows during volume construction.
		Geometry *m_shadowVolume[ MAX_SHADOW_LIGHTS ][MAX_SHADOW_CASTER_MESHES];
		W3DBufferManager::W3DVertexBufferSlot *m_shadowVolumeVB[ MAX_SHADOW_LIGHTS ][MAX_SHADOW_CASTER_MESHES];
		W3DBufferManager::W3DIndexBufferSlot *m_shadowVolumeIB[ MAX_SHADOW_LIGHTS ][MAX_SHADOW_CASTER_MESHES];
		W3DVolumetricShadowRenderTask m_shadowVolumeRenderTask[ MAX_SHADOW_LIGHTS ][MAX_SHADOW_CASTER_MESHES];
		Int m_shadowVolumeCount[MAX_SHADOW_CASTER_MESHES];  // how man shadows are valid in m_shadowVolume
		Vector3 m_lightPosHistory[ MAX_SHADOW_LIGHTS ][MAX_SHADOW_CASTER_MESHES];
		Matrix4x4 m_objectXformHistory[ MAX_SHADOW_LIGHTS ][MAX_SHADOW_CASTER_MESHES];

		// silhouette building space
		Short *m_silhouetteIndex[MAX_SHADOW_CASTER_MESHES];  // silhouette vertex index list, edges occur
									 // as disjoint pairs.  The acutal size of this
									 // piece of memory must accomodate #vertices*2
		Short m_numSilhouetteIndices[MAX_SHADOW_CASTER_MESHES];  // total number of edge indices in the index
									   // array, these are pairs and therefore
									   // always a multiple of two
		Short m_maxSilhouetteEntries[MAX_SHADOW_CASTER_MESHES];  // how big the silhouette index can hold max

		Int	  m_numIndicesPerMesh[MAX_SHADOW_CASTER_MESHES];	///<silhouette indices from each mesh.

};
