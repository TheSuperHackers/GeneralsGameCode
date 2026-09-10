module;

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>

export module Assets.Adapters.W3D;

import Assets.Adapters.W3D.Chunks;
import Assets.Adapters.W3D.Mesh;
import Assets.Adapters.W3D.Model;
import Assets.Adapters.W3D.Rig;
import Assets.Identity;
import Assets.Importers.Models;
import Assets.Models;

namespace Assets
{
namespace W3DAdapterDetail
{

std::string Extension(std::string_view name)
{
	const std::size_t separator = name.find_last_of('/');
	const std::size_t dot = name.find_last_of('.');
	if (dot == std::string_view::npos || (separator != std::string_view::npos && dot < separator))
		return {};

	std::string extension(name.substr(dot));
	for (char &character : extension) {
		if (character >= 'A' && character <= 'Z')
			character = static_cast<char>(character - 'A' + 'a');
	}
	return extension;
}

bool Has_W3D_Root(W3D::W3DByteSpan source) noexcept
{
	if (source.size() < 8)
		return false;

	std::uint32_t id = 0;
	std::uint32_t encoded_size = 0;
	if (!W3D::W3DRead_U32(source, 0, id) || !W3D::W3DRead_U32(source, 4, encoded_size))
		return false;

	return id == W3D::W3DChunkMesh && (encoded_size & W3D::W3DChunkContainsChildren) != 0 &&
		(encoded_size & W3D::W3DChunkSizeMask) <= source.size() - 8;
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

void Read_Top_Level_Dependency(const W3D::W3DChunkView &chunk, ModelAssetDesc &description)
{
	if (!chunk.contains_children)
		return;

	W3D::W3DVisit_Chunks(chunk.payload, [&description, &chunk](const W3D::W3DChunkView &child) {
		if (chunk.id == W3D::W3DChunkHierarchy && child.id == W3D::W3DChunkHierarchyHeader)
			Add_Dependency(description, AssetType::Skeleton, W3D::W3DRead_Fixed_String(child.payload, 4, 16));
		if (chunk.id == W3D::W3DChunkAnimation && child.id == W3D::W3DChunkAnimationHeader)
			Add_Dependency(description, AssetType::Animation, W3D::W3DRead_Fixed_String(child.payload, 4, 16));
		return true;
	});
}

}

export class W3DAdapter final : public IModelAdapter
{
public:
	bool Can_Import(const AssetIdentity &identity, std::span<const std::byte> source) const noexcept override
	{
		if (W3DAdapterDetail::Extension(identity.canonical_name) == ".w3d")
			return true;
		return W3DAdapterDetail::Has_W3D_Root(source);
	}

	ModelImportResult Import(const AssetIdentity &identity, std::span<const std::byte> source) const override
	{
		if (source.empty())
			return {nullptr, "W3D source is empty"};
		if (!W3D::W3DValidate_Chunk_Tree(source))
			return {nullptr, "W3D source contains a malformed or truncated chunk"};

		auto description = std::make_unique<ModelAssetDesc>();
		description->source_name = identity.canonical_name;
		description->source_format = "W3D";
		bool found_mesh = false;
		std::string error;
		if (!W3D::W3DRead_Model_Rig(source, description->rig, error))
			return {nullptr, std::move(error)};
		if (!W3D::W3DVisit_Chunks(source, [&description, &found_mesh, &error](const W3D::W3DChunkView &chunk) {
			if (chunk.id == W3D::W3DChunkMesh) {
				W3D::W3DParsedMesh mesh;
				if (!chunk.contains_children || !W3D::W3DParse_Mesh(chunk.payload, mesh, error))
					return false;
				W3D::W3DAppend_Mesh(*description, mesh);
				found_mesh = true;
			} else if (chunk.id == W3D::W3DChunkHierarchy || chunk.id == W3D::W3DChunkAnimation) {
				W3DAdapterDetail::Read_Top_Level_Dependency(chunk, *description);
			}
			return true;
		}))
			return {nullptr, error.empty() ? "W3D top-level chunk traversal failed" : std::move(error)};

		if (!found_mesh || description->vertices.empty() || description->indices.empty())
			return {nullptr, "W3D source contains no static mesh"};
		if (description->name.empty())
			description->name = identity.canonical_name;
		if (!description->bounds.Is_Valid())
			return {nullptr, "W3D mesh has invalid bounds"};
		return {std::move(description), {}};
	}
};

}
