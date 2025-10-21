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
 *                     $Archive:: /Commando/Code/ww3d2/meshbuild.cpp                          $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MeshBuilderClass::VertClass::Reset -- reset this vertex                                   *
 *   MeshBuilderClass::FaceClass::Reset -- rest this face                                      *
 *   MeshBuilderClass::FaceClass::Compute_Plane -- compute the plane for this face             *
 *   MeshBuilderClass::FaceClass::Is_Degenerate -- check if a face is degenerate               *
 *   MeshBuilderClass::MeshStatsStruct::Reset -- reset the stats to all false                  *
 *   MeshBuilderClass::MeshBuilderClass -- constructor                                         *
 *   MeshBuilderClass::~MeshBuilderClass -- destructor                                         *
 *   MeshBuilderClass::Free -- release all memory in use                                       *
 *   MeshBuilderClass::Reset -- Get the builder ready to process a mesh                        *
 *   MeshBuilderClass::Add_Face -- Add a face to the mesh                                      *
 *   MeshBuilderClass::Build_Mesh -- process the mesh                                          *
 *   MeshBuilderClass::Compute_Face_Normals -- computes all of the face normals from the index *
 *   MeshBuilderClass::Verify_Face_Normals -- checks if any faces have flipped                 *
 *   MeshBuilderClass::Compute_Vertex_Normals -- Computes the vertex normals for the mesh      *
 *   MeshBuilderClass::Remove_Degenerate_Faces -- discard invalid or duplicated faces          *
 *   MeshBuilderClass::Compute_Mesh_Stats -- compute some stats about the mesh                 *
 *   MeshBuilderClass::Compute_Bounding_Box -- computes an axis-aligned bounding box for the m *
 *   MeshBuilderClass::Compute_Bounding_Sphere -- computes a bounding sphere for the mesh      *
 *   MeshBuilderClass::Optimize_Mesh -- "optimize" the mesh                                    *
 *   MeshBuilderClass::Strip_Optimize_Mesh -- optimize the mesh for triangle strips            *
 *   MeshBuilderClass::Grow_Face_Array -- increases the size of the face array                 *
 *   MeshBuilderClass::Sort_Vertices -- Sorts vertices according to the given key array        *
 *   MeshBuilderClass::Sort_Vertices_By_Bone_Index -- sorts verts by bone index                *
 *   MeshBuilderClass::Sort_Vertices_By_Vertex_Material -- sorts verts by vertex mtl in pass0  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "meshbuild.h"
#include "uarray.h"
#include <stdlib.h>
#include <assert.h>


const float EPSILON = 0.0001f;

/*
** qsort compare functions
*/
static int face_material_compare(const void *elem1, const void *elem2);
static int pass0_stage0_compare(const void *elem1, const void *elem2);
static int pass0_stage1_compare(const void *elem1, const void *elem2);
static int pass1_stage0_compare(const void *elem1, const void *elem2);
static int pass1_stage1_compare(const void *elem1, const void *elem2);
static int pass2_stage0_compare(const void *elem1, const void *elem2);
static int pass2_stage1_compare(const void *elem1, const void *elem2);
static int pass3_stage0_compare(const void *elem1, const void *elem2);
static int pass3_stage1_compare(const void *elem1, const void *elem2);
static int vertex_compare(const void *elem1, const void *elem2);

typedef int (*COMPARE_FUNC_TYPE)(const void * elem1,const void * elem2);

COMPARE_FUNC_TYPE Texture_Compare_Funcs[MeshBuilderClass::MAX_PASSES][MeshBuilderClass::MAX_STAGES] =
{
	{ pass0_stage0_compare, pass0_stage1_compare },
	{ pass1_stage0_compare, pass1_stage1_compare },
	{ pass2_stage0_compare, pass2_stage1_compare },
	{ pass3_stage0_compare, pass3_stage1_compare },
};


/************************************************************************************
**
** FaceHasherClass, support class for the unique faces hash table.  The unique
** faces table is going to detect exactly duplicated faces and discard them.  It
** appears that the artists occasionally accidentally duplicate a face which causes
** problems in the stripping algorithm...
**
************************************************************************************/
class FaceHasherClass : public HashCalculatorClass<MeshBuilderClass::FaceClass>
{
public:

	virtual bool	Items_Match(const MeshBuilderClass::FaceClass & a, const MeshBuilderClass::FaceClass & b)
	{
		// Note: if we want this to detect duplicates that are "rotated", must change
		// both this function and the Compute_Hash function...
		return
		(
			(a.VertIdx[0] == b.VertIdx[0]) &&
			(a.VertIdx[1] == b.VertIdx[1]) &&
			(a.VertIdx[2] == b.VertIdx[2])
		);
	}

	virtual void	Compute_Hash(const MeshBuilderClass::FaceClass & item)
	{
		HashVal = (int)(item.VertIdx[0]*12345.6f + item.VertIdx[1]*1714.38484f + item.VertIdx[2]*27561.3f)&1023;
	}

	virtual int		Num_Hash_Bits(void)
	{
		return 10;
	}

	virtual int		Num_Hash_Values(void)
	{
		return 1;
	}

	virtual int		Get_Hash_Value(int /*index*/)
	{
		return HashVal;
	}

private:

	int HashVal;

};


/************************************************************************************
**
** VertexArray, build an array of unique vertices.  Can't use the UniqueArray
** template due to the special considerations needed to properly handle smoothing
** groups...  (DAMN!!! this was the reason I *WROTE* UniqueArray, oh well :-)
**
************************************************************************************/
class VertexArrayClass
{
public:

	enum
	{
		HASH_TABLE_SIZE = 4096,
	};

	VertexArrayClass(int maxsize,int match_normals = 0)
	{
		Verts = NULL;
		assert(maxsize > 0);
		Verts = W3DNEWARRAY MeshBuilderClass::VertClass[maxsize];
		assert(Verts);
		VertCount = 0;
		UVSplits = 0;

		HashTable = W3DNEWARRAY MeshBuilderClass::VertClass * [HASH_TABLE_SIZE];
		memset(HashTable,0,sizeof(MeshBuilderClass::VertClass *) * HASH_TABLE_SIZE);

		MatchNormals = match_normals;

		// initialize the center and extent to do nothing to the points input
		Center.Set(0.0f,0.0f,0.0f);
		Extent.Set(1.0f,1.0f,1.0f);
	}

	~VertexArrayClass(void)
	{
		delete[] Verts;
		delete[] HashTable;
	}

	void Set_Bounds(const Vector3 & minv,const Vector3 & maxv)
	{
		Extent = (maxv - minv) / 2.0f;
		Center = (maxv + minv) / 2.0f;
	}

	int Submit_Vertex(const MeshBuilderClass::VertClass & vert)
	{
		// 2D floating point hashing...
		unsigned int lasthash = 0xFFFFFFFF;
		unsigned int hash;
		unsigned int shadeindex = 0xFFFFFFFF;

		// transform the position of the point into the range
		// -1 < p < 1  as defined by the center and extent.
		// aja/ehc 19991005 - Handle the case where an extent is zero.
		Vector3 position = (vert.Position - Center);
		double xstart;
		if(fabs(Extent.X) > EPSILON)
			xstart = (vert.Position.X - Center.X) / Extent.X;
		else
			xstart = Center.X;

		double ystart;
		if(fabs(Extent.Y) > EPSILON)
			ystart = (vert.Position.Y - Center.Y) / Extent.Y;
		else
			ystart = Center.Y;

		double x,y;

		for (x = xstart - EPSILON; x <= xstart + EPSILON + 0.0000001; x+= EPSILON) {
			for (y = ystart - EPSILON; y <= ystart + EPSILON + 0.000001; y+= EPSILON) {

				hash = compute_hash((float)x,(float)y);

				if (hash != lasthash) {
					MeshBuilderClass::VertClass * test_vert = HashTable[hash];
					while (test_vert) {

						if (Verts_Shading_Match(vert,*test_vert)) {
							if (shadeindex == 0xFFFFFFFF) {
								shadeindex = test_vert->UniqueIndex;

								// mask the "master" smoothed vertex's smoothing group with our
								// face's smoothing group since we are going to be smoothed with it.
								Verts[shadeindex].SharedSmGroup &= vert.SmGroup;
							}
						}

						if (Verts_Match(vert,*test_vert)) {
							return test_vert->UniqueIndex;
						}
						test_vert = test_vert->NextHash;
					}
				}

				lasthash = hash;
			}
		}

		// Not found, add to the end of the array
		int newindex = VertCount;
		VertCount++;

		Verts[newindex] = vert;
		Verts[newindex].UniqueIndex = newindex;
		if (shadeindex == 0xFFFFFFFF) {

			Verts[newindex].ShadeIndex = newindex;

			// This is a new vertex,so store the face's smoothing group into SharedSmGroup
			Verts[newindex].SharedSmGroup = Verts[newindex].SmGroup;

		} else {

			Verts[newindex].ShadeIndex = shadeindex;

		}

		// This is a new vertex, store the face's smoothing group with it
		//Verts[newindex].SmoothingGroup = face_smooth_group;

		// And add to the hash table
		x = (vert.Position.X - Center.X) / Extent.X;
		y = (vert.Position.Y - Center.Y) / Extent.Y;
		hash = compute_hash((float)x,(float)y);
		Verts[newindex].NextHash = HashTable[hash];
		HashTable[hash] = &Verts[newindex];

		return newindex;
	}

	int Verts_Match(const MeshBuilderClass::VertClass & v0,const MeshBuilderClass::VertClass & v1)
	{
		// if user is specifying unique id's, they must match:
		if (v0.Id != v1.Id) return 0;

		// position must match:
		float dp = (v0.Position - v1.Position).Length();
		if (dp > EPSILON) return 0;

		// normal or smoothing group must match:
		if (MatchNormals == 0) {
			int smgroup_match = ((v0.SmGroup & v1.SmGroup) || (v0.SmGroup == v1.SmGroup));
			if (!smgroup_match) return 0;
		} else {
			float dn = (v0.Normal - v1.Normal).Length();
			if (dn > EPSILON) return 0;
		}

		// colors, and material id's must match for all passes
		int pass=0;
		for (; pass < MeshBuilderClass::MAX_PASSES; pass++) {

			if (v0.DiffuseColor[pass] != v1.DiffuseColor[pass]) return 0;
			if (v0.SpecularColor[pass] != v1.SpecularColor[pass]) return 0;
			if (v0.DiffuseIllumination[pass] != v1.DiffuseIllumination[pass]) return 0;
			if (v0.Alpha[pass] != v1.Alpha[pass]) return 0;
			if (v0.VertexMaterialIndex[pass] != v1.VertexMaterialIndex[pass]) return 0;

		}

		// texcoords must match for all passes and stages
		// Note: I'm checking them separately and last so that I can keep track
		// of how many splits are caused solely by u-v discontinuities...
		for (pass=0; pass<MeshBuilderClass::MAX_PASSES; pass++) {
			for (int stage=0; stage < MeshBuilderClass::MAX_STAGES; stage++) {
				if (v0.TexCoord[pass][stage] != v1.TexCoord[pass][stage]) {
					UVSplits++;
					return 0;
				}
			}
		}
		return 1;
	}

	int Verts_Shading_Match(const MeshBuilderClass::VertClass & v0,const MeshBuilderClass::VertClass & v1)
	{
		float dv = (v0.Position - v1.Position).Length();
		int smgroup_match = ((v0.SmGroup & v1.SmGroup) || (v0.SmGroup == v1.SmGroup));

		if (	(dv < EPSILON) &&
				(smgroup_match) &&
				(v0.Id == v1.Id))
		{
			return 1;
		}
		return 0;
	}

	void Propogate_Shared_Smooth_Groups(void)
	{
		for (int i=0; i<VertCount; i++) {
			if (Verts[i].ShadeIndex != i) {
				Verts[i].SharedSmGroup = Verts[Verts[i].ShadeIndex].SharedSmGroup;
			}
		}
	}

	const MeshBuilderClass::VertClass & operator [] (int i) { return Verts[i]; }

	int										VertCount;
	int										UVSplits;
	MeshBuilderClass::VertClass *		Verts;

private:

	MeshBuilderClass::VertClass * *	HashTable;
	int										MatchNormals;

	Vector3									Center;
	Vector3									Extent;

	unsigned int compute_hash(float x,float y)
	{
		// 12 bit hash, 6 bits for x and 6 for y, points near each other
		// need to generate the same hash value...
		int ix = ((int)(x) & 0x0000003F);
		int iy = ((int)(y) & 0x0000003F);
		return (iy<<6) | ix;
	}
};


/***********************************************************************************************
 * MeshBuilderClass::VertClass::Reset -- reset this vertex                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/19/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::VertClass::Reset(void)
{
	Position.Set(0,0,0);
	Normal.Set(0,0,0);
	SmGroup = 0;
	Id = 0;
	MaxVertColIndex = 0;

	for (int pass = 0; pass < MeshBuilderClass::MAX_PASSES; pass++) {

		DiffuseColor[pass].Set(1,1,1);
		SpecularColor[pass].Set(1,1,1);
		DiffuseIllumination[pass].Set(0,0,0);
		Alpha[pass] = 1.0f;
		VertexMaterialIndex[pass] = -1;

		for (int stage=0; stage < MeshBuilderClass::MAX_STAGES; stage++) {
			TexCoord[pass][stage].Set(0,0);
		}
	}

	BoneIndex = 0;
	Attribute0 = 0;
	Attribute1 = 0;
	UniqueIndex = 0;
	ShadeIndex = 0;
	NextHash = NULL;

}


/***********************************************************************************************
 * MeshBuilderClass::FaceClass::Reset -- rest this face                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/19/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::FaceClass::Reset(void)
{
	for (int i=0; i<3; i++) {
		Verts[i].Reset();
		VertIdx[i] = 0;
	}

	SmGroup = 0;
	Index = 0;
	Attributes = 0;
	SurfaceType = 0;

	for (int pass=0; pass<MeshBuilderClass::MAX_PASSES; pass++) {
		for (int stage=0; stage < MeshBuilderClass::MAX_STAGES; stage++) {
			TextureIndex[pass][stage] = -1;
		}
		ShaderIndex[pass] = -1;
	}

	AddIndex = 0;
	Normal.Set(0,0,0);
	Dist = 0.0f;
}


/***********************************************************************************************
 * MeshBuilderClass::FaceClass::Compute_Plane -- compute the plane for this face               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::FaceClass::Compute_Plane(void)
{
#ifdef ALLOW_TEMPORARIES
	Normal = Vector3::Cross_Product((Verts[1].Position - Verts[0].Position),(Verts[2].Position - Verts[0].Position));
	Normal.Normalize();
#else
	Vector3::Normalized_Cross_Product((Verts[1].Position - Verts[0].Position),(Verts[2].Position - Verts[0].Position),&Normal);
#endif
	Dist = Vector3::Dot_Product(Normal,Verts[0].Position);
}


/***********************************************************************************************
 * MeshBuilderClass::FaceClass::Is_Degenerate -- check if a face is degenerate                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * assumes we have already built the unique vertex table and installed their indices           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/7/98     GTH : Created.                                                                 *
 *=============================================================================================*/
bool MeshBuilderClass::FaceClass::Is_Degenerate(void)
{
	for (int v0 = 0; v0 < 3; v0++) {
		for (int v1 = v0+1; v1 < 3; v1++) {

			if (VertIdx[v0] == VertIdx[v1]) {
				return true;
			}

			if (	(Verts[v0].Position.X == Verts[v1].Position.X) &&
					(Verts[v0].Position.Y == Verts[v1].Position.Y) &&
					(Verts[v0].Position.Z == Verts[v1].Position.Z) )
			{
				return true;
			}
		}
	}
	return false;
}


/***********************************************************************************************
 * MeshBuilderClass::MeshStatsStruct::Reset -- reset the stats to all false                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::MeshStatsStruct::Reset(void)
{
	for (int pass = 0; pass < MeshBuilderClass::MAX_PASSES; pass++) {
		HasPerPolyShader[pass] = false;
		HasPerVertexMaterial[pass] = false;
		HasDiffuseColor[pass] = false;
		HasSpecularColor[pass] = false;
		HasDiffuseIllumination[pass] = false;
		HasVertexMaterial[pass] = false;
		HasShader[pass] = false;

		for (int stage = 0; stage < MeshBuilderClass::MAX_STAGES; stage++) {
			HasPerPolyTexture[pass][stage] = false;
			HasTexCoords[pass][stage] = false;
			HasTexture[pass][stage] = false;
		}
	}

	UVSplitCount = 0;
	StripCount = 0;
	MaxStripLength = 0;
	AvgStripLength = 0.0f;
}

/***********************************************************************************************
 * MeshBuilderClass::MeshBuilderClass -- constructor                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
MeshBuilderClass::MeshBuilderClass(int pass_count,int face_count_guess,int face_count_growth_rate) :
	State(STATE_ACCEPTING_INPUT),
	PassCount(pass_count),
	FaceCount(0),
	Faces(NULL),
	InputVertCount(0),
	VertCount(0),
	Verts(NULL),
	CurFace(0),
	AllocFaceCount(0),
	AllocFaceGrowth(0),
	PolyOrderPass(0),
	PolyOrderStage(0),
	WorldInfo (NULL)
{
	Reset(pass_count,face_count_guess,face_count_growth_rate);
}

/***********************************************************************************************
 * MeshBuilderClass::~MeshBuilderClass -- destructor                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
MeshBuilderClass::~MeshBuilderClass(void)
{
	Free();
	Set_World_Info(NULL);
}


/***********************************************************************************************
 * MeshBuilderClass::Free -- release all memory in use                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Free(void)
{
	delete[] Faces;
	Faces = NULL;

	delete Verts;
	Verts = NULL;

	FaceCount = 0;
	VertCount = 0;
	AllocFaceCount = 0;
	AllocFaceGrowth = 0;
}


/***********************************************************************************************
 * MeshBuilderClass::Reset -- Get the builder ready to process a mesh                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Reset(int passcount,int face_count_guess,int face_count_growth_rate)
{
	Free();

	State = STATE_ACCEPTING_INPUT;
	PassCount = passcount;
	AllocFaceCount = face_count_guess;
	AllocFaceGrowth = face_count_growth_rate;

	Faces = W3DNEWARRAY FaceClass[AllocFaceCount];
	CurFace = 0;
	Stats.Reset();
}


/***********************************************************************************************
 * MeshBuilderClass::Add_Face -- Add a face to the mesh                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
int MeshBuilderClass::Add_Face(const FaceClass & face)
{
	assert(State == STATE_ACCEPTING_INPUT);
	assert(CurFace <= AllocFaceCount);
	if (CurFace == AllocFaceCount) {
		Grow_Face_Array();
	}

	// copy the face
	Faces[CurFace] = face;

	// calculate the normal
	Faces[CurFace].Compute_Plane();

	// set the "add index"
	Faces[CurFace].AddIndex = CurFace;

	// copy the smoothing group into each vert
	for (int i=0; i<3; i++) {
		Faces[CurFace].Verts[i].SmGroup = Faces[CurFace].SmGroup;
	}

	// increment the face index
	CurFace++;

	return CurFace-1;
}


/***********************************************************************************************
 * MeshBuilderClass::Build_Mesh -- process the mesh                                            *
 *                                                                                             *
 *    This function finds all sharable vertices, calculates vertex normals, sorts the          *
 *    faces by material, sorts the faces into stripping order...                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Build_Mesh(bool compute_normals)
{
	/*
	** Check the state, set state to "mesh processed"
	*/
	assert(State == STATE_ACCEPTING_INPUT);
	State = STATE_MESH_PROCESSED;

	/*
	** Seal off the face count
	*/
	FaceCount = CurFace;

	/*
	** Optimize the mesh and calculate vertex normals
	*/
	Optimize_Mesh(compute_normals);

}


/***********************************************************************************************
 * MeshBuilderClass::Compute_Face_Normals -- computes all of the face normals from the indexed *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/10/98    GTH : Created.                                                                 *
 *   10/19/98   GTH : Modified to use the FaceClass::Compute_Plane function                    *
 *=============================================================================================*/
void MeshBuilderClass::Compute_Face_Normals(void)
{
	for (int faceidx = 0; faceidx < FaceCount; faceidx++) {
		Faces[faceidx].Compute_Plane();
	}
}

/***********************************************************************************************
 * MeshBuilderClass::Verify_Face_Normals -- checks if any faces have flipped                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/10/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool MeshBuilderClass::Verify_Face_Normals(void)
{
	int faceidx;
	bool ok = true;
	/*
	** Check each face to see if any have flipped normals
	*/
	for (faceidx = 0; faceidx < FaceCount; faceidx++) {

		VertClass & v0 = Verts[Faces[faceidx].VertIdx[0]];
		VertClass & v1 = Verts[Faces[faceidx].VertIdx[1]];
		VertClass & v2 = Verts[Faces[faceidx].VertIdx[2]];
#ifdef ALLOW_TEMPORARIES
		Vector3 normal = Vector3::Cross_Product((v1.Position - v0.Position),(v2.Position - v0.Position));
		normal.Normalize();
#else
		Vector3 normal;
		Vector3::Normalized_Cross_Product((v1.Position - v0.Position),(v2.Position - v0.Position),&normal);
#endif

		float dn = (Faces[faceidx].Normal - normal).Length();
		if (dn > 0.0000001f) {
			ok = false;
		}
	}
	return ok;
}

/***********************************************************************************************
 * MeshBuilderClass::Compute_Vertex_Normals -- Computes the vertex normals for the mesh        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    This function should only be called after the mesh has been "optimized".  The algorithm  *
 *    relies on non-smooth vertices having been split...                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Compute_Vertex_Normals(void)
{
	int vertidx;
	int faceidx;
	int facevertidx;

	/*
	** First, zero all vertex normals
	*/
	for (vertidx = 0; vertidx < VertCount; vertidx++) {
		Verts[vertidx].Normal.Set(0,0,0);
	}

	/*
	** Now, go through all of the faces, accumulating the face normals into the
	** "first" vertices containing the appropriate vertex normal for each vertex.
	*/
	for (faceidx = 0; faceidx < FaceCount; faceidx++) {
		for (facevertidx = 0; facevertidx < 3; facevertidx++) {
			int vertindex = Faces[faceidx].VertIdx[facevertidx];
			int shadeindex = Verts[vertindex].ShadeIndex;
			Verts[shadeindex].Normal += Faces[faceidx].Normal;
		}
	}

	/*
	** Smooth this mesh with neighboring meshes!
	*/
	if (WorldInfo != NULL && WorldInfo->Are_Meshes_Smoothed ()) {
		for (vertidx = 0; vertidx < VertCount; vertidx++) {
			if (Verts[vertidx].ShadeIndex == vertidx) {
				Verts[vertidx].Normal += WorldInfo->Get_Shared_Vertex_Normal(Verts[vertidx].Position, Verts[vertidx].SharedSmGroup);
			}
		}
	}

	/*
	** Propogate the accumulated normals to all of the other verts which share them
	*/
	for (vertidx = 0; vertidx < VertCount; vertidx++) {
		int shadeindex = Verts[vertidx].ShadeIndex;
		Verts[vertidx].Normal = Verts[shadeindex].Normal;
		Verts[vertidx].Normal.Normalize();
	}
}

/***********************************************************************************************
 * MeshBuilderClass::Remove_Degenerate_Faces -- discard invalid or duplicated faces            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/10/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Remove_Degenerate_Faces(void)
{
	int faceidx;
	FaceHasherClass facehasher;
	UniqueArrayClass<MeshBuilderClass::FaceClass> uniquefaces(FaceCount,FaceCount/4,&facehasher);

	for (faceidx = 0; faceidx < FaceCount; faceidx++) {
		if (!Faces[faceidx].Is_Degenerate()) {
			uniquefaces.Add(Faces[faceidx]);
		}
	}

	FaceCount = uniquefaces.Count();
	AllocFaceCount = uniquefaces.Count();
	CurFace = FaceCount;

	delete[] Faces;
	Faces = W3DNEWARRAY FaceClass[AllocFaceCount];

	for (faceidx = 0; faceidx < FaceCount; faceidx++) {
		Faces[faceidx] = uniquefaces.Get(faceidx);
	}
}



/***********************************************************************************************
 * MeshBuilderClass::Compute_Mesh_Stats -- compute some stats about the mesh                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/19/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Compute_Mesh_Stats(void)
{
	int	pass;
	int	stage;
	int	face_index;
	int	vert_index;

	int	tex_index[MAX_PASSES][MAX_STAGES];
	int	shader_index[MAX_PASSES];
	int	vmat_index[MAX_PASSES];

	Stats.Reset();

	for (pass = 0; pass<MAX_PASSES; pass++) {
		for (stage = 0; stage < MAX_STAGES; stage++) {
			tex_index[pass][stage] = Faces[0].TextureIndex[pass][stage];
		}
		shader_index[pass] = Faces[0].ShaderIndex[pass];
		vmat_index[pass] = Verts[0].VertexMaterialIndex[pass];
	}

	for (pass = 0; pass<MAX_PASSES; pass++) {

		for (stage = 0; stage<MAX_STAGES; stage++) {
			for (face_index=0; face_index < FaceCount; face_index++) {
				if (Faces[face_index].TextureIndex[pass][stage] != tex_index[pass][stage]) {
					Stats.HasPerPolyTexture[pass][stage] = true;
					break;
				}
			}

			for (vert_index=0; vert_index < VertCount; vert_index++) {
				if (Verts[vert_index].TexCoord[pass][stage] != Vector2(0,0)) {
					Stats.HasTexCoords[pass][stage] = true;
					break;
				}
			}
		}

		for (face_index=0; face_index < FaceCount; face_index++) {
			if (Faces[face_index].ShaderIndex[pass] != shader_index[pass]) {
				Stats.HasPerPolyShader[pass] = true;
				break;
			}
		}

		for (vert_index=0; vert_index < VertCount; vert_index++) {
			if (Verts[vert_index].VertexMaterialIndex[pass] != vmat_index[pass]) {
				Stats.HasPerVertexMaterial[pass] = true;
				break;
			}
		}

		for (vert_index=0; vert_index < VertCount; vert_index++) {
			if (	(Verts[vert_index].DiffuseColor[pass] != Vector3(1,1,1)) ||
					(Verts[vert_index].Alpha[pass] != 1.0f) )
			{
				Stats.HasDiffuseColor[pass] = true;
				break;
			}
		}

		for (vert_index=0; vert_index < VertCount; vert_index++) {
			if (Verts[vert_index].SpecularColor[pass] != Vector3(1,1,1)) {
				Stats.HasSpecularColor[pass] = true;
				break;
			}
		}

		for (vert_index=0; vert_index < VertCount; vert_index++) {
			if (Verts[vert_index].DiffuseIllumination[pass] != Vector3(0,0,0)) {
				Stats.HasDiffuseIllumination[pass] = true;
				break;
			}
		}


		for (stage = 0; stage<MAX_STAGES; stage++) {
			for (face_index=0; face_index < FaceCount; face_index++) {
				if (Faces[face_index].TextureIndex[pass][stage] != -1) {
					Stats.HasTexture[pass][stage] = true;
					break;
				}
			}
		}

		for (face_index=0; face_index < FaceCount; face_index++) {
			if (Faces[face_index].ShaderIndex[pass] != -1) {
				Stats.HasShader[pass] = true;
				break;
			}
		}

		for (vert_index=0; vert_index < VertCount; vert_index++) {
			if (Verts[vert_index].VertexMaterialIndex[pass] != -1) {
				Stats.HasVertexMaterial[pass] = true;
				break;
			}
		}
	}
}


/***********************************************************************************************
 * MeshBuilderClass::Compute_Bounding_Box -- computes an axis-aligned bounding box for the mes *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Compute_Bounding_Box(Vector3 * set_min,Vector3 * set_max)
{
	int i;

	assert(set_min != NULL);
	assert(set_max != NULL);

	// Bounding Box
	// straightforward, axis-aligned bounding box.
	set_min->X = Verts[0].Position.X;
	set_min->Y = Verts[0].Position.Y;
	set_min->Z = Verts[0].Position.Z;
	set_max->X = Verts[0].Position.X;
	set_max->Y = Verts[0].Position.Y;
	set_max->Z = Verts[0].Position.Z;

	for (i=0; i<VertCount; i++) {
		if (Verts[i].Position.X < set_min->X) set_min->X = Verts[i].Position.X;
		if (Verts[i].Position.Y < set_min->Y) set_min->Y = Verts[i].Position.Y;
		if (Verts[i].Position.Z < set_min->Z) set_min->Z = Verts[i].Position.Z;

		if (Verts[i].Position.X > set_max->X) set_max->X = Verts[i].Position.X;
		if (Verts[i].Position.Y > set_max->Y) set_max->Y = Verts[i].Position.Y;
		if (Verts[i].Position.Z > set_max->Z) set_max->Z = Verts[i].Position.Z;
	}
}


/***********************************************************************************************
 * MeshBuilderClass::Compute_Bounding_Sphere -- computes a bounding sphere for the mesh        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/29/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Compute_Bounding_Sphere(Vector3 * set_center,float * set_radius)
{
	int i;
	double dx,dy,dz;

	// bounding sphere
	// Using the algorithm described in Graphics Gems I page 301.
	// This algorithm supposedly generates a bounding sphere within
	// 5% of the optimal one but is much faster and simpler to implement.
	Vector3 xmin(Verts[0].Position.X,Verts[0].Position.Y,Verts[0].Position.Z);
	Vector3 xmax(Verts[0].Position.X,Verts[0].Position.Y,Verts[0].Position.Z);
	Vector3 ymin(Verts[0].Position.X,Verts[0].Position.Y,Verts[0].Position.Z);
	Vector3 ymax(Verts[0].Position.X,Verts[0].Position.Y,Verts[0].Position.Z);
	Vector3 zmin(Verts[0].Position.X,Verts[0].Position.Y,Verts[0].Position.Z);
	Vector3 zmax(Verts[0].Position.X,Verts[0].Position.Y,Verts[0].Position.Z);


	// FIRST PASS:
	// finding the 6 minima and maxima points
	for (i=1; i<VertCount; i++) {
		if (Verts[i].Position.X < xmin.X) {
			xmin.X = Verts[i].Position.X; xmin.Y = Verts[i].Position.Y; xmin.Z = Verts[i].Position.Z;
		}
		if (Verts[i].Position.X > xmax.X) {
			xmax.X = Verts[i].Position.X; xmax.Y = Verts[i].Position.Y; xmax.Z = Verts[i].Position.Z;
		}
		if (Verts[i].Position.Y < ymin.Y) {
			ymin.X = Verts[i].Position.X; ymin.Y = Verts[i].Position.Y; ymin.Z = Verts[i].Position.Z;
		}
		if (Verts[i].Position.Y > ymax.Y) {
			ymax.X = Verts[i].Position.X; ymax.Y = Verts[i].Position.Y; ymax.Z = Verts[i].Position.Z;
		}
		if (Verts[i].Position.Z < zmin.Z) {
			zmin.X = Verts[i].Position.X; zmin.Y = Verts[i].Position.Y; zmin.Z = Verts[i].Position.Z;
		}
		if (Verts[i].Position.Z > zmax.Z) {
			zmax.X = Verts[i].Position.X; zmax.Y = Verts[i].Position.Y; zmax.Z = Verts[i].Position.Z;
		}
	}

	// xspan = distance between the 2 points xmin and xmax squared.
	// same goes for yspan and zspan.
	dx = xmax.X - xmin.X;
	dy = xmax.Y - xmin.Y;
	dz = xmax.Z - xmin.Z;
	double xspan = dx*dx + dy*dy + dz*dz;

	dx = ymax.X - ymin.X;
	dy = ymax.Y - ymin.Y;
	dz = ymax.Z - ymin.Z;
	double yspan = dx*dx + dy*dy + dz*dz;

	dx = zmax.X - zmin.X;
	dy = zmax.Y - zmin.Y;
	dz = zmax.Z - zmin.Z;
	double zspan = dx*dx + dy*dy + dz*dz;


	// Set points dia1 and dia2 to the maximally separated pair
	// This will be the diameter of the initial sphere
	Vector3 dia1 = xmin;
	Vector3 dia2 = xmax;
	double maxspan = xspan;

	if (yspan > maxspan) {
		maxspan = yspan;
		dia1 = ymin;
		dia2 = ymax;
	}
	if (zspan > maxspan) {
		maxspan = zspan;
		dia1 = zmin;
		dia2 = zmax;
	}


	// Compute initial center and radius and radius squared
	Vector3 center;
	center.X = (dia1.X + dia2.X) / 2.0f;
	center.Y = (dia1.Y + dia2.Y) / 2.0f;
	center.Z = (dia1.Z + dia2.Z) / 2.0f;

	dx = dia2.X - center.X;
	dy = dia2.Y - center.Y;
	dz = dia2.Z - center.Z;

	double radsqr = dx*dx + dy*dy + dz*dz;
	double radius = sqrt(radsqr);


	// SECOND PASS:
	// Increment current sphere if any points fall outside of it.
	for (i=0; i<VertCount; i++) {

		dx = Verts[i].Position.X - center.X;
		dy = Verts[i].Position.Y - center.Y;
		dz = Verts[i].Position.Z - center.Z;

		double testrad2 = dx*dx + dy*dy + dz*dz;

		if (testrad2 > radsqr) {

			// this point was outside the old sphere, compute a new
			// center point and radius which contains this point
			double testrad = sqrt(testrad2);

			// adjust center and radius
			radius = (radius + testrad) / 2.0;
			radsqr = radius * radius;

			double oldtonew = testrad - radius;
			center.X = (radius * center.X + oldtonew * Verts[i].Position.X) / testrad;
			center.Y = (radius * center.Y + oldtonew * Verts[i].Position.Y) / testrad;
			center.Z = (radius * center.Z + oldtonew * Verts[i].Position.Z) / testrad;
		}
	}

	*set_center = center;
	*set_radius = radius;
}

/***********************************************************************************************
 * MeshBuilderClass::Optimize_Mesh -- "optimize" the mesh                                      *
 *                                                                                             *
 *    Generates the array of unique vertices and sets the vertex indices in each face.  Then   *
 *    all of the faces are sorted by material and then into stripping order.                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Optimize_Mesh(bool compute_normals)
{
	int faceidx;
	int vertidx;
	int facevertidx;

	int match_normals = 0;
	if (!compute_normals) {
		match_normals = 1;
	}

	VertexArrayClass unique_verts(FaceCount * 3,match_normals);

	/*
	** Find the min and max of the array of vertices
	*/
	Vector3 minv = Faces[0].Verts[0].Position;
	Vector3 maxv = Faces[0].Verts[0].Position;

	for (faceidx = 0; faceidx < FaceCount; faceidx++) {
		for (facevertidx = 0; facevertidx < 3; facevertidx++) {

			minv.Update_Min(Faces[faceidx].Verts[facevertidx].Position);
			maxv.Update_Max(Faces[faceidx].Verts[facevertidx].Position);
		}
	}

	/*
	** Tell the vertex array the bounds so that it can do better hashing.
	*/
	unique_verts.Set_Bounds(minv,maxv);

	/*
	** Build the array of unique vertices
	*/
	for (faceidx = 0; faceidx < FaceCount; faceidx++) {
		for (facevertidx = 0; facevertidx < 3; facevertidx++) {
			Faces[faceidx].VertIdx[facevertidx] =
				unique_verts.Submit_Vertex(Faces[faceidx].Verts[facevertidx]);
		}
	}

	/*
	** Assign the shared smoothing groups from each 'master' vertex
	** to each referring vertex.
	*/
	unique_verts.Propogate_Shared_Smooth_Groups();

	/*
	** Replace the vertex array with the new unique vertex array
	*/
	VertCount = unique_verts.VertCount;
	Verts = W3DNEWARRAY VertClass[VertCount];

	for (vertidx=0; vertidx<VertCount;vertidx++) {
		Verts[vertidx] = unique_verts[vertidx];
	}

	/*
	** Build a new array of faces, removing degenerate faces
	*/
	Remove_Degenerate_Faces();

	/*
	** Calculate the vertex normals if user didn't supply them.
	** Recompute the face normals just in case they changed a little
	** due to vertex welding.
	*/
	Compute_Face_Normals();
	if (compute_normals) {
		Compute_Vertex_Normals();
	}

	/*
	** An "Optimal" mesh will follow the following rules:
	**
	** Triangle Ordering:
	** - Triangles are in strip order
	** - Triangles are sorted by textures and detail textures in all passes (!?)
	** - Triangles are sorted by shader in all passes(!)
	**
	** Vertex Ordering:
	** - Skin matrix order if the mesh is being "skinned"
   ** - Few vertex material switches
	** - Strip ordering
	**
	** All verts should use the same data fields in each pass (alpha, dcg, ect)
	** All triangles must use same number of passes.
	**
	** Here is how I will attempt to get as "close as possible" to this
	** - find the pass and stage which has the "worst-case" number of textures
	**   and sort all polys with respect to the textures in that pass and stage
	** - see which pass has the most vertex materials (if any have more than one)
	**   and sort the vertices with respect to the mats in that pass
	** - put the triangles into strip order based on the pass and stage with the
	**   most texture switches.
	**
	** NOTE: Most meshes should be able to use *one* vertex material and *one* shader
	** We should throw up a warning flag in the asset manager whenever we encounter
	** a mesh which uses more than one of these per pass...
	*/
	Compute_Mesh_Stats();
	Stats.UVSplitCount = unique_verts.UVSplits;

	/*
	** Sort the polys by texture
	*/
	qsort(Faces,FaceCount,sizeof(FaceClass),Texture_Compare_Funcs[PolyOrderPass][PolyOrderStage]);

	/*
	** Sort the vertices by bone index and vertex material
	*/
	Sort_Vertices();

	/*
	** Strip optimize the mesh
	*/
	Strip_Optimize_Mesh();
	Verify_Face_Normals();
}


/***********************************************************************************************
 * MeshSaveClass::Strip_Optimize_Mesh -- optimize the mesh for triangle strips                 *
 *                                                                                             *
 * This algorithm was copied/modified from the surrender srModeller code so that we don't have *
 * to pass our models through the modeller at run-time.                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/6/98     GTH : Created.                                                                 *
 *   10/20/98   GTH : Modified to strip based on the desired texture channel                   *
 *   2/11/99    GTH : Added strip stats tracking                                               *
 *=============================================================================================*/
void MeshBuilderClass::Strip_Optimize_Mesh(void)
{
	WingedEdgeStruct *		edgehash[512];
	WingedEdgeStruct *		edgetab			= W3DNEWARRAY WingedEdgeStruct[FaceCount*3];
	WingedEdgePolyStruct *	pedges			= W3DNEWARRAY WingedEdgePolyStruct[FaceCount];

	FaceClass *					newpolys			= W3DNEWARRAY FaceClass[FaceCount];
	int *							newmat			= W3DNEWARRAY int[FaceCount];
	int *							premap			= W3DNEWARRAY int[FaceCount];
	int *							vtimestamp		= W3DNEWARRAY int[VertCount];

	int							vcount			= 0;
	int							polysinserted	= 0;
	int							lastmat			= 0;
	int							edgecount		= 0;
	int							i,j;

	// init the tables
	memset(edgehash,0,sizeof(WingedEdgeStruct *) * 512);
	memset(edgetab,0,sizeof(WingedEdgeStruct) * FaceCount * 3);
	for (i=0; i<FaceCount; i++) {
		premap[i] = -1;
	}
	for (i=0; i<VertCount; i++) {
		vtimestamp[i] = -1;
	}

	// Build the edge table for the mesh
	// Oct 20,1998: modifying so that every place we were looking at the
	// material index, we now use TextureIndex[PolyOrderPass][PolyOrderStage]
	for (i=0; i<FaceCount; i++) {

		int mat = Faces[i].TextureIndex[PolyOrderPass][PolyOrderStage];

		for (j=0; j<3; j++) {

			// build the WingedEdgePolyStruct for this face.  basically, for each edge,
			// see if we already have the edge and if so, link to it.  otherwise
			// add the edge to the edge table
			int						hash;
			int						v0,v1;
			WingedEdgeStruct *	edge;

			v0 = j;
			if (v0 < 2) v1 = v0+1; else v1 = 0;

			v0 = Faces[i].VertIdx[v0];
			v1 = Faces[i].VertIdx[v1];

			// order the vertices before hashing (hash depends on vert order)
			if (v0 > v1) { int tmp = v0; v0 = v1; v1 = tmp; }

			// hash value for the edge
			hash = (v0 + v1*119)&511;

			// seek edge from hash table
			for (edge = edgehash[hash]; edge; edge = edge->Next) {
				if (edge->Vertex[0] == v0 && edge->Vertex[1] == v1 && edge->MaterialIdx == mat)
					break;
			}

			if (edge) {

				// found the edge
				edge->Poly[1] = i;

			} else {

				// create new edge and link it to hash table
				edge = edgetab + edgecount++;
				edge->Next = edgehash[hash];
				edgehash[hash] = edge;
				edge->Vertex[0] = v0;
				edge->Vertex[1] = v1;
				edge->Poly[0] = i;
				edge->Poly[1] = -1;
				edge->MaterialIdx  = mat;
			}

			pedges[i].Edge[j] = edge;
		}
	}

	// the following loop inserts polygons into a new polygon list until
	// all polygons have been inserted. Internally it attempts to create
	// as long strips as possible while minimizing material switches
	// and maintaining vertex reusage coherency to optimize geometry
	// engine performance.
	while (polysinserted < FaceCount) {

		int		startpoly	= -1;									// best polygon found so far
		int		bestc			= (1<<29);							// best polygon weight value found so far
		int		startpass	= 0;									// should we start from pass 0 or 1?
		int		findpass;											// 0 = same material only, 1 = any polygon
		int		c;														// c = number of shared edges

		// first attempt to minimize material switches by choosing a polygon with same material
		// as the starting poly. Basically what we want is the poly with same material with
		// least shared edges and most recent vertices (as we'd like to start the strip from
		// a 'corner polygon'). This pass is done only if mesh has multiple materials.
		// The second pass scans through all polygons.

		// this loop is O(N*N) -> might turn a bit nasty on larger meshes. Try
		// to figure out a way to solve the problem...

		for (findpass = startpass; findpass < 2; findpass++)
		{
			// loop through all polygons
			for (i = 0; i < FaceCount; i++) {

				// if polygon not picked yet
				if (premap[i]==-1) {

					// if material mismatch, cannot choose poly (except in pass 1)
					if (findpass == 0 && Faces[i].TextureIndex[PolyOrderPass][PolyOrderStage] != lastmat)
						continue;

					// calculate number of shared edges
					for (c = 0, j = 0; j < 3; j++) {

						// if edge j is shared by two tris,
						// use a weight factor of vCount for each edge
						if (pedges[i].Edge[j]->Poly[1] >= 0) {
							c += (vcount+1);
						}
					}

					// calculate delta vertex timestamp
					for (j = 0; j < 3; j++) {
						c += (vcount-vtimestamp[Faces[i].VertIdx[j]]);
					}

					// if better than current best pick
					if (c < bestc) {
						bestc			= c;
						startpoly	= i;
					}
				}
			}

			// if we managed to find a suitable starting poly
			if (startpoly != -1)
				break;
		}

		// track the fact that we created a new strip:
		Stats.StripCount++;

		// update the material index
		lastmat						= Faces[startpoly].TextureIndex[PolyOrderPass][PolyOrderStage];
		newmat[polysinserted]	= Faces[startpoly].TextureIndex[PolyOrderPass][PolyOrderStage];

		// Add the selected polygon to the new polygon list.
		// for each edge of start poly, see if the "other" polygon using that edge
		// is untaken, if so, store the new polygon such that that edge index is last

		bool found_shared_edge = false;
		newpolys[polysinserted] = Faces[startpoly];
		FaceClass * newpoly = &(newpolys[polysinserted]);

		for (int edge_index = 0; (edge_index < 3) && !found_shared_edge; edge_index++) {

			for (int side_index = 0; (side_index < 2) && !found_shared_edge; side_index++) {

				// if this polygon is not the startpoly and it is not "taken", then this edge is ok!
				int poly = pedges[startpoly].Edge[edge_index]->Poly[side_index];
				if ((poly != -1) && (poly != startpoly) && (premap[poly] == -1)) {

					// find vert which is not on the final edge
					int first_vert = -1;
					for (int vidx=0; vidx<3; vidx++) {
						if (	(newpoly->VertIdx[vidx] != pedges[startpoly].Edge[edge_index]->Vertex[0]) &&
								(newpoly->VertIdx[vidx] != pedges[startpoly].Edge[edge_index]->Vertex[1])) {

								first_vert = newpoly->VertIdx[vidx];
								break;
						}
					}
					assert(first_vert != -1);

					// rotate the vertex indices until first_vert is the index of VertIdx[0]
					while (newpoly->VertIdx[0] != first_vert) {
						int tmp = newpoly->VertIdx[0];
						newpoly->VertIdx[0] = newpoly->VertIdx[1];
						newpoly->VertIdx[1] = newpoly->VertIdx[2];
						newpoly->VertIdx[2] = tmp;
					}

					// ok, we found a shareable edge and adjusted our poly so the strip
					// will start with it.  Now break out of this loop...
					found_shared_edge = true;
					break;
				}
			}
		}

		// if a shared edge wasn't found, just copy the poly
		if (!found_shared_edge) {
			newpolys[polysinserted] = Faces[startpoly];
		}

		// Increment the count. Mark the vertices as used (i.e. update their timestamps)
		premap[startpoly]			= polysinserted;
		polysinserted++;

		for (i = 0; i < 3; i++) {
			if (vtimestamp[Faces[startpoly].VertIdx[i]]==-1) {
				vtimestamp[Faces[startpoly].VertIdx[i]] = vcount++;
			}
		}

		// If we have no shared edges, this is a lone poly, start another strip
		if (pedges[startpoly].Edge[0]->Poly[1] == -1 &&
			pedges[startpoly].Edge[1]->Poly[1] == -1 &&
			pedges[startpoly].Edge[2]->Poly[1] == -1)
				continue;

		// Build the strip starting from the polygon chosen in the previous loop
		int		vFIFO[2];											// vertex fifo
		int		scnt		= 0;										// strip index count (for poly order flipping)
		int		nextpoly	= startpoly;

		vFIFO[0] = newpoly->VertIdx[1];				// setup the vFIFO
		vFIFO[1] = newpoly->VertIdx[2];

		while (nextpoly != -1)
		{
			startpoly	= nextpoly;
			nextpoly		= -1;

			for (i = 0; i < 3; i++) {

				// if edge 'i' of startpoly matches the vertices in the fifo
				if	((pedges[startpoly].Edge[i]->Vertex[0] == vFIFO[0] && pedges[startpoly].Edge[i]->Vertex[1] == vFIFO[1]) ||
					(pedges[startpoly].Edge[i]->Vertex[1] == vFIFO[0] && pedges[startpoly].Edge[i]->Vertex[0] == vFIFO[1]))
				{

					for (j = 0; j < 2; j++) {

						// if poly 'j' attached to this edge has not been used already, use it!
						if (pedges[startpoly].Edge[i]->Poly[j] > -1 &&
							premap[pedges[startpoly].Edge[i]->Poly[j]] == -1)
						{
							nextpoly = pedges[startpoly].Edge[i]->Poly[j];
							goto found;
						}
					}
				}
			}

			// couldn't find another poly, break from loop
			break;

			found:;

			// now, find the new vertex (two verts are on the edge, find the third)
			int nw = -1;
			for (i = 0; i < 3; i++) {
				if (Faces[nextpoly].VertIdx[i] != vFIFO[0] && Faces[nextpoly].VertIdx[i] != vFIFO[1]) {
					nw = i;
					break;
				}
			}
			assert(nw != -1);

			int new_vindex = Faces[nextpoly].VertIdx[nw];

			newmat[polysinserted] = Faces[nextpoly].TextureIndex[PolyOrderPass][PolyOrderStage];

			// add the poly to the newpolys array.
			newpolys[polysinserted].VertIdx[0] = vFIFO[0];
			newpolys[polysinserted].VertIdx[1] = vFIFO[1];
			newpolys[polysinserted].VertIdx[2] = new_vindex;

			// if we are on an even triangle, swap the vertex ordering
			if (!(scnt&1)) {
				int tmp = newpolys[polysinserted].VertIdx[0];
				newpolys[polysinserted].VertIdx[0] = newpolys[polysinserted].VertIdx[1];
				newpolys[polysinserted].VertIdx[1] = tmp;
			}

			// push the new vertex into the fifo
			vFIFO[0] = vFIFO[1];
			vFIFO[1] = new_vindex;

			if (vtimestamp[new_vindex]==-1) {
				vtimestamp[new_vindex] = vcount++;
			}

			premap[nextpoly] = polysinserted++;
			scnt++;
		}

		// scnt+1 is the number of polys that were added to the strip
		Stats.AvgStripLength += scnt+1;
		if (scnt+1 > Stats.MaxStripLength) {
			Stats.MaxStripLength = scnt+1;
		}
	}

	// Use the premap array to get the rest of the info into the new face table,
	for (i=0; i<FaceCount; i++) {

		int old_idx = i;
		int new_idx = premap[i];

		for (int pass=0; pass < MAX_PASSES; pass++) {
			for (int stage=0; stage < MAX_STAGES; stage++) {
				newpolys[new_idx].TextureIndex[pass][stage] = Faces[old_idx].TextureIndex[pass][stage];
			}
			newpolys[new_idx].ShaderIndex[pass] = Faces[old_idx].ShaderIndex[pass];
		}

		newpolys[new_idx].SmGroup =		Faces[old_idx].SmGroup;
		newpolys[new_idx].Index =			Faces[old_idx].Index;
		newpolys[new_idx].Attributes =	Faces[old_idx].Attributes;
		newpolys[new_idx].AddIndex =		Faces[old_idx].AddIndex;
		newpolys[new_idx].Normal =			Faces[old_idx].Normal;
		newpolys[new_idx].Dist =			Faces[old_idx].Dist;
		newpolys[new_idx].SurfaceType =	Faces[old_idx].SurfaceType;

		// just checking
		assert(newmat[new_idx] == Faces[old_idx].TextureIndex[PolyOrderPass][PolyOrderStage]);
	}

	// then install the pointer to the new face table.
	FaceClass * oldfaces = Faces;
	Faces = newpolys;
	delete[] oldfaces;
	AllocFaceCount = FaceCount;

	// release allocated memory
	delete[] edgetab;
	delete[] pedges;
	delete[] premap;
	delete[] newmat;
	delete[] vtimestamp;

	// finish the computation of the average strip length
	Stats.AvgStripLength /= Stats.StripCount;
}



/***********************************************************************************************
 * MeshBuilderClass::Grow_Face_Array -- increases the size of the face array                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Grow_Face_Array(void)
{
	int oldcount = AllocFaceCount;
	FaceClass * oldfaces = Faces;

	AllocFaceCount += AllocFaceGrowth;
	Faces = W3DNEWARRAY FaceClass[AllocFaceCount];

	for (int i=0; i<oldcount; i++) {
		Faces[i] = oldfaces[i];
	}

	delete[] oldfaces;
}

/***********************************************************************************************
 * MeshBuilderClass::Sort_Vertices -- sorts vertices                                           *
 *                                                                                             *
 * the vertices are sorted first by bone index, second by vertex material.                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/1/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void MeshBuilderClass::Sort_Vertices(void)
{
	/*
	** Sort the vertices
	*/
	qsort(Verts,VertCount,sizeof(VertClass),vertex_compare);

	/*
	** Build a vertex remap table (table[old_index] = new_index) and
	** reset each vertex's UniqueIndex.
	*/
	int * vertex_remap_table = W3DNEWARRAY int[VertCount];
	for (int vi=0; vi<VertCount; vi++) {
		vertex_remap_table[Verts[vi].UniqueIndex] = vi;
	}

	/*
	** Remap the faces' vertex indices
	*/
	for (int fi=0; fi<FaceCount; fi++) {
		for (int vi=0; vi<3; vi++) {
			int old_index = Faces[fi].VertIdx[vi];
			Faces[fi].VertIdx[vi] = vertex_remap_table[old_index];
		}
	}

	/*
	** Deallocate the temporary remap table
	*/
	delete[] vertex_remap_table;
}


/*
** Compare functions for qsorting the polygons by texture.
*/

int face_material_compare(const void *elem1, const void *elem2 )
{
	MeshBuilderClass::FaceClass * f0 = (MeshBuilderClass::FaceClass *)elem1;
	MeshBuilderClass::FaceClass * f1 = (MeshBuilderClass::FaceClass *)elem2;

	if (f0->TextureIndex[0][0] < f1->TextureIndex[0][0]) return -1;
	if (f0->TextureIndex[0][0] > f1->TextureIndex[0][0]) return 1;
	return 0;
}

inline int tex_compare(const void * elem1,const void * elem2,int pass,int stage)
{
	MeshBuilderClass::FaceClass * f0 = (MeshBuilderClass::FaceClass *)elem1;
	MeshBuilderClass::FaceClass * f1 = (MeshBuilderClass::FaceClass *)elem2;

	/*
	** Primarily, sort by texture
	*/
	if (f0->TextureIndex[pass][stage] < f1->TextureIndex[pass][stage]) return -1;
	if (f0->TextureIndex[pass][stage] > f1->TextureIndex[pass][stage]) return 1;

	/*
	** Secondarily, sort by vertex material index
	*/
	if (f0->Verts[0].VertexMaterialIndex[pass] < f1->Verts[0].VertexMaterialIndex[pass]) return -1;
	if (f0->Verts[0].VertexMaterialIndex[pass] > f1->Verts[0].VertexMaterialIndex[pass]) return 1;

	return 0;
}

int pass0_stage0_compare(const void *elem1, const void *elem2)
{
	return tex_compare(elem1,elem2,0,0);
}

int pass0_stage1_compare(const void *elem1, const void *elem2)
{
	return tex_compare(elem1,elem2,0,1);
}

int pass1_stage0_compare(const void *elem1, const void *elem2)
{
	return tex_compare(elem1,elem2,1,0);
}

int pass1_stage1_compare(const void *elem1, const void *elem2)
{
	return tex_compare(elem1,elem2,1,1);
}

int pass2_stage0_compare(const void *elem1, const void *elem2)
{
	return tex_compare(elem1,elem2,2,0);
}

int pass2_stage1_compare(const void *elem1, const void *elem2)
{
	return tex_compare(elem1,elem2,2,1);
}

int pass3_stage0_compare(const void *elem1, const void *elem2)
{
	return tex_compare(elem1,elem2,3,0);
}

int pass3_stage1_compare(const void *elem1, const void *elem2)
{
	return tex_compare(elem1,elem2,3,1);
}

int vertex_compare(const void *elem1, const void *elem2)
{
	MeshBuilderClass::VertClass * v0 = (MeshBuilderClass::VertClass *)elem1;
	MeshBuilderClass::VertClass * v1 = (MeshBuilderClass::VertClass *)elem2;

	/*
	** Sort first by bone index, then by vertex material in pass0
	*/
	if (v0->BoneIndex < v1->BoneIndex) return -1;
	if (v0->BoneIndex > v1->BoneIndex) return 1;

	if (v0->VertexMaterialIndex[0] < v1->VertexMaterialIndex[0]) return -1;
	if (v0->VertexMaterialIndex[0] > v1->VertexMaterialIndex[0]) return 1;

	return 0;
}


