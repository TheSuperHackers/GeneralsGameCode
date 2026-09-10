module;

#define BOOST_TEST_MODULE GeneralsAssetsTextureTests

#include <boost/test/included/unit_test.hpp>

export module Assets.Tests.TextureAsset;

import Assets.Identity;
import Assets.Textures;

BOOST_AUTO_TEST_CASE(texture_runtime_data_is_immutable_and_identity_based)
{
	const Assets::TextureAsset texture(
		{Assets::AssetType::Texture, "textures/paint.tga"},
		"TGA",
		128);

	BOOST_CHECK(texture.Identity().canonical_name == "textures/paint.tga");
	BOOST_CHECK(texture.Source_Format() == "TGA");
	BOOST_CHECK(texture.Source_Size() == 128);
}
