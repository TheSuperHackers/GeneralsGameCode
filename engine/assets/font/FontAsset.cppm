module;

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

export module Assets.Fonts;

import Assets.Identity;

namespace Assets
{

export struct FontGlyphAsset final
{
	std::uint16_t character = 0;
	std::uint16_t width = 0;
	std::int16_t spacing = 0;
	std::vector<std::uint8_t> alpha;
};

export class FontAsset final
{
public:
	FontAsset() = default;

	FontAsset(
		AssetIdentity identity,
		std::string family,
		std::uint32_t point_size,
		bool bold,
		std::uint32_t height,
		std::int32_t extra_overlap,
		std::vector<FontGlyphAsset> glyphs)
		: m_identity(std::move(identity)),
		  m_family(std::move(family)),
		  m_point_size(point_size),
		  m_bold(bold),
		  m_height(height),
		  m_extra_overlap(extra_overlap),
		  m_glyphs(std::move(glyphs))
	{
		std::sort(m_glyphs.begin(), m_glyphs.end(), [](const FontGlyphAsset &left, const FontGlyphAsset &right) {
			return left.character < right.character;
		});
	}

	FontAsset(
		std::string family,
		std::uint32_t point_size,
		bool bold,
		std::uint32_t height,
		std::int32_t extra_overlap,
		std::vector<FontGlyphAsset> glyphs)
		: FontAsset({}, std::move(family), point_size, bold, height, extra_overlap, std::move(glyphs))
	{
	}

	const AssetIdentity &Identity() const noexcept { return m_identity; }
	const std::string &Family() const noexcept { return m_family; }
	std::uint32_t Point_Size() const noexcept { return m_point_size; }
	bool Bold() const noexcept { return m_bold; }
	std::uint32_t Height() const noexcept { return m_height; }
	std::int32_t Extra_Overlap() const noexcept { return m_extra_overlap; }
	const std::vector<FontGlyphAsset> &Glyphs() const noexcept { return m_glyphs; }

private:
	AssetIdentity m_identity{};
	std::string m_family;
	std::uint32_t m_point_size = 0;
	bool m_bold = false;
	std::uint32_t m_height = 0;
	std::int32_t m_extra_overlap = 0;
	std::vector<FontGlyphAsset> m_glyphs;
};

}
