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
 *                 Project Name : CommandoEdit                                                 *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TGAToDXT.h       $*
 *                                                                                             *
 *                       Author:: Ian Leslie		                                               *
 *                                                                                             *
 *                     $Modtime:: 8/10/01 12:20p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef TGATODXT_H
#define TGATODXT_H

#pragma message ("(gth) disabling TGAtoDXTClass temporarily so I can test the WW libs merge...")
#if 0

// Includes.
#include "always.h"
#include <windows.h>
#include <winbase.h>


// Class declerations.
class TGAToDXTClass
{
	public:
		 TGAToDXTClass();
		~TGAToDXTClass();

		enum ErrorCode {
			OK=0,
			INVALID_BIT_DEPTH,
			INVALID_SIZE,
			INVALID_ASPECT_RATIO,
			TGA_LOAD_ERROR,
			COMPRESSION_ERROR
		};

		ErrorCode Convert (const char *inputfilename, const char *outputfilename, FILETIME *writetimeptr, bool &redundantalpha, bool dontcheckalpha=false);

	protected:
		void Write (const char *outputfilename);

		FILETIME		  *WriteTimePtr;	// Time stamp of write time of DXT file.
		unsigned char *Buffer;			// Staging buffer.
		unsigned			BufferSize;		// Size of buffer in bytes.
		unsigned			BufferCount;	// No. of bytes written to buffer.

	friend void ReadDTXnFile (DWORD count, void *buffer);
	friend void WriteDTXnFile (DWORD datacount, void *data);
};


// Externals.
extern TGAToDXTClass _TGAToDXTConverter;

#endif //0

#endif
