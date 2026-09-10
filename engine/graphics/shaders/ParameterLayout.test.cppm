module;

#define BOOST_TEST_MODULE GraphicsParameterLayoutTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

export module Graphics.Shaders.ParameterLayout.Tests;

import Graphics.Scene.DrawGeneration;
import Graphics.Shaders.ParameterLayout;

using namespace Graphics;

static_assert(std::is_trivially_copyable_v<ShaderFloat4>);
static_assert(std::is_trivially_copyable_v<ShaderMatrix4x4>);
static_assert(std::is_nothrow_move_constructible_v<ShaderParameterBlockData>);
static_assert(std::is_nothrow_move_assignable_v<ShaderParameterBlockData>);

BOOST_AUTO_TEST_CASE(parameter_layout_uses_deterministic_hlsl_offsets)
{
	ShaderParameterLayout layout(ShaderParameterBlock::View);
	const ShaderParameterLocation position = layout.Add_Constant(ShaderValueType::Float3);
	const ShaderParameterLocation padding = layout.Add_Constant(ShaderValueType::Float32);
	const ShaderParameterLocation scale = layout.Add_Constant(ShaderValueType::Float2);
	const ShaderParameterLocation transform = layout.Add_Constant(ShaderValueType::Matrix4x4);

	BOOST_CHECK(position.Is_Valid());
	BOOST_CHECK(position.constant_offset == 0);
	BOOST_CHECK(position.constant_size == 12);
	BOOST_CHECK(padding.constant_offset == 12);
	BOOST_CHECK(scale.constant_offset == 16);
	BOOST_CHECK(transform.constant_offset == 32);
	BOOST_CHECK(layout.Constant_Byte_Size() == 96);

	ShaderParameterLayout same_layout(ShaderParameterBlock::View);
	same_layout.Add_Constant(ShaderValueType::Float3);
	same_layout.Add_Constant(ShaderValueType::Float32);
	same_layout.Add_Constant(ShaderValueType::Float2);
	same_layout.Add_Constant(ShaderValueType::Matrix4x4);
	BOOST_CHECK(layout.Key() == same_layout.Key());
	BOOST_CHECK(layout.Is_Compatible(same_layout));
}

BOOST_AUTO_TEST_CASE(parameter_layout_resolves_locations_without_names)
{
	ShaderParameterLayout layout(ShaderParameterBlock::Material);
	const ShaderParameterLocation color = layout.Add_Constant(ShaderValueType::Float4);
	const ShaderParameterLocation texture = layout.Add_Bindless_Resource(ShaderResourceClass::Texture);
	const ShaderParameterLocation sampler = layout.Add_Bindless_Resource(ShaderResourceClass::Sampler);
	const ShaderParameterLocation buffer = layout.Add_Buffer();

	BOOST_REQUIRE(layout.Is_Compatible(color));
	BOOST_REQUIRE(layout.Is_Compatible(texture));
	BOOST_REQUIRE(layout.Is_Compatible(sampler));
	BOOST_REQUIRE(layout.Is_Compatible(buffer));
	BOOST_CHECK(color.parameter_index == 0);
	BOOST_CHECK(texture.resource_offset == 0);
	BOOST_CHECK(sampler.resource_offset == 1);
	BOOST_CHECK(buffer.resource_offset == 2);
	BOOST_CHECK(layout.Resource_Count() == 3);

	ShaderParameterLocation wrong = texture;
	wrong.resource_class = ShaderResourceClass::Material;
	BOOST_CHECK(!layout.Is_Compatible(wrong));
}

BOOST_AUTO_TEST_CASE(parameter_block_matches_hlsl_storage_and_propagates_indices)
{
	ShaderParameterLayout layout(ShaderParameterBlock::Material);
	const ShaderParameterLocation constants = layout.Add_Constant(ShaderValueType::Float4);
	const ShaderParameterLocation texture = layout.Add_Bindless_Resource(ShaderResourceClass::Texture);
	const ShaderParameterLocation material = layout.Add_Bindless_Resource(ShaderResourceClass::Material);
	ShaderParameterBlockData data;
	data.Clear();

	const ShaderFloat4 color{{0.25f, 0.5f, 0.75f, 1.0f}};
	BOOST_REQUIRE(data.Set_Constant(constants, color));
	std::array<float, 4> stored_color{};
	std::memcpy(stored_color.data(), data.Constant_Bytes(layout.Constant_Byte_Size()).data(), sizeof(stored_color));
	BOOST_CHECK(stored_color == color.values);

	const ResourceIndex texture_index(17, 3);
	BOOST_REQUIRE(data.Set_Resource_Index(texture, texture_index));
	BOOST_REQUIRE(data.Set_Resource_Index(material, 23));
	BOOST_CHECK(data.Resource_Index(texture) == texture_index.Get_Index());
	BOOST_CHECK(data.Resource_Index(material) == 23);
	BOOST_CHECK(data.Resource_Indices()[texture.resource_offset] == 17);
	BOOST_CHECK(data.Resource_Indices()[material.resource_offset] == 23);
}

BOOST_AUTO_TEST_CASE(parameter_layout_integrates_pipeline_and_draw_indices)
{
	ShaderInterfaceLayout interface_layout;
	interface_layout.frame.Add_Constant(ShaderValueType::Float4);
	interface_layout.view.Add_Constant(ShaderValueType::Matrix4x4);
	interface_layout.material.Add_Bindless_Resource(ShaderResourceClass::Texture);
	interface_layout.draw.Add_Bindless_Resource(ShaderResourceClass::Material);

	PipelineDesc pipeline;
	pipeline.Set_Parameter_Layout(interface_layout);
	const PipelineKey key = pipeline.Key();
	PipelineDesc same_pipeline = pipeline;
	BOOST_CHECK(same_pipeline.Key() == key);

	ShaderInterfaceLayout different_layout = interface_layout;
	different_layout.draw.Add_Bindless_Resource(ShaderResourceClass::Buffer);
	PipelineDesc different_pipeline = pipeline;
	different_pipeline.Set_Parameter_Layout(different_layout);
	BOOST_CHECK(different_pipeline.Key() != key);

	const DrawData draw{4, 23, 17, 1, PipelineHandle(8, 1), 0};
	ShaderParameterBlockData draw_parameters;
	draw_parameters.Clear();
	const ShaderParameterLocation material_index = interface_layout.draw.Parameters()[0];
	BOOST_REQUIRE(draw_parameters.Set_Resource_Index(material_index, draw.material_index));
	BOOST_CHECK(draw_parameters.Resource_Index(material_index) == draw.material_index);
}
