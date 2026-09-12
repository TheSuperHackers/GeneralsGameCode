/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
*/

module;
#include "Precompiled/PreRTS.h"
#define ENGINE_UI_WND_RUNTIME_MODULE 1

#include "GameClient/GadgetSlider.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GameWindowGlobal.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplay.h"

#include <algorithm>

export module Engine.UI.WND.Runtime.Renderer.Gadget.HorizontalSlider;
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

Engine::UI::WND::HorizontalSliderImageVisual Build_Image_Visual(
	GameWindow *window,
	WinInstanceData *instance_data)
{
	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	const Image *highlight = GadgetSliderGetHiliteImageLeft(window);
	const Image *blank = GadgetSliderGetDisabledImageRight(window);
	const Image *fill = GadgetSliderGetDisabledImageLeft(window);
	Engine::UI::WND::HorizontalSliderImageVisual visual;
	if (highlight == nullptr || blank == nullptr || fill == nullptr)
		return visual;

	SliderData *slider = static_cast<SliderData *>(window->winGetUserData());
	if (slider == nullptr)
		return visual;
	visual.highlighted_image = To_WND_Image(highlight);
	visual.selected_image = To_WND_Image(fill);
	visual.unselected_image = To_WND_Image(blank);
	Engine::UI::WND::Layout_Horizontal_Slider_Images(visual,
		{static_cast<float>(origin.x), static_cast<float>(origin.y),
			static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)},
		static_cast<float>(fill->getImageWidth()), TheWindowManager->winGetLayoutScale(window),
		slider->minVal, slider->maxVal, slider->position);
	visual.highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	return visual;
}

Bool Append_Horizontal_Slider_Draw_Data(
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
		const Engine::UI::WND::HorizontalSliderImageVisual visual =
			Build_Image_Visual(window, instance_data);
		if (!visual.selected_image.texture.Is_Valid()
			|| !visual.unselected_image.texture.Is_Valid())
			return TRUE;
		return Engine::UI::WND::Add_Horizontal_Slider_Image_Visual(draw_list, visual)
			? TRUE : FALSE;
	}
	const Engine::UI::WND::SliderVisual visual = Build_Color_Visual(window, instance_data);
	return Engine::UI::WND::Add_Slider_Visual(draw_list, visual) ? TRUE : FALSE;
}

}

Bool W3DGadgetHorizontalSliderDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Horizontal_Slider_Draw_Data(window, instance_data, draw_list, false);
}

Bool W3DGadgetHorizontalSliderImageDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Horizontal_Slider_Draw_Data(window, instance_data, draw_list, true);
}
