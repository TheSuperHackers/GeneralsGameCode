module;

#define BOOST_TEST_MODULE EngineUIWNDAssetSnapshotTests

#include <boost/test/included/unit_test.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

export module Engine.UI.WND.AssetSnapshot.Tests;

import Engine.UI.WND.Document;
import Engine.UI.WND;
import Engine.UI.WND.Tests.Support.GameData;
import Graphics.Tests.Device;
import Graphics.Testing.VisualRegression;

using namespace Engine::UI::WND;
using namespace Engine::UI::WND::Tests;
using namespace Graphics;

#ifndef ENGINE_UI_WND_GAME_DATA_ROOT
#define ENGINE_UI_WND_GAME_DATA_ROOT "."
#endif

#ifndef ENGINE_UI_WND_SHADER_DIRECTORY
#define ENGINE_UI_WND_SHADER_DIRECTORY "."
#endif

#ifndef ENGINE_UI_WND_VISUAL_REFERENCE_DIRECTORY
#define ENGINE_UI_WND_VISUAL_REFERENCE_DIRECTORY "."
#endif

#ifndef ENGINE_UI_WND_VISUAL_FAILURE_DIRECTORY
#define ENGINE_UI_WND_VISUAL_FAILURE_DIRECTORY "."
#endif

namespace
{

struct WNDSnapshotContext final
{
	WNDDocument document;
	RenderList list;
	Renderer wnd_renderer;
	Renderer2D graphics_renderer;
	ImageRef shell_background{};
};

bool Render_WND(
	Device &device,
	CommandList &commands,
	RHITextureHandle color_target,
	RHITextureHandle depth_target,
	RHIViewport viewport,
	void *opaque_context) noexcept
{
	WNDSnapshotContext &context = *static_cast<WNDSnapshotContext *>(opaque_context);
	if (!commands.Set_Render_Targets(color_target, depth_target)
		|| !commands.Set_Viewport(viewport)
		|| !commands.Clear({0.0f, 0.0f, 0.0f, 1.0f}, 1.0f))
		return false;
	context.graphics_renderer.Begin(viewport.width, viewport.height);
	if (context.shell_background.texture.Is_Valid()
		&& !Draw_Image(context.graphics_renderer, context.shell_background,
			{0.0f, 0.0f, static_cast<float>(viewport.width), static_cast<float>(viewport.height)}))
		return false;
	return context.wnd_renderer.Render(context.list, context.graphics_renderer)
		&& context.graphics_renderer.Execute(
			device,
			commands, color_target, depth_target, viewport);
}

bool Prepare_WND(
	const GameData &data,
	std::string_view filename,
	WNDSnapshotContext &context)
{
	std::string source;
	if (!Load_WND_Source(data.root / "Window" / "Menus" / filename, source)
		|| !context.document.Parse(source))
		return false;

	ImageCatalog catalog;
	if (!Load_Game_Image_Catalog(data, catalog))
		return false;
	if (filename == "SinglePlayerMenu.wnd")
		context.shell_background = catalog.Resolve("MainMenuBackdrop");
	WNDDocumentResolveReport report;
	if (!context.document.Resolve_Images(catalog, report)
		|| !context.document.Resolve_Fonts(report)
		|| report.resolved_images == 0
		|| report.built_fonts == 0)
		return false;

	context.list = RenderList(context.document.Size());
	return context.document.Build_Render_List(context.list);
}

void Run_WND_Snapshot(
	const std::shared_ptr<const GameData> &data,
	std::string_view wnd_filename,
	std::string_view snapshot_name,
	std::string_view menu_name)
{
	BOOST_REQUIRE(data != nullptr);
	GraphicsTestDevice device({true});
	BOOST_REQUIRE_MESSAGE(device.Is_Valid(), "graphics test device unavailable for " << menu_name);

	WNDSnapshotContext context;
	BOOST_REQUIRE_MESSAGE(Prepare_WND(*data, wnd_filename, context),
		"unable to prepare asset-backed " << menu_name << " WND (images resolved="
		<< context.document.Report().resolved_images << ", missing="
		<< context.document.Report().missing_images << ", fonts built="
		<< context.document.Report().built_fonts << ", fonts missing="
		<< context.document.Report().missing_fonts << ")");
	const std::filesystem::path shader_directory =
		Test_Shader_Directory(ENGINE_UI_WND_SHADER_DIRECTORY);
	BOOST_REQUIRE_MESSAGE(
		context.graphics_renderer.Initialize(device, shader_directory, 65536, 98304, 4096),
		"unable to initialize WND renderer with " << shader_directory.string());

	VisualRegressionHarness harness({
		800,
		600,
		2,
		std::filesystem::path(ENGINE_UI_WND_VISUAL_REFERENCE_DIRECTORY),
		std::filesystem::path(ENGINE_UI_WND_VISUAL_FAILURE_DIRECTORY)});
	const VisualComparisonResult result = harness.Run(
		device, snapshot_name, &Render_WND, &context);
	BOOST_CHECK_MESSAGE(result.expected_loaded,
		"missing asset-backed " << menu_name << " WND snapshot");
	BOOST_CHECK_MESSAGE(result.matched,
		"asset-backed " << menu_name << " WND snapshot mismatch (different pixels="
		<< result.differing_pixels << ", max channel error="
		<< static_cast<unsigned>(result.maximum_channel_error) << ")");
	BOOST_CHECK(context.graphics_renderer.Has_Draws());
	BOOST_CHECK_GT(context.graphics_renderer.Vertex_Count(), 0u);
	BOOST_CHECK_GT(context.graphics_renderer.Batch_Count(), 0u);
	context.graphics_renderer.Shutdown();
}

}

BOOST_AUTO_TEST_CASE(asset_backed_generalsmd_wnd_menus_match_snapshots)
{
	const auto data = std::make_shared<const GameData>(
		Load_Game_Data(std::filesystem::path(ENGINE_UI_WND_GAME_DATA_ROOT)));
	BOOST_REQUIRE(!data->textures.empty());
	BOOST_REQUIRE(Initialize_Game_Asset_Runtime(data));

	Run_WND_Snapshot(data, "MainMenu.wnd", "generalsmd_main_menu_wnd", "MainMenu");
	Run_WND_Snapshot(
		data, "SinglePlayerMenu.wnd", "generalsmd_single_player_menu_wnd", "SinglePlayerMenu");
	Run_WND_Snapshot(
		data,
		"SkirmishGameOptionsMenu.wnd",
		"generalsmd_skirmish_options_menu_wnd",
		"SkirmishGameOptionsMenu");
}
