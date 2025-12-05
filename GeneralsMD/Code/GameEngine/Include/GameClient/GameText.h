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

		virtual UnicodeString fetch( const Char *label, Bool *exists = nullptr ) = 0;		///< Returns the associated labeled unicode text
		virtual UnicodeString fetch( AsciiString label, Bool *exists = nullptr ) = 0;		///< Returns the associated labeled unicode text ; TheSuperHackers @todo Remove
		virtual UnicodeString fetchFormat( const Char *label, ... ) = 0;

		// Do not call this directly, but use the FETCH_OR_SUBSTITUTE macro
		virtual UnicodeString fetchOrSubstitute( const Char *label, const WideChar *substituteText ) = 0;
		virtual UnicodeString fetchOrSubstituteFormat( const Char *label, const WideChar *substituteFormat, ... ) = 0;
		virtual UnicodeString fetchOrSubstituteFormatVA( const Char *label, const WideChar *substituteFormat, va_list args ) = 0;

		// This function is not performance tuned.. Its really only for Worldbuilder. jkmcd
		virtual AsciiStringVec& getStringsWithLabelPrefix(AsciiString label) = 0;

		virtual void					initMapStringFile( const AsciiString& filename ) = 0;

#if __cplusplus < 201103L // TheSuperHackers @todo Remove function when abandoning VC6
		inline UnicodeString FETCH_OR_SUBSTITUTE_FORMAT( const Char *label, const WideChar *substituteFormat, ... )
		{
			va_list args;
			va_start(args, substituteFormat);
			UnicodeString str = fetchOrSubstituteFormatVA(label, substituteFormat, args);
			va_end(args);
			return str;
		}
#endif
};


extern GameTextInterface *TheGameText;
extern GameTextInterface* CreateGameTextInterface( void );

//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------

// TheSuperHackers @info This is meant to be used like:
// TheGameText->FETCH_OR_SUBSTITUTE("GUI:LabelName", L"Substitute Fallback Text")
// TheGameText->FETCH_OR_SUBSTITUTE_FORMAT("GUI:LabelName", L"Substitute Fallback Text %d %d", 1, 2)
// The substitute text will be compiled out if ENABLE_GAMETEXT_SUBSTITUTES is not defined.
#if ENABLE_GAMETEXT_SUBSTITUTES

#define FETCH_OR_SUBSTITUTE(labelA, substituteTextW) fetchOrSubstitute(labelA, substituteTextW)
#if __cplusplus >= 201103L // TheSuperHackers @todo Remove condition when abandoning VC6
#define FETCH_OR_SUBSTITUTE_FORMAT(labelA, substituteFormatW, ...) fetchOrSubstituteFormat(labelA, substituteFormatW, __VA_ARGS__)
#endif

#else

#define FETCH_OR_SUBSTITUTE(labelA, substituteTextW) fetch(labelA)
#if __cplusplus >= 201103L // TheSuperHackers @todo Remove condition when abandoning VC6
#define FETCH_OR_SUBSTITUTE_FORMAT(labelA, substituteTextW, ...) fetchFormat(labelA, __VA_ARGS__)
#endif

#endif // ENABLE_GAMETEXT_SUBSTITUTES
