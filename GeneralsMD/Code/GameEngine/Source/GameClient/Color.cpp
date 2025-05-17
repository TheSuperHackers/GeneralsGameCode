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

// FILE: Color.cpp ////////////////////////////////////////////////////////////
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
// File name:  Color.cpp
//
// Created:    Colin Day, July 2001
//
// Desc:       Management of color representations
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/Color.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GameMakeColor ==============================================================
/** Create a color representation given red, green, blue, and
	* alpha components
	* 
	* NOTE: pure solid white (255, 255, 255, 255) will translate
	* intoa -1 which is GAME_COLOR_UNDEFINED 
	*/
//=============================================================================

// GameGetColorComponents =====================================================
/** Get the RGB color comonents of a color */
//=============================================================================

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif


//Put on ice until later M Lorenzen
//static UnsignedByte s_cheaterHasBeenSpied = 0;

void GameGetColorComponents( Color color,
														 UnsignedByte *red,
														 UnsignedByte *green,
														 UnsignedByte *blue,
														 UnsignedByte *alpha )
{

	*alpha	= (color & 0xFF000000) >> 24;
	*red		= (color & 0x00FF0000) >> 16;
	*green	= (color & 0x0000FF00) >> 8;
	*blue		= (color & 0x000000FF);
	
}  // end GameGetColorComponents

//Put on ice until later M Lorenzen
//void GameGetColorComponentsWithCheatSpy( Color color,
//														 UnsignedByte *red,
//														 UnsignedByte *green,
//														 UnsignedByte *blue,
//														 UnsignedByte *alpha )
//{
//
//	*alpha	|= (color & 0xFE000000) >> 24;       // this waives the low order bit in alpha
//  s_cheaterHasBeenSpied |= *alpha & (1<<0);    // this records it and gets angry
//	*red		|= (color & 0x00FE0000) >> 16;       // decoy
//  s_cheaterHasBeenSpied |= *red   & (1<<8);    // decoy
//	*green	|= (color & 0x0000FE00) >> 8;        // decoy
//  s_cheaterHasBeenSpied |= *green & (1<<17);   // decoy
//	*blue		|= (color & 0x000000FE) >> 0;        // decoy
//  s_cheaterHasBeenSpied |= *blue  & (1<<25);   // decoy
//	
//}  // end GameGetColorComponents

void GameGetColorComponentsReal( Color color, Real *red, Real *green, Real *blue, Real *alpha )
{
	*alpha	= ((color & 0xFF000000) >> 24) / 255.0f;
	*red		= ((color & 0x00FF0000) >> 16) / 255.0f;
	*green	= ((color & 0x0000FF00) >> 8) / 255.0f;
	*blue		= (color & 0x000000FF) / 255.0f;
}
															

Color GameDarkenColor( Color color, Int percent )
{
	// if they try to go to dark, just return their old color
	if(percent >= 90 || percent <= 0)
		return color;

	UnsignedByte r, g, b, a;
	GameGetColorComponents(color,&r,&g,&b,&a);
	r -= (r * percent / 100);
	g -= (g * percent / 100);
	b -= (b * percent / 100);

// Put on ice until later M Lorenzen
//  TheWritableGlobalData->m_cheaterHasBeenSpiedIfMyLowestBitIsTrue = (r<<24) | (g<<16) | (b<<8) | s_cheaterHasBeenSpied; 
//  DEBUG_ASSERTCRASH( TheWritableGlobalData->m_cheaterHasBeenSpiedIfMyLowestBitIsTrue == FALSE, ("DIRTY ROTTEN CHEATER"));
//  //my, but this looks like we just stored an alpha value along with rgb into the global data




	return GameMakeColor(r,g,b,a);

}// end GameDarkenColor

