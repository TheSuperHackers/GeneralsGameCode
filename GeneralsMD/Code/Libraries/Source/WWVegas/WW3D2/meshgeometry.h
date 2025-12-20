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
 *                     $Archive:: /Commando/Code/ww3d2/meshgeometry.h                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/24/01 7:28p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "bittype.h"
#include "simplevec.h"
#include "sharebuf.h"
#include "w3derr.h"
#include "vector3.h"
#include "Vector3i.h"
#include "vector4.h"
#include "wwdebug.h"
#include "multilist.h"
#include "coltest.h"
#include "inttest.h"


class AABoxClass;
class OBBoxClass;
class SphereClass;
class ChunkLoadClass;
class AABTreeClass;
class HTreeClass;
class RenderInfoClass;

// Define which kind of index vector to use (16- or 32 bit)
typedef Vector3i16 TriIndex;
//typedef Vector3i TriIndex;

/*
** The following two defines control two space-saving optimizations.  In Renegade I've found
** that the plane equations are about 8% of the geometry space and vertex normals are about 10%
** so I'm trying to see if we can get by without them.  The plane equations are mainly used
** by collision detection functions and those are culled pretty well.  Anyway, collision is
** already so expensive that adding a cross product to it doesn't seem to matter.
**
** NOTE: currently with optimizations enabled, memory gets trashed if you use OPTIMIZE_VNORM_RAM
** I suspect this is due to the way Dynamesh uses (abuses) MeshGeometryClass and haven't had
** time to track it down yet.
*/
#define OPTIMIZE_PLANEEQ_RAM			1
#define OPTIMIZE_VNORM_RAM				0

/**
** MeshGeometryClass
** This class encapsulates the geometry data for a triangle mesh.
*/

class MeshGeometryClass : public W3DMPO, public RefCountClass, public MultiListObjectClass
{
	//W3DMPO_GLUE(MeshGeometryClass)

public:

	MeshGeometryClass(void);
	MeshGeometryClass(const MeshGeometryClass & that);
	virtual ~MeshGeometryClass(void);

	MeshGeometryClass & operator = (const MeshGeometryClass & that);

	enum FlagsType
	{
		DIRTY_BOUNDS							= 0x00000001,
		DIRTY_PLANES							= 0x00000002,
		DIRTY_VNORMALS							= 0x00000004,

		SORT										= 0x00000010,
		DISABLE_BOUNDING_BOX					= 0x00000020,
		DISABLE_BOUNDING_SPHERE				= 0x00000040,
		DISABLE_PLANE_EQ						= 0x00000080,
		TWO_SIDED								= 0x00000100,

		ALIGNED									= 0x00000200,
		SKIN										= 0x00000400,
		ORIENTED									= 0x00000800,
		CAST_SHADOW								= 0x00001000,

		PRELIT_MASK								= 0x0000E000,
		PRELIT_VERTEX							= 0x00002000,
		PRELIT_LIGHTMAP_MULTI_PASS			= 0x00004000,
		PRELIT_LIGHTMAP_MULTI_TEXTURE		= 0x00008000,

		ALLOW_NPATCHES							= 0x00010000,
	};

	void							Reset_Geometry(int polycount,int vertcount);

	const char *				Get_Name(void) const;
	void							Set_Name(const char * newname);

	const char *				Get_User_Text(void);
	void							Set_User_Text(char * usertext);

	void							Set_Flag(FlagsType flag,bool onoff)						{ if (onoff) {	Flags |= flag;	} else {	Flags &= ~flag; } }
	int							Get_Flag(FlagsType flag)									{ return Flags & flag; }

	void							Set_Sort_Level(int level)									{ SortLevel = level; }
	int							Get_Sort_Level(void) const									{ return SortLevel; }

	int							Get_Polygon_Count(void) const								{ return PolyCount; }
	int							Get_Vertex_Count(void) const								{ return VertexCount; }

	const TriIndex*			Get_Polygon_Array(void)										{ return get_polys(); }
	Vector3 *					Get_Vertex_Array(void)										{ WWASSERT(Vertex); return Vertex->Get_Array(); }
	const Vector3 *			Get_Vertex_Normal_Array(void);
	const Vector4 *			Get_Plane_Array(bool create = true);
	void							Compute_Plane(int pidx,PlaneClass * set_plane) const;
	const uint32 *				Get_Vertex_Shade_Index_Array(bool create = true)	{ return get_shade_indices(create); }
	const uint16 *				Get_Vertex_Bone_Links(void)								{ return get_bone_links(); }
	uint8 *						Get_Poly_Surface_Type_Array(void)						{ WWASSERT(PolySurfaceType); return PolySurfaceType->Get_Array(); }
	uint8							Get_Poly_Surface_Type(int poly_index) const;

	void							Get_Bounding_Box(AABoxClass * set_box);
	void							Get_Bounding_Sphere(SphereClass * set_sphere);

	// exposed culling support
	bool							Has_Cull_Tree(void)											{ return CullTree != NULL; }

	void							Generate_Rigid_APT(const Vector3 & view_dir, SimpleDynVecClass<uint32> & apt);
	void							Generate_Rigid_APT(const OBBoxClass & local_box, SimpleDynVecClass<uint32> & apt);
	void							Generate_Rigid_APT(const OBBoxClass & local_box, const Vector3 & view_dir, SimpleDynVecClass<uint32> & apt);

	void							Generate_Skin_APT(const OBBoxClass & world_box, SimpleDynVecClass<uint32> & apt, const Vector3 *world_vertex_locs);

	// containment
	bool							Contains(const Vector3 &point);

	// ray casting and intersection (takes a transform for the mesh). Note that unlike the MeshClass
	// functions with similar names, these work in object space.
	bool							Cast_Ray(RayCollisionTestClass & raytest);
	bool							Cast_AABox(AABoxCollisionTestClass & boxtest);
	bool							Cast_OBBox(OBBoxCollisionTestClass & boxtest);
	bool							Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);

	// This function analyses the transform passed into it to call various optimized functions if
	// the transform is identity or a simple rotation about the z-axis. Otherwise it transforms
	// boxtest into object space, performs an oob cast and transforms the result back.
	bool							Cast_World_Space_AABox(AABoxCollisionTestClass & boxtest, const Matrix3D &transform);

	// W3D File Format support.  Note that derived classes have to override these functions completely
	// so that they can handle their extra chunks.  Using these functions you could load mesh data out
	// of a W3D file while ignoring all materials, textures, etc.
	virtual WW3DErrorType	Load_W3D(ChunkLoadClass & cload);

	void							Scale(const Vector3 &sc);

protected:

	// internal accessor functions that are not exposed to the user (non-const...)
	TriIndex *					get_polys(void);
	Vector3 *					get_vert_normals(void);
	uint32 *						get_shade_indices(bool create = true);
	Vector4 *					get_planes(bool create = true);
	uint16 *						get_bone_links(bool create = true);

	// Utility functions (used by collision/intersection functions)
	int							cast_semi_infinite_axis_aligned_ray(const Vector3 & start_point, int axis_dir, unsigned char & flags);

	bool							cast_aabox_identity(AABoxCollisionTestClass & boxtest,const Vector3 & trans);
	bool							cast_aabox_z90(AABoxCollisionTestClass & boxtest,const Vector3 & trans);
	bool							cast_aabox_z180(AABoxCollisionTestClass & boxtest,const Vector3 & trans);
	bool							cast_aabox_z270(AABoxCollisionTestClass & boxtest,const Vector3 & trans);

	bool							intersect_obbox_brute_force(OBBoxIntersectionTestClass & localtest);
	bool							cast_ray_brute_force(RayCollisionTestClass & raytest);
	bool							cast_aabox_brute_force(AABoxCollisionTestClass & boxtest);
	bool							cast_obbox_brute_force(OBBoxCollisionTestClass & boxtest);

	// functions to recompute dirty normals and bounding volumes.
	virtual void				Compute_Plane_Equations(Vector4 * array);
	virtual void				Compute_Vertex_Normals(Vector3 * array);
	virtual void				Compute_Bounds(Vector3 * verts);
	void							Generate_Culling_Tree(void);

	// W3D chunk reading
	WW3DErrorType				read_chunks(ChunkLoadClass & cload);
	WW3DErrorType				read_vertices(ChunkLoadClass & cload);
	WW3DErrorType				read_vertex_normals(ChunkLoadClass & cload);
	WW3DErrorType				read_triangles(ChunkLoadClass & cload);
	WW3DErrorType				read_user_text(ChunkLoadClass & cload);
	WW3DErrorType				read_vertex_influences(ChunkLoadClass & cload);
	WW3DErrorType				read_vertex_shade_indices(ChunkLoadClass & cload);
	WW3DErrorType				read_aabtree(ChunkLoadClass &cload);

	// functions to compute the deformed vertices of skins.
	// Destination pointers MUST point to arrays large enough to hold all vertices
	void get_deformed_vertices(Vector3 *dst_vert, Vector3 *dst_norm, const HTreeClass * htree);
	void get_deformed_vertices(Vector3 *dst_vert, const HTreeClass * htree);
	void get_deformed_screenspace_vertices(Vector4 *dst_vert,const RenderInfoClass & rinfo,const Matrix3D & mesh_tm,const HTreeClass * htree);

	// General info
	ShareBufferClass<char> *							MeshName;
	ShareBufferClass<char> *							UserText;
	int														Flags;
	char														SortLevel;
	uint32													W3dAttributes;

	// Geometry
	int														PolyCount;
	int														VertexCount;

	ShareBufferClass<TriIndex> *						Poly;
	ShareBufferClass<Vector3> *						Vertex;
	ShareBufferClass<Vector3> *						VertexNorm;
	ShareBufferClass<Vector4> *						PlaneEq;
	ShareBufferClass<uint32> *							VertexShadeIdx;
	ShareBufferClass<uint16> *							VertexBoneLink;
	ShareBufferClass<uint8> *							PolySurfaceType;

	Vector3													BoundBoxMin;
	Vector3													BoundBoxMax;
	Vector3													BoundSphereCenter;
	float														BoundSphereRadius;
	AABTreeClass *											CullTree;

};

/*
** Inline functions for MeshGeometryClass
*/
inline TriIndex * MeshGeometryClass::get_polys(void)
{
	WWASSERT(Poly);
	return Poly->Get_Array();
}


inline uint32 * MeshGeometryClass::get_shade_indices(bool create)
{
	if (create && !VertexShadeIdx) {
		VertexShadeIdx = NEW_REF(ShareBufferClass<uint32>,(VertexCount, "MeshGeometryClass::VertexShadeIdx"));
	}
	if (VertexShadeIdx) {
		return VertexShadeIdx->Get_Array();
	}
	return NULL;
}

inline uint16 * MeshGeometryClass::get_bone_links(bool create)
{
	if (create && !VertexBoneLink) {
		VertexBoneLink = NEW_REF(ShareBufferClass<uint16>,(VertexCount, "MeshGeometryClass::VertexBoneLink"));
	}
	if (VertexBoneLink) {
		return VertexBoneLink->Get_Array();
	}
	return NULL;
}

inline uint8 MeshGeometryClass::Get_Poly_Surface_Type(int poly_index) const
{
	WWASSERT(PolySurfaceType);
	WWASSERT(poly_index >= 0 && poly_index < PolyCount);
	uint8 *type = PolySurfaceType->Get_Array();
	return type[poly_index];
}
