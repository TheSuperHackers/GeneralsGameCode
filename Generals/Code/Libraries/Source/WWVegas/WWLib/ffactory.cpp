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
 *                     $Archive:: /Commando/Code/wwlib/ffactory.cpp                           $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 8/24/01 11:50a                                              $*
 *                                                                                             * 
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"ffactory.h"
#include	"RAWFILE.H"
#include "bufffile.h"
#include "realcrc.h"
#include	<stdio.h>
#include <stdlib.h>
#include	<assert.h>
#include <string.h>

/*
** Statics
** NOTE: If _TheFileFactory is ever changed to point to an object of a different class which does
** not derive from SimpleFileFactoryClass, _TheSimpleFileFactory should be set to NULL.
*/
SimpleFileFactoryClass		_DefaultFileFactory;
FileFactoryClass *			_TheFileFactory = &_DefaultFileFactory;
SimpleFileFactoryClass *			_TheSimpleFileFactory = &_DefaultFileFactory;

RawFileFactoryClass		_DefaultWritingFileFactory;
RawFileFactoryClass *			_TheWritingFileFactory = &_DefaultWritingFileFactory;

/*
**
*/
file_auto_ptr::file_auto_ptr(FileFactoryClass *fac, const char *filename) :
	_Ptr(NULL), _Fac(fac)
{
	assert(_Fac);
	_Ptr=_Fac->Get_File(filename);
	if ( _Ptr == NULL ) {
		_Ptr = W3DNEW BufferedFileClass();
	}
}

file_auto_ptr::~file_auto_ptr()
{
	_Fac->Return_File(_Ptr);
}



/*
** RawFileFactoryClass implementation
*/
RawFileClass * RawFileFactoryClass::Get_File( char const *filename )
{
	return W3DNEW RawFileClass( filename );
}

void RawFileFactoryClass::Return_File( FileClass *file )
{
	delete file;
}



/*
** SimpleFileFactoryClass implementation
*/

SimpleFileFactoryClass::SimpleFileFactoryClass( void ) :
	IsStripPath( false ),
	Mutex( )
{
}


void SimpleFileFactoryClass::Get_Sub_Directory( StringClass& new_dir ) const
{
	// BEGIN SERIALIZATION

	// We cannot return a const char * here because the StringClass
	// may reallocate its buffer during a call to Set_Sub_Directory.
	// I opted to return a StringClass instead of a reference to 
	// StringClass because it seems like that would behave more
	// reasonably. (no sudden changes from or to empty string in
	// the middle of a calling function.) (DRM, 04/19/01)

	// Jani: Returning a StringClass object causes a memory allocation
	// and release so it is better to take a reference to the
	// destination StringClass object and modify that.

	CriticalSectionClass::LockClass lock(Mutex);
	new_dir=SubDirectory;
	// END SERIALIZATION
}


void SimpleFileFactoryClass::Set_Sub_Directory( const char * sub_directory )
{
	// BEGIN SERIALIZATION

	// StringClass makes no guarantees on the atomicity of assignment.
	// Just to be safe, we lock before executing the assignment code.
	// (DRM, 04/19/01)

	CriticalSectionClass::LockClass lock(Mutex);
	SubDirectory = sub_directory;
	// END SERIALIZATION
}


void SimpleFileFactoryClass::Prepend_Sub_Directory( const char * sub_directory )
{
	int sub_len = strlen(sub_directory);
	// Overflow prevention
	if (sub_len > 1021) {
		WWASSERT(0);
		return;
	} else if (sub_len < 1) {
		return;
	}

	// Ensure sub_directory ends with a slash, and append a semicolon
	char temp_sub_dir[1024];
	strcpy(temp_sub_dir, sub_directory);
	if (temp_sub_dir[sub_len - 1] != '\\') {
		temp_sub_dir[sub_len] = '\\';
		temp_sub_dir[sub_len + 1] = 0;
		sub_len++;
	}
	temp_sub_dir[sub_len] = ';';
	temp_sub_dir[sub_len + 1] = 0;

	// BEGIN SERIALIZATION

	// StringClass makes no guarantees on the atomicity of concatenation.
	// Just to be safe, we lock before executing the concatenation code.
	// (NH, 04/23/01)

	CriticalSectionClass::LockClass lock(Mutex);
	SubDirectory = temp_sub_dir + SubDirectory;

	// END SERIALIZATION
}


void SimpleFileFactoryClass::Append_Sub_Directory( const char * sub_directory )
{
	int sub_len = strlen(sub_directory);
	// Overflow prevention
	if (sub_len > 1022) {
		WWASSERT(0);
		return;
	} else if (sub_len < 1) {
		return;
	}

	// Ensure sub_directory ends with a slash
	char temp_sub_dir[1024];
	strcpy(temp_sub_dir, sub_directory);
	if (temp_sub_dir[sub_len - 1] != '\\') {
		temp_sub_dir[sub_len] = '\\';
		temp_sub_dir[sub_len + 1] = 0;
		sub_len++;
	}

	// BEGIN SERIALIZATION

	// We are doing various dependent operations on SubDirectory.
	// Just to be safe, we lock before this section.
	// (NH, 04/23/01)

	CriticalSectionClass::LockClass lock(Mutex);

	// Ensure a trailing semicolon is present, unless the directory list is empty
	int len = SubDirectory.Get_Length();
	if (len && SubDirectory[len - 1] != ';') {
		SubDirectory += ';';
	}

	SubDirectory += temp_sub_dir;
	// END SERIALIZATION
}


/*
**	Is_Full_Path
*/
static bool
Is_Full_Path (const char *path)
{
	bool retval = false;

	if (path != NULL && path[0] != 0) {
		
		// Check for drive designation
		retval = bool(path[1] == ':');

		// Check for network path
		retval |= bool((path[0] == '\\') && (path[1] == '\\'));
	}

	return retval;
}

/*
**
*/
FileClass * SimpleFileFactoryClass::Get_File( char const *filename )
{
	// strip off the path (if needed). Note that if path stripping is off, and the requested file
	// has a path in its name, and the current subdirectory is not empty, the paths will just be
	// concatenated which may not produce reasonable results.
	StringClass stripped_name(true);
	if (IsStripPath) {
		const char * ptr = ::strrchr( filename, '\\' );

		if (ptr != 0) {
			ptr++;
			stripped_name = ptr;
		} else {
			stripped_name = filename;
		}
	} else {
		stripped_name = filename;
	}

	RawFileClass *file = W3DNEW BufferedFileClass();// new RawWritingFileClass();
	assert( file );

	//
	//	Do we need to find the path for this file request?
	//
	StringClass new_name(stripped_name,true);
	if (Is_Full_Path ( new_name ) == false) {

		// BEGIN SERIALIZATION

		// We need to lock here because we are using the contents of SubDirectory
		// in two places. I'd rather be overly cautious about the implementation
		// of StringClass and wrap all calls to it. We can optimize later if this
		// proves too slow. (DRM, 04/19/01)

		CriticalSectionClass::LockClass lock(Mutex);

		if (!SubDirectory.Is_Empty()) {

			//
			// SubDirectory may contain a semicolon seperated search path...
			// If the file doesn't exist, we'll set the path to the last dir in
			// the search path.  Therefore newly created files will always go in the
			// last dir in the search path.
			//
			StringClass subdir(SubDirectory,true);

			if (strchr(subdir,';'))
			{
				char *tokstart=subdir.Peek_Buffer();
				const char *tok;
				while((tok=strtok(tokstart, ";")) != NULL) {
					tokstart=NULL;
					new_name.Format("%s%s",tok,stripped_name.Peek_Buffer());
					file->Set_Name( new_name );	// Call Set_Name to force an allocated name
					if (file->Open()) {
						file->Close();
						break;
					}
				}
			} else {
				new_name.Format("%s%s",SubDirectory,stripped_name);
			}
		}

		// END SERIALIZATION
	}
	
	file->Set_Name( new_name );	// Call Set_Name to force an allocated name
	return file;
}

void SimpleFileFactoryClass::Return_File( FileClass *file )
{
	delete file;
}

