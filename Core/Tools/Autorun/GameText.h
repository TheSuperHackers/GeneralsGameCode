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

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// GameTextInterface
//===============================
/** Game text interface object for localised text.
	*/
//===============================

class GameTextInterface
{

	public:

		virtual ~GameTextInterface() {};

		virtual void					init( void ) = 0;						///< Initlaizes the text system
		virtual const wchar_t * fetch( const char *label ) = 0;		///< Returns the associated labeled unicode text

};


extern GameTextInterface *TheGameText;
extern GameTextInterface* CreateGameTextInterface( void );

//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------


#endif // __GAMECLIENT_GAMETEXT_H_
