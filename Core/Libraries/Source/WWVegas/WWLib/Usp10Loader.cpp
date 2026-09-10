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

#include "Usp10Loader.h"


CriticalSectionClass Usp10Loader::CriticalSection;
HMODULE Usp10Loader::Module = HMODULE(nullptr);
bool Usp10Loader::LoadAttempted = false;
Usp10Loader::ScriptIsComplex_t Usp10Loader::ScriptIsComplexPtr = nullptr;
Usp10Loader::ScriptItemize_t Usp10Loader::ScriptItemizePtr = nullptr;
Usp10Loader::ScriptBreak_t Usp10Loader::ScriptBreakPtr = nullptr;
Usp10Loader::ScriptLayout_t Usp10Loader::ScriptLayoutPtr = nullptr;
Usp10Loader::ScriptStringAnalyse_t Usp10Loader::ScriptStringAnalysePtr = nullptr;
Usp10Loader::ScriptStringFree_t Usp10Loader::ScriptStringFreePtr = nullptr;
Usp10Loader::ScriptString_pSize_t Usp10Loader::ScriptString_pSizePtr = nullptr;
Usp10Loader::ScriptStringOut_t Usp10Loader::ScriptStringOutPtr = nullptr;


bool Usp10Loader::load()
{
	if (LoadAttempted) {
		return Module != HMODULE(nullptr);
	}
	LoadAttempted = true;

	char dll_path[MAX_PATH];
	const char dll_name[] = "\\usp10.dll";
	const UINT path_length = ::GetSystemDirectoryA(dll_path, ARRAY_SIZE(dll_path));
	if (path_length == 0 || path_length + ARRAY_SIZE(dll_name) > ARRAY_SIZE(dll_path)) {
		return false;
	}
	strcpy(dll_path + path_length, dll_name);

	Module = ::LoadLibraryA(dll_path);
	if (Module == HMODULE(nullptr)) {
		return false;
	}

	ScriptIsComplexPtr = reinterpret_cast<ScriptIsComplex_t>(::GetProcAddress(Module, "ScriptIsComplex"));
	ScriptItemizePtr = reinterpret_cast<ScriptItemize_t>(::GetProcAddress(Module, "ScriptItemize"));
	ScriptBreakPtr = reinterpret_cast<ScriptBreak_t>(::GetProcAddress(Module, "ScriptBreak"));
	ScriptLayoutPtr = reinterpret_cast<ScriptLayout_t>(::GetProcAddress(Module, "ScriptLayout"));
	ScriptStringAnalysePtr = reinterpret_cast<ScriptStringAnalyse_t>(::GetProcAddress(Module, "ScriptStringAnalyse"));
	ScriptStringFreePtr = reinterpret_cast<ScriptStringFree_t>(::GetProcAddress(Module, "ScriptStringFree"));
	ScriptString_pSizePtr = reinterpret_cast<ScriptString_pSize_t>(::GetProcAddress(Module, "ScriptString_pSize"));
	ScriptStringOutPtr = reinterpret_cast<ScriptStringOut_t>(::GetProcAddress(Module, "ScriptStringOut"));

	if (ScriptIsComplexPtr == nullptr || ScriptItemizePtr == nullptr || ScriptBreakPtr == nullptr ||
		ScriptLayoutPtr == nullptr || ScriptStringAnalysePtr == nullptr || ScriptStringFreePtr == nullptr ||
		ScriptString_pSizePtr == nullptr || ScriptStringOutPtr == nullptr)
	{
		freeResources();
		return false;
	}

	return true;
}


void Usp10Loader::unload()
{
	CriticalSectionClass::LockClass lock(CriticalSection);

	freeResources();
	LoadAttempted = false;
}


void Usp10Loader::freeResources()
{
	if (Module != HMODULE(nullptr)) {
		::FreeLibrary(Module);
		Module = HMODULE(nullptr);
	}

	ScriptIsComplexPtr = nullptr;
	ScriptItemizePtr = nullptr;
	ScriptBreakPtr = nullptr;
	ScriptLayoutPtr = nullptr;
	ScriptStringAnalysePtr = nullptr;
	ScriptStringFreePtr = nullptr;
	ScriptString_pSizePtr = nullptr;
	ScriptStringOutPtr = nullptr;
}


HRESULT Usp10Loader::ScriptIsComplex(const WCHAR *text, int text_length, DWORD flags)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	return load() ? ScriptIsComplexPtr(text, text_length, flags) : E_FAIL;
}


HRESULT Usp10Loader::ScriptItemize(const WCHAR *text, int text_length, int item_capacity,
	const ScriptControl *control, const ScriptState *state, ScriptItem *items, int *item_count)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	return load() ? ScriptItemizePtr(text, text_length, item_capacity, control, state, items, item_count) : E_FAIL;
}


HRESULT Usp10Loader::ScriptBreak(const WCHAR *text, int text_length, const ScriptAnalysis *analysis,
	ScriptLogAttr *attributes)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	return load() ? ScriptBreakPtr(text, text_length, analysis, attributes) : E_FAIL;
}


HRESULT Usp10Loader::ScriptLayout(int run_count, const BYTE *levels, int *visual_to_logical,
	int *logical_to_visual)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	return load() ? ScriptLayoutPtr(run_count, levels, visual_to_logical, logical_to_visual) : E_FAIL;
}


HRESULT Usp10Loader::ScriptStringAnalyse(HDC dc, const void *text, int text_length, int glyph_count,
	int charset, DWORD flags, int required_width, ScriptControl *control, ScriptState *state,
	const int *spacing, ScriptTabDefinition *tabs, const BYTE *character_classes, ScriptStringAnalysis *analysis)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	return load() ? ScriptStringAnalysePtr(dc, text, text_length, glyph_count, charset, flags, required_width,
		control, state, spacing, tabs, character_classes, analysis) : E_FAIL;
}


HRESULT Usp10Loader::ScriptStringFree(ScriptStringAnalysis *analysis)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	return load() ? ScriptStringFreePtr(analysis) : E_FAIL;
}


const SIZE *Usp10Loader::ScriptString_pSize(ScriptStringAnalysis analysis)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	return load() ? ScriptString_pSizePtr(analysis) : nullptr;
}


HRESULT Usp10Loader::ScriptStringOut(ScriptStringAnalysis analysis, int x, int y, UINT options,
	const RECT *rect, int minimum_selection, int maximum_selection, BOOL disabled)
{
	CriticalSectionClass::LockClass lock(CriticalSection);
	return load() ? ScriptStringOutPtr(analysis, x, y, options, rect, minimum_selection, maximum_selection, disabled) : E_FAIL;
}
