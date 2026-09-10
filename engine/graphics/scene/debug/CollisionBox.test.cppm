module;

#define BOOST_TEST_MODULE CollisionBoxDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>

export module Graphics.Scene.Debug.CollisionBox.Tests;

import Graphics.RHI;
import Graphics.Tests.Device;
import Graphics.Scene.Debug.CollisionBox;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Shadows.DirectionalRenderer;

namespace
{

using namespace Graphics;

void Prepare_Target(GraphicsTestDevice &device, RHITextureHandle target, RHITextureHandle depth)
{
	auto &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, 16, 16}));
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
}

std::array<std::byte, 16 * 16 * 4> Read_Target(GraphicsTestDevice &device, RHITextureHandle target)
{
	std::array<std::byte, 16 * 16 * 4> pixels{};
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, 16 * 4));
	return pixels;
}

std::size_t Pixel_Offset(unsigned x, unsigned y)
{
	return (static_cast<std::size_t>(y) * 16 + x) * 4;
}

void Check_Pixel(const std::array<std::byte, 16 * 16 * 4> &pixels,
	std::size_t offset, const std::array<unsigned, 4> &expected)
{
	for (unsigned channel = 0; channel < expected.size(); ++channel) {
		const int actual = static_cast<int>(std::to_integer<unsigned>(pixels[offset + channel]));
		const int difference = actual - static_cast<int>(expected[channel]);
		BOOST_CHECK(std::abs(difference) <= 1);
	}
}

}

BOOST_AUTO_TEST_CASE(box_geometry_retains_corner_topology_and_independent_packed_color)
{
	CollisionBoxDrawData data;
	data.center = {2, -3, 4};
	data.extent = {1, 2, 3};
	data.color = {.125f, .5f, .875f, .25f};
	CollisionBoxGeometry geometry;
	BOOST_REQUIRE(Build_Collision_Box_Geometry(data, geometry));
	BOOST_CHECK_EQUAL(geometry.vertices[0].position[0], 3);
	BOOST_CHECK_EQUAL(geometry.vertices[0].position[1], -1);
	BOOST_CHECK_EQUAL(geometry.vertices[0].position[2], 7);
	BOOST_CHECK_EQUAL(geometry.vertices[6].position[0], 1);
	BOOST_CHECK_EQUAL(geometry.vertices[6].position[1], -5);
	BOOST_CHECK_EQUAL(geometry.vertices[6].position[2], 1);
	// Color_To_ARGB rounds these channels to raw bytes 32, 128, 223, 64.
	BOOST_CHECK_CLOSE(geometry.vertices[0].color[0], 32.0f / 255.0f, .001f);
	BOOST_CHECK_CLOSE(geometry.vertices[0].color[1], 128.0f / 255.0f, .001f);
	BOOST_CHECK_CLOSE(geometry.vertices[0].color[2], 223.0f / 255.0f, .001f);
	BOOST_CHECK_CLOSE(geometry.vertices[0].color[3], 64.0f / 255.0f, .001f);
	const std::array<std::uint32_t, CollisionBoxIndexCount> expected{{
		0, 1, 2, 0, 2, 3, 4, 7, 6, 4, 6, 5,
		0, 3, 7, 0, 7, 4, 1, 5, 6, 1, 6, 2,
		4, 5, 1, 4, 1, 0, 3, 2, 6, 3, 6, 7}};
	BOOST_CHECK(geometry.indices == expected);
}

BOOST_AUTO_TEST_CASE(box_geometry_rejects_non_finite_input_without_clobbering_previous_output)
{
	CollisionBoxDrawData data;
	CollisionBoxGeometry geometry;
	geometry.vertices[0].position = {9, 8, 7};
	data.extent[1] = std::numeric_limits<float>::quiet_NaN();
	BOOST_CHECK(!Build_Collision_Box_Geometry(data, geometry));
	BOOST_CHECK_EQUAL(geometry.vertices[0].position[0], 9);
	BOOST_CHECK_EQUAL(geometry.vertices[0].position[1], 8);
	BOOST_CHECK_EQUAL(geometry.vertices[0].position[2], 7);
}

BOOST_AUTO_TEST_CASE(box_submission_respects_display_mask_alpha_and_device_resource_recreation)
{
	GraphicsTestDevice device({true});
	PropRenderer renderer;
	DirectionalShadowRenderer shadows;
	PropSubmission submission;
	const auto shader_directory = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	BOOST_REQUIRE(shadows.Initialize(device, shader_directory));
	submission.Initialize(device, renderer, shadows);
	const auto target = device.Create_Texture({16, 16, 1, RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({16, 16, 1, RHITextureFormat::D32_Float,
		static_cast<unsigned>(RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid());
	BOOST_REQUIRE(depth.Is_Valid());
	Prepare_Target(device, target, depth);

	Set_Collision_Box_Display_Mask(4);
	CollisionBoxRenderer box;
	CollisionBoxDrawData data;
	data.center = {0, 0, .5f};
	data.extent = {.75f, .75f, .1f};
	data.color = {1, 0, 0, .5f};
	data.collision_type = 4;
	BOOST_REQUIRE(box.Submit(renderer, submission, data));
	const auto first = Read_Target(device, target);
	const auto center = Pixel_Offset(8, 8);
	Check_Pixel(first, center, {128, 0, 0, 64});
	const auto first_triangle = Pixel_Offset(5, 10);
	const auto second_triangle = Pixel_Offset(10, 5);
	for (const auto interior : {first_triangle, second_triangle}) {
		Check_Pixel(first, interior, {128, 0, 0, 64});
	}
	const auto outside = Pixel_Offset(0, 0);
	for (unsigned channel = 0; channel < 4; ++channel)
		BOOST_CHECK_EQUAL(std::to_integer<unsigned>(first[outside + channel]), 0u);

	Prepare_Target(device, target, depth);
	data.collision_type = 8;
	BOOST_CHECK(!box.Submit(renderer, submission, data));
	const auto masked = Read_Target(device, target);
	for (unsigned channel = 0; channel < 4; ++channel)
		BOOST_CHECK_EQUAL(std::to_integer<unsigned>(masked[center + channel]), 0u);
	for (const auto interior : {first_triangle, second_triangle})
		for (unsigned channel = 0; channel < 4; ++channel)
			BOOST_CHECK_EQUAL(std::to_integer<unsigned>(masked[interior + channel]), 0u);

	// The mesh description is retained by CollisionBoxRenderer, so the source
	// draw record may be discarded before the next submission.
	data.collision_type = 4;
	{
		CollisionBoxDrawData scoped = data;
		scoped.color = {0, 1, 0, .5f};
		BOOST_REQUIRE(box.Submit(renderer, submission, scoped));
	}
	Prepare_Target(device, target, depth);
	CollisionBoxDrawData retained = data;
	retained.color = {0, 1, 0, .5f};
	BOOST_REQUIRE(box.Submit(renderer, submission, retained));
	const auto retained_pixels = Read_Target(device, target);
	Check_Pixel(retained_pixels, center, {0, 128, 0, 64});
	for (const auto interior : {first_triangle, second_triangle}) {
		Check_Pixel(retained_pixels, interior, {0, 128, 0, 64});
	}

	// Shutdown releases GPU buffers but keeps PropRenderer's CPU geometry. The
	// same box handle must upload and draw again after renderer reinitialization.
	renderer.Shutdown();
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	Prepare_Target(device, target, depth);
	BOOST_REQUIRE(box.Submit(renderer, submission, retained));
	const auto recreated = Read_Target(device, target);
	Check_Pixel(recreated, center, {0, 128, 0, 64});
	for (const auto interior : {first_triangle, second_triangle})
		Check_Pixel(recreated, interior, {0, 128, 0, 64});

	box.Release(renderer);
	Set_Collision_Box_Display_Mask(0);
	submission.Shutdown();
	shadows.Shutdown();
	renderer.Shutdown();
	device.Destroy_Texture(target);
	device.Destroy_Texture(depth);
}
