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
 *                     $Archive:: /Commando/Library/BFIOFILE.H                                $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef BFIOFILE_H
#define BFIOFILE_H

#include	"RAWFILE.H"

/*
**	This derivation of the raw file class handles buffering the input/output in order to
**	achieve greater speed. The buffering is not active by default. It must be activated
**	by setting the appropriate buffer through the Cache() function.
*/
class BufferIOFileClass : public RawFileClass
{
		typedef RawFileClass BASECLASS;

	public:

		BufferIOFileClass(char const * filename);
		BufferIOFileClass(void);
		virtual ~BufferIOFileClass(void);

		bool Cache( long size=0, void * ptr=NULL);
		void Free( void);
		bool Commit( void);
		virtual char const * Set_Name(char const * filename);
		virtual bool Is_Available(int forced=false);
		virtual bool Is_Open(void) const;
		virtual int Open(char const * filename, int rights=READ);
		virtual int Open(int rights=READ);
		virtual int Read(void * buffer, int size);
		virtual int Seek(int pos, int dir=SEEK_CUR);
		virtual int Size(void);
		virtual int Write(void const * buffer, int size);
		virtual void Close(void);

		enum {MINIMUM_BUFFER_SIZE=1024};

	private:

		bool IsAllocated;
		bool IsOpen;
		bool IsDiskOpen;
		bool IsCached;
		bool IsChanged;
		bool UseBuffer;

		int BufferRights;

		void *Buffer;

		long BufferSize;
		long BufferPos;
		long BufferFilePos;
		long BufferChangeBeg;
		long BufferChangeEnd;
		long FileSize;
		long FilePos;
		long TrueFileStart;
};

#endif
