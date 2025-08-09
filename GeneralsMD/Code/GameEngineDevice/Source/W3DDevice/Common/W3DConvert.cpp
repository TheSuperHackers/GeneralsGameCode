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

//=============================================================================
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//=============================================================================
//
// Project:		RTS3
//
// File name: W3DConvert.cpp
//
// Created:   Colin Day, April 2001
//
//=============================================================================

//=============================================================================
//         Includes
//=============================================================================
#include "W3DDevice/Common/W3DConvert.h"

//=============================================================================
//         Externals
//=============================================================================

//=============================================================================
//         Defines
//=============================================================================

//=============================================================================
//         Private Types
//=============================================================================

//=============================================================================
//         Private Data
//=============================================================================

//=============================================================================
//         Public Data
//=============================================================================

//=============================================================================
//         Private Prototypes
//=============================================================================

//=============================================================================
//         Private Functions
//=============================================================================

//=============================================================================
//         Public Functions
//=============================================================================

//=============================================================================
// W3DLogicalScreenToPixelScreen
//=============================================================================
/** Translate a W3D logical pixel coord with (-1,-1) at the lower left and
  * (1,1) at the upper right to a pixel screen coord with (0,0) at the
	* upper left */
//=============================================================================
void W3DLogicalScreenToPixelScreen( Real logX, Real logY,
																		Int *screenX, Int *screenY,
																		Int screenWidth, Int screenHeight )
{
	*screenX = REAL_TO_INT((screenWidth  * (logX + 1.0f)) / 2.0f);
	*screenY = REAL_TO_INT((screenHeight * (-logY + 1.0f)) / 2.0f);

}  // end W3DLogicalScreenToPixelScreen

//=============================================================================
// PixelScreenToW3DLogicalScreen
//=============================================================================
/** Translate a pixel coord with (0,0) at the upper left to the W3D logical
  * coord system used with (-1,-1) in the lower left corner and (1,1) the
	* upper right corner */
//=============================================================================
void PixelScreenToW3DLogicalScreen( Int screenX, Int screenY,
																		Real *logX, Real *logY,
																		Int screenWidth, Int screenHeight )
{

	*logX = ((2.0f * screenX) / (Real)screenWidth) - 1.0f;
	*logY = -(((2.0f * screenY) / (Real)screenHeight) - 1.0f);

}  // end PixelScreenToW3DLogicalScreen
