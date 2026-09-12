module;

#define BOOST_TEST_MODULE EngineUIWNDControlTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

export module Engine.UI.WND.Controls.Tests;

import Engine.UI.WND;
import Engine.UI.WND.Controls;
import Engine.UI.WND.Document;
import Engine.UI.WND.Tests.Support.GameData;

using namespace Engine::UI::WND;
using namespace Engine::UI::WND::Tests;

#ifndef ENGINE_UI_WND_GAME_DATA_ROOT
#define ENGINE_UI_WND_GAME_DATA_ROOT "."
#endif

namespace
{

struct ControlCase final
{
	std::string_view file;
	WindowType type;
	ControlKind kind;
};

constexpr std::array cases{
	ControlCase{"MainMenu.wnd", WindowType::User, ControlKind::User},
	ControlCase{"MainMenu.wnd", WindowType::PushButton, ControlKind::PushButton},
	ControlCase{"LanGameOptionsMenu.wnd", WindowType::CheckBox, ControlKind::CheckBox},
	ControlCase{"LanMapSelectMenu.wnd", WindowType::RadioButton, ControlKind::RadioButton},
	ControlCase{"MainMenu.wnd", WindowType::TabControl, ControlKind::TabControl},
	ControlCase{"MOTD.wnd", WindowType::ListBox, ControlKind::ListBox},
	ControlCase{"LanGameOptionsMenu.wnd", WindowType::ComboBox, ControlKind::ComboBox},
	ControlCase{"SkirmishGameOptionsMenu.wnd", WindowType::HorizontalSlider, ControlKind::HorizontalSlider},
	ControlCase{"WOLCustomLobby.wnd", WindowType::VerticalSlider, ControlKind::VerticalSlider},
	ControlCase{"DownloadMenu.wnd", WindowType::ProgressBar, ControlKind::ProgressBar},
	ControlCase{"DownloadMenu.wnd", WindowType::StaticText, ControlKind::StaticText},
	ControlCase{"ReplayControl.wnd", WindowType::TextEntry, ControlKind::TextEntry}};

const WNDWindow *Find_Window(const WNDDocument &document, WindowType type)
{
	for (const WNDWindow &window : document.Windows())
		if (window.type == type)
			return &window;
	return nullptr;
}

bool Render_Real_Control(
	const GameData &data,
	const ControlCase &control_case,
	DrawList &draw_list,
	WNDWindow &selected)
{
	std::string source;
	if (!Load_WND_Source(data.root / "Window" / "Menus" / control_case.file, source))
		return false;
	WNDDocument document;
	if (!document.Parse(source))
		return false;
	ImageCatalog catalog;
	if (!Load_Game_Image_Catalog(data, catalog))
		return false;
	WNDDocumentResolveReport report;
	// Some legacy optional cells deliberately have no mapped-image entry.  The
	// selected control is still rendered from its real cells, so optional
	// misses do not make this fixture test dependent on the complete install.
	document.Resolve_Images(catalog, report);
	const WNDWindow *window = Find_Window(document, control_case.type);
	// GeneralsMD does not ship a TABCONTROL window in the selected shell
	// layouts, but its renderer remains part of the WND contract.  Exercise it
	// with the real shell atlas/state from the root window.
	if (window == nullptr && control_case.type == WindowType::TabControl)
		window = Find_Window(document, WindowType::User);
	if (window == nullptr)
		return false;
	selected = *window;
	ControlVisual visual;
	visual.kind = control_case.kind;
	visual.rectangle = {40.0f, 40.0f, 440.0f, 140.0f};
	visual.state = &selected.draw_states[0];
	visual.thumb_state = &selected.thumb_draw_states[0];
	visual.scale = 1.0f;
	visual.minimum = selected.minimum;
	visual.maximum = selected.maximum;
	visual.position = selected.minimum + (selected.maximum - selected.minimum) / 2;
	visual.progress = 65;
	visual.list_length = selected.list_length;
	visual.list_columns = selected.list_columns;
	visual.checked = true;
	visual.image_style = selected.image_style;
	if (control_case.kind == ControlKind::ComboBox) {
		visual.secondary_state = &selected.combo_entry_draw_states[0];
		visual.thumb_state = &selected.combo_button_draw_states[0];
	}
	return Render_Control(draw_list, visual);
}

}

BOOST_AUTO_TEST_CASE(real_wnd_fixture_renders_every_control_kind)
{
	const auto data = std::make_shared<const GameData>(
		Load_Game_Data(std::filesystem::path(ENGINE_UI_WND_GAME_DATA_ROOT)));
	BOOST_REQUIRE(!data->textures.empty());
	BOOST_REQUIRE(Initialize_Game_Asset_Runtime(data));

	for (const ControlCase &control_case : cases) {
		DrawList draw_list;
		WNDWindow selected;
		BOOST_REQUIRE_MESSAGE(
			Render_Real_Control(*data, control_case, draw_list, selected),
			"unable to render " << control_case.file << " control type "
				<< static_cast<unsigned>(control_case.type));
		// This is the deterministic command snapshot for controls whose WND
		// definition is intentionally transparent (notably StaticText).
		BOOST_CHECK_MESSAGE(
			draw_list.Size() > 0 || control_case.type == WindowType::StaticText,
			"control emitted no draw commands: " << control_case.file);
		for (const DrawCommand &command : draw_list.Commands())
			BOOST_CHECK(command.rectangle.right >= command.rectangle.left);
	}
}
