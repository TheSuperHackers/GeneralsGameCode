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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwlib/mixfile.h                              $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 8/06/01 3:14p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	MIXFILE_H
#define	MIXFILE_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	FFACTORY_H
	#include "ffactory.h"
#endif

#ifndef	WWSTRING_H
	#include "wwstring.h"
#endif

#include "Vector.H"

class FileClass;

/*
**
*/
class	MixFileFactoryClass : public FileFactoryClass {

public:
	MixFileFactoryClass( const char * mix_filename, FileFactoryClass * factory );
	~MixFileFactoryClass( void );

	//
	//	Inherited
	//
	virtual FileClass * Get_File( char const *filename );
	virtual void Return_File( FileClass *file );

	//
	//	Filename access
	//
	bool		Build_Filename_List (DynamicVectorClass<StringClass> &list);
	bool		Build_Internal_Filename_List (void)									{ return Build_Filename_List (FilenameList); }
	void		Get_Filename_List (DynamicVectorClass<StringClass> **list)	{ *list = &FilenameList; }
	void		Get_Filename_List (DynamicVectorClass<StringClass> &list)	{ list = FilenameList; }

	//
	//	Content control
	//
	void		Add_File (const char *full_path, const char *filename);
	void		Delete_File (const char *filename);
	void		Flush_Changes (void);

	//
	//	Information
	//
	bool		Is_Valid (void) const	{ return IsValid; }

private:

	//
	//	Utility functions
	//
	bool		Get_Temp_Filename (const char *path, StringClass &full_path);

	struct FileInfoStruct {
		bool operator== (const FileInfoStruct &src)	{ return false; }
		bool operator!= (const FileInfoStruct &src)	{ return true; }

		unsigned long CRC;				// CRC code for embedded file.
		unsigned long Offset;			// Offset from start of data section.
		unsigned long Size;				// Size of data subfile.
	};

	struct AddInfoStruct {
		bool operator== (const AddInfoStruct &src)	{ return false; }
		bool operator!= (const AddInfoStruct &src)	{ return true; }

		StringClass FullPath;
		StringClass	Filename;
	};

	FileFactoryClass *						Factory;
	DynamicVectorClass<FileInfoStruct>	FileInfo;
	StringClass									MixFilename;
	int											BaseOffset;

	int											FileCount;
	int											NamesOffset;
	bool											IsValid;
	DynamicVectorClass<StringClass>		FilenameList;

	DynamicVectorClass<AddInfoStruct>	PendingAddFileList;
	bool											IsModified;
};

/*
**
*/
class	MixFileCreator {

public:
	MixFileCreator( const char * filename );
	~MixFileCreator( void );

	void	Add_File( const char * source_filename, const char * saved_filename = NULL );
	void	Add_File( const char * filename, FileClass *file );

private:

	static int File_Info_Compare(const void * a, const void * b);

	struct FileInfoStruct {
		bool operator== (const FileInfoStruct &src)	{ return false; }
		bool operator!= (const FileInfoStruct &src)	{ return true; }

		unsigned long	CRC;				// CRC code for embedded file.
		unsigned long	Offset;			// Offset from start of data section.
		unsigned long	Size;				// Size of data subfile.
		StringClass		Filename;
	};

	DynamicVectorClass<FileInfoStruct>	FileInfo;
	FileClass								*	MixFile;
};

/*
**
*/
void	Setup_Mix_File( void );

#endif
