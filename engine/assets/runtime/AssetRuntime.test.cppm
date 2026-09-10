module;

#define BOOST_TEST_MODULE AssetsRuntimeTests

#include <boost/test/included/unit_test.hpp>

export module Assets.Runtime.Tests;

import Assets.Importers.Models;
import Assets.Runtime;

BOOST_AUTO_TEST_CASE(runtime_lifecycle_is_idempotent)
{
	Assets::Shutdown_Asset_Runtime();
	BOOST_TEST(Assets::Try_Get_Asset_Cache() == nullptr);
	BOOST_REQUIRE(Assets::Initialize_Asset_Runtime(Assets::AssetSource{}, {}));
	BOOST_TEST(Assets::Try_Get_Asset_Cache() != nullptr);
	BOOST_REQUIRE(Assets::Initialize_Asset_Runtime(Assets::AssetSource{}, {}));
	Assets::Shutdown_Asset_Runtime();
	BOOST_TEST(Assets::Try_Get_Asset_Cache() == nullptr);
}
