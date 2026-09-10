module;

#define BOOST_TEST_MODULE GraphicsBindlessDrawingTests

#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#include <boost/test/included/unit_test.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

export module Graphics.RHI.BindlessDrawing.Tests;

import Graphics.RHI;
import Graphics.Tests.Device;

using namespace Graphics;

#ifndef GRAPHICS_BINDLESS_SHADER_DIRECTORY
#ifdef GRAPHICS_TERRAIN_SHADER_DIRECTORY
#define GRAPHICS_BINDLESS_SHADER_DIRECTORY GRAPHICS_TERRAIN_SHADER_DIRECTORY
#else
#define GRAPHICS_BINDLESS_SHADER_DIRECTORY "."
#endif
#endif

namespace
{

constexpr std::uint32_t StressWidth = 64;
constexpr std::uint32_t StressHeight = 64;
constexpr std::uint32_t StressBlockSize = 768;
constexpr std::uint32_t StressDrawCount = StressBlockSize * 4;

constexpr std::uint32_t Usage(RHITextureUsage usage) noexcept
{
	return static_cast<std::uint32_t>(usage);
}

struct BeamVertex final
{
	float position[3]{};
	float color[4]{};
	float uv[2]{};
	std::uint32_t resource_index = 0xffffffffu;
};

static_assert(sizeof(BeamVertex) == 40);
static_assert(offsetof(BeamVertex, color) == 12);

RHITextureHandle Create_Solid_Texture(Device &device, std::array<std::byte, 4> pixel)
{
	const std::span<const std::byte> bytes(pixel.data(), pixel.size());
	return device.Create_Texture_Initialized(
		{1, 1, 1, RHITextureFormat::RGBA8_UNorm, Usage(RHITextureUsage::ShaderResource)},
		{bytes, 4});
}

RHIPipelineHandle Create_Beam_Pipeline(Device &device, bool scissor_test = false)
{
	RHIPipeline description;
	description.key = scissor_test ? 0x42494e444c455353ull : 0x42494e444c455353ull + 1;
	description.depth_test = false;
	description.depth_write = false;
	description.cull_mode = RHICullMode::None;
	description.scissor_test = scissor_test;
	description.vertex_format = RHIVertexFormat::Position3Color4UV2ResourceIndex;
	description.samplers[0].Set_Filter(RHISamplerFilter::Point);
	description.sampler_count = 1;
	return device.Create_Pipeline(description);
}

void Set_Texture_Binding(RHIBindlessResource &resource, std::uint32_t slot,
	RHITextureHandle texture) noexcept
{
	resource = {};
	resource.index = ResourceIndex(slot, 1);
	resource.type = RHIResourceType::Texture;
	resource.texture = texture;
	resource.stage = RHIShaderStage::Fragment;
}

void Set_Material_Binding(RHIBindlessResource &resource, RHIBufferHandle buffer) noexcept
{
	resource = {};
	resource.index = ResourceIndex(0, 1);
	resource.type = RHIResourceType::Material;
	resource.buffer = buffer;
	resource.constant_buffer_slot = 0;
}

std::array<BeamVertex, 6> Make_Quad(float left, float top, float right,
	float bottom, std::uint32_t resource_index)
{
	const BeamVertex top_left{{left, top, 0.5f}, {1, 1, 1, 1}, {0, 0}, resource_index};
	const BeamVertex top_right{{right, top, 0.5f}, {1, 1, 1, 1}, {1, 0}, resource_index};
	const BeamVertex bottom_left{{left, bottom, 0.5f}, {1, 1, 1, 1}, {0, 1}, resource_index};
	const BeamVertex bottom_right{{right, bottom, 0.5f}, {1, 1, 1, 1}, {1, 1}, resource_index};
	return {top_left, top_right, bottom_left, bottom_left, top_right, bottom_right};
}

float Pixel_Left(std::uint32_t pixel, std::uint32_t width) noexcept
{
	return -1.0f + 2.0f * static_cast<float>(pixel) / static_cast<float>(width);
}

float Pixel_Right(std::uint32_t pixel, std::uint32_t width) noexcept
{
	return -1.0f + 2.0f * static_cast<float>(pixel + 1) / static_cast<float>(width);
}

float Pixel_Top(std::uint32_t pixel, std::uint32_t height) noexcept
{
	return 1.0f - 2.0f * static_cast<float>(pixel) / static_cast<float>(height);
}

float Pixel_Bottom(std::uint32_t pixel, std::uint32_t height) noexcept
{
	return 1.0f - 2.0f * static_cast<float>(pixel + 1) / static_cast<float>(height);
}

void Append_Quad(std::span<BeamVertex> vertices, std::uint32_t quad,
	std::uint32_t left, std::uint32_t top, std::uint32_t right,
	std::uint32_t bottom, std::uint32_t resource_index, std::uint32_t width,
	std::uint32_t height)
{
	const auto quad_vertices = Make_Quad(
		Pixel_Left(left, width), Pixel_Top(top, height), Pixel_Right(right, width),
		Pixel_Bottom(bottom, height), resource_index);
	const auto destination = vertices.subspan(static_cast<std::size_t>(quad) * 6, 6);
	std::copy(quad_vertices.begin(), quad_vertices.end(), destination.begin());
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
	for (std::size_t channel = 0; channel < actual.size(); ++channel) {
		BOOST_CHECK_SMALL(
			static_cast<int>(actual[channel]) - static_cast<int>(expected[channel]),
			static_cast<int>(tolerance));
	}
}

GraphicsTestDeviceOptions Beam_Device_Options()
{
	GraphicsTestDeviceOptions options;
	options.use_warp = true;
	options.shader_directory = GRAPHICS_BINDLESS_SHADER_DIRECTORY;
	options.vertex_shader_name = "beam.vso";
	options.fragment_shader_name = "beam.pso";
	return options;
}

}

BOOST_AUTO_TEST_CASE(retired_sampler_descriptors_are_reused_after_gpu_completion)
{
	constexpr std::uint32_t width = 129;
	constexpr std::uint32_t height = 2;
	GraphicsTestDevice device(Beam_Device_Options());
	BOOST_REQUIRE(device.Is_Valid());
	const auto target = device.Create_Texture({width, height, 1, RHITextureFormat::RGBA8_UNorm,
		Usage(RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({width, height, 1, RHITextureFormat::D32_Float,
		Usage(RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid());
	BOOST_REQUIRE(depth.Is_Valid());
	const std::array<std::byte, 8> texels{std::byte{20}, std::byte{200}, std::byte{40}, std::byte{64},
		std::byte{200}, std::byte{30}, std::byte{180}, std::byte{192}};
	const auto texture = device.Create_Texture_Initialized(
		{2, 1, 1, RHITextureFormat::RGBA8_UNorm, Usage(RHITextureUsage::ShaderResource)}, {texels, 8});
	BOOST_REQUIRE(texture.Is_Valid());
	auto vertices = Make_Quad(-1, 1, 1, -1, 0);
	for (auto &vertex : vertices) { vertex.uv[0] = 1; vertex.uv[1] = 0.5f; }
	const auto buffer = device.Create_Buffer_Initialized(
		{sizeof(vertices), RHIBufferUsage::Vertex, sizeof(BeamVertex)}, std::as_bytes(std::span(vertices)));
	BOOST_REQUIRE(buffer.Is_Valid());
	auto &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, width, height, 0, 1}));
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
	BOOST_REQUIRE(commands.Set_Vertex_Buffer(0, buffer, sizeof(BeamVertex), 0));
	RHIBindlessResource binding;
	Set_Texture_Binding(binding, 0, texture);
	std::vector<std::byte> pixels(width * height * 4);
	for (std::uint32_t x = 0; x < width; ++x) {
		BOOST_TEST_CONTEXT("pipeline generation " << x) {
			RHIPipeline description;
			description.depth_test = description.depth_write = false;
			description.cull_mode = RHICullMode::None;
			description.scissor_test = true;
			description.vertex_format = RHIVertexFormat::Position3Color4UV2ResourceIndex;
			description.sampler_count = 16;
			for (std::uint32_t slot = 0; slot < description.samplers.size(); ++slot) {
				auto &sampler = description.samplers[slot];
				sampler.Set_Filter(RHISamplerFilter::Point);
				sampler.address[0] = x % 2 == 0 ? RHISamplerAddress::Wrap : RHISamplerAddress::Clamp;
				sampler.min_lod = (x * 16 + slot) * 0.0001f;
			}
			const auto pipeline = device.Create_Pipeline(description);
			BOOST_REQUIRE(pipeline.Is_Valid());
			BOOST_REQUIRE(commands.Bind_Pipeline(pipeline));
			BOOST_REQUIRE(commands.Set_Bindless_Resources(std::span(&binding, 1)));
			BOOST_REQUIRE(commands.Set_Scissor({x, 0, 1, height}));
			BOOST_REQUIRE(commands.Draw(6));
			BOOST_REQUIRE(device.Destroy_Pipeline(pipeline));
			// Retired slots cannot be overwritten before these recorded draws finish.
			// Across all groups, 2,064 distinct samplers exceed the physical heap.
			if (x % 16 == 15 || x + 1 == width) {
				BOOST_REQUIRE(device.Readback_Texture(target, pixels, width * 4));
				for (std::uint32_t drawn = 0; drawn <= x; ++drawn)
					for (std::uint32_t y = 0; y < height; ++y)
						Check_Pixel(pixels, width, drawn, y, drawn % 2 == 0
							? std::array<unsigned, 4>{20, 200, 40, 64}
							: std::array<unsigned, 4>{200, 30, 180, 192}, 1);
			}
		}
	}
	BOOST_REQUIRE(device.Destroy_Buffer(buffer));
	BOOST_REQUIRE(device.Destroy_Texture(texture));
	BOOST_REQUIRE(device.Destroy_Texture(depth));
	BOOST_REQUIRE(device.Destroy_Texture(target));
}

BOOST_AUTO_TEST_CASE(pipeline_switches_preserve_sampler_addressing_filtering_and_alpha)
{
	constexpr std::uint32_t width = 96;
	constexpr std::uint32_t height = 2;
	GraphicsTestDevice device(Beam_Device_Options());
	BOOST_REQUIRE(device.Is_Valid());
	const auto target = device.Create_Texture({width, height, 1, RHITextureFormat::RGBA8_UNorm,
		Usage(RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({width, height, 1, RHITextureFormat::D32_Float,
		Usage(RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid());
	BOOST_REQUIRE(depth.Is_Valid());
	const std::array<std::byte, 8> texels{std::byte{20}, std::byte{200}, std::byte{40}, std::byte{64},
		std::byte{200}, std::byte{30}, std::byte{180}, std::byte{192}};
	const auto texture = device.Create_Texture_Initialized(
		{2, 1, 1, RHITextureFormat::RGBA8_UNorm, Usage(RHITextureUsage::ShaderResource)}, {texels, 8});
	BOOST_REQUIRE(texture.Is_Valid());
	auto vertices = Make_Quad(-1, 1, 1, -1, 0);
	for (auto &vertex : vertices) {
		vertex.uv[0] = 1.0f;
		vertex.uv[1] = 0.5f;
	}
	const auto buffer = device.Create_Buffer_Initialized(
		{sizeof(vertices), RHIBufferUsage::Vertex, sizeof(BeamVertex)}, std::as_bytes(std::span(vertices)));
	BOOST_REQUIRE(buffer.Is_Valid());
	std::array<RHIPipelineHandle, 3> pipelines{};
	for (std::uint32_t index = 0; index < pipelines.size(); ++index) {
		RHIPipeline description;
		description.key = 0x53414d504c455200ull + index;
		description.depth_test = false;
		description.depth_write = false;
		description.cull_mode = RHICullMode::None;
		description.scissor_test = true;
		description.vertex_format = RHIVertexFormat::Position3Color4UV2ResourceIndex;
		description.samplers[0].Set_Filter(index == 2 ? RHISamplerFilter::Linear : RHISamplerFilter::Point);
		description.samplers[0].address[0] = index == 1 ? RHISamplerAddress::Clamp : RHISamplerAddress::Wrap;
		pipelines[index] = device.Create_Pipeline(description);
		BOOST_REQUIRE(pipelines[index].Is_Valid());
	}
	auto &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, width, height, 0, 1}));
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
	BOOST_REQUIRE(commands.Set_Vertex_Buffer(0, buffer, sizeof(BeamVertex), 0));
	RHIBindlessResource binding;
	Set_Texture_Binding(binding, 0, texture);
	// More than 32 switches crosses the former 512-descriptor frame region.
	// Each stripe selects a distinct addressing/filtering result, including alpha.
	for (std::uint32_t x = 0; x < width; ++x) {
		BOOST_REQUIRE(commands.Bind_Pipeline(pipelines[x % pipelines.size()]));
		BOOST_REQUIRE(commands.Set_Bindless_Resources(std::span(&binding, 1)));
		BOOST_REQUIRE(commands.Set_Scissor({x, 0, 1, height}));
		BOOST_REQUIRE(commands.Draw(6));
	}
	for (const auto pipeline : pipelines) BOOST_REQUIRE(device.Destroy_Pipeline(pipeline));
	BOOST_REQUIRE(device.Destroy_Buffer(buffer));
	BOOST_REQUIRE(device.Destroy_Texture(texture));
	std::vector<std::byte> pixels(width * height * 4);
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, width * 4));
	const std::array<std::array<unsigned, 4>, 3> expected{{
		{20, 200, 40, 64}, {200, 30, 180, 192}, {110, 115, 110, 128}}};
	for (std::uint32_t x = 0; x < width; ++x)
		for (std::uint32_t y = 0; y < height; ++y)
			Check_Pixel(pixels, width, x, y, expected[x % expected.size()], 1);
	BOOST_REQUIRE(device.Destroy_Texture(depth));
	BOOST_REQUIRE(device.Destroy_Texture(target));
}

BOOST_AUTO_TEST_CASE(bindless_draw_uses_multiple_indices_and_retains_replaced_resources)
{
	constexpr std::uint32_t width = 32;
	constexpr std::uint32_t height = 24;

	GraphicsTestDevice device(Beam_Device_Options());
	BOOST_REQUIRE(device.Is_Valid());

	const auto target = device.Create_Texture({width, height, 1, RHITextureFormat::RGBA8_UNorm,
		Usage(RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({width, height, 1, RHITextureFormat::D32_Float,
		Usage(RHITextureUsage::DepthStencil)});
	const auto pipeline = Create_Beam_Pipeline(device);
	BOOST_REQUIRE(target.Is_Valid());
	BOOST_REQUIRE(depth.Is_Valid());
	BOOST_REQUIRE(pipeline.Is_Valid());

	// These source bytes and expected values are deliberately separate literals.
	const std::array<RHITextureHandle, 4> first_textures{{
		Create_Solid_Texture(device, {std::byte{235}, std::byte{17}, std::byte{91}, std::byte{65}}),
		Create_Solid_Texture(device, {std::byte{19}, std::byte{201}, std::byte{53}, std::byte{131}}),
		Create_Solid_Texture(device, {std::byte{49}, std::byte{73}, std::byte{227}, std::byte{193}}),
		Create_Solid_Texture(device, {std::byte{221}, std::byte{143}, std::byte{11}, std::byte{239}})}};
	const std::array<std::array<unsigned, 4>, 4> first_expected{{
		{{235, 17, 91, 65}}, {{19, 201, 53, 131}},
		{{49, 73, 227, 193}}, {{221, 143, 11, 239}}}};
	const std::array<std::array<unsigned, 4>, 4> second_expected{{
		{{12, 218, 42, 91}}, {{207, 34, 166, 123}},
		{{41, 89, 233, 177}}, {{243, 119, 9, 221}}}};
	std::array<RHITextureHandle, 4> second_textures{};
	for (const auto texture : first_textures)
		BOOST_REQUIRE(texture.Is_Valid());

	std::array<BeamVertex, 48> vertices{};
	const std::array<std::uint32_t, 4> slots{{0, 1, 2, 127}};
	for (std::uint32_t quad = 0; quad < 4; ++quad)
		Append_Quad(vertices, quad, 1 + quad * 8, 2, 7 + quad * 8, 10,
			slots[quad], width, height);
	for (std::uint32_t quad = 0; quad < 4; ++quad)
		Append_Quad(vertices, 4 + quad, 1 + quad * 8, 14, 7 + quad * 8, 22,
			slots[quad], width, height);
	const auto vertex_buffer = device.Create_Buffer_Initialized(
		{static_cast<std::uint32_t>(sizeof(vertices)), RHIBufferUsage::Vertex,
			static_cast<std::uint32_t>(sizeof(BeamVertex))},
		std::as_bytes(std::span<const BeamVertex>(vertices)));
	BOOST_REQUIRE(vertex_buffer.Is_Valid());

	std::array<RHIBindlessResource, 128> first_bindings{};
	for (std::uint32_t quad = 0; quad < first_textures.size(); ++quad)
		Set_Texture_Binding(first_bindings[slots[quad]], slots[quad], first_textures[quad]);

	CommandList &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, width, height, 0.0f, 1.0f}));
	BOOST_REQUIRE(commands.Clear({0.125f, 0.25f, 0.375f, 0.5f}, 1.0f));
	BOOST_REQUIRE(commands.Bind_Pipeline(pipeline));
	BOOST_REQUIRE(commands.Set_Vertex_Buffer(0, vertex_buffer, sizeof(BeamVertex), 0));
	BOOST_REQUIRE(commands.Set_Bindless_Resources(first_bindings));
	BOOST_REQUIRE(commands.Draw(24));

	// Release the logical sources before replacement and readback. The first
	// draw still owns its native views through the in-flight submission.
	for (const auto texture : first_textures)
		BOOST_REQUIRE(device.Destroy_Texture(texture));
	second_textures = {{
		Create_Solid_Texture(device, {std::byte{12}, std::byte{218}, std::byte{42}, std::byte{91}}),
		Create_Solid_Texture(device, {std::byte{207}, std::byte{34}, std::byte{166}, std::byte{123}}),
		Create_Solid_Texture(device, {std::byte{41}, std::byte{89}, std::byte{233}, std::byte{177}}),
		Create_Solid_Texture(device, {std::byte{243}, std::byte{119}, std::byte{9}, std::byte{221}})}};
	for (const auto texture : second_textures)
		BOOST_REQUIRE(texture.Is_Valid());
	std::array<RHIBindlessResource, 128> second_bindings{};
	for (std::uint32_t quad = 0; quad < second_textures.size(); ++quad)
		Set_Texture_Binding(second_bindings[slots[quad]], slots[quad], second_textures[quad]);
	BOOST_REQUIRE(commands.Set_Bindless_Resources(second_bindings));
	BOOST_REQUIRE(commands.Draw(24, 24));
	for (const auto texture : second_textures)
		BOOST_REQUIRE(device.Destroy_Texture(texture));
	BOOST_REQUIRE(device.Destroy_Buffer(vertex_buffer));
	BOOST_REQUIRE(device.Destroy_Pipeline(pipeline));

	std::vector<std::byte> pixels(static_cast<std::size_t>(width) * height * 4);
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, width * 4));
	for (std::uint32_t quad = 0; quad < 4; ++quad) {
		Check_Pixel(pixels, width, 4 + quad * 8, 6, first_expected[quad]);
		Check_Pixel(pixels, width, 4 + quad * 8, 18, second_expected[quad]);
	}
	Check_Pixel(pixels, width, 0, 0, {32, 64, 96, 128}, 1);
	Check_Pixel(pixels, width, 8, 6, {32, 64, 96, 128}, 1);
	Check_Pixel(pixels, width, 4, 12, {32, 64, 96, 128}, 1);
	Check_Pixel(pixels, width, 31, 23, {32, 64, 96, 128}, 1);

	BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls, 2u);
	BOOST_REQUIRE(device.Destroy_Texture(depth));
	BOOST_REQUIRE(device.Destroy_Texture(target));
}

BOOST_AUTO_TEST_CASE(bindless_draw_retains_updates_across_descriptor_page_stress)
{
	GraphicsTestDevice device(Beam_Device_Options());
	BOOST_REQUIRE(device.Is_Valid());

	const auto target = device.Create_Texture({StressWidth, StressHeight, 1,
		RHITextureFormat::RGBA8_UNorm, Usage(RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({StressWidth, StressHeight, 1,
		RHITextureFormat::D32_Float, Usage(RHITextureUsage::DepthStencil)});
	const auto pipeline = Create_Beam_Pipeline(device, true);
	BOOST_REQUIRE(target.Is_Valid());
	BOOST_REQUIRE(depth.Is_Valid());
	BOOST_REQUIRE(pipeline.Is_Valid());

	const std::array<RHITextureHandle, 3> first_textures{{
		Create_Solid_Texture(device, {std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255}}),
		Create_Solid_Texture(device, {std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255}}),
		Create_Solid_Texture(device, {std::byte{0}, std::byte{0}, std::byte{255}, std::byte{255}})}};
	const std::array<std::array<unsigned, 4>, 3> first_expected{{
		{{255, 0, 0, 255}}, {{0, 255, 0, 255}}, {{0, 0, 255, 255}}}};
	const std::array<std::array<unsigned, 4>, 3> second_expected{{
		{{255, 255, 0, 128}}, {{255, 0, 255, 128}}, {{0, 255, 255, 128}}}};
	const std::array<std::array<unsigned, 4>, 3> discard_expected{{
		{{255, 255, 0, 64}}, {{255, 0, 255, 64}}, {{0, 255, 255, 64}}}};
	std::array<RHITextureHandle, 3> second_textures{};
	for (const auto texture : first_textures)
		BOOST_REQUIRE(texture.Is_Valid());

	const std::array<float, 4> initial_color{{1, 1, 1, 1}};
	const std::array<float, 4> preserve_color{{1, 1, 1, 0.5f}};
	const std::array<float, 4> discard_color{{1, 1, 1, 0.25f}};
	auto make_vertices = [&](const std::array<float, 4> &color) {
		return std::array<BeamVertex, 3>{{
			{{-1.0f, -1.0f, 0.5f}, {color[0], color[1], color[2], color[3]}, {0, 0}, 127},
			{{-1.0f, 3.0f, 0.5f}, {color[0], color[1], color[2], color[3]}, {0, 0}, 127},
			{{3.0f, -1.0f, 0.5f}, {color[0], color[1], color[2], color[3]}, {0, 0}, 127}}};
	};
	const auto initial_vertices = make_vertices(initial_color);
	const auto discard_vertices = make_vertices(discard_color);
	const auto preserve_buffer = device.Create_Buffer_Initialized(
		{static_cast<std::uint32_t>(sizeof(initial_vertices)), RHIBufferUsage::Vertex,
			static_cast<std::uint32_t>(sizeof(BeamVertex)), RHIBufferUpdateMode::Preserve},
		std::as_bytes(std::span<const BeamVertex>(initial_vertices)));
	const auto discard_buffer = device.Create_Buffer_Initialized(
		{static_cast<std::uint32_t>(sizeof(initial_vertices)), RHIBufferUsage::Vertex,
			static_cast<std::uint32_t>(sizeof(BeamVertex)), RHIBufferUpdateMode::Discard},
		std::as_bytes(std::span<const BeamVertex>(initial_vertices)));
	const std::array<float, 4> material_a{{1, 0, 0, 1}};
	const std::array<float, 4> material_b{{0, 1, 0, 1}};
	const auto material_buffer_a = device.Create_Buffer_Initialized(
		{sizeof(material_a), RHIBufferUsage::Constant, 16},
		std::as_bytes(std::span<const float>(material_a)));
	const auto material_buffer_b = device.Create_Buffer_Initialized(
		{sizeof(material_b), RHIBufferUsage::Constant, 16},
		std::as_bytes(std::span<const float>(material_b)));
	BOOST_REQUIRE(preserve_buffer.Is_Valid());
	BOOST_REQUIRE(discard_buffer.Is_Valid());
	BOOST_REQUIRE(material_buffer_a.Is_Valid());
	BOOST_REQUIRE(material_buffer_b.Is_Valid());

	CommandList &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, StressWidth, StressHeight, 0.0f, 1.0f}));
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1.0f));
	BOOST_REQUIRE(commands.Bind_Pipeline(pipeline));

	const auto submit_block = [&](RHIBufferHandle vertex_buffer,
		std::uint32_t block, const std::array<RHITextureHandle, 3> &textures,
		RHIBufferHandle material_a_handle, RHIBufferHandle material_b_handle) {
		BOOST_REQUIRE(commands.Set_Vertex_Buffer(0, vertex_buffer, sizeof(BeamVertex), 0));
		for (std::uint32_t local = 0; local < StressBlockSize; ++local) {
			const std::uint32_t tile = block * StressBlockSize + local;
			const std::uint32_t x = tile % StressWidth;
			const std::uint32_t y = tile / StressWidth;
			BOOST_REQUIRE(commands.Set_Scissor({x, y, 1, 1}));
			std::array<RHIBindlessResource, 2> bindings{};
			Set_Texture_Binding(bindings[0], 127, textures[local % textures.size()]);
			Set_Material_Binding(bindings[1], (local & 1u) == 0 ? material_a_handle : material_b_handle);
			BOOST_REQUIRE(commands.Set_Bindless_Resources(bindings));
			BOOST_REQUIRE(commands.Draw(3));
		}
	};

	// Four blocks submit 3072 one-pixel draws. Every alternating texture and
	// material binding changes the descriptor table, crossing 128 table pages.
	submit_block(preserve_buffer, 0, first_textures, material_buffer_a, material_buffer_b);
	submit_block(discard_buffer, 1, first_textures, material_buffer_b, material_buffer_a);

	for (const auto texture : first_textures)
		BOOST_REQUIRE(device.Destroy_Texture(texture));
	second_textures = {{
		Create_Solid_Texture(device, {std::byte{255}, std::byte{255}, std::byte{0}, std::byte{255}}),
		Create_Solid_Texture(device, {std::byte{255}, std::byte{0}, std::byte{255}, std::byte{255}}),
		Create_Solid_Texture(device, {std::byte{0}, std::byte{255}, std::byte{255}, std::byte{255}})}};
	for (const auto texture : second_textures)
		BOOST_REQUIRE(texture.Is_Valid());
	BOOST_REQUIRE(commands.Set_Vertex_Buffer(0, preserve_buffer, sizeof(BeamVertex), 0));
	for (std::uint32_t vertex = 0; vertex < 3; ++vertex)
		BOOST_REQUIRE(device.Update_Buffer(preserve_buffer,
			vertex * sizeof(BeamVertex) + offsetof(BeamVertex, color),
			std::as_bytes(std::span<const float>(preserve_color))));
	BOOST_REQUIRE(device.Update_Buffer(discard_buffer, 0,
		std::as_bytes(std::span<const BeamVertex>(discard_vertices))));

	submit_block(preserve_buffer, 2, second_textures, material_buffer_a, material_buffer_b);
	submit_block(discard_buffer, 3, second_textures, material_buffer_b, material_buffer_a);

	std::vector<std::byte> pixels(static_cast<std::size_t>(StressWidth) * StressHeight * 4);
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, StressWidth * 4));
	for (std::uint32_t tile = 0; tile < StressDrawCount; ++tile) {
		const std::uint32_t x = tile % StressWidth;
		const std::uint32_t y = tile / StressWidth;
		const std::uint32_t texture = tile % 3;
		if (tile < StressBlockSize * 2)
			Check_Pixel(pixels, StressWidth, x, y, first_expected[texture]);
		else if (tile < StressBlockSize * 3)
			Check_Pixel(pixels, StressWidth, x, y, second_expected[texture], 1);
		else
			Check_Pixel(pixels, StressWidth, x, y, discard_expected[texture], 1);
	}
	Check_Pixel(pixels, StressWidth, 63, 63, {0, 0, 0, 0});
	BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls, StressDrawCount);

	for (const auto texture : second_textures)
		BOOST_REQUIRE(device.Destroy_Texture(texture));
	BOOST_REQUIRE(device.Destroy_Buffer(material_buffer_a));
	BOOST_REQUIRE(device.Destroy_Buffer(material_buffer_b));
	BOOST_REQUIRE(device.Destroy_Buffer(preserve_buffer));
	BOOST_REQUIRE(device.Destroy_Buffer(discard_buffer));
	BOOST_REQUIRE(device.Destroy_Pipeline(pipeline));
	BOOST_REQUIRE(device.Destroy_Texture(depth));
	BOOST_REQUIRE(device.Destroy_Texture(target));
}
