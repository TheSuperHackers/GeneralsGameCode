module;

#define BOOST_TEST_MODULE GraphicsTracerTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Scene.Lines.Tracer.Tests;

import Graphics.Tests.Device;
import Graphics.RHI;
import Graphics.Scene.OrderedDraws;
import Graphics.Scene.Lines.Tracer;
import Graphics.Scene.Shadows.DirectionalRenderer;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;

using namespace Graphics;

namespace
{

constexpr std::array<std::uint32_t, TracerIndexCount> ExpectedTopology{
	3, 5, 1,
	7, 5, 3,
	1, 5, 0,
	5, 4, 0,
	4, 2, 0,
	4, 6, 2,
	7, 3, 2,
	6, 7, 2,
	7, 6, 5,
	5, 6, 4,
	2, 3, 1,
	2, 1, 0};

TracerDrawData Make_Test_Draw_Data(float opacity)
{
	TracerDrawData data;
	data.view_projection = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	data.world = {1, 0, 0, -.5f, 0, 1, 0, 0, 0, 0, 1, .5f, 0, 0, 0, 1};
	data.camera_depth = {0, 0, 1, 0};
	data.opacity = opacity;
	return data;
}

bool Read_Pixel(GraphicsTestDevice &device, RHITextureHandle target, unsigned x, unsigned y,
	std::array<int, 4> &result)
{
	std::array<std::byte, 8 * 8 * 4> pixels{};
	if (!device.Readback_Texture(target, pixels, 8 * 4))
		return false;
	const std::size_t offset = (y * 8 + x) * 4;
	result = {
		std::to_integer<int>(pixels[offset]),
		std::to_integer<int>(pixels[offset + 1]),
		std::to_integer<int>(pixels[offset + 2]),
		std::to_integer<int>(pixels[offset + 3])};
	return true;
}

TracerDrawData Make_Overlapping_Draw_Data(float z, float opacity = .5f)
{
	TracerDrawData data = Make_Test_Draw_Data(opacity);
	data.world[3] = -1.0f;
	data.world[11] = z;
	return data;
}

struct OrderedTracerSource final
{
	OrderedTracerSource(PropRenderer &renderer, unsigned value, TracerDrawData draw_data,
		unsigned *deaths = nullptr)
		: owner(&renderer), value(value), data(draw_data), deaths(deaths)
	{
	}

	~OrderedTracerSource()
	{
		tracer.Release(*owner);
	}

	void Add_Ref() noexcept
	{
		++references;
	}

	void Release_Ref() noexcept
	{
		if (--references == 0) {
			if (deaths != nullptr)
				++*deaths;
			delete this;
		}
	}

	PropRenderer *owner;
	TracerRenderer tracer;
	unsigned value;
	TracerDrawData data;
	unsigned *deaths;
	unsigned references = 1;
};

struct OrderedTracerContext final
{
	PropRenderer *renderer;
	PropSubmission *submission;
	CommandList *commands = nullptr;
	std::vector<unsigned> order;
};

bool Submit_Tracer(OrderedTracerSource &source, void *context)
{
	if (context == nullptr)
		return false;
	auto &drawing = *static_cast<OrderedTracerContext *>(context);
	drawing.order.push_back(source.value);
	return source.tracer.Submit(*drawing.renderer, *drawing.submission, source.data);
}

struct OrderedImmediateSource final
{
	OrderedImmediateSource(PropRenderer &renderer, unsigned value, PropMeshHandle mesh)
		: owner(&renderer), mesh(mesh), value(value)
	{
		style.blend = RHIBlendMode::Alpha;
		style.depth_write = false;
		style.depth_test = false;
		style.source_blend = RHIBlendFactor::SourceAlpha;
		style.destination_blend = RHIBlendFactor::InverseSourceAlpha;
		style.cull = RHICullMode::None;
		parameters.view_projection = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1};
		parameters.textured = 0.0f;
	}

	~OrderedImmediateSource()
	{
		Release();
	}

	void Add_Ref() noexcept
	{
		++references;
	}

	void Release_Ref() noexcept
	{
		if (--references == 0)
			delete this;
	}

	void Release() noexcept
	{
		if (mesh.Is_Valid())
			owner->Destroy_Mesh(mesh);
		mesh = {};
	}

	PropRenderer *owner;
	PropMeshHandle mesh;
	unsigned value;
	PropStyle style;
	PropParameters parameters;
	unsigned references = 1;
};

bool Draw_Ordered_Immediate(OrderedImmediateSource &source, void *context)
{
	if (context == nullptr)
		return false;
	auto &drawing = *static_cast<OrderedTracerContext *>(context);
	if (drawing.commands == nullptr)
		return false;
	drawing.order.push_back(source.value);
	return drawing.renderer->Draw(*drawing.commands, source.mesh, source.style, source.parameters, {});
}

}

BOOST_AUTO_TEST_CASE(box_geometry_preserves_volume_topology_and_vertex_color)
{
	const TracerDescription description{4.0f, .5f, {.123f, .456f, .789f, .321f}};
	std::array<PropVertex, TracerVertexCount> vertices{};
	std::array<std::uint32_t, TracerIndexCount> indices{};

	BOOST_REQUIRE_EQUAL(Build_Tracer_Geometry(description, vertices, indices), TracerVertexCount);
	BOOST_CHECK_EQUAL(vertices[0].position[0], 0.0f);
	BOOST_CHECK_EQUAL(vertices[0].position[1], -.25f);
	BOOST_CHECK_EQUAL(vertices[0].position[2], -.25f);
	BOOST_CHECK_EQUAL(vertices[4].position[0], 4.0f);
	BOOST_CHECK_EQUAL(vertices[4].position[1], -.25f);
	BOOST_CHECK_EQUAL(vertices[4].position[2], -.25f);
	for (const auto &vertex : vertices) {
		BOOST_CHECK_EQUAL(vertex.color[0], 31.0f / 255.0f);
		BOOST_CHECK_EQUAL(vertex.color[1], 116.0f / 255.0f);
		BOOST_CHECK_EQUAL(vertex.color[2], 201.0f / 255.0f);
		BOOST_CHECK_EQUAL(vertex.color[3], 82.0f / 255.0f);
		BOOST_CHECK_EQUAL(vertex.material_diffuse[3], 1.0f);
	}
	BOOST_CHECK(indices == ExpectedTopology);
}

BOOST_AUTO_TEST_CASE(invalid_geometry_is_rejected_without_partial_output)
{
	TracerDescription invalid{1.0f, 1.0f, {1.0f, 0.0f, 0.0f, 1.0f}};
	invalid.length = std::numeric_limits<float>::quiet_NaN();
	std::array<PropVertex, TracerVertexCount> vertices{};
	std::array<std::uint32_t, TracerIndexCount> indices{};
	BOOST_CHECK_EQUAL(Build_Tracer_Geometry(invalid, vertices, indices), 0);
	invalid.length = 1.0f;
	invalid.width = -1.0f;
	BOOST_CHECK_EQUAL(Build_Tracer_Geometry(invalid, vertices, indices), 0);
	BOOST_CHECK_EQUAL(Build_Tracer_Geometry(invalid,
		std::span<PropVertex>(vertices.data(), TracerVertexCount - 1), indices), 0);
}

BOOST_AUTO_TEST_CASE(replacing_a_mesh_releases_unretained_source)
{
	GraphicsTestDevice device({true});
	PropRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
	TracerRenderer tracer;
	BOOST_REQUIRE(tracer.Set_Description(renderer, {2.0f, .25f, {1, 0, 0, 1}}));
	const PropMeshHandle previous_mesh = tracer.Mesh();
	BOOST_REQUIRE(previous_mesh.Is_Valid());
	BOOST_REQUIRE(tracer.Set_Description(renderer, {3.0f, .5f, {0, 1, 0, 1}}));
	BOOST_CHECK(tracer.Mesh() != previous_mesh);
	BOOST_CHECK(renderer.Mesh_Geometry(previous_mesh) == nullptr);

	tracer.Release(renderer);
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(authored_tracer_layers_preserve_priority_fifo_and_source_lifetime)
{
	GraphicsTestDevice device({true});
	PropRenderer renderer;
	DirectionalShadowRenderer shadows;
	PropSubmission submission;
	const auto shader_directory = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	BOOST_REQUIRE(shadows.Initialize(device, shader_directory));
	submission.Initialize(device, renderer, shadows);

	const auto target = device.Create_Texture({8, 8, 1, RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({8, 8, 1, RHITextureFormat::D32_Float,
		static_cast<unsigned>(RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid());
	BOOST_REQUIRE(depth.Is_Valid());
	auto &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, 8, 8}));

	OrderedTracerSource red(renderer, 1, Make_Overlapping_Draw_Data(.25f));
	OrderedTracerSource green(renderer, 2, Make_Overlapping_Draw_Data(.45f));
	unsigned deaths = 0;
	auto *blue = new OrderedTracerSource(renderer, 3, Make_Overlapping_Draw_Data(.75f), &deaths);
	std::array<PropVertex, 3> marker_vertices{};
	marker_vertices[0].position = {-1, -1, .5f};
	marker_vertices[1].position = {3, -1, .5f};
	marker_vertices[2].position = {-1, 3, .5f};
	for (auto &vertex : marker_vertices)
		vertex.color = {1, 1, 0, .5f};
	const auto marker_mesh = renderer.Create_Mesh(marker_vertices, std::array<std::uint32_t, 3>{0, 1, 2});
	BOOST_REQUIRE(marker_mesh.Is_Valid());
	OrderedImmediateSource marker(renderer, 99, marker_mesh);
	BOOST_REQUIRE(red.tracer.Set_Description(renderer, {2.0f, .5f, {1, 0, 0, 1}}));
	BOOST_REQUIRE(green.tracer.Set_Description(renderer, {2.0f, .5f, {0, 1, 0, 1}}));
	BOOST_REQUIRE(blue->tracer.Set_Description(renderer, {2.0f, .5f, {0, 0, 1, 1}}));

	OrderedTracerContext drawing{&renderer, &submission, &commands};
	OrderedDrawQueue queue;
	queue.Set_Enabled(true);
	BOOST_REQUIRE(queue.Enqueue<Submit_Tracer>(TracerAuthoredLayer, red));
	BOOST_REQUIRE(queue.Enqueue<Draw_Ordered_Immediate>(TracerAuthoredLayer, marker));
	BOOST_REQUIRE(queue.Enqueue<Submit_Tracer>(TracerAuthoredLayer, green));
	BOOST_REQUIRE(queue.Enqueue<Submit_Tracer>(3, *blue));
	blue->Release_Ref();
	BOOST_CHECK_EQUAL(deaths, 0u);

	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
	bool layers_flushed = false;
	BOOST_REQUIRE(queue.Drain(&drawing, [&] { layers_flushed = submission.Flush_Materials(); }));
	BOOST_CHECK(layers_flushed);
	BOOST_CHECK((drawing.order == std::vector<unsigned>{3, 1, 99, 2}));
	BOOST_CHECK_EQUAL(deaths, 1u);

	std::array<int, 4> composited{};
	BOOST_REQUIRE(Read_Pixel(device, target, 4, 4, composited));
	// The authored layer 3 blue draw is first, followed by red then green in
	// FIFO order at layer 1 includes the non-tracer immediate draw. The prop
	// shader keeps RGB unmultiplied and the source-alpha blend contributes one
	// half of each pure colour.
	BOOST_CHECK_SMALL(composited[0] - 96, 2);
	BOOST_CHECK_SMALL(composited[1] - 192, 2);
	BOOST_CHECK_SMALL(composited[2] - 16, 2);
	BOOST_CHECK_SMALL(composited[3] - 120, 2);

	// Queue-disabled submission is immediate, so the draw is visible before
	// any scene flush.
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
	OrderedTracerSource direct(renderer, 4, Make_Overlapping_Draw_Data(.5f));
	BOOST_REQUIRE(direct.tracer.Set_Description(renderer, {2.0f, .5f, {1, 1, 0, 1}}));
	BOOST_REQUIRE(direct.tracer.Submit(renderer, submission, direct.data));
	std::array<int, 4> immediate{};
	BOOST_REQUIRE(Read_Pixel(device, target, 4, 4, immediate));
	BOOST_CHECK_SMALL(immediate[0] - 128, 2);
	BOOST_CHECK_SMALL(immediate[1] - 128, 2);
	BOOST_CHECK_SMALL(immediate[2], 2);
	BOOST_CHECK_SMALL(immediate[3] - 64, 2);

	direct.tracer.Release(renderer);
	red.tracer.Release(renderer);
	green.tracer.Release(renderer);
	marker.Release();
	submission.Shutdown();
	shadows.Shutdown();
	renderer.Shutdown();
	device.Destroy_Texture(target);
	device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(gpu_drawing_preserves_opaque_alpha_and_resource_recreation)
{
	GraphicsTestDevice device({true});
	PropRenderer renderer;
	const auto shader_directory = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	const auto target = device.Create_Texture({8, 8, 1, RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({8, 8, 1, RHITextureFormat::D32_Float,
		static_cast<unsigned>(RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid());
	BOOST_REQUIRE(depth.Is_Valid());
	TracerRenderer tracer;
	BOOST_REQUIRE(tracer.Set_Description(renderer, {1.0f, .5f, {.25f, .5f, .75f, 1.0f}}));
	auto &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, 8, 8}));

	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
	BOOST_REQUIRE(tracer.Draw(renderer, commands, Make_Test_Draw_Data(1.0f)));
	std::array<int, 4> opaque{};
	BOOST_REQUIRE(Read_Pixel(device, target, 4, 4, opaque));
	BOOST_CHECK_SMALL(opaque[0] - 64, 2);
	BOOST_CHECK_SMALL(opaque[1] - 128, 2);
	BOOST_CHECK_SMALL(opaque[2] - 191, 2);
	BOOST_CHECK_SMALL(opaque[3] - 255, 2);

	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
	BOOST_REQUIRE(tracer.Draw(renderer, commands, Make_Test_Draw_Data(.5f)));
	std::array<int, 4> alpha{};
	BOOST_REQUIRE(Read_Pixel(device, target, 4, 4, alpha));
	BOOST_CHECK_SMALL(alpha[0] - 32, 2);
	BOOST_CHECK_SMALL(alpha[1] - 64, 2);
	BOOST_CHECK_SMALL(alpha[2] - 96, 2);
	// The prop shader multiplies vertex alpha by opacity, then the generic
	// alpha-like blend path applies source alpha to the alpha channel again.
	// With a cleared destination this is (128 / 255)^2, which quantizes to 64.
	BOOST_CHECK_SMALL(alpha[3] - 64, 2);

	// Colour the two depth-facing sides differently so each triangle in the
	// front quad contributes a separately sampled interior pixel.
	std::array<PropVertex, TracerVertexCount> topology_vertices{};
	std::array<std::uint32_t, TracerIndexCount> topology_indices{};
	BOOST_REQUIRE_EQUAL(Build_Tracer_Geometry(
		{1.0f, .5f, {1.0f, 1.0f, 1.0f, 1.0f}}, topology_vertices, topology_indices),
		TracerVertexCount);
	for (auto &vertex : topology_vertices)
		vertex.color = vertex.position[2] < 0.0f
			? std::array<float, 4>{1.0f, 0.0f, 0.0f, 1.0f}
			: std::array<float, 4>{0.0f, 1.0f, 0.0f, 1.0f};
	const auto topology_mesh = renderer.Create_Mesh(topology_vertices, topology_indices);
	BOOST_REQUIRE(topology_mesh.Is_Valid());
	PropStyle topology_style;
	topology_style.blend = RHIBlendMode::Disabled;
	topology_style.cull = RHICullMode::None;
	PropParameters topology_parameters;
	topology_parameters.view_projection = Make_Test_Draw_Data(1.0f).view_projection;
	topology_parameters.world = Make_Test_Draw_Data(1.0f).world;
	topology_parameters.textured = 0.0f;
	topology_parameters.primary_gradient = 1.0f;
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
	BOOST_REQUIRE(renderer.Draw(commands, topology_mesh, topology_style, topology_parameters, {}));
	std::array<int, 4> topology_left{};
	std::array<int, 4> topology_right{};
	std::array<int, 4> topology_background{};
	BOOST_REQUIRE(Read_Pixel(device, target, 3, 4, topology_left));
	BOOST_REQUIRE(Read_Pixel(device, target, 5, 4, topology_right));
	BOOST_REQUIRE(Read_Pixel(device, target, 1, 4, topology_background));
	BOOST_CHECK_SMALL(topology_left[0] - 255, 2);
	BOOST_CHECK_SMALL(topology_left[1], 2);
	BOOST_CHECK_SMALL(topology_left[2], 2);
	BOOST_CHECK_SMALL(topology_left[3] - 255, 2);
	BOOST_CHECK_SMALL(topology_right[0] - 255, 2);
	BOOST_CHECK_SMALL(topology_right[1], 2);
	BOOST_CHECK_SMALL(topology_right[2], 2);
	BOOST_CHECK_SMALL(topology_right[3] - 255, 2);
	BOOST_CHECK_SMALL(topology_background[0], 2);
	BOOST_CHECK_SMALL(topology_background[1], 2);
	BOOST_CHECK_SMALL(topology_background[2], 2);
	BOOST_CHECK_SMALL(topology_background[3], 2);
	renderer.Destroy_Mesh(topology_mesh);

	renderer.Shutdown();
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	device.Destroy_Texture(target);
	device.Destroy_Texture(depth);
	const auto recreated_target = device.Create_Texture({8, 8, 1, RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(RHITextureUsage::RenderTarget)});
	const auto recreated_depth = device.Create_Texture({8, 8, 1, RHITextureFormat::D32_Float,
		static_cast<unsigned>(RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(recreated_target.Is_Valid());
	BOOST_REQUIRE(recreated_depth.Is_Valid());
	BOOST_REQUIRE(commands.Set_Render_Targets(recreated_target, recreated_depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, 8, 8}));
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
	BOOST_REQUIRE(tracer.Draw(renderer, commands, Make_Test_Draw_Data(1.0f)));
	std::array<int, 4> recreated{};
	BOOST_REQUIRE(Read_Pixel(device, recreated_target, 4, 4, recreated));
	BOOST_CHECK_SMALL(recreated[0] - 64, 2);
	BOOST_CHECK_SMALL(recreated[1] - 128, 2);
	BOOST_CHECK_SMALL(recreated[2] - 191, 2);
	BOOST_CHECK_SMALL(recreated[3] - 255, 2);

	tracer.Release(renderer);
	renderer.Shutdown();
	device.Destroy_Texture(recreated_target);
	device.Destroy_Texture(recreated_depth);
}
