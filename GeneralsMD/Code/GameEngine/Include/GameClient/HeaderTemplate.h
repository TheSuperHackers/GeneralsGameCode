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

// FILE: HeaderTemplate.h /////////////////////////////////////////////////
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
//	Filename: 	HeaderTemplate.h
//
//	author:		Chris Huybregts
//
//	purpose:
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/STLTypedefs.h"
//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/AsciiString.h"
//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class GameFont;
struct FieldParse;
class INI;
//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class HeaderTemplate
{
public:
	HeaderTemplate( void );
	~HeaderTemplate( void );

	GameFont *m_font;
	AsciiString m_name;
	AsciiString m_fontName;
	Int m_point;
	Bool m_bold;

};

class HeaderTemplateManager
{
public:
	HeaderTemplateManager( void );
	~HeaderTemplateManager( void );

	void init( void );

	const FieldParse *getFieldParse( void ) const { return m_headerFieldParseTable; }		///< Return the field parse info
	static const FieldParse m_headerFieldParseTable[];

	HeaderTemplate *findHeaderTemplate( AsciiString name );
	HeaderTemplate *newHeaderTemplate( AsciiString name );

	GameFont *getFontFromTemplate( AsciiString name );
	HeaderTemplate *getFirstHeader( void );
	HeaderTemplate *getNextHeader( HeaderTemplate *ht );

	void onResolutionChanged(void);

private:
	void populateGameFonts( void );
	typedef std::list< HeaderTemplate* > HeaderTemplateList;
	typedef HeaderTemplateList::iterator HeaderTemplateListIt;
	HeaderTemplateList m_headerTemplateList;

};


//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
extern HeaderTemplateManager *TheHeaderTemplateManager;
