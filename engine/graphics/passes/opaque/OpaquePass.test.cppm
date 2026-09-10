module;

#define BOOST_TEST_MODULE GraphicsOpaquePassTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <cstring>
#include <span>

export module Graphics.Passes.Opaque.Tests;

#ifndef GRAPHICS_TEST_SHADER_DIRECTORY
#define GRAPHICS_TEST_SHADER_DIRECTORY "."
#endif

import Graphics.Passes.Opaque;

#if defined(_WIN32)
import Graphics.Tests.Device;

using namespace Graphics;
#endif

class RecordingOpaqueCommandList final : public CommandList
{
public:
	bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept override
	{
		++pipeline_bind_count;
		pipeline_bound = pipeline.Is_Valid();
		return pipeline_bound;
	}

	bool Set_Bindless_Resources(std::span<const RHIBindlessResource> resources) noexcept override
	{
		++bindless_set_count;
		bindless_resources_set = !resources.empty();
		return true;
	}

	bool Set_Render_Targets(RHITextureHandle color_target, RHITextureHandle depth_target) noexcept override
	{
		targets_bound = color_target.Is_Valid() && depth_target.Is_Valid();
		return targets_bound;
	}

	bool Set_Depth_Target(RHITextureHandle depth_target) noexcept override
	{
		return depth_target.Is_Valid();
	}

	bool Clear(const std::array<float, 4> &, float) noexcept override
	{
		cleared = true;
		return true;
	}

	bool Clear_Depth(float) noexcept override
	{
		return true;
	}

	bool Set_Viewport(RHIViewport viewport) noexcept override
	{
		viewport_set = viewport.width != 0 && viewport.height != 0;
		return viewport_set;
	}

	bool Set_Draw_Constants(std::span<const std::byte> data) noexcept override
	{
		if (data.size() < sizeof(std::uint32_t) || draw_constants_set_count >= draw_indices.size())
			return false;
		std::memcpy(&draw_indices[draw_constants_set_count], data.data(), sizeof(std::uint32_t));
		++draw_constants_set_count;
		return true;
	}

	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t) noexcept override
	{
		vertex_buffer_set = buffer.Is_Valid() && stride != 0;
		return vertex_buffer_set;
	}

	bool Set_Index_Buffer(RHIBufferHandle buffer, RHIIndexFormat, std::uint32_t) noexcept override
	{
		index_buffer_set = buffer.Is_Valid();
		return index_buffer_set;
	}

	bool Draw(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept override
	{
		return false;
	}

	bool Draw_Indexed(std::uint32_t index_count, std::uint32_t, std::int32_t, std::uint32_t instance_count, std::uint32_t) noexcept override
	{
		++draw_count;
		return index_count != 0 && instance_count != 0;
	}

	bool pipeline_bound = false;
	bool targets_bound = false;
	bool cleared = false;
	bool viewport_set = false;
	bool vertex_buffer_set = false;
	bool index_buffer_set = false;
	bool bindless_resources_set = false;
	std::uint32_t bindless_set_count = 0;
	std::uint32_t pipeline_bind_count = 0;
	std::uint32_t draw_count = 0;
	std::uint32_t draw_constants_set_count = 0;
	std::array<std::uint32_t, 4> draw_indices{};
};

BOOST_AUTO_TEST_CASE(opaque_pass_records_the_complete_draw_path)
{
	RenderGraph graph;
	const GraphResourceHandle color_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle pass = OpaquePass::Add_To_Graph(graph, color_target, depth_target, 9);
	BOOST_REQUIRE(pass != nullptr);

	const std::array<GraphResourceBinding, 2> resources = {
		GraphResourceBinding::Texture(color_target, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth_target, RHITextureHandle(2, 1))
	};
	ExecutionPlan plan;
	BOOST_REQUIRE(plan.Compile(graph, resources));

	const std::array<DrawData, 2> draws = {
		DrawData{0, 2, 4, 1, PipelineHandle(8, 1), 0, 0, 17},
		DrawData{0, 2, 5, 1, PipelineHandle(8, 1), 0, 0, 23}
	};
	const std::array<OpaqueMeshBinding, 1> meshes = {
		OpaqueMeshBinding{RHIBufferHandle(3, 1), RHIBufferHandle(4, 1), RHIIndexFormat::UInt16, 36, 3, 0, 0}
	};
	const std::array<RHIBindlessResource, 3> bindless_resources = {
		RHIBindlessResource{ResourceIndex(0, 1), RHIResourceType::Buffer, RHIBufferHandle(7, 1), {}},
		RHIBindlessResource{ResourceIndex(1, 1), RHIResourceType::Texture, {}, RHITextureHandle(6, 1)},
		RHIBindlessResource{ResourceIndex(2, 1), RHIResourceType::Material, RHIBufferHandle(5, 1), {}}
	};
	OpaquePassInput input{
		draws,
		meshes,
		bindless_resources,
		color_target,
		depth_target,
		{0, 0, 1280, 720, 0.0f, 1.0f},
		{0.1f, 0.2f, 0.3f, 1.0f},
		1.0f
	};
	input.uses_gpu_draw_table = true;

	RecordingOpaqueCommandList command_list;
	BOOST_REQUIRE(plan.Execute(graph, command_list, [&](GraphPassHandle current_pass, CommandList &commands, const PassResources &pass_resources) noexcept {
		return current_pass == pass && OpaquePass::Execute(commands, pass_resources, input);
	}));
	BOOST_CHECK(command_list.targets_bound);
	BOOST_CHECK(command_list.cleared);
	BOOST_CHECK(command_list.viewport_set);
	BOOST_CHECK(command_list.pipeline_bound);
	BOOST_CHECK(command_list.vertex_buffer_set);
	BOOST_CHECK(command_list.index_buffer_set);
	BOOST_CHECK(command_list.bindless_resources_set);
	BOOST_CHECK(command_list.bindless_set_count == 1);
	BOOST_CHECK(command_list.pipeline_bind_count == 1);
	BOOST_CHECK(command_list.draw_count == 2);
	BOOST_CHECK(command_list.draw_constants_set_count == 2);
	BOOST_CHECK(command_list.draw_indices[0] == 17);
	BOOST_CHECK(command_list.draw_indices[1] == 23);
}

BOOST_AUTO_TEST_CASE(opaque_pass_rejects_undeclared_targets_and_invalid_draw_records)
{
	RenderGraph graph;
	const GraphResourceHandle color_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle pass = OpaquePass::Add_To_Graph(graph, color_target, depth_target);
	const std::array<GraphResourceBinding, 2> resources = {
		GraphResourceBinding::Texture(color_target, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth_target, RHITextureHandle(2, 1))
	};
	ExecutionPlan plan;
	BOOST_REQUIRE(plan.Compile(graph, resources));

	const std::array<DrawData, 1> draws = {DrawData{4, 2, 0, 1, PipelineHandle(8, 1), 0}};
	const std::array<OpaqueMeshBinding, 1> meshes = {
		OpaqueMeshBinding{RHIBufferHandle(3, 1), RHIBufferHandle(4, 1), RHIIndexFormat::UInt16, 36, 3, 0, 0}
	};
	const std::array<RHIBindlessResource, 3> bindless_resources = {
		RHIBindlessResource{ResourceIndex(0, 1), RHIResourceType::Buffer, RHIBufferHandle(7, 1), {}},
		RHIBindlessResource{ResourceIndex(1, 1), RHIResourceType::Texture, {}, RHITextureHandle(6, 1)},
		RHIBindlessResource{ResourceIndex(2, 1), RHIResourceType::Material, RHIBufferHandle(5, 1), {}}
	};
	const OpaquePassInput input{
		draws,
		meshes,
		bindless_resources,
		color_target,
		depth_target,
		{0, 0, 1280, 720, 0.0f, 1.0f},
		{},
		1.0f
	};

	RecordingOpaqueCommandList command_list;
	BOOST_CHECK(!plan.Execute(graph, command_list, [&](GraphPassHandle, CommandList &commands, const PassResources &pass_resources) noexcept {
		return OpaquePass::Execute(commands, pass_resources, input);
	}));
}

#if defined(_WIN32)
BOOST_AUTO_TEST_CASE(opaque_pass_executes_through_dx11)
{
	GraphicsTestDeviceOptions options;
	options.use_warp = true;
	options.shader_directory = GRAPHICS_TEST_SHADER_DIRECTORY;
	GraphicsTestDevice device(options);
	BOOST_REQUIRE(device.Is_Valid());

	const RHIBufferHandle vertex_buffer = device.Create_Buffer({108, RHIBufferUsage::Vertex, 36});
	const RHIBufferHandle index_buffer = device.Create_Buffer({6, RHIBufferUsage::Index, 0});
	const RHIBufferHandle constants = device.Create_Buffer({16, RHIBufferUsage::Constant, 16});
	const RHIBufferHandle instances = device.Create_Buffer({96, RHIBufferUsage::Storage, 96});
	const RHITextureHandle material_texture = device.Create_Texture({1, 1, 1, RHITextureFormat::RGBA8_UNorm, static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)});
	const RHITextureHandle color_target = device.Create_Texture({16, 16, 1, RHITextureFormat::RGBA8_UNorm, static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
	const RHITextureHandle depth_target = device.Create_Texture({16, 16, 1, RHITextureFormat::D24_UNorm_S8, static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
	const RHIPipelineHandle pipeline = device.Create_Pipeline({9});

	BOOST_REQUIRE(vertex_buffer != nullptr);
	BOOST_REQUIRE(index_buffer != nullptr);
	BOOST_REQUIRE(constants != nullptr);
	BOOST_REQUIRE(instances != nullptr);
	BOOST_REQUIRE(material_texture != nullptr);
	BOOST_REQUIRE(color_target != nullptr);
	BOOST_REQUIRE(depth_target != nullptr);
	BOOST_REQUIRE(pipeline != nullptr);

	RenderGraph graph;
	const GraphResourceHandle color_resource = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth_resource = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle pass = OpaquePass::Add_To_Graph(graph, color_resource, depth_resource);
	const std::array<GraphResourceBinding, 2> resources = {
		GraphResourceBinding::Texture(color_resource, color_target),
		GraphResourceBinding::Texture(depth_resource, depth_target)
	};
	ExecutionPlan plan;
	BOOST_REQUIRE(pass != nullptr);
	BOOST_REQUIRE(plan.Compile(graph, resources));

	const std::array<DrawData, 1> draws = {DrawData{0, 2, 0, 1, pipeline, 0}};
	const std::array<OpaqueMeshBinding, 1> meshes = {
		OpaqueMeshBinding{vertex_buffer, index_buffer, RHIIndexFormat::UInt16, 36, 3, 0, 0}
	};
	const std::array<RHIBindlessResource, 3> bindless_resources = {
		RHIBindlessResource{ResourceIndex(0, 1), RHIResourceType::Buffer, instances, {}},
		RHIBindlessResource{ResourceIndex(1, 1), RHIResourceType::Texture, {}, material_texture},
		RHIBindlessResource{ResourceIndex(2, 1), RHIResourceType::Material, constants, {}}
	};
	const OpaquePassInput input{
		draws,
		meshes,
		bindless_resources,
		color_resource,
		depth_resource,
		{0, 0, 16, 16, 0.0f, 1.0f},
		{0.1f, 0.2f, 0.3f, 1.0f},
		1.0f
	};

	BOOST_REQUIRE(plan.Execute(graph, device.Immediate_Command_List(), [&](GraphPassHandle current_pass, CommandList &commands, const PassResources &pass_resources) noexcept {
		return current_pass == pass && OpaquePass::Execute(commands, pass_resources, input);
	}));

	BOOST_CHECK(device.Destroy_Pipeline(pipeline));
	BOOST_CHECK(device.Destroy_Texture(depth_target));
	BOOST_CHECK(device.Destroy_Texture(color_target));
	BOOST_CHECK(device.Destroy_Texture(material_texture));
	BOOST_CHECK(device.Destroy_Buffer(instances));
	BOOST_CHECK(device.Destroy_Buffer(constants));
	BOOST_CHECK(device.Destroy_Buffer(index_buffer));
	BOOST_CHECK(device.Destroy_Buffer(vertex_buffer));
}
#endif
