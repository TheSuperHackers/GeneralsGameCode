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
 *                     $Archive:: /G/wwlib/XSTRAW.H                                           $* 
 *                                                                                             * 
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 4/02/99 12:01p                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef XSTRAW_H
#define XSTRAW_H

#include	"BUFF.H"
#include	"STRAW.H"
#include	"WWFILE.H"
#include	<stddef.h>

/*
**	This class is used to manage a buffer as a data source. Data requests will draw from the
**	buffer supplied until the buffer is exhausted.
*/
class BufferStraw : public Straw
{
	public:
		BufferStraw(Buffer const & buffer) : BufferPtr(buffer), Index(0) {}
		BufferStraw(void const * buffer, int length) : BufferPtr((void*)buffer, length), Index(0) {}
		virtual int Get(void * source, int slen);

	private:
		Buffer BufferPtr;
		int Index;
//		void const * BufferPtr;
//		int Length;

		bool Is_Valid(void) {return(BufferPtr.Is_Valid());}
		BufferStraw(BufferStraw & rvalue);
		BufferStraw & operator = (BufferStraw const & pipe);
};

/*
**	This class is used to manage a file as a data source. Data requests will draw from the
**	file until the file has been completely read.
*/
class FileStraw : public Straw
{
	public:
		FileStraw(FileClass * file) : File(file), HasOpened(false) {}
		FileStraw(FileClass & file) : File(&file), HasOpened(false) {}
		virtual ~FileStraw(void);
		virtual int Get(void * source, int slen);

	private:
		FileClass * File;
		bool HasOpened;

		bool Valid_File(void) {return(File != NULL);}
		FileStraw(FileStraw & rvalue);
		FileStraw & operator = (FileStraw const & pipe);
};


#endif
