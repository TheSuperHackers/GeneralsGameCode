module;

#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

export module Engine.UI.WND.Document;

import Assets.Runtime;
import Assets.Cache;
import Engine.UI.WND;
import Engine.UI.WND.Controls;

namespace Engine::UI::WND
{

export enum class WindowType : std::uint8_t
{
	Unknown,
	User,
	PushButton,
	CheckBox,
	RadioButton,
	TabControl,
	ListBox,
	ComboBox,
	HorizontalSlider,
	VerticalSlider,
	ProgressBar,
	StaticText,
	TextEntry
};

export struct ImageDefinition final
{
	std::string name;
	std::string texture;
	std::uint32_t texture_width = 0;
	std::uint32_t texture_height = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	Graphics::Rect2D uv{0.0f, 0.0f, 1.0f, 1.0f};
};

export class ImageCatalog final
{
public:
	bool Add(ImageDefinition definition)
	{
		if (definition.name.empty() || definition.texture.empty()
			|| definition.texture_width == 0 || definition.texture_height == 0
			|| definition.width == 0 || definition.height == 0)
			return false;

		const std::string key = Key(definition.name);
		m_definitions.insert_or_assign(key, std::move(definition));
		return true;
	}

	const ImageDefinition *Find(std::string_view name) const noexcept
	{
		const std::string key = Key(name);
		const auto found = m_definitions.find(key);
		return found == m_definitions.end() ? nullptr : &found->second;
	}

	ImageRef Resolve(std::string_view name) const
	{
		ImageRef result;
		const ImageDefinition *definition = Find(name);
		if (definition == nullptr)
			return result;
		result = Resolve_Image_Reference(definition->texture);
		result.uv = definition->uv;
		return result;
	}

	std::size_t Size() const noexcept { return m_definitions.size(); }

private:
	static std::string Key(std::string_view value)
	{
		std::string result;
		result.reserve(value.size());
		for (const char character : value)
			result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
		return result;
	}

	std::unordered_map<std::string, ImageDefinition> m_definitions;
};

namespace WNDDocumentDetail
{

std::string Trim(std::string_view value)
{
	while (!value.empty() && (value.front() == ' ' || value.front() == '\t'
		|| value.front() == '\r' || value.front() == '\n'))
		value.remove_prefix(1);
	while (!value.empty() && (value.back() == ' ' || value.back() == '\t'
		|| value.back() == '\r' || value.back() == '\n'))
		value.remove_suffix(1);
	return std::string(value);
}

bool Starts_With(std::string_view value, std::string_view prefix) noexcept
{
	if (value.size() < prefix.size())
		return false;
	for (std::size_t index = 0; index < prefix.size(); ++index) {
		if (std::toupper(static_cast<unsigned char>(value[index]))
			!= std::toupper(static_cast<unsigned char>(prefix[index])))
			return false;
	}
	return true;
}

std::string Key(std::string_view value)
{
	std::string result = Trim(value);
	for (char &character : result)
		character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
	return result;
}

std::string Unquote(std::string value)
{
	value = Trim(value);
	if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
		return value.substr(1, value.size() - 2);
	return value;
}

bool Read_Int(std::string_view statement, std::string_view marker, int &value)
{
	const std::size_t position = statement.find(marker);
	if (position == std::string_view::npos)
		return false;
	std::istringstream values(std::string(statement.substr(position + marker.size())));
	return static_cast<bool>(values >> value);
}

bool Read_Point(std::string_view statement, std::string_view marker, int &x, int &y)
{
	const std::size_t position = statement.find(marker);
	if (position == std::string_view::npos)
		return false;
	std::string values(statement.substr(position + marker.size()));
	for (char &character : values)
		if (character == ',')
			character = ' ';
	std::istringstream stream(values);
	return static_cast<bool>(stream >> x >> y);
}

bool Read_Color(std::string_view statement, std::string_view marker, Graphics::Color2D &color)
{
	const std::size_t position = statement.find(marker);
	if (position == std::string_view::npos)
		return false;
	std::istringstream values(std::string(statement.substr(position + marker.size())));
	int red = 0;
	int green = 0;
	int blue = 0;
	int alpha = 0;
	if (!(values >> red >> green >> blue >> alpha))
		return false;
	color = {
		static_cast<float>(std::clamp(red, 0, 255)) / 255.0f,
		static_cast<float>(std::clamp(green, 0, 255)) / 255.0f,
		static_cast<float>(std::clamp(blue, 0, 255)) / 255.0f,
		static_cast<float>(std::clamp(alpha, 0, 255)) / 255.0f};
	return true;
}

bool Read_Non_Negative(std::string_view value, std::uint32_t &result) noexcept
{
	const std::string trimmed = Trim(value);
	if (trimmed.empty())
		return false;
	std::uint32_t parsed = 0;
	const auto [end, error] = std::from_chars(
		trimmed.data(), trimmed.data() + trimmed.size(), parsed);
	if (error != std::errc{} || end != trimmed.data() + trimmed.size())
		return false;
	result = parsed;
	return true;
}

WindowType Parse_Window_Type(std::string_view type) noexcept
{
	const std::string key = Key(type);
	if (key == "USER") return WindowType::User;
	if (key == "PUSHBUTTON") return WindowType::PushButton;
	if (key == "CHECKBOX") return WindowType::CheckBox;
	if (key == "RADIOBUTTON") return WindowType::RadioButton;
	if (key == "TABCONTROL") return WindowType::TabControl;
	if (key == "SCROLLLISTBOX") return WindowType::ListBox;
	if (key == "COMBOBOX") return WindowType::ComboBox;
	if (key == "HORZSLIDER") return WindowType::HorizontalSlider;
	if (key == "VERTSLIDER") return WindowType::VerticalSlider;
	if (key == "PROGRESSBAR") return WindowType::ProgressBar;
	if (key == "STATICTEXT") return WindowType::StaticText;
	if (key == "ENTRYFIELD") return WindowType::TextEntry;
	return WindowType::Unknown;
}

void Parse_Draw_Data(std::string_view statement, std::array<WNDDrawCell, WND_Draw_Cell_Count> &cells);

}

namespace WNDDocumentDetail
{

void Parse_Draw_Data(std::string_view statement, std::array<WNDDrawCell, WND_Draw_Cell_Count> &cells)
{
	std::size_t cursor = 0;
	for (WNDDrawCell &cell : cells) {
		const std::size_t image_position = statement.find("IMAGE:", cursor);
		if (image_position == std::string_view::npos)
			return;
		const std::size_t value_end = statement.find(',', image_position);
		if (value_end == std::string_view::npos)
			return;
		cell.image_name = Trim(statement.substr(image_position + 6, value_end - image_position - 6));
		const std::size_t next_image = statement.find("IMAGE:", value_end + 1);
		const std::string_view cell_text = statement.substr(
			value_end + 1,
			next_image == std::string_view::npos ? statement.size() - value_end - 1
				: next_image - value_end - 1);
		Read_Color(cell_text, "COLOR:", cell.color);
		Read_Color(cell_text, "BORDERCOLOR:", cell.border_color);
		cursor = next_image == std::string_view::npos ? statement.size() : next_image;
	}
}

}

export bool Parse_Mapped_Image_INI(std::string_view source, ImageCatalog &catalog)
{
	ImageDefinition definition;
	bool in_definition = false;
	std::istringstream lines{std::string(source)};
	std::string line;
	while (std::getline(lines, line)) {
		const std::string trimmed = WNDDocumentDetail::Trim(line);
		if (trimmed.empty() || trimmed.front() == ';')
			continue;
		if (WNDDocumentDetail::Starts_With(trimmed, "MappedImage ")) {
			definition = {};
			definition.name = WNDDocumentDetail::Trim(
				std::string_view(trimmed).substr(std::string_view("MappedImage ").size()));
			in_definition = true;
			continue;
		}
		if (WNDDocumentDetail::Key(trimmed) == "END") {
			if (in_definition && !catalog.Add(std::move(definition)))
				return false;
			in_definition = false;
			continue;
		}
		if (!in_definition)
			continue;
		const std::size_t equals = trimmed.find('=');
		if (equals == std::string::npos)
			continue;
		const std::string field = WNDDocumentDetail::Key(std::string_view(trimmed).substr(0, equals));
		const std::string value = WNDDocumentDetail::Trim(std::string_view(trimmed).substr(equals + 1));
		if (field == "TEXTURE")
			definition.texture = value;
		else if (field == "TEXTUREWIDTH")
			WNDDocumentDetail::Read_Non_Negative(value, definition.texture_width);
		else if (field == "TEXTUREHEIGHT")
			WNDDocumentDetail::Read_Non_Negative(value, definition.texture_height);
		else if (field == "COORDS") {
			int left = 0;
			int top = 0;
			int right = 0;
			int bottom = 0;
			WNDDocumentDetail::Read_Int(value, "Left:", left);
			WNDDocumentDetail::Read_Int(value, "Top:", top);
			WNDDocumentDetail::Read_Int(value, "Right:", right);
			WNDDocumentDetail::Read_Int(value, "Bottom:", bottom);
			definition.width = static_cast<std::uint32_t>(std::max(0, right - left));
			definition.height = static_cast<std::uint32_t>(std::max(0, bottom - top));
			definition.uv = {
				static_cast<float>(left) / std::max(1u, definition.texture_width),
				static_cast<float>(top) / std::max(1u, definition.texture_height),
				static_cast<float>(right) / std::max(1u, definition.texture_width),
				static_cast<float>(bottom) / std::max(1u, definition.texture_height)};
		}
	}
	return !in_definition;
}

export struct WNDDocumentResolveReport final
{
	std::size_t resolved_images = 0;
	std::size_t missing_images = 0;
	std::size_t built_fonts = 0;
	std::size_t missing_fonts = 0;
};

export struct WNDWindow final
{
	std::string type_name;
	WindowType type = WindowType::Unknown;
	std::string name;
	std::string draw_callback;
	std::string text_label;
	std::u16string text;
	std::string font_name;
	std::uint32_t font_size = 0;
	bool font_bold = false;
	bool image_style = false;
	bool centered_text = false;
	bool centered_text_vertically = true;
	std::uint32_t flags = static_cast<std::uint32_t>(WindowFlag::None);
	Layer layer = Layer::Normal;
	VisualState visual_state = VisualState::Normal;
	Rect authored_region{};
	Rect screen_region{};
	std::array<WNDDrawState, 3> draw_states{};
	std::array<WNDDrawState, 3> thumb_draw_states{};
	std::array<WNDDrawState, 3> combo_button_draw_states{};
	std::array<WNDDrawState, 3> combo_entry_draw_states{};
	std::array<TextStyle, 3> text_styles{};
	int minimum = 0;
	int maximum = 100;
	int position = 50;
	int progress = 50;
	std::uint32_t list_length = 4;
	std::uint32_t list_columns = 1;
	NodeIndex first_child = Invalid_Node;
	NodeIndex last_child = Invalid_Node;
	NodeIndex next_sibling = Invalid_Node;
	const FontFace *font = nullptr;
};

namespace WNDDocumentDetail
{

void Parse_Status(WNDWindow &window, std::string_view value)
{
	std::size_t start = 0;
	while (start <= value.size()) {
		const std::size_t end = value.find('+', start);
		const std::string token = Key(value.substr(start, end == std::string_view::npos ? value.size() - start : end - start));
		if (token == "HIDDEN") window.flags |= static_cast<std::uint32_t>(WindowFlag::Hidden);
		else if (token == "SEE_THRU") window.flags |= static_cast<std::uint32_t>(WindowFlag::SeeThrough);
		else if (token == "BORDER") window.flags |= static_cast<std::uint32_t>(WindowFlag::Border);
		else if (token == "BELOW") window.layer = Layer::Below;
		else if (token == "ABOVE") window.layer = Layer::Above;
		else if (token == "IMAGE") window.image_style = true;
		if (end == std::string_view::npos)
			break;
		start = end + 1;
	}
}

void Parse_Text_Colors(WNDWindow &window, std::string_view value)
{
	const std::array<std::pair<std::string_view, std::size_t>, 3> states = {{
		{"ENABLED:", 0}, {"DISABLED:", 1}, {"HILITE:", 2}}};
	for (const auto &[marker, index] : states) {
		Graphics::Color2D color;
		if (Read_Color(value, marker, color))
			window.text_styles[index].color = color;
		const std::string border_marker = std::string(marker.substr(0, marker.size() - 1)) + "BORDER:";
		if (Read_Color(value, border_marker, color))
			window.text_styles[index].drop_color = color;
	}
}

void Parse_Statement(
	WNDWindow &window,
	std::string_view statement,
	WNDDocumentResolveReport &,
	int &creation_width,
	int &creation_height)
{
	const std::size_t equals = statement.find('=');
	if (equals == std::string_view::npos)
		return;
	const std::string field = Key(statement.substr(0, equals));
	const std::string value = Trim(statement.substr(equals + 1));
	if (field == "WINDOWTYPE") {
		window.type_name = value;
		window.type = Parse_Window_Type(value);
	} else if (field == "SCREENRECT") {
		Read_Point(statement, "UPPERLEFT:", window.authored_region.left, window.authored_region.top);
		Read_Point(statement, "BOTTOMRIGHT:", window.authored_region.right, window.authored_region.bottom);
		Read_Point(statement, "CREATIONRESOLUTION:", creation_width, creation_height);
	} else if (field == "NAME") {
		window.name = Unquote(value);
	} else if (field == "DRAWCALLBACK") {
		window.draw_callback = Unquote(value);
	} else if (field == "STATUS") {
		Parse_Status(window, value);
	} else if (field == "STYLE") {
		if (Key(value).find("CLIP") != std::string::npos)
			window.flags |= static_cast<std::uint32_t>(WindowFlag::ClipChildren);
	} else if (field == "FONT") {
		const std::size_t name_position = value.find("NAME:");
		if (name_position != std::string::npos) {
			const std::size_t comma = value.find(',', name_position);
			window.font_name = Unquote(Trim(value.substr(name_position + 5,
				comma == std::string::npos ? value.size() - name_position - 5 : comma - name_position - 5)));
		}
		int size = 0;
		if (Read_Int(value, "SIZE:", size))
			window.font_size = static_cast<std::uint32_t>(std::max(0, size));
		int bold = 0;
		if (Read_Int(value, "BOLD:", bold))
			window.font_bold = bold != 0;
	} else if (field == "TEXT") {
		window.text_label = Unquote(value);
		window.text.clear();
		window.text.reserve(window.text_label.size());
		for (const unsigned char character : window.text_label)
			window.text.push_back(static_cast<char16_t>(character));
	} else if (field == "TEXTCOLOR") {
		Parse_Text_Colors(window, value);
	} else if (field == "STATICTEXTDATA") {
		window.centered_text = Key(value).find("CENTERED: 1") != std::string::npos;
	} else if (field == "ENABLEDDRAWDATA") {
		Parse_Draw_Data(value, window.draw_states[0].cells);
	} else if (field == "DISABLEDDRAWDATA") {
		Parse_Draw_Data(value, window.draw_states[1].cells);
	} else if (field == "HILITEDRAWDATA") {
		Parse_Draw_Data(value, window.draw_states[2].cells);
	} else if (field == "SLIDERDATA") {
		Read_Int(value, "MINVALUE:", window.minimum);
		Read_Int(value, "MAXVALUE:", window.maximum);
		window.position = window.minimum;
	} else if (field == "LISTBOXDATA") {
		int length = 0;
		int columns = 0;
		if (Read_Int(value, "LENGTH:", length))
			window.list_length = static_cast<std::uint32_t>((std::max)(0, length));
		if (Read_Int(value, "COLUMNS:", columns))
			window.list_columns = static_cast<std::uint32_t>((std::max)(0, columns));
	} else if (field == "SLIDERTHUMBENABLEDDRAWDATA") {
		Parse_Draw_Data(value, window.thumb_draw_states[0].cells);
	} else if (field == "SLIDERTHUMBDISABLEDDRAWDATA") {
		Parse_Draw_Data(value, window.thumb_draw_states[1].cells);
	} else if (field == "SLIDERTHUMBHILITEDRAWDATA") {
		Parse_Draw_Data(value, window.thumb_draw_states[2].cells);
	} else if (field == "COMBOBOXDROPDOWNBUTTONENABLEDDRAWDATA") {
		Parse_Draw_Data(value, window.combo_button_draw_states[0].cells);
	} else if (field == "COMBOBOXDROPDOWNBUTTONDISABLEDDRAWDATA") {
		Parse_Draw_Data(value, window.combo_button_draw_states[1].cells);
	} else if (field == "COMBOBOXDROPDOWNBUTTONHILITEDRAWDATA") {
		Parse_Draw_Data(value, window.combo_button_draw_states[2].cells);
	} else if (field == "COMBOBOXEDITBOXENABLEDDRAWDATA") {
		Parse_Draw_Data(value, window.combo_entry_draw_states[0].cells);
	} else if (field == "COMBOBOXEDITBOXDISABLEDDRAWDATA") {
		Parse_Draw_Data(value, window.combo_entry_draw_states[1].cells);
	} else if (field == "COMBOBOXEDITBOXHILITEDRAWDATA") {
		Parse_Draw_Data(value, window.combo_entry_draw_states[2].cells);
	}
}

}

export class WNDDocument final
{
public:
	bool Parse(std::string_view source)
	{
		m_windows.clear();
		m_fonts.clear();
		m_report = {};
		m_creation_width = 0;
		m_creation_height = 0;
		std::vector<NodeIndex> parents;
		NodeIndex last_root = Invalid_Node;
		std::string pending;
		std::istringstream lines{std::string(source)};
		std::string line;
		while (std::getline(lines, line)) {
			const std::string trimmed = WNDDocumentDetail::Trim(line);
			if (trimmed.empty() || trimmed.front() == ';')
				continue;
			if (trimmed == "WINDOW") {
				const NodeIndex index = static_cast<NodeIndex>(m_windows.size());
				m_windows.emplace_back();
				for (TextStyle &style : m_windows.back().text_styles) {
					style.color = {1.0f, 1.0f, 1.0f, 1.0f};
					style.drop_color = {0.0f, 0.0f, 0.0f, 1.0f};
					style.hotkey_color = {1.0f, 1.0f, 1.0f, 1.0f};
				}
				if (parents.empty()) {
					if (last_root != Invalid_Node)
						m_windows[last_root].next_sibling = index;
					last_root = index;
				} else {
					WNDWindow &parent = m_windows[parents.back()];
					if (parent.first_child == Invalid_Node)
						parent.first_child = index;
					else
						m_windows[parent.last_child].next_sibling = index;
					parent.last_child = index;
				}
				parents.push_back(index);
				pending.clear();
				continue;
			}
			if (trimmed == "END") {
				if (!pending.empty()) {
					WNDDocumentDetail::Parse_Statement(
						m_windows[parents.back()], pending, m_report, m_creation_width, m_creation_height);
					pending.clear();
				}
				if (parents.empty())
					return false;
				parents.pop_back();
				continue;
			}
			if (trimmed == "CHILD" || trimmed == "STARTLAYOUTBLOCK" || trimmed == "ENDLAYOUTBLOCK")
				continue;
			if (parents.empty())
				continue;
			if (!pending.empty())
				pending.push_back(' ');
			pending += trimmed;
			const std::size_t semicolon = pending.find(';');
			if (semicolon != std::string::npos) {
				WNDDocumentDetail::Parse_Statement(
					m_windows[parents.back()], std::string_view(pending).substr(0, semicolon), m_report,
					m_creation_width, m_creation_height);
				pending.clear();
			}
		}
		if (!pending.empty() && !parents.empty())
			WNDDocumentDetail::Parse_Statement(
				m_windows[parents.back()], pending, m_report, m_creation_width, m_creation_height);
		if (!parents.empty() || m_windows.empty())
			return false;
		Compute_Screen_Regions();
		return true;
	}

	bool Resolve_Images(const ImageCatalog &catalog, WNDDocumentResolveReport &report,
		bool strict = true)
	{
		auto resolve_states = [&](WNDWindow &window, std::array<WNDDrawState, 3> &states) {
			for (WNDDrawState &state : states)
				for (WNDDrawCell &cell : state.cells) {
					if (cell.image_name.empty() || WNDDocumentDetail::Key(cell.image_name) == "NOIMAGE")
						continue;
					const ImageDefinition *definition = catalog.Find(cell.image_name);
					cell.image = catalog.Resolve(cell.image_name);
					if (definition == nullptr) {
						// Static-text image cells are optional until the runtime
						// gadget switches into image-background mode.  Image-backed
						// windows and segmented controls, however, must resolve now.
						if (strict && (window.image_style || window.type == WindowType::PushButton))
							++report.missing_images;
						continue;
					}
					cell.image_width = definition->width;
					cell.image_height = definition->height;
					++report.resolved_images;
				}
		};
		for (WNDWindow &window : m_windows) {
			resolve_states(window, window.draw_states);
			resolve_states(window, window.thumb_draw_states);
			resolve_states(window, window.combo_button_draw_states);
			resolve_states(window, window.combo_entry_draw_states);
		}
		m_report = report;
		return report.missing_images == 0;
	}

	bool Resolve_Fonts(WNDDocumentResolveReport &report)
	{
		Assets::AssetCache *cache = Assets::Try_Get_Asset_Cache();
		if (cache == nullptr)
			return false;
		std::unordered_map<std::string, const FontFace *> faces;
		for (WNDWindow &window : m_windows) {
			if (window.text_label.empty() || window.font_name.empty() || window.font_size == 0)
				continue;
			const std::string key = window.font_name + '/' + std::to_string(window.font_size)
				+ '/' + (window.font_bold ? "1" : "0");
			if (const auto found = faces.find(key); found != faces.end()) {
				window.font = found->second;
				continue;
			}
			const Assets::FontAssetHandle handle = cache->Request_Font(
				window.font_name, window.font_size, window.font_bold);
			cache->Wait(handle);
			const Assets::FontAsset *asset = cache->Try_Get_Font(handle);
			if (asset == nullptr) {
				++report.missing_fonts;
				continue;
			}
			auto face = std::make_unique<FontFace>();
			if (!face->Build(*asset)) {
				++report.missing_fonts;
				continue;
			}
			const FontFace *face_pointer = face.get();
			m_fonts.push_back(std::move(face));
			faces.emplace(key, face_pointer);
			window.font = face_pointer;
			++report.built_fonts;
		}
		m_report = report;
		return report.missing_fonts == 0;
	}

	bool Build_Render_List(RenderList &list, float scale_x = 1.0f, float scale_y = 1.0f) noexcept
	{
		if (m_windows.empty())
			return false;
		m_last_scale_x = scale_x;
		m_last_scale_y = scale_y;
		DocumentRenderContext context{this, scale_x, scale_y};
		return Engine::UI::WND::Build_Render_List(list, &m_windows[0], {
			&context, &Next, &Child, &Describe});
	}

	std::span<const WNDWindow> Windows() const noexcept { return m_windows; }
	std::size_t Size() const noexcept { return m_windows.size(); }
	int Creation_Width() const noexcept { return m_creation_width; }
	int Creation_Height() const noexcept { return m_creation_height; }
	const WNDDocumentResolveReport &Report() const noexcept { return m_report; }

private:
	struct DocumentRenderContext final
	{
		const WNDDocument *document = nullptr;
		float scale_x = 1.0f;
		float scale_y = 1.0f;
	};

	static WNDWindow *Window(void *pointer) noexcept
	{
		return static_cast<WNDWindow *>(pointer);
	}

	static const WNDWindow *Window(const void *pointer) noexcept
	{
		return static_cast<const WNDWindow *>(pointer);
	}

	static void *Next(void *context, void *pointer) noexcept
	{
		DocumentRenderContext *render_context = static_cast<DocumentRenderContext *>(context);
		WNDWindow *window = Window(pointer);
		return window->next_sibling == Invalid_Node ? nullptr : const_cast<WNDWindow *>(
			&render_context->document->m_windows[window->next_sibling]);
	}

	static void *Child(void *context, void *pointer) noexcept
	{
		DocumentRenderContext *render_context = static_cast<DocumentRenderContext *>(context);
		const WNDWindow *window = Window(pointer);
		return window->first_child == Invalid_Node ? nullptr
			: const_cast<WNDWindow *>(&render_context->document->m_windows[window->first_child]);
	}

	static bool Describe(void *context, void *pointer, RenderNode &node) noexcept
	{
		DocumentRenderContext *render_context = static_cast<DocumentRenderContext *>(context);
		WNDWindow *window = Window(pointer);
		node.extract = &Extract;
		node.extract_context = const_cast<WNDDocument *>(render_context->document);
		node.layer = window->layer;
		node.visual_state = window->visual_state;
		node.flags = window->flags;
		node.screen_region = {
		static_cast<std::int32_t>(window->screen_region.left * render_context->scale_x),
		static_cast<std::int32_t>(window->screen_region.top * render_context->scale_y),
		static_cast<std::int32_t>(window->screen_region.right * render_context->scale_x),
		static_cast<std::int32_t>(window->screen_region.bottom * render_context->scale_y)};
		return true;
	}

	static bool Extract(void *context, void *pointer, void *, DrawList &draw_list) noexcept
	{
		const WNDDocument *document = static_cast<const WNDDocument *>(context);
		const WNDWindow &window = *Window(pointer);
		const std::size_t state_index = window.visual_state == VisualState::Disabled ? 1
			: window.visual_state == VisualState::Normal ? 0 : 2;
		const auto &state = window.draw_states[state_index];
		const float scale_x = document->m_last_scale_x;
		const float scale_y = document->m_last_scale_y;
		const Graphics::Rect2D rectangle{
			window.screen_region.left * scale_x, window.screen_region.top * scale_y,
			window.screen_region.right * scale_x, window.screen_region.bottom * scale_y};

		ControlVisual control;
		control.kind = ControlKind::Unknown;
		switch (window.type) {
		case WindowType::User: control.kind = ControlKind::User; break;
		case WindowType::PushButton: control.kind = ControlKind::PushButton; break;
		case WindowType::CheckBox: control.kind = ControlKind::CheckBox; break;
		case WindowType::RadioButton: control.kind = ControlKind::RadioButton; break;
		case WindowType::TabControl: control.kind = ControlKind::TabControl; break;
		case WindowType::ListBox: control.kind = ControlKind::ListBox; break;
		case WindowType::ComboBox: control.kind = ControlKind::ComboBox; break;
		case WindowType::HorizontalSlider: control.kind = ControlKind::HorizontalSlider; break;
		case WindowType::VerticalSlider: control.kind = ControlKind::VerticalSlider; break;
		case WindowType::ProgressBar: control.kind = ControlKind::ProgressBar; break;
		case WindowType::StaticText: control.kind = ControlKind::StaticText; break;
		case WindowType::TextEntry: control.kind = ControlKind::TextEntry; break;
		default: break;
		}
		control.rectangle = rectangle;
		control.state = &state;
		control.thumb_state = &window.thumb_draw_states[state_index];
		if (window.type == WindowType::ComboBox) {
			control.secondary_state = &window.combo_entry_draw_states[state_index];
			control.thumb_state = &window.combo_button_draw_states[state_index];
		}
		control.scale = (scale_x + scale_y) * 0.5f;
		control.minimum = window.minimum;
		control.maximum = window.maximum;
		control.position = window.position;
		control.progress = window.progress;
		control.list_length = window.list_length;
		control.list_columns = window.list_columns;
		control.checked = window.visual_state == VisualState::Selected;
		control.image_style = window.image_style;
		control.font = window.font;
		control.text = reinterpret_cast<const std::uint16_t *>(window.text.c_str());
		control.text_style = window.text_styles[state_index];
		control.centered_text = window.centered_text;
		control.centered_text_vertically = window.centered_text_vertically;
		if (!Render_Control(draw_list, control))
			return false;
		if (Has_Flag(window.flags, WindowFlag::Border)
			&& state.cells[0].border_color.alpha > 0.0f
			&& !draw_list.Add_Outline(rectangle, 1.0f, state.cells[0].border_color))
			return false;

		return true;
	}

	void Compute_Screen_Regions()
	{
		// Generals WND SCREENRECT values are authored in the document's
		// viewport, including child windows. They are not parent-relative
		// offsets; adding ancestors would move nested menu controls off-screen.
		for (WNDWindow &window : m_windows)
			window.screen_region = window.authored_region;
	}

	std::vector<WNDWindow> m_windows;
	std::vector<std::unique_ptr<FontFace>> m_fonts;
	int m_creation_width = 800;
	int m_creation_height = 600;
	float m_last_scale_x = 1.0f;
	float m_last_scale_y = 1.0f;
	WNDDocumentResolveReport m_report{};
};

}
