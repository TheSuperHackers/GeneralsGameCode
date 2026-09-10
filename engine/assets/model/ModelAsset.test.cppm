module;

#define BOOST_TEST_MODULE GeneralsAssetsModelTests

#include <boost/test/included/unit_test.hpp>

#include <utility>

export module Assets.Tests.ModelAsset;

import Assets.Identity;
import Assets.Models;
import Assets.Materials;

BOOST_AUTO_TEST_CASE(model_runtime_is_immutable_and_copies_generic_description)
{
	Assets::ModelAssetDesc description;
	description.name = "tank";
	description.source_format = "test";
	description.bounds = {{0.0f, 0.0f, 0.0f}, {2.0f, 3.0f, 4.0f}};
	description.vertices.push_back({{1.0f, 2.0f, 3.0f}});
	description.vertices[0].tangent = {1, 0, 0};
	description.vertices[0].tangent_sign = -1;
	description.indices = {0, 0, 0};
	description.submeshes.push_back({0, 3, 0, "body"});
	description.materials.push_back({"paint", "Textures\\Paint.TGA"});
	description.materials[0].ambient_color = {0.2f, 0.3f, 0.4f, 1};
	description.materials[0].specular_color = {0.3f, 0.4f, 0.5f, 1};
	description.materials[0].emissive_color = {0.4f, 0.5f, 0.6f, 1};
	description.dependencies.push_back({Assets::AssetType::Texture, "Textures\\Paint.TGA"});

	const Assets::ModelAsset runtime(
		{Assets::AssetType::Model, "models/tank.w3d"},
		std::move(description));

	BOOST_CHECK(runtime.Name() == "tank");
	BOOST_CHECK(runtime.Source_Format() == "test");
	BOOST_CHECK(runtime.Vertices().size() == 1);
	BOOST_TEST(runtime.Vertices()[0].tangent.x == 1.0f);
	BOOST_TEST(runtime.Vertices()[0].tangent_sign == -1.0f);
	BOOST_CHECK(runtime.Indices().size() == 3);
	BOOST_CHECK(runtime.Submeshes()[0].material_index == 0);
	BOOST_CHECK(runtime.Materials()[0].primary_texture.canonical_name == "textures/paint.tga");
	BOOST_CHECK_EQUAL(runtime.Materials()[0].ambient_color.g, 0.3f);
	BOOST_CHECK_EQUAL(runtime.Materials()[0].specular_color.b, 0.5f);
	BOOST_CHECK_EQUAL(runtime.Materials()[0].emissive_color.r, 0.4f);
	BOOST_CHECK(runtime.Dependencies()[0].identity.canonical_name == "textures/paint.tga");
	BOOST_CHECK(runtime.Bounds().Is_Valid());
}

BOOST_AUTO_TEST_CASE(model_runtime_keeps_surface_texture_identity_and_shading_parameters)
{
	Assets::ModelAssetDesc description;
	Assets::ModelMaterialDesc material;
	material.name = "metal";
	material.surface.shading_model = Assets::MaterialShadingModel::MetallicRoughness;
	material.surface.roughness = 0.3f;
	const auto role = static_cast<std::size_t>(Assets::MaterialTextureRole::Normal);
	material.surface_textures[role] = "Textures\\NORMAL.DDS";
	description.materials.push_back(material);
	const Assets::ModelAsset model({Assets::AssetType::Model,"test"}, description);
	BOOST_REQUIRE_EQUAL(model.Materials().size(), 1u);
	BOOST_TEST(model.Materials()[0].surface.roughness == 0.3f);
	BOOST_CHECK(model.Materials()[0].surface.shading_model == Assets::MaterialShadingModel::MetallicRoughness);
	BOOST_TEST(model.Materials()[0].surface_textures[role].canonical_name == "textures/normal.dds");
	BOOST_CHECK(model.Materials()[0].surface_textures[role].type == Assets::AssetType::Texture);
}
