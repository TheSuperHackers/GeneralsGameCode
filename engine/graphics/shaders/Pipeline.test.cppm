module;

#define BOOST_TEST_MODULE GraphicsPipelineTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

export module Graphics.Shaders.Pipeline.Tests;

import Graphics.Shaders.Pipeline;

using namespace Graphics;

class TestCommandList final : public CommandList
{
public:
	bool Bind_Pipeline(RHIPipelineHandle) noexcept override
	{
		return true;
	}

	bool Set_Bindless_Resources(std::span<const RHIBindlessResource>) noexcept override
	{
		return true;
	}

	bool Set_Render_Targets(RHITextureHandle, RHITextureHandle) noexcept override
	{
		return true;
	}

	bool Set_Depth_Target(RHITextureHandle) noexcept override
	{
		return true;
	}

	bool Clear(const std::array<float, 4> &, float) noexcept override
	{
		return true;
	}

	bool Clear_Depth(float) noexcept override
	{
		return true;
	}

	bool Set_Viewport(RHIViewport) noexcept override
	{
		return true;
	}

	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle, std::uint32_t, std::uint32_t) noexcept override
	{
		return true;
	}

	bool Set_Index_Buffer(RHIBufferHandle, RHIIndexFormat, std::uint32_t) noexcept override
	{
		return true;
	}

	bool Draw(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept override
	{
		return true;
	}

	bool Draw_Indexed(std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t, std::uint32_t) noexcept override
	{
		return true;
	}
};

class TestSwapChain final : public SwapChain
{
public:
	bool Is_Valid() const noexcept override
	{
		return true;
	}

	RHIBackbuffer Backbuffer() const noexcept override
	{
		return {};
	}

	RHIDepthTarget Depth_Target() const noexcept override
	{
		return {};
	}

	bool Resize(std::uint32_t, std::uint32_t) override
	{
		return true;
	}

	bool Present() noexcept override
	{
		return true;
	}
};

class TestDevice final : public Device
{
public:
	bool Is_Valid() const noexcept override
	{
		return true;
	}

	RHIBufferHandle Create_Buffer(const RHIBuffer &) override
	{
		return {};
	}

	RHITextureHandle Create_Texture(const RHITexture &) override
	{
		return {};
	}

	RHIPipelineHandle Create_Pipeline(const RHIPipeline &) override
	{
		if (fail_creation)
			return {};

		return RHIPipelineHandle(++create_count, 1);
	}

	bool Destroy_Buffer(RHIBufferHandle) noexcept override
	{
		return true;
	}

	bool Destroy_Texture(RHITextureHandle) noexcept override
	{
		return true;
	}

	bool Destroy_Pipeline(RHIPipelineHandle) noexcept override
	{
		++destroy_count;
		return true;
	}

	CommandList &Immediate_Command_List() noexcept override
	{
		return command_list;
	}

	SwapChain &Get_Swap_Chain() noexcept override
	{
		return swap_chain;
	}

	bool Begin_Frame() noexcept override
	{
		return true;
	}

	bool End_Frame() noexcept override
	{
		return true;
	}

	bool fail_creation = false;
	std::uint32_t create_count = 0;
	std::uint32_t destroy_count = 0;

private:
	TestCommandList command_list;
	TestSwapChain swap_chain;
};

static PipelineDesc Make_Description(std::uint16_t vertex_shader, std::uint16_t fragment_shader) noexcept
{
	PipelineDesc description;
	description.vertex_shader = vertex_shader;
	description.fragment_shader = fragment_shader;
	return description;
}

BOOST_AUTO_TEST_CASE(pipeline_keys_are_deterministic)
{
	const PipelineDesc first = Make_Description(3, 7);
	const PipelineDesc same = Make_Description(3, 7);
	PipelineDesc different = first;
	different.depth_write = !different.depth_write;

	BOOST_CHECK(first.Key() == same.Key());
	BOOST_CHECK(first.Key() != different.Key());
	BOOST_CHECK(PipelineKey::From(first) == first.Key());
}

BOOST_AUTO_TEST_CASE(pipeline_cache_precreates_known_pipelines_and_hits_without_creation)
{
	TestDevice device;
	const PipelineDesc fallback = Make_Description(1, 1);
	const PipelineDesc opaque = Make_Description(1, 2);
	PipelineDesc depth_only = opaque;
	depth_only.depth_write = false;
	const std::array<PipelineDesc, 2> known = {opaque, depth_only};

	PipelineCache cache;
	BOOST_REQUIRE(cache.Initialize(device, fallback, known));
	BOOST_CHECK(cache.Size() == 3);
	BOOST_CHECK(cache.Fallback().Is_Valid());

	const PipelineHandle opaque_handle = cache.Resolve(opaque.Key());
	BOOST_CHECK(opaque_handle.Is_Valid());
	BOOST_CHECK(opaque_handle != cache.Fallback());

	const std::uint32_t creation_count = device.create_count;
	BOOST_REQUIRE(cache.Precreate(device, known));
	BOOST_CHECK(device.create_count == creation_count);

	const PipelineDesc missing = Make_Description(8, 9);
	BOOST_CHECK(cache.Resolve(missing.Key()) == cache.Fallback());
	BOOST_CHECK(device.create_count == creation_count);

	BOOST_REQUIRE(cache.Shutdown(device));
	BOOST_CHECK(device.destroy_count == 3);
}

BOOST_AUTO_TEST_CASE(pipeline_handles_are_strongly_typed)
{
	static_assert(!std::is_convertible_v<PipelineHandle, MeshHandle>);
	static_assert(!std::is_convertible_v<PipelineHandle, TextureHandle>);

	const PipelineHandle handle(4, 2);
	BOOST_CHECK(handle.Is_Valid());
	BOOST_CHECK(handle.Get_Index() == 4);
	BOOST_CHECK(handle.Get_Generation() == 2);
}

BOOST_AUTO_TEST_CASE(pipeline_cache_failure_does_not_create_a_fallback)
{
	TestDevice device;
	device.fail_creation = true;
	PipelineCache cache;

	BOOST_CHECK(!cache.Initialize(device, Make_Description(1, 1)));
	BOOST_CHECK(!cache.Fallback().Is_Valid());
	BOOST_CHECK(cache.Size() == 0);
}
