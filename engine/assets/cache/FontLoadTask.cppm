module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

export module Assets.Cache.FontLoadTask;

import Assets.Fonts;
import Assets.Identity;
import Assets.Importers.Models;

namespace Assets
{

export struct FontLoadResult final
{
	std::shared_ptr<const FontAsset> asset;
	std::string error;

	bool Succeeded() const noexcept
	{
		return asset != nullptr && error.empty();
	}
};

export FontLoadResult Load_Font_Asset(const AssetIdentity &identity, const AssetSource &source);

namespace FontLoadDetail
{

struct FontRequest final
{
	std::string family;
	std::uint32_t point_size = 0;
	bool bold = false;
	std::uint32_t average_width = 0;
};

bool Parse_Request(std::string_view name, FontRequest &request)
{
	if (!name.starts_with("font/"))
		return false;
	const std::size_t family_end = name.find('/', 5);
	if (family_end == std::string_view::npos)
		return false;
	const std::size_t size_end = name.find('/', family_end + 1);
	if (size_end == std::string_view::npos)
		return false;
	request.family = std::string(name.substr(5, family_end - 5));
	try {
		request.point_size = static_cast<std::uint32_t>(std::stoul(
			std::string(name.substr(family_end + 1, size_end - family_end - 1))));
		const std::size_t width_start = name.find('/', size_end + 1);
		request.bold = name.substr(size_end + 1, width_start - size_end - 1) == "1";
		if (width_start != std::string_view::npos)
			request.average_width = static_cast<std::uint32_t>(std::stoul(
				std::string(name.substr(width_start + 1))));
	}
	catch (...) {
		return false;
	}
	return !request.family.empty() && request.point_size != 0;
}

std::int32_t Round_Int(float value) noexcept
{
	return static_cast<std::int32_t>(std::lround(value));
}

std::shared_ptr<const FontAsset> Decode(
	const AssetIdentity &identity,
	const FontRequest &request,
	std::span<const std::byte> source)
{
	if (source.empty())
		return {};

	stbtt_fontinfo font;
	const auto *font_bytes = reinterpret_cast<const unsigned char *>(source.data());
	if (stbtt_InitFont(&font, font_bytes, stbtt_GetFontOffsetForIndex(font_bytes, 0)) == 0)
		return {};

	const std::int32_t pixel_height = std::max<std::int32_t>(
		1, static_cast<std::int32_t>(request.point_size * 96ull / 72ull));
	// Point sizes specify the em square, not the ascent/descent bounding box.
	const float scale = stbtt_ScaleForMappingEmToPixels(&font, static_cast<float>(pixel_height));
	float horizontal_scale = scale;
	if (request.average_width != 0) {
		// stb's table reader is local to this decoder's implementation unit.
		const auto metrics_table = stbtt__find_table(font.data, font.fontstart, "OS/2");
		if (metrics_table == 0 || metrics_table + 4 > source.size())
			return {};
		const int average_width = ttSHORT(font.data + metrics_table + 2);
		if (average_width <= 0)
			return {};
		horizontal_scale = static_cast<float>(request.average_width) / average_width;
	}
	int ascent = 0;
	int descent = 0;
	int line_gap = 0;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
	const std::uint32_t height = static_cast<std::uint32_t>(std::max<std::int32_t>(
		1, Round_Int(ascent * scale) - Round_Int(descent * scale)));
	const std::int32_t overlap = std::clamp(pixel_height / 8, 0, 4);

	std::vector<FontGlyphAsset> glyphs;
	glyphs.reserve(512);
	for (std::uint32_t value = 0; value <= 0xffffu; ++value) {
		const int character = static_cast<int>(value);
		if (stbtt_FindGlyphIndex(&font, character) == 0)
			continue;
		int advance = 0;
		int left_bearing = 0;
		stbtt_GetCodepointHMetrics(&font, character, &advance, &left_bearing);
		(void)left_bearing;
		int width = 0;
		int bitmap_height = 0;
		int x_offset = 0;
		int y_offset = 0;
		unsigned char *bitmap = stbtt_GetCodepointBitmap(
			&font, horizontal_scale, scale, character, &width, &bitmap_height, &x_offset, &y_offset);
		FontGlyphAsset glyph;
		glyph.character = static_cast<std::uint16_t>(value);
		glyph.width = static_cast<std::uint16_t>(std::clamp(width, 0, 0xffff));
		glyph.spacing = static_cast<std::int16_t>(std::clamp(
			Round_Int(advance * horizontal_scale), -32768, 32767));
		if (width > 0 && bitmap != nullptr) {
			glyph.alpha.assign(static_cast<std::size_t>(width) * height, 0);
			const int baseline = Round_Int(ascent * scale);
			for (int y = 0; y < bitmap_height; ++y) {
				const int destination_y = baseline + y_offset + y;
				if (destination_y < 0 || destination_y >= static_cast<int>(height))
					continue;
				for (int x = 0; x < width; ++x)
					glyph.alpha[static_cast<std::size_t>(destination_y) * width + x] =
						bitmap[static_cast<std::size_t>(y) * width + x];
			}
		}
		if (bitmap != nullptr)
			stbtt_FreeBitmap(bitmap, nullptr);
		if (glyph.width != 0 || glyph.spacing != 0)
			glyphs.push_back(std::move(glyph));
	}

	return std::make_shared<const FontAsset>(
		identity,
		request.family,
		request.point_size,
		request.bold,
		height,
		overlap,
		std::move(glyphs));
}

}

FontLoadResult Load_Font_Asset(const AssetIdentity &identity, const AssetSource &source)
{
	try {
		if (!source)
			return {nullptr, "no asset source is configured"};
		FontLoadDetail::FontRequest request;
		if (!FontLoadDetail::Parse_Request(identity.canonical_name, request))
			return {nullptr, "invalid font asset identity"};
		const std::vector<std::byte> bytes = source(identity);
		const std::shared_ptr<const FontAsset> asset = FontLoadDetail::Decode(
			identity, request, bytes);
		return asset != nullptr
			? FontLoadResult{asset, {}}
			: FontLoadResult{nullptr, "font source is empty or invalid"};
	}
	catch (const std::exception &exception) {
		return {nullptr, exception.what()};
	}
	catch (...) {
		return {nullptr, "unknown exception while loading font"};
	}
}

}
