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

#include "Common/ArchiveFile.h"
#include "Common/Debug.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/MapObject.h"
#include "Common/Registry.h"
#include "W3DDevice/GameClient/W3DFileSystem.h"

#include <io.h>

// DEFINES ////////////////////////////////////////////////////////////////////////////////////////

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

	m_theFile = NULL;
	m_fileExists = FALSE;
	m_filePath[0] = 0;
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
//-------------------------------------------------------------------------------------------------
static GameFileType getFileType( char const *filename )
{
	if (char const *extension = strrchr( filename, '.' ))
	{
		// test the extension to recognize a few key file types
		if( stricmp( extension, ".w3d" ) == 0 )
			return FILE_TYPE_W3D;
		else if( stricmp( extension, ".tga" ) == 0 )
			return FILE_TYPE_TGA;
		else if( stricmp( extension, ".dds" ) == 0 )
			return FILE_TYPE_DDS;
	}

	return FILE_TYPE_COMPLETELY_UNKNOWN;  // MBL FILE_TYPE_UNKNOWN change due to compile error
}

//-------------------------------------------------------------------------------------------------
/** Sets the file name, and finds the GDI asset if present. */
//-------------------------------------------------------------------------------------------------
char const * GameFileClass::Set_Name( char const *filename )
{

	if( Is_Open() )
		Close();

	// save the filename
	strlcpy( m_filename, filename, _MAX_PATH );

	GameFileType fileType = getFileType(filename);

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
		else if( isImageFileType(fileType) )
		{
			sprintf(m_filePath,USER_TGA_DIR_PATH, TheGlobalData->getPath_UserData().str());
			//strcpy( m_filePath, USER_TGA_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}

		// see if the file exists
		m_fileExists = TheFileSystem->doesFileExist( m_filePath );

	}


	// We need to be able to temporarily copy over the map preview for whichever directory it came from
	if( m_fileExists == FALSE  && TheGlobalData)
	{
		if( fileType == FILE_TYPE_TGA ) // just TGA, since we don't do dds previews
		{
			sprintf(m_filePath,MAP_PREVIEW_DIR_PATH, TheGlobalData->getPath_UserData().str());
			//strcpy( m_filePath, USER_TGA_DIR_PATH );
			strlcat(m_filePath, filename, ARRAY_SIZE(m_filePath));

		}

		// see if the file exists
		m_fileExists = TheFileSystem->doesFileExist( m_filePath );

	}

	// We need to be able to grab images from a localization dir, because Art has a fetish for baked-in text.  Munkee.
	if( m_fileExists == FALSE )
	{
		if( isImageFileType(fileType) )
		{
			static const char *localizedPathFormat = "Data/%s/Art/Textures/";
			sprintf(m_filePath,localizedPathFormat, GetRegistryLanguage().str());
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
/** Constructor.  Creating an instance of this class overrides the default
W3D file factory.  */
//-------------------------------------------------------------------------------------------------
W3DFileSystem::W3DFileSystem(void)
{
	_TheFileFactory = this; // override the w3d file factory.

#if RTS_ZEROHOUR && PRIORITIZE_TEXTURES_BY_SIZE
	reprioritizeTexturesBySize();
#endif
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

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DFileSystem::reprioritizeTexturesBySize()
{
	ArchivedDirectoryInfo* dirInfo = TheArchiveFileSystem->friend_getArchivedDirectoryInfo(TGA_DIR_PATH);
	if (dirInfo != NULL)
	{
		reprioritizeTexturesBySize(*dirInfo);
	}
}

//-------------------------------------------------------------------------------------------------
// TheSuperHackers @info This function moves the largest texture of its name to the front of the
// directory info. The algorithm only prioritizes the first item in the multimap, because this is
// what we currently need:
// Before: A(256kb) B(128kb) C(512kb)
// After:  C(512kb) B(128kb) A(256kb)
//
// Catered to specific game archives only. This ensures that user created archives are not included
// for the re-prioritization of textures.
//-------------------------------------------------------------------------------------------------
void W3DFileSystem::reprioritizeTexturesBySize(ArchivedDirectoryInfo& dirInfo)
{
	const char* const superiorArchive = "Textures.big";
	const char* const inferiorArchive = "TexturesZH.big";

	ArchivedFileLocationMap::iterator it0;
	ArchivedFileLocationMap::iterator it1 = dirInfo.m_files.begin();
	ArchivedFileLocationMap::iterator end = dirInfo.m_files.end();

	if (it1 != end)
	{
		it0 = it1;
		++it1;
	}

	for (; it1 != end; ++it1)
	{
		const AsciiString& file0 = it0->first;
		const AsciiString& file1 = it1->first;

		if (file0 == file1)
		{
			GameFileType type = getFileType(file0.str());
			if (isImageFileType(type))
			{
				ArchiveFile* archive0 = it0->second;
				ArchiveFile* archive1 = it1->second;
				FileInfo info0;
				FileInfo info1;
				AsciiString filepath(dirInfo.m_path);
				filepath.concat(file0);

				if (archive0->getFileInfo(filepath, &info0) && archive1->getFileInfo(filepath, &info1))
				{
					if (info0.size() < info1.size()
						&& archive0->getName().endsWithNoCase(inferiorArchive)
						&& archive1->getName().endsWithNoCase(superiorArchive))
					{
						std::swap(it0->second, it1->second);

#if ENABLE_FILESYSTEM_LOGGING
						DEBUG_LOG(("W3DFileSystem::reprioritizeTexturesBySize - prioritize %s(%ukb) from %s over %s(%ukb) from %s",
							file1.str(), UnsignedInt(info1.size() / 1024), archive1->getName().str(),
							file0.str(), UnsignedInt(info0.size() / 1024), archive0->getName().str()));
#endif
					}
				}
			}
		}
		else
		{
			it0 = it1;
		}
	}
}
