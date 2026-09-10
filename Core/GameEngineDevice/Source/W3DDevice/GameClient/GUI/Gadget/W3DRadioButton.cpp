/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
*/

#include "Precompiled/PreRTS.h"

#include "GameClient/GadgetRadioButton.h"
#include "GameClient/GameWindowGlobal.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplayString.h"

#include <cstdint>

import Engine.UI.WND;

namespace
{

Graphics::Color2D To_UI_Color(Color color) noexcept
{
	return {
		static_cast<float>((color >> 16) & 0xff) / 255.0f,
		static_cast<float>((color >> 8) & 0xff) / 255.0f,
		static_cast<float>(color & 0xff) / 255.0f,
		static_cast<float>((color >> 24) & 0xff) / 255.0f};
}

Engine::UI::WND::ImageRef To_WND_Image(const Image *image)
{
	if (image == nullptr || image->getUV() == nullptr)
		return {};
	const Region2D *uv = image->getUV();
	Engine::UI::WND::ImageRef reference =
		Engine::UI::WND::Resolve_Image_Reference(image->getFilename().str());
	if (uv != nullptr)
		reference.uv = {uv->lo.x, uv->lo.y, uv->hi.x, uv->hi.y};
	return reference;
}

void Select_Color_Visual(
	GameWindow *window,
	WinInstanceData *instance_data,
	Color &background,
	Color &background_border,
	Color &box)
{
	const Bool enabled = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED);
	const Bool highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	const Bool checked = BitIsSet(instance_data->getState(), WIN_STATE_SELECTED);
	if (!enabled) {
		background = GadgetRadioGetDisabledColor(window);
		background_border = GadgetRadioGetDisabledBorderColor(window);
		box = checked
			? GadgetRadioGetDisabledCheckedBoxColor(window)
			: GadgetRadioGetDisabledUncheckedBoxColor(window);
	}
	else if (highlighted) {
		background = GadgetRadioGetHiliteColor(window);
		background_border = GadgetRadioGetHiliteBorderColor(window);
		box = checked
			? GadgetRadioGetHiliteCheckedBoxColor(window)
			: GadgetRadioGetHiliteUncheckedBoxColor(window);
	}
	else {
		background = GadgetRadioGetEnabledColor(window);
		background_border = GadgetRadioGetEnabledBorderColor(window);
		box = checked
			? GadgetRadioGetEnabledCheckedBoxColor(window)
			: GadgetRadioGetEnabledUncheckedBoxColor(window);
	}
}

void Select_Image_Visual(
	GameWindow *window,
	WinInstanceData *instance_data,
	const Image *&left,
	const Image *&middle,
	const Image *&right)
{
	const Bool selected = BitIsSet(instance_data->getState(), WIN_STATE_SELECTED);
	const Bool enabled = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED);
	const Bool highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	if (selected) {
		left = GadgetRadioGetSelectedImage(window);
		middle = GadgetRadioGetSelectedUncheckedBoxImage(window);
		right = GadgetRadioGetSelectedCheckedBoxImage(window);
	}
	else if (!enabled) {
		left = GadgetRadioGetDisabledImage(window);
		middle = GadgetRadioGetDisabledUncheckedBoxImage(window);
		right = GadgetRadioGetDisabledCheckedBoxImage(window);
	}
	else if (highlighted) {
		left = GadgetRadioGetHiliteImage(window);
		middle = GadgetRadioGetHiliteUncheckedBoxImage(window);
		right = GadgetRadioGetHiliteCheckedBoxImage(window);
	}
	else {
		left = GadgetRadioGetEnabledImage(window);
		middle = GadgetRadioGetEnabledUncheckedBoxImage(window);
		right = GadgetRadioGetEnabledCheckedBoxImage(window);
	}
}

Engine::UI::WND::RadioButtonVisual Build_Color_Visual(
	GameWindow *window,
	WinInstanceData *instance_data)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	Color background = WIN_COLOR_UNDEFINED;
	Color background_border = WIN_COLOR_UNDEFINED;
	Color box = WIN_COLOR_UNDEFINED;
	Select_Color_Visual(window, instance_data, background, background_border, box);

	Engine::UI::WND::RadioButtonVisual visual;
	visual.rectangle = {
		static_cast<float>(origin.x), static_cast<float>(origin.y),
		static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)};
	visual.line_width = WIN_DRAW_LINE_WIDTH;
	visual.has_background_fill = background != WIN_COLOR_UNDEFINED;
	visual.background_fill = To_UI_Color(background);
	visual.has_background_border = background_border != WIN_COLOR_UNDEFINED;
	visual.background_border = To_UI_Color(background_border);
	visual.has_box_fill = box != WIN_COLOR_UNDEFINED;
	visual.box_fill = To_UI_Color(box);
	visual.left_box_rectangle = {
		visual.rectangle.left + 1.0f,
		visual.rectangle.top + 1.0f,
		visual.rectangle.left + static_cast<float>(size.y) - 1.0f,
		visual.rectangle.bottom - 1.0f};
	visual.right_box_rectangle = {
		visual.rectangle.right - static_cast<float>(size.y),
		visual.rectangle.top + 1.0f,
		visual.rectangle.right - 1.0f,
		visual.rectangle.bottom - 1.0f};
	return visual;
}

Engine::UI::WND::RadioButtonVisual Build_Image_Visual(
	GameWindow *window,
	WinInstanceData *instance_data)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	const Image *left = nullptr;
	const Image *middle = nullptr;
	const Image *right = nullptr;
	Select_Image_Visual(window, instance_data, left, middle, right);

	Engine::UI::WND::RadioButtonVisual visual;
	visual.segmented_images = left != nullptr && middle != nullptr && right != nullptr;
	if (!visual.segmented_images)
		return visual;

	const float x_offset = static_cast<float>(instance_data->m_imageOffset.x);
	const float y_offset = static_cast<float>(instance_data->m_imageOffset.y);
	const float left_end = static_cast<float>(origin.x) + left->getImageWidth() + x_offset;
	const float right_start = static_cast<float>(origin.x + size.x)
		- right->getImageWidth() + x_offset;
	visual.left_image = To_WND_Image(left);
	visual.middle_image = To_WND_Image(middle);
	visual.right_image = To_WND_Image(right);
	visual.left_image_rectangle = {
		static_cast<float>(origin.x) + x_offset,
		static_cast<float>(origin.y) + y_offset,
		left_end,
		static_cast<float>(origin.y + size.y) + y_offset};
	visual.middle_image_rectangle = {
		left_end,
		static_cast<float>(origin.y) + y_offset,
		right_start,
		static_cast<float>(origin.y + size.y) + y_offset};
	visual.right_image_rectangle = {
		right_start,
		static_cast<float>(origin.y) + y_offset,
		static_cast<float>(origin.x + size.x),
		static_cast<float>(origin.y + size.y) + y_offset};
	visual.middle_width = static_cast<float>(middle->getImageWidth());
	return visual;
}

bool Append_Radio_Button_Text(
	Engine::UI::WND::DrawList &draw_list,
	GameWindow *window,
	WinInstanceData *instance_data,
	const Engine::UI::WND::RadioButtonVisual &visual)
{
	DisplayString *text = instance_data->getTextDisplayString();
	if (text == nullptr || text->getTextLength() == 0)
		return true;
	if (text->getFont() != window->winGetFont())
		text->setFont(window->winGetFont());

	Color text_color;
	Color drop_color;
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
		text_color = window->winGetDisabledTextColor();
		drop_color = window->winGetDisabledTextBorderColor();
	}
	else if (BitIsSet(instance_data->getState(), WIN_STATE_HILITED)) {
		text_color = window->winGetHiliteTextColor();
		drop_color = window->winGetHiliteTextBorderColor();
	}
	else {
		text_color = window->winGetEnabledTextColor();
		drop_color = window->winGetEnabledTextBorderColor();
	}

	Int width = 0;
	Int height = 0;
	text->getSize(&width, &height);
	const Graphics::Point2D position = Engine::UI::WND::Get_Radio_Button_Text_Position(
		visual,
		static_cast<std::uint32_t>(width > 0 ? width : 0),
		static_cast<std::uint32_t>(height > 0 ? height : 0));
	return static_cast<W3DDisplayString *>(text)->appendDrawData(
		draw_list,
		static_cast<Int>(position.x),
		static_cast<Int>(position.y),
		text_color,
		drop_color);
}

Bool Append_Radio_Button_Draw_Data(
	GameWindow *window,
	WinInstanceData *instance_data,
	void *opaque_draw_list,
	bool image_visual)
{
	if (window == nullptr || instance_data == nullptr || opaque_draw_list == nullptr)
		return FALSE;
	Engine::UI::WND::DrawList &draw_list =
		*static_cast<Engine::UI::WND::DrawList *>(opaque_draw_list);
	const Engine::UI::WND::RadioButtonVisual visual = image_visual
		? Build_Image_Visual(window, instance_data)
		: Build_Color_Visual(window, instance_data);
	if (image_visual && !visual.segmented_images)
		return TRUE;
	if (!Engine::UI::WND::Add_Radio_Button_Visual(draw_list, visual))
		return FALSE;
	return Append_Radio_Button_Text(draw_list, window, instance_data, visual) ? TRUE : FALSE;
}

}

Bool W3DGadgetRadioButtonDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Radio_Button_Draw_Data(window, instance_data, draw_list, false);
}

Bool W3DGadgetRadioButtonImageDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Radio_Button_Draw_Data(window, instance_data, draw_list, true);
}
