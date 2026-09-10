module;
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
export module Assets.Adapters.W3D.PassBindings;
import Assets.Adapters.W3D.Chunks;
import Assets.Math;
namespace Assets::W3D
{
export struct W3DTextureStageBindings final
{
    std::vector<std::uint32_t> texture_ids;
    std::vector<Vector2f> texcoords;
    std::vector<std::array<std::uint32_t, 3>> face_texcoord_ids;
};

export enum class W3DPassColorSource : std::uint8_t
{
    Diffuse,
    Illumination,
    Specular
};

export struct W3DPassBindings final
{
    std::vector<std::uint32_t> vertex_material_ids;
    std::vector<std::uint32_t> shader_ids;
    std::vector<std::uint32_t> shader_material_ids;
    std::vector<Color4f> diffuse_colors;
    std::vector<Color4f> diffuse_illumination;
    std::vector<Color4f> specular_colors;
    std::vector<W3DPassColorSource> color_order;
    std::vector<W3DTextureStageBindings> stages;
};

namespace PassDetail
{
bool Read_Ids(W3DByteSpan bytes, std::uint32_t count, std::vector<std::uint32_t>& ids)
{
    if (bytes.size() != 4 && bytes.size() != std::uint64_t(count) * 4)
        return false;
    if (bytes.empty())
        return false;
    ids.resize(bytes.size() / 4);
    for (std::size_t index = 0; index != ids.size(); ++index)
        if (!W3DRead_U32(bytes, index * 4, ids[index]))
            return false;
    return true;
}

bool Read_Colors(W3DByteSpan bytes, std::uint32_t count, bool alpha, std::vector<Color4f>& colors)
{
    // Both W3dRGBStruct and W3dRGBAStruct occupy four bytes on disk.
    if (bytes.size() != std::uint64_t(count) * 4)
        return false;
    colors.resize(count);
    for (std::size_t index = 0; index != colors.size(); ++index) {
        const auto* value = bytes.data() + index * 4;
        colors[index] = {std::to_integer<unsigned>(value[0]) / 255.0f,
            std::to_integer<unsigned>(value[1]) / 255.0f,
            std::to_integer<unsigned>(value[2]) / 255.0f,
            alpha ? std::to_integer<unsigned>(value[3]) / 255.0f : 1.0f};
    }
    return true;
}

bool Read_Stage(W3DByteSpan bytes, std::uint32_t vertex_count,
    std::uint32_t triangle_count, W3DTextureStageBindings& stage)
{
    const bool valid = W3DVisit_Chunks(bytes, [&](const W3DChunkView& chunk) {
        switch (chunk.id) {
        case W3DChunkTextureIds:
            return stage.texture_ids.empty() && Read_Ids(chunk.payload, triangle_count, stage.texture_ids);
        case W3DChunkStageTextureCoords:
        case W3DChunkTextureCoords:
            if (!stage.texcoords.empty() || chunk.payload.size() % 8 != 0)
                return false;
            stage.texcoords.resize(chunk.payload.size() / 8);
            for (std::size_t index = 0; index != stage.texcoords.size(); ++index) {
                auto& uv = stage.texcoords[index];
                if (!W3DRead_F32(chunk.payload, index * 8, uv.x)
                    || !W3DRead_F32(chunk.payload, index * 8 + 4, uv.y))
                    return false;
                // Shipped meshes contain unused non-finite UV entries. Match
                // the importer contract and publish finite coordinates.
                if (!std::isfinite(uv.x)) uv.x = 0.0f;
                if (!std::isfinite(uv.y)) uv.y = 0.0f;
            }
            return true;
        case W3DChunkPerFaceTextureCoordIds:
            if (!stage.face_texcoord_ids.empty() || chunk.payload.size() != std::uint64_t(triangle_count) * 12)
                return false;
            stage.face_texcoord_ids.resize(triangle_count);
            for (std::size_t face = 0; face != triangle_count; ++face)
                for (std::size_t corner = 0; corner != 3; ++corner)
                    if (!W3DRead_U32(chunk.payload, face * 12 + corner * 4, stage.face_texcoord_ids[face][corner]))
                        return false;
            return true;
        default:
            return true;
        }
    });
    if (!valid)
        return false;
    if (stage.face_texcoord_ids.empty())
        return stage.texcoords.empty() || stage.texcoords.size() == vertex_count;
    for (const auto& face : stage.face_texcoord_ids)
        for (const auto index : face)
            if (index >= stage.texcoords.size())
                return false;
    return true;
}
}

// Retains every binding in source order. UVs remain in the W3D coordinate
// convention until the model builder converts them to its generic contract.
export bool W3DRead_Pass_Bindings(W3DByteSpan bytes, std::uint32_t vertex_count,
    std::uint32_t triangle_count, W3DPassBindings& result)
{
    result = {};
    if (!W3DValidate_Chunk_Tree(bytes))
        return false;
    W3DPassBindings parsed;
    const bool valid = W3DVisit_Chunks(bytes, [&](const W3DChunkView& chunk) {
        switch (chunk.id) {
        case W3DChunkVertexMaterialIds:
            return parsed.vertex_material_ids.empty()
                && PassDetail::Read_Ids(chunk.payload, vertex_count, parsed.vertex_material_ids);
        case W3DChunkShaderIds:
            return parsed.shader_ids.empty()
                && PassDetail::Read_Ids(chunk.payload, triangle_count, parsed.shader_ids);
        case W3DChunkShaderMaterialIds:
            return parsed.shader_material_ids.empty()
                && PassDetail::Read_Ids(chunk.payload, triangle_count, parsed.shader_material_ids);
        case 0x3B:
            parsed.color_order.push_back(W3DPassColorSource::Diffuse);
            return parsed.diffuse_colors.empty()
                && PassDetail::Read_Colors(chunk.payload, vertex_count, true, parsed.diffuse_colors);
        case 0x3C:
            parsed.color_order.push_back(W3DPassColorSource::Illumination);
            return parsed.diffuse_illumination.empty()
                && PassDetail::Read_Colors(chunk.payload, vertex_count, false, parsed.diffuse_illumination);
        case 0x3E:
            parsed.color_order.push_back(W3DPassColorSource::Specular);
            return parsed.specular_colors.empty()
                && PassDetail::Read_Colors(chunk.payload, vertex_count, false, parsed.specular_colors);
        case W3DChunkTextureStage:
            parsed.stages.emplace_back();
            return chunk.contains_children
                && PassDetail::Read_Stage(chunk.payload, vertex_count, triangle_count, parsed.stages.back());
        default:
            return true;
        }
    });
    if (valid)
        result = std::move(parsed);
    return valid;
}
}
