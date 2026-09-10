module;

#define BOOST_TEST_MODULE GraphicsModelAssetGeometryTests

#include <boost/test/included/unit_test.hpp>

#include <utility>

export module Graphics.Scene.Models.ModelAssetGeometry.Tests;

import Assets.Models;
import Graphics.Scene.Models.ModelAssetGeometry;

BOOST_AUTO_TEST_CASE(model_asset_geometry_translates_generic_data)
{
	Assets::ModelAssetDesc description;
	description.name = "test_model";
	description.bounds = {{-1.0f, -2.0f, -3.0f}, {1.0f, 2.0f, 3.0f}};
	description.vertices = {
		{{-1.0f, 0.0f, 0.0f}, {}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{1.0f, 0.0f, 0.0f}, {}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{0.0f, 1.0f, 0.0f}, {}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}};
	description.indices = {0, 1, 2};
	description.submeshes = {{0, 3, 0, "body"}};
	description.materials = {{"body"}};
	const Assets::ModelAsset asset({Assets::AssetType::Model, "test_model"}, std::move(description));

	Graphics::ModelAssetGeometry geometry;
	BOOST_REQUIRE(Graphics::Build_Model_Asset_Geometry(asset, geometry));
	BOOST_TEST(geometry.source.vertex_count == 3u);
	BOOST_TEST(geometry.source.index_count == 3u);
	BOOST_TEST(geometry.parts.size() == 1u);
	BOOST_TEST(geometry.bounds.radius == 3.0f);
	BOOST_TEST(geometry.vertices[1].position[0] == 1.0f);
}

BOOST_AUTO_TEST_CASE(model_asset_geometry_translates_skinned_data)
{
	Assets::ModelAssetDesc description;
	description.bounds = {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
	description.skin_bone_count = 2;
	description.vertices = {{{0.0f, 0.0f, 0.0f}, {}, {}, {}, {1, 0, 0, 0}, {1.0f, 0.0f, 0.0f, 0.0f}}};
	description.indices = {0, 0, 0};
	description.submeshes = {{0, 3, 0, "body"}};
	description.materials = {{"body"}};
	const Assets::ModelAsset asset({Assets::AssetType::Model, "skinned"}, std::move(description));

	Graphics::ModelAssetGeometry geometry;
	BOOST_REQUIRE(Graphics::Build_Model_Asset_Geometry(asset, geometry));
	BOOST_TEST(geometry.skinned_vertices.size() == 1u);
	BOOST_TEST(static_cast<unsigned>(geometry.source.vertex_format) == static_cast<unsigned>(Graphics::MeshVertexFormat::Position3Color4UV2Skinned));
	BOOST_TEST(geometry.source.skin_bone_count == 2u);
	BOOST_TEST(geometry.skinned_vertices[0].skinning.bone_indices[0] == 1u);
}
