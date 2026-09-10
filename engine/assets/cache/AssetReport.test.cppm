module;

#define BOOST_TEST_MODULE GeneralsAssetsAssetReportTests

#include <boost/test/included/unit_test.hpp>
#include <string>

export module Assets.Tests.AssetReport;

import Assets.Cache.AssetReport;

using Assets::AssetReport;
using Assets::AssetReportCategory;

BOOST_AUTO_TEST_CASE(asset_report_defaults_keep_missing_records_and_gate_load_on_demand_records)
{
	AssetReport report;

	BOOST_CHECK(report.Reporting_Enabled());
	BOOST_CHECK(!report.Load_On_Demand_Reporting_Enabled());

	report.Record_Load_On_Demand(AssetReportCategory::Model, "Tank.W3D");
	report.Record_Missing(AssetReportCategory::Model, "Tank.W3D");
	report.Record_Missing(AssetReportCategory::Model, "tank.w3d");

	BOOST_TEST(report.Load_On_Demand_Count(AssetReportCategory::Model, "tank.w3d") == 0u);
	BOOST_TEST(report.Load_On_Demand_Name_Count(AssetReportCategory::Model) == 0u);
	BOOST_TEST(report.Missing_Count(AssetReportCategory::Model, "TANK.W3D") == 2u);
	BOOST_TEST(report.Missing_Name_Count(AssetReportCategory::Model) == 1u);

	report.Enable_Load_On_Demand_Reporting(true);
	report.Record_Load_On_Demand(AssetReportCategory::Model, "TANK.W3D");
	report.Record_Load_On_Demand(AssetReportCategory::Model, "tank.w3d");
	BOOST_TEST(report.Load_On_Demand_Count(AssetReportCategory::Model, "Tank.W3D") == 2u);
	BOOST_TEST(report.Load_On_Demand_Name_Count(AssetReportCategory::Model) == 1u);
}

BOOST_AUTO_TEST_CASE(asset_report_separates_asset_categories_and_output_flag)
{
	AssetReport report;
	report.Enable_Load_On_Demand_Reporting(true);
	report.Record_Load_On_Demand(AssetReportCategory::Model, "Zeta");
	report.Record_Load_On_Demand(AssetReportCategory::Model, "alpha");
	report.Record_Load_On_Demand(AssetReportCategory::Animation, "Walk");
	report.Record_Load_On_Demand(AssetReportCategory::Skeleton, "Humanoid");
	report.Record_Missing(AssetReportCategory::Model, "Tank");
	report.Record_Missing(AssetReportCategory::Model, "tank");
	report.Record_Missing(AssetReportCategory::Animation, "Walk");

	report.Enable_Reporting(false);
	BOOST_CHECK(!report.Reporting_Enabled());
	// Reporting controls publication by the owner; it does not erase records.
	BOOST_TEST(report.Missing_Count(AssetReportCategory::Model, "TANK") == 2u);

	const std::string expected =
		"Load-on-demand and missing assets report\n\n"
		"Category: LOAD_ON_DEMAND_ROBJ\n\n"
		"alpha\n"
		"zeta\n\n"
		"Category: LOAD_ON_DEMAND_HANIM\n\n"
		"walk\n\n"
		"Category: LOAD_ON_DEMAND_HTREE\n\n"
		"humanoid\n\n"
		"Category: MISSING_ROBJ\n\n"
		"tank\t(reported 2 times)\n\n"
		"Category: MISSING_HANIM\n\n"
		"walk\n\n"
		"Category: MISSING_HTREE\n\n\n";
	BOOST_CHECK(report.Format_Report() == expected);
}

BOOST_AUTO_TEST_CASE(asset_report_clear_removes_records_without_changing_reporting_policy)
{
	AssetReport report;
	report.Enable_Reporting(false);
	report.Enable_Load_On_Demand_Reporting(true);
	report.Record_Load_On_Demand(AssetReportCategory::Skeleton, "Tree");
	report.Record_Missing(AssetReportCategory::Animation, "Clip");

	report.Clear();
	BOOST_TEST(report.Load_On_Demand_Name_Count(AssetReportCategory::Skeleton) == 0u);
	BOOST_TEST(report.Missing_Name_Count(AssetReportCategory::Animation) == 0u);
	BOOST_CHECK(!report.Reporting_Enabled());
	BOOST_CHECK(report.Load_On_Demand_Reporting_Enabled());
}
