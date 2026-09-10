module;

#define BOOST_TEST_MODULE GeneralsAssetsStateTests

#include <boost/test/included/unit_test.hpp>

export module Assets.Tests.AssetState;

import Assets.States;

BOOST_AUTO_TEST_CASE(asset_states_have_explicit_lifecycle_values)
{
	BOOST_CHECK(Assets::AssetState::Unloaded != Assets::AssetState::Loading);
	BOOST_CHECK(Assets::AssetState::Loading != Assets::AssetState::Ready);
	BOOST_CHECK(Assets::AssetState::Ready != Assets::AssetState::Failed);
}
