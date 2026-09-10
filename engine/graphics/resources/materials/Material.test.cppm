module;

#define BOOST_TEST_MODULE GraphicsMaterialTests

#include <boost/test/included/unit_test.hpp>

#include <type_traits>

export module Graphics.Resources.Materials.Material.Tests;

import Graphics.Resources.Materials.Material;

using namespace Graphics;

static_assert(std::is_nothrow_move_constructible_v<MaterialParameterBlock>);
static_assert(std::is_nothrow_move_assignable_v<MaterialParameterBlock>);
static_assert(std::is_nothrow_move_constructible_v<Material>);
static_assert(std::is_nothrow_move_assignable_v<Material>);

BOOST_AUTO_TEST_CASE(material_references_resources_and_constants)
{
	const TextureHandle texture(3, 1);
	const SamplerHandle sampler(5, 1);
	const ShaderHandle shader(7, 1);
	Material material;

	material.shader = shader;
	material.textures[0] = texture;
	material.samplers[0] = sampler;
	material.parameters.values[0] = 0.5f;
	material.parameters.values[1] = 1.0f;
	material.flags = MaterialFlags::Unlit | MaterialFlags::DoubleSided;

	BOOST_CHECK(material.textures[0] == texture);
	BOOST_CHECK(material.samplers[0] == sampler);
	BOOST_CHECK(material.shader == shader);
	BOOST_CHECK(material.parameters.values[0] == 0.5f);
	BOOST_CHECK(material.parameters.values[1] == 1.0f);
	BOOST_CHECK(Has_Material_Flag(material.flags, MaterialFlags::Unlit));
	BOOST_CHECK(Has_Material_Flag(material.flags, MaterialFlags::DoubleSided));
}

BOOST_AUTO_TEST_CASE(material_pool_uses_material_handles)
{
	MaterialPool pool;
	const MaterialHandle handle = pool.Create();

	BOOST_REQUIRE(pool.Resolve(handle) != nullptr);
	BOOST_CHECK(pool.Resolve(handle)->textures[0] == nullptr);
	BOOST_CHECK(pool.Resolve(handle)->samplers[0] == nullptr);
}
