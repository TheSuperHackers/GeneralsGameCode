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

// FILE: W3DDisplayStringManager.cpp //////////////////////////////////////////////////////////////
// Created: Colin Day, July 2001
// Desc:    Display string Manager for W3D
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "Common/Debug.h"
#include "GameClient/GameClient.h"
#include "GameClient/GameText.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DrawGroupInfo.h"
#include "GameClient/GlobalLanguage.h"
#include "W3DDevice/GameClient/W3DDisplayStringManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
W3DDisplayStringManager::W3DDisplayStringManager( void )
{
	for (Int i = 0; i < MAX_GROUPS; ++i)
	{
		m_groupNumeralStrings[i] = NULL;
	}

	m_formationLetterDisplayString = NULL;

}

//-------------------------------------------------------------------------------------------------
W3DDisplayStringManager::~W3DDisplayStringManager( void )
{
	for (Int i = 0; i < MAX_GROUPS; ++i)
	{
		if (m_groupNumeralStrings[i])
			freeDisplayString(m_groupNumeralStrings[i]);
		m_groupNumeralStrings[i] = NULL;
	}

	if (m_formationLetterDisplayString)
		freeDisplayString( m_formationLetterDisplayString );
	m_formationLetterDisplayString = NULL;


}

//-------------------------------------------------------------------------------------------------
void W3DDisplayStringManager::postProcessLoad( void )
{
	// Get the font.
	GameFont *font = TheFontLibrary->getFont(
		TheDrawGroupInfo->m_fontName,
		TheDrawGroupInfo->m_fontSize,
		TheDrawGroupInfo->m_fontIsBold );

	for (Int i = 0; i < MAX_GROUPS; ++i)
	{
		m_groupNumeralStrings[i] = newDisplayString();
		m_groupNumeralStrings[i]->setFont(font);

#ifdef KRIS_BRUTAL_HACK_FOR_AIRCRAFT_CARRIER_DEBUGGING
		UnicodeString displayNumber;
		displayNumber.format( L"%d", i);
		m_groupNumeralStrings[i]->setText( displayNumber );
#else
 		AsciiString displayNumber;
 		displayNumber.format("NUMBER:%d", i);
 		m_groupNumeralStrings[i]->setText(TheGameText->fetch(displayNumber));
#endif
	}

	m_formationLetterDisplayString = newDisplayString();
	m_formationLetterDisplayString->setFont(font);
	AsciiString displayLetter;
	displayLetter.format("LABEL:FORMATION");
	m_formationLetterDisplayString->setText(TheGameText->fetch(displayLetter));


}

//-------------------------------------------------------------------------------------------------
/** Allocate a new display string and tie it to the master list so we
	* can keep track of it */
//-------------------------------------------------------------------------------------------------
DisplayString *W3DDisplayStringManager::newDisplayString( void )
{
	DisplayString *newString = newInstance(W3DDisplayString);

	// sanity
	if( newString == NULL )
	{

		DEBUG_LOG(( "newDisplayString: Could not allcoate new W3D display string" ));
		assert( 0 );
		return NULL;

	}  // end if

	// assign a default font
	if (TheGlobalLanguageData && TheGlobalLanguageData->m_defaultDisplayStringFont.name.isNotEmpty())
	{
		newString->setFont(TheFontLibrary->getFont(
			TheGlobalLanguageData->m_defaultDisplayStringFont.name,
			TheGlobalLanguageData->m_defaultDisplayStringFont.size,
			TheGlobalLanguageData->m_defaultDisplayStringFont.bold) );
	}
	else
		newString->setFont( TheFontLibrary->getFont( AsciiString("Times New Roman"), 12, FALSE ) );

	// link string to list
	link( newString );

	// return our new string
	return newString;

}  // end newDisplayString

//-------------------------------------------------------------------------------------------------
/** Remove a display string from the master list and delete the data */
//-------------------------------------------------------------------------------------------------
void W3DDisplayStringManager::freeDisplayString( DisplayString *string )
{

	// sanity
	if( string == NULL )
		return;

	// unlink
	unLink( string );

	// if the string happens to fall where our current checkpoint was, set the checkpoint to null
	if ( m_currentCheckpoint == string) {
		m_currentCheckpoint = NULL;
	}

	// free data
	deleteInstance(string);

}  // end freeDisplayString

//-------------------------------------------------------------------------------------------------
/** Update method for our display string Manager ... if it's been too
	* long since the last time a string has been rendered, we will free
	* the rendering resources of the string, if it needs to render again
	* the DisplayString will have to rebuild the rendering data before
	* the draw will work */
//-------------------------------------------------------------------------------------------------
void W3DDisplayStringManager::update( void )
{
	// call base in case we add something later
	DisplayStringManager::update();

	W3DDisplayString *string = static_cast<W3DDisplayString *>(m_stringList);

	// if the m_currentCheckpoint is valid, use it for the starting point for the search
	if (m_currentCheckpoint) {
		string = static_cast<W3DDisplayString *>(m_currentCheckpoint);
	}

	UnsignedInt currFrame = TheGameClient->getFrame();
	const UnsignedInt w3dCleanupTime = 60;  /** any string not rendered after
																					this many frames will have its
																					render resources freed */

	int numStrings = 10;
	// looping through all the strings eats up a lot of ambient time. Instead,
	// loop through 10 (arbitrarily chosen) or till the end is hit.
	while ( numStrings-- && string)
	{

		//
		// has this string "expired" in terms of using resources, a string
		// with a resource frame of zero isn't using any resources at all
		//
		if( string->m_lastResourceFrame != 0 &&
				currFrame - string->m_lastResourceFrame > w3dCleanupTime )
		{

			// free the resources
			string->m_textRenderer.Reset();
			string->m_textRendererHotKey.Reset();
			//
			// mark data in the string as changed so that if it needs to
			// be drawn again it will know to reconstruct the render data
			//
			string->m_textChanged = TRUE;

			//
			// set the last resource frame to zero, this allows us to ignore it
			// in future cleanup passes of this update routine
			//
			string->m_lastResourceFrame = 0;

		}  // end if

		// move to next string
		string = static_cast<W3DDisplayString *>(string->next());

	}  // end while

	// reset the starting point for our next search
	m_currentCheckpoint = string;
}  // end update

//-------------------------------------------------------------------------------------------------
DisplayString *W3DDisplayStringManager::getGroupNumeralString( Int numeral )
{
	if (numeral < 0 || numeral > MAX_GROUPS - 1 )
	{
		DEBUG_CRASH(("Numeral '%d' out of range.", numeral));
		return m_groupNumeralStrings[0];
	}

	return m_groupNumeralStrings[numeral];
}
