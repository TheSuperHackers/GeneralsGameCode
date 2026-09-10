module;

#define BOOST_TEST_MODULE GraphicsWorldQuadsSnapshotTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <filesystem>

export module Graphics.Scene.WorldQuads.Snapshot.Tests;

import Graphics.Tests.Device;
import Graphics.Scene.WorldQuads;
import Graphics.Testing.VisualRegression;

using namespace Graphics;

#ifndef GRAPHICS_WORLD_QUAD_REFERENCE_DIRECTORY
#define GRAPHICS_WORLD_QUAD_REFERENCE_DIRECTORY "."
#endif

#ifndef GRAPHICS_WORLD_QUAD_FAILURE_DIRECTORY
#define GRAPHICS_WORLD_QUAD_FAILURE_DIRECTORY "."
#endif

#ifndef GRAPHICS_WORLD_QUAD_SHADER_DIRECTORY
#define GRAPHICS_WORLD_QUAD_SHADER_DIRECTORY "."
#endif

namespace
{
struct WorldQuadSnapshotContext final
{
	WorldQuadRenderer *renderer = nullptr;
	MaterialHandle normal_material{};
	MaterialHandle highlight_material{};
};

static bool Render_World_Quad_Field(Device &, CommandList &commands, RHITextureHandle color_target,
	RHITextureHandle depth_target, RHIViewport viewport, void *context) noexcept
{
	const WorldQuadSnapshotContext &snapshot = *static_cast<const WorldQuadSnapshotContext *>(context);
	if (!commands.Set_Render_Targets(color_target, depth_target)
		|| !commands.Clear({0.03f, 0.04f, 0.07f, 1.0f}, 1.0f))
		return false;

	snapshot.renderer->Clear();
	const std::array<std::array<float, 3>, 4> left = {{
		{{-0.90f, -0.72f, 0.30f}},
		{{-0.08f, -0.72f, 0.30f}},
		{{-0.08f, 0.72f, 0.30f}},
		{{-0.90f, 0.72f, 0.30f}}
	}};
	const std::array<std::array<float, 3>, 4> right = {{
		{{0.08f, -0.72f, 0.20f}},
		{{0.90f, -0.72f, 0.20f}},
		{{0.90f, 0.72f, 0.20f}},
		{{0.08f, 0.72f, 0.20f}}
	}};
	WorldQuadDescription normal;
	normal.corners = left;
	normal.color = {0.72f, 0.81f, 0.94f, 0.80f};
	normal.material = snapshot.normal_material;
	normal.pipeline = snapshot.renderer->Pipeline();
	normal.sort_group = 0;
	WorldQuadDescription highlight;
	highlight.corners = right;
	highlight.color = {0.86f, 0.70f, 0.58f, 0.74f};
	highlight.material = snapshot.highlight_material;
	highlight.pipeline = snapshot.renderer->Pipeline();
	highlight.sort_group = 1;
	WorldQuadDescription culled = normal;
	for (std::array<float, 3> &corner : culled.corners)
		corner[0] += 3.0f;
	return snapshot.renderer->Submit(normal)
		&& snapshot.renderer->Submit(highlight)
		&& snapshot.renderer->Submit(culled)
		&& snapshot.renderer->Render(commands, color_target, depth_target, viewport);
}

static MaterialHandle Make_Material(WorldQuadRenderer &renderer, TextureHandle texture)
{
	Material description;
	description.shader = renderer.World_Quad_Shader();
	description.textures[0] = texture;
	description.flags = MaterialFlags::Unlit | MaterialFlags::Transparent
		| MaterialFlags::DoubleSided | MaterialFlags::VertexColor;
	description.parameters.values[0] = 1.0f;
	description.parameters.values[1] = 1.0f;
	description.parameters.values[2] = 1.0f;
	description.parameters.values[3] = 1.0f;
	return renderer.Create_Material(description);
}

static void Run_World_Quad_Snapshot(const char *name)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	WorldQuadRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_WORLD_QUAD_SHADER_DIRECTORY), 4, 2));
	const View view{
		Matrix4x4::Identity(),
		Matrix4x4::Identity(),
		{},
		{0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}};
	BOOST_REQUIRE(renderer.Set_View(view));

	const std::array<std::byte, 16> normal_pixels = {
		std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255},
		std::byte{220}, std::byte{240}, std::byte{255}, std::byte{180},
		std::byte{64}, std::byte{96}, std::byte{128}, std::byte{120},
		std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255}};
	const std::array<std::byte, 16> highlight_pixels = {
		std::byte{255}, std::byte{224}, std::byte{192}, std::byte{255},
		std::byte{224}, std::byte{128}, std::byte{96}, std::byte{180},
		std::byte{128}, std::byte{64}, std::byte{32}, std::byte{140},
		std::byte{255}, std::byte{224}, std::byte{192}, std::byte{255}};
	Texture texture_description;
	texture_description.width = 2;
	texture_description.height = 2;
	texture_description.mip_count = 1;
	texture_description.format = TextureFormat::RGBA8_UNorm;
	texture_description.usage = TextureUsage::Sampled;
	texture_description.row_pitch = 8;
	const TextureHandle normal_texture = renderer.Create_Texture(texture_description, normal_pixels);
	BOOST_REQUIRE(normal_texture.Is_Valid());
	const TextureHandle highlight_texture = renderer.Create_Texture(texture_description, highlight_pixels);
	BOOST_REQUIRE(highlight_texture.Is_Valid());
	const MaterialHandle normal_material = Make_Material(renderer, normal_texture);
	BOOST_REQUIRE(normal_material.Is_Valid());
	const MaterialHandle highlight_material = Make_Material(renderer, highlight_texture);
	BOOST_REQUIRE(highlight_material.Is_Valid());

	VisualRegressionHarness harness({
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_WORLD_QUAD_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_WORLD_QUAD_FAILURE_DIRECTORY)});
	WorldQuadSnapshotContext context{&renderer, normal_material, highlight_material};
	const VisualComparisonResult result = harness.Run(device, name, Render_World_Quad_Field, &context);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated world-quad snapshot");
	BOOST_CHECK_MESSAGE(result.matched, "world-quad production shader snapshot mismatch");
	BOOST_CHECK_EQUAL(renderer.Quad_Count(), 3);
	BOOST_CHECK_EQUAL(renderer.Visible_Quad_Count(), 2);
	BOOST_CHECK_EQUAL(renderer.Draw_Count(), 1);

	BOOST_REQUIRE(renderer.Destroy_Material(highlight_material));
	BOOST_REQUIRE(renderer.Destroy_Material(normal_material));
	BOOST_REQUIRE(renderer.Destroy_Texture(highlight_texture));
	BOOST_REQUIRE(renderer.Destroy_Texture(normal_texture));
	renderer.Shutdown();
}
}

BOOST_AUTO_TEST_CASE(world_quad_normal_and_highlight_match_colocated_snapshot)
{
	Run_World_Quad_Snapshot("world_quads_normal_highlight");
}
