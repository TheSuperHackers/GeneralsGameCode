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
 *                     $Archive:: wwshade/shdsubmesh.cpp													$*
 *                                                                                             *
 *                   Org Author:: Jani P                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 07/12/02 10:31a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 * 07/12/02 KM Removed legacy mesh conversion                                                  *
 *---------------------------------------------------------------------------------------------*
 *---------------------------------------------------------------------------------------------*/

#include "shddefmanager.h"
#include "shdsubmesh.h"
#include "shdrenderer.h"
#include "shdlegacyw3d.h"
#include "shdclassids.h"

#include "chunkio.h"
#include "texture.h"
#include "w3d_file.h"
#include "ww3d.h"
#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "meshmdl.h"
#include "texture.h"
#include "aabtree.h"
#include "sharebuf.h"

ShdSubMeshClass::ShdSubMeshClass(void)
	:
	Shader(NULL),
	S(NULL),
	T(NULL),
	SxT(NULL),
	Diffuse(NULL),
	FirstVisiblePolygon(0),
	VisiblePolygonCount(0),
	Sorting(false)
{
	for (int i=0;i<MAX_TEXTURE_STAGES;++i) 
	{
		UV[i]=NULL;
	}
}

ShdSubMeshClass::ShdSubMeshClass(const ShdSubMeshClass& that)
	:
	Shader(NULL),
	S(NULL),
	T(NULL),
	SxT(NULL),
	Diffuse(NULL),
	FirstVisiblePolygon(that.FirstVisiblePolygon),
	VisiblePolygonCount(that.VisiblePolygonCount),
	Sorting(false)
{
	for (int i=0;i<MAX_TEXTURE_STAGES;++i) 
	{
		UV[i]=NULL;
	}
	*this = that;
}

ShdSubMeshClass & ShdSubMeshClass::operator = (const ShdSubMeshClass & that)
{
	if (this != &that) 
	{
		FirstVisiblePolygon=that.FirstVisiblePolygon;
		VisiblePolygonCount=that.VisiblePolygonCount;
		Sorting=that.Sorting;
		REF_PTR_SET(Shader,that.Shader);
		for (int i=0;i<MAX_TEXTURE_STAGES;++i) 
		{
			REF_PTR_SET(UV[i],that.UV[i]);
		}

		REF_PTR_SET(S,that.S);
		REF_PTR_SET(T,that.T);
		REF_PTR_SET(SxT,that.SxT);

		REF_PTR_SET(Diffuse,that.Diffuse);
		MeshGeometryClass::operator =(that);
	}
	return * this;
}


ShdSubMeshClass::~ShdSubMeshClass(void)
{
	REF_PTR_RELEASE(Shader);
	for (int i=0;i<MAX_TEXTURE_STAGES;++i) 
	{
		REF_PTR_RELEASE(UV[i]);
	}

	REF_PTR_RELEASE(S);
	REF_PTR_RELEASE(T);
	REF_PTR_RELEASE(SxT);

	REF_PTR_RELEASE(Diffuse);
}

// The legacy renderer is FVF-based so we need to figure out what FVF format to use
static unsigned Define_FVF(MeshModelClass* mmc,bool enable_lighting)
{
	if (mmc->Get_Flag(MeshGeometryClass::SKIN)) {
		return dynamic_fvf_type;
	}
	if ((!!mmc->Get_Flag(MeshGeometryClass::SORT)) && WW3D::Is_Sorting_Enabled()) {
		return dynamic_fvf_type;
	}

	unsigned fvf=D3DFVF_XYZ;

	int tex_coord_count=mmc->Get_UV_Array_Count();

	if (mmc->Get_Color_Array(0,false)) {
		fvf|=D3DFVF_DIFFUSE;
	}
	if (mmc->Get_Color_Array(1,false)) {
		fvf|=D3DFVF_SPECULAR;
	}
	
	switch (tex_coord_count) {
	default:
	case 0:
		break;
	case 1: fvf|=D3DFVF_TEX1; break;
	case 2: fvf|=D3DFVF_TEX2; break;
	case 3: fvf|=D3DFVF_TEX3; break;
	case 4: fvf|=D3DFVF_TEX4; break;
	case 5: fvf|=D3DFVF_TEX5; break;
	case 6: fvf|=D3DFVF_TEX6; break;
	case 7: fvf|=D3DFVF_TEX7; break;
	case 8: fvf|=D3DFVF_TEX8; break;
	}

	if (!mmc->Needs_Vertex_Normals()) {  //enable_lighting || mmc->Get_Flag(MeshModelClass::PRELIT_MASK)) {
		return fvf;
	}

	fvf|=D3DFVF_NORMAL;	// Realtime-lit
	return fvf;
}


// The legacy renderer is FVF-based so we need to figure out what FVF format to use
static unsigned Define_FVF(ShdSubMeshClass* ssm,bool enable_lighting)
{
	if (ssm->Get_Flag(MeshGeometryClass::SKIN)) {
		return dynamic_fvf_type;
	}
	if ((!!ssm->Get_Flag(MeshGeometryClass::SORT)) && WW3D::Is_Sorting_Enabled()) {
		return dynamic_fvf_type;
	}

	unsigned fvf=D3DFVF_XYZ;

	int tex_coord_count=0;
	for (tex_coord_count=0;tex_coord_count < MeshMatDescClass::MAX_TEX_STAGES; tex_coord_count++) {
		if (ssm->Get_UV_Array(tex_coord_count) == NULL) {
			break;
		}
	}

	if (ssm->Get_Diffuse_Array() != NULL) {
		fvf|=D3DFVF_DIFFUSE;
	}
	
	switch (tex_coord_count) {
	default:
	case 0:
		break;
	case 1: fvf|=D3DFVF_TEX1; break;
	case 2: fvf|=D3DFVF_TEX2; break;
	case 3: fvf|=D3DFVF_TEX3; break;
	case 4: fvf|=D3DFVF_TEX4; break;
	case 5: fvf|=D3DFVF_TEX5; break;
	case 6: fvf|=D3DFVF_TEX6; break;
	case 7: fvf|=D3DFVF_TEX7; break;
	case 8: fvf|=D3DFVF_TEX8; break;
	}

	if (ssm->Get_Vertex_Normal_Array() != NULL) {
		fvf|=D3DFVF_NORMAL;	// Realtime-lit
	}

	return fvf;
}


void ShdSubMeshClass::Init_From_Legacy_Mesh_Model(MeshModelClass* model,int first_polygon)
{
	WWASSERT(first_polygon<model->Get_Polygon_Count());

	Sorting=!!model->Get_Flag(MeshGeometryClass::SORT) && WW3D::Is_Sorting_Enabled();

	int pass;

	// Figure out how many polygons we can use
	int active_polygon_count=0;
	bool arrays_used=false;
	for (pass=0;pass<model->Get_Pass_Count() && !arrays_used;++pass) {
		if (model->Has_Shader_Array(pass) || model->Has_Material_Array(pass)) {
			arrays_used=true;
			break;
		}
		for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
			TextureClass* texture=NULL;
			if (model->Has_Texture_Array(pass,stage)) {
				arrays_used=true;
				break;
			}
		}
	}

	// If any arrays are used by the mesh, count how many polygons we have that share properties.
	if (arrays_used) {
		bool stop=false;
		active_polygon_count=0;
		for (int poly=first_polygon;poly<model->Get_Polygon_Count() && !stop;++poly) {
			for (pass=0;pass<model->Get_Pass_Count();++pass) {
				if (model->Has_Shader_Array(pass)) {
					if (model->Get_Shader(first_polygon,pass)!=model->Get_Shader(poly,pass)) {
						stop=true;
						break;
					}
				}

				if (model->Has_Material_Array(pass)) {
					TriIndex first_tri=model->Get_Polygon_Array()[first_polygon];
					TriIndex cur_tri=model->Get_Polygon_Array()[poly];
					if (model->Peek_Material(first_tri[0],pass)!=model->Peek_Material(cur_tri[0],pass) ||
						model->Peek_Material(first_tri[1],pass)!=model->Peek_Material(cur_tri[1],pass) ||
						model->Peek_Material(first_tri[2],pass)!=model->Peek_Material(cur_tri[2],pass)) {
						stop=true;
						break;
					}
				}

				for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
					if (model->Has_Texture_Array(pass,stage)) {
						if (model->Peek_Texture(first_polygon,pass,stage)!=model->Peek_Texture(poly,pass,stage)) {
							stop=true;
							break;
						}
					}
				}
			}
			active_polygon_count++;
		}
	}
	FirstVisiblePolygon=first_polygon;
	VisiblePolygonCount=active_polygon_count;
	if (FirstVisiblePolygon!=0 && VisiblePolygonCount==0) {
		VisiblePolygonCount=model->Get_Polygon_Count()-FirstVisiblePolygon;
	}
	WWASSERT(FirstVisiblePolygon+VisiblePolygonCount<=model->Get_Polygon_Count());

	// Now that the definition is initialized, create a shader instance (there's really
	// only one def/instance pair with the legacy meshes, no optimization planned currently)
	ShdLegacyW3DDefClass* shader_def=NEW_REF( ShdLegacyW3DDefClass, () );
	Shader=shader_def->Create();
	REF_PTR_RELEASE(shader_def);

	// Danger! We're assuming Shd6LegacyW3DClass is the only type created by legacy shader!!!
	((Shd6LegacyW3DClass*)Shader)->Set_FVF(Define_FVF(model,true));
	((Shd6LegacyW3DClass*)Shader)->Set_Pass_Count(model->Get_Pass_Count());

	for (pass=0;pass<model->Get_Pass_Count();++pass) {
		ShaderClass legacy_shader=ShaderClass::_PresetOpaqueShader;
		if (model->Has_Shader_Array(pass)) {
			legacy_shader=model->Get_Shader(first_polygon,pass);
		}
		else {
			legacy_shader=model->Get_Single_Shader(pass);
		}

		VertexMaterialClass* legacy_material=NULL;
		if (model->Has_Material_Array(pass)) {
			TriIndex first_tri=model->Get_Polygon_Array()[first_polygon];
			legacy_material=model->Get_Material(first_tri[0],pass);
		}
		else {
			legacy_material=model->Get_Single_Material(pass);
		}

		for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
			TextureClass* texture=NULL;
			if (model->Has_Texture_Array(pass,stage)) {
				texture=model->Get_Texture(first_polygon,pass,stage);
			}
			else {
				texture=model->Get_Single_Texture(pass,stage);
			}

			if (texture) {
//				StringClass texture_name=texture->Get_Full_Path();
//				texture->Release_Ref();
//				shader_def->Set_Texture_Name(pass,stage,texture_name);
				((Shd6LegacyW3DClass*)Shader)->Set_Texture(pass,stage,texture);
				texture->Release_Ref();
			}
		}


		((Shd6LegacyW3DClass*)Shader)->Set_Shader(pass,legacy_shader);
		((Shd6LegacyW3DClass*)Shader)->Set_Material(pass,legacy_material);
	}

	*(MeshGeometryClass*)this = *(MeshGeometryClass*)model;
	if (CullTree)
		CullTree->Set_Mesh(this);

	// Copy texture coordinates
	int uv_array_count=model->Get_UV_Array_Count();
	WWASSERT(uv_array_count<=MAX_TEXTURE_STAGES);
	for (int uvi=0;uvi<uv_array_count;++uvi) {
		const Vector2 *uv=model->Get_UV_Array_By_Index(uvi);
		if (uv) {
			UV[uvi]=NEW_REF(ShareBufferClass<Vector2>,(model->Get_Vertex_Count(),"ShdSubMeshClass::UVCoords"));
			memcpy(UV[uvi]->Get_Array(),uv,sizeof(Vector2)*model->Get_Vertex_Count());
		}
	}

	// Copy vertex color array
	const unsigned* diffuse=model->Get_DCG_Array(0);
	if (diffuse) {
		Diffuse=NEW_REF(ShareBufferClass<unsigned>,(model->Get_Vertex_Count(),"ShdSubMeshClass::Diffuse"));
		memcpy(Diffuse->Get_Array(),diffuse,sizeof(unsigned)*model->Get_Vertex_Count());
	}

}


WW3DErrorType ShdSubMeshClass::Load_W3D(ChunkLoadClass& cload)
{
	// read header
	W3dShdSubMeshHeaderStruct hdr;
	cload.Open_Chunk();
	if 
	(
		cload.Read
		(
			&hdr,
			sizeof(W3dShdSubMeshHeaderStruct)
		)!=sizeof(W3dShdSubMeshHeaderStruct)
	)
	{
		return WW3D_ERROR_LOAD_FAILED;
	}
	cload.Close_Chunk();

	// process header
	PolyCount=hdr.NumTris;
	VertexCount=hdr.NumVertices;

	if ((PolyCount!=0) && (VertexCount!=0)) 
	{
		Poly				= NEW_REF(ShareBufferClass<TriIndex>,(PolyCount,"ShdSubMesh::Poly"));
		PolySurfaceType		= NEW_REF(ShareBufferClass<uint8>,(PolyCount,"ShdSubMesh::PolySurfaceType"));
		Vertex				= NEW_REF(ShareBufferClass<Vector3>,(VertexCount,"ShdSubMesh::Vertex"));

		Poly->Clear();
		PolySurfaceType->Clear();
		Vertex->Clear();

#if (!OPTIMIZE_VNORM_RAM)
		VertexNorm		=NEW_REF(ShareBufferClass<Vector3>,(VertexCount,"ShdSubMesh::VertexNorm"));
		VertexNorm->Clear();
#endif
	}

	BoundBoxMin.Set(hdr.BoxMin.X,hdr.BoxMin.Y,hdr.BoxMin.Z);
	BoundBoxMax.Set(hdr.BoxMax.X,hdr.BoxMax.Y,hdr.BoxMax.Z);
	BoundSphereCenter.Set(hdr.SphCenter.X,hdr.SphCenter.Y,hdr.SphCenter.Z);
	BoundSphereRadius=hdr.SphRadius;

	read_chunks(cload);

	if ((Shader->Peek_Definition() != NULL) && (Shader->Peek_Definition()->Get_Class_ID() == SHDDEF_CLASSID_LEGACYW3D)) {
		// Danger! We're assuming Shd6LegacyW3DClass is the only type created by legacy shader!!!
		Shd6LegacyW3DClass * legacy_shader = (Shd6LegacyW3DClass*)Shader;
		legacy_shader->Set_FVF(Define_FVF(this,true));
		if (!legacy_shader->Is_Opaque()) {
			Set_Flag(MeshGeometryClass::SORT,true);
		}
	}

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * ShdSubMeshClass::read_chunks -- read w3d chunks                                           *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/21/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_chunks(ChunkLoadClass& cload) 
{
	// If there are no more chunks within the mesh chunk,
	// we are done.

	while (cload.Open_Chunk()) 
	{
		// Process the chunk
		WW3DErrorType error=WW3D_ERROR_OK;

		switch (cload.Cur_Chunk_ID()) 
		{
		case W3D_CHUNK_SHDSUBMESH_VERTICES					: error=read_vertices(cload); break;
		case W3D_CHUNK_SHDSUBMESH_VERTEX_NORMALS			: error=read_vertex_normals(cload); break;
		case W3D_CHUNK_SHDSUBMESH_UV0							: error=read_uv0(cload); break;
		case W3D_CHUNK_SHDSUBMESH_UV1							: error=read_uv1(cload); break;

		case W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_S			: error=read_tangent_basis_s(cload); break;
		case W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_T			: error=read_tangent_basis_t(cload); break;
		case W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_SxT		: error=read_tangent_basis_sxt(cload); break;

		case W3D_CHUNK_SHDSUBMESH_TRIANGLES					: error=read_triangles(cload); break;
		case W3D_CHUNK_SHDSUBMESH_VERTEX_SHADE_INDICES	: error=read_vertex_shade_indices(cload); break;
		case W3D_CHUNK_SHDSUBMESH_SHADER						: error=read_shader(cload); break;
		case W3D_CHUNK_SHDSUBMESH_VERTEX_INFLUENCES		: error=read_vertex_influences(cload); break;
		default: break;
		}	
		
		cload.Close_Chunk();

		if (error!=WW3D_ERROR_OK) 
		{
			return error;
		}
	}

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * ShdSubMeshClass::read_vertices -- read the vertex chunk from a W3D file                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/21/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_vertices(ChunkLoadClass& cload)
{
	W3dVectorStruct vert;
	Vector3* loc=Vertex->Get_Array();
	assert(loc);

	for (int i=0; i<Get_Vertex_Count(); i++) 
	{
		if (cload.Read(&vert,sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct)) 
		{
			return WW3D_ERROR_LOAD_FAILED;
		}
		
		loc[i].X=vert.X;
		loc[i].Y=vert.Y;
		loc[i].Z=vert.Z;
	}

	return WW3D_ERROR_OK;	
}


/***********************************************************************************************
 * ShdSubMeshClass::read_vertex_normals -- read the vertex normals chunk from a w3d file     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/21/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_vertex_normals(ChunkLoadClass& cload)
{
	W3dVectorStruct norm;
	Vector3 * mdlnorms=get_vert_normals();
	WWASSERT(mdlnorms);

	for (int i=0; i<VertexCount; i++) 
	{
		if (cload.Read(&norm,sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct)) 
		{
			return WW3D_ERROR_LOAD_FAILED;
		}

		mdlnorms[i].Set(norm.X,norm.Y,norm.Z);
	}

	return WW3D_ERROR_OK;	
}

/***********************************************************************************************
 * ShdSubMeshClass::read_uv0 -- read the texture coords chunk from a w3d file     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/26/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_uv0(ChunkLoadClass& cload)
{
	UV[0]=NEW_REF(ShareBufferClass<Vector2>,(VertexCount,"ShdSubMeshClass::UV"));
	UV[0]->Clear();

	Vector2* uv=UV[0]->Get_Array();

	for (int i=0; i<VertexCount; i++) 
	{
		if 
		(
			cload.Read
			(
				uv++,
				sizeof(Vector2)
			)!=sizeof(Vector2)
		) 
		{
			return WW3D_ERROR_LOAD_FAILED;
		}
	}

	return WW3D_ERROR_OK;	

}

/***********************************************************************************************
 * ShdSubMeshClass::read_uv1 -- read the texture coords chunk from a w3d file     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/26/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_uv1(ChunkLoadClass& cload)
{
	UV[1]=NEW_REF(ShareBufferClass<Vector2>,(VertexCount,"ShdSubMeshClass::UV"));
	UV[1]->Clear();

	Vector2* uv=UV[1]->Get_Array();

	for (int i=0; i<VertexCount; i++) 
	{
		if 
		(
			cload.Read
			(
				uv++,
				sizeof(Vector2)
			)!=sizeof(Vector2)
		) 
		{
			return WW3D_ERROR_LOAD_FAILED;
		}
	}

	return WW3D_ERROR_OK;	

}

/***********************************************************************************************
 * ShdSubMeshClass::read_tangent_basis_s -- read the tangent basis chunk from a w3d file     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/26/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_tangent_basis_s(ChunkLoadClass& cload)
{
	S=NEW_REF(ShareBufferClass<Vector3>,(VertexCount,"ShdSubMeshClass::S"));
	S->Clear();

	Vector3* t=S->Get_Array();

	for (int i=0; i<VertexCount; i++) 
	{
		if 
		(
			cload.Read
			(
				t++,
				sizeof(Vector3)
			)!=sizeof(Vector3)
		) 
		{
			return WW3D_ERROR_LOAD_FAILED;
		}
	}

	return WW3D_ERROR_OK;	

}

/***********************************************************************************************
 * ShdSubMeshClass::read_tangent_basis_t -- read the tangent basis chunk from a w3d file     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/26/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_tangent_basis_t(ChunkLoadClass& cload)
{
	T=NEW_REF(ShareBufferClass<Vector3>,(VertexCount,"ShdSubMeshClass::T"));
	T->Clear();

	Vector3* t=T->Get_Array();

	for (int i=0; i<VertexCount; i++) 
	{
		if 
		(
			cload.Read
			(
				t++,
				sizeof(Vector3)
			)!=sizeof(Vector3)
		) 
		{
			return WW3D_ERROR_LOAD_FAILED;
		}
	}

	return WW3D_ERROR_OK;	

}

/***********************************************************************************************
 * ShdSubMeshClass::read_tangent_basis_sxt -- read the tangent basis chunk from a w3d file     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/26/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_tangent_basis_sxt(ChunkLoadClass& cload)
{
	SxT=NEW_REF(ShareBufferClass<Vector3>,(VertexCount,"ShdSubMeshClass::SxT"));
	SxT->Clear();

	Vector3* t=SxT->Get_Array();

	for (int i=0; i<VertexCount; i++) 
	{
		if 
		(
			cload.Read
			(
				t++,
				sizeof(Vector3)
			)!=sizeof(Vector3)
		) 
		{
			return WW3D_ERROR_LOAD_FAILED;
		}
	}

	return WW3D_ERROR_OK;	

}

/***********************************************************************************************
 * ShdSubMeshClass::read_vertex_influences -- read the vertex bone links from a w3d file       *
 *                                                                                             *
 * This is for WWSKin support, presumably when we implement HW skining this may become obsolete*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/07/2002  gth: Created                                                                  *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_vertex_influences(ChunkLoadClass& cload)
{
	uint16 * bone_links = get_bone_links();

	for (int i=0; i<VertexCount; i++) 
	{
		if 
		(
			cload.Read
			(
				bone_links++,
				sizeof(uint8)
			)!=sizeof(uint8)
		) 
		{
			return WW3D_ERROR_LOAD_FAILED;
		}
	}

	Set_Flag(SKIN,true);

	return WW3D_ERROR_OK;	
}

/***********************************************************************************************
 * ShdSubMeshClass::read_triangles -- read the triangles chunk from a w3d file               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/21/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_triangles(ChunkLoadClass & cload)
{
	unsigned short tri[3];

	// cache pointers to various arrays in the surrender mesh
	TriIndex * vi = get_polys();
	Set_Flag(DIRTY_PLANES,false);
//	Vector4 * peq = get_planes();
//	uint8 * surface_types = Get_Poly_Surface_Type_Array();

	// read in each polygon one by one
	for (int i=0; i<Get_Polygon_Count(); i++) 
	{
		if 
		(
			cload.Read
			(
				tri,
				sizeof(unsigned short)*3
			)!=sizeof(unsigned short)*3
		)
		{
			return WW3D_ERROR_LOAD_FAILED;
		}

		// set the vertex indices
		vi[i].I = tri[0];
		vi[i].J = tri[1];
		vi[i].K = tri[2];

		// set the normal
		/*peq[i].X = tri.Normal.X;
		peq[i].Y = tri.Normal.Y;
		peq[i].Z = tri.Normal.Z;
		peq[i].W = -tri.Dist;

		// set the surface type
		WWASSERT(tri.Attributes < 256);
		surface_types[i] = (uint8)(tri.Attributes);*/
	}

	return WW3D_ERROR_OK;	
}

/***********************************************************************************************
 * ShdSubMeshClass::read_vertex_shade_indices -- read the vertex shade indices chunk         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/21/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_vertex_shade_indices(ChunkLoadClass & cload)
{
	uint32 * shade_index = get_shade_indices(true);
	uint32 si;

	for (int i=0; i<Get_Vertex_Count(); i++) 
	{
		if (cload.Read(&si,sizeof(uint32)) != sizeof(uint32)) 
		{
			return WW3D_ERROR_LOAD_FAILED;
		}
		shade_index[i] = si;
	}
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * ShdSubMeshClass::read_shader -- read the shader chunk													 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/21/2002  kjm : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType ShdSubMeshClass::read_shader(ChunkLoadClass& cload)
{
	// load the shader definition
	ShdDefClass* shddef=NULL;

	ShdDefManagerClass::Load_Shader(cload, &shddef);

	// create the shader interface
	Shader=shddef->Create();

	REF_PTR_RELEASE(shddef);
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * ShdSubMesh::Get_Deformed_Vertices -- Gets the deformed vertices for a skin                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/2001   gth : Created.                                                                 *
 *=============================================================================================*/
void	ShdSubMeshClass::Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm, const HTreeClass* htree)
{
	WWASSERT(Get_Flag(MeshGeometryClass::SKIN));
	WWASSERT(htree);
	get_deformed_vertices(dst_vert,dst_norm,htree);
}


/***********************************************************************************************
 * ShdSubMeshClass::Get_Deformed_Vertices -- Gets the deformed vertices for a skin                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/2001   gth : Created.                                                                 *
 *=============================================================================================*/
void ShdSubMeshClass::Get_Deformed_Vertices(Vector3 *dst_vert,const HTreeClass* htree)
{
	WWASSERT(Get_Flag(MeshGeometryClass::SKIN));
	WWASSERT(htree);

	get_deformed_vertices(dst_vert,htree);
}
