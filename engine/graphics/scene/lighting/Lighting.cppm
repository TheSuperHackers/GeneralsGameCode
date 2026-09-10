module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

export module Graphics.Scene.Lighting;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export inline constexpr std::uint32_t Invalid_Shadow_Data_Index = std::numeric_limits<std::uint32_t>::max();

export enum class RenderLightType : std::uint8_t
{
	Directional,
	Point,
	Spot
};

export enum class RenderLightFlags : std::uint32_t
{
	None = 0,
	Enabled = 1u << 0
};

export constexpr RenderLightFlags operator|(RenderLightFlags left, RenderLightFlags right) noexcept
{
	return static_cast<RenderLightFlags>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

export constexpr RenderLightFlags operator&(RenderLightFlags left, RenderLightFlags right) noexcept
{
	return static_cast<RenderLightFlags>(static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

export constexpr bool Has_Render_Light_Flag(RenderLightFlags flags, RenderLightFlags flag) noexcept
{
	return (flags & flag) == flag;
}

export struct RenderLight final
{
	RenderLightType type = RenderLightType::Point;
	RenderLightFlags flags = RenderLightFlags::Enabled;
	Vector3 position{};
	Vector3 direction{0.0f, 0.0f, -1.0f};
	Vector3 color{1.0f, 1.0f, 1.0f};
	float intensity = 1.0f;
	float range = 1.0f;
	float inner_angle = 0.0f;
	float outer_angle = 0.0f;
};

export struct RenderLightView final
{
	LightHandle handle{};
	RenderLightType type = RenderLightType::Point;
	RenderLightFlags flags = RenderLightFlags::None;
	Vector3 position{};
	Vector3 direction{};
	Vector3 color{};
	float intensity = 0.0f;
	float range = 0.0f;
	float inner_angle = 0.0f;
	float outer_angle = 0.0f;
};

export struct RenderLightData final
{
	std::span<const RenderLightType> types{};
	std::span<const RenderLightFlags> flags{};
	std::span<const float> position_x{};
	std::span<const float> position_y{};
	std::span<const float> position_z{};
	std::span<const float> direction_x{};
	std::span<const float> direction_y{};
	std::span<const float> direction_z{};
	std::span<const float> color_r{};
	std::span<const float> color_g{};
	std::span<const float> color_b{};
	std::span<const float> intensities{};
	std::span<const float> ranges{};
	std::span<const float> inner_angles{};
	std::span<const float> outer_angles{};
	std::span<const LightHandle> handles{};

	std::size_t Size() const noexcept
	{
		return types.size();
	}
};

export struct alignas(16) GPULightData final
{
	std::array<float, 4> position_range{};
	std::array<float, 4> direction_intensity{};
	std::array<float, 4> color_inner_angle{};
	float outer_angle = 0.0f;
	std::uint32_t type = 0;
	std::uint32_t flags = 0;
	std::uint32_t shadow_data_index = Invalid_Shadow_Data_Index;

	friend constexpr bool operator==(const GPULightData &left, const GPULightData &right) noexcept
	{
		for (std::size_t index = 0; index < left.position_range.size(); ++index) {
			if (left.position_range[index] != right.position_range[index]
				|| left.direction_intensity[index] != right.direction_intensity[index]
				|| left.color_inner_angle[index] != right.color_inner_angle[index])
				return false;
		}

		return left.outer_angle == right.outer_angle
			&& left.type == right.type
			&& left.flags == right.flags
			&& left.shadow_data_index == right.shadow_data_index;
	}
};

static_assert(sizeof(GPULightData) == 64);

}
