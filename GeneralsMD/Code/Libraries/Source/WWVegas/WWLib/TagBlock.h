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

/* $Header: /VSS_Sync/wwlib/tagblock.h 6     10/17/00 4:48p Vss_sync $ */
/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : WWLib                                                        * 
 *                                                                                             * 
 *                     $Archive:: /VSS_Sync/wwlib/tagblock.h                                  $* 
 *                                                                                             * 
 *                      $Author:: Vss_sync                                                    $* 
 *                                                                                             * 
 *                     $Modtime:: 10/16/00 11:42a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 6                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef TAGBLOCK_H
#define TAGBLOCK_H

#include "SLIST.H"
#include "CRC.H"
#include "RAWFILE.H"

#include <string.h>

class TagBlockHandle;
class TagBlockIndex;

  //////////////////////////////////////////////////////////////////////////////////////////////////
 //////////////////////////////////// Start of TagBlockHandle///////////////////////////////////////
// 	TagBlockFile: Enables a file to have named (tagged) variable size blocks.  User can
// then open a block for reading.  User may also create new blocks at the end of the
// file.  There may only be one block open for writting, unlimited blocks can be open for 
// reading.  It can be thought of as a .MIX file that can have files added to it at any time.
// One problem is it is a Write Once/Read Many solution.
//
//		Usage: All user access to a TagBlockFile is done through TagBlockHandle.
// A TagBlockHandle is created by TagBlockFile::Create_Tag() or Open_Tag().  It is destroyed
// by either TagBlockFile::Close_Tag() or you can just destroy the handle with delete().
//	
//	

class TagBlockFile : protected RawFileClass
{
	public:
		//////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////// Public Member Functions ///////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////

		// Open up the tag file.  It may or may not exist.
		TagBlockFile(const char *fname);
		virtual ~TagBlockFile();
								
		// DANGEROUS!!  Resets entire file and makes small															
		virtual void Reset_File();

		// Creation of a Handle so block can be crated/writen/read.
		// Use delete to destroy handle or use Close_Tag().
		// Open_Tag() returns NULL if tag not found.
		// Create_Tag() returns NULL if tag already exists.
		TagBlockHandle *Open_Tag(const char *tagname);
		TagBlockHandle *Create_Tag(const char *tagname);
		void Close_Tag(TagBlockHandle *handle);

		int Does_Tag_Exist(const char *tagname)  {
			return(Find_Block(tagname) != NULL);
		}						
		
		virtual unsigned long Get_Date_Time(void)  {
			return(FileTime);
		}

		// Methods to figure an offset of the tag name and the data
		// given the offset of the start of the block (BlockHeader)..
		static int Calc_Tag_Offset(int blockoffset)  {
			return(blockoffset + sizeof(BlockHeader));
		}
		static int Calc_Data_Offset(int blockoffset, const char *tagname)  {
			return(Calc_Tag_Offset(blockoffset) + strlen(tagname) + 1);
			
		}
	protected:
		///////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////// Supporting Structures /////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////
		enum {
			// Put in date when format is changed.
			FILE_VERSION 		= 20000626,
			MAX_TAG_NAME_SIZE = 1024,
		};

		// This is the header that is in both the IndexFile and the DataFile.  
		// They should be match except for the difference in the Version number as defined by enum.
		struct FileHeader 
		{
			FileHeader()  {memset(this, 0, sizeof(*this));}

			// Version number to make sure that it we are compatable and also to 
			// verify that this is the file we think it is.
			unsigned 		Version;

			// Number of blocks in file.
			int				NumBlocks;

			// This is how much data is actually valid in the file.  There may be
			// extra room at end of file if file size is preset or the file is corrupt.
			int				FileSize;
		};

		// Each block in the file has a header before it.
		struct BlockHeader
		{
			BlockHeader()  {memset(this, 0, sizeof(*this));}
			BlockHeader(int index, int tagsize, int datasize):Index(index),TagSize(tagsize),DataSize(datasize) {}
			BlockHeader(BlockHeader& bh)  {memcpy(this, &bh, sizeof(BlockHeader));}

			// Used to verify file integrity.
			int				Index;

			// Size of tagname (including NULL) that follows this block.
			int				TagSize;

			// Size of block not including header.
			int				DataSize;

			// A variable length name (NULL terminated) follows this structure.
			// The name is then followed by the Data.

			// The entire length of the block is sizeof(BlockHeader) + TagSize + DataSize.
		};

	protected:
		///////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////// Member Data Fields ////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////

		// This is the data at the start of the file.
		FileHeader 			Header;

		// Only one handle has permission to write to the end of the file if any.
		// This is a pointer to that handle.  
		TagBlockHandle		*CreateHandle;

		// To help those stupid programmers from leaving open handles when
		// this file is closed down.
		int					NumOpenHandles;
										  
		// Last time file was written to before we opened it.
		unsigned long 		FileTime;

		// Keep list of all blocks in file.  This list is sorted by CRC value.
		// TagBlockIndex is defined in TagBlock.cpp.
		SList<TagBlockIndex> IndexList;

	protected:
		///////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////// Protected Member Functions ////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////
		// Search for block given tag.
		TagBlockIndex *Find_Block(const char *tagname);

		// Create an index that can be used for seaching.
		TagBlockIndex *Create_Index(const char *tagname, int blockoffset);

		// Is this the handle that has creation priveledges?
		int Handle_Can_Write(TagBlockHandle *handle)  {
			return(CreateHandle == handle);
		}

		// Called only by ~TagBlockHandle!
		void Destroy_Handle(TagBlockHandle *handle);

		// Stop write access - flushes data out but keeps handle available for reading.
		int End_Write_Access(TagBlockHandle *handle);

		// Save the header when it has been updated.
		void Save_Header()  {
			Seek(0, SEEK_SET);
			Write(&Header, sizeof(Header));
		}		  
		
		void Empty_Index_List();

	friend class TagBlockHandle;
};

///////////////////////////////////////// End of TagBlockFile /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// Start of TagBlockHandle/////////////////////////////////////
// All external access to the TagBlockFile is done through handles.
class TagBlockHandle
{
	public:
		// Access functions.
		int Write(const void *buf, int nbytes);
		int Read(void *buf, int nbytes);
		int Seek(int pos, int dir = SEEK_CUR);

		// Stop write access - flushes data out but keeps handle available for reading.
		int End_Write_Access() {
			return (File->End_Write_Access(this));
		}

		int Tell() {
			return(Position);
		}

		int Get_Data_Size()  {
			return(BlockHeader->DataSize);
		}

		// User must call TagBlockFile::New_Handle() to create a TagBlockHandle object.
		// User may call this delete to destry handle or 
		// he may call TagBlockFile::Close_Tag().
		~TagBlockHandle();

	private:

		// Pointer to parent file object.
		TagBlockFile						*File;

		// Pointer to index for aditional information.
		TagBlockIndex						*Index;

		// Keep header infomation in memory so that it can be updated.
		TagBlockFile::BlockHeader		*BlockHeader;

		// Current postion we are in the file.
		int									Position;

	private:
		// User must call TagBlockFile::New_Handle() to create a TagBlockHandle object.
		// The constructor is private so only TagBlockFile can create the handle.
		// This is so that a handle will not be created if the TagBlock
		// does not exist on a CREAD or if there was already a WRITE access granted.
		// User needs to call detete to destroy the handle.
		TagBlockHandle(TagBlockFile *tagfile, TagBlockIndex *tagindex, TagBlockFile::BlockHeader *blockheader);
		friend class TagBlockFile;

		// Used to prevent TagBlockFile::Destroy_Handle() from being called
		// except by this destructor.
		static	int _InDestructor;

		int Called_By_Destructor()  {
			return(_InDestructor);
		}
};


////////////////////////////////////// End of TagBlockHandle///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif //TAGBLOCK_H
