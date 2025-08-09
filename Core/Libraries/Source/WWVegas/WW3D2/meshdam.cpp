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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/meshdam.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if 0


#include "meshdam.h"
#include "w3d_file.h"
#include "w3derr.h"
#include "chunkio.h"
//#include <sr.hpp>


/***********************************************************************************************
 * DamageClass::DamageClass -- constructor                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
DamageClass::DamageClass(void):
	DamageIndex(0),
	NumMaterials(0),
	NumVerts(0),
	NumColors(0),
	Verts(NULL),
	Colors(NULL)
{
}


/***********************************************************************************************
 * DamageClass::~DamageClass -- destructor                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
DamageClass::~DamageClass(void)
{
	if (Verts != NULL) {
		delete [] Verts;
		Verts = NULL;
	}

	if (Colors != NULL) {
		delete[] Colors;
		Colors = NULL;
	}
}

/***********************************************************************************************
 * DamageClass::Load -- load damage data from a W3D file                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType DamageClass::Load(ChunkLoadClass & cload,MeshModelClass * basemesh)
{
#if 0
	/*
	** Open the first chunk, it should be the damage header
	*/
	cload.Open_Chunk();

	if (cload.Cur_Chunk_ID() != W3D_CHUNK_DAMAGE_HEADER) {
		return WW3D_ERROR_LOAD_FAILED;
	}

	W3dMeshDamageStruct header;
	if (cload.Read(&header,sizeof(W3dMeshDamageStruct)) != sizeof(W3dMeshDamageStruct)) {
		return WW3D_ERROR_LOAD_FAILED;
	}

	cload.Close_Chunk();

	/*
	** allocate the arrays based on the info in the header
	*/
	NumVerts = header.NumDamageVerts;
	if (NumVerts > 0) {
		Verts = W3DNEWARRAY DamageVertexStruct[NumVerts];
	}

	NumColors = header.NumDamageColors;
	if (NumColors > 0) {
		Colors = W3DNEWARRAY DamageColorStruct[NumColors];
	}

	// TODO: allocate materials

	/*
	** Now read in the rest of the chunks
	*/
	while (1) {

		/*
		**	Read in the chunk header
		** If there are no more chunks within the chunk, we are done.
		*/
		if (!cload.Open_Chunk()) {
			return WW3D_ERROR_OK;
		}

		/*
		** Process the chunk
		*/
		int error = WW3D_ERROR_OK;

		switch (cload.Cur_Chunk_ID()) {

//			case W3D_CHUNK_DAMAGE_MATERIALS:
//					//TODO: read new set of materials
//					break;

			case W3D_CHUNK_DAMAGE_VERTICES:
					error = read_vertices(cload,basemesh);
					break;

			case W3D_CHUNK_DAMAGE_COLORS:
					error = read_colors(cload,basemesh);
					break;

			default:
					break;

		}

		cload.Close_Chunk();

		if (error != WW3D_ERROR_OK) {
			return error;
		}
	}
#endif

	return WW3D_ERROR_OK;
}

/***********************************************************************************************
 * DamageClass::read_vertices -- read damage vertices from a W3D file                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/04/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType DamageClass::read_vertices(ChunkLoadClass & cload,MeshModelClass * basemesh)
{
#if 0
	W3dMeshDamageVertexStruct dv;

	for (int i=0; i<NumVerts; i++) {

		if (cload.Read(&dv,sizeof(W3dMeshDamageVertexStruct)) != sizeof(W3dMeshDamageVertexStruct)) {
			return WW3D_ERROR_LOAD_FAILED;
		}

		srVector3 * vert_array = basemesh->getVertexLoc();

		srVector3 sr_v;
		sr_v = vert_array[dv.VertexIndex];

		Verts[i].VertexIdx = dv.VertexIndex;
		Verts[i].Vertex0.Set(sr_v[0],sr_v[1],sr_v[2]);
		Verts[i].Vertex1.Set(dv.NewVertex.X,dv.NewVertex.Y,dv.NewVertex.Z);
	}
#endif
	return WW3D_ERROR_OK;
}

/***********************************************************************************************
 * DamageClass::read_colors -- read damage colors from a w3d file                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/04/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType DamageClass::read_colors(ChunkLoadClass & cload,MeshModelClass * basemesh)
{
#if 0
	W3dMeshDamageColorStruct dc;

	for (int i=0; i<NumColors; i++) {

		if (cload.Read(&dc,sizeof(W3dMeshDamageColorStruct)) != sizeof(W3dMeshDamageColorStruct)) {
			return WW3D_ERROR_LOAD_FAILED;
		}


		Colors[i].VertexIdx = dc.VertexIndex;

		// TODO: check for existing vertex colors, store original here
		Colors[i].Color0.R = 255;
		Colors[i].Color0.G = 255;
		Colors[i].Color0.B = 255;

		Colors[i].Color1.R = dc.NewColor.R;
		Colors[i].Color1.G = dc.NewColor.G;
		Colors[i].Color1.B = dc.NewColor.B;
	}
#endif
	return WW3D_ERROR_OK;
}


#endif //0
