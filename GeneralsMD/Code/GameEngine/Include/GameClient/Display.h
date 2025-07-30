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

// FILE: Display.h ////////////////////////////////////////////////////////////
// The graphics display singleton
// Author: Michael S. Booth, March 2001

#pragma once

#ifndef _GAME_DISPLAY_H_
#define _GAME_DISPLAY_H_

#include <stdio.h>
#include "Common/SubsystemInterface.h"
#include "View.h"
#include "GameClient/Color.h"
#include "GameClient/GameFont.h"

class View;

struct ShroudLevel
{
	Short m_currentShroud;		///< A Value of 1 means shrouded.  0 is not.  Negative is the count of people looking.
	Short m_activeShroudLevel;///< A Value of 0 means passive shroud.  Positive is the count of people shrouding.
};

class VideoBuffer;
class VideoStreamInterface;
class DebugDisplayInterface;
class Radar;
class Image;
class DisplayString;
enum StaticGameLODLevel CPP_11(: Int);
/**
 * The Display class implements the Display interface
 */
class Display : public SubsystemInterface
{

public:
	enum DrawImageMode
	{
		DRAW_IMAGE_SOLID,
		DRAW_IMAGE_GRAYSCALE,		//draw image without blending and ignoring alpha
		DRAW_IMAGE_ALPHA,		//alpha blend the image into frame buffer
		DRAW_IMAGE_ADDITIVE	//additive blend the image into frame buffer
	};

	typedef void (DebugDisplayCallback)( DebugDisplayInterface *debugDisplay, void *userData, FILE *fp );

	Display();
	virtual ~Display();

	virtual void init( void ) { };																///< Initialize
	virtual void reset( void );																		///< Reset system
	virtual void update( void );																	///< Update system

	//---------------------------------------------------------------------------------------
	// Display attribute methods
	virtual void setWidth( UnsignedInt width );										///< Sets the width of the display		
	virtual void setHeight( UnsignedInt height );									///< Sets the height of the display
	virtual UnsignedInt getWidth( void ) { return m_width; }			///< Returns the width of the display
	virtual UnsignedInt getHeight( void ) { return m_height; }		///< Returns the height of the display
	virtual void setBitDepth( UnsignedInt bitDepth ) { m_bitDepth = bitDepth; }
	virtual UnsignedInt getBitDepth( void ) { return m_bitDepth; }
	virtual void setWindowed( Bool windowed ) { m_windowed = windowed; }  ///< set windowd/fullscreen flag
	virtual Bool getWindowed( void ) { return m_windowed; }				///< return widowed/fullscreen flag
	virtual Bool setDisplayMode( UnsignedInt xres, UnsignedInt yres, UnsignedInt bitdepth, Bool windowed );	///<sets screen resolution/mode
	virtual Int getDisplayModeCount(void) {return 0;}	///<return number of display modes/resolutions supported by video card.
	virtual void getDisplayModeDescription(Int modeIndex, Int *xres, Int *yres, Int *bitDepth) {}	///<return description of mode
 	virtual void setGamma(Real gamma, Real bright, Real contrast, Bool calibrate) {};
	virtual Bool testMinSpecRequirements(Bool *videoPassed, Bool *cpuPassed, Bool *memPassed,StaticGameLODLevel *idealVideoLevel=NULL, Real *cpuTime=NULL) {*videoPassed=*cpuPassed=*memPassed=true; return true;}
	virtual void doSmartAssetPurgeAndPreload(const char* usageFileName) = 0;
#if defined(RTS_DEBUG)
	virtual void dumpAssetUsage(const char* mapname) = 0;
#endif

	//---------------------------------------------------------------------------------------
	// View management
	virtual void attachView( View *view );												///< Attach the given view to the world
	virtual View *getFirstView( void ) { return m_viewList; }				///< Return the first view of the world
	virtual View *getNextView( View *view )
	{
		if( view )
			return view->getNextView();
		return NULL;
	}

	virtual void drawViews( void );																///< Render all views of the world
	virtual void updateViews ( void );															///< Updates state of world views

	virtual VideoBuffer*	createVideoBuffer( void ) = 0;							///< Create a video buffer that can be used for this display

	//---------------------------------------------------------------------------------------
	// Drawing management
	virtual void setClipRegion( IRegion2D *region ) = 0;	///< Set clip rectangle for 2D draw operations.
	virtual	Bool isClippingEnabled( void ) = 0;
	virtual	void enableClipping( Bool onoff ) = 0;

	virtual void draw( void );																		///< Redraw the entire display
	virtual void setTimeOfDay( TimeOfDay tod ) = 0;								///< Set the time of day for this display
	virtual void createLightPulse( const Coord3D *pos, const RGBColor *color, Real innerRadius,Real attenuationWidth, 
																 UnsignedInt increaseFrameTime, UnsignedInt decayFrameTime//, Bool donut = FALSE
																 ) = 0;

	/// draw a line on the display in pixel coordinates with the specified color
	virtual void drawLine( Int startX, Int startY, Int endX, Int endY, 
												 Real lineWidth, UnsignedInt lineColor ) = 0;
	/// draw a line on the display in pixel coordinates with the specified 2 colors
	virtual void drawLine( Int startX, Int startY, Int endX, Int endY, 
												 Real lineWidth, UnsignedInt lineColor1, UnsignedInt lineColor2 ) = 0;
	/// draw a rect border on the display in pixel coordinates with the specified color
	virtual void drawOpenRect( Int startX, Int startY, Int width, Int height,
														 Real lineWidth, UnsignedInt lineColor ) = 0;
	/// draw a filled rect on the display in pixel coords with the specified color
	virtual void drawFillRect( Int startX, Int startY, Int width, Int height,
														 UnsignedInt color ) = 0;
	
	/// Draw a percentage of a rectange, much like a clock
	virtual void drawRectClock(Int startX, Int startY, Int width, Int height, Int percent, UnsignedInt color) = 0;
	virtual void drawRemainingRectClock(Int startX, Int startY, Int width, Int height, Int percent, UnsignedInt color) = 0;

	/// draw an image fit within the screen coordinates
	virtual void drawImage( const Image *image, Int startX, Int startY, 
													Int endX, Int endY, Color color = 0xFFFFFFFF, DrawImageMode mode=DRAW_IMAGE_ALPHA) = 0;

	/// draw a video buffer fit within the screen coordinates
	virtual void drawScaledVideoBuffer( VideoBuffer *buffer, VideoStreamInterface *stream ) = 0;
	virtual void drawVideoBuffer( VideoBuffer *buffer, Int startX, Int startY, 
													Int endX, Int endY ) = 0;

	/// FullScreen video playback 
	virtual void playLogoMovie( AsciiString movieName, Int minMovieLength, Int minCopyrightLength );
	virtual void playMovie( AsciiString movieName );
	virtual void stopMovie( void );
	virtual Bool isMoviePlaying(void);

	/// Register debug display callback
	virtual void setDebugDisplayCallback( DebugDisplayCallback *callback, void *userData = NULL  );
	virtual DebugDisplayCallback *getDebugDisplayCallback();

	virtual void setShroudLevel(Int x, Int y, CellShroudStatus setting ) = 0;	  ///< set shroud
	virtual void clearShroud() = 0;														///< empty the entire shroud
	virtual void setBorderShroudLevel(UnsignedByte level) = 0;	///<color that will appear in unused border terrain.

#if defined(RTS_DEBUG)
	virtual void dumpModelAssets(const char *path) = 0;	///< dump all used models/textures to a file.
#endif
	virtual void preloadModelAssets( AsciiString model ) = 0;	///< preload model asset
	virtual void preloadTextureAssets( AsciiString texture ) = 0;	///< preload texture asset

	virtual void takeScreenShot(void) = 0;										///< saves screenshot to a file
	virtual void toggleMovieCapture(void) = 0;							///< starts saving frames to an avi or frame sequence
	virtual void toggleLetterBox(void) = 0;										///< enabled letter-boxed display
	virtual void enableLetterBox(Bool enable) = 0;						///< forces letter-boxed display on/off
	virtual Bool isLetterBoxFading( void ) { return FALSE; }	///< returns true while letterbox fades in/out
	virtual Bool isLetterBoxed( void ) { return FALSE; }	//WST 10/2/2002. Added query interface

	virtual void setCinematicText( AsciiString string ) { m_cinematicText = string; }
	virtual void setCinematicFont( GameFont *font ) { m_cinematicFont = font; }
	virtual void setCinematicTextFrames( Int frames ) { m_cinematicTextFrames = frames; }

	virtual Real getAverageFPS( void ) = 0;	///< returns the average FPS.
	virtual Real getCurrentFPS( void ) = 0;	///< returns the current FPS.
	virtual Int getLastFrameDrawCalls( void ) = 0;  ///< returns the number of draw calls issued in the previous frame

protected:

	virtual void deleteViews( void );   ///< delete all views
	UnsignedInt m_width, m_height;			///< Dimensions of the display
	UnsignedInt m_bitDepth;							///< bit depth of the display
	Bool m_windowed;										///< TRUE when windowed, FALSE when fullscreen
	View *m_viewList;										///< All of the views into the world

	// Cinematic text data
	AsciiString m_cinematicText;        ///< string of the cinematic text that should be displayed
	GameFont *m_cinematicFont;           ///< font for cinematic text
	Int m_cinematicTextFrames;          ///< count of how long the cinematic text should be displayed

	// Video playback data
	VideoBuffer						*m_videoBuffer;						///< Video playback buffer
	VideoStreamInterface	*m_videoStream;						///< Video stream;
	AsciiString						 m_currentlyPlayingMovie;	///< The currently playing video. Used to notify TheScriptEngine of completed videos.

	// Debug display data
	DebugDisplayInterface *m_debugDisplay;					///< Actual debug display
	DebugDisplayCallback	*m_debugDisplayCallback;	///< Code to update the debug display
	void									*m_debugDisplayUserData;	///< Data for debug display update handler
	Real	m_letterBoxFadeLevel;	///<tracks the current alpha level for fading letter-boxed mode in/out.
	Bool	m_letterBoxEnabled;		///<current state of letterbox
	UnsignedInt	m_letterBoxFadeStartTime;		///< time of letterbox fade start
	Int		m_movieHoldTime;									///< time that we hold on the last frame of the movie
	Int		m_copyrightHoldTime;							///< time that the copyright must be on the screen
	UnsignedInt m_elapsedMovieTime;					///< used to make sure we show the stuff long enough
	UnsignedInt m_elapsedCopywriteTime;			///< Hold on the last frame until both have expired
	DisplayString *m_copyrightDisplayString;///< this'll hold the display string
};

// the singleton
extern Display *TheDisplay;

extern void StatDebugDisplay( DebugDisplayInterface *dd, void *, FILE *fp = NULL );

//Added By Saad
//Necessary for display resolution confirmation dialog box
//Holds the previous and current display settings

typedef struct _DisplaySettings
{
	Int xRes;  //Resolution width
	Int yRes;  //Resolution height
	Int bitDepth; //Color Depth
	Bool windowed; //Window mode TRUE: we're windowed, FALSE: we're not windowed
} DisplaySettings;


#endif // _GAME_DISPLAY_H_
