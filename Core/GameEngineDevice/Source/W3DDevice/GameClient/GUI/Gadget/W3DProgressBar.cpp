/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
*/

#include "Precompiled/PreRTS.h"

#include "GameClient/GadgetProgressBar.h"
#include "GameClient/GameWindowGlobal.h"
#include "W3DDevice/GameClient/W3DGadget.h"

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
	Engine::UI::WND::ImageRef reference =
		Engine::UI::WND::Resolve_Image_Reference(image->getFilename().str());
	const Region2D *uv = image->getUV();
	if (uv != nullptr)
		reference.uv = {uv->lo.x, uv->lo.y, uv->hi.x, uv->hi.y};
	return reference;
}

Int Get_Progress(GameWindow *window) noexcept
{
	return static_cast<Int>(reinterpret_cast<std::intptr_t>(window->winGetUserData()));
}

void Select_Colors(
	GameWindow *window,
	WinInstanceData *instance_data,
	Color &background,
	Color &background_border,
	Color &bar,
	Color &bar_border)
{
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
		background = GadgetProgressBarGetDisabledColor(window);
		background_border = GadgetProgressBarGetDisabledBorderColor(window);
		bar = GadgetProgressBarGetDisabledBarColor(window);
		bar_border = GadgetProgressBarGetDisabledBarBorderColor(window);
	}
	else if (BitIsSet(instance_data->getState(), WIN_STATE_HILITED)) {
		background = GadgetProgressBarGetHiliteColor(window);
		background_border = GadgetProgressBarGetHiliteBorderColor(window);
		bar = GadgetProgressBarGetHiliteBarColor(window);
		bar_border = GadgetProgressBarGetHiliteBarBorderColor(window);
	}
	else {
		background = GadgetProgressBarGetEnabledColor(window);
		background_border = GadgetProgressBarGetEnabledBorderColor(window);
		bar = GadgetProgressBarGetEnabledBarColor(window);
		bar_border = GadgetProgressBarGetEnabledBarBorderColor(window);
	}
}

Engine::UI::WND::ProgressBarVisual Build_Color_Visual(
	GameWindow *window,
	WinInstanceData *instance_data)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	Color background = WIN_COLOR_UNDEFINED;
	Color background_border = WIN_COLOR_UNDEFINED;
	Color bar = WIN_COLOR_UNDEFINED;
	Color bar_border = WIN_COLOR_UNDEFINED;
	Select_Colors(window, instance_data, background, background_border, bar, bar_border);

	Engine::UI::WND::ProgressBarVisual visual;
	visual.rectangle = {
		static_cast<float>(origin.x), static_cast<float>(origin.y),
		static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)};
	visual.progress = Get_Progress(window);
	visual.line_width = WIN_DRAW_LINE_WIDTH;
	visual.has_background_fill = background != WIN_COLOR_UNDEFINED;
	visual.background_fill = To_UI_Color(background);
	visual.has_background_border = background_border != WIN_COLOR_UNDEFINED;
	visual.background_border = To_UI_Color(background_border);
	visual.has_bar_fill = bar != WIN_COLOR_UNDEFINED;
	visual.bar_fill = To_UI_Color(bar);
	visual.has_bar_border = bar_border != WIN_COLOR_UNDEFINED;
	visual.bar_border = To_UI_Color(bar_border);
	return visual;
}

const Image *Get_Image(
	GameWindow *window,
	WinInstanceData *instance_data,
	int slot,
	bool bar)
{
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED))
		return window->winGetDisabledImage(slot + (bar ? 4 : 0));
	if (BitIsSet(instance_data->getState(), WIN_STATE_HILITED))
		return window->winGetHiliteImage(slot + (bar ? 4 : 0));
	return window->winGetEnabledImage(slot + (bar ? 4 : 0));
}

Engine::UI::WND::ProgressBarImageVisual Build_Image_Visual(
	GameWindow *window,
	WinInstanceData *instance_data)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	const Image *left = Get_Image(window, instance_data, 0, false);
	const Image *right = Get_Image(window, instance_data, 1, false);
	const Image *center = Get_Image(window, instance_data, 2, false);
	const Image *bar_center = Get_Image(window, instance_data, 2, true);
	const Image *bar_right = Get_Image(window, instance_data, 1, true);

	Engine::UI::WND::ProgressBarImageVisual visual;
	if (left == nullptr || right == nullptr || center == nullptr
		|| bar_center == nullptr || bar_right == nullptr)
		return visual;

	const float x_offset = static_cast<float>(instance_data->m_imageOffset.x);
	const float y_offset = static_cast<float>(instance_data->m_imageOffset.y);
	const float left_end = static_cast<float>(origin.x) + left->getImageWidth() + x_offset;
	const float right_start = static_cast<float>(origin.x + size.x)
		- right->getImageWidth() + x_offset;
	visual.rectangle = {
		static_cast<float>(origin.x), static_cast<float>(origin.y),
		static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)};
	visual.background_left = To_WND_Image(left);
	visual.background_center = To_WND_Image(center);
	visual.background_right = To_WND_Image(right);
	visual.background_left_rectangle = {
		static_cast<float>(origin.x) + x_offset,
		static_cast<float>(origin.y) + y_offset,
		left_end,
		static_cast<float>(origin.y + size.y) + y_offset};
	visual.background_center_rectangle = {
		left_end,
		static_cast<float>(origin.y) + y_offset,
		right_start,
		static_cast<float>(origin.y + size.y) + y_offset};
	visual.background_right_rectangle = {
		right_start,
		static_cast<float>(origin.y) + y_offset,
		right_start + right->getImageWidth(),
		static_cast<float>(origin.y + size.y) + y_offset};
	visual.background_center_width = static_cast<float>(center->getImageWidth());
	visual.bar_center = To_WND_Image(bar_center);
	visual.bar_right = To_WND_Image(bar_right);
	const float progress_width = static_cast<float>((size.x - 20) * Get_Progress(window)) / 100.0f;
	visual.bar_rectangle = {
		static_cast<float>(origin.x + 10),
		static_cast<float>(origin.y) + y_offset + 5.0f,
		static_cast<float>(origin.x + 10) + progress_width,
		static_cast<float>(origin.y) + y_offset + size.y - 5.0f};
	visual.bar_center_width = static_cast<float>(bar_center->getImageWidth());
	visual.bar_right_width = static_cast<float>(bar_right->getImageWidth());
	return visual;
}

Bool Append_Progress_Bar_Draw_Data(
	GameWindow *window,
	WinInstanceData *instance_data,
	void *opaque_draw_list,
	bool image_visual)
{
	if (window == nullptr || instance_data == nullptr || opaque_draw_list == nullptr)
		return FALSE;
	Engine::UI::WND::DrawList &draw_list =
		*static_cast<Engine::UI::WND::DrawList *>(opaque_draw_list);
	if (image_visual) {
		const Engine::UI::WND::ProgressBarImageVisual visual =
			Build_Image_Visual(window, instance_data);
		if (visual.background_left.texture.Is_Valid() == false
			|| visual.background_center.texture.Is_Valid() == false
			|| visual.background_right.texture.Is_Valid() == false
			|| visual.bar_center.texture.Is_Valid() == false
			|| visual.bar_right.texture.Is_Valid() == false)
			return TRUE;
		return Engine::UI::WND::Add_Progress_Bar_Image_Visual(draw_list, visual) ? TRUE : FALSE;
	}

	const Engine::UI::WND::ProgressBarVisual visual = Build_Color_Visual(window, instance_data);
	return Engine::UI::WND::Add_Progress_Bar_Visual(draw_list, visual) ? TRUE : FALSE;
}

}

Bool W3DGadgetProgressBarDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Progress_Bar_Draw_Data(window, instance_data, draw_list, false);
}

Bool W3DGadgetProgressBarImageDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Progress_Bar_Draw_Data(window, instance_data, draw_list, true);
}
