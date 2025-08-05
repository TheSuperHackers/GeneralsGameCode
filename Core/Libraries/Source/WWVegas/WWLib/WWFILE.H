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
 *                     $Archive:: /Commando/Code/wwlib/wwfile.h                               $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/31/01 2:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef WWFILE_Hx
#define WWFILE_Hx

#define YEAR(dt)	(((dt & 0xFE000000) >> (9 + 16)) + 1980)
#define MONTH(dt)	 ((dt & 0x01E00000) >> (5 + 16))
#define DAY(dt)	 ((dt & 0x001F0000) >> (0 + 16))
#define HOUR(dt)	 ((dt & 0x0000F800) >> 11)
#define MINUTE(dt) ((dt & 0x000007E0) >> 5)
#define SECOND(dt) ((dt & 0x0000001F) << 1)

#ifndef SEEK_SET
#define SEEK_SET					0	// Seek from start of file.
#define SEEK_CUR					1	// Seek relative from current location.
#define SEEK_END					2	// Seek from end of file.
#endif

#ifndef NULL
	#define	NULL	0
#endif


class FileClass
{
	public:

		enum
		{
			READ = 1,
			WRITE = 2,
			PRINTF_BUFFER_SIZE = 1024
		};

		virtual ~FileClass(void) {};
		virtual char const * File_Name(void) const = 0;
		virtual char const * Set_Name(char const *filename) = 0;
		virtual int Create(void) = 0;
		virtual int Delete(void) = 0;
		virtual bool Is_Available(int forced=false) = 0;
		virtual bool Is_Open(void) const = 0;
		virtual int Open(char const *filename, int rights=READ) = 0;
		virtual int Open(int rights=READ) = 0;
		virtual int Read(void *buffer, int size) = 0;
		virtual int Seek(int pos, int dir=SEEK_CUR) = 0;
		virtual int Tell(void) { return Seek(0); }
		virtual int Size(void) = 0;
		virtual int Write(void const *buffer, int size) = 0;
		virtual void Close(void) = 0;
		virtual unsigned long Get_Date_Time(void) {return(0);}
		virtual bool Set_Date_Time(unsigned long ) {return(false);}
//		virtual void Error(int error, int canretry = false, char const * filename=NULL) = 0;
		virtual void * Get_File_Handle(void) { return reinterpret_cast<void *>(-1); }
//		virtual void Bias(int start, int length=-1) = 0;

		operator char const * ()
		{
			return File_Name();
		}

		// this form uses a stack buffer of PRINTF_BUFFER_SIZE
		int Printf(char *str, ...);

		// this form uses the supplied buffer if PRINTF_BUFFER_SIZE is expected to be too small.
		int Printf(char *buffer, int bufferSize, char *str, ...);

		// this form uses the stack buffer but will prepend any output with the indicated number of tab characters '\t'
		int Printf_Indented(unsigned depth, char *str, ...);

};

#endif
