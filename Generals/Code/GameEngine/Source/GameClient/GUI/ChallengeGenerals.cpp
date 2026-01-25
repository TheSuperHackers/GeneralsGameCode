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

// FILE: ChallengeGenerals.h //////////////////////////////////////////////////////////////////////
// Author: Steve Copeland, 6/24/2003
// Desc:   This is a manager for data pertaining to the Generals' Challenge personas and related GUI.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/ChallengeGenerals.h"


ChallengeGenerals *TheChallengeGenerals = nullptr;

ChallengeGenerals *createChallengeGenerals( void )
{
	return MSGNEW("ChallengeGenerals") ChallengeGenerals;
}


ChallengeGenerals::ChallengeGenerals()
{
	//ctor
}


ChallengeGenerals::~ChallengeGenerals()
{
	//dtor
}


void ChallengeGenerals::init( void )
{
	INI ini;
	ini.loadFileDirectory( "Data\\INI\\ChallengeMode", INI_LOAD_OVERWRITE, nullptr );
}


void ChallengeGenerals::parseGeneralPersona(INI *ini, void *instance, void *store, const void *userData)
{
	static const FieldParse dataFieldParse[] =
	{
		{ "StartsEnabled",			INI::parseBool, nullptr, offsetof( GeneralPersona, m_bStartsEnabled ) },
		{ "BioNameString",			INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strBioName ) },
		{ "BioDOBString",			INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strBioDOB ) },
		{ "BioBirthplaceString",	INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strBioBirthplace ) },
		{ "BioStrategyString",		INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strBioStrategy ) },
		{ "BioRankString",			INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strBioRank ) },
		{ "BioBranchString",		INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strBioBranch ) },
		{ "BioClassNumberString",	INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strBioClassNumber ) },
		{ "BioPortraitSmall",		INI::parseMappedImage, nullptr, offsetof( GeneralPersona, m_imageBioPortraitSmall ) },
		{ "BioPortraitLarge",		INI::parseMappedImage, nullptr, offsetof( GeneralPersona, m_imageBioPortraitLarge ) },
		{ "Campaign",				INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strCampaign ) },
		{ "PlayerTemplate",			INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strPlayerTemplateName ) },
		{ "PortraitMovieLeftName",		INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strPortraitMovieLeftName ) },
		{ "PortraitMovieRightName",		INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strPortraitMovieRightName ) },
		{ "DefeatedImage",			INI::parseMappedImage, nullptr, offsetof( GeneralPersona, m_imageDefeated ) },
		{ "VictoriousImage",		INI::parseMappedImage, nullptr, offsetof( GeneralPersona, m_imageVictorious ) },
		{ "DefeatedString",			INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strDefeated ) },
		{ "VictoriousString",		INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strVictorious ) },
		{ "SelectionSound",			INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strSelectionSound ) },
		{ "TauntSound1",				INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strTauntSound1 ) },
		{ "TauntSound2",				INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strTauntSound2 ) },
		{ "TauntSound3",				INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strTauntSound3 ) },
		{ "WinSound",						INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strWinSound ) },
		{ "LossSound",					INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strLossSound ) },
		{ "PreviewSound",					INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strPreviewSound ) },
		{ "NameSound",					INI::parseAsciiString, nullptr, offsetof( GeneralPersona, m_strNameSound ) },

		{ nullptr, nullptr, nullptr, 0 }
	};
	ini->initFromINI(store, dataFieldParse);
}


const FieldParse ChallengeGenerals::s_fieldParseTable[] =
{
	{ "GeneralPersona0", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[0] ) },
	{ "GeneralPersona1", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[1] ) },
	{ "GeneralPersona2", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[2] ) },
	{ "GeneralPersona3", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[3] ) },
	{ "GeneralPersona4", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[4] ) },
	{ "GeneralPersona5", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[5] ) },
	{ "GeneralPersona6", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[6] ) },
	{ "GeneralPersona7", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[7] ) },
	{ "GeneralPersona8", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[8] ) },
	{ "GeneralPersona9", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[9] ) },
	{ "GeneralPersona10", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[10] ) },
	{ "GeneralPersona11", ChallengeGenerals::parseGeneralPersona, nullptr, offsetof( ChallengeGenerals, m_position[11] ) },
	{ nullptr, nullptr, nullptr, 0 }
};


//-------------------------------------------------------------------------------------------------
/** Parse Gen Challenge entries */
//-------------------------------------------------------------------------------------------------
void INI::parseChallengeModeDefinition( INI* ini )
{
	if( TheChallengeGenerals )
	{
		ini->initFromINI( TheChallengeGenerals, TheChallengeGenerals->getFieldParse() );
	}
}

const GeneralPersona* ChallengeGenerals::getPlayerGeneralByCampaignName( AsciiString name ) const
{
	for (Int i = 0; i < NUM_GENERALS; i++)
	{
		AsciiString campaignName = m_position[i].getCampaign();
		if (campaignName.compareNoCase( name.str() ) == 0)
			return &m_position[i];
	}
	DEBUG_CRASH(("Can't find General by Campaign Name"));
	return nullptr;
}

const GeneralPersona* ChallengeGenerals::getGeneralByGeneralName( AsciiString name ) const
{
	for (Int i = 0; i < NUM_GENERALS; i++)
	{
		AsciiString generalName = m_position[i].getBioName();
		if (generalName.compareNoCase( name.str() ) == 0)
			return &m_position[i];
	}
	return nullptr;
}

const GeneralPersona* ChallengeGenerals::getGeneralByTemplateName( AsciiString name ) const
{
	for (Int i = 0; i < NUM_GENERALS; i++)
	{
		AsciiString templateName = m_position[i].getPlayerTemplateName();
		if (templateName.compareNoCase( name.str() ) == 0)
			return &m_position[i];
	}
	return nullptr;
}
