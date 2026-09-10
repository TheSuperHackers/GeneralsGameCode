module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module Assets.Cache.TextureLoadTask;

import Assets.Adapters.DDS;
import Assets.Adapters.TGA.Image;
import Assets.Images.Preparation;
import Assets.Identity;
import Assets.Importers.Models;
import Assets.Textures;

namespace Assets
{

export struct TextureLoadResult final
{
	std::shared_ptr<const TextureAsset> asset;
	std::string error;

	bool Succeeded() const noexcept
	{
		return asset != nullptr && error.empty();
	}
};

export TextureLoadResult Load_Texture_Asset(const AssetIdentity &identity, const AssetSource &source);

namespace TextureLoadDetail
{

std::string Source_Format(std::string_view name)
{
	const std::size_t extension = name.find_last_of('.');
	return extension == std::string_view::npos ? "unknown" : std::string(name.substr(extension + 1));
}

bool Decode_TGA(std::span<const std::byte> source, std::uint32_t& width,
    std::uint32_t& height, std::vector<std::byte>& pixels)
{
    TGAImage image;
    if (!Decode_TGA_Image(source, image)) return false;
    std::vector<PreparedImage> prepared;
    if (!Prepare_Image_Levels(image.View(), PixelEncoding::RGBA8,
        image.info.width, image.info.height, 1, {}, prepared)) return false;
    width = image.info.width;
    height = image.info.height;
    pixels = std::move(prepared.front().bytes);
    return true;
}

bool Decode_DDS(std::span<const std::byte> source, std::uint32_t& width,
    std::uint32_t& height, std::vector<std::byte>& pixels)
{
    DDSLayout layout;
    if (!Read_DDS_Layout(source, source.size(), layout)
        || !Decode_DDS_Surface(source, layout, 0, 0, 0, pixels)) return false;
    width = layout.Surface(0)->width;
    height = layout.Surface(0)->height;
    return true;
}

}

TextureLoadResult Load_Texture_Asset(const AssetIdentity &identity, const AssetSource &source)
{
	try {
		if (!source)
			return {nullptr, "no asset source is configured"};
		const std::vector<std::byte> bytes = source(identity);
		if (bytes.empty())
			return {nullptr, "texture source is empty"};

		std::uint32_t width = 0;
		std::uint32_t height = 0;
		std::vector<std::byte> pixels;
		if (!TextureLoadDetail::Decode_TGA(bytes, width, height, pixels))
			TextureLoadDetail::Decode_DDS(bytes, width, height, pixels);
		return {
			std::make_shared<const TextureAsset>(
				identity,
				TextureLoadDetail::Source_Format(identity.canonical_name),
				bytes.size(),
				width,
				height,
				width * 4,
				std::move(pixels)),
			{}};
	} catch (const std::exception &exception) {
		return {nullptr, exception.what()};
	} catch (...) {
		return {nullptr, "unknown exception while loading texture"};
	}
}

}
