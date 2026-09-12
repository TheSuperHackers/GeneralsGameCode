/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
*/

module;
#include "Precompiled/PreRTS.h"
#define ENGINE_UI_WND_RUNTIME_MODULE 1

#include "GameClient/GadgetTabControl.h"
#include "GameClient/GameWindowGlobal.h"
#include "W3DDevice/GameClient/W3DGadget.h"

#include <algorithm>

export module Engine.UI.WND.Runtime.Renderer.Gadget.TabControl;
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
	Engine::UI::WND::ImageRef reference =
		Engine::UI::WND::Resolve_Image_Reference(image->getFilename().str());
	const Region2D *uv = image->getUV();
	if (uv != nullptr)
		reference.uv = {uv->lo.x, uv->lo.y, uv->hi.x, uv->hi.y};
	return reference;
}

Engine::UI::WND::TabControlVisual Build_Visual(
	GameWindow *window,
	WinInstanceData *instance_data,
	bool image_visual)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	Engine::UI::WND::TabControlVisual visual;
	visual.background_image_rectangle = {
		static_cast<float>(origin.x), static_cast<float>(origin.y),
		static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)};
	if (image_visual) {
		const Image *background = nullptr;
		if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED))
			background = window->winGetDisabledImage(0);
		else if (BitIsSet(instance_data->getState(), WIN_STATE_HILITED))
			background = window->winGetHiliteImage(0);
		else
			background = window->winGetEnabledImage(0);
		visual.has_background_image = background != nullptr;
		visual.background_image = To_WND_Image(background);
	}
	else {
		Color fill = WIN_COLOR_UNDEFINED;
		Color border = WIN_COLOR_UNDEFINED;
		if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
			fill = window->winGetDisabledColor(0);
			border = window->winGetDisabledBorderColor(0);
		}
		else if (BitIsSet(instance_data->getState(), WIN_STATE_HILITED)) {
			fill = window->winGetHiliteColor(0);
			border = window->winGetHiliteBorderColor(0);
		}
		else {
			fill = window->winGetEnabledColor(0);
			border = window->winGetEnabledBorderColor(0);
		}
		visual.has_background_fill = fill != WIN_COLOR_UNDEFINED;
		visual.background_fill = To_UI_Color(fill);
		visual.has_background_border = border != WIN_COLOR_UNDEFINED;
		visual.background_border = To_UI_Color(border);
	}

	TabControlData *data = static_cast<TabControlData *>(window->winGetUserData());
	if (data == nullptr)
		return visual;
	visual.count = static_cast<std::size_t>(std::clamp(data->tabCount, 0, 8));
	Int tab_x = origin.x + data->tabsLeftLimit;
	Int tab_y = origin.y + data->tabsTopLimit;
	const Int tab_delta_x = (data->tabEdge == TP_TOP_SIDE || data->tabEdge == TP_BOTTOM_SIDE)
		? data->tabWidth : 0;
	const Int tab_delta_y = (data->tabEdge == TP_TOP_SIDE || data->tabEdge == TP_BOTTOM_SIDE)
		? 0 : data->tabHeight;
	for (std::size_t index = 0; index < visual.count; ++index) {
		visual.rectangles[index] = {
			static_cast<float>(tab_x), static_cast<float>(tab_y),
			static_cast<float>(tab_x + data->tabWidth),
			static_cast<float>(tab_y + data->tabHeight)};
		const bool disabled = data->subPaneDisabled[index];
		const bool active = data->activeTab == static_cast<Int>(index);
		const Image *image = nullptr;
		Color fill = WIN_COLOR_UNDEFINED;
		Color border = WIN_COLOR_UNDEFINED;
		if (disabled) {
			image = window->winGetDisabledImage(static_cast<Int>(index));
			fill = window->winGetDisabledColor(static_cast<Int>(index));
			border = window->winGetDisabledBorderColor(static_cast<Int>(index));
		}
		else if (active) {
			image = window->winGetHiliteImage(static_cast<Int>(index));
			fill = window->winGetHiliteColor(static_cast<Int>(index));
			border = window->winGetHiliteBorderColor(static_cast<Int>(index));
		}
		else {
			image = window->winGetEnabledImage(static_cast<Int>(index));
			fill = window->winGetEnabledColor(static_cast<Int>(index));
			border = window->winGetEnabledBorderColor(static_cast<Int>(index));
		}
		visual.has_images[index] = image_visual && image != nullptr;
		visual.images[index] = To_WND_Image(image);
		visual.has_fills[index] = !image_visual && fill != WIN_COLOR_UNDEFINED;
		visual.fills[index] = To_UI_Color(fill);
		visual.has_borders[index] = !image_visual && border != WIN_COLOR_UNDEFINED;
		visual.borders[index] = To_UI_Color(border);
		tab_x += tab_delta_x;
		tab_y += tab_delta_y;
	}
	return visual;
}

Bool Append_Tab_Control_Draw_Data(
	GameWindow *window,
	WinInstanceData *instance_data,
	void *opaque_draw_list,
	bool image_visual)
{
	if (window == nullptr || instance_data == nullptr || opaque_draw_list == nullptr)
		return FALSE;
	Engine::UI::WND::DrawList &draw_list =
		*static_cast<Engine::UI::WND::DrawList *>(opaque_draw_list);
	const Engine::UI::WND::TabControlVisual visual =
		Build_Visual(window, instance_data, image_visual);
	return Engine::UI::WND::Add_Tab_Control_Visual(draw_list, visual) ? TRUE : FALSE;
}

}

Bool W3DGadgetTabControlDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Tab_Control_Draw_Data(window, instance_data, draw_list, false);
}

Bool W3DGadgetTabControlImageDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Tab_Control_Draw_Data(window, instance_data, draw_list, true);
}
