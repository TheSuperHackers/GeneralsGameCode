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
 *                     $Archive:: /Commando/Code/wwlib/mixfile.cpp                            $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 8/06/01 3:01p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "mixfile.h"
#include "wwdebug.h"
#include "ffactory.h"
#include "wwfile.h"
#include "realcrc.h"
#include "rawfile.h"
#include "win.h"
#include "bittype.h"

/*
**
*/
typedef struct
{
	char	signature[4];
	long	header_offset;
	long	names_offset;

} MIXFILE_HEADER;

typedef struct
{
	long	file_count;

} MIXFILE_DATA_HEADER;


/*
**
*/					
MixFileFactoryClass::MixFileFactoryClass( const char * mix_filename, FileFactoryClass * factory )	:
	FileCount (0),
	NamesOffset (0),
	IsValid (false),
	BaseOffset (0),
	Factory (NULL),
	IsModified (false)
{
//	WWDEBUG_SAY(( "MixFileFactory( %s )\n", mix_filename ));
	MixFilename	= mix_filename;
	Factory		= factory;

	// First, open the mix file
	FileClass * file = factory->Get_File( mix_filename );

//	WWASSERT( file );

	if ( file && file->Is_Available() ) {

		file->Open();

		IsValid = true;

		//
		//	Read the file header
		//
		MIXFILE_HEADER header = { 0 };
		IsValid = (file->Read( &header, sizeof( header ) ) == sizeof( header ));

		//
		//	Validate the file header
		//
		if ( IsValid ) {
			IsValid = (::memcmp( header.signature, "MIX1", sizeof ( header.signature ) ) == 0);
		}

		//
		//	Seek to the data start
		//
		FileCount = 0;
		if ( IsValid ) {
			file->Seek( header.header_offset, SEEK_SET );
			IsValid = ( file->Read( &FileCount, sizeof( FileCount ) ) == sizeof( FileCount ) );
		}
		
		//
		//	Read the array of data headers
		//
		if ( IsValid ) {
			FileInfo.Resize( FileCount );
			int size = FileCount * sizeof( FileInfoStruct );
			IsValid = ( file->Read( &FileInfo[0], size ) == size );
		}

		//
		//	Check for success
		//
		if ( IsValid ) {
			BaseOffset	= 0;
			NamesOffset	= header.names_offset;
			WWDEBUG_SAY(( "MixFileFactory( %s ) loaded successfully  %d files\n", MixFilename, FileInfo.Length() ));
		} else {
			FileInfo.Resize(0);
		}	

		factory->Return_File( file );

	} else {
		WWDEBUG_SAY(( "MixFileFactory( %s ) FAILED\n", mix_filename ));
	}
}

MixFileFactoryClass::~MixFileFactoryClass( void )
{
	FileInfo.Resize(0);
}

bool	MixFileFactoryClass::Build_Filename_List (DynamicVectorClass<StringClass> &list)
{
	if (IsValid == false) {
		return false;
	}

	bool retval = false;

	//
	//	Attempt to open the file
	//
	RawFileClass *file = (RawFileClass *)Factory->Get_File( MixFilename );
	if ( file != NULL && file->Open ( RawFileClass::READ ) ) {

		//
		//	Seek to the names offset header
		//
		file->Seek (NamesOffset, SEEK_SET);
		retval = true;
		
		//
		//	Read the count of files
		//
		int file_count = 0;
		if (file->Read( &file_count, sizeof( file_count) ) == sizeof( file_count )) {

			//
			//	Loop over each saved filename
			//
			bool keep_going = true;
			for (int index = 0; index < file_count && keep_going; index ++) {
				keep_going = false;
				
				//
				//	Get the length of the filename
				//
				uint8 name_len = 0;
				if (file->Read( &name_len, sizeof( name_len ) ) == sizeof( name_len )) {
					
					//
					//	Read the filename
					//
					StringClass filename;
					if (file->Read( filename.Get_Buffer( name_len ), name_len ) == name_len ) {
						
						//
						//	Add the filename to our list
						//
						list.Add( filename );
						keep_going = true;
					}
				}
			}
		}

		//
		//	Close the file
		//
		Factory->Return_File( file );
	}

	return retval;
}

FileClass * MixFileFactoryClass::Get_File( char const *filename )
{
	if ( FileInfo.Length() == 0 ) {
		return NULL;
	}
//	WWDEBUG_SAY(( "MixFileFactoryClass::Get_File( %s )\n", filename ));

	RawFileClass *file = NULL;

	//	Create the key block that will be used to binary search for the file.
	unsigned long crc = CRC_Stringi( filename );

	//	Binary search for the file in this mixfile. If it is found, then create the file
	FileInfoStruct * info = NULL;
	FileInfoStruct * base = &FileInfo[0];
	int stride = FileInfo.Length();
	while (stride > 0) {
		int pivot = stride / 2;
		FileInfoStruct * tryptr = base + pivot;
		if (crc < tryptr->CRC) {
			stride = pivot;
		} else {
			if (tryptr->CRC == crc) {
				info = tryptr;
				break;
			}
		   base = tryptr + 1;
			stride -= pivot + 1;
		}
	}		

	if ( info != NULL) {
//		WWDEBUG_SAY(( "MixFileFactoryClass::Get_File( %s ) FOUND\n", filename ));
		file = (RawFileClass *)Factory->Get_File( MixFilename );
		if ( file ) {
			file->Bias( BaseOffset + info->Offset, info->Size );
		}
//		WWDEBUG_SAY(( "MixFileFactoryClass::Get_File( %s ) FOUND\n", filename ));
	} else {
//		WWDEBUG_SAY(( "MixFileFactoryClass::Get_File( %s ) NOT FOUND\n", filename ));
	}

	return file;
}

void	MixFileFactoryClass::Return_File( FileClass * file )
{
	if ( file != NULL ) {
		Factory->Return_File( file );
	}
}


/*
**
*/
void
MixFileFactoryClass::Add_File (const char *full_path, const char *filename)
{
	AddInfoStruct info;
	info.FullPath = full_path;
	info.Filename = filename;
	PendingAddFileList.Add (info);
	IsModified = true;
	return ;
}


/*
**
*/
void
MixFileFactoryClass::Delete_File (const char *filename)
{
	//
	//	Remove this file (if it exists) from our filename list
	//
	for (int list_index = 0; list_index < FilenameList.Count (); list_index ++) {
		if (FilenameList[list_index].Compare_No_Case (filename) == 0) {
			FilenameList.Delete (list_index);
			IsModified = true;
			break;
		}
	}

	return ;
}


/*
**
*/
void
MixFileFactoryClass::Flush_Changes (void)
{
	//
	//	Exit if there's nothing to do.
	//
	if (IsModified == false) {
		return ;
	}

	//
	//	Get the path of the mix file
	//
	char drive[_MAX_DRIVE] = { 0 };
	char dir[_MAX_DIR] = { 0 };
	::_splitpath (MixFilename, drive, dir, NULL, NULL);
	StringClass path	= drive;
	path					+= dir;

	//
	//	Try to find a temp filename
	//
	StringClass full_path;
	if (Get_Temp_Filename (path, full_path)) {
		MixFileCreator new_mix_file (full_path);

		//
		//	Add all the remaining files from our file set
		//
		int index = 0;
		for (; index < FilenameList.Count (); index ++) {
			StringClass &filename = FilenameList[index];

			//
			//	Copy this file data to the mix file
			//
			FileClass *file_data = Get_File (filename);
			if (file_data != NULL) {
				file_data->Open ();
				new_mix_file.Add_File (filename, file_data);
				Return_File (file_data);

				//
				//	Remove this file from the pending list (if necessary)
				//
				for (int temp_index = 0; temp_index < PendingAddFileList.Count (); temp_index ++) {
					if (filename.Compare_No_Case (PendingAddFileList[temp_index].Filename) == 0) {
						PendingAddFileList.Delete (temp_index);
						break;
					}
				}
			}
		}

		//
		//	Add the new files that are pending
		//
		for (index = 0; index < PendingAddFileList.Count (); index ++) {
			new_mix_file.Add_File (PendingAddFileList[index].FullPath, PendingAddFileList[index].Filename);
		}
	}

	//
	//	Delete the old mix file and rename the new one
	//
	::DeleteFile (MixFilename);
	::MoveFile (full_path, MixFilename);

	//
	//	Reset the lists
	//
	IsModified = false;	
	PendingAddFileList.Delete_All ();
	return ;
}


/*
**
*/
bool
MixFileFactoryClass::Get_Temp_Filename (const char *path, StringClass &full_path)
{
	bool retval = false;

	StringClass temp_path	= path;
	temp_path					+= "_tmpmix";
	
	//
	//	Try to find a unique temp filename
	//
	for (int index = 0; index < 20; index ++) {
		full_path.Format ("%s%.2d.dat", (const char *)temp_path, index + 1);
		if (GetFileAttributes (full_path) == 0xFFFFFFFF) {
			retval = true;
			break;
		}
	}

	return retval;
}


/*
**
*/
SimpleFileFactoryClass _SimpleFileFactory;

MixFileCreator::MixFileCreator( const char * filename )
{
	WWDEBUG_SAY(( "Creating Mix File %s\n", filename ));

	MixFile = _SimpleFileFactory.Get_File(filename);
	if ( MixFile != NULL ) {
		MixFile->Open( FileClass::WRITE );
		MixFile->Write( "MIX1", 4 );
		long	header_offset = 0;
		MixFile->Write( &header_offset, sizeof( header_offset ) );
		long	names_offset = 0;
		MixFile->Write( &names_offset, sizeof( names_offset ) );
		long	unused = 0;
		MixFile->Write( &unused, sizeof( unused ) );
	}
}

int MixFileCreator::File_Info_Compare(const void * a, const void * b) 
{
	unsigned int CRCA = ((FileInfoStruct*)a)->CRC;
	unsigned int CRCB = ((FileInfoStruct*)b)->CRC;
	if ( CRCA < CRCB ) return -1;
	if ( CRCA > CRCB ) return 1;
	return 0;
//	return ((FileInfoStruct*)a)->CRC - ((FileInfoStruct*)b)->CRC;
}

MixFileCreator::~MixFileCreator( void )
{
	if ( MixFile != NULL ) {

		// Save Header Data
		int header_offset = MixFile->Tell();

		// Save file count
		int i,num_files = FileInfo.Count();
		WWDEBUG_SAY(( "Closing with %d files\n", num_files ));
		MixFile->Write( &num_files, sizeof( num_files ) );

		if ( num_files > 1 ) {
		   qsort( &FileInfo[0], num_files, sizeof(FileInfo[0]), &File_Info_Compare);
		}

		// Save file info (CRC, Offset, Size )
		for ( i = 0; i < num_files; i++ ) {
			MixFile->Write( &FileInfo[i].CRC, 4 );
			MixFile->Write( &FileInfo[i].Offset, 4 );
			MixFile->Write( &FileInfo[i].Size, 4 );
//			WWDEBUG_SAY(( "Write CRC %08X\n", FileInfo[i].CRC ));
		}

		// ---------------------------------------

		// Save Names Data
		int names_offset = MixFile->Tell();

		// Save file count
		MixFile->Write( &num_files, sizeof( num_files ) );

		// Save file info
		for ( i = 0; i < num_files; i++ ) {
			const char * filename = FileInfo[i].Filename;
			int size = FileInfo[i].Filename.Get_Length()+1;
			WWASSERT( size < 255 );
			unsigned char csize = size;
			MixFile->Write( &csize, 1 );
			MixFile->Write( filename, size );
		}

		// ---------------------------------------

		MixFile->Seek( 4, SEEK_SET );

		// Save header offset
		WWDEBUG_SAY(( "Writing header offset %d (%08X)\n", header_offset, header_offset ));
		MixFile->Write( &header_offset, sizeof( header_offset ) );

		// Save names offset
		WWDEBUG_SAY(( "Writing names offset %d (%08X)\n", names_offset, names_offset ));
		MixFile->Write( &names_offset, sizeof( names_offset ) );

		// ---------------------------------------

		MixFile->Close();

		_SimpleFileFactory.Return_File(MixFile);
	}
}

void	MixFileCreator::Add_File( const char * source_filename, const char * saved_filename )
{
	if ( saved_filename == NULL ) {
		saved_filename = source_filename;
	}

	if ( MixFile != NULL ) {

		FileClass * file = _SimpleFileFactory.Get_File( source_filename );

		if ( file && file->Is_Available() ) {

			file->Open();

			MixFileCreator::FileInfoStruct info;
			info.CRC			= CRC_Stringi( saved_filename );
			info.Offset		= MixFile->Tell();
			info.Size		= file->Size();
			FileInfo.Add( info );
			FileInfo[ FileInfo.Count()-1 ].Filename = saved_filename;

			WWDEBUG_SAY(( "Saving File %s CRC %08X Offset %d (0x%08X) Size %d (0x%08X)\n",
					saved_filename, info.CRC, info.Offset, info.Offset, info.Size, info.Size ));

			int size = file->Size();
			while ( size ) {
				char buffer[ 4096 ];
				int amount = MIN( sizeof( buffer ), size );
				size -= amount;
				file->Read( buffer, amount );
				if ( MixFile->Write( buffer, amount ) != amount ) {
					WWDEBUG_SAY(( "Failed to write MixFile\n" ));
				}
			}

			// Pad the MixFile to make DWord Aligned
			int offset = MixFile->Tell();
			offset = (8-(offset & 7)) & 7;
			if ( offset != 0 ) {
				char zeros[8] = {0,0,0,0,0,0,0,0};
				if ( MixFile->Write( zeros, offset ) != offset ) {
					WWDEBUG_SAY(( "Failed to write padding\n" ));
				}
			}

			file->Close();
			_SimpleFileFactory.Return_File( file );

		} else {
			WWDEBUG_SAY(( "MixFileCreator::Failed to open \"%s\"\n", source_filename ));
		}
	}
}


void	MixFileCreator::Add_File( const char * filename, FileClass *file )
{
	if ( MixFile != NULL ) {

		MixFileCreator::FileInfoStruct info;
		info.CRC			= CRC_Stringi( filename );
		info.Offset		= MixFile->Tell();
		info.Size		= file->Size();
		FileInfo.Add( info );
		FileInfo[ FileInfo.Count()-1 ].Filename = filename;

		WWDEBUG_SAY(( "Saving File %s CRC %08X Offset %d (0x%08X) Size %d (0x%08X)\n",
				filename, info.CRC, info.Offset, info.Offset, info.Size, info.Size ));

		int size = file->Size();
		while ( size ) {
			char buffer[ 4096 ];
			int amount = MIN( sizeof( buffer ), size );
			size -= amount;
			file->Read( buffer, amount );
			if ( MixFile->Write( buffer, amount ) != amount ) {
				WWDEBUG_SAY(( "Failed to write MixFile\n" ));
			}
		}

		// Pad the MixFile to make DWord Aligned
		int offset = MixFile->Tell();
		offset = (8-(offset & 7)) & 7;
		if ( offset != 0 ) {
			char zeros[8] = {0,0,0,0,0,0,0,0};
			if ( MixFile->Write( zeros, offset ) != offset ) {
				WWDEBUG_SAY(( "Failed to write padding\n" ));
			}
		}
	}

	return ;
}


/*
**
*/
void	Add_Files( const char * dir, MixFileCreator & mix )
{
	BOOL bcontinue = TRUE;
	HANDLE hfile_find;
	WIN32_FIND_DATA find_info = {0};
	StringClass path;
	path.Format( "data\\makemix\\%s*.*", dir );
	WWDEBUG_SAY(( "Adding files from %s\n", path ));

	for (hfile_find = ::FindFirstFile( path, &find_info);
		 (hfile_find != INVALID_HANDLE_VALUE) && bcontinue;
		  bcontinue = ::FindNextFile(hfile_find, &find_info)) {
		if ( find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			if ( find_info.cFileName[0] != '.' ) {
				StringClass	path;
				path.Format( "%s%s\\", dir, find_info.cFileName );
				Add_Files( path, mix );
			}
		} else {
			StringClass name;
			name.Format( "%s%s", dir, find_info.cFileName );
			StringClass	source;
			source.Format( "makemix\\%s", name );
			mix.Add_File( source, name );
//			WWDEBUG_SAY(( "Adding file from %s %s\n", source, name ));
		}
	}
}

void	Setup_Mix_File( void )
{
	_SimpleFileFactory.Set_Sub_Directory( "DATA\\" );
//	_SimpleFileFactory.Set_Strip_Path( true );

	WWDEBUG_SAY(( "Mix File Create .....\n" ));

	{
		MixFileCreator mix( "MAKEMIX.MIX" );
		Add_Files( "", mix );
	}

}
