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

/* $Header: /Commando/Code/ww3d2/texfcach.h 3     3/26/01 10:45a Jani_p $ */
/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : WW3D                                                         * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/ww3d2/texfcach.h                             $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 3/23/01 11:15a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef TEXTFCACH_H
#define TEXTFCACH_H

#include "always.h"

#include <assert.h>
#include <TagBlock.h>

#ifdef WW3D_DX8

//#include <srTextureIFace.hpp>

class FileClass;
//class srColorSurfaceIFace;

class TextureFileCache  
{
	public:
		TextureFileCache(const char *fileprefix);
		virtual ~TextureFileCache();

		virtual void Reset_File();

		// Find texture in the cache.  Returns TRUE if texture found inside of cache.
		int Texture_Exists(const char *fname);

		// Create the initial surface that is based off of the original texture.
		// The surface is not filled in with texels since we don't convert this.
		// This will also set TextureHandle up.
		srColorSurfaceIFace *Load_Original_Texture_Surface(const char *texturename);

		// Given a texture that has been loaded, save it in our file cache.
		bool Save_Texture(const char *texturename, srTextureIFace::MultiRequest& mreq, srColorSurfaceIFace& origsurface);

		// Load texture data from cache into a multirequest structure.
		bool Load_Texture(const char *texturename, srTextureIFace::MultiRequest& mreq);
		//bool TextureFileCache::Load_Texture(const char *texturename, srTextureIFace::MultiRequest& mreq);

		// Check that file is in cache, is not - load and save all mipmap levels
		bool Validate_Texture(const char* texturename);

		// Load a surface from the file cache, null if does not exist.
		srColorSurfaceIFace *Get_Surface(const char *texturename, unsigned int reduce_factor);

	protected:
		struct FileHeader {
			enum {
				// Put in date when format is changed.
				TCF_VERSION 		= 20000814,
			};

			// Will change whenever a new file version is made.
			int						Version;
		};

		// The file format for each texture is as follows:
		//		_TextureBlockHeader {...}
		//		int	MipMap[0].Offset
		//		...
		//		int	MipMap[_TextureBlockHeader.NumMipMaps - 1].Offset
		//		int	Offset of end of block to get size of last MipMap.
		//		rawdata MipMap[0]
		//		...
		//		rawdata MipMap[_TextureBlockHeader.NumMipMaps - 1]
		//		End of BLock

		struct TextureBlockHeader 
		{
			// Time data stamp of file.
			unsigned long 								FileTime;

			// Number of mip maps in texture (including first one).
			int											NumMipMaps;

			// Dimensions of first mip map level saved.  This is normally the same as
			// Source* but can be different when certain texture creation methods are done.
			int											LargestWidth;
			int											LargestHeight;

			// Dimensions of original surface/art work.
			int											SourceWidth;
			int											SourceHeight;

			// This is the pixel format used to create the sources original surface.
			srColorSurfaceIFace::PixelFormat		SourcePixelFormat;

			// Pixel format that we have saved in.
			srColorSurfaceIFace::PixelFormat		PixelFormat;

			TextureBlockHeader():NumMipMaps(-1),SourceWidth(-1),SourceHeight(-1),LargestWidth(-1),LargestHeight(-1) {}
		};

		// Each texture has an offset into the file and it's size when uncompressed.
		struct OffsetTableType
		{
			OffsetTableType() : Offset(0), Size (0) {}

			// Offset of texture in file.  
			int	Offset;

			// Size (uncompressed) of texture.  The size of the compressed texture
			// is caclcuated by Texture_Size() below.
			int	Size;
		};
	protected:										 
		enum  {
			MAX_CACHED_SURFACES = srTextureIFace::MAX_LOD,
		};

		// Pointer to the low level file managment.  TagBlockFile handles the seperation
		// of the different 'texture files'.  This class deals with each seperate 'texture file'.
		TagBlockFile					File;

		// Name of last texture loaded so we know if things need to be thrown away or not.
		char								*CurrentTexture;

		// Handle for current texture in cache. This is created by Open_Texture_Handle().
		TagBlockHandle					*TextureHandle;

		// Header that has been loaded by Open_Texture_Handle().
		TextureBlockHeader			Header;

		// The offset table is loaded into memory when the file is opened.
		OffsetTableType				*Offsets;

		// Cache pointers to data that has already been loaded for this texture.
		srColorSurface *				CachedSurfaces[MAX_CACHED_SURFACES];
																	
		// Number of cached textures we have.
		int								NumCachedTextures;

	protected:
		// Buffer to compress into and decompress out of.
//		static char						*CompressionBuffer;
//		static char						*EOCompressionBuffer;

		// This keeps track of number of instances of the TextureFileCache.  
		// This way static variables are only freed when all instances have been deleted.
//		static int						Instances;
																	 
		// Access to previously cached textures so they do not have to be read off of disk.
		void Add_Cached_Surface(srColorSurface *surface);
		srColorSurface *Find_Cached_Surface(int size);
		srColorSurface *Find_Smallest_Cached_Surface();

		int	Texture_Size(int lod)  {
			assert(Offsets);
			assert(lod < Header.NumMipMaps);
			return(Offsets[lod].Size);
		}

		int	Compressed_Texture_Size(int lod)  {
			assert(Offsets);
			assert(lod < Header.NumMipMaps);
			return(Offsets[lod + 1].Offset - Offsets[lod].Offset);
		}

	 	bool	Open_Texture_Handle(const char *texturename);
	 	void	Close_Texture_Handle();

		void	Read_Texture(int offsetidx, srColorSurface *surface);
		srColorSurfaceIFace *Create_First_Texture_As_Surface(srColorSurfaceIFace *surftype);

		static char *_Create_File_Name(const char *fileprefix);
		static char *_FileNamePtr;
};

#endif //WW3D_DX8


#endif //TEXTFCACH_H
