module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

export module Graphics.Scene.Models.ModelAssetGeometry;

export import Graphics.Scene.StaticMeshes;

import Assets.Models;
import Assets.Math;

namespace Graphics
{

export struct ModelAssetGeometry final
{
	std::vector<StaticMeshVertex> vertices;
	std::vector<SkinnedMeshVertex> skinned_vertices;
	std::vector<std::uint16_t> indices;
	std::vector<MeshPart> parts;
	StaticMeshSource source{};
	RenderBounds bounds{};
};

export bool Build_Model_Asset_Geometry(
	const Assets::ModelAsset &asset,
	ModelAssetGeometry &geometry);

export bool Find_Model_Asset_Part(
	const Assets::ModelAsset &asset,
	std::string_view name,
	ModelPartId &part) noexcept;

bool Build_Model_Asset_Geometry(
	const Assets::ModelAsset &asset,
	ModelAssetGeometry &geometry)
{
	geometry = {};
	const std::span<const Assets::ModelVertex> vertices = asset.Vertices();
	const std::span<const std::uint32_t> indices = asset.Indices();
	const std::span<const Assets::ModelSubmesh> submeshes = asset.Submeshes();
	const std::span<const Assets::ModelMaterial> asset_materials = asset.Materials();
	if (vertices.empty() || indices.empty() || vertices.size() > std::numeric_limits<std::uint16_t>::max()
		|| submeshes.empty() || asset_materials.empty() || submeshes.size() > Max_Model_Part_Count)
		return false;

	const std::uint32_t skin_bone_count = asset.Skin_Bone_Count();
	const bool skinned = skin_bone_count != 0;
	if (skinned) {
		if (skin_bone_count > std::numeric_limits<std::uint16_t>::max() + 1u)
			return false;
		geometry.skinned_vertices.resize(vertices.size());
	} else {
		geometry.vertices.resize(vertices.size());
	}
	for (std::size_t index = 0; index < vertices.size(); ++index) {
		const Assets::ModelVertex &source = vertices[index];
		if (!skinned && (source.bone_indices != std::array<std::uint16_t, 4>{}
			|| source.bone_weights != std::array<float, 4>{1.0f, 0.0f, 0.0f, 0.0f}))
			return false;
		if (skinned) {
			for (const std::uint16_t bone_index : source.bone_indices)
				if (bone_index >= skin_bone_count)
					return false;
			SkinnedMeshVertex &destination = geometry.skinned_vertices[index];
			destination.skinning.bone_indices = source.bone_indices;
			destination.skinning.bone_weights = source.bone_weights;
			destination.position[0] = source.position.x;
			destination.position[1] = source.position.y;
			destination.position[2] = source.position.z;
			destination.color[0] = source.color.r;
			destination.color[1] = source.color.g;
			destination.color[2] = source.color.b;
			destination.color[3] = source.color.a;
			destination.uv[0] = source.texcoord.x;
			destination.uv[1] = source.texcoord.y;
		} else {
			StaticMeshVertex &destination = geometry.vertices[index];
			destination.position[0] = source.position.x;
			destination.position[1] = source.position.y;
			destination.position[2] = source.position.z;
			destination.color[0] = source.color.r;
			destination.color[1] = source.color.g;
			destination.color[2] = source.color.b;
			destination.color[3] = source.color.a;
			destination.uv[0] = source.texcoord.x;
			destination.uv[1] = source.texcoord.y;
		}
	}

	geometry.indices.resize(indices.size());
	for (std::size_t index = 0; index < indices.size(); ++index) {
		if (indices[index] >= vertices.size())
			return false;
		geometry.indices[index] = static_cast<std::uint16_t>(indices[index]);
	}

	geometry.parts.reserve(submeshes.size());
	for (const Assets::ModelSubmesh &submesh : submeshes) {
		if (submesh.material_index >= asset_materials.size()
			|| static_cast<std::uint64_t>(submesh.first_index) + submesh.index_count > indices.size()
			|| submesh.index_count == 0)
			return false;
		geometry.parts.push_back({
			submesh.first_index,
			submesh.index_count,
			0,
			{},
			static_cast<std::uint32_t>(submesh.material_index),
			static_cast<std::uint32_t>(geometry.parts.size())});
	}

	if (!asset.Bounds().Is_Valid())
		return false;
	const Assets::Vector3f center{
		(asset.Bounds().minimum.x + asset.Bounds().maximum.x) * 0.5f,
		(asset.Bounds().minimum.y + asset.Bounds().maximum.y) * 0.5f,
		(asset.Bounds().minimum.z + asset.Bounds().maximum.z) * 0.5f};
	const float radius_x = asset.Bounds().maximum.x - center.x;
	const float radius_y = asset.Bounds().maximum.y - center.y;
	const float radius_z = asset.Bounds().maximum.z - center.z;
	geometry.bounds = {
		{center.x, center.y, center.z},
		radius_x > radius_y
			? (radius_x > radius_z ? radius_x : radius_z)
			: (radius_y > radius_z ? radius_y : radius_z)};
	const std::uint32_t vertex_stride = skinned
		? static_cast<std::uint32_t>(sizeof(SkinnedMeshVertex))
		: static_cast<std::uint32_t>(sizeof(StaticMeshVertex));
	const std::span<const std::byte> vertex_data = skinned
		? std::as_bytes(std::span<const SkinnedMeshVertex>(geometry.skinned_vertices))
		: std::as_bytes(std::span<const StaticMeshVertex>(geometry.vertices));
	geometry.source = {
		static_cast<std::uint32_t>(vertices.size()),
		static_cast<std::uint32_t>(geometry.indices.size()),
		vertex_stride,
		MeshIndexFormat::UInt16,
		vertex_data,
		std::as_bytes(std::span<const std::uint16_t>(geometry.indices)),
		geometry.bounds.center,
		geometry.bounds.radius,
		std::span<const MeshPart>(geometry.parts),
		skinned ? MeshVertexFormat::Position3Color4UV2Skinned : MeshVertexFormat::Position3Color4UV2,
		skinned ? skin_bone_count : 0};
	return Validate_Static_Mesh_Source(geometry.source);
}

bool Find_Model_Asset_Part(
	const Assets::ModelAsset &asset,
	std::string_view name,
	ModelPartId &part) noexcept
{
	for (std::size_t index = 0; index < asset.Submeshes().size(); ++index) {
		if (asset.Submeshes()[index].name == name) {
			part = static_cast<ModelPartId>(index);
			return true;
		}
	}
	return false;
}

}
