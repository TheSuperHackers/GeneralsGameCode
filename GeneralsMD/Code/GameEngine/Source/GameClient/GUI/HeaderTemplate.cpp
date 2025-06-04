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
#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif
//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
const FieldParse HeaderTemplateManager::m_headerFieldParseTable[] =
{
	{ "Font",								INI::parseQuotedAsciiString,						NULL, offsetof( HeaderTemplate, m_fontName ) },
	{ "Point",							INI::parseInt,										NULL, offsetof( HeaderTemplate, m_point) },
	{ "Bold",								INI::parseBool,										NULL, offsetof( HeaderTemplate, m_bold ) },
	{ NULL, NULL, NULL, 0 },
};

HeaderTemplateManager *TheHeaderTemplateManager = NULL;
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
	if( hTemplate == NULL )
	{

		// allocate a new item
		hTemplate = TheHeaderTemplateManager->newHeaderTemplate( name );

	}  // end if
	else
	{
		DEBUG_CRASH(( "[LINE: %d in '%s'] Duplicate header Template %s found!", ini->getLineNum(), ini->getFilename().str(), name.str() ));
	}
	// parse the ini definition
	ini->initFromINI( hTemplate, TheHeaderTemplateManager->getFieldParse() );

}  // end parseCommandButtonDefinition

HeaderTemplate::HeaderTemplate( void ) :
m_font(NULL),
m_point(0),
m_bold(FALSE)
{
	//Added By Sadullah Nader
	//Initializations missing and needed 
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
		if(hTemplate)
		{
			hTemplate->m_font = NULL;
			delete hTemplate;
		}
		it = m_headerTemplateList.erase(it);

	}
}

void HeaderTemplateManager::init( void )
{
	INI ini;
	AsciiString fname;
	fname.format("Data\\%s\\HeaderTemplate.ini", GetRegistryLanguage().str());
	OSVERSIONINFO	osvi;
	osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi))
	{	//check if we're running Win9x variant since they may need different fonts
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{	AsciiString tempName;

			tempName.format("Data\\%s\\HeaderTemplate9x.ini", GetRegistryLanguage().str());
			if (TheFileSystem->doesFileExist(tempName.str()))
				fname = tempName;
		}
	}
	ini.load( fname, INI_LOAD_OVERWRITE, NULL );
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
	return NULL;
}

HeaderTemplate *HeaderTemplateManager::newHeaderTemplate( AsciiString name )
{
	HeaderTemplate *newHTemplate = NEW HeaderTemplate;
	DEBUG_ASSERTCRASH(newHTemplate, ("Unable to create a new Header Template in HeaderTemplateManager::newHeaderTemplate"));
	if(!newHTemplate)
		return NULL;
	
	newHTemplate->m_name = name;
	m_headerTemplateList.push_front(newHTemplate);
	return newHTemplate;

}

GameFont *HeaderTemplateManager::getFontFromTemplate( AsciiString name )
{
	HeaderTemplate *ht = findHeaderTemplate( name );
	if(!ht)
	{
		//DEBUG_LOG(("HeaderTemplateManager::getFontFromTemplate - Could not find header %s\n", name.str()));
		return NULL;
	}
	
	return ht->m_font;
}

HeaderTemplate *HeaderTemplateManager::getFirstHeader( void )
{
	HeaderTemplateListIt it = m_headerTemplateList.begin();
	if( it == m_headerTemplateList.end())
		return NULL;

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
				return NULL;
			return *it;
		}
		++it;
	}
	return NULL;

}

void HeaderTemplateManager::headerNotifyResolutionChange( void )
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