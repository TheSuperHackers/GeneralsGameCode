module;

#define BOOST_TEST_MODULE EngineUIWNDControlSnapshotTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

export module Engine.UI.WND.ControlSnapshot.Tests;

import Engine.UI.WND;
import Engine.UI.WND.Controls;
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

struct ControlCase final
{
	std::string_view file;
	WindowType source_type;
	ControlKind kind;
	std::string_view snapshot;
};

constexpr std::array cases{
	ControlCase{"MainMenu.wnd", WindowType::User, ControlKind::User, "generalsmd_wnd_control_user"},
	ControlCase{"MainMenu.wnd", WindowType::PushButton, ControlKind::PushButton, "generalsmd_wnd_control_push_button"},
	ControlCase{"LanGameOptionsMenu.wnd", WindowType::CheckBox, ControlKind::CheckBox, "generalsmd_wnd_control_check_box"},
	ControlCase{"LanMapSelectMenu.wnd", WindowType::RadioButton, ControlKind::RadioButton, "generalsmd_wnd_control_radio_button"},
	ControlCase{"MainMenu.wnd", WindowType::TabControl, ControlKind::TabControl, "generalsmd_wnd_control_tab"},
	ControlCase{"MOTD.wnd", WindowType::ListBox, ControlKind::ListBox, "generalsmd_wnd_control_list_box"},
	ControlCase{"LanGameOptionsMenu.wnd", WindowType::ComboBox, ControlKind::ComboBox, "generalsmd_wnd_control_combo_box"},
	ControlCase{"SkirmishGameOptionsMenu.wnd", WindowType::HorizontalSlider, ControlKind::HorizontalSlider, "generalsmd_wnd_control_horizontal_slider"},
	ControlCase{"WOLCustomLobby.wnd", WindowType::VerticalSlider, ControlKind::VerticalSlider, "generalsmd_wnd_control_vertical_slider"},
	ControlCase{"DownloadMenu.wnd", WindowType::ProgressBar, ControlKind::ProgressBar, "generalsmd_wnd_control_progress_bar"},
	ControlCase{"DownloadMenu.wnd", WindowType::StaticText, ControlKind::StaticText, "generalsmd_wnd_control_static_text"},
	ControlCase{"ReplayControl.wnd", WindowType::TextEntry, ControlKind::TextEntry, "generalsmd_wnd_control_text_entry"}};

struct ControlSnapshot final
{
	WNDDocument document;
	WNDWindow window;
	ControlVisual visual;
	std::u16string gallery_text;
};

struct SnapshotContext final
{
	std::unique_ptr<ControlSnapshot> control;
	RenderList list;
	Renderer wnd_renderer;
	Renderer2D graphics_renderer;
};

const WNDWindow *Find_Window(const WNDDocument &document, WindowType type)
{
	for (const WNDWindow &window : document.Windows())
		if (window.type == type)
			return &window;
	return nullptr;
}

bool Extract_Control(void *, void *window, void *, DrawList &draw_list) noexcept
{
	const auto *control = static_cast<const ControlVisual *>(window);
	return control != nullptr && Render_Control(draw_list, *control);
}

bool Render_Control_Snapshot(
	Device &device,
	CommandList &commands,
	RHITextureHandle color_target,
	RHITextureHandle depth_target,
	RHIViewport viewport,
	void *opaque_context) noexcept
{
	SnapshotContext &context = *static_cast<SnapshotContext *>(opaque_context);
	if (!commands.Set_Render_Targets(color_target, depth_target)
		|| !commands.Set_Viewport(viewport)
		|| !commands.Clear({0.025f, 0.025f, 0.025f, 1.0f}, 1.0f))
		return false;
	context.graphics_renderer.Begin(viewport.width, viewport.height);
	return context.wnd_renderer.Render(context.list, context.graphics_renderer)
		&& context.graphics_renderer.Execute(
			device, commands, color_target, depth_target, viewport);
}

bool Prepare_Control(
	const GameData &data,
	const ControlCase &control_case,
	SnapshotContext &context)
{
	std::string source;
	if (!Load_WND_Source(data.root / "Window" / "Menus" / control_case.file, source)
		|| !context.control->document.Parse(source))
		return false;

	ImageCatalog catalog;
	if (!Load_Game_Image_Catalog(data, catalog))
		return false;
	WNDDocumentResolveReport report;
	if (!context.control->document.Resolve_Images(catalog, report, false))
		return false;
	// Some shipped windows reference optional font families. Keep the control
	// gallery usable when those are absent, while requiring a real font for the
	// text-specific cases below.
	context.control->document.Resolve_Fonts(report);
	const WNDWindow *window = Find_Window(context.control->document, control_case.source_type);
	if (window == nullptr && control_case.source_type == WindowType::TabControl)
		window = Find_Window(context.control->document, WindowType::User);
	if (window != nullptr
		&& (control_case.kind == ControlKind::StaticText || control_case.kind == ControlKind::TextEntry)
		&& (window->font == nullptr || window->text.empty())) {
		for (const WNDWindow &candidate : context.control->document.Windows()) {
			if (candidate.type == control_case.source_type
				&& candidate.font != nullptr && !candidate.text.empty()) {
				window = &candidate;
				break;
			}
		}
	}
	if (window == nullptr)
		return false;

	context.control->window = *window;
	ControlVisual &visual = context.control->visual;
	visual.kind = control_case.kind;
	visual.rectangle = {48.0f, 48.0f, 752.0f, 152.0f};
	visual.state = &context.control->window.draw_states[0];
	visual.thumb_state = &context.control->window.thumb_draw_states[0];
	visual.secondary_state = &context.control->window.combo_entry_draw_states[0];
	visual.scale = 1.0f;
	visual.minimum = context.control->window.minimum;
	visual.maximum = context.control->window.maximum;
	visual.position = visual.minimum + (visual.maximum - visual.minimum) / 2;
	visual.progress = 65;
	visual.list_length = context.control->window.list_length;
	visual.list_columns = context.control->window.list_columns;
	visual.checked = true;
	visual.image_style = context.control->window.image_style;
	visual.font = context.control->window.font;
	if (visual.font == nullptr) {
		for (const WNDWindow &candidate : context.control->document.Windows()) {
			if (candidate.font != nullptr) {
				visual.font = candidate.font;
				break;
			}
		}
	}
	context.control->gallery_text = context.control->window.text;
	if (context.control->gallery_text.empty())
		context.control->gallery_text = control_case.kind == ControlKind::StaticText
			? u"STATIC TEXT" : u"ENTRY TEXT";
	visual.text = reinterpret_cast<const std::uint16_t *>(context.control->gallery_text.c_str());
	visual.text_style = context.control->window.text_styles[0];
	visual.centered_text = context.control->window.centered_text;
	visual.centered_text_vertically = context.control->window.centered_text_vertically;
	if (control_case.kind == ControlKind::ComboBox) {
		visual.secondary_state = &context.control->window.combo_entry_draw_states[0];
		visual.thumb_state = &context.control->window.combo_button_draw_states[0];
	}

	RenderNode node;
	node.window = &visual;
	node.extract = &Extract_Control;
	node.screen_region = {48, 48, 752, 152};
	NodeIndex node_index = Invalid_Node;
	return context.list.Add_Node(node, node_index)
		&& (context.list.Set_Roots(node_index, node_index), true);
}

void Run_Control_Snapshot(
	const std::shared_ptr<const GameData> &data,
	const ControlCase &control_case)
{
	SnapshotContext context;
	context.control = std::make_unique<ControlSnapshot>();
	BOOST_REQUIRE_MESSAGE(Prepare_Control(*data, control_case, context),
		"unable to prepare control " << control_case.snapshot
		<< " (missing images=" << context.control->document.Report().missing_images
		<< ", missing fonts=" << context.control->document.Report().missing_fonts << ")");

	GraphicsTestDevice device({true});
	BOOST_REQUIRE_MESSAGE(device.Is_Valid(),
		"graphics test device unavailable for " << control_case.snapshot);
	const std::filesystem::path shader_directory =
		Test_Shader_Directory(ENGINE_UI_WND_SHADER_DIRECTORY);
	BOOST_REQUIRE(context.graphics_renderer.Initialize(device, shader_directory, 8192, 12288, 1024));

	VisualRegressionHarness harness({
		800,
		200,
		2,
		std::filesystem::path(ENGINE_UI_WND_VISUAL_REFERENCE_DIRECTORY),
		std::filesystem::path(ENGINE_UI_WND_VISUAL_FAILURE_DIRECTORY)});
	const VisualComparisonResult result = harness.Run(
		device, control_case.snapshot, &Render_Control_Snapshot, &context);
	BOOST_CHECK_MESSAGE(result.expected_loaded,
		"missing WND control snapshot " << control_case.snapshot);
	BOOST_CHECK_MESSAGE(result.matched,
		"WND control snapshot mismatch for " << control_case.snapshot
		<< " (different pixels=" << result.differing_pixels
		<< ", max channel error=" << static_cast<unsigned>(result.maximum_channel_error) << ")");
	BOOST_CHECK_MESSAGE(context.graphics_renderer.Has_Draws(),
		"control produced no draw commands: " << control_case.snapshot);
	BOOST_CHECK_MESSAGE(context.graphics_renderer.Vertex_Count() > 0u,
		"control produced no vertices: " << control_case.snapshot);
	context.graphics_renderer.Shutdown();
}

}

BOOST_AUTO_TEST_CASE(real_generalsmd_wnd_controls_match_snapshots)
{
	const auto data = std::make_shared<const GameData>(
		Load_Game_Data(std::filesystem::path(ENGINE_UI_WND_GAME_DATA_ROOT)));
	BOOST_REQUIRE(!data->textures.empty());
	BOOST_REQUIRE(Initialize_Game_Asset_Runtime(data));
	for (const ControlCase &control_case : cases)
		Run_Control_Snapshot(data, control_case);
}
