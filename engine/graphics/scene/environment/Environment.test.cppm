module;

#define BOOST_TEST_MODULE GraphicsEnvironmentTests

#include <boost/test/included/unit_test.hpp>

#include <type_traits>

export module Graphics.Scene.Environment.Tests;

import Graphics.Scene.Environment;

using namespace Graphics;

static_assert(std::is_nothrow_move_constructible_v<RenderEnvironment>);
static_assert(std::is_nothrow_move_assignable_v<RenderEnvironment>);
static_assert(std::is_trivially_copyable_v<GPUEnvironmentData>);
static_assert(std::is_nothrow_move_constructible_v<EnvironmentViewData>);

BOOST_AUTO_TEST_CASE(environment_data_packs_sky_lighting_fog_and_exposure)
{
	RenderEnvironment environment;
	environment.sky.zenith_color = {0.1f, 0.2f, 0.3f};
	environment.sky.horizon_color = {0.4f, 0.5f, 0.6f};
	environment.sky.intensity = 1.25f;
	environment.sun_direction = {0.0f, 1.0f, -1.0f};
	environment.sun_color = {0.7f, 0.8f, 0.9f};
	environment.sun_intensity = 4.0f;
	environment.ambient_light = {0.1f, 0.15f, 0.2f};
	environment.ambient_intensity = 0.75f;
	environment.fog.color = {0.2f, 0.3f, 0.4f};
	environment.fog.density = 0.01f;
	environment.fog.start_distance = 10.0f;
	environment.fog.end_distance = 500.0f;
	environment.exposure = 1.5f;
	environment.flags = EnvironmentFlags::SkyEnabled | EnvironmentFlags::SunEnabled | EnvironmentFlags::FogEnabled;

	BOOST_REQUIRE(environment.Is_Valid());
	const GPUEnvironmentData data = Pack_Environment_Data(environment);
	BOOST_CHECK(data.sky_zenith_intensity[0] == 0.1f);
	BOOST_CHECK(data.sky_zenith_intensity[3] == 1.25f);
	BOOST_CHECK(data.sky_horizon[2] == 0.6f);
	BOOST_CHECK(data.sun_direction_intensity[1] == 1.0f);
	BOOST_CHECK(data.sun_direction_intensity[3] == 4.0f);
	BOOST_CHECK(data.sun_color[0] == 0.7f);
	BOOST_CHECK(data.ambient_color_intensity[3] == 0.75f);
	BOOST_CHECK(data.fog_color_density[3] == 0.01f);
	BOOST_CHECK(data.fog_start_end_exposure[0] == 10.0f);
	BOOST_CHECK(data.fog_start_end_exposure[1] == 500.0f);
	BOOST_CHECK(data.fog_start_end_exposure[2] == 1.5f);
	BOOST_CHECK(data.flags == static_cast<std::uint32_t>(environment.flags));
}

BOOST_AUTO_TEST_CASE(environment_propagates_view_and_sun_to_existing_systems)
{
	const View view(Matrix4x4::Identity(), Matrix4x4::Identity(), {10.0f, 20.0f, 30.0f}, {0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f});
	const EnvironmentViewData view_data = Pack_Environment_View(view);
	BOOST_CHECK(view_data.view_matrix[0] == 1.0f);
	BOOST_CHECK(view_data.projection_matrix[15] == 1.0f);
	BOOST_CHECK(view_data.camera_position[0] == 10.0f);
	BOOST_CHECK(view_data.camera_position[2] == 30.0f);

	RenderEnvironment environment;
	environment.sun_direction = {1.0f, 0.0f, -1.0f};
	environment.sun_color = {0.8f, 0.7f, 0.6f};
	environment.sun_intensity = 3.0f;
	const RenderLight sun = Make_Environment_Sun_Light(environment);
	BOOST_CHECK(sun.type == RenderLightType::Directional);
	BOOST_CHECK(Has_Render_Light_Flag(sun.flags, RenderLightFlags::Enabled));
	BOOST_CHECK(sun.direction.x == 1.0f);
	BOOST_CHECK(sun.direction.z == -1.0f);
	BOOST_CHECK(sun.color.y == 0.7f);
	BOOST_CHECK(sun.intensity == 3.0f);

	environment.flags = EnvironmentFlags::SkyEnabled;
	const RenderLight disabled_sun = Make_Environment_Sun_Light(environment);
	BOOST_CHECK(!Has_Render_Light_Flag(disabled_sun.flags, RenderLightFlags::Enabled));
}

BOOST_AUTO_TEST_CASE(environment_rejects_invalid_fog_and_sun_data)
{
	RenderEnvironment environment;
	environment.fog.end_distance = -1.0f;
	BOOST_CHECK(!environment.Is_Valid());

	environment = {};
	environment.sun_direction = {};
	BOOST_CHECK(!environment.Is_Valid());
}
