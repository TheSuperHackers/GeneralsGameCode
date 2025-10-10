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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DFileSystem.cpp ////////////////////////////////////////////////////////////////////////
//
// W3D implementation of a file factory.  This replaces the W3D file factory,
// and uses GDI assets, so that
// W3D files and targa files are loaded using the GDI file interface.
// Note - this only servers up read only files.
//
// Author: John Ahlquist, Sept 2001
//				 Colin Day, November 2001
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////


// for now we maintain old legacy files
// #define MAINTAIN_LEGACY_FILES

#include "Common/Debug.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/MapObject.h"
#include "Common/Registry.h"
#include "W3DDevice/GameClient/W3DFileSystem.h"
// DEFINES ////////////////////////////////////////////////////////////////////////////////////////

#include <io.h>

//-------------------------------------------------------------------------------------------------
/** Game file access.  At present this allows us to access test assets, assets from
	* legacy GDI assets, and the current flat directory access for textures, models etc */
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
typedef enum
{
	FILE_TYPE_COMPLETELY_UNKNOWN = 0,	// MBL 08.15.2002 - compile error with FILE_TYPE_UNKNOWN, is constant
	FILE_TYPE_W3D,
	FILE_TYPE_TGA,
	FILE_TYPE_DDS,
} GameFileType;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GameFileClass::GameFileClass( char const *filename )
{

	m_fileExists = FALSE;
	m_theFile = NULL;
	m_filePath[ 0 ] = 0;
	m_filename[0] = 0;

	if( filename )
		Set_Name( filename );

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GameFileClass::GameFileClass( void )
{

	m_fileExists = FALSE;
	m_theFile = NULL;
	m_filePath[ 0 ] = 0;
	m_filename[ 0 ] = 0;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GameFileClass::~GameFileClass()
{

	Close();

}

//-------------------------------------------------------------------------------------------------
/** Gets the file name */
//-------------------------------------------------------------------------------------------------
char const * GameFileClass::File_Name( void ) const
{

	return m_filename;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
inline static Bool isImageFileType( GameFileType fileType )
{
	return (fileType == FILE_TYPE_TGA || fileType == FILE_TYPE_DDS);
}

//-------------------------------------------------------------------------------------------------
/**
	Sets the file name, and finds the GDI asset if present.


	Well, that is the worst comment ever for the most important function there is.
	Everything comes through this.  This builds the directory and tests for the file
	in several different places.

	First we look in Language subfolders so that our Perforce	build can handle files that have
	been localized but were in Generals.

	Then we do the normal TheFileSystem lookup.  In there it does LocalFile (Art/Textures) then it does
	big files (which internally are also Art/Textures).

	Finally we try UserData.
*/
//-------------------------------------------------------------------------------------------------
char const * GameFileClass::Set_Name( char const *filename )
{

	if( Is_Open() )
		Close();

	// save the filename
	strlcpy( m_filename, filename, _MAX_PATH );

	char name[_MAX_PATH];
	const Int EXT_LEN = 32;
	char extension[EXT_LEN];
	extension[0] = 0;
	strcpy(name, filename);
	Int i = strlen(name);
	i--;
	Int extLen = 1;
	while(i>0 && extLen < EXT_LEN) {
		if (name[i] == '.') {
			strcpy(extension, name+i);
			name[i] = 0;
			break;
		}
		i--;
		extLen++;
	}
	Int j = 0;
	// Strip out spaces.
	for (i=0; name[i]; i++) {
		if (name[i] != ' ') {
			name[j] = name[i];
			j++;
		}
	}
	name[j] = 0;

	// test the extension to recognize a few key file types
	GameFileType fileType = FILE_TYPE_COMPLETELY_UNKNOWN;  // MBL FILE_TYPE_UNKNOWN change due to compile error
	if( stricmp( extension, ".w3d" ) == 0 )
		fileType = FILE_TYPE_W3D;
	else if( stricmp( extension, ".tga" ) == 0 )
		fileType = FILE_TYPE_TGA;
	else if( stricmp( extension, ".dds" ) == 0 )
		fileType = FILE_TYPE_DDS;



	// We need to be able to grab w3d's from a localization dir, since Germany hates exploding people units.
	if( fileType == FILE_TYPE_W3D )
	{
		static const char *localizedPathFormat = "Data/%s/Art/W3D/";
		sprintf(m_filePath,localizedPathFormat, GetRegistryLanguage().str());
		strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

	}

	// We need to be able to grab images from a localization dir, because Art has a fetish for baked-in text.  Munkee.
	if( isImageFileType(fileType) )
	{
		static const char *localizedPathFormat = "Data/%s/Art/Textures/";
		sprintf(m_filePath,localizedPathFormat, GetRegistryLanguage().str());
		strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

	}

	// see if the file exists
	m_fileExists = TheFileSystem->doesFileExist( m_filePath );



	// Now try the main lookup of hitting local files and big files
	if( m_fileExists == FALSE )
	{
		// all .w3d files are in W3D_DIR_PATH, all .tga files are in TGA_DIR_PATH
		if( fileType == FILE_TYPE_W3D )
		{

			strcpy( m_filePath, W3D_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}
		else if( isImageFileType(fileType) )
		{

			strcpy( m_filePath, TGA_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}
		else
			strcpy( m_filePath, filename );

		// see if the file exists
		m_fileExists = TheFileSystem->doesFileExist( m_filePath );
	}



	// maintain legacy compatibility directories for now
	#ifdef MAINTAIN_LEGACY_FILES
	if( m_fileExists == FALSE )
	{

		if( fileType == FILE_TYPE_W3D )
		{

			strcpy( m_filePath, LEGACY_W3D_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}
		else if( isImageFileType(fileType) )
		{

			strcpy( m_filePath, LEGACY_TGA_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}

		// see if the file exists
		m_fileExists = TheFileSystem->doesFileExist( m_filePath );

	}
	#endif



	// if file is still not found, try the test art folders
	#ifdef LOAD_TEST_ASSETS
	if( m_fileExists == FALSE )
	{

		if( fileType == FILE_TYPE_W3D )
		{

			strcpy( m_filePath, TEST_W3D_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}
		else if( isImageFileType(fileType) )
		{

			strcpy( m_filePath, TEST_TGA_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}

		// see if the file exists
		m_fileExists = TheFileSystem->doesFileExist( m_filePath );

	}
	#endif

	// We allow the user to load their own images for various assets (like the control bar)
	if( m_fileExists == FALSE  && TheGlobalData)
	{
		if( fileType == FILE_TYPE_W3D )
		{
			sprintf(m_filePath,USER_W3D_DIR_PATH, TheGlobalData->getPath_UserData().str());
			//strcpy( m_filePath, USER_W3D_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}
		if( isImageFileType(fileType) )
		{
			sprintf(m_filePath,USER_TGA_DIR_PATH, TheGlobalData->getPath_UserData().str());
			//strcpy( m_filePath, USER_TGA_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}

		// see if the file exists
		m_fileExists = TheFileSystem->doesFileExist( m_filePath );

	}


	// We Need to be able to "temporarily copy over the map preview for whichever directory it came from
	if( m_fileExists == FALSE  && TheGlobalData)
	{
		if( fileType == FILE_TYPE_TGA ) // just TGA, since we don't dds previews
		{
			sprintf(m_filePath,MAP_PREVIEW_DIR_PATH, TheGlobalData->getPath_UserData().str());
			//strcpy( m_filePath, USER_TGA_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}

		// see if the file exists
		m_fileExists = TheFileSystem->doesFileExist( m_filePath );

	}

	return m_filename;

}

//-------------------------------------------------------------------------------------------------
/** If we found a gdi asset, the file is available. */
//-------------------------------------------------------------------------------------------------
bool GameFileClass::Is_Available( int forced )
{

	// not maintaining any GDF compatibility, all files should be where the m_filePath says
	return m_fileExists;

}

//-------------------------------------------------------------------------------------------------
/** Is the file open. */
//-------------------------------------------------------------------------------------------------
bool GameFileClass::Is_Open(void) const
{
	return m_theFile != NULL;
}

//-------------------------------------------------------------------------------------------------
/** Open the named file. */
//-------------------------------------------------------------------------------------------------
int  GameFileClass::Open(char const *filename, int rights)
{
	Set_Name(filename);
	if (Is_Available(false)) {
		return(Open(rights));
	}
	return(false);
}

//-------------------------------------------------------------------------------------------------
/** Open the file using the current file name. */
//-------------------------------------------------------------------------------------------------
int  GameFileClass::Open(int rights)
{
	if( rights != READ )
	{
		return(false);
	}

	// just open up the file in m_filePath
	m_theFile = TheFileSystem->openFile( m_filePath, File::READ | File::BINARY );

	return (m_theFile != NULL);

}

//-------------------------------------------------------------------------------------------------
/** Read. */
//-------------------------------------------------------------------------------------------------
int GameFileClass::Read(void *buffer, int len)
{
	if (m_theFile) {
		return m_theFile->read(buffer, len);
	}
	return(0);
}

//-------------------------------------------------------------------------------------------------
/** Seek. */
//-------------------------------------------------------------------------------------------------
int GameFileClass::Seek(int pos, int dir)
{
	File::seekMode mode = File::CURRENT;
	switch (dir) {
		default:
		case SEEK_CUR: mode = File::CURRENT; break;
		case SEEK_SET: mode = File::START; break;
		case SEEK_END: mode = File::END; break;
	}
	if (m_theFile) {
		return m_theFile->seek(pos, mode);
	}
	return 0xFFFFFFFF;
}

//-------------------------------------------------------------------------------------------------
/** Size. */
//-------------------------------------------------------------------------------------------------
int GameFileClass::Size(void)
{
	if (m_theFile) {
		return m_theFile->size();
	}
	return 0xFFFFFFFF;
}

//-------------------------------------------------------------------------------------------------
/** Write. */
//-------------------------------------------------------------------------------------------------
int GameFileClass::Write(void const *buffer, Int len)
{
#ifdef RTS_DEBUG
#endif
	return(0);
}

//-------------------------------------------------------------------------------------------------
/** Close. */
//-------------------------------------------------------------------------------------------------
void GameFileClass::Close(void)
{
	if (m_theFile) {
		m_theFile->close();
		m_theFile = NULL;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// W3DFileSystem Class ////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
extern W3DFileSystem *TheW3DFileSystem = NULL;

//-------------------------------------------------------------------------------------------------
/** Constructor.  Creating an instance of this class overrices the default
W3D file factory.  */
//-------------------------------------------------------------------------------------------------
W3DFileSystem::W3DFileSystem(void)
{
	_TheFileFactory = this; // override the w3d file factory.
}

//-------------------------------------------------------------------------------------------------
/** Destructor.  This removes the W3D file factory, so shouldn't be done until
after W3D is shutdown.  */
//-------------------------------------------------------------------------------------------------
W3DFileSystem::~W3DFileSystem(void)
{
	_TheFileFactory = NULL; // remove the w3d file factory.
}

//-------------------------------------------------------------------------------------------------
/** Gets a file with the specified filename. */
//-------------------------------------------------------------------------------------------------
FileClass * W3DFileSystem::Get_File( char const *filename )
{
	return NEW GameFileClass( filename );	// poolify
}

//-------------------------------------------------------------------------------------------------
/** Releases a file returned by Get_File. */
//-------------------------------------------------------------------------------------------------
void W3DFileSystem::Return_File( FileClass *file )
{
	delete file;
}

