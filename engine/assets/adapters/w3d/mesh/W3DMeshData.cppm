module;
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>
export module Assets.Adapters.W3D.MeshData;
import Assets.Math;
import Assets.MeshBoundsTree;
import Assets.Adapters.W3D.Chunks;
import Assets.Adapters.W3D.Geometry;
import Assets.Adapters.W3D.Materials;
import Assets.Adapters.W3D.PassBindings;

namespace Assets::W3D {
export enum class W3DMeshPrelighting { Unlit, Vertex, LightmapMultiPass, LightmapMultiTexture };

// Material records retain file order. Repeated passes supply alternate arrays,
// and color records can multiply an earlier illumination set or replace alpha.
export using W3DMeshMaterialRecord = std::variant<W3DMaterialInfo,
    std::vector<W3DShaderSettings>, std::vector<W3DVertexMaterialData>,
    std::vector<W3DTextureData>, W3DPassBindings, std::vector<Vector2f>,
    std::vector<W3DMaterial3Data>, std::vector<std::uint16_t>, std::vector<Color4f>>;

export struct W3DMeshData final {
    W3DMeshHeader header;
    std::vector<Vector3f> positions;
    std::vector<Vector3f> normals;
    std::vector<W3DTriangleRecord> triangles;
    std::optional<std::vector<std::uint16_t>> bone_indices;
    std::vector<std::uint32_t> shade_indices;
    std::optional<std::string> user_text;
    std::optional<MeshBoundsTree> bounds_tree;
    std::uint32_t prelit_chunk = 0xffffffffu;
    std::vector<W3DMeshMaterialRecord> materials;
};

namespace MeshDataDetail {
std::uint32_t Select_Prelighting(std::uint32_t attributes, W3DMeshPrelighting preference) {
    if (!(attributes & 0x0f000000u)) return 0xffffffffu;
    if (preference >= W3DMeshPrelighting::LightmapMultiTexture && (attributes & 0x08000000u)) return 0x26;
    if (preference >= W3DMeshPrelighting::LightmapMultiPass && (attributes & 0x04000000u)) return 0x25;
    if (preference >= W3DMeshPrelighting::Vertex && (attributes & 0x02000000u)) return 0x24;
    return 0x23;
}

bool Read_Material(const W3DChunkView& chunk, W3DMeshData& mesh, W3DMaterialInfo& info) {
    const auto bytes = chunk.payload;
    switch (chunk.id) {
    case W3DChunkMaterialInfo:
        if (bytes.size() != 16 || !W3DRead_U32(bytes, 0, info.pass_count)
            || !W3DRead_U32(bytes, 4, info.vertex_material_count)
            || !W3DRead_U32(bytes, 8, info.shader_count)
            || !W3DRead_U32(bytes, 12, info.texture_count)) return false;
        mesh.materials.emplace_back(info);
        return true;
    case W3DChunkShaders: {
        std::vector<W3DShaderSettings> shaders;
        if (!W3DRead_Shaders(bytes, shaders) || shaders.size() != info.shader_count) return false;
        mesh.materials.emplace_back(std::move(shaders));
        return true;
    }
    case W3DChunkVertexMaterials: {
        std::vector<W3DVertexMaterialData> materials;
        if (!W3DVisit_Chunks(bytes, [&](const W3DChunkView& child) {
            if (child.id != W3DChunkVertexMaterial) return false;
            W3DVertexMaterialData material;
            if (!W3DRead_Vertex_Material(child.payload, material)) return false;
            materials.push_back(std::move(material));
            return true;
        })) return false;
        mesh.materials.emplace_back(std::move(materials));
        return true;
    }
    case W3DChunkTextures: {
        std::vector<W3DTextureData> textures;
        if (!W3DVisit_Chunks(bytes, [&](const W3DChunkView& child) {
            if (child.id != W3DChunkTexture) return false;
            W3DTextureData texture;
            if (!W3DRead_Texture(child.payload, texture)) return false;
            textures.push_back(std::move(texture));
            return true;
        })) return false;
        mesh.materials.emplace_back(std::move(textures));
        return true;
    }
    case W3DChunkMaterialPass: {
        W3DPassBindings bindings;
        if (!W3DRead_Pass_Bindings(bytes, mesh.header.vertex_count, mesh.header.triangle_count, bindings)) return false;
        mesh.materials.emplace_back(std::move(bindings));
        return true;
    }
    default: return true;
    }
}
}

export bool W3DRead_Mesh_Data(W3DByteSpan bytes, W3DMeshPrelighting preference,
    W3DMeshData& result, std::string& error)
{
    const auto fail = [&](const char* text) { error = text; return false; };
    if (!W3DValidate_Chunk_Tree(bytes)) return fail("Malformed mesh chunk tree");
    W3DMeshData mesh;
    W3DMaterialInfo info;
    bool has_header = false;
    const bool decoded = W3DVisit_Chunks(bytes, [&](const W3DChunkView& chunk) {
        if (!has_header) {
            if (chunk.id != W3DChunkMeshHeader3 || !W3DRead_Mesh_Header(chunk.payload, mesh.header)) return false;
            has_header = true;
            mesh.prelit_chunk = MeshDataDetail::Select_Prelighting(mesh.header.attributes, preference);
            return true;
        }
        const auto payload = chunk.payload;
        switch (chunk.id) {
        case W3DChunkMeshHeader3: return false;
        case W3DChunkVertices:
            return W3DRead_Geometry_Vectors(payload, mesh.header.vertex_count, mesh.positions);
        case W3DChunkVertexNormals:
        case W3DChunkSurrenderNormals:
            return W3DRead_Geometry_Vectors(payload, mesh.header.vertex_count, mesh.normals);
        case W3DChunkTriangles:
            return W3DRead_Geometry_Triangles(payload, mesh.header.triangle_count, mesh.triangles);
        case W3DChunkVertexInfluences: {
            std::vector<std::uint16_t> bones;
            if (!W3DRead_Geometry_Bone_Links(payload, mesh.header.vertex_count, bones)) return false;
            if (mesh.header.version < 0x00030000u)
                for (auto& bone : bones) ++bone;
            mesh.bone_indices = std::move(bones);
            return true;
        }
        case W3DChunkVertexShadeIndices:
            return W3DRead_Geometry_Shade_Indices(payload, mesh.header.vertex_count, mesh.shade_indices);
        case W3DChunkMeshUserText:
            if (!mesh.user_text) mesh.user_text = std::string(reinterpret_cast<const char*>(payload.data()), payload.size());
            return true;
        case 0x90: {
            MeshBoundsTree tree;
            if (!W3DRead_Mesh_Bounds_Tree(payload, mesh.header.triangle_count, tree)) return false;
            mesh.bounds_tree = std::move(tree);
            return true;
        }
        case 0x23: case 0x24: case 0x25: case 0x26:
            return chunk.id != mesh.prelit_chunk || W3DVisit_Chunks(payload,
                [&](const W3DChunkView& child) { return MeshDataDetail::Read_Material(child, mesh, info); });
        case W3DChunkTextureCoords: {
            if (payload.size() != static_cast<std::size_t>(mesh.header.vertex_count) * 8) return false;
            std::vector<Vector2f> coordinates(mesh.header.vertex_count);
            for (std::size_t i = 0; i < coordinates.size(); ++i)
                if (!W3DRead_F32(payload, i * 8, coordinates[i].x) || !W3DRead_F32(payload, i * 8 + 4, coordinates[i].y)
                    || !std::isfinite(coordinates[i].x) || !std::isfinite(coordinates[i].y)) return false;
            mesh.materials.emplace_back(std::move(coordinates));
            return true;
        }
        case W3DChunkMaterials3: {
            std::vector<W3DMaterial3Data> materials;
            if (!W3DRead_Material3_Container(payload, materials) || materials.size() != mesh.header.material_count) return false;
            mesh.materials.emplace_back(std::move(materials));
            return true;
        }
        case W3DChunkPerTriMaterials: {
            // A single Material3 entry needs no per-triangle table.
            if (mesh.header.material_count == 1) return true;
            if (payload.size() != static_cast<std::size_t>(mesh.header.triangle_count) * 2) return false;
            std::vector<std::uint16_t> ids(mesh.header.triangle_count);
            for (std::size_t i = 0; i < ids.size(); ++i) {
                ids[i] = static_cast<std::uint16_t>(std::to_integer<unsigned>(payload[i * 2]) |
                    (std::to_integer<unsigned>(payload[i * 2 + 1]) << 8));
                if (ids[i] >= mesh.header.material_count) return false;
            }
            mesh.materials.emplace_back(std::move(ids));
            return true;
        }
        case W3DChunkVertexColors: {
            if (payload.size() != static_cast<std::size_t>(mesh.header.vertex_count) * 4) return false;
            std::vector<Color4f> colors(mesh.header.vertex_count);
            for (std::size_t i = 0; i < colors.size(); ++i)
                colors[i] = {std::to_integer<unsigned>(payload[i * 4]) / 255.0f,
                    std::to_integer<unsigned>(payload[i * 4 + 1]) / 255.0f,
                    std::to_integer<unsigned>(payload[i * 4 + 2]) / 255.0f, 1};
            mesh.materials.emplace_back(std::move(colors));
            return true;
        }
        default: return MeshDataDetail::Read_Material(chunk, mesh, info);
        }
    });
    if (!decoded || !has_header) return fail("Invalid mesh record");
    if (mesh.positions.size() != mesh.header.vertex_count || mesh.triangles.size() != mesh.header.triangle_count)
        return fail("Mesh geometry counts do not match its header");
    for (const auto& triangle : mesh.triangles) {
        for (const auto index : triangle.indices)
            if (index >= mesh.positions.size()) return fail("Mesh triangle references a missing vertex");
        if (triangle.surface_type > 255 || !std::isfinite(triangle.normal.x) || !std::isfinite(triangle.normal.y)
            || !std::isfinite(triangle.normal.z) || !std::isfinite(triangle.distance)) return fail("Invalid mesh triangle metadata");
    }
    for (const auto index : mesh.shade_indices)
        if (index >= mesh.positions.size()) return fail("Mesh smoothing index references a missing vertex");
    result = std::move(mesh);
    error.clear();
    return true;
}
}
