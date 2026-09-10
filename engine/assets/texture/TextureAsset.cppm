module;

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <utility>
#include <vector>

export module Assets.Textures;

import Assets.Identity;

namespace Assets
{

export class TextureAsset final
{
public:
	TextureAsset(AssetIdentity identity, std::string source_format, std::size_t source_size);
	TextureAsset(
		AssetIdentity identity,
		std::string source_format,
		std::size_t source_size,
		std::uint32_t width,
		std::uint32_t height,
		std::uint32_t row_pitch,
		std::vector<std::byte> pixels);

	const AssetIdentity &Identity() const noexcept;
	const std::string &Source_Format() const noexcept;
	std::size_t Source_Size() const noexcept;
	std::uint32_t Width() const noexcept;
	std::uint32_t Height() const noexcept;
	std::uint32_t Row_Pitch() const noexcept;
	std::span<const std::byte> Pixels() const noexcept;
	bool Has_Pixels() const noexcept;

private:
	AssetIdentity m_identity;
	std::string m_source_format;
	std::size_t m_source_size = 0;
	std::uint32_t m_width = 0;
	std::uint32_t m_height = 0;
	std::uint32_t m_row_pitch = 0;
	std::vector<std::byte> m_pixels;
};

}

namespace Assets
{

TextureAsset::TextureAsset(AssetIdentity identity, std::string source_format, std::size_t source_size)
	: m_identity(std::move(identity)),
	  m_source_format(std::move(source_format)),
	  m_source_size(source_size)
{
}

TextureAsset::TextureAsset(
	AssetIdentity identity,
	std::string source_format,
	std::size_t source_size,
	std::uint32_t width,
	std::uint32_t height,
	std::uint32_t row_pitch,
	std::vector<std::byte> pixels)
	: m_identity(std::move(identity)),
	  m_source_format(std::move(source_format)),
	  m_source_size(source_size),
	  m_width(width),
	  m_height(height),
	  m_row_pitch(row_pitch),
	  m_pixels(std::move(pixels))
{
}

const AssetIdentity &TextureAsset::Identity() const noexcept
{
	return m_identity;
}

const std::string &TextureAsset::Source_Format() const noexcept
{
	return m_source_format;
}

std::size_t TextureAsset::Source_Size() const noexcept
{
	return m_source_size;
}

std::uint32_t TextureAsset::Width() const noexcept
{
	return m_width;
}

std::uint32_t TextureAsset::Height() const noexcept
{
	return m_height;
}

std::uint32_t TextureAsset::Row_Pitch() const noexcept
{
	return m_row_pitch;
}

std::span<const std::byte> TextureAsset::Pixels() const noexcept
{
	return m_pixels;
}

bool TextureAsset::Has_Pixels() const noexcept
{
	return m_width != 0 && m_height != 0 && m_row_pitch >= m_width * 4 && !m_pixels.empty();
}

}
