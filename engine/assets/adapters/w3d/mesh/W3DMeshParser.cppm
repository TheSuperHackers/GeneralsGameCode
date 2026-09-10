module;

#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

export module Assets.Adapters.W3D.Mesh;

import Assets.Adapters.W3D.Chunks;
export import Assets.Adapters.W3D.Geometry;
import Assets.Adapters.W3D.Materials;
import Assets.Adapters.W3D.PassBindings;
import Assets.Adapters.W3D.SurfaceMaterial;
import Assets.Math;
import Assets.Models;

namespace Assets::W3D
{

// Optional versioned extension. Legacy influence padding is never reinterpreted.
export inline constexpr std::uint32_t W3DChunkSkinBindings = 0x00FE0001;



export struct W3DParsedMesh final
{
	W3DMeshHeader header;
	std::vector<Vector3f> positions;
	std::vector<Vector3f> normals;
	std::vector<Vector3f> tangents;
	std::vector<Vector3f> bitangents;
	std::vector<Vector2f> legacy_texcoords;
	std::vector<Vector2f> stage_texcoords;
	std::vector<Color4f> colors;
	std::vector<std::uint16_t> bone_indices;
	std::vector<std::array<std::uint16_t, 4>> skin_indices;
	std::vector<std::array<float, 4>> skin_weights;
	std::vector<std::array<std::uint32_t, 3>> triangles;
	W3DMaterialData materials;
	std::vector<W3DPassBindings> shader_pass_bindings;
	std::vector<ModelMaterialDesc> surface_materials;
};

namespace MeshDetail
{





bool Read_UV_Array(W3DByteSpan bytes, std::uint32_t count, std::vector<Vector2f> &values)
{
	if (count > bytes.size() / 8 || bytes.size() != static_cast<std::size_t>(count) * 8)
		return false;

	values.resize(count);
	for (std::uint32_t index = 0; index < count; ++index) {
		const std::size_t offset = static_cast<std::size_t>(index) * 8;
		if (!W3DRead_F32(bytes, offset, values[index].x) || !W3DRead_F32(bytes, offset + 4, values[index].y))
			return false;
		if (!std::isfinite(values[index].x))
			values[index].x = 0.0f;
		if (!std::isfinite(values[index].y))
			values[index].y = 0.0f;
		values[index].y = 1.0f - values[index].y;
	}
	return true;
}



}

export bool W3DParse_Mesh(W3DByteSpan bytes, W3DParsedMesh &mesh, std::string &error)
{
	if (!W3DValidate_Chunk_Tree(bytes)) {
		error = "malformed chunk tree";
		return false;
	}

	bool has_header = false;
	if (!W3DVisit_Chunks(bytes, [&mesh, &has_header](const W3DChunkView &chunk) {
		if (chunk.id != W3DChunkMeshHeader3)
			return true;
		if (has_header)
			return false;
		has_header = W3DRead_Mesh_Header(chunk.payload, mesh.header);
		return has_header;
	})) {
		error = "invalid mesh header chunk";
		return false;
	}
	if (!has_header || mesh.header.vertex_count == 0 || mesh.header.triangle_count == 0 || mesh.header.face_channels == 0) {
		error = "mesh header has no usable geometry";
		return false;
	}

	if (!W3DVisit_Chunks(bytes, [&mesh](const W3DChunkView &chunk) {
		switch (chunk.id) {
			case W3DChunkVertices:
				return W3DRead_Geometry_Vectors(chunk.payload, mesh.header.vertex_count, mesh.positions);
			case W3DChunkVertexNormals:
				return W3DRead_Geometry_Vectors(chunk.payload, mesh.header.vertex_count, mesh.normals);
			case W3DChunkTangents:
				return mesh.tangents.empty() && W3DRead_Geometry_Vectors(chunk.payload, mesh.header.vertex_count, mesh.tangents);
			case W3DChunkBitangents:
				return mesh.bitangents.empty() && W3DRead_Geometry_Vectors(chunk.payload, mesh.header.vertex_count, mesh.bitangents);
			case W3DChunkTextureCoords:
				return MeshDetail::Read_UV_Array(chunk.payload, mesh.header.vertex_count, mesh.legacy_texcoords);
			case W3DChunkTriangles:
				{
                std::vector<W3DTriangleRecord> records;
                if(!W3DRead_Geometry_Triangles(chunk.payload,mesh.header.triangle_count,records))return false;
                mesh.triangles.resize(records.size());
                for(std::size_t i=0;i<records.size();++i)mesh.triangles[i]=records[i].indices;
                return true;
            }
			case W3DChunkVertexColors:
				if (chunk.payload.size() != static_cast<std::size_t>(mesh.header.vertex_count) * 4)
					return false;
				mesh.colors.resize(mesh.header.vertex_count);
				for (std::uint32_t index = 0; index < mesh.header.vertex_count; ++index) {
					const auto *color = reinterpret_cast<const std::uint8_t *>(chunk.payload.data() + index * 4);
					mesh.colors[index] = {
						static_cast<float>(color[0]) / 255.0f,
						static_cast<float>(color[1]) / 255.0f,
						static_cast<float>(color[2]) / 255.0f,
						1.0f};
				}
				return true;
            case W3DChunkVertexInfluences:
                return W3DRead_Geometry_Bone_Links(chunk.payload,mesh.header.vertex_count,mesh.bone_indices);
			case W3DChunkSkinBindings: {
				std::uint32_t version = 0, count = 0;
				if (!mesh.skin_indices.empty() || (mesh.header.attributes & 0x00FF0000) != 0x00020000
					|| !W3DRead_U32(chunk.payload, 0, version) || version != 1
					|| !W3DRead_U32(chunk.payload, 4, count) || !count || count != mesh.header.vertex_count
					|| chunk.payload.size() < 8 || (chunk.payload.size() - 8) / 24 != count
					|| (chunk.payload.size() - 8) % 24 != 0) return false;
				mesh.skin_indices.resize(count); mesh.skin_weights.resize(count);
				for (std::uint32_t index = 0; index < count; ++index) {
					const std::size_t offset = 8 + static_cast<std::size_t>(index) * 24;
					float total = 0;
					for (std::size_t influence = 0; influence < 4; ++influence) {
						const auto at = offset + influence * 2;
						mesh.skin_indices[index][influence] = static_cast<std::uint16_t>(
							std::to_integer<unsigned>(chunk.payload[at]) | (std::to_integer<unsigned>(chunk.payload[at + 1]) << 8));
						float weight = 0;
						if (!W3DRead_F32(chunk.payload, offset + 8 + influence * 4, weight)
							|| !std::isfinite(weight) || weight < 0 || weight > 1) return false;
						mesh.skin_weights[index][influence] = weight; total += weight;
					}
					if (std::abs(total - 1.0f) > 0.00001f) return false;
				}
				return true;
			}
			case W3DChunkVertexShadeIndices:
				return chunk.payload.size() == static_cast<std::size_t>(mesh.header.vertex_count) * 4;
			case W3DChunkMeshUserText:
			case W3DChunkMaterialInfo:
			case W3DChunkShaders:
			case W3DChunkVertexMaterials:
			case W3DChunkTextures:
			case W3DChunkMaterialPass:
				return true;
			default:
				return true;
		}
	})) {
		error = "invalid mesh data chunk";
		return false;
	}

	if (mesh.positions.size() != mesh.header.vertex_count ||
		mesh.triangles.size() != mesh.header.triangle_count ||
		(!mesh.normals.empty() && mesh.normals.size() != mesh.header.vertex_count)) {
		error = "mesh data counts do not match the header";
		return false;
	}

	if (!W3DParse_Materials(bytes, mesh.header.vertex_count, mesh.materials)) {
		error = "invalid material chunk";
		return false;
	}
	if (mesh.materials.passes.empty() && mesh.materials.vertex_materials.empty() && mesh.header.material_count != 0) {
		error = "mesh declares materials but contains no material data";
		return false;
	}
	for (const W3DMaterialPass &pass : mesh.materials.passes) {
		if (pass.uses_shader_material)
			continue;
		if (pass.vertex_material_index >= mesh.materials.vertex_materials.size()) {
			error = "material pass references a vertex material outside the material table";
			return false;
		}
		if (pass.shader_index != W3DInvalidIndex && pass.shader_index >= mesh.materials.shaders.size()) {
			error = "material pass references a shader outside the shader table";
			return false;
		}
		if (pass.texture_index != W3DInvalidIndex && pass.texture_index >= mesh.materials.textures.size()) {
			error = "material pass references a texture outside the texture table";
			return false;
		}
	}
	mesh.shader_pass_bindings.resize(mesh.materials.passes.size());
	mesh.surface_materials.resize(mesh.materials.shader_materials.size());
	std::vector<bool> resolved(mesh.materials.shader_materials.size());
	std::size_t pass_index = 0;
	if (!W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
		if (chunk.id != W3DChunkMaterialPass) return true;
		const auto index = pass_index++;
		if (!mesh.materials.passes[index].uses_shader_material) return true;
		auto &bindings = mesh.shader_pass_bindings[index];
		if (!W3DRead_Pass_Bindings(chunk.payload, mesh.header.vertex_count, mesh.header.triangle_count, bindings)
			|| bindings.shader_material_ids.empty()) {
			error = "invalid shader material pass bindings";
			return false;
		}
		if (!bindings.diffuse_illumination.empty() || !bindings.specular_colors.empty()) {
			error = "unsupported lighting color array in surface material pass";
			return false;
		}
		// The supported Objects.fxh programs do not enable damage/lightmap
		// sampling and consume UV0 only. Keep validated auxiliary UV bindings in
		// the decoded source, but do not reinterpret them as texture stages.
		for (std::size_t stage = 1; stage < bindings.stages.size(); ++stage) {
			if (!bindings.stages[stage].texture_ids.empty()) {
				error = "unsupported auxiliary texture binding in surface material pass";
				return false;
			}
		}
		for (const auto material_id : bindings.shader_material_ids) {
			if (material_id >= mesh.materials.shader_materials.size()) {
				error = "shader material pass references a material outside the shader material table";
				return false;
			}
			if (!resolved[material_id]) {
				if (!W3DResolve_Surface_Material(mesh.materials.shader_materials[material_id], mesh.surface_materials[material_id], error))
					return false;
				resolved[material_id] = true;
			}
		}
		return true;
	})) return false;
	for (const auto &triangle : mesh.triangles) {
		for (const std::uint32_t index : triangle) {
			if (index >= mesh.positions.size()) {
				error = "triangle references a vertex outside the vertex array";
				return false;
			}
		}
	}

	if (mesh.normals.empty())
		mesh.normals.assign(mesh.header.vertex_count, {0.0f, 0.0f, 1.0f});
	if (mesh.colors.empty())
		mesh.colors.assign(mesh.header.vertex_count, {});
	if (mesh.legacy_texcoords.size() != mesh.header.vertex_count)
		mesh.legacy_texcoords.assign(mesh.header.vertex_count, {});
	if (!mesh.materials.passes.empty() && !mesh.materials.passes.front().texcoords.empty())
		mesh.stage_texcoords = std::move(mesh.materials.passes.front().texcoords);
	if (mesh.stage_texcoords.size() != mesh.header.vertex_count)
		mesh.stage_texcoords = mesh.legacy_texcoords;

	if (mesh.materials.vertex_materials.empty())
		mesh.materials.vertex_materials.push_back({{"default", {}, {}, {}, 1.0f, 1.0f, 0.0f, 0}});

	return true;
}

}
