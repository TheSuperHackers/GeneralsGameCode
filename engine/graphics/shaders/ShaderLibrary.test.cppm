module;

#define BOOST_TEST_MODULE GraphicsShaderLibraryTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>

export module Graphics.Shaders.Library.Tests;

import Graphics.Shaders.Library;

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
	RHIBackbuffer Backbuffer() const noexcept override { return {}; }
	RHIDepthTarget Depth_Target() const noexcept override { return {}; }
	bool Resize(std::uint32_t, std::uint32_t) override { return true; }
	bool Present() noexcept override { return true; }
};

class TestDevice final : public Device
{
public:
	bool Is_Valid() const noexcept override { return true; }
	RHIBufferHandle Create_Buffer(const RHIBuffer &) override { return {}; }
	RHITextureHandle Create_Texture(const RHITexture &) override { return {}; }
	RHIPipelineHandle Create_Pipeline(const RHIPipeline &) override { return {}; }
	RHIPipelineHandle Create_Pipeline(const RHIPipeline &, RHIShaderBytecode vertex_shader, RHIShaderBytecode fragment_shader) override
	{
		vertex_bytecode_size = vertex_shader.data.size();
		fragment_bytecode_size = fragment_shader.data.size();
		return RHIPipelineHandle(1, 1);
	}
	bool Destroy_Buffer(RHIBufferHandle) noexcept override { return true; }
	bool Destroy_Texture(RHITextureHandle) noexcept override { return true; }
	bool Destroy_Pipeline(RHIPipelineHandle) noexcept override { return true; }
	CommandList &Immediate_Command_List() noexcept override { return command_list; }
	SwapChain &Get_Swap_Chain() noexcept override { return swap_chain; }
	bool Begin_Frame() noexcept override { return true; }
	bool End_Frame() noexcept override { return true; }

	std::size_t vertex_bytecode_size = 0;
	std::size_t fragment_bytecode_size = 0;

private:
	TestCommandList command_list;
	TestSwapChain swap_chain;
};

static bool Write_Test_Binary(const std::filesystem::path &path)
{
	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	if (!file)
		return false;

	file.write("shader", 6);
	return static_cast<bool>(file);
}

static ShaderPrecompiledDesc Make_Test_Description(const std::filesystem::path &vertex_path, const std::filesystem::path &fragment_path, std::uint16_t vertex_shader, std::uint16_t fragment_shader, std::uint64_t source_key)
{
	ShaderPrecompiledDesc description;
	description.program = Make_Basic_Opaque_Description();
	description.program.vertex_shader = vertex_shader;
	description.program.fragment_shader = fragment_shader;
	description.program.source_key = source_key;
	description.vertex_path = vertex_path;
	description.fragment_path = fragment_path;
	return description;
}

BOOST_AUTO_TEST_CASE(shader_handles_are_created_and_program_descriptors_are_stable)
{
	std::error_code error;
	const std::filesystem::path directory = std::filesystem::temp_directory_path(error) / "graphics_shader_library_handle_test";
	BOOST_REQUIRE(!error);
	std::filesystem::create_directories(directory, error);
	BOOST_REQUIRE(!error);
	const std::filesystem::path vertex_path = directory / "basic_opaque.vso";
	const std::filesystem::path fragment_path = directory / "basic_opaque.pso";
	BOOST_REQUIRE(Write_Test_Binary(vertex_path));
	BOOST_REQUIRE(Write_Test_Binary(fragment_path));

	ShaderLibrary library;
	const ShaderPrecompiledDesc description = Make_Test_Description(vertex_path, fragment_path, 1, 1, 100);
	const ShaderHandle shader = library.Load_Precompiled(description);
	BOOST_REQUIRE(shader.Is_Valid());
	BOOST_CHECK(library.Is_Loaded(shader));
	BOOST_CHECK(library.Description(shader).source_key == 100);
	BOOST_CHECK(library.Interface(shader).Key() == description.program.interface_layout.Key());
	BOOST_CHECK(!library.Load_Precompiled(description).Is_Valid());

	BOOST_CHECK(library.Destroy(shader));
	BOOST_CHECK(!library.Is_Loaded(shader));

	std::filesystem::remove(vertex_path, error);
	std::filesystem::remove(fragment_path, error);
	std::filesystem::remove(directory, error);
}

BOOST_AUTO_TEST_CASE(materials_select_default_and_custom_shaders)
{
	std::error_code error;
	const std::filesystem::path directory = std::filesystem::temp_directory_path(error) / "graphics_shader_library_selection_test";
	BOOST_REQUIRE(!error);
	std::filesystem::create_directories(directory, error);
	BOOST_REQUIRE(!error);
	const std::filesystem::path vertex_path = directory / "shader.vso";
	const std::filesystem::path fragment_path = directory / "shader.pso";
	BOOST_REQUIRE(Write_Test_Binary(vertex_path));
	BOOST_REQUIRE(Write_Test_Binary(fragment_path));

	ShaderLibrary library;
	const ShaderHandle default_shader = library.Load_Precompiled(Make_Test_Description(vertex_path, fragment_path, 1, 1, 101));
	const ShaderHandle custom_shader = library.Load_Precompiled(Make_Test_Description(vertex_path, fragment_path, 2, 3, 102));
	BOOST_REQUIRE(default_shader.Is_Valid());
	BOOST_REQUIRE(custom_shader.Is_Valid());

	Material default_material;
	BOOST_CHECK(library.Select_Shader(default_material, default_shader) == default_shader);

	Material custom_material;
	custom_material.shader = custom_shader;
	BOOST_CHECK(library.Select_Shader(custom_material, default_shader) == custom_shader);

	std::filesystem::remove(vertex_path, error);
	std::filesystem::remove(fragment_path, error);
	std::filesystem::remove(directory, error);
}

BOOST_AUTO_TEST_CASE(shader_resolution_creates_a_pipeline_from_precompiled_bytecode)
{
	std::error_code error;
	const std::filesystem::path directory = std::filesystem::temp_directory_path(error) / "graphics_shader_library_pipeline_test";
	BOOST_REQUIRE(!error);
	std::filesystem::create_directories(directory, error);
	BOOST_REQUIRE(!error);
	const std::filesystem::path vertex_path = directory / "basic_opaque.vso";
	const std::filesystem::path fragment_path = directory / "basic_opaque.pso";
	BOOST_REQUIRE(Write_Test_Binary(vertex_path));
	BOOST_REQUIRE(Write_Test_Binary(fragment_path));

	ShaderLibrary library;
	const ShaderHandle shader = library.Load_Precompiled(Make_Test_Description(vertex_path, fragment_path, 4, 5, 103));
	BOOST_REQUIRE(shader.Is_Valid());

	const PipelineDesc state = Make_Basic_Opaque_Pipeline();
	const PipelineDesc resolved_description = library.Make_Pipeline_Description(shader, state);
	BOOST_CHECK(resolved_description.vertex_shader == 4);
	BOOST_CHECK(resolved_description.fragment_shader == 5);
	BOOST_CHECK(resolved_description.parameter_layout_key == Make_Basic_Opaque_Interface().Key());

	TestDevice device;
	Material material;
	material.shader = shader;
	const PipelineHandle pipeline = library.Create_Pipeline(device, material, {}, state);
	BOOST_CHECK(pipeline.Is_Valid());
	BOOST_CHECK(device.vertex_bytecode_size == 6);
	BOOST_CHECK(device.fragment_bytecode_size == 6);

	std::filesystem::remove(vertex_path, error);
	std::filesystem::remove(fragment_path, error);
	std::filesystem::remove(directory, error);
}
