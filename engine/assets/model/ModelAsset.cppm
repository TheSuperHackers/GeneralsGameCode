module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

export module Assets.Models;

import Assets.Handles;
import Assets.Identity;
import Assets.Materials;
import Assets.Math;
export import Assets.ModelRig;

namespace Assets
{

export struct ModelVertexDesc final
{
	Vector3f position{};
	Vector3f normal{0.0f, 0.0f, 1.0f};
	Vector2f texcoord{};
	Color4f color{};
	std::array<std::uint16_t, 4> bone_indices{};
	std::array<float, 4> bone_weights{1.0f, 0.0f, 0.0f, 0.0f};
	Vector3f tangent{};
	float tangent_sign = 1.0f;
};

export struct ModelSubmeshDesc final
{
	std::uint32_t first_index = 0;
	std::uint32_t index_count = 0;
	std::uint32_t material_index = 0;
	std::string name;
	bool skinned = false;
};

export using ModelMaterialDesc = MaterialAssetDesc;

export struct ModelAssetDesc final
{
	std::string name;
	std::string container_name;
	std::string source_name;
	std::string source_format;
	std::uint32_t source_attributes = 0;
	std::int32_t sort_level = 0;
	float lod_min = 0.0f;
	float lod_max = 0.0f;
	Bounds3f bounds{};
	std::uint32_t skin_bone_count = 0;
	std::vector<ModelVertexDesc> vertices;
	std::vector<std::uint32_t> indices;
	std::vector<ModelSubmeshDesc> submeshes;
	std::vector<ModelMaterialDesc> materials;
	std::vector<AssetDependencyDesc> dependencies;
	ModelRigDesc rig;
};

export struct ModelVertex final
{
	Vector3f position{};
	Vector3f normal{0.0f, 0.0f, 1.0f};
	Vector2f texcoord{};
	Color4f color{};
	std::array<std::uint16_t, 4> bone_indices{};
	std::array<float, 4> bone_weights{1.0f, 0.0f, 0.0f, 0.0f};
	Vector3f tangent{};
	float tangent_sign = 1.0f;
};

export struct ModelSubmesh final
{
	std::uint32_t first_index = 0;
	std::uint32_t index_count = 0;
	std::uint32_t material_index = 0;
	std::string name;
	bool skinned = false;
};

export struct ModelMaterial final
{
	std::string name;
	MaterialAssetHandle asset_handle;
	AssetIdentity primary_texture{AssetType::Texture, {}};
	AssetIdentity secondary_texture{AssetType::Texture, {}};
	Color4f base_color{};
	float shininess = 1.0f;
	float opacity = 1.0f;
	float translucency = 0.0f;
	std::uint32_t source_attributes = 0;
	MaterialRenderMode render_mode = MaterialRenderMode::Opaque;
	bool depth_write = true;
	bool texturing = true;
	Color4f ambient_color{};
	Color4f specular_color{0, 0, 0, 1};
	Color4f emissive_color{0, 0, 0, 1};
	MaterialSurfaceParameters surface{};
	std::array<AssetIdentity, MaterialSurfaceTextureCount> surface_textures{};
};

export class ModelAsset final
{
public:
	ModelAsset(
		AssetIdentity identity,
		ModelAssetDesc description,
		std::span<const MaterialAssetHandle> material_handles = {});

	const AssetIdentity &Identity() const noexcept;
	const std::string &Name() const noexcept;
	const std::string &Container_Name() const noexcept;
	const std::string &Source_Format() const noexcept;
	std::uint32_t Source_Attributes() const noexcept;
	std::int32_t Sort_Level() const noexcept;
	float LOD_Min() const noexcept;
	float LOD_Max() const noexcept;
	const Bounds3f &Bounds() const noexcept;
	std::uint32_t Skin_Bone_Count() const noexcept;
	std::span<const ModelVertex> Vertices() const noexcept;
	std::span<const std::uint32_t> Indices() const noexcept;
	std::span<const ModelSubmesh> Submeshes() const noexcept;
	std::span<const ModelMaterial> Materials() const noexcept;
	std::span<const AssetDependency> Dependencies() const noexcept;
	const ModelRigDesc &Rig() const noexcept { return m_rig; }

private:
	AssetIdentity m_identity;
	std::string m_name;
	std::string m_container_name;
	std::string m_source_format;
	std::uint32_t m_source_attributes = 0;
	std::int32_t m_sort_level = 0;
	float m_lod_min = 0.0f;
	float m_lod_max = 0.0f;
	Bounds3f m_bounds{};
	std::uint32_t m_skin_bone_count = 0;
	std::vector<ModelVertex> m_vertices;
	std::vector<std::uint32_t> m_indices;
	std::vector<ModelSubmesh> m_submeshes;
	std::vector<ModelMaterial> m_materials;
	std::vector<AssetDependency> m_dependencies;
	ModelRigDesc m_rig;
};

}

namespace Assets
{

ModelAsset::ModelAsset(
	AssetIdentity identity,
	ModelAssetDesc description,
	std::span<const MaterialAssetHandle> material_handles)
	: m_identity(std::move(identity)),
	  m_name(std::move(description.name)),
	  m_container_name(std::move(description.container_name)),
	  m_source_format(std::move(description.source_format)),
	  m_source_attributes(description.source_attributes),
	  m_sort_level(description.sort_level),
	  m_lod_min(description.lod_min),
	  m_lod_max(description.lod_max),
	  m_bounds(description.bounds),
	  m_skin_bone_count(description.skin_bone_count),
	  m_rig(std::move(description.rig))
{
	m_vertices.reserve(description.vertices.size());
	for (const ModelVertexDesc &vertex : description.vertices) {
		m_vertices.push_back({
			vertex.position,
			vertex.normal,
			vertex.texcoord,
			vertex.color,
			vertex.bone_indices,
			vertex.bone_weights,
			vertex.tangent,
			vertex.tangent_sign});
	}

	m_indices = std::move(description.indices);

	m_submeshes.reserve(description.submeshes.size());
	for (ModelSubmeshDesc &submesh : description.submeshes) {
		m_submeshes.push_back({
			submesh.first_index,
			submesh.index_count,
			submesh.material_index,
			std::move(submesh.name), submesh.skinned});
	}

	m_materials.reserve(description.materials.size());
	for (std::size_t material_index = 0; material_index < description.materials.size(); ++material_index) {
		ModelMaterialDesc &material = description.materials[material_index];
		const MaterialAssetHandle material_handle = material_index < material_handles.size()
			? material_handles[material_index]
			: MaterialAssetHandle::Invalid();
		m_materials.push_back({
			std::move(material.name),
			material_handle,
			{AssetType::Texture, Canonicalize_Asset_Name(material.primary_texture)},
			{AssetType::Texture, Canonicalize_Asset_Name(material.secondary_texture)},
			material.base_color,
			material.shininess,
			material.opacity,
			material.translucency,
			material.source_attributes,
			material.render_mode,
			material.depth_write,
			material.texturing,
			material.ambient_color,
			material.specular_color,
			material.emissive_color,
			material.surface});
		for (std::size_t index = 0; index < material.surface_textures.size(); ++index)
			m_materials.back().surface_textures[index] =
				{AssetType::Texture, Canonicalize_Asset_Name(material.surface_textures[index])};
	}

	m_dependencies.reserve(description.dependencies.size());
	for (AssetDependencyDesc &dependency : description.dependencies) {
		m_dependencies.push_back({
			dependency.type,
			{dependency.type, Canonicalize_Asset_Name(dependency.name)}});
	}
}

const AssetIdentity &ModelAsset::Identity() const noexcept
{
	return m_identity;
}

const std::string &ModelAsset::Name() const noexcept
{
	return m_name;
}

const std::string &ModelAsset::Container_Name() const noexcept
{
	return m_container_name;
}

const std::string &ModelAsset::Source_Format() const noexcept
{
	return m_source_format;
}

std::uint32_t ModelAsset::Source_Attributes() const noexcept
{
	return m_source_attributes;
}

std::int32_t ModelAsset::Sort_Level() const noexcept
{
	return m_sort_level;
}

float ModelAsset::LOD_Min() const noexcept
{
	return m_lod_min;
}

float ModelAsset::LOD_Max() const noexcept
{
	return m_lod_max;
}

const Bounds3f &ModelAsset::Bounds() const noexcept
{
	return m_bounds;
}

std::uint32_t ModelAsset::Skin_Bone_Count() const noexcept
{
	return m_skin_bone_count;
}

std::span<const ModelVertex> ModelAsset::Vertices() const noexcept
{
	return m_vertices;
}

std::span<const std::uint32_t> ModelAsset::Indices() const noexcept
{
	return m_indices;
}

std::span<const ModelSubmesh> ModelAsset::Submeshes() const noexcept
{
	return m_submeshes;
}

std::span<const ModelMaterial> ModelAsset::Materials() const noexcept
{
	return m_materials;
}

std::span<const AssetDependency> ModelAsset::Dependencies() const noexcept
{
	return m_dependencies;
}

}
