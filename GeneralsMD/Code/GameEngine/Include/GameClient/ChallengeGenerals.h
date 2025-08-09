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

#pragma once

#ifndef __CHALLENGEGENERALS_H_
#define __CHALLENGEGENERALS_H_


// INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/GameType.h"
#include "Common/Overridable.h"

// DEFINES ////////////////////////////////////////////////////////////////////////////////////////
//static const Int NUM_GENERALS = 12;  // ChallengeMenu.wnd dependent
#define NUM_GENERALS (12)

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////


// CLASS DEFINITIONS //////////////////////////////////////////////////////////////////////////////
class GeneralPersona
{
	friend class ChallengeGenerals;

private:
	Bool m_bStartsEnabled;
	AsciiString m_strBioName;
	AsciiString m_strBioDOB;
	AsciiString m_strBioBirthplace;
	AsciiString m_strBioStrategy;
	AsciiString m_strBioRank;
	AsciiString m_strBioBranch;
	AsciiString m_strBioClassNumber;
	Image *m_imageBioPortraitSmall;
	Image *m_imageBioPortraitLarge;
	AsciiString m_strCampaign;
	AsciiString m_strPlayerTemplateName;
	AsciiString m_strPortraitMovieLeftName;
	AsciiString m_strPortraitMovieRightName;
	Image *m_imageDefeated;
	Image *m_imageVictorious;
	AsciiString m_strDefeated;
	AsciiString m_strVictorious;
	AsciiString m_strSelectionSound;
	AsciiString m_strTauntSound1;
	AsciiString m_strTauntSound2;
	AsciiString m_strTauntSound3;
	AsciiString m_strWinSound;
	AsciiString m_strLossSound;
	AsciiString m_strPreviewSound;
	AsciiString m_strNameSound ;


public:
	GeneralPersona( void ) :
		m_imageBioPortraitSmall(NULL),
		m_imageBioPortraitLarge(NULL)
	{
	}
//	~GeneralPersona( void );

	Bool isStartingEnabled() const { return m_bStartsEnabled; }
	const AsciiString& getBioName() const { return m_strBioName; }
	const AsciiString& getBioDOB() const { return m_strBioDOB; }
	const AsciiString& getBioBirthplace() const { return m_strBioBirthplace; }
	const AsciiString& getBioStrategy() const { return m_strBioStrategy; }
	const AsciiString& getBioRank() const { return m_strBioRank; }
	const AsciiString& getBioClassNumber() const { return m_strBioClassNumber; }
	const AsciiString& getBioBranch() const { return m_strBioBranch; }
	const Image *getBioPortraitSmall() const { return m_imageBioPortraitSmall; }
	const Image *getBioPortraitLarge() const { return m_imageBioPortraitLarge; }
	const AsciiString& getPortraitMovieLeftName() const { return m_strPortraitMovieLeftName; }
	const AsciiString& getPortraitMovieRightName() const { return m_strPortraitMovieRightName; }
	const AsciiString& getCampaign() const { return m_strCampaign; }
	const AsciiString& getPlayerTemplateName() const { return m_strPlayerTemplateName; }	// template name, as parsed in from ini
	const Image *getImageDefeated() const { return m_imageDefeated; }
	const Image *getImageVictorious() const { return m_imageVictorious; }
	const AsciiString& getStringDefeated() const { return m_strDefeated; }
	const AsciiString& getStringVictorious() const { return m_strVictorious; }
	const AsciiString& getSelectionSound() const { return m_strSelectionSound; }
	const AsciiString& getRandomTauntSound() const {
		switch (rand()%3) // don't care about distribution or exactly how random this is
		{
			case 0:	return m_strTauntSound1;
			case 1: return m_strTauntSound2;
		}
		return m_strTauntSound3;
	}
	const AsciiString& getWinSound() const { return m_strWinSound; }
	const AsciiString& getLossSound() const { return m_strLossSound; }
	const AsciiString& getPreviewSound() const { return m_strPreviewSound; }
	const AsciiString& getNameSound() const { return m_strNameSound; }
};


class ChallengeGenerals
{

private:
	/*const*/ GeneralPersona m_position[ NUM_GENERALS ];
	Int m_PlayerTemplateNum;		// the template number as ThePlayerTemplateStore has it
	GameDifficulty m_currentDifficulty; // the last selected game difficulty for the challenge generals

	static void parseGeneralPersona( INI* ini, void *instance, void *store, const void *userData );

public:
	ChallengeGenerals( void );
	~ChallengeGenerals( void );

	void init( void );
	const GeneralPersona* getChallengeGenerals() const { return m_position; }
	const FieldParse* getFieldParse( void ) const { return s_fieldParseTable; }	// for INI file parsing
	const GeneralPersona* getPlayerGeneralByCampaignName( AsciiString name ) const;
	const GeneralPersona* getGeneralByGeneralName( AsciiString name ) const;
	const GeneralPersona* getGeneralByTemplateName( AsciiString name ) const;

	void setCurrentPlayerTemplateNum( Int playerTemplateNum) { m_PlayerTemplateNum = playerTemplateNum; }
	Int getCurrentPlayerTemplateNum( void ) { return m_PlayerTemplateNum; }

	void setCurrentDifficulty( GameDifficulty diff ) { m_currentDifficulty = diff; }
	GameDifficulty getCurrentDifficulty( void ) { return m_currentDifficulty; }
protected:
	static const FieldParse s_fieldParseTable[];

};



// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern ChallengeGenerals *TheChallengeGenerals;
extern ChallengeGenerals *createChallengeGenerals( void );

#endif  // end __CHALLENGEGENERALS_H_
