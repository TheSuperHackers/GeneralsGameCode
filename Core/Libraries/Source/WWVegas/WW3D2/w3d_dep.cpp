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
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/w3d_dep.cpp                            $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/23/01 6:17p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Get_W3D_Dependencies -- Scans a W3D file to determine which other files it depends on.    *
 *   Scan_Chunk -- Chooses the correct chunk loader for this chunk.                            *
 *   Scan_Mesh -- Scans a mesh for references to other files.                                  *
 *   Scan_Mesh_Header -- Scans a mesh's header for file references.                            *
 *   Scan_Mesh_Textures -- Scans a mesh's textures.                                            *
 *   Scan_HTree -- Scans an HTree for references to other files.                               *
 *   Scan_Anim -- Scans an animation for references to other files.                            *
 *   Scan_Compressed_Anim -- Scans an animation for references to other files.                 *
 *   Scan_HModel -- Scans an HModel for references to other files.                             *
 *   Scan_Emitter -- Scans an emitter for references to other files.                           *
 *   Scan_Aggregate -- Scans an aggregate for references to other files.                       *
 *   Scan_HLOD -- Scans an HLOD for references to other files.                                 *
 *   Get_W3D_Name -- Gets a W3D object name from a W3D filename.                               *
 *   Make_W3D_Filename -- Converts a W3D object name into a W3D filename.                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//-----------------------------------------------------------------------------
// srj sez: hack festival :-(
class STLSpecialAlloc
{
public:
  // this one is needed for proper simple_alloc wrapping
  static void*  allocate(size_t __n) {  return ::operator new(__n); }
  static void deallocate(void* __p, size_t) { ::operator delete(__p); }
};


#include "w3d_dep.h"
#include "w3d_file.h"
#include <assert.h>
#include <chunkio.h>
#include "ffactory.h"


/*
** Forward declarations.
*/

static void Scan_Chunk (ChunkLoadClass &cload, StringList &files, const char *w3d_name);

static void Scan_Mesh (ChunkLoadClass &cload, StringList &files, const char *w3d_name);
static void Scan_Mesh_Header (ChunkLoadClass &cload, StringList &files, const char *w3d_name);
static void Scan_Mesh_Textures (ChunkLoadClass &cload, StringList &files, const char *w3d_name);

static void Scan_Anim (ChunkLoadClass &cload, StringList &files, const char *w3d_name);
static void Scan_Compressed_Anim (ChunkLoadClass &cload, StringList &files, const char *w3d_name);
static void Scan_HModel (ChunkLoadClass &cload, StringList &files, const char *w3d_name);
static void Scan_Emitter (ChunkLoadClass &cload, StringList &files, const char *w3d_name);
static void Scan_Aggregate (ChunkLoadClass &cload, StringList &files, const char *w3d_name);
static void Scan_HLOD (ChunkLoadClass &cload, StringList &files, const char *w3d_name);

static void Get_W3D_Name (const char *filename, char *w3d_name, size_t w3d_name_size);
static const char * Make_W3D_Filename (const char *w3d_name);


/***********************************************************************************************
 * Get_W3D_Dependencies -- Scans a W3D file to determine which other files it depends on.      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/3/00     AJA : Created.                                                                 *
 *=============================================================================================*/
bool Get_W3D_Dependencies (const char *w3d_filename, StringList &files)
{
	assert(w3d_filename);

	// Open the W3D file.
	FileClass *file=_TheFileFactory->Get_File(w3d_filename);
	if ( file ) {
		file->Open();
		if ( ! file->Is_Open()) {
			_TheFileFactory->Return_File(file);
			file=nullptr;
			return false;
		}
	} else {
		return false;
	}

	// Get the W3D name from the filename.
	char w3d_name[W3D_NAME_LEN];
	Get_W3D_Name(w3d_filename, w3d_name, ARRAY_SIZE(w3d_name));

	// Create a chunk loader for this file, and scan the file.
	ChunkLoadClass cload(file);
	while (cload.Open_Chunk())
	{
		Scan_Chunk(cload, files, w3d_name);
		cload.Close_Chunk();
	}

	// Close the file.
	file->Close();
	_TheFileFactory->Return_File(file);
	file=nullptr;

	// Sort the set of filenames, and remove any duplicates.
	files.sort();
	files.unique();

	return true;
}


/***********************************************************************************************
 * Scan_Chunk -- Chooses the correct chunk loader for this chunk.                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/3/00     AJA : Created.                                                                 *
 *=============================================================================================*/
static void Scan_Chunk (ChunkLoadClass &cload, StringList &files, const char *w3d_name)
{
	assert(w3d_name);

	switch (cload.Cur_Chunk_ID())
	{
	case W3D_CHUNK_MESH:
		Scan_Mesh(cload, files, w3d_name);
		break;

	case W3D_CHUNK_ANIMATION:
		Scan_Anim(cload, files, w3d_name);
		break;

	case W3D_CHUNK_COMPRESSED_ANIMATION:
		Scan_Compressed_Anim(cload, files, w3d_name);
		break;

	case W3D_CHUNK_HMODEL:
		Scan_HModel(cload, files, w3d_name);
		break;

	case W3D_CHUNK_EMITTER:
		Scan_Emitter(cload, files, w3d_name);
		break;

	case W3D_CHUNK_AGGREGATE:
		Scan_Aggregate(cload, files, w3d_name);
		break;

	case W3D_CHUNK_HLOD:
		Scan_HLOD(cload, files, w3d_name);
		break;
	}
}


/***********************************************************************************************
 * Scan_Mesh -- Scans a mesh for references to other files.                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
static void Scan_Mesh (ChunkLoadClass &cload, StringList &files, const char *w3d_name)
{
	// Scan the mesh's sub-chunks.
	while (cload.Open_Chunk())
	{
		switch (cload.Cur_Chunk_ID())
		{
		case W3D_CHUNK_MESH_HEADER3:
			// Ahh, the mesh header.
			Scan_Mesh_Header(cload, files, w3d_name);
			break;

		case W3D_CHUNK_TEXTURES:
			// We sure want textures...
			Scan_Mesh_Textures(cload, files, w3d_name);
			break;
		}
		cload.Close_Chunk();
	}
}

/***********************************************************************************************
 * Scan_Mesh_Header -- Scans a mesh's header for file references.                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/3/00     AJA : Created.                                                                 *
 *=============================================================================================*/
static void Scan_Mesh_Header (ChunkLoadClass &cload, StringList &files, const char *w3d_name)
{
	// In the mesh header, we want the 'ContainerName' string.
	W3dMeshHeader3Struct	hdr;
	cload.Read(&hdr, sizeof(hdr));
	if (strcmp(hdr.ContainerName, w3d_name) != 0)
	{
		// The container is not this file... Create a W3D filename
		// for the container object and add it to the list of
		// files we refer to.
		const char *filename = Make_W3D_Filename(hdr.ContainerName);
		if (*filename)	// don't push empty filenames
			files.push_back(filename);
	}
}

/***********************************************************************************************
 * Scan_Mesh_Textures -- Scans a mesh's textures.                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/3/00     AJA : Created.                                                                 *
 *=============================================================================================*/
static void Scan_Mesh_Textures (ChunkLoadClass &cload, StringList &files, const char *w3d_name)
{
	// Let's see which textures are used by this mesh...
	while (cload.Open_Chunk())
	{
		// We're interested in the TEXTURE sub-chunk.
		if (cload.Cur_Chunk_ID() == W3D_CHUNK_TEXTURE)
		{
			while (cload.Open_Chunk())
			{
				// We're interested in the TEXTURE_NAME sub-chunk.
				if (cload.Cur_Chunk_ID() == W3D_CHUNK_TEXTURE_NAME)
				{
					// This chunk's data is a null-terminated string
					// which is the texture filename. Read it and
					// add it to the list of files referred to.
					char texture[_MAX_PATH];
					cload.Read(texture, cload.Cur_Chunk_Length());
					if (*texture)	// don't push empty filenames
						files.push_back(texture);
				}
				cload.Close_Chunk();
			}
		}
		cload.Close_Chunk();
	}
}


/***********************************************************************************************
 * Scan_Anim -- Scans an animation for references to other files.                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
static void Scan_Anim (ChunkLoadClass &cload, StringList &files, const char *w3d_name)
{
	while (cload.Open_Chunk())
	{
		// We only want the animation header, because it can refer to an HTree.
		if (cload.Cur_Chunk_ID() == W3D_CHUNK_ANIMATION_HEADER)
		{
			// Read in the header.
			W3dAnimHeaderStruct hdr;
			cload.Read(&hdr, sizeof(hdr));
			if (strcmp(hdr.HierarchyName, w3d_name) != 0)
			{
				const char *hierarchy = Make_W3D_Filename(hdr.HierarchyName);
				if (*hierarchy)	// don't push an empty filename
					files.push_back(hierarchy);
			}
		}
		cload.Close_Chunk();
	}
}


/***********************************************************************************************
 * Scan_Compressed_Anim -- Scans a compressed animation mesh for references to other files.    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
static void Scan_Compressed_Anim (ChunkLoadClass &cload, StringList &files, const char *w3d_name)
{
	while (cload.Open_Chunk())
	{
		// We only want the animation header, because it can refer to an HTree.
		if (cload.Cur_Chunk_ID() == W3D_CHUNK_ANIMATION_HEADER)
		{
			// Read in the header.
			W3dCompressedAnimHeaderStruct hdr;
			cload.Read(&hdr, sizeof(hdr));
			if (strcmp(hdr.HierarchyName, w3d_name) != 0)
			{
				const char *hierarchy = Make_W3D_Filename(hdr.HierarchyName);
				if (*hierarchy)	// don't push an empty filename
					files.push_back(hierarchy);
			}
		}
		cload.Close_Chunk();
	}
}


/***********************************************************************************************
 * Scan_HModel -- Scans an HModel for references to other files.                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
static void Scan_HModel (ChunkLoadClass &cload, StringList &files, const char *w3d_name)
{
	while (cload.Open_Chunk())
	{
		// We only want the header because it can refer to an HTree.
		if (cload.Cur_Chunk_ID() == W3D_CHUNK_HMODEL_HEADER)
		{
			// Read in the header.
			W3dHModelHeaderStruct hdr;
			cload.Read(&hdr, sizeof(hdr));
			if (strcmp(hdr.HierarchyName, w3d_name) != 0)
			{
				const char *hierarchy = Make_W3D_Filename(hdr.HierarchyName);
				if (*hierarchy)	// don't push an empty filename
					files.push_back(hierarchy);
			}
		}
		cload.Close_Chunk();
	}
}


/***********************************************************************************************
 * Scan_Emitter -- Scans an emitter for references to other files.                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
static void Scan_Emitter (ChunkLoadClass &cload, StringList &files, const char *w3d_name)
{
	while (cload.Open_Chunk())
	{
		// We only want the emitter info chunk, it has a texture name in it.
		if (cload.Cur_Chunk_ID() == W3D_CHUNK_EMITTER_INFO)
		{
			// Read in the header.
			W3dEmitterInfoStruct hdr;
			cload.Read(&hdr, sizeof(hdr));
			if (hdr.TextureFilename[0])	// don't push an empty texture name
				files.push_back(hdr.TextureFilename);
		}
		cload.Close_Chunk();
	}
}


/***********************************************************************************************
 * Scan_Aggregate -- Scans an aggregate for references to other files.                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
static void Scan_Aggregate (ChunkLoadClass &cload, StringList &files, const char *w3d_name)
{
	while (cload.Open_Chunk())
	{
		// We want the aggregate info chunk.
		if (cload.Cur_Chunk_ID() == W3D_CHUNK_AGGREGATE_INFO)
		{
			W3dAggregateInfoStruct chunk;
			cload.Read(&chunk, sizeof(chunk));
			if (strcmp(chunk.BaseModelName, w3d_name) != 0)
			{
				// Check the name of the base model against the name of this file.
				const char *base_model = Make_W3D_Filename(chunk.BaseModelName);
				if (*base_model)
					files.push_back(base_model);
			}

			// Iterate through the sub-objects.
			unsigned int i;
			for (i = 0; i < chunk.SubobjectCount; ++i)
			{
				W3dAggregateSubobjectStruct subchunk;
				cload.Read(&subchunk, sizeof(subchunk));
				if (strcmp(subchunk.SubobjectName, w3d_name) != 0)
				{
					// Check the name of the subobject against the name of this file.
					const char *subobject = Make_W3D_Filename(subchunk.SubobjectName);
					if (*subobject)
						files.push_back(subobject);
				}
			}
		}
		cload.Close_Chunk();
	}
}


/***********************************************************************************************
 * Scan_HLOD -- Scans an HLOD for references to other files.                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
static void Scan_HLOD (ChunkLoadClass &cload, StringList &files, const char *w3d_name)
{
	while (cload.Open_Chunk())
	{
		// We only want the header.
		if (cload.Cur_Chunk_ID() == W3D_CHUNK_HLOD_HEADER)
		{
			W3dHLodHeaderStruct hdr;
			cload.Read(&hdr, sizeof(hdr));
			if (strcmp(hdr.HierarchyName, w3d_name) != 0)
			{
				const char *hierarchy = Make_W3D_Filename(hdr.HierarchyName);
				if (*hierarchy)
					files.push_back(hierarchy);
			}
		}
		cload.Close_Chunk();
	}
}


/***********************************************************************************************
 * Get_W3D_Name -- Gets a W3D object name from a W3D filename.                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/3/00     AJA : Created.                                                                 *
 *=============================================================================================*/
static void Get_W3D_Name(const char* filename, char* w3d_name, size_t w3d_name_size)
{
	assert(filename);
	assert(w3d_name);

	// Figure out the first character of the name of the file
	// (bypass the path if it was given).
	const char *start = strrchr(filename, '\\');
	if (start)
		++start;					// point to first character after the last backslash
	else
		start = filename;		// point to the start of the filename

	// We don't want to copy the .w3d extension. Find where
	// it occurs.
	const char *end = strrchr(start, '.');
	if (!end)
		end = start + strlen(start);	// point to the null character

	// Copy all characters from start to end (excluding 'end')
	// into the w3d_name buffer. Then capitalize the string.
	size_t num_chars = end - start;
	WWASSERT(num_chars < w3d_name_size);
	strlcpy(w3d_name, start, min(w3d_name_size, num_chars));
	strupr(w3d_name);
}


/***********************************************************************************************
 * Make_W3D_Filename -- Converts a W3D object name into a W3D filename.                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/3/00     AJA : Created.                                                                 *
 *=============================================================================================*/
static const char * Make_W3D_Filename (const char *w3d_name)
{
	assert(w3d_name);

	// Copy the w3d name into a static buffer, turn it into lowercase
	// letters, and append a ".w3d" file extension. That's the filename.
	static char buffer[64];
	if (*w3d_name == 0)
	{
		// Empty W3D name case.
		buffer[0] = 0;
		return buffer;
	}
	const size_t bufferLen = strlcpy(buffer, w3d_name, ARRAY_SIZE(buffer));
	(void)bufferLen; WWASSERT(bufferLen < W3D_NAME_LEN);
	char *dot = strchr(buffer, '.');
	if (dot)
		*dot = 0;
	strlwr(buffer);
	strlcat(buffer, ".w3d", ARRAY_SIZE(buffer));
	return buffer;
}
