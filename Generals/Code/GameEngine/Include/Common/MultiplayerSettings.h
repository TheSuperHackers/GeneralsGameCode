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

// FILE: MultiplayerSettings.h /////////////////////////////////////////////////////////////////////////////
// Settings common to multiplayer games
// Author: Matthew D. Campbell, January 2002
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameClient/Color.h"

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////////////////////////
struct FieldParse;
class MultiplayerSettings;

// PUBLIC /////////////////////////////////////////////////////////////////////////////////////////

class MultiplayerColorDefinition
{
public:
	MultiplayerColorDefinition();
	//-----------------------------------------------------------------------------------------------
	static const FieldParse m_colorFieldParseTable[];		///< the parse table for INI definition
	const FieldParse *getFieldParse( void ) const { return m_colorFieldParseTable; }

	inline AsciiString getTooltipName(void) const { return m_tooltipName; };
	inline RGBColor getRGBValue(void) const { return m_rgbValue; };
	inline RGBColor getRGBNightValue(void) const { return m_rgbValueNight; };
	inline Color getColor(void) const { return m_color; }
	inline Color getNightColor(void) const { return m_colorNight; }
	void setColor( RGBColor rgb );
	void setNightColor( RGBColor rgb );

	MultiplayerColorDefinition * operator =(const MultiplayerColorDefinition& other);

private:
	AsciiString m_tooltipName;	///< tooltip name for color combo box (AsciiString to pass to TheGameText->fetch())
	RGBColor m_rgbValue;						///< RGB color value
	Color m_color;
	RGBColor m_rgbValueNight;						///< RGB color value
	Color m_colorNight;
};

typedef std::map<Int, MultiplayerColorDefinition> MultiplayerColorList;
typedef std::map<Int, MultiplayerColorDefinition>::iterator MultiplayerColorIter;

//-------------------------------------------------------------------------------------------------
/** Multiplayer Settings container class
  *	Defines multiplayer settings */
//-------------------------------------------------------------------------------------------------
class MultiplayerSettings : public SubsystemInterface
{
public:

	MultiplayerSettings( void );

	virtual void init() { }
	virtual void update() { }
	virtual void reset() { }

	//-----------------------------------------------------------------------------------------------
	static const FieldParse m_multiplayerSettingsFieldParseTable[];		///< the parse table for INI definition
	const FieldParse *getFieldParse( void ) const { return m_multiplayerSettingsFieldParseTable; }

	// Color management --------------------
	MultiplayerColorDefinition * findMultiplayerColorDefinitionByName(AsciiString name);
	MultiplayerColorDefinition * newMultiplayerColorDefinition(AsciiString name);

	inline Int getInitialCreditsMin( void ) { return m_initialCreditsMin; }
	inline Int getInitialCreditsMax( void ) { return m_initialCreditsMax; }
	inline Int getStartCountdownTimerSeconds( void ) { return m_startCountdownTimerSeconds; }
	inline Int getMaxBeaconsPerPlayer( void ) { return m_maxBeaconsPerPlayer; }
	inline Bool isShroudInMultiplayer( void ) { return m_isShroudInMultiplayer; }
	inline Bool showRandomPlayerTemplate( void ) { return m_showRandomPlayerTemplate; }
	inline Bool showRandomStartPos( void ) { return m_showRandomStartPos; }
	inline Bool showRandomColor( void ) { return m_showRandomColor; }

	inline Int getNumColors( void )
	{
		if (m_numColors == 0) {
			m_numColors = m_colorList.size();
		}
		return m_numColors;
	}
	MultiplayerColorDefinition * getColor(Int which);

private:
	Int m_initialCreditsMin;
	Int m_initialCreditsMax;
	Int m_startCountdownTimerSeconds;
	Int m_maxBeaconsPerPlayer;
	Bool m_isShroudInMultiplayer;
	Bool m_showRandomPlayerTemplate;
	Bool m_showRandomStartPos;
	Bool m_showRandomColor;

	MultiplayerColorList m_colorList;
	Int m_numColors;
	MultiplayerColorDefinition m_observerColor;
	MultiplayerColorDefinition m_randomColor;
};

// singleton
extern MultiplayerSettings *TheMultiplayerSettings;
