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

// Subset of the Uniscribe API of the Windows SDK usp10.h, for compilers that do not have it, such as VC6.
// Is included by usp10_adapter.h, which should be used instead of this file.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SSA_FALLBACK 0x00000020
#define SSA_GLYPHS 0x00000080
#define SSA_RTL 0x00000100

#define SIC_COMPLEX 1

typedef struct tag_SCRIPT_CONTROL {
	DWORD uDefaultLanguage :16;
	DWORD fContextDigits :1;
	DWORD fInvertPreBoundDir :1;
	DWORD fInvertPostBoundDir :1;
	DWORD fLinkStringBefore :1;
	DWORD fLinkStringAfter :1;
	DWORD fNeutralOverride :1;
	DWORD fNumericOverride :1;
	DWORD fLegacyBidiClass :1;
	DWORD fMergeNeutralItems :1;
	DWORD fUseStandardBidi :1;
	DWORD fReserved :6;
} SCRIPT_CONTROL;

typedef struct tag_SCRIPT_STATE {
	WORD uBidiLevel :5;
	WORD fOverrideDirection :1;
	WORD fInhibitSymSwap :1;
	WORD fCharShape :1;
	WORD fDigitSubstitute :1;
	WORD fInhibitLigate :1;
	WORD fDisplayZWG :1;
	WORD fArabicNumContext :1;
	WORD fGcpClusters :1;
	WORD fReserved :1;
	WORD fEngineReserved :2;
} SCRIPT_STATE;

typedef struct tag_SCRIPT_ANALYSIS {
	WORD eScript :10;
	WORD fRTL :1;
	WORD fLayoutRTL :1;
	WORD fLinkBefore :1;
	WORD fLinkAfter :1;
	WORD fLogicalOrder :1;
	WORD fNoGlyphIndex :1;
	SCRIPT_STATE s;
} SCRIPT_ANALYSIS;

typedef struct tag_SCRIPT_ITEM {
	int iCharPos;
	SCRIPT_ANALYSIS a;
} SCRIPT_ITEM;

typedef struct tag_SCRIPT_LOGATTR {
	BYTE fSoftBreak :1;
	BYTE fWhiteSpace :1;
	BYTE fCharStop :1;
	BYTE fWordStop :1;
	BYTE fInvalid :1;
	BYTE fReserved :3;
} SCRIPT_LOGATTR;

typedef struct tag_SCRIPT_TABDEF {
	int cTabStops;
	int iScale;
	int *pTabStops;
	int iTabOrigin;
} SCRIPT_TABDEF;

typedef void *SCRIPT_STRING_ANALYSIS;

HRESULT WINAPI ScriptIsComplex(const WCHAR *pwcInChars, int cInChars, DWORD dwFlags);

HRESULT WINAPI ScriptItemize(const WCHAR *pwcInChars, int cInChars, int cMaxItems,
	const SCRIPT_CONTROL *psControl, const SCRIPT_STATE *psState, SCRIPT_ITEM *pItems, int *pcItems);

HRESULT WINAPI ScriptBreak(const WCHAR *pwcChars, int cChars, const SCRIPT_ANALYSIS *psa, SCRIPT_LOGATTR *psla);

HRESULT WINAPI ScriptLayout(int cRuns, const BYTE *pbLevel, int *piVisualToLogical, int *piLogicalToVisual);

HRESULT WINAPI ScriptStringAnalyse(HDC hdc, const void *pString, int cString, int cGlyphs, int iCharset,
	DWORD dwFlags, int iReqWidth, SCRIPT_CONTROL *psControl, SCRIPT_STATE *psState, const int *piDx,
	SCRIPT_TABDEF *pTabdef, const BYTE *pbInClass, SCRIPT_STRING_ANALYSIS *pssa);

HRESULT WINAPI ScriptStringFree(SCRIPT_STRING_ANALYSIS *pssa);

const SIZE *WINAPI ScriptString_pSize(SCRIPT_STRING_ANALYSIS ssa);

HRESULT WINAPI ScriptStringOut(SCRIPT_STRING_ANALYSIS ssa, int iX, int iY, UINT uOptions, const RECT *prc,
	int iMinSel, int iMaxSel, BOOL fDisabled);

#ifdef __cplusplus
} // extern "C"
#endif
