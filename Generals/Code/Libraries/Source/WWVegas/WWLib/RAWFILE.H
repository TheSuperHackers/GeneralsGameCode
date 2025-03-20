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
 *                     $Archive:: /G/wwlib/rawfile.h                                          $* 
 *                                                                                             * 
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 10/04/99 10:25a                                             $*
 *                                                                                             * 
 *                    $Revision:: 8                                                           $*
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

#ifndef RAWFILE_Hx
#define RAWFILE_Hx

//#include	<errno.h>

// #include	"win.h"

#ifdef _UNIX
#include <stdio.h>
#include "osdep.h"
  #define	NULL_HANDLE	 	NULL	
  #define	HANDLE_TYPE		FILE*	
#else
  #define	NULL_HANDLE		INVALID_HANDLE_VALUE
  #define	HANDLE_TYPE		HANDLE
#endif

#include	"WWFILE.H"


#ifndef WWERROR
#define WWERROR	-1
#endif

/*
**	This is the definition of the raw file class. It is derived from the abstract base FileClass
**	and handles the interface to the low level DOS routines. This is the first class in the
**	chain of derived file classes that actually performs a useful function. With this class,
**	I/O is possible. More sophisticated features, such as packed files, CD-ROM support,
**	file caching, and XMS/EMS memory support, are handled by derived classes.
**
**	Of particular importance is the need to override the error routine if more sophisticated
**	error handling is required. This is more than likely if greater functionality is derived
**	from this base class.
*/
class RawFileClass : public FileClass
{
		typedef FileClass BASECLASS;

	public:

		/*
		**	This is a record of the access rights used to open the file. These rights are
		**	used if the file object is duplicated.
		*/
		int Rights;

		RawFileClass(char const *filename);
		RawFileClass(void);
		RawFileClass (RawFileClass const & f);
		RawFileClass & operator = (RawFileClass const & f);
		virtual ~RawFileClass(void);

		virtual char const * File_Name(void) const;
		virtual char const * Set_Name(char const *filename);
		virtual int Create(void);
		virtual int Delete(void);
		virtual bool Is_Available(int forced=false);
		virtual bool Is_Open(void) const;
		virtual int Open(char const *filename, int rights=READ);
		virtual int Open(int rights=READ);
		virtual int Read(void *buffer, int size);
		virtual int Seek(int pos, int dir=SEEK_CUR);
		virtual int Size(void);
		virtual int Write(void const *buffer, int size);
		virtual void Close(void);
		virtual unsigned long Get_Date_Time(void);
		virtual bool Set_Date_Time(unsigned long datetime);
		virtual void Error(int error, int canretry = false, char const * filename=NULL);

		void Bias(int start, int length=-1);

		virtual void * Get_File_Handle(void) { return Handle; } 

		virtual void	Attach (void *handle, int rights=READ);
		virtual void	Detach (void);		

		/*
		**	These bias values enable a sub-portion of a file to appear as if it
		**	were the whole file. This comes in very handy for multi-part files such as
		**	mixfiles.
		*/
		int BiasStart;
		int BiasLength;

	protected:

		/*
		**	This function returns the largest size a low level DOS read or write may
		**	perform. Larger file transfers are performed in chunks of this size or less.
		*/
		int Transfer_Block_Size(void);

		int Raw_Seek(int pos, int dir=SEEK_CUR);
		void Reset(void);

	private:

		/*
		**	This is the low level DOS handle. A -1 indicates an empty condition.
		*/
		#ifdef _UNIX
			FILE*  Handle;
		#else
			void * Handle;
		#endif

		/*
		**	This points to the filename as a NULL terminated string. It may point to either a
		**	constant or an allocated string as indicated by the "Allocated" flag.
		*/
		char const * Filename;

		//
		// file date and time are in the following formats:
		//
		//      date   bits 0-4   day (0-31)
		//             bits 5-8   month (1-12)
		//             bits 9-15  year (0-119 representing 1980-2099)
		//
		//      time   bits 0-4   second/2 (0-29)
		//             bits 5-10  minutes (0-59)
		//             bits 11-15 hours (0-23)
		//
		unsigned short Date;
		unsigned short Time;

		/*
		**	Filenames that were assigned as part of the construction process
		**	are not allocated. It is assumed that the filename string is a
		**	constant in that case and thus making duplication unnecessary.
		**	This value will be non-zero if the filename has be allocated
		**	(using strdup()).
		*/
		bool Allocated;
};

#endif
