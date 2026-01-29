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

/* $Header: /Commando/Code/Tools/pluglib/errclass.h 5     6/25/99 10:46a Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/pluglib/errclass.h                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/24/99 3:38p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <stdarg.h>


class ErrorClass
{
public:
	ErrorClass(char * format,...);
	ErrorClass(const ErrorClass & that);
	~ErrorClass(void) { if (error_message != nullptr) free(error_message); }

	ErrorClass & operator = (const ErrorClass & that);

	char * error_message;
};

inline ErrorClass::ErrorClass(char * format,...)
{
	va_list va;
	char tmp[1024];
	va_start(va,format);
	vsprintf(tmp,format,va);
	assert(strlen(tmp) < 1024);
	va_end(va);
	error_message = strdup(tmp);
}

inline ErrorClass::ErrorClass(const ErrorClass & that)	:
	error_message(nullptr)
{
	*this = that;
}

inline ErrorClass & ErrorClass::operator = (const ErrorClass & that)
{
	if (error_message != nullptr) {
		free(error_message);
		error_message = nullptr;
	}

	if (that.error_message != nullptr) {
		error_message = strdup(that.error_message);
	}

	return *this;
}
