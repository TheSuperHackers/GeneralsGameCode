module;

#define BOOST_TEST_MODULE GeneralsAssetsImporterTests

#include <boost/test/included/unit_test.hpp>

export module Assets.Tests.ModelImporter;

import Assets.Importers.Models;

BOOST_AUTO_TEST_CASE(model_import_result_reports_uninitialized_imports_as_failed)
{
	const Assets::ModelImportResult result;
	BOOST_CHECK(!result.Succeeded());
}
