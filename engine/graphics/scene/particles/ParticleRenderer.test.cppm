module;

#define BOOST_TEST_MODULE GraphicsParticleRendererTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

export module Graphics.Scene.Particles.Renderer.Tests;

import Graphics.Scene.Particles.Renderer;
import Graphics.Testing.VisualRegression;
import Graphics.Tests.Device;

using namespace Graphics;

#ifndef GRAPHICS_PARTICLE_REFERENCE_DIRECTORY
#define GRAPHICS_PARTICLE_REFERENCE_DIRECTORY "."
#endif

#ifndef GRAPHICS_PARTICLE_FAILURE_DIRECTORY
#define GRAPHICS_PARTICLE_FAILURE_DIRECTORY "."
#endif

#ifndef GRAPHICS_PARTICLE_SHADER_DIRECTORY
#define GRAPHICS_PARTICLE_SHADER_DIRECTORY "."
#endif

namespace
{
Matrix4x4 Make_D3D_Projection(float near_clip, float far_clip) noexcept
{
	Matrix4x4 projection{};
	projection.values[0] = 1.0f;
	projection.values[5] = 1.0f;
	projection.values[10] = -far_clip / (far_clip - near_clip);
	projection.values[11] = -far_clip * near_clip / (far_clip - near_clip);
	projection.values[14] = -1.0f;
	return projection;
}

struct ParticleSnapshot final
{
	std::string_view name;
	ParticleEmitterFlags flags;
	float size;
	std::array<float, 4> color;
};

constexpr std::array<ParticleSnapshot, 6> particle_snapshots = {{
	{"ParticleRenderer", ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard, 0.35f, {1.0f, 1.0f, 1.0f, 1.0f}},
	{"ParticleRenderer.Oriented", ParticleEmitterFlags::Enabled, 0.35f, {1.0f, 0.5f, 0.1f, 1.0f}},
	{"ParticleRenderer.PointSprite", ParticleEmitterFlags::Enabled | ParticleEmitterFlags::PointSprite, 16.0f, {0.1f, 1.0f, 0.2f, 0.9f}},
	{"ParticleRenderer.AlphaTest", ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard | ParticleEmitterFlags::AlphaTest, 0.35f, {0.2f, 0.8f, 1.0f, 1.0f}},
	{"ParticleRenderer.Additive", ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard | ParticleEmitterFlags::Additive, 0.35f, {0.25f, 0.5f, 1.0f, 0.5f}},
	{"ParticleRenderer.Multiply", ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard | ParticleEmitterFlags::Multiply, 0.35f, {0.5f, 0.8f, 1.0f, 1.0f}}
}};

bool Render_Particle_Scene(Device &, CommandList &commands, RHITextureHandle color_target, RHITextureHandle depth_target, RHIViewport viewport, void *context) noexcept
{
	ParticleRenderer &renderer = *static_cast<ParticleRenderer *>(context);
	return commands.Set_Render_Targets(color_target, depth_target)
		&& commands.Clear({0.5f, 0.5f, 0.5f, 1.0f}, 1.0f)
		&& renderer.Render(commands, color_target, depth_target, viewport);
}
}

BOOST_AUTO_TEST_CASE(particle_renderer_matches_colocated_golden_images)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	ParticleRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 1, 1));

	const View view{
		Matrix4x4::Identity(),
		Matrix4x4::Identity(),
		{},
		{0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}
	};
	BOOST_REQUIRE(renderer.Set_View(view));

	ParticleEmitter emitter;
	emitter.material = renderer.Default_Material();
	emitter.max_particles = 1;
	const ParticleEmitterHandle emitter_handle = renderer.Create_Emitter(emitter);
	BOOST_REQUIRE(emitter_handle.Is_Valid());

	VisualRegressionHarness harness({
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_PARTICLE_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_PARTICLE_FAILURE_DIRECTORY)
	});
	for (const ParticleSnapshot &snapshot : particle_snapshots) {
		emitter.flags = snapshot.flags;
		emitter.particle_size = snapshot.size;
		emitter.color = snapshot.color;
		emitter.pipeline = renderer.Pipeline_For_Flags(snapshot.flags);
		BOOST_REQUIRE(renderer.Update_Emitter(emitter_handle, emitter));
		renderer.Reset_Particles();
		BOOST_REQUIRE(renderer.Spawn(emitter_handle, 1));

		const VisualComparisonResult result = harness.Run(device, snapshot.name, Render_Particle_Scene, &renderer);
		BOOST_CHECK(renderer.Particle_Count() == 1);
		BOOST_CHECK(renderer.Visible_Particle_Count() == 1);
		BOOST_CHECK(renderer.Draw_Count() == 1);
		BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated particle renderer reference image");
		BOOST_CHECK_MESSAGE(result.matched, "particle renderer visual regression mismatch");
	}

	BOOST_REQUIRE(renderer.Destroy_Emitter(emitter_handle));
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(particle_renderer_samples_bindless_material_texture)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	ParticleRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 1, 1));
	BOOST_REQUIRE(renderer.Set_View({
		Matrix4x4::Identity(),
		Matrix4x4::Identity(),
		{},
		{0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}
	}));

	const std::array<std::byte, 16> texture_pixels = {
		std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255},
		std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255},
		std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255},
		std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255}
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

	Material material;
	material.shader = renderer.Particle_Shader();
	material.textures[0] = texture;
	material.parameters.values[0] = 1.0f;
	material.parameters.values[1] = 1.0f;
	material.parameters.values[2] = 1.0f;
	material.parameters.values[3] = 1.0f;
	const MaterialHandle material_handle = renderer.Create_Material(material);
	BOOST_REQUIRE(material_handle.Is_Valid());

	ParticleEmitter emitter;
	emitter.material = material_handle;
	emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard | ParticleEmitterFlags::Additive;
	emitter.color = {0.65f, 0.65f, 0.65f, 0.0f};
	emitter.particle_size = 0.35f;
	emitter.max_particles = 1;
	emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
	const ParticleEmitterHandle emitter_handle = renderer.Create_Emitter(emitter);
	BOOST_REQUIRE(emitter_handle.Is_Valid());
	BOOST_REQUIRE(renderer.Spawn(emitter_handle, 1));

	VisualRegressionHarness harness({
		128,
		72,
		2,
		{},
		{}
	});
	RGBAImage actual;
	BOOST_REQUIRE(harness.Render_Offscreen(device, Render_Particle_Scene, &renderer, actual));
	BOOST_REQUIRE_EQUAL(renderer.Particle_Count(), 1);
	BOOST_REQUIRE_EQUAL(renderer.Visible_Particle_Count(), 1);
	BOOST_REQUIRE_EQUAL(renderer.Draw_Count(), 1);
	const std::size_t center_pixel = (static_cast<std::size_t>(36) * actual.width + 64) * 4;
	BOOST_REQUIRE(actual.pixels.size() > center_pixel + 3);
	BOOST_TEST_MESSAGE("center=" << static_cast<unsigned>(actual.pixels[center_pixel]) << "," << static_cast<unsigned>(actual.pixels[center_pixel + 1]) << "," << static_cast<unsigned>(actual.pixels[center_pixel + 2]) << "," << static_cast<unsigned>(actual.pixels[center_pixel + 3]));
	BOOST_CHECK_MESSAGE(actual.pixels[center_pixel + 1] > actual.pixels[center_pixel] + 10, "unexpected center RGB: " << static_cast<unsigned>(actual.pixels[center_pixel]) << "," << static_cast<unsigned>(actual.pixels[center_pixel + 1]) << "," << static_cast<unsigned>(actual.pixels[center_pixel + 2]));
	BOOST_CHECK_MESSAGE(actual.pixels[center_pixel + 1] > actual.pixels[center_pixel + 2] + 10, "unexpected center RGB: " << static_cast<unsigned>(actual.pixels[center_pixel]) << "," << static_cast<unsigned>(actual.pixels[center_pixel + 1]) << "," << static_cast<unsigned>(actual.pixels[center_pixel + 2]));

	BOOST_REQUIRE(renderer.Destroy_Emitter(emitter_handle));
	BOOST_REQUIRE(renderer.Destroy_Material(material_handle));
	BOOST_REQUIRE(renderer.Destroy_Texture(texture));
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(particle_renderer_keeps_interleaved_texture_indices_in_dx11_range)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	ParticleRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 1, 1));
	BOOST_REQUIRE(renderer.Set_View({
		Matrix4x4::Identity(),
		Matrix4x4::Identity(),
		{},
		{0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}
	}));

	const std::array<std::byte, 16> texture_pixels = {
		std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255},
		std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255},
		std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255},
		std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255}
	};
	Texture texture_description;
	texture_description.width = 2;
	texture_description.height = 2;
	texture_description.mip_count = 1;
	texture_description.format = TextureFormat::RGBA8_UNorm;
	texture_description.usage = TextureUsage::Sampled;
	texture_description.row_pitch = 8;

	std::vector<TextureHandle> textures;
	std::vector<MaterialHandle> materials;
	textures.reserve(64);
	materials.reserve(64);
	for (std::uint32_t index = 0; index < 64; ++index) {
		const TextureHandle texture = renderer.Create_Texture(texture_description, texture_pixels);
		BOOST_REQUIRE(texture.Is_Valid());
		textures.push_back(texture);

		Material material;
		material.shader = renderer.Particle_Shader();
		material.textures[0] = texture;
		material.parameters.values[0] = 1.0f;
		material.parameters.values[1] = 1.0f;
		material.parameters.values[2] = 1.0f;
		material.parameters.values[3] = 1.0f;
		const MaterialHandle material_handle = renderer.Create_Material(material);
		BOOST_REQUIRE(material_handle.Is_Valid());
		materials.push_back(material_handle);
	}

	ParticleEmitter emitter;
	emitter.material = materials.back();
	emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard | ParticleEmitterFlags::Additive;
	emitter.color = {1.0f, 1.0f, 1.0f, 1.0f};
	emitter.particle_size = 0.35f;
	emitter.max_particles = 1;
	emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
	const ParticleEmitterHandle emitter_handle = renderer.Create_Emitter(emitter);
	BOOST_REQUIRE(emitter_handle.Is_Valid());
	BOOST_REQUIRE(renderer.Spawn(emitter_handle, 1));

	VisualRegressionHarness harness({128, 72, 2, {}, {}});
	RGBAImage actual;
	BOOST_REQUIRE(harness.Render_Offscreen(device, Render_Particle_Scene, &renderer, actual));
	BOOST_CHECK_EQUAL(renderer.Visible_Particle_Count(), 1);
	BOOST_CHECK_EQUAL(renderer.Draw_Count(), 1);

	BOOST_REQUIRE(renderer.Destroy_Emitter(emitter_handle));
	for (const MaterialHandle material : materials)
		BOOST_REQUIRE(renderer.Destroy_Material(material));
	for (const TextureHandle texture : textures)
		BOOST_REQUIRE(renderer.Destroy_Texture(texture));
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(particle_renderer_draws_with_d3d_depth_projection)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	ParticleRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 1, 1));
	const View view{
		Matrix4x4::Identity(),
		Make_D3D_Projection(1.0f, 100.0f),
		{},
		{0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}
	};
	BOOST_REQUIRE(renderer.Set_View(view));

	ParticleEmitter emitter;
	emitter.position = {0.0f, 0.0f, -5.0f};
	emitter.material = renderer.Default_Material();
	emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard;
	emitter.color = {1.0f, 0.0f, 0.0f, 1.0f};
	emitter.particle_size = 0.35f;
	emitter.max_particles = 1;
	emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
	const ParticleEmitterHandle emitter_handle = renderer.Create_Emitter(emitter);
	BOOST_REQUIRE(emitter_handle.Is_Valid());
	BOOST_REQUIRE(renderer.Spawn(emitter_handle, 1));

	VisualRegressionHarness harness({128, 72, 0, {}, {}});
	RGBAImage actual;
	BOOST_REQUIRE(harness.Render_Offscreen(device, Render_Particle_Scene, &renderer, actual));
	BOOST_REQUIRE_EQUAL(renderer.Visible_Particle_Count(), 1);
	BOOST_REQUIRE_EQUAL(renderer.Draw_Count(), 1);

	std::size_t changed_pixels = 0;
	for (std::size_t pixel = 0; pixel < actual.pixels.size(); pixel += 4)
		if (actual.pixels[pixel] > 160 && actual.pixels[pixel + 1] < 100 && actual.pixels[pixel + 2] < 100)
			++changed_pixels;
	BOOST_CHECK(changed_pixels != 0);

	BOOST_REQUIRE(renderer.Destroy_Emitter(emitter_handle));
	renderer.Shutdown();
}
