module;

#define BOOST_TEST_MODULE GraphicsVideoRendererTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>

export module Graphics.Video.Renderer.Tests;

import Graphics.Tests.Device;
import Graphics.Testing.VisualRegression;
import Graphics.Video.Renderer;
import Video.Frame;

using namespace Graphics;

#ifndef GRAPHICS_VIDEO_REFERENCE_DIRECTORY
#define GRAPHICS_VIDEO_REFERENCE_DIRECTORY "."
#endif

#ifndef GRAPHICS_VIDEO_FAILURE_DIRECTORY
#define GRAPHICS_VIDEO_FAILURE_DIRECTORY "."
#endif

#ifndef GRAPHICS_VIDEO_SHADER_DIRECTORY
#define GRAPHICS_VIDEO_SHADER_DIRECTORY "."
#endif

namespace
{
constexpr std::uint32_t Video_Width = 128;
constexpr std::uint32_t Video_Height = 72;

struct VideoSnapshotContext final
{
	VideoRenderer *renderer = nullptr;
	Engine::Video::DecodedVideoFrame frame{};
};

std::array<std::byte, static_cast<std::size_t>(Video_Width) * Video_Height * 4> Make_Frame(std::uint8_t phase)
{
	std::array<std::byte, static_cast<std::size_t>(Video_Width) * Video_Height * 4> pixels{};
	for (std::uint32_t y = 0; y < Video_Height; ++y) {
		for (std::uint32_t x = 0; x < Video_Width; ++x) {
			const std::size_t offset = (static_cast<std::size_t>(y) * Video_Width + x) * 4;
			const std::uint8_t red = static_cast<std::uint8_t>((x * 255u) / (Video_Width - 1));
			const std::uint8_t green = static_cast<std::uint8_t>((y * 255u) / (Video_Height - 1));
			const std::uint8_t blue = static_cast<std::uint8_t>(((x / 16u + y / 9u + phase) & 1u) != 0 ? 220 : 40);
			pixels[offset + 0] = static_cast<std::byte>(red);
			pixels[offset + 1] = static_cast<std::byte>(green);
			pixels[offset + 2] = static_cast<std::byte>(blue);
			pixels[offset + 3] = static_cast<std::byte>(0xff);
		}
	}
	return pixels;
}

bool Render_Video_Frame(
	Device &,
	CommandList &commands,
	RHITextureHandle color_target,
	RHITextureHandle depth_target,
	RHIViewport viewport,
	void *context) noexcept
{
	const VideoSnapshotContext &snapshot = *static_cast<const VideoSnapshotContext *>(context);
	if (snapshot.renderer == nullptr
		|| !commands.Set_Render_Targets(color_target, depth_target)
		|| !commands.Clear({0.02f, 0.03f, 0.05f, 1.0f}, 1.0f))
		return false;

	snapshot.renderer->Begin_Frame();
	if (!snapshot.renderer->Submit(
		1,
		snapshot.frame,
		{0, 0, viewport.width, viewport.height}))
		return false;

	return snapshot.renderer->Render(commands, {
		{color_target, viewport.width, viewport.height},
		{depth_target, viewport.width, viewport.height}
	});
}
}

BOOST_AUTO_TEST_CASE(video_frame_presentation_matches_colocated_snapshot_with_real_rhi)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	VideoRenderer renderer;
	const std::filesystem::path shader_directory(Graphics::Test_Shader_Directory(GRAPHICS_VIDEO_SHADER_DIRECTORY));
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));

	const auto first_pixels = Make_Frame(0);
	VideoSnapshotContext context{
		&renderer,
		{Video_Width, Video_Height, Video_Width * 4, Engine::Video::PixelFormat::RGBA8, 0, 0, first_pixels}
	};
	VisualRegressionHarness harness({
		Video_Width,
		Video_Height,
		2,
		std::filesystem::path(GRAPHICS_VIDEO_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_VIDEO_FAILURE_DIRECTORY)
	});

	RGBAImage first_image;
	BOOST_REQUIRE(harness.Render_Offscreen(device, Render_Video_Frame, &context, first_image));
	BOOST_REQUIRE(first_image.Is_Valid());
	const VisualComparisonResult result = harness.Run(device, "video_frame", Render_Video_Frame, &context);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated video frame snapshot");
	BOOST_CHECK_MESSAGE(result.matched, "video frame presentation snapshot mismatch");
	const RHITextureHandle resident_texture = renderer.Texture_Handle();
	BOOST_REQUIRE(resident_texture.Is_Valid());
	BOOST_CHECK_EQUAL(renderer.Presentation_Count(), 1);
	BOOST_CHECK_EQUAL(renderer.Draw_Count(), 1);

	const auto second_pixels = Make_Frame(1);
	context.frame.pixels = second_pixels;
	context.frame.frame_index = 1;
	context.frame.presentation_time_us = 33333;
	RGBAImage second_image;
	BOOST_REQUIRE(harness.Render_Offscreen(device, Render_Video_Frame, &context, second_image));
	BOOST_CHECK(second_image.Is_Valid());
	BOOST_CHECK(renderer.Texture_Handle() == resident_texture);
	BOOST_CHECK(second_image.pixels != first_image.pixels);
	BOOST_CHECK_EQUAL(renderer.Presentation_Count(), 1);
	BOOST_CHECK_EQUAL(renderer.Draw_Count(), 1);

	renderer.Shutdown();
	BOOST_CHECK(!renderer.Is_Initialized());
	BOOST_CHECK(!renderer.Accepting_Submissions());
}

BOOST_AUTO_TEST_CASE(video_renderer_rejects_submissions_outside_real_rhi_frame_lifecycle)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	VideoRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_VIDEO_SHADER_DIRECTORY)));
	const auto pixels = Make_Frame(0);
	const Engine::Video::DecodedVideoFrame frame{
		Video_Width, Video_Height, Video_Width * 4, Engine::Video::PixelFormat::RGBA8, 0, 0, pixels};

	BOOST_CHECK(!renderer.Submit(1, frame, {0, 0, Video_Width, Video_Height}));
	renderer.Begin_Frame();
	BOOST_REQUIRE(renderer.Submit(1, frame, {0, 0, Video_Width, Video_Height}));
	renderer.Shutdown();
	BOOST_CHECK(!renderer.Submit(1, frame, {0, 0, Video_Width, Video_Height}));
}
