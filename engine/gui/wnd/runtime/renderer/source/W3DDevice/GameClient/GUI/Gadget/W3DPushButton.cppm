/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

module;
#include "Precompiled/PreRTS.h"
#define ENGINE_UI_WND_RUNTIME_MODULE 1

#include "GameClient/Gadget.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetPushButton.h"
#include "W3DDevice/GameClient/W3DDisplayString.h"
#include "W3DDevice/GameClient/W3DGadget.h"

export module Engine.UI.WND.Runtime.Renderer.Gadget.PushButton;
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
	reference.uv = {uv->lo.x, uv->lo.y, uv->hi.x, uv->hi.y};
	return reference;
}

void Select_Button_Colors(
	GameWindow *window,
	WinInstanceData *instance_data,
	Color &color,
	Color &border) noexcept
{
	const Bool enabled = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED);
	const Bool highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	const Bool selected = BitIsSet(instance_data->getState(), WIN_STATE_SELECTED);
	if (!enabled) {
		color = selected ? GadgetButtonGetDisabledSelectedColor(window) : GadgetButtonGetDisabledColor(window);
		border = selected
			? GadgetButtonGetDisabledSelectedBorderColor(window)
			: GadgetButtonGetDisabledBorderColor(window);
	}
	else if (highlighted) {
		color = selected ? GadgetButtonGetHiliteSelectedColor(window) : GadgetButtonGetHiliteColor(window);
		border = selected
			? GadgetButtonGetHiliteSelectedBorderColor(window)
			: GadgetButtonGetHiliteBorderColor(window);
	}
	else {
		color = selected ? GadgetButtonGetEnabledSelectedColor(window) : GadgetButtonGetEnabledColor(window);
		border = selected
			? GadgetButtonGetEnabledSelectedBorderColor(window)
			: GadgetButtonGetEnabledBorderColor(window);
	}
}

void Add_Button_Extras(
	Engine::UI::WND::PushButtonVisual &visual,
	GameWindow *window,
	const Graphics::Rect2D &overlay_rectangle)
{
	PushButtonData *data = static_cast<PushButtonData *>(window->winGetUserData());
	if (data == nullptr)
		return;

	visual.overlay_rectangle = overlay_rectangle;
	visual.clock_rectangle = overlay_rectangle;
	visual.flashing_rectangle = overlay_rectangle;
	visual.state_rectangle = overlay_rectangle;
	if (data->overlayImage != nullptr) {
		visual.has_overlay = true;
		visual.overlay_image = To_WND_Image(data->overlayImage);
	}
	if (data->drawClock == NORMAL_CLOCK || data->drawClock == INVERSE_CLOCK) {
		visual.has_clock = true;
		visual.clock_percent = data->percentClock;
		visual.remaining_clock = data->drawClock == INVERSE_CLOCK;
		visual.clock_color = To_UI_Color(data->colorClock);
		data->drawClock = NO_CLOCK;
		window->winSetUserData(data);
	}
	if (data->drawBorder && data->colorBorder != GAME_COLOR_UNDEFINED) {
		visual.has_extra_border = true;
		visual.extra_border = {
			overlay_rectangle.left - 1.0f, overlay_rectangle.top - 1.0f,
			overlay_rectangle.right + 1.0f, overlay_rectangle.bottom + 1.0f};
		visual.extra_border_color = To_UI_Color(data->colorBorder);
	}
}

Engine::UI::WND::PushButtonVisual Build_Color_Button(
	GameWindow *window,
	WinInstanceData *instance_data)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	Color color = WIN_COLOR_UNDEFINED;
	Color border = WIN_COLOR_UNDEFINED;
	Select_Button_Colors(window, instance_data, color, border);

	Engine::UI::WND::PushButtonVisual visual;
	visual.rectangle = {
		static_cast<float>(origin.x), static_cast<float>(origin.y),
		static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)};
	visual.has_border = border != WIN_COLOR_UNDEFINED;
	visual.border_color = To_UI_Color(border);
	visual.has_fill = color != WIN_COLOR_UNDEFINED;
	visual.fill_color = To_UI_Color(color);
	Add_Button_Extras(visual, window, visual.rectangle);
	return visual;
}

const Image *Select_Button_Image(GameWindow *window, WinInstanceData *instance_data)
{
	const Image *image = GadgetButtonGetEnabledImage(window);
	if (BitIsSet(window->winGetStatus(), WIN_STATUS_USE_OVERLAY_STATES))
		return image;

	const Bool enabled = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED);
	const Bool highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	const Bool selected = BitIsSet(instance_data->getState(), WIN_STATE_SELECTED);
	if (!enabled)
		return selected ? GadgetButtonGetDisabledSelectedImage(window) : GadgetButtonGetDisabledImage(window);
	if (highlighted)
		return selected ? GadgetButtonGetHiliteSelectedImage(window) : GadgetButtonGetHiliteImage(window);
	return selected ? GadgetButtonGetHiliteSelectedImage(window) : image;
}

void Select_Button_Segment_Images(
	GameWindow *window,
	WinInstanceData *instance_data,
	const Image *&left,
	const Image *&middle,
	const Image *&right)
{
	const Bool enabled = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED);
	const Bool highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	const Bool selected = BitIsSet(instance_data->getState(), WIN_STATE_SELECTED);
	if (!enabled) {
		left = selected ? GadgetButtonGetLeftDisabledSelectedImage(window) : GadgetButtonGetLeftDisabledImage(window);
		middle = selected ? GadgetButtonGetMiddleDisabledSelectedImage(window) : GadgetButtonGetMiddleDisabledImage(window);
		right = selected ? GadgetButtonGetRightDisabledSelectedImage(window) : GadgetButtonGetRightDisabledImage(window);
	}
	else if (highlighted) {
		left = selected ? GadgetButtonGetLeftHiliteSelectedImage(window) : GadgetButtonGetLeftHiliteImage(window);
		middle = selected ? GadgetButtonGetMiddleHiliteSelectedImage(window) : GadgetButtonGetMiddleHiliteImage(window);
		right = selected ? GadgetButtonGetRightHiliteSelectedImage(window) : GadgetButtonGetRightHiliteImage(window);
	}
	else {
		left = selected ? GadgetButtonGetLeftEnabledSelectedImage(window) : GadgetButtonGetLeftEnabledImage(window);
		middle = selected ? GadgetButtonGetMiddleEnabledSelectedImage(window) : GadgetButtonGetMiddleEnabledImage(window);
		right = selected ? GadgetButtonGetRightEnabledSelectedImage(window) : GadgetButtonGetRightEnabledImage(window);
	}
}

Engine::UI::WND::PushButtonVisual Build_Image_Button(
	GameWindow *window,
	WinInstanceData *instance_data)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	const Graphics::Rect2D overlay_rectangle{
		static_cast<float>(origin.x), static_cast<float>(origin.y),
		static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)};
	const Graphics::Rect2D image_rectangle{
		static_cast<float>(origin.x + instance_data->m_imageOffset.x),
		static_cast<float>(origin.y + instance_data->m_imageOffset.y),
		static_cast<float>(origin.x + instance_data->m_imageOffset.x + size.x),
		static_cast<float>(origin.y + instance_data->m_imageOffset.y + size.y)};

	Engine::UI::WND::PushButtonVisual visual;
	visual.rectangle = image_rectangle;
	visual.image_color = {};
	visual.enabled = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED);
	visual.highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	visual.selected = BitIsSet(instance_data->getState(), WIN_STATE_SELECTED);
	visual.use_overlay_states = BitIsSet(window->winGetStatus(), WIN_STATUS_USE_OVERLAY_STATES);

	const Image *middle = GadgetButtonGetMiddleEnabledImage(window);
	if (middle != nullptr && !visual.use_overlay_states) {
		const Image *left = nullptr;
		const Image *right = nullptr;
		Select_Button_Segment_Images(window, instance_data, left, middle, right);
		if (left != nullptr && right != nullptr) {
			visual.segmented = true;
			visual.left_image = To_WND_Image(left);
			visual.middle_image = To_WND_Image(middle);
			visual.right_image = To_WND_Image(right);
			visual.left_width = static_cast<float>(left->getImageWidth());
			visual.middle_width = static_cast<float>(middle->getImageWidth());
			visual.right_width = static_cast<float>(right->getImageWidth());
		}
	}
	else {
		const Image *image = Select_Button_Image(window, instance_data);
		visual.has_image = image != nullptr;
		visual.image = To_WND_Image(image);
		if (visual.use_overlay_states && !visual.enabled
			&& !BitIsSet(window->winGetStatus(), WIN_STATUS_NOT_READY)) {
			if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ALWAYS_COLOR))
				visual.grayscale = true;
			else
				visual.image_color = {144.0f / 255.0f, 144.0f / 255.0f, 144.0f / 255.0f, 1.0f};
		}
	}

	Add_Button_Extras(visual, window, overlay_rectangle);
	if (visual.use_overlay_states && TheMappedImageCollection != nullptr) {
		const Image *pushed = TheMappedImageCollection->findImageByName("Cameo_push");
		const Image *highlighted = TheMappedImageCollection->findImageByName("Cameo_hilited");
		visual.has_pushed_overlay = pushed != nullptr;
		visual.pushed_overlay = To_WND_Image(pushed);
		visual.has_highlighted_overlay = highlighted != nullptr;
		visual.highlighted_overlay = To_WND_Image(highlighted);
	}
	visual.flashing = BitIsSet(window->winGetStatus(), WIN_STATUS_FLASHING);
	if (visual.flashing && TheMappedImageCollection != nullptr)
		visual.flashing_image = To_WND_Image(TheMappedImageCollection->findImageByName("Cameo_push"));
	return visual;
}

bool Append_Button_Text(
	Engine::UI::WND::DrawList &draw_list,
	GameWindow *window,
	WinInstanceData *instance_data)
{
	DisplayString *text = instance_data->getTextDisplayString();
	if (text == nullptr || text->getTextLength() == 0)
		return true;

	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);
	text->setWordWrapCentered(BitIsSet(instance_data->getStatus(), WIN_STATUS_WRAP_CENTERED));
	text->setWordWrap(size.x);
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
	ICoord2D text_position;
	if (BitIsSet(window->winGetStatus(), WIN_STATUS_SHORTCUT_BUTTON)) {
		text_position.x = origin.x + 2;
		text_position.y = origin.y;
	}
	else {
		text_position.x = origin.x + (size.x / 2) - (width / 2);
		text_position.y = origin.y + (size.y / 2) - (height / 2);
	}

	return static_cast<W3DDisplayString *>(text)->appendDrawData(
		draw_list, text_position.x, text_position.y, text_color, drop_color);
}

Bool Append_Button_Draw_Data(
	GameWindow *window,
	WinInstanceData *instance_data,
	void *opaque_draw_list,
	bool image_button)
{
	if (window == nullptr || instance_data == nullptr || opaque_draw_list == nullptr)
		return FALSE;

	Engine::UI::WND::DrawList &draw_list =
		*static_cast<Engine::UI::WND::DrawList *>(opaque_draw_list);
	const Engine::UI::WND::PushButtonVisual visual = image_button
		? Build_Image_Button(window, instance_data)
		: Build_Color_Button(window, instance_data);
	if (!Engine::UI::WND::Add_Push_Button_Background(draw_list, visual)
		|| !Append_Button_Text(draw_list, window, instance_data)
		|| !Engine::UI::WND::Add_Push_Button_Overlays(draw_list, visual))
		return FALSE;
	return TRUE;
}

}

Bool W3DGadgetPushButtonDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Button_Draw_Data(window, instance_data, draw_list, false);
}

Bool W3DGadgetPushButtonImageDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Button_Draw_Data(window, instance_data, draw_list, true);
}
