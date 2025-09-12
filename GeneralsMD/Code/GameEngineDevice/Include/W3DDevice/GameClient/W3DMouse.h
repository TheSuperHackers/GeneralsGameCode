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

// FILE: W3DMouse.h /////////////////////////////////////////////////////////
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
// File name:  Win32Mouse.h
//
// Created:    Mark Wilczynski, Jan 2002
//
// Desc:       Interface for the mouse using W3D methods to display cursor.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DMOUSE_H_
#define __W3DMOUSE_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Win32Device/GameClient/Win32Mouse.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////
class CameraClass;
class SurfaceClass;

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// W3DMouse -----------------------------------------------------------------
/** Mouse interface for when using only the Win32 messages and W3D for cursor */
//-----------------------------------------------------------------------------
class W3DMouse : public Win32Mouse
{

public:

	W3DMouse( void );
	virtual ~W3DMouse( void );

	virtual void init( void );		///< init mouse, extend this functionality, do not replace
	virtual void reset( void );		///< reset the system

	virtual void setCursor( MouseCursor cursor );		///< set mouse cursor
	virtual void draw( void );		///< draw the cursor or refresh the image
	virtual void setRedrawMode(RedrawMode mode);	///<set cursor drawing method.

private:
	MouseCursor m_currentD3DCursor;	///< keep track of last cursor image sent to D3D.
	SurfaceClass *m_currentD3DSurface[MAX_2D_CURSOR_ANIM_FRAMES];
	ICoord2D m_currentHotSpot;
	Int	m_currentFrames;	///< total number of frames in current 2D cursor animation.
	Real m_currentAnimFrame;///< current frame of 2D cursor animation.
	Int m_currentD3DFrame;	///< current frame actually sent to the hardware.
	Int m_lastAnimTime;		///< ms at last animation update.
	Real m_currentFMS;		///< frames per ms.
	Bool m_drawing;			///< flag to indicate mouse cursor is currently in the act of drawing.
///@todo: remove the textures if we only need surfaces
	void initD3DAssets(void);		///< load textures for mouse cursors, etc.
	void freeD3DAssets(void);		///< unload textures used by mouse cursors.
	Bool loadD3DCursorTextures(MouseCursor cursor);	///<load the textures/animation for given cursor.
	Bool releaseD3DCursorTextures(MouseCursor cursor);	///<release loaded textures for cursor.

	// W3D animated model cursor
	CameraClass *m_camera;								///< our camera
	MouseCursor m_currentW3DCursor;
	void initW3DAssets(void);		///< load models for mouse cursors, etc.
	void freeW3DAssets(void);		///< unload models used by mouse cursors.

	MouseCursor m_currentPolygonCursor;
	void initPolygonAssets(void);		///< load images for cursor polygon.
	void freePolygonAssets(void);		///< free images for cursor polygon.

	void setCursorDirection(MouseCursor cursor);	///figure out direction for oriented 2D cursors.

};

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __W3DMOUSE_H_
