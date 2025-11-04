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

// FILE: WinInstanceData.cpp //////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  WinInstanceData.cpp
//
// Created:    Colin Day, July 2001
//
// Desc:       Game window instance data
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/WinInstanceData.h"
#include "GameClient/GameWindow.h"
#include "GameClient/DisplayStringManager.h"
#include "Common/Debug.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// WinInstanceData::WinInstanceData ===========================================
//=============================================================================
WinInstanceData::WinInstanceData( void )
{

	// we don't allocate strings unless we need them
	m_text = NULL;
	m_tooltip = NULL;
	m_videoBuffer = NULL;
	init();

}

// WinInstanceData::~WinInstanceData ==========================================
//=============================================================================
WinInstanceData::~WinInstanceData( void )
{

	if( m_text )
		TheDisplayStringManager->freeDisplayString( m_text );

	if( m_tooltip )
		TheDisplayStringManager->freeDisplayString( m_tooltip );

	m_videoBuffer = NULL; //Video Buffer needs to be clean up by the control that is in charge of the video.

}

// WinInstanceData::init ======================================================
/** Set initial values for instance data if desired */
//=============================================================================
void WinInstanceData::init( void )
{
	Int i;

	// init our draw data images/colors for the states
	for( i = 0; i < MAX_DRAW_DATA; i++ )
	{

		m_enabledDrawData[ i ].image = NULL;
		m_enabledDrawData[ i ].color = WIN_COLOR_UNDEFINED;
		m_enabledDrawData[ i ].borderColor = WIN_COLOR_UNDEFINED;

		m_disabledDrawData[ i ].image = NULL;
		m_disabledDrawData[ i ].color = WIN_COLOR_UNDEFINED;
		m_disabledDrawData[ i ].borderColor = WIN_COLOR_UNDEFINED;

		m_hiliteDrawData[ i ].image = NULL;
		m_hiliteDrawData[ i ].color = WIN_COLOR_UNDEFINED;
		m_hiliteDrawData[ i ].borderColor = WIN_COLOR_UNDEFINED;

	}

	// initialize text colors
	m_enabledText.color					= WIN_COLOR_UNDEFINED;
	m_enabledText.borderColor		= WIN_COLOR_UNDEFINED;
	m_disabledText.color				= WIN_COLOR_UNDEFINED;
	m_disabledText.borderColor	= WIN_COLOR_UNDEFINED;
	m_hiliteText.color					= WIN_COLOR_UNDEFINED;
	m_hiliteText.borderColor		= WIN_COLOR_UNDEFINED;

	m_id = 0;
	m_state = 0;
	m_style = 0;
	m_status = WIN_STATUS_NONE;
	m_owner = NULL;
	m_textLabelString.clear();
	m_tooltipString.clear();
  m_tooltipDelay = -1; ///< default value
	m_decoratedNameString.clear();

	m_imageOffset.x = 0;
	m_imageOffset.y = 0;

	// reset all data for the text display strings and font for window
	m_font = NULL;
	if( m_text )
	{

		TheDisplayStringManager->freeDisplayString( m_text );
		m_text = NULL;

	}
	if( m_tooltip )
	{

		TheDisplayStringManager->freeDisplayString( m_tooltip );
		m_tooltip = NULL;

	}

	m_videoBuffer = NULL;


}

// WinInstanceData::setTooltipText ============================================
//=============================================================================
void WinInstanceData::setTooltipText( UnicodeString tip )
{

	// allocate a text tooltip string if needed
	if( m_tooltip == NULL )
		m_tooltip = TheDisplayStringManager->newDisplayString();
	DEBUG_ASSERTCRASH( m_tooltip, ("no tooltip") );

	// set text
	m_tooltip->setText( tip );

}

// WinInstanceData:setText ====================================================
/** Set the text for this window instance data */
//=============================================================================
void WinInstanceData::setText( UnicodeString text )
{

	// allocate a text instance if needed
	if( m_text == NULL )
		m_text = TheDisplayStringManager->newDisplayString();
	DEBUG_ASSERTCRASH( m_text, ("no text") );

	// set the text
	m_text->setText( text );

}

// WinInstanceData:setText ====================================================
/** Set the text for this window instance data */
//=============================================================================
void WinInstanceData::setVideoBuffer( VideoBuffer * videoBuffer )
{
	m_videoBuffer = videoBuffer;
}
