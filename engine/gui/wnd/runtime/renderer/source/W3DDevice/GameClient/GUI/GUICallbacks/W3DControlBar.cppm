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

// FILE: W3DControlBar.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day
// Desc: Control bar callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

module;

#include "Precompiled/PreRTS.h"
#include <algorithm>

#include "Common/GameUtility.h"
#include "Common/GlobalData.h"
#include "Common/Radar.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameClient/GameWindow.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Display.h"
#include "GameClient/ControlBar.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/ControlBarScheme.h"
#include "GameClient/MapUtil.h"
#include "GameLogic/GameLogic.h"
#include "Common/NameKeyGenerator.h"

export module Engine.UI.WND.Runtime.Renderer.W3DControlBar;

import Engine.UI.WND;

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
	Int bottom)
{
	if (image == nullptr)
		return TRUE;
	return draw_list.Add_Image(
		To_WND_Image(image),
		{static_cast<float>(left), static_cast<float>(top),
			static_cast<float>(right), static_cast<float>(bottom)}) ? TRUE : FALSE;
}

Bool Add_WND_Tiled_Image(
	Engine::UI::WND::DrawList &draw_list,
	const Image *image,
	Int left,
	Int top,
	Int right,
	Int bottom,
	Bool vertical = FALSE)
{
	if (image == nullptr)
		return TRUE;
	const Engine::UI::WND::ImageRef image_ref = To_WND_Image(image);
	float cursor = 0.0f;
	if (vertical) {
		return Engine::UI::WND::Add_Vertical_Tiled_Image(
			draw_list,
			image_ref,
			{static_cast<float>(left), static_cast<float>(top),
				static_cast<float>(right), static_cast<float>(bottom)},
			static_cast<float>(image->getImageHeight()), cursor) ? TRUE : FALSE;
	}
	return Engine::UI::WND::Add_Tiled_Image(
		draw_list,
		image_ref,
		{static_cast<float>(left), static_cast<float>(top),
			static_cast<float>(right), static_cast<float>(bottom)},
		static_cast<float>(image->getImageWidth()), cursor) ? TRUE : FALSE;
}

Bool Add_Control_Bar_Scheme_Layer(
	Engine::UI::WND::DrawList &draw_list,
	Bool foreground)
{
	if (TheControlBar == nullptr || TheWindowManager == nullptr || TheNameKeyGenerator == nullptr)
		return FALSE;
	ControlBarSchemeManager *manager = TheControlBar->getControlBarSchemeManager();
	ControlBarScheme *scheme = manager != nullptr ? manager->getCurrentScheme() : nullptr;
	if (scheme == nullptr)
		return TRUE;

	const NameKeyType marker_key = TheNameKeyGenerator->nameToKey(
		"ControlBar.wnd:BackgroundMarker");
	GameWindow *marker = TheWindowManager->winGetWindowFromId(nullptr, marker_key);
	if (marker == nullptr)
		return TRUE;

	ICoord2D marker_position;
	marker->winGetScreenPosition(&marker_position.x, &marker_position.y);
	ICoord2D base_position;
	if (foreground)
		TheControlBar->getForegroundMarkerPos(&base_position.x, &base_position.y);
	else
		TheControlBar->getBackgroundMarkerPos(&base_position.x, &base_position.y);
	const Coord2D screen_offset = manager->getScreenOffset();
	const Coord2D offset{
		marker_position.x - base_position.x + screen_offset.x,
		marker_position.y - base_position.y + screen_offset.y};
	const Coord2D multiplier = manager->getMultiplier();
	const Int first_layer = foreground ? CONTROL_BAR_SCHEME_FOREGROUND_IMAGE_LAYERS - 1
		: MAX_CONTROL_BAR_SCHEME_IMAGE_LAYERS - 1;
	const Int last_layer = foreground ? 0 : CONTROL_BAR_SCHEME_FOREGROUND_IMAGE_LAYERS;
	for (Int layer = first_layer; layer >= last_layer; --layer) {
		for (ControlBarSchemeImage *image : scheme->m_layer[layer]) {
			if (image == nullptr || image->m_image == nullptr)
				continue;
			const Int left = static_cast<Int>(image->m_position.x * multiplier.x + offset.x);
			const Int top = static_cast<Int>(image->m_position.y * multiplier.y + offset.y);
			const Int right = static_cast<Int>(
				(image->m_position.x + image->m_size.x) * multiplier.x + offset.x);
			const Int bottom = static_cast<Int>(
				(image->m_position.y + image->m_size.y) * multiplier.y + offset.y);
			if (!Add_WND_Image(draw_list, image->m_image, left, top, right, bottom))
				return FALSE;
		}
	}
	return TRUE;
}

}

Bool W3DCameoMovieDrawData(GameWindow *, WinInstanceData *, void *)
{
	return TRUE;
}

Real logN(Real value, Real logBase)
{
	return (Real)log10(value)/ log10(logBase);
}

Bool W3DCommandBarGridDrawData(GameWindow *window, WinInstanceData *instData, void *drawList)
{
	if (window == nullptr || instData == nullptr || drawList == nullptr || TheControlBar == nullptr)
		return FALSE;
	if (BitIsSet(window->winGetStatus(), WIN_STATUS_IMAGE))
		return W3DGameWinDefaultDrawData(window, instData, drawList);

	ICoord2D position, size;
	window->winGetScreenPosition(&position.x, &position.y);
	window->winGetSize(&size.x, &size.y);
	const Color color = TheControlBar->getBorderColor();
	window->winSetEnabledBorderColor(0, color);
	Engine::UI::WND::DrawList &list = *static_cast<Engine::UI::WND::DrawList *>(drawList);
	if (!W3DGameWinDefaultDrawData(window, instData, drawList))
		return FALSE;
	return list.Add_Line(
		{static_cast<float>(position.x), position.y + size.y * .33f},
		{static_cast<float>(position.x + size.x), position.y + size.y * .33f},
		1.0f, {static_cast<float>((color >> 16) & 0xff) / 255.0f,
			static_cast<float>((color >> 8) & 0xff) / 255.0f,
			static_cast<float>(color & 0xff) / 255.0f,
			static_cast<float>((color >> 24) & 0xff) / 255.0f})
		&& list.Add_Line(
			{static_cast<float>(position.x), position.y + size.y * .66f},
			{static_cast<float>(position.x + size.x), position.y + size.y * .66f},
			1.0f, {static_cast<float>((color >> 16) & 0xff) / 255.0f,
				static_cast<float>((color >> 8) & 0xff) / 255.0f,
				static_cast<float>(color & 0xff) / 255.0f,
				static_cast<float>((color >> 24) & 0xff) / 255.0f})
		&& list.Add_Line(
			{position.x + size.x * .33f, static_cast<float>(position.y)},
			{position.x + size.x * .33f, static_cast<float>(position.y + size.y)},
			1.0f, {static_cast<float>((color >> 16) & 0xff) / 255.0f,
				static_cast<float>((color >> 8) & 0xff) / 255.0f,
				static_cast<float>(color & 0xff) / 255.0f,
				static_cast<float>((color >> 24) & 0xff) / 255.0f})
		&& list.Add_Line(
			{position.x + size.x * .66f, static_cast<float>(position.y)},
			{position.x + size.x * .66f, static_cast<float>(position.y + size.y)},
			1.0f, {static_cast<float>((color >> 16) & 0xff) / 255.0f,
				static_cast<float>((color >> 8) & 0xff) / 255.0f,
				static_cast<float>(color & 0xff) / 255.0f,
			static_cast<float>((color >> 24) & 0xff) / 255.0f}) ? TRUE : FALSE;
}

Bool W3DPowerDrawData(GameWindow *window, WinInstanceData *, void *drawList)
{
	if (window == nullptr || drawList == nullptr || TheMappedImageCollection == nullptr
		|| TheControlBar == nullptr || TheGlobalData == nullptr)
		return FALSE;
	Player *player = TheControlBar->getCurrentlyViewedPlayer();
	if (player == nullptr)
		return TRUE;
	Energy *energy = player->getEnergy();
	if (energy == nullptr)
		return TRUE;

	const Image *center_bar = nullptr;
	const Image *slider = TheMappedImageCollection->findImageByName("PowerBarSlider");
	const Int consumption = energy->getConsumption();
	const Int production = energy->getProduction();
	const Int delta = TheGlobalData->m_powerBarYellowRange;
	if (consumption > production - delta && consumption <= production)
		center_bar = TheMappedImageCollection->findImageByName("PowerPointY");
	else if (consumption > production)
		center_bar = TheMappedImageCollection->findImageByName("PowerPointR");
	else
		center_bar = TheMappedImageCollection->findImageByName("PowerPointG");
	if (center_bar == nullptr || slider == nullptr || production <= 0
		|| TheGlobalData->m_powerBarIntervals <= 0)
		return TRUE;

	ICoord2D position, size;
	window->winGetScreenPosition(&position.x, &position.y);
	window->winGetSize(&size.x, &size.y);
	const Int range = std::min(
		size.x,
		static_cast<Int>(logN(production, TheGlobalData->m_powerBarBase)
			* (size.x / TheGlobalData->m_powerBarIntervals)));
	Engine::UI::WND::DrawList &list =
		*static_cast<Engine::UI::WND::DrawList *>(drawList);
	if (range > 0 && !Add_WND_Tiled_Image(
		list, center_bar, position.x, position.y, position.x + range, position.y + size.y))
		return FALSE;

	const Real consumption_for_needle = consumption == 1 ? 1.5f : INT_TO_REAL(consumption);
	const Int needle_range = consumption_for_needle > 0.0f
		? static_cast<Int>(logN(consumption_for_needle, TheGlobalData->m_powerBarBase)
			* (size.x / TheGlobalData->m_powerBarIntervals))
		: 0;
	Int needle_left = position.x + needle_range - slider->getImageWidth() / 2;
	Int needle_right = needle_left + slider->getImageWidth();
	if (needle_range >= size.x) {
		needle_left = position.x + size.x - slider->getImageWidth();
		needle_right = position.x + size.x;
	}
	if (needle_left <= position.x) {
		needle_left = position.x;
		needle_right = position.x + slider->getImageWidth();
	}
	return Add_WND_Image(
		list,
		slider,
		needle_left,
		position.y + size.y - slider->getImageHeight(),
		needle_right,
		position.y + size.y);
}

Bool W3DCommandBarGenExpDrawData(GameWindow *window, WinInstanceData *, void *drawList)
{
	if (window == nullptr || drawList == nullptr || TheMappedImageCollection == nullptr
		|| TheControlBar == nullptr)
		return FALSE;
	Player *player = TheControlBar->getCurrentlyViewedPlayer();
	if (player == nullptr)
		return TRUE;
	const Image *top = TheMappedImageCollection->findImageByName("GenExpBarTop1");
	const Image *bottom = TheMappedImageCollection->findImageByName("GenExpBarBottom1");
	const Image *center = TheMappedImageCollection->findImageByName("GenExpBar1");
	const Int required = player->getSkillPointsLevelUp() - player->getSkillPointsLevelDown();
	if (top == nullptr || bottom == nullptr || center == nullptr || required <= 0)
		return TRUE;
	const Int progress = std::clamp(
		((player->getSkillPoints() - player->getSkillPointsLevelDown()) * 100) / required,
		0,
		100);
	if (progress <= 0)
		return TRUE;
	ICoord2D position, size;
	window->winGetScreenPosition(&position.x, &position.y);
	window->winGetSize(&size.x, &size.y);
	const Int range = size.y * progress / 100;
	const Int bottom_y = position.y + size.y - bottom->getImageHeight();
	const Int top_y = position.y + size.y - range - top->getImageHeight();
	Engine::UI::WND::DrawList &list =
		*static_cast<Engine::UI::WND::DrawList *>(drawList);
	if (bottom_y <= top_y) {
		return Add_WND_Image(list, bottom, position.x, position.y + size.y - bottom->getImageHeight(),
			position.x + size.x, position.y + size.y)
			&& Add_WND_Image(list, top, position.x, top_y, position.x + size.x,
				top_y + top->getImageHeight());
	}
	float cursor = 0.0f;
	if (!Engine::UI::WND::Add_Vertical_Tiled_Image(
			list,
			To_WND_Image(center),
			{static_cast<float>(position.x), static_cast<float>(top_y),
				static_cast<float>(position.x + size.x), static_cast<float>(bottom_y)},
			static_cast<float>(center->getImageHeight()), cursor))
		return FALSE;
	return Add_WND_Image(list, bottom, position.x, bottom_y, position.x + size.x, position.y + size.y)
		&& Add_WND_Image(list, top, position.x, position.y + size.y - range - top->getImageHeight(),
			position.x + size.x, position.y + size.y - range);
}

Bool W3DCommandBarHelpPopupDrawData(GameWindow *window, WinInstanceData *, void *drawList)
{
	if (window == nullptr || drawList == nullptr || TheMappedImageCollection == nullptr)
		return FALSE;
	const Image *top = TheMappedImageCollection->findImageByName("Helpbox-top");
	const Image *bottom = TheMappedImageCollection->findImageByName("Helpbox-bottom");
	const Image *center = TheMappedImageCollection->findImageByName("Helpbox-middle");
	if (top == nullptr || bottom == nullptr || center == nullptr)
		return TRUE;
	ICoord2D position, size;
	window->winGetScreenPosition(&position.x, &position.y);
	window->winGetSize(&size.x, &size.y);
	Engine::UI::WND::DrawList &list =
		*static_cast<Engine::UI::WND::DrawList *>(drawList);
	const Int center_top = position.y + top->getImageHeight();
	const Int center_bottom = position.y + size.y - bottom->getImageHeight();
	float cursor = 0.0f;
	if (!Engine::UI::WND::Add_Vertical_Tiled_Image(
			list,
			To_WND_Image(center),
			{static_cast<float>(position.x), static_cast<float>(center_top),
				static_cast<float>(position.x + size.x), static_cast<float>(center_bottom)},
			static_cast<float>(center->getImageHeight()), cursor))
		return FALSE;
	return Add_WND_Image(list, bottom, position.x, center_bottom, position.x + size.x, position.y + size.y)
		&& Add_WND_Image(list, top, position.x, position.y, position.x + size.x,
			position.y + top->getImageHeight());
}

Bool W3DLeftHUDDrawData(GameWindow *window, WinInstanceData *, void *drawList)
{
	if (window == nullptr || drawList == nullptr) return FALSE;
	if (TheRadar == nullptr || !rts::localPlayerHasRadar()) return TRUE;
	ICoord2D position, size;
	window->winGetScreenPosition(&position.x, &position.y);
	window->winGetSize(&size.x, &size.y);
	return TheRadar->drawData(position.x + 1, position.y + 1, size.x - 2, size.y - 2, drawList);
}

Bool W3DRightHUDDrawData(GameWindow *window, WinInstanceData *instance_data, void *drawList)
{
	if (window == nullptr || instance_data == nullptr || drawList == nullptr)
		return FALSE;
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_IMAGE))
		return TRUE;
	return W3DGameWinDefaultDrawData(window, instance_data, drawList);
}

Bool W3DCommandBarBackgroundDrawData(GameWindow *, WinInstanceData *, void *drawList)
{
	return drawList != nullptr
		&& Add_Control_Bar_Scheme_Layer(
			*static_cast<Engine::UI::WND::DrawList *>(drawList), FALSE);
}

Bool W3DCommandBarForegroundDrawData(GameWindow *, WinInstanceData *, void *drawList)
{
	return drawList != nullptr
		&& Add_Control_Bar_Scheme_Layer(
			*static_cast<Engine::UI::WND::DrawList *>(drawList), TRUE);
}

Bool Add_WND_Skinny_Border(
	Engine::UI::WND::DrawList &list,
	Int x,
	Int y,
	Int width,
	Int height)
{
	if (TheMappedImageCollection == nullptr)
		return FALSE;
	const Int original_x = x;
	const Int original_y = y;
	const Int maximum_x = x + width;
	const Int maximum_y = y + height;
	const Int size = 5;
	const Int half_size = size / 2;
	const Int offset = 2;
	const Int lower_offset = 5;
	const Image *top = TheMappedImageCollection->findImageByName("FrameT");
	const Image *bottom = TheMappedImageCollection->findImageByName("FrameB");
	const Int horizontal_end = maximum_x - (lower_offset + size);
	for (x = original_x + 3; x <= horizontal_end; x += size) {
		if (!Add_WND_Image(list, top, x, original_y - offset, x + size, original_y - offset + size)
			|| !Add_WND_Image(list, bottom, x, maximum_y - lower_offset,
				x + size, maximum_y - lower_offset + size))
			return FALSE;
	}
	Int remainder = maximum_x - 5;
	if (remainder - x >= half_size) {
		if (!Add_WND_Image(list, top, x, original_y - offset, x + half_size, original_y - offset + size)
			|| !Add_WND_Image(list, bottom, x, maximum_y - lower_offset,
				x + half_size, maximum_y - lower_offset + size))
			return FALSE;
		x += half_size;
	}
	if (x < remainder) {
		x -= half_size - (((remainder - x) + 1) & ~1);
		if (!Add_WND_Image(list, top, x, original_y - offset, x + half_size, original_y - offset + size)
			|| !Add_WND_Image(list, bottom, x, maximum_y - lower_offset,
				x + half_size, maximum_y - lower_offset + size))
			return FALSE;
	}

	const Image *left = TheMappedImageCollection->findImageByName("FrameL");
	const Image *right = TheMappedImageCollection->findImageByName("FrameR");
	const Int vertical_end = maximum_y - (lower_offset + size);
	for (y = original_y + 3; y <= vertical_end; y += size) {
		if (!Add_WND_Image(list, left, original_x - offset, y,
			original_x - offset + size, y + size)
			|| !Add_WND_Image(list, right, maximum_x - lower_offset, y,
				maximum_x - lower_offset + size, y + size))
			return FALSE;
	}
	remainder = maximum_y - lower_offset;
	if (remainder - y >= half_size) {
		if (!Add_WND_Image(list, left, original_x - offset, y,
			original_x - offset + size, y + half_size)
			|| !Add_WND_Image(list, right, maximum_x - lower_offset, y,
				maximum_x - lower_offset + size, y + half_size))
			return FALSE;
		y += half_size;
	}
	if (y < remainder) {
		y -= half_size - (((remainder - y) + 1) & ~1);
		if (!Add_WND_Image(list, left, original_x - offset, y,
			original_x - offset + size, y + half_size)
			|| !Add_WND_Image(list, right, maximum_x - lower_offset, y,
				maximum_x - lower_offset + size, y + half_size))
			return FALSE;
	}

	return Add_WND_Image(list, TheMappedImageCollection->findImageByName("FrameCornerUL"),
		original_x - 2, original_y - 2, original_x + 3, original_y + 3)
		&& Add_WND_Image(list, TheMappedImageCollection->findImageByName("FrameCornerUR"),
			maximum_x - 5, original_y - 2, maximum_x, original_y + 3)
		&& Add_WND_Image(list, TheMappedImageCollection->findImageByName("FrameCornerLL"),
			original_x - 2, maximum_y - 5, original_x + 3, maximum_y)
		&& Add_WND_Image(list, TheMappedImageCollection->findImageByName("FrameCornerLR"),
			maximum_x - 5, maximum_y - 5, maximum_x, maximum_y);
}

Bool W3DDrawMapPreviewData(GameWindow *window, WinInstanceData *instance_data, void *drawList)
{
	if (window == nullptr || instance_data == nullptr || drawList == nullptr)
		return FALSE;
	MapMetaData *map_data = static_cast<MapMetaData *>(window->winGetUserData());
	ICoord2D position, size;
	window->winGetScreenPosition(&position.x, &position.y);
	window->winGetSize(&size.x, &size.y);
	Engine::UI::WND::DrawList &list =
		*static_cast<Engine::UI::WND::DrawList *>(drawList);
	if (size.x <= 0 || size.y <= 0)
		return TRUE;
	if (map_data == nullptr) {
		return W3DGameWinDefaultDrawData(window, instance_data, drawList)
			&& Add_WND_Skinny_Border(list, position.x - 1, position.y - 1, size.x + 2, size.y + 2);
	}

	ICoord2D upper_left, lower_right;
	findDrawPositions(position.x, position.y, size.x, size.y, map_data->m_extent,
		&upper_left, &lower_right);
	const Graphics::Color2D fill_color{0.0f, 0.0f, 0.0f, 1.0f};
	const Graphics::Color2D line_color{
		50.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f, 1.0f};
	if (map_data->m_extent.width() / size.x >= map_data->m_extent.height() / size.y) {
		if (upper_left.y > position.y && !list.Add_Rect(
			{static_cast<float>(position.x), static_cast<float>(position.y),
				static_cast<float>(position.x + size.x), static_cast<float>(upper_left.y)}, fill_color))
			return FALSE;
		if (lower_right.y < position.y + size.y && !list.Add_Rect(
			{static_cast<float>(position.x), static_cast<float>(lower_right.y),
				static_cast<float>(position.x + size.x), static_cast<float>(position.y + size.y)}, fill_color))
			return FALSE;
		if (!list.Add_Line({static_cast<float>(position.x), static_cast<float>(upper_left.y)},
			{static_cast<float>(position.x + size.x), static_cast<float>(upper_left.y)}, 1.0f, line_color)
			|| !list.Add_Line({static_cast<float>(position.x), static_cast<float>(lower_right.y + 1)},
				{static_cast<float>(position.x + size.x), static_cast<float>(lower_right.y + 1)}, 1.0f, line_color))
			return FALSE;
	}
	else {
		if (upper_left.x > position.x && !list.Add_Rect(
			{static_cast<float>(position.x), static_cast<float>(position.y),
				static_cast<float>(upper_left.x), static_cast<float>(position.y + size.y)}, fill_color))
			return FALSE;
		if (lower_right.x < position.x + size.x && !list.Add_Rect(
			{static_cast<float>(lower_right.x), static_cast<float>(position.y),
				static_cast<float>(position.x + size.x), static_cast<float>(position.y + size.y)}, fill_color))
			return FALSE;
		if (!list.Add_Line({static_cast<float>(upper_left.x), static_cast<float>(position.y)},
			{static_cast<float>(upper_left.x), static_cast<float>(position.y + size.y)}, 1.0f, line_color)
			|| !list.Add_Line({static_cast<float>(lower_right.x + 1), static_cast<float>(position.y)},
				{static_cast<float>(lower_right.x + 1), static_cast<float>(position.y + size.y)}, 1.0f, line_color))
			return FALSE;
	}
	const Image *map_image = BitIsSet(window->winGetStatus(), WIN_STATUS_IMAGE)
		? window->winGetEnabledImage(0) : nullptr;
	if (map_image != nullptr) {
		if (!Add_WND_Image(list, map_image, upper_left.x, upper_left.y,
			lower_right.x, lower_right.y))
			return FALSE;
	}
	else if (!list.Add_Rect(
		{static_cast<float>(upper_left.x), static_cast<float>(upper_left.y),
			static_cast<float>(lower_right.x), static_cast<float>(lower_right.y)}, line_color))
		return FALSE;

	const Image *marker = TheMappedImageCollection != nullptr
		? TheMappedImageCollection->findImageByName("TecBuilding") : nullptr;
	for (ICoord2DList::const_iterator it = TheSupplyAndTechImageLocations.m_techPosList.begin();
		marker != nullptr && it != TheSupplyAndTechImageLocations.m_techPosList.end(); ++it) {
		if (!Add_WND_Image(list, marker, position.x + it->x, position.y + it->y,
			position.x + it->x + SUPPLY_TECH_SIZE, position.y + it->y + SUPPLY_TECH_SIZE))
			return FALSE;
	}
	marker = TheMappedImageCollection != nullptr
		? TheMappedImageCollection->findImageByName("Cash") : nullptr;
	for (ICoord2DList::const_iterator it = TheSupplyAndTechImageLocations.m_supplyPosList.begin();
		marker != nullptr && it != TheSupplyAndTechImageLocations.m_supplyPosList.end(); ++it) {
		if (!Add_WND_Image(list, marker, position.x + it->x, position.y + it->y,
			position.x + it->x + SUPPLY_TECH_SIZE, position.y + it->y + SUPPLY_TECH_SIZE))
			return FALSE;
	}
	return Add_WND_Skinny_Border(list, position.x - 1, position.y - 1, size.x + 2, size.y + 2);
}

Bool W3DCommandBarTopDrawData(GameWindow *, WinInstanceData *, void *)
{
	return TRUE;
}

Bool W3DNoDrawData(GameWindow *, WinInstanceData *, void *)
{
	return TRUE;
}
