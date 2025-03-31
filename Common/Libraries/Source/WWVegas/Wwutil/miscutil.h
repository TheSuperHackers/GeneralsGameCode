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

//
// Filename:     miscutil.h
// Project:      wwutil
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:  
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef MISCUTIL_H
#define MISCUTIL_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#include	"bittype.h"
#include	"wwstring.h"

const float MISCUTIL_EPSILON = 0.0001f;

class cMiscUtil
{
	public:
		static LPCSTR	Get_Text_Time(void);
      static void		Seconds_To_Hms(float seconds, int & h, int & m, int & s);
		static bool		Is_String_Same(LPCSTR str1, LPCSTR str2);
		static bool		Is_String_Different(LPCSTR str1, LPCSTR str2);
		static void		Get_File_Id_String(LPCSTR filename, StringClass & str);
      static bool		File_Exists(LPCSTR filename);
		static bool		File_Is_Read_Only(LPCSTR filename);
      static bool		Is_Alphabetic(char c);
      static bool		Is_Numeric(char c);
      static bool		Is_Alphanumeric(char c);
		static bool		Is_Whitespace(char c);
		static void		Trim_Trailing_Whitespace(char * text);
      static void		Remove_File(LPCSTR filename);

	private:
};


#endif // MISCUTIL_H





		//static int		Get_Exe_Key(void);
