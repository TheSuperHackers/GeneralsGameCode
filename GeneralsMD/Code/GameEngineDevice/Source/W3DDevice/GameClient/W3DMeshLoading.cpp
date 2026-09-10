#include <functional>
import Graphics.Frame.RenderClock;
import Graphics.Frame.RenderSettings;
#include "W3DDevice/GameClient/W3DRenderServices.h"
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
#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include "W3DDevice/GameClient/W3DMeshResource.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"

#include "WWLib/chunkio.h"
#include "WWLib/RANDOM.h"
import Assets.Adapters.W3D.MeshData;
import Assets.Adapters.W3D.Materials;
import Assets.Adapters.W3D.Geometry;
import Assets.Adapters.W3D.Chunks;
import Assets.Images.PixelEncoding;
import Graphics.RHI;
import Graphics.Resources.Textures.Sampling;
import Graphics.Scene.Models.W3DMaterialLoading;
import Graphics.Scene.Models.MeshMaterialPreparation;
namespace {
Random4Class texture_mapping_random;
float Sample_Texture_Mapping() { return texture_mapping_random.Get_Float(); }
float Texture_Bump_Sine(float angle) { return WWMath::Fast_Sin(angle); }
float Texture_Bump_Cosine(float angle) { return WWMath::Fast_Cos(angle); }
RefCountPtr<W3DTextureHandle> Acquire_Texture(const Assets::W3D::W3DTextureData& decoded) {
    W3DTextureHandle *newtex = nullptr;
    if (decoded.has_info)
    {

        MipCountType mipcount;

        bool no_lod = ((decoded.attributes & Assets::W3D::W3DTextureAttributeNoLod)
            == Assets::W3D::W3DTextureAttributeNoLod);

        if (no_lod)
        {
            mipcount = MIP_LEVELS_1;
        }
        else
        {
            switch (decoded.attributes & Assets::W3D::W3DTextureAttributeMipLevelsMask) {

                case Assets::W3D::W3DTextureAttributeMipLevelsAll:
                    mipcount = MIP_LEVELS_ALL;
                    break;

                case Assets::W3D::W3DTextureAttributeMipLevels2:
                    mipcount = MIP_LEVELS_2;
                    break;

                case Assets::W3D::W3DTextureAttributeMipLevels3:
                    mipcount = MIP_LEVELS_3;
                    break;

                case Assets::W3D::W3DTextureAttributeMipLevels4:
                    mipcount = MIP_LEVELS_4;
                    break;

                default:
                    WWASSERT (false);
                    mipcount = MIP_LEVELS_ALL;
                    break;
            }
        }

        Assets::PixelEncoding format=Assets::PixelEncoding::Unknown;

        switch (decoded.attributes & Assets::W3D::W3DTextureAttributeTypeMask)
        {

            case Assets::W3D::W3DTextureAttributeTypeColorMap:
                // Do nothing.
                break;

            case Assets::W3D::W3DTextureAttributeTypeBumpMap:
            {
                if (Get_W3D_Render_Services().Is_Initialized())
                {
                    // No mipmaps to bumpmap for now
                    mipcount=MIP_LEVELS_1;

                    format=Assets::PixelEncoding::RG8_SNorm;
                }
                break;
            }

            default:
                WWASSERT (false);
                break;
        }

        newtex = W3DAssetCatalog::Get_Instance()->Get_Texture (decoded.name.c_str(), mipcount, format);

        if (no_lod)
        {
            newtex->Get_Sampling().mipmap = Graphics::SamplingFilter::Disabled;
        }
        bool u_clamp = ((decoded.attributes & Assets::W3D::W3DTextureAttributeClampU) != 0);
        newtex->Get_Sampling().address[0] = u_clamp ? Graphics::RHISamplerAddress::Clamp : Graphics::RHISamplerAddress::Wrap;
        bool v_clamp = ((decoded.attributes & Assets::W3D::W3DTextureAttributeClampV) != 0);
        newtex->Get_Sampling().address[1] = v_clamp ? Graphics::RHISamplerAddress::Clamp : Graphics::RHISamplerAddress::Wrap;

    } else
    {
        newtex = W3DAssetCatalog::Get_Instance()->Get_Texture(decoded.name.c_str());
    }

    WWASSERT(newtex);
    return RefCountPtr<W3DTextureHandle>::Create_No_Add_Ref(newtex);
}
}

bool W3DMeshResource::Load_W3D(ChunkLoadClass& cload) {
    std::vector<std::byte> bytes(cload.Cur_Chunk_Length());
    if (cload.Read(bytes.data(), static_cast<unsigned>(bytes.size())) != bytes.size()) return false;
    Assets::W3D::W3DMeshData mesh;
    std::string error;
    const auto preference = static_cast<Assets::W3D::W3DMeshPrelighting>(static_cast<int>(Graphics::Get_Render_Settings().Get_Prelit_Mode()) + 1);
    if (!Assets::W3D::W3DRead_Mesh_Data(bytes, preference, mesh, error)
        || !Can_Install_Geometry(mesh)) return false;
    Reset(static_cast<int>(mesh.header.triangle_count), static_cast<int>(mesh.header.vertex_count), 1);
    Install_Geometry(mesh);
    if (Get_Flag(SKIN) && mesh.header.version >= Assets::W3D::W3DMeshVersion4_1
        && (mesh.header.attributes & Assets::W3D::W3DMeshAttributeGeometryTypeMask)
            == Assets::W3D::W3DMeshAttributeGeometryTypeSkin)
        Set_Flag(ALLOW_NPATCHES, true);
    if (mesh.header.attributes & Assets::W3D::W3DMeshAttributeNPatchable) Set_Flag(ALLOW_NPATCHES, true);
    switch (mesh.prelit_chunk) {
    case Assets::W3D::W3DChunkPrelitVertex: Set_Flag(PRELIT_VERTEX, true); break;
    case Assets::W3D::W3DChunkPrelitLightmapMultiPass: Set_Flag(PRELIT_LIGHTMAP_MULTI_PASS, true); break;
    case Assets::W3D::W3DChunkPrelitLightmapMultiTexture: Set_Flag(PRELIT_LIGHTMAP_MULTI_TEXTURE, true); break;
    default:
        if (!(mesh.header.attributes & Assets::W3D::W3DMeshAttributePrelitMask)
            && (mesh.header.attributes & Assets::W3D::W3DMeshAttributeObsoleteLightmapped))
            Set_Flag(PRELIT_LIGHTMAP_MULTI_PASS, true);
        break;
    }
    Graphics::W3DMeshMaterialLoadOptions options;
    options.time = Graphics::Get_Render_Clock().Sync_Time();
    options.random_sample = Sample_Texture_Mapping;
    options.bump_sine = Texture_Bump_Sine;
    options.bump_cosine = Texture_Bump_Cosine;
    const auto *catalog = W3DAssetCatalog::Get_Instance();
    options.fog = catalog != nullptr && catalog->Get_Fog_On_Load();
    options.assign_sort_level = Graphics::Get_Render_Settings().Is_Munge_Sort_On_Load_Enabled();
    options.overbright = Graphics::Get_Render_Settings().Is_Overbright_Modify_On_Load_Enabled();
    Graphics::W3DMeshMaterialLoading loading(mesh, DefMatDesc, options, Acquire_Texture,
        [](const RefCountPtr<W3DTextureHandle>& texture) { return texture->Get_Texture_Name(); });
    if (!loading.Read()) return false;
    // Construction consumes the existing game RNG even when skin cleanup will
    // discard this tree. Preserve that sequence for collision-authored skins.
    if ((W3dAttributes & Assets::W3D::W3DMeshAttributeCollisionTypeMask) && !CullTree)
        Generate_Culling_Tree();
    const auto loaded = loading.Finish(AlternateMatDesc, *MatInfo);
    Set_Flag(SORT, loaded.sorted);
    SortLevel = loaded.sort_level;
    if (loaded.animated_material3_texture)
        WWDEBUG_SAY(("ERROR: Animated Material3 texture detected in model: %s", Get_Name()));
    if (Get_Flag(SKIN)) CullTree.reset();
    return true;
}

void W3DMeshResource::compute_static_sort_levels() {
    SortLevel = Graphics::Mesh_Material_Sort_Level(*CurMatDesc);
}
void W3DMeshResource::modify_for_overbright() {
    Graphics::Apply_Mesh_Material_Overbright(*CurMatDesc);
}

void W3DMeshResource::clone_materials(const W3DMeshResource & srcmesh)
{
	/*
	** Copy the material info and the materials within
	*/
	MatInfo = std::make_shared<Graphics::ModelMaterials<RefCountPtr<W3DTextureHandle>>>(
        srcmesh.MatInfo->Clone(Graphics::Get_Render_Clock().Sync_Time()));

	/*
	** remap!
	*/
	CurMatDesc->Remap_Resources(*srcmesh.CurMatDesc, *srcmesh.MatInfo, *MatInfo);
}
