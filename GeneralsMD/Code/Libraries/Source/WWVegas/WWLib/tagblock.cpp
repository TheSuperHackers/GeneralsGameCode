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

/* $Header: /G/wwlib/tagblock.cpp 5     11/30/99 3:46p Scott_b $ */
/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : WWLib                                                        * 
 *                                                                                             * 
 *                     $Archive:: /G/wwlib/tagblock.cpp                                       $* 
 *                                                                                             * 
 *                      $Author:: Scott_b                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 11/29/99 6:42p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 5                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   TagBlockFile::TagBlockFile -- Create/open tag file													  *
 *   TagBlockFile::~TagBlockFile -- Close down the tag file.                                   * 
 *   TagBlockFile::Create_Index -- Create a index into the IndexList sorted by CRC.            * 
 *   TagBlockFile::Find_Block -- Find block assocated with name.                               * 
 *   TagBlockFile::Open_Tag -- Open an existing tag block.                                     * 
 *   TagBlockFile::Create_Tag -- Create a new tag at the end of the block.                     * 
 *   TagBlockFile::Close_Tag -- Close the handle that Create or Open made.                     * 
 *   TagBlockFile::Destroy_Handle -- Shut down a handle.                                       * 
 *   TagBlockFile::End_Write_Access -- Stop write access for handle - flushes data bug keeps ha* 
 *   TagBlockFile::Reset_File -- Clear file so no blocks exist.                                * 
 *   TagBlockFile::Empty_Index_List -- Clear out tag block list in memory                      * 
 *---------------------------------------------------------------------------------------------* 
 *   TagBlockHandle::Write -- Write data to the block.                                         * 
 *   TagBlockHandle::Read -- Read from a tag block.                                            * 
 *   TagBlockHandle::Seek -- Seek within the file.                                             * 
 *   TagBlockHandle::~TagBlockHandle -- Destroy handle.                                        * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "TagBlock.h"
#include "realcrc.h"

#include <assert.h>

int TagBlockHandle::_InDestructor = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// Start of TagBlockIndex ///////////////////////////////////////
class TagBlockIndex
{
public:
	TagBlockIndex(const char *tagname, int blockoffset):
		CRC(CRC_Stringi(tagname)),
		BlockOffset(blockoffset),
		DataOffset(TagBlockFile::Calc_Data_Offset(blockoffset, tagname))
	{}

	unsigned Get_CRC()  {
		return(CRC);
	}
	int Get_BlockOffset()  {
		return(BlockOffset);
	}
	int Get_TagOffset()  {
		return(TagBlockFile::Calc_Tag_Offset(BlockOffset));
	}
	int Get_TagSize()  {
		return(DataOffset - Get_TagOffset());
	}
	int Get_DataOffset()  {
		return(DataOffset);
	}
private:

	// The index file is sorted by the CRC of the file name for
	// quicker retrieval.  The filename is saved in the texture file.
	unsigned long	CRC;

	// Start of the block - this is the start of TagBlockFile::BlockHeader.
	int				BlockOffset;
						
	// Offset of block inside of TagFile.  
	// This is first byte after header and TagName.
	// It is actual data used by external methods.
	int 				DataOffset;
};

///////////////////////////////////// End of TagBlockIndex /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Start of TagBlockHandle/////////////////////////////////////////


/*********************************************************************************************** 
 * RawFileClass -- Open up the tag file (it may not exist).                                    * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *        const char *fname - name of file that is or wants to be a TagBlockFile.              * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *        Will assume a file that has invalid data to be corrupt and will write over it.       * 
 *        So don't pass in a file that is not a tag file.                                      * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/11/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
TagBlockFile::TagBlockFile(const char *fname):
	RawFileClass(),
	Header(),
	CreateHandle(NULL),
	NumOpenHandles(0),
	IndexList()
{
	// Open file up, create it if it does not exist.
	// Pass in name to Open function so that the file name will be strdup'd.
	Open(fname, READ|WRITE);
	
	FileTime = RawFileClass::Get_Date_Time();

	// Read in header so we can tell if it is proper file.
	Read(&Header, sizeof(Header));

	// See if there is any data in file (or was it just created?)
	if (Header.Version == FILE_VERSION) {
		TagBlockIndex *lasttag = NULL;
		int curpos = sizeof(Header);

		// Loop through each block in file and create an in memory index for it.
		int block;
		for (block = 0; block < Header.NumBlocks; block++) {
			BlockHeader	blockheader;

			// Read in next header.
			Seek(curpos, SEEK_SET);
			Read(&blockheader, sizeof(blockheader));

			// Make sure things are in order.
			if (blockheader.Index == block) {
				char tagname[MAX_TAG_NAME_SIZE];

				// Read in tag name, this includes the NULL at end of string.
				Read(tagname, blockheader.TagSize);

				// Create new tag index for fast retrievel.
				lasttag = Create_Index(tagname, curpos);

				// Now get past the data.
				curpos = Calc_Tag_Offset(curpos) + blockheader.TagSize + blockheader.DataSize;
			} else {
				break;
			}
		}

		// See if there is a difference between the header and actual data.
		if ((curpos != Header.FileSize) || (block != Header.NumBlocks)) {
			Header.NumBlocks = block;
			Header.FileSize = curpos;

			// Start at begining of file and write out our new header.
			Seek(0, SEEK_SET);
			Write(&Header, sizeof(Header));
		} 

	} else {
		Reset_File();
	}
}	

/*********************************************************************************************** 
 * TagBlockFile::~TagBlockFile -- Close down the tag file.                                     * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *       Any TagBlockHandles that have not been deleted are now invalide but cannot be deleted.* 
 *       You must delete any handles associated with this before closing the TagFile.          * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/11/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
TagBlockFile::~TagBlockFile()
{
	Empty_Index_List();
}	

/*********************************************************************************************** 
 * TagBlockFile::Reset_File -- Clear file so no blocks exist.                                  * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   11/29/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
void TagBlockFile::Reset_File()
{
	Empty_Index_List();
												  
	// Save a clean header out.
	Header.Version = FILE_VERSION;
	Header.NumBlocks = 0;
	Header.FileSize = sizeof(Header);

	Save_Header();
			
	// Close, then open file so we get a new time stamp on it.									 
	Close();
	Open(READ|WRITE);

	// Reget file creation time.
	FileTime = RawFileClass::Get_Date_Time();
}	

/*********************************************************************************************** 
 * *TagBlockFile::Open_Tag -- Open an existing tag block.                                      * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/11/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
TagBlockHandle *TagBlockFile::Open_Tag(const char *tagname)
{
	// Find tag to open up.
	TagBlockIndex *index = Find_Block(tagname);
	if (!index) {
		return(NULL);
	}

	// Load up the block header information.
	BlockHeader *blockheader = W3DNEW BlockHeader();
	Seek(index->Get_BlockOffset(), SEEK_SET);
	Read(blockheader, sizeof(*blockheader));

	// Now that we have all that we need, create the 
	TagBlockHandle *handle = W3DNEW TagBlockHandle(this, index, blockheader);

	// Keep track of how many handles there are so we can assert if they are not all shut down.
	NumOpenHandles++;

	// Return with our new handle.
	return(handle);
}	

/*********************************************************************************************** 
 * *TagBlockFile::Create_Tag -- Create a new tag at the end of the block.                      * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/11/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
TagBlockHandle *TagBlockFile::Create_Tag(const char *tagname)
{
	// Only allow one handle to be creating open at a time.
	if (CreateHandle) {
		return(NULL);
	}

	// Create a new index that we can write too.
	TagBlockIndex *index = Create_Index(tagname, Header.FileSize);

	// An index may not be created if a tag of the same name already exists.
	if (!index) {
		return(NULL);
	}

	// Create a header.
	// Use -1 for index to indecate that block is not yet written out competely.
	BlockHeader *blockheader = W3DNEW BlockHeader(-1, index->Get_TagSize(), 0);

	// Write out the block header and the tag.
	Seek(index->Get_BlockOffset(), SEEK_SET);
	Write(blockheader, sizeof(*blockheader));
	Write(tagname, strlen(tagname) + 1);

	// Now that we have all that we need, create the 
	CreateHandle = W3DNEW TagBlockHandle(this, index, blockheader);

	// Keep track of how many handles there are so we can assert if they are not all shut down.
	NumOpenHandles++;

	return(CreateHandle);
}	

/*********************************************************************************************** 
 * TagBlockFile::Close_Tag -- Close the handle that Create or Open made.                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/12/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
void TagBlockFile::Close_Tag(TagBlockHandle *handle)
{
	delete handle;
}

/*********************************************************************************************** 
 * TagBlockFile::Destroy_Handle -- Shut down a handle.                                         * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/12/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
void TagBlockFile::Destroy_Handle(TagBlockHandle *handle)
{
	// Make sure those sneaky programmers aren't trying to fool me.
	assert(handle->Called_By_Destructor());

	// If we had write access to handle, flush it out.
	End_Write_Access(handle);

	// This was allocated by TagBlockFile::Create_Tag() or Open_Tag().
	delete handle->BlockHeader;

	// Keep track of how many handles there are so we can assert if they are not all shut down.
	NumOpenHandles--;
}	

/*********************************************************************************************** 
 * TagBlockFile::End_Write_Access -- Stop write access for handle - flushes data bug keeps han * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/02/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
int TagBlockFile::End_Write_Access(TagBlockHandle *handle)
{
	// Make sure this handle is the proper one.
	if (CreateHandle == handle) {
		// Update file header and block header.
		handle->BlockHeader->Index = Header.NumBlocks;
		Header.NumBlocks++;
		Header.FileSize = handle->Index->Get_DataOffset();
		Header.FileSize += handle->BlockHeader->DataSize;

		// Write both headers out.
		Seek(handle->Index->Get_BlockOffset(), SEEK_SET);
		Write(handle->BlockHeader, sizeof(*handle->BlockHeader));
		Save_Header();

		// Don't allow writing with this handle anymore.
		CreateHandle = NULL;
		return(true);
	}
	return(false);
}	


/*********************************************************************************************** 
 * *TagBlockFile::Create_Index -- Create a index into the IndexList sorted by CRC.             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/11/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
TagBlockIndex *TagBlockFile::Create_Index(const char *tagname, int blockoffset)
{
	// Don't allow duplicate tags.
	if (Find_Block(tagname)) {
		return(NULL);
	}

	TagBlockIndex *index;
	index = W3DNEW TagBlockIndex(tagname, blockoffset);

	// Put it into the list.
	if (IndexList.Is_Empty()) {
		IndexList.Add_Head(index);
	} else {
		// Put in list sorted by CRC, smallest to largest.
		SLNode<TagBlockIndex> *node = IndexList.Head();
		while (node) {
			TagBlockIndex *cur = node->Data();
			if (index->Get_CRC() > cur->Get_CRC()) {
				IndexList.Insert_Before(index, cur);
				break;
			}
			node = node->Next();
		}
		// If we did not find a place, then add at end of list.
		if (!node) {
			IndexList.Add_Tail(index);
		}
	}
	return (index);
}	

/*********************************************************************************************** 
 * *TagBlockFile::Find_Block -- Find block assocated with name.                                * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/11/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
TagBlockIndex *TagBlockFile::Find_Block(const char *tagname)
{
	if (IndexList.Is_Empty()) {
		return(NULL);
	}

	unsigned long crc = CRC_Stringi(tagname);

	SLNode<TagBlockIndex> *node = IndexList.Head();
	while (node) {
		TagBlockIndex *cur = node->Data();

		// Did we find it?
		if (cur->Get_CRC() == crc) {
			// Now read from file to verify that it is the right name.
			char name[MAX_TAG_NAME_SIZE];
			Seek(cur->Get_TagOffset(), SEEK_SET);

			// Read in the name.
			Read(name, cur->Get_TagSize());

			// Is it a match?
         assert(name != NULL);
         assert(tagname != NULL);
			if (!strcmpi(name, tagname)) {
				return(cur);
			}
		}

		// Are we out of range?
		if (cur->Get_CRC() < crc) {
			break;
		}

		// Next in line please.
		node = node->Next();
	}

	return(NULL);
}	

						  
/*********************************************************************************************** 
 * TagBlockFile::Empty_Index_List -- Clear out tag block list in memory                      * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   11/29/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
void TagBlockFile::Empty_Index_List()
{	
	assert(!NumOpenHandles);	

	// Get rid of index list in memory.
	while (!IndexList.Is_Empty()) {
		TagBlockIndex *index = IndexList.Remove_Head();
		delete index;
	}
	
}	


///////////////////////////////////////// End of TagBlockFile /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// Start of TagBlockHandle/////////////////////////////////////

/*********************************************************************************************** 
 * Position -- Create a handle for user to access the TagBlock.                                * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/12/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
TagBlockHandle::TagBlockHandle(TagBlockFile *tagfile, TagBlockIndex *tagindex, TagBlockFile::BlockHeader *blockheader):
	File(tagfile),
	Index(tagindex),
	BlockHeader(blockheader),
	Position(0)
{
}	

/*********************************************************************************************** 
 * TagBlockHandle::~TagBlockHandle -- Destroy handle.                                          * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/12/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
TagBlockHandle::~TagBlockHandle()
{
	_InDestructor++;
	File->Destroy_Handle(this);
	_InDestructor--;
}	

/*********************************************************************************************** 
 * TagBlockHandle::Write -- Write data to the block.                                           * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/12/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
int TagBlockHandle::Write(const void *buf, int nbytes)
{
	// Make sure this handle is the proper one.
	if (!File->Handle_Can_Write(this)) {
		return(-1);
	}

	// Get to correct position to write out and write the buffer.
	File->Seek(Index->Get_DataOffset() + Position, SEEK_SET);
	nbytes = File->Write(buf, nbytes);

	// Advance the EOF marker for the block.
	Position += nbytes;
	if (Position > BlockHeader->DataSize) {
		BlockHeader->DataSize = Position;
	}

	// Return about written out.
	return(nbytes);
}	

/*********************************************************************************************** 
 * TagBlockHandle::Read -- Read from a tag block.                                              * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/12/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
int TagBlockHandle::Read(void *buf, int nbytes)
{
	// Make sure user does not read past end of buffer.
	if ((Position + nbytes) > BlockHeader->DataSize) {
		nbytes = BlockHeader->DataSize - Position;
	}

	// Get to correct position to write out and read in the buffer.
	File->Seek(Index->Get_DataOffset() + Position, SEEK_SET);
	nbytes = File->Read(buf, nbytes);

	// Adjust the read head.
	Position += nbytes;

	// Tell user how much was read from the file.
	return(nbytes);
}	

/*********************************************************************************************** 
 * TagBlockHandle::Seek -- Seek within the file.                                               * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/12/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
int TagBlockHandle::Seek(int pos, int dir)
{
	switch (dir) {
		case SEEK_CUR:
			Position += pos;
			break;
		case SEEK_SET:
			Position = pos;
			break;
		case SEEK_END:
			Position = BlockHeader->DataSize + pos;
			break;
	}
	return(Position);
}	
		
// EOF