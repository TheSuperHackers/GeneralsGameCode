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
// File name:  W3DMouse.h
//
// Created:    Mark Wilczynski, Jan 2002
//
// Desc:       Interface for the mouse using W3D methods to display cursor.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "SDL3Device/GameClient/SDL3Mouse.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////
class W3DCamera;
import Graphics.Cursors.Cursor;

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// W3DMouse -----------------------------------------------------------------
/** Mouse interface for rendering the cursor through W3D. */
//-----------------------------------------------------------------------------
class W3DMouse : public SDL3Mouse
{

public:

	W3DMouse();
	virtual ~W3DMouse() override;

	virtual void init() override;		///< init mouse, extend this functionality, do not replace
	virtual void reset() override;		///< reset the system

	virtual void setCursor( MouseCursor cursor ) override;		///< set mouse cursor
	virtual void draw() override;		///< draw the cursor or refresh the image
	virtual void setRedrawMode(RedrawMode mode) override;	///<set cursor drawing method.

private:
	MouseCursor m_currentHardwareCursor;
	Graphics::Cursor m_hardwareCursor;
	ICoord2D m_currentHotSpot;
	Int m_directionFrame;
	Bool loadHardwareCursor(MouseCursor cursor);
	void applyCursor() override;

	// W3D animated model cursor
	W3DCamera *m_camera;								///< our camera
	MouseCursor m_currentW3DCursor;
	void initW3DAssets();		///< load models for mouse cursors, etc.
	void freeW3DAssets();		///< unload models used by mouse cursors.

	MouseCursor m_currentPolygonCursor;
	void initPolygonAssets();		///< load images for cursor polygon.
	void freePolygonAssets();		///< free images for cursor polygon.

	void setCursorDirection(MouseCursor cursor);	///figure out direction for oriented 2D cursors.

};

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////
