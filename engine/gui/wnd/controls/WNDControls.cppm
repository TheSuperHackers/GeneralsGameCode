module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

export module Engine.UI.WND.Controls;

import Engine.UI.WND;

namespace Engine::UI::WND
{

// These are the control types authored by the Generals WND format.  The
// parser has its own WindowType because it also preserves unknown type names;
// this enum is intentionally usable by tools and hand-built test fixtures.
export enum class ControlKind : std::uint8_t
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

export struct ControlVisual final
{
	ControlKind kind = ControlKind::Unknown;
	Graphics::Rect2D rectangle{};
	const WNDDrawState *state = nullptr;
	const WNDDrawState *thumb_state = nullptr;
	const WNDDrawState *secondary_state = nullptr;
	float scale = 1.0f;
	int minimum = 0;
	int maximum = 100;
	int position = 50;
	int progress = 50;
	std::uint32_t list_length = 4;
	std::uint32_t list_columns = 1;
	bool checked = false;
	bool image_style = false;
	const FontFace *font = nullptr;
	const FontFace *hotkey_font = nullptr;
	const std::uint16_t *text = nullptr;
	TextStyle text_style{};
	bool centered_text = false;
	bool centered_text_vertically = true;
};

namespace WNDControlsDetail
{

const WNDDrawCell &Cell(const WNDDrawState &state, std::size_t index) noexcept
{
	return state.cells[(std::min)(index, WND_Draw_Cell_Count - 1)];
}

bool Has_Image(const WNDDrawState &state, std::size_t index) noexcept
{
	return Cell(state, index).image.texture.Is_Valid();
}

bool Add_Generic_Background(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	if (visual.state == nullptr)
		return false;
	const WNDDrawCell &cell = Cell(*visual.state, 0);
	if (cell.image.texture.Is_Valid()) {
		if (!draw_list.Add_Image(cell.image, visual.rectangle))
			return false;
	}
	else if (cell.color.alpha > 0.0f
		&& !draw_list.Add_Rect(visual.rectangle, cell.color)) {
		return false;
	}
	return cell.border_color.alpha <= 0.0f
		|| draw_list.Add_Outline(visual.rectangle, 1.0f, cell.border_color);
}

struct ListContext final
{
	Graphics::Rect2D rectangle{};
	std::uint32_t rows = 4;
};

bool Query_List_Row(void *opaque, std::uint32_t row, ListBoxRowVisual &visual) noexcept
{
	const auto &context = *static_cast<const ListContext *>(opaque);
	const float height = (context.rectangle.bottom - context.rectangle.top)
		/ static_cast<float>((std::max)(1u, context.rows));
	visual.rectangle = {
		context.rectangle.left,
		context.rectangle.top + height * static_cast<float>(row),
		context.rectangle.right,
		context.rectangle.top + height * static_cast<float>(row + 1)};
	visual.selected = row == 1;
	return true;
}

bool Emit_List_Cell(
	void *, DrawList &, std::uint32_t, std::uint32_t,
	Graphics::Rect2D, Graphics::Rect2D) noexcept
{
	// A WND list box receives its row text from the runtime list model.  The
	// visual regression gallery deliberately leaves those cells empty so that
	// the atlas-backed selection treatment is tested independently.
	return true;
}

bool Render_User(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	return Add_Generic_Background(draw_list, visual);
}

bool Render_Push_Button(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	if (visual.state == nullptr)
		return false;
	const WNDDrawState &state = *visual.state;
	PushButtonVisual button;
	button.rectangle = visual.rectangle;
	button.image_color = {1.0f, 1.0f, 1.0f, 1.0f};
	if (Has_Image(state, 0) && Has_Image(state, 5) && Has_Image(state, 6)) {
		button.segmented = true;
		button.left_image = Cell(state, 0).image;
		button.middle_image = Cell(state, 5).image;
		button.right_image = Cell(state, 6).image;
		button.left_width = Cell(state, 0).image_width * visual.scale;
		button.middle_width = Cell(state, 5).image_width * visual.scale;
		button.right_width = Cell(state, 6).image_width * visual.scale;
	}
	else if (Has_Image(state, 0)) {
		button.has_image = true;
		button.image = Cell(state, 0).image;
	}
	else {
		button.has_fill = Cell(state, 0).color.alpha > 0.0f;
		button.fill_color = Cell(state, 0).color;
		button.has_border = Cell(state, 0).border_color.alpha > 0.0f;
		button.border_color = Cell(state, 0).border_color;
	}
	return Add_Push_Button_Background(draw_list, button)
		&& Add_Push_Button_Overlays(draw_list, button);
}

bool Render_Check_Box(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	if (visual.state == nullptr)
		return false;
	const WNDDrawState &state = *visual.state;
	const float box_size = (std::max)(1.0f,
		visual.rectangle.bottom - visual.rectangle.top);
	CheckBoxVisual box;
	box.rectangle = visual.rectangle;
	box.box_rectangle = {
		visual.rectangle.left,
		visual.rectangle.top,
		visual.rectangle.left + box_size,
		visual.rectangle.top + box_size};
	box.checked = visual.checked;
	box.line_width = visual.scale;
	if (Has_Image(state, 0)) {
		box.has_background_image = true;
		box.background_image = Cell(state, 0).image;
		box.background_image_rectangle = visual.rectangle;
	}
	else {
		box.has_background_fill = Cell(state, 0).color.alpha > 0.0f;
		box.background_fill = Cell(state, 0).color;
		box.has_background_border = Cell(state, 0).border_color.alpha > 0.0f;
		box.background_border = Cell(state, 0).border_color;
	}
	if (Has_Image(state, visual.checked ? 2 : 1)) {
		box.has_box_image = true;
		box.box_image = Cell(state, visual.checked ? 2 : 1).image;
		box.box_image_rectangle = box.box_rectangle;
	}
	else {
		box.has_box_border = true;
		box.box_border = Cell(state, 1).border_color;
		box.has_box_fill = Cell(state, 1).color.alpha > 0.0f;
		box.box_fill = Cell(state, 1).color;
	}
	box.check_color = Cell(state, 2).border_color;
	return Add_Check_Box_Visual(draw_list, box);
}

bool Render_Radio_Button(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	if (visual.state == nullptr)
		return false;
	const WNDDrawState &state = *visual.state;
	RadioButtonVisual radio;
	radio.rectangle = visual.rectangle;
	radio.middle_width = Has_Image(state, 1)
		? Cell(state, 1).image_width * visual.scale : 0.0f;
	if (Has_Image(state, 0) && Has_Image(state, 1) && Has_Image(state, 2)) {
		radio.segmented_images = true;
		radio.left_image = Cell(state, 0).image;
		radio.middle_image = Cell(state, 1).image;
		radio.right_image = Cell(state, 2).image;
		const float left = visual.rectangle.left;
		const float right = visual.rectangle.right;
		const float left_width = Cell(state, 0).image_width * visual.scale;
		const float right_width = Cell(state, 2).image_width * visual.scale;
		radio.left_image_rectangle = {left, visual.rectangle.top,
		left + left_width, visual.rectangle.bottom};
		radio.right_image_rectangle = {right - right_width, visual.rectangle.top,
		right, visual.rectangle.bottom};
		radio.middle_image_rectangle = {radio.left_image_rectangle.right,
		visual.rectangle.top, radio.right_image_rectangle.left, visual.rectangle.bottom};
	}
	else {
		radio.has_background_fill = Cell(state, 0).color.alpha > 0.0f;
		radio.background_fill = Cell(state, 0).color;
		radio.has_background_border = Cell(state, 0).border_color.alpha > 0.0f;
		radio.background_border = Cell(state, 0).border_color;
		const float half = (visual.rectangle.right - visual.rectangle.left) * 0.5f;
		radio.left_box_rectangle = {visual.rectangle.left, visual.rectangle.top,
		visual.rectangle.left + half - 1.0f, visual.rectangle.bottom};
		radio.right_box_rectangle = {visual.rectangle.left + half + 1.0f,
		visual.rectangle.top, visual.rectangle.right, visual.rectangle.bottom};
		radio.has_box_fill = visual.checked;
		radio.box_fill = Cell(state, 1).color;
	}
	return Add_Radio_Button_Visual(draw_list, radio);
}

bool Render_Progress_Bar(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	if (visual.state == nullptr)
		return false;
	const WNDDrawState &state = *visual.state;
	if (Has_Image(state, 0) && Has_Image(state, 1) && Has_Image(state, 2)) {
		ProgressBarImageVisual image;
		image.rectangle = visual.rectangle;
		const float left_width = Cell(state, 0).image_width * visual.scale;
		const float right_width = Cell(state, 1).image_width * visual.scale;
		image.background_left = Cell(state, 0).image;
		image.background_center = Cell(state, 2).image;
		image.background_right = Cell(state, 1).image;
		image.background_left_rectangle = {visual.rectangle.left, visual.rectangle.top,
			visual.rectangle.left + left_width, visual.rectangle.bottom};
		image.background_right_rectangle = {visual.rectangle.right - right_width,
			visual.rectangle.top, visual.rectangle.right, visual.rectangle.bottom};
		image.background_center_rectangle = {image.background_left_rectangle.right,
			visual.rectangle.top, image.background_right_rectangle.left, visual.rectangle.bottom};
		image.background_center_width = (std::max)(1.0f,
			Cell(state, 2).image_width * visual.scale);
		image.bar_center = Has_Image(state, 6) ? Cell(state, 6).image : Cell(state, 2).image;
		image.bar_right = image.bar_center;
		const float progress = (std::clamp(visual.progress, 0, 100) / 100.0f);
		image.bar_rectangle = {image.background_center_rectangle.left,
			visual.rectangle.top,
			image.background_center_rectangle.left
				+ (image.background_center_rectangle.right - image.background_center_rectangle.left) * progress,
			visual.rectangle.bottom};
		image.bar_center_width = (std::max)(1.0f, Cell(state, 6).image_width * visual.scale);
		image.bar_right_width = image.bar_center_width;
		return Add_Progress_Bar_Image_Visual(draw_list, image);
	}
	ProgressBarVisual bar;
	bar.rectangle = visual.rectangle;
	bar.progress = visual.progress;
	bar.has_background_fill = Cell(state, 0).color.alpha > 0.0f;
	bar.background_fill = Cell(state, 0).color;
	bar.has_background_border = Cell(state, 0).border_color.alpha > 0.0f;
	bar.background_border = Cell(state, 0).border_color;
	bar.has_bar_fill = true;
	bar.bar_fill = Cell(state, 4).color;
	return Add_Progress_Bar_Visual(draw_list, bar);
}

bool Render_Slider(DrawList &draw_list, const ControlVisual &visual, bool vertical) noexcept
{
	if (visual.state == nullptr)
		return false;
	const WNDDrawState &state = *visual.state;
	SliderVisual slider;
	slider.rectangle = visual.rectangle;
	slider.has_background_fill = Cell(state, 0).color.alpha > 0.0f;
	slider.background_fill = Cell(state, 0).color;
	slider.has_background_border = Cell(state, 0).border_color.alpha > 0.0f;
	slider.background_border = Cell(state, 0).border_color;
	if (!Add_Slider_Visual(draw_list, slider))
		return false;
	if (visual.thumb_state == nullptr || !Has_Image(*visual.thumb_state, 0))
		return true;
	const WNDDrawCell &thumb = Cell(*visual.thumb_state, 0);
	const float ratio = visual.maximum > visual.minimum
		? static_cast<float>(visual.position - visual.minimum)
			/ static_cast<float>(visual.maximum - visual.minimum) : 0.0f;
	const float width = thumb.image_width * visual.scale;
	const float height = thumb.image_height * visual.scale;
	Graphics::Rect2D target = visual.rectangle;
	if (vertical) {
		target.left = visual.rectangle.left + (visual.rectangle.right - visual.rectangle.left - width) * 0.5f;
		target.right = target.left + width;
		target.top = visual.rectangle.top + (visual.rectangle.bottom - visual.rectangle.top - height) * ratio;
		target.bottom = target.top + height;
	}
	else {
		target.left = visual.rectangle.left + (visual.rectangle.right - visual.rectangle.left - width) * ratio;
		target.right = target.left + width;
		target.top = visual.rectangle.top + (visual.rectangle.bottom - visual.rectangle.top - height) * 0.5f;
		target.bottom = target.top + height;
	}
	return draw_list.Add_Image(thumb.image, target);
}

bool Render_Text_Entry(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	const WNDDrawState *source = visual.secondary_state != nullptr
		? visual.secondary_state : visual.state;
	if (source == nullptr)
		return false;
	const WNDDrawState &state = *source;
	TextEntryVisual entry;
	entry.rectangle = visual.rectangle;
	if (Has_Image(state, 0) && Has_Image(state, 1) && Has_Image(state, 2)) {
		entry.segmented_image = true;
		entry.left_image = Cell(state, 0).image;
		entry.right_image = Cell(state, 1).image;
		entry.center_image = Cell(state, 2).image;
		entry.small_center_image = Cell(state, 3).image;
		entry.left_width = Cell(state, 0).image_width * visual.scale;
		entry.right_width = Cell(state, 1).image_width * visual.scale;
		entry.center_width = (std::max)(1u, Cell(state, 2).image_width) * visual.scale;
		entry.small_center_width = (std::max)(1u, Cell(state, 3).image_width) * visual.scale;
	}
	else {
		entry.has_fill = Cell(state, 0).color.alpha > 0.0f;
		entry.fill = Cell(state, 0).color;
		entry.has_border = Cell(state, 0).border_color.alpha > 0.0f;
		entry.border = Cell(state, 0).border_color;
	}
	return Add_Text_Entry_Background(draw_list, entry);
}

bool Render_Combo_Box(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	if (visual.state == nullptr)
		return false;
	if (!Render_Text_Entry(draw_list, visual))
		return false;
	if (visual.thumb_state == nullptr)
		return true;
	const WNDDrawState &button = *visual.thumb_state;
	const float width = (std::max)(1.0f,
		Cell(button, 0).image_width * visual.scale);
	const Graphics::Rect2D rectangle{
		visual.rectangle.right - width,
		visual.rectangle.top,
		visual.rectangle.right,
		visual.rectangle.bottom};
	if (Has_Image(button, 0))
		return draw_list.Add_Image(Cell(button, 0).image, rectangle);
	if (Cell(button, 0).color.alpha > 0.0f
		&& !draw_list.Add_Rect(rectangle, Cell(button, 0).color))
		return false;
	return Cell(button, 0).border_color.alpha <= 0.0f
		|| draw_list.Add_Outline(rectangle, 1.0f, Cell(button, 0).border_color);
}

bool Render_Tab_Control(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	if (visual.state == nullptr)
		return false;
	const WNDDrawState &state = *visual.state;
	TabControlVisual tabs;
	tabs.background_image_rectangle = visual.rectangle;
	if (Has_Image(state, 0)) {
		tabs.has_background_image = true;
		tabs.background_image = Cell(state, 0).image;
	}
	else {
		tabs.has_background_fill = Cell(state, 0).color.alpha > 0.0f;
		tabs.background_fill = Cell(state, 0).color;
		tabs.has_background_border = Cell(state, 0).border_color.alpha > 0.0f;
		tabs.background_border = Cell(state, 0).border_color;
	}
	tabs.count = 3;
	const float width = (visual.rectangle.right - visual.rectangle.left) / 3.0f;
	for (std::size_t index = 0; index != tabs.count; ++index) {
		tabs.rectangles[index] = {
			visual.rectangle.left + width * static_cast<float>(index),
			visual.rectangle.top,
			visual.rectangle.left + width * static_cast<float>(index + 1),
			visual.rectangle.bottom};
		const std::size_t cell_index = index + 1;
		if (Has_Image(state, cell_index)) {
			tabs.has_images[index] = true;
			tabs.images[index] = Cell(state, cell_index).image;
		}
		else {
			tabs.has_fills[index] = Cell(state, cell_index).color.alpha > 0.0f;
			tabs.fills[index] = Cell(state, cell_index).color;
			tabs.has_borders[index] = Cell(state, cell_index).border_color.alpha > 0.0f;
			tabs.borders[index] = Cell(state, cell_index).border_color;
		}
	}
	return Add_Tab_Control_Visual(draw_list, tabs);
}

bool Render_List_Box(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	if (visual.state == nullptr)
		return false;
	const WNDDrawState &state = *visual.state;
	ListContext context{visual.rectangle, (std::max)(1u, visual.list_length)};
	ListBoxVisual list;
	list.clip_rectangle = visual.rectangle;
	list.row_count = context.rows;
	list.column_count = (std::max)(1u, visual.list_columns);
	list.context = &context;
	list.query_row = &Query_List_Row;
	list.emit_cell = &Emit_List_Cell;
	list.selection.rectangle = visual.rectangle;
	list.selection.segmented_image = Has_Image(state, 1)
		&& Has_Image(state, 2) && Has_Image(state, 3) && Has_Image(state, 4);
	list.selection.left_image = Cell(state, 1).image;
	list.selection.center_image = Cell(state, 3).image;
	list.selection.small_center_image = Cell(state, 4).image;
	list.selection.right_image = Cell(state, 2).image;
	list.selection.left_width = Cell(state, 1).image_width * visual.scale;
	list.selection.center_width = Cell(state, 3).image_width * visual.scale;
	list.selection.small_center_width = Cell(state, 4).image_width * visual.scale;
	list.selection.right_width = Cell(state, 2).image_width * visual.scale;
	list.selection.has_fill = Cell(state, 0).color.alpha > 0.0f;
	list.selection.fill = Cell(state, 0).color;
	list.selection.has_border = Cell(state, 0).border_color.alpha > 0.0f;
	list.selection.border = Cell(state, 0).border_color;
	return Add_List_Box_Visual(draw_list, list);
}

bool Render_Control_Text(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	if (visual.font == nullptr || visual.text == nullptr || *visual.text == 0)
		return true;

	StaticTextVisual text_visual;
	text_visual.rectangle = visual.rectangle;
	text_visual.centered = visual.centered_text || visual.kind == ControlKind::PushButton;
	text_visual.centered_vertically = visual.centered_text_vertically;
	if (visual.kind == ControlKind::CheckBox)
		text_visual.left_margin = visual.rectangle.bottom - visual.rectangle.top;

	StaticTextContent content;
	content.font = visual.font;
	content.hotkey_font = visual.hotkey_font;
	content.text = visual.text;
	content.options.parse_hotkey = visual.kind == ControlKind::PushButton;
	content.style = visual.text_style;
	return Add_Static_Text(draw_list, text_visual, content);
}

}

export bool Render_Control(DrawList &draw_list, const ControlVisual &visual) noexcept
{
	bool rendered = false;
	switch (visual.kind) {
	case ControlKind::PushButton: rendered = WNDControlsDetail::Render_Push_Button(draw_list, visual); break;
	case ControlKind::CheckBox: rendered = WNDControlsDetail::Render_Check_Box(draw_list, visual); break;
	case ControlKind::RadioButton: rendered = WNDControlsDetail::Render_Radio_Button(draw_list, visual); break;
	case ControlKind::ListBox: rendered = WNDControlsDetail::Render_List_Box(draw_list, visual); break;
	case ControlKind::ProgressBar: rendered = WNDControlsDetail::Render_Progress_Bar(draw_list, visual); break;
	case ControlKind::HorizontalSlider: rendered = WNDControlsDetail::Render_Slider(draw_list, visual, false); break;
	case ControlKind::VerticalSlider: rendered = WNDControlsDetail::Render_Slider(draw_list, visual, true); break;
	case ControlKind::TextEntry: rendered = WNDControlsDetail::Render_Text_Entry(draw_list, visual); break;
	case ControlKind::ComboBox: rendered = WNDControlsDetail::Render_Combo_Box(draw_list, visual); break;
	case ControlKind::TabControl: rendered = WNDControlsDetail::Render_Tab_Control(draw_list, visual); break;
	case ControlKind::StaticText:
	case ControlKind::User:
	case ControlKind::Unknown:
	default: rendered = WNDControlsDetail::Render_User(draw_list, visual); break;
	}
	return rendered && WNDControlsDetail::Render_Control_Text(draw_list, visual);
}

}
