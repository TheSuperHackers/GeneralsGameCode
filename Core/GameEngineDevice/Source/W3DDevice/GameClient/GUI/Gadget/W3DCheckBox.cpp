/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/

#include "Precompiled/PreRTS.h"

#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GameWindowGlobal.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplayString.h"

#include <algorithm>
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
	Color &box,
	Color &box_border)
{
	const Bool enabled = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED);
	const Bool highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	const Bool checked = BitIsSet(instance_data->getState(), WIN_STATE_SELECTED);
	if (!enabled) {
		background = GadgetCheckBoxGetDisabledColor(window);
		background_border = GadgetCheckBoxGetDisabledBorderColor(window);
		box = checked
			? GadgetCheckBoxGetDisabledCheckedBoxColor(window)
			: GadgetCheckBoxGetDisabledUncheckedBoxColor(window);
		box_border = checked
			? GadgetCheckBoxGetDisabledCheckedBoxBorderColor(window)
			: GadgetCheckBoxGetDisabledUncheckedBoxBorderColor(window);
	}
	else if (highlighted) {
		background = GadgetCheckBoxGetHiliteColor(window);
		background_border = GadgetCheckBoxGetHiliteBorderColor(window);
		box = checked
			? GadgetCheckBoxGetHiliteCheckedBoxColor(window)
			: GadgetCheckBoxGetHiliteUncheckedBoxColor(window);
		box_border = checked
			? GadgetCheckBoxGetHiliteCheckedBoxBorderColor(window)
			: GadgetCheckBoxGetHiliteUncheckedBoxBorderColor(window);
	}
	else {
		background = GadgetCheckBoxGetEnabledColor(window);
		background_border = GadgetCheckBoxGetEnabledBorderColor(window);
		box = checked
			? GadgetCheckBoxGetEnabledCheckedBoxColor(window)
			: GadgetCheckBoxGetEnabledUncheckedBoxColor(window);
		box_border = checked
			? GadgetCheckBoxGetEnabledCheckedBoxBorderColor(window)
			: GadgetCheckBoxGetEnabledUncheckedBoxBorderColor(window);
	}
}

const Image *Select_Image_Visual(GameWindow *window, WinInstanceData *instance_data)
{
	const Bool enabled = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED);
	const Bool highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	const Bool checked = BitIsSet(instance_data->getState(), WIN_STATE_SELECTED);
	if (!enabled)
		return checked
			? GadgetCheckBoxGetDisabledCheckedBoxImage(window)
			: GadgetCheckBoxGetDisabledUncheckedBoxImage(window);
	if (highlighted)
		return checked
			? GadgetCheckBoxGetHiliteCheckedBoxImage(window)
			: GadgetCheckBoxGetHiliteUncheckedBoxImage(window);
	return checked
		? GadgetCheckBoxGetEnabledCheckedBoxImage(window)
		: GadgetCheckBoxGetEnabledUncheckedBoxImage(window);
}

Engine::UI::WND::CheckBoxVisual Build_Check_Box_Visual(
	GameWindow *window,
	WinInstanceData *instance_data,
	bool image_visual)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	Engine::UI::WND::CheckBoxVisual visual;
	visual.rectangle = {
		static_cast<float>(origin.x), static_cast<float>(origin.y),
		static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)};
	visual.line_width = 1.0f;
	visual.checked = BitIsSet(instance_data->getState(), WIN_STATE_SELECTED);

	if (image_visual) {
		const Image *box_image = Select_Image_Visual(window, instance_data);
		visual.has_box_image = box_image != nullptr;
		visual.box_image = To_WND_Image(box_image);
		visual.box_image_rectangle = {
			visual.rectangle.left + instance_data->m_imageOffset.x,
			visual.rectangle.top + 3.0f,
			visual.rectangle.left + instance_data->m_imageOffset.x + size.y - 6.0f,
			visual.rectangle.top + size.y - 3.0f};
		return visual;
	}

	Color background = WIN_COLOR_UNDEFINED;
	Color background_border = WIN_COLOR_UNDEFINED;
	Color box = WIN_COLOR_UNDEFINED;
	Color box_border = WIN_COLOR_UNDEFINED;
	Select_Color_Visual(window, instance_data, background, background_border, box, box_border);
	const float check_offset = static_cast<float>(size.x / 16);
	visual.box_rectangle = {
		visual.rectangle.left + check_offset,
		visual.rectangle.top + static_cast<float>(size.y / 3),
		visual.rectangle.left + check_offset + static_cast<float>(size.y / 3),
		visual.rectangle.top + static_cast<float>(2 * size.y / 3)};
	visual.has_background_fill = background != WIN_COLOR_UNDEFINED;
	visual.background_fill = To_UI_Color(background);
	visual.has_background_border = background_border != WIN_COLOR_UNDEFINED;
	visual.background_border = To_UI_Color(background_border);
	visual.has_box_border = box_border != WIN_COLOR_UNDEFINED;
	visual.box_border = To_UI_Color(box_border);
	visual.check_color = To_UI_Color(box);
	return visual;
}

bool Append_Check_Box_Text(
	Engine::UI::WND::DrawList &draw_list,
	GameWindow *window,
	WinInstanceData *instance_data,
	const Engine::UI::WND::CheckBoxVisual &visual)
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
	const Graphics::Point2D position = Engine::UI::WND::Get_Check_Box_Text_Position(
		visual, static_cast<std::uint32_t>(std::max(0, height)));
	return static_cast<W3DDisplayString *>(text)->appendDrawData(
		draw_list,
		static_cast<Int>(position.x),
		static_cast<Int>(position.y),
		text_color,
		drop_color);
}

Bool Append_Check_Box_Draw_Data(
	GameWindow *window,
	WinInstanceData *instance_data,
	void *opaque_draw_list,
	bool image_visual)
{
	if (window == nullptr || instance_data == nullptr || opaque_draw_list == nullptr)
		return FALSE;
	Engine::UI::WND::DrawList &draw_list =
		*static_cast<Engine::UI::WND::DrawList *>(opaque_draw_list);
	const Engine::UI::WND::CheckBoxVisual visual =
		Build_Check_Box_Visual(window, instance_data, image_visual);
	if (!Engine::UI::WND::Add_Check_Box_Visual(draw_list, visual)
		|| !Append_Check_Box_Text(draw_list, window, instance_data, visual))
		return FALSE;
	return TRUE;
}

}

Bool W3DGadgetCheckBoxDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Check_Box_Draw_Data(window, instance_data, draw_list, false);
}

Bool W3DGadgetCheckBoxImageDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Check_Box_Draw_Data(window, instance_data, draw_list, true);
}
