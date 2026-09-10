module;

#define BOOST_TEST_MODULE GraphicsLaserDrawingTests

#include <boost/test/included/unit_test.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Scene.Beams.LaserDrawing.Tests;

import Graphics.Scene.Beams.Laser;
import Graphics.Scene.Beams;
import Graphics.RHI;
import Graphics.Tests.Device;

using namespace Graphics;

namespace
{

constexpr std::uint32_t Width = 32;
constexpr std::uint32_t Height = 32;

std::uint32_t Usage(RHITextureUsage usage) noexcept
{
	return static_cast<std::uint32_t>(usage);
}

RHITextureHandle Create_Image(Device &device, std::uint32_t width,
	std::uint32_t height, std::span<const std::byte> pixels,
	std::uint32_t row_pitch, RHITextureUsage usage = RHITextureUsage::ShaderResource)
{
	return device.Create_Texture_Initialized(
		{width, height, 1, RHITextureFormat::RGBA8_UNorm, Usage(usage)},
		{pixels, row_pitch});
}

RHITextureHandle Create_Solid_Image(Device &device, std::array<std::byte, 4> pixel,
	RHITextureUsage usage = RHITextureUsage::ShaderResource)
{
	return Create_Image(device, 1, 1, std::span<const std::byte>(pixel), 4, usage);
}

RHITextureHandle Create_Render_Target(Device &device, std::uint32_t width,
	std::uint32_t height)
{
	return device.Create_Texture({width, height, 1, RHITextureFormat::RGBA8_UNorm,
		Usage(RHITextureUsage::RenderTarget)});
}

RHITextureHandle Create_Depth_Target(Device &device, std::uint32_t width,
	std::uint32_t height)
{
	return device.Create_Texture({width, height, 1, RHITextureFormat::D32_Float,
		Usage(RHITextureUsage::DepthStencil)});
}

LaserView Make_View(std::uint32_t time_milliseconds)
{
	LaserView view;
	view.camera_right = {1.0f, 0.0f, 0.0f};
	view.camera_forward = {0.0f, 0.0f, -1.0f};
	view.time_milliseconds = time_milliseconds;
	return view;
}

LaserDescription Make_Laser(RHITextureHandle texture, Color4 color = {1, 1, 1, 1},
	float distortion = 0.0f, float z = 0.5f)
{
	LaserDescription description;
	description.start = {-0.75f, 0.0f, z};
	description.end = {0.75f, 0.0f, z};
	description.width = 0.5f;
	description.color = color;
	description.uv_scale = 1.0f;
	description.uv_offset = 0.0f;
	description.scroll_rate = 0.0f;
	description.distortion = distortion;
	description.texture = texture;
	return description;
}

template <typename Description>
bool Set_Texture_Inputs(Description &description, RHITextureHandle detail,
	RHITextureHandle normal) noexcept
{
	if constexpr (requires(Description &value) {
		value.detail_texture = detail;
		value.normal_texture = normal;
	}) {
		description.detail_texture = detail;
		description.normal_texture = normal;
		return true;
	}
	return false;
}

std::vector<std::byte> Readback(Device &device, RHITextureHandle texture,
	std::uint32_t width, std::uint32_t height)
{
	std::vector<std::byte> pixels(static_cast<std::size_t>(width) * height * 4);
	if (!device.Readback_Texture(texture, pixels, width * 4))
		return {};
	return pixels;
}

std::array<unsigned, 4> Pixel(const std::vector<std::byte> &pixels,
	std::uint32_t width, std::uint32_t x, std::uint32_t y)
{
	const std::size_t offset = (static_cast<std::size_t>(y) * width + x) * 4;
	return {
		std::to_integer<unsigned>(pixels[offset]),
		std::to_integer<unsigned>(pixels[offset + 1]),
		std::to_integer<unsigned>(pixels[offset + 2]),
		std::to_integer<unsigned>(pixels[offset + 3])};
}

void Check_Pixel(const std::vector<std::byte> &pixels, std::uint32_t width,
	std::uint32_t x, std::uint32_t y, std::array<unsigned, 4> expected,
	unsigned tolerance = 0)
{
	const auto actual = Pixel(pixels, width, x, y);
	for (std::size_t channel = 0; channel < actual.size(); ++channel)
		BOOST_CHECK_LE(static_cast<unsigned>(std::abs(
			static_cast<int>(actual[channel]) - static_cast<int>(expected[channel]))), tolerance);
}

bool Draw(Device &device, LaserRenderer &renderer, RHITextureHandle target,
	RHITextureHandle depth, std::uint32_t width, std::uint32_t height,
	RHITextureHandle shroud, RHITextureHandle background, bool enable_distortion,
	std::vector<std::byte> &pixels)
{
	auto &commands = device.Immediate_Command_List();
	if (!commands.Set_Render_Targets(target, depth)
		|| !commands.Set_Viewport({0, 0, width, height, 0.0f, 1.0f}))
		return false;
	if (background.Is_Valid()) {
		if (!commands.Copy_Texture(background, target) || !commands.Clear_Depth(1.0f))
			return false;
	} else if (!commands.Clear({0.0f, 0.0f, 0.0f, 0.0f}, 1.0f)) {
		return false;
	}
	if (!renderer.Render(commands, target, depth, {0, 0, width, height}, shroud,
		enable_distortion, RHITextureFormat::RGBA8_UNorm))
		return false;
	pixels = Readback(device, target, width, height);
	return !pixels.empty();
}

void Check_Exterior_Is_Background(const std::vector<std::byte> &pixels,
	const std::vector<std::byte> &background, std::uint32_t width,
	std::uint32_t x, std::uint32_t y)
{
	const auto actual = Pixel(pixels, width, x, y);
	const auto expected = Pixel(background, width, x, y);
	for (std::size_t channel = 0; channel < actual.size(); ++channel)
		BOOST_CHECK_EQUAL(actual[channel], expected[channel]);
}

std::vector<std::byte> Make_Background(std::uint32_t width, std::uint32_t height)
{
	std::vector<std::byte> pixels(static_cast<std::size_t>(width) * height * 4);
	for (std::uint32_t y = 0; y < height; ++y) {
		for (std::uint32_t x = 0; x < width; ++x) {
			const std::size_t offset = (static_cast<std::size_t>(y) * width + x) * 4;
			pixels[offset] = std::byte(20u + x * 5u);
			pixels[offset + 1] = std::byte(30u + y * 5u);
			pixels[offset + 2] = std::byte(70u);
			pixels[offset + 3] = std::byte(91u);
		}
	}
	return pixels;
}

std::vector<std::byte> Make_Normal_Field()
{
	constexpr std::uint32_t size = 16;
	std::vector<std::byte> pixels(static_cast<std::size_t>(size) * size * 4);
	for (std::size_t offset = 0; offset < pixels.size(); offset += 4) {
		pixels[offset] = std::byte(128);
		pixels[offset + 1] = std::byte(128);
		pixels[offset + 2] = std::byte(255);
		pixels[offset + 3] = std::byte(255);
	}
	return pixels;
}

void Paint_Normal_Block(std::vector<std::byte> &pixels, std::uint32_t x0,
	std::uint32_t x1, std::uint32_t y0, std::uint32_t y1,
	std::array<std::byte, 4> value)
{
	constexpr std::uint32_t size = 16;
	for (std::uint32_t y = y0; y <= y1; ++y) {
		for (std::uint32_t x = x0; x <= x1; ++x) {
			const std::size_t offset = (static_cast<std::size_t>(y) * size + x) * 4;
			for (std::size_t channel = 0; channel < value.size(); ++channel)
				pixels[offset + channel] = value[channel];
		}
	}
}

unsigned Difference(const std::vector<std::byte> &first,
	const std::vector<std::byte> &second, std::uint32_t width,
	std::uint32_t height, std::size_t channel)
{
	unsigned difference = 0;
	for (std::uint32_t y = 4; y + 4 < height; ++y) {
		for (std::uint32_t x = 4; x + 4 < width; ++x) {
			const std::size_t offset = (static_cast<std::size_t>(y) * width + x) * 4 + channel;
			difference += static_cast<unsigned>(std::abs(
				std::to_integer<int>(first[offset]) - std::to_integer<int>(second[offset])));
		}
	}
	return difference;
}

void Write_Optional_PPM(const std::vector<std::byte> &pixels,
	std::uint32_t width, std::uint32_t height)
{
	const char *path = std::getenv("GRAPHICS_LASER_TEST_OUTPUT");
	if (path == nullptr || *path == '\0')
		return;
	std::ofstream output(path, std::ios::binary);
	if (!output)
		return;
	output << "P6\n" << width << ' ' << height << "\n255\n";
	for (std::uint32_t y = 0; y < height; ++y) {
		for (std::uint32_t x = 0; x < width; ++x) {
			const auto pixel = Pixel(pixels, width, x, y);
			const std::array<char, 3> rgb{
				static_cast<char>(pixel[0]), static_cast<char>(pixel[1]),
				static_cast<char>(pixel[2])};
			output.write(rgb.data(), static_cast<std::streamsize>(rgb.size()));
		}
	}
}

}

BOOST_AUTO_TEST_CASE(laser_core_multiplies_authored_rgba_and_biases_shroud_visibility)
{
	for (const bool warp : {true, false}) {
		GraphicsTestDevice device({warp});
		if (!warp && !device.Is_Valid())
			continue;
		BOOST_REQUIRE(device.Is_Valid());

		LaserRenderer renderer;
		BOOST_REQUIRE(renderer.Initialize(device,
			Graphics::Test_Shader_Directory(GRAPHICS_LASER_SHADER_DIRECTORY), 2));
		BOOST_REQUIRE(renderer.Set_View(Make_View(0)));

		const std::array<std::byte, 4> base_pixel{
			std::byte(255), std::byte(127), std::byte(63), std::byte(128)};
		const auto base = Create_Image(device, 1, 1, std::span<const std::byte>(base_pixel), 4);
		const auto target = Create_Render_Target(device, Width, Height);
		const auto depth = Create_Depth_Target(device, Width, Height);
		const auto shroud_zero = Create_Solid_Image(device,
			{std::byte(191), std::byte(191), std::byte(191), std::byte(255)});
		const auto shroud_half = Create_Solid_Image(device,
			{std::byte(223), std::byte(223), std::byte(223), std::byte(255)});
		const auto shroud_full = Create_Solid_Image(device,
			{std::byte(255), std::byte(255), std::byte(255), std::byte(255)});
		const auto detail_full = Create_Solid_Image(device,
			{std::byte(128), std::byte(128), std::byte(128), std::byte(255)});
		const auto detail_half_alpha = Create_Solid_Image(device,
			{std::byte(128), std::byte(128), std::byte(128), std::byte(128)});
		const auto normal_flat = Create_Solid_Image(device,
			{std::byte(128), std::byte(128), std::byte(255), std::byte(255)});
		BOOST_REQUIRE(base.Is_Valid() && target.Is_Valid() && depth.Is_Valid());
		BOOST_REQUIRE(shroud_zero.Is_Valid() && shroud_half.Is_Valid()
			&& shroud_full.Is_Valid() && detail_full.Is_Valid()
			&& detail_half_alpha.Is_Valid() && normal_flat.Is_Valid());

		LaserDescription description = Make_Laser(base, {0.5f, 0.75f, 0.25f, 0.5f});
		description.uv_scale = 1.0f;
		description.uv_offset = 0.0f;
		BOOST_REQUIRE_MESSAGE(Set_Texture_Inputs(description, detail_full, normal_flat),
			"LaserDescription must expose independent detail_texture and normal_texture inputs");
		const LaserHandle laser = renderer.Create(description, 0);
		BOOST_REQUIRE(laser.Is_Valid());

		std::vector<std::byte> full;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, {}, false, full));
		// The authored base/detail/color values have a fixed independent
		// reference at the center: detail RGB is 128/255, detail alpha is one,
		// and SourceAlpha/One blends the result.
		// The RGB-only laser mask preserves the cleared destination alpha.
		Check_Pixel(full, Width, Width / 2, Height / 2, {16, 12, 2, 0}, 2);
		BOOST_CHECK_GT(Pixel(full, Width, 10, Height / 2)[0], 4u);
		BOOST_CHECK_GT(Pixel(full, Width, 24, Height / 2)[0], 4u);
		Check_Pixel(full, Width, 1, 1, {0, 0, 0, 0}, 0);

		std::vector<std::byte> zero;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_zero, {}, false, zero));
		Check_Pixel(zero, Width, Width / 2, Height / 2, {0, 0, 0, 0}, 1);

		std::vector<std::byte> half;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_half, {}, false, half));
		const auto full_pixel = Pixel(full, Width, Width / 2, Height / 2);
		const auto half_pixel = Pixel(half, Width, Width / 2, Height / 2);
		BOOST_CHECK_GT(half_pixel[0], 0u);
		BOOST_CHECK_LT(half_pixel[0], full_pixel[0]);
		BOOST_CHECK_GT(half_pixel[1], 0u);
		BOOST_CHECK_LT(half_pixel[1], full_pixel[1]);
		BOOST_CHECK_EQUAL(half_pixel[3], 0u);

		BOOST_REQUIRE(Set_Texture_Inputs(description, detail_half_alpha, normal_flat));
		BOOST_REQUIRE(renderer.Update(laser, description));
		std::vector<std::byte> half_detail_alpha;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, {}, false, half_detail_alpha));
		Check_Pixel(half_detail_alpha, Width, Width / 2, Height / 2,
			{8, 6, 1, 0}, 2);
		BOOST_CHECK_LT(Pixel(half_detail_alpha, Width, Width / 2, Height / 2)[0],
			full_pixel[0]);
		BOOST_REQUIRE(Set_Texture_Inputs(description, detail_full, normal_flat));

		std::vector<std::byte> fallback;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			{}, {}, false, fallback));
		Check_Pixel(fallback, Width, Width / 2, Height / 2, full_pixel, 1);

		// The laser pass tests depth but does not write it.  A farther beam in
		// the same draw therefore contributes after the nearer beam.
		description.start.z = 0.25f;
		description.end.z = 0.25f;
		description.color = {0.5f, 0.75f, 0.25f, 0.5f};
		description.texture = base;
		description.distortion = 0.0f;
		BOOST_REQUIRE(renderer.Update(laser, description));
		std::vector<std::byte> near_only;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, {}, false, near_only));
		LaserDescription far_description = description;
		far_description.start.z = 0.75f;
		far_description.end.z = 0.75f;
		const LaserHandle far_laser = renderer.Create(far_description, 0);
		BOOST_REQUIRE(far_laser.Is_Valid());
		std::vector<std::byte> near_and_far;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, {}, false, near_and_far));
		BOOST_CHECK_GT(Pixel(near_and_far, Width, Width / 2, Height / 2)[0],
			Pixel(near_only, Width, Width / 2, Height / 2)[0] + 2u);
		BOOST_REQUIRE(renderer.Destroy(far_laser));

		const std::array<std::byte, 4> transparent_pixel{
			std::byte(255), std::byte(255), std::byte(255), std::byte(0)};
		const auto transparent = Create_Image(device, 1, 1,
			std::span<const std::byte>(transparent_pixel), 4);
		description.texture = transparent;
		BOOST_REQUIRE(renderer.Update(laser, description));
		std::vector<std::byte> transparent_result;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, {}, false, transparent_result));
		Check_Pixel(transparent_result, Width, Width / 2, Height / 2,
			{0, 0, 0, 0}, 1);

		description.texture = base;
		description.color.a = 0.0f;
		BOOST_REQUIRE(renderer.Update(laser, description));
		std::vector<std::byte> zero_opacity;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, {}, false, zero_opacity));
		Check_Pixel(zero_opacity, Width, Width / 2, Height / 2,
			{0, 0, 0, 0}, 1);

		BOOST_REQUIRE(renderer.Destroy(laser));
		renderer.Shutdown();
		device.Destroy_Texture(base);
		device.Destroy_Texture(transparent);
		device.Destroy_Texture(target);
		device.Destroy_Texture(depth);
		device.Destroy_Texture(shroud_zero);
		device.Destroy_Texture(shroud_half);
		device.Destroy_Texture(shroud_full);
		device.Destroy_Texture(detail_full);
		device.Destroy_Texture(detail_half_alpha);
		device.Destroy_Texture(normal_flat);
	}
}

BOOST_AUTO_TEST_CASE(laser_distortion_is_masked_keeps_destination_alpha_and_survives_resize)
{
	for (const bool warp : {true, false}) {
		GraphicsTestDevice device({warp});
		if (!warp && !device.Is_Valid())
			continue;
		BOOST_REQUIRE(device.Is_Valid());

		LaserRenderer renderer;
		BOOST_REQUIRE(renderer.Initialize(device,
			Graphics::Test_Shader_Directory(GRAPHICS_LASER_SHADER_DIRECTORY), 4));
		BOOST_REQUIRE(renderer.Set_View(Make_View(1000)));

		const auto background_pixels = Make_Background(Width, Height);
		const auto background = Create_Image(device, Width, Height,
			std::span<const std::byte>(background_pixels),
			Width * 4);
		const auto base = Create_Solid_Image(device,
			{std::byte(255), std::byte(255), std::byte(255), std::byte(255)});
		const auto detail_white = Create_Solid_Image(device,
			{std::byte(255), std::byte(255), std::byte(255), std::byte(255)});
		const auto normal_flat = Create_Solid_Image(device,
			{std::byte(128), std::byte(128), std::byte(255), std::byte(255)});
		const auto normal_weak = Create_Solid_Image(device,
			{std::byte(140), std::byte(128), std::byte(244), std::byte(255)});
		const auto normal_steep = Create_Solid_Image(device,
			{std::byte(230), std::byte(128), std::byte(180), std::byte(255)});
		auto combined_normal_pixels = Make_Normal_Field();
		// At the center UV, the two authored sample transforms land near the
		// right edge/top half and right-center/lower quarter respectively.
		// Their unequal magnitudes make normalize(a+b) distinguishable from
		// normalize(a)+normalize(b).
		Paint_Normal_Block(combined_normal_pixels, 14, 15, 7, 9,
			{std::byte(140), std::byte(128), std::byte(140), std::byte(255)});
		Paint_Normal_Block(combined_normal_pixels, 13, 15, 3, 5,
			{std::byte(25), std::byte(128), std::byte(204), std::byte(255)});
		Paint_Normal_Block(combined_normal_pixels, 0, 1, 7, 9,
			{std::byte(140), std::byte(128), std::byte(140), std::byte(255)});
		const auto combined_normal = Create_Image(device, 16, 16,
			std::span<const std::byte>(combined_normal_pixels), 16 * 4);
		const auto shroud_full = Create_Solid_Image(device,
			{std::byte(255), std::byte(255), std::byte(255), std::byte(255)});
		const auto shroud_half = Create_Solid_Image(device,
			{std::byte(223), std::byte(223), std::byte(223), std::byte(255)});
		const auto shroud_zero = Create_Solid_Image(device,
			{std::byte(191), std::byte(191), std::byte(191), std::byte(255)});
		const auto target = Create_Render_Target(device, Width, Height);
		const auto depth = Create_Depth_Target(device, Width, Height);
		BOOST_REQUIRE(background.Is_Valid() && base.Is_Valid() && detail_white.Is_Valid()
			&& normal_flat.Is_Valid() && normal_weak.Is_Valid() && normal_steep.Is_Valid()
			&& combined_normal.Is_Valid()
			&& target.Is_Valid()
			&& depth.Is_Valid());
		BOOST_REQUIRE(shroud_full.Is_Valid() && shroud_half.Is_Valid()
			&& shroud_zero.Is_Valid());

		// Black RGB leaves the independent refraction mask active while making
		// the additive core visually silent. Alpha remains one on purpose.
		LaserDescription description = Make_Laser(base, {0.0f, 0.0f, 0.0f, 1.0f}, 1.0f);
		BOOST_REQUIRE_MESSAGE(Set_Texture_Inputs(description, detail_white, normal_steep),
			"LaserDescription must expose independent detail_texture and normal_texture inputs");
		const LaserHandle laser = renderer.Create(description, 1000);
		BOOST_REQUIRE(laser.Is_Valid());

		std::vector<std::byte> no_distortion;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, background, false, no_distortion));
		std::vector<std::byte> distorted;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, background, true, distorted));
		Check_Exterior_Is_Background(distorted, background_pixels, Width, 1, 1);
		Check_Exterior_Is_Background(distorted, background_pixels, Width, 30, 30);
		const auto center_background = Pixel(background_pixels, Width, Width / 2, Height / 2);
		const auto center_distorted = Pixel(distorted, Width, Width / 2, Height / 2);
		const unsigned center_difference =
			static_cast<unsigned>(std::abs(static_cast<int>(center_distorted[0]) -
				static_cast<int>(center_background[0]))) +
			static_cast<unsigned>(std::abs(static_cast<int>(center_distorted[1]) -
				static_cast<int>(center_background[1])));
		BOOST_CHECK_GE(center_difference, 3u);
		BOOST_CHECK_LE(center_difference, 40u);
		BOOST_CHECK_EQUAL(center_distorted[3],
			Pixel(no_distortion, Width, Width / 2, Height / 2)[3]);
		BOOST_CHECK_EQUAL(center_distorted[3], center_background[3]);
		Write_Optional_PPM(distorted, Width, Height);

		// A flat decoded normal produces no refraction.  A weak projected slope
		// shifts less than a steep slope; normalizing the projected XY vector
		// would incorrectly make these two cases equal.
		BOOST_REQUIRE(Set_Texture_Inputs(description, detail_white, normal_flat));
		BOOST_REQUIRE(renderer.Update(laser, description));
		std::vector<std::byte> flat_normal;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, background, true, flat_normal));
		BOOST_REQUIRE(Set_Texture_Inputs(description, detail_white, normal_weak));
		BOOST_REQUIRE(renderer.Update(laser, description));
		std::vector<std::byte> weak_normal;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, background, true, weak_normal));
		BOOST_REQUIRE(Set_Texture_Inputs(description, detail_white, normal_steep));
		BOOST_REQUIRE(renderer.Update(laser, description));
		std::vector<std::byte> steep_normal;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, background, true, steep_normal));
		const unsigned flat_difference = Difference(flat_normal, background_pixels, Width, Height, 0)
			+ Difference(flat_normal, background_pixels, Width, Height, 1);
		const unsigned weak_difference = Difference(weak_normal, background_pixels, Width, Height, 0)
			+ Difference(weak_normal, background_pixels, Width, Height, 1);
		const unsigned steep_difference = Difference(steep_normal, background_pixels, Width, Height, 0)
			+ Difference(steep_normal, background_pixels, Width, Height, 1);
		BOOST_CHECK_LE(flat_difference, 4u);
		BOOST_CHECK_GT(weak_difference, flat_difference + 4u);
		BOOST_CHECK_GT(steep_difference, weak_difference + 4u);

		BOOST_REQUIRE(Set_Texture_Inputs(description, detail_white, combined_normal));
		BOOST_REQUIRE(renderer.Update(laser, description));
		std::vector<std::byte> combined_normals;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, background, true, combined_normals));
		const auto combined_pixel = Pixel(combined_normals, Width, Width / 2, Height / 2);
		const auto background_pixel = Pixel(background_pixels, Width, Width / 2, Height / 2);
		const unsigned combined_center_difference =
			static_cast<unsigned>(std::abs(static_cast<int>(combined_pixel[0]) -
				static_cast<int>(background_pixel[0]))) +
			static_cast<unsigned>(std::abs(static_cast<int>(combined_pixel[1]) -
				static_cast<int>(background_pixel[1])));
		const auto steep_pixel = Pixel(steep_normal, Width, Width / 2, Height / 2);
		const unsigned steep_center_difference =
			static_cast<unsigned>(std::abs(static_cast<int>(steep_pixel[0]) -
				static_cast<int>(background_pixel[0]))) +
			static_cast<unsigned>(std::abs(static_cast<int>(steep_pixel[1]) -
				static_cast<int>(background_pixel[1])));
		BOOST_CHECK_GT(combined_center_difference, 2u);
		BOOST_CHECK_LT(combined_center_difference, steep_center_difference);

		const auto half_target = Create_Render_Target(device, Width, Height);
		const auto half_depth = Create_Depth_Target(device, Width, Height);
		std::vector<std::byte> half;
		BOOST_REQUIRE(Draw(device, renderer, half_target, half_depth, Width, Height,
			shroud_half, background, true, half));
		const auto zero_target = Create_Render_Target(device, Width, Height);
		const auto zero_depth = Create_Depth_Target(device, Width, Height);
		std::vector<std::byte> zero;
		BOOST_REQUIRE(Draw(device, renderer, zero_target, zero_depth, Width, Height,
			shroud_zero, background, true, zero));
		BOOST_CHECK_EQUAL(Difference(zero, background_pixels, Width, Height, 0), 0u);
		BOOST_CHECK_EQUAL(Difference(zero, background_pixels, Width, Height, 1), 0u);
		const unsigned half_difference = Difference(half, background_pixels, Width, Height, 0)
			+ Difference(half, background_pixels, Width, Height, 1);
		const unsigned full_difference = Difference(distorted, background_pixels, Width, Height, 0)
			+ Difference(distorted, background_pixels, Width, Height, 1);
		BOOST_CHECK_GT(half_difference, 0u);
		BOOST_CHECK_LT(half_difference, full_difference);

		const std::array<std::byte, 4> transparent_pixel{
			std::byte(255), std::byte(255), std::byte(255), std::byte(0)};
		const auto transparent = Create_Image(device, 1, 1,
			std::span<const std::byte>(transparent_pixel), 4);
		description.texture = transparent;
		BOOST_REQUIRE(renderer.Update(laser, description));
		const auto transparent_target = Create_Render_Target(device, Width, Height);
		const auto transparent_depth = Create_Depth_Target(device, Width, Height);
		std::vector<std::byte> transparent_result;
		BOOST_REQUIRE(Draw(device, renderer, transparent_target, transparent_depth,
			Width, Height, shroud_full, background, true, transparent_result));
		BOOST_CHECK(transparent_result == background_pixels);

		description.enabled = false;
		BOOST_REQUIRE(renderer.Update(laser, description));
		std::vector<std::byte> disabled;
		BOOST_REQUIRE(Draw(device, renderer, transparent_target, transparent_depth,
			Width, Height, shroud_full, background, true, disabled));
		BOOST_CHECK(disabled == background_pixels);

		description.enabled = true;
		description.texture = base;
		BOOST_REQUIRE(renderer.Update(laser, description));
		BOOST_REQUIRE(renderer.Set_Fully_Obscured(laser, true));
		std::vector<std::byte> obscured;
		BOOST_REQUIRE(Draw(device, renderer, transparent_target, transparent_depth,
			Width, Height, shroud_full, background, true, obscured));
		BOOST_CHECK(obscured == background_pixels);
		BOOST_REQUIRE(renderer.Set_Fully_Obscured(laser, false));

		description.color.a = 0.0f;
		description.distortion = 0.0f;
		BOOST_REQUIRE(renderer.Update(laser, description));
		std::vector<std::byte> zero_strength;
		BOOST_REQUIRE(Draw(device, renderer, transparent_target, transparent_depth,
			Width, Height, shroud_full, background, true, zero_strength));
		BOOST_CHECK(zero_strength == background_pixels);

		// An overlapping green core remains after the refraction pass.  The
		// distinction is visible in the additive channel even though the first
		// beam has black RGB and supplies only the distortion footprint.
		description.color = {0.0f, 0.0f, 0.0f, 1.0f};
		description.distortion = 1.0f;
		BOOST_REQUIRE(renderer.Update(laser, description));
		LaserDescription additive = Make_Laser(base, {0.0f, 1.0f, 0.0f, 1.0f}, 0.0f);
		const LaserHandle additive_laser = renderer.Create(additive, 1000);
		BOOST_REQUIRE(additive_laser.Is_Valid());
		std::vector<std::byte> overlapping;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, background, true, overlapping));
		BOOST_CHECK_GT(Pixel(overlapping, Width, Width / 2, Height / 2)[1], 60u);
		std::vector<std::byte> overlapping_without_distortion;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud_full, background, false, overlapping_without_distortion));
		BOOST_CHECK_EQUAL(Pixel(overlapping, Width, Width / 2, Height / 2)[3],
			Pixel(overlapping_without_distortion, Width, Width / 2, Height / 2)[3]);

		// The retained source survives its caller's release.  A new target size
		// forces a new distortion snapshot and still samples the old base image.
		BOOST_REQUIRE(device.Destroy_Texture(base));
		const std::uint32_t resized_width = 24;
		const std::uint32_t resized_height = 16;
		const auto resized_background_pixels = Make_Background(resized_width, resized_height);
		const auto resized_background = Create_Image(device, resized_width, resized_height,
			std::span<const std::byte>(resized_background_pixels), resized_width * 4);
		const auto resized_target = Create_Render_Target(device, resized_width, resized_height);
		const auto resized_depth = Create_Depth_Target(device, resized_width, resized_height);
		BOOST_REQUIRE(resized_background.Is_Valid() && resized_target.Is_Valid()
			&& resized_depth.Is_Valid());
		std::vector<std::byte> resized;
		BOOST_REQUIRE(Draw(device, renderer, resized_target, resized_depth,
			resized_width, resized_height, shroud_full, resized_background, true, resized));
		BOOST_CHECK_GT(Pixel(resized, resized_width, resized_width / 2,
			resized_height / 2)[1], 60u);

		BOOST_REQUIRE(renderer.Destroy(additive_laser));
		BOOST_REQUIRE(renderer.Destroy(laser));
		renderer.Shutdown();
		device.Destroy_Texture(transparent);
		device.Destroy_Texture(background);
		device.Destroy_Texture(detail_white);
		device.Destroy_Texture(normal_flat);
		device.Destroy_Texture(normal_weak);
		device.Destroy_Texture(normal_steep);
		device.Destroy_Texture(combined_normal);
		device.Destroy_Texture(shroud_full);
		device.Destroy_Texture(shroud_half);
		device.Destroy_Texture(shroud_zero);
		device.Destroy_Texture(target);
		device.Destroy_Texture(depth);
		device.Destroy_Texture(half_target);
		device.Destroy_Texture(half_depth);
		device.Destroy_Texture(zero_target);
		device.Destroy_Texture(zero_depth);
		device.Destroy_Texture(transparent_target);
		device.Destroy_Texture(transparent_depth);
		device.Destroy_Texture(resized_background);
		device.Destroy_Texture(resized_target);
		device.Destroy_Texture(resized_depth);
	}
}

BOOST_AUTO_TEST_CASE(laser_scroll_starts_at_creation_time_and_repeats_are_stable)
{
	for (const bool warp : {true, false}) {
		GraphicsTestDevice device({warp});
		if (!warp && !device.Is_Valid())
			continue;
		BOOST_REQUIRE(device.Is_Valid());

		LaserRenderer renderer;
		BOOST_REQUIRE(renderer.Initialize(device,
			Graphics::Test_Shader_Directory(GRAPHICS_LASER_SHADER_DIRECTORY), 1));
		const std::array<std::byte, 16> scroll_pixels{
			std::byte(255), std::byte(0), std::byte(0), std::byte(255),
			std::byte(0), std::byte(255), std::byte(0), std::byte(255),
			std::byte(0), std::byte(0), std::byte(255), std::byte(255),
			std::byte(255), std::byte(255), std::byte(0), std::byte(255)};
		const auto texture = Create_Image(device, 1, 4,
			std::span<const std::byte>(scroll_pixels), 4);
		const auto shroud = Create_Solid_Image(device,
			{std::byte(255), std::byte(255), std::byte(255), std::byte(255)});
		const auto target = Create_Render_Target(device, Width, Height);
		const auto depth = Create_Depth_Target(device, Width, Height);
		BOOST_REQUIRE(texture.Is_Valid() && shroud.Is_Valid() && target.Is_Valid()
			&& depth.Is_Valid());

		LaserDescription description = Make_Laser(texture, {1, 1, 1, 1}, 0.0f);
		description.scroll_rate = -0.5f;
		const std::uint32_t creation_time = (std::numeric_limits<std::uint32_t>::max)() - 100u;
		const LaserHandle delayed = renderer.Create(description, creation_time);
		BOOST_REQUIRE(delayed.Is_Valid());
		BOOST_REQUIRE(renderer.Set_View(Make_View(50u)));
		std::vector<std::byte> delayed_pixels;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud, {}, false, delayed_pixels));

		// A second draw at the same unsigned clock value is a second view of the
		// same frame, so it must not advance the scroll phase.
		std::vector<std::byte> delayed_repeat;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud, {}, false, delayed_repeat));
		BOOST_CHECK(delayed_repeat == delayed_pixels);

		BOOST_REQUIRE(renderer.Destroy(delayed));
		const LaserHandle immediate = renderer.Create(description, creation_time);
		BOOST_REQUIRE(immediate.Is_Valid());
		BOOST_REQUIRE(renderer.Set_View(Make_View(creation_time)));
		std::vector<std::byte> at_creation;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud, {}, false, at_creation));
		BOOST_REQUIRE(renderer.Set_View(Make_View(50u)));
		std::vector<std::byte> immediate_pixels;
		BOOST_REQUIRE(Draw(device, renderer, target, depth, Width, Height,
			shroud, {}, false, immediate_pixels));
		BOOST_CHECK(immediate_pixels == delayed_pixels);

		BOOST_REQUIRE(renderer.Destroy(immediate));
		renderer.Shutdown();
		device.Destroy_Texture(texture);
		device.Destroy_Texture(shroud);
		device.Destroy_Texture(target);
		device.Destroy_Texture(depth);
	}
}
