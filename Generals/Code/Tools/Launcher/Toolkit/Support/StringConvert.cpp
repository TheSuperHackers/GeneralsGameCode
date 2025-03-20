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

/******************************************************************************
*
* FILE
*     $Archive:  $
*
* DESCRIPTION
*     Perform ANSI <-> Unicode string conversions
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author:  $
*
* VERSION INFO
*     $Modtime:  $
*     $Revision:  $
*
******************************************************************************/

#include "StringConvert.h"
#include "UString.h"
#include <windows.h>
#include <Debug/DebugPrint.h>
#include <assert.h>

/******************************************************************************
*
* NAME
*     UStringToANSI
*
* DESCRIPTION
*     Convert UString to an ANSI string
*
* INPUTS
*     String - String to convert
*     Buffer - Pointer to buffer to receive conversion.
*     BufferLength - Length of buffer
*
* RESULT
*     ANSI - Pointer to ANSI string
*
******************************************************************************/

Char* UStringToANSI(const UString& string, Char* buffer, UInt bufferLength)
	{
	return UnicodeToANSI(string.Get(), buffer, bufferLength);
	}


/******************************************************************************
*
* NAME
*     UnicodeToANSI
*
* DESCRIPTION
*     Convert Unicode string to an ANSI string
*
* INPUTS
*     String - Unicode string to convert
*     Buffer - Pointer to buffer to receive conversion.
*     BufferLength - Length of buffer
*
* RESULT
*     ANSI - Pointer to ANSI string
*
******************************************************************************/

Char* UnicodeToANSI(const WChar* string, Char* buffer, UInt bufferLength)
	{
	if ((string == NULL) || (buffer == NULL))
		{
		return NULL;
		}

	#ifdef _DEBUG
	int result = 
	#endif
		WideCharToMultiByte(CP_ACP, 0, string, -1, buffer, bufferLength,
			NULL, NULL);

	#ifdef _DEBUG
	if (result == 0)
		{
		PrintWin32Error("ConvertToANSI() Failed");
		assert(false);
		}
	#endif

	return buffer;
	}
