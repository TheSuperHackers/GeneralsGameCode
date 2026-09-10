/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#pragma once

#include <array>
#include <span>
#include <vector>
#include "WWLib/always.h"
#include "Lib/BaseType.h"
#include "Common/GameType.h"
#include "W3DDevice/GameClient/WaterMaterial.h"
#include "W3DDevice/GameClient/WaterGeometry.h"
#include "W3DDevice/GameClient/WaterReflectionRenderer.h"

#define INVALID_WATER_HEIGHT 0.0f

class W3DCamera;
class AABoxClass;
class W3DRenderContext;
class W3DTextureHandle;
class WaterTracksRenderSystem;
class WaterSkyboxSystem;

// Water render system. It is submitted explicitly by RTS3DScene after
// opaque scene rendering; it is not a legacy W3DRenderObject scene node.
class WaterRenderSystem
{
public:
	enum WaterType
	{
		WATER_TYPE_SURFACE = 0,
		WATER_TYPE_SURFACE_VARIANT,
		WATER_TYPE_OCEAN,
		WATER_TYPE_GRID,
	};

	WaterRenderSystem();
	~WaterRenderSystem();

	void Render(W3DRenderContext &rinfo);
	void renderWater();
	void Set_Surface_Geometry(const WaterGeometry &geometry);
	void Set_Grid_Render_Data(const WaterGridRenderData &data);
	void Set_World_Position(Real x, Real y, Real z);
	void Rebuild_Grid_Geometry();
	Int init(Real waterLevel, Real dx, Real dy,
		WaterReflectionRenderer *reflectionRenderer, WaterType type);
	void reset();
	void load();
	void update();
	void updateMapOverrides();
	void setTimeOfDay(TimeOfDay tod);
	void toggleCloudLayer(Bool state) { m_useCloudLayer = state; }
	void updateRenderTargetTextures(W3DCamera *cam);
	void Capture_Refraction_Texture();
	void ReleaseResources();
	void ReAcquireResources();
	Real getWaterHeight(Real x, Real y);
	void replaceSkyboxTexture(const AsciiString &oldTexName,
		const AsciiString &newTextName);

protected:
	WaterReflectionRenderer *m_reflectionRenderer;
	W3DTextureHandle *m_skyBodyTexture;
	Real m_dx;
	Real m_dy;
	Real m_level;
    Real m_reflectionHeight = 0;
	Real m_worldPositionX;
	Real m_worldPositionY;
	Real m_worldPositionZ;
	Real m_uOffset;
	Real m_vOffset;
	Real m_uScrollPerMs;
	Real m_vScrollPerMs;
	Int m_LastUpdateTime;
	Bool m_useCloudLayer;
	WaterType m_waterType;

    std::vector<WaterSurfaceVertex> m_gridVertices;
    std::vector<UnsignedShort> m_gridIndices;
    Graphics::WaterMeshHandle m_gridMesh;
    Graphics::WaterMeshHandle m_surfaceMesh;

	Int m_numVertices;
	Int m_numIndices;
	W3DTextureHandle *m_pReflectionTexture;
	Graphics::RHITextureHandle m_sceneColorTexture;
	Graphics::RHITextureHandle m_sceneDepthTexture{};
	WaterSkyboxSystem *m_skyBox;
	WaterTracksRenderSystem *m_waterTrackSystem;

	Real m_riverVOrigin;
	Real m_waterTime;
	W3DTextureHandle *m_riverTexture;
	W3DTextureHandle *m_whiteTexture;
	W3DTextureHandle *m_waterNoiseTexture;
	W3DTextureHandle *m_waterOceanHeightTexture;
	W3DTextureHandle *m_waterOceanNormalTexture;
	W3DTextureHandle *m_waterEnvironmentTexture;
	W3DTextureHandle *m_waterCausticsTexture;
	W3DTextureHandle *m_waterDepthLutTexture;
	W3DTextureHandle *m_waterSparklesTexture;
	Real m_riverXOffset;
	Real m_riverYOffset;
	Bool m_drawingRiver = false;
	Bool m_renderingOffscreen;
	W3DTextureHandle *m_riverAlphaEdge;
	WaterGeometry m_surfaceGeometry;
    Vector4 m_surfaceDomain{0,0,0,0};
    std::vector<std::vector<Graphics::WaterMeshHandle>> m_surfaceMeshes;
    std::vector<Graphics::WaterBodyMotion> m_waterBodies;
	WaterGridRenderData m_gridRenderData;
	WaterMaterialClass m_waterMaterial;

	TimeOfDay m_tod;

	struct Setting
	{
		W3DTextureHandle *skyTexture;
		W3DTextureHandle *waterTexture;
		Int waterRepeatCount;
		Real skyTexelsPerUnit;
		std::uint32_t vertex00Diffuse;
		std::uint32_t vertex10Diffuse;
		std::uint32_t vertex11Diffuse;
		std::uint32_t vertex01Diffuse;
		std::uint32_t waterDiffuse;
		std::uint32_t transparentWaterDiffuse;
		Real uScrollPerMs;
		Real vScrollPerMs;
	};

	Setting m_settings[TIME_OF_DAY_COUNT];
	void drawRiverWater(const WaterSurfacePolygon &polygon);
    void rebuildSurfaceMeshes();
    Vector4 getDisplacementDomain() const;
    void updateWaterBodies();
    void drawSurfaceMesh(Graphics::WaterMeshHandle mesh);
	void loadSetting(Setting *skySetting, TimeOfDay timeOfDay);
	void updateTextureAnimation();
	void renderUnderwater(W3DRenderContext &rinfo, bool draw_grid);
	void testCurvedWater();
	bool updateGridGeometry();
	void renderWaterMesh();
	void renderMirror(W3DCamera *cam);
	void drawSea(W3DRenderContext &rinfo);
	bool updateDisplacementTexture();
	Bool getClippedWaterPlane(W3DCamera *cam, AABoxClass *box);
	WaterMaterialParameters makeWaterMaterialParameters(bool river,
		bool reflection, bool underwater) const;

	bool generateIndexBuffer(int sizeX, int sizeY);
	bool generateVertexBuffer(Int sizeX, Int sizeY, Bool doFill);
	std::uint32_t getSurfaceDiffuse(bool reduce_alpha) const;
	bool uploadSurfaceGeometry(const WaterSurfaceVertex *vertices,
		unsigned vertex_count, const UnsignedShort *indices, unsigned index_count);
};

extern WaterRenderSystem *TheWaterRenderSystem;
