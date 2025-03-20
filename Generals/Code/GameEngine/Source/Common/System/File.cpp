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

//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright(C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:   WSYS Library
//
// Module:    IO_
//
// File name: IO_File.cpp
//
// Created:   4/23/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes                                                      
//----------------------------------------------------------------------------

#include "PreRTS.h"

#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


#include "Common/file.h"


//----------------------------------------------------------------------------
//         Externals                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines                                                         
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data                                                      
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Prototypes                                               
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions                                               
//----------------------------------------------------------------------------

//=================================================================
// File::File
//=================================================================

File::File()
:	m_open(FALSE),
	m_deleteOnClose(FALSE),
	m_access(NONE)
{

	setName("<no file>");

}


//----------------------------------------------------------------------------
//         Public Functions                                                
//----------------------------------------------------------------------------


//=================================================================
// File::~File	
//=================================================================

File::~File()
{
	close();
}

//=================================================================
// File::open	
//=================================================================
/**
  * Any derived open() members must first call File::open. If File::open
	* succeeds but the derived class's open failes then make sure to call
	* File::close() before returning.
	*/
//=================================================================

Bool File::open( const Char *filename, Int access )
{
	if( m_open )
	{
		return FALSE;
	}

	setName( filename );

	if( (access & ( STREAMING | WRITE )) == ( STREAMING | WRITE ))
	{
		// illegal access
		return FALSE;
	}

	if( (access & ( TEXT | BINARY)) == ( TEXT | BINARY ))
	{
		// illegal access
		return FALSE;
	}

	if ( (access & (READ|WRITE)) == 0 )
	{
		access |= READ;
	}

	if ( !(access & (READ|APPEND)) )
	{
		access |= TRUNCATE;
	}

	if ( (access & (TEXT|BINARY)) == 0 )
	{
		access |= BINARY;
	}

	m_access = access;
	m_open = TRUE;
	return TRUE;
}

//=================================================================
// File::close 	
//=================================================================
/**
  * Must call File::close() for each successful File::open() call.
	*/
//=================================================================

void File::close( void )
{
	if( m_open )
	{
		setName( "<no file>" );
		m_open = FALSE;
		if ( m_deleteOnClose )
		{
			this->deleteInstance(); // on special cases File object will delete itself when closing
		}
	}
}

//=================================================================
// File::size 
//=================================================================
/**
  * Default implementation of File::size. Derived classes can optimize
	* this member function.
	*/
//=================================================================

Int File::size( void )
{
	Int pos = seek( 0, CURRENT );
	Int size = seek( 0, END );

	seek( pos, START );

	return size < 0 ? 0 : size;
}

//============================================================================
// File::position
//============================================================================

Int File::position( void )
{
	return seek(0, CURRENT);
}

//============================================================================
// File::print
//============================================================================

Bool	File::print ( const Char *format, ...)
{
	Char buffer[10*1024];
	Int len;

	if ( ! (m_access & TEXT ) )
	{
		return FALSE;
	}

	va_list args;
	va_start( args, format );     /* Initialize variable arguments. */
	len = vsprintf( buffer, format, args );
	va_end( args );

	if ( len >= sizeof(buffer) )
	{
		// Big Problem
		assert( FALSE );
		return FALSE;
	}

	return (write ( buffer, len ) == len);
}

Bool	File::eof() {
	return (position() == size());
}