module;

#define BOOST_TEST_MODULE GraphicsModelAssetBindingTests

#include <boost/test/included/unit_test.hpp>

#include <utility>
#include <vector>

export module Graphics.Scene.Models.ModelAssetBinding.Tests;

import Assets.Identity;
import Assets.Models;
import Graphics.Scene.Models.ModelAssetBinding;

BOOST_AUTO_TEST_CASE(model_asset_binding_rejects_uninitialized_renderer)
{
	Assets::ModelAssetDesc description;
	description.name = "test_model";
	description.bounds = {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
	description.vertices = {{{0.0f, 0.0f, 0.0f}}};
	description.indices = {0, 0, 0};
	description.submeshes = {{0, 3, 0, "body"}};
	description.materials = {{"body"}};
	const Assets::ModelAsset asset({Assets::AssetType::Model, "test_model"}, std::move(description));

	Graphics::StaticMeshRenderer &renderer = Graphics::GetStaticMeshRenderer();
	Graphics::StaticMeshBinding binding;
	std::vector<Graphics::MaterialHandle> materials;
	BOOST_TEST(!Graphics::Create_Model_Asset_Binding(
		renderer, asset, {}, Graphics::RenderInstanceFlags::None,
		Graphics::All_Submeshes_Visible, binding, materials));
}
