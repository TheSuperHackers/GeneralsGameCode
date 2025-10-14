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
 *                     $Archive:: /Commando/Code/ww3d2/meshmdlio.cpp                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/18/02 3:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 27                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MeshModelClass::Load_W3D -- Load a mesh from a W3D file                                   *
 *   MeshModelClass::read_chunks -- read all of the chunks for a mesh                          *
 *   MeshModelClass::read_vertices -- reads the vertex chunk                                   *
 *   MeshModelClass::read_texcoords -- read in the texture coordinates chunk                   *
 *   MeshModelClass::read_vertex_normals -- reads a vertex normal chunk from the w3d file      *
 *   MeshModelClass::read_v3_materials -- Reads in version 3 materials.                        *
 *   MeshModelClass::read_triangles -- read the triangles chunk                                *
 *   MeshModelClass::read_per_tri_materials -- read the material indices for each triangle     *
 *   MeshModelClass::read_user_text -- read in the user text chunk                             *
 *   MeshModelClass::read_vertex_colors -- read in the vertex colors chunk                     *
 *   MeshModelClass::read_vertex_influences -- read in the vertex influences chunk             *
 *   MeshModelClass::read_vertex_shade_indices -- read the shade index chunk                   *
 *   MeshModelClass::read_material_info -- read the material info chunk                        *
 *   MeshModelClass::read_shaders -- read the shaders chunk                                    *
 *   MeshModelClass::read_vertex_materials -- read the vertex materials chunk                  *
 *   MeshModelClass::read_textures -- read the textures chunk                                  *
 *   MeshModelClass::read_material_pass -- read a material pass chunk                          *
 *   MeshModelClass::read_vertex_material_ids -- read the vmat ids for a pass                  *
 *   MeshModelClass::read_shader_ids -- read the shader indexes for a pass                     *
 *   MeshModelClass::read_dcg -- read the per-vertex diffuse color for a pass                  *
 *   MeshModelClass::read_dig -- read the per-vertex diffuse illumination for a pass           *
 *   MeshModelClass::read_scg -- read the specular color for a pass                            *
 *   MeshModelClass::read_texture_stage -- read texture stage chunks                           *
 *   MeshModelClass::read_texture_ids -- read the texture ids for a pass,stage                 *
 *   MeshModelClass::read_stage_texcoords -- read the texcoords for a pass,stage               *
 *	  MeshModelClass::read_per_face_texcoord_ids -- read uv indices for given (pass,stage).	  *
 *	  MeshModelClass::read_prelit_material -- read prelit material chunks.							  *
 *   MeshModelClass::read_aabtree -- loads the aabtree chunk                                   *
 *   MeshModelClass::Save -- Save this mesh model!                                             *
 *   MeshLoadContextClass::MeshLoadContextClass -- constructor for MeshLoadContextClass        *
 *   MeshLoadContextClass::~MeshLoadContextClass -- destructor                                 *
 *   MeshLoadContextClass::Get_Texcoord_Array -- returns the texture coordinates array         *
 *   MeshLoadContextClass::Add_Shader -- adds a shader to the array                            *
 *   MeshLoadContextClass::Add_Vertex_Materail -- adds a vertex material                       *
 *   MeshLoadContextClass::Add_Texture -- adds a texture                                       *
 *   MeshLoadContextClass::Add_Legacy_Material -- adds a legacy material                       *
 *   MeshLoadContextClass::Peek_Legacy_Shader -- returns a legacy shader                       *
 *   MeshLoadContextClass::Peek_Legacy_Vertex_Material -- returns a pointer to a legacy vmat   *
 *   MeshLoadContextClass::Peek_Legacy_Texture -- returns a pointer to a texture               *
 *   MeshSaveContextClass::MeshSaveContextClass -- constructor                                 *
 *   MeshSaveContextClass::~MeshSaveContextClass -- destructor                                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "meshmdl.h"
#include "aabtree.h"
#include "matinfo.h"
#include "vertmaterial.h"
#include "shader.h"
#include "texture.h"
#include "chunkio.h"
#include "w3derr.h"
#include "w3d_file.h"
#include "w3d_util.h"
#include "assetmgr.h"
#include "simplevec.h"
#include "realcrc.h"
#include "dx8wrapper.h"

#include <stdio.h>

#ifdef _UNIX
#include "osdep/osdep.h"
#endif

#define MESH_SINGLE_MATERIAL_HACK		0		// (gth) forces all multi-material meshes to use their first material only. (NOT RECOMMENDED, TESTING ONLY!)
#define MESH_FORCE_STATIC_SORT_HACK	0		// (gth) forces all sorting meshes to use static sort level 1 instead.
/**
** MeshLoadContextClass
** This class is just used as a temporary scratchpad while a mesh is being
** loaded.  In some cases, a chunk will be encountered before I have a place
** to plug it into the mesh model, etc.  It is also used to convert from the
** old material format to the new one (detecting duplicated materials in the
** process).
**
** This object will hold refs to all of the unique material objects.  These
** objects will later be transferred into the MaterialInfo for the mesh and it
** will own the refs for the mesh.  The load context object is destroyed once
** loading is complete...
*/
class MeshLoadContextClass : public W3DMPO
{
	W3DMPO_GLUE(MeshLoadContextClass)
private:
	MeshLoadContextClass(void);
	~MeshLoadContextClass(void);

	W3dTexCoordStruct *		Get_Texcoord_Array(void);

	int							Add_Shader(ShaderClass shader);
	int							Add_Vertex_Material(VertexMaterialClass * vmat);
	int							Add_Texture(TextureClass* tex);

	ShaderClass					Peek_Shader(int index)											{ return Shaders[index]; }
	VertexMaterialClass *	Peek_Vertex_Material(int index)								{ return VertexMaterials[index]; }
	TextureClass *				Peek_Texture(int index)											{ return Textures[index]; }

	int							Shader_Count(void)												{ return Shaders.Count(); }
	int							Vertex_Material_Count(void)									{ return VertexMaterials.Count(); }
	int							Texture_Count(void)												{ return Textures.Count(); }

	/*
	** Legacy material support.
	*/
	void							Add_Legacy_Material(ShaderClass shader,VertexMaterialClass * vmat,TextureClass * tex);
	ShaderClass					Peek_Legacy_Shader(int legacy_material_index);
	VertexMaterialClass *	Peek_Legacy_Vertex_Material(int legacy_material_index);
	TextureClass *				Peek_Legacy_Texture(int legacy_material_index);

	/*
	** Redundant UV detection support.  The context provides a temporary buffer to
	** load the uv coordinates into.
	*/
	Vector2 *					Get_Temporary_UV_Array(int elementcount);


	/*
	** Currently, the only tool that creates DIG chunks is the lightmap tool.  Since DX8 support
	** for linking the emissive material color to an array seems poor, We're just going to multiply
	** the DIG array into the DCG array (or make it the DCG array).  Now, to properly support the
	** "alternate material set", we have to know whether we've already encountered a DIG chunk so
	** these flags provide that functionality.
	*/
	void							Notify_Loaded_DIG_Chunk(bool onoff = true)				{ LoadedDIG = onoff; }
	bool							Already_Loaded_DIG(void)										{ return LoadedDIG; }

private:

	struct LegacyMaterialClass
	{
		LegacyMaterialClass(void) : VertexMaterialIdx(0),ShaderIdx(0),TextureIdx(0)	{ }
		~LegacyMaterialClass(void)	{ }
		void		Set_Name(const char * name) { Name=name; }

		StringClass Name;
		int		VertexMaterialIdx;
		int		ShaderIdx;
		int		TextureIdx;
	};


	W3dMeshHeader3Struct		Header;
	W3dTexCoordStruct *		TexCoords;
	W3dMaterialInfoStruct	MatInfo;

	uint32						PrelitChunkID;

	int							CurPass;
	int							CurTexStage;

	DynamicVectorClass < LegacyMaterialClass * >		LegacyMaterials;
	DynamicVectorClass < ShaderClass >					Shaders;
	DynamicVectorClass < VertexMaterialClass * >		VertexMaterials;
	DynamicVectorClass < unsigned long >				VertexMaterialCrcs;
	DynamicVectorClass < TextureClass * >				Textures;

	/*
	** Alternate material data.  Any alternate material data will be loaded into
	** this MeshMatDescClass object.  When loading is finished, an alternate MeshMatDescClass
	** will be allocated in the mesh model.  This MeshMatDescClass will be initialized to be
	** identical to the default MeshMatDescClass and then any data contained in this
	** MeshMatDescClass will replace the relevant arrays.
	*/
	MeshMatDescClass											AlternateMatDesc;

	SimpleVecClass<Vector2>									TempUVArray;

	/*
	** Record when we load the DIG chunk
	*/
	bool															LoadedDIG;

	friend class MeshClass;
	friend class MeshModelClass;
};


/*
** MeshSaveContextClass
** This class is used to pass information between the saving code in a mesh
*/
class MeshSaveContextClass
{
public:
	MeshSaveContextClass(void);
	~MeshSaveContextClass(void);

	int								CurPass;
	int								CurStage;
	MaterialCollectorClass		Materials;
};


/***********************************************************************************************
 * MeshModelClass::Load_W3D -- Load a mesh from a W3D file                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::Load_W3D(ChunkLoadClass & cload)
{
	MeshLoadContextClass * context = NULL;

	/*
	**	Open the first chunk, it should be the mesh header
	*/
	cload.Open_Chunk();

	if (cload.Cur_Chunk_ID() != W3D_CHUNK_MESH_HEADER3) {
		WWDEBUG_SAY(("Old format mesh mesh, no longer supported."));
		goto Error;
	}

	context = W3DNEW MeshLoadContextClass;

	if (cload.Read(&(context->Header),sizeof(W3dMeshHeader3Struct)) != sizeof(W3dMeshHeader3Struct)) {
		goto Error;
	}
	cload.Close_Chunk();

	/*
	** Process the header
	*/
	char *	tmpname;
	int		namelen;

	Reset(context->Header.NumTris,context->Header.NumVertices,1);

	namelen = strlen(context->Header.ContainerName);
	namelen += strlen(context->Header.MeshName);
	namelen += 2;
	W3dAttributes = context->Header.Attributes;
	SortLevel = context->Header.SortLevel;
	tmpname = W3DNEWARRAY char[namelen];
	memset(tmpname,0,namelen);

	if (strlen(context->Header.ContainerName) > 0) {
		strcpy(tmpname,context->Header.ContainerName);
		strcat(tmpname,".");
	}
	strcat(tmpname,context->Header.MeshName);

	Set_Name(tmpname);

	delete[] tmpname;
	tmpname = NULL;

	context->AlternateMatDesc.Set_Vertex_Count(VertexCount);
	context->AlternateMatDesc.Set_Polygon_Count(PolyCount);

	/*
	** Set Bounding Info
	*/
	BoundBoxMin.Set(context->Header.Min.X,context->Header.Min.Y,context->Header.Min.Z);
	BoundBoxMax.Set(context->Header.Max.X,context->Header.Max.Y,context->Header.Max.Z);

	BoundSphereCenter.Set(context->Header.SphCenter.X,context->Header.SphCenter.Y,context->Header.SphCenter.Z);
	BoundSphereRadius = context->Header.SphRadius;

	/*
	** Flags
	*/
	if (context->Header.Version >= W3D_MAKE_VERSION(4,1)) {
		int geometry_type = context->Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;
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
				Set_Flag(ALLOW_NPATCHES,true);
				break;
		}
	}

	if (context->Header.Attributes & W3D_MESH_FLAG_TWO_SIDED) {
		Set_Flag(TWO_SIDED,true);
	}

	if (context->Header.Attributes & W3D_MESH_FLAG_CAST_SHADOW) {
		Set_Flag(CAST_SHADOW,true);
	}

	if (context->Header.Attributes & W3D_MESH_FLAG_NPATCHABLE) {
		Set_Flag(ALLOW_NPATCHES,true);
	}

	// Configure the load sequence for prelighting.
	if (context->Header.Attributes & W3D_MESH_FLAG_PRELIT_MASK) {

		// Select from the available prelit materials based on current prelit lighting mode.
		// If the model does not have the current prelit mode, select the next highest quality
		// prelit material that is available.
		switch (WW3D::Get_Prelit_Mode()) {

			case WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE:
				if (context->Header.Attributes & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE) {
					context->PrelitChunkID = W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE;
					Set_Flag (PRELIT_LIGHTMAP_MULTI_TEXTURE, true);
					break;
				}
				FALLTHROUGH; // Else fall thru...

			case WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS:
				if (context->Header.Attributes & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS) {
					context->PrelitChunkID = W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS;
					Set_Flag (PRELIT_LIGHTMAP_MULTI_PASS, true);
					break;
				}
				FALLTHROUGH; // Else fall thru...

			case WW3D::PRELIT_MODE_VERTEX:
				if (context->Header.Attributes & W3D_MESH_FLAG_PRELIT_VERTEX) {
					context->PrelitChunkID = W3D_CHUNK_PRELIT_VERTEX;
					Set_Flag (PRELIT_VERTEX, true);
					break;
				}
				FALLTHROUGH; // Else fall thru...

			default:

				// This prelighting option MUST be available if none of the others are available.
				WWASSERT (context->Header.Attributes & W3D_MESH_FLAG_PRELIT_UNLIT);
				context->PrelitChunkID = W3D_CHUNK_PRELIT_UNLIT;
				break;
		}

	} else {

		// For backwards compatibility, test for obsolete lightmap flag.
		if (context->Header.Attributes & OBSOLETE_W3D_MESH_FLAG_LIGHTMAPPED) {
			Set_Flag (PRELIT_LIGHTMAP_MULTI_PASS, true);
		}

		// Else this mesh has no prelighting.
	}

	read_chunks(cload,context);

	/*
	** If this is a pre-3.0 mesh and it has vertex influences,
	** fixup the bone indices to account for the new root node
	*/
	if ((context->Header.Version < W3D_MAKE_VERSION(3,0)) && (Get_Flag(SKIN))) {

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

	/*
	** Transfer the materials into the MatInfo
	*/
	install_materials(context);

	/*
	** Delete the temporary LoadInfo object
	*/
	delete context;

	/*
	** Post-process the model: optimize passes, activate fog etc.
	*/
	post_process();

	return WW3D_ERROR_OK;

Error:

	return WW3D_ERROR_LOAD_FAILED;
}


/***********************************************************************************************
 * MeshModelClass::read_chunks -- read all of the chunks for a mesh                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_chunks(ChunkLoadClass & cload,MeshLoadContextClass * context)
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
					// call up to MeshGeometryClass
					error = read_vertices(cload);
					break;

			case W3D_CHUNK_SURRENDER_NORMALS:
			case W3D_CHUNK_VERTEX_NORMALS:
					// call up to MeshGeometryClass
					error = read_vertex_normals(cload);
					break;

			case W3D_CHUNK_TEXCOORDS:
					error = read_texcoords(cload,context);
					break;

			case O_W3D_CHUNK_MATERIALS:
			case O_W3D_CHUNK_MATERIALS2:
					WWDEBUG_SAY(( "Obsolete material chunk encountered in mesh: %s.%s", context->Header.ContainerName,context->Header.MeshName));
					WWASSERT(0);
					break;

			case W3D_CHUNK_MATERIALS3:
					WWDEBUG_SAY(( "Obsolete material chunk encountered in mesh: %s.%s", context->Header.ContainerName,context->Header.MeshName));
					error = read_v3_materials(cload,context);
					break;

			case O_W3D_CHUNK_SURRENDER_TRIANGLES:
					WWASSERT_PRINT( 0, "Obsolete Triangle Chunk Encountered!" );
					break;

			case W3D_CHUNK_TRIANGLES:
					// call up to MeshGeometryClass
					error = read_triangles(cload);
					break;

			case W3D_CHUNK_PER_TRI_MATERIALS:
					error = read_per_tri_materials(cload,context);
					break;

			case W3D_CHUNK_MESH_USER_TEXT:
					// call up to MeshGeometryClass
					error = read_user_text(cload);
					break;

			case W3D_CHUNK_VERTEX_COLORS:
					error = read_vertex_colors(cload,context);
					break;

			case W3D_CHUNK_VERTEX_INFLUENCES:
					// call up to MeshGeometryClass
					error = read_vertex_influences(cload);
					break;

			case W3D_CHUNK_VERTEX_SHADE_INDICES:
					// call up to MeshGeometryClass
					error = read_vertex_shade_indices(cload);
					break;

			case W3D_CHUNK_MATERIAL_INFO:
					error = read_material_info(cload,context);
					break;

			case W3D_CHUNK_SHADERS:
					error = read_shaders(cload,context);
					break;

			case W3D_CHUNK_VERTEX_MATERIALS:
					error = read_vertex_materials(cload,context);
					break;

			case W3D_CHUNK_TEXTURES:
					error = read_textures(cload,context);
					break;

			case W3D_CHUNK_MATERIAL_PASS:
					error = read_material_pass(cload,context);
					break;

			case W3D_CHUNK_DEFORM:
					WWDEBUG_SAY(("Obsolete deform chunk encountered in mesh: %s.%s", context->Header.ContainerName,context->Header.MeshName));
					break;

			case W3D_CHUNK_DAMAGE:
					WWDEBUG_SAY(("Obsolete damage chunk encountered in mesh: %s.%s", context->Header.ContainerName,context->Header.MeshName));
					break;

			case W3D_CHUNK_PRELIT_UNLIT:
			case W3D_CHUNK_PRELIT_VERTEX:
			case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS:
			case W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE:
					read_prelit_material (cload, context);
					break;

			case W3D_CHUNK_AABTREE:
					// call up to MeshGeometryClass
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
 * MeshModelClass::read_texcoords -- read in the texture coordinates chunk                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/6/98     GTH : Created.                                                                 *
 *   2/16/99    GTH : Moved into MeshModel                                                     *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_texcoords(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	W3dTexCoordStruct texcoord;
	Vector2 * uvarray = 0;
	int elementcount = cload.Cur_Chunk_Length() / sizeof (W3dTexCoordStruct);

	uvarray = context->Get_Temporary_UV_Array(elementcount);

	if (uvarray != NULL) {
		/*
		** Read the uv's into the first u-v pass array
		** NOTE: this is an obsolete function.  Texture coordinates are now
		** loaded in the pass chunks
		*/
		for (int i=0; i<VertexCount; i++) {
			if (cload.Read(&(texcoord),sizeof(W3dTexCoordStruct)) != sizeof(W3dTexCoordStruct)) {
				return WW3D_ERROR_LOAD_FAILED;
			}
			uvarray[i].Set(texcoord.U,1.0f - texcoord.V);
		}

		DefMatDesc->Install_UV_Array(context->CurPass,context->CurTexStage,uvarray,elementcount);
	}

	return WW3D_ERROR_OK;
}

/***********************************************************************************************
 * MeshModelClass::read_v3_materials -- Reads in version 3 materials.                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/2/98     GTH : Created.                                                                 *
 *   2/16/99    GTH : Moved into MeshModelClass                                                *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_v3_materials(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	for (unsigned int mi=0; mi<context->Header.NumMaterials; mi++) {

		/*
		** First, we expect a W3D_CHUNK_MATERIAL3 to wrap the entire material
		*/
		if (!cload.Open_Chunk()) goto Error;
		if (cload.Cur_Chunk_ID() != W3D_CHUNK_MATERIAL3) goto Error;

		/*
		** Inside the MATERIAL3 will be the following:
		**
		** W3D_MATERIAL3_NAME - name of the material
		** W3D_MATERIAL3_INFO - equivalent to 1.40 vertex material parameters
		** W3D_MATERIAL3_DC_MAP - diffuse color mapping
		**   W3D_MAP3_FILENAME - filename of the texture map
		**   W3D_MAP3_INFO - animation, etc information
		** W3D_MATERIAL3_DI_MAP - diffuse illumination map
		** W3D_MATERIAL3_SC_MAP - specular color map
		** W3D_MATERIAL3_SI_MAP - specular illumination map
		*/
		VertexMaterialClass *		vmat = NULL;
		ShaderClass						shader;
		TextureClass *					tex = NULL;
		char								name[256];

		/*
		** Read the material name
		*/
		if (!cload.Open_Chunk()) goto Error;
		if (cload.Cur_Chunk_ID() != W3D_CHUNK_MATERIAL3_NAME) goto Error;
		cload.Read(name,cload.Cur_Chunk_Length());
		if (!cload.Close_Chunk()) goto Error;

		/*
		** Read the vertex material parameters
		*/
		if (!cload.Open_Chunk()) goto Error;

			W3dMaterial3Struct mat;
			if (cload.Cur_Chunk_ID() != W3D_CHUNK_MATERIAL3_INFO) goto Error;
			if (cload.Read(&mat,sizeof(W3dMaterial3Struct)) != sizeof(W3dMaterial3Struct)) goto Error;
			vmat = W3DNEW VertexMaterialClass;
			vmat->Init_From_Material3(mat);
			vmat->Set_Name(name);
			shader.Init_From_Material3(mat);

			/*
			** If this shader does alpha blending, the mesh must be sorted.
			*/
			if (shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO) {
				Set_Flag(MeshModelClass::SORT,true);
			}

		if (!cload.Close_Chunk()) goto Error;

		/*
		** Look for the DC map and read it in
		*/
		while (cload.Open_Chunk()) {
			if (cload.Cur_Chunk_ID() == W3D_CHUNK_MATERIAL3_DC_MAP) {

				/*
				** Read in the texture filename
				*/
				char filename[_MAX_FNAME + _MAX_EXT];
				if (!cload.Open_Chunk()) goto Error;
					if (cload.Cur_Chunk_ID() != W3D_CHUNK_MAP3_FILENAME) goto Error;
					if (cload.Cur_Chunk_Length() >= sizeof(filename)) goto Error;
					cload.Read(filename,cload.Cur_Chunk_Length());
				if (!cload.Close_Chunk()) goto Error;

				/*
				** Read in the auxiliary map info
				*/
				W3dMap3Struct mapinfo;
				if (!cload.Open_Chunk()) goto Error;
					if (cload.Cur_Chunk_ID() != W3D_CHUNK_MAP3_INFO) goto Error;
					if (cload.Read(&mapinfo,sizeof(W3dMap3Struct)) != sizeof(W3dMap3Struct)) goto Error;
				if (!cload.Close_Chunk()) goto Error;

				if ( mapinfo.FrameCount > 1 ) {
					WWDEBUG_SAY(("ERROR: Obsolete Animated Texture detected in model: %s",context->Header.MeshName));
				}

				tex = WW3DAssetManager::Get_Instance()->Get_Texture(filename);

				shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);

			} else if (cload.Cur_Chunk_ID() == W3D_CHUNK_MATERIAL3_SI_MAP) {
				Vector3	diffuse_color;
				vmat->Get_Diffuse( &diffuse_color);
				if ( diffuse_color == Vector3( 0,0,0 ) ) {

					/*
					** Read in the texture filename
					*/
					char filename[_MAX_FNAME + _MAX_EXT];
					if (!cload.Open_Chunk()) goto Error;
						if (cload.Cur_Chunk_ID() != W3D_CHUNK_MAP3_FILENAME) goto Error;
						if (cload.Cur_Chunk_Length() >= sizeof(filename)) goto Error;
						cload.Read(filename,cload.Cur_Chunk_Length());
					if (!cload.Close_Chunk()) goto Error;

					if (tex) tex->Release_Ref();

					/*
					** Read in the auxiliary map info
					*/
					W3dMap3Struct mapinfo;
					if (!cload.Open_Chunk()) goto Error;
						if (cload.Cur_Chunk_ID() != W3D_CHUNK_MAP3_INFO) goto Error;
						if (cload.Read(&mapinfo,sizeof(W3dMap3Struct)) != sizeof(W3dMap3Struct)) goto Error;
					if (!cload.Close_Chunk()) goto Error;

					if ( mapinfo.FrameCount > 1 ) {
						WWDEBUG_SAY(("ERROR: Obsolete Animated Texture detected in model: %s",context->Header.MeshName));
					}

					tex = WW3DAssetManager::Get_Instance()->Get_Texture(filename);

					shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
					shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE);
					shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ONE);
					shader.Set_Primary_Gradient(ShaderClass::GRADIENT_DISABLE);
				}
			}

			cload.Close_Chunk();
		}

		// If not texturing, move the diffuse color to ambient (simulating old behavior)
		if ( shader.Get_Texturing() == ShaderClass::TEXTURING_DISABLE ) {
			Vector3 color;
			vmat->Get_Diffuse( &color );
			vmat->Set_Ambient( color );
			vmat->Set_Diffuse( Vector3( 0, 0, 0 ) );
		}

		context->Add_Legacy_Material(shader,vmat,tex);

		vmat->Release_Ref();
		if (tex) tex->Release_Ref();
		vmat = NULL;
		tex = NULL;

		/*
		** Close the W3D_CHUNK_MATERIAL3
		*/
		cload.Close_Chunk();
	}

	/*
	** Install the default materials to use in the absence of an array
	*/
	if (context->Vertex_Material_Count() >= 1) {
		Set_Single_Material(context->Peek_Vertex_Material(0),0);
	}

	if (context->Texture_Count() >= 1) {
		Set_Single_Texture(context->Peek_Texture(0),0);
	}

	if (context->Shader_Count() >= 1) {
		Set_Single_Shader(context->Peek_Shader(0),0);
	}

	return WW3D_ERROR_OK;

Error:

	return WW3D_ERROR_LOAD_FAILED;

}


/***********************************************************************************************
 * MeshModelClass::read_per_tri_materials -- read the material indices for each triangle       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/7/98     GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_per_tri_materials(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	if (context->Header.NumMaterials == 1) return WW3D_ERROR_OK;

	TriIndex * polys = get_polys();

	bool multi_mtl = (context->Vertex_Material_Count() > 1);
	bool multi_tex = (context->Texture_Count() > 1);
	bool multi_shad = (context->Shader_Count() > 1);

	if (!multi_mtl) {
		Set_Single_Material(context->Peek_Legacy_Vertex_Material(0));
	}
	if (!multi_tex) {
		Set_Single_Texture(context->Peek_Legacy_Texture(0));
	}
	if (!multi_shad) {
		Set_Single_Shader(context->Peek_Legacy_Shader(0));
	}

	/*
	** Read in each polygon's material id and assign pointer to the
	** shader, texture, and vertex material as needed.
	*/
	for (int i=0; i<Get_Polygon_Count(); i++) {

		// read in the mat id for this poly
		uint16 matid;

		if (cload.Read(&matid,sizeof(uint16)) != sizeof(uint16)) {
			return WW3D_ERROR_LOAD_FAILED;
		}

		if (multi_shad) {
			Set_Shader(i,context->Peek_Legacy_Shader(matid));
		}
		if (multi_tex) {
			Set_Texture(i,context->Peek_Legacy_Texture(matid));
		}
		if (multi_mtl) {
			Set_Material(polys[i].I,context->Peek_Legacy_Vertex_Material(matid));
			Set_Material(polys[i].J,context->Peek_Legacy_Vertex_Material(matid));
			Set_Material(polys[i].K,context->Peek_Legacy_Vertex_Material(matid));
		}
	}

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_vertex_colors -- read in the vertex colors chunk                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/1997 GH  : Created.                                                                 *
 *   2/16/99    GTH : Moved into MeshModelClass                                                *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_vertex_colors(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	/*
	** The W3D file format supports arbitrary vertex color arrays for each pass; however since
	** our conversion to hardware T&L, we only support two unique color arrays.  So here is
	** what is happening in this function:
	**
	** 1 - If this is the first diffuse color array we've encountered, load the values
	** 2 - Always set the DCG source for this pass to the array (vertex materials will be fixed later)
	**
	** A side effect is that if two DCG chunks are encountered, only the first is used...
	*/
	if (CurMatDesc->Has_Color_Array(0) == NULL) {
		W3dRGBStruct color;
		unsigned * dcg = Get_Color_Array(0,true);
		assert(dcg != NULL);

		for (int i=0; i<Get_Vertex_Count(); i++) {

			if (cload.Read(&color,sizeof(W3dRGBStruct)) != sizeof(W3dRGBStruct)) {
				return WW3D_ERROR_LOAD_FAILED;
			}

			Vector4 col;
			col.Set((float)color.R / 255.0f,(float)color.G / 255.0f,(float)color.B / 255.0f, 1.0f);
			dcg[i]=DX8Wrapper::Convert_Color(col);
		}
	}
	CurMatDesc->Set_DCG_Source(context->CurPass,VertexMaterialClass::COLOR1);

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_material_info -- read the material info chunk                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_material_info(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	if (cload.Read(&(context->MatInfo),sizeof(W3dMaterialInfoStruct)) != sizeof(W3dMaterialInfoStruct)) {
		return WW3D_ERROR_LOAD_FAILED;
	}
	Set_Pass_Count(context->MatInfo.PassCount);
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_shaders -- read the shaders chunk                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_shaders(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	W3dShaderStruct shader;
	for (unsigned int i=0; i<context->MatInfo.ShaderCount; i++) {
		if (cload.Read(&shader,sizeof(shader)) != sizeof(shader)) {
			return WW3D_ERROR_LOAD_FAILED;
		}
		ShaderClass newshader;
		W3dUtilityClass::Convert_Shader(shader,&newshader);

		int index = context->Add_Shader(newshader);
		WWASSERT(index == (int)i);
	}
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_vertex_materials -- read the vertex materials chunk                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_vertex_materials(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	while (cload.Open_Chunk()) {
		WWASSERT(cload.Cur_Chunk_ID() == W3D_CHUNK_VERTEX_MATERIAL);
		VertexMaterialClass * vmat = NEW_REF(VertexMaterialClass,());
		WW3DErrorType error = vmat->Load_W3D(cload);
		if (error != WW3D_ERROR_OK) {
			return error;
		}
		context->Add_Vertex_Material(vmat);
		vmat->Release_Ref();

		cload.Close_Chunk();
	}
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_textures -- read the textures chunk                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *   3/05/99	 PDS : Broke the guts of this function into a util function in texture.cpp		  *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_textures(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	// Keep reading textures until there are no more...
	for (TextureClass *newtex = ::Load_Texture (cload);
		  newtex != NULL;
		  newtex = ::Load_Texture (cload)) {

		// Add this texture to our contex and release our local hold on it
		context->Add_Texture(newtex);
		newtex->Release_Ref();
	}

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_material_pass -- read a material pass chunk                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_material_pass(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	context->CurTexStage = 0;

	while (cload.Open_Chunk()) {

		WW3DErrorType error = WW3D_ERROR_OK;

		switch (cload.Cur_Chunk_ID()) {
			case W3D_CHUNK_VERTEX_MATERIAL_IDS:
				error = read_vertex_material_ids(cload,context);
				break;

			case W3D_CHUNK_SHADER_IDS:
				error = read_shader_ids(cload,context);
				break;

			case W3D_CHUNK_DCG:
				error = read_dcg(cload,context);
				break;

			case W3D_CHUNK_DIG:
				error = read_dig(cload,context);
				break;

			case W3D_CHUNK_SCG:
				error = read_scg(cload,context);
				break;

			case W3D_CHUNK_TEXTURE_STAGE:
				error = read_texture_stage(cload,context);
				break;
		};

		if (error != WW3D_ERROR_OK) {
			return error;
		}
		cload.Close_Chunk();
	}

	context->CurPass++;
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_vertex_material_ids -- read the vmat ids for a pass                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *   9/1/2000   gth : Added alternate material desc support                                    *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_vertex_material_ids(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	/*
	** Determine whether this chunk should be read into the default or alternate material description
	*/
	MeshMatDescClass * matdesc = DefMatDesc;
	if (DefMatDesc->Has_Material_Data(context->CurPass)) {
		matdesc = &(context->AlternateMatDesc);
	}

	/*
	** This chunk will either have a single index in it or an array of indices
	** with the length equal to the vertex count.
	*/
	uint32 vmat;
#if (!MESH_SINGLE_MATERIAL_HACK)
	if (cload.Cur_Chunk_Length() == 1*sizeof(uint32)) {

		cload.Read(&vmat,sizeof(uint32));
		matdesc->Set_Single_Material(context->Peek_Vertex_Material(vmat),context->CurPass);

	} else {

		for (int i=0; i<Get_Vertex_Count(); i++) {
			cload.Read(&vmat,sizeof(uint32));
			matdesc->Set_Material(i,context->Peek_Vertex_Material(vmat),context->CurPass);
		}
	}
#else
#pragma message ("(gth) Hacking to make Generals behave as if all meshes have 1 material")
		cload.Read(&vmat,sizeof(uint32));
		matdesc->Set_Single_Material(context->Peek_Vertex_Material(vmat),context->CurPass);
#endif //0

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_shader_ids -- read the shader indexes for a pass                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *   9/1/2000   gth : Added alternate material desc support                                    *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_shader_ids(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	/*
	** Determine whether this chunk should be read into the default or alternate material description
	*/
	MeshMatDescClass * matdesc = DefMatDesc;
	if (DefMatDesc->Has_Shader_Data(context->CurPass)) {
		matdesc = &(context->AlternateMatDesc);
	}

	/*
	** Read in the shader id's and plug in the appropriate shader
	*/
	uint32 shaderid;
#if (!MESH_SINGLE_MATERIAL_HACK)
	if (cload.Cur_Chunk_Length() == 1*sizeof(uint32)) {

		cload.Read(&shaderid,sizeof(shaderid));
		ShaderClass shader = context->Peek_Shader(shaderid);
		matdesc->Set_Single_Shader(shader,context->CurPass);

		// turn on sorting of pass 0 has non-zero dest blend (unless alpha testing on)
		if (	(context->CurPass == 0) &&
				(shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO) &&
				(shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_DISABLE) &&
				(SortLevel == SORT_LEVEL_NONE) )
		{
#if (MESH_FORCE_STATIC_SORT_HACK)
			SortLevel = 1;
#else
			Set_Flag(SORT,true);
#endif
		}

	} else {

		for (int i=0; i<Get_Polygon_Count(); i++) {
			cload.Read(&shaderid,sizeof(uint32));
			ShaderClass shader = context->Peek_Shader(shaderid);
			matdesc->Set_Shader(i,shader,context->CurPass);

			// turn on sorting of pass 0 has non-zero dest blend (unless alpha testing on)
			if (	(context->CurPass == 0) &&
				(shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO) &&
				(shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_DISABLE) &&
				(SortLevel == SORT_LEVEL_NONE) )
			{
#if (MESH_FORCE_STATIC_SORT_HACK)
				SortLevel = 1;
#else
				Set_Flag(SORT,true);
#endif
			}
		}
	}
#else
#pragma message ("(gth) Hacking to make Generals behave as if all meshes have 1 material")

		cload.Read(&shaderid,sizeof(shaderid));
		ShaderClass shader = context->Peek_Shader(shaderid);
		matdesc->Set_Single_Shader(shader,context->CurPass);

		// turn on sorting of pass 0 has non-zero dest blend (unless alpha testing on)
		if (	(context->CurPass == 0) &&
				(shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO) &&
				(shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_DISABLE) &&
				(SortLevel == SORT_LEVEL_NONE) )
		{
#if (MESH_FORCE_STATIC_SORT_HACK)
			SortLevel = 1;
#else
			Set_Flag(SORT,true);
#endif
		}

#endif //0
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_dcg -- read the per-vertex diffuse color for a pass                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *   9/1/2000   gth : Added alternate material desc support                                    *
 *   2/9/2001   gth : converted to handle dx8 limitations                                      *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_dcg(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	/*
	** Determine whether this chunk should be read into the default or alternate material description
	*/
	MeshMatDescClass * matdesc = DefMatDesc;
	if (DefMatDesc->Get_DCG_Source(context->CurPass) != VertexMaterialClass::MATERIAL) {
		matdesc = &(context->AlternateMatDesc);
	}

	/*
	** The W3D file format supports arbitrary vertex color arrays for each pass; however since
	** our conversion to hardware T&L, we only support two unique color arrays.  So here is
	** what is happening in this function:
	**
	** 1 - If this is the first diffuse color array we've encountered, load the values.
	** 2 - Otherwise, if we are in PRELIT_VERTEX mode, put the alpha from this array into the color array.
	** 3 - Always set the DCG source for this pass to the array.
	**
	** Our tools *currently* will only generate two color arrays in the case where one of them
	** is being used for alpha and the other is used for precomputed vertex lighting...  This will
	** break if our tools change.  The file format isn't restricting you from defining something
	** we can't render right now...
	*/
	if (matdesc->Has_Color_Array(0) == false) {
		W3dRGBAStruct color;
		unsigned * dcg = matdesc->Get_Color_Array(0);

		for (int i=0; i<Get_Vertex_Count(); i++) {
			cload.Read(&color,sizeof(color));
			Vector4 col;
			W3dUtilityClass::Convert_Color(color,&col);
			dcg[i]=DX8Wrapper::Convert_Color(col);
		}
	} else if (context->PrelitChunkID==W3D_CHUNK_PRELIT_VERTEX) {

		W3dRGBAStruct color;
		unsigned * dcg = matdesc->Get_Color_Array(0);

		for (int i=0; i<Get_Vertex_Count(); i++) {
			cload.Read(&color,sizeof(color));
			Vector4 col;
			col=DX8Wrapper::Convert_Color(dcg[i]);
			col.W = float(color.A)/255.0f;
			dcg[i]=DX8Wrapper::Convert_Color(col);
		}
	}

	matdesc->Set_DCG_Source(context->CurPass,VertexMaterialClass::COLOR1);

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_dig -- read the per-vertex diffuse illumination for a pass             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *   9/1/2000   gth : Added alternate material desc support                                    *
 *   2/9/2001   gth : converted to handle dx8 limitations                                      *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_dig(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	/*
	** Determine whether this chunk should be read into the default or alternate material description
	*/
	MeshMatDescClass * matdesc = DefMatDesc;
	if (context->Already_Loaded_DIG()) {
		matdesc = &(context->AlternateMatDesc);
	}
	context->Notify_Loaded_DIG_Chunk(true);

	/*
	** It appears that there isn't wide support for having the emissive material color source
	** be a vertex color array so when there is a pre-existing color array, I'm just multiplying
	** the DIG values into it.
	*/
	W3dRGBAStruct color;
	if (matdesc->Has_Color_Array(0) == false) {
		unsigned * dcg = matdesc->Get_Color_Array(0);
		for (int i=0; i<Get_Vertex_Count(); i++) {
			cload.Read(&color,sizeof(color));
			Vector4 col;
			col.X = float(color.R)/255.0f;
			col.Y = float(color.G)/255.0f;
			col.Z = float(color.B)/255.0f;
			col.W = 1.0f;
			dcg[i]=DX8Wrapper::Convert_Color(col);


		}
	} else {
		unsigned * dcg = matdesc->Get_Color_Array(0);
		for (int i=0; i<Get_Vertex_Count(); i++) {
			cload.Read(&color,sizeof(color));
			Vector4 col=DX8Wrapper::Convert_Color(dcg[i]);
			col.X *= float(color.R)/255.0f;
			col.Y *= float(color.G)/255.0f;
			col.Z *= float(color.B)/255.0f;
			dcg[i]=DX8Wrapper::Convert_Color(col);
		}
	}

	matdesc->Set_DCG_Source(context->CurPass,VertexMaterialClass::COLOR1);

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_scg -- read the specular color for a pass                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *   9/1/2000   gth : Added alternate material desc support                                    *
 *   2/9/2001   gth : new dx8 code no longer supports this chunk                               *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_scg(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_texture_stage -- read texture stage chunks                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_texture_stage(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	while (cload.Open_Chunk()) {

		WW3DErrorType error = WW3D_ERROR_OK;

		switch(cload.Cur_Chunk_ID()) {
			case W3D_CHUNK_TEXTURE_IDS:
				error = read_texture_ids(cload,context);
				break;

			case W3D_CHUNK_STAGE_TEXCOORDS:
			case W3D_CHUNK_TEXCOORDS:
				error = read_stage_texcoords(cload,context);
				break;

			case W3D_CHUNK_PER_FACE_TEXCOORD_IDS:
				error = read_per_face_texcoord_ids (cload, context);
				break;

		}

		if (error != WW3D_ERROR_OK) {
			return error;
		}

		cload.Close_Chunk();
	}

	context->CurTexStage++;
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_texture_ids -- read the texture ids for a pass,stage                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *   9/1/2000   gth : Added alternate material desc support                                    *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_texture_ids(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	uint32 texid;
	int pass = context->CurPass;
	int stage = context->CurTexStage;

	/*
	** Determine whether this chunk should be read into the default or alternate material description
	*/
	MeshMatDescClass * matdesc = DefMatDesc;
	if (DefMatDesc->Has_Texture_Data(pass,stage)) {
		matdesc = &(context->AlternateMatDesc);
	}

	/*
	** Read in the texture(s) array
	*/
#if (!MESH_SINGLE_MATERIAL_HACK)
	if (cload.Cur_Chunk_Length() == 1*sizeof(uint32)) {
		cload.Read(&texid,sizeof(texid));
		matdesc->Set_Single_Texture(context->Peek_Texture(texid),pass,stage);

	} else {

		for (int i=0; i<Get_Polygon_Count(); i++) {
			cload.Read(&texid,sizeof(uint32));
			if (texid != 0xffffffff) {
				matdesc->Set_Texture(i,context->Peek_Texture(texid),pass,stage);
			}
		}
	}
#else
#pragma message ("(gth) Hacking to make Generals behave as if all meshes have 1 material")

		cload.Read(&texid,sizeof(texid));
		matdesc->Set_Single_Texture(context->Peek_Texture(texid),pass,stage);

#endif

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * MeshModelClass::read_stage_texcoords -- read the texcoords for a pass,stage                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *   7/14/99    IML : Lightmap support: calculate vertex count directly from chunk size.		  *
 *   9/1/2000   gth : Added alternate material desc support                                    *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_stage_texcoords(ChunkLoadClass & cload,MeshLoadContextClass * context)
{
	unsigned				elementcount;
	Vector2			  *uvs;
	W3dTexCoordStruct texcoord;

	/*
	** Determine whether this chunk should be read into the default or alternate material description
	*/
	MeshMatDescClass * matdesc = DefMatDesc;
	if (DefMatDesc->Has_UV(context->CurPass,context->CurTexStage)) {
		matdesc = &(context->AlternateMatDesc);
	}

	/*
	** Read in the texture coordiantes
	*/
	elementcount = cload.Cur_Chunk_Length() / sizeof (W3dTexCoordStruct);
	uvs = context->Get_Temporary_UV_Array(elementcount);

	if (uvs != NULL) {
		for (unsigned i = 0; i < elementcount; i++) {
			cload.Read (&texcoord, sizeof (texcoord));
			uvs[i].X = texcoord.U;
			uvs[i].Y = 1.0f - texcoord.V;
		}
	}

	matdesc->Install_UV_Array(context->CurPass,context->CurTexStage,uvs,elementcount);

	return (WW3D_ERROR_OK);
}


/***********************************************************************************************
 * MeshModelClass::read_per_face_texcoord_ids -- read uv indices for given (pass,stage).		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/02/99    IML : Created.                                                                *
 *   9/1/2000   gth : Added alternate material desc support                                    *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_per_face_texcoord_ids (ChunkLoadClass &cload, MeshLoadContextClass *context)
{
	unsigned size;

	/*
	** Determine whether this chunk should be read into the default or alternate material description
	*/
//	MeshMatDescClass * matdesc = DefMatDesc;
//	if (DefMatDesc->Has_UVIndex(context->CurPass)) {
//		matdesc = &(context->AlternateMatDesc);
//	}

	/*
	** Read in the texture coordiante indices
	** There must be polygon count vectors in this chunk.
	*/
	size = sizeof (Vector3i) * Get_Polygon_Count();
	if (cload.Cur_Chunk_Length() == size) {

//		Vector3i *uvindices;
//
//		uvindices = matdesc->Get_UVIndex_Array (context->CurPass, true);
//		WWASSERT (uvindices != NULL);

//uvindices=W3DNEWARRAY Vector3i[Get_Polygon_Count()];
//		cload.Read (uvindices, size);
//delete[] uvindices;
		cload.Seek(size);
		return (WW3D_ERROR_OK);

	} else {
		return (WW3D_ERROR_LOAD_FAILED);
	}
}


/***********************************************************************************************
 * MeshModelClass::read_prelit_material -- read prelit material chunks.								  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/02/99    IML : Created.                                                                *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::read_prelit_material (ChunkLoadClass &cload, MeshLoadContextClass *context)
{
	// If this chunk ID matches the selected prelit chunk ID then load it, otherwise skip it.
	if (cload.Cur_Chunk_ID() == context->PrelitChunkID) {

		// While there are chunks in the prelit material chunk wrapper...
		while (cload.Open_Chunk()) {

			WW3DErrorType error = WW3D_ERROR_OK;

			switch (cload.Cur_Chunk_ID()) {

				case W3D_CHUNK_MATERIAL_INFO:
					error = read_material_info (cload,context);
					break;

				case W3D_CHUNK_VERTEX_MATERIALS:
					error = read_vertex_materials (cload,context);
					break;

				case W3D_CHUNK_SHADERS:
					error = read_shaders (cload,context);
					break;

				case W3D_CHUNK_TEXTURES:
					error = read_textures (cload,context);
					break;

				case W3D_CHUNK_MATERIAL_PASS:
					error = read_material_pass (cload,context);
					break;

				default:

					// Unknown chunk.
					break;
			}
			cload.Close_Chunk();
			if (error != WW3D_ERROR_OK) return (error);
		}
	}

	return (WW3D_ERROR_OK);
}


/***********************************************************************************************
 * MeshModelClass::post_process -- post loading, perform any processing on this model.			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/02/00   IML : Created.                                                                 *
 *   7/13/2001  hy : Added static sort postprocessing                                          *
 *=============================================================================================*/
void MeshModelClass::post_process()
{
#if 0
	// we want to allow this now due to usage of the static sort
	// Ensure no sorting, multipass meshes (for they are abomination...)
	if (DefMatDesc->Get_Pass_Count() > 1 && Get_Flag(SORT)) {
		WWDEBUG_SAY(( "Turning SORT off for multipass mesh %s",Get_Name() ));
		Set_Flag(SORT, false);
	}
#endif

	// skinned meshes should not have cull trees
	if (Get_Flag(MeshGeometryClass::SKIN)) {
		if (CullTree) {
			REF_PTR_RELEASE(CullTree);
		}
	}

	// turn off backface culling if the mesh is supposed to be two-sided
	if (Get_Flag(MeshGeometryClass::TWO_SIDED)) {

		DefMatDesc->Disable_Backface_Culling();
		if (AlternateMatDesc != NULL) {
			AlternateMatDesc->Disable_Backface_Culling();
		}

	}

	// fog activation.
	if (WW3DAssetManager::Get_Instance()->Get_Activate_Fog_On_Load()) {
		post_process_fog();
	}

	// if the mesh is sorting, pick an appropriate static sort level
	// if default isn't set
	if (Get_Flag(SORT) && SortLevel==SORT_LEVEL_NONE && WW3D::Is_Munge_Sort_On_Load_Enabled()) {
		compute_static_sort_levels();
	}

	// If we need to, modify the mesh model to support overbrightening (change all
	// GRADIENT_MODULATE to GRADIENT_MODULATE2X)
	if (WW3D::Is_Overbright_Modify_On_Load_Enabled()) {
		modify_for_overbright();
	}
}

void MeshModelClass::post_process_fog(void)
{
	// If two pass...
	if (DefMatDesc->Get_Pass_Count() == 2) {

		// If single shader on both passes...
		if (!DefMatDesc->ShaderArray[0] && !DefMatDesc->ShaderArray[1]) {

			ShaderClass &shader0 = DefMatDesc->Shader [0];
			ShaderClass &shader1 = DefMatDesc->Shader [1];

			// Analyze the mesh to determine if it is the emissive map effect and if it is, fix it up appropriately.
			bool emissive_map_effect = DefMatDesc->PassCount == 2 &&
												shader0.Get_Texturing() == ShaderClass::TEXTURING_DISABLE &&
												shader0.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_ONE &&
												shader0.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ZERO &&
												shader0.Get_Primary_Gradient() == ShaderClass::GRADIENT_MODULATE &&
												shader0.Get_Secondary_Gradient() == ShaderClass::SECONDARY_GRADIENT_DISABLE &&
												shader1.Get_Texturing() == ShaderClass::TEXTURING_ENABLE &&
												shader1.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_SRC_ALPHA &&
												shader1.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_SRC_COLOR;

			if (emissive_map_effect) {

				// Change the shader/texture setting into an equivalent one which will enable setting fog
				// correctly: Note that we are setting up pass 0 to have a texture now.
				shader0.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
				shader1.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE);
				shader0.Set_Fog_Func(ShaderClass::FOG_ENABLE);
				shader1.Set_Fog_Func(ShaderClass::FOG_SCALE_FRAGMENT);

				// Copy pass 1 texture/texture array to pass 0.
				REF_PTR_SET (DefMatDesc->Texture[0][0], DefMatDesc->Texture [1][0]);
				if (DefMatDesc->TextureArray [1][0]) {
					if (!DefMatDesc->TextureArray [0][0]) {
						DefMatDesc->TextureArray [0][0] = NEW_REF (TexBufferClass, (PolyCount, "MeshModelClass::DefMatDesc::TextureArray"));
						for (int i = 0; i < PolyCount; i++) {
							DefMatDesc->TextureArray [0][0]->Set_Element (i, DefMatDesc->TextureArray [1][0]->Peek_Element (i));
						}
					}
				}

				// Make pass 0 point to the same UV array as pass 1. If pass 1 has a vertex material
				// array, we only take the first one for determining UV source. The UV source is
				// used to set the UV source of all the vertex materials in pass 0.
				int uv_source = 0;
				if (DefMatDesc->MaterialArray[1]) {
					uv_source = DefMatDesc->MaterialArray[1]->Peek_Element(0)->Get_UV_Source(0);
				} else {
					DefMatDesc->Material[1]->Get_UV_Source(0);
				}
				if (DefMatDesc->MaterialArray[0]) {
					for (int i = 0; i < VertexCount; i++) {
						DefMatDesc->MaterialArray[0]->Peek_Element(i)->Set_UV_Source(0, uv_source);
					}
				} else {
					DefMatDesc->Material[0]->Set_UV_Source(0, uv_source);
				}

				return;
			}

			// Analyze the mesh to determine if it is the shiny mask effect and if it is, fix it up appropriately.
			bool shiny_mask_effect = shader0.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_ONE &&
											 shader0.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ZERO &&
											 shader1.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_ONE &&
											(shader1.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_SRC_ALPHA ||
											 shader1.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA);

			if (shiny_mask_effect) {
				shader0.Set_Fog_Func(ShaderClass::FOG_SCALE_FRAGMENT);
				shader1.Set_Fog_Func(ShaderClass::FOG_ENABLE);
				return;
			}
		}
	}

	// Mesh is not one of the special two-pass combinations. Apply a per-pass generic fix-up.
	for (int pass = 0; pass < DefMatDesc->PassCount; pass++) {
		DefMatDesc->Shader [pass].Enable_Fog (Get_Name());
		if (DefMatDesc->ShaderArray [pass]) {
			for (int tri = 0; tri < DefMatDesc->ShaderArray [pass]->Get_Count(); tri++) {
				DefMatDesc->ShaderArray [pass]->Get_Element (tri).Enable_Fog (Get_Name());
			}
		}
	}
}

unsigned int MeshModelClass::get_sort_flags(int pass) const
{
	unsigned int flags = 0;
	ShaderClass::StaticSortCategoryType scat;
	if (Has_Shader_Array(pass)) {
		for (int tri = 0; tri < CurMatDesc->ShaderArray[pass]->Get_Count(); tri++) {
			scat = CurMatDesc->ShaderArray[pass]->Get_Element(tri).Get_SS_Category();
			flags |= (1 << scat);
		}
	} else {
		scat = Get_Single_Shader(pass).Get_SS_Category();
		flags |= (1 << scat);
	}
	return flags;
}

unsigned int MeshModelClass::get_sort_flags(void) const
{
	unsigned int flags = 0;
	for (int pass = 0; pass < Get_Pass_Count(); pass++) {
		flags |= get_sort_flags(pass);
	}
	return flags;
}

void MeshModelClass::compute_static_sort_levels(void)
{
	enum StaticSortCategoryBitFieldType
	{
		SSCAT_OPAQUE_BF		= (1 << ShaderClass::SSCAT_OPAQUE),
		SSCAT_ALPHA_TEST_BF	= (1 << ShaderClass::SSCAT_ALPHA_TEST),
		SSCAT_ADDITIVE_BF		= (1 << ShaderClass::SSCAT_ADDITIVE),
		SSCAT_SCREEN_BF		= (1 << ShaderClass::SSCAT_SCREEN),
		SSCAT_OTHER_BF			= (1 << ShaderClass::SSCAT_OTHER)
	};

	if (get_sort_flags(0) == SSCAT_OPAQUE_BF) {
		SortLevel = SORT_LEVEL_NONE;
		return;
	}

	switch (get_sort_flags())
	{
	case (SSCAT_OPAQUE_BF | SSCAT_ALPHA_TEST_BF):
		SortLevel = SORT_LEVEL_NONE;
		break;

	case SSCAT_ADDITIVE_BF:
		SortLevel = SORT_LEVEL_BIN3;
		break;

	case SSCAT_SCREEN_BF:
		SortLevel = SORT_LEVEL_BIN2;
		break;

	default:
		SortLevel = SORT_LEVEL_BIN1;
		break;
	};
}

void MeshModelClass::modify_for_overbright(void)
{
	// Iterate over all passes
	int pass_cnt = Get_Pass_Count();
	for (int pass_idx = 0; pass_idx < pass_cnt; pass_idx++) {

		// First do single shader
		ShaderClass shader = Get_Single_Shader(pass_idx);
		if (shader.Get_Primary_Gradient() == ShaderClass::GRADIENT_MODULATE) {
			shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE2X);
			Set_Single_Shader(shader, pass_idx);
		}

		// Now do all other shaders
		if (Has_Shader_Array(pass_idx)) {
			int p_cnt = Get_Polygon_Count();
			for (int p_idx = 0; p_idx < p_cnt; p_idx++) {
				shader = Get_Shader(p_idx, pass_idx);
				if (shader.Get_Primary_Gradient() == ShaderClass::GRADIENT_MODULATE) {
					shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE2X);
					Set_Shader(p_idx, shader, pass_idx);
				}
			}
		}
	}

}

void MeshModelClass::install_materials(MeshLoadContextClass * context)
{
	int i;

	/*
	** If alternate material chunks were loaded, initialize the AlternateMatDesc
	*/
	install_alternate_material_desc(context);

	/*
	** Finish configuring the vertex materials and color arrays.
	*/
	bool lighting_enabled=true;
	// vertex-lit models need the lighting turned off!
	if (Get_Flag(MeshGeometryClass::PRELIT_VERTEX)) {
		lighting_enabled=false;
	}
	DefMatDesc->Post_Load_Process (lighting_enabled,this);
	if (AlternateMatDesc != NULL) {
		AlternateMatDesc->Post_Load_Process (lighting_enabled,this);
	}

	/*
	** transfer the refs to our textures into the MatInfo
	*/
	for (i=0; i<context->Texture_Count(); i++) {
		MatInfo->Add_Texture(context->Peek_Texture(i));
	}

	/*
	** transfer the refs to our vertex materials into the MatInfo
	*/
	for (i=0; i<context->Vertex_Material_Count(); i++) {
		MatInfo->Add_Vertex_Material(context->Peek_Vertex_Material(i));
	}
}


void MeshModelClass::clone_materials(const MeshModelClass & srcmesh)
{
	/*
	** Copy the material info and the materials within
	*/
	REF_PTR_RELEASE(MatInfo);
	MatInfo = NEW_REF( MaterialInfoClass,(*(srcmesh.MatInfo)));

	/*
	** remap!
	*/
	MaterialRemapperClass remapper(srcmesh.MatInfo, MatInfo);
	remapper.Remap_Mesh(srcmesh.CurMatDesc, CurMatDesc);
}


void MeshModelClass::install_alternate_material_desc(MeshLoadContextClass * context)
{
	if (context->AlternateMatDesc.Is_Empty() == false) {
		WWASSERT(AlternateMatDesc == NULL);
		AlternateMatDesc = W3DNEW MeshMatDescClass;
		AlternateMatDesc->Init_Alternate(*DefMatDesc,context->AlternateMatDesc);
	}
}

/***********************************************************************************************
 * MeshLoadContextClass::MeshLoadContextClass -- constructor for MeshLoadContextClass          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
MeshLoadContextClass::MeshLoadContextClass(void)
{
	memset(&Header,0,sizeof(Header));
	memset(&MatInfo,0,sizeof(MatInfo));
	PrelitChunkID = 0xffffffff;
	CurPass = 0;
	CurTexStage = 0;
	TexCoords = NULL;
	LoadedDIG = false;
}


/***********************************************************************************************
 * MeshLoadContextClass::~MeshLoadContextClass -- destructor                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
MeshLoadContextClass::~MeshLoadContextClass(void)
{
	int i;

	delete TexCoords;
	TexCoords = NULL;

	for (i=0; i<Textures.Count(); i++) {
		Textures[i]->Release_Ref();
	}
	for (i=0; i<VertexMaterials.Count(); i++) {
		VertexMaterials[i]->Release_Ref();
	}
	for (i=0; i<LegacyMaterials.Count(); i++) {
		delete LegacyMaterials[i];
	}
}


/***********************************************************************************************
 * MeshLoadContextClass::Get_Texcoord_Array -- returns the texture coordinates array           *
 *                                                                                             *
 * This function mainly exists to support the obsolete version 3.0 w3d files                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
W3dTexCoordStruct * MeshLoadContextClass::Get_Texcoord_Array(void)
{
	if (TexCoords == NULL) {
		TexCoords = W3DNEWARRAY W3dTexCoordStruct[Header.NumVertices];
	}
	return TexCoords;
}


/***********************************************************************************************
 * MeshLoadContextClass::Add_Shader -- adds a shader to the array                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
int MeshLoadContextClass::Add_Shader(ShaderClass shader)
{
	int index = Shaders.Count();
	Shaders.Add(shader);
	return index;
}


/***********************************************************************************************
 * MeshLoadContextClass::Add_Vertex_Materail -- adds a vertex material                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
int MeshLoadContextClass::Add_Vertex_Material(VertexMaterialClass * vmat)
{
	WWASSERT(vmat != NULL);
	vmat->Add_Ref();
	int index = VertexMaterials.Count();
	VertexMaterials.Add(vmat);
	return index;
}


/***********************************************************************************************
 * MeshLoadContextClass::Add_Texture -- adds a texture                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
int MeshLoadContextClass::Add_Texture(TextureClass * tex)
{
	WWASSERT(tex != NULL);
	tex->Add_Ref();
	int index = Textures.Count();
	Textures.Add(tex);
	return index;
}


/***********************************************************************************************
 * MeshLoadContextClass::Add_Legacy_Material -- adds a legacy material                         *
 *                                                                                             *
 * This function will check to see if the parameters of any of the parts of the material are   *
 * duplicated.  Only unique shaders/vertexmaterials will be actually added.                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void MeshLoadContextClass::Add_Legacy_Material(ShaderClass shader,VertexMaterialClass * vmat,TextureClass * tex)
{
	// create a new legacy material
	LegacyMaterialClass * mat = W3DNEW LegacyMaterialClass;

	// add the shader if it is unique
	int si=0;
	for (; si<Shaders.Count(); si++) {
		if (Shaders[si] == shader) break;
	}
	if (si == Shaders.Count()) {
		mat->ShaderIdx = Add_Shader(shader);
	} else {
		mat->ShaderIdx = si;
	}

	// add the vertex material if it is unique
	if (vmat == NULL) {
		mat->VertexMaterialIdx = -1;
	} else {
		unsigned long crc = vmat->Get_CRC();
		int vi=0;
		for (; vi<VertexMaterialCrcs.Count(); vi++) {
			if (VertexMaterialCrcs[vi] == crc) break;
		}
		if (vi == VertexMaterials.Count()) {
			mat->VertexMaterialIdx = Add_Vertex_Material(vmat);
			VertexMaterialCrcs.Add(crc);
			WWASSERT(VertexMaterialCrcs.Count() == VertexMaterials.Count());
		} else {
			mat->VertexMaterialIdx = vi;
		}
	}

	// add the texture if it is unique
	if (tex == NULL) {
		mat->TextureIdx = -1;
	} else {
		int ti=0;
		for (; ti<Textures.Count(); ti++) {
			if (Textures[ti] == tex) break;
			if (stricmp(Textures[ti]->Get_Texture_Name(),tex->Get_Texture_Name()) == 0) break;
		}
		if (ti == Textures.Count()) {
			mat->TextureIdx = Add_Texture(tex);
		} else {
			mat->TextureIdx = ti;
		}
	}

	LegacyMaterials.Add(mat);
}


/***********************************************************************************************
 * MeshLoadContextClass::Peek_Legacy_Shader -- returns a legacy shader								  *
 *                                                                                             *
 * This function does the re-indexing to go from the legacy shader index to the actual shader  *
 * index and then returns that shader                                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
ShaderClass MeshLoadContextClass::Peek_Legacy_Shader(int legacy_material_index)
{
	WWASSERT(legacy_material_index >= 0);
	WWASSERT(legacy_material_index < LegacyMaterials.Count());
	int si = LegacyMaterials[legacy_material_index]->ShaderIdx;
	return Peek_Shader(si);
}


/***********************************************************************************************
 * MeshLoadContextClass::Peek_Legacy_Vertex_Material -- returns a pointer to a legacy vertex ma*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
VertexMaterialClass * MeshLoadContextClass::Peek_Legacy_Vertex_Material(int legacy_material_index)
{
	WWASSERT(legacy_material_index >= 0);
	WWASSERT(legacy_material_index < LegacyMaterials.Count());
	int vi = LegacyMaterials[legacy_material_index]->VertexMaterialIdx;
	if (vi != -1) {
		return Peek_Vertex_Material(vi);
	} else {
		return NULL;
	}
}


/***********************************************************************************************
 * MeshLoadContextClass::Peek_Legacy_Texture -- returns a pointer to a texture                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/98   GTH : Created.                                                                 *
 *=============================================================================================*/
TextureClass * MeshLoadContextClass::Peek_Legacy_Texture(int legacy_material_index)
{
	WWASSERT(legacy_material_index >= 0);
	WWASSERT(legacy_material_index < LegacyMaterials.Count());
	int ti = LegacyMaterials[legacy_material_index]->TextureIdx;
	if (ti != -1) {
		return Peek_Texture(ti);
	} else {
		return NULL;
	}
}


Vector2 * MeshLoadContextClass::Get_Temporary_UV_Array(int elementcount)
{
	TempUVArray.Uninitialised_Grow(elementcount);
	return &(TempUVArray[0]);
}



/***********************************************************************************************
 * MeshSaveContextClass::MeshSaveContextClass -- constructor                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
MeshSaveContextClass::MeshSaveContextClass(void) :
	CurPass(0),
	CurStage(0)
{
}


/***********************************************************************************************
 * MeshSaveContextClass::~MeshSaveContextClass -- destructor                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
MeshSaveContextClass::~MeshSaveContextClass(void)
{
}





#if 0 // MESH SAVING CODE HAS NOT BEEN MAINTAINED... Leaving here for future reference if we ever need it :-)

/***********************************************************************************************
 * MeshModelClass::Save -- Save this mesh model!                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/16/99    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType MeshModelClass::Save_W3D(ChunkSaveClass & csave)
{
	MeshSaveContextClass * context = W3DNEW MeshSaveContextClass;
	context->Materials.Collect_Materials(this);

	write_chunks(csave,context);

	delete context;
	return WW3D_ERROR_OK;
}


WW3DErrorType MeshModelClass::write_chunks(ChunkSaveClass & csave,MeshSaveContextClass * context)
{

	// write the header
	write_header(csave,context);
	write_user_text(csave,context);

	// write the geometry
	write_triangles(csave,context);
	write_vertices(csave,context);
	write_vertex_normals(csave,context);
	write_vertex_shade_indices(csave,context);
	write_vertex_influences(csave,context);
	//write_cull_tree(csave);

	// material stuff
	write_material_info(csave,context);
	write_vertex_materials(csave,context);
	write_shaders(csave,context);
	write_textures(csave,context);

	// passes
	for (int i=0; i<Get_Pass_Count(); i++) {
		context->CurPass = i;
		write_material_pass(csave,context);
	}
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_header(ChunkSaveClass & csave,MeshSaveContextClass * /*context*/)
{
	W3dMeshHeader3Struct header;
	memset(&header,0,sizeof(header));

	// Set names
	if (MeshName) {

		char * name = MeshName->Get_Array();
		char * mesh_name = strchr(name,'.');

		int hierarchy_name_len = 0;
		if (mesh_name == NULL) {
			mesh_name = name;
		} else {
			hierarchy_name_len = (int)mesh_name - (int)name;
			mesh_name++;
		}
		assert( hierarchy_name_len < W3D_NAME_LEN);
		strlcpy( header.MeshName, mesh_name, W3D_NAME_LEN);
		strlcpy( header.ContainerName, name, hierarchy_name_len + 1);
	} else {
		sprintf(header.MeshName,"UnNamed");
	}

	header.Version = W3D_CURRENT_MESH_VERSION;
	header.Attributes = W3dAttributes;
	header.NumTris = Get_Polygon_Count();
	header.NumVertices = Get_Vertex_Count();
	header.NumMaterials = 0;
	header.NumDamageStages = 0;

	header.VertexChannels = W3D_VERTEX_CHANNEL_LOCATION;
	if (Get_Flag(SKIN)) {
		header.VertexChannels |= W3D_VERTEX_CHANNEL_BONEID;
	}

	header.FaceChannels = W3D_FACE_CHANNEL_FACE;

	W3dUtilityClass::Convert_Vector(BoundBoxMin,&(header.Min));
	W3dUtilityClass::Convert_Vector(BoundBoxMax,&(header.Max));
	W3dUtilityClass::Convert_Vector(BoundSphereCenter,&header.SphCenter);
	header.SphRadius = BoundSphereRadius;

	csave.Begin_Chunk(W3D_CHUNK_MESH_HEADER3);
	if (csave.Write(&header,sizeof(header)) != sizeof(header)) {
		return WW3D_ERROR_SAVE_FAILED;
	}
	csave.End_Chunk();

	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_user_text(ChunkSaveClass & csave,MeshSaveContextClass * /*context*/)
{
	if (UserText == NULL) return WW3D_ERROR_OK;
	if (strlen(UserText->Get_Array()) < 1) return WW3D_ERROR_OK;

	csave.Begin_Chunk(W3D_CHUNK_MESH_USER_TEXT);
	csave.Write(UserText->Get_Array(),strlen(UserText->Get_Array()) + 1);
	csave.End_Chunk();

	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_triangles(ChunkSaveClass & csave,MeshSaveContextClass * /*context*/)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_TRIANGLES)) {
		return WW3D_ERROR_LOAD_FAILED;
	}

	TriIndex	* poly_verts = Poly->Get_Array();
	Vector4 * poly_eq = (PlaneEq ? PlaneEq->Get_Array() : NULL);

	for (int i=0; i<Get_Polygon_Count(); i++) {

		W3dTriStruct tri;
		memset(&tri,0,sizeof(W3dTriStruct));

		// convert each triangle into surrender format
		tri.Vindex[0] =			poly_verts[i].I;
		tri.Vindex[1] = 			poly_verts[i].J;
		tri.Vindex[2] = 			poly_verts[i].K;

		if (poly_eq)  {
			tri.Attributes =		0;
			tri.Normal.X = 		poly_eq[i].X;
			tri.Normal.Y = 		poly_eq[i].Y;
			tri.Normal.Z = 		poly_eq[i].Z;
			tri.Dist =				poly_eq[i].W;
		} else {

			Vector3 a,b,normal;
			Vector3 * verts = Vertex->Get_Array();
			const Vector3 & p0= verts[poly_verts[i][0]];

			Vector3::Subtract(verts[poly_verts[i][1]],p0,&a);
			Vector3::Subtract(verts[poly_verts[i][2]],p0,&b);
			Vector3::Cross_Product(a,b,&normal);
			normal.Normalize();

			tri.Attributes =		0;
			tri.Normal.X =			normal.X;
			tri.Normal.Y =			normal.Y;
			tri.Normal.Z =			normal.Z;
			tri.Dist =				Vector3::Dot_Product(p0,normal);
		}

		if (csave.Write(&tri,sizeof(W3dTriStruct)) != sizeof(W3dTriStruct)) {
			return WW3D_ERROR_SAVE_FAILED;
		}
	}

	if (!csave.End_Chunk()) {
		return WW3D_ERROR_SAVE_FAILED;
	}

	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_vertices(ChunkSaveClass & csave,MeshSaveContextClass * /*context*/)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_VERTICES)) {
		return WW3D_ERROR_SAVE_FAILED;
	}

	WWASSERT(Get_Vertex_Count() > 0);
	Vector3 * verts = Vertex->Get_Array();

	for (int i=0; i<Get_Vertex_Count(); i++) {

		W3dVectorStruct vert;
   	vert.X = verts[i].X;
		vert.Y = verts[i].Y;
		vert.Z = verts[i].Z;

		if (csave.Write(&(vert),sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct)) {
			return WW3D_ERROR_SAVE_FAILED;
		}
	}

	if (!csave.End_Chunk()) {
		return WW3D_ERROR_SAVE_FAILED;
	}
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_vertex_normals(ChunkSaveClass & csave,MeshSaveContextClass * /*context*/)
{
	WWASSERT( Get_Vertex_Count() > 0);
	if (!csave.Begin_Chunk(W3D_CHUNK_VERTEX_NORMALS)) {
		return WW3D_ERROR_SAVE_FAILED;
	}

	const Vector3 * verts = Get_Vertex_Normal_Array();

	for (int i=0; i<Get_Vertex_Count(); i++) {

		W3dVectorStruct vert;
   	vert.X = verts[i].X;
		vert.Y = verts[i].Y;
		vert.Z = verts[i].Z;

		if (csave.Write(&(vert),sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct)) {
			return WW3D_ERROR_SAVE_FAILED;
		}
	}

	if (!csave.End_Chunk()) {
		return WW3D_ERROR_SAVE_FAILED;
	}

	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_vertex_shade_indices(ChunkSaveClass & csave,MeshSaveContextClass * /*context*/)
{
	WWASSERT(Get_Vertex_Count() > 0);
	if (VertexShadeIdx == NULL) return WW3D_ERROR_OK;

	if (!csave.Begin_Chunk(W3D_CHUNK_VERTEX_SHADE_INDICES)) {
		return WW3D_ERROR_SAVE_FAILED;
	}

	for (int i=0; i<Get_Vertex_Count(); i++) {
		uint32 idx = VertexShadeIdx->Get_Array()[i];
		if (csave.Write(&idx,sizeof(idx)) != sizeof(idx)) {
			return WW3D_ERROR_SAVE_FAILED;
		}
	}

	if (!csave.End_Chunk()) {
		return WW3D_ERROR_SAVE_FAILED;
	}
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_vertex_influences(ChunkSaveClass & csave,MeshSaveContextClass * /*context*/)
{
	WWASSERT(Get_Vertex_Count() > 0);
	if (VertexBoneLink == NULL) return WW3D_ERROR_OK;

	if (!csave.Begin_Chunk(W3D_CHUNK_VERTEX_INFLUENCES)) {
		return WW3D_ERROR_SAVE_FAILED;
	}

	W3dVertInfStruct vinf;
	memset(&vinf,0,sizeof(vinf));

	for (int i=0; i<Get_Vertex_Count(); i++) {
		vinf.BoneIdx = VertexBoneLink->Get_Array()[i];
		if (csave.Write(&vinf,sizeof(vinf)) != sizeof(vinf)) {
			return WW3D_ERROR_SAVE_FAILED;
		}
	}

	if (!csave.End_Chunk()) {
		return WW3D_ERROR_SAVE_FAILED;
	}
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_material_info(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_MATERIAL_INFO)) {
		return WW3D_ERROR_SAVE_FAILED;
	}

	W3dMaterialInfoStruct info;
	memset(&info,0,sizeof(info));

	info.PassCount = DefMatDesc->PassCount;
	info.VertexMaterialCount = context->Materials.Get_Vertex_Material_Count();
	info.TextureCount = context->Materials.Get_Texture_Count();
	info.ShaderCount = context->Materials.Get_Shader_Count();

	if (csave.Write(&info,sizeof(info)) != sizeof(info)) {
		return WW3D_ERROR_SAVE_FAILED;
	}

	if (!csave.End_Chunk()) {
		return WW3D_ERROR_SAVE_FAILED;
	}

	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_shaders(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	if (context->Materials.Get_Shader_Count() <= 0) {
		return WW3D_ERROR_OK;
	}

	if (!csave.Begin_Chunk(W3D_CHUNK_SHADERS)) return WW3D_ERROR_SAVE_FAILED;

	for (int si=0; si<context->Materials.Get_Shader_Count(); si++) {
		W3dShaderStruct file_shader;
		ShaderClass	shader = context->Materials.Peek_Shader(si);
		W3dUtilityClass::Convert_Shader(shader,&file_shader);

		if (csave.Write(&file_shader,sizeof(file_shader)) != sizeof(file_shader)) {
			return WW3D_ERROR_SAVE_FAILED;
		}
	}

	if (!csave.End_Chunk()) return WW3D_ERROR_SAVE_FAILED;
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_vertex_materials(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	if (context->Materials.Get_Vertex_Material_Count() <= 0) return WW3D_ERROR_OK;
	if (!csave.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIALS)) return WW3D_ERROR_SAVE_FAILED;

	for (int vi=0; vi<context->Materials.Get_Vertex_Material_Count(); vi++) {

		csave.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL);
		VertexMaterialClass * vmat = context->Materials.Peek_Vertex_Material(vi);
		vmat->Save_W3D(csave);
		csave.End_Chunk();
	}

	if (!csave.End_Chunk()) return WW3D_ERROR_SAVE_FAILED;
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_textures(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	if (context->Materials.Get_Texture_Count() <= 0) return WW3D_ERROR_OK;
	if (!csave.Begin_Chunk(W3D_CHUNK_TEXTURES)) return WW3D_ERROR_SAVE_FAILED;

	for (int ti=0; ti<context->Materials.Get_Texture_Count(); ti++) {
		TextureClass * tex = context->Materials.Peek_Texture(ti);
		Save_Texture(tex,csave);
	}

	if (!csave.End_Chunk()) return WW3D_ERROR_SAVE_FAILED;
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_material_pass(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	context->CurStage = 0;
	csave.Begin_Chunk(W3D_CHUNK_MATERIAL_PASS);

	write_vertex_material_ids(csave,context);
	write_shader_ids(csave,context);
	write_dcg(csave,context);
	write_dig(csave,context);
	write_scg(csave,context);
	write_texture_stage(csave,context);
	write_texture_stage(csave,context);

	csave.End_Chunk();
	context->CurPass++;

	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_vertex_material_ids(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	// first check if all vertex material pointers are Null (is this legal?)
	if (	(DefMatDesc->Material[context->CurPass] == NULL) &&
			(DefMatDesc->MaterialArray[context->CurPass] == NULL)) return WW3D_ERROR_OK;

	csave.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL_IDS);

	uint32 id = 0;
	if (DefMatDesc->MaterialArray[context->CurPass] == NULL) {

		id = context->Materials.Find_Vertex_Material(DefMatDesc->Material[context->CurPass]);
		csave.Write(&id,sizeof(id));

	} else {

		VertexMaterialClass ** array = DefMatDesc->MaterialArray[context->CurPass]->Get_Array();
		for (int vi=0; vi<Get_Vertex_Count(); vi++) {
			id = context->Materials.Find_Vertex_Material(array[vi]);
			csave.Write(&id,sizeof(id));
		}

	}

	csave.End_Chunk();
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_shader_ids(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	csave.Begin_Chunk(W3D_CHUNK_SHADER_IDS);

	uint32 id = 0;
	if (DefMatDesc->ShaderArray[context->CurPass] == NULL) {

		id = context->Materials.Find_Shader(DefMatDesc->Shader[context->CurPass]);
		csave.Write(&id,sizeof(id));

	} else {

		ShaderClass * array = DefMatDesc->ShaderArray[context->CurPass]->Get_Array();
		for (int pi=0; pi<Get_Polygon_Count(); pi++) {
			id = context->Materials.Find_Shader(array[pi]);
			csave.Write(&id,sizeof(id));
		}
	}

	csave.End_Chunk();
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_scg(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	if (DefMatDesc->SCG[context->CurPass] == NULL) return WW3D_ERROR_OK;
	csave.Begin_Chunk(W3D_CHUNK_SCG);

	W3dRGBAStruct color;
	Vector4 * color_array = DefMatDesc->SCG[context->CurPass]->Get_Array();
	for (int vi=0; vi<Get_Vertex_Count(); vi++) {
		W3dUtilityClass::Convert_Color(color_array[vi],&color);
		csave.Write(&color,sizeof(color));
	}

	csave.End_Chunk();
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_dig(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	if (DefMatDesc->DIG[context->CurPass] == NULL) return WW3D_ERROR_OK;
	csave.Begin_Chunk(W3D_CHUNK_DIG);

	W3dRGBStruct color;
	Vector3 * color_array = DefMatDesc->DIG[context->CurPass]->Get_Array();
	for (int vi=0; vi<Get_Vertex_Count(); vi++) {
		W3dUtilityClass::Convert_Color(color_array[vi],&color);
		csave.Write(&color,sizeof(color));
	}
	csave.End_Chunk();
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_dcg(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	if (DefMatDesc->DCG[context->CurPass] == NULL) return WW3D_ERROR_OK;
	csave.Begin_Chunk(W3D_CHUNK_DCG);

	W3dRGBAStruct color;
	Vector4 * color_array = DefMatDesc->DCG[context->CurPass]->Get_Array();
	for (int vi=0; vi<Get_Vertex_Count(); vi++) {
		W3dUtilityClass::Convert_Color(color_array[vi],&color);
		csave.Write(&color,sizeof(color));
	}
	csave.End_Chunk();
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_texture_stage(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	if (	(DefMatDesc->Texture[context->CurPass][context->CurStage] == NULL) &&
			(DefMatDesc->TextureArray[context->CurPass][context->CurStage] == NULL)) return WW3D_ERROR_OK;

	csave.Begin_Chunk(W3D_CHUNK_TEXTURE_STAGE);
	write_texture_ids(csave,context);
	write_stage_texcoords(csave,context);
	csave.End_Chunk();

	context->CurStage++;
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_texture_ids(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	csave.Begin_Chunk(W3D_CHUNK_TEXTURE_IDS);

	uint32 id = 0;
	if (DefMatDesc->TextureArray[context->CurPass][context->CurStage] == NULL) {

		id = context->Materials.Find_Texture(DefMatDesc->Texture[context->CurPass][context->CurStage]);
		csave.Write(&id,sizeof(id));

	} else {

		srTextureIFace ** array = DefMatDesc->TextureArray[context->CurPass][context->CurStage]->Get_Array();
		for (int pi=0; pi<Get_Polygon_Count(); pi++) {
			id = context->Materials.Find_Texture(array[pi]);
			csave.Write(&id,sizeof(id));
		}
	}

	csave.End_Chunk();
	return WW3D_ERROR_OK;
}

WW3DErrorType MeshModelClass::write_stage_texcoords(ChunkSaveClass & csave,MeshSaveContextClass * context)
{
	if (DefMatDesc->UV[context->CurPass][context->CurStage] == NULL) return WW3D_ERROR_OK;
	csave.Begin_Chunk(W3D_CHUNK_STAGE_TEXCOORDS);

	W3dTexCoordStruct tex;
	Vector2 * array = DefMatDesc->UV[context->CurPass][context->CurStage]->Get_Array();

	for (int vi=0; vi<Get_Vertex_Count(); vi++) {

		tex.U = array[vi].X;
		tex.V = 1.0f - array[vi].Y;

		csave.Write(&tex,sizeof(tex));
	}

	csave.End_Chunk();
	return WW3D_ERROR_OK;
}

#endif // 0 (disabled mesh saving code)
