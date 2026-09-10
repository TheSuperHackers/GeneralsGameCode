module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

export module Graphics.Scene.Decals;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Shaders.Pipeline;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export inline constexpr std::uint32_t Invalid_Decal_GPU_Index = std::numeric_limits<std::uint32_t>::max();

export enum class RenderDecalFlags : std::uint32_t
{
	None = 0,
	Enabled = 1u << 0
};

export constexpr RenderDecalFlags operator|(RenderDecalFlags left, RenderDecalFlags right) noexcept
{
	return static_cast<RenderDecalFlags>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

export constexpr RenderDecalFlags operator&(RenderDecalFlags left, RenderDecalFlags right) noexcept
{
	return static_cast<RenderDecalFlags>(static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

export constexpr bool Has_Render_Decal_Flag(RenderDecalFlags flags, RenderDecalFlags flag) noexcept
{
	return (flags & flag) == flag;
}

export struct DecalTransform final
{
	Matrix4x4 matrix = Matrix4x4::Identity();
};

export struct DecalBounds final
{
	Vector3 center{};
	float radius = 1.0f;
};

export struct DecalBoundsData final
{
	std::span<const float> center_x{};
	std::span<const float> center_y{};
	std::span<const float> center_z{};
	std::span<const float> radii{};

	std::size_t Size() const noexcept
	{
		return center_x.size();
	}

	DecalBounds operator[](std::size_t dense_index) const noexcept
	{
		return {
			{center_x[dense_index], center_y[dense_index], center_z[dense_index]},
			radii[dense_index]
		};
	}
};

export struct RenderDecal final
{
	DecalTransform transform{};
	DecalBounds bounds{};
	MaterialHandle material{};
	RenderDecalFlags flags = RenderDecalFlags::Enabled;
};

export struct RenderDecalView final
{
	const DecalTransform &transform;
	DecalBounds bounds{};
	const MaterialHandle &material;
	const RenderDecalFlags &flags;
};

export struct RenderDecalData final
{
	std::span<const DecalTransform> transforms{};
	DecalBoundsData bounds{};
	std::span<const MaterialHandle> materials{};
	std::span<const RenderDecalFlags> flags{};
	std::span<const DecalHandle> handles{};

	std::size_t Size() const noexcept
	{
		return transforms.size();
	}
};

export struct alignas(16) GPUDecalData final
{
	std::array<float, 16> transform{};
	std::array<float, 4> bounds{};
	std::uint32_t material_index = Invalid_Decal_GPU_Index;
	std::uint32_t flags = 0;
	std::uint32_t reserved0 = 0;
	std::uint32_t reserved1 = 0;
};

static_assert(sizeof(GPUDecalData) == 96);

export struct alignas(16) DecalDrawData final
{
	std::uint32_t decal_index = Invalid_Decal_GPU_Index;
	std::uint32_t material_index = Invalid_Decal_GPU_Index;
	PipelineHandle pipeline{};
	std::uint64_t sort_key = 0;
};

static_assert(sizeof(DecalDrawData) == 32);

export PipelineDesc Make_Decal_Pipeline(PipelineDesc description) noexcept
{
	description.depth_test = true;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Alpha;
	return description;
}

}
