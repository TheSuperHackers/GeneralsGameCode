/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#pragma once

#include "WWLib/Usp10Loader.h"
#include "unicodebidi.h"
#include <algorithm>
#include <string.h>
#include <vector>
#include <limits.h>

struct ComplexTextRun
{
	ComplexTextRun () :
		Analysis(nullptr),
		Font(nullptr),
		CharacterPosition(0),
		CharacterCount(0),
		Width(0),
		Ascent(0)
	{
		::memset(&State, 0, sizeof(State));
	}

	Usp10Loader::ScriptStringAnalysis Analysis;
	HFONT Font;
	int CharacterPosition;
	int CharacterCount;
	int Width;
	int Ascent;
	Usp10Loader::ScriptState State;
};


struct ComplexTextLayout
{
	ComplexTextLayout (HDC source_dc) :
		DC(::CreateCompatibleDC(source_dc)),
		Width(0), Height(0), Ascent(0), Descent(0)
	{
	}

	~ComplexTextLayout ()
	{
		for (size_t index = 0; index < Runs.size(); ++index) {
			if (Runs[index].Analysis != nullptr) {
				Usp10Loader::ScriptStringFree(&Runs[index].Analysis);
			}
		}

		if (DC != nullptr) {
			::DeleteDC(DC);
		}
	}

	HDC DC;
	std::vector<ComplexTextRun> Runs;
	std::vector<int> VisualToLogical;
	int Width;
	int Height;
	int Ascent;
	int Descent;

private:
	ComplexTextLayout (const ComplexTextLayout &);
	ComplexTextLayout &operator= (const ComplexTextLayout &);
};


static bool Uses_Alternate_Unicode_Font (const WCHAR *text, int character_position, HFONT alternate_font)
{
	return alternate_font != nullptr && text[character_position] >= 256;
}


static bool Build_Complex_Text_Layout (HDC dc, const WCHAR *text, int text_length,
	HFONT primary_font, HFONT alternate_font, ComplexTextLayout *layout)
{
	if (dc == nullptr || text == nullptr || text_length <= 0 ||
		text_length > (INT_MAX - 16) / 3 || primary_font == nullptr)
	{
		return false;
	}

	if (alternate_font == primary_font) {
		alternate_font = nullptr;
	}

	// TheSuperHackers @bugfix Omar Aglan 13/09/2026 Preserve paragraph order for mixed RTL text.
	// ScriptItemize needs both control and state for full bidirectional analysis.
	Usp10Loader::ScriptControl control = { 0 };
	Usp10Loader::ScriptState initial_state = { 0 };
	initial_state.bidi_level = UnicodeBidi::Get_Paragraph_Level(text, text_length);

	std::vector<Usp10Loader::ScriptItem> items(text_length + 2);
	int item_count = 0;
	if (Usp10Loader::ScriptItemize(text, text_length, text_length + 1, &control, &initial_state, &items[0], &item_count) != S_OK ||
		item_count <= 0)
	{
		return false;
	}

	std::vector<Usp10Loader::ScriptLogAttr> attributes(text_length);
	for (int break_item_index = 0; break_item_index < item_count; ++break_item_index) {
		const int item_start = items[break_item_index].character_position;
		const int item_length = items[break_item_index + 1].character_position - item_start;
		if (Usp10Loader::ScriptBreak(text + item_start, item_length, &items[break_item_index].analysis,
			&attributes[item_start]) != S_OK)
		{
			return false;
		}
	}

	layout->Runs.reserve(text_length);
	for (int run_item_index = 0; run_item_index < item_count; ++run_item_index) {
		const int item_end = items[run_item_index + 1].character_position;
		int run_start = items[run_item_index].character_position;
		bool uses_alternate_font = Uses_Alternate_Unicode_Font(text, run_start, alternate_font);
		for (int run_end = run_start + 1; run_end <= item_end; ++run_end) {
			const bool run_ends = run_end == item_end ||
				(attributes[run_end].char_stop &&
					uses_alternate_font != Uses_Alternate_Unicode_Font(text, run_end, alternate_font));
			if (run_ends) {
				layout->Runs.push_back(ComplexTextRun());
				ComplexTextRun &run = layout->Runs.back();
				run.Font = uses_alternate_font ? alternate_font : primary_font;
				run.CharacterPosition = run_start;
				run.CharacterCount = run_end - run_start;
				run.State = items[run_item_index].analysis.state;
				run_start = run_end;
				if (run_end < item_end) {
					uses_alternate_font = Uses_Alternate_Unicode_Font(text, run_end, alternate_font);
				}
			}
		}
	}
	const int run_count = (int)layout->Runs.size();
	layout->VisualToLogical.resize(run_count);

	std::vector<BYTE> bidi_levels(run_count);
	HFONT old_font = (HFONT)::GetCurrentObject(dc, OBJ_FONT);
	bool success = true;
	for (int index = 0; index < run_count; ++index) {
		ComplexTextRun &run = layout->Runs[index];
		if (::SelectObject(dc, run.Font) == nullptr) {
			success = false;
			break;
		}

		bidi_levels[index] = (BYTE)run.State.bidi_level;
		const int glyph_count = run.CharacterCount + run.CharacterCount / 2 + 16;
		DWORD flags = Usp10Loader::SSA_GLYPHS | Usp10Loader::SSA_FALLBACK;
		if ((run.State.bidi_level & 1) != 0) {
			flags |= Usp10Loader::SSA_RTL;
		}

		// TheSuperHackers @bugfix Omar Aglan 13/09/2026 Keep directional overrides inside font runs.
		const HRESULT analysis_result = Usp10Loader::ScriptStringAnalyse(dc,
			text + run.CharacterPosition, run.CharacterCount, glyph_count, -1, flags, 0,
			&control, &run.State, nullptr, nullptr, nullptr, &run.Analysis);
		const SIZE *run_size = analysis_result == S_OK ? Usp10Loader::ScriptString_pSize(run.Analysis) : nullptr;
		TEXTMETRIC text_metrics = { 0 };
		if (run_size == nullptr || run_size->cx < 0 || run_size->cx > INT_MAX - layout->Width ||
			run_size->cy <= 0 || !::GetTextMetrics(dc, &text_metrics))
		{
			success = false;
			break;
		}

		run.Width = run_size->cx;
		run.Ascent = (std::min)((int)text_metrics.tmAscent, (int)run_size->cy);
		layout->Width += run.Width;
		layout->Ascent = (std::max)(layout->Ascent, run.Ascent);
		layout->Descent = (std::max)(layout->Descent, (int)run_size->cy - run.Ascent);
	}
	if (layout->Ascent > INT_MAX - layout->Descent) {
		success = false;
	} else {
		layout->Height = layout->Ascent + layout->Descent;
	}

	if (success) {
		success = Usp10Loader::ScriptLayout(run_count, &bidi_levels[0], &layout->VisualToLogical[0], nullptr) == S_OK &&
			layout->Width > 0 && layout->Height > 0;
	}

	::SelectObject(dc, old_font);
	return success;
}


static bool Draw_Complex_Text_Layout (ComplexTextLayout &layout)
{
	HFONT old_font = (HFONT)::GetCurrentObject(layout.DC, OBJ_FONT);
	int x = 0;
	bool success = true;
	for (size_t visual_index = 0; visual_index < layout.Runs.size(); ++visual_index) {
		ComplexTextRun &run = layout.Runs[layout.VisualToLogical[visual_index]];
		if (::SelectObject(layout.DC, run.Font) == nullptr ||
			Usp10Loader::ScriptStringOut(run.Analysis, x, layout.Ascent - run.Ascent,
				0, nullptr, 0, 0, FALSE) != S_OK)
		{
			success = false;
			break;
		}
		x += run.Width;
	}
	::SelectObject(layout.DC, old_font);

	// TheSuperHackers @bugfix Omar Aglan 13/09/2026 Complete GDI drawing before the bitmap is read or freed.
	return ::GdiFlush() != FALSE && success;
}
