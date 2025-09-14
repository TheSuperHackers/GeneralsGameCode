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


#pragma once

#ifndef __HEIGHTMAP_H_
#define __HEIGHTMAP_H_

#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "shader.h"
#include "vertmaterial.h"
#include "Lib/BaseType.h"
#include "Common/GameType.h"
#include "WorldHeightMap.h"

#define MAX_ENABLED_DYNAMIC_LIGHTS 20
typedef UnsignedByte HeightSampleType;	//type of data to store in heightmap
class W3DTreeBuffer;
class W3DBibBuffer;
class W3DRoadBuffer;
class W3DBridgeBuffer;
class W3DWaypointBuffer;
class W3DTerrainLogic;
class W3DCustomEdging;
class W3DAssetManager;
class SimpleSceneClass;
class W3DShroud;

#define no_TIMING_TESTS	1

#define no_PRE_TRANSFORM_VERTEX // Don't do this, not a performance win.  jba.

#define no_USE_TREE_BUFFER	///@todoRe-enable this optimization later... jba.

typedef struct {
	Int minX, maxX;
	Int minY, maxY;
} TBounds;

#define VERTEX_BUFFER_TILE_LENGTH	32		//tiles of side length 32 (grid of 33x33 vertices).

class LightMapTerrainTextureClass;
class CloudMapTerrainTextureClass;
class W3DDynamicLight;

#if 0
#define USE_NORMALS	1
#define VERTEX_FORMAT VertexFormatXYZNUV2
#else
#define USE_DIFFUSE	1
#define VERTEX_FORMAT VertexFormatXYZDUV2
#define DX8_VERTEX_FORMAT DX8_FVF_XYZDUV2
#endif

#define DO_SCORCH 1

#define DO_ROADS 1

#define TEST_CUSTOM_EDGING	1

// Adjust the triangles to make cliff sides most attractive.  jba.
#define FLIP_TRIANGLES 1

#ifdef DO_SCORCH
typedef struct {
	Vector3 location;
	Real		radius;
	Int			scorchType;
} TScorch;
#endif

/// Custom render object that draws the heightmap and handles intersection tests.
/**
Custom W3D render object that's used to process the terrain.  It handles
virtually everything to do with the terrain, including: drawing, lighting,
scorchmarks and intersection tests.
*/
class HeightMapRenderObjClass : public RenderObjClass, public DX8_CleanupHook
{

public:

	HeightMapRenderObjClass(void);
	~HeightMapRenderObjClass(void);

	// DX8_CleanupHook methods
	virtual void ReleaseResources(void);	///< Release all dx8 resources so the device can be reset.
	virtual void ReAcquireResources(void);  ///< Reacquire all resources after device reset.


	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface (W3D methods)
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
	virtual bool					Cast_Ray(RayCollisionTestClass & raytest); // This CANNOT be Bool, as it will not inherit properly if you make Bool == Int
///@todo: Add methods for collision detection with terrain
//	virtual Bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
//	virtual Bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
//	virtual Bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
//	virtual Bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);

	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
	virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & aabox) const;


	virtual void					On_Frame_Update(void);
	virtual void					Notify_Added(SceneClass * scene);

	///allocate resources needed to render heightmap
	int initHeightData(Int width, Int height, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator);
	Int freeMapResources(void);	///< free resources used to render heightmap
	inline UnsignedByte getClipHeight(Int x, Int y) const
	{
		Int xextent = m_map->getXExtent() - 1;
		Int yextent = m_map->getYExtent() - 1;

		if (x < 0)
			x = 0;
		else if (x > xextent)
			x = xextent;

		if (y < 0)
			y = 0;
		else if (y > yextent)
			y = yextent;

		return m_map->getDataPtr()[x + y*m_map->getXExtent()];
	}
	void updateCenter(CameraClass *camera, RefRenderObjListIterator *pLightsIterator);

	/// Update the macro texture (pass 3).
	void updateMacroTexture(AsciiString textureName);
	void doTextures(Bool flag) {m_disableTextures = !flag;};
	/// Update the diffuse value from static light info for one vertex.
	void doTheLight(VERTEX_FORMAT *vb, Vector3*light, Vector3*normal, RefRenderObjListIterator *pLightsIterator, UnsignedByte alpha);
	void addScorch(Vector3 location, Real radius, Scorches type);
	void addTree(Coord3D location, Real scale, Real angle, AsciiString name, Bool visibleInMirror);
	void renderTrees(CameraClass * camera); ///< renders the tree buffer.

	/// Add a bib at location.
	void addTerrainBib(Vector3 corners[4], ObjectID id, Bool highlight);
	void addTerrainBibDrawable(Vector3 corners[4], DrawableID id, Bool highlight);
	/// Remove a bib.
	void removeTerrainBib(ObjectID id);
	void removeTerrainBibDrawable(DrawableID id);

	/// Removes all bibs.
	void removeAllTerrainBibs(void);
	/// Remove all highlighting.
	void removeTerrainBibHighlighting(void);

	void renderTerrainPass(CameraClass *pCamera);	///< renders additional terrain pass.
	W3DShroud *getShroud()	{return m_shroud;}
	void updateShorelineTiles(Int minX, Int minY, Int maxX, Int maxY, WorldHeightMap *pMap);	///<figure out which tiles on this map cross water plane
	void updateViewImpassableAreas(Bool partial = FALSE, Int minX = 0, Int maxX = 0, Int minY = 0, Int maxY = 0);
	void clearAllScorches(void);
	void setTimeOfDay( TimeOfDay tod );
	void staticLightingChanged(void);
	void loadRoadsAndBridges(W3DTerrainLogic *pTerrainLogic, Bool saveGame); ///< Load the roads from the map objects.
	void worldBuilderUpdateBridgeTowers( W3DAssetManager *assetManager, SimpleSceneClass *scene );							///< for the editor updating of bridge tower visuals
	Int  getStaticDiffuse(Int x, Int y); ///< Gets the diffuse terrain lighting value for a point on the mesh.
	void adjustTerrainLOD(Int adj);
	void reset(void);
	void doPartialUpdate(const IRegion2D &partialRange, WorldHeightMap *htMap, RefRenderObjListIterator *pLightsIterator);

	Int	getNumExtraBlendTiles(void) { return m_numExtraBlendTiles;}
	Int getNumShoreLineTiles(void)	{ return m_numShoreLineTiles;}
	void setShoreLineDetail(void);	///<update shoreline tiles in case the feature was toggled by user.
	Bool getMaximumVisibleBox(const FrustumClass &frustum,  AABoxClass *box, Bool ignoreMaxHeight);	///<3d extent of visible terrain.
	Real getHeightMapHeight(Real x, Real y, Coord3D* normal) const;	///<return height and normal at given point
	Bool isCliffCell(Real x, Real y);	///<return height and normal at given point
	Real getMinHeight(void) const {return m_minHeight;}	///<return minimum height of entire terrain
	Real getMaxHeight(void) const {return m_maxHeight;}	///<return maximum height of entire terrain
	Real getMaxCellHeight(Real x, Real y) const;	///< returns maximum height of the 4 cell corners.
	WorldHeightMap *getMap(void) {return m_map;}	///< returns object holding the heightmap samples - need this for fast access.
	Bool isClearLineOfSight(const Coord3D& pos, const Coord3D& posOther) const;

	Bool getShowImpassableAreas(void) {return m_showImpassableAreas;}
	void setShowImpassableAreas(Bool show) {m_showImpassableAreas = show;}

	Bool showAsVisibleCliff(Int xIndex, Int yIndex) const;

	Bool evaluateAsVisibleCliff(Int xIndex, Int yIndex, Real valuesGreaterThanRad);

	void oversizeTerrain(Int tilesToOversize);

	Real getViewImpassableAreaSlope(void) const { return m_curImpassableSlope; }
	void setViewImpassableAreaSlope(Real viewSlope) { m_curImpassableSlope = viewSlope; }

	Bool doesNeedFullUpdate(void) {return m_needFullUpdate;}

protected:
#ifdef DO_SCORCH
	enum { MAX_SCORCH_VERTEX=8194,
					MAX_SCORCH_INDEX=6*8194,
					MAX_SCORCH_MARKS=500,
					SCORCH_MARKS_IN_TEXTURE=9,
					SCORCH_PER_ROW = 3};
	DX8VertexBufferClass	*m_vertexScorch;	///<Scorch vertex buffer.
	DX8IndexBufferClass			*m_indexScorch;	///<indices defining a triangles for the scorch drawing.
	TextureClass *m_scorchTexture;	///<Scorch mark texture
	Int			m_curNumScorchVertices;	 ///<number of vertices used in m_vertexScorch.
	Int			m_curNumScorchIndices;	 ///<number of indices used in m_indexScorch.
	TScorch	m_scorches[MAX_SCORCH_MARKS];
	Int			m_numScorches;

	Int			m_scorchesInBuffer;		///< how many are in the buffers.  If less than numScorches, we need to update

	// NOTE: This argument (contrary to most of the rest of the engine), is in degrees, not radians.
	Real		m_curImpassableSlope;

	void updateScorches(void);	 ///<Update m_vertexScorch and m_indexScorch so all scorches will be drawn.
	void allocateScorchBuffers(void);	 ///<allocate static buffers for drawing scorch marks.
	void freeScorchBuffers(void);		 ///< frees up scorch buffers.
	void drawScorches(void);		///< Draws the scorch mark polygons in m_vertexScorch.
#endif
	WorldHeightMap *m_map;
	Int	m_x;	///< dimensions of heightmap
	Int	m_y;	///< dimensions of heightmap
	Int m_originX; ///<  Origin point in the grid.  Slides around.
	Int m_originY; ///< Origin point in the grid.  Slides around.
	Bool m_useDepthFade;	///<fade terrain lighting under water
	Vector3 m_depthFade;	///<depth based fall off values for r,g,b
	Bool m_disableTextures;
	Bool m_needFullUpdate; ///< True if lighting changed, and we need to update all instead of what moved.
	Bool m_updating;	 ///< True if we are updating vertex buffers.  (can't draw while buffers are locked.)
	Bool m_halfResMesh; ///< True if we are doing half resolution mesh of the terrain.
	Bool m_doXNextTime; ///< True if we updated y scroll, and need to do x scroll next frame.
	Real	m_minHeight;	///<minimum value of height samples in heightmap
	Real	m_maxHeight;	///<maximum value of height samples in heightmap
	Int	m_numVBTilesX;	///<dimensions of array containing all the vertex buffers
	Int	m_numVBTilesY;	///<dimensions of array containing all the vertex buffers
	Int m_numVertexBufferTiles;	///<number of vertex buffers needed to store this heightmap
	Int	m_numBlockColumnsInLastVB;///<a VB tile may be partially filled, this indicates how many 2x2 vertex blocks are filled.
	Int	m_numBlockRowsInLastVB;///<a VB tile may be partially filled, this indicates how many 2x2 vertex blocks are filled.
	Bool m_showImpassableAreas; ///< If true, shade impassable areas.

	// STL is "smart." This is a variable sized bitset. Very memory efficient.
	std::vector<bool> m_showAsVisibleCliff;


	DX8IndexBufferClass			*m_indexBuffer;	///<indices defining triangles in a VB tile.
#ifdef PRE_TRANSFORM_VERTEX
	IDirect3DVertexBuffer8 **m_xformedVertexBuffer;
#endif
	ShaderClass m_shaderClass; ///<shader or rendering state for heightmap
	VertexMaterialClass	  	  *m_vertexMaterialClass;	///< vertex shader (lighting) for terrain
	TextureClass *m_stageZeroTexture;	///<primary texture
	TextureClass *m_stageOneTexture;	///<transparent edging texture
	CloudMapTerrainTextureClass *m_stageTwoTexture;	///<Cloud map texture
	TextureClass *m_stageThreeTexture;	///<light/noise map texture
	AsciiString m_macroTextureName; ///< Name for stage 3 texture.
	TextureClass *m_destAlphaTexture;	///< Texture holding destination alpha LUT for water depth.
	DX8VertexBufferClass	**m_vertexBufferTiles;	///<collection of smaller vertex buffers that make up 1 heightmap
	char	**m_vertexBufferBackup;	///< In memory copy of the vertex buffer data for quick update of dynamic lighting.

	W3DTreeBuffer *m_treeBuffer; ///< Class for drawing trees and other alpha objects.
	W3DBibBuffer *m_bibBuffer; ///< Class for drawing trees and other alpha objects.
	W3DWaypointBuffer *m_waypointBuffer; ///< Draws waypoints.
#ifdef TEST_CUSTOM_EDGING
	W3DCustomEdging *m_customEdging; ///< Class for drawing custom edging.
#endif
#ifdef DO_ROADS
	W3DRoadBuffer *m_roadBuffer; ///< Class for drawing roads.
#endif
	W3DBridgeBuffer *m_bridgeBuffer;
	W3DShroud *m_shroud;	///< Class for drawing the shroud over terrain.
	Int *m_extraBlendTilePositions;	///<array holding x,y tile positions of all extra blend tiles. (used for 3 textures per tile).
	Int m_numExtraBlendTiles;		///<number of blend tiles in m_extraBlendTilePositions.
	Int m_extraBlendTilePositionsSize;	//<total size of array including unused memory.
	struct shoreLineTileInfo
	{	Int m_xy;	//x,y position of tile
		Real t0,t1,t2,t3;	//index into water depth alpha LUT.
	};
	shoreLineTileInfo *m_shoreLineTilePositions;	///<array holding x,y tile positions of all shoreline terrain.
	Int m_numShoreLineTiles;		///<number of tiles in m_shoreLineTilePositions.
	Int m_shoreLineTilePositionsSize;	///<total size of array including unused memory.
	Real m_currentMinWaterOpacity;		///<current value inside the gradient lookup texture.
	/// Update the diffuse value from dynamic light info for one vertex.
	UnsignedInt doTheDynamicLight(VERTEX_FORMAT *vb, VERTEX_FORMAT *vbMirror, Vector3*light, Vector3*normal, W3DDynamicLight *pLights[], Int numLights);
	Int getXWithOrigin(Int x);
	Int getYWithOrigin(Int x);
	///update vertex diffuse color for dynamic lights inside given rectangle
	Int updateVBForLight(DX8VertexBufferClass *pVB, char *data, Int x0, Int y0, Int x1, Int y1, Int originX, Int originY, W3DDynamicLight *pLights[], Int numLights);
	Int updateVBForLightOptimized(DX8VertexBufferClass	*pVB, char *data, Int x0, Int y0, Int x1, Int y1, Int originX, Int originY, W3DDynamicLight *pLights[], Int numLights);
	///update vertex buffer vertices inside given rectangle
	Int updateVB(DX8VertexBufferClass	*pVB, char *data, Int x0, Int y0, Int x1, Int y1, Int originX, Int originY, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator);
	///upate vertex buffers associated with the given rectangle
	int updateBlock(Int x0, Int y0, Int x1, Int y1, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator);
	AABoxClass & getTileBoundingBox(AABoxClass *aabox, Int x, Int y);	///<Vertex buffer bounding box
	void initDestAlphaLUT(void);	///<initialize water depth LUT stored in m_destAlphaTexture
	void renderShoreLines(CameraClass *pCamera);	///<re-render parts of terrain that need custom blending into water edge
	void renderExtraBlendTiles(void);	///< render 3-way blend tiles that have blend of 3 textures.

	static Bool useCloud();
};

extern HeightMapRenderObjClass *TheTerrainRenderObject;
#endif  // end __HEIGHTMAP_H_
