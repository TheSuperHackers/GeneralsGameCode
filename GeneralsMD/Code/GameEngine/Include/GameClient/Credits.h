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

// FILE: Credits.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Dec 2002
//
//	Filename: 	Credits.h
//
//	author:		Chris Huybregts
//
//	purpose:	header file for the credits
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "GameClient/FontDesc.h"

//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class DisplayString;
//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
enum
{
CREDIT_STYLE_TITLE = 0,
CREDIT_STYLE_POSITION,
CREDIT_STYLE_NORMAL,
CREDIT_STYLE_COLUMN,
CREDIT_STYLE_BLANK,			///< Keep this second to last

MAX_CREDIT_STYLES
};

enum{ CREDIT_SPACE_OFFSET = 2 };

static const LookupListRec CreditStyleNames[] =
{
	{ "TITLE",					CREDIT_STYLE_TITLE },
	{ "MINORTITLE",				CREDIT_STYLE_POSITION },
	{ "NORMAL",			CREDIT_STYLE_NORMAL },
	{ "COLUMN",				CREDIT_STYLE_COLUMN },
	// CREDIT_STYLE_BLANK
	{ nullptr, 0	}
};
static_assert(ARRAY_SIZE(CreditStyleNames) == MAX_CREDIT_STYLES, "Incorrect array size");


class CreditsLine
{
public:
	CreditsLine();
	~CreditsLine();

// parsing variables
	Int m_style;
	UnicodeString m_text;
	UnicodeString m_secondText;
	Bool m_useSecond;
	Bool m_done;

// drawing variables
	DisplayString *m_displayString;
	DisplayString *m_secondDisplayString;
	ICoord2D m_pos;
	Int m_height;
	Int m_color;
};

class CreditsManager: public SubsystemInterface
{
public:
	CreditsManager(void);
	~CreditsManager(void);

	void init(void );
	void load(void );
	void reset( void );
	void update( void );
	void draw( void );

	const FieldParse *getFieldParse() const { return m_creditsFieldParseTable; }								///< returns the parsing fields
	static const FieldParse m_creditsFieldParseTable[];																				///< the parse table
	static void parseBlank( INI* ini, void *instance, void *store, const void *userData );					///< Parse the image part of the INI file
	static void parseText( INI* ini, void *instance, void *store, const void *userData );					///< Parse the image part of the INI file

	Bool isFinished( void ) { return m_isFinished;	}
	void addBlank( void );
	void addText( AsciiString text );
private:

	UnicodeString getUnicodeString(AsciiString str);

	typedef std::list<CreditsLine *> CreditsLineList;
	CreditsLineList m_creditLineList;
	CreditsLineList::iterator m_creditLineListIt;

	CreditsLineList m_displayedCreditLineList;

	Int m_scrollRate; // in pixels
	Int m_scrollRatePerFrames;
	Bool m_scrollDown;	// if TRUE text will come from the top to the bottom if False, it will go from the bottom up

	Color			m_titleColor;
	Color			m_positionColor;
	Color			m_normalColor;

	Int m_currentStyle;

	Bool m_isFinished;

	Int m_framesSinceStarted;
	Int m_normalFontHeight;
};


//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
extern CreditsManager *TheCredits;
