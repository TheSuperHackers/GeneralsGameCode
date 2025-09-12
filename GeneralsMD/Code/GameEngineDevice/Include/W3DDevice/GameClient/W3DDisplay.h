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

// FILE: W3DDisplay.h /////////////////////////////////////////////////////////
//
// W3D Implementation for the W3D Display which is responsible for creating
// and maintaning the entire visual display
//
// Author: Colin Day, April 2001
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DDISPLAY_H_
#define __W3DDISPLAY_H_

#include "GameClient/Display.h"
#include "WW3D2/lightenvironment.h"

class VideoBuffer;
class W3DDebugDisplay;
class DisplayString;
class W3DAssetManager;
class LightClass;
class Render2DClass;
class RTS3DScene;
class RTS2DScene;
class RTS3DInterfaceScene;


//=============================================================================
/** W3D implementation of the game display which is responsible for creating
  * all interaction with the screen and updating the display
	*/
class W3DDisplay : public Display
{

public:
	W3DDisplay();
	~W3DDisplay();

	virtual void init( void );  ///< initialize or re-initialize the sytsem
 	virtual void reset( void ) ;																///< Reset system

	virtual void setWidth( UnsignedInt width );
	virtual void setHeight( UnsignedInt height );
	virtual Bool setDisplayMode( UnsignedInt xres, UnsignedInt yres, UnsignedInt bitdepth, Bool windowed );
	virtual Int getDisplayModeCount(void);	///<return number of display modes/resolutions supported by video card.
	virtual void getDisplayModeDescription(Int modeIndex, Int *xres, Int *yres, Int *bitDepth);	///<return description of mode
 	virtual void setGamma(Real gamma, Real bright, Real contrast, Bool calibrate);
	virtual void doSmartAssetPurgeAndPreload(const char* usageFileName);
#if defined(RTS_DEBUG)
	virtual void dumpAssetUsage(const char* mapname);
#endif

	//---------------------------------------------------------------------------
	// Drawing management
	virtual void setClipRegion( IRegion2D *region );	///< Set clip rectangle for 2D draw operations.
	virtual Bool	isClippingEnabled( void ) 	{ return m_isClippedEnabled; }
	virtual void	enableClipping( Bool onoff )		{ m_isClippedEnabled = onoff; }

	virtual void step(); ///< Do one fixed time step
	virtual void draw( void );  ///< redraw the entire display

	/// @todo Replace these light management routines with a LightManager singleton
	virtual void createLightPulse( const Coord3D *pos, const RGBColor *color, Real innerRadius,Real outerRadius,
																 UnsignedInt increaseFrameTime, UnsignedInt decayFrameTime//, Bool donut = FALSE
																 );
	virtual void setTimeOfDay ( TimeOfDay tod );

	/// draw a line on the display in screen coordinates
	virtual void drawLine( Int startX, Int startY, Int endX, Int endY,
												 Real lineWidth, UnsignedInt lineColor );

	/// draw a line on the display in screen coordinates
	virtual void drawLine( Int startX, Int startY, Int endX, Int endY,
												 Real lineWidth, UnsignedInt lineColor1, UnsignedInt lineColor2 );

	/// draw a rect border on the display in pixel coordinates with the specified color
	virtual void drawOpenRect( Int startX, Int startY, Int width, Int height,
														 Real lineWidth, UnsignedInt lineColor );

	/// draw a filled rect on the display in pixel coords with the specified color
	virtual void drawFillRect( Int startX, Int startY, Int width, Int height,
														 UnsignedInt color );

	/// Draw a percentage of a rectangle, much like a clock (0 to x%)
	virtual void drawRectClock(Int startX, Int startY, Int width, Int height, Int percent, UnsignedInt color);

	/// Draw's the remaining percentage of a rectangle (x% to 100)
	virtual void drawRemainingRectClock(Int startX, Int startY, Int width, Int height, Int percent, UnsignedInt color);

	/// draw an image fit within the screen coordinates
	virtual void drawImage( const Image *image, Int startX, Int startY,
													Int endX, Int endY, Color color = 0xFFFFFFFF, DrawImageMode mode=DRAW_IMAGE_ALPHA);

	/// draw a video buffer fit within the screen coordinates
	virtual void drawScaledVideoBuffer( VideoBuffer *buffer, VideoStreamInterface *stream );
	virtual void drawVideoBuffer( VideoBuffer *buffer, Int startX, Int startY,
													Int endX, Int endY );

	virtual VideoBuffer*	createVideoBuffer( void ) ;							///< Create a video buffer that can be used for this display

	virtual void takeScreenShot(void);						//save screenshot to file
	virtual void toggleMovieCapture(void);			//enable AVI or frame capture mode.

	virtual void toggleLetterBox(void);	///<enabled letter-boxed display
	virtual void enableLetterBox(Bool enable);	///<forces letter-boxed display on/off

	virtual Bool isLetterBoxFading(void);	///<returns true while letterbox fades in/out
	virtual Bool isLetterBoxed(void);

	virtual void clearShroud();
	virtual void setShroudLevel(Int x, Int y, CellShroudStatus setting);
	virtual void setBorderShroudLevel(UnsignedByte level);	///<color that will appear in unused border terrain.
#if defined(RTS_DEBUG)
	virtual void dumpModelAssets(const char *path);	///< dump all used models/textures to a file.
#endif
	virtual void preloadModelAssets( AsciiString model );			///< preload model asset
	virtual void preloadTextureAssets( AsciiString texture );	///< preload texture asset

	/// @todo Need a scene abstraction
	static RTS3DScene *m_3DScene;							///< our 3d scene representation
	static RTS2DScene *m_2DScene;							///< our 2d scene representation
	static RTS3DInterfaceScene *m_3DInterfaceScene;	///< our 3d interface scene that draws last (for 3d mouse cursor, etc)
	static W3DAssetManager *m_assetManager;		///< W3D asset manager

	void drawFPSStats( void );								///< draw the fps on the screen
	virtual Real getAverageFPS( void );						///< return the average FPS.
	virtual Real getCurrentFPS( void );						///< return the current FPS.
	virtual Int getLastFrameDrawCalls( void );				///< returns the number of draw calls issued in the previous frame

protected:

	void initAssets( void );									///< init assets for WW3D
	void init3DScene( void );									///< init 3D scene for WW3D
	void init2DScene( void );									///< init 2D scene for WW3D
	void gatherDebugStats( void );						///< compute debug stats
	void drawDebugStats( void );							///< display debug stats
	void drawCurrentDebugDisplay( void );			///< draws current debug display
	void calculateTerrainLOD(void);						///< Calculate terrain LOD.
	void renderLetterBox(UnsignedInt time);							///< draw letter box border
	void updateAverageFPS(void);	///< figure out the average fps over the last 30 frames.

	Byte m_initialized;												///< TRUE when system is initialized
	LightClass *m_myLight[LightEnvironmentClass::MAX_LIGHTS];										///< light hack for now
	Render2DClass *m_2DRender;								///< interface for common 2D functions
	IRegion2D m_clipRegion;									///< the clipping region for images
	Bool m_isClippedEnabled;	///<used by 2D drawing operations to define clip re
	Real m_averageFPS;		///<average fps over the last 30 frames.
	Real m_currentFPS;		///<current fps value.
#if defined(RTS_DEBUG)
	Int64 m_timerAtCumuFPSStart;
#endif

	enum
	{
		FPS,							///< debug display frames per second
		Frame,						///< debug display current frame
		Polygons,					///< debug display polygons
		Vertices,					///< debug display vertices
		VideoRam,					///< debug display for video ram used
		DebugInfo,				///< miscellaneous debug info string
		KEY_MOUSE_STATES, ///< keyboard modifier and mouse button states.
		MousePosition,    ///< debug display mouse position
		Particles,        ///< debug display particles
		Objects,          ///< debug display total number of objects
		NetIncoming,			///< debug display network incoming stats
		NetOutgoing,			///< debug display network outgoing stats
		NetStats,					///< debug display network performance stats.
		NetFPSAverages,		///< debug display all players' average fps.
		SelectedInfo,			///< debug display for the selected object in the UI
		TerrainStats,			///< debug display for the terrain renderer

		DisplayStringCount
	};

	DisplayString *m_displayStrings[DisplayStringCount];
	DisplayString *m_benchmarkDisplayString;

	W3DDebugDisplay *m_nativeDebugDisplay;		///< W3D specific debug display interface

};

#endif  // end __W3DDISPLAY_H_
