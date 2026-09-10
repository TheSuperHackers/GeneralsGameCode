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

#include "Common/GlobalData.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GadgetStaticText.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplayString.h"

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

Graphics::Rect2D To_Rect(const ICoord2D &origin, const ICoord2D &size) noexcept
{
	return {
		static_cast<float>(origin.x), static_cast<float>(origin.y),
		static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)};
}

bool Append_Static_Text_Draw_Data(
	GameWindow *window,
	WinInstanceData *instance_data,
	void *opaque_draw_list,
	bool image_background)
{
	if (window == nullptr || instance_data == nullptr || opaque_draw_list == nullptr)
		return false;

	TextData *text_data = static_cast<TextData *>(window->winGetUserData());
	if (text_data == nullptr)
		return true;

	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	Engine::UI::WND::StaticTextVisual visual;
	visual.rectangle = To_Rect(origin, size);
	visual.centered = text_data->centered != FALSE;
	visual.centered_vertically = text_data->centeredVertically != FALSE;
	visual.left_margin = static_cast<float>(text_data->leftMargin);
	visual.top_margin = static_cast<float>(text_data->topMargin);

	Color text_color = WIN_COLOR_UNDEFINED;
	Color drop_color = WIN_COLOR_UNDEFINED;
	if (BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED) == FALSE) {
		text_color = window->winGetDisabledTextColor();
		drop_color = window->winGetDisabledTextBorderColor();
	}
	else {
		text_color = window->winGetEnabledTextColor();
		drop_color = window->winGetEnabledTextBorderColor();
	}

	if (image_background) {
		const Image *image = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)
			? GadgetStaticTextGetEnabledImage(window)
			: GadgetStaticTextGetDisabledImage(window);
		if (image != nullptr) {
			visual.has_image = true;
			visual.image = To_WND_Image(image);
			visual.image_rectangle = {
				visual.rectangle.left + instance_data->m_imageOffset.x,
				visual.rectangle.top + instance_data->m_imageOffset.y,
				visual.rectangle.right + instance_data->m_imageOffset.x,
				visual.rectangle.bottom + instance_data->m_imageOffset.y};
		}
	}
	else {
		const Color fill = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)
			? GadgetStaticTextGetEnabledColor(window)
			: GadgetStaticTextGetDisabledColor(window);
		const Color border = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)
			? GadgetStaticTextGetEnabledBorderColor(window)
			: GadgetStaticTextGetDisabledBorderColor(window);
		visual.has_fill = fill != WIN_COLOR_UNDEFINED;
		visual.fill_color = To_UI_Color(fill);
		visual.has_border = border != WIN_COLOR_UNDEFINED;
		visual.border_color = To_UI_Color(border);
	}

	Engine::UI::WND::DrawList &draw_list =
		*static_cast<Engine::UI::WND::DrawList *>(opaque_draw_list);
	if (text_data->text == nullptr || text_color == WIN_COLOR_UNDEFINED)
		return Engine::UI::WND::Add_Static_Text_Background(draw_list, visual) ? true : false;

	text_data->text->setWordWrap(size.x - 10);
	text_data->text->setWordWrapCentered(
		BitIsSet(window->winGetStatus(), WIN_STATUS_WRAP_CENTERED));
	if (BitIsSet(window->winGetStatus(), WIN_STATUS_HOTKEY_TEXT) && TheGlobalData != nullptr)
		text_data->text->setUseHotkey(TRUE, TheGlobalData->m_hotKeyTextColor);
	else
		text_data->text->setUseHotkey(FALSE, 0);

	return static_cast<W3DDisplayString *>(text_data->text)->appendStaticTextDrawData(
		draw_list, visual, text_color, drop_color);
}

}

Bool W3DGadgetStaticTextDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Static_Text_Draw_Data(window, instance_data, draw_list, false) ? TRUE : FALSE;
}

Bool W3DGadgetStaticTextImageDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Static_Text_Draw_Data(window, instance_data, draw_list, true) ? TRUE : FALSE;
}
