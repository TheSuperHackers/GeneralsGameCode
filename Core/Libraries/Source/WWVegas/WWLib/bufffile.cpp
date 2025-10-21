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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/bufffile.cpp                           $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/13/01 7:15p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include	"always.h"
#include	"bufffile.h"
#include	"wwdebug.h"

int		BufferedFileClass::_DesiredBufferSize	=	1024*16;

/***********************************************************************************************
 * BufferedFileClass::BufferedFileClass -- Default constructor for a file object.              *
 *=============================================================================================*/
BufferedFileClass::BufferedFileClass(void) :
	RawFileClass(),
	Buffer( NULL ),
	BufferSize( 0 ),
	BufferAvailable( 0 ),
	BufferOffset( 0 )
{
}

/***********************************************************************************************
 * BufferedFileClass::BufferedFileClass -- Simple constructor for a file object.                         *
 *=============================================================================================*/
BufferedFileClass::BufferedFileClass(char const * filename) :
	RawFileClass( filename ),
	Buffer( NULL ),
	BufferSize( 0 ),
	BufferAvailable( 0 ),
	BufferOffset( 0 )
{
}

/***********************************************************************************************
 * BufferedFileClass::~BufferedFileClass -- Default deconstructor for a file object.                     *
 *=============================================================================================*/
BufferedFileClass::~BufferedFileClass(void)
{
	Reset_Buffer();
}

/***********************************************************************************************
 * BufferedFileClass::Close -- Perform a closure of the file.                                       *
 *=============================================================================================*/
void BufferedFileClass::Close(void)
{
	BASECLASS::Close();

	Reset_Buffer();
}


/***********************************************************************************************
 * BufferedFileClass::Read -- Reads the specified number of bytes into a memory buffer.             *
 *                                                                                             *
 *    This routine will read the specified number of bytes and place the data into the buffer  *
 *    indicated. It is legal to call this routine with a request for more bytes than are in    *
 *    the file. This condition can result in fewer bytes being read than requested. Determine  *
 *    this by examining the return value.                                                      *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the buffer to read data into. If NULL is passed, no read    *
 *                      is performed.                                                          *
 *                                                                                             *
 *          size     -- The number of bytes to read. If NULL is passed, then no read is        *
 *                      performed.                                                             *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes read into the buffer. If this number is less      *
 *          than requested, it indicates that the file has been exhausted.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int BufferedFileClass::Read(void * buffer, int size)
{
	int read = 0;

	// If there is anything in the buffer, copy it in.
	if ( BufferAvailable > 0 ) {
		int amount = min( size, BufferAvailable );
		::memcpy( buffer, &Buffer[BufferOffset], amount );
		BufferAvailable -= amount;
		BufferOffset += amount;
		size -= amount;
		buffer = (char *)buffer + amount;
		read += amount;
	}

	if ( size == 0 ) {
		return read;
	}

	// We need to get a copy of the _DesiredBufferSize into
	// a local variable to protect us from modifications
	// from another thread. Otherwise, we could pass the test
	// (size > amount) below, only to allocate a buffer that's
	// too small in the next block. (DRM, 04/20/01)
	int desired_buffer_size = _DesiredBufferSize;

	// If we need more than the buffer will hold, just read it
	int amount = BufferSize;
	if ( amount == 0 ) {
		amount = desired_buffer_size;
	}
	if ( size > amount ) {
		return BASECLASS::Read( buffer, size ) + read;
	}

	// If we dont have a buffer, get one
	if ( BufferSize == 0 ) {
		BufferSize = desired_buffer_size;
		Buffer = W3DNEWARRAY unsigned char [BufferSize];
		BufferAvailable = 0;
		BufferOffset = 0;
	}

	// Fill the buffer
	if ( BufferAvailable == 0 ) {
		BufferAvailable = BASECLASS::Read( Buffer, BufferSize );
		BufferOffset = 0;
	}

	// If there is anything in the buffer, copy it in.
	if ( BufferAvailable > 0 ) {
		int amount = min( size, BufferAvailable );
		::memcpy( buffer, &Buffer[BufferOffset], amount );
		BufferAvailable -= amount;
		BufferOffset += amount;
		read += amount;
	}

	return read;
}


/***********************************************************************************************
 * BufferedFileClass::Write -- Writes the specified data to the buffer specified.                   *
 *                                                                                             *
 *    This routine will write the data specified to the file.                                  *
 *                                                                                             *
 * INPUT:   buffer   -- The buffer that holds the data to write.                               *
 *                                                                                             *
 *          size     -- The number of bytes to write to the file.                              *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes written to the file. This routine catches the     *
 *          case of a disk full condition, so this routine will always return with the number  *
 *          matching the size request.                                                         *
 *                                                                                             *
 * WARNINGS:   A fatal file condition could cause this routine to never return.                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int BufferedFileClass::Write(void const * buffer, int size)
{
	if ( BufferSize != 0 ) {
		WWASSERT( 0 );
	}

	return BASECLASS::Write( buffer, size );
}


/***********************************************************************************************
 * BufferedFileClass::Seek -- Reposition the file pointer as indicated.                             *
 *                                                                                             *
 *    Use this routine to move the filepointer to the position indicated. It can move either   *
 *    relative to current position or absolute from the beginning or ending of the file. This  *
 *    routine will only return if it successfully performed the seek.                          *
 *                                                                                             *
 * INPUT:   pos   -- The position to seek to. This is interpreted as relative to the position  *
 *                   indicated by the "dir" parameter.                                         *
 *                                                                                             *
 *          dir   -- The relative position to relate the seek to. This can be either SEEK_SET  *
 *                   for the beginning of the file, SEEK_CUR for the current position, or      *
 *                   SEEK_END for the end of the file.                                         *
 *                                                                                             *
 * OUTPUT:  This routine returns the position that the seek ended up at.                       *
 *                                                                                             *
 * WARNINGS:   If there was a file error, then this routine might never return.                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int BufferedFileClass::Seek(int pos, int dir)
{
	if ( (dir != SEEK_CUR) || (pos < 0) ) {
		Reset_Buffer();
	}

	// If not buffered, pass through
	if ( BufferAvailable == 0 ) {
		return BASECLASS::Seek( pos, dir );
	}

	// use up what we can of the buffer
	int amount = min( pos, BufferAvailable );
	pos -= amount;
	BufferAvailable -= amount;
	BufferOffset += amount;

	return BASECLASS::Seek( pos, dir ) - BufferAvailable;
}

/*
**
*/
void	BufferedFileClass::Reset_Buffer( void )
{
	delete [] Buffer;
	Buffer = NULL;
	BufferSize = 0;
	BufferAvailable = 0;
	BufferOffset = 0;
}
