module;

#define BOOST_TEST_MODULE AuthoredRingRendererTests
#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Scene.Ring.Renderer.Tests;

import Assets.Math;
import Assets.Rings;
import Graphics.Scene.Ring.Renderer;
import Graphics.Scene.Ring.Runtime;

#if defined(_WIN32)
import Graphics.Tests.Device;
import Graphics.RHI;
import Graphics.Resources.Textures.References;
import Graphics.Resources.Textures.Sampling;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Shadows.DirectionalRenderer;
#endif

using Graphics::Build_Authored_Ring_Camera_Aligned_World;

namespace
{

void Check_Matrix(const std::array<float, 16> &actual,
	const std::array<float, 16> &expected)
{
	for (std::size_t index = 0; index < actual.size(); ++index)
		BOOST_CHECK_SMALL(actual[index] - expected[index], 0.00001f);
}

}

BOOST_AUTO_TEST_CASE(camera_aligned_world_matches_look_at_axes)
{
	std::array<float, 16> result{};
	BOOST_REQUIRE(Build_Authored_Ring_Camera_Aligned_World(
		{4.0f, 5.0f, 6.0f}, {1.0f, 0.0f, 0.0f}, result));
	Check_Matrix(result, {
		0.0f, 0.0f, -1.0f, 4.0f,
		-1.0f, 0.0f, 0.0f, 5.0f,
		0.0f, 1.0f, 0.0f, 6.0f,
		0.0f, 0.0f, 0.0f, 1.0f});

	BOOST_REQUIRE(Build_Authored_Ring_Camera_Aligned_World(
		{4.0f, 5.0f, 6.0f}, {0.0f, 1.0f, 0.0f}, result));
	Check_Matrix(result, {
		1.0f, 0.0f, 0.0f, 4.0f,
		0.0f, 0.0f, -1.0f, 5.0f,
		0.0f, 1.0f, 0.0f, 6.0f,
		0.0f, 0.0f, 0.0f, 1.0f});

	BOOST_REQUIRE(Build_Authored_Ring_Camera_Aligned_World(
		{4.0f, 5.0f, 6.0f}, {0.0f, 0.0f, 1.0f}, result));
	Check_Matrix(result, {
		0.0f, -1.0f, 0.0f, 4.0f,
		-1.0f, 0.0f, 0.0f, 5.0f,
		0.0f, 0.0f, -1.0f, 6.0f,
		0.0f, 0.0f, 0.0f, 1.0f});
}

BOOST_AUTO_TEST_CASE(camera_aligned_world_matches_look_at_non_axis_direction)
{
	std::array<float, 16> result{};
	BOOST_REQUIRE(Build_Authored_Ring_Camera_Aligned_World(
		{4.0f, 5.0f, 6.0f}, {1.0f, 2.0f, 3.0f}, result));
	Check_Matrix(result, {
		0.89442719f, -0.35856858f, -0.26726124f, 4.0f,
		-0.44721360f, -0.71713716f, -0.53452248f, 5.0f,
		0.0f, 0.59761430f, -0.80178373f, 6.0f,
		0.0f, 0.0f, 0.0f, 1.0f});
}

#if defined(_WIN32)

namespace
{

using PixelBytes = std::vector<std::byte>;

std::size_t Pixel_Offset(unsigned width, unsigned x, unsigned y) noexcept
{
	return (static_cast<std::size_t>(y) * width + x) * 4u;
}

void Check_Pixel_Bounds(const PixelBytes &pixels, unsigned width, unsigned x,
	unsigned y, const std::array<unsigned, 4> &expected, unsigned tolerance)
{
	const std::size_t offset = Pixel_Offset(width, x, y);
	BOOST_REQUIRE(offset + expected.size() <= pixels.size());
	for (unsigned channel = 0; channel < expected.size(); ++channel) {
		const unsigned actual = std::to_integer<unsigned>(pixels[offset + channel]);
		const unsigned lower = expected[channel] > tolerance
			? expected[channel] - tolerance : 0u;
		const unsigned upper = expected[channel] + tolerance;
		BOOST_TEST_CONTEXT("pixel (" << x << ", " << y << "), channel "
			<< channel << ", actual " << actual << ", expected "
			<< expected[channel]) {
			BOOST_CHECK(actual >= lower);
			BOOST_CHECK(actual <= upper);
		}
	}
}

void Prepare_Target(Graphics::GraphicsTestDevice &device,
	Graphics::RHITextureHandle target, Graphics::RHITextureHandle depth,
	unsigned width, unsigned height, float alpha = 0.0f)
{
	auto &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, width, height}));
	BOOST_REQUIRE(commands.Clear({0.0f, 0.0f, 0.0f, alpha}, 1.0f));
}

PixelBytes Read_Target(Graphics::GraphicsTestDevice &device,
	Graphics::RHITextureHandle target, unsigned width, unsigned height)
{
	PixelBytes pixels(static_cast<std::size_t>(width) * height * 4u);
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, width * 4u));
	return pixels;
}

Assets::RingAssetDesc Make_Ring_Asset()
{
	Assets::RingAssetDesc asset;
	asset.name = "authored-ring-test";
	asset.inner_extent = {0.3f, 0.3f};
	asset.outer_extent = {0.8f, 0.8f};
	asset.extent = {0.8f, 0.8f, 0.0f};
	asset.default_color = {1.0f, 0.4f, 0.2f, 1.0f};
	asset.default_alpha = 0.5f;
	asset.material.texturing = false;
	return asset;
}

Graphics::RHITextureHandle Make_Solid_Texture(Graphics::GraphicsTestDevice &device,
	std::array<std::uint8_t, 4> color)
{
	return device.Create_Texture_Initialized({1, 1},
		{std::as_bytes(std::span(color)), 4});
}

Graphics::RHITextureHandle Make_Tiled_Texture(Graphics::GraphicsTestDevice &device)
{
	const std::array<std::uint8_t, 32> texels{{
		255, 0, 0, 255, 0, 255, 0, 255,
		0, 0, 255, 255, 255, 255, 0, 255,
		255, 0, 0, 255, 0, 255, 0, 255,
		0, 0, 255, 255, 255, 255, 0, 255}};
	return device.Create_Texture_Initialized({4, 2},
		{std::as_bytes(std::span(texels)), 16});
}

}

BOOST_AUTO_TEST_CASE(authored_annulus_draws_topology_alpha_and_survives_target_recreation)
{
	Graphics::GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());
	const auto shader_directory = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
	Graphics::PropRenderer renderer;
	Graphics::DirectionalShadowRenderer shadows;
	Graphics::PropSubmission submission;
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	BOOST_REQUIRE(shadows.Initialize(device, shader_directory));
	submission.Initialize(device, renderer, shadows);

	Graphics::AuthoredRingRuntime runtime(Make_Ring_Asset());
	Graphics::AuthoredRingRenderer ring;
	Graphics::AuthoredRingDrawInput input;
	input.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 1};
	input.view = input.view_projection;
	input.world = input.view_projection;

	auto target = device.Create_Texture({32, 32, 1, Graphics::RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(Graphics::RHITextureUsage::RenderTarget)});
	auto depth = device.Create_Texture({32, 32, 1, Graphics::RHITextureFormat::D32_Float,
		static_cast<unsigned>(Graphics::RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
	Prepare_Target(device, target, depth, 32, 32);
	BOOST_REQUIRE(ring.Submit(renderer, submission, runtime, input,
		std::span<const Graphics::RHITextureHandle>{}));

	const PixelBytes first = Read_Target(device, target, 32, 32);
	// The quantized source is {255,102,51,128}; source-alpha blending over
	// transparent black therefore produces this bounded, nonzero alpha.
	Check_Pixel_Bounds(first, 32, 16, 6, {128, 51, 26, 64}, 3);
	Check_Pixel_Bounds(first, 32, 6, 16, {128, 51, 26, 64}, 3);
	Check_Pixel_Bounds(first, 32, 16, 25, {128, 51, 26, 64}, 3);
	// These centers fall inside the [2j, 2j+1, 2j+2] triangles. With the
	// identity matrices and 32x32 viewport their screen-space signed area is
	// negative, the orientation removed by the default back-face cull. Keep
	// explicit clear probes so a winding or cull-policy change is observable.
	Check_Pixel_Bounds(first, 32, 9, 12, {0, 0, 0, 0}, 0);
	Check_Pixel_Bounds(first, 32, 12, 22, {0, 0, 0, 0}, 0);
	Check_Pixel_Bounds(first, 32, 22, 19, {0, 0, 0, 0}, 0);
	Check_Pixel_Bounds(first, 32, 19, 9, {0, 0, 0, 0}, 0);
	Check_Pixel_Bounds(first, 32, 16, 16, {0, 0, 0, 0}, 0);
	Check_Pixel_Bounds(first, 32, 0, 0, {0, 0, 0, 0}, 0);

	// Recreate both the targets and the renderer's GPU state. The authored
	// mesh remains CPU-owned by the renderer and must upload again on demand.
	submission.Shutdown();
	shadows.Shutdown();
	renderer.Shutdown();
	BOOST_REQUIRE(device.Destroy_Texture(target));
	BOOST_REQUIRE(device.Destroy_Texture(depth));
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	BOOST_REQUIRE(shadows.Initialize(device, shader_directory));
	submission.Initialize(device, renderer, shadows);
	target = device.Create_Texture({48, 24, 1, Graphics::RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(Graphics::RHITextureUsage::RenderTarget)});
	depth = device.Create_Texture({48, 24, 1, Graphics::RHITextureFormat::D32_Float,
		static_cast<unsigned>(Graphics::RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
	input.scene.color_write_mask = 7;
	Prepare_Target(device, target, depth, 48, 24, 37.0f / 255.0f);
	BOOST_REQUIRE(ring.Submit(renderer, submission, runtime, input,
		std::span<const Graphics::RHITextureHandle>{}));
	const PixelBytes recreated = Read_Target(device, target, 48, 24);
	Check_Pixel_Bounds(recreated, 48, 36, 12, {128, 51, 26, 37}, 3);
	Check_Pixel_Bounds(recreated, 48, 24, 12, {0, 0, 0, 37}, 0);
	Check_Pixel_Bounds(recreated, 48, 0, 0, {0, 0, 0, 37}, 0);

	ring.Release(renderer);
	submission.Shutdown();
	shadows.Shutdown();
	renderer.Shutdown();
	BOOST_REQUIRE(device.Destroy_Texture(target));
	BOOST_REQUIRE(device.Destroy_Texture(depth));
}

BOOST_AUTO_TEST_CASE(authored_ring_samples_tiled_texture_after_invalid_and_source_release)
{
	Graphics::GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());
	const auto shader_directory = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
	Graphics::PropRenderer renderer;
	Graphics::DirectionalShadowRenderer shadows;
	Graphics::PropSubmission submission;
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	BOOST_REQUIRE(shadows.Initialize(device, shader_directory));
	submission.Initialize(device, renderer, shadows);

	Assets::RingAssetDesc asset = Make_Ring_Asset();
	asset.default_color = {1.0f, 1.0f, 1.0f, 1.0f};
	asset.default_alpha = 1.0f;
	asset.texture_name = "authored-ring-tiles";
	asset.texture_tile_count = 2;
	asset.material.source_blend = Assets::RingBlendFactor::One;
	asset.material.destination_blend = Assets::RingBlendFactor::Zero;
	// The authored strip preserves (2j, 2j+1, 2j+2) and
	// (2j+1, 2j+2, 2j+3), so adjacent facets have opposite winding. Disable
	// culling here so the UV checks exercise both triangles of both repeats;
	// the default culling path remains covered by the annulus draw above.
	asset.material.cull_enabled = false;
	Graphics::AuthoredRingRuntime runtime(asset);
	Graphics::AuthoredRingRenderer ring;
	Graphics::AuthoredRingDrawInput input;
	input.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 1};
	input.view = input.view_projection;
	input.world = input.view_projection;
	input.texture_sampling.minification = Graphics::SamplingFilter::Disabled;
	input.texture_sampling.magnification = Graphics::SamplingFilter::Disabled;
	input.texture_sampling.mipmap = Graphics::SamplingFilter::Disabled;

	auto target = device.Create_Texture({32, 32, 1, Graphics::RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(Graphics::RHITextureUsage::RenderTarget)});
	auto depth = device.Create_Texture({32, 32, 1, Graphics::RHITextureFormat::D32_Float,
		static_cast<unsigned>(Graphics::RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
	Prepare_Target(device, target, depth, 32, 32);

	// A rejected draw must leave a valid caller-owned handle untouched.
	const auto rejected_texture = Make_Solid_Texture(device, {17, 29, 43, 61});
	BOOST_REQUIRE(rejected_texture.Is_Valid());
	Graphics::AuthoredRingDrawInput invalid_input = input;
	invalid_input.view_projection[0] = std::numeric_limits<float>::quiet_NaN();
	BOOST_CHECK(!ring.Submit(renderer, submission, runtime, invalid_input,
		std::array{rejected_texture}));
	std::array<std::byte, 4> rejected_readback{};
	BOOST_REQUIRE(device.Readback_Texture(rejected_texture, rejected_readback, 4));
	BOOST_CHECK(rejected_readback[0] == std::byte{17});
	BOOST_CHECK(rejected_readback[1] == std::byte{29});
	BOOST_CHECK(rejected_readback[2] == std::byte{43});
	BOOST_CHECK(rejected_readback[3] == std::byte{61});
	BOOST_REQUIRE(device.Destroy_Texture(rejected_texture));

	const auto texture = Make_Tiled_Texture(device);
	BOOST_REQUIRE(texture.Is_Valid());
	// Keep one caller reference while the immediate submission consumes the
	// transferred reference, then release the caller's copy explicitly.
	BOOST_REQUIRE(device.Retain_Texture(texture));
	BOOST_REQUIRE(ring.Submit(renderer, submission, runtime, input,
		std::array{texture}));
	BOOST_REQUIRE(device.Destroy_Texture(texture));
	BOOST_CHECK(!device.Readback_Texture(texture, rejected_readback, 4));

	const PixelBytes pixels = Read_Target(device, target, 32, 32);
	// For the generated 48-segment strip, triangle vertices at segment j have
	// U={j/24,j/24,(j+1)/24,(j+1)/24}; rasterization therefore uses the
	// triangle's barycentric interpolation of those endpoint values. These
	// pixel centers are well inside a specific triangle and the annulus, with
	// U comfortably inside each point-sampled texel, so the expected columns are
	// independent of an ideal-circle approximation. The two source rows are
	// identical, which isolates U while covering all four columns twice.
	Check_Pixel_Bounds(pixels, 32, 12, 6, {255, 0, 0, 255}, 2);
	Check_Pixel_Bounds(pixels, 32, 9, 12, {0, 255, 0, 255}, 2);
	Check_Pixel_Bounds(pixels, 32, 6, 19, {0, 0, 255, 255}, 2);
	Check_Pixel_Bounds(pixels, 32, 12, 22, {255, 255, 0, 255}, 2);
	Check_Pixel_Bounds(pixels, 32, 19, 25, {255, 0, 0, 255}, 2);
	Check_Pixel_Bounds(pixels, 32, 22, 19, {0, 255, 0, 255}, 2);
	Check_Pixel_Bounds(pixels, 32, 25, 12, {0, 0, 255, 255}, 2);
	Check_Pixel_Bounds(pixels, 32, 19, 9, {255, 255, 0, 255}, 2);
	Check_Pixel_Bounds(pixels, 32, 16, 16, {0, 0, 0, 0}, 0);
	Check_Pixel_Bounds(pixels, 32, 0, 0, {0, 0, 0, 0}, 0);

	ring.Release(renderer);
	submission.Shutdown();
	shadows.Shutdown();
	renderer.Shutdown();
	BOOST_REQUIRE(device.Destroy_Texture(target));
	BOOST_REQUIRE(device.Destroy_Texture(depth));
}

BOOST_AUTO_TEST_CASE(authored_ring_texture_transfer_preserves_cached_source_across_repeated_and_deferred_draws)
{
	Graphics::GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());
	const auto shader_directory = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
	Graphics::PropRenderer renderer;
	Graphics::DirectionalShadowRenderer shadows;
	Graphics::PropSubmission submission;
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	BOOST_REQUIRE(shadows.Initialize(device, shader_directory));
	submission.Initialize(device, renderer, shadows);
	Graphics::TextureReferences references;

	const auto target = device.Create_Texture({32, 32, 1, Graphics::RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(Graphics::RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({32, 32, 1, Graphics::RHITextureFormat::D32_Float,
		static_cast<unsigned>(Graphics::RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());

	Assets::RingAssetDesc asset = Make_Ring_Asset();
	asset.default_color = {1.0f, 1.0f, 1.0f, 1.0f};
	asset.default_alpha = 1.0f;
	asset.texture_name = "cached-ring";
	asset.texture_tile_count = 1;
	asset.material.source_blend = Assets::RingBlendFactor::One;
	asset.material.destination_blend = Assets::RingBlendFactor::Zero;
	Graphics::AuthoredRingRuntime runtime(asset);
	Graphics::AuthoredRingRenderer ring;
	Graphics::AuthoredRingDrawInput input;
	input.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 1};
	input.view = input.view_projection;
	input.world = input.view_projection;
	input.texture_sampling.minification = Graphics::SamplingFilter::Disabled;
	input.texture_sampling.magnification = Graphics::SamplingFilter::Disabled;
	input.texture_sampling.mipmap = Graphics::SamplingFilter::Disabled;

	const std::array<std::uint8_t, 4> texel{255, 0, 0, 255};
	const auto source = device.Create_Texture_Initialized({1, 1},
		{std::as_bytes(std::span(texel)), 4});
	BOOST_REQUIRE(source.Is_Valid());
	const auto borrowed = references.Retain(device, source);
	BOOST_REQUIRE(borrowed.Is_Valid());
	// The cache owns the only remaining source reference after this release.
	BOOST_REQUIRE(device.Destroy_Texture(source));

	PixelBytes pixels(32u * 32u * 4u);
	std::array<std::byte, 4> source_readback{};
	for (unsigned draw = 0; draw < 2; ++draw) {
		Prepare_Target(device, target, depth, 32, 32);
		// PropSubmission consumes this transfer reference on success. The
		// cache-owned reference must survive both immediate draws.
		BOOST_REQUIRE(device.Retain_Texture(borrowed));
		BOOST_REQUIRE(ring.Submit(renderer, submission, runtime, input,
			std::array{borrowed}));
		BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32 * 4));
		Check_Pixel_Bounds(pixels, 32, 16, 6, {255u, 0u, 0u, 255u}, 3);
		BOOST_REQUIRE(device.Readback_Texture(borrowed, source_readback, 4));
		BOOST_CHECK((source_readback == std::array<std::byte, 4>{
			std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255}}));
	}

	// A rejected submission leaves the transfer reference with its caller;
	// release that reference while retaining the cache entry.
	Graphics::AuthoredRingDrawInput invalid_input = input;
	invalid_input.view_projection[0] = std::numeric_limits<float>::quiet_NaN();
	BOOST_REQUIRE(device.Retain_Texture(borrowed));
	BOOST_CHECK(!ring.Submit(renderer, submission, runtime, invalid_input,
		std::array{borrowed}));
	BOOST_REQUIRE(device.Destroy_Texture(borrowed));
	BOOST_REQUIRE(device.Readback_Texture(borrowed, source_readback, 4));

	// A transparent draw retains the transfer until the queue flushes. The
	// original source reference was already released above, so this also
	// covers the deferred lifetime supplied solely by the cache and queue.
	Assets::RingAssetDesc deferred_asset = asset;
	deferred_asset.material.destination_blend = Assets::RingBlendFactor::InverseSourceAlpha;
	Graphics::AuthoredRingRuntime deferred_runtime(deferred_asset);
	Graphics::AuthoredRingDrawInput deferred_input = input;
	deferred_input.sorting_enabled = true;
	Prepare_Target(device, target, depth, 32, 32);
	BOOST_REQUIRE(device.Retain_Texture(borrowed));
	BOOST_REQUIRE(ring.Submit(renderer, submission, deferred_runtime,
		deferred_input, std::array{borrowed}));
	BOOST_REQUIRE(submission.Flush_Transparent());
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32 * 4));
	Check_Pixel_Bounds(pixels, 32, 16, 6, {255u, 0u, 0u, 255u}, 3);
	BOOST_REQUIRE(device.Readback_Texture(borrowed, source_readback, 4));

	ring.Release(renderer);
	submission.Shutdown();
	shadows.Shutdown();
	renderer.Shutdown();
	references.Clear();
	BOOST_CHECK(!device.Retain_Texture(borrowed));
	BOOST_REQUIRE(device.Destroy_Texture(target));
	BOOST_REQUIRE(device.Destroy_Texture(depth));
}

BOOST_AUTO_TEST_CASE(authored_ring_submission_draws_immediately_and_preserves_transparent_fifo)
{
	Graphics::GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());
	const auto shader_directory = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
	Graphics::PropRenderer renderer;
	Graphics::DirectionalShadowRenderer shadows;
	Graphics::PropSubmission submission;
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	BOOST_REQUIRE(shadows.Initialize(device, shader_directory));
	submission.Initialize(device, renderer, shadows);

	auto target = device.Create_Texture({32, 32, 1, Graphics::RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(Graphics::RHITextureUsage::RenderTarget)});
	auto depth = device.Create_Texture({32, 32, 1, Graphics::RHITextureFormat::D32_Float,
		static_cast<unsigned>(Graphics::RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
	Graphics::AuthoredRingDrawInput input;
	input.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 1};
	input.view = input.view_projection;
	input.world = input.view_projection;

	Assets::RingAssetDesc immediate_asset = Make_Ring_Asset();
	immediate_asset.default_color = {0.2f, 0.8f, 0.1f, 1.0f};
	immediate_asset.default_alpha = 1.0f;
	immediate_asset.material.source_blend = Assets::RingBlendFactor::One;
	immediate_asset.material.destination_blend = Assets::RingBlendFactor::Zero;
	Graphics::AuthoredRingRuntime immediate_runtime(immediate_asset);
	Graphics::AuthoredRingRenderer immediate_ring;
	Prepare_Target(device, target, depth, 32, 32);
	BOOST_REQUIRE(immediate_ring.Submit(renderer, submission, immediate_runtime, input,
		std::span<const Graphics::RHITextureHandle>{}));
	const PixelBytes immediate = Read_Target(device, target, 32, 32);
	Check_Pixel_Bounds(immediate, 32, 16, 8, {51, 204, 26, 255}, 2);

	// Queue two overlapping, noncommutative alpha draws. The first ring's
	// mesh and texture must remain alive after its owner publishes a replacement.
	BOOST_REQUIRE(device.Immediate_Command_List().Clear({0, 0, 0, 0}, 1.0f));
	Assets::RingAssetDesc queued_asset = Make_Ring_Asset();
	queued_asset.default_color = {1.0f, 1.0f, 1.0f, 1.0f};
	queued_asset.default_alpha = 1.0f;
	queued_asset.texture_name = "queued-ring-texture";
	queued_asset.texture_tile_count = 1;
	Graphics::AuthoredRingRuntime queued_runtime(queued_asset);
	Graphics::AuthoredRingRenderer queued_ring;
	input.sorting_enabled = true;

	const auto red = Make_Solid_Texture(device, {255, 0, 0, 128});
	BOOST_REQUIRE(red.Is_Valid());
	BOOST_REQUIRE(device.Retain_Texture(red));
	BOOST_REQUIRE(queued_ring.Submit(renderer, submission, queued_runtime, input,
		std::array{red}));
	BOOST_REQUIRE(device.Destroy_Texture(red));
	const Graphics::PropMeshHandle first_mesh = queued_ring.Mesh();
	BOOST_REQUIRE(first_mesh.Is_Valid());

	queued_runtime.Set_Outer_Scale({1.1f, 1.1f});
	const auto blue = Make_Solid_Texture(device, {0, 0, 255, 128});
	BOOST_REQUIRE(blue.Is_Valid());
	input.world[11] = 0.0f;
	BOOST_REQUIRE(device.Retain_Texture(blue));
	BOOST_REQUIRE(queued_ring.Submit(renderer, submission, queued_runtime, input,
		std::array{blue}));
	BOOST_REQUIRE(device.Destroy_Texture(blue));
	const Graphics::PropMeshHandle second_mesh = queued_ring.Mesh();
	BOOST_REQUIRE(second_mesh.Is_Valid());
	BOOST_CHECK(first_mesh != second_mesh);

	const PixelBytes before_flush = Read_Target(device, target, 32, 32);
	Check_Pixel_Bounds(before_flush, 32, 16, 8, {0, 0, 0, 0}, 0);
	queued_ring.Release(renderer);
	BOOST_REQUIRE(submission.Flush_Transparent());
	const PixelBytes after_flush = Read_Target(device, target, 32, 32);
	// Red then blue, each with alpha 128, yields {64,0,128,96}; reversing
	// either queue order or triangle order fails these channel expectations.
	Check_Pixel_Bounds(after_flush, 32, 16, 8, {64, 0, 128, 96}, 3);
	Check_Pixel_Bounds(after_flush, 32, 16, 16, {0, 0, 0, 0}, 0);
	Check_Pixel_Bounds(after_flush, 32, 0, 0, {0, 0, 0, 0}, 0);
	std::array<std::byte, 4> released_texture{};
	BOOST_CHECK(!device.Readback_Texture(red, released_texture, 4));
	BOOST_CHECK(!device.Readback_Texture(blue, released_texture, 4));

	immediate_ring.Release(renderer);
	submission.Shutdown();
	shadows.Shutdown();
	renderer.Shutdown();
	BOOST_REQUIRE(device.Destroy_Texture(target));
	BOOST_REQUIRE(device.Destroy_Texture(depth));
}

#endif
