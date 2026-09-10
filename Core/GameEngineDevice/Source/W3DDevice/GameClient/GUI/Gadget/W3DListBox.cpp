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

#include <algorithm>
#include <cstdint>

#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
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

struct ListBoxRenderContext final
{
	GameWindow *window = nullptr;
	ListboxData *list = nullptr;
	Int x = 0;
	Int content_width = 0;
	Int next_row_y = 0;
};

bool Is_Row_Selected(const ListboxData &list, Int row) noexcept
{
	if (list.multiSelect) {
		if (list.selections == nullptr)
			return false;
		for (Int index = 0; list.selections[index] >= 0; ++index) {
			if (list.selections[index] == row)
				return true;
		}
		return false;
	}
	return row == list.selectPos;
}

bool Query_List_Box_Row(
	void *context_pointer,
	std::uint32_t row_index,
	Engine::UI::WND::ListBoxRowVisual &row) noexcept
{
	ListBoxRenderContext &context = *static_cast<ListBoxRenderContext *>(context_pointer);
	if (context.list == nullptr || row_index >= static_cast<std::uint32_t>(context.list->endPos)
		|| context.list->listData == nullptr)
		return false;

	const Int row_number = static_cast<Int>(row_index);
	const Int row_height = context.list->listData[row_number].height + 1;
	row.rectangle = {
		static_cast<float>(context.x),
		static_cast<float>(context.next_row_y),
		static_cast<float>(context.x + context.content_width),
		static_cast<float>(context.next_row_y + row_height)};
	row.selected = Is_Row_Selected(*context.list, row_number);
	context.next_row_y += row_height;
	return true;
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
		left = GadgetListBoxGetDisabledSelectedItemImageLeft(window);
		right = GadgetListBoxGetDisabledSelectedItemImageRight(window);
		center = GadgetListBoxGetDisabledSelectedItemImageCenter(window);
		small_center = GadgetListBoxGetDisabledSelectedItemImageSmallCenter(window);
	}
	else if (instance_data != nullptr && BitIsSet(instance_data->getState(), WIN_STATE_HILITED)) {
		left = GadgetListBoxGetHiliteSelectedItemImageLeft(window);
		right = GadgetListBoxGetHiliteSelectedItemImageRight(window);
		center = GadgetListBoxGetHiliteSelectedItemImageCenter(window);
		small_center = GadgetListBoxGetHiliteSelectedItemImageSmallCenter(window);
	}
	else {
		left = GadgetListBoxGetEnabledSelectedItemImageLeft(window);
		right = GadgetListBoxGetEnabledSelectedItemImageRight(window);
		center = GadgetListBoxGetEnabledSelectedItemImageCenter(window);
		small_center = GadgetListBoxGetEnabledSelectedItemImageSmallCenter(window);
	}
}

Engine::UI::WND::ListBoxSelectionVisual Build_Selection(
	GameWindow *window,
	WinInstanceData *instance_data,
	Bool use_images) noexcept
{
	Engine::UI::WND::ListBoxSelectionVisual selection;
	if (use_images) {
		const Image *left = nullptr;
		const Image *right = nullptr;
		const Image *center = nullptr;
		const Image *small_center = nullptr;
		Select_Images(window, instance_data, left, right, center, small_center);
		if (left != nullptr && right != nullptr && center != nullptr && small_center != nullptr) {
			selection.segmented_image = true;
			selection.left_image = To_WND_Image(left);
			selection.center_image = To_WND_Image(center);
			selection.small_center_image = To_WND_Image(small_center);
			selection.right_image = To_WND_Image(right);
			selection.left_width = static_cast<float>(left->getImageWidth());
			selection.right_width = static_cast<float>(right->getImageWidth());
			selection.center_width = static_cast<float>(center->getImageWidth());
			selection.small_center_width = static_cast<float>(small_center->getImageWidth());
		}
		return selection;
	}

	Color fill = WIN_COLOR_UNDEFINED;
	Color border = WIN_COLOR_UNDEFINED;
	if (!BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED)) {
		fill = GadgetListBoxGetDisabledSelectedItemColor(window);
		border = GadgetListBoxGetDisabledSelectedItemBorderColor(window);
	}
	else if (instance_data != nullptr && BitIsSet(instance_data->getState(), WIN_STATE_HILITED)) {
		fill = GadgetListBoxGetHiliteSelectedItemColor(window);
		border = GadgetListBoxGetHiliteSelectedItemBorderColor(window);
	}
	else {
		fill = GadgetListBoxGetEnabledSelectedItemColor(window);
		border = GadgetListBoxGetEnabledSelectedItemBorderColor(window);
	}
	selection.has_fill = fill != WIN_COLOR_UNDEFINED;
	selection.fill = To_WND_Color(fill);
	selection.has_border = border != WIN_COLOR_UNDEFINED;
	selection.border = To_WND_Color(border);
	return selection;
}

bool Emit_List_Box_Cell(
	void *context_pointer,
	Engine::UI::WND::DrawList &draw_list,
	std::uint32_t row_index,
	std::uint32_t column_index,
	Graphics::Rect2D row_rectangle,
	Graphics::Rect2D cell_clip) noexcept
{
	ListBoxRenderContext &context = *static_cast<ListBoxRenderContext *>(context_pointer);
	if (context.list == nullptr || context.list->listData == nullptr
		|| column_index >= static_cast<std::uint32_t>(context.list->columns))
		return true;

	const Int row = static_cast<Int>(row_index);
	const Int column = static_cast<Int>(column_index);
	ListEntryCell *cells = context.list->listData[row].cell;
	if (cells == nullptr)
		return true;

	Int column_x = context.x;
	for (Int index = 0; index < column; ++index)
		column_x += context.list->columnWidth[index];
	Int column_width = context.list->columnWidth[column];
	if (context.list->columns == 1 && context.list->slider != nullptr
		&& context.list->slider->winIsHidden())
		column_width = context.content_width - (column_x - context.x) - 3;

	const Graphics::Rect2D column_rectangle{
		static_cast<float>(column_x), row_rectangle.top,
		static_cast<float>(column_x + column_width), row_rectangle.bottom};
	const Graphics::Rect2D clip{
		std::max(column_rectangle.left, cell_clip.left),
		std::max(column_rectangle.top, cell_clip.top),
		std::min(column_rectangle.right, cell_clip.right),
		std::min(column_rectangle.bottom, cell_clip.bottom)};
	if (clip.right <= clip.left || clip.bottom <= clip.top)
		return true;

	ListEntryCell &cell = cells[column];
	if (cell.cellType == LISTBOX_TEXT && cell.data != nullptr) {
		W3DDisplayString *text = static_cast<W3DDisplayString *>(
			static_cast<DisplayString *>(cell.data));
		if (text->getFont() != context.window->winGetFont())
			text->setFont(context.window->winGetFont());
		if (BitIsSet(context.window->winGetStatus(), WIN_STATUS_ONE_LINE))
			text->setWordWrap(0);
		IRegion2D text_clip{
			{static_cast<Int>(clip.left), static_cast<Int>(clip.top)},
			{static_cast<Int>(clip.right), static_cast<Int>(clip.bottom)}};
		return text->appendDrawData(
			draw_list,
			column_x + TEXT_X_OFFSET,
			static_cast<Int>(row_rectangle.top),
			cell.color,
			GameMakeColor(0, 0, 0, 255),
			1,
			1,
			&text_clip);
	}

	if (cell.cellType != LISTBOX_IMAGE || cell.data == nullptr)
		return true;
	const Image *image = static_cast<const Image *>(cell.data);
	Int width = cell.width > 0 ? cell.width : column_width;
	Int height = cell.height > 0 ? cell.height : context.list->listData[row].height;
	if (column == 0)
		--width;
	Int offset_x = width < column_width
		? column_x + (column_width - width) / 2
		: column_x;
	Int offset_y = height < context.list->listData[row].height
		? static_cast<Int>(row_rectangle.top) +
			(context.list->listData[row].height - height) / 2
		: static_cast<Int>(row_rectangle.top);
	++offset_y;
	if (offset_x < context.x + 1)
		offset_x = context.x + 1;
	const Graphics::Rect2D source_rectangle{
		static_cast<float>(offset_x), static_cast<float>(offset_y),
		static_cast<float>(offset_x + width), static_cast<float>(offset_y + height)};
	return Engine::UI::WND::Add_Clipped_Image(
		draw_list, To_WND_Image(image), source_rectangle, clip, To_WND_Color(cell.color));
}

bool Add_List_Box_Rows(
	Engine::UI::WND::DrawList &draw_list,
	GameWindow *window,
	WinInstanceData *instance_data,
	ListboxData *list,
	Int x,
	Int y,
	Int width,
	Int height,
	Bool use_images) noexcept
{
	if (list == nullptr || list->listData == nullptr || list->endPos <= 0)
		return true;

	ListBoxRenderContext context;
	context.window = window;
	context.list = list;
	context.x = x;
	context.content_width = width;
	context.next_row_y = y - list->displayPos;

	Engine::UI::WND::ListBoxVisual visual;
	visual.clip_rectangle = {
		static_cast<float>(x + 1), static_cast<float>(y - 3),
		static_cast<float>(x + width - 1), static_cast<float>(y + height - 1)};
	visual.row_count = static_cast<std::uint32_t>(list->endPos);
	visual.column_count = static_cast<std::uint32_t>(std::max<Int>(list->columns, 0));
	visual.selection = Build_Selection(window, instance_data, use_images);
	visual.context = &context;
	visual.query_row = &Query_List_Box_Row;
	visual.emit_cell = &Emit_List_Box_Cell;
	return Engine::UI::WND::Add_List_Box_Visual(draw_list, visual);
}

bool Extract_List_Box(
	GameWindow *window,
	WinInstanceData *instance_data,
	Engine::UI::WND::DrawList &draw_list,
	Bool use_images) noexcept
{
	if (window == nullptr || instance_data == nullptr)
		return false;
	ListboxData *list = static_cast<ListboxData *>(window->winGetUserData());
	if (list == nullptr)
		return true;

	Int width = 0;
	Int height = 0;
	Int x = 0;
	Int y = 0;
	ICoord2D size;
	window->winGetScreenPosition(&x, &y);
	window->winGetSize(&size.x, &size.y);
	width = size.x;
	height = size.y;

	const Bool enabled = BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED);
	const Bool highlighted = BitIsSet(instance_data->getState(), WIN_STATE_HILITED);
	Color title_color = enabled
		? (highlighted ? window->winGetHiliteTextColor() : window->winGetEnabledTextColor())
		: window->winGetDisabledTextColor();
	Color title_border = enabled
		? (highlighted ? window->winGetHiliteTextBorderColor() : window->winGetEnabledTextBorderColor())
		: window->winGetDisabledTextBorderColor();
	DisplayString *title = instance_data->getTextDisplayString();
	const Int font_height = TheWindowManager->winFontHeight(instance_data->getFont());
	if (use_images) {
		const Image *image = !enabled
			? GadgetListBoxGetDisabledImage(window)
			: highlighted ? GadgetListBoxGetHiliteImage(window) : GadgetListBoxGetEnabledImage(window);
		if (image != nullptr && !draw_list.Add_Image(To_WND_Image(image), {
			static_cast<float>(x + instance_data->m_imageOffset.x),
			static_cast<float>(y + instance_data->m_imageOffset.y),
			static_cast<float>(x + instance_data->m_imageOffset.x + width),
			static_cast<float>(y + instance_data->m_imageOffset.y + height)}))
			return false;
		if (list->slider != nullptr) {
			ICoord2D slider_size;
			list->slider->winGetSize(&slider_size.x, &slider_size.y);
			width -= slider_size.x;
		}
	}
	if (title != nullptr && title->getTextLength() != 0) {
		if (title->getFont() != window->winGetFont())
			title->setFont(window->winGetFont());
		W3DDisplayString *display_title = static_cast<W3DDisplayString *>(title);
		if (!display_title->appendDrawData(draw_list, x + 1, y, title_color, title_border))
			return false;
		y += font_height + 1;
		height -= font_height + 1;
	}

	if (!use_images) {
		Color background = WIN_COLOR_UNDEFINED;
		Color border = WIN_COLOR_UNDEFINED;
		if (!enabled) {
			background = GadgetListBoxGetDisabledColor(window);
			border = GadgetListBoxGetDisabledBorderColor(window);
		}
		else if (highlighted) {
			background = GadgetListBoxGetHiliteColor(window);
			border = GadgetListBoxGetHiliteBorderColor(window);
		}
		else {
			background = GadgetListBoxGetEnabledColor(window);
			border = GadgetListBoxGetEnabledBorderColor(window);
		}
		if (border != WIN_COLOR_UNDEFINED && !draw_list.Add_Outline(
			{static_cast<float>(x), static_cast<float>(y),
			 static_cast<float>(x + width), static_cast<float>(y + height)},
			WIN_DRAW_LINE_WIDTH, To_WND_Color(border)))
			return false;
		if (background != WIN_COLOR_UNDEFINED && !draw_list.Add_Rect(
			{static_cast<float>(x + 1), static_cast<float>(y + 1),
			 static_cast<float>(x + width - 1), static_cast<float>(y + height - 1)},
			To_WND_Color(background)))
			return false;
		if (list->slider != nullptr && !list->slider->winIsHidden()) {
			ICoord2D slider_size;
			list->slider->winGetSize(&slider_size.x, &slider_size.y);
			width -= slider_size.x + 3;
		}
	}

	return Add_List_Box_Rows(draw_list, window, instance_data, list, x, y + 4, width, height - 4, use_images);
}

} // namespace

Bool W3DGadgetListBoxDrawData(GameWindow *window, WinInstanceData *instData, void *drawList)
{
	return Extract_List_Box(
		window, instData, *static_cast<Engine::UI::WND::DrawList *>(drawList), FALSE) ? TRUE : FALSE;
}

Bool W3DGadgetListBoxImageDrawData(GameWindow *window, WinInstanceData *instData, void *drawList)
{
	return Extract_List_Box(
		window, instData, *static_cast<Engine::UI::WND::DrawList *>(drawList), TRUE) ? TRUE : FALSE;
}

