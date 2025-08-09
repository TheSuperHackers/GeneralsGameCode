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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /G/wwlib/wwfile.cpp                                         $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/19/99 2:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <Utility/stdio_adapter.h>
#include <stdarg.h>
#include <memory.h>
#include "WWFILE.H"

#pragma warning(disable : 4514)

int FileClass::Printf(char *str, ...)
{
	char text[PRINTF_BUFFER_SIZE];
	va_list args;
	va_start(args, str);
	int length = vsnprintf(text, PRINTF_BUFFER_SIZE, str, args);
	va_end(args);
	return Write(text, length);
}

int FileClass::Printf(char *buffer, int bufferSize, char *str, ...)
{
	va_list args;
	va_start(args, str);
	int length = vsnprintf(buffer, bufferSize, str, args);
	va_end(args);
	return Write(buffer, length);
}

int FileClass::Printf_Indented(unsigned depth, char *str, ...)
{
	char text[PRINTF_BUFFER_SIZE];
	va_list args;
	va_start(args, str);

	if(depth > PRINTF_BUFFER_SIZE)
		depth = PRINTF_BUFFER_SIZE;

	memset(text, '\t', depth);

	int length;
	if(depth < PRINTF_BUFFER_SIZE)
		length = vsnprintf(text + depth, PRINTF_BUFFER_SIZE - depth, str, args);
	else
		length = PRINTF_BUFFER_SIZE;

	va_end(args);

	return Write(text, length + depth);
}

