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

// FILE: .cpp /////////////////////////////////////////////////////////////////
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
// File name:  .cpp
//
// Created:
//
// Desc:
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEditDisplay.h"
#include "EditWindow.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GUIEditDisplay::GUIEditDisplay =============================================
/** */
//=============================================================================
GUIEditDisplay::GUIEditDisplay( void )
{

}

// GUIEditDisplay::~GUIEditDisplay ============================================
/** */
//=============================================================================
GUIEditDisplay::~GUIEditDisplay( void )
{

}

// GUIEditDisplay::drawLine ===================================================
/** draw a line on the display in pixel coordinates with the specified color */
//=============================================================================
void GUIEditDisplay::drawLine( Int startX, Int startY,
															 Int endX, Int endY,
															 Real lineWidth, UnsignedInt lineColor )
{

	TheEditWindow->drawLine( startX, startY, endX, endY, lineWidth, lineColor );

}

// GUIEditDisplay::drawOpenRect ===============================================
/** draw a rect border on the display in pixel coordinates with the
	* specified color */
//=============================================================================
void GUIEditDisplay::drawOpenRect( Int startX, Int startY,
																	 Int width, Int height,
																	 Real lineWidth, UnsignedInt lineColor )
{

	TheEditWindow->drawOpenRect( startX, startY, width, height,
															 lineWidth, lineColor );

}

// GUIEditDisplay::drawFillRect ===============================================
/** draw a filled rect on the display in pixel coords with the
	* specified color */
//=============================================================================
void GUIEditDisplay::drawFillRect( Int startX, Int startY,
																	 Int width, Int height,
																	 UnsignedInt color )
{

	TheEditWindow->drawFillRect( startX, startY,
															 width, height,
															 color );

}

// GUIEditDisplay::drawImage ==================================================
/** draw an image fit within the screen coordinates */
//=============================================================================
void GUIEditDisplay::drawImage( const Image *image,
																Int startX, Int startY,
																Int endX, Int endY,
																Color color, DrawImageMode mode )
{

	TheEditWindow->drawImage( image,
														startX, startY,
														endX, endY,
														color );


}

// GUIEditDisplay::setClipRegion ==============================================
/** sets clipping rectangle for 2D drawing operations */
//=============================================================================
void GUIEditDisplay::setClipRegion( IRegion2D *region )
{
	TheEditWindow->setClipRegion(region);
}

// GUIEditDisplay::isClippingEnabled ==============================================
/** returns current state of 2D image clipping */
//=============================================================================
Bool GUIEditDisplay::isClippingEnabled( void )
{
	return	TheEditWindow->isClippingEnabled();
}

// GUIEditDisplay::isClippingEnabled ==============================================
/** returns current state of 2D image clipping */
//=============================================================================
void GUIEditDisplay::enableClipping( Bool onoff )
{
	TheEditWindow->enableClipping(onoff);
}
