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

// FILE: GUIEditDisplay.h /////////////////////////////////////////////////////
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
// File name:  GUIEditDisplay.h
//
// Created:    Colin Day, July 2001
//
// Desc:       Display implementation for the GUI edit tool
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Display.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////
class VideoBuffer;

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// GUIEditDisplay -------------------------------------------------------------
/** Stripped down display for the GUI tool editor */
//-----------------------------------------------------------------------------
class GUIEditDisplay : public Display
{

public:

	GUIEditDisplay( void );
	virtual ~GUIEditDisplay( void );

	virtual void draw( void ) { };

	/// draw a line on the display in pixel coordinates with the specified color
	virtual void drawLine( Int startX, Int startY, Int endX, Int endY,
												 Real lineWidth, UnsignedInt lineColor );
	virtual void drawLine( Int startX, Int startY, Int endX, Int endY,
												 Real lineWidth, UnsignedInt lineColor1, UnsignedInt lineColor2 ) { }
	/// draw a rect border on the display in pixel coordinates with the specified color
	virtual void drawOpenRect( Int startX, Int startY, Int width, Int height,
														 Real lineWidth, UnsignedInt lineColor );
	/// draw a filled rect on the display in pixel coords with the specified color
	virtual void drawFillRect( Int startX, Int startY, Int width, Int height,
														 UnsignedInt color );

	/// Draw a percentage of a rectange, much like a clock
	virtual void drawRectClock(Int startX, Int startY, Int width, Int height, Int percent, UnsignedInt color) { }
	virtual void drawRemainingRectClock(Int startX, Int startY, Int width, Int height, Int percent, UnsignedInt color) { }

	/// draw an image fit within the screen coordinates
	virtual void drawImage( const Image *image, Int startX, Int startY,
													Int endX, Int endY, Color color = 0xFFFFFFFF, DrawImageMode mode=DRAW_IMAGE_ALPHA);
	/// image clipping support
	virtual void setClipRegion( IRegion2D *region );
	virtual Bool isClippingEnabled( void );
	virtual void enableClipping( Bool onoff );

	// These are stub functions to allow compilation:

	/// Create a video buffer that can be used for this display
	virtual VideoBuffer*	createVideoBuffer( void ) { return NULL; }

	/// draw a video buffer fit within the screen coordinates
	virtual void drawScaledVideoBuffer( VideoBuffer *buffer, VideoStreamInterface *stream ) { }
	virtual void drawVideoBuffer( VideoBuffer *buffer, Int startX, Int startY,
																Int endX, Int endY ) { }
	virtual void takeScreenShot(void){ }
	virtual void toggleMovieCapture(void) {}

	// methods that we need to stub
	virtual void setTimeOfDay( TimeOfDay tod ) {}
	virtual void createLightPulse( const Coord3D *pos, const RGBColor *color, Real innerRadius, Real attenuationWidth,
																 UnsignedInt increaseFrameTime, UnsignedInt decayFrameTime ) {}
	virtual void setShroudLevel(Int x, Int y, CellShroudStatus setting) {}
	void setBorderShroudLevel(UnsignedByte level){}
	virtual void clearShroud() {}
	virtual void preloadModelAssets( AsciiString model ) {}
	virtual void preloadTextureAssets( AsciiString texture ) {}
	virtual void toggleLetterBox(void) {}
	virtual void enableLetterBox(Bool enable) {}
#if defined(RTS_DEBUG)
	virtual void dumpModelAssets(const char *path) {}
#endif
	virtual void doSmartAssetPurgeAndPreload(const char* usageFileName) {}
#if defined(RTS_DEBUG)
	virtual void dumpAssetUsage(const char* mapname) {}
#endif

	virtual Real getAverageFPS(void) { return 0; }
	virtual Real getCurrentFPS(void) { return 0; }
	virtual Int getLastFrameDrawCalls( void ) { return 0; }

protected:

};

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////
