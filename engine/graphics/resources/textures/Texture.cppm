module;

#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.Resources.Textures.Texture;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Resources.Pools.ResourcePool;

namespace Graphics
{

export enum class TextureFormat : std::uint8_t
{
	Unknown,
	R8_UNorm,
	RG8_UNorm,
	RGBA8_UNorm,
	BGRA8_UNorm,
	RGBA16_Float,
	RGBA32_Float,
	Depth32_Float
};

export enum class TextureUsage : std::uint32_t
{
	None = 0,
	Sampled = 1u << 0,
	Storage = 1u << 1,
	RenderTarget = 1u << 2,
	DepthStencil = 1u << 3
};

export constexpr TextureUsage operator|(TextureUsage left, TextureUsage right) noexcept
{
	return static_cast<TextureUsage>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

export constexpr TextureUsage operator&(TextureUsage left, TextureUsage right) noexcept
{
	return static_cast<TextureUsage>(static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

export constexpr bool Has_Texture_Usage(TextureUsage usage, TextureUsage flag) noexcept
{
	return (usage & flag) == flag;
}

export struct Texture final
{
	using Dimension = std::uint32_t;
	using MipCount = std::uint32_t;

	Dimension width = 0;
	Dimension height = 0;
	Dimension depth = 1;
	MipCount mip_count = 0;
	TextureFormat format = TextureFormat::Unknown;
	TextureUsage usage = TextureUsage::None;
	std::span<const std::byte> pixel_data{};
	std::uint32_t row_pitch = 0;
	std::uint32_t revision = 1;

	void Mark_Dirty() noexcept
	{
		++revision;
		if (revision == 0)
			revision = 1;
	}
};

export using TexturePool = ResourcePool<Texture, TextureHandle>;

}
