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
 *                 Project Name : WWShade                                                      *
 *                                                                                             *
 *                     $Archive:: wwshade/shdmesh.h														$*
 *                                                                                             *
 *                   Org Author:: Jani P                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 07/12/02 10:31a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * 07/12/02 KM Removed legacy mesh conversion                                                  *
 *---------------------------------------------------------------------------------------------*
 *---------------------------------------------------------------------------------------------*/

#ifndef SHDMESH_H
#define SHDMESH_H

#include "rendobj.h"
#include "wwstring.h"
#include "simplevec.h"
#include "w3derr.h"
#include "matrix4.h"

class ChunkLoadClass;
class LightEnvironmentClass;
class MaterialPassClass;
class ShdSubMeshClass;
class ShdRendererNodeClass;
class MeshClass;
class TexProjectClass;

struct ShdSubMeshStruct
{
	ShdSubMeshClass* Mesh;
	ShdRendererNodeClass* Renderer;
};

/**
** ShdMeshClass - Mesh which uses the new shader system.  This is a render object which
** has the following characteristics:
** - It uses the new shader system
** - It contains one or more sub-meshes which each use only a single shader (unlike the old mesh system)
*/
class ShdMeshClass : public RenderObjClass
{
public:
	ShdMeshClass(void);
	ShdMeshClass(const ShdMeshClass & src);
	virtual ~ShdMeshClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface 
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const { return CLASSID_SHDMESH; }
	virtual const char *			Get_Name(void) const;
	virtual void					Set_Name(const char * name);
	virtual int						Get_Num_Polys(void) const;
	int								Get_Num_Vertices(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
//	void								Render_Material_Pass(MaterialPassClass * pass,IndexBufferClass * ib);
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Collision Detection
	/////////////////////////////////////////////////////////////////////////////	
	virtual bool					Cast_Ray(RayCollisionTestClass & raytest);
	virtual bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
	virtual bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
	virtual bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
	virtual bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);
   
	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Bounding Volumes
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
	virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Attributes, Options, Properties, etc
	/////////////////////////////////////////////////////////////////////////////
//   virtual int						Get_Sort_Level(void) const;
//   virtual void					Set_Sort_Level(int level);	

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Decals
	/////////////////////////////////////////////////////////////////////////////
//	virtual void					Create_Decal(DecalGeneratorClass * generator);
//	virtual void					Delete_Decal(uint32 decal_id);
	
	/////////////////////////////////////////////////////////////////////////////
	// MeshClass Interface
	/////////////////////////////////////////////////////////////////////////////
	void								Init_From_Legacy_Mesh(MeshClass* mesh);
//	WW3DErrorType					Init(const MeshBuilderClass & builder,MaterialInfoClass * matinfo,const char * name,const char * hmodelname);
	WW3DErrorType					Load_W3D(ChunkLoadClass & cload);
	void								Generate_Culling_Tree(void);
//	MeshModelClass *				Get_Model(void);
//	MeshModelClass *				Peek_Model(void);
	uint32							Get_W3D_Flags(void);
	const char *					Get_User_Text(void) const;

	bool								Contains(const Vector3 &point);

	void								Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm);
	void								Get_Deformed_Vertices(Vector3 *dst_vert);

	void								Set_Lighting_Environment(LightEnvironmentClass * light_env) { LightEnvironment = light_env; }
	LightEnvironmentClass *		Get_Lighting_Environment(void) { return LightEnvironment; }

//	void								Set_Next_Visible_Skin(MeshClass * next_visible) { NextVisibleSkin = next_visible; }
//	MeshClass *						Peek_Next_Visible_Skin(void) { return NextVisibleSkin; }
//
//	void								Set_Base_Vertex_Offset(int base) { BaseVertexOffset = base; }
//	int								Get_Base_Vertex_Offset(void) { return BaseVertexOffset; }

	// Do old .w3d mesh files get fog turned on or off?
	static bool						Legacy_Meshes_Fogged;

//	void								Replace_Texture(TextureClass* texture,TextureClass* new_texture);
//	void								Replace_VertexMaterial(VertexMaterialClass* vmat,VertexMaterialClass* new_vmat);

	void								Make_Unique();
//	unsigned							Get_Debug_Id() const { return  MeshDebugId; }
//
//	void								Set_Debugger_Disable(bool b) { IsDisabledByDebugger=b; }
//	bool								Is_Disabled_By_Debugger() const { return IsDisabledByDebugger; }

	/*
	** User Lighting feature, meshes can have a user lighting array.
	*/
//	void								Install_User_Lighting_Array(Vector4 * lighting);
//	unsigned int *					Get_User_Lighting_Array(bool alloc = false);
//
//	virtual void					Save_User_Lighting (ChunkSaveClass & csave);
//	virtual void					Load_User_Lighting (ChunkLoadClass & cload);

	void								Set_Is_Applying_Shadow_Map() { Applying_Shadow_Map=true; }
	void								Unset_Is_Applying_Shadow_Map() { Applying_Shadow_Map=false; }
	bool								Is_Applying_Shadow_Map() const { return Applying_Shadow_Map; }

	TexProjectClass*				Peek_Texture_Projector() const { return Texture_Projector; }

	// ShdMesh Accessors
	int								Get_Sub_Mesh_Count(void) const;
	ShdSubMeshClass *				Peek_Sub_Mesh(int i) const;

protected:

	void								Free(void);

	virtual void					Add_Dependencies_To_List (DynamicVectorClass<StringClass> &file_list, bool textures_only = false);
	virtual void					Update_Cached_Bounding_Volumes(void) const;

	StringClass										Name;
	SimpleVecClass<ShdSubMeshStruct>			SubMeshes;
	LightEnvironmentClass *						LightEnvironment;		// cached pointer to the light environment for this mesh
	bool												Applying_Shadow_Map;
	TexProjectClass*								Texture_Projector;
};

#endif //SHDMESH_H
