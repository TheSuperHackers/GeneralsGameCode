module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.Resources.Materials.Material;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Resources.Pools.ResourcePool;
export import Graphics.Resources.Samplers.Sampler;
export import Graphics.Resources.Textures.Texture;
export import Graphics.Shaders.ParameterLayout;

namespace Graphics
{

export enum class MaterialFlags : std::uint32_t
{
	None = 0,
	Unlit = 1u << 0,
	AlphaTest = 1u << 1,
	Transparent = 1u << 2,
	DoubleSided = 1u << 3,
	VertexColor = 1u << 4,
	PrimaryGradient = 1u << 5,
	SecondaryGradient = 1u << 6,
	Fog = 1u << 7,
	Additive = 1u << 8,
	Multiply = 1u << 9
};

export constexpr MaterialFlags operator|(MaterialFlags left, MaterialFlags right) noexcept
{
	return static_cast<MaterialFlags>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

export constexpr MaterialFlags operator&(MaterialFlags left, MaterialFlags right) noexcept
{
	return static_cast<MaterialFlags>(static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

export constexpr bool Has_Material_Flag(MaterialFlags flags, MaterialFlags flag) noexcept
{
	return (flags & flag) == flag;
}

export struct MaterialParameterBlock final
{
	static constexpr std::size_t ValueCount = 16;
	static constexpr ShaderParameterBlock Block = ShaderParameterBlock::Material;

	std::array<float, ValueCount> values{};

	std::span<const std::byte> Bytes() const noexcept
	{
		return std::as_bytes(std::span<const float>(values));
	}
};

export struct Material final
{
	static constexpr std::size_t TextureSlotCount = 8;
	static constexpr std::size_t SamplerSlotCount = 8;

	ShaderHandle shader{};
	std::array<TextureHandle, TextureSlotCount> textures{};
	std::array<SamplerHandle, SamplerSlotCount> samplers{};
	MaterialParameterBlock parameters{};
	MaterialFlags flags = MaterialFlags::None;
	std::uint32_t revision = 1;

	void Mark_Dirty() noexcept
	{
		++revision;
		if (revision == 0)
			revision = 1;
	}
};

export using MaterialPool = ResourcePool<Material, MaterialHandle>;

}
