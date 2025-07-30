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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/ww3d2/meshgeometry.cpp                            $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MeshGeometryClass::MeshGeometryClass -- Constructor                                       *
 *   MeshGeometryClass::MeshGeometryClass -- Copy Constructor                                  *
 *    -- assignment operator                                                                   *
 *   MeshGeometryClass::~MeshGeometryClass -- destructor                                       *
 *   MeshGeometryClass::Reset_Geometry -- releases current resources and allocates space if ne *
 *   MeshGeometryClass::Get_Name -- returns the name                                           *
 *   MeshGeometryClass::Set_Name -- set the name of this model                                 *
 *   MeshGeometryClass::Get_User_Text -- get the user-text buffer                              *
 *   MeshGeometryClass::Set_User_Text -- set the user text buffer                              *
 *   MeshGeometryClass::Get_Bounding_Box -- get the bounding box                               *
 *   MeshGeometryClass::Get_Bounding_Sphere -- get the bounding sphere                         *
 *   MeshGeometryClass::Generate_Rigid_APT -- generate active polygon table                    *
 *   MeshGeometryClass::Generate_Skin_APT -- generate an active polygon table                  *
 *   MeshGeometryClass::Contains -- test if the mesh contains the given point                  *
 *   MeshGeometryClass::Cast_Ray -- compute a ray intersection with this mesh                  *
 *   MeshGeometryClass::Cast_AABox -- cast an AABox against this mesh                          *
 *   MeshGeometryClass::Cast_OBBox -- Cast an obbox against this mesh                          *
 *   MeshGeometryClass::Intersect_OBBox -- test for intersection with the given OBBox          *
 *   MeshGeometryClass::Cast_World_Space_AABox -- test for intersection with a worldspace AABox*
 *   MeshGeometryClass::cast_semi_infinite_axis_aligned_ray -- casts an axis aligned ray       *
 *   MeshGeometryClass::cast_aabox_identity -- aligned aabox test                              *
 *   MeshGeometryClass::cast_aabox_z90 -- aabox test which is rotated about z by 90            *
 *   MeshGeometryClass::cast_aabox_z180 -- aabox test which is rotated about z by 180          *
 *   MeshGeometryClass::cast_aabox_z270 -- aabox test which is rotated about z by 270          *
 *   MeshGeometryClass::cast_ray_brute_force -- brute force ray-cast                           *
 *   MeshGeometryClass::cast_aabox_brute_force -- brute force aabox cast                       *
 *   MeshGeometryClass::cast_obbox_brute_force -- brute force obbox cast                       *
 *   MeshGeometryClass::intersect_obbox_brute_force -- brute force intersection check          *
 *   MeshGeometryClass::Compute_Plane_Equations -- Recalculates the plane equations            *
 *   MeshGeometryClass::Compute_Vertex_Normals -- recompute the vertex normals                 *
 *   MeshGeometryClass::Compute_Bounds -- recomputes the bounding volumes                      *
 *   MeshGeometryClass::get_vert_normals -- get the vertex normal array                        *
 *   MeshGeometryClass::Get_Vertex_Normal_Array -- validates and returns the vertex normal arr *
 *   MeshGeometryClass::get_planes -- get the plane array memory (internal)                    *
 *   MeshGeometryClass::Get_Plane_Array -- validates and returns the array of plane equations  *
 *   MeshGeometryClass::Compute_Plane -- compute the plane equation of a single poly           *
 *   MeshGeometryClass::Generate_Culling_Tree -- Generate an AABTree for this mesh             *
 *   MeshGeometryClass::Load_W3D -- Load a mesh from a w3d                                     *
 *   MeshGeometryClass::read_chunks -- read w3d chunks                                         *
 *   MeshGeometryClass::read_vertices -- read the vertex chunk from a W3D file                 *
 *   MeshGeometryClass::read_vertex_normals -- read the vertex normals chunk from a w3d file   *
 *   MeshGeometryClass::read_triangles -- read the triangles chunk from a w3d file             *
 *   MeshGeometryClass::read_user_text -- read the user text chunk from a w3d file             *
 *   MeshGeometryClass::read_vertex_influences -- read the vertex influences chunk from a w3d  *
 *   MeshGeometryClass::read_vertex_shade_indices -- read the vertex shade indices chunk       *
 *   MeshGeometryClass::read_aabtree -- read the AABTree chunk from a w3d file                 *
 *   MeshGeometryClass::Generate_APT -- generate an apt for a box and view direction           *
 *   MeshGeometryClass::Generate_Rigid_APT -- generate an apt using backface culling           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "meshgeometry.h"
#include "aabtree.h"
#include "chunkio.h"
#include "aabox.h"
#include "obbox.h"
#include "sphere.h"
#include "plane.h"
#include "wwdebug.h"
#include "wwmemlog.h"
#include "w3d_file.h"
#include "vp.h"


#if (OPTIMIZE_PLANEEQ_RAM)
static SimpleVecClass<Vector4> _PlaneEQArray(1024);
#endif


#if (OPTIMIZE_VNORM_RAM)
static SimpleVecClass<Vector3> _VNormArray(1024);
#endif


/***********************************************************************************************
 * MeshGeometryClass::MeshGeometryClass -- Constructor                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
MeshGeometryClass::MeshGeometryClass(void) :
	MeshName(NULL),
	UserText(NULL),
	Flags(0),
	SortLevel(SORT_LEVEL_NONE),
	W3dAttributes(0),
	PolyCount(0),
	VertexCount(0),
	Poly(NULL),
	PolySurfaceType(NULL),
	Vertex(NULL),
	VertexNorm(NULL),
	PlaneEq(NULL),
	VertexShadeIdx(NULL),
	VertexBoneLink(NULL),
	BoundBoxMin(0,0,0),
	BoundBoxMax(1,1,1),
	BoundSphereCenter(0,0,0),
	BoundSphereRadius(1),
	CullTree(NULL)
{
}


/***********************************************************************************************
 * MeshGeometryClass::MeshGeometryClass -- Copy Constructor                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
MeshGeometryClass::MeshGeometryClass(const MeshGeometryClass & that) :
	MeshName(NULL),
	UserText(NULL),
	Flags(0),
	SortLevel(SORT_LEVEL_NONE),
	W3dAttributes(0),
	PolyCount(0),
	VertexCount(0),
	Poly(NULL),
	PolySurfaceType(NULL),
	Vertex(NULL),
	VertexNorm(NULL),
	PlaneEq(NULL),
	VertexShadeIdx(NULL),
	VertexBoneLink(NULL),
	BoundBoxMin(0,0,0),
	BoundBoxMax(1,1,1),
	BoundSphereCenter(0,0,0),
	BoundSphereRadius(1),
	CullTree(NULL)
{
	*this = that;
}


/***********************************************************************************************
 *  -- assignment operator                                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
MeshGeometryClass & MeshGeometryClass::operator = (const MeshGeometryClass & that)
{
	if (this != &that) {
		Flags = that.Flags;
		SortLevel = that.SortLevel;
		W3dAttributes = that.W3dAttributes;
		PolyCount = that.PolyCount;
		VertexCount = that.VertexCount;

		BoundBoxMin = that.BoundBoxMin;
		BoundBoxMax = that.BoundBoxMax;
		BoundSphereCenter = that.BoundSphereCenter;
		BoundSphereRadius = that.BoundSphereRadius;

		REF_PTR_SET(MeshName,that.MeshName);
		REF_PTR_SET(UserText,that.UserText);
		REF_PTR_SET(Poly,that.Poly);
		REF_PTR_SET(PolySurfaceType,that.PolySurfaceType);		
		REF_PTR_SET(Vertex,that.Vertex);
		REF_PTR_SET(VertexNorm,that.VertexNorm);
		REF_PTR_SET(PlaneEq,that.PlaneEq);
		REF_PTR_SET(VertexShadeIdx,that.VertexShadeIdx);
		REF_PTR_SET(VertexBoneLink,that.VertexBoneLink);
		REF_PTR_SET(CullTree,that.CullTree);
	}
	return * this;
}


/***********************************************************************************************
 * MeshGeometryClass::~MeshGeometryClass -- destructor                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
MeshGeometryClass::~MeshGeometryClass(void)
{
	Reset_Geometry(0,0);
}


/***********************************************************************************************
 * MeshGeometryClass::Reset_Geometry -- releases current resources and allocates space if need *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Reset_Geometry(int polycount,int vertcount)
{
	// Release everything we have and reset to initial state
	Flags = 0;
	PolyCount = 0;
	VertexCount = 0;
	SortLevel = SORT_LEVEL_NONE;

	REF_PTR_RELEASE(MeshName);
	REF_PTR_RELEASE(UserText);
	REF_PTR_RELEASE(Poly);
	REF_PTR_RELEASE(PolySurfaceType);	
	REF_PTR_RELEASE(Vertex);
	REF_PTR_RELEASE(VertexNorm);
	REF_PTR_RELEASE(PlaneEq);
	REF_PTR_RELEASE(VertexShadeIdx);
	REF_PTR_RELEASE(VertexBoneLink);
	REF_PTR_RELEASE(CullTree);

	PolyCount = polycount;
	VertexCount = vertcount;

	// allocate new geometry arrays
	if ((polycount != 0) && (vertcount != 0)) {
		Poly = NEW_REF(ShareBufferClass<TriIndex>,(PolyCount, "MeshGeometryClass::Poly"));
		PolySurfaceType = NEW_REF(ShareBufferClass<uint8>,(PolyCount, "MeshGeometryClass::PolySurfaceType"));
		Vertex = NEW_REF(ShareBufferClass<Vector3>,(VertexCount, "MeshGeometryClass::Vertex"));

		Poly->Clear();
		PolySurfaceType->Clear();
		Vertex->Clear();

#if (!OPTIMIZE_VNORM_RAM)
		VertexNorm = NEW_REF(ShareBufferClass<Vector3>,(VertexCount, "MeshGeometryClass::VertexNorm"));
		VertexNorm->Clear();
#endif
	}

	return ;
}


/***********************************************************************************************
 * MeshGeometryClass::Get_Name -- returns the name                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
const char * MeshGeometryClass::Get_Name(void) const
{
	if (MeshName) {
		return MeshName->Get_Array();
	} 
	return NULL;
}


/***********************************************************************************************
 * MeshGeometryClass::Set_Name -- set the name of this model                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Set_Name(const char * newname)
{
	if (MeshName) {
		MeshName->Release_Ref();
	}
	if (newname) {
		MeshName = NEW_REF(ShareBufferClass<char>,(strlen(newname)+1, "MeshGeometryClass::MeshName"));
		strcpy(MeshName->Get_Array(),newname);
	}
}


/***********************************************************************************************
 * MeshGeometryClass::Get_User_Text -- get the user-text buffer                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
const char * MeshGeometryClass::Get_User_Text(void)
{
	if (UserText) {
		return UserText->Get_Array();
	}
	return NULL;
}


/***********************************************************************************************
 * MeshGeometryClass::Set_User_Text -- set the user text buffer                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Set_User_Text(char * usertext)
{
	if (UserText) {
		UserText->Release_Ref();
	}
	if (usertext) {
		UserText = NEW_REF(ShareBufferClass<char>,(strlen(usertext)+1, "MeshGeometryClass::UserText"));
		strcpy(UserText->Get_Array(),usertext);
	}
}


/***********************************************************************************************
 * MeshGeometryClass::Get_Bounding_Box -- get the bounding box                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Get_Bounding_Box(AABoxClass * set_box)
{
	WWASSERT(set_box != NULL);
	set_box->Center = (BoundBoxMax + BoundBoxMin) * 0.5f;
	set_box->Extent = (BoundBoxMax - BoundBoxMin) * 0.5f;
}	


/***********************************************************************************************
 * MeshGeometryClass::Get_Bounding_Sphere -- get the bounding sphere                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Get_Bounding_Sphere(SphereClass * set_sphere)
{
	WWASSERT(set_sphere != NULL);
	set_sphere->Center = BoundSphereCenter;
	set_sphere->Radius = BoundSphereRadius;
}


/***********************************************************************************************
 * MeshGeometryClass::Generate_Rigid_APT -- generate an apt using backface culling             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/10/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Generate_Rigid_APT(const Vector3 & view_dir, SimpleDynVecClass<uint32> & apt)
{
	const Vector3 * loc = Get_Vertex_Array();
	const Vector4 * norms = Get_Plane_Array();
	const TriIndex * polys = Get_Polygon_Array();
	TriClass tri;

	for (int poly_counter = 0; poly_counter < PolyCount; poly_counter++) {

		tri.V[0] = &(loc[ polys[poly_counter][0] ]);
		tri.V[1] = &(loc[ polys[poly_counter][1] ]);
		tri.V[2] = &(loc[ polys[poly_counter][2] ]);
		tri.N = (Vector3*)&(norms[poly_counter]);
		
		if (Vector3::Dot_Product(*tri.N,view_dir) < 0.0f) {
			apt.Add(poly_counter);
		}
	}
}


/***********************************************************************************************
 * MeshGeometryClass::Generate_Rigid_APT -- generate active polygon table                      *
 *                                                                                             *
 *    This function will cull the mesh against the specified volume                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Generate_Rigid_APT(const OBBoxClass & local_box, SimpleDynVecClass<uint32> & apt)
{
	if (CullTree != NULL) {
		CullTree->Generate_APT(local_box, apt);
	} else {

		// Beware, this is gonna be expensive!
		const Vector3 * loc = Get_Vertex_Array();
		const Vector4 * norms = Get_Plane_Array();
		const TriIndex * polys = Get_Polygon_Array();
		TriClass tri;

		for (int poly_counter = 0; poly_counter < PolyCount; poly_counter++) {

			tri.V[0] = &(loc[ polys[poly_counter][0] ]);
			tri.V[1] = &(loc[ polys[poly_counter][1] ]);
			tri.V[2] = &(loc[ polys[poly_counter][2] ]);
			tri.N = (Vector3*)&(norms[poly_counter]);
				
			if (CollisionMath::Intersection_Test(local_box, tri)) {;
				apt.Add(poly_counter);
			}
		}
	}
}


/***********************************************************************************************
 * MeshGeometryClass::Generate_APT -- generate an apt for a box and view direction             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/10/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Generate_Rigid_APT(const OBBoxClass & local_box,const Vector3 & viewdir,SimpleDynVecClass<uint32> & apt)
{
	if (CullTree != NULL) {
		CullTree->Generate_APT(local_box, viewdir,apt);
	} else {

		// Beware, this is gonna be expensive!
		const Vector3 * loc = Get_Vertex_Array();
		const Vector4 * norms = Get_Plane_Array();
		const TriIndex * polys = Get_Polygon_Array();
		TriClass tri;

		for (int poly_counter = 0; poly_counter < PolyCount; poly_counter++) {

			tri.V[0] = &(loc[ polys[poly_counter][0] ]);
			tri.V[1] = &(loc[ polys[poly_counter][1] ]);
			tri.V[2] = &(loc[ polys[poly_counter][2] ]);
			tri.N = (Vector3*)&(norms[poly_counter]);
				
			if (Vector3::Dot_Product(*tri.N,viewdir) < 0.0f) {
				if (CollisionMath::Intersection_Test(local_box,tri)) {
					apt.Add(poly_counter);
				} 
			}
		}
	}
}

/***********************************************************************************************
 * MeshGeometryClass::Generate_Skin_APT -- generate an active polygon table                    *
 *                                                                                             *
 *    This function culls the mesh against the specified volume                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Generate_Skin_APT(const OBBoxClass & world_box, SimpleDynVecClass<uint32> & apt, const Vector3 *world_vertex_locs)
{
	WWASSERT(world_vertex_locs);

	// Beware, this is gonna be expensive!
	const TriIndex * polys = Get_Polygon_Array();
	TriClass tri;

	for (int poly_counter=0; poly_counter < PolyCount; poly_counter++) {

		tri.V[0] = &(world_vertex_locs[ polys[poly_counter][0] ]);
		tri.V[1] = &(world_vertex_locs[ polys[poly_counter][1] ]);
		tri.V[2] = &(world_vertex_locs[ polys[poly_counter][2] ]);

		// We would have to do a non-trivial amount of computation to get the triangle normal
		// (since this is a skin we have no valid plane equations) and we know that N is not used
		// in the Intersection_Test, so we set it to a dummy value.
		static const Vector3 dummy_vec(0.0f, 0.0f, 1.0f);
		tri.N = &dummy_vec;
			
		if (CollisionMath::Intersection_Test(world_box,tri)) {;
			apt.Add(poly_counter);
		}
	}
}


/***********************************************************************************************
 * MeshGeometryClass::Contains -- test if the mesh contains the given point                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool MeshGeometryClass::Contains(const Vector3 &point)
{
	// To determine whether the mesh contains a point, we will use the parity method (cast a
	// semi-infinite ray from the point and check the number of intersections with the mesh.
	// An even number of intersections means the point is outside the mesh, and an odd number
	// means the point is inside the mesh. Since we can cast the ray in any direction we will
	// use axis-aligned rays for speed.

	// The method fails if the ray goes through any triangle edge or corner. For robustness
	// we will cast rays in all six axis-aligned directions, and take a majority vote. We will
	// significantly reduce the weighting of rays which go through corners/edges.
	// Also, if any raycast reports that the point is embedded in a triangle, we will stop and
	// report that the point is contained in the mesh (using the normal algorithm in this case
	// could lead to errors).
	float yes = 0.0f;	// weighted sum of rays indicating the point is contained in the mesh
	float no = 0.0f;	// weighted sum of rays indicating the point is not contained in the mesh
	for (int axis_dir = 0; axis_dir < 6; axis_dir++) {
		unsigned char flags = TRI_RAYCAST_FLAG_NONE;
		int intersections = cast_semi_infinite_axis_aligned_ray(point, axis_dir, flags);
		if (flags & TRI_RAYCAST_FLAG_START_IN_TRI) return true;
		float weight = flags & TRI_RAYCAST_FLAG_HIT_EDGE ? 0.1f : 1.0f;
		if (intersections & 0x01) {
			yes += weight;
		} else {
			no += weight;
		}
	}

	return yes > no;
}


/***********************************************************************************************
 * MeshGeometryClass::Cast_Ray -- compute a ray intersection with this mesh                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::Cast_Ray(RayCollisionTestClass & raytest)
{
	bool hit = false;

	if (CullTree) {
		hit = CullTree->Cast_Ray(raytest);
	} else {
		hit = cast_ray_brute_force(raytest);
	}

	return hit;
}

 
/***********************************************************************************************
 * MeshGeometryClass::Cast_AABox -- cast an AABox against this mesh                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::Cast_AABox(AABoxCollisionTestClass & boxtest)
{
	bool hit = false;
	
	if (CullTree) {
		hit = CullTree->Cast_AABox(boxtest);
	} else {
		hit = cast_aabox_brute_force(boxtest);
	}

	return hit;
}

  
/***********************************************************************************************
 * MeshGeometryClass::Cast_OBBox -- Cast an obbox against this mesh                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::Cast_OBBox(OBBoxCollisionTestClass & boxtest)
{
	bool hit = false;

	if (CullTree) {
		hit = CullTree->Cast_OBBox(boxtest);
	} else {
		hit = cast_obbox_brute_force(boxtest);
	}

	return hit;
}

  
/***********************************************************************************************
 * MeshGeometryClass::Intersect_OBBox -- test for intersection with the given OBBox            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::Intersect_OBBox(OBBoxIntersectionTestClass & boxtest)
{
	bool hit = false;

	if (CullTree) {
		hit = CullTree->Intersect_OBBox(boxtest);
	} else {
		hit = intersect_obbox_brute_force(boxtest);
	}

	return hit;
}


/***********************************************************************************************
 *   MeshGeometryClass::Cast_World_Space_AABox -- test for intersection with a worldspace AABox*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::Cast_World_Space_AABox(AABoxCollisionTestClass & boxtest, const Matrix3D &transform)
{
	/*
	** Attempt to classify the transform:
	** NOTE: This code assumes that the matrix is orthogonal!  Much code in W3D assumes
	** orthogonal matrices, if that convention is broken this code is also broken.
	** Basically what I'm doing here is doing the minimum number of compares needed
	** to identify a transform which is a 90 degree rotation about the z axis.
	** TODO: cache some transform flags somewhere to reduce the number of times
	** that these compares are done
	*/
	bool hit = false;
	
	if ((transform[0][0] == 1.0f) && (transform[1][1] == 1.0f)) {

		hit = cast_aabox_identity(boxtest,-transform.Get_Translation());
	
	} else if ((transform[0][1] == -1.0f) && (transform[1][0] == 1.0f)) {
	
		// this mesh has been rotated 90 degrees about z
		hit = cast_aabox_z90(boxtest,-transform.Get_Translation());
	
	} else if ((transform[0][0] == -1.0f) && (transform[1][1] == -1.0f)) {
	
		// this mesh has been rotated 180
		hit = cast_aabox_z180(boxtest,-transform.Get_Translation());
	
	} else if ((transform[0][1] == 1.0f) && (transform[1][0] == -1.0f)) {
	
		// this mesh has been rotated 270 
		hit = cast_aabox_z270(boxtest,-transform.Get_Translation());
	
	} else {

		/*
		** Ok, we fell through to here, that means there is a more general
		** transform on this mesh.  In this case, I create a new test which
		** is an oriented box test in the coordinate system of the mesh and cast it.
		*/
		Matrix3D world_to_obj;
		transform.Get_Orthogonal_Inverse(world_to_obj);
		OBBoxCollisionTestClass obbox(boxtest, world_to_obj);

		if (CullTree) {
			hit = CullTree->Cast_OBBox(obbox);
		} else {
			hit = cast_obbox_brute_force(obbox);
		}

		/*
		** now, we must transform the results of the test back to the original
		** coordinate system.
		*/
		if (hit) {
			Matrix3D::Rotate_Vector(transform, obbox.Result->Normal, &(obbox.Result->Normal));
			if (boxtest.Result->ComputeContactPoint) {
				Matrix3D::Transform_Vector(transform, obbox.Result->ContactPoint, &(obbox.Result->ContactPoint));
			}
		}
	}

	return hit;
}


/***********************************************************************************************
 * MeshGeometryClass::cast_semi_infinite_axis_aligned_ray -- casts an axis aligned ray         *
 *                                                                                             *
 *    This function is used by the 'Contains' function                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
int MeshGeometryClass::cast_semi_infinite_axis_aligned_ray(const Vector3 & start_point, int axis_dir,
	unsigned char & flags)
{
	int count = 0;
	if (CullTree) {
		count = CullTree->Cast_Semi_Infinite_Axis_Aligned_Ray(start_point, axis_dir, flags);
	} else {
	
		const Vector3 * loc = Get_Vertex_Array();
		const Vector4 * plane = Get_Plane_Array();
		const TriIndex * polyverts = Get_Polygon_Array();

		// These tables translate between the axis_dir representation (which is an integer in which 0
		// indicates a ray along the positive x axis, 1 along the negative x axis, 2 the positive y
		// axis, 3 negative y axis, 4 positive z axis, 5 negative z axis) and a four-integer
		// representation (axis_r is the axis number - 0, 1 or 2 - of the axis along which the ray is
		// cast; axis_1 and axis_2 are the axis numbers of the other two axes; direction is 0 for
		// negative and 1 for positive direction of the ray).
		static const int axis_r[6] =		{ 0, 0, 1, 1, 2, 2 };
		static const int axis_1[6] =		{ 1, 1, 2, 2, 0, 0 };
		static const int axis_2[6] =		{ 2, 2, 0, 0, 1, 1 };
		static const int direction[6] =	{ 1, 0, 1, 0, 1, 0 };
		WWASSERT(axis_dir >= 0);
		WWASSERT(axis_dir < 6);

		// The functions called after this point will 'or' bits into this variable, so it needs to
		// be initialized here to TRI_RAYCAST_FLAG_NONE.
		flags = TRI_RAYCAST_FLAG_NONE;

		/*
		** Loop over each polygon
		*/
		int poly_count = Get_Polygon_Count();
		for (int poly_counter=0; poly_counter < poly_count; poly_counter++) {
		
			const Vector3 &v0 = loc[ polyverts[poly_counter][0] ];
			const Vector3 &v1 = loc[ polyverts[poly_counter][1] ];
			const Vector3 &v2 = loc[ polyverts[poly_counter][2] ];
			const Vector4 &tri_plane = plane[poly_counter];

			// Since (int)true is defined as 1, and (int)false as 0:
			count += (unsigned int)Cast_Semi_Infinite_Axis_Aligned_Ray_To_Triangle(v0,	v1, v2,
				tri_plane, start_point, axis_r[axis_dir], axis_1[axis_dir], axis_2[axis_dir],
				direction[axis_dir], flags);
		}
	}

	return count;
}


/***********************************************************************************************
 * MeshGeometryClass::cast_aabox_identity -- aligned aabox test                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::cast_aabox_identity(AABoxCollisionTestClass & boxtest, const Vector3 & translation)
{
	// transform the test into the mesh's coordinate system
	AABoxCollisionTestClass newbox(boxtest);
	newbox.Translate(translation);

	// cast the box against the mesh
	if (CullTree) {
		return CullTree->Cast_AABox(newbox);
	} else {
		return cast_aabox_brute_force(newbox);
	}
}


/***********************************************************************************************
 * MeshGeometryClass::cast_aabox_z90 -- aabox test which is rotated about z by 90              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::cast_aabox_z90(AABoxCollisionTestClass & boxtest, const Vector3 & translation)
{
	// transform the test into the mesh's coordinate system
	AABoxCollisionTestClass newbox(boxtest);
	newbox.Translate(translation);
	newbox.Rotate(AABoxCollisionTestClass::ROTATE_Z270);

	// cast the box against the mesh, using culling if possible
	bool hit;
	if (CullTree) {
		hit = CullTree->Cast_AABox(newbox);
	} else {
		hit = cast_aabox_brute_force(newbox);
	}

	// if we hit something, we need to rotate the normal back out of the mesh coordinate system
	if (hit) {
		// rotating the normal by 90 degrees about Z
		float tmp = boxtest.Result->Normal.X;
		boxtest.Result->Normal.X = -boxtest.Result->Normal.Y;
		boxtest.Result->Normal.Y = tmp;
	}

	return hit;
}


/***********************************************************************************************
 * MeshGeometryClass::cast_aabox_z180 -- aabox test which is rotated about z by 180            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::cast_aabox_z180(AABoxCollisionTestClass & boxtest, const Vector3 & translation)
{
	// transform the test into the meshes coordinate system
	AABoxCollisionTestClass newbox(boxtest);
	newbox.Translate(translation);
	newbox.Rotate(AABoxCollisionTestClass::ROTATE_Z180);

	// cast the box against the mesh, using culling if possible
	bool hit;
	if (CullTree) {
		hit = CullTree->Cast_AABox(newbox);
	} else {
		hit = cast_aabox_brute_force(newbox);
	}
	
	// if we hit something, we need to rotate the normal back out of the mesh coordinate system
	if (hit) {
		// rotating the normal by 180 degrees about Z
		boxtest.Result->Normal.X = -boxtest.Result->Normal.X;
		boxtest.Result->Normal.Y = -boxtest.Result->Normal.Y;
	}

	return hit;
}


/***********************************************************************************************
 * MeshGeometryClass::cast_aabox_z270 -- aabox test which is rotated about z by 270            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::cast_aabox_z270(AABoxCollisionTestClass & boxtest, const Vector3 & translation)
{
	// transform the test into the mesh's coordinate system
	AABoxCollisionTestClass newbox(boxtest);
	newbox.Translate(translation);
	newbox.Rotate(AABoxCollisionTestClass::ROTATE_Z90);

	// cast the box against the mesh, using culling if possible
	bool hit;
	if (CullTree) {
		hit = CullTree->Cast_AABox(newbox);
	} else {
		hit = cast_aabox_brute_force(newbox);
	}

	// if we hit something, we need to rotate the normal back out of the mesh coordinate system
	if (hit) {
		// rotating the normal by 270 degrees about Z
		float tmp = boxtest.Result->Normal.X;
		boxtest.Result->Normal.X = boxtest.Result->Normal.Y;
		boxtest.Result->Normal.Y = -tmp;
	}

	return hit;
}


/***********************************************************************************************
 * MeshGeometryClass::intersect_obbox_brute_force -- brute force intersection check            *
 *                                                                                             *
 *    This function gets used if the mesh does not have a CullTree                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::intersect_obbox_brute_force(OBBoxIntersectionTestClass & localtest)
{
	TriClass tri;
	const Vector3 * loc = Get_Vertex_Array();
	const TriIndex * polyverts = Get_Polygon_Array();
#ifndef COMPUTE_NORMALS
	const Vector4 * norms = Get_Plane_Array();
#endif

	/*
	** Loop over each polygon
	*/
	for (int srtri=0; srtri < Get_Polygon_Count(); srtri++) {
	
		tri.V[0] = &(loc[ polyverts[srtri][0] ]);
		tri.V[1] = &(loc[ polyverts[srtri][1] ]);
		tri.V[2] = &(loc[ polyverts[srtri][2] ]);

#ifdef COMPUTE_NORMALS					
		static Vector3 _normal;
		tri.N = &_normal;
		tri.Compute_Normal();
#else
		tri.N = (Vector3 *)&(norms[srtri]);
#endif
		
		if (CollisionMath::Intersection_Test(localtest.Box, tri)) {
			return true;
		}
	}
	return false;
}


/***********************************************************************************************
 * MeshGeometryClass::cast_ray_brute_force -- brute force ray-cast                             *
 *                                                                                             *
 * This function gets used if the mesh does not have a CullTree                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::cast_ray_brute_force(RayCollisionTestClass & raytest)
{
	int srtri;
	TriClass tri;
	const Vector3 * loc = Get_Vertex_Array();
	const TriIndex * polyverts = Get_Polygon_Array();
#ifndef COMPUTE_NORMALS
	const Vector4 * norms = Get_Plane_Array();
#endif

	/*
	** Loop over each polygon
	*/
	bool hit = false;
	for (srtri=0; srtri < Get_Polygon_Count(); srtri++) {
	
		// TODO: find a better way to do this?
		tri.V[0] = &(loc[ polyverts[srtri][0] ]);
		tri.V[1] = &(loc[ polyverts[srtri][1] ]);
		tri.V[2] = &(loc[ polyverts[srtri][2] ]);

#ifdef COMPUTE_NORMALS					
		static Vector3 _normal;
		tri.N = &_normal;
		tri.Compute_Normal();
#else
		tri.N = (Vector3 *)&(norms[srtri]);
#endif
		
		hit = hit | CollisionMath::Collide(raytest.Ray, tri, raytest.Result);
		if (hit) {
			raytest.Result->SurfaceType = Get_Poly_Surface_Type (srtri);
		}
	
		if (raytest.Result->StartBad) return true;

	}
	return hit;
}


/***********************************************************************************************
 * MeshGeometryClass::cast_aabox_brute_force -- brute force aabox cast                         *
 *                                                                                             *
 *    This function gets used only if the mesh doesn't have a CullTree                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::cast_aabox_brute_force(AABoxCollisionTestClass & boxtest)
{
	/*
	** Loop over each polygon
	*/
	TriClass tri;
	int polyhit = -1;

	const Vector3 * loc = Get_Vertex_Array();
	const TriIndex * polyverts = Get_Polygon_Array();
#ifndef COMPUTE_NORMALS
	const Vector4 * norms = Get_Plane_Array();
#endif

	for (int srtri = 0; srtri < Get_Polygon_Count(); srtri++) {

		tri.V[0] = &(loc[ polyverts[srtri][0] ]);
		tri.V[1] = &(loc[ polyverts[srtri][1] ]);
		tri.V[2] = &(loc[ polyverts[srtri][2] ]);

#ifdef COMPUTE_NORMALS					
		static Vector3 _normal;
		tri.N = &_normal;
		tri.Compute_Normal();
#else
		tri.N = (Vector3 *)&(norms[srtri]);
#endif

		if (CollisionMath::Collide(boxtest.Box, boxtest.Move, tri, boxtest.Result)) {
			polyhit = srtri;
		}

		if (boxtest.Result->StartBad) {
			return true;
		}
	}
	if (polyhit != -1) {
		boxtest.Result->SurfaceType = Get_Poly_Surface_Type(polyhit);
		return true;
	}
	return false;
}


/***********************************************************************************************
 * MeshGeometryClass::cast_obbox_brute_force -- brute force obbox cast                         *
 *                                                                                             *
 *    This function gets used only if the mesh doesn't have a CullTree                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/2001  NH : Created.                                                                   *
 *=============================================================================================*/
bool MeshGeometryClass::cast_obbox_brute_force(OBBoxCollisionTestClass & boxtest)
{
	/*
	** Loop over each polygon
	*/
	TriClass tri;
	int polyhit = -1;

	const Vector3 * loc = Get_Vertex_Array();
	const TriIndex * polyverts = Get_Polygon_Array();
#ifndef COMPUTE_NORMALS
	const Vector4 * norms = Get_Plane_Array();
#endif

	for (int srtri = 0; srtri < Get_Polygon_Count(); srtri++) {

		tri.V[0] = &(loc[ polyverts[srtri][0] ]);
		tri.V[1] = &(loc[ polyverts[srtri][1] ]);
		tri.V[2] = &(loc[ polyverts[srtri][2] ]);

#ifdef COMPUTE_NORMALS					
		static Vector3 _normal;
		tri.N = &_normal;
		tri.Compute_Normal();
#else
		tri.N = (Vector3 *)&(norms[srtri]);
#endif

		if (CollisionMath::Collide(boxtest.Box, boxtest.Move, tri, Vector3(0,0,0), boxtest.Result)) {
			polyhit = srtri;
		}

		if (boxtest.Result->StartBad) {
			return true;
		}
	}
	if (polyhit != -1) {
		boxtest.Result->SurfaceType = Get_Poly_Surface_Type(polyhit);
		return true;
	}
	return false;
}


/***********************************************************************************************
 * MeshGeometryClass::Compute_Plane_Equations -- Recalculates the plane equations              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    This function should not normally be needed during run-time.                             *
 *    The array pointer must point to enough memory to hold a plane equation per polygon       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Compute_Plane_Equations(Vector4 * peq)
{
	WWASSERT(peq!=NULL);

	TriIndex * poly	= Poly->Get_Array();
	Vector3 * vert		= Vertex->Get_Array();

	for(int pidx = 0; pidx < PolyCount; pidx++)
	{
		Vector3 a,b,normal;
		const Vector3 & p0= vert[poly[pidx][0]];

		Vector3::Subtract(vert[poly[pidx][1]],p0,&a);
		Vector3::Subtract(vert[poly[pidx][2]],p0,&b);
		Vector3::Cross_Product(a,b,&normal);
		normal.Normalize();

		peq[pidx].Set( normal.X, normal.Y, normal.Z, -(Vector3::Dot_Product(p0,normal)) );
	}
	Set_Flag(DIRTY_PLANES,false);
}


/***********************************************************************************************
 * MeshGeometryClass::Compute_Vertex_Normals -- recompute the vertex normals                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    This function should not normally be needed during run-time.                             *
 *		The array pointer must point to an array of Vector3's of size NumVerts                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Compute_Vertex_Normals(Vector3 * vnorm)
{
	WWASSERT(vnorm != NULL);
	if ((PolyCount == 0)|| (VertexCount == 0)) {
		return;
	}

	const Vector4 * peq = Get_Plane_Array();
	TriIndex * poly = Poly->Get_Array();
	const uint32 * shadeIx	= Get_Vertex_Shade_Index_Array(false);

	// Two cases, with or without vertex shade indices.  The vertex shade indices
	// implicitly contain the smoothing groups information from the original mesh.
	// In their abscesnce, the entire mesh is smoothed.
	if (!shadeIx) {

		VectorProcessorClass::Clear(vnorm, VertexCount);

		for(int pidx = 0; pidx < PolyCount; pidx++) {

			vnorm[poly[pidx].I].X += peq[pidx].X;
			vnorm[poly[pidx].I].Y += peq[pidx].Y;
			vnorm[poly[pidx].I].Z += peq[pidx].Z;
			
			vnorm[poly[pidx].J].X += peq[pidx].X;
			vnorm[poly[pidx].J].Y += peq[pidx].Y;
			vnorm[poly[pidx].J].Z += peq[pidx].Z;
			
			vnorm[poly[pidx].K].X += peq[pidx].X;
			vnorm[poly[pidx].K].Y += peq[pidx].Y;
			vnorm[poly[pidx].K].Z += peq[pidx].Z;
		}
	
	} else {
	
		VectorProcessorClass::Clear (vnorm, VertexCount);

		for (int pidx = 0; pidx < PolyCount; pidx++)	{

			vnorm[shadeIx[poly[pidx].I]].X += peq[pidx].X;
			vnorm[shadeIx[poly[pidx].I]].Y += peq[pidx].Y;
			vnorm[shadeIx[poly[pidx].I]].Z += peq[pidx].Z;

			vnorm[shadeIx[poly[pidx].J]].X += peq[pidx].X;
			vnorm[shadeIx[poly[pidx].J]].Y += peq[pidx].Y;
			vnorm[shadeIx[poly[pidx].J]].Z += peq[pidx].Z;

			vnorm[shadeIx[poly[pidx].K]].X += peq[pidx].X;
			vnorm[shadeIx[poly[pidx].K]].Y += peq[pidx].Y;
			vnorm[shadeIx[poly[pidx].K]].Z += peq[pidx].Z;
		}

		// normalize the "master" vertex normals and copy the smoothed ones
		// (note: we always encounter the "master" ones first)
		for (unsigned vidx = 0; vidx < (unsigned)VertexCount; vidx ++) {
			if (shadeIx[vidx] == vidx) {
				vnorm[vidx].Normalize();
			} else {
				vnorm[vidx] = vnorm[shadeIx[vidx]];
			}
		}
	}

	VectorProcessorClass::Normalize(vnorm, VertexCount);

	Set_Flag(DIRTY_VNORMALS,false);
}


/***********************************************************************************************
 * MeshGeometryClass::Compute_Bounds -- recomputes the bounding volumes                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    This function should not normally be needed during run-time.                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Compute_Bounds(Vector3 * verts)
{
	BoundBoxMin.Set(0,0,0);
	BoundBoxMax.Set(0,0,0);
	BoundSphereCenter.Set(0,0,0);
	BoundSphereRadius = 0.0;

	if (VertexCount == 0) {
		return;
	}

	// find bounding box minimum and maximum
	if (verts == NULL) {
		verts = Vertex->Get_Array();
	}
	VectorProcessorClass::MinMax(verts,BoundBoxMin,BoundBoxMax,VertexCount);

	// calculate the bounding sphere
	BoundSphereCenter = (BoundBoxMin + BoundBoxMax)/2.0f;
	BoundSphereRadius = (float)(BoundBoxMax-BoundSphereCenter).Length2();
	BoundSphereRadius = ((float)sqrt(BoundSphereRadius))*1.00001f;
	Set_Flag(DIRTY_BOUNDS,false);
}



/***********************************************************************************************
 * MeshGeometryClass::get_vert_normals -- get the vertex normal array                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/14/2001  gth : Created.                                                                 *
 *=============================================================================================*/
Vector3 * MeshGeometryClass::get_vert_normals(void)
{
#if (OPTIMIZE_VNORM_RAM)
	_VNormArray.Uninitialised_Grow(VertexCount);
	return &(_VNormArray[0]);
#else
	WWASSERT(VertexNorm);
	return VertexNorm->Get_Array();
#endif
}


/***********************************************************************************************
 * MeshGeometryClass::Get_Vertex_Normal_Array -- validates and returns the vertex normal array *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/14/2001  gth : Created.                                                                 *
 *=============================================================================================*/
const Vector3 * MeshGeometryClass::Get_Vertex_Normal_Array(void)
{ 
#if (OPTIMIZE_VNORM_RAM)
	Compute_Vertex_Normals(get_vert_normals());
	return get_vert_normals(); 
#else
	if (Get_Flag(DIRTY_VNORMALS)) {
		Compute_Vertex_Normals(get_vert_normals());
	}
	return get_vert_normals(); 
#endif
}



/***********************************************************************************************
 * MeshGeometryClass::get_planes -- get the plane array memory (internal)                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/14/2001  gth : Created.                                                                 *
 *=============================================================================================*/
Vector4 * MeshGeometryClass::get_planes(bool create)
{
#if (OPTIMIZE_PLANEEQ_RAM)
	_PlaneEQArray.Uninitialised_Grow(PolyCount);
	return &(_PlaneEQArray[0]);
#else
	if (create && !PlaneEq) {
		PlaneEq = NEW_REF(ShareBufferClass<Vector4>,(PolyCount, "MeshGeometryClass::PlaneEq"));
	}
	if (PlaneEq) {
		return PlaneEq->Get_Array();
	}
	return NULL;
#endif
}


/***********************************************************************************************
 * MeshGeometryClass::Get_Plane_Array -- validates and returns the array of plane equations    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/14/2001  gth : Created.                                                                 *
 *=============================================================================================*/
const Vector4 * MeshGeometryClass::Get_Plane_Array(bool create)
{ 
#if (OPTIMIZE_PLANEEQ_RAM)
	Vector4 * planes = get_planes(create);
	Compute_Plane_Equations(planes);
	return planes;
#else
	Vector4 * planes = get_planes(create);
	if (planes && Get_Flag(DIRTY_PLANES)) {
		Compute_Plane_Equations(planes);
	}
	return planes;
#endif
}

/***********************************************************************************************
 * MeshGeometryClass::Compute_Plane -- compute the plane equation of a single poly             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/14/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Compute_Plane(int pidx,PlaneClass * set_plane) const
{
	WWASSERT(pidx >= 0);
	WWASSERT(pidx < PolyCount);
	TriIndex & poly = Poly->Get_Array()[pidx];
	Vector3 * verts = Vertex->Get_Array();

	set_plane->Set(verts[poly.I],verts[poly.J],verts[poly.K]);
}


/***********************************************************************************************
 * MeshGeometryClass::Generate_Culling_Tree -- Generate an AABTree for this mesh               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    This function should not normally be needed during run-time.                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryClass::Generate_Culling_Tree(void)
{
	WWMEMLOG(MEM_CULLINGDATA);
	{
		AABTreeBuilderClass builder;
		builder.Build_AABTree(PolyCount,Poly->Get_Array(),VertexCount,Vertex->Get_Array());

		DEBUG_ASSERTCRASH(CullTree == NULL, ("MeshGeometryClass::Generate_Culling_Tree: Leaking CullTree"));
		CullTree = NEW_REF(AABTreeClass,(&builder));
		CullTree->Set_Mesh(this);
	}
}


/***********************************************************************************************
 * MeshGeometryClass::Load_W3D -- Load a mesh from a w3d file                                  *
 *                                                                                             *
 *    This function will extract just the geometry information from a W3D mesh.  It must       *
 *    be completely replaced by any derived classes that want to handle all of the chunks.     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshGeometryClass::Load_W3D(ChunkLoadClass & cload)
{
	/*
	** This function will initialize this MeshGeometryClass from the contents of a W3D file.
	** Note that derived classes need to completely replace this function; only re-using the individual
	** chunk handling functions.  
	*/

	/*
	**	Open the first chunk, it should be the mesh header
	*/
	cload.Open_Chunk();
	
	if (cload.Cur_Chunk_ID() != W3D_CHUNK_MESH_HEADER3) {
		WWDEBUG_SAY(("Old format mesh mesh, no longer supported."));
		goto Error;
	}
	
	W3dMeshHeader3Struct header;
	if (cload.Read(&header,sizeof(W3dMeshHeader3Struct)) != sizeof(W3dMeshHeader3Struct)) {
		goto Error;
	}
	cload.Close_Chunk();

	/*
	** Process the header
	*/
	char *	tmpname;
	int		namelen;
	
	Reset_Geometry(header.NumTris,header.NumVertices);
	
	namelen = strlen(header.ContainerName);
	namelen += strlen(header.MeshName);
	namelen += 2;
	W3dAttributes = header.Attributes;	
	SortLevel = header.SortLevel;
	tmpname = W3DNEWARRAY char[namelen];
	memset(tmpname,0,namelen);

	if (strlen(header.ContainerName) > 0) {
		strcpy(tmpname,header.ContainerName);
		strcat(tmpname,".");
	}
	strcat(tmpname,header.MeshName);

	Set_Name(tmpname);

	delete[] tmpname;
	tmpname = NULL;

	/*
	** Set Bounding Info
	*/
	BoundBoxMin.Set(header.Min.X,header.Min.Y,header.Min.Z);
	BoundBoxMax.Set(header.Max.X,header.Max.Y,header.Max.Z);

	BoundSphereCenter.Set(header.SphCenter.X,header.SphCenter.Y,header.SphCenter.Z);
	BoundSphereRadius = header.SphRadius;

	/*
	** Flags
	*/
	if (header.Version >= W3D_MAKE_VERSION(4,1)) {
		int geometry_type = header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;
		switch (geometry_type) 
		{
			case W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL:
				break;
			case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED:
				Set_Flag(ALIGNED,true);
				break;
			case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED:
				Set_Flag(ORIENTED,true);
				break;
			case W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN:
				Set_Flag(SKIN,true);
				break;
		} 
	}

	if (header.Attributes & W3D_MESH_FLAG_TWO_SIDED) {
		Set_Flag(TWO_SIDED,true);
	}

	if (header.Attributes & W3D_MESH_FLAG_CAST_SHADOW) {
		Set_Flag(CAST_SHADOW,true);
	}

	read_chunks(cload);

	/*
	** If this is a pre-3.0 mesh and it has vertex influences,
	** fixup the bone indices to account for the new root node
	*/
	if ((header.Version < W3D_MAKE_VERSION(3,0)) && (Get_Flag(SKIN))) {

		uint16 * links = get_bone_links();
		WWASSERT(links);
		
		for (int bi = 0; bi < Get_Vertex_Count(); bi++) {
			links[bi] += 1;
		}
	}

	/*
	** If this mesh is collideable and no AABTree was in the file, generate one now
	*/
	if (	(((W3dAttributes & W3D_MESH_FLAG_COLLISION_TYPE_MASK) >> W3D_MESH_FLAG_COLLISION_TYPE_SHIFT) != 0) &&
			(CullTree == NULL)) 
	{
		Generate_Culling_Tree();
	}

	return WW3D_ERROR_OK;

Error:

	return WW3D_ERROR_LOAD_FAILED;
}


/***********************************************************************************************
 * MeshGeometryClass::read_chunks -- read w3d chunks                                           *
 *                                                                                             *
 *    Again, derived classes must replace this function with one that handles all of their     *
 *    chunks in addition to calling to this class for the individual chunks that it handles.   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshGeometryClass::read_chunks(ChunkLoadClass & cload) 
{
	/*
	**	Read in the chunk header
	** If there are no more chunks within the mesh chunk,
	** we are done.
	*/
	while (cload.Open_Chunk()) {

		/*
		** Process the chunk
		*/
		WW3DErrorType error = WW3D_ERROR_OK;

		switch (cload.Cur_Chunk_ID()) {

			case W3D_CHUNK_VERTICES:
					error = read_vertices(cload);
					break;

			case W3D_CHUNK_SURRENDER_NORMALS:
			case W3D_CHUNK_VERTEX_NORMALS:
					error = read_vertex_normals(cload);
					break;

			case W3D_CHUNK_TRIANGLES:
					error = read_triangles(cload);
					break;

			case W3D_CHUNK_MESH_USER_TEXT:
					error = read_user_text(cload);
					break;
			
			case W3D_CHUNK_VERTEX_INFLUENCES:
					error = read_vertex_influences(cload);
					break;

			case W3D_CHUNK_VERTEX_SHADE_INDICES:
					error = read_vertex_shade_indices(cload);
					break;

			case W3D_CHUNK_AABTREE:
					read_aabtree(cload);
					break;

			default:
					break;

		}	
		
		cload.Close_Chunk();

		if (error != WW3D_ERROR_OK) {
			return error;
		}
	}

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshGeometryClass::read_vertices -- read the vertex chunk from a W3D file                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshGeometryClass::read_vertices(ChunkLoadClass & cload)
{
	W3dVectorStruct vert;
	Vector3 * loc = Vertex->Get_Array();
	assert(loc);

	for (int i=0; i<Get_Vertex_Count(); i++) {

		if (cload.Read(&vert,sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct)) {
			return WW3D_ERROR_LOAD_FAILED;
		}
		
		loc[i].X = vert.X;
		loc[i].Y = vert.Y;
		loc[i].Z = vert.Z;
	}

	return WW3D_ERROR_OK;	
}


/***********************************************************************************************
 * MeshGeometryClass::read_vertex_normals -- read the vertex normals chunk from a w3d file     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshGeometryClass::read_vertex_normals(ChunkLoadClass & cload)
{
	W3dVectorStruct norm;
	Vector3 * mdlnorms = get_vert_normals();
	WWASSERT(mdlnorms);

	for (int i=0; i<VertexCount; i++) {
		if (cload.Read(&norm,sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct)) {
			return WW3D_ERROR_LOAD_FAILED;
		}

		mdlnorms[i].Set(norm.X,norm.Y,norm.Z);
	}

	return WW3D_ERROR_OK;	
}


/***********************************************************************************************
 * MeshGeometryClass::read_triangles -- read the triangles chunk from a w3d file               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshGeometryClass::read_triangles(ChunkLoadClass & cload)
{
	W3dTriStruct tri;

	// cache pointers to various arrays in the surrender mesh
	TriIndex * vi = get_polys();
	Set_Flag(DIRTY_PLANES,false);
	Vector4 * peq = get_planes();
	uint8 * surface_types = Get_Poly_Surface_Type_Array();

	// read in each polygon one by one
	for (int i=0; i<Get_Polygon_Count(); i++) {

		if (cload.Read(&tri,sizeof(W3dTriStruct)) != sizeof(W3dTriStruct)) {
			return WW3D_ERROR_LOAD_FAILED;
		}

		// set the vertex indices
		vi[i].I = tri.Vindex[0];
		vi[i].J = tri.Vindex[1];
		vi[i].K = tri.Vindex[2];

		// set the normal
		peq[i].X = tri.Normal.X;
		peq[i].Y = tri.Normal.Y;
		peq[i].Z = tri.Normal.Z;
		peq[i].W = -tri.Dist;

		// set the surface type
		WWASSERT(tri.Attributes < 256);
		surface_types[i] = (uint8)(tri.Attributes);
	}

	return WW3D_ERROR_OK;	
}


/***********************************************************************************************
 * MeshGeometryClass::read_user_text -- read the user text chunk from a w3d file               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshGeometryClass::read_user_text(ChunkLoadClass & cload)
{
	unsigned int textlen = cload.Cur_Chunk_Length();

	/*
	** This shouldn't happen but if there are more than one
	** USER_TEXT chunks in the mesh file, store only the first
	** one.  I am assuming that if the UserText buffer is not
	** NULL, then a previous user text chunk has been read in...
	*/
	if (UserText != NULL) {
		return WW3D_ERROR_OK;
	}
	
	/*
	** Allocate the buffer and read in the text
	*/
	UserText = NEW_REF(ShareBufferClass<char>,(textlen, "MeshGeometryClass::UserText"));

	if (cload.Read(UserText->Get_Array(),textlen) != textlen) {
		return WW3D_ERROR_LOAD_FAILED;
	}

	return WW3D_ERROR_OK;	
}


/***********************************************************************************************
 * MeshGeometryClass::read_vertex_influences -- read the vertex influences chunk from a w3d fi *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshGeometryClass::read_vertex_influences(ChunkLoadClass & cload)
{
	W3dVertInfStruct vinf;
	uint16 * links = get_bone_links(true);
	WWASSERT(links);

	for (int i=0; i<Get_Vertex_Count(); i++) {

		if (cload.Read(&vinf,sizeof(W3dVertInfStruct)) != sizeof(W3dVertInfStruct)) {
			return WW3D_ERROR_LOAD_FAILED;
		}
		links[i] = vinf.BoneIdx;		
	}	
	Set_Flag(SKIN,true);

	return WW3D_ERROR_OK;	
}


/***********************************************************************************************
 * MeshGeometryClass::read_vertex_shade_indices -- read the vertex shade indices chunk         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshGeometryClass::read_vertex_shade_indices(ChunkLoadClass & cload)
{
	uint32 * shade_index = get_shade_indices(true);
	uint32 si;

	for (int i=0; i<Get_Vertex_Count(); i++) {
		if (cload.Read(&si,sizeof(uint32)) != sizeof(uint32)) {
			return WW3D_ERROR_LOAD_FAILED;
		}
		shade_index[i] = si;
	}
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshGeometryClass::read_aabtree -- read the AABTree chunk from a w3d file                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/9/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshGeometryClass::read_aabtree(ChunkLoadClass &cload)
{
	REF_PTR_RELEASE(CullTree);
	CullTree = NEW_REF(AABTreeClass,());
	CullTree->Load_W3D(cload);
	CullTree->Set_Mesh(this);
	return (WW3D_ERROR_OK);
}

void MeshGeometryClass::Scale(const Vector3 &sc)
{
	WWASSERT(Vertex);
	Vector3 * vert = Vertex->Get_Array();
	
	for (int i=0;i<VertexCount; i++) {
		vert[i].X *= sc.X;
		vert[i].Y *= sc.Y;
		vert[i].Z *= sc.Z;
	}
		
	BoundBoxMin.Scale(sc);
	BoundBoxMax.Scale(sc);
	BoundSphereCenter.Scale(sc);
	
	float max;
	max = (sc.X > sc.Y)	? sc.X	: sc.Y;
	max = (max > sc.Z)	? max		: sc.Z;
	BoundSphereRadius *= max;

	// If scaling uniformly normals are OK:
	if (sc.X != sc.Y || sc.Y != sc.Z) {
		Set_Flag(DIRTY_VNORMALS,true);
	}
	// pnormals are plane equations...
	Set_Flag(DIRTY_PLANES,true);
}