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

#include "mutex.h"
#include "win.h"


// This static class loads usp10.dll on first use and unloads it during engine shutdown.

class Usp10Loader
{
public:

	typedef void *ScriptStringAnalysis;
	struct ScriptControl;
	struct ScriptTabDefinition;

	struct ScriptState
	{
		WORD bidi_level : 5;
		WORD reserved : 11;
	};

	struct ScriptAnalysis
	{
		WORD script : 10;
		WORD right_to_left : 1;
		WORD layout_right_to_left : 1;
		WORD link_before : 1;
		WORD link_after : 1;
		WORD logical_order : 1;
		WORD no_glyph_index : 1;
		ScriptState state;
	};

	struct ScriptItem
	{
		int character_position;
		ScriptAnalysis analysis;
	};

	struct ScriptLogAttr
	{
		BYTE soft_break : 1;
		BYTE white_space : 1;
		BYTE char_stop : 1;
		BYTE word_stop : 1;
		BYTE invalid : 1;
		BYTE reserved : 3;
	};

	enum
	{
		SIC_COMPLEX = 0x00000001,
		SSA_FALLBACK = 0x00000020,
		SSA_GLYPHS = 0x00000080,
		SSA_RTL = 0x00000100,
	};

	static HRESULT ScriptIsComplex(const WCHAR *text, int text_length, DWORD flags);
	static HRESULT ScriptItemize(const WCHAR *text, int text_length, int item_capacity,
		const ScriptControl *control, const ScriptState *state, ScriptItem *items, int *item_count);
	static HRESULT ScriptBreak(const WCHAR *text, int text_length, const ScriptAnalysis *analysis,
		ScriptLogAttr *attributes);
	static HRESULT ScriptLayout(int run_count, const BYTE *levels, int *visual_to_logical,
		int *logical_to_visual);
	static HRESULT ScriptStringAnalyse(HDC dc, const void *text, int text_length, int glyph_count,
		int charset, DWORD flags, int required_width, ScriptControl *control, ScriptState *state,
		const int *spacing, ScriptTabDefinition *tabs, const BYTE *character_classes,
		ScriptStringAnalysis *analysis);
	static HRESULT ScriptStringFree(ScriptStringAnalysis *analysis);
	static const SIZE *ScriptString_pSize(ScriptStringAnalysis analysis);
	static HRESULT ScriptStringOut(ScriptStringAnalysis analysis, int x, int y, UINT options,
		const RECT *rect, int minimum_selection, int maximum_selection, BOOL disabled);
	static void unload();

private:

	static bool load();
	static void freeResources();

	typedef HRESULT (WINAPI *ScriptIsComplex_t)(const WCHAR *, int, DWORD);
	typedef HRESULT (WINAPI *ScriptItemize_t)(const WCHAR *, int, int, const ScriptControl *,
		const ScriptState *, ScriptItem *, int *);
	typedef HRESULT (WINAPI *ScriptBreak_t)(const WCHAR *, int, const ScriptAnalysis *, ScriptLogAttr *);
	typedef HRESULT (WINAPI *ScriptLayout_t)(int, const BYTE *, int *, int *);
	typedef HRESULT (WINAPI *ScriptStringAnalyse_t)(HDC, const void *, int, int, int, DWORD, int,
		ScriptControl *, ScriptState *, const int *, ScriptTabDefinition *, const BYTE *, ScriptStringAnalysis *);
	typedef HRESULT (WINAPI *ScriptStringFree_t)(ScriptStringAnalysis *);
	typedef const SIZE *(WINAPI *ScriptString_pSize_t)(ScriptStringAnalysis);
	typedef HRESULT (WINAPI *ScriptStringOut_t)(ScriptStringAnalysis, int, int, UINT, const RECT *, int, int, BOOL);

	static CriticalSectionClass CriticalSection;
	static HMODULE Module;
	static bool LoadAttempted;
	static ScriptIsComplex_t ScriptIsComplexPtr;
	static ScriptItemize_t ScriptItemizePtr;
	static ScriptBreak_t ScriptBreakPtr;
	static ScriptLayout_t ScriptLayoutPtr;
	static ScriptStringAnalyse_t ScriptStringAnalysePtr;
	static ScriptStringFree_t ScriptStringFreePtr;
	static ScriptString_pSize_t ScriptString_pSizePtr;
	static ScriptStringOut_t ScriptStringOutPtr;
};
