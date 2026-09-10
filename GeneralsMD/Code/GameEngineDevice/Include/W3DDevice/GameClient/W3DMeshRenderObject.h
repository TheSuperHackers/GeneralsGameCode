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

#pragma once
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
import Graphics.Scene.Models.Factory;
import Graphics.Scene.MuzzleFlash;


#include "WWLib/always.h"
#include "WWLib/ref_ptr.h"
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "WWLib/bittype.h"
import Graphics.Scene.Lighting.Local;
import Graphics.Scene.Models.Materials;

class HModelClass;
class W3DMeshClass;
class ChunkLoadClass;
class ChunkSaveClass;
class W3DRenderContext;
class W3DMeshResource;
import Graphics.Scene.Models.MeshDrawing;
import Graphics.Scene.Props.Instances;
import Graphics.Scene.Props.SkinPalettes;
struct W3dTexCoordStruct;
class W3DTextureHandle;
import Graphics.Materials.MeshMaterial;

/**
** W3DMeshRenderObject -- Render3DObject for rendering meshes.
*/
class W3DMeshRenderObject : public W3DRenderObject
{
	W3DMPO_CODE(W3DMeshRenderObject)
public:

	W3DMeshRenderObject();
	W3DMeshRenderObject(const W3DMeshRenderObject & src);
	W3DMeshRenderObject & operator = (const W3DMeshRenderObject &);
	virtual ~W3DMeshRenderObject() override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface
	/////////////////////////////////////////////////////////////////////////////
	virtual W3DRenderObject *	Clone() const override;
	virtual int						Class_ID() const override { return CLASSID_MESH; }
	virtual const char *			Get_Name() const override;
	virtual void					Set_Name(const char * name) override;
	virtual int						Get_Num_Polys() const override;
	virtual void					Render(W3DRenderContext & rinfo) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Collision Detection
	/////////////////////////////////////////////////////////////////////////////
	virtual bool					Cast_Ray(W3DRayCastQuery & raytest) override;
	virtual bool					Cast_AABox(W3DBoxCastQuery & boxtest) override;
	virtual bool					Cast_OBBox(W3DOrientedBoxCastQuery & boxtest) override;
	virtual bool					Intersect_AABox(W3DBoxIntersectionQuery & boxtest) override;
	virtual bool					Intersect_OBBox(W3DOrientedBoxIntersectionQuery & boxtest) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Bounding Volumes
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const override;
   virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Attributes, Options, Properties, etc
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Scale(float scale) override;
	virtual void					Scale(float scalex, float scaley, float scalez) override;
	virtual std::shared_ptr<Graphics::ModelMaterials<RefCountPtr<W3DTextureHandle>>> Get_Material_Info() override;

   virtual int						Get_Sort_Level() const override;
   virtual void					Set_Sort_Level(int level) override;


	/////////////////////////////////////////////////////////////////////////////
	// W3DMeshRenderObject Interface
	/////////////////////////////////////////////////////////////////////////////
	bool							Load_W3D(ChunkLoadClass & cload);
	void								Generate_Culling_Tree();
	W3DMeshResource *				Get_Model();
	W3DMeshResource *				Peek_Model();
	uint32							Get_W3D_Flags();
	const char *					Get_User_Text() const;
	int								Get_Draw_Call_Count() const;

	bool								Contains(const Vector3 &point);

	void								Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm);
	void								Get_Deformed_Vertices(Vector3 *dst_vert);

	void								Set_Lighting_Environment(Graphics::LocalLighting * light_env) { if (light_env) {m_localLightEnv=*light_env;LightEnvironment = &m_localLightEnv;} else {LightEnvironment = nullptr;} }
	Graphics::LocalLighting *		Get_Lighting_Environment() { return LightEnvironment; }
	void Set_Muzzle_Flash_Designation(Graphics::MuzzleFlashDesignation designation) { m_muzzleFlashDesignation=designation; }
	Graphics::MuzzleFlashDesignation Get_Muzzle_Flash_Designation() const { return m_muzzleFlashDesignation; }
	float	Get_Alpha_Override() { return m_alphaOverride;}
    Graphics::PropInstanceOwner& Graphics_Instance() { return GraphicsInstance; }
    Graphics::PropSkinOwner& Graphics_Skin() { return GraphicsSkin; }


	// Do old .w3d mesh files get fog turned on or off?

	void								Replace_Texture(W3DTextureHandle* texture,W3DTextureHandle* new_texture);
	void								Replace_VertexMaterial(Graphics::MeshMaterial* vmat,const std::shared_ptr<Graphics::MeshMaterial>& new_vmat);

	void								Make_Unique(bool force_meshmdl_clone = false);


protected:

	virtual void					Add_Dependencies_To_List (DynamicVectorClass<StringClass> &file_list, bool textures_only = false) override;

	virtual void					Update_Cached_Bounding_Volumes() const override;

	void								Free();


	W3DMeshResource *				Model;
    Graphics::PropSkinOwner GraphicsSkin;
    Graphics::PropInstanceOwner GraphicsInstance;

	Graphics::LocalLighting *		LightEnvironment;		// cached pointer to the light environment for this mesh
	Graphics::LocalLighting     m_localLightEnv;	//added for 'Generals'
	float					m_alphaOverride;	//added for 'Generals' to allow variable alpha on meshes.
	float					m_materialPassEmissiveOverride;	//added for 'Generals' to allow variable emissive on additional passes.
	float					m_materialPassAlphaOverride;	//added for 'Generals' to allow variable alpha on additional render passes.
	Graphics::MuzzleFlashDesignation m_muzzleFlashDesignation;


};

inline W3DMeshResource * W3DMeshRenderObject::Peek_Model()
{
	return Model;
}


// This utility function recurses throughout the subobjects of a renderobject,
// and for each W3DMeshRenderObject it finds it sets the given MeshModel flag on its
// model. This is useful for stuff like making a RenderObjects' polys sort.
//void Set_MeshModel_Flag(W3DRenderObject *robj, W3DMeshResource::FlagsType flag, int onoff);
void Set_MeshModel_Flag(W3DRenderObject *robj, int flag, int onoff);

Graphics::ModelFactory<W3DRenderObject>* Load_Mesh_Factory(ChunkLoadClass& cload);
