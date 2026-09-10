module;

#define BOOST_TEST_MODULE GraphicsRHITests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <span>
#include <type_traits>

export module Graphics.RHI.Tests;

import Graphics.RHI;

using namespace Graphics;

static_assert(!std::is_convertible_v<RHIBufferHandle, RHITextureHandle>);
static_assert(!std::is_convertible_v<RHITextureHandle, RHIPipelineHandle>);

class TestCommandList final : public CommandList
{
public:
	using CommandList::Record_Draw;

	bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept override
	{
		return pipeline.Is_Valid();
	}

	bool Set_Bindless_Resources(std::span<const RHIBindlessResource>) noexcept override
	{
		return true;
	}

	bool Set_Render_Targets(RHITextureHandle color_target, RHITextureHandle depth_target) noexcept override
	{
		return color_target.Is_Valid() && depth_target.Is_Valid();
	}

	bool Set_Color_Target(RHITextureHandle color_target) noexcept override
	{
		return color_target.Is_Valid();
	}

	bool Set_Depth_Target(RHITextureHandle depth_target) noexcept override
	{
		return depth_target.Is_Valid();
	}

	bool Clear(const std::array<float, 4> &, float) noexcept override
	{
		return true;
	}

	bool Clear_Depth(float) noexcept override
	{
		return true;
	}

	bool Set_Viewport(RHIViewport viewport) noexcept override
	{
		return viewport.width != 0 && viewport.height != 0;
	}

	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t) noexcept override
	{
		return buffer.Is_Valid() && stride != 0;
	}

	bool Set_Index_Buffer(RHIBufferHandle buffer, RHIIndexFormat, std::uint32_t) noexcept override
	{
		return buffer.Is_Valid();
	}

	bool Draw(std::uint32_t vertex_count, std::uint32_t, std::uint32_t instance_count, std::uint32_t) noexcept override
	{
		return vertex_count != 0 && instance_count != 0;
	}

	bool Draw_Indexed(std::uint32_t index_count, std::uint32_t, std::int32_t, std::uint32_t instance_count, std::uint32_t) noexcept override
	{
		return index_count != 0 && instance_count != 0;
	}
};

BOOST_AUTO_TEST_CASE(rhi_descriptions_and_handles_are_typed)
{
	const RHIBuffer buffer{256, RHIBufferUsage::Vertex, 32};
	const RHITexture texture{128, 64, 4, RHITextureFormat::RGBA8_UNorm, static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)};
	const RHIPipeline pipeline{42};
	const RHIBufferHandle buffer_handle(1, 2);
	const RHITextureHandle texture_handle(3, 4);
	const RHIPipelineHandle pipeline_handle(5, 6);

	BOOST_CHECK(buffer.byte_size == 256);
	BOOST_CHECK(buffer.stride == 32);
	BOOST_CHECK(texture.width == 128);
	BOOST_CHECK(texture.height == 64);
	BOOST_CHECK(texture.mip_count == 4);
	BOOST_CHECK(pipeline.key == 42);
	BOOST_CHECK(buffer_handle.Is_Valid());
	BOOST_CHECK(texture_handle.Is_Valid());
	BOOST_CHECK(pipeline_handle.Is_Valid());
}

BOOST_AUTO_TEST_CASE(command_list_contract_is_small_and_usable)
{
	TestCommandList command_list;
	const RHIBufferHandle buffer(1, 1);
	const RHITextureHandle texture(2, 1);
	const RHIPipelineHandle pipeline(3, 1);
	const std::array<RHIBindlessResource, 2> resources = {
		RHIBindlessResource{ResourceIndex(0, 1), RHIResourceType::Texture, {}, texture},
		RHIBindlessResource{ResourceIndex(1, 1), RHIResourceType::Buffer, buffer, {}}
	};

	BOOST_CHECK(command_list.Bind_Pipeline(pipeline));
	BOOST_CHECK(command_list.Set_Bindless_Resources(resources));
	BOOST_CHECK(command_list.Set_Color_Target(texture));
	BOOST_CHECK(command_list.Set_Depth_Target(texture));
	BOOST_CHECK(command_list.Clear_Depth(1.0f));
	BOOST_CHECK(command_list.Set_Vertex_Buffer(0, buffer, 32, 0));
	BOOST_CHECK(command_list.Set_Index_Buffer(buffer, RHIIndexFormat::UInt32, 0));
	BOOST_CHECK(command_list.Draw(3, 0, 1, 0));
	BOOST_CHECK(command_list.Draw_Indexed(3, 0, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(submission_arithmetic_widens_before_instance_multiplication)
{
	TestCommandList commands;
	commands.Record_Draw(RHIPrimitiveTopology::TriangleList, 3000000000u, 3u);
	BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls, 1u);
	BOOST_CHECK_EQUAL(commands.Submission_Counts().triangles, 3000000000ull);
	BOOST_CHECK_EQUAL(commands.Submission_Counts().vertex_invocations, 9000000000ull);
	commands.Record_Draw(RHIPrimitiveTopology::TriangleStrip, 3000000002u, 3u);
	BOOST_CHECK_EQUAL(commands.Submission_Counts().triangles, 12000000000ull);
	BOOST_CHECK_EQUAL(commands.Submission_Counts().vertex_invocations, 18000000006ull);
	commands.Record_Draw(RHIPrimitiveTopology::TriangleStrip, 2u, 3u);
	BOOST_CHECK_EQUAL(commands.Submission_Counts().triangles, 12000000000ull);
	BOOST_CHECK_EQUAL(commands.Submission_Counts().vertex_invocations, 18000000012ull);
	commands.Record_Draw(RHIPrimitiveTopology::PointList, 3000000000u, 3u);
	BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls, 4u);
	BOOST_CHECK_EQUAL(commands.Submission_Counts().triangles, 12000000000ull);
	BOOST_CHECK_EQUAL(commands.Submission_Counts().vertex_invocations, 27000000012ull);
}
