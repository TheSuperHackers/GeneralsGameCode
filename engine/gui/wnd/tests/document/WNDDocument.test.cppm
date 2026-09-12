module;

#define BOOST_TEST_MODULE EngineUIWNDDocumentTests

#include <boost/test/included/unit_test.hpp>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>

export module Engine.UI.WND.Document.Tests;

import Assets.Cache;
import Engine.UI.WND;
import Engine.UI.WND.Document;
import Engine.UI.WND.Tests.Support.GameData;

using namespace Engine::UI::WND;
using namespace Engine::UI::WND::Tests;

#ifndef ENGINE_UI_WND_GAME_DATA_ROOT
#define ENGINE_UI_WND_GAME_DATA_ROOT "."
#endif

namespace
{

std::filesystem::path Game_Data_Root()
{
	return std::filesystem::path(ENGINE_UI_WND_GAME_DATA_ROOT);
}

std::shared_ptr<const GameData> Load_Fixture(GameData &storage)
{
	storage = Load_Game_Data(Game_Data_Root());
	if (storage.root.empty() || storage.textures.empty())
		return {};
	return std::make_shared<const GameData>(storage);
}

bool Load_Menu(
	const GameData &data,
	std::string_view filename,
	WNDDocument &document,
	ImageCatalog &catalog)
{
	std::string source;
	if (!Load_WND_Source(data.root / "Window" / "Menus" / filename, source)
		|| !document.Parse(source)
		|| !Load_Game_Image_Catalog(data, catalog))
		return false;
	return true;
}

}

BOOST_AUTO_TEST_CASE(parses_generalsmd_wnd_hierarchy_and_authored_geometry)
{
	GameData data;
	const auto fixture = Load_Fixture(data);
	BOOST_REQUIRE(fixture != nullptr);

	WNDDocument document;
	ImageCatalog catalog;
	BOOST_REQUIRE(Load_Menu(*fixture, "MainMenu.wnd", document, catalog));
	BOOST_CHECK_GT(document.Size(), 20u);
	BOOST_CHECK_EQUAL(document.Creation_Width(), 800);
	BOOST_CHECK_EQUAL(document.Creation_Height(), 600);
	BOOST_REQUIRE(!document.Windows().empty());
	BOOST_CHECK(document.Windows()[0].type == WindowType::User);
	BOOST_CHECK_EQUAL(document.Windows()[0].authored_region.right, 800);
	BOOST_CHECK_EQUAL(document.Windows()[0].authored_region.bottom, 600);
	BOOST_CHECK(document.Windows()[0].first_child != Invalid_Node);
	BOOST_CHECK_EQUAL(document.Windows()[1].screen_region.left,
		document.Windows()[1].authored_region.left);
	BOOST_CHECK_EQUAL(document.Windows()[1].screen_region.top,
		document.Windows()[1].authored_region.top);
	BOOST_CHECK(catalog.Size() > 100u);
	BOOST_REQUIRE(catalog.Find("MainMenuBackdrop") != nullptr);
	BOOST_CHECK_EQUAL(
		catalog.Find("MainMenuBackdrop")->texture,
		"MainMenuBackdropuserinterface.tga");
}

BOOST_AUTO_TEST_CASE(resolves_generalsmd_wnd_images_and_fonts_through_asset_runtime)
{
	GameData data;
	const auto fixture = Load_Fixture(data);
	BOOST_REQUIRE(fixture != nullptr);
	BOOST_REQUIRE(Initialize_Game_Asset_Runtime(fixture));

	WNDDocument document;
	ImageCatalog catalog;
	BOOST_REQUIRE(Load_Menu(*fixture, "MainMenu.wnd", document, catalog));
	WNDDocumentResolveReport report;
	BOOST_REQUIRE_MESSAGE(document.Resolve_Images(catalog, report),
		"WND image references were not fully resolved: " << report.missing_images);
	BOOST_REQUIRE(document.Resolve_Fonts(report));
	BOOST_CHECK_GT(report.resolved_images, 0u);
	BOOST_CHECK_GT(report.built_fonts, 0u);
	BOOST_CHECK_EQUAL(report.missing_images, 0u);
	BOOST_CHECK_EQUAL(report.missing_fonts, 0u);

	BOOST_REQUIRE(!document.Windows().empty());
	const WNDDrawCell &cell = document.Windows()[0].draw_states[0].cells[0];
	BOOST_CHECK(cell.image.texture.Is_Valid());
	BOOST_CHECK_GT(cell.image_width, 0u);
	BOOST_CHECK_GT(cell.image_height, 0u);

	Assets::AssetCache *cache = Assets::Try_Get_Asset_Cache();
	BOOST_REQUIRE(cache != nullptr);
	cache->Wait(cell.image.texture);
	const Assets::TextureAsset *asset = cache->Try_Get_Texture(cell.image.texture);
	BOOST_REQUIRE(asset != nullptr);
	BOOST_CHECK(asset->Has_Pixels());
	BOOST_CHECK_GT(asset->Width(), 0u);
	BOOST_CHECK_GT(asset->Height(), 0u);
}

BOOST_AUTO_TEST_CASE(builds_a_modern_render_list_from_each_menu_document)
{
	GameData data;
	const auto fixture = Load_Fixture(data);
	BOOST_REQUIRE(fixture != nullptr);
	BOOST_REQUIRE(Initialize_Game_Asset_Runtime(fixture));

	for (const std::string_view filename : {
		std::string_view("MainMenu.wnd"),
		std::string_view("SinglePlayerMenu.wnd"),
		std::string_view("SkirmishGameOptionsMenu.wnd")}) {
		WNDDocument document;
		ImageCatalog catalog;
		BOOST_REQUIRE(Load_Menu(*fixture, filename, document, catalog));
		WNDDocumentResolveReport report;
		BOOST_REQUIRE(document.Resolve_Images(catalog, report));
		BOOST_REQUIRE(document.Resolve_Fonts(report));
		RenderList list(document.Size());
		BOOST_REQUIRE(document.Build_Render_List(list, 1.0f, 1.0f));
		BOOST_CHECK_EQUAL(list.Size(), document.Size());
		BOOST_CHECK(list.Root_Head() != Invalid_Node);
		BOOST_CHECK(list.Root_Tail() != Invalid_Node);
	}
}

BOOST_AUTO_TEST_CASE(parses_every_checked_in_generalsmd_wnd_fixture)
{
	GameData data;
	const auto fixture = Load_Fixture(data);
	BOOST_REQUIRE(fixture != nullptr);

	std::size_t checked = 0;
	for (const auto &entry : std::filesystem::recursive_directory_iterator(
		fixture->root, std::filesystem::directory_options::skip_permission_denied)) {
		if (!entry.is_regular_file() || entry.path().extension() != ".wnd")
			continue;
		std::string source;
		BOOST_REQUIRE_MESSAGE(Load_WND_Source(entry.path(), source), entry.path().string());
		WNDDocument document;
		BOOST_REQUIRE_MESSAGE(document.Parse(source), entry.path().string());
		BOOST_CHECK_GT(document.Size(), 0u);
		++checked;
	}
	BOOST_REQUIRE_MESSAGE(checked >= 10u, "The checked-in fixture must cover the real WND menu set");
}
