module;

#define BOOST_TEST_MODULE EngineUIWNDButtonSnapshotTests

#include <boost/test/included/unit_test.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

export module Engine.UI.WND.ButtonSnapshot.Tests;

import Engine.UI.WND;
import Engine.UI.WND.Document;
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

constexpr float HighDPIScale = 2.0f;
constexpr std::uint32_t HighDPIWidth = 1600;
constexpr std::uint32_t HighDPIHeight = 1200;

struct ButtonSnapshotContext final
{
	PushButtonVisual button;
	RenderList list;
	Renderer wnd_renderer;
	Renderer2D graphics_renderer;
};

bool Extract_Button(void *context, void *, void *, DrawList &draw_list) noexcept
{
	return Add_Push_Button_Background(
		draw_list, static_cast<ButtonSnapshotContext *>(context)->button);
}

bool Render_Button(
	Device &device,
	CommandList &commands,
	RHITextureHandle color_target,
	RHITextureHandle depth_target,
	RHIViewport viewport,
	void *opaque_context) noexcept
{
	ButtonSnapshotContext &context = *static_cast<ButtonSnapshotContext *>(opaque_context);
	if (!commands.Set_Render_Targets(color_target, depth_target)
		|| !commands.Set_Viewport(viewport)
		|| !commands.Clear({0.03f, 0.03f, 0.03f, 1.0f}, 1.0f))
		return false;
	context.graphics_renderer.Begin(viewport.width, viewport.height);
	return context.wnd_renderer.Render(context.list, context.graphics_renderer)
		&& context.graphics_renderer.Execute(
			device,
			commands,
			color_target,
			depth_target,
			viewport);
}

bool Prepare_Fully_Textured_Button(
	const GameData &data,
	ButtonSnapshotContext &context)
{
	std::string source;
	WNDDocument document;
	if (!Load_WND_Source(data.root / "Window" / "Menus" / "MainMenu.wnd", source)
		|| !document.Parse(source))
		return false;

	ImageCatalog catalog;
	if (!Load_Game_Image_Catalog(data, catalog))
		return false;
	WNDDocumentResolveReport report;
	if (!document.Resolve_Images(catalog, report))
		return false;

	const WNDWindow *button_window = nullptr;
	for (const WNDWindow &window : document.Windows()) {
		const WNDDrawState &state = window.draw_states[0];
		if (window.type == WindowType::PushButton
			&& state.cells[0].image.texture.Is_Valid()
			&& state.cells[5].image.texture.Is_Valid()
			&& state.cells[6].image.texture.Is_Valid()
			&& state.cells[0].image_width > 0
			&& state.cells[5].image_width > 0
			&& state.cells[6].image_width > 0) {
			button_window = &window;
			break;
		}
	}
	if (button_window == nullptr)
		return false;

	const WNDDrawState &state = button_window->draw_states[0];
	const float width = static_cast<float>(
		button_window->authored_region.right - button_window->authored_region.left);
	const float height = static_cast<float>(
		button_window->authored_region.bottom - button_window->authored_region.top);
	context.button.rectangle = {
		(HighDPIWidth - width * HighDPIScale) * 0.5f,
		(HighDPIHeight - height * HighDPIScale) * 0.5f,
		(HighDPIWidth + width * HighDPIScale) * 0.5f,
		(HighDPIHeight + height * HighDPIScale) * 0.5f};
	context.button.segmented = true;
	context.button.left_image = state.cells[0].image;
	context.button.middle_image = state.cells[5].image;
	context.button.right_image = state.cells[6].image;
	context.button.left_width = state.cells[0].image_width * HighDPIScale;
	context.button.middle_width = state.cells[5].image_width * HighDPIScale;
	context.button.right_width = state.cells[6].image_width * HighDPIScale;
	context.button.image_color = {1.0f, 1.0f, 1.0f, 1.0f};

	RenderNode node;
	node.extract = &Extract_Button;
	node.extract_context = &context;
	node.screen_region = {
		static_cast<std::int32_t>(context.button.rectangle.left),
		static_cast<std::int32_t>(context.button.rectangle.top),
		static_cast<std::int32_t>(context.button.rectangle.right),
		static_cast<std::int32_t>(context.button.rectangle.bottom)};
	NodeIndex node_index = Invalid_Node;
	if (!context.list.Add_Node(node, node_index))
		return false;
	context.list.Set_Roots(node_index, node_index);
	return true;
}

}

BOOST_AUTO_TEST_CASE(real_wnd_button_has_no_high_dpi_texture_seams)
{
	const auto data = std::make_shared<const GameData>(
		Load_Game_Data(std::filesystem::path(ENGINE_UI_WND_GAME_DATA_ROOT)));
	BOOST_REQUIRE(!data->textures.empty());
	BOOST_REQUIRE(Initialize_Game_Asset_Runtime(data));

	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	ButtonSnapshotContext context;
	BOOST_REQUIRE(Prepare_Fully_Textured_Button(*data, context));
	const std::filesystem::path shader_directory =
		Test_Shader_Directory(ENGINE_UI_WND_SHADER_DIRECTORY);
	BOOST_REQUIRE(context.graphics_renderer.Initialize(device, shader_directory, 65536, 98304, 4096));

	VisualRegressionHarness harness({
		HighDPIWidth,
		HighDPIHeight,
		2,
		std::filesystem::path(ENGINE_UI_WND_VISUAL_REFERENCE_DIRECTORY),
		std::filesystem::path(ENGINE_UI_WND_VISUAL_FAILURE_DIRECTORY)});
	const VisualComparisonResult result = harness.Run(
		device,
		"generalsmd_highdpi_fully_textured_button",
		&Render_Button,
		&context);
	BOOST_CHECK(result.expected_loaded);
	BOOST_CHECK_MESSAGE(result.matched,
		"high-DPI fully textured WND button snapshot mismatch (different pixels="
		<< result.differing_pixels << ", max channel error="
		<< static_cast<unsigned>(result.maximum_channel_error) << ")");
	BOOST_CHECK(context.graphics_renderer.Has_Draws());
	BOOST_CHECK_GT(context.graphics_renderer.Vertex_Count(), 0u);
	context.graphics_renderer.Shutdown();
}
