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

// FILE: HeaderTemplate.cpp /////////////////////////////////////////////////
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
//	Filename: 	HeaderTemplate.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	The header template system is used to maintain a unified look across
//						windows.  It also allows Localization to customize the looks based
//						on language fonts.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"

#include "Common/INI.h"
#include "Common/FileSystem.h"
#include "Common/Registry.h"
#include "GameClient/HeaderTemplate.h"
#include "GameClient/GameFont.h"
#include "GameClient/GlobalLanguage.h"
//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
const FieldParse HeaderTemplateManager::m_headerFieldParseTable[] =
{
	{ "Font",								INI::parseQuotedAsciiString,						nullptr, offsetof( HeaderTemplate, m_fontName ) },
	{ "Point",							INI::parseInt,										nullptr, offsetof( HeaderTemplate, m_point) },
	{ "Bold",								INI::parseBool,										nullptr, offsetof( HeaderTemplate, m_bold ) },
	{ nullptr, nullptr, nullptr, 0 },
};

HeaderTemplateManager *TheHeaderTemplateManager = nullptr;
//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
void INI::parseHeaderTemplateDefinition( INI *ini )
{
	AsciiString name;
	HeaderTemplate *hTemplate;

	// read the name
	const char* c = ini->getNextToken();
	name.set( c );

	// find existing item if present
	hTemplate = TheHeaderTemplateManager->findHeaderTemplate( name );
	if( hTemplate == nullptr )
	{

		// allocate a new item
		hTemplate = TheHeaderTemplateManager->newHeaderTemplate( name );

	}
	else
	{
		DEBUG_CRASH(( "[LINE: %d in '%s'] Duplicate header Template %s found!", ini->getLineNum(), ini->getFilename().str(), name.str() ));
	}
	// parse the ini definition
	ini->initFromINI( hTemplate, TheHeaderTemplateManager->getFieldParse() );

}

HeaderTemplate::HeaderTemplate( void ) :
m_font(nullptr),
m_point(0),
m_bold(FALSE)
{
	m_fontName.clear();
	m_name.clear();
}

HeaderTemplate::~HeaderTemplate( void ){}

HeaderTemplateManager::HeaderTemplateManager( void )
{}

HeaderTemplateManager::~HeaderTemplateManager( void )
{
	HeaderTemplateListIt it = m_headerTemplateList.begin();
	while(it != m_headerTemplateList.end())
	{
		HeaderTemplate *hTemplate = *it;
		delete hTemplate;
		it = m_headerTemplateList.erase(it);

	}
}

void HeaderTemplateManager::init( void )
{
	{
		AsciiString fname;
		fname.format("Data\\%s\\HeaderTemplate", GetRegistryLanguage().str());

		INI ini;
		ini.loadFileDirectory( fname, INI_LOAD_OVERWRITE, nullptr );
	}

	populateGameFonts();
}

HeaderTemplate *HeaderTemplateManager::findHeaderTemplate( AsciiString name )
{
	HeaderTemplateListIt it = m_headerTemplateList.begin();
	while(it != m_headerTemplateList.end())
	{
		HeaderTemplate *hTemplate = *it;
		if(hTemplate->m_name.compare(name) == 0)
			return hTemplate;
		++it;
	}
	return nullptr;
}

HeaderTemplate *HeaderTemplateManager::newHeaderTemplate( AsciiString name )
{
	HeaderTemplate *newHTemplate = NEW HeaderTemplate;
	DEBUG_ASSERTCRASH(newHTemplate, ("Unable to create a new Header Template in HeaderTemplateManager::newHeaderTemplate"));
	if(!newHTemplate)
		return nullptr;

	newHTemplate->m_name = name;
	m_headerTemplateList.push_front(newHTemplate);
	return newHTemplate;

}

GameFont *HeaderTemplateManager::getFontFromTemplate( AsciiString name )
{
	HeaderTemplate *ht = findHeaderTemplate( name );
	if(!ht)
	{
		//DEBUG_LOG(("HeaderTemplateManager::getFontFromTemplate - Could not find header %s", name.str()));
		return nullptr;
	}

	return ht->m_font;
}

HeaderTemplate *HeaderTemplateManager::getFirstHeader( void )
{
	HeaderTemplateListIt it = m_headerTemplateList.begin();
	if( it == m_headerTemplateList.end())
		return nullptr;

	return *it;
}

HeaderTemplate *HeaderTemplateManager::getNextHeader( HeaderTemplate *ht )
{
	HeaderTemplateListIt it = m_headerTemplateList.begin();
	while(it != m_headerTemplateList.end())
	{
		if(*it == ht)
		{
			++it;
			if( it == m_headerTemplateList.end())
				return nullptr;
			return *it;
		}
		++it;
	}
	return nullptr;

}

void HeaderTemplateManager::onResolutionChanged( void )
{
	populateGameFonts();
}
//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

void HeaderTemplateManager::populateGameFonts( void )
{
	HeaderTemplateListIt it = m_headerTemplateList.begin();
	while(it != m_headerTemplateList.end())
	{
		HeaderTemplate *hTemplate = *it;
		Real pointSize = TheGlobalLanguageData->adjustFontSize(hTemplate->m_point);
		GameFont *font = TheFontLibrary->getFont(hTemplate->m_fontName, pointSize,hTemplate->m_bold);
		DEBUG_ASSERTCRASH(font,("HeaderTemplateManager::populateGameFonts - Could not find font %s %d",hTemplate->m_fontName.str(), hTemplate->m_point));

		hTemplate->m_font = font;

		++it;
	}
}
