/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
*/

module;
#include "Precompiled/PreRTS.h"
#define ENGINE_UI_WND_RUNTIME_MODULE 1

#include "GameClient/GadgetComboBox.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplayString.h"

export module Engine.UI.WND.Runtime.Renderer.Gadget.ComboBox;
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

void Select_Visual(
	GameWindow *window,
	WinInstanceData *instance_data,
	Color &background,
	Color &border,
	Color &text_color,
	Color &text_border,
	const Image *&image)
{
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
		background = GadgetComboBoxGetDisabledColor(window);
		border = GadgetComboBoxGetDisabledBorderColor(window);
		text_color = window->winGetDisabledTextColor();
		text_border = window->winGetDisabledTextBorderColor();
		image = GadgetComboBoxGetDisabledImage(window);
	}
	else if (BitIsSet(instance_data->getState(), WIN_STATE_HILITED)) {
		background = GadgetComboBoxGetHiliteColor(window);
		border = GadgetComboBoxGetHiliteBorderColor(window);
		text_color = window->winGetHiliteTextColor();
		text_border = window->winGetHiliteTextBorderColor();
		image = GadgetComboBoxGetHiliteImage(window);
	}
	else {
		background = GadgetComboBoxGetEnabledColor(window);
		border = GadgetComboBoxGetEnabledBorderColor(window);
		text_color = window->winGetEnabledTextColor();
		text_border = window->winGetEnabledTextBorderColor();
		image = GadgetComboBoxGetEnabledImage(window);
	}
}

bool Append_Title(
	Engine::UI::WND::DrawList &draw_list,
	GameWindow *window,
	WinInstanceData *instance_data,
	Color text_color,
	Color text_border,
	Int x,
	Int y)
{
	DisplayString *title = instance_data->getTextDisplayString();
	if (title == nullptr || title->getTextLength() == 0)
		return true;
	if (title->getFont() != window->winGetFont())
		title->setFont(window->winGetFont());
	return static_cast<W3DDisplayString *>(title)->appendDrawData(
		draw_list, x, y, text_color, text_border);
}

Int Get_Title_Height(GameWindow *window, WinInstanceData *instance_data)
{
	DisplayString *title = instance_data->getTextDisplayString();
	if (title != nullptr && title->getTextLength() != 0) {
		if (title->getFont() != window->winGetFont())
			title->setFont(window->winGetFont());
		Int width = 0;
		Int height = 0;
		title->getSize(&width, &height);
		return height;
	}
	return TheWindowManager->winFontHeight(instance_data->getFont());
}

Bool Append_Combo_Box_Draw_Data(
	GameWindow *window,
	WinInstanceData *instance_data,
	void *opaque_draw_list,
	bool image_visual)
{
	if (window == nullptr || instance_data == nullptr || opaque_draw_list == nullptr)
		return FALSE;
	Engine::UI::WND::DrawList &draw_list =
		*static_cast<Engine::UI::WND::DrawList *>(opaque_draw_list);

	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);
	Color background = WIN_COLOR_UNDEFINED;
	Color border = WIN_COLOR_UNDEFINED;
	Color text_color = WIN_COLOR_UNDEFINED;
	Color text_border = WIN_COLOR_UNDEFINED;
	const Image *image = nullptr;
	Select_Visual(window, instance_data, background, border, text_color, text_border, image);

	if (image_visual) {
		if (image != nullptr) {
			const float x = static_cast<float>(origin.x + instance_data->m_imageOffset.x);
			const float y = static_cast<float>(origin.y + instance_data->m_imageOffset.y);
			if (!draw_list.Add_Image(
					To_WND_Image(image), {x, y, x + size.x, y + size.y}))
				return FALSE;
		}
	}
	else {
		const Int title_height = Get_Title_Height(window, instance_data);
		const float top = static_cast<float>(origin.y
			+ (instance_data->getTextDisplayString() != nullptr
				&& instance_data->getTextDisplayString()->getTextLength() != 0
				? title_height + 1 : 0));
		if (!draw_list.Add_Window_Background(
				{static_cast<float>(origin.x), top,
				 static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)},
				false,
				{},
				border != WIN_COLOR_UNDEFINED,
				To_UI_Color(border),
				background != WIN_COLOR_UNDEFINED,
				To_UI_Color(background)))
			return FALSE;
	}

	const Int title_height = Get_Title_Height(window, instance_data);
	if (!Append_Title(
			draw_list,
			window,
			instance_data,
			text_color,
			text_border,
			origin.x + 1,
			origin.y))
		return FALSE;
	(void)title_height;
	return TRUE;
}

}

Bool W3DGadgetComboBoxDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Combo_Box_Draw_Data(window, instance_data, draw_list, false);
}

Bool W3DGadgetComboBoxImageDrawData(
	GameWindow *window, WinInstanceData *instance_data, void *draw_list)
{
	return Append_Combo_Box_Draw_Data(window, instance_data, draw_list, true);
}
