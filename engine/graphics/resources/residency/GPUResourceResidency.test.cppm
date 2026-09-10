module;

#define BOOST_TEST_MODULE GraphicsGPUResourceResidencyTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.Resources.Residency.GPUResourceResidency.Tests;

import Graphics.Resources.Residency.GPUResourceResidency;

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

	RHIBufferHandle Create_Buffer(const RHIBuffer &) override
	{
		++buffer_create_count;
		return RHIBufferHandle(next_buffer_index++, 1);
	}

	RHITextureHandle Create_Texture(const RHITexture &) override
	{
		++texture_create_count;
		return RHITextureHandle(next_texture_index++, 1);
	}

	RHIPipelineHandle Create_Pipeline(const RHIPipeline &) override { return RHIPipelineHandle(1, 1); }

	RHIBufferHandle Create_Buffer_Initialized(const RHIBuffer &description, std::span<const std::byte> data) override
	{
		if (data.empty() || data.size() != description.byte_size)
			return {};
		++buffer_initialized_count;
		return Create_Buffer(description);
	}

	RHITextureHandle Create_Texture_Initialized(const RHITexture &description, const RHITextureUpload &data) override
	{
		if (data.data.empty())
			return {};
		++texture_initialized_count;
		return Create_Texture(description);
	}

	bool Update_Buffer(RHIBufferHandle buffer, std::uint32_t, std::span<const std::byte> data) noexcept override
	{
		if (!buffer.Is_Valid() || data.empty())
			return false;
		++buffer_update_count;
		return true;
	}

	bool Update_Texture(RHITextureHandle texture, const RHITextureUpload &data) noexcept override
	{
		if (!texture.Is_Valid() || data.data.empty())
			return false;
		++texture_update_count;
		return true;
	}

	bool Destroy_Buffer(RHIBufferHandle buffer) noexcept override
	{
		if (!buffer.Is_Valid())
			return false;
		++buffer_destroy_count;
		return true;
	}

	bool Destroy_Texture(RHITextureHandle texture) noexcept override
	{
		if (!texture.Is_Valid())
			return false;
		++texture_destroy_count;
		return true;
	}

	bool Destroy_Pipeline(RHIPipelineHandle) noexcept override { return true; }
	CommandList &Immediate_Command_List() noexcept override { return command_list; }
	SwapChain &Get_Swap_Chain() noexcept override { return swap_chain; }
	bool Begin_Frame() noexcept override { return true; }
	bool End_Frame() noexcept override { return true; }

	std::uint32_t buffer_create_count = 0;
	std::uint32_t buffer_initialized_count = 0;
	std::uint32_t buffer_update_count = 0;
	std::uint32_t buffer_destroy_count = 0;
	std::uint32_t texture_initialized_count = 0;
	std::uint32_t texture_create_count = 0;
	std::uint32_t texture_update_count = 0;
	std::uint32_t texture_destroy_count = 0;

private:
	std::uint32_t next_buffer_index = 1;
	std::uint32_t next_texture_index = 1;
	TestCommandList command_list;
	TestSwapChain swap_chain;
};

BOOST_AUTO_TEST_CASE(mesh_texture_and_material_uploads_are_mapped_and_incremental)
{
	std::array<std::byte, 36> vertex_data{};
	std::array<std::byte, 6> index_data{};
	std::array<std::byte, 16> texture_data{};
	Mesh mesh{3, 3, 12, MeshIndexFormat::UInt16};
	mesh.vertex_data = vertex_data;
	mesh.index_data = index_data;
	Texture texture{2, 2, 1, 1, TextureFormat::RGBA8_UNorm, TextureUsage::Sampled};
	texture.pixel_data = texture_data;
	texture.row_pitch = 8;
	Material material;
	const MeshHandle mesh_handle(4, 7);
	const TextureHandle texture_handle(6, 3);
	const MaterialHandle material_handle(8, 2);
	material.textures[0] = texture_handle;
	material.parameters.values[0] = 0.5f;

	TestDevice device;
	GPUResourceResidency residency(device);
	BOOST_REQUIRE(residency.Upload_Mesh(mesh_handle, mesh));
	BOOST_REQUIRE(residency.Upload_Texture(texture_handle, texture));
	BOOST_REQUIRE(residency.Upload_Material(material_handle, material));

	const GPUResidentMesh uploaded_mesh = residency.Mesh_Info(mesh_handle);
	const GPUResidentTexture uploaded_texture = residency.Texture_Info(texture_handle);
	const GPUResidentMaterial uploaded_material = residency.Material_Info(material_handle);
	BOOST_CHECK_EQUAL(uploaded_mesh.gpu_index, 4);
	BOOST_CHECK(uploaded_mesh.vertex_buffer.Is_Valid());
	BOOST_CHECK(uploaded_mesh.index_buffer.Is_Valid());
	BOOST_CHECK_EQUAL(uploaded_mesh.vertex_byte_size, 36);
	BOOST_CHECK_EQUAL(uploaded_mesh.index_byte_size, 6);
	BOOST_CHECK_EQUAL(uploaded_texture.gpu_index, 6);
	BOOST_CHECK(uploaded_texture.texture.Is_Valid());
	BOOST_CHECK_EQUAL(uploaded_material.gpu_index, 8);
	BOOST_CHECK(uploaded_material.constants.Is_Valid());
	BOOST_CHECK_EQUAL(uploaded_material.texture_indices[0], 6);
	BOOST_CHECK(uploaded_material.textures[0] == uploaded_texture.texture);
	BOOST_CHECK_EQUAL(device.buffer_initialized_count, 3);
	BOOST_CHECK_EQUAL(device.texture_initialized_count, 1);
	BOOST_CHECK_EQUAL(device.buffer_update_count, 0);
	BOOST_CHECK_EQUAL(device.texture_update_count, 0);

	BOOST_REQUIRE(residency.Upload_Mesh(mesh_handle, mesh));
	BOOST_REQUIRE(residency.Upload_Texture(texture_handle, texture));
	BOOST_REQUIRE(residency.Upload_Material(material_handle, material));
	BOOST_CHECK_EQUAL(device.buffer_initialized_count, 3);
	BOOST_CHECK_EQUAL(device.texture_initialized_count, 1);
	BOOST_CHECK_EQUAL(device.buffer_update_count, 0);
	BOOST_CHECK_EQUAL(device.texture_update_count, 0);

	mesh.Mark_Dirty();
	texture.Mark_Dirty();
	material.Mark_Dirty();
	BOOST_REQUIRE(residency.Upload_Mesh(mesh_handle, mesh));
	BOOST_REQUIRE(residency.Upload_Texture(texture_handle, texture));
	BOOST_REQUIRE(residency.Upload_Material(material_handle, material));
	BOOST_CHECK_EQUAL(device.buffer_update_count, 3);
	BOOST_CHECK_EQUAL(device.texture_update_count, 1);

	BOOST_REQUIRE(residency.Destroy_Texture(texture_handle));
	BOOST_CHECK(!residency.Material_Info(material_handle).constants.Is_Valid());
}

BOOST_AUTO_TEST_CASE(destroyed_handles_are_invalid_and_slots_reuse_gpu_indices)
{
	std::array<std::byte, 12> vertex_data{};
	std::array<std::byte, 2> index_data{};
	Mesh mesh{1, 1, 12, MeshIndexFormat::UInt16};
	mesh.vertex_data = vertex_data;
	mesh.index_data = index_data;
	TestDevice device;
	GPUResourceResidency residency(device);
	const MeshHandle first(2, 1);
	const MeshHandle replacement(2, 2);

	BOOST_REQUIRE(residency.Upload_Mesh(first, mesh));
	const GPUResidentMesh first_resource = residency.Mesh_Info(first);
	BOOST_REQUIRE(residency.Destroy_Mesh(first));
	BOOST_CHECK(residency.Mesh_Index(first) == Invalid_GPU_Resource_Index);
	BOOST_CHECK(!residency.Mesh_Info(first).vertex_buffer.Is_Valid());
	BOOST_CHECK(!residency.Upload_Mesh(first, mesh));

	mesh.Mark_Dirty();
	BOOST_REQUIRE(residency.Upload_Mesh(replacement, mesh));
	const GPUResidentMesh replacement_resource = residency.Mesh_Info(replacement);
	BOOST_CHECK_EQUAL(replacement_resource.gpu_index, first_resource.gpu_index);
	BOOST_CHECK(replacement_resource.vertex_buffer != first_resource.vertex_buffer);
	BOOST_CHECK(residency.Mesh_Index(first) == Invalid_GPU_Resource_Index);
}

BOOST_AUTO_TEST_CASE(material_upload_requires_resident_texture)
{
	Material material;
	material.textures[0] = TextureHandle(3, 1);
	TestDevice device;
	GPUResourceResidency residency(device);

	BOOST_CHECK(!residency.Upload_Material(MaterialHandle(1, 1), material));
	BOOST_CHECK(!residency.Material_Info(MaterialHandle(1, 1)).constants.Is_Valid());
}
