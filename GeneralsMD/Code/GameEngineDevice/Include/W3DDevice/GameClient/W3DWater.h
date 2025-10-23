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

// FILE: W3DWater.h ///////////////////////////////////////////////////

#pragma once

#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "shader.h"
#include "vertmaterial.h"
#include "light.h"
#include "Lib/BaseType.h"
#include "Common/GameType.h"
#include "Common/Snapshot.h"

#define INVALID_WATER_HEIGHT 0.0f	///water height guaranteed to be below all terrain.

#define NUM_BUMP_FRAMES 32	///number of animation frames in bump map
//Offsets in constant register file to Vertex shader constants
#define CV_ZERO 0
#define CV_ONE 1
#define CV_WORLDVIEWPROJ_0 2
#define CV_TEXPROJ_0 6
#define CV_PATCH_SCALE_OFFSET 10

class PolygonTrigger;
class WaterTracksRenderSystem;
class Xfer;
/// Custom render object that draws mirrors, water, and skies.
/**
This render object handles drawing reflected W3D scenes.  It will only work
with rectangular planar surfaces and was tuned with an emphasis on water.
Since skies are only visible in reflections, this code will also
render clouds and sky bodies.
*/
class WaterRenderObjClass : public Snapshot,
														public RenderObjClass
{

public:

	enum WaterType
	{
		WATER_TYPE_0_TRANSLUCENT = 0,	//translucent water, no reflection
		WATER_TYPE_1_FB_REFLECTION,		//legacy frame buffer reflection (non translucent)
		WATER_TYPE_2_PVSHADER,		//pixel/vertex shader, texture reflection
		WATER_TYPE_3_GRIDMESH,		//3D Mesh based water
	};

	WaterRenderObjClass(void);
	~WaterRenderObjClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface (W3D methods)
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
/// @todo: Add methods for collision detection with water surface
//	virtual Bool					Cast_Ray(RayCollisionTestClass & raytest);
//	virtual Bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
//	virtual Bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
//	virtual Bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
//	virtual Bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);

	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
    virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & aabox) const;
	// Get and set static sort level
	virtual int		Get_Sort_Level(void) const		{ return m_sortLevel; }
  	virtual void	Set_Sort_Level(int level)		{ m_sortLevel = level;}

	///allocate W3D resources needed to render water
	void renderWater(void);				///<draw the water surface (flat)
	Int init(Real waterLevel, Real dx, Real dy, SceneClass *parentScene, WaterType type);
	void reset( void );  ///< reset any resources we need to
	void load(void);	///< load/setup any map dependent features
	void update( void ); ///< update phase of the water
	void enableWaterGrid(Bool state);	///< used to active custom water for special maps. (i.e DAM).
	void updateMapOverrides(void);	///< used to update any map specific map overrides for water appearance.
	void setTimeOfDay(TimeOfDay tod); ///<change sky/water for time of day
	void toggleCloudLayer(Bool state)	{	m_useCloudLayer=state;}	///<enables/disables the cloud layer
	void updateRenderTargetTextures(CameraClass *cam);	///< renders into any required textures.
	void ReleaseResources(void);	///< Release all dx8 resources so the device can be reset.
	void ReAcquireResources(void);  ///< Reacquire all resources after device reset.
	Real getWaterHeight(Real x, Real y);	///<return water height at given point - for use by WB.
	void setGridHeightClamps(Real minz, Real maxz);	///<set min/max height values alllowed in grid
	void addVelocity( Real worldX, Real worldY, Real zVelocity, Real preferredHeight );	///< add velocity value
	void changeGridHeight(Real x, Real y, Real delta);	///<change height of grid at world point including neighbors within falloff.
	void setGridChangeAttenuationFactors(Real a, Real b, Real c, Real range);	///<adjust falloff parameters for grid change method.
	void setGridTransform(Real angle, Real x, Real y, Real z);	///<positoin/orientation of grid in space
	void setGridTransform(const Matrix3D *transform);	///< set transform by matrix
	void getGridTransform(Matrix3D *transform);	///< get grid transform matrix
	void setGridResolution(Real gridCellsX, Real gridCellsY, Real cellSize);	///<grid resolution and spacing
	void getGridResolution(Real *gridCellsX, Real *gridCellsY, Real *cellSize);  ///<get grid resolution params
	inline void setGridVertexHeight(Int x, Int y, Real value);
	inline void getGridVertexHeight(Int x, Int y, Real *value)
	{	if (m_meshData)	*value=m_meshData[(y+1)*(m_gridCellsX+1+2)+x+1].height+ Get_Position().Z;}
	inline Bool worldToGridSpace(Real worldX, Real worldY, Real &gridX, Real &gridY);	///<convert from world coordinates to grid's local coordinate system.

	void replaceSkyboxTexture(const AsciiString& oldTexName, const AsciiString& newTextName);

protected:
	DX8IndexBufferClass			*m_indexBuffer;	///<indices defining quad
	SceneClass							*m_parentScene;	///<scene to be reflected
	ShaderClass m_shaderClass; ///<shader or rendering state for heightmap
	VertexMaterialClass	  		*m_vertexMaterialClass;	///<vertex lighting material
	VertexMaterialClass			*m_meshVertexMaterialClass; ///<vertex lighting marial for 3D water.
	LightClass					*m_meshLight;				///<light used for 3D Mesh Water.
	TextureClass *m_alphaClippingTexture;	///<used for faked clipping using alpha
	Real	m_dx;	///<x extent of water surface (offset from local center)
	Real	m_dy;	///<y extent of water surface (offset from local center)
	Vector3 m_planeNormal;		///<water plane normal
	Real		m_planeDistance;	///<water plane distance
	Real		m_level;			///<level of water (hack for water)
	Real		m_uOffset;			///<current texture offset on u axis
	Real		m_vOffset;			///<current texture offset on v axis
	Real		m_uScrollPerMs;		///<texels per/ms scroll rate in u direction
	Real		m_vScrollPerMs;		///<texels per/ms scroll rate in v direction
	Int			m_LastUpdateTime;	///<time of last cloud update
	Bool		m_useCloudLayer;	///<flag if clouds are on/off
	WaterType	m_waterType;		///<type of water being used
	Int			m_sortLevel;		///<sort order after main scene is rendered

	//Data used in GeForce3 bump-mapped water (uses direct D3D resources for better
	//performance and compatibility (most of these featues are not supported by W3D).
	struct SEA_PATCH_VERTEX	//vertex structure passed to D3D
	{
		float x,y,z;
		unsigned int c;
		float tu, tv;
	};

	LPDIRECT3DDEVICE8 m_pDev;						///<pointer to D3D Device
	LPDIRECT3DVERTEXBUFFER8 m_vertexBufferD3D;		///<D3D vertex buffer
	LPDIRECT3DINDEXBUFFER8	m_indexBufferD3D;	///<D3D index buffer
	Int						m_vertexBufferD3DOffset;	///<location to start writing vertices
	DWORD					m_dwWavePixelShader;	///<handle to D3D pixel shader
	DWORD					m_dwWaveVertexShader;	///<handle to D3D vertex shader
	Int	m_numVertices;				///<number of vertices in D3D vertex buffer
	Int m_numIndices;				///<number of indices in D3D index buffer
	LPDIRECT3DTEXTURE8 m_pBumpTexture[NUM_BUMP_FRAMES]; ///<animation frames
	LPDIRECT3DTEXTURE8 m_pBumpTexture2[NUM_BUMP_FRAMES]; ///<animation frames
	Real				m_fBumpFrame;	///<current animation frame
	Real				m_fBumpScale;	///<scales bump map uv perturbation
	TextureClass * m_pReflectionTexture;	///<render target for reflection
	RenderObjClass	*m_skyBox;		///<box around level
	WaterTracksRenderSystem *m_waterTrackSystem;	///<object responsible for rendering water wakes

	enum WaterMeshStatus
	{
		AT_REST = 0x00,
		IN_MOTION = 0x01
	};
	struct WaterMeshData
	{
		Real height;										///< height of the 3D mesh at this point
		Real velocity;									///< velocity in Z that this point is moving up and down
		UnsignedByte status;						///< status for this grid point
		UnsignedByte preferredHeight;		///< the hight we prefer to be
	};
	WaterMeshData *m_meshData;  ///< heightmap data for 3D Mesh based water.
	UnsignedInt m_meshDataSize;	///< size of m_meshData
	Bool m_meshInMotion;				///< TRUE once we've messed with velocities and are in motion
	Bool m_doWaterGrid;	///< allows/prevents water grid rendering.

	Vector2	m_gridDirectionX;			///<vector along water grid's x-axis (scaled to world-space)
	Vector2	m_gridDirectionY;			///<vector along water grid's y-axis (scaled to world-space)
	Vector2	m_gridOrigin;				///<unit vector along water grid's x-axis
	Real m_gridWidth;					///<object-space width of water grid
	Real m_gridHeight;					///<object-space width of water grid
	Real m_minGridHeight;				///<minimum height value allowed for water mesh
	Real m_maxGridHeight;				///<maximum height value allowed for water mesh
	Real m_gridChangeMaxRange;			///<maximum range of changeGridHeight() method
	Real m_gridChangeAtt0;
	Real m_gridChangeAtt1;
	Real m_gridChangeAtt2;
	Real m_gridCellSize;				///<world-space width/height of each cell.
	Int  m_gridCellsX;					///<number of cells along width
	Int  m_gridCellsY;					///<number of cells along height

	Real m_riverVOrigin;
	TextureClass *m_riverTexture;
	TextureClass *m_whiteTexture;		///< a texture containing only white used for NULL pixel shader stages.
	TextureClass *m_waterNoiseTexture;
	DWORD	m_waterPixelShader;		///<D3D handle to pixel shader.
	DWORD	m_riverWaterPixelShader;		///<D3D handle to pixel shader.
	DWORD	m_trapezoidWaterPixelShader;	///<handle to D3D vertex shader
	TextureClass *m_waterSparklesTexture;
	Real m_riverXOffset;
	Real m_riverYOffset;
	Bool m_drawingRiver;
	Bool m_disableRiver;
	TextureClass *m_riverAlphaEdge;

	TimeOfDay m_tod;	///<time of day setting for reflected cloud layer

	struct Setting
	{
		TextureClass	*skyTexture;
		TextureClass	*waterTexture;
		Int				waterRepeatCount;
		Real			skyTexelsPerUnit;	//texel density of sky plane (higher value repeats texture more).
		DWORD			vertex00Diffuse;
		DWORD			vertex10Diffuse;
		DWORD			vertex11Diffuse;
		DWORD			vertex01Diffuse;
		DWORD			waterDiffuse;
		DWORD			transparentWaterDiffuse;
		Real			uScrollPerMs;
		Real			vScrollPerMs;
	};

	Setting m_settings[ TIME_OF_DAY_COUNT ];	///< settings for each time of day
	void drawRiverWater(PolygonTrigger *pTrig);
	void drawTrapezoidWater(Vector3 points[4]);
	void loadSetting ( Setting *skySetting, TimeOfDay timeOfDay );	///<init sky/water settings from GDF
	void renderSky(void);	///<draw the sky layer (clouds, stars, etc.)
	void testCurvedWater(void);	///<draw the sky layer (clouds, stars, etc.)
	void renderSkyBody(Matrix3D *mat);	///<draw the sky body (sun, moon, etc.)
	void renderWaterMesh(void);			///<draw the water surface mesh (deformed 3d mesh).
	HRESULT initBumpMap(LPDIRECT3DTEXTURE8 *pTex, TextureClass *pBumpSource);	///<copies data into bump-map format.
	void renderMirror(CameraClass *cam);	///< Draw reflected scene into texture
	void drawSea(RenderInfoClass & rinfo);	///< Draw the surface of the water
	///bounding box of frustum clipped polygon plane
	Bool getClippedWaterPlane(CameraClass *cam, AABoxClass *box);

	void setupFlatWaterShader(void);
	void setupJbaWaterShader(void);
	void cleanupJbaWaterShader(void);

	//Methods used for GeForce3 specific water
	HRESULT generateIndexBuffer(int sizeX, int sizeY);	///<Generate static index buufer
	HRESULT generateVertexBuffer( Int sizeX, Int sizeY, Int vertexSize, Bool doFill);///<Generate static vertex buffer

	// snapshot methods for save/load
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

};

//Public inline function declarations
inline Bool WaterRenderObjClass::worldToGridSpace(Real worldX, Real worldY, Real &gridX, Real &gridY)
{
	Real dx,dy;
	Real ooGridCellSize = 1.0f/m_gridCellSize;

	dx=worldX - m_gridOrigin.X;
	dy=worldY - m_gridOrigin.Y;
	gridX = ooGridCellSize * (dx * m_gridDirectionX.X + dy * m_gridDirectionX.Y);
	gridY = ooGridCellSize * (dx * m_gridDirectionY.X + dy * m_gridDirectionY.Y);

	if (gridX < 0)
		return FALSE;
	if (gridX > m_gridCellsX-1 )
		return FALSE;
	if (gridY < 0)
		return FALSE;
	if (gridY > m_gridCellsY-1 )
		return FALSE;

	return TRUE;
}

extern WaterRenderObjClass *TheWaterRenderObj; ///<global water rendering object
