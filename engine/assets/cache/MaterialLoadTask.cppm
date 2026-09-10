module;

#include <exception>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>

export module Assets.Cache.MaterialLoadTask;

import Assets.Handles;
import Assets.Identity;
import Assets.Materials;

namespace Assets
{

export struct MaterialLoadResult final
{
	std::shared_ptr<const MaterialAsset> asset;
	std::string error;

	bool Succeeded() const noexcept
	{
		return asset != nullptr && error.empty();
	}
};

export MaterialLoadResult Finalize_Material_Asset(
	const AssetIdentity &identity,
	const MaterialAssetDesc &description,
	TextureAssetHandle primary_texture,
	TextureAssetHandle secondary_texture);

export MaterialLoadResult Finalize_Material_Asset(
	const AssetIdentity &identity,
	const MaterialAssetDesc &description,
	TextureAssetHandle primary_texture,
	TextureAssetHandle secondary_texture,
	MaterialSurfaceTextureHandles surface_textures);

MaterialLoadResult Finalize_Material_Asset(
	const AssetIdentity &identity,
	const MaterialAssetDesc &description,
	TextureAssetHandle primary_texture,
	TextureAssetHandle secondary_texture)
{
	return Finalize_Material_Asset(identity, description, primary_texture, secondary_texture, MaterialSurfaceTextureHandles{});
}

MaterialLoadResult Finalize_Material_Asset(
	const AssetIdentity &identity,
	const MaterialAssetDesc &description,
	TextureAssetHandle primary_texture,
	TextureAssetHandle secondary_texture,
	MaterialSurfaceTextureHandles surface_textures)
{
	if (!Validate_Material_Surface(description.surface))
		return {nullptr, "invalid material surface parameters"};
	for (std::size_t index = 0; index < surface_textures.size(); ++index)
		if (!description.surface_textures[index].empty() && !surface_textures[index].Is_Valid())
			return {nullptr, "material surface texture dependency is not available"};
	try {
		return {
			std::make_shared<const MaterialAsset>(identity, description, primary_texture, secondary_texture, surface_textures),
			{}};
	} catch (const std::exception &exception) {
		return {nullptr, exception.what()};
	} catch (...) {
		return {nullptr, "unknown exception while finalizing material"};
	}
}

}
