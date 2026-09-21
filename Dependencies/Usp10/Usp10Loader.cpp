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
#include "usp10_adapter.h"

#include "Utility/stringex.h"


namespace
{

typedef HRESULT (WINAPI *ScriptIsComplex_t)(const WCHAR *pwcInChars, int cInChars, DWORD dwFlags);
typedef HRESULT (WINAPI *ScriptItemize_t)(const WCHAR *pwcInChars, int cInChars, int cMaxItems,
	const SCRIPT_CONTROL *psControl, const SCRIPT_STATE *psState, SCRIPT_ITEM *pItems, int *pcItems);
typedef HRESULT (WINAPI *ScriptBreak_t)(const WCHAR *pwcChars, int cChars, const SCRIPT_ANALYSIS *psa,
	SCRIPT_LOGATTR *psla);
typedef HRESULT (WINAPI *ScriptLayout_t)(int cRuns, const BYTE *pbLevel, int *piVisualToLogical,
	int *piLogicalToVisual);
typedef HRESULT (WINAPI *ScriptStringAnalyse_t)(HDC hdc, const void *pString, int cString, int cGlyphs,
	int iCharset, DWORD dwFlags, int iReqWidth, SCRIPT_CONTROL *psControl, SCRIPT_STATE *psState,
	const int *piDx, SCRIPT_TABDEF *pTabdef, const BYTE *pbInClass, SCRIPT_STRING_ANALYSIS *pssa);
typedef HRESULT (WINAPI *ScriptStringFree_t)(SCRIPT_STRING_ANALYSIS *pssa);
typedef const SIZE *(WINAPI *ScriptString_pSize_t)(SCRIPT_STRING_ANALYSIS ssa);
typedef HRESULT (WINAPI *ScriptStringOut_t)(SCRIPT_STRING_ANALYSIS ssa, int iX, int iY, UINT uOptions,
	const RECT *prc, int iMinSel, int iMaxSel, BOOL fDisabled);

ScriptIsComplex_t ScriptIsComplexPtr = nullptr;
ScriptItemize_t ScriptItemizePtr = nullptr;
ScriptBreak_t ScriptBreakPtr = nullptr;
ScriptLayout_t ScriptLayoutPtr = nullptr;
ScriptStringAnalyse_t ScriptStringAnalysePtr = nullptr;
ScriptStringFree_t ScriptStringFreePtr = nullptr;
ScriptString_pSize_t ScriptString_pSizePtr = nullptr;
ScriptStringOut_t ScriptStringOutPtr = nullptr;

HMODULE Module = HMODULE(nullptr);
int ReferenceCount = 0;
bool Failed = false;
unsigned long LastError = 0;

#define USP10_RESOLVE(name) \
	name##Ptr = reinterpret_cast<name##_t>(::GetProcAddress(Module, #name))

static bool resolveAll()
{
	USP10_RESOLVE(ScriptIsComplex);
	USP10_RESOLVE(ScriptItemize);
	USP10_RESOLVE(ScriptBreak);
	USP10_RESOLVE(ScriptLayout);
	USP10_RESOLVE(ScriptStringAnalyse);
	USP10_RESOLVE(ScriptStringFree);
	USP10_RESOLVE(ScriptString_pSize);
	USP10_RESOLVE(ScriptStringOut);

	return ScriptIsComplexPtr != nullptr && ScriptItemizePtr != nullptr && ScriptBreakPtr != nullptr
		&& ScriptLayoutPtr != nullptr && ScriptStringAnalysePtr != nullptr && ScriptStringFreePtr != nullptr
		&& ScriptString_pSizePtr != nullptr && ScriptStringOutPtr != nullptr;
}
#undef USP10_RESOLVE

static void freeResources()
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

} // namespace


bool Usp10Loader::isLoaded()
{
	return Module != HMODULE(nullptr);
}


bool Usp10Loader::isFailed()
{
	return Failed;
}


unsigned long Usp10Loader::getLastError()
{
	return LastError;
}


bool Usp10Loader::load()
{
	// Always increment the reference count.
	++ReferenceCount;

	// Optimization: return early if it failed before.
	if (Failed)
		return false;

	// Return early if someone else already loaded it.
	if (ReferenceCount > 1)
		return true;

	// Load usp10.dll from the system directory only.
	char dll_path[MAX_PATH];
	const UINT path_length = ::GetSystemDirectoryA(dll_path, sizeof(dll_path));
	if (path_length == 0 || path_length >= sizeof(dll_path) ||
		strlcat(dll_path, "\\usp10.dll", sizeof(dll_path)) >= sizeof(dll_path)) {
		LastError = ERROR_BUFFER_OVERFLOW;
		Failed = true;
		return false;
	}

	Module = ::LoadLibraryA(dll_path);
	if (Module == HMODULE(nullptr)) {
		LastError = ::GetLastError();
		Failed = true;
		return false;
	}

	if (!resolveAll()) {
		freeResources();
		LastError = ERROR_PROC_NOT_FOUND;
		Failed = true;
		return false;
	}

	return true;
}


void Usp10Loader::unload()
{
	if (ReferenceCount > 0)
		--ReferenceCount;

	if (ReferenceCount > 0)
		return;

	freeResources();
	Failed = false;
	LastError = 0;
}


// The Uniscribe functions below stand in for the imports of usp10.dll.

HRESULT WINAPI ScriptIsComplex(const WCHAR *pwcInChars, int cInChars, DWORD dwFlags)
{
	return ScriptIsComplexPtr != nullptr ? ScriptIsComplexPtr(pwcInChars, cInChars, dwFlags) : E_FAIL;
}

HRESULT WINAPI ScriptItemize(const WCHAR *pwcInChars, int cInChars, int cMaxItems,
	const SCRIPT_CONTROL *psControl, const SCRIPT_STATE *psState, SCRIPT_ITEM *pItems, int *pcItems)
{
	if (ScriptItemizePtr != nullptr)
		return ScriptItemizePtr(pwcInChars, cInChars, cMaxItems, psControl, psState, pItems, pcItems);

	if (pcItems != nullptr)
		*pcItems = 0;
	return E_FAIL;
}

HRESULT WINAPI ScriptBreak(const WCHAR *pwcChars, int cChars, const SCRIPT_ANALYSIS *psa, SCRIPT_LOGATTR *psla)
{
	return ScriptBreakPtr != nullptr ? ScriptBreakPtr(pwcChars, cChars, psa, psla) : E_FAIL;
}

HRESULT WINAPI ScriptLayout(int cRuns, const BYTE *pbLevel, int *piVisualToLogical, int *piLogicalToVisual)
{
	return ScriptLayoutPtr != nullptr
		? ScriptLayoutPtr(cRuns, pbLevel, piVisualToLogical, piLogicalToVisual)
		: E_FAIL;
}

HRESULT WINAPI ScriptStringAnalyse(HDC hdc, const void *pString, int cString, int cGlyphs, int iCharset,
	DWORD dwFlags, int iReqWidth, SCRIPT_CONTROL *psControl, SCRIPT_STATE *psState, const int *piDx,
	SCRIPT_TABDEF *pTabdef, const BYTE *pbInClass, SCRIPT_STRING_ANALYSIS *pssa)
{
	if (ScriptStringAnalysePtr != nullptr)
		return ScriptStringAnalysePtr(hdc, pString, cString, cGlyphs, iCharset, dwFlags, iReqWidth, psControl,
			psState, piDx, pTabdef, pbInClass, pssa);

	if (pssa != nullptr)
		*pssa = nullptr;
	return E_FAIL;
}

HRESULT WINAPI ScriptStringFree(SCRIPT_STRING_ANALYSIS *pssa)
{
	return ScriptStringFreePtr != nullptr ? ScriptStringFreePtr(pssa) : E_FAIL;
}

const SIZE *WINAPI ScriptString_pSize(SCRIPT_STRING_ANALYSIS ssa)
{
	return ScriptString_pSizePtr != nullptr ? ScriptString_pSizePtr(ssa) : nullptr;
}

HRESULT WINAPI ScriptStringOut(SCRIPT_STRING_ANALYSIS ssa, int iX, int iY, UINT uOptions, const RECT *prc,
	int iMinSel, int iMaxSel, BOOL fDisabled)
{
	return ScriptStringOutPtr != nullptr
		? ScriptStringOutPtr(ssa, iX, iY, uOptions, prc, iMinSel, iMaxSel, fDisabled)
		: E_FAIL;
}
