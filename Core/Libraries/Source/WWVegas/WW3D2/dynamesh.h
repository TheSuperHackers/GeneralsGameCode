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

/***************************************************************************
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     ***
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Commando	                              *
 *                                                                         *
 *                     $Archive:: /Commando/Code/ww3d2/dynamesh.h         $*
 *                                                                         *
 *                      $Author:: Greg_h                                  $*
 *                                                                         *
 *                     $Modtime:: 12/03/01 4:20p                          $*
 *                                                                         *
 *                    $Revision:: 15                                      $*
 *                                                                         *
 *-------------------------------------------------------------------------*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DYNAMESH_H
#define DYNAMESH_H

#include "meshgeometry.h"
#include "meshmatdesc.h"
#include "matinfo.h"
#include "rendobj.h"
#include "polyinfo.h"
#include "dx8wrapper.h"

class	ShaderClass;
class	IntersectionClass;
class	IntersectionResultClass;

/*
** DynamicMeshModel: used for low-level rendering of DynamicMeshClass.
** It is composed of the same two classes (one base, one embedded as
** MeshModelClass, so many of its features are similar (see meshmdl.h)
*/
class DynamicMeshModel : public MeshGeometryClass
{
	W3DMPO_GLUE(DynamicMeshModel)

public:

	DynamicMeshModel(unsigned int max_polys, unsigned int max_verts);
	DynamicMeshModel(unsigned int max_polys, unsigned int max_verts, MaterialInfoClass *mat_info);
	DynamicMeshModel(const DynamicMeshModel &src);
	~DynamicMeshModel(void);

	// Inherited from MeshGeometryClass
	virtual void	Compute_Plane_Equations(void);
	virtual void	Compute_Vertex_Normals(void);
	virtual void	Compute_Bounds(Vector3 * verts);

	// Reset mesh (with existing max polygon and max vertex counts)
	void	Reset(void);

	// Render mesh
	void	Render(RenderInfoClass & rinfo);

	// Set current polygon and vertex counts
	void	Set_Counts(int pnum, int vnum)	{ DynamicMeshPNum = pnum;	DynamicMeshVNum = vnum;	}

	// Access to material stuff:
	unsigned *	Get_Color_Array(int color_array_index) { return MatDesc->Get_Color_Array(color_array_index); }
	Vector2 *	Get_UV_Array(int uv_array_index) { return MatDesc->Get_UV_Array_By_Index(uv_array_index); }

	void			Set_Single_Material(VertexMaterialClass * vmat, int pass=0) { MatDesc->Set_Single_Material(vmat, pass); }
	void			Set_Single_Texture(TextureClass * tex, int pass=0, int stage=0) { MatDesc->Set_Single_Texture(tex, pass, stage); }
	void			Set_Single_Shader(ShaderClass shader, int pass=0) { MatDesc->Set_Single_Shader(shader, pass); }

	void			Set_Material(int vidx, VertexMaterialClass * vmat, int pass=0) { MatDesc->Set_Material(vidx, vmat, pass); }
	void			Set_Shader(int pidx, ShaderClass shader, int pass=0)  { MatDesc->Set_Shader(pidx, shader, pass); }
	void			Set_Texture(int pidx, TextureClass * tex, int pass=0, int stage=0)  { MatDesc->Set_Texture(pidx, tex, pass, stage); }
	void			Set_Pass_Count(int passes)	{ MatDesc->Set_Pass_Count(passes); }
	int			Get_Pass_Count(void) const	{ return MatDesc->Get_Pass_Count(); }

	// Create the array (if it doesn't exist), fill it with the supplied value.
	void			Initialize_Texture_Array(int pass, int stage, TextureClass *texture = NULL);
	void			Initialize_Material_Array(int pass, VertexMaterialClass *vmat = NULL);

	// Accessors to material info:
	MaterialInfoClass		*Peek_Material_Info(void)			{ return MatInfo; }
	MaterialInfoClass		*Get_Material_Info(void)			{ if (MatInfo) MatInfo->Add_Ref(); return MatInfo;}
	void Set_Material_Info(MaterialInfoClass *mat_info)
	{
		if (MatInfo)
			MatInfo->Release_Ref();
		WWASSERT(MatInfo != 0);
		MatInfo = mat_info;
		MatInfo->Add_Ref();
	}

	// New geometry accessors (non-const)
	TriIndex *	Get_Non_Const_Polygon_Array(void);
	Vector3 *	Get_Non_Const_Vertex_Normal_Array(void);

private:

	// These are current counts, as opposed to those in the mesh geometry and
	// material description which are actually maximum counts.
	int						DynamicMeshPNum;
	int						DynamicMeshVNum;

	// All non-geometry properties (uvs, colors, textures, shaders, etc)
	MeshMatDescClass *	MatDesc;

	// Lists of textures and vertex materials for ease of processing
	MaterialInfoClass *	MatInfo;

};

/*
** Dynamic Meshes
*/
class DynamicMeshClass : public RenderObjClass {

public:

	// constructor and destructor
	DynamicMeshClass( int max_poly, int max_vert);
	DynamicMeshClass( int max_poly, int max_vert, MaterialInfoClass *mat_info);
	DynamicMeshClass( const DynamicMeshClass & src);
	virtual ~DynamicMeshClass( void);

	// Inherited from RenderObjClass:
	virtual RenderObjClass * 		Clone(void) const;
	virtual int							Class_ID(void) const					{ return CLASSID_DYNAMESH; }
	virtual void						Render(RenderInfoClass & rinfo);

	virtual MaterialInfoClass		*Peek_Material_Info(void)			{ return Model->Peek_Material_Info(); }
	virtual MaterialInfoClass		*Get_Material_Info(void)			{ return Model->Get_Material_Info(); }
	virtual void Set_Material_Info(MaterialInfoClass *mat_info)		{ Model->Set_Material_Info(mat_info); }

	// all render objects should be able to tell you how many polygons were
	// used in the making of the render object.
	virtual int Get_Num_Polys(void) const { return PolyCount; }

	// return the number of vertices used by this renderobject
	virtual int Get_Num_Vertices(void) const { return VertCount; }

	// Get and set static sort level
   virtual int		Get_Sort_Level(void) const		{ return SortLevel; }
  	virtual void	Set_Sort_Level(int level)		{ SortLevel = level; if(level != SORT_LEVEL_NONE) Disable_Sort();}

	// object space bounding volumes
	virtual inline void Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
	virtual inline void Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

	// Set the vertex material for the current triangle
	int	Set_Vertex_Material( int idx, int pass = 0);
	int	Set_Vertex_Material( VertexMaterialClass *material, bool dont_search = false, int pass = 0);

	// Set the number of passes for the mesh
	void	Set_Pass_Count(int passes) { assert(PolyCount == 0); Model->Set_Pass_Count(passes); }
	int	Get_Pass_Count() { return Model->Get_Pass_Count(); }

	// Set the texture for the current triangle
	int	Set_Texture( int idx, int pass = 0);
	int	Set_Texture( TextureClass *texture, bool dont_search = false, int pass = 0);

	// Set the shader for the current triangle
	int	Set_Shader( const ShaderClass & shader, int pass = 0) { Model->Set_Single_Shader(shader, pass); return 0; }

	// set the shader, texture, and vertex material as found in the polygon info object
	void	Set_Polygon_Info(const PolygonInfoClass &polyInfo, bool dont_search_texture = false, bool dont_search_vertex_material = false, int pass = 0)
	{
		// there must be a shader..
		assert(polyInfo.Peek_Shader() != 0);
		Set_Shader(* (polyInfo.Peek_Shader()), pass);

		assert(polyInfo.Peek_Vertex_Material() != 0);
		Set_Vertex_Material(polyInfo.Peek_Vertex_Material(), dont_search_vertex_material, pass);

		if (polyInfo.Peek_Texture())
			Set_Texture(polyInfo.Peek_Texture(), dont_search_texture, pass);
	}

	// Set vertex Color
	inline int	Set_Vertex_Color(const Vector4 & color, int color_array_index = 0);
	inline int	Set_Vertex_Color(const Vector3 & color, int color_array_index = 0);

	// reset the mesh flags
	void Reset_Flags() {	Set_Dirty(); }

	// Flush the mesh
	void Reset_Native_Mesh() {	Model->Reset(); }

	// reset our poly and vertex counts
	void Reset_Mesh_Counters()
	{
		Model->Set_Counts(0, 0);
		Disable_Sort();
		PolyCount = 0;
		VertCount = 0;
	}

	// Reset all polys and verts. Call the other reset functions directly if you do not want all
	// characteristics to be reset.
	virtual void Reset( void )
	{
		// Note that the active poly count has changed since the last render call by setting the dirty flag
		Reset_Flags();
		Reset_Native_Mesh();
		Reset_Mesh_Counters();

		// reset all the texture & vertex material indices
		int pass = MAX_PASSES;
		while (pass--) {
			TextureIdx[pass] = -1;
			VertexMaterialIdx[pass] = -1;
			MultiVertexMaterial[pass] = false;
		}
	}

	// Deletes mesh and recreates it with new max_polys and verts.
	void Resize(int max_polys, int max_verts);

	// Triangle creation routines
	void	Begin_Tri_Strip( void )	{ 	TriVertexCount = 0; TriMode = TRI_MODE_STRIPS; }
	void	Begin_Tri_Fan( void )	{	TriVertexCount = 0; TriMode = TRI_MODE_FANS; FanVertex = VertCount; }

	// vertex creation routines
	void	Begin_Vertex( void) {}

	virtual void Location( float x, float y, float z);

	// version for speedier use in certain cases
	void Location_Inline( float x, float y, float z )
	{
		Vector3 * loc = Model->Get_Vertex_Array();
		assert(loc);

		loc[VertCount].X = x;
		loc[VertCount].Y = y;
		loc[VertCount].Z = z;
	}
	void Location_Inline(Vector3 &v)	{ Location_Inline(v.X,v.Y,v.Z); }

	// retrieve a reference to the vertex in the object
	// WARNING: does not validate index
	Vector3 & Get_Location(int index)	{
		return Model->Get_Vertex_Array()[index];
	}

	void Normal( float x, float y, float z)
	{
		Vector3 * norms = Model->Get_Non_Const_Vertex_Normal_Array();
		assert(norms);

		norms[VertCount].X = x;
		norms[VertCount].Y = y;
		norms[VertCount].Z = z;
	}
	void Normal(Vector3 &v) { Normal(v.X,v.Y,v.Z); }

	// retrieve a reference to the normal vector3 in the object
	// WARNING: does not validate index
	Vector3 & Get_Normal(int index) { return Model->Get_Non_Const_Vertex_Normal_Array()[index]; }

	void Color(float r, float g, float b, float a, int color_array_index = 0)
	{
//		Vector4 * color = Model->Get_Color_Array(color_array_index);
		unsigned * color = Model->Get_Color_Array(color_array_index);
		assert(color);

		color[VertCount]=DX8Wrapper::Convert_Color_Clamp(Vector4(r,g,b,a));
//		color[VertCount].X = r;
//		color[VertCount].Y = g;
//		color[VertCount].Z = b;
//		color[VertCount].W = a;
	}
	void Color(const Vector4 &v, int color_array_index = 0) { Color(v.X, v.Y, v.Z, v.W, color_array_index); }
	void Color(unsigned v, int color_array_index=0)
	{
		unsigned * color = Model->Get_Color_Array(color_array_index);
		assert(color);
		color[VertCount]=v;
	}

	// retrieve a reference to a color entry in the object
	// WARNING: does not validate index
//	Vector4 & Get_Color(int index, int color_array_index = 0) { return Model->Get_Color_Array(color_array_index)[index]; }
	unsigned Get_Color(int index, int color_array_index = 0) { return Model->Get_Color_Array(color_array_index)[index]; }

	void	UV(float u, float v, int uv_array_index = 0)
	{
		Vector2 * uv = Model->Get_UV_Array(uv_array_index);
		assert(uv);

		uv[VertCount].U = u;
		uv[VertCount].V = v;
	}
	void UV( Vector2 &v, int uv_array_index = 0) { UV(v.U, v.V, uv_array_index); }
	// retrieve a reference to a UV entry in the object
	// WARNING: does not validate index
	Vector2 & Get_UV(int index, int uv_array_index = 0 )
	{
		return Model->Get_UV_Array(uv_array_index)[index];
	}

	bool End_Vertex( void);

	// vertex creation shortcut, performs a begin, projected, rotated, and end
	bool Vertex(float x, float y, float z, float u, float v)
	{
		Begin_Vertex();
		Location(x, y, z);
		UV(u, v);
		return End_Vertex();
	}

	bool Vertex(Vector2 v)
	{
		Begin_Vertex();
		Location(v.X, v.Y, 0);
		return End_Vertex();
	}

	void End_Tri_Strip( void )
	{
		TriVertexCount = 0;
	}

	void End_Tri_Fan( void )
	{
		TriVertexCount = 0;
	}

	// For moving a vertex after the DynaMesh has already been created.
	virtual	void Move_Vertex(int index, float x, float y, float z);
	virtual	void Get_Vertex(int index, float &x, float &y, float &z);

	// For moving all vertices in the mesh by a fixed amount
	void Translate_Vertices(const Vector3 & offset);

	// For changing the color of a vertex after DynaMesh has been created.
	virtual void Change_Vertex_Color(int index, const Vector4 &color, int color_array_index)
	{
		// check if switching to multivertexcolor
		if (!MultiVertexColor[color_array_index]) {
			Switch_To_Multi_Vertex_Color(color_array_index);
		}
		CurVertexColor[color_array_index].X = color.X;
		CurVertexColor[color_array_index].Y = color.Y;
		CurVertexColor[color_array_index].Z = color.Z;
		CurVertexColor[color_array_index].W = color.W;
//		Vector4 * color_list = Model->Get_Color_Array(color_array_index);
		unsigned * color_list = Model->Get_Color_Array(color_array_index);
		color_list[index] = DX8Wrapper::Convert_Color_Clamp(color);
	}


	/*
	** The following are a bunch of inlined functions for setting & clearing the mesh model's various flags
	*/
	// dirty flags
	void Set_Dirty_Bounds(void)				{ Model->Set_Flag(MeshGeometryClass::DIRTY_BOUNDS, true); }
	void Clear_Dirty_Bounds(void)				{ Model->Set_Flag(MeshGeometryClass::DIRTY_BOUNDS, false); }
	void Set_Dirty_Planes(void)				{ Model->Set_Flag(MeshGeometryClass::DIRTY_PLANES, true); }
	void Clear_Dirty_Planes(void)				{ Model->Set_Flag(MeshGeometryClass::DIRTY_PLANES, false); }
	void Set_Dirty_Vertex_Normals(void) 	{ Model->Set_Flag(MeshGeometryClass::DIRTY_VNORMALS, true); }
	void Clear_Dirty_Vertex_Normals(void)	{ Model->Set_Flag(MeshGeometryClass::DIRTY_VNORMALS, false); }

	// control flags
	void Disable_Sort(void)						{ Model->Set_Flag(MeshGeometryClass::SORT, false); }
	void Enable_Sort(void)						{ Model->Set_Flag(MeshGeometryClass::SORT, true); }
	bool Sort_Enabled(void)						{ return (Model->Get_Flag(MeshGeometryClass::SORT) != 0); }

	void Disable_Bounding_Box(void)			{ Model->Set_Flag(MeshGeometryClass::DISABLE_BOUNDING_BOX, true); }
	void Enable_Bounding_Box(void)			{ Model->Set_Flag(MeshGeometryClass::DISABLE_BOUNDING_BOX, false); }
	bool Test_Bounding_Box(void)				{ return (Model->Get_Flag(MeshGeometryClass::DISABLE_BOUNDING_BOX) == 0); }

	void Disable_Bounding_Sphere(void)		{ Model->Set_Flag(MeshGeometryClass::DISABLE_BOUNDING_SPHERE, true); }
	void Enable_Bounding_Sphere(void)		{ Model->Set_Flag(MeshGeometryClass::DISABLE_BOUNDING_SPHERE, false); }
	bool Test_Bounding_Sphere(void)			{ return (Model->Get_Flag(MeshGeometryClass::DISABLE_BOUNDING_SPHERE) == 0); }

	// this is called by the Reset function
	void Set_Dirty( void) { Set_Dirty_Bounds(); Set_Dirty_Planes(); Set_Dirty_Vertex_Normals(); }

	enum {
		MAX_COLOR_ARRAYS = MeshMatDescClass::MAX_COLOR_ARRAYS,
		MAX_PASSES = MeshMatDescClass::MAX_PASSES
	};

	// USER BE WARNED: This hack is only here because DynamicMeshClass does not expose all of the
	// features that DynamicMeshModel provides.  It may be dangerous to modify the model behind the
	// DynamicMeshClass's back so use at your own risk!
	DynamicMeshModel *		Peek_Model(void)	{ return Model; }

protected:

	inline void	Switch_To_Multi_Vertex_Color(int color_array_index = 0);

	// tells when the triangle needs to be back flipped
	virtual bool	Flip_Face( void) { return (!(TriVertexCount & 1)); }

	// Low-level mesh object
	DynamicMeshModel *		Model;

	int							VertexMaterialIdx[MAX_PASSES];
	bool							MultiVertexMaterial[MAX_PASSES];
	int							TextureIdx[MAX_PASSES];
	bool							MultiTexture[MAX_PASSES];

	Vector4						CurVertexColor[MAX_COLOR_ARRAYS];
	bool							MultiVertexColor[MAX_COLOR_ARRAYS];

	// number of polygons in the mesh
	int PolyCount;

	// number of vertices in the mesh
	int VertCount;

	// triangle vertex number
	int TriVertexCount;

	// base vertex when submitting fans
	int FanVertex;

	// is user currently submitting strips or fans
	enum { TRI_MODE_STRIPS = 0, TRI_MODE_FANS = 1 };
	int	TriMode;

	// The static sort level
	char SortLevel;
};

inline Vector3 * DynamicMeshModel::Get_Non_Const_Vertex_Normal_Array(void)
{
	if (Get_Flag(DIRTY_VNORMALS)) {
		Compute_Vertex_Normals();
	}
	return get_vert_normals();
}

inline TriIndex * DynamicMeshModel::Get_Non_Const_Polygon_Array(void)
{
	return get_polys();
}

inline void DynamicMeshClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	if (!Bounding_Volumes_Valid()) {
		Model->Compute_Bounds(NULL);
	}
	Model->Get_Bounding_Sphere(&sphere);
}

inline void DynamicMeshClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	if (!Bounding_Volumes_Valid()) {
		Model->Compute_Bounds(NULL);
	}
	Model->Get_Bounding_Box(&box);
}

/*
**
*/
void DynamicMeshClass::Switch_To_Multi_Vertex_Color(int color_array_index)
{
/*	Vector4 * color_list = Model->Get_Color_Array(color_array_index);
	// set the proper color for all the existing vertices
	for (int lp = 0; lp < VertCount; lp++) {
		color_list[lp].X = CurVertexColor[color_array_index].X;
		color_list[lp].Y = CurVertexColor[color_array_index].Y;
		color_list[lp].Z = CurVertexColor[color_array_index].Z;
		color_list[lp].W = CurVertexColor[color_array_index].W;
	}
*/
	unsigned * color_list = Model->Get_Color_Array(color_array_index);
	// set the proper color for all the existing vertices
	unsigned vertex_color=DX8Wrapper::Convert_Color_Clamp(CurVertexColor[color_array_index]);
	for (int lp = 0; lp < VertCount; lp++) {
		color_list[lp]=vertex_color;
	}

	MultiVertexColor[color_array_index]  = true;
}

/*
**	Set the color of all vertices in the mesh for one color_array_index
*/
int DynamicMeshClass::Set_Vertex_Color(const Vector4 & color, int color_array_index)
{
	// check if switching to multivertexcolor
	if (!MultiVertexColor[color_array_index]) {
		Switch_To_Multi_Vertex_Color(color_array_index);
	}

	CurVertexColor[color_array_index] = color;
	return 0;
}

int DynamicMeshClass::Set_Vertex_Color(const Vector3 & color, int color_array_index)
{
	Set_Vertex_Color(Vector4(color.X,color.Y,color.Z,1),color_array_index);
	return 0;
}


/*
** Dynamic Screen Meshes
**
** Same as DynamicMesh, but mapped in Screen Coordinates
**
** Screen -> 0,0		1,0
**					 +---+
**					 |   |
**					 +---+
**				 0,1	   1,1
**
**
** View  -> -1,1		1,1
**					 +---+
**					 |   |
**					 +---+
**				-1,-1    1,-1
**
** Note: since y is inverted, it switches from right handed to left handed
** (from counter-clockwise to clockwise), so Flip Face accounts for this
*/
class DynamicScreenMeshClass : public DynamicMeshClass {

public:
	// constructor and destructor
	DynamicScreenMeshClass( int max_poly, int max_vert, float aspect = 1.0f ) : DynamicMeshClass( max_poly, max_vert), Aspect( aspect ) {}
	DynamicScreenMeshClass( const DynamicScreenMeshClass & src) : DynamicMeshClass(src), Aspect(src.Aspect) {}
	virtual ~DynamicScreenMeshClass( void) {}

	// function to clone a dynamic screen mesh class
	virtual RenderObjClass * 		Clone(void) const	{ return NEW_REF( DynamicScreenMeshClass, (*this)); }

	// class id of this render object
	virtual int	Class_ID(void) const	{ return CLASSID_DYNASCREENMESH; }

	// Remap locations to match a screen
	virtual void Location( float x, float y, float z = 0.0f);

	// For moving a vertex after the DynaMesh has already been created.
   virtual void Move_Vertex(int index, float x, float y, float z = 0.0f);

	// Set position
	virtual void Set_Position(const Vector3 &v);

	virtual void Reset( void);

	virtual void Set_Aspect(float aspect) { Aspect=aspect; };

protected:

	//	 Aspect Ratio of the virtual screen.
	//  1.0 gives a -1,-1 to 1,1 display
	//  3/4 givs a -1,-3/4 to 1,3/4 display
	float		Aspect;

	// tells when the triangle needs to be back flipped
	virtual	bool	Flip_Face( void) { return !DynamicMeshClass::Flip_Face(); }
};

#endif	// DYNAMESH
