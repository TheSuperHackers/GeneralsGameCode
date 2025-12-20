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

// FILE: EditWindow.h /////////////////////////////////////////////////////////
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
// File name:  EditWindow.h
//
// Created:    Colin Day, July 2001
//
// Desc:       Main edit window for the GUI editing tool
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <windows.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEditColor.h"
#include "Lib/BaseType.h"
#include "GameClient/Image.h"
#include "GameClient/GameWindow.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/render2d.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// EditWindow -----------------------------------------------------------------
/** The edit window singleton definition, this is where we create and
	* interact with GUI controls for this tool */
//-----------------------------------------------------------------------------
class EditWindow
{

public:

	EditWindow( void );
	~EditWindow( void );

	/// initialize the edit window singleton
	void init( UnsignedInt clientWidth, UnsignedInt clientHeight );
	void shutdown( void );  ///< free all data
	void draw( void );  ///< draw the edit window

	void updatePulse( void );  ///< pulse message from timer

	HWND getWindowHandle( void );  ///< get window handle
	void setSize( ICoord2D *size );  ///< set width and height for edit window
	void getSize( ICoord2D *size );  ///< get width and height for edit window

	RGBColorReal getBackgroundColor( void );  ///< return the background color
	void setBackgroundColor( RGBColorReal color );  ///< set background color

	void setDragMoveOrigin( ICoord2D *pos );  ///< for drag moving
	void setDragMoveDest( ICoord2D *pos );  ///< for drag moving
	ICoord2D getDragMoveOrigin( void );  ///< for keybord moving
	ICoord2D getDragMoveDest( void );  ///< for keyboard moving

	void notifyWindowDeleted( GameWindow *window );  ///< window has been deleted

	/// mouse event has occurred, button up/down, move etc.
	void mouseEvent( UnsignedInt windowsMessage, WPARAM wParam, LPARAM lParam );

	void getPopupMenuClickPos( ICoord2D *pos );  ///< get popup menu click loc
	void openPopupMenu( Int x, Int y );  ///< open floating popup right click menu

	// **************************************************************************

	/// draw a line on the display in screen coordinates
	void drawLine( Int startX, Int startY, Int endX, Int endY,
								 Real lineWidth, UnsignedInt lineColor );

	/// draw a rect border on the display in pixel coordinates with the specified color
	void drawOpenRect( Int startX, Int startY, Int width, Int height,
										 Real lineWidth, UnsignedInt lineColor );

	/// draw a filled rect on the display in pixel coords with the specified color
	void drawFillRect( Int startX, Int startY, Int width, Int height,
										 UnsignedInt color );

	/// draw an image fit within the screen coordinates
	void drawImage( const Image *image, Int startX, Int startY,
									Int endX, Int endY, Color color = 0xFFFFFFFF );

	/// image clipping support
	void setClipRegion( IRegion2D *region ) {m_clipRegion = *region; m_isClippedEnabled = TRUE;}
	Bool isClippingEnabled( void )	{ return m_isClippedEnabled; }
	void enableClipping( Bool onoff )	{ m_isClippedEnabled = onoff; }

protected:

	void registerEditWindowClass( void );  ///< register class with OS

	/// callback from windows, NOTE that it's static and has no this pointer
	static LRESULT CALLBACK editProc( HWND hWnd, UINT message,
																		WPARAM wParam, LPARAM lParam );

	void drawGrid( void );  ///< draw the grid
	void drawSeeThruOutlines( GameWindow *windowList, Color c );
	void drawHiddenOutlines( GameWindow *windowList, Color c );
	void drawUIFeedback( void );  ///< draw UI visual feedback

	/// if mouse is close to selected window allow resize
	void handleResizeAvailable( Int mouseX, Int mouseY );

	//
	// these methods check to see if the mouse is close enough to the
	// given geometry region (usually for resizing controls)
	//
	Bool inCornerTolerance( ICoord2D *dest, ICoord2D *source, Int tolerance );
	Bool inLineTolerance( ICoord2D *dest, ICoord2D *lineStart, ICoord2D *lineEnd,
												Int tolerance );

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	static Bool m_classRegistered;  ///< TRUE when we've register with OS
	static const char *m_className;  ///< name for windows class

	ICoord2D m_size;  ///< width and height of edit window
	UnsignedByte m_bitDepth;  ///< bit depth for edit window
	HWND m_editWindowHWnd;  ///< edit window handle

	Int m_pulse;  ///< for visual feedback that looks cool!

	RGBColorReal m_backgroundColor;  ///< the background color

	Bool m_w3dInitialized;  ///< TRUE once W3D is up
	WW3DAssetManager *m_assetManager;  ///< asset manager for WW3D
	Render2DClass *m_2DRender;  ///< our 2D renderer

	ICoord2D m_popupMenuClickPos;  ///< position where popup menu was created at
	GameWindow *m_pickedWindow;  ///< picked window from mouse click editing

	ICoord2D m_dragMoveOrigin;  ///< mouse click position to start drag move
	ICoord2D m_dragMoveDest;  ///< destination for drag move

	Bool m_dragSelecting;  ///< TRUE when drawing a selection box
	IRegion2D m_selectRegion;  ///< region for selection box

	Bool m_resizingWindow;  ///< TRUE when drag resizing a window
	GameWindow *m_windowToResize;  ///< the window to resize
	ICoord2D m_resizeOrigin;  ///< mouse clicked down here to drag resize
	ICoord2D m_resizeDest;  ///< mouse pos when dragging around to resize

	IRegion2D m_clipRegion;		///< the clipping region for images
	Bool m_isClippedEnabled;	///<used by 2D drawing operations to define clip re

};

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
inline HWND EditWindow::getWindowHandle( void ) { return m_editWindowHWnd; }
inline void EditWindow::getSize( ICoord2D *size ) { *size = m_size; }
inline void EditWindow::getPopupMenuClickPos( ICoord2D *pos ) { *pos = m_popupMenuClickPos; }
inline void EditWindow::setDragMoveDest( ICoord2D *pos ) { if( pos ) m_dragMoveDest = *pos; }
inline void EditWindow::setDragMoveOrigin( ICoord2D *pos ) { if( pos ) m_dragMoveOrigin = *pos; }
inline ICoord2D EditWindow::getDragMoveDest( void ) { return m_dragMoveDest; }
inline ICoord2D EditWindow::getDragMoveOrigin( void ) { return m_dragMoveOrigin; }

///////////////////////////////////////////////////////////////////////////////
// EXTERNALS //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern EditWindow *TheEditWindow;  ///< edit window singleton extern
