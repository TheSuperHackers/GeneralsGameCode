module;

#define BOOST_TEST_MODULE GraphicsWeatherParticlesTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <type_traits>

export module Graphics.Scene.WeatherParticles.Tests;

import Graphics.Scene.WeatherParticles;

using namespace Graphics;

static_assert(!std::is_convertible_v<MaterialHandle, ParticleEmitterHandle>);

namespace
{
template <std::size_t Count>
WeatherParticleFieldDescription Make_Field(std::array<float, Count> &starting_heights) noexcept
{
	return {
		starting_heights,
		4,
		4,
		4.0f,
		1.0f,
		2.0f,
		0.25f,
		0.50f,
		0.10f,
		0.25f,
		0.35f,
		{0.25f, 0.5f, 0.75f, 0.8f},
		MaterialHandle(4, 2),
		ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard,
		PipelineHandle(8, 3)
	};
}

View No_Cull_View() noexcept
{
	return {};
}
}

BOOST_AUTO_TEST_CASE(weather_particle_lifecycle_is_fixed_capacity)
{
	WeatherParticles particles;
	BOOST_CHECK(!particles.Is_Initialized());
	BOOST_REQUIRE(particles.Initialize(32));
	BOOST_CHECK(particles.Is_Initialized());
	BOOST_CHECK_EQUAL(particles.Capacity(), 32);
	BOOST_CHECK(!particles.Initialize(32));
	particles.Shutdown();
	BOOST_CHECK(!particles.Is_Initialized());
}

BOOST_AUTO_TEST_CASE(weather_field_preserves_density_and_fall_speed)
{
	std::array<float, 16> starting_heights{};
	WeatherParticleFieldDescription field = Make_Field(starting_heights);
	WeatherParticles particles;
	BOOST_REQUIRE(particles.Initialize(16));
	BOOST_REQUIRE(particles.Configure(field));
	BOOST_REQUIRE(particles.Update(field, {0.0f, 0.0f, 0.0f}, 0.0f, No_Cull_View(), {}));
	BOOST_CHECK_EQUAL(particles.Candidate_Count(), 16);
	BOOST_CHECK_EQUAL(particles.Particle_Count(), 16);
	const float first_height = particles.Data().position_z[0];
	BOOST_REQUIRE(particles.Update(field, {0.0f, 0.0f, 0.0f}, 0.5f, No_Cull_View(), {}));
	const ParticleData after = particles.Data();
	BOOST_CHECK_CLOSE(after.position_z[0], first_height - 1.0f, 0.001);
	BOOST_CHECK_EQUAL(after.sizes[0], field.particle_size);
	BOOST_CHECK_EQUAL(after.color_a[0], field.color[3]);
}

BOOST_AUTO_TEST_CASE(weather_field_culls_to_bounds_and_follows_camera_cells)
{
	std::array<float, 4> starting_heights{};
	WeatherParticleFieldDescription field = Make_Field(starting_heights);
	field.noise_width = 2;
	field.noise_height = 2;
	field.box_dimensions = 2.0f;
	field.emitter_spacing = 1.0f;
	field.drift_amplitude = 0.0f;
	WeatherParticles particles;
	BOOST_REQUIRE(particles.Initialize(4));
	BOOST_REQUIRE(particles.Configure(field));

	WeatherParticleCullingBounds bounds;
	bounds.enabled = true;
	bounds.center = {0.0f, 0.0f, 1.0f};
	bounds.extent = {0.1f, 0.1f, 0.1f};
	BOOST_REQUIRE(particles.Update(field, {0.0f, 0.0f, 0.0f}, 0.0f, No_Cull_View(), bounds));
	BOOST_CHECK_EQUAL(particles.Candidate_Count(), 4);
	BOOST_CHECK_EQUAL(particles.Particle_Count(), 1);
	BOOST_CHECK_EQUAL(particles.Data().position_x[0], 0.0f);
	BOOST_CHECK_EQUAL(particles.Data().position_y[0], 0.0f);

	bounds.enabled = false;
	BOOST_REQUIRE(particles.Update(field, {1.0f, 0.0f, 0.0f}, 0.0f, No_Cull_View(), bounds));
	BOOST_CHECK_EQUAL(particles.Particle_Count(), 4);
	BOOST_CHECK_EQUAL(particles.Data().position_x[0], 0.0f);
}

BOOST_AUTO_TEST_CASE(weather_field_output_is_deterministic)
{
	std::array<float, 16> starting_heights{};
	for (std::size_t index = 0; index < starting_heights.size(); ++index)
		starting_heights[index] = static_cast<float>(index) * 0.125f;
	const WeatherParticleFieldDescription field = Make_Field(starting_heights);
	WeatherParticles first;
	WeatherParticles second;
	BOOST_REQUIRE(first.Initialize(16));
	BOOST_REQUIRE(second.Initialize(16));
	BOOST_REQUIRE(first.Configure(field));
	BOOST_REQUIRE(second.Configure(field));
	BOOST_REQUIRE(first.Update(field, {3.25f, -2.5f, 7.0f}, 1.75f, No_Cull_View(), {}));
	BOOST_REQUIRE(second.Update(field, {3.25f, -2.5f, 7.0f}, 1.75f, No_Cull_View(), {}));
	const ParticleData left = first.Data();
	const ParticleData right = second.Data();
	BOOST_REQUIRE_EQUAL(left.Size(), right.Size());
	for (std::size_t index = 0; index < left.Size(); ++index) {
		BOOST_CHECK_EQUAL(left.position_x[index], right.position_x[index]);
		BOOST_CHECK_EQUAL(left.position_y[index], right.position_y[index]);
		BOOST_CHECK_EQUAL(left.position_z[index], right.position_z[index]);
		BOOST_CHECK_EQUAL(left.color_a[index], right.color_a[index]);
	}
}
