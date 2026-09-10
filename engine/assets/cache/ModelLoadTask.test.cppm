module;

#define BOOST_TEST_MODULE GeneralsAssetsLoadTaskTests

#include <boost/test/included/unit_test.hpp>

export module Assets.Tests.ModelLoadTask;

import Assets.Cache.ModelLoadTask;
import Assets.Identity;

BOOST_AUTO_TEST_CASE(model_load_task_reports_missing_sources_without_throwing)
{
	const Assets::AssetIdentity identity{Assets::AssetType::Model, "missing.w3d"};
	const Assets::ModelDescriptionLoadResult result = Assets::Load_Model_Description(identity, {}, {});
	BOOST_CHECK(!result.Succeeded());
	BOOST_CHECK(!result.error.empty());
}
