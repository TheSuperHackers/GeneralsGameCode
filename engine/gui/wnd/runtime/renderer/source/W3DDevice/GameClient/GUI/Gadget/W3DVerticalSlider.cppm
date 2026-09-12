/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
*/

module;
#include "Precompiled/PreRTS.h"
#define ENGINE_UI_WND_RUNTIME_MODULE 1

#include "GameClient/GadgetSlider.h"
#include "GameClient/GameWindowGlobal.h"
#include "W3DDevice/GameClient/W3DGadget.h"

export module Engine.UI.WND.Runtime.Renderer.Gadget.VerticalSlider;
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

Engine::UI::WND::SliderVisual Build_Color_Visual(
	GameWindow *window,
	WinInstanceData *instance_data)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	Color background = WIN_COLOR_UNDEFINED;
	Color border = WIN_COLOR_UNDEFINED;
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
		background = GadgetSliderGetDisabledColor(window);
		border = GadgetSliderGetDisabledBorderColor(window);
	}
	else if (BitIsSet(instance_data->getState(), WIN_STATE_HILITED)) {
		background = GadgetSliderGetHiliteColor(window);
		border = GadgetSliderGetHiliteBorderColor(window);
	}
	else {
		background = GadgetSliderGetEnabledColor(window);
		border = GadgetSliderGetEnabledBorderColor(window);
	}

	Engine::UI::WND::SliderVisual visual;
	visual.rectangle = {
		static_cast<float>(origin.x), static_cast<float>(origin.y),
		static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)};
	visual.has_background_fill = background != WIN_COLOR_UNDEFINED;
	visual.background_fill = To_UI_Color(background);
	visual.has_background_border = border != WIN_COLOR_UNDEFINED;
	visual.background_border = To_UI_Color(border);
	return visual;
}

void Select_Images(
	GameWindow *window,
	WinInstanceData *instance_data,
	const Image *&top,
	const Image *&bottom,
	const Image *&center,
	const Image *&small_center)
{
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
		top = GadgetSliderGetDisabledImageTop(window);
		bottom = GadgetSliderGetDisabledImageBottom(window);
		center = GadgetSliderGetDisabledImageCenter(window);
		small_center = GadgetSliderGetDisabledImageSmallCenter(window);
	}
	else if (BitIsSet(instance_data->getState(), WIN_STATE_HILITED)) {
		top = GadgetSliderGetHiliteImageTop(window);
		bottom = GadgetSliderGetHiliteImageBottom(window);
		center = GadgetSliderGetHiliteImageCenter(window);
		small_center = GadgetSliderGetHiliteImageSmallCenter(window);
	}
	else {
		top = GadgetSliderGetEnabledImageTop(window);
		bottom = GadgetSliderGetEnabledImageBottom(window);
		center = GadgetSliderGetEnabledImageCenter(window);
		small_center = GadgetSliderGetEnabledImageSmallCenter(window);
	}
}

Engine::UI::WND::VerticalSliderImageVisual Build_Image_Visual(
	GameWindow *window,
	WinInstanceData *instance_data)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	const Image *top = nullptr;
	const Image *bottom = nullptr;
	const Image *center = nullptr;
	const Image *small_center = nullptr;
	Select_Images(window, instance_data, top, bottom, center, small_center);

	Engine::UI::WND::VerticalSliderImageVisual visual;
	if (top == nullptr || bottom == nullptr || center == nullptr || small_center == nullptr)
		return visual;

	const float x_offset = static_cast<float>(instance_data->m_imageOffset.x);
	const float y_offset = static_cast<float>(instance_data->m_imageOffset.y);
	const float top_height = static_cast<float>(top->getImageHeight());
	const float bottom_height = static_cast<float>(bottom->getImageHeight());
	const float top_width = static_cast<float>(top->getImageWidth());
	const float bottom_width = static_cast<float>(bottom->getImageWidth());
	const float left = static_cast<float>(origin.x) + x_offset;
	const float top_y = static_cast<float>(origin.y) + y_offset;
	const float bottom_y = static_cast<float>(origin.y + size.y) - bottom_height + y_offset;
	visual.top_image = To_WND_Image(top);
	visual.bottom_image = To_WND_Image(bottom);
	visual.center_image = To_WND_Image(center);
	visual.small_center_image = To_WND_Image(small_center);
	if (top_height + bottom_height >= static_cast<float>(size.y)) {
		visual.compact = true;
		visual.top_rectangle = {left, top_y, left + top_width, static_cast<float>(origin.y) + size.y / 2.0f};
		visual.bottom_rectangle = {
			left, static_cast<float>(origin.y) + size.y / 2.0f,
			left + bottom_width, static_cast<float>(origin.y) + y_offset + size.y};
		return visual;
	}

	const float top_end = top_y + top_height;
	const float bottom_start = bottom_y;
	const float center_height = static_cast<float>(center->getImageHeight());
	const float small_center_height = static_cast<float>(small_center->getImageHeight());
	const int center_pieces = center_height > 0.0f
		? static_cast<int>((bottom_start - top_end) / center_height)
		: 0;
	const float center_end = top_end + center_pieces * center_height;
	visual.top_rectangle = {left, top_y, left + top_width, top_end};
	visual.bottom_rectangle = {
		left, bottom_start, left + bottom_width, bottom_start + bottom_height};
	visual.center_rectangle = {left, top_end, left + static_cast<float>(center->getImageWidth()), center_end};
	visual.small_center_rectangle = {left, center_end, left + static_cast<float>(small_center->getImageWidth()), bottom_start};
	visual.center_height = center_height;
	visual.small_center_height = small_center_height;
	return visual;
}

Bool Append_Vertical_Slider_Draw_Data(
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
		const Engine::UI::WND::VerticalSliderImageVisual visual =
			Build_Image_Visual(window, instance_data);
		if (!visual.top_image.texture.Is_Valid()
			|| !visual.bottom_image.texture.Is_Valid()
			|| !visual.center_image.texture.Is_Valid()
			|| !visual.small_center_image.texture.Is_Valid())
			return TRUE;
		return Engine::UI::WND::Add_Vertical_Slider_Image_Visual(draw_list, visual)
			? TRUE : FALSE;
	}
	const Engine::UI::WND::SliderVisual visual = Build_Color_Visual(window, instance_data);
	return Engine::UI::WND::Add_Slider_Visual(draw_list, visual) ? TRUE : FALSE;
}

}

Bool W3DGadgetVerticalSliderDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Vertical_Slider_Draw_Data(window, instance_data, draw_list, false);
}

Bool W3DGadgetVerticalSliderImageDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Vertical_Slider_Draw_Data(window, instance_data, draw_list, true);
}
