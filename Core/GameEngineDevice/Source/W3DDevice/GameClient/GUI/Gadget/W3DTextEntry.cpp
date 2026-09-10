/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#include "Precompiled/PreRTS.h"

#include <cstdint>

#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/IMEManager.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplayString.h"

import Engine.UI.WND;

namespace
{

Engine::UI::WND::ImageRef To_WND_Image(const Image *image) noexcept
{
	if (image == nullptr || image->getUV() == nullptr)
		return {};
	const Region2D *uv = image->getUV();
	Engine::UI::WND::ImageRef result =
		Engine::UI::WND::Resolve_Image_Reference(image->getFilename().str());
	result.uv = {uv->lo.x, uv->lo.y, uv->hi.x, uv->hi.y};
	return result;
}

Graphics::Color2D To_WND_Color(Color color) noexcept
{
	return {
		static_cast<float>((color >> 16) & 0xff) / 255.0f,
		static_cast<float>((color >> 8) & 0xff) / 255.0f,
		static_cast<float>(color & 0xff) / 255.0f,
		static_cast<float>((color >> 24) & 0xff) / 255.0f};
}

Engine::UI::WND::FontFace *Get_Font_Face(GameFont *font) noexcept
{
	return font != nullptr
		? static_cast<Engine::UI::WND::FontFace *>(font->fontData)
		: nullptr;
}

void Select_Colors(
	GameWindow *window,
	WinInstanceData *instance_data,
	Color &text,
	Color &text_border,
	Color &composite,
	Color &composite_border,
	Color &background,
	Color &background_border) noexcept
{
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
		composite = window->winGetDisabledTextColor();
		composite_border = window->winGetDisabledTextBorderColor();
		text = window->winGetDisabledTextColor();
		text_border = window->winGetDisabledTextBorderColor();
		background = GadgetTextEntryGetDisabledColor(window);
		background_border = GadgetTextEntryGetDisabledBorderColor(window);
	}
	else if (instance_data != nullptr && BitIsSet(instance_data->getState(), WIN_STATE_HILITED)) {
		composite = window->winGetIMECompositeTextColor();
		composite_border = window->winGetIMECompositeBorderColor();
		text = window->winGetHiliteTextColor();
		text_border = window->winGetHiliteTextBorderColor();
		background = GadgetTextEntryGetHiliteColor(window);
		background_border = GadgetTextEntryGetHiliteBorderColor(window);
	}
	else {
		composite = window->winGetIMECompositeTextColor();
		composite_border = window->winGetIMECompositeBorderColor();
		text = window->winGetEnabledTextColor();
		text_border = window->winGetEnabledTextBorderColor();
		background = GadgetTextEntryGetEnabledColor(window);
		background_border = GadgetTextEntryGetEnabledBorderColor(window);
	}
}

void Select_Images(
	GameWindow *window,
	WinInstanceData *instance_data,
	const Image *&left,
	const Image *&right,
	const Image *&center,
	const Image *&small_center) noexcept
{
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
		left = GadgetTextEntryGetDisabledImageLeft(window);
		right = GadgetTextEntryGetDisabledImageRight(window);
		center = GadgetTextEntryGetDisabledImageCenter(window);
		small_center = GadgetTextEntryGetDisabledImageSmallCenter(window);
	}
	else if (instance_data != nullptr && BitIsSet(instance_data->getState(), WIN_STATE_HILITED)) {
		left = GadgetTextEntryGetHiliteImageLeft(window);
		right = GadgetTextEntryGetHiliteImageRight(window);
		center = GadgetTextEntryGetHiliteImageCenter(window);
		small_center = GadgetTextEntryGetHiliteImageSmallCenter(window);
	}
	else {
		left = GadgetTextEntryGetEnabledImageLeft(window);
		right = GadgetTextEntryGetEnabledImageRight(window);
		center = GadgetTextEntryGetEnabledImageCenter(window);
		small_center = GadgetTextEntryGetEnabledImageSmallCenter(window);
	}
}

bool Extract_Text_Entry(
	GameWindow *window,
	WinInstanceData *instance_data,
	Engine::UI::WND::DrawList &draw_list,
	Bool use_images) noexcept
{
	if (window == nullptr || instance_data == nullptr)
		return false;
	EntryData *entry = static_cast<EntryData *>(window->winGetUserData());
	if (entry == nullptr || entry->text == nullptr || entry->sText == nullptr
		|| entry->constructText == nullptr)
		return true;

	entry->receivedUnichar = FALSE;
	entry->constructText->setText(UnicodeString::TheEmptyString);
	Int composition_cursor = 0;
	if (TheIMEManager != nullptr && TheIMEManager->isAttachedTo(window)
		&& TheIMEManager->isComposing()) {
		UnicodeString composition;
		TheIMEManager->getCompositionString(composition);
		if (entry->secretText) {
			entry->sText->setText(UnicodeString::TheEmptyString);
			const Int length = composition.getLength() + entry->text->getTextLength();
			for (Int index = 0; index < length; ++index)
				entry->sText->appendChar('*');
		}
		else {
			entry->constructText->setText(composition);
			composition_cursor = TheIMEManager->getCompositionCursorPosition();
		}
	}

	Int origin_x = 0;
	Int origin_y = 0;
	ICoord2D size;
	window->winGetScreenPosition(&origin_x, &origin_y);
	window->winGetSize(&size.x, &size.y);

	Color text_color = WIN_COLOR_UNDEFINED;
	Color text_border = WIN_COLOR_UNDEFINED;
	Color composite_color = WIN_COLOR_UNDEFINED;
	Color composite_border = WIN_COLOR_UNDEFINED;
	Color background = WIN_COLOR_UNDEFINED;
	Color background_border = WIN_COLOR_UNDEFINED;
	Select_Colors(window, instance_data, text_color, text_border,
		composite_color, composite_border, background, background_border);

	Engine::UI::WND::TextEntryVisual background_visual;
	background_visual.rectangle = {
		static_cast<float>(origin_x), static_cast<float>(origin_y),
		static_cast<float>(origin_x + size.x), static_cast<float>(origin_y + size.y)};
	background_visual.has_fill = background != WIN_COLOR_UNDEFINED;
	background_visual.fill = To_WND_Color(background);
	background_visual.has_border = background_border != WIN_COLOR_UNDEFINED;
	background_visual.border = To_WND_Color(background_border);
	if (use_images) {
		const Image *left = nullptr;
		const Image *right = nullptr;
		const Image *center = nullptr;
		const Image *small_center = nullptr;
		Select_Images(window, instance_data, left, right, center, small_center);
		if (left != nullptr && right != nullptr && center != nullptr && small_center != nullptr) {
			background_visual.segmented_image = true;
			background_visual.left_image = To_WND_Image(left);
			background_visual.right_image = To_WND_Image(right);
			background_visual.center_image = To_WND_Image(center);
			background_visual.small_center_image = To_WND_Image(small_center);
			background_visual.image_offset_x = static_cast<float>(instance_data->m_imageOffset.x);
			background_visual.image_offset_y = static_cast<float>(instance_data->m_imageOffset.y);
			background_visual.left_width = static_cast<float>(left->getImageWidth());
			background_visual.right_width = static_cast<float>(right->getImageWidth());
			background_visual.center_width = static_cast<float>(center->getImageWidth());
			background_visual.small_center_width = static_cast<float>(small_center->getImageWidth());
		}
	}
	if (!Engine::UI::WND::Add_Text_Entry_Background(draw_list, background_visual))
		return false;
	if (text_color == WIN_COLOR_UNDEFINED || sizeof(WideChar) != sizeof(std::uint16_t))
		return true;

	DisplayString *text = entry->secretText ? entry->sText : entry->text;
	if (text->getFont() != window->winGetFont())
		text->setFont(window->winGetFont());
	if (entry->constructText->getFont() != window->winGetFont())
		entry->constructText->setFont(window->winGetFont());
	const Int font_height = TheWindowManager->winFontHeight(instance_data->getFont());
	const Int start_offset = 5;
	const Int visible_width = size.x - (2 * start_offset);
	const Int text_x = origin_x + start_offset;
	const Int text_y = BitIsSet(window->winGetStatus(), WIN_STATUS_ONE_LINE)
		? size.y / 2 - font_height / 2
		: origin_y + start_offset;
	const Int text_width = text->getWidth();

	Engine::UI::WND::TextEntryTextVisual text_visual;
	text_visual.font = Get_Font_Face(text->getFont());
	text_visual.text = reinterpret_cast<const std::uint16_t *>(
		static_cast<W3DDisplayString *>(text)->getTextData());
	text_visual.composite_font = Get_Font_Face(entry->constructText->getFont());
	text_visual.composite_text = reinterpret_cast<const std::uint16_t *>(
		static_cast<W3DDisplayString *>(entry->constructText)->getTextData());
	text_visual.text_color = To_WND_Color(text_color);
	text_visual.text_drop_color = To_WND_Color(text_border);
	text_visual.composite_color = To_WND_Color(composite_color);
	text_visual.composite_drop_color = To_WND_Color(composite_border);
	text_visual.x = static_cast<float>(text_x);
	text_visual.y = static_cast<float>(text_y);
	text_visual.visible_width = static_cast<float>(visible_width);
	text_visual.font_height = static_cast<float>(font_height);
	text_visual.text_width = text_width;
	text_visual.draw_from_start = entry->drawTextFromStart != FALSE;
	text_visual.has_composite = entry->constructText->getTextLength() > 0;
	text_visual.composite_width = entry->constructText->getWidth();
	text_visual.composite_cursor_width = entry->constructText->getWidth(composition_cursor);
	if (!text_visual.draw_from_start) {
		text_visual.clip_rectangle = {
			static_cast<float>(text_x), static_cast<float>(text_y),
			static_cast<float>(text_x + visible_width),
			static_cast<float>(text_y + font_height)};
	}
	else {
		text_visual.clip_rectangle = {
			static_cast<float>(origin_x), static_cast<float>(origin_y),
			static_cast<float>(origin_x + size.x), static_cast<float>(origin_y + size.y)};
	}

	Int cursor_x = text_x;
	if (text_visual.draw_from_start) {
		cursor_x += 5 + text_width;
	}
	else if (text_width < visible_width) {
		cursor_x += 2 + text_width;
	}
	else if (visible_width > 1) {
		const Int half_width = visible_width / 2;
		const Int divisor = text_width / half_width - 1;
		cursor_x += 2 + text_width - divisor * half_width;
	}
	cursor_x += text_visual.composite_cursor_width;

	static Byte draw_count = 0;
	GameWindow *parent = window->winGetParent();
	if (parent != nullptr && !BitIsSet(parent->winGetStyle(), GWS_COMBO_BOX))
		parent = nullptr;
	text_visual.show_cursor = (window == TheWindowManager->winGetFocus()
		|| (parent != nullptr && parent == TheWindowManager->winGetFocus()))
		&& ((draw_count++ >> 3) & 0x1);
	text_visual.cursor_rectangle = {
		static_cast<float>(cursor_x), static_cast<float>(origin_y + 3),
		static_cast<float>(cursor_x + 2), static_cast<float>(origin_y + size.y - 3)};
	text_visual.cursor_color = To_WND_Color(text_color);
	window->winSetCursorPosition(cursor_x + 2 - origin_x, 0);
	return Engine::UI::WND::Add_Text_Entry_Text(draw_list, text_visual);
}

} // namespace

Bool W3DGadgetTextEntryDrawData(GameWindow *window, WinInstanceData *instData, void *drawList)
{
	return Extract_Text_Entry(
		window, instData, *static_cast<Engine::UI::WND::DrawList *>(drawList), FALSE) ? TRUE : FALSE;
}

Bool W3DGadgetTextEntryImageDrawData(GameWindow *window, WinInstanceData *instData, void *drawList)
{
	return Extract_Text_Entry(
		window, instData, *static_cast<Engine::UI::WND::DrawList *>(drawList), TRUE) ? TRUE : FALSE;
}

