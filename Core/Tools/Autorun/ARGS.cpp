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

//======================================================================================
//
// @@@@@  @@   @  @@@@   @@@@   @@@@  @@   @  @@@@   @@@@  @@@@@    @@@@@   @@@@  @@   @
// @@   @ @@   @ @@   @ @@   @ @@  	@ @@@  @ @@	  @ @@	@ @@	 @	  @@   @ @@   @  @@ @
// @@@@@  @@   @ @@	   @@	    @@@@@@ @@ @ @ @@@@   @@@@   @@@@@	  @@@@@  @@@@@@   @@
// @@   @ @@   @ @@   @ @@   @ @@ 	@ @@  @@ @@	  @ @@	@ @@  @	  @@   @ @@   @   @@
// @@@@@   @@@@	@@@@   @@@@  @@ 	@ @@   @  @@@@   @@@@  @@   @	  @@@@@  @@   @   @@
//
// Copyright (c) 1998, 1999 Westwood Studios -- CONFIDENTIAL
//
// ArgC_ArgV.cpp
//
//======================================================================================

//-----------------------------------------------------------------------------
// INCLUDES
//-----------------------------------------------------------------------------
#define  STRICT
#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ARGS.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------

Command_Line_Arguments *Args = NULL;

//*****************************************************************************
// COMMAND_LINE_ARGUMENTS::COMMAND_LINE_ARGUMENTS -- Constructor.
//
// INPUT:     	HINSTANCE hInstance
//		  		LPSTR lpszCmdLine
//
// OUTPUT:    	none.
//
// WARNINGS:  	none.
//
// HISTORY:
//   09/01/1997  ML/MG : Created.
//=============================================================================

Command_Line_Arguments::Command_Line_Arguments (
	HINSTANCE current_instance_handle,
	LPTSTR windows_command_line_string )
{
	//--------------------------------------------------------------------------
	// debug checks...
	//--------------------------------------------------------------------------
	assert( windows_command_line_string != NULL );

	//--------------------------------------------------------------------------
	// reset all class data
	//--------------------------------------------------------------------------
	memset( ArgV, 0, sizeof( ArgV ) );

	//--------------------------------------------------------------------------
	// Store the program name in ArgV[ 0 ].
	//--------------------------------------------------------------------------
	GetModuleFileName( current_instance_handle, ArgV[ 0 ], MAX_ARGUMENT_LENGTH	);

	const char * ptr = windows_command_line_string;
	bool potentially_forever = true;

	ArgC = 1;

	while( potentially_forever ) {

		//-----------------------------------------------------------------------
		// Find the next non-whitespace character in the string.
		//-----------------------------------------------------------------------
		while ( *ptr == ' ' ) {
			++ ptr;
		}

		//-----------------------------------------------------------------------
		// If we're at the end of the string, quit.
		//-----------------------------------------------------------------------
		if ( *ptr == '\0' ) {
			break;
		}

		//-----------------------------------------------------------------------
		// Store a command-line argument.
		//-----------------------------------------------------------------------
		int i = 0;

		if ( *ptr == '"' ) {

			//--------------------------------------------------------------------
			// Skip the opening quotation mark.
			//--------------------------------------------------------------------
			++ ptr;

			//--------------------------------------------------------------------
			// Collect characters until another quotation mark is encountered.
			//--------------------------------------------------------------------
			while ( *ptr != '\0' && *ptr != '"' ) {
				if ( i < MAX_ARGUMENT_LENGTH - 1 ) {
					ArgV [ ArgC ][ i ] = *ptr;
					++ i;
				}
				++ ptr;
			}

			//--------------------------------------------------------------------
			// Skip the closing quotation mark.
			//--------------------------------------------------------------------
			if ( *ptr == '"' ) {
				++ ptr;
			}

		} else {

			//--------------------------------------------------------------------
			// Collect characters until a whitespace character is encountered.
			//--------------------------------------------------------------------
			while ( *ptr != '\0' && *ptr != ' ' ) {
				if ( i < MAX_ARGUMENT_LENGTH - 1 ) {
					ArgV [ ArgC ][ i ] = *ptr;
					++ i;
				}
				++ ptr;
			}
		}

		ArgV [ ArgC ][ i ] = '\0';
		++ ArgC;
	}
}

//*****************************************************************************
// COMMAND_LINE_ARGUMENTS::COMMAND_LINE_ARGUMENTS -- Constructor.
//
// INPUT:     	HINSTANCE hInstance
//
// OUTPUT:    	none.
//
// WARNINGS:  	none.
//
// HISTORY:
//   09/01/1997  ML/MG : Created.
//=============================================================================

Command_Line_Arguments::Command_Line_Arguments ( HINSTANCE current_instance_handle )
{

	char * windows_command_line_string = GetCommandLine();

	//--------------------------------------------------------------------------
	// debug checks...
	//--------------------------------------------------------------------------
	assert( windows_command_line_string != NULL );

	//--------------------------------------------------------------------------
	// reset all class data
	//--------------------------------------------------------------------------
	memset( ArgV, 0, sizeof( ArgV ) );

	const char * ptr = windows_command_line_string;
	bool potentially_forever = true;

	ArgC = 1;

	while( potentially_forever ) {

		//-----------------------------------------------------------------------
		// Find the next non-whitespace character in the string.
		//-----------------------------------------------------------------------
		while ( *ptr == ' ' ) {
			++ ptr;
		}

		//-----------------------------------------------------------------------
		// If we're at the end of the string, quit.
		//-----------------------------------------------------------------------
		if ( *ptr == '\0' ) {
			break;
		}

		//-----------------------------------------------------------------------
		// Store a command-line argument.
		//-----------------------------------------------------------------------
		int i = 0;

		if ( *ptr == '"' ) {

			//--------------------------------------------------------------------
			// Skip the opening quotation mark.
			//--------------------------------------------------------------------
			++ ptr;

			//--------------------------------------------------------------------
			// Collect characters until another quotation mark is encountered.
			//--------------------------------------------------------------------
			while ( *ptr != '\0' && *ptr != '"' ) {
				if ( i < MAX_ARGUMENT_LENGTH - 1 ) {
					ArgV [ ArgC ][ i ] = *ptr;
					++ i;
				}
				++ ptr;
			}

			//--------------------------------------------------------------------
			// Skip the closing quotation mark.
			//--------------------------------------------------------------------
			if ( *ptr == '"' ) {
				++ ptr;
			}

		} else {

			//--------------------------------------------------------------------
			// Collect characters until a whitespace character is encountered.
			//--------------------------------------------------------------------
			while ( *ptr != '\0' && *ptr != ' ' ) {
				if ( i < MAX_ARGUMENT_LENGTH - 1 ) {
					ArgV [ ArgC ][ i ] = *ptr;
					++ i;
				}
				++ ptr;
			}
		}

		ArgV [ ArgC ][ i ] = '\0';
		++ ArgC;
	}
}

//*****************************************************************************
// COMMAND_LINE_ARGUMENTS::~COMMAND_LINE_ARGUMENTS -- Destructor.
//
// INPUT:     	HINSTANCE hInstance
//			  		LPSTR lpszCmdLine
//
// OUTPUT:    	none.
//
// WARNINGS:  	none.
//
// HISTORY:
//   09/01/1997  ML/MG : Created.
//=============================================================================

Command_Line_Arguments::~Command_Line_Arguments ( void )
{
	//--------------------------------------------------------------------------
	// reset all data...
	//--------------------------------------------------------------------------
	ArgC = -1;
	memset( ArgV, 0, sizeof( ArgV ) );
}

//*****************************************************************************
// COMMAND_LINE_ARGUMENTS::GET_ARGC --	Return ArgC.
//
// INPUT:     	none.
//
// OUTPUT:    	int ArgC.
//
// WARNINGS:  	none.
//
// HISTORY:
//   09/01/1997  ML/MG : Created.
//=============================================================================

int Command_Line_Arguments::Get_argc ( void )
{
	//--------------------------------------------------------------------------
	// debug checks - make sure we at least have the application name
	//--------------------------------------------------------------------------
	assert( ArgC >= 1 );

	//--------------------------------------------------------------------------
	// return how many string parameters there are in the "argv" list
	//--------------------------------------------------------------------------
	return( ArgC );
}

//*****************************************************************************
// COMMAND_LINE_ARGUMENTS::GET_ARGV --	Return ArgV.
//
// INPUT:     	none.
//
// OUTPUT:    	int ArgV.
//
// WARNINGS:  	none.
//
// HISTORY:
//   09/01/1997  ML/MG : Created.
//=============================================================================

const char *Command_Line_Arguments::Get_argv ( int argument_index )
{
	//--------------------------------------------------------------------------
	// debug checks - make sure we at least have the application name
	//--------------------------------------------------------------------------
	assert( argument_index >= 0 );
	assert( argument_index < MAX_COMMAND_LINE_ARGUMENTS );
	assert( argument_index < ArgC );
	assert( ArgC >= 1 );

	//--------------------------------------------------------------------------
	// return
	//--------------------------------------------------------------------------
	return( ArgV[ argument_index ] );
}

void Command_Line_Arguments::Set_argv( int argument_index, char *arg )
{
	if( arg == NULL || *arg == '\0' ) {
		return;
	}

	//--------------------------------------------------------------------------
	// debug checks - make sure we at least have the application name
	//--------------------------------------------------------------------------
	assert( argument_index >= 0 );
	assert( argument_index < MAX_COMMAND_LINE_ARGUMENTS );
	assert( argument_index < ArgC );
	assert( ArgC >= 1 );

	if (( argument_index >= 0 ) &&
		( argument_index < MAX_COMMAND_LINE_ARGUMENTS ) &&
		( argument_index < ArgC )) {

		strcpy( ArgV[ argument_index ], arg );
	}
}


