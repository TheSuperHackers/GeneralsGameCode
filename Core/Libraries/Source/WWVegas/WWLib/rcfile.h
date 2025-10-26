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
 *                 Project Name : wwlib                                                        *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/rcfile.h                               $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/02/01 1:21p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "WWFILE.H"
#include "win.h"

/*
** ResourceFileClass
** This is a file class which allows you to read from a binary file that you have
** imported into your resources.  Just import the file as a custom resource of the
** type "File".  Replace the "id" of the resource with its filename (change
** IDR_FILE1 to "MyFile.w3d") and then you will be able to access it by using this
** class.
*/
class ResourceFileClass : public FileClass
{
	public:

		ResourceFileClass(HMODULE hmodule, char const *filename);
		virtual ~ResourceFileClass(void);

		virtual char const * File_Name(void) const					{ return ResourceName; }
		virtual char const * Set_Name(char const *filename);
		virtual int Create(void)											{ return false; }
		virtual int Delete(void)											{ return false; }
		virtual bool Is_Available(int /*forced=false*/)				{ return Is_Open (); }
		virtual bool Is_Open(void) const									{ return (FileBytes != NULL); }

		virtual int Open(char const * /*fname*/, int /*rights=READ*/)	{ return Is_Open(); }
		virtual int Open(int /*rights=READ*/)							{ return Is_Open(); }

		virtual int Read(void *buffer, int size);
		virtual int Seek(int pos, int dir=SEEK_CUR);
		virtual int Size(void);
		virtual int Write(void const * /*buffer*/, int /*size*/)	{ return 0; }
		virtual void Close(void)											{ }
		virtual void Error(int error, int canretry = false, char const * filename=NULL);
		virtual void Bias(int start, int length=-1) {}

		virtual unsigned char *Peek_Data(void) const					{ return FileBytes; }

	protected:

		char *				ResourceName;

		HMODULE				hModule;

		unsigned char *	FileBytes;
		unsigned char *	FilePtr;
		unsigned char *	EndOfFile;

};
