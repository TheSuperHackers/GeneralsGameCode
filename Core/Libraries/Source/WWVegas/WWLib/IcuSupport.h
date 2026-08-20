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

// Engine entry point for ICU4C.
//
// RTS_HAS_ICU        - ICU C API is linked; include this header and call ICU functions.
// RTS_HAS_ICU_CXX    - ICU C++ API (icu::UnicodeString, icu::Locale, ...).
// RTS_HAS_ICU_I18N   - Collation, break iteration, converters, and related i18n APIs.
// RTS_HAS_ICU_WINSDK - Windows SDK merged C API via <icu.h> (no C++ API).
// RTS_ICU_DYNAMIC    - No import library; utf8.cpp LoadLibrary's OS icu.dll (VC6).

#if defined(RTS_HAS_ICU_WINSDK)

#include <icu.h>

#elif defined(RTS_HAS_ICU)

#include <unicode/uchar.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/utf8.h>
#include <unicode/utypes.h>

#if defined(RTS_HAS_ICU_I18N)
#include <unicode/ubrk.h>
#include <unicode/ucol.h>
#include <unicode/uidna.h>
#include <unicode/uloc.h>
#include <unicode/unorm2.h>
#endif

#if defined(RTS_HAS_ICU_CXX)
#include <unicode/locid.h>
#include <unicode/normalizer2.h>
#include <unicode/unistr.h>
#if defined(RTS_HAS_ICU_I18N)
#include <unicode/coll.h>
#endif
#endif

#endif
