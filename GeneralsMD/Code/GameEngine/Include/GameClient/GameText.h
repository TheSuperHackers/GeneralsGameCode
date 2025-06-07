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


//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					                  
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:    RTS 3
//
// File name:  GameClient/GameText.h
//
// Created:    11/07/01
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __GAMECLIENT_GAMETEXT_H_
#define __GAMECLIENT_GAMETEXT_H_


//----------------------------------------------------------------------------
//           Includes                                                      
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

class AsciiString;
class UnicodeString;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------
typedef std::vector<AsciiString> AsciiStringVec;

//===============================
// GameTextInterface 
//===============================
/** Game text interface object for localised text.
	*/
//===============================

class GameTextInterface : public SubsystemInterface
{

	public:

		virtual ~GameTextInterface() {};

		virtual UnicodeString fetch( const Char *label, Bool *exists = NULL ) = 0;		///< Returns the associated labeled unicode text
		virtual UnicodeString fetch( AsciiString label, Bool *exists = NULL ) = 0;		///< Returns the associated labeled unicode text

		// Do not call this directly, but use the FETCH_OR_SUBSTITUTE macro
		virtual UnicodeString fetchOrSubstitute( const Char *label, const WideChar *substituteText ) = 0;

		// This function is not performance tuned.. Its really only for Worldbuilder. jkmcd
		virtual AsciiStringVec& getStringsWithLabelPrefix(AsciiString label) = 0;

		virtual void					initMapStringFile( const AsciiString& filename ) = 0;
};


extern GameTextInterface *TheGameText;
extern GameTextInterface* CreateGameTextInterface( void );

//----------------------------------------------------------------------------
//           Inlining                                                       
//----------------------------------------------------------------------------

// TheSuperHackers @info This is meant to be used like:
// TheGameText->FETCH_OR_SUBSTITUTE("GUI:LabelName", L"Substitute Fallback Text")
// The substitute text will be compiled out if ENABLE_GAMETEXT_SUBSTITUTES is not defined.
#if ENABLE_GAMETEXT_SUBSTITUTES
#define FETCH_OR_SUBSTITUTE(labelA, substituteTextW) fetchOrSubstitute(labelA, substituteTextW)
#else
#define FETCH_OR_SUBSTITUTE(labelA, substituteTextW) fetch(labelA)
#endif


#endif // __GAMECLIENT_GAMETEXT_H_
