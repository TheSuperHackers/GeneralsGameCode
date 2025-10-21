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

/* $Header: /Commando/Code/ww3d2/mesh.h 16    11/07/01 5:50p Jani_p $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D engine                                       *
 *                                                                                             *
 *                    File Name : MESH.H                                                       *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 06/11/97                                                     *
 *                                                                                             *
 *                  Last Update : June 11, 1997 [GH]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef MESH_H
#define MESH_H

#include "always.h"
#include "rendobj.h"
#include "bittype.h"
#include "w3derr.h"
#include "lightenvironment.h"	//added for 'Generals'

class MeshBuilderClass;
class HModelClass;
class AuxMeshDataClass;
class MeshLoadInfoClass;
class W3DMeshClass;
class MaterialInfoClass;
class ChunkLoadClass;
class ChunkSaveClass;
class RenderInfoClass;
class MeshModelClass;
class DecalMeshClass;
class MaterialPassClass;
class IndexBufferClass;
struct W3dMeshHeaderStruct;
struct W3dTexCoordStruct;
class TextureClass;
class VertexMaterialClass;
struct VertexFormatXYZNDUV2;

/**
** MeshClass -- Render3DObject for rendering meshes.
*/
class MeshClass : public W3DMPO, public RenderObjClass
{
	W3DMPO_GLUE(MeshClass)
public:

	MeshClass(void);
	MeshClass(const MeshClass & src);
	MeshClass & operator = (const MeshClass &);
	virtual ~MeshClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const { return CLASSID_MESH; }
	virtual const char *			Get_Name(void) const;
	virtual void					Set_Name(const char * name);
	virtual int						Get_Num_Polys(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
	void								Render_Material_Pass(MaterialPassClass * pass,IndexBufferClass * ib);
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
	virtual void					Scale(float scale);
	virtual void					Scale(float scalex, float scaley, float scalez);
	virtual MaterialInfoClass * Get_Material_Info(void);

   virtual int						Get_Sort_Level(void) const;
   virtual void					Set_Sort_Level(int level);


	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Decals
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Create_Decal(DecalGeneratorClass * generator);
	virtual void					Delete_Decal(uint32 decal_id);

	/////////////////////////////////////////////////////////////////////////////
	// MeshClass Interface
	/////////////////////////////////////////////////////////////////////////////
	WW3DErrorType					Init(const MeshBuilderClass & builder,MaterialInfoClass * matinfo,const char * name,const char * hmodelname);
	WW3DErrorType					Load_W3D(ChunkLoadClass & cload);
	void								Generate_Culling_Tree(void);
	MeshModelClass *				Get_Model(void);
	MeshModelClass *				Peek_Model(void);
	uint32							Get_W3D_Flags(void);
	const char *					Get_User_Text(void) const;
	int								Get_Draw_Call_Count(void) const;

	bool								Contains(const Vector3 &point);

	void								Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm);
	void								Get_Deformed_Vertices(Vector3 *dst_vert);

	void								Set_Lighting_Environment(LightEnvironmentClass * light_env) { if (light_env) {m_localLightEnv=*light_env;LightEnvironment = &m_localLightEnv;} else {LightEnvironment = NULL;} }
	LightEnvironmentClass *		Get_Lighting_Environment(void) { return LightEnvironment; }
	inline float	Get_Alpha_Override(void) { return m_alphaOverride;}

	void								Set_Next_Visible_Skin(MeshClass * next_visible) { NextVisibleSkin = next_visible; }
	MeshClass *						Peek_Next_Visible_Skin(void) { return NextVisibleSkin; }

	void								Set_Base_Vertex_Offset(int base) { BaseVertexOffset = base; }
	int								Get_Base_Vertex_Offset(void) { return BaseVertexOffset; }

	// Do old .w3d mesh files get fog turned on or off?
	static bool						Legacy_Meshes_Fogged;

	void								Replace_Texture(TextureClass* texture,TextureClass* new_texture);
	void								Replace_VertexMaterial(VertexMaterialClass* vmat,VertexMaterialClass* new_vmat);

	void								Make_Unique(bool force_meshmdl_clone = false);
	unsigned							Get_Debug_Id() const { return  MeshDebugId; }

	void								Set_Debugger_Disable(bool b) { IsDisabledByDebugger=b; }
	bool								Is_Disabled_By_Debugger() const { return IsDisabledByDebugger; }

protected:

	virtual void					Add_Dependencies_To_List (DynamicVectorClass<StringClass> &file_list, bool textures_only = false);

	virtual void					Update_Cached_Bounding_Volumes(void) const;

	void								Free(void);

	void								install_materials(MeshLoadInfoClass * loadinfo);
	void								clone_materials(const MeshClass & srcmesh);

	MeshModelClass *				Model;
	DecalMeshClass *				DecalMesh;

	LightEnvironmentClass *		LightEnvironment;		// cached pointer to the light environment for this mesh
	LightEnvironmentClass     m_localLightEnv;	//added for 'Generals'
	float					m_alphaOverride;	//added for 'Generals' to allow variable alpha on meshes.
	float					m_materialPassEmissiveOverride;	//added for 'Generals' to allow variable emissive on additional passes.
	float					m_materialPassAlphaOverride;	//added for 'Generals' to allow variable alpha on additional render passes.
	int								BaseVertexOffset;		// offset to our first vertex in whatever vb this mesh is in.
	MeshClass *						NextVisibleSkin;		// linked list of visible skins

	unsigned							MeshDebugId;
	bool								IsDisabledByDebugger;

	friend class MeshBuilderClass;
};

inline MeshModelClass * MeshClass::Peek_Model(void)
{
	return Model;
}


// This utility function recurses throughout the subobjects of a renderobject,
// and for each MeshClass it finds it sets the given MeshModel flag on its
// model. This is useful for stuff like making a RenderObjects' polys sort.
//void Set_MeshModel_Flag(RenderObjClass *robj, MeshModelClass::FlagsType flag, int onoff);
void Set_MeshModel_Flag(RenderObjClass *robj, int flag, int onoff);

#endif /*MESH_H*/

