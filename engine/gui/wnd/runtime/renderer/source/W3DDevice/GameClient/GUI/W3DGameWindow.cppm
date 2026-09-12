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

// FILE: W3DGameWindow.cpp ////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: W3DGameWindow.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      W3D implementation of a game window
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "Precompiled/PreRTS.h"
#include <stdlib.h>

import Engine.UI.WND;
import Graphics.Renderer2D;

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowGlobal.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "W3DDevice/GameClient/W3DGameWindowManager.h"


// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static Bool bordersInit = FALSE;
static const Image *borderPieces[NUM_BORDER_PIECES] = { nullptr };

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

static void initBorders();

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static Engine::UI::WND::ImageRef To_WND_Image(const Image *image)
{
	if (image == nullptr || image->getUV() == nullptr)
		return {};
	const Region2D *uv = image->getUV();
	Engine::UI::WND::ImageRef reference =
		Engine::UI::WND::Resolve_Image_Reference(image->getFilename().str());
	reference.uv = {uv->lo.x, uv->lo.y, uv->hi.x, uv->hi.y};
	return reference;
}

static Engine::UI::WND::BorderAtlas Get_Border_Atlas()
{
	if (!bordersInit)
		initBorders();

	Engine::UI::WND::BorderAtlas atlas;
	for (Int index = 0; index < NUM_BORDER_PIECES; ++index) {
		if (borderPieces[index] != nullptr)
			atlas.pieces[index] = To_WND_Image(borderPieces[index]);
	}
	return atlas;
}

static Graphics::Color2D To_UI_Color(Color color) noexcept
{
	return {
		static_cast<float>((color >> 16) & 0xff) / 255.0f,
		static_cast<float>((color >> 8) & 0xff) / 255.0f,
		static_cast<float>(color & 0xff) / 255.0f,
		static_cast<float>((color >> 24) & 0xff) / 255.0f};
}

// initBorders ================================================================
//=============================================================================
static void initBorders()
{

	borderPieces[ BORDER_CORNER_UL ] =
						TheMappedImageCollection->findImageByName( "BorderCornerUL" );

	borderPieces[ BORDER_CORNER_UR ] =
						TheMappedImageCollection->findImageByName( "BorderCornerUR" );

	borderPieces[ BORDER_CORNER_LL ] =
						TheMappedImageCollection->findImageByName( "BorderCornerLL" );

	borderPieces[ BORDER_CORNER_LR ] =
						TheMappedImageCollection->findImageByName( "BorderCornerLR" );

	borderPieces[ BORDER_VERTICAL_LEFT ] =
						TheMappedImageCollection->findImageByName( "BorderLeft" );

	borderPieces[ BORDER_VERTICAL_LEFT_SHORT ] =
						TheMappedImageCollection->findImageByName( "BorderLeftShort" );

	borderPieces[ BORDER_HORIZONTAL_TOP ] =
						TheMappedImageCollection->findImageByName( "BorderTop" );

	borderPieces[ BORDER_HORIZONTAL_TOP_SHORT ] =
						TheMappedImageCollection->findImageByName( "BorderTopShort" );

	borderPieces[ BORDER_VERTICAL_RIGHT ] =
						TheMappedImageCollection->findImageByName( "BorderRight" );

	borderPieces[ BORDER_VERTICAL_RIGHT_SHORT ] =
						TheMappedImageCollection->findImageByName( "BorderRightShort" );

	borderPieces[ BORDER_HORIZONTAL_BOTTOM ] =
						TheMappedImageCollection->findImageByName( "BorderBottom" );

	borderPieces[ BORDER_HORIZONTAL_BOTTOM_SHORT ] =
						TheMappedImageCollection->findImageByName( "BorderBottomShort" );

	bordersInit = TRUE;

}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// W3DGameWindow::W3DGameWindow ===============================================
//=============================================================================
W3DGameWindow::W3DGameWindow()
{

	// override the default draw with our own default draw function for W3D
	winSetDrawFunc( TheWindowManager->getDefaultDraw() );

}

// W3DGameWindow::~W3DGameWindow ==============================================
//=============================================================================
W3DGameWindow::~W3DGameWindow()
{

}

Bool W3DGameWinDefaultDrawData(GameWindow *window, WinInstanceData *instData, void *drawList)
{
	if (window == nullptr || instData == nullptr || drawList == nullptr)
		return FALSE;

	ICoord2D origin;
	ICoord2D size;
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);
	Engine::UI::WND::DrawList &list =
		*static_cast<Engine::UI::WND::DrawList *>(drawList);
	if (BitIsSet(window->winGetStatus(), WIN_STATUS_IMAGE)) {
		const Image *image = nullptr;
		if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED))
			image = window->winGetDisabledImage(0);
		else if (BitIsSet(instData->getState(), WIN_STATE_HILITED))
			image = window->winGetHiliteImage(0);
		else
			image = window->winGetEnabledImage(0);

		const Graphics::Rect2D rectangle{
			static_cast<float>(origin.x + instData->m_imageOffset.x),
			static_cast<float>(origin.y + instData->m_imageOffset.y),
			static_cast<float>(origin.x + instData->m_imageOffset.x + size.x),
			static_cast<float>(origin.y + instData->m_imageOffset.y + size.y)};
		return list.Add_Image(To_WND_Image(image), rectangle) ? TRUE : FALSE;
	}

	Color color = WIN_COLOR_UNDEFINED;
	Color border_color = WIN_COLOR_UNDEFINED;
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
		color = window->winGetDisabledColor(0);
		border_color = window->winGetDisabledBorderColor(0);
	} else if (BitIsSet(instData->getState(), WIN_STATE_HILITED)) {
		color = window->winGetHiliteColor(0);
		border_color = window->winGetHiliteBorderColor(0);
	} else {
		color = window->winGetEnabledColor(0);
		border_color = window->winGetEnabledBorderColor(0);
	}

	const Graphics::Rect2D rectangle{
		static_cast<float>(origin.x), static_cast<float>(origin.y),
		static_cast<float>(origin.x + size.x), static_cast<float>(origin.y + size.y)};
	return list.Add_Window_Background(
		rectangle,
		false,
		{},
		border_color != WIN_COLOR_UNDEFINED,
		To_UI_Color(border_color),
		color != WIN_COLOR_UNDEFINED,
		To_UI_Color(color)) ? TRUE : FALSE;
}

Bool W3DGameWinBorderDrawData(GameWindow *window, WinInstanceData *instData, void *drawList)
{
	if (window == nullptr || instData == nullptr || drawList == nullptr)
		return FALSE;

	Int original_x = 0;
	Int original_y = 0;
	window->winGetScreenPosition(&original_x, &original_y);
	Int border_x = original_x;
	Int border_y = original_y;
	Int border_width = 0;
	Int border_height = 0;
	window->winGetSize(&border_width, &border_height);

	for (Int bit = 0; bit < static_cast<Int>(sizeof(UnsignedInt) * 8); ++bit) {
		const UnsignedInt style = 1u << bit;
		if ((instData->getStyle() & style) == 0)
			continue;

		switch (instData->getStyle() & style) {
			case GWS_CHECK_BOX:
		case GWS_VERT_SLIDER:
			case GWS_HORZ_SLIDER:
				return TRUE;
			case GWS_ENTRY_FIELD: {
				if (instData->getTextLength()) {
					Int text_width = 0;
					TheWindowManager->winGetTextSize(
						instData->getFont(), instData->getText(), &text_width, nullptr, 0);
					border_width -= text_width + 6;
					border_x += text_width + 6;
				}
				break;
			}
			case GWS_SCROLL_LISTBOX: {
				ListboxData *list_data = static_cast<ListboxData *>(window->winGetUserData());
				Int slider_adjustment = 0;
				if (list_data != nullptr && list_data->scrollBar && list_data->slider != nullptr) {
					ICoord2D slider_size;
					list_data->slider->winGetSize(&slider_size.x, &slider_size.y);
					slider_adjustment = slider_size.y;
				}
				border_x -= 3;
				border_y -= 3 + (instData->getTextLength() ? 4 : 0);
				border_width += 3 - slider_adjustment;
				border_height += 6;
				break;
			}
			case GWS_RADIO_BUTTON:
			case GWS_STATIC_TEXT:
			case GWS_PROGRESS_BAR:
			case GWS_PUSH_BUTTON:
			case GWS_USER_WINDOW:
			case GWS_TAB_CONTROL:
				break;
			default:
				continue;
		}

		return Engine::UI::WND::Draw_Border(
			*static_cast<Engine::UI::WND::DrawList *>(drawList),
			Get_Border_Atlas(), border_x, border_y, border_width, border_height) ? TRUE : FALSE;
	}
	return TRUE;
}

// W3DGameWindow::winDrawBorder is retained for the GameWindow ABI.  WND
// border extraction is performed by W3DGameWinBorderDrawData above.
//=============================================================================
void W3DGameWindow::winDrawBorder()
{
	(void)this;
}

