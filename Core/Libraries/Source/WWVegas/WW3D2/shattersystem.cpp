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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/shattersystem.cpp                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/03/01 4:57p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "shattersystem.h"
#include "assetmgr.h"
#include "mesh.h"
#include "meshmdl.h"
#include "dynamesh.h"
#include "htree.h"
#include "plane.h"
#include "simplevec.h"
#include "wwstring.h"
#include "vp.h"
#include "meshmatdesc.h"
#include <stdlib.h>

/*
** Debug logging for the shatter system
*/
#define SHATTER_DEBUG_LOG_ENABLED	0
#if (SHATTER_DEBUG_LOG_ENABLED)
#define SHATTER_DEBUG_SAY(x)			WWDEBUG_SAY(x)
#else
#define SHATTER_DEBUG_SAY(x)
#endif



#define SHATTER_PATTERN_FORMAT	"ShatterPlanes%d"

#define BPT_FRONT						0x01
#define BPT_BACK						0x02
#define BPT_ON							0x04
#define BPT_BOTH						0x08
#define BPT_EPSILON					0.0001f
#define BPT_COINCIDENCE_EPSILON	0.000001f
 

/**
** Mesh Material Parameters
*/
class MeshMtlParamsClass
{
public:
	MeshMtlParamsClass(MeshModelClass * model);
	~MeshMtlParamsClass(void);

	int							PassCount;

	/*
	** NOTE: currently I do not fully support stage 1 textures or texture arrays in any pass.
	** The uv coordinates will be computed but the textures will not be set up.
	*/
	unsigned *					DCG[MeshMatDescClass::MAX_PASSES];
	unsigned *					DIG[MeshMatDescClass::MAX_PASSES];
	Vector2 *					UV[MeshMatDescClass::MAX_PASSES][MeshMatDescClass::MAX_TEX_STAGES];
//	Vector3i *					UVIndexArray[MeshMatDescClass::MAX_PASSES];
};



/**
** VertexClass
** Temporary representation of vertices as they are being clipped
*/
class VertexClass
{
public:

	VertexClass(void);
	VertexClass(const VertexClass & that);
	VertexClass &		operator = (const VertexClass & that);
	
	int					Which_Side(const PlaneClass & plane) const;
	
	Vector3				Position;
	Vector3				Normal;
	
	int					PassCount;
	unsigned				DCG[MeshMatDescClass::MAX_PASSES];
	unsigned				DIG[MeshMatDescClass::MAX_PASSES];
	Vector2				TexCoord[MeshMatDescClass::MAX_PASSES][MeshMatDescClass::MAX_TEX_STAGES];
	
	static void			Lerp(const VertexClass & v0,const VertexClass & v1,float lerp,VertexClass * res);
	static void			Intersect_Plane(const VertexClass & v0,const VertexClass & v1,const PlaneClass & plane,VertexClass * res);
};

/**
** PolygonClass
** Temporary representation of polygons as they are being clipped
*/
class PolygonClass
{
public:

	enum { BPT_POLY_MAX_VERTS = 24 };

	PolygonClass(void);
	PolygonClass(const PolygonClass & that);
	PolygonClass(const VertexClass * points, int num);
	PolygonClass &				operator = (const PolygonClass & that);

	// Accessors
	const VertexClass &		operator[] (int i) const { return Verts[i]; }
	VertexClass &				operator[] (int i) { return Verts[i]; }

	int							Get_Vertex_Count(void) const			{ return NumVerts; }
	int							Get_Material_ID(void) const			{ return MaterialId; }
	const PlaneClass &		Get_Plane(void) const					{ return Plane; }

	void							Set_Vertex_Count(int count)			{ NumVerts = count; }
	void							Set_Material_Id(int id)					{ MaterialId = id; }
	void							Set_Plane(const PlaneClass & plane)	{ Plane = plane; }

	// Operations
	void							Compute_Plane(void);
	int							Which_Side(const PlaneClass & plane) const;
	void							Split(const PlaneClass & plane,PolygonClass & front,PolygonClass & back) const;
	bool							Is_Degenerate(void);
	bool							Salvage_Degenerate(void);

public:

	int							MaterialId;
	int							NumVerts;
	VertexClass					Verts[BPT_POLY_MAX_VERTS];
	
	PlaneClass					Plane;

};

/**
** BSPClass
** Representation of a node in a BSP clipping tree in the shatter system
** Meshes are "shattered" by clipping them with a fixed BSP tree.
** The resulting leaf polygons are turned into separate dynamic objects
** which are thrown around in the world.  The BSP tree is constructed 
** using an HTree where each transform in the tree is used as a clipping 
** plane.  The plane is defined by the x-y plane of the coordinate system 
** (i.e. z-axis is the normal of the plane, origin is a point on the plane).
** Leaf nodes of this tree will have two indices.  These are indices into
** the MeshFragments array where they put the polygons in thier front and
** back half-spaces.
*/
class BSPClass
{
public:
	BSPClass(HTreeClass * tree,int bone_index,int & leaf_index);
	~BSPClass(void);

	const PlaneClass &	Get_Plane(void)			{ return Plane; }
	BSPClass *				Get_Front_Child(void)	{ return Front; }
	BSPClass *				Get_Back_Child(void)		{ return Back; }

	void						Clip_Polygon(const PolygonClass & poly);

protected:

	void						Set_Plane_From_Transform(const Matrix3D & tm);
	
	PlaneClass				Plane;					// plane equation
	BSPClass *				Front;					// pointers to child clipping planes
	BSPClass *				Back;
	int						FrontLeafIndex;		// if a leaf node, has two mesh fragment indices
	int						BackLeafIndex;
};


/***********************************************************************************************
**
** Static Variables
** ShatterPatterns - Array of BSP-trees for clipping (shattering) messhes 
** ClippedPolygonPool - Array of polygon arrays, one per leaf.
** MeshFragments - Array of resultant clipped meshes, one per leaf 
** TmpVertPositions - Workspace for transforming vertex positions
** TmpVertNormals - Workspace for transforming vertex normals
**
***********************************************************************************************/

enum { MAX_MESH_FRAGMENTS = 32 };
static SimpleDynVecClass<BSPClass *>				ShatterPatterns;
static SimpleDynVecClass<PolygonClass>				ClipPools[MAX_MESH_FRAGMENTS];
static SimpleDynVecClass<DynamicMeshClass	*>		MeshFragments(MAX_MESH_FRAGMENTS);
static SimpleVecClass<Vector3>						TmpVertPositions(256);
static SimpleVecClass<Vector3>						TmpVertNormals(256);


/***********************************************************************************************
**
** Local static utility functions
**
***********************************************************************************************/
static Vector3 * _get_temp_vertex_position_array(int count)
{
	if (TmpVertPositions.Length() < count) {
		TmpVertPositions.Resize(count);
	}
	return &(TmpVertPositions[0]);
}

static Vector3 * _get_temp_vertex_normal_array(int count)
{
	if (TmpVertNormals.Length() < count) {
		TmpVertNormals.Resize(count);
	}
	return &(TmpVertNormals[0]);
}

/***********************************************************************************************
**
** MeshMtlParamsClass Implementation
**
***********************************************************************************************/
MeshMtlParamsClass::MeshMtlParamsClass(MeshModelClass * model)
{
	PassCount = model->Get_Pass_Count();
	
	for (int ipass=0; ipass<MeshMatDescClass::MAX_PASSES; ipass++) {

		DCG[ipass] = model->Get_DCG_Array(ipass);
		DIG[ipass] = model->Get_DIG_Array(ipass);		
//		UVIndexArray[ipass] = model->Get_UVIndex_Array(ipass,false);
		for (int istage=0; istage<MeshMatDescClass::MAX_TEX_STAGES; istage++) {
			UV[ipass][istage] = const_cast<Vector2*>(model->Get_UV_Array(ipass,istage));
		}
	}
}

MeshMtlParamsClass::~MeshMtlParamsClass(void)
{
}


/***********************************************************************************************
**
** VertexClass Implementation
**
***********************************************************************************************/
VertexClass::VertexClass(void) :
	Position(0,0,0),
	Normal(0,0,1),
	PassCount(0)
{
	for (int ipass=0; ipass<MeshMatDescClass::MAX_PASSES; ipass++) {
		DCG[ipass]=0xffffffff;
		DIG[ipass]=0xffffffff;
		for (int istage=0; istage<MeshMatDescClass::MAX_TEX_STAGES; istage++) {
			TexCoord[ipass][istage].Set(0,0);
		}
	}
}

VertexClass::VertexClass(const VertexClass & that)
{
	Position = that.Position;
	Normal = that.Normal;
	PassCount = that.PassCount;

	for (int ipass=0; ipass<PassCount; ipass++) {
		DCG[ipass] = that.DCG[ipass];
		DIG[ipass] = that.DIG[ipass];
		for (int istage=0; istage<MeshMatDescClass::MAX_TEX_STAGES; istage++) {
			TexCoord[ipass][istage] = that.TexCoord[ipass][istage];
		}
	}
}

VertexClass & VertexClass::operator = (const VertexClass & that)
{
	if (this != &that) {
		Position = that.Position;
		Normal = that.Normal;
		PassCount = that.PassCount;
		for (int ipass=0; ipass<PassCount; ipass++) {
			DCG[ipass] = that.DCG[ipass];
			DIG[ipass] = that.DIG[ipass];
			for (int istage=0; istage<MeshMatDescClass::MAX_TEX_STAGES; istage++) {
				TexCoord[ipass][istage] = that.TexCoord[ipass][istage];
			}
		}
	}
	return *this;
}

void VertexClass::Lerp
(
	const VertexClass & v0,
	const VertexClass & v1,
	float lerp,
	VertexClass * res
)
{
	assert(lerp >= -BPT_EPSILON);
	assert(lerp <= 1.0 + BPT_EPSILON);
	assert(v0.PassCount==v1.PassCount);
	res->PassCount = v0.PassCount;

	// interpolate position
	Vector3::Lerp(v0.Position,v1.Position,lerp,&(res->Position));

	// interpolate normal, renormalize
	Vector3::Lerp(v0.Normal,v1.Normal,lerp,&(res->Normal));
	res->Normal.Normalize();

	// interpolate material properies
	for (int ipass=0; ipass<v0.PassCount; ipass++) {
		Vector4 dcg_v0=DX8Wrapper::Convert_Color(v0.DCG[ipass]);
		Vector4 dcg_v1=DX8Wrapper::Convert_Color(v1.DCG[ipass]);
		Vector4 dig_v0=DX8Wrapper::Convert_Color(v0.DIG[ipass]);
		Vector4 dig_v1=DX8Wrapper::Convert_Color(v1.DIG[ipass]);
		Vector4::Lerp(dcg_v0,dcg_v1,res->DCG[ipass]);
		Vector4::Lerp(dig_v0,dig_v1,res->DIG[ipass]);
//		Vector4::Lerp(v0.DCG[ipass],v1.DCG[ipass],lerp,&(res->DCG[ipass]));
//		Vector4::Lerp(v0.DIG[ipass],v1.DIG[ipass],lerp,&(res->DIG[ipass]));
		for (int istage=0; istage<MeshMatDescClass::MAX_TEX_STAGES; istage++) {
			Vector2::Lerp(v0.TexCoord[ipass][istage],v1.TexCoord[ipass][istage],lerp,&(res->TexCoord[ipass][istage]));
		}
	}
}

int VertexClass::Which_Side(const PlaneClass & plane) const
{
	float d = Vector3::Dot_Product(plane.N,Position);
	d -= plane.D;

	if (d > BPT_EPSILON) {
		return BPT_FRONT;
	} 

	if (d < -BPT_EPSILON) {
		return BPT_BACK;
	}

	return BPT_ON;
}

void VertexClass::Intersect_Plane
(
	const VertexClass & p0,
	const VertexClass & p1,
	const PlaneClass & plane,
	VertexClass * res
)
{
	float alpha = 0.0f;
	plane.Compute_Intersection(p0.Position,p1.Position,&alpha);
	VertexClass::Lerp(p0,p1,alpha,res);				
}



/***********************************************************************************************
**
** PolygonClass Implementation
**
***********************************************************************************************/
PolygonClass::PolygonClass(void) :
	NumVerts(0)
{
}

PolygonClass::PolygonClass(const PolygonClass & that)
{
	NumVerts = that.NumVerts;
	for (int i=0;i<NumVerts;i++) {
		Verts[i] = that.Verts[i];
	}
}

PolygonClass::PolygonClass(const VertexClass * points, int num)
{
	NumVerts = num;
	for (int i=0; i<NumVerts; i++) {
		Verts[i] = points[i];
	}
}

PolygonClass & PolygonClass::operator = (const PolygonClass & that)
{
	if (this != &that) {
		MaterialId = that.MaterialId;
		NumVerts = that.NumVerts;
		Plane = that.Plane;
		for (int i=0;i<NumVerts;i++) {
			Verts[i] = that.Verts[i];
		}
	}
	return * this;
}

void PolygonClass::Compute_Plane(void)
{
	double nx = 0;
	double ny = 0;
	double nz = 0;
	double ax = 0;
	double ay = 0;
	double az = 0;

	int i,j;

	for (i=0; i<NumVerts; i++) {
		j = (i+1) % NumVerts;
		
		nx += (double)(Verts[i].Position.Y - Verts[j].Position.Y) * (double)(Verts[i].Position.Z + Verts[j].Position.Z);
		ny += (double)(Verts[i].Position.Z - Verts[j].Position.Z) * (double)(Verts[i].Position.X + Verts[j].Position.X);
		nz += (double)(Verts[i].Position.X - Verts[j].Position.X) * (double)(Verts[i].Position.Y + Verts[j].Position.Y);

		ax += (double)Verts[i].Position.X;
		ay += (double)Verts[i].Position.Y;
		az += (double)Verts[i].Position.Z;
	}

	ax /= (double)NumVerts;
	ay /= (double)NumVerts;
	az /= (double)NumVerts;

	double len = WWMath::Sqrt(nx*nx + ny*ny + nz*nz);
	nx /= len;
	ny /= len;
	nz /= len;
	
	Plane.Set(Vector3(nx,ny,nz),Vector3(ax,ay,az));
}

int PolygonClass::Which_Side(const PlaneClass & plane) const
{
	int side_mask = 0;
	for (int i=0; i<NumVerts;i++) {
		side_mask |= Verts[i].Which_Side(plane);
	}

	// check if all verts are "ON"
	if (side_mask == BPT_ON) {
		return BPT_ON;
	}

	// check if all verts are either "ON" or "FRONT"
	if ((side_mask & ~(BPT_FRONT | BPT_ON)) == 0) {
		return BPT_FRONT;
	}

	// check if all verts are either "ON" or "BACK"
	if ((side_mask & ~(BPT_BACK | BPT_ON)) == 0) {
		return BPT_BACK;
	}

	// otherwise, poly spans the plane.
	return BPT_BOTH;
}

void PolygonClass::Split(const PlaneClass & plane,PolygonClass & front,PolygonClass & back) const
{
	front = *this;
	back = *this;
	front.NumVerts = back.NumVerts = 0;
	assert(Which_Side(plane) == BPT_BOTH);

	VertexClass point;
	front.NumVerts = 0; 
	back.NumVerts = 0;

	// find a vertex on one side or the other
	int side = BPT_ON;
	int i;
	for (i = 0; (i < NumVerts) && ((side = Verts[i].Which_Side(plane)) == BPT_ON); i++);

	// perform clipping
	int iprev = i;
	int sideprev = side;
	int sidelastdefinite = 0;
	
	i = (i+1) % NumVerts;

	for (int j=0; j<NumVerts; j++) { 
		
		side = Verts[i].Which_Side(plane);
		
		if (sideprev == BPT_FRONT) {

			if (side == BPT_FRONT) {
			
				// Previous vertex was in front of plane and this vertex is in
				// front of the plane so we emit this vertex in the front poly
				front.Verts[(front.NumVerts)++] = Verts[i];

			} else if (side == BPT_ON) {

				// Previous vert was in front, this vert is "on" so emit
				// the vertex into the front poly.
				sidelastdefinite = BPT_FRONT;
				front.Verts[(front.NumVerts)++] = Verts[i];
			
			} else { // side == BPT_BACK

				// Previous vert was in front, this vert is behind, compute
				// the intersection and emit the point in both the front
				// and back polys.  Then continue the edge into the back halfspace
				VertexClass::Intersect_Plane(Verts[iprev],Verts[i],plane,&point);
				front.Verts[(front.NumVerts)++] = point;
				back.Verts[(back.NumVerts)++] = point;
				back.Verts[(back.NumVerts)++] = Verts[i];

			}

		} else if (sideprev == BPT_BACK) {

			if (side == BPT_FRONT) {

				// segment is going from the back halfspace to the front halfspace
				// compute the intersection and emit it in both polys, then continue
				// the edge into the front halfspace.
				VertexClass::Intersect_Plane(Verts[iprev],Verts[i],plane,&point);
				back.Verts[(back.NumVerts)++] = point;
				front.Verts[(front.NumVerts)++] = point;
				front.Verts[(front.NumVerts)++] = Verts[i];
			
			} else if (side == BPT_ON) {
			
				// segment went from back halfspace to "on" the plane.  Emit
				// the vertex into the back poly and remember that we came 
				// from the back halfspace.
				sidelastdefinite = BPT_BACK;
				back.Verts[(back.NumVerts)++] = Verts[i];

			} else {  // side == BPT_BACK

				// segment is completely in the back halfspace, just emit the
				// vertex into the back poly
				back.Verts[(back.NumVerts)++] = Verts[i];

			}
		
		} else if (sideprev == BPT_ON) {

			if (side == BPT_FRONT) {

				// segment is on the plane
				if (sidelastdefinite == BPT_BACK) {
					front.Verts[(front.NumVerts)++] =	Verts[iprev];
				}
				front.Verts[(front.NumVerts)++] = Verts[i];
			
			} else if (side == BPT_ON) {

				if (sidelastdefinite == BPT_FRONT) {
					front.Verts[(front.NumVerts)++] = Verts[i];
				} else {
					back.Verts[(back.NumVerts)++] = Verts[i];
				}
			
			} else { // side == BPT_BACK

				if (sidelastdefinite == BPT_FRONT) {
					back.Verts[(back.NumVerts)++] =	Verts[iprev];
				}
				back.Verts[(back.NumVerts)++] = Verts[i];
			}
		} else {
			WWASSERT_PRINT(0,"PolygonClass::Split : invalid side\n");
		}

		sideprev = side;
		iprev = i;
		i = (i+1)%NumVerts;

	}

	front.Compute_Plane();
	back.Compute_Plane();

	// check the two polygons
	if (front.Is_Degenerate()) {
		front.Salvage_Degenerate();
	}

	if (back.Is_Degenerate()) {
		back.Salvage_Degenerate();
	}
}


bool PolygonClass::Is_Degenerate(void)
{
	int i,j;

	if (NumVerts <= 2) {
		WWDEBUG_SAY(("Degenerate Poly - fewer than 3 vertices\r\n"));
		return true;
	}

	for (i=0; i<NumVerts; i++) {
		for (j = i+1; j < NumVerts; j++) {
			
			float delta = (Verts[i].Position - Verts[j].Position).Length();
			if (delta < BPT_COINCIDENCE_EPSILON) {
				WWDEBUG_SAY(("Degenerate Poly - coincident vertices!\r\n"));
				return true;
			}
		}
	}

	for (i=0; i<NumVerts; i++) {
		int side = Verts[i].Which_Side(Plane);
		if (side != BPT_ON) {

			// hmmm, try to recalculate the plane, if it is still bad, then give up
			Compute_Plane();

			if (Verts[i].Which_Side(Plane) != BPT_ON) {
				WWDEBUG_SAY(("Degenerate Poly - invalid plane!\r\n"));
				return true;
			}
		}
	}

	return false;
}

bool PolygonClass::Salvage_Degenerate(void)
{
	/*
	** About all we can do is combine sequential vertices which are co-incident
	*/
	int i = 0;
	while (i < NumVerts) {

		float delta = (Verts[i].Position - Verts[i+1].Position).Length();
		if (delta < BPT_COINCIDENCE_EPSILON) {
			
			for (int j=i+1; j<NumVerts-1; j++) {
				Verts[j] = Verts[j+1];
			}

			NumVerts--;
		
		} else {

			i++;
		}
	}

	return !Is_Degenerate();
}


/***********************************************************************************************
**
** BSPClass Implementation
**
***********************************************************************************************/

BSPClass::BSPClass(HTreeClass * tree,int bone_index,int & leaf_index) :
	Plane(0,0,1,0),
	Front(NULL),
	Back(NULL),
	FrontLeafIndex(-1),
	BackLeafIndex(-1)
{
	// initialize our plane equation from the transform
	Set_Plane_From_Transform(tree->Get_Transform(bone_index));
	int front = -1;
	int back = -1;

	// search for a front and back child of this bone
	for (int ibone=0; ibone < tree->Num_Pivots(); ibone++) {
		if (tree->Get_Parent_Index(ibone) == bone_index) {

			// found a child, now see if it is the front or back child
			Vector3 point = tree->Get_Transform(ibone).Get_Translation();
			if (Plane.In_Front(point)) {
				front = ibone;
			} else {
				back = ibone;
			}

		}
	}

	// Recurse if we have children, otherwise assign leaf indices
	if (front != -1) {
		Front = W3DNEW BSPClass(tree,front,leaf_index);
	} else {
		FrontLeafIndex = leaf_index++;
	}

	if (back != -1) {
		Back = W3DNEW BSPClass(tree,back,leaf_index);
	} else {
		BackLeafIndex = leaf_index++;
	}
}

BSPClass::~BSPClass(void)
{
	if (Front != NULL) {
		delete Front;
	}
	if (Back != NULL) {
		delete Back;
	}
	Front = Back = NULL;
}

void BSPClass::Set_Plane_From_Transform(const Matrix3D & tm)
{
	Plane.Set(tm.Get_Z_Vector(),tm.Get_Translation());
}

void BSPClass::Clip_Polygon(const PolygonClass & polygon)
{
	PolygonClass front_poly,back_poly;
	front_poly.Set_Vertex_Count(0);
	back_poly.Set_Vertex_Count(0);
	
	switch(polygon.Which_Side(Plane)) 
	{
	case BPT_FRONT: case BPT_ON:
		front_poly = polygon;
		break;
	case BPT_BACK:
		back_poly = polygon;
		break;
	default:
		polygon.Split(Plane,front_poly,back_poly);
		break;
	};

	// Process the front halfspace: Recurse if we have a child clipping plane,
	// otherwise add our polygons to our assigned clipping pool 
	if (Front == NULL) {
		// We're a leaf node so put the polygons into the mesh fragment arrays
		if (front_poly.Get_Vertex_Count() >= 3) {
			ClipPools[FrontLeafIndex].Add(front_poly);	
		} 
	} else {
		// Pass the polygons to our child for further clipping
		if (front_poly.Get_Vertex_Count() >= 3) {
			Front->Clip_Polygon(front_poly);
		}
	}

	// Process the back halfspace:
	if (Back==NULL) {
		if (back_poly.Get_Vertex_Count() >= 3) {
			ClipPools[BackLeafIndex].Add(back_poly);	
		}
	} else {
		if (back_poly.Get_Vertex_Count() >= 3) {
			Back->Clip_Polygon(back_poly);
		}
	}
}




/***********************************************************************************************
**
** ShatterSystem Implementation - this is the interface with the outside world
** 
** Transform meshes into space needed for shattering:
** Vshatter = Mscale-to-unit * Mworld-shatterview * Mobj-world * Vobj
**
***********************************************************************************************/
void ShatterSystem::Init(void)
{
	/*
	** Resize the Mesh Fragment pointer array to handle the maximum number
	** of mesh fragments.
	*/
	MeshFragments.Resize(MAX_MESH_FRAGMENTS);

	/*
	** Search for ShatterPattern hierarchy tree objects, beginning with ShatterPattern00
	** Create a BSP structure for each one.
	*/
	StringClass htree_name;
	htree_name.Format(SHATTER_PATTERN_FORMAT,0);

 	if (WW3DAssetManager::Get_Instance()==NULL)
		return;  // WorldBuilderTool doesn't initialize the asset manager.  jba.
#if 1
	HTreeClass *htree = NULL;
#else
	HTreeClass * htree = WW3DAssetManager::Get_Instance()->Get_HTree(htree_name);
#endif
	while (htree != NULL) {
		if ((htree->Num_Pivots() > 1) && (htree->Num_Pivots() < MAX_MESH_FRAGMENTS)) {
			int leaf_counter = 0;
			htree->Base_Update(Matrix3D(1));
			ShatterPatterns.Add(W3DNEW BSPClass(htree,1,leaf_counter));			
		}
		
		/*
		** Try to load the next tree
		*/
		htree_name.Format(SHATTER_PATTERN_FORMAT,ShatterPatterns.Count());
		htree = WW3DAssetManager::Get_Instance()->Get_HTree(htree_name);
	}
}

void ShatterSystem::Shutdown(void)
{
	/*
	** Release all mesh fragments
	*/
	Release_Fragments();

	/*
	** Release any loaded BSP trees
	*/
	for (int i=0; i<ShatterPatterns.Count(); i++) {
		delete ShatterPatterns[i];
		ShatterPatterns[i] = NULL;
	}
	ShatterPatterns.Delete_All();
}


void ShatterSystem::Shatter_Mesh(MeshClass * mesh,const Vector3 & point,const Vector3 & direction)
{
	if (ShatterPatterns.Count() == 0) {
		return ;
	}

	int ivert,ipoly;
	int ipass,istage;

	/*
	** Reset the temporary clip arrays
	** Release any old mesh fragment render objects
	*/
	Reset_Clip_Pools();
	Release_Fragments();

	/*
	** Verify that this mesh meets the criteria for being shattered
	** - it has a maximum of two passes
	** - it uses only one vertex material per pass
	** - it uses only one shader per pass
	** - it uses only one texture per pass
	*/
	MeshModelClass * model = mesh->Get_Model();
	if (model->Get_Pass_Count() > MeshMatDescClass::MAX_PASSES) {
		WWDEBUG_SAY(("Failed to shatter model: %s.  Too many passes (%d)\n",model->Get_Name(),model->Get_Pass_Count()));
		REF_PTR_RELEASE(model);
		return;
	}
	for (ipass=0; ipass<model->Get_Pass_Count(); ipass++) {
		if (model->Has_Material_Array(ipass) || model->Has_Shader_Array(ipass)) {
			WWDEBUG_SAY(("Failed to shatter model: %s.  It has shader or material arrays\n",model->Get_Name()));
			REF_PTR_RELEASE(model);
			return;
		}
		
		for (istage=0; istage<MeshMatDescClass::MAX_TEX_STAGES; istage++) {
			if (model->Has_Texture_Array(ipass,istage)) {
				WWDEBUG_SAY(("Failed to shatter model: %s.  Texture array in pass: %d stage: %d\n",model->Get_Name(),ipass,istage));
				REF_PTR_RELEASE(model);
				return;
			}
		}
	}

	/*
	** Grab a random shatter pattern
	*/
	BSPClass * clipper = ShatterPatterns[rand() % ShatterPatterns.Count()];

	/*
	** Compute transforms which take vertices from mesh-space to shatter-space
	** and back again.  Transform polygons into "shatter-space", clip, then 
	** transform the results back out
	**
	** Take vertices from obj-space to shatter space:
	** Vshatter = Mscale-to-unit * Mworld-shatterview * Mobj-world * Vobj
	** 
	** Clip the polygons to the BSP
	** Vclipped = BSP_CLIP(Vshatter)
	**
	** Next, take the verts back to object space:
	** Vobjclip = Inverse(Mscale-to-unit * Mworld-shatter * Mobj-world) * Vclipped
	**          = Inv(Mobj-world)*Inv(Mworld-shatter)*Inv(Mscl-to-unit) * Vclipped
	** 
	** Next, create separate, re-centered meshes
	** Vnewobj = Mold-obj-to-new-obj * Vobjclip 
	*/

	/*
	** Object-space to world-space transform
	*/
	Matrix3D Mobj_to_world = mesh->Get_Transform();

	/*
	** World-space to shatter-space transform
	*/
	Matrix3D Mshatter_to_world;
	Matrix3D Mworld_to_shatter;
	Mshatter_to_world.Look_At(point,point+direction,0.0f);		// TODO: could put random roll
	Mshatter_to_world.Get_Orthogonal_Inverse(Mworld_to_shatter);

	/*
	** Final pair of matrices
	*/
	Matrix3D Mobj_to_shatter;
	Matrix3D Mshatter_to_obj;

	Matrix3D::Multiply(Mworld_to_shatter,Mobj_to_world,&Mobj_to_shatter);
	Mobj_to_shatter.Get_Orthogonal_Inverse(Mshatter_to_obj);

	/*
	** Scaling matrices.  This could be simpler if Matrix3D had a full inverse function.
	*/
	SphereClass sphere;
	model->Get_Bounding_Sphere(&sphere);
	
	float scale_factor = 5.0f / sphere.Radius;	// mesh scales to 5x shatter pattern.
	Matrix3D Mscale_to_shatter(1);
	Matrix3D Mscale_from_shatter(1);
	Mscale_to_shatter.Scale(scale_factor);
	Mscale_from_shatter.Scale(1.0f / scale_factor);
	
	Matrix3D::Multiply(Mscale_to_shatter,Mobj_to_shatter,&Mobj_to_shatter);
	Matrix3D::Multiply(Mshatter_to_obj,Mscale_from_shatter,&Mshatter_to_obj);

	/*
	** Grab the arrays out of the mesh and transform verts and vnorms
	** into "shatter-space"
	*/
	const TriIndex * polys = model->Get_Polygon_Array();
	const Vector3 * src_verts = model->Get_Vertex_Array();
	const Vector3 * src_vnorms = model->Get_Vertex_Normal_Array();

	Vector3 * verts = _get_temp_vertex_position_array(model->Get_Vertex_Count());
	VectorProcessorClass::Transform(verts,src_verts,Mobj_to_shatter,model->Get_Vertex_Count());

	/*
	** Build a description of the material parameters for the mesh
	*/
	MeshMtlParamsClass mtl_params(model);

	
	SHATTER_DEBUG_SAY(("****************************************************\n"));
	SHATTER_DEBUG_SAY((" Clipping model: %s\n",model->Get_Name()));
	SHATTER_DEBUG_SAY(("****************************************************\n"));

	/*
	** Pass each polygon of the source model through the BSP clipper
	*/
	for (ipoly=0; ipoly<model->Get_Polygon_Count(); ipoly++) {

		/*
		** Set up a PolygonClass for polygon 'i' in the mesh
		*/
		SHATTER_DEBUG_SAY(("Passing polygon %d to clipper.\n",ipoly));
		PolygonClass polygon;
		for (ivert=0; ivert<3; ivert++) { 
			
			int vert_index = polys[ipoly][ivert];
			polygon.Verts[ivert].PassCount = mtl_params.PassCount;
			polygon.Verts[ivert].Position = verts[vert_index];
			polygon.Verts[ivert].Normal = src_vnorms[vert_index];
			SHATTER_DEBUG_SAY(("position: %f %f %f\n",verts[vert_index].X,verts[vert_index].Y,verts[vert_index].Z));
			SHATTER_DEBUG_SAY(("normal: %f %f %f\n",src_vnorms[vert_index].X,src_vnorms[vert_index].Y,src_vnorms[vert_index].Z));
			
			for (ipass=0; ipass<MeshMatDescClass::MAX_PASSES; ipass++) {
				if (mtl_params.DCG[ipass] != NULL) {
					polygon.Verts[ivert].DCG[ipass] = mtl_params.DCG[ipass][vert_index];
					SHATTER_DEBUG_SAY(("DCG: pass: %d : %f %f %f\n",ipass,mtl_params.DCG[ipass][vert_index].X,mtl_params.DCG[ipass][vert_index].Y,mtl_params.DCG[ipass][vert_index].Z));
				}
				
				if (mtl_params.DIG[ipass] != NULL) {
					polygon.Verts[ivert].DIG[ipass] = mtl_params.DIG[ipass][vert_index];
					SHATTER_DEBUG_SAY(("DIG: pass: %d : %f %f %f\n",ipass,mtl_params.DIG[ipass][vert_index].X,mtl_params.DIG[ipass][vert_index].Y,mtl_params.DIG[ipass][vert_index].Z));
				}

				for (istage=0; istage<MeshMatDescClass::MAX_TEX_STAGES; istage++) {
					if (mtl_params.UV[ipass][istage] != NULL) {
						polygon.Verts[ivert].TexCoord[ipass][istage] = mtl_params.UV[ipass][istage][vert_index];
						SHATTER_DEBUG_SAY(("UV pass: %d stage: %d: %f %f\n",ipass,istage,mtl_params.UV[ipass][istage][vert_index].X,mtl_params.UV[ipass][istage][vert_index].Y));
					}
				}

/*				if (mtl_params.UVIndexArray[ipass] != NULL) {
					int uv_index = mtl_params.UVIndexArray[ipass][ipoly][ivert];
					polygon.Verts[ivert].TexCoord[ipass][0] = mtl_params.UV[ipass][0][uv_index];
					SHATTER_DEBUG_SAY(("Per-Face UV pass: %d: %f %f\n",ipass,polygon.Verts[ivert].TexCoord[ipass][0].X,polygon.Verts[ivert].TexCoord[ipass][0].Y));
				}
*/			}
		}
		polygon.Set_Vertex_Count(3);
		polygon.Compute_Plane();

		/*
		** Pass it through the BSP
		*/
		clipper->Clip_Polygon(polygon);
	}

	/*
	** convert the clipped polygons into meshes
	*/
	Process_Clip_Pools(Mshatter_to_obj,mesh,mtl_params);

	/*
	** release resources
	*/
	REF_PTR_RELEASE(model);
}

int ShatterSystem::Get_Fragment_Count(void)
{
	return MeshFragments.Count();
}

RenderObjClass * ShatterSystem::Get_Fragment(int fragment_index)
{
	if (MeshFragments[fragment_index] != NULL) {
		MeshFragments[fragment_index]->Add_Ref();
	}
	return MeshFragments[fragment_index];
}

RenderObjClass * ShatterSystem::Peek_Fragment(int fragment_index)
{
	return MeshFragments[fragment_index];
}

void ShatterSystem::Release_Fragments(void)
{
	// release any ref's to render objects
	for (int i=0; i<MeshFragments.Count(); i++) {
		REF_PTR_RELEASE(MeshFragments[i]);
	}
	
	// reset array but don't resize
	MeshFragments.Delete_All(false);					
}

void ShatterSystem::Reset_Clip_Pools(void)
{
	for (int i=0; i<MAX_MESH_FRAGMENTS; i++) {
		// reset array but don't resize
		ClipPools[i].Delete_All(false);				
	}
}

void ShatterSystem::Process_Clip_Pools
(
	const Matrix3D & Mshatter_to_mesh,
	MeshClass * mesh,
	MeshMtlParamsClass & mtl_params
)
{
	/*
	** Release any render objects we currently have and reset the array count
	*/
	Release_Fragments();
	
	/*
	** Grab the model
	*/
	MeshModelClass * model = mesh->Get_Model();
	WWASSERT(model != NULL);

	/*
	** Loop over all ClipPools and build a mesh for any that contain polygons
	*/
	for (int ipool=0; ipool<MAX_MESH_FRAGMENTS; ipool++) {

		if (ClipPools[ipool].Count() > 0) {

			int ivert,ipoly,ipass,istage;

			/*
			** Count the verts and polys
			*/
			int pcount = 0;
			int vcount = 0;
			for (ipoly=0;ipoly<ClipPools[ipool].Count();ipoly++) {
				int poly_vert_count = ClipPools[ipool][ipoly].Get_Vertex_Count();
				vcount += poly_vert_count;
				pcount += poly_vert_count-2;
			}

			SHATTER_DEBUG_SAY(("****************************************************\n"));
			SHATTER_DEBUG_SAY((" Reassembling fragment %d of model: %s\n",ipool,model->Get_Name()));
			SHATTER_DEBUG_SAY((" polycount = %d   vertexcount = %d\n",pcount,vcount));
			SHATTER_DEBUG_SAY(("****************************************************\n"));

			/*
			** Create the new mesh, install materials
			*/
			DynamicMeshClass * new_mesh = NEW_REF(DynamicMeshClass,(pcount,vcount));
			MaterialInfoClass * matinfo = NEW_REF(MaterialInfoClass,());

			if (model->Get_Flag(MeshModelClass::SORT)) {
				new_mesh->Enable_Sort();
			}
			new_mesh->Set_Pass_Count(mtl_params.PassCount);
			
			bool has_textures = false;
			for (ipass=0; ipass<model->Get_Pass_Count(); ipass++) {
				if (model->Peek_Single_Material(ipass) != NULL) {
					matinfo->Add_Vertex_Material(model->Peek_Single_Material(ipass));
				}
				for (int istage=0; istage<MeshMatDescClass::MAX_TEX_STAGES; istage++) {
					if (model->Peek_Single_Texture(ipass,istage) != NULL) {
						matinfo->Add_Texture(model->Peek_Single_Texture(ipass,istage));
						has_textures = true;
					}
				}
			}
			new_mesh->Set_Material_Info(matinfo);
			
			for (ipass=0; ipass<model->Get_Pass_Count(); ipass++) {
				new_mesh->Set_Vertex_Material(model->Peek_Single_Material(ipass),false,ipass);
				new_mesh->Set_Shader(model->Get_Single_Shader(ipass),ipass);

				for (istage=0; istage<MeshMatDescClass::MAX_TEX_STAGES; istage++) {
					TextureClass * tex = model->Peek_Single_Texture(ipass,istage);	
					if (tex != NULL) {
						new_mesh->Peek_Model()->Set_Single_Texture(tex,ipass,istage);
					}
				}
			}
			
			REF_PTR_RELEASE(matinfo);


			/*
			** Add the polygons and vertices to the mesh, transform the vertices
			** back into the original mesh's coordinate system as we do this
			*/
			for (ipoly=0; ipoly<ClipPools[ipool].Count(); ipoly++) {
				
				PolygonClass & poly = ClipPools[ipool][ipoly];
				
				new_mesh->Begin_Tri_Fan();
				SHATTER_DEBUG_SAY(("Begin Tri Fan\n"));

				for(ivert=0; ivert<poly.Get_Vertex_Count(); ivert++) {

					Vector3 pos,norm;
					VertexClass & vert = poly[ivert];
				
					Matrix3D::Transform_Vector(Mshatter_to_mesh,vert.Position,&pos);
					norm = vert.Normal;

					SHATTER_DEBUG_SAY(("Begin Vertex:\n"));
					new_mesh->Begin_Vertex();
					SHATTER_DEBUG_SAY(("postion: %f %f %f\n",pos.X,pos.Y,pos.Z));
					new_mesh->Location_Inline(pos);
					new_mesh->Normal(norm);

					for (ipass=0; ipass<mtl_params.PassCount; ipass++) {

						unsigned mycolor=0;
						
						/*
						** If there were vertex colors for this pass in the original mesh, then
						** copy the color out of the vertex into the new mesh.
						*/
						if (mtl_params.DCG[ipass] != NULL) {
							SHATTER_DEBUG_SAY(("DCG: pass:%d: %f %f %f\n",ipass,vert.DCG[ipass].X,vert.DCG[ipass].Y,vert.DCG[ipass].Z));
							/* OLD CODE
							new_mesh->DCG(Vector3(vert.DCG[ipass].X,vert.DCG[ipass].Y,vert.DCG[ipass].Z),ipass);
							new_mesh->Alpha(vert.DCG[ipass].W,ipass);
							*/
							mycolor=vert.DCG[ipass];							
						}
						
						// HY- Multiplying DIG with DCG as in meshmdlio
						if (mtl_params.DIG[ipass] != NULL) {
							SHATTER_DEBUG_SAY(("DIG: pass:%d: %f %f %f\n",ipass,vert.DIG[ipass].X,vert.DIG[ipass].Y,vert.DIG[ipass].Z));
							Vector4 mc=DX8Wrapper::Convert_Color(mycolor);
							Vector4 dc=DX8Wrapper::Convert_Color(vert.DIG[ipass]);
							mc=Vector4(mc.X*dc.X,mc.Y*dc.Y,mc.Z*dc.Z,mc.W);
							mycolor=DX8Wrapper::Convert_Color(mc);
						}

						new_mesh->Color(mycolor);

						/*
						** If there were UV coordinates in the original mesh for either stage,
						** then copy the vertex's uv's into into the new mesh.
						*/
//						#pragma MESSAGE("HY- Naty, will dynamesh support multiple stages of UV?")
						for (istage=0; istage<MeshMatDescClass::MAX_TEX_STAGES; istage++) {
							if (mtl_params.UV[ipass][istage] != NULL) {
								SHATTER_DEBUG_SAY(("UV: pass:%d stage: %d: %f %f\n",ipass,istage,vert.TexCoord[ipass][istage].X,vert.TexCoord[ipass][istage].Y));
								new_mesh->UV(vert.TexCoord[ipass][istage],istage);
							}
						}
					}

					new_mesh->End_Vertex();
			
				}
				new_mesh->End_Tri_Fan();
			}

			/*
			** Offset all vertices so that the bounding box center is 0,0,0 and
			** record that offset into the transform of the mesh
			*/
			new_mesh->Set_Dirty_Bounds();
			Vector3 center = new_mesh->Get_Bounding_Box().Center;

			new_mesh->Translate_Vertices(-center);
			
			Matrix3D tm(1);// = mesh->Get_Transform();
			tm.Translate(center);
			Matrix3D::Multiply(tm,mesh->Get_Transform(),&tm);
			new_mesh->Set_Transform(tm);
			
			/*
			** We gave it good vertex normals so clear their dirty flag
			*/
			new_mesh->Set_Dirty_Bounds();
			new_mesh->Set_Dirty_Planes();
			new_mesh->Clear_Dirty_Vertex_Normals();

			/*
			** Install it in the mesh fragment pool, transferring our reference
			** to the fragment array...
			*/
			MeshFragments.Add(new_mesh);
		}
	}
	REF_PTR_RELEASE(model);
}
