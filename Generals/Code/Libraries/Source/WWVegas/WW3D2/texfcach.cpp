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

/* $Header: /Commando/Code/ww3d2/texfcach.cpp 5     8/24/01 3:23p Jani_p $ */
/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : WW3D                                                         * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/ww3d2/texfcach.cpp                           $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 8/24/01 11:50a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 5                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   TextureFileCache::TextureFileCache -- Open cache.                              				* 
 *   ~TextureFileCache::TextureFileCache -- Shut down texture cache system.         				* 
 *   TextureFileCache::Save_Texture -- Save the texture into the cache.     							* 
 *   TextureFileCache::Load_Texture -- Load texture from cache into surface.							* 
 *   *TextureFileCache::_Create_File_Name -- Create a file name from prefix 							* 
 *   *TextureFileCache::Load_Original_Texture_Surface -- Create the initial							* 
 *   *TextureFileCache::Open_Texture_Handle -- Set the TextureHandle and Header..            * 
 *   TextureFileCache::Close_Texture_Handle -- Close the current texture so we can open anoth* 
 *   TextureFileCache::Read_Texture -- Read in the texture into surface buffer.              * 
 *   *TextureFileCache::Create_First_Texture_As_Surface -- Load first texture into a surface.  * 
 *   *TextureFileCache::Find_Cached_Surface -- Search for a texture already cached.            * 
 *   TextureFileCache::Add_Cached_Surface -- Add a new cached texture.                         * 
 *   *TFC::Get_Surface -- Load a texture reduced N times.                                      * 
 *   TextureFileCache::Reset_File -- virtual function to reset file and write out file.        * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifdef WW3D_DX8
#include "texfcach.h"
#include "mutex.h"
#include "thread.h"
#include <assert.h>
#include "wwdebug.h"
#include "simplevec.h"
#include "wwstring.h"
#include "textureloader.h"
#include "texture.h"
#include "ffactory.h"

#include <srSurfaceIOManager.hpp>
#include <srExtension.hpp>
#include <srCore.hpp>
#include <srBinIStream.hpp>
#include <srColorSurface.hpp>
#include <srTextureIFace.hpp>
						  
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _UNIX
#include "osdep.h"
#endif

#define FILE_HEADER_NAME	"Texture File Cache Header"


char  *TextureFileCache::_FileNamePtr = NULL;
static int Instances=0;

static CriticalSectionClass mutex(0);
static SimpleVecClass<char> compression_buffer;

const char BUFFER_OVERRUN_TEST_VALUE=((char)0x7d);

static char* Get_Compression_Buffer(int size)
{
	compression_buffer.Uninitialised_Grow(size+1);
	compression_buffer[size]=BUFFER_OVERRUN_TEST_VALUE;
	return &(compression_buffer[0]);
}

static void Verify_Compression_Buffer()
{
	WWASSERT(compression_buffer[compression_buffer.Length()-1]==BUFFER_OVERRUN_TEST_VALUE);
}


class Compressor
{
public:

	static int Compress
	(
		const unsigned char *	in,
		unsigned int					in_len,
		unsigned char *				out,
		unsigned int *				out_len
	);

	static int Decompress
	(
		const unsigned char *	in,
		unsigned int					in_len,
		unsigned char *				out,
		unsigned int *				out_len
	);

};

int Compressor::Compress( const unsigned char * in, unsigned int in_len,
												 unsigned char * out, unsigned int * out_len )
{
	if (!in || !out || !out_len)
		return FALSE;

	if (in_len <= 0)
		return false;

	memcpy(out, in, in_len);

	*out_len = in_len;

	return TRUE;
}

int Compressor::Decompress( const unsigned char * in, unsigned int in_len,
													 unsigned char * out, unsigned int * out_len )
{
	if (!in || !out || !out_len)
		return FALSE;

	if (in_len <= 0)
		return false;

	memcpy(out, in, in_len);

	*out_len = in_len;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// TextureFileCache /////////////////////////////////////////////

/*********************************************************************************************** 
 * TextureFileCache::TextureFileCache -- Open cache.                                 * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/07/1999 SKB : Created.                                                                 * 
 *   06/27/2000 SKB : added CachedSurfaces                                                     * 
 *   08/14/2000     : Check for revision number.                                               * 
 *=============================================================================================*/
TextureFileCache::TextureFileCache(const char *fileprefix):
	File(_Create_File_Name(fileprefix)),
	CurrentTexture(NULL),
	TextureHandle(NULL),
	Header(),
	CachedSurfaces(),
	Offsets(NULL),
	NumCachedTextures(0)
{
	WWASSERT(!Instances);
	Instances++;

	// This was allocated by _Create_File_Name() and need to go away now.
	delete _FileNamePtr;
	_FileNamePtr = NULL;

	memset(CachedSurfaces, 0, sizeof(CachedSurfaces));

	bool reset = false;
	TagBlockHandle	*handle = File.Open_Tag(FILE_HEADER_NAME);
	if (handle) {
		FileHeader fileheader;

		// Read in header for others to use.
		handle->Read(&fileheader, sizeof(fileheader));

		if (fileheader.Version != FileHeader::TCF_VERSION) {
			reset = true;
		}

		// Close down handle.
		delete handle;
	} else {
		reset = true;
	}

	if (reset) {
		Reset_File();
	}
}	

/*********************************************************************************************** 
 * *TextureFileCache::_Create_File_Name -- Create a file name from prefix passed in.           * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *    Caller of this function must free _FileNamePtr when done with it.                        * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/13/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
char *TextureFileCache::_Create_File_Name(const char *fileprefix)
{	
	bool addpath = (*fileprefix != '\\' && fileprefix[1] != ':');

	assert(!_FileNamePtr);
	_FileNamePtr = W3DNEWARRAY char[strlen(fileprefix) + (addpath ? 256 : 6)];
			
   char path[_MAX_PATH];
   if (addpath && _getcwd(path, _MAX_PATH ))  {
		sprintf(_FileNamePtr, "%s\\%s.tfc", path, fileprefix);
   } else {
		// Create a file name.
		strcpy(_FileNamePtr, fileprefix);
		strcat(_FileNamePtr, ".tfc");
		*_FileNamePtr = 0;
	}

	return(_FileNamePtr);
}	


/*********************************************************************************************** 
 * ~TextureFileCache::TextureFileCache -- Shut down texture cache system.            * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/07/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
TextureFileCache::~TextureFileCache()
{
	// Make sure we have shut down everything.
	Close_Texture_Handle();

	Instances--;
	WWASSERT(!Instances);
}	


/************************************************************************** 
 * TextureFileCache::Reset_File -- virtual function to reset file and wri * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   08/14/2000     : Created.                                            * 
 *========================================================================*/
void TextureFileCache::Reset_File()
{
	File.Reset_File();

	TagBlockHandle	*handle = File.Create_Tag(FILE_HEADER_NAME);
	if (handle) {
		FileHeader fileheader;
		fileheader.Version = FileHeader::TCF_VERSION;

		// Read in header for others to use.
		handle->Write(&fileheader, sizeof(fileheader));

		// Close down handle.
		delete handle;
	} else {
		assert(false);
	}
}	

/*********************************************************************************************** 
 * TextureFileCache::Save_Texture -- Save the texture into the cache.                 * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/12/1999 SKB : Created.                                                                 * 
 *   06/27/2000 SKB : added Header.PixelFormat                                                 * 
 *   06/27/2000 SKB : changed cached textures to surfaces                                      * 
 *   08/14/2000     : save file's datatime stamp.                                              * 
 *=============================================================================================*/
bool TextureFileCache::Save_Texture(const char *texturename, srTextureIFace::MultiRequest& mreq, srColorSurfaceIFace& origsurface)
{
	CriticalSectionClass::LockClass m(mutex);
	int idx;
	unsigned lod;
	
	// Open up our new texture.
	Open_Texture_Handle(texturename);

	// Create a new texture now.
	TextureHandle = File.Create_Tag(texturename);
	if (!TextureHandle) {
		Close_Texture_Handle();
		return(false);
	}

	// Setup the Header.
	FileClass *asset=_TheFileFactory->Get_File(texturename);
	WWASSERT( asset );
	asset->Open();
	Header.FileTime = asset->Get_Date_Time();
	Header.NumMipMaps = (mreq.smallLOD - mreq.largeLOD) + 1;
	Header.LargestWidth =  mreq.levels[mreq.largeLOD]->getWidth();
	Header.LargestHeight = mreq.levels[mreq.largeLOD]->getHeight();
	Header.SourceWidth = origsurface.getWidth();
	Header.SourceHeight = origsurface.getHeight();
	mreq.levels[mreq.largeLOD]->getPixelFormat(Header.PixelFormat);
	origsurface.getPixelFormat(Header.SourcePixelFormat);

	_TheFileFactory->Return_File(asset);
	asset=NULL;

	// Write it out.
	TextureHandle->Write(&Header, sizeof(Header));

	// Setup offset table.
	Offsets = W3DNEWARRAY OffsetTableType[Header.NumMipMaps + 1];

	// Write for now, but we will need to seek back to write final data.
	int tableoffset = TextureHandle->Tell();
	TextureHandle->Write(Offsets, sizeof(OffsetTableType) * (Header.NumMipMaps + 1));

	// Now write out the textures.
	for (idx = 0, lod = mreq.largeLOD; lod <= mreq.smallLOD; idx++, lod++) {
		srColorSurface *surface = mreq.levels[lod];
		WWASSERT(surface->getDataPtr());

		Offsets[idx].Offset = TextureHandle->Tell();
		Offsets[idx].Size = surface->getDataSize();

		// Save data pointers so we don't need to read them from disk next time.
		Add_Cached_Surface(surface);

		int compsize,retcode;

		int buf_size=surface->getDataSize();
		retcode = Compressor::Compress(	(const unsigned char *)	surface->getDataPtr(), 
														(unsigned int) 		  		surface->getDataSize(), 
														(unsigned char *) 			Get_Compression_Buffer(buf_size),
														(unsigned *)			&compsize);


		// Lots-o-test to make sure that the compression did what we want.
		assert(retcode == TRUE);
		Verify_Compression_Buffer();

		int readin = TextureHandle->Write(Get_Compression_Buffer(compsize), compsize);
		assert(readin == compsize);
	}

	int pos = TextureHandle->Seek(0, SEEK_END);

	// Set last one so we can get a compressed size of last texture.
	Offsets[idx].Offset = TextureHandle->Tell();
	Offsets[idx].Size = 0;

	// Now write out header for good.
	TextureHandle->Seek(tableoffset, SEEK_SET);
	TextureHandle->Write(Offsets, sizeof(OffsetTableType) * (Header.NumMipMaps + 1));

	pos = TextureHandle->Seek(0, SEEK_END);

	// End write access so that it gets flushed out to disk.
	TextureHandle->End_Write_Access();

	return (true);
}	

/*********************************************************************************************** 
 * *TextureFileCache::Load_Original_Texture_Surface -- Create the initial surface that would  * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *     Sureface is only needed to get default values.  The texel data does not get filled in.  * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/13/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
srColorSurfaceIFace *TextureFileCache::Load_Original_Texture_Surface(const char *texturename)
{
	CriticalSectionClass::LockClass m(mutex);
	if (Open_Texture_Handle(texturename)) {
		// Create surface that we wish to return.
		srColorSurfaceIFace *surface = W3DNEW srColorSurface(Header.SourcePixelFormat, Header.SourceWidth, Header.SourceHeight);
		return(surface);
	}
	return(NULL);
}	

/*********************************************************************************************** 
 * *TFC::Get_Surface -- Load a texture reduced N times.                                        * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/2000 SKB : Created.                                                                 * 
 *=============================================================================================*/
srColorSurfaceIFace *TextureFileCache::Get_Surface(const char *texturename, unsigned int reduce_factor)
{
	CriticalSectionClass::LockClass m(mutex);

	// If we can't get a handle, then we can't load the texture.
	if (!Open_Texture_Handle(texturename))  {
		return(0);
	}

	if (reduce_factor >= (unsigned int)(Header.NumMipMaps)) {
		reduce_factor=Header.NumMipMaps-1;
	}

	// Figure out the width and height of texture.
	int width = Header.LargestWidth >> reduce_factor;
	int height = Header.LargestHeight >> reduce_factor;
	if (!width) width = 1;
	if (!height) height = 1;

	// Create surface to return.
	srColorSurface *surface = W3DNEW srColorSurface(Header.PixelFormat, width, height);
	int size = Texture_Size(reduce_factor);
	assert(size == surface->getDataSize());

	// See if this texture is already in memory.
	srColorSurface *cached = Find_Cached_Surface(size);
	if (cached) {
		assert(size == cached->getDataSize());
		memcpy(surface->getDataPtr(), cached->getDataPtr(), size);
	} else {
		// Cache pointer so if we need texture again, we will have a pointer to
		// valid texture data instead of reading it from the file.
		Add_Cached_Surface(surface);

		// Read texture in - note that Textures[lod] has to be set prior to this.
		Read_Texture(reduce_factor, surface);
	}
	return(surface);
}	

/*********************************************************************************************** 
 * TextureFileCache::Load_Texture -- Load texture from cache into surface.                     * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/12/1999 SKB : Created.                                                                 * 
 *   06/02/1999 SKB : deal with texture requests larger then what is in cache.                 * 
 *   06/27/2000 SKB : changed cached textures to surfaces                                      * 
 *=============================================================================================*/
bool TextureFileCache::Load_Texture(const char *texturename, srTextureIFace::MultiRequest& mreq)
{
	CriticalSectionClass::LockClass m(mutex);
	// If we can't get a handle, then we can't load the texture.
	if (!Open_Texture_Handle(texturename))  {
		return(false);
	}

	// NOTE: Mip maps that are saved in the cache are the first mip maps requested and created
	//			for a given texture name.  If a user susequently requests mip maps of different 
	// 		sizes that are not in the cache, they will be created by use of surface copy functions.
	//			When this happens it is unfortunate because it indicates some sort of inefficiency.
	//			In code below I have put in comments 'A WASTE' to indicate what part of code deels with it.
	int idx;
	unsigned lod;

	// Get largest size of texture in file (idx) and what is required (lod).
	int idxsize = Texture_Size(0);
	int lodsize = -1;

	// Now skip any mip maps that we want but are not in the cache. (A WASTE).
	for (lod = mreq.largeLOD; lod <= mreq.smallLOD; lod++) {	
		lodsize = mreq.levels[lod]->getDataSize();
		if (lodsize <= idxsize)  {
			break;
		}
	}
	// make sure largeLOD <= smallLOD.
	assert(lodsize != -1);

	// Skip past all mipmap levels that we do not want in the file.
	for (idx = 0; idx < Header.NumMipMaps; idx++) {
		idxsize = Texture_Size(idx);
		if (idxsize <= lodsize)  {
			break;
		}
	}

	// Save off the first lod and what index it is.
	unsigned firstlod = lod;

	// Now if we have in the file something that the texture needs, load it. (This is most desired.)
	if (idxsize == lodsize) {
		// Read in textures that we have and need.
		while ((lod <= mreq.smallLOD) && (idx < Header.NumMipMaps)) {
			srColorSurface *surface = mreq.levels[lod];
			int size = surface->getDataSize();						assert(Texture_Size(idx) == size);
				 		  
			// See if we can find the texture already cached.
			srColorSurface *cached = Find_Cached_Surface(size);

			// See if texture has already been read in.
			if (cached) {
				assert(size == cached->getDataSize());

				// Copy data over using size of dest surface incase asserts disabled and sizes are not right.
				memcpy(surface->getDataPtr(), cached->getDataPtr(), size);
			} else {
				// Cache pointer so if we need texture again, we will have a pointer to
				// valid texture data instead of reading it from the file.
				Add_Cached_Surface(surface);

				// Read texture in - note that Textures[lod] has to be set prior to this.
				Read_Texture(idx, surface);
			}

			idx++, lod++;
		}
	}

	// Save last lod.
	unsigned lastlod = lod - 1;

	// largest surface loaded.
	srColorSurfaceIFace *surface = NULL; 
	if (firstlod < lastlod) {
		surface = mreq.levels[firstlod];
		surface->addReference();
	}

	// If user requested a texture larger then what we have saved, then
	// we will have to scale it up for him.
	// NOTE: This is normally not desired.  It typically means the user is
	// 		creating a texture larger then the original source - a waste.
	if (mreq.largeLOD < firstlod)  {
		// If there has not been a surface loaded yet, then do so now.
		if (!surface) {
			surface = Create_First_Texture_As_Surface(mreq.levels[mreq.largeLOD]);
		}

		// Now scale the largest mip map we had up to what user wants (ONCE AGAIN: A WASTE).
		for (lod = mreq.largeLOD; lod <= firstlod; lod++) {
			// Do a surface scale.
			surface->copy(*mreq.levels[lod]);
		}
	}

	// Are there more smaller lod's that are not in the cache?
	if (lastlod < mreq.smallLOD) {
		// If there has not been a surface loaded yet, then do so now.
		if (!surface) {
			surface = Create_First_Texture_As_Surface(mreq.levels[mreq.largeLOD]);
		}

		// Now scale the largest mip map we had up to what user wants (ONCE AGAIN: A WASTE).
		for (lod = lastlod + 1; lod <= mreq.smallLOD; lod++) {
			// Do a surface scale.
			surface->copy(*mreq.levels[lod]);
		}
	}

	// Done with this one.
	if (surface) {
		surface->release();
	}

	return (true);
}	

/*********************************************************************************************** 
 * TextureFileCache::Read_Texture -- Read in the texture into surface buffer.                  * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/01/1999 SKB : Created.                                                                 * 
 *   06/27/2000 SKB : changed cached textures to surfaces                                      * 
 *=============================================================================================*/
void TextureFileCache::Read_Texture(int offsetidx, srColorSurface *surface)
{
	// Seek to correct spot.
	TextureHandle->Seek(Offsets[offsetidx].Offset, SEEK_SET);

	// Figure out how much read in and make sure it is correct size.
	int compsize = Compressed_Texture_Size(offsetidx);

	// Read in texture from cache.
	int readin = TextureHandle->Read(Get_Compression_Buffer(compsize), compsize);
	assert(readin == compsize);

	// Set values so we can assert if overrun.
	Verify_Compression_Buffer();

	// Decompress into texture pointer.
	int retcode, decompsize;
	retcode = Compressor::Decompress(	(const unsigned char*)		Get_Compression_Buffer(compsize),
														(unsigned int)				compsize, 
														(unsigned char*)				surface->getDataPtr(),
														(unsigned *)			&decompsize);

	// Lots-o-test to make sure that the compression did what we want.
	assert(retcode == TRUE);
	assert(decompsize == Texture_Size(offsetidx));
	Verify_Compression_Buffer();
}	

/*********************************************************************************************** 
 * *TextureFileCache::Create_First_Texture_As_Surface -- Load first texture into a surface.    * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *  	srColorSurfaceIFace *surfacetype - only used to create surface type we want.				  *
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/02/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
srColorSurfaceIFace *TextureFileCache::Create_First_Texture_As_Surface(srColorSurfaceIFace *surfacetype)
{
	srColorSurfaceIFace::PixelFormat	pf;

	// Get our pixel format.
	surfacetype->getPixelFormat(pf);

	// Create a surface we can load the largest texture into it
	srColorSurface *surface = W3DNEW srColorSurface(pf, Header.LargestWidth, Header.LargestHeight);
	assert(Texture_Size(0) == surface->getDataSize());

	// Read in texture to our surface data..
	Read_Texture(0, surface);

	return(surface);
}	

/*********************************************************************************************** 
 * *TextureFileCache::Open_Texture_Handle -- Set the TextureHandle and Header..             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/13/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
bool TextureFileCache::Open_Texture_Handle(const char *fname)
{
	if (TextureHandle) {
		assert(CurrentTexture);
		if (!strcmpi(fname, CurrentTexture)) {
			return(true);
		}
		// Wrong texture, close it down so we can open another.
		Close_Texture_Handle();
	}
	if (!CurrentTexture) {
		CurrentTexture = _strdup(fname);
	}			  
			
	#if 0					 
	// If this is texture is newer then our texture file, reset file.
	// This means that next time the texture cache file gets opened, all the
	// previous textures will be loaded.
	// SKB 8/14/2000 : Removed to put in better system below..
	RawFileClass asset(fname);
	asset.Open();
	if (asset.Get_Date_Time() > File.Get_Date_Time()) {
		Reset_File();
	}
	#endif
	
	// See if we have the texture in the file cache yet.
	TextureHandle = File.Open_Tag(fname);
	if (TextureHandle) {
		// Read in header for others to use.
		TextureHandle->Read(&Header, sizeof(Header));

		file_auto_ptr asset(_TheFileFactory, fname);
		asset->Open();

		// Make sure it is same file.
		if (Header.FileTime != asset->Get_Date_Time()) {

			delete TextureHandle;
			TextureHandle = NULL;

			Reset_File();
			return(false);
		}

		// Load up the offset table.
		Offsets = W3DNEWARRAY OffsetTableType[Header.NumMipMaps + 1];
		TextureHandle->Read(Offsets, sizeof(OffsetTableType) * (Header.NumMipMaps + 1));

		return(true);
	}

	return(false);
}	

/*********************************************************************************************** 
 * TextureFileCache::Close_Texture_Handle -- Close the current texture so we can open another. * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/01/1999 SKB : Created.                                                                 * 
 *=============================================================================================*/
void TextureFileCache::Close_Texture_Handle()
{
	if (CurrentTexture) {
		free(CurrentTexture);
		CurrentTexture = NULL;

		if (TextureHandle) {
			delete TextureHandle;
			TextureHandle = NULL;
		}
		while (NumCachedTextures--) {
			assert(CachedSurfaces[NumCachedTextures]);
			CachedSurfaces[NumCachedTextures]->release();
			CachedSurfaces[NumCachedTextures] = 0;
		}
		NumCachedTextures = 0;

		if (Offsets) {
			delete[] Offsets;
			Offsets = NULL;
		}
	} else {
		assert(!CurrentTexture);
	}
}	

/*********************************************************************************************** 
 * *TextureFileCache::Find_Cached_Surface -- Search for a texture already cached.              * 
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
srColorSurface *TextureFileCache::Find_Cached_Surface(int size)
{			
	// Search through each allocated record for the right size.
	for (int idx = 0; idx < NumCachedTextures; idx++) {
		if (CachedSurfaces[idx]->getDataSize() == size) {
			return(CachedSurfaces[idx]);
		}
	}				 
	return(NULL);
}	
								  
/*********************************************************************************************** 
 * TextureFileCache::Add_Cached_Surface -- Add a new cached texture.                           * 
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
void TextureFileCache::Add_Cached_Surface(srColorSurface *surface)
{		
//	return;
	assert(!Find_Cached_Surface(surface->getDataSize()));
			 															
	// If are cache is full, use the smallest one's space.
	int surface_idx = -1;
	if (NumCachedTextures == MAX_CACHED_SURFACES)	{
		// Make sure there are some cached textures.
		// Assume first is smallest.
		int smallsize = 0xfffffff;
		int smallidx = -1;
							 
		// Look for any others that might be smaller.
		for (int idx = 0; idx < NumCachedTextures; idx++) {
			int size = CachedSurfaces[idx]->getDataSize();
			if (size < smallsize) {
				// Found one.
				smallsize = size;
				smallidx = idx;
			}
		}
		surface_idx = smallidx;
		CachedSurfaces[idx]->release();
		CachedSurfaces[idx] = 0;
	} else {						
		// Use next slot in array.
		assert(NumCachedTextures < MAX_CACHED_SURFACES);
		surface_idx = NumCachedTextures;
		NumCachedTextures++;		  
	}
	assert(surface_idx >= 0);
	assert(!CachedSurfaces[surface_idx]);

	CachedSurfaces[surface_idx] = surface;
	surface->addReference();
}	


bool TextureFileCache::Validate_Texture(const char* FileName)
{
	// The functions used in here are all thread safe so this function doesn't have to be mutex guarded

	if (!Texture_Exists(FileName)) {
		if (!TextureLoader::Texture_File_Exists(FileName)) {
			return false;
		}
		// We need to load the surface from the file first to determine the real size		
		srColorSurfaceIFace* TempSurfacePtr = ::Load_Surface(FileName);
		if (!TempSurfacePtr) return false;

		int w=TempSurfacePtr->getWidth();
		srColorSurfaceIFace::PixelFormat pf;
		TempSurfacePtr->getPixelFormat(pf);

		srTextureIFace::MultiRequest mr;
		for (int cnt=0;cnt<srTextureIFace::MAX_LOD;++cnt) mr.levels[cnt]=0;
		for (cnt=0;cnt<srTextureIFace::MAX_LOD;) {
			mr.levels[cnt]=W3DNEW srColorSurface(pf,w,w);
			mr.levels[cnt]->copy(*TempSurfacePtr);
			w>>=1;
			++cnt;
			if (!w) break;
		}
		mr.smallLOD=cnt-1;
		mr.largeLOD=0;
		Save_Texture(FileName, mr, *TempSurfacePtr);
		TempSurfacePtr->release();
		TempSurfacePtr=0;
		for (cnt=0;cnt<srTextureIFace::MAX_LOD;++cnt) {
			if (mr.levels[cnt]) mr.levels[cnt]->release();
		}
	}
	return true;
}

int TextureFileCache::Texture_Exists(const char *fname)
{
	CriticalSectionClass::LockClass m(mutex);
	return(File.Does_Tag_Exist(fname));
}

#endif // WW3D_DX8