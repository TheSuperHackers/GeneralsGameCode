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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwlib/bufffile.h                             $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             * 
 *                     $Modtime:: 10/31/01 3:33p                                              $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   RawFileClass::File_Name -- Returns with the filename associate with the file object.      *
 *   RawFileClass::RawFileClass -- Default constructor for a file object.                      *
 *   RawFileClass::~RawFileClass -- Default deconstructor for a file object.                   *
 *   RawFileClass::Is_Open -- Checks to see if the file is open or not.                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef BUFFFILE_H
#define BUFFFILE_H

#include	"RAWFILE.H"


/*
**	This is the definition of a buffered read raw file class. 
*/
class BufferedFileClass : public RawFileClass
{
	typedef RawFileClass BASECLASS;

	public:

		BufferedFileClass(char const *filename);
		BufferedFileClass(void);
		BufferedFileClass (RawFileClass const & f);
		BufferedFileClass & operator = (BufferedFileClass const & f);
		virtual ~BufferedFileClass(void);

		virtual int Read(void *buffer, int size);
		virtual int Seek(int pos, int dir=SEEK_CUR);
		virtual int Write(void const *buffer, int size);
		virtual void Close(void);

	protected:

		static	void		Set_Desired_Buffer_Size( int size ) { _DesiredBufferSize = size; }

		void					Reset_Buffer( void );
		
	private:
		unsigned char *	Buffer;				// The read buffer 
		unsigned int		BufferSize;			// The allocated size of the read buffer
		int					BufferAvailable;	// The amount of data in the read buffer
		int					BufferOffset;		// The data already given out
		static	int		_DesiredBufferSize;
};

#endif
