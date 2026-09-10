module;

#define BOOST_TEST_MODULE GeneralsAssetsHandleTests

#include <boost/test/included/unit_test.hpp>

#include <type_traits>

export module Assets.Tests.AssetHandle;

import Assets.Handles;

BOOST_AUTO_TEST_CASE(asset_handles_are_typed_and_generation_aware)
{
	const Assets::ModelAssetHandle invalid;
	const Assets::ModelAssetHandle first(4, 1);
	const Assets::ModelAssetHandle replacement(4, 2);

	BOOST_CHECK(!invalid.Is_Valid());
	BOOST_CHECK(first.Is_Valid());
	BOOST_CHECK(first != replacement);
	BOOST_CHECK(first.Get_Index() == replacement.Get_Index());
	BOOST_CHECK(first.Get_Generation() != replacement.Get_Generation());
	BOOST_CHECK((!std::is_constructible_v<Assets::TextureAssetHandle, Assets::ModelAssetHandle>));
}
