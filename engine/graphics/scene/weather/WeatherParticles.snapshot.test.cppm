module;

#define BOOST_TEST_MODULE GraphicsWeatherParticlesSnapshotTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <filesystem>

export module Graphics.Scene.WeatherParticles.Snapshot.Tests;

import Graphics.Tests.Device;
import Graphics.Scene.Particles.Renderer;
import Graphics.Scene.WeatherParticles;
import Graphics.Testing.VisualRegression;

using namespace Graphics;

#ifndef GRAPHICS_WEATHER_REFERENCE_DIRECTORY
#define GRAPHICS_WEATHER_REFERENCE_DIRECTORY "."
#endif

#ifndef GRAPHICS_WEATHER_FAILURE_DIRECTORY
#define GRAPHICS_WEATHER_FAILURE_DIRECTORY "."
#endif

#ifndef GRAPHICS_WEATHER_SHADER_DIRECTORY
#define GRAPHICS_WEATHER_SHADER_DIRECTORY "."
#endif

namespace
{
struct WeatherSnapshotContext final
{
	ParticleRenderer *renderer = nullptr;
};

bool Render_Weather_Field(Device &, CommandList &commands, RHITextureHandle color_target,
	RHITextureHandle depth_target, RHIViewport viewport, void *context) noexcept
{
	const WeatherSnapshotContext &snapshot = *static_cast<const WeatherSnapshotContext *>(context);
	return commands.Set_Render_Targets(color_target, depth_target)
		&& commands.Clear({0.03f, 0.04f, 0.07f, 1.0f}, 1.0f)
		&& snapshot.renderer->Render(commands, color_target, depth_target, viewport);
}

WeatherParticleFieldDescription Make_Field(std::array<float, 4> &starting_heights,
	ParticleRenderer &renderer, ParticleEmitterFlags flags, MaterialHandle material) noexcept
{
	return {
		starting_heights,
		2,
		2,
		1.5f,
		0.75f,
		0.75f,
		0.0f,
		0.0f,
		0.0f,
		Has_Particle_Emitter_Flag(flags, ParticleEmitterFlags::PointSprite) ? 12.0f : 0.18f,
		0.30f,
		{0.70f, 0.86f, 1.0f, 0.85f},
		material,
		flags,
		renderer.Pipeline_For_Flags(flags)
	};
}

static void Run_Weather_Snapshot(ParticleEmitterFlags flags, const char *name)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	ParticleRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_WEATHER_SHADER_DIRECTORY), 1, 4));
	const View view{
		Matrix4x4::Identity(),
		Matrix4x4::Identity(),
		{},
		{0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}
	};
	BOOST_REQUIRE(renderer.Set_View(view));

	const std::array<std::byte, 16> texture_pixels = {
		std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255},
		std::byte{255}, std::byte{255}, std::byte{255}, std::byte{96},
		std::byte{255}, std::byte{255}, std::byte{255}, std::byte{96},
		std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255}
	};
	Texture texture_description;
	texture_description.width = 2;
	texture_description.height = 2;
	texture_description.mip_count = 1;
	texture_description.format = TextureFormat::RGBA8_UNorm;
	texture_description.usage = TextureUsage::Sampled;
	texture_description.row_pitch = 8;
	const TextureHandle texture = renderer.Create_Texture(texture_description, texture_pixels);
	BOOST_REQUIRE(texture.Is_Valid());

	Material material_description;
	material_description.shader = renderer.Particle_Shader();
	material_description.textures[0] = texture;
	material_description.parameters.values[0] = 1.0f;
	material_description.parameters.values[1] = 1.0f;
	material_description.parameters.values[2] = 1.0f;
	material_description.parameters.values[3] = 1.0f;
	const MaterialHandle material = renderer.Create_Material(material_description);
	BOOST_REQUIRE(material.Is_Valid());

	std::array<float, 4> starting_heights = {0.0f, 0.0f, 0.0f, 0.0f};
	WeatherParticles particles;
	BOOST_REQUIRE(particles.Initialize(4));
	const WeatherParticleFieldDescription field = Make_Field(starting_heights, renderer, flags, material);
	BOOST_REQUIRE(particles.Configure(field));
	BOOST_REQUIRE(particles.Bind(renderer));
	BOOST_REQUIRE(particles.Update(field, {0.0f, 0.0f, 0.0f}, 1.0f, view, {}));
	BOOST_REQUIRE_EQUAL(particles.Candidate_Count(), 4);
	BOOST_REQUIRE_EQUAL(particles.Particle_Count(), 4);
	renderer.Reset_Particles();
	BOOST_REQUIRE(particles.Append());

	VisualRegressionHarness harness({
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_WEATHER_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_WEATHER_FAILURE_DIRECTORY)
	});
	WeatherSnapshotContext context{&renderer};
	const VisualComparisonResult result = harness.Run(device, name, Render_Weather_Field, &context);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated weather particle snapshot");
	BOOST_CHECK_MESSAGE(result.matched, "weather particle rendering snapshot mismatch");
	BOOST_CHECK_EQUAL(renderer.Visible_Particle_Count(), 4);
	BOOST_CHECK_EQUAL(renderer.Draw_Count(), 4);

	BOOST_REQUIRE(particles.Unbind(renderer));
	BOOST_REQUIRE(renderer.Destroy_Material(material));
	BOOST_REQUIRE(renderer.Destroy_Texture(texture));
	renderer.Shutdown();
}
}

BOOST_AUTO_TEST_CASE(weather_billboard_field_matches_colocated_snapshot)
{
	Run_Weather_Snapshot(ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard, "weather_particles_billboard");
}

BOOST_AUTO_TEST_CASE(weather_point_sprite_field_matches_colocated_snapshot)
{
	Run_Weather_Snapshot(ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard | ParticleEmitterFlags::PointSprite, "weather_particles_point_sprite");
}
