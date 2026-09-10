module;

#define BOOST_TEST_MODULE GraphicsRenderer2DTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <vector>

export module Graphics.Renderer2D.Tests;

import Graphics.Renderer2D;

using namespace Graphics;

namespace
{

class RecordingCommandList final : public CommandList
{
public:
	bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept override
	{
		return pipeline.Is_Valid();
	}

	bool Set_Bindless_Resources(std::span<const RHIBindlessResource> resources) noexcept override
	{
		bindless_count = resources.size();
		return true;
	}

	bool Set_Render_Targets(RHITextureHandle color, RHITextureHandle depth) noexcept override
	{
		return color.Is_Valid() && depth.Is_Valid();
	}

	bool Set_Depth_Target(RHITextureHandle target) noexcept override
	{
		return target.Is_Valid();
	}

	bool Clear(const std::array<float, 4> &, float) noexcept override { return true; }
	bool Clear_Depth(float) noexcept override { return true; }

	bool Set_Viewport(RHIViewport viewport) noexcept override
	{
		return viewport.width != 0 && viewport.height != 0;
	}

	bool Set_Scissor(RHIScissorRect scissor) noexcept override
	{
		scissors.push_back(scissor);
		return scissor.width != 0 && scissor.height != 0;
	}

	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t) noexcept override
	{
		return buffer.Is_Valid() && stride == sizeof(Renderer2DVertex);
	}

	bool Set_Index_Buffer(RHIBufferHandle buffer, RHIIndexFormat, std::uint32_t) noexcept override
	{
		return buffer.Is_Valid();
	}

	bool Draw(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept override { return false; }

	bool Draw_Indexed(std::uint32_t index_count, std::uint32_t first_index, std::int32_t, std::uint32_t, std::uint32_t) noexcept override
	{
		draws.push_back({index_count, first_index});
		return index_count != 0;
	}

	struct DrawRecord final
	{
		std::uint32_t index_count = 0;
		std::uint32_t first_index = 0;
	};

	std::size_t bindless_count = 0;
	std::vector<RHIScissorRect> scissors;
	std::vector<DrawRecord> draws;
};

class TestSwapChain final : public SwapChain
{
public:
	bool Is_Valid() const noexcept override { return true; }
	RHIBackbuffer Backbuffer() const noexcept override { return {}; }
	RHIDepthTarget Depth_Target() const noexcept override { return {}; }
	bool Resize(std::uint32_t, std::uint32_t) override { return true; }
	bool Present() noexcept override { return true; }
};

class TestDevice final : public Device
{
public:
	bool Is_Valid() const noexcept override { return true; }

	RHIBufferHandle Create_Buffer(const RHIBuffer &) override
	{
		return RHIBufferHandle(++next_buffer, 1);
	}

	RHITextureHandle Create_Texture(const RHITexture &) override
	{
		return RHITextureHandle(++next_texture, 1);
	}

	RHIPipelineHandle Create_Pipeline(const RHIPipeline &) override
	{
		return RHIPipelineHandle(++next_pipeline, 1);
	}

	RHIPipelineHandle Create_Pipeline(const RHIPipeline &, RHIShaderBytecode vertex_shader, RHIShaderBytecode fragment_shader) override
	{
		return vertex_shader.data.empty() || fragment_shader.data.empty() ? RHIPipelineHandle{} : RHIPipelineHandle(++next_pipeline, 1);
	}

	RHITextureHandle Create_Texture_Initialized(const RHITexture &, const RHITextureUpload &upload) override
	{
		return upload.data.empty() ? RHITextureHandle{} : RHITextureHandle(++next_texture, 1);
	}

	bool Update_Buffer(RHIBufferHandle buffer, std::uint32_t, std::span<const std::byte> data) noexcept override
	{
		return buffer.Is_Valid() && !data.empty();
	}

	bool Destroy_Buffer(RHIBufferHandle) noexcept override { return true; }
	bool Destroy_Texture(RHITextureHandle) noexcept override { return true; }
	bool Destroy_Pipeline(RHIPipelineHandle) noexcept override { return true; }
	CommandList &Immediate_Command_List() noexcept override { return command_list; }
	SwapChain &Get_Swap_Chain() noexcept override { return swap_chain; }
	bool Begin_Frame() noexcept override { return true; }
	bool End_Frame() noexcept override { return true; }

	RecordingCommandList command_list;

private:
	std::uint32_t next_buffer = 0;
	std::uint32_t next_texture = 100;
	std::uint32_t next_pipeline = 200;
	TestSwapChain swap_chain;
};

static bool Write_Shader_Stubs(const std::filesystem::path &directory)
{
	std::error_code error;
	std::filesystem::create_directories(directory, error);
	if (error)
		return false;

	for (const char *name : {"ui_2d.vso", "ui_2d.pso"}) {
		std::ofstream file(directory / name, std::ios::binary | std::ios::trunc);
		if (!file)
			return false;
		file.write("ui2d", 4);
		if (!file)
			return false;
	}
	return true;
}

}

BOOST_AUTO_TEST_CASE(records_contiguous_ordered_batches_with_scissors)
{
	std::error_code error;
	const std::filesystem::path shader_directory = std::filesystem::temp_directory_path(error) / "generals_renderer2d_test";
	BOOST_REQUIRE(!error);
	BOOST_REQUIRE(Write_Shader_Stubs(shader_directory));

	TestDevice device;
	Renderer2D renderer;
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory, 64, 96, 8));

	renderer.Begin(320, 200);
	BOOST_REQUIRE(renderer.Add_Rect({0, 0, 32, 24}, {1, 0, 0, 1}));
	BOOST_REQUIRE(renderer.Add_Rect({32, 0, 64, 24}, {0, 1, 0, 1}));
	renderer.Set_Clip(true, {10, 20, 110, 80});
	BOOST_REQUIRE(renderer.Add_Outline({10, 20, 100, 60}, 1, {0, 0, 1, 1}));
	renderer.Set_Clip(false, {});
	BOOST_REQUIRE(renderer.Add_Line({0, 100}, {80, 100}, 2, Color2D{1, 1, 1, 1}));
	BOOST_REQUIRE(renderer.Add_Quad(Rect2D{80, 100, 120, 140}, {0, 0, 1, 1}, renderer.White_Texture(), Color2D{1, 1, 1, 1}));

	BOOST_CHECK_EQUAL(renderer.Vertex_Count(), 32);
	BOOST_CHECK_EQUAL(renderer.Index_Count(), 48);
	BOOST_CHECK_EQUAL(renderer.Batch_Count(), 3);

	BOOST_REQUIRE(renderer.Execute(device, device.command_list, RHITextureHandle(900, 1), RHITextureHandle(901, 1), {0, 0, 320, 200, 0.0f, 1.0f}));
	BOOST_CHECK_EQUAL(device.command_list.bindless_count, 1);
	BOOST_REQUIRE_EQUAL(device.command_list.draws.size(), 3);
	BOOST_CHECK(device.command_list.draws[0].index_count == 12 && device.command_list.draws[0].first_index == 0);
	BOOST_CHECK(device.command_list.draws[1].index_count == 24 && device.command_list.draws[1].first_index == 12);
	BOOST_CHECK(device.command_list.draws[2].index_count == 12 && device.command_list.draws[2].first_index == 36);
	BOOST_REQUIRE_EQUAL(device.command_list.scissors.size(), 3);
	BOOST_CHECK(device.command_list.scissors[0].x == 0 && device.command_list.scissors[0].y == 0
		&& device.command_list.scissors[0].width == 320 && device.command_list.scissors[0].height == 200);
	BOOST_CHECK(device.command_list.scissors[1].x == 10 && device.command_list.scissors[1].y == 20
		&& device.command_list.scissors[1].width == 100 && device.command_list.scissors[1].height == 60);
	BOOST_CHECK(device.command_list.scissors[2].x == 0 && device.command_list.scissors[2].y == 0
		&& device.command_list.scissors[2].width == 320 && device.command_list.scissors[2].height == 200);

	renderer.Shutdown();
}
