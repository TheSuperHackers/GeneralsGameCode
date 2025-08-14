/*
**	Command & Conquer Generals(tm)
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

// FILE: GlobalLanguage.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Aug 2002
//
//	Filename: 	GlobalLanguage.h
//
//	author:		Chris Huybregts
//
//	purpose:	With workingwith different languages, we need some options that
//						change.  Essentially, this is the global data that's unique to languages
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __GLOBAL_LANGUAGE_H_
#define __GLOBAL_LANGUAGE_H_

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/SubsystemInterface.h"
#include "Common/STLTypedefs.h"
#include "GameClient/FontDesc.h"
//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class AsciiString;
//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class GlobalLanguage : public SubsystemInterface
{
public:

	GlobalLanguage();
	virtual ~GlobalLanguage();

	void init();
	void reset();
	void update() { }

	AsciiString m_unicodeFontName;
	AsciiString m_unicodeFontFileName;
	Bool m_useHardWrap;
	Int m_militaryCaptionSpeed;
	FontDesc	m_copyrightFont;
	FontDesc	m_messageFont;
	FontDesc	m_militaryCaptionTitleFont;
	FontDesc	m_militaryCaptionFont;
	FontDesc	m_superweaponCountdownNormalFont;
	FontDesc	m_superweaponCountdownReadyFont;
	FontDesc	m_namedTimerCountdownNormalFont;
	FontDesc	m_namedTimerCountdownReadyFont;
	FontDesc	m_drawableCaptionFont;
	FontDesc	m_defaultWindowFont;
	FontDesc	m_defaultDisplayStringFont;
	FontDesc	m_tooltipFontName;
	FontDesc	m_nativeDebugDisplay;
	FontDesc	m_drawGroupInfoFont;
	FontDesc	m_creditsTitleFont;
	FontDesc  m_creditsPositionFont;
	FontDesc  m_creditsNormalFont;

	Real			m_resolutionFontSizeAdjustment;

	//UnicodeString	m_unicodeFontNameUStr;

	Int adjustFontSize(Int theFontSize);	// Adjusts font size for resolution. jba.

	typedef std::list<AsciiString> StringList;					// Used for our font file names that we want to load
	typedef StringList::iterator StringListIt;

	StringList m_localFonts;														// List of the font filenames that are in our local directory
	static void parseFontFileName( INI *ini, void *instance, void *store, const void* userData );
	static void parseFontDesc(INI *ini, void *instance, void *store, const void* userData);
};
//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
extern GlobalLanguage *TheGlobalLanguageData;
#endif // __GLOBAL_LANGUAGE_H_
