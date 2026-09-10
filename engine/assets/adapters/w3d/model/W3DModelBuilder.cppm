module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module Assets.Adapters.W3D.Model;

import Assets.Adapters.W3D.Chunks;
import Assets.Adapters.W3D.Materials;
import Assets.Adapters.W3D.Mesh;
import Assets.Adapters.W3D.PassBindings;
import Assets.Identity;
import Assets.Models;
import Assets.Materials;
import Assets.Math;

namespace Assets::W3D
{

namespace ModelBuilderDetail
{

Vector3f Subtract(Vector3f a, Vector3f b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vector3f Scale(Vector3f v, float scale) { return {v.x * scale, v.y * scale, v.z * scale}; }
float Dot(Vector3f a, Vector3f b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vector3f Cross(Vector3f a, Vector3f b) { return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }

void Set_Tangent(ModelVertexDesc &vertex, Vector3f tangent, Vector3f bitangent)
{
	const float normal_length = std::sqrt(Dot(vertex.normal, vertex.normal));
	const Vector3f normal = normal_length > 1e-10f ? Scale(vertex.normal, 1 / normal_length) : Vector3f{0, 0, 1};
	tangent = Subtract(tangent, Scale(normal, Dot(normal, tangent)));
	float length = std::sqrt(Dot(tangent, tangent));
	if (length < 1e-10f) {
		tangent = Cross(std::abs(normal.z) < .9f ? Vector3f{0, 0, 1} : Vector3f{0, 1, 0}, normal);
		length = std::sqrt(Dot(tangent, tangent));
	}
	vertex.tangent = Scale(tangent, 1 / length);
	vertex.tangent_sign = Dot(Cross(normal, vertex.tangent), bitangent) < 0 ? -1.0f : 1.0f;
}

void Complete_Face_Tangents(ModelAssetDesc &description)
{
	const auto first = description.vertices.size() - 3;
	const auto &a = description.vertices[first];
	const auto &b = description.vertices[first + 1];
	const auto &c = description.vertices[first + 2];
	const auto edge1 = Subtract(b.position, a.position), edge2 = Subtract(c.position, a.position);
	const float du1 = b.texcoord.x - a.texcoord.x, dv1 = b.texcoord.y - a.texcoord.y;
	const float du2 = c.texcoord.x - a.texcoord.x, dv2 = c.texcoord.y - a.texcoord.y;
	const float determinant = du1 * dv2 - du2 * dv1;
	Vector3f tangent{}, bitangent{};
	if (std::abs(determinant) > 1e-10f) {
		tangent = Scale(Subtract(Scale(edge1, dv2), Scale(edge2, dv1)), 1 / determinant);
		bitangent = Scale(Subtract(Scale(edge2, du1), Scale(edge1, du2)), 1 / determinant);
	}
	for (std::size_t i = first; i < first + 3; ++i)
		if (Dot(description.vertices[i].tangent, description.vertices[i].tangent) < 1e-10f)
			Set_Tangent(description.vertices[i], tangent, bitangent);
}

void Apply_Shader_Settings(ModelMaterialDesc &material, const W3DShaderSettings &shader) noexcept
{
	material.depth_write = shader.depth_mask != 0;
	material.texturing = shader.texturing != 0;
	if (shader.alpha_test != 0) {
		material.render_mode = MaterialRenderMode::AlphaTest;
		return;
	}
	if (shader.source_blend == 1 && shader.destination_blend == 1) {
		material.render_mode = MaterialRenderMode::Additive;
		return;
	}
	if (shader.source_blend == 1 && shader.destination_blend == 2) {
		material.render_mode = MaterialRenderMode::Multiply;
		return;
	}
	if (shader.source_blend == 2 && shader.destination_blend == 5) {
		material.render_mode = MaterialRenderMode::AlphaBlend;
		return;
	}
	if (!material.depth_write)
		material.render_mode = MaterialRenderMode::AlphaBlend;
}

void Add_Dependency(ModelAssetDesc &description, AssetType type, std::string_view name)
{
	if (name.empty())
		return;

	const std::string canonical_name = Canonicalize_Asset_Name(name);
	if (canonical_name.empty())
		return;
	for (const AssetDependencyDesc &dependency : description.dependencies) {
		if (dependency.type == type && Canonicalize_Asset_Name(dependency.name) == canonical_name)
			return;
	}
	description.dependencies.push_back({type, std::string(name)});
}

void Append_Surface_Pass(ModelAssetDesc &description, const W3DParsedMesh &mesh,
	const W3DPassBindings &bindings, std::uint32_t vertex_base)
{
	std::vector<std::uint32_t> materials(mesh.surface_materials.size(), W3DInvalidIndex);
	std::uint32_t previous_material = W3DInvalidIndex;
	for (std::size_t face = 0; face < mesh.triangles.size(); ++face) {
		const auto source_material = bindings.shader_material_ids[bindings.shader_material_ids.size() == 1 ? 0 : face];
		if (materials[source_material] == W3DInvalidIndex) {
			materials[source_material] = static_cast<std::uint32_t>(description.materials.size());
			description.materials.push_back(mesh.surface_materials[source_material]);
		}
		const auto material_index = materials[source_material];
		// Keep authored face order, including noncontiguous material runs.
		if (material_index != previous_material) {
			description.submeshes.push_back({static_cast<std::uint32_t>(description.indices.size()), 0,
				material_index, mesh.header.name, !mesh.bone_indices.empty() || !mesh.skin_indices.empty()});
			previous_material = material_index;
		}
		for (std::size_t corner = 0; corner < 3; ++corner) {
			const auto source_vertex = mesh.triangles[face][corner];
			ModelVertexDesc vertex = description.vertices[vertex_base + source_vertex];
			if (!bindings.stages.empty()) {
				const auto &stage = bindings.stages.front();
				if (!stage.texcoords.empty()) {
					const auto uv_index = stage.face_texcoord_ids.empty() ? source_vertex : stage.face_texcoord_ids[face][corner];
					vertex.texcoord = stage.texcoords[uv_index];
					vertex.texcoord.y = 1.0f - vertex.texcoord.y;
				}
			}
			if (!bindings.diffuse_colors.empty()) vertex.color = bindings.diffuse_colors[source_vertex];
			description.indices.push_back(static_cast<std::uint32_t>(description.vertices.size()));
			description.vertices.push_back(vertex);
		}
		Complete_Face_Tangents(description);
		description.submeshes.back().index_count += 3;
	}
}

}

export void W3DAppend_Mesh(ModelAssetDesc &description, W3DParsedMesh &mesh)
{
	const bool first_mesh = description.vertices.empty();
	const std::uint32_t vertex_base = static_cast<std::uint32_t>(description.vertices.size());
	const std::uint32_t index_base = static_cast<std::uint32_t>(description.indices.size());
	const std::uint32_t material_base = static_cast<std::uint32_t>(description.materials.size());

	description.vertices.reserve(description.vertices.size() + mesh.positions.size());
	for (std::size_t index = 0; index < mesh.positions.size(); ++index) {
		ModelVertexDesc vertex;
		vertex.position = mesh.positions[index];
		vertex.normal = mesh.normals[index];
		vertex.texcoord = mesh.stage_texcoords[index];
		vertex.color = mesh.colors[index];
		if (!mesh.tangents.empty() && !mesh.bitangents.empty()) {
			// Generic UVs flip W3D's V axis, so its authored bitangent flips too.
			ModelBuilderDetail::Set_Tangent(vertex, mesh.tangents[index],
				ModelBuilderDetail::Scale(mesh.bitangents[index], -1.0f));
		}
		if (!mesh.bone_indices.empty()) {
			vertex.bone_indices[0] = mesh.bone_indices[index];
			vertex.bone_weights = {1.0f, 0.0f, 0.0f, 0.0f};
		}
		if (!mesh.skin_indices.empty()) {
			vertex.bone_indices = mesh.skin_indices[index];
			vertex.bone_weights = mesh.skin_weights[index];
		}
		 description.vertices.push_back(vertex);
	}
	if (!mesh.bone_indices.empty()) {
		std::uint16_t maximum_bone = 0;
		for (const std::uint16_t bone_index : mesh.bone_indices)
			maximum_bone = std::max(maximum_bone, bone_index);
		description.skin_bone_count = std::max(
			description.skin_bone_count,
			static_cast<std::uint32_t>(maximum_bone) + 1u);
	}
	for (const auto &indices : mesh.skin_indices)
		for (const auto bone : indices)
			description.skin_bone_count = std::max(description.skin_bone_count, static_cast<std::uint32_t>(bone) + 1u);

	const bool has_legacy_pass = mesh.materials.passes.empty() ||
		std::any_of(mesh.materials.passes.begin(), mesh.materials.passes.end(),
			[](const W3DMaterialPass &pass) { return !pass.uses_shader_material; });
	if (has_legacy_pass) {
		description.indices.reserve(description.indices.size() + mesh.triangles.size() * 3);
		for (const auto &triangle : mesh.triangles)
			for (const std::uint32_t index : triangle)
				description.indices.push_back(vertex_base + index);
	}
	const auto legacy_index_count = static_cast<std::uint32_t>(description.indices.size() - index_base);
	const auto submesh_base = description.submeshes.size();
	if (mesh.materials.passes.empty()) {
		for (auto &source : mesh.materials.vertex_materials)
			description.materials.push_back(std::move(source.material));
	} else {
		for (std::size_t pass_index = 0; pass_index < mesh.materials.passes.size(); ++pass_index) {
			const W3DMaterialPass &pass = mesh.materials.passes[pass_index];
			if (pass.uses_shader_material) {
				ModelBuilderDetail::Append_Surface_Pass(description, mesh, mesh.shader_pass_bindings[pass_index], vertex_base);
				continue;
			}
			if (pass.vertex_material_index >= mesh.materials.vertex_materials.size())
				continue;
			ModelMaterialDesc material = mesh.materials.vertex_materials[pass.vertex_material_index].material;
			if (pass.texture_index < mesh.materials.textures.size())
				material.primary_texture = mesh.materials.textures[pass.texture_index].name;
			if (pass.shader_index < mesh.materials.shaders.size())
				ModelBuilderDetail::Apply_Shader_Settings(material, mesh.materials.shaders[pass.shader_index]);
			description.submeshes.push_back({index_base, legacy_index_count,
				static_cast<std::uint32_t>(description.materials.size()), mesh.header.name, !mesh.bone_indices.empty() || !mesh.skin_indices.empty()});
			description.materials.push_back(std::move(material));
		}
	}
	if (description.materials.size() == material_base)
		description.materials.push_back({"default", {}, {}, {}, 1.0f, 1.0f, 0.0f, 0});
	if (description.submeshes.size() == submesh_base)
		description.submeshes.push_back({index_base, legacy_index_count, material_base, mesh.header.name, !mesh.bone_indices.empty() || !mesh.skin_indices.empty()});

	for (std::size_t material_index = material_base; material_index < description.materials.size(); ++material_index) {
		ModelMaterialDesc &material = description.materials[material_index];
		material.scope = MaterialScope::Model;
		ModelBuilderDetail::Add_Dependency(description, AssetType::Material, material.name);
		ModelBuilderDetail::Add_Dependency(description, AssetType::Texture, material.primary_texture);
		ModelBuilderDetail::Add_Dependency(description, AssetType::Texture, material.secondary_texture);
		for (const auto &texture : material.surface_textures)
			ModelBuilderDetail::Add_Dependency(description, AssetType::Texture, texture);
	}

	if (description.name.empty())
		description.name = mesh.header.name;
	if (description.container_name.empty())
		description.container_name = mesh.header.container_name;
	description.source_attributes |= mesh.header.attributes;
	description.sort_level = mesh.header.sort_level;
	if (first_mesh) {
		description.bounds = mesh.header.bounds;
	} else {
		description.bounds.minimum.x = std::min(description.bounds.minimum.x, mesh.header.bounds.minimum.x);
		description.bounds.minimum.y = std::min(description.bounds.minimum.y, mesh.header.bounds.minimum.y);
		description.bounds.minimum.z = std::min(description.bounds.minimum.z, mesh.header.bounds.minimum.z);
		description.bounds.maximum.x = std::max(description.bounds.maximum.x, mesh.header.bounds.maximum.x);
		description.bounds.maximum.y = std::max(description.bounds.maximum.y, mesh.header.bounds.maximum.y);
		description.bounds.maximum.z = std::max(description.bounds.maximum.z, mesh.header.bounds.maximum.z);
	}
}

}
