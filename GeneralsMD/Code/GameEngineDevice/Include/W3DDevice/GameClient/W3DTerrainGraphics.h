#pragma once

#include "W3DDevice/GameClient/BaseHeightMap.h"
#include <array>
#include <vector>

import Graphics.Scene.Terrain.Renderer;

// Keeps the existing map/query and scene lifecycle while translating terrain
// surface data into the graphics subsystem.
class W3DTerrainGraphics : public BaseHeightMapRenderObjClass
{
public:
    W3DTerrainGraphics();
    ~W3DTerrainGraphics() override;
    void Render(W3DRenderContext &info) override;
    Bool collectShadowCasters() override;
    int initHeightData(Int width, Int height, WorldHeightMap *map, Graphics::SceneObjectList<W3DRenderObject>::Cursor *lights, Bool extra = TRUE) override;
    Int freeMapResources() override;
    void ReleaseResources() override;
    void ReAcquireResources() override;
    void updateCenter(W3DCamera *camera, const Vector3 *pivot, Graphics::SceneObjectList<W3DRenderObject>::Cursor *lights) override;
    void doPartialUpdate(const IRegion2D &range, WorldHeightMap *map, Graphics::SceneObjectList<W3DRenderObject>::Cursor *lights) override;
    int updateBlock(Int x0, Int y0, Int x1, Int y1, WorldHeightMap *map, Graphics::SceneObjectList<W3DRenderObject>::Cursor *lights) override;
    void oversizeTerrain(Int tiles) override;
    void setTerrainDrawSize(Int width, Int height) override;
    Int getNumExtraBlendTiles(Bool visible) override;
    static void Release_Graphics() noexcept;

protected:
    virtual float Get_Surface_Height(int x, int y) const;

private:
    bool Update_Surface();
    bool Update_Textures();
    bool Draw_Surface(W3DRenderContext &info);
    void Release_Texture_References() noexcept;
    std::array<Graphics::RHITextureHandle, 6> m_textures{};
    Graphics::Device *m_graphicsDevice = nullptr;
    Int m_extraCells = 0;
    Int m_requestedWidth = WorldHeightMap::NORMAL_DRAW_WIDTH;
    Int m_requestedHeight = WorldHeightMap::NORMAL_DRAW_HEIGHT;
    Int m_oversizeWidth = 0;
    Int m_oversizeHeight = 0;
    static W3DTerrainGraphics *s_active;
};
