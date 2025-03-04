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

#ifndef CHATAPI_HEADER
#define CHATAPI_HEADER

#include "cominit.h"
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <winerror.h>
#include <ocidl.h>
#include <olectl.h>

/**********************************************************************
**	This macro serves as a general way to determine the number of elements
**	within an array.
*/
#define	ARRAY_SIZE(x)		int(sizeof(x)/sizeof(x[0]))
#define size_of(typ,id) sizeof(((typ*)0)->id)

namespace patchget
{

int main(int argc, char *argv[]);

void Startup_Chat(void);
void Shutdown_Chat(void);
void Update_If_Required(void);

char const * Fetch_String(int id);

} // namespace patchget

#endif
