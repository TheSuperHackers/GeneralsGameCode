module;

#include <array>
#include <cmath>
#include <cstdint>

export module Graphics.Scene.Environment;

export import Graphics.Scene.Lighting;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export enum class EnvironmentFlags : std::uint32_t
{
	None = 0,
	SkyEnabled = 1u << 0,
	SunEnabled = 1u << 1,
	AmbientEnabled = 1u << 2,
	FogEnabled = 1u << 3
};

export constexpr EnvironmentFlags operator|(EnvironmentFlags left, EnvironmentFlags right) noexcept
{
	return static_cast<EnvironmentFlags>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

export constexpr EnvironmentFlags operator&(EnvironmentFlags left, EnvironmentFlags right) noexcept
{
	return static_cast<EnvironmentFlags>(static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

export constexpr bool Has_Environment_Flag(EnvironmentFlags flags, EnvironmentFlags flag) noexcept
{
	return (flags & flag) == flag;
}

export struct EnvironmentSkyParameters final
{
	Vector3 zenith_color{0.12f, 0.28f, 0.65f};
	Vector3 horizon_color{0.70f, 0.82f, 1.0f};
	float intensity = 1.0f;
};

export struct EnvironmentFogParameters final
{
	Vector3 color{0.60f, 0.70f, 0.80f};
	float density = 0.0f;
	float start_distance = 0.0f;
	float end_distance = 10000.0f;
};

export struct RenderEnvironment final
{
	EnvironmentSkyParameters sky{};
	Vector3 sun_direction{0.0f, 0.0f, -1.0f};
	Vector3 sun_color{1.0f, 1.0f, 1.0f};
	float sun_intensity = 1.0f;
	Vector3 ambient_light{0.20f, 0.24f, 0.30f};
	float ambient_intensity = 1.0f;
	EnvironmentFogParameters fog{};
	float exposure = 0.0f;
	EnvironmentFlags flags = EnvironmentFlags::SkyEnabled
		| EnvironmentFlags::SunEnabled
		| EnvironmentFlags::AmbientEnabled;

	bool Is_Valid() const noexcept;
};

export struct alignas(16) GPUEnvironmentData final
{
	std::array<float, 4> sky_zenith_intensity{};
	std::array<float, 4> sky_horizon{};
	std::array<float, 4> sun_direction_intensity{};
	std::array<float, 4> sun_color{};
	std::array<float, 4> ambient_color_intensity{};
	std::array<float, 4> fog_color_density{};
	std::array<float, 4> fog_start_end_exposure{};
	std::uint32_t flags = 0;
	std::array<std::uint32_t, 3> reserved{};
};

static_assert(sizeof(GPUEnvironmentData) == 128);
static_assert(alignof(GPUEnvironmentData) == 16);

export struct alignas(16) EnvironmentViewData final
{
	std::array<float, 16> view_matrix{};
	std::array<float, 16> projection_matrix{};
	std::array<float, 4> camera_position{};
};

static_assert(sizeof(EnvironmentViewData) == 144);

namespace
{
bool Is_Finite(Vector3 value) noexcept
{
	return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}
}

bool RenderEnvironment::Is_Valid() const noexcept
{
	const float sun_direction_length_squared = sun_direction.x * sun_direction.x
		+ sun_direction.y * sun_direction.y
		+ sun_direction.z * sun_direction.z;
	return Is_Finite(sky.zenith_color)
		&& Is_Finite(sky.horizon_color)
		&& std::isfinite(sky.intensity)
		&& Is_Finite(sun_direction)
		&& sun_direction_length_squared > 0.0f
		&& Is_Finite(sun_color)
		&& std::isfinite(sun_intensity)
		&& Is_Finite(ambient_light)
		&& std::isfinite(ambient_intensity)
		&& Is_Finite(fog.color)
		&& std::isfinite(fog.density)
		&& fog.density >= 0.0f
		&& std::isfinite(fog.start_distance)
		&& std::isfinite(fog.end_distance)
		&& fog.start_distance >= 0.0f
		&& fog.end_distance >= fog.start_distance
		&& std::isfinite(exposure);
}

export GPUEnvironmentData Pack_Environment_Data(const RenderEnvironment &environment) noexcept
{
	GPUEnvironmentData data;
	data.sky_zenith_intensity = {
		environment.sky.zenith_color.x,
		environment.sky.zenith_color.y,
		environment.sky.zenith_color.z,
		environment.sky.intensity
	};
	data.sky_horizon = {
		environment.sky.horizon_color.x,
		environment.sky.horizon_color.y,
		environment.sky.horizon_color.z,
		0.0f
	};
	data.sun_direction_intensity = {
		environment.sun_direction.x,
		environment.sun_direction.y,
		environment.sun_direction.z,
		environment.sun_intensity
	};
	data.sun_color = {
		environment.sun_color.x,
		environment.sun_color.y,
		environment.sun_color.z,
		0.0f
	};
	data.ambient_color_intensity = {
		environment.ambient_light.x,
		environment.ambient_light.y,
		environment.ambient_light.z,
		environment.ambient_intensity
	};
	data.fog_color_density = {
		environment.fog.color.x,
		environment.fog.color.y,
		environment.fog.color.z,
		environment.fog.density
	};
	data.fog_start_end_exposure = {
		environment.fog.start_distance,
		environment.fog.end_distance,
		environment.exposure,
		0.0f
	};
	data.flags = static_cast<std::uint32_t>(environment.flags);
	return data;
}

export EnvironmentViewData Pack_Environment_View(const View &view) noexcept
{
	EnvironmentViewData data;
	data.view_matrix = view.view_matrix.values;
	data.projection_matrix = view.projection_matrix.values;
	data.camera_position = {view.position.x, view.position.y, view.position.z, 0.0f};
	return data;
}

export RenderLight Make_Environment_Sun_Light(const RenderEnvironment &environment) noexcept
{
	RenderLight light;
	light.type = RenderLightType::Directional;
	light.flags = Has_Environment_Flag(environment.flags, EnvironmentFlags::SunEnabled)
		? RenderLightFlags::Enabled
		: RenderLightFlags::None;
	light.direction = environment.sun_direction;
	light.color = environment.sun_color;
	light.intensity = environment.sun_intensity;
	light.range = 0.0f;
	return light;
}

}
