module;

#define BOOST_TEST_MODULE GeneralsAssetsIdentityTests

#include <boost/test/included/unit_test.hpp>

#include <clocale>
#include <string>

export module Assets.Tests.AssetIdentity;

import Assets.Identity;

BOOST_AUTO_TEST_CASE(asset_identity_is_canonical_and_typed)
{
	BOOST_CHECK(Assets::Canonicalize_Asset_Name("Models\\./UNIT/../Tank.W3D") == "models/tank.w3d");
	BOOST_CHECK(Assets::Canonicalize_Asset_Name("//Models//Tank.W3D//") == "models/tank.w3d");

	const Assets::AssetIdentity model{Assets::AssetType::Model, "models/tank.w3d"};
	const Assets::AssetIdentity texture{Assets::AssetType::Texture, "models/tank.w3d"};
	BOOST_CHECK(model != texture);
}

BOOST_AUTO_TEST_CASE(asset_name_comparisons_match_legacy_byte_semantics)
{
	BOOST_CHECK(Assets::Asset_Name_Equals_No_Case("Model.W3D", "model.w3d"));
	BOOST_CHECK(!Assets::Asset_Name_Equals_No_Case("model.w3d", "model.w3dx"));
	BOOST_CHECK(!Assets::Asset_Name_Equals_No_Case("model.w3dx", "model.w3d"));
	BOOST_CHECK(Assets::Asset_Name_Equals_No_Case("", ""));
	BOOST_CHECK(!Assets::Asset_Name_Equals_No_Case("a\\b", "a/b"));
	BOOST_CHECK(!Assets::Asset_Name_Equals_No_Case("./name", "name"));
	BOOST_CHECK(Assets::Asset_Name_Equals_No_Case(nullptr, nullptr));
	BOOST_CHECK(!Assets::Asset_Name_Equals_No_Case(nullptr, ""));
	BOOST_CHECK(!Assets::Asset_Name_Equals_No_Case("", nullptr));

	BOOST_CHECK(Assets::Asset_Name_Prefix_Equals_No_Case("HouseColor", "house", 5));
	BOOST_CHECK(Assets::Asset_Name_Prefix_Equals_No_Case("house", "HOUSE", 5));
	BOOST_CHECK(!Assets::Asset_Name_Prefix_Equals_No_Case("Hou", "house", 5));
	BOOST_CHECK(!Assets::Asset_Name_Prefix_Equals_No_Case("HouseX", "house", 6));
	BOOST_CHECK(Assets::Asset_Name_Prefix_Equals_No_Case("House", "houseXX", 5));
	BOOST_CHECK(!Assets::Asset_Name_Prefix_Equals_No_Case("a\\b", "a/b", 3));
	BOOST_CHECK(!Assets::Asset_Name_Prefix_Equals_No_Case("./name", "name", 4));
	BOOST_CHECK(Assets::Asset_Name_Prefix_Equals_No_Case(nullptr, nullptr, 4));
	BOOST_CHECK(!Assets::Asset_Name_Prefix_Equals_No_Case(nullptr, "house", 1));
	BOOST_CHECK(!Assets::Asset_Name_Prefix_Equals_No_Case("house", nullptr, 1));
	BOOST_CHECK(Assets::Asset_Name_Prefix_Equals_No_Case(nullptr, "house", 0));
	BOOST_CHECK(Assets::Asset_Name_Prefix_Equals_No_Case("different", "names", 0));

	const char embedded_left[] = {'A', '\0', 'x'};
	const char embedded_right[] = {'a', '\0', 'Y'};
	BOOST_CHECK(Assets::Asset_Name_Equals_No_Case(embedded_left, embedded_right));
	BOOST_CHECK(Assets::Asset_Name_Prefix_Equals_No_Case(embedded_left, embedded_right, 3));

	const char *previous_locale = std::setlocale(LC_CTYPE, nullptr);
	const std::string saved_locale = previous_locale != nullptr ? previous_locale : "";
	BOOST_REQUIRE(std::setlocale(LC_CTYPE, "C") != nullptr);
	const char high_left[] = {static_cast<char>(0x80), 'A', '\0'};
	const char high_right[] = {static_cast<char>(0x80), 'a', '\0'};
	const char high_different[] = {static_cast<char>(0x81), 'a', '\0'};
	BOOST_CHECK(Assets::Asset_Name_Equals_No_Case(high_left, high_right));
	BOOST_CHECK(!Assets::Asset_Name_Equals_No_Case(high_left, high_different));
	BOOST_CHECK(Assets::Asset_Name_Prefix_Equals_No_Case(high_left, high_right, 2));
	if (!saved_locale.empty())
		std::setlocale(LC_CTYPE, saved_locale.c_str());
}
