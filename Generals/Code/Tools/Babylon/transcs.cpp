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

//
// transcs.cpp
//

#include "StdAfx.h"
#include <windows.h>
#include <winnls.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

void CreateTranslationTable ( void )
{
	int i; 
	FILE *out;
	unsigned short wc;
	unsigned short mb;
	DWORD last_error;

	if ( ! ( out = fopen ( "utable.c", "wt" )))
	{
		return;
	}


	fprintf (out, "static unsigned short Utable[0x10000] =\n{" );

	for ( i = 0; i < 0x10000; i++ )
	{

		if ( ( i %32 ) == 0 )
		{
			fprintf ( out, "\n\t/* 0x%04x */\t", i );
		}

		mb = i;
		if ( MultiByteToWideChar (CP_ACP, MB_ERR_INVALID_CHARS, (LPCSTR) &mb, 2, &wc, 2 ) == 0 )
		{
			wc = 0;
			last_error = GetLastError ( );
		}

		fprintf (out, "0x%04x", wc );
		if ( i != 0xffff )
		{
			fprintf (out, "," );
		}
	}

	fprintf ( out, "\n};\n");

	fclose ( out );

}
