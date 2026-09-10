import Graphics.Frame.RenderClock;
#include "W3DDevice/GameClient/W3DRenderServices.h"
#include "W3DDevice/GameClient/W3DTerrainGraphics.h"
#include "W3DDevice/GameClient/W3DGraphicsResources.h"

#include <algorithm>
#include <cmath>
#include <span>

#include "Common/GlobalData.h"
#include "GameClient/Water.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"

#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DCustomScene.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/W3DLight.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DTreeBuffer.h"
#include "W3DDevice/GameClient/W3DPropBuffer.h"
#include "W3DDevice/GameClient/W3DRoadBuffer.h"
#include "W3DDevice/GameClient/W3DBridgeBuffer.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DWaypointBuffer.h"
#include "W3DDevice/GameClient/W3DBibBuffer.h"

import Graphics.Scene.DrawParameters;
import Graphics.Frame.Runtime;
import Graphics.Scene.Lighting.Environment;
import Graphics.Scene.Shadows.DirectionalRenderer;
import Graphics.Materials.ProceduralPass;

W3DTerrainGraphics *W3DTerrainGraphics::s_active = nullptr;

W3DTerrainGraphics::W3DTerrainGraphics() { s_active = this; }
W3DTerrainGraphics::~W3DTerrainGraphics()
{
    freeMapResources();
    if (s_active == this) s_active = nullptr;
}

void W3DTerrainGraphics::Release_Graphics() noexcept
{
    if (s_active != nullptr) {
        s_active->Release_Texture_References();
        s_active->scheduleFullUpdate();
    }
}

void W3DTerrainGraphics::Release_Texture_References() noexcept
{
    auto& environment = Graphics::Get_Environment_Lighting();
    environment.cloud_texture = {};
    environment.parameters.cloud_offset_strength[3] = 0;
    if (m_graphicsDevice != nullptr)
        for (Graphics::RHITextureHandle texture : m_textures)
            if (texture.Is_Valid()) m_graphicsDevice->Destroy_Texture(texture);
    m_textures = {};
    m_graphicsDevice = nullptr;
}

void W3DTerrainGraphics::ReleaseResources()
{
    Release_Texture_References();
    BaseHeightMapRenderObjClass::ReleaseResources();
    scheduleFullUpdate();
}

void W3DTerrainGraphics::ReAcquireResources()
{
    BaseHeightMapRenderObjClass::ReAcquireResources();
    scheduleFullUpdate();
}

Int W3DTerrainGraphics::freeMapResources()
{
    Release_Graphics_Textures();
    Release_Texture_References();
    Graphics::Get_Terrain_Renderer().Release_Surface();
    Graphics::Get_Terrain_Overlay_Renderer().Release_Surface();
    Graphics::Get_Terrain_Shoreline_Renderer().Release_Surface();
    m_extraCells = 0;
    return BaseHeightMapRenderObjClass::freeMapResources();
}

int W3DTerrainGraphics::initHeightData(Int width, Int height, WorldHeightMap *map,
    Graphics::SceneObjectList<W3DRenderObject>::Cursor *lights, Bool extra)
{
    const int result = BaseHeightMapRenderObjClass::initHeightData(width, height, map, lights, extra);
    m_x = width;
    m_y = height;
    scheduleFullUpdate();
    if (result != 0) return result;
    // Finish map resources while the loading screen is still active. Atlas
    // placement must precede UV extraction, including shadow preparation.
    return Update_Textures() && Update_Surface() ? 0 : -1;
}

void W3DTerrainGraphics::doPartialUpdate(const IRegion2D &range, WorldHeightMap *map, Graphics::SceneObjectList<W3DRenderObject>::Cursor *lights)
{
    updateBlock(range.lo.x, range.lo.y, range.hi.x, range.hi.y, map, lights);
}

int W3DTerrainGraphics::updateBlock(Int, Int, Int, Int, WorldHeightMap *map, Graphics::SceneObjectList<W3DRenderObject>::Cursor *)
{
    REF_PTR_SET(m_map, map);
    Invalidate_Cached_Bounding_Volumes();
    if (map != nullptr) updateShorelineTiles(0, 0, map->getXExtent() - 1, map->getYExtent() - 1, map);
    scheduleFullUpdate();
    return 0;
}

void W3DTerrainGraphics::updateCenter(W3DCamera *camera, const Vector3 *pivot, Graphics::SceneObjectList<W3DRenderObject>::Cursor *lights)
{
    if (m_map == nullptr || m_updating) return;
    // This window belongs to the remaining scene consumers (roads and
    // shroud). The graphics terrain surface covers the complete map.
    if (pivot != nullptr) {
        const int border = m_map->getBorderSizeInline();
        m_map->setDrawOrg(static_cast<int>(std::floor(pivot->X / MAP_XY_FACTOR)) + border - m_x / 2,
            static_cast<int>(std::floor(pivot->Y / MAP_XY_FACTOR)) + border - m_y / 2);
    }
    BaseHeightMapRenderObjClass::updateCenter(camera, pivot, lights);
}

void W3DTerrainGraphics::oversizeTerrain(Int tiles)
{
    m_oversizeWidth = m_oversizeHeight = tiles > 0 ? tiles + 1 : 0;
    setTerrainDrawSize(0, 0);
}

void W3DTerrainGraphics::setTerrainDrawSize(Int width, Int height)
{
    if (width > 0) m_requestedWidth = width;
    if (height > 0) m_requestedHeight = height;
    if (m_map == nullptr) return;
    m_x = std::min(m_map->getXExtent(), std::max(m_requestedWidth, m_oversizeWidth));
    m_y = std::min(m_map->getYExtent(), std::max(m_requestedHeight, m_oversizeHeight));
    m_map->setDrawWidth(m_x);
    m_map->setDrawHeight(m_y);
}

Int W3DTerrainGraphics::getNumExtraBlendTiles(Bool) { return m_extraCells; }

bool W3DTerrainGraphics::Update_Textures()
{
    Graphics::Device *device = Graphics::Shared_Frame_Device();
    if (device == nullptr || m_map == nullptr) return false;
    if (m_graphicsDevice != device) {
        Release_Texture_References();
        m_graphicsDevice = device;
        scheduleFullUpdate();
    }
    REF_PTR_SET(m_stageZeroTexture, m_map->getTerrainTexture());
    REF_PTR_SET(m_stageOneTexture, m_map->getAlphaTerrainTexture());
    const std::array<W3DTextureHandle *, 6> sources{
        m_stageZeroTexture, m_stageOneTexture, m_stageTwoTexture, m_stageThreeTexture,
        m_shroud != nullptr ? m_shroud->getShroudTexture() : nullptr, m_destAlphaTexture};
    for (std::size_t slot = 0; slot < sources.size(); ++slot) {
        W3DTextureHandle* texture = sources[slot];
        const auto handle = texture != nullptr && texture->Ensure_Render_Backend_Texture() && !texture->Is_Missing_Texture()
            ? texture->Peek_Graphics_Texture() : Graphics::RHITextureHandle{};
        if (handle == m_textures[slot]) continue;
        if (handle.Is_Valid() && !device->Retain_Texture(handle)) return false;
        if (m_textures[slot].Is_Valid()) device->Destroy_Texture(m_textures[slot]);
        m_textures[slot] = handle;
    }
    return m_disableTextures || (m_textures[0].Is_Valid() && m_textures[1].Is_Valid());
}

bool W3DTerrainGraphics::Update_Surface()
{
    if (!m_needFullUpdate) return true;
    if (m_map == nullptr) return false;
    const int width = m_map->getXExtent();
    const int height = m_map->getYExtent();
    if (width < 2 || height < 2) return false;
    const int border = m_map->getBorderSizeInline();
    std::vector<Graphics::TerrainCell> cells;
    std::vector<Graphics::TerrainCell> overlays;
    cells.reserve(static_cast<std::size_t>(width - 1) * (height - 1));
    constexpr int corner_x[4]{0, 1, 1, 0};
    constexpr int corner_y[4]{0, 0, 1, 1};
    for (int y = 0; y < height - 1; ++y) {
        for (int x = 0; x < width - 1; ++x) {
            Graphics::TerrainCell cell;
            cell.origin = {static_cast<float>((x - border) * MAP_XY_FACTOR), static_cast<float>((y - border) * MAP_XY_FACTOR)};
            cell.spacing = {MAP_XY_FACTOR, MAP_XY_FACTOR};
            float u[4]{}, v[4]{}, blend_u[4]{}, blend_v[4]{};
            UnsignedByte alpha[4]{};
            Bool flip = FALSE;
            m_map->getUVData(x - m_map->getDrawOrgX(), y - m_map->getDrawOrgY(), u, v);
            m_map->getAlphaUVData(x - m_map->getDrawOrgX(), y - m_map->getDrawOrgY(), blend_u, blend_v, alpha, &flip);
            cell.alternate_diagonal = flip != FALSE;
            for (int corner = 0; corner < 4; ++corner) {
                const int cx = x + corner_x[corner], cy = y + corner_y[corner];
                cell.heights[corner] = Get_Surface_Height(cx, cy);
                cell.normals[corner] = Graphics::Terrain_Surface_Normal(
                    Get_Surface_Height(std::max(0, cx - 1), cy),
                    Get_Surface_Height(std::min(width - 1, cx + 1), cy),
                    Get_Surface_Height(cx, std::max(0, cy - 1)),
                    Get_Surface_Height(cx, std::min(height - 1, cy + 1)), MAP_XY_FACTOR);
                const UnsignedInt color = getStaticDiffuse(cx, cy);
                cell.colors[corner] = {((color >> 16) & 255) / 255.0f,
                    ((color >> 8) & 255) / 255.0f, (color & 255) / 255.0f, alpha[corner] / 255.0f};
                cell.base_uv[corner] = {u[corner], v[corner]};
                cell.blend_uv[corner] = {blend_u[corner], blend_v[corner]};
            }
            if (m_showImpassableAreas) {
                const float high_x = (width - 2 * border) * MAP_XY_FACTOR;
                const float high_y = (height - 2 * border) * MAP_XY_FACTOR;
                const bool boundary = cell.origin[0] == -MAP_XY_FACTOR || cell.origin[1] == -MAP_XY_FACTOR
                    || cell.origin[0] == high_x || cell.origin[1] == high_y;
                const bool cliff = m_map->getCliffState(x, y) || showAsVisibleCliff(x, y);
                const bool mapped = m_map->isCliffMappedTexture(x - m_map->getDrawOrgX(), y - m_map->getDrawOrgY());
                for (int corner = 0; corner < 4; ++corner) {
                    auto& color = cell.colors[corner];
                    const float px = cell.origin[0] + corner_x[corner] * MAP_XY_FACTOR;
                    const float py = cell.origin[1] + corner_y[corner] * MAP_XY_FACTOR;
                    const bool edge = (py >= 0 && py <= high_y && (px == 0 || px == high_x))
                        || (px >= 0 && px <= high_x && (py == 0 || py == high_y));
                    if (boundary && edge) color[0] = color[1] = 0;
                    else if (!boundary && cliff) color[1] = color[2] = 0;
                    if (mapped && corner == (flip ? 1 : 0)) {
                        color[0] = color[2] = 0;
                        color[1] = 1;
                    }
                }
            }
            cells.push_back(cell);
            Bool cliff = FALSE;
            if (m_map->getExtraAlphaUVData(x, y, u, v, alpha, &flip, &cliff)) {
                cell.alternate_diagonal = flip || (cliff && std::abs(cell.heights[0] - cell.heights[2]) > std::abs(cell.heights[1] - cell.heights[3]));
                for (int corner = 0; corner < 4; ++corner) {
                    cell.base_uv[corner] = {u[corner], v[corner]};
                    cell.colors[corner][3] = alpha[corner] / 255.0f;
                }
                overlays.push_back(cell);
            }
        }
    }
    if (!Graphics::Get_Terrain_Renderer().Set_Cells(cells)
        || !Graphics::Get_Terrain_Overlay_Renderer().Set_Cells(overlays)) return false;
    m_extraCells = static_cast<Int>(overlays.size());
    m_needFullUpdate = false;
    return true;
}

float W3DTerrainGraphics::Get_Surface_Height(int x, int y) const
{
    return m_map->getHeight(x, y) * MAP_HEIGHT_SCALE;
}

Bool W3DTerrainGraphics::collectShadowCasters()
{
    if (!Update_Textures() || !Update_Surface()) return FALSE;
    const Matrix4x4 transform(Transform);
    std::array<float,16> world;
    for (unsigned row=0;row<4;++row)
        for (unsigned column=0;column<4;++column)
            world[row*4+column] = transform[row][column];
    return Graphics::Get_Terrain_Renderer().Add_Shadow_Caster(
        Graphics::Get_Directional_Shadow_Renderer(),world)
        && BaseHeightMapRenderObjClass::collectShadowCasters();
}

bool W3DTerrainGraphics::Draw_Surface(W3DRenderContext &info)
{
    if (!Update_Textures() || !Update_Surface()) return false;
    Graphics::TerrainDrawParameters parameters;
    Matrix3D camera_view;
    Matrix4x4 projection;
    info.Camera.Get_View_Matrix(&camera_view);
    info.Camera.Get_Backend_Projection_Matrix(&projection);
    const Matrix4x4 transform = projection * Matrix4x4(camera_view) * Matrix4x4(Transform);
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            parameters.view_projection[row * 4 + column] = transform[row][column];
    const bool cloud = useCloud() && m_textures[2].Is_Valid();
    const float stretch = 1.0f / (63.0f * MAP_XY_FACTOR / 2.0f);
    parameters.cloud_projection = {stretch, stretch, cloud ? m_stageTwoTexture->Get_X_Offset() : 0.0f,
        cloud ? m_stageTwoTexture->Get_Y_Offset() : 0.0f};
    auto& environment = Graphics::Get_Environment_Lighting();
    environment.cloud_texture = cloud ? m_textures[2] : Graphics::RHITextureHandle{};
    const auto& light = TheGlobalData->m_terrainLightPos[0];
    const float inverse_z = std::abs(light.z) > 0.0001f ? 1.0f/light.z : 0.0f;
    environment.parameters.cloud_multiplier = {stretch, stretch,
        stretch*light.x*inverse_z, stretch*light.y*inverse_z};
    environment.parameters.cloud_offset_strength = {parameters.cloud_projection[2],
        parameters.cloud_projection[3], 1.0f, cloud ? 1.0f : 0.0f};
    environment.parameters.ambient = {};
    for (const auto& ambient : TheGlobalData->m_terrainAmbient) {
        environment.parameters.ambient[0] += ambient.red;
        environment.parameters.ambient[1] += ambient.green;
        environment.parameters.ambient[2] += ambient.blue;
    }
    parameters.lightmap_projection = {stretch, stretch, 0, 0};
    parameters.features = {cloud ? 1.0f : 0.0f,
        TheGlobalData->m_useLightMap && m_textures[3].Is_Valid() ? 1.0f : 0.0f,
        m_disableTextures ? 1.0f : 0.0f, 0};
    if (m_shroud != nullptr && m_textures[4].Is_Valid()
        && m_shroud->getCellWidth() > 0 && m_shroud->getCellHeight() > 0
        && m_shroud->getTextureWidth() > 0 && m_shroud->getTextureHeight() > 0) {
        const float sx = 1.0f / (m_shroud->getCellWidth() * m_shroud->getTextureWidth());
        const float sy = 1.0f / (m_shroud->getCellHeight() * m_shroud->getTextureHeight());
        parameters.shroud_projection = {sx, sy,
            (-m_shroud->getDrawOriginX() + m_shroud->getCellWidth()) * sx,
            (-m_shroud->getDrawOriginY() + m_shroud->getCellHeight()) * sy};
        parameters.options[1] = 1;
    }
    if (Scene != nullptr) {
        RTS3DScene *scene = static_cast<RTS3DScene *>(Scene);
        Graphics::SceneObjectList<W3DRenderObject>::Cursor lights(scene->getDynamicLights());
        std::size_t count = 0;
        for (lights.First(); !lights.Is_Done() && count < parameters.lights.size(); lights.Next()) {
            W3DDynamicLight *light = static_cast<W3DDynamicLight *>(lights.Peek_Obj());
            if (!light->isEnabled()) continue;
            const Vector3 position = light->Get_Position();
            double inner, outer;
            light->Get_Far_Attenuation_Range(inner, outer);
            const float min_x = (m_map->getDrawOrgX() - m_map->getBorderSizeInline()) * MAP_XY_FACTOR;
            const float min_y = (m_map->getDrawOrgY() - m_map->getBorderSizeInline()) * MAP_XY_FACTOR;
            if (light->Get_Type() != W3DLight::DIRECTIONAL &&
                (position.X + outer < min_x || position.Y + outer < min_y ||
                 position.X - outer > min_x + m_x * MAP_XY_FACTOR || position.Y - outer > min_y + m_y * MAP_XY_FACTOR)) continue;
            Vector3 diffuse, ambient, direction;
            light->Get_Diffuse(&diffuse);
            light->Get_Ambient(&ambient);
            light->Get_Spot_Direction(direction);
            Graphics::TerrainLight &output = parameters.lights[count++];
            output.position_range = {position.X, position.Y, position.Z, static_cast<float>(outer)};
            output.diffuse_inner = {diffuse.X, diffuse.Y, diffuse.Z, static_cast<float>(inner)};
            output.ambient_kind = {ambient.X, ambient.Y, ambient.Z, light->Get_Type() == W3DLight::DIRECTIONAL ? 1.0f : 0.0f};
            output.direction = {direction.X, direction.Y, direction.Z, 0};
        }
        parameters.light_options[0] = static_cast<float>(count);
    }
    Graphics::CommandList &commands = m_graphicsDevice->Immediate_Command_List();
    const bool filtered = TheGlobalData->m_bilinearTerrainTex || TheGlobalData->m_trilinearTerrainTex;
    const bool wireframe = Graphics::Get_Scene_Draw_Parameters().wireframe;
    RTS3DScene *render_scene = static_cast<RTS3DScene *>(info.Camera.Get_User_Data());
    if (render_scene != nullptr && render_scene->getCustomPassMode() == SCENE_PASS_ALPHA_MASK) {
        if (info.Additional_Pass_Count() == 0) return false;
        NativeMaterialPass::Description description;
        NativeMaterialPass *pass = info.Peek_Additional_Pass(info.Additional_Pass_Count() - 1);
        if (pass == nullptr || !pass->Describe(description)
            || description.textures[0] == nullptr) return false;
        W3DTextureHandle *texture = description.textures[0];
        if (!texture->Ensure_Render_Backend_Texture()) return false;
        const auto mask = texture->Peek_Graphics_Texture();
        if (!m_graphicsDevice->Retain_Texture(mask)) return false;
        parameters.shroud_projection = {description.world_texture_transform[0], description.world_texture_transform[5],
            description.world_texture_transform[3], description.world_texture_transform[7]};
        parameters.features = {0, 0, 0, 5};
        parameters.light_options[0] = 0;
        std::array<Graphics::RHITextureHandle, 5> textures{};
        textures[4] = mask;
        const bool result = Graphics::Get_Terrain_Renderer().Render(commands,
            Graphics::TerrainSurfacePass::Mask, parameters, textures);
        m_graphicsDevice->Destroy_Texture(mask);
        return result;
    }
    if (!Graphics::Get_Terrain_Renderer().Render(commands, Graphics::TerrainSurfacePass::Surface,
        parameters, std::span<const Graphics::RHITextureHandle>(m_textures.data(), 5), filtered, wireframe)) return false;
    if (TheGlobalData->m_use3WayTerrainBlends) {
        parameters.features[3] = 3;
        if (TheGlobalData->m_use3WayTerrainBlends == 2) parameters.features[2] = 1;
        parameters.light_options[0] = 0;
        const std::array<Graphics::RHITextureHandle, 5> overlay_textures{
            m_textures[1], {}, m_textures[2], m_textures[3], m_textures[4]};
        if (!Graphics::Get_Terrain_Overlay_Renderer().Render(commands, Graphics::TerrainSurfacePass::Overlay, parameters, overlay_textures, filtered, wireframe)) return false;
    }
    if (TheGlobalData->m_showSoftWaterEdge && TheWaterTransparency->m_transparentWaterDepth != 0 && m_textures[5].Is_Valid()) {
        std::vector<Graphics::TerrainCell> shoreline;
        shoreline.reserve(m_numShoreLineTiles);
        for (int index = 0; index < m_numShoreLineTiles; ++index) {
            const shoreLineTileInfo &source = m_shoreLineTilePositions[index];
            Graphics::TerrainCell cell;
            cell.origin = {source.verts[0], source.verts[1]};
            cell.spacing = {MAP_XY_FACTOR, MAP_XY_FACTOR};
            for (int corner = 0; corner < 4; ++corner) cell.heights[corner] = source.verts[corner * 3 + 2];
            cell.base_uv = {{{source.t0, 0}, {source.t1, 0}, {source.t2, 0}, {source.t3, 0}}};
            shoreline.push_back(cell);
        }
        parameters.features[3] = 4;
        const std::array<Graphics::RHITextureHandle, 1> coverage_texture{m_textures[5]};
        if (!Graphics::Get_Terrain_Shoreline_Renderer().Set_Cells(shoreline)
            || !Graphics::Get_Terrain_Shoreline_Renderer().Render(commands, Graphics::TerrainSurfacePass::Shoreline, parameters, coverage_texture)) return false;
        m_numVisibleShoreLineTiles = static_cast<Int>(shoreline.size());
    }
    return true;
}

void W3DTerrainGraphics::Render(W3DRenderContext &info)
{
    if (m_map == nullptr || Is_Hidden()) return;
    if (useCloud() && m_stageTwoTexture != nullptr && !Get_W3D_Render_Services().Is_Reflection_Render_Pass())
        m_stageTwoTexture->Update_Animation(Graphics::Get_Render_Clock().Logic_Frame_Time_Seconds());
    if (m_treeBuffer != nullptr) m_treeBuffer->setIsTerrain();

    if (Graphics::Shared_Frame_Device() == nullptr) return;
    const bool rendered = Draw_Surface(info);
    if (!rendered) {
        DEBUG_LOG(("Terrain graphics submission failed.\n"));
        return;
    }
    RTS3DScene *render_scene = static_cast<RTS3DScene *>(info.Camera.Get_User_Data());
    if (render_scene != nullptr && render_scene->getCustomPassMode() == SCENE_PASS_ALPHA_MASK) return;
    Graphics::Get_Scene_Draw_Parameters().color_write_mask = 0x07;
    const Bool cloud = useCloud();
    if (!Get_W3D_Render_Services().Is_Reflection_Render_Pass() && Scene != nullptr && m_roadBuffer != nullptr) {
        RTS3DScene *scene = static_cast<RTS3DScene *>(Scene);
        Graphics::SceneObjectList<W3DRenderObject>::Cursor lights(scene->getDynamicLights());
        const int border = m_map->getBorderSizeInline();
        m_roadBuffer->drawRoads(&info.Camera, cloud ? m_stageTwoTexture : nullptr,
            TheGlobalData->m_useLightMap ? m_stageThreeTexture : nullptr, m_disableTextures,
            m_map->getDrawOrgX() - border, m_map->getDrawOrgX() + m_x - 1 - border,
            m_map->getDrawOrgY() - border, m_map->getDrawOrgY() + m_y - 1 - border, &lights);
    }
    if (m_propBuffer != nullptr) m_propBuffer->drawProps(info);
    drawScorches(info.Camera);
    if (m_bridgeBuffer != nullptr) m_bridgeBuffer->drawBridges(&info.Camera, m_disableTextures, cloud ? m_stageTwoTexture : nullptr);
    if (TheTerrainTracksRenderObjClassSystem != nullptr) TheTerrainTracksRenderObjClassSystem->flush(info.Camera);
    if (m_waypointBuffer != nullptr) m_waypointBuffer->drawWaypoints(info);
    if (m_bibBuffer != nullptr) m_bibBuffer->renderBibs(info.Camera);
}
