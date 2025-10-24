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

// FILE: Language.h ///////////////////////////////////////////////////////////
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
// File name:  Language.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Header for dealing with multiple languages
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __LANGUAGE_H_
#define __LANGUAGE_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////

// FORWARD REFERENCES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// IMPORTANT: Make sure this enum is identical to the one in Noxstring tool
typedef enum
{

	LANGUAGE_ID_US = 0,
	LANGUAGE_ID_UK,
	LANGUAGE_ID_GERMAN,
	LANGUAGE_ID_FRENCH,
	LANGUAGE_ID_SPANISH,
	LANGUAGE_ID_ITALIAN,
	LANGUAGE_ID_JAPANESE,
	LANGUAGE_ID_JABBER,
	LANGUAGE_ID_KOREAN,
	LANGUAGE_ID_UNKNOWN

} LanguageID;

#define GameArrayEnd(array) (array)[(sizeof(array)/sizeof((array)[0]))-1] = 0

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////
extern LanguageID OurLanguage;  ///< our current language definition

#endif // __LANGUAGE_H_

