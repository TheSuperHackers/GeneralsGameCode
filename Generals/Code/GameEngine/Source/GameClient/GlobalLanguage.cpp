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

// FILE: GlobalLanguage.cpp /////////////////////////////////////////////////
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
//	Filename: 	GlobalLanguage.cpp
//
//	author:		Chris Huybregts
//	
//	purpose:	Contains the member functions for the language munkee
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
#include "Common/Registry.h"
#include "GameClient/GlobalLanguage.h"
#include "Common/FileSystem.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
GlobalLanguage *TheGlobalLanguageData = NULL;				///< The global language singalton

static const FieldParse TheGlobalLanguageDataFieldParseTable[] = 
{
	{ "UnicodeFontName",									INI::parseAsciiString,NULL,									offsetof( GlobalLanguage, m_unicodeFontName ) },
	//{	"UnicodeFontFileName",							INI::parseAsciiString,NULL,									offsetof( GlobalLanguage, m_unicodeFontFileName ) },
	{ "LocalFontFile",										GlobalLanguage::parseFontFileName,					NULL,			0},
	{ "MilitaryCaptionSpeed",						INI::parseInt,					NULL,		offsetof( GlobalLanguage, m_militaryCaptionSpeed ) },
	{ "UseHardWordWrap",						INI::parseBool,					NULL,		offsetof( GlobalLanguage, m_useHardWrap) },
	{ "ResolutionFontAdjustment",						INI::parseReal,					NULL,		offsetof( GlobalLanguage, m_resolutionFontSizeAdjustment) },
	
	{ "CopyrightFont",					GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_copyrightFont ) },
	{ "MessageFont",					GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_messageFont) },					
	{ "MilitaryCaptionTitleFont",		GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_militaryCaptionTitleFont) },
	{ "MilitaryCaptionFont",			GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_militaryCaptionFont) },
	{ "SuperweaponCountdownNormalFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_superweaponCountdownNormalFont) },
	{ "SuperweaponCountdownReadyFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_superweaponCountdownReadyFont) },
	{ "NamedTimerCountdownNormalFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_namedTimerCountdownNormalFont) },
	{ "NamedTimerCountdownReadyFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_namedTimerCountdownReadyFont) },
	{ "DrawableCaptionFont",			GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_drawableCaptionFont) },
	{ "DefaultWindowFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_defaultWindowFont) },
	{ "DefaultDisplayStringFont",		GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_defaultDisplayStringFont) },
	{ "TooltipFontName",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_tooltipFontName) },
	{ "NativeDebugDisplay",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_nativeDebugDisplay) },
	{ "DrawGroupInfoFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_drawGroupInfoFont) },
	{ "CreditsTitleFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_creditsTitleFont) },
	{ "CreditsMinorTitleFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_creditsPositionFont) },
	{ "CreditsNormalFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_creditsNormalFont) },

	{ NULL,					NULL,						NULL,						0 }  // keep this last
};

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
void INI::parseLanguageDefinition( INI *ini )
{
	if( !TheGlobalLanguageData )
	{
		DEBUG_ASSERTCRASH(TheGlobalLanguageData, ("INI::parseLanguageDefinition - TheGlobalLanguage Data is not around, please create it before trying to parse the ini file."));
		return;
	}  // end if
	// parse the ini weapon definition
	ini->initFromINI( TheGlobalLanguageData, TheGlobalLanguageDataFieldParseTable );
}

GlobalLanguage::GlobalLanguage()
{
	m_unicodeFontName.clear();
	//Added By Sadullah Nader
	//Initializations missing and needed
	m_unicodeFontFileName.clear();
	m_unicodeFontName.clear();
	m_militaryCaptionSpeed = 0;
	m_useHardWrap = FALSE;
	m_resolutionFontSizeAdjustment = 0.7f;
	//End Add
}

GlobalLanguage::~GlobalLanguage()
{
	StringListIt it = m_localFonts.begin();
	while( it != m_localFonts.end())
	{
		AsciiString font = *it;
		RemoveFontResource(font.str());
		//SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
		++it;
	}
}

void GlobalLanguage::init( void ) 
{

	INI ini;
	AsciiString fname;
	fname.format("Data\\%s\\Language.ini", GetRegistryLanguage().str());

	OSVERSIONINFO	osvi;
	osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	//GS NOTE: Must call doesFileExist in either case so that NameKeyGenerator will stay in sync
	AsciiString tempName;
	tempName.format("Data\\%s\\Language9x.ini", GetRegistryLanguage().str());
	bool isExist = TheFileSystem->doesFileExist(tempName.str());
	if (GetVersionEx(&osvi)  &&  osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS  && isExist)
	{	//check if we're running Win9x variant since they may need different fonts
		fname = tempName;
	}

	ini.load( fname, INI_LOAD_OVERWRITE, NULL );
	StringListIt it = m_localFonts.begin();
	while( it != m_localFonts.end())
	{
		AsciiString font = *it;
		if(AddFontResource(font.str()) == 0)
		{
			DEBUG_ASSERTCRASH(FALSE,("GlobalLanguage::init Failed to add font %s", font.str()));
		}
		else
		{
			//SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
		}
		++it;
	}

	
}
void GlobalLanguage::reset( void ) {}


void GlobalLanguage::parseFontDesc(INI *ini, void *instance, void *store, const void* userData)
{
	FontDesc *fontDesc = (FontDesc *)store;
	fontDesc->name = ini->getNextQuotedAsciiString();
	fontDesc->size = ini->scanInt(ini->getNextToken());
	fontDesc->bold = ini->scanBool(ini->getNextToken());
}

void GlobalLanguage::parseFontFileName( INI *ini, void * instance, void *store, const void* userData )
{
	GlobalLanguage *monkey = (GlobalLanguage *)instance;
	AsciiString asciiString = ini->getNextAsciiString();
	monkey->m_localFonts.push_front(asciiString);
}	 

Int GlobalLanguage::adjustFontSize(Int theFontSize) 
{
	Real adjustFactor = TheGlobalData->m_xResolution/800.0f;
	adjustFactor = 1.0f + (adjustFactor-1.0f) * m_resolutionFontSizeAdjustment;
	if (adjustFactor<1.0f) adjustFactor = 1.0f;
	if (adjustFactor>2.0f) adjustFactor = 2.0f;
	Int pointSize = REAL_TO_INT_FLOOR(theFontSize*adjustFactor);
	return pointSize;
}

FontDesc::FontDesc(void)
{
	name = "Arial Unicode MS";	///<name of font
	size = 12;			///<point size
	bold = FALSE;			///<is bold?
}
//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

