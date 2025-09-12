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

// FILE: GUIEditColor.h ///////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    GUIEdit
//
// File name:  GUIEditColor.h
//
// Created:    Colin Day, July 2001
//
// Desc:       Color structures for the editor
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __GUIEDITCOLOR_H_
#define __GUIEDITCOLOR_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////

// FORWARD REFERENCES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// RGBColorInt ----------------------------------------------------------------
/** Integer color representation */
//-----------------------------------------------------------------------------
struct RGBColorInt
{
	Int red;
	Int green;
	Int blue;
	Int alpha;
};

// RGBColorReal ---------------------------------------------------------------
/** Colors using 0.0 to 1.0 reals */
//-----------------------------------------------------------------------------
struct RGBColorReal
{
	Real red;
	Real green;
	Real blue;
	Real alpha;
};

// HSVColorReal ---------------------------------------------------------------
/** Colors using hue, saturation, value using 0.0 to 1.0 reals */
//-----------------------------------------------------------------------------
struct HSVColorReal
{
  Real hue;
  Real saturation;
  Real value;
	Real alpha;
};

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////
extern RGBColorInt *SelectColor( Int red, Int green, Int blue, Int alpha,
																 Int mouseX = 0, Int mouseY = 0 );

#endif // __GUIEDITCOLOR_H_

