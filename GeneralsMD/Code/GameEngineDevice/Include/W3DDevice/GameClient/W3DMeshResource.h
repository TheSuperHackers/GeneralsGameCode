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

#include <cstdint>
#include <memory>
import Graphics.Scene.Models.Materials;

import Graphics.Scene.Models.MeshDrawing;

#include "WWMath/vector2.h"
#include "WWMath/vector3.h"
#include "WWMath/vector4.h"
#include "WWMath/Vector3i.h"
#include "WWLib/sharebuf.h"
import Graphics.Materials.State;
#include "WWDebug/wwdebug.h"
import Graphics.Materials.MeshMaterial;
import Graphics.Scene.Props.Material;
#include "WWLib/ref_ptr.h"
#include "WWLib/bittype.h"
#include "WWMath/colmath.h"
#include "WWLib/simplevec.h"
#include "WWLib/wwstring.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DMeshGeometry.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
import Graphics.Scene.Models.MeshMaterialBindings;
import Graphics.Scene.Models.MaterialSlots;

class W3DRenderContext;
class AABoxClass;
class OBBoxClass;
class FrustumClass;
class SphereClass;
class ChunkLoadClass;
class ChunkSaveClass;
class W3DMeshRenderObject;


/**
** W3DMeshResource
** This class is a repository for all of the geometry information that defines the mesh.
** Its purpose is to allow separate instances of a mesh to share as much data as possible.
**
** Material slot collections retain their resource owners independently of the mesh
** description.  Alternate descriptions share those collections, while a copied mesh
** description receives an independent collection with retained resource owners.
**
** Copy/Add_Ref Rules:
** The purpose of this model was to share data between models whenever possible.  To this
** end, some of the arrays of data are handled differently:
**
** ALWAYS SHARED: These are *ALWAYS* Add_Ref'd and thus all point to the same array
** Poly - Connectivity of a mesh are always shared (cannot be changed at runtime)
** VertexShadeIdx - Shade indices of a mesh are always shared (cannot be changed at runtime)
** VertexInfluences - Vertex bone attachments are always shared (cannot be changed at runtime)
**
** SHARED UNTIL SCALED, SKIN DEFORMED, or DAMAGED:
** Vertex - vertex positions must be copied if any are moved...
** VertexNorm - vertex normals cannot be shared if a vertex is moved
** PlaneEq - plane equations cannot be shared if a vertex is moved
** CullTree - culling tree becomes instance specific if a vertex moves (shouldn't even use this with skins...)
**
** ALWAYS UNIQUE, BUT SHARE ARRAYS BETWEEN ALTERNATE MATERIAL REPRESENTATIONS
** UV, DIG, DCG, SCG
** Texture, Shader, Material,
** TextureArray, MaterialArray, ShaderArray
*/


class W3DMeshResource : public W3DMeshGeometry
{
	W3DMPO_CODE(W3DMeshResource)

public:
	using MaterialDescription = Graphics::MeshMaterialBindings<RefCountPtr<W3DTextureHandle>, Vector2>;
	using TextureSlots = Graphics::MaterialSlots<RefCountPtr<W3DTextureHandle>>;
	using MaterialSlots = Graphics::MaterialSlots<std::shared_ptr<Graphics::MeshMaterial>>;

	W3DMeshResource();
	W3DMeshResource(const W3DMeshResource & that);
	virtual ~W3DMeshResource() override;

    std::unique_ptr<Graphics::ModelMeshState<RefCountPtr<W3DTextureHandle>>>& Graphics_Mesh_State() noexcept { return GraphicsMeshes; }
    MaterialDescription& Material_Bindings() noexcept { return *CurMatDesc; }
    std::uint64_t UV_Revision(int pass,int stage) const noexcept { return CurMatDesc->UV_Revision(pass,stage); }

	W3DMeshResource & operator = (const W3DMeshResource & that);
	void							Reset(int polycount,int vertcount,int passcount);

	/////////////////////////////////////////////////////////////////////////////////////
	// Material interface, All of these functions call through to the current
	// material description.
	/////////////////////////////////////////////////////////////////////////////////////
	void							Set_Pass_Count(int passes)														{ CurMatDesc->Set_Pass_Count(passes); }
	int							Get_Pass_Count() const														{ return CurMatDesc->Get_Pass_Count(); }

	const Vector2 *			Get_UV_Array(int pass = 0, int stage = 0)									{ return CurMatDesc->Peek_UV_Array(pass,stage); }
	int							Get_UV_Array_Count()														{ return CurMatDesc->Get_UV_Array_Count(); }
	const Vector2 *			Get_UV_Array_By_Index(int index)												{ return CurMatDesc->Peek_UV_Array_By_Index(index); }

	unsigned *					Get_DCG_Array(int pass)															{ return CurMatDesc->Get_DCG_Array(pass); }
	unsigned *					Get_DIG_Array(int pass)															{ return CurMatDesc->Get_DIG_Array(pass); }
	Graphics::PropColorSource Get_DCG_Source(int pass)										{ return CurMatDesc->Get_DCG_Source(pass); }
	Graphics::PropColorSource Get_DIG_Source(int pass)										{ return CurMatDesc->Get_DIG_Source(pass); }

	unsigned *					Get_Color_Array(int array_index,bool create = true)					{ return CurMatDesc->Get_Color_Array(array_index,create); }

	void							Set_Single_Material(const std::shared_ptr<Graphics::MeshMaterial>& vmat,int pass=0)			{ CurMatDesc->Set_Single_Material(vmat,pass); }
	void							Set_Single_Texture(W3DTextureHandle * tex,int pass=0,int stage=0)
	{
		CurMatDesc->Set_Single_Texture(RefCountPtr<W3DTextureHandle>::Create_Add_Ref(tex),pass,stage);
	}
	void							Set_Single_Shader(Graphics::MaterialState shader,int pass=0)						{ CurMatDesc->Set_Single_Shader(shader,pass); }

	// the "Get" functions add a reference before returning the pointer (if appropriate)
	std::shared_ptr<Graphics::MeshMaterial>	Get_Single_Material(int pass=0) const										{ return CurMatDesc->Get_Single_Material(pass); }
	W3DTextureHandle *				Get_Single_Texture(int pass=0,int stage=0) const
	{
		auto texture = CurMatDesc->Get_Single_Texture(pass,stage);
		return texture.Release();
	}
	Graphics::MaterialState					Get_Single_Shader(int pass=0) const											{ return CurMatDesc->Get_Single_Shader(pass); }

	// the "Peek" functions just return the pointer and it's the caller's responsibility to
	// maintain a reference to an object with a reference to the data
	Graphics::MeshMaterial *	Peek_Single_Material(int pass=0) const										{ return CurMatDesc->Peek_Single_Material(pass); }
	W3DTextureHandle *				Peek_Single_Texture(int pass=0,int stage=0) const						{ return CurMatDesc->Peek_Single_Texture(pass,stage); }

	void							Set_Material(int vidx,const std::shared_ptr<Graphics::MeshMaterial>& vmat,int pass=0)		{ CurMatDesc->Set_Material(vidx,vmat,pass); }
	void							Set_Shader(int pidx,Graphics::MaterialState shader,int pass=0)						{ CurMatDesc->Set_Shader(pidx,shader,pass); }
	void							Set_Texture(int pidx,W3DTextureHandle * tex,int pass=0,int stage=0)
	{
		CurMatDesc->Set_Texture(static_cast<std::size_t>(pidx),
			RefCountPtr<W3DTextureHandle>::Create_Add_Ref(tex),pass,stage);
	}

	// Queries for determining whether this model has per-polygon arrays of Materials, Shaders, or Textures
	bool							Has_Material_Array(int pass) const											{ return CurMatDesc->Has_Material_Array(pass); }
	bool							Has_Shader_Array(int pass) const												{ return CurMatDesc->Has_Shader_Array(pass); }
	bool							Has_Texture_Array(int pass,int stage) const								{ return CurMatDesc->Has_Texture_Array(pass,stage); }

	// "Get" functions for Materials, Textures, and Shaders when there are more than one (per-polygon/per-vertex)
	std::shared_ptr<Graphics::MeshMaterial>	Get_Material(int vidx,int pass=0) const									{ return CurMatDesc->Get_Material(vidx,pass); }
	W3DTextureHandle *				Get_Texture(int pidx,int pass=0,int stage=0) const
	{
		auto texture = CurMatDesc->Get_Texture(static_cast<std::size_t>(pidx),pass,stage);
		return texture.Release();
	}
	Graphics::MaterialState					Get_Shader(int pidx,int pass=0) const										{ return CurMatDesc->Get_Shader(pidx,pass); }

	// "Peek" functions for Materials and Textures when there are more than one (per-polygon/per-vertex)
	Graphics::MeshMaterial *	Peek_Material(int vidx,int pass=0) const									{ return CurMatDesc->Peek_Material(vidx,pass); }
	W3DTextureHandle *				Peek_Texture(int pidx,int pass=0,int stage=0) const					{ return CurMatDesc->Peek_Texture(pidx,pass,stage); }

	void							Replace_Texture(W3DTextureHandle* texture,W3DTextureHandle* new_texture);
	void							Replace_VertexMaterial(Graphics::MeshMaterial* vmat,const std::shared_ptr<Graphics::MeshMaterial>& new_vmat);

	/////////////////////////////////////////////////////////////////////////////////////
	// Modification interface.  Call these functions to cause the model to ensure
	// that the specified array is unique to this instance.  (I.e. if the specified
	// data is being shared, break the link!)
	/////////////////////////////////////////////////////////////////////////////////////
	void							Make_Geometry_Unique();
	void							Make_UV_Array_Unique(int pass=0,int stage=0);
	void							Make_Color_Array_Unique(int array_index=0);

	// Load the w3d file format
	virtual bool						Load_W3D(ChunkLoadClass & cload) override;

	/////////////////////////////////////////////////////////////////////////////////////
	//	Alternate Material Description Interface
	// Some models will allow you to alternate between multiple material descriptions
	/////////////////////////////////////////////////////////////////////////////////////
	void							Enable_Alternate_Material_Description(bool onoff);
	bool							Is_Alternate_Material_Description_Enabled();

	// Process texture reductions
//	void							Process_Texture_Reduction();

	// Determine whether any rendering feature used by this mesh requires vertex normals
	bool							Needs_Vertex_Normals();


protected:

	// W3DMeshRenderObject will set this for skins so that they can get the bone transforms

public: // Jani: I need to have an access to these for now...

	TextureSlots *	Get_Texture_Array(int pass,int stage,bool create = true)
	{
		return CurMatDesc->Get_Texture_Array(pass,stage,create);
	}
	MaterialSlots *	Get_Material_Array(int pass,bool create = true)
	{
		return CurMatDesc->Get_Material_Array(pass,create);
	}
	Graphics::MaterialState *				Get_Shader_Array(int pass,bool create = true)
	{
		return CurMatDesc->Get_Shader_Array(pass,create);
	}

protected:


    void compute_static_sort_levels();
    void modify_for_overbright();
    void clone_materials(const W3DMeshResource& source);

	// Material Descriptions
	// DefMatDesc - the default material description, allocated in constructor, always present.
	// AlternateMatDes - an optional alternate material description, allocated at load time if needed
	MaterialDescription DefMatDesc;
	std::unique_ptr<MaterialDescription> AlternateMatDesc;
	MaterialDescription * CurMatDesc;

	// Collection of the unique materials in the mesh
	std::shared_ptr<Graphics::ModelMaterials<RefCountPtr<W3DTextureHandle>>> MatInfo;

    std::unique_ptr<Graphics::ModelMeshState<RefCountPtr<W3DTextureHandle>>> GraphicsMeshes;


	friend class W3DMeshRenderObject;
	};

