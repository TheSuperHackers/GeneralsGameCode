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

//================================================================================================
//
// @@@@@  @@   @  @@@@   @@@@   @@@@  @@   @  @@@@   @@@@  @@@@@    @@@@@   @@@@  @@   @
// @@   @ @@   @ @@   @ @@   @ @@  	@ @@@  @ @@	  @ @@	@ @@	 @	  @@   @ @@   @  @@ @
// @@@@@  @@   @ @@	   @@	    @@@@@@ @@ @ @ @@@@   @@@@   @@@@@	  @@@@@  @@@@@@   @@
// @@   @ @@   @ @@   @ @@   @ @@ 	@ @@  @@ @@	  @ @@	@ @@  @	  @@   @ @@   @   @@
// @@@@@   @@@@	@@@@   @@@@  @@ 	@ @@   @  @@@@   @@@@  @@   @	  @@@@@  @@   @   @@
//
// Copyright (c) 1998, 1999 Westwood Studios -- CONFIDENTIAL
//
// ArgC_ArgV.h
//
//================================================================================================

#ifndef ARGS_H
#define ARGS_H

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------
#include <windows.h>
//#include "autorun.h"

//----------------------------------------------------------------------------
// DEFINES
//----------------------------------------------------------------------------
#define MAX_COMMAND_LINE_ARGUMENTS	10
#define MAX_ARGUMENT_LENGTH			80

//----------------------------------------------------------------------------
// CLASS
//----------------------------------------------------------------------------
class Command_Line_Arguments
{
	public:
		Command_Line_Arguments ( HINSTANCE, LPTSTR );
		Command_Line_Arguments ( LPTSTR )    {};
		Command_Line_Arguments ( HINSTANCE );
		~Command_Line_Arguments( void );

		const char *	Get_argv( int argument_index );
		int		 		Get_argc();
		void			Set_argv( int argument_index, char*arg );

	private:
		int		ArgC;
		char	ArgV[ MAX_COMMAND_LINE_ARGUMENTS ][ MAX_ARGUMENT_LENGTH ];
};

extern Command_Line_Arguments *Args;

#endif // ARGC_ARGV_H