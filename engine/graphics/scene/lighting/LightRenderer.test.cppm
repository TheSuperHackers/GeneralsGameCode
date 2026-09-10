module;

#define BOOST_TEST_MODULE GraphicsLightRendererTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

export module Graphics.Scene.Lighting.Renderer.Tests;

import Graphics.Scene.Lighting.Renderer;

using namespace Graphics;

class TestCommandList final : public CommandList
{
public:
	bool Bind_Pipeline(RHIPipelineHandle) noexcept override { return true; }
	bool Set_Bindless_Resources(std::span<const RHIBindlessResource>) noexcept override { return true; }
	bool Set_Render_Targets(RHITextureHandle, RHITextureHandle) noexcept override { return true; }
	bool Set_Depth_Target(RHITextureHandle) noexcept override { return true; }
	bool Clear(const std::array<float, 4> &, float) noexcept override { return true; }
	bool Clear_Depth(float) noexcept override { return true; }
	bool Set_Viewport(RHIViewport) noexcept override { return true; }
	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle, std::uint32_t, std::uint32_t) noexcept override { return true; }
	bool Set_Index_Buffer(RHIBufferHandle, RHIIndexFormat, std::uint32_t) noexcept override { return true; }
	bool Draw(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept override { return true; }
	bool Draw_Indexed(std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t, std::uint32_t) noexcept override { return true; }
};

class TestSwapChain final : public SwapChain
{
public:
	bool Is_Valid() const noexcept override { return true; }
	RHIBackbuffer Backbuffer() const noexcept override { return {{1, 1}, 1, 1}; }
	RHIDepthTarget Depth_Target() const noexcept override { return {{2, 1}, 1, 1}; }
	bool Resize(std::uint32_t, std::uint32_t) override { return true; }
	bool Present() noexcept override { return true; }
};

class TestDevice final : public Device
{
public:
	bool Is_Valid() const noexcept override { return true; }

	RHIBufferHandle Create_Buffer(const RHIBuffer &description) override
	{
		buffer_size = description.byte_size;
		return RHIBufferHandle(next_buffer_index++, 1);
	}

	RHITextureHandle Create_Texture(const RHITexture &) override { return RHITextureHandle(1, 1); }
	RHIPipelineHandle Create_Pipeline(const RHIPipeline &) override { return RHIPipelineHandle(1, 1); }

	bool Update_Buffer(RHIBufferHandle buffer, std::uint32_t offset, std::span<const std::byte> data) noexcept override
	{
		if (!buffer.Is_Valid() || data.empty() || offset + data.size() > buffer_size)
			return false;
		last_update_offset = offset;
		last_update_size = data.size();
		++update_count;
		return true;
	}

	bool Destroy_Buffer(RHIBufferHandle buffer) noexcept override
	{
		if (!buffer.Is_Valid())
			return false;
		++destroy_count;
		return true;
	}

	bool Destroy_Texture(RHITextureHandle) noexcept override { return true; }
	bool Destroy_Pipeline(RHIPipelineHandle) noexcept override { return true; }
	CommandList &Immediate_Command_List() noexcept override { return command_list; }
	SwapChain &Get_Swap_Chain() noexcept override { return swap_chain; }
	bool Begin_Frame() noexcept override { return true; }
	bool End_Frame() noexcept override { return true; }

	std::size_t buffer_size = 0;
	std::size_t last_update_offset = 0;
	std::size_t last_update_size = 0;
	std::size_t update_count = 0;
	std::size_t destroy_count = 0;

private:
	std::uint32_t next_buffer_index = 1;
	TestCommandList command_list;
	TestSwapChain swap_chain;
};

static RenderLight Make_Light(float range, float intensity) noexcept
{
	RenderLight light;
	light.position = {1.0f, 2.0f, 3.0f};
	light.color = {0.25f, 0.5f, 0.75f};
	light.range = range;
	light.intensity = intensity;
	return light;
}

BOOST_AUTO_TEST_CASE(light_lifecycle_and_incremental_gpu_sync)
{
	TestDevice device;
	LightRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, 4));

	const LightHandle first = renderer.Create_Point_Light(Make_Light(12.0f, 2.0f));
	BOOST_REQUIRE(first.Is_Valid());
	BOOST_REQUIRE(renderer.Sync());
	BOOST_CHECK_EQUAL(device.update_count, 1);
	BOOST_CHECK_EQUAL(device.last_update_offset, 0);
	BOOST_CHECK_EQUAL(device.last_update_size, sizeof(GPULightData));
	BOOST_CHECK_EQUAL(renderer.Light_Count(), 1);
	BOOST_CHECK(renderer.Packed_Lights()[0].position_range[3] == 12.0f);
	BOOST_CHECK(renderer.Packed_Lights()[0].direction_intensity[3] == 2.0f);
	BOOST_CHECK(renderer.Light_Buffer_Index().Is_Valid());
	BOOST_REQUIRE_EQUAL(renderer.Bindless_Resources().size(), 1);
	BOOST_CHECK(renderer.Bindless_Resources()[0].index == renderer.Light_Buffer_Index());

	BOOST_REQUIRE(renderer.Sync());
	BOOST_CHECK_EQUAL(device.update_count, 1);

	BOOST_REQUIRE(renderer.Update_Point_Light(first, Make_Light(8.0f, 1.5f)));
	BOOST_REQUIRE(renderer.Sync());
	BOOST_CHECK_EQUAL(device.update_count, 2);
	BOOST_CHECK(renderer.Packed_Lights()[0].position_range[3] == 8.0f);
	BOOST_CHECK(renderer.Packed_Lights()[0].direction_intensity[3] == 1.5f);

	renderer.Shutdown();
	BOOST_CHECK_EQUAL(device.destroy_count, 1);
}

BOOST_AUTO_TEST_CASE(stale_light_handles_cannot_update_reused_slots)
{
	TestDevice device;
	LightRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, 2));

	const LightHandle first = renderer.Create_Point_Light(Make_Light(10.0f, 1.0f));
	BOOST_REQUIRE(renderer.Sync());
	BOOST_REQUIRE(renderer.Destroy_Point_Light(first));
	BOOST_CHECK(!renderer.Update_Point_Light(first, Make_Light(20.0f, 3.0f)));

	const LightHandle replacement = renderer.Create_Point_Light(Make_Light(20.0f, 3.0f));
	BOOST_REQUIRE(replacement.Is_Valid());
	BOOST_CHECK_EQUAL(replacement.Get_Index(), first.Get_Index());
	BOOST_CHECK(replacement.Get_Generation() != first.Get_Generation());
	BOOST_CHECK(!renderer.Update_Point_Light(first, Make_Light(30.0f, 4.0f)));
	BOOST_REQUIRE(renderer.Update_Point_Light(replacement, Make_Light(24.0f, 4.0f)));
	BOOST_REQUIRE(renderer.Sync());
	BOOST_CHECK(renderer.Packed_Lights()[0].position_range[3] == 24.0f);
	BOOST_CHECK(renderer.Packed_Lights()[0].direction_intensity[3] == 4.0f);
}

BOOST_AUTO_TEST_CASE(disabled_and_faded_light_data_reaches_the_scene)
{
	LightRenderer renderer;
	RenderLight light = Make_Light(16.0f, 2.0f);
	const LightHandle handle = renderer.Create_Point_Light(light);
	BOOST_REQUIRE(handle.Is_Valid());

	light.range = 8.0f;
	light.intensity = 1.0f;
	light.color = {0.125f, 0.25f, 0.375f};
	BOOST_REQUIRE(renderer.Update_Point_Light(handle, light));
	BOOST_CHECK(renderer.Scene().Lights().ranges[0] == 8.0f);
	BOOST_CHECK(renderer.Scene().Lights().intensities[0] == 1.0f);
	BOOST_CHECK(renderer.Scene().Lights().color_r[0] == 0.125f);

	light.flags = RenderLightFlags::None;
	BOOST_REQUIRE(renderer.Update_Point_Light(handle, light));
	BOOST_CHECK(!Has_Render_Light_Flag(renderer.Scene().Lights().flags[0], RenderLightFlags::Enabled));
}
