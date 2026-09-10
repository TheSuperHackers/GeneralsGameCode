/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DMainMenu.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Apr 2002
//
//	Filename: 	W3DMainMenu.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	The Draw Routine for the main menu
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include <SDL3/SDL.h>
#include <time.h>
//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "GameClient/GameWindow.h"
#include "Lib/BaseType.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "GameClient/Display.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/Shell.h"
#include "GameClient/ShellMenuScheme.h"
#include "GameClient/Credits.h"

#include "GameClient/Gadget.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetPushButton.h"
#include "W3DDevice/GameClient/W3DDisplayString.h"
#include "W3DDevice/GameClient/W3DGadget.h"

#include "GameClient/GUICallbacks.h"

import Engine.UI.WND;

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

namespace
{

Engine::UI::WND::ImageRef To_WND_Image(const Image *image)
{
	if (image == nullptr || image->getUV() == nullptr)
		return {};
	const Region2D *uv = image->getUV();
	Engine::UI::WND::ImageRef result =
		Engine::UI::WND::Resolve_Image_Reference(image->getFilename().str());
	result.uv = {uv->lo.x, uv->lo.y, uv->hi.x, uv->hi.y};
	return result;
}

Graphics::Color2D To_WND_Color(Color color)
{
	return {
		static_cast<float>((color >> 16) & 0xff) / 255.0f,
		static_cast<float>((color >> 8) & 0xff) / 255.0f,
		static_cast<float>(color & 0xff) / 255.0f,
		static_cast<float>((color >> 24) & 0xff) / 255.0f};
}

Bool Add_WND_Image(
	Engine::UI::WND::DrawList &draw_list,
	const Image *image,
	Int left,
	Int top,
	Int right,
	Int bottom,
	Color color = GameMakeColor(255, 255, 255, 255))
{
	if (image == nullptr)
		return TRUE;
	return draw_list.Add_Image(
		To_WND_Image(image),
		{static_cast<float>(left), static_cast<float>(top),
			static_cast<float>(right), static_cast<float>(bottom)},
		To_WND_Color(color)) ? TRUE : FALSE;
}

Bool Add_WND_Display_Text(
	Engine::UI::WND::DrawList &draw_list,
	GameWindow *window,
	DisplayString *text,
	Int x,
	Int y,
	Color color,
	Color drop_color,
	const IRegion2D *clip = nullptr)
{
	if (window == nullptr || text == nullptr || text->getTextLength() == 0)
		return TRUE;
	ICoord2D size;
	window->winGetSize(&size.x, &size.y);
	text->setWordWrap(size.x);
	if (text->getFont() != window->winGetFont())
		text->setFont(window->winGetFont());
	W3DDisplayString *device_text = static_cast<W3DDisplayString *>(text);
	return device_text->appendDrawData(draw_list, x, y, color, drop_color, 1, 1, clip)
		? TRUE : FALSE;
}

Bool Add_WND_Text(
	Engine::UI::WND::DrawList &draw_list,
	GameWindow *window,
	WinInstanceData *instance_data,
	Int x,
	Int y,
	Color color,
	Color drop_color,
	const IRegion2D *clip = nullptr)
{
	if (window == nullptr || instance_data == nullptr)
		return FALSE;
	DisplayString *text = instance_data->getTextDisplayString();
	if (text == nullptr)
		return TRUE;
	text->setWordWrapCentered(BitIsSet(instance_data->getStatus(), WIN_STATUS_WRAP_CENTERED));
	return Add_WND_Display_Text(draw_list, window, text, x, y, color, drop_color, clip);
}

Bool Add_Menu_Frame(
	Engine::UI::WND::DrawList &draw_list,
	GameWindow *window,
	Bool four_column)
{
	if (window == nullptr)
		return FALSE;
	ICoord2D position;
	ICoord2D size;
	window->winGetScreenPosition(&position.x, &position.y);
	window->winGetSize(&size.x, &size.y);
	const Color color = GameMakeColor(167,134,94,255);
	const Color drop = GameMakeColor(38, 30, 21, 255);
	const Int first_vertical = four_column ? Int(size.x * .295f) : Int(size.x * .225f);
	const Int second_vertical = four_column ? Int(size.x * .59f) : Int(size.x * .445f);
	const Int third_vertical = four_column ? 0 : Int(size.x * .6662f);
	const Int fourth_vertical = Int(size.x * .885f);
	const auto line = [&draw_list](Int x1, Int y1, Int x2, Int y2, float width, Color line_color) {
		return draw_list.Add_Line(
			{static_cast<float>(x1), static_cast<float>(y1)},
			{static_cast<float>(x2), static_cast<float>(y2)},
			width,
			To_WND_Color(line_color));
	};
	if (!line(position.x, position.y, position.x + size.x, position.y, 2.0f, color)
		|| !line(position.x, position.y + 1, position.x + size.x, position.y + 1, 2.0f, drop)
		|| !line(position.x, position.y + Int(size.y * .1f), position.x + size.x,
			position.y + Int(size.y * .1f), 1.0f, color)
		|| !line(position.x, position.y + Int(size.y * .12f), position.x + size.x,
			position.y + Int(size.y * .12f), 1.0f, drop)
		|| !line(position.x, position.y + Int(size.y * .9f), position.x + size.x,
			position.y + Int(size.y * .9f), 1.0f, color)
		|| !line(position.x, position.y + Int(size.y * .92f), position.x + size.x,
			position.y + Int(size.y * .92f), 1.0f, drop)
		|| !line(position.x, position.y + size.y, position.x + size.x, position.y + size.y, 2.0f, color)
		|| !line(position.x, position.y + size.y + 1, position.x + size.x, position.y + size.y + 1, 2.0f, drop)
		|| !line(position.x + first_vertical, position.y, position.x + first_vertical,
			position.y + size.y, 3.0f, color)
		|| !line(position.x + second_vertical, position.y, position.x + second_vertical,
			position.y + size.y, 3.0f, color)
		|| (!four_column && !line(position.x + third_vertical, position.y,
			position.x + third_vertical, position.y + size.y, 3.0f, color))
		|| !line(position.x + fourth_vertical, position.y, position.x + fourth_vertical,
			position.y + size.y, 3.0f, color))
		return FALSE;

	const Image *pulse = TheMappedImageCollection != nullptr
		? TheMappedImageCollection->findImageByName("MainMenuPulse") : nullptr;
	if (pulse == nullptr)
		return TRUE;
	static Bool going_forward = TRUE;
	static UnsignedInt start_time = SDL_GetTicks();
	const UnsignedInt elapsed = SDL_GetTicks() - start_time;
	const Real percent = INT_TO_REAL(elapsed) / 10000.0f;
	Int x = position.x - pulse->getImageWidth();
	Int y = position.y - pulse->getImageHeight() / 2;
	if (going_forward) {
		if (percent >= 1.0f) {
			start_time = SDL_GetTicks();
			going_forward = FALSE;
			y = position.y + size.y - pulse->getImageHeight() / 2;
		} else {
			x = static_cast<Int>(percent * (size.x + pulse->getImageWidth()))
				- pulse->getImageWidth();
		}
	} else if (percent >= 1.0f) {
		start_time = SDL_GetTicks();
		going_forward = TRUE;
		y = position.y - pulse->getImageHeight() / 2;
	} else {
		y = position.y + size.y - pulse->getImageHeight() / 2;
		x = position.x + size.x - static_cast<Int>(percent * (size.x + pulse->getImageWidth()));
	}
	return Add_WND_Image(draw_list, pulse, x, y, x + pulse->getImageWidth(), y + pulse->getImageHeight());
}

}

Bool W3DMainMenuDrawData(GameWindow *window, WinInstanceData *, void *drawList)
{
	return drawList != nullptr && Add_Menu_Frame(
		*static_cast<Engine::UI::WND::DrawList *>(drawList), window, FALSE);
}

Bool W3DMainMenuFourDrawData(GameWindow *window, WinInstanceData *, void *drawList)
{
	return drawList != nullptr && Add_Menu_Frame(
		*static_cast<Engine::UI::WND::DrawList *>(drawList), window, TRUE);
}

Bool W3DMainMenuMapBorderDrawData(GameWindow *window, WinInstanceData *, void *drawList)
{
	if (window == nullptr || drawList == nullptr || TheMappedImageCollection == nullptr)
		return FALSE;
	Int x, y, width, height;
	window->winGetScreenPosition(&x, &y);
	window->winGetSize(&width, &height);
	constexpr Int corner = 10;
	constexpr Int line = 20;
	constexpr Int piece = 20;
	const Int maximum_x = x + width;
	const Int maximum_y = y + height;
	Engine::UI::WND::DrawList &list = *static_cast<Engine::UI::WND::DrawList *>(drawList);
	const Image *horizontal = TheMappedImageCollection->findImageByName("FrameCornerHorizontal");
	const Image *vertical = TheMappedImageCollection->findImageByName("FrameCornerVertical");
	if (horizontal != nullptr) {
		Int cursor = x + corner;
		const Int end = maximum_x - (corner + line);
		for (; cursor <= end; cursor += line) {
			if (!Add_WND_Image(list, horizontal, cursor, y - corner, cursor + piece, y + corner)
				|| !Add_WND_Image(list, horizontal, cursor, maximum_y - corner,
					cursor + piece, maximum_y + corner))
				return FALSE;
		}
		const Int remainder_end = maximum_x - corner;
		if (remainder_end - cursor >= line / 2) {
			if (!Add_WND_Image(list, horizontal, cursor, y - corner,
				cursor + line / 2, y + corner)
				|| !Add_WND_Image(list, horizontal, cursor, maximum_y - corner,
					cursor + line / 2, maximum_y + corner))
				return FALSE;
			cursor += line / 2;
		}
		if (cursor < remainder_end) {
			cursor -= line / 2 - (((remainder_end - cursor) + 1) & ~1);
			if (!Add_WND_Image(list, horizontal, cursor, y - corner,
				cursor + line / 2, y + corner)
				|| !Add_WND_Image(list, horizontal, cursor, maximum_y - corner,
					cursor + line / 2, maximum_y + corner))
				return FALSE;
		}
	}
	if (vertical != nullptr) {
		Int cursor = y + corner;
		const Int end = maximum_y - (corner + line);
		for (; cursor <= end; cursor += line) {
			if (!Add_WND_Image(list, vertical, x - corner, cursor, x + corner,
				cursor + piece)
				|| !Add_WND_Image(list, vertical, maximum_x - corner, cursor,
					maximum_x + corner, cursor + piece))
				return FALSE;
		}
		const Int remainder_end = maximum_y - corner;
		if (remainder_end - cursor >= line / 2) {
			if (!Add_WND_Image(list, vertical, x - corner, cursor, x + corner,
				cursor + line / 2)
				|| !Add_WND_Image(list, vertical, maximum_x - corner, cursor,
					maximum_x + corner, cursor + line / 2))
				return FALSE;
			cursor += line / 2;
		}
		if (cursor < remainder_end) {
			cursor -= line / 2 - (((remainder_end - cursor) + 1) & ~1);
			if (!Add_WND_Image(list, vertical, x - corner, cursor, x + corner,
				cursor + line / 2)
				|| !Add_WND_Image(list, vertical, maximum_x - corner, cursor,
					maximum_x + corner, cursor + line / 2))
				return FALSE;
		}
	}
	return Add_WND_Image(list, TheMappedImageCollection->findImageByName("FrameCornerUL"),
		x - corner, y - corner, x + corner, y + corner)
		&& Add_WND_Image(list, TheMappedImageCollection->findImageByName("FrameCornerUR"),
			maximum_x - corner, y - corner, maximum_x + corner, y + corner)
		&& Add_WND_Image(list, TheMappedImageCollection->findImageByName("FrameCornerLL"),
			x - corner, maximum_y - corner, x + corner, maximum_y + corner)
		&& Add_WND_Image(list, TheMappedImageCollection->findImageByName("FrameCornerLR"),
			maximum_x - corner, maximum_y - corner, maximum_x + corner, maximum_y + corner);
}

Bool W3DMainMenuRandomTextDrawData(GameWindow *window, WinInstanceData *, void *drawList)
{
	if (window == nullptr || drawList == nullptr)
		return FALSE;
	TextData *data = static_cast<TextData *>(window->winGetUserData());
	if (data == nullptr || data->text == nullptr)
		return TRUE;
	ICoord2D origin, size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);
	Int text_width = 0;
	Int text_height = 0;
	data->text->getSize(&text_width, &text_height);
	const IRegion2D clip{origin.x + 1, origin.y + 1,
		origin.x + size.x - 1, origin.y + size.y - 1};
	return Add_WND_Display_Text(
		*static_cast<Engine::UI::WND::DrawList *>(drawList),
		window,
		data->text,
		origin.x,
		origin.y + (size.y - text_height) / 2,
		window->winGetDisabledTextColor(),
		window->winGetDisabledTextBorderColor(),
		&clip);
}

Bool W3DMainMenuButtonDropShadowDrawData(GameWindow *window, WinInstanceData *instance_data, void *drawList)
{
	if (window == nullptr || instance_data == nullptr || drawList == nullptr)
		return FALSE;

	const Image *left_image = nullptr;
	const Image *middle_image = nullptr;
	const Image *right_image = nullptr;
	const Bool selected = BitIsSet(instance_data->getState(), WIN_STATE_SELECTED);
	const Bool highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
		left_image = selected ? GadgetButtonGetLeftDisabledSelectedImage(window)
			: GadgetButtonGetLeftDisabledImage(window);
		middle_image = selected ? GadgetButtonGetMiddleDisabledSelectedImage(window)
			: GadgetButtonGetMiddleDisabledImage(window);
		right_image = selected ? GadgetButtonGetRightDisabledSelectedImage(window)
			: GadgetButtonGetRightDisabledImage(window);
	} else if (highlighted) {
		left_image = selected ? GadgetButtonGetLeftHiliteSelectedImage(window)
			: GadgetButtonGetLeftHiliteImage(window);
		middle_image = selected ? GadgetButtonGetMiddleHiliteSelectedImage(window)
			: GadgetButtonGetMiddleHiliteImage(window);
		right_image = selected ? GadgetButtonGetRightHiliteSelectedImage(window)
			: GadgetButtonGetRightHiliteImage(window);
	} else {
		left_image = selected ? GadgetButtonGetLeftEnabledSelectedImage(window)
			: GadgetButtonGetLeftEnabledImage(window);
		middle_image = selected ? GadgetButtonGetMiddleEnabledSelectedImage(window)
			: GadgetButtonGetMiddleEnabledImage(window);
		right_image = selected ? GadgetButtonGetRightEnabledSelectedImage(window)
			: GadgetButtonGetRightEnabledImage(window);
	}
	if (left_image == nullptr || middle_image == nullptr || right_image == nullptr)
		return TRUE;

	ICoord2D origin, size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);
	const float left = static_cast<float>(origin.x + instance_data->m_imageOffset.x);
	const float top = static_cast<float>(origin.y + instance_data->m_imageOffset.y);
	Engine::UI::WND::PushButtonVisual visual;
	visual.rectangle = {left, top, left + size.x, top + size.y};
	visual.segmented = true;
	visual.left_image = To_WND_Image(left_image);
	visual.middle_image = To_WND_Image(middle_image);
	visual.right_image = To_WND_Image(right_image);
	visual.left_width = static_cast<float>(left_image->getImageWidth());
	visual.middle_width = static_cast<float>(middle_image->getImageWidth());
	visual.right_width = static_cast<float>(right_image->getImageWidth());
	Engine::UI::WND::DrawList &list = *static_cast<Engine::UI::WND::DrawList *>(drawList);
	if (!Engine::UI::WND::Add_Push_Button_Background(list, visual))
		return FALSE;

	if (instance_data->getTextLength()) {
		DisplayString *text = instance_data->getTextDisplayString();
		if (text != nullptr) {
			Int text_width = 0;
			Int text_height = 0;
			text->getSize(&text_width, &text_height);
			const Color text_color = !BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)
				? window->winGetDisabledTextColor()
				: highlighted ? window->winGetHiliteTextColor() : window->winGetEnabledTextColor();
			const Color drop_color = !BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)
				? window->winGetDisabledTextBorderColor()
				: highlighted ? window->winGetHiliteTextBorderColor() : window->winGetEnabledTextBorderColor();
			if (!Add_WND_Display_Text(list, window, text,
				origin.x + (size.x - text_width) / 2,
				origin.y + (size.y - text_height) / 2,
				text_color, drop_color))
				return FALSE;
		}
	}

	PushButtonData *data = static_cast<PushButtonData *>(window->winGetUserData());
	if (data != nullptr) {
		visual.has_overlay = data->overlayImage != nullptr;
		visual.overlay_image = To_WND_Image(data->overlayImage);
		visual.has_clock = data->drawClock == NORMAL_CLOCK || data->drawClock == INVERSE_CLOCK;
		visual.clock_percent = data->percentClock;
		visual.remaining_clock = data->drawClock == INVERSE_CLOCK;
		visual.clock_color = To_WND_Color(data->colorClock);
		visual.has_extra_border = data->drawBorder && data->colorBorder != GAME_COLOR_UNDEFINED;
		visual.extra_border = {visual.rectangle.left - 1.0f, visual.rectangle.top - 1.0f,
			visual.rectangle.right + 1.0f, visual.rectangle.bottom + 1.0f};
		visual.extra_border_color = To_WND_Color(data->colorBorder);
		if (!Engine::UI::WND::Add_Push_Button_Overlays(list, visual))
			return FALSE;
		if (data->drawClock != NO_CLOCK) {
			data->drawClock = NO_CLOCK;
			window->winSetUserData(data);
		}
	}
	return TRUE;
}

Bool W3DThinBorderDrawData(GameWindow *window, WinInstanceData *instance_data, void *drawList)
{
	if (window == nullptr || instance_data == nullptr || drawList == nullptr)
		return FALSE;
	ICoord2D origin, size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);
	const Image *image = window->winGetEnabledImage(0);
	return Add_WND_Image(
		*static_cast<Engine::UI::WND::DrawList *>(drawList), image,
		origin.x + instance_data->m_imageOffset.x,
		origin.y + instance_data->m_imageOffset.y,
		origin.x + instance_data->m_imageOffset.x + size.x,
		origin.y + instance_data->m_imageOffset.y + size.y);
}

Bool W3DMetalBarMenuDrawData(GameWindow *window, WinInstanceData *instance_data, void *drawList)
{
	if (window == nullptr || instance_data == nullptr || drawList == nullptr)
		return FALSE;
	return W3DGameWinBorderDrawData(window, instance_data, drawList);
}

Bool W3DCreditsMenuDrawData(GameWindow *, WinInstanceData *, void *)
{
	return TRUE;
}

Bool W3DClockDrawData(GameWindow *window, WinInstanceData *instance_data, void *drawList)
{
	if (window == nullptr || instance_data == nullptr || drawList == nullptr)
		return FALSE;
	if (!W3DGameWinDefaultDrawData(window, instance_data, drawList))
		return FALSE;

	char date_string[256] = "";
	time_t long_time;
	std::time(&long_time);
	const tm *current_time = std::localtime(&long_time);
	if (current_time == nullptr || std::strftime(date_string, sizeof(date_string), "%H:%M:%S", current_time) == 0)
		return TRUE;

	UnicodeString clock_text;
	clock_text.translate(date_string);
	instance_data->setText(clock_text);
	DisplayString *display_string = instance_data->getTextDisplayString();
	if (display_string == nullptr)
		return TRUE;
	display_string->setFont(TheFontLibrary->getFont("Arial", 16, 0));

	ICoord2D position;
	ICoord2D size;
	window->winGetScreenPosition(&position.x, &position.y);
	window->winGetSize(&size.x, &size.y);
	Int text_width = 0;
	Int text_height = 0;
	display_string->getSize(&text_width, &text_height);
	const IRegion2D clip{position.x + 1, position.y + 1,
		position.x + size.x - 1, position.y + size.y - 1};
	return Add_WND_Display_Text(
		*static_cast<Engine::UI::WND::DrawList *>(drawList),
		window,
		display_string,
		position.x + (size.x - text_width) / 2,
		position.y + (size.y - text_height) / 2,
		GameMakeColor(255, 255, 255, 255),
		GameMakeColor(0, 0, 0, 255),
		&clip);
}

Bool W3DShellMenuSchemeDrawData(GameWindow *, WinInstanceData *, void *drawList)
{
	if (drawList == nullptr || TheShell == nullptr)
		return FALSE;
	ShellMenuSchemeManager *manager = TheShell->getShellMenuSchemeManager();
	ShellMenuScheme *scheme = manager != nullptr ? manager->getCurrentScheme() : nullptr;
	if (scheme == nullptr)
		return TRUE;

	Engine::UI::WND::DrawList &list =
		*static_cast<Engine::UI::WND::DrawList *>(drawList);
	for (ShellMenuSchemeImage *image : scheme->m_imageList) {
		if (image == nullptr || image->m_image == nullptr)
			continue;
		if (!Add_WND_Image(
				list,
				image->m_image,
				image->m_position.x,
				image->m_position.y,
				image->m_position.x + image->m_size.x,
				image->m_position.y + image->m_size.y))
			return FALSE;
	}
	for (ShellMenuSchemeLine *line : scheme->m_lineList) {
		if (line == nullptr)
			continue;
		if (!list.Add_Line(
				{static_cast<float>(line->m_startPos.x), static_cast<float>(line->m_startPos.y)},
				{static_cast<float>(line->m_endPos.x), static_cast<float>(line->m_endPos.y)},
				static_cast<float>(line->m_width),
				To_WND_Color(line->m_color)))
			return FALSE;
	}
	return TRUE;
}

void W3DMainMenuInit( WindowLayout *layout, void *userData )
{
	MainMenuInit( layout, userData );
}
