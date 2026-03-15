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

/* $Header: /Commando/Code/Tools/max2w3d/meshsave.cpp 107   8/21/01 10:28a Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G                                                 *
 *                                                                                             *
 *                    File Name : MESHSAVE.cpp                                                 *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 06/10/97                                                     *
 *                                                                                             *
 *                  Last Update : 10/20/1999997 [GH]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MeshSaveClass::MeshSaveClass -- constructor, processes a Max mesh                         *
 *   MeshSaveClass::~MeshSaveClass -- destructor, frees all allocated memory                   *
 *   MeshSaveClass::write_verts -- write the vertex chunk into a wtm file                      *
 *   MeshSaveClass::write_header -- write a mesh header chunk into a wtm file                  *
 *   MeshSaveClass::Write_To_File -- Append the mesh to an open wtm file                       *
 *   MeshSaveClass::write_normals -- writes the vertex normals chunk into a wtm file           *
 *   MeshSaveClass::write_vert_normals -- Writes the surrender normal chunk into a wtm file    *
 *   MeshSaveClass::write_triangles -- Write the triangles chunk into a wtm file.              *
 *   MeshSaveClass::write_sr_triangles -- writes the triangles in surrender friendly format    *
 *   MeshSaveClass::write_triangles -- write the triangles chunk                               *
 *   MeshSaveClass::compute_surrender_vertex -- Compute the surrender vertex normals           *
 *   MeshSaveClass::setup_material -- Gets the texture names and base colors for a material    *
 *   MeshSaveClass::compute_bounding_volumes -- computes a bounding box and bounding sphere for*
 *   MeshSaveClass::set_transform -- set the default transformation matrix for the mesh        *
 *   MeshSaveClass::compute_physical_properties -- computes the volume and moment of inertia   *
 *   MeshSaveClass::prep_mesh -- pre-transform the MAX mesh by a specified matrix              *
 *   MeshSaveClass::write_user_text -- write the user text chunk                               *
 *   MeshSaveClass::get_htree_bone_index_for_inode -- searches the htree for the given INode   *
 *   MeshSaveClass::get_skin_modifier_objects -- Searches for the WWSkin modifier for this mes *
 *   MeshSaveClass::inv_deform_mesh -- preprocess the mesh for skinning                        *
 *   MeshSaveClass::create_materials -- create the materials for this mesh                     *
 *   MeshSaveClass::write_ps2_shaders -- Write shaders specific to the PS2 in their own chunk. *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "meshsave.h"
#include <max.h>
#include <stdmat.h>
#include <modstack.h>
#include "gamemtl.h"
#include "errclass.h"
#include "vxl.h"
#include "vxldbg.h"
#include "nodelist.h"
#include "hiersave.h"
#include "util.h"
#include "w3dappdata.h"
#include "skin.h"
#include "skindata.h"
#include "meshbuild.h"
#include "AlphaModifier.h"
#include "aabtreebuilder.h"
#include "exportlog.h"


static char _string1[512];
const int VOXEL_RESOLUTION = 64;		// resolution to use when computing I, V and CM

#define DEBUG_VOXELS				0
#define MIN_AABTREE_POLYGONS	8
#define DIFFUSE_HOUSECOLOR_TEXTURE_PREFIX 0x4443485A		//'ZHCD' prefix put on all code generated textures
#define DIFFUSE_COLOR_TEXTURE_PREFIX 0x44434d5A		//'ZMCD' prefix put on all code generated textures
#define DIFFUSE_COLOR_TEXTURE_MASK	 0x4443005A

/************************************************************************************
**
** Compute the determinant of the 3x3 portion of the given matrix
**
************************************************************************************/
float Compute_3x3_Determinant(const Matrix3 & tm)
{
	float det = tm[0][0] * (tm[1][1]*tm[2][2] - tm[1][2]*tm[2][1]);
	det -=		tm[0][1] * (tm[1][0]*tm[2][2] - tm[1][2]*tm[2][0]);
	det +=		tm[0][2] * (tm[1][0]*tm[2][1] - tm[1][1]*tm[2][0]);

	return det;
}

/************************************************************************************
**
** check if this is a mesh which should use a simple rendering method.  I don't
** compute vertex normals or store u-v's in that case (prevents vertex splitting)
**
************************************************************************************/
bool use_simple_rendering(int geo_type)
{
	geo_type &= W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;

	if (	(geo_type == OBSOLETE_W3D_MESH_FLAG_GEOMETRY_TYPE_SHADOW) ||
			(geo_type == W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX) ||
			(geo_type == W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX) )
	{
		return true;
	} else {
		return false;
	}
}

/************************************************************************************
**
** build the bitfield of W3D mesh attributes for the given node
**
************************************************************************************/
uint32 setup_mesh_attributes(INode * node)
{
	uint32 attributes = W3D_MESH_FLAG_NONE;

	/*
	** Mesh will be one of:
	** W3D_MESH_FLAG_NONE,
	** W3D_MESH_FLAG_COLLISION_BOX,
	** W3D_MESH_FLAG_SKIN,
	** W3D_MESH_FLAG_ALIGNED
	** W3D_MESH_FLAG_ORIENTED
	*/
	if (Is_Collision_AABox(node)) {
		attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX;
	} else if (Is_Collision_OBBox(node)) {
		attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX;
	} else if (Is_Skin(node)) {
		attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN;
	} else if (Is_Camera_Aligned_Mesh(node)) {
		attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED;
	} else if (Is_Camera_Oriented_Mesh(node)) {
		attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED;
	}

	/*
	** And, a mesh may have one or more types of collision detection enabled.
	** W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL
	** W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE
	** However, if the mesh is SKIN, SHADOW, ALIGNED, ORIENTED or nullptr, don't let
	** the collision bits get set...
	*/
	if (	attributes != W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN &&
			attributes != W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED &&
			attributes != W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED )
	{

		if (Is_Physical_Collision(node)) {
			attributes |= W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL;
		}

		if (Is_Projectile_Collision(node)) {
			attributes |= W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE;
		}

		if (Is_Vis_Collision(node)) {
			attributes |= W3D_MESH_FLAG_COLLISION_TYPE_VIS;
		}

		if (Is_Camera_Collision(node)) {
			attributes |= W3D_MESH_FLAG_COLLISION_TYPE_CAMERA;
		}

		if (Is_Vehicle_Collision(node)) {
			attributes |= W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE;
		}

	}

	/*
	** A mesh may have one of the following bits set as well
	*/
	if (Is_Hidden(node)) {
		attributes |= W3D_MESH_FLAG_HIDDEN;
	}
	if (Is_Two_Sided(node)) {
		attributes |= W3D_MESH_FLAG_TWO_SIDED;
	}
	if (Is_Shadow(node)) {
		attributes |= W3D_MESH_FLAG_CAST_SHADOW;
	}
	if (Is_Shatterable(node)) {
		attributes |= W3D_MESH_FLAG_SHATTERABLE;
	}
	if (Is_NPatchable(node)) {
		attributes |= W3D_MESH_FLAG_NPATCHABLE;
	}

	return attributes;
}



/***********************************************************************************************
 * MeshSaveClass::MeshSaveClass -- constructor, processes a Max mesh                           *
 *                                                                                             *
 * This class takes a MAX mesh and computes the information for a W3D mesh or skin.            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * inode - the max INode containing the mesh/skin to export                                    *
 * exportspace - matrix defining the desired coordinate system for the mesh                    *
 * htree - hierarchy tree that this mesh is being connected to                                 *
 * curtime - current time in Max.                                                              *
 * meter - progress meter                                                                      *
 * mesh_name - name to use for the mesh                                                        *
 * container_name - name of the container                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
MeshSaveClass::MeshSaveClass
(
	char * mesh_name,
	char * container_name,
	INode * inode,
	const Mesh * input_mesh,
	Matrix3 & exportspace,
	W3DAppData2Struct	& exportoptions,
	HierarchySaveClass * htree,
	TimeValue curtime,
	Progress_Meter_Class & meter,
	unsigned int *	materialColors,
	int	&numMaterialColors,
	int &numHouseColors,
	char *materialColorTexture,
	WorldInfoClass * world_info
) :
	MaxINode(inode),
	ExportOptions(exportoptions),
	CurTime(curtime),
	ExportSpace(exportspace),
	HTree(htree),
	UserText(nullptr),
	VertInfluences(nullptr),
	MaterialRemapTable(nullptr)
{
	Mesh			mesh = *input_mesh;		// copy the mesh so we can modify it
	Mtl *		   nodemtl = inode->GetMtl();
	DWORD		   wirecolor = inode->GetWireColor();

	PS2Material = FALSE;

	// Check to see if the mesh uses PS2 game materials.  If so, set a flag so
	// that write_shaders will know to make a PS2 shader chunk.
	if (nodemtl) {
		if (nodemtl->ClassID() == PS2GameMaterialClassID) {
			PS2Material = TRUE;
		} else if (nodemtl->IsMultiMtl()) {
			for (int i = 0; i < nodemtl->NumSubMtls(); i++) {
				Mtl *sub = nodemtl->GetSubMtl(i);
				if (sub->ClassID() == PS2GameMaterialClassID) {
					PS2Material = TRUE;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////
	// Check if the mesh is being inverted by its transform.  If this
	// is the case, then we will need to reverse the winding of all
	// polygons later.
	//////////////////////////////////////////////////////////////////////
	Matrix3 objtm = MaxINode->GetObjectTM(curtime);
	MeshInverted = (Compute_3x3_Determinant(objtm) < 0.0f);

	//////////////////////////////////////////////////////////////////////
	// Prep the mesh by transforming it by the delta between exportspace
	// and this INodes current space
	// (this is the delta between the bone and the mesh if one exists...)
	//////////////////////////////////////////////////////////////////////
	MeshToExportSpace = objtm * Inverse(ExportSpace);
	prep_mesh(mesh,MeshToExportSpace);

	//////////////////////////////////////////////////////////////////////
	// Prepare the mesh header.
	//////////////////////////////////////////////////////////////////////
	assert(mesh_name != nullptr);
	assert(container_name != nullptr);

	memset(&Header,0,sizeof(Header));
	Set_W3D_Name(Header.MeshName,mesh_name);
	Set_W3D_Name(Header.ContainerName,container_name);

	Header.Version = W3D_CURRENT_MESH_VERSION;
	Header.Attributes = setup_mesh_attributes(MaxINode);

	meter.Finish_In_Steps(
			3*Header.NumTris +		// normals
			Header.NumVertices +		// surrender normals
			64								// voxelization
	);

	ExportLog::printf("\nProcessing Mesh: %s\n",Header.MeshName);

	//////////////////////////////////////////////////////////////////////
	// Enforce that we have enough data to actually make a mesh
	//////////////////////////////////////////////////////////////////////
	if (mesh.getNumFaces() <= 0) {
		throw ErrorClass("No Triangles in Mesh: %s",Header.MeshName);
	}

	if (mesh.getNumVerts() <= 0) {
		throw ErrorClass("No Vertices in Mesh: %s",Header.MeshName);
	}

	//////////////////////////////////////////////////////////////////////
	// process the materials
	//////////////////////////////////////////////////////////////////////
	DebugPrint("processing materials\n");
	scan_used_materials(mesh,nodemtl);
	create_materials(nodemtl,wirecolor,materialColorTexture);

	//////////////////////////////////////////////////////////////////////
	// what face and vertex attributes are we going to export?
	//////////////////////////////////////////////////////////////////////
	Header.FaceChannels = W3D_FACE_CHANNEL_FACE;
	Header.VertexChannels = W3D_VERTEX_CHANNEL_LOCATION;

	if (!use_simple_rendering(Header.Attributes)) {
		Header.VertexChannels |= W3D_VERTEX_CHANNEL_NORMAL;
	}

	if (((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) == W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN) && (HTree != nullptr)) {
		Header.VertexChannels |= W3D_VERTEX_CHANNEL_BONEID;
	}

	//////////////////////////////////////////////////////////////////////
	// Process the mesh
	//////////////////////////////////////////////////////////////////////
	Builder.Set_World_Info (world_info);
	Build_Mesh(mesh, nodemtl, materialColors, numMaterialColors, numHouseColors);

	if (materialColorTexture)
	{	//diffuse color materials are replaced by textures
		//set diffuse to 255,255,255 so it has no effect.
		fix_diffuse_materials(numHouseColors != 0);
	}

	//////////////////////////////////////////////////////////////////////
	// Create damage (deform) information for the mesh
	//////////////////////////////////////////////////////////////////////
	Object *ref_obj = MaxINode->GetObjectRef ();
	DeformSave.Initialize(Builder, ref_obj, mesh, &MeshToExportSpace);

	//////////////////////////////////////////////////////////////////////
	// Determine if the deformer should use alpha or v-color info
	//////////////////////////////////////////////////////////////////////
	if (ExportOptions.Is_Vertex_Alpha_Enabled()) {
		unsigned int alpha_passes = 0;
		for (int pass=0; pass < MaterialDesc.Pass_Count(); pass++) {
			if (MaterialDesc.Pass_Uses_Vertex_Alpha(pass)) {
				alpha_passes |= (1 << pass);
			}
		}
		DeformSave.Set_Alpha_Passes(alpha_passes);
	}

	//////////////////////////////////////////////////////////////////////
	// Set the counts in the mesh header
	//////////////////////////////////////////////////////////////////////
	Header.NumTris = Builder.Get_Face_Count();
	Header.NumVertices = Builder.Get_Vertex_Count();

	//////////////////////////////////////////////////////////////////////
	// Compute the mesh's bounding box and sphere. This must be done
	// before we pre-deform the mesh (if its a skin).
	//////////////////////////////////////////////////////////////////////
	compute_bounding_volumes();

	//////////////////////////////////////////////////////////////////////
	// Voxelize the mesh and compute the Moment of Inertia and
	// Center of Mass.  This must come after we compute the bounding
	// volumes and before we pre-deform the mesh.
	//////////////////////////////////////////////////////////////////////
	Progress_Meter_Class voxelmeter(meter, 64.0f * meter.Increment);
	compute_physical_constants(MaxINode,voxelmeter,false /*usevoxelizer*/);

	//////////////////////////////////////////////////////////////////////
	// If this is a skin, pre-deform the mesh.
	//////////////////////////////////////////////////////////////////////
	if (((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) == W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN) && (HTree != nullptr)) {
		inv_deform_mesh();
	}

	//////////////////////////////////////////////////////////////////////
	// Get the user text from MAX's properties window.
	//////////////////////////////////////////////////////////////////////
	TSTR usertext;
	MaxINode->GetUserPropBuffer(usertext);
	CStr usertext8 = usertext;

	if (usertext8.Length() > 0) {
		UserText = new char[usertext8.Length() + 1];
		memset(UserText,0,usertext8.Length() + 1);
		memcpy(UserText,usertext8.data(),usertext8.Length());
	}
}

/***********************************************************************************************
 * MeshSaveClass::~MeshSaveClass -- destructor, frees all allocated memory                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
MeshSaveClass::~MeshSaveClass(void)
{
	if (UserText) {
		delete[] UserText;
		UserText = nullptr;
	}

	if (VertInfluences) {
		delete[] VertInfluences;
		VertInfluences = nullptr;
	}

	if (MaterialRemapTable) {
		delete[] MaterialRemapTable;
		MaterialRemapTable = nullptr;
	}
}

//search through previously found material colors and return index.  house colors are always placed in top row.
void getMaterialUV(UVVert &tvert,unsigned int diffuse, unsigned int *materialColors, int &numMaterialColors, int &numHouseColors, bool house)
{
	int i;

	if (house)
	{	//this material is a house color, place it in first row.
		for (i=0; i<16; i++)
		{
			if (materialColors[i]==diffuse)
			{
				tvert.x=((double)(i%16)+0.5)/16.0;	///@todo: MW: Remove hard-coded texture size
				tvert.y=1.0-((double)(i/16)+0.5)/16.0;
				numHouseColors=16;
				return;
			}
		}

		ExportLog::printf("\nUndefined House Color %d,%d,%d",(diffuse>>16)&0xff,(diffuse>>8)&0xff,diffuse&0xff);
		assert(0);	//all house colors must be from a predefined range of reds
	}

	for (i=16; i<(16+numMaterialColors); i++)
	{
		if (materialColors[i]==diffuse)
		{
			tvert.x=((double)(i%16)+0.5)/16.0;	///@todo: MW: Remove hard-coded texture size
			tvert.y=1.0-((double)(i/16)+0.5)/16.0;
			return;
		}
	}

	//new color found
	tvert.x=((double)(i%16)+0.5)/16.0;	///@todo: MW: Remove hard-coded texture size
	tvert.y=1.0-((double)(i/16)+0.5)/16.0;

	materialColors[i]=diffuse;
	numMaterialColors++;
}

void MeshSaveClass::Build_Mesh(Mesh & mesh, Mtl *node_mtl, unsigned int *materialColors, int &numMaterialColors, int &numHouseColors)
{
	int vert_counter;
	int face_index;
	int pass;
	int stage;
	float *vdata = nullptr;
	int firstSolidColoredMaterial=-1;

	Builder.Reset(true,mesh.getNumFaces(),mesh.getNumFaces()/3);

	// Get a pointer to the channel that has alpha values entered by the artist.
	// This pointer will be null if they didn't use the channel.
	vdata = mesh.vertexFloat(ALPHA_VERTEX_CHANNEL);

	/*
	** Get the skin info
	*/
	bool								is_skin = false;
	SkinDataClass *				skindata = nullptr;
	SkinWSMObjectClass *			skinobj = nullptr;

	get_skin_modifier_objects(&skindata,&skinobj);

	if (	((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) == W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN) &&
			(HTree != nullptr)	)
	{
		is_skin = ((skindata != nullptr) && (skinobj != nullptr));
	}

	/*
	** Submit all of the faces
	*/
	MeshBuilderClass::FaceClass face;
	for (face_index = 0; face_index < mesh.getNumFaces(); face_index++) {

		Face maxface = mesh.faces[face_index];

		int mat_index = 0;
		if (Header.NumMaterials > 0) {
			mat_index = MaterialRemapTable[(maxface.getMatID() % Header.NumMaterials)];
		}
		assert(mat_index != -1);

		for (pass=0; pass<MaterialDesc.Pass_Count(); pass++) {
			face.ShaderIndex[pass] = MaterialDesc.Get_Shader_Index(mat_index,pass);
			for (stage=0; stage<W3dMaterialClass::MAX_STAGES; stage++) {
				face.TextureIndex[pass][stage] = MaterialDesc.Get_Texture_Index(mat_index,pass,stage);
			}
		}

		int geo_type = Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;

		if ( use_simple_rendering(geo_type) ) {
			face.SmGroup = 0xFFFFFFFF;
		} else {
			face.SmGroup = maxface.getSmGroup();
		}

		face.Index = face_index;
		face.Attributes = 0;
		face.SurfaceType = 0;

		/*
		**	Lookup this face's surface type
		*/
		Mtl *mtl_to_use = node_mtl;
		if ((node_mtl != nullptr) && (node_mtl->NumSubMtls() > 1)) {
			mtl_to_use = node_mtl->GetSubMtl (maxface.getMatID() % node_mtl->NumSubMtls());
		}

		if ((mtl_to_use != nullptr) && ((mtl_to_use->ClassID() == GameMaterialClassID) ||
			 (mtl_to_use->ClassID() == PS2GameMaterialClassID))) {
			face.SurfaceType = ((GameMtl *)mtl_to_use)->Get_Surface_Type ();
		}

		for (vert_counter = 0; vert_counter < 3; vert_counter++) {

			/*
			** if this mesh is being inverted, we need to insert the verts in
			** the opposite order.  max_vert_counter will count backwards
			** in this case; causing all vertex data to be entered in the
			** reverse winding.
			*/
			int max_vert_counter;

			if (MeshInverted) {
				max_vert_counter = 2 - vert_counter;
			} else {
				max_vert_counter = vert_counter;
			}

			int max_vert_index = maxface.v[max_vert_counter];

			/*
			** Vertex Id, to prevent unwanted welding!
			*/
			face.Verts[vert_counter].Id = max_vert_index;

			/*
			** Vertex Position
			*/
			face.Verts[vert_counter].Position.X =	mesh.verts[max_vert_index].x;
			face.Verts[vert_counter].Position.Y =	mesh.verts[max_vert_index].y;
			face.Verts[vert_counter].Position.Z =	mesh.verts[max_vert_index].z;

			if (vdata) {

				// If an alpha channel has been created, use its value.
				for (int pass=0; pass < MaterialDesc.Pass_Count(); pass++) {
					if (MaterialDesc.Pass_Uses_Vertex_Alpha(pass)) {
						// Mulitiply by .01 to change from percentage.
						face.Verts[vert_counter].Alpha[pass] = vdata[max_vert_index] * .01;
					}
				}
			}


			/*
			** Texture coordinate.  Apply uv coords if the mesh has them and is not being
			** instructed to ignore them.
			** - check if the mesh needs them (uses_simple_rendering() == false)
			** - for each pass and stage, look up what map channel this face's material is using
			** - ask Max for the tfaces and tverts for that map channel
			** - copy the values into each vertex
			*/
			for (pass=0; pass<MaterialDesc.Pass_Count(); pass++) {
				for (stage=0; stage<W3dMaterialClass::MAX_STAGES; stage++) {

					/*
					** Start with a 0,0 uv coordinate
					*/
					UVVert tvert(0.0,0.0,0.0);

					W3dVertexMaterialStruct *vmat=MaterialDesc.Get_Vertex_Material(mat_index, pass);
					//Check if this material needs material color texture substitution.
					W3dMapClass *map3d=MaterialDesc.Get_Texture(mat_index,pass,stage);
					if (map3d && map3d->Filename && *((unsigned int *)map3d->Filename) == DIFFUSE_COLOR_TEXTURE_PREFIX)	//check for prefix
					{
						double Diffuse = vmat->Diffuse.Get_Color() >> 8;	//get material color

						//MW: Encode the material color into the u texture coordinate
						tvert.x=Diffuse;
						tvert.y=Diffuse;

						//find out material color location within texture page
						if (strnicmp(MaterialDesc.Get_Vertex_Material_Name(mat_index,pass),"HouseColor",10)==0)
							getMaterialUV(tvert,vmat->Diffuse.Get_Color() >> 8, materialColors, numMaterialColors, numHouseColors, true);
						else
							getMaterialUV(tvert,vmat->Diffuse.Get_Color() >> 8, materialColors, numMaterialColors, numHouseColors, false);

						//Keep track of first vertex material converted, so we can remap all other non-textured
						//materials to use the same material.
						if (firstSolidColoredMaterial == -1)
							firstSolidColoredMaterial=MaterialDesc.Get_Vertex_Material_Index(mat_index,pass);
					}

					/*
					** If the mesh needs uv coords and they are present, copy the uv into tvert
					*/
					else
					if (!use_simple_rendering(Header.Attributes)) {

						int channel = MaterialDesc.Get_Map_Channel(mat_index,pass,stage);

						UVVert * uvarray = mesh.mapVerts(channel);
						TVFace * tvfacearray = mesh.mapFaces(channel);

						///@todo: MW: Forced ingoring of uv coordinates if no texture!  Is this ok?
						W3dMapClass *map3d=MaterialDesc.Get_Texture(mat_index,pass,stage);

						if (map3d && (uvarray != nullptr) && (tvfacearray != nullptr)) {

							int tvert_index = tvfacearray[face_index].t[max_vert_counter];
							tvert = uvarray[tvert_index];
						}
					}

					/*
					** Copy the texture coordinate into the vertex structure
					*/
					face.Verts[vert_counter].TexCoord[pass][stage].X = tvert.x;
					face.Verts[vert_counter].TexCoord[pass][stage].Y = tvert.y;

				}
			}

			/*
			** Vertex Color
			*/
			if (mesh.vcFace) {

				/*
				** If the mesh is being mirrored, remap the index
				*/
				int max_cvert_index = mesh.vcFace[face_index].t[max_vert_counter];

				VertColor vc;
				vc = mesh.vertCol[max_cvert_index];

				/*
				** If Vertex Alpha is specified, the vertex color is converted
				** to alpha.  If the (obsolete) Node-flag for vertex alpha is enabled,
				** the alpha is put into each pass which has alpha enabled.  Otherwise,
				** we check the material settings for which passes should get the alpha
				** values.  If neither alpha options are enabled, we put the color
				** value into the first pass.
				*/
				if (ExportOptions.Is_Vertex_Alpha_Enabled()) {

					float	alpha = (vc.x + vc.y + vc.z) / 3.0f;
					for (int pass=0; pass < MaterialDesc.Pass_Count(); pass++) {
						if (MaterialDesc.Pass_Uses_Vertex_Alpha(pass)) {
							face.Verts[vert_counter].Alpha[pass] = alpha;
						}
					}
				} else {
					face.Verts[vert_counter].DiffuseColor[0].X = vc.x;
					face.Verts[vert_counter].DiffuseColor[0].Y = vc.y;
					face.Verts[vert_counter].DiffuseColor[0].Z = vc.z;
				}

				face.Verts[vert_counter].MaxVertColIndex = max_cvert_index;

			} else {
				face.Verts[vert_counter].MaxVertColIndex = 0;
			}

			/*
			** Vertex materials (get's index of sub-material)
			*/
			for (pass = 0; pass<MaterialDesc.Pass_Count(); pass++) {

				//if solid-color texture substitution, replace the vertex material with first solid one found.
				W3dMapClass *map3d=MaterialDesc.Get_Texture(mat_index,pass,0);
				if (map3d && map3d->Filename && *((unsigned int *)map3d->Filename) == DIFFUSE_COLOR_TEXTURE_PREFIX)	//check for prefix
					face.Verts[vert_counter].VertexMaterialIndex[pass]=firstSolidColoredMaterial;
				else
					face.Verts[vert_counter].VertexMaterialIndex[pass] = MaterialDesc.Get_Vertex_Material_Index(mat_index,pass);
			}

			face.Verts[vert_counter].Attribute0 = max_vert_index;
			face.Verts[vert_counter].Attribute1 = face_index;

			/*
			** Skin attachment
			*/
			face.Verts[vert_counter].BoneIndex = 0;
			if (is_skin) {

				int skin_bone_index = skindata->VertData[max_vert_index].BoneIdx[0];

				// If this is a valid bone, try to find the corresponding bone index in the HTree
				if (	(skin_bone_index != -1) &&
						(skin_bone_index < skinobj->Num_Bones()) &&
						(skinobj->BoneTab[skin_bone_index] != nullptr)	)
				{
					face.Verts[vert_counter].BoneIndex = get_htree_bone_index_for_inode(skinobj->BoneTab[skin_bone_index]);
				}
			}
		}

		Builder.Add_Face(face);
	}

	/*
	** Process the mesh
	*/
	Builder.Build_Mesh(true);

	const MeshBuilderClass::MeshStatsStruct & stats = Builder.Get_Mesh_Stats();
	int vcount = Builder.Get_Vertex_Count();
	int pcount = mesh.numFaces;
	float vert_poly_ratio = (float)vcount / (float)pcount;

	ExportLog::printf(" triangle count: %d\n",pcount);
	ExportLog::printf(" final vertex count: %d\n",vcount);
	ExportLog::printf(" vertex/triangle ratio: %f\n",vert_poly_ratio);
	ExportLog::printf(" strip count: %d\n",stats.StripCount);
	ExportLog::printf(" average strip length: %f\n",stats.AvgStripLength);
	ExportLog::printf(" longest strip: %d\n",stats.MaxStripLength);
}


/***********************************************************************************************
 * MeshSaveClass::get_skin_modifier_objects -- Searches for the WWSkin modifier for this mesh  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void MeshSaveClass::get_skin_modifier_objects(SkinDataClass ** skin_data_ptr,SkinWSMObjectClass ** skin_obj_ptr)
{
	*skin_data_ptr = nullptr;
	*skin_obj_ptr = nullptr;

	// loop through the references that our node has
	for (int i = 0; i < MaxINode->NumRefs(); i++) {

		ReferenceTarget *refTarg = MaxINode->GetReference(i);

		// if the reference is a WSM Derived Object.
		if (refTarg != nullptr && refTarg->ClassID() == Class_ID(WSM_DERIVOB_CLASS_ID,0)) {

			IDerivedObject * wsm_der_obj = (IDerivedObject *)refTarg;

			// loop through the WSM's attached to this WSM Derived object
			for (int j = 0; j < wsm_der_obj->NumModifiers(); j++) {
				Modifier * mod = wsm_der_obj->GetModifier(j);
				if (mod->ClassID() == SKIN_MOD_CLASS_ID) {

					// This is our modifier!  Get the data from it!
					SkinModifierClass * skinmod = (SkinModifierClass *)mod;
					ModContext * mc = wsm_der_obj->GetModContext(j);
					*skin_data_ptr = (SkinDataClass *)(mc->localData);
					*skin_obj_ptr = (SkinWSMObjectClass *)skinmod->GetReference(SkinModifierClass::OBJ_REF);
				}
			}
		}
	}
}


/***********************************************************************************************
 * MeshSaveClass::get_htree_bone_index_for_inode -- searches the htree for the given INode     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/1/2000   gth : Created.                                                                 *
 *=============================================================================================*/
int MeshSaveClass::get_htree_bone_index_for_inode(INode * node)
{
	// index of this INode in the hierarchy tree (it better be there :-)
	char w3dname[W3D_NAME_LEN];
	Set_W3D_Name(w3dname,node->GetName());
	int bindex = HTree->Find_Named_Node(w3dname);

	// If the desired bone isn't being exported, export the point
	// relative to the root.
	if (bindex == -1) {
		bindex = 0;
	}

	return bindex;
}

/***********************************************************************************************
 * MeshSaveClass::inv_deform_mesh -- preprocess the mesh for skinning                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *   5/1/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void MeshSaveClass::inv_deform_mesh()
{
	// Got the skinning info, now pre-deform each vertex and
	// create an array of vertex influence indexes.
	VertInfluences = new W3dVertInfStruct[Builder.Get_Vertex_Count()];
	memset(VertInfluences,0,sizeof(W3dVertInfStruct) * Builder.Get_Vertex_Count());
	Header.VertexChannels |= W3D_VERTEX_CHANNEL_BONEID;

	for (int vert_index = 0; vert_index < Builder.Get_Vertex_Count(); vert_index++) {

		MeshBuilderClass::VertClass & vert = Builder.Get_Vertex(vert_index);

		if (vert.BoneIndex == 0) {

			// set the influence to 0 (root node) and leave the vert and normal unchanged
			// (gth) 07/11/2000: Note that in this case, the mesh coordinates have already been
			// transformed relative to the user or scene origin and do not need to be further modified.
			VertInfluences[vert_index].BoneIdx = 0;

		} else {

			// here we go! get the matrix from the hierarchy tree and transform
			// the point into the bone's coordinate system.
			// (gth) 07/11/2000: The origin_offset_tm is no longer needed because
			// skin meshes are transformed into the origin coordinate system before
			// we get to this code.
			Matrix3 bonetm = HTree->Get_Node_Transform(vert.BoneIndex);
			Matrix3 invbonetm = Inverse(bonetm);
			Point3 pos;

			pos.x = vert.Position.X;
			pos.y = vert.Position.Y;
			pos.z = vert.Position.Z;

			pos = pos * invbonetm;

			vert.Position.X = pos.x;
			vert.Position.Y = pos.y;
			vert.Position.Z = pos.z;

			// Now, transform the normal into the bone's coordinate system.
			invbonetm.NoTrans();
			Point3 norm;

			norm.x = vert.Normal.X;
			norm.y = vert.Normal.Y;
			norm.z = vert.Normal.Z;

			norm = norm * invbonetm;

			vert.Normal.X = norm.x;
			vert.Normal.Y = norm.y;
			vert.Normal.Z = norm.z;

			VertInfluences[vert_index].BoneIdx = vert.BoneIndex;
		}
	}
}


/***********************************************************************************************
 * MeshSaveClass::Write_To_File -- Append the mesh to an open wtm file                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *    csave - ChunkSaveClass object to handle writing the chunk-based file 						  *
 * 	export_aabtree - should we generate an aabtree for this mesh                             *
 * 																														  *
 * OUTPUT:																												  *
 *    0 if nothing went wrong, Non-Zero otherwise															  *
 * 																														  *
 * WARNINGS:																											  *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int MeshSaveClass::Write_To_File(ChunkSaveClass & csave,bool export_aabtree)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_MESH)) {
		return 1;
	}

	if (write_header(csave) != 0) {
		return 1;
	}

	if (write_user_text(csave) != 0) {
		return 1;
	}

	if (write_verts(csave) != 0) {
		return 1;
	}

	if (write_vert_normals(csave) != 0) {
		return 1;
	}

	if (write_triangles(csave) != 0) {
		return 1;
	}

	if (write_vert_influences(csave) != 0) {
		return 1;
	}

	if (write_vert_shade_indices(csave) != 0) {
		return 1;
	}

	if (write_material_info(csave) != 0) {
		return 1;
	}

	if (write_vertex_materials(csave) != 0) {
		return 1;
	}

	if (PS2Material == TRUE) {

		// The ps2 shaders must be written out first.
		if (write_ps2_shaders(csave) != 0) {
			return 1;
		}
	}

	if (write_shaders(csave) != 0) {
		return 1;
	}


	if (write_textures(csave) != 0) {
		return 1;
	}

	for (int pass=0; pass<MaterialDesc.Pass_Count(); pass++) {
		if (write_pass(csave,pass) != 0) {
			return 1;
		}
	}

	if (DeformSave.Export (csave) != true) {
		return 1;
	}

	if (export_aabtree == true) {
		if (write_aabtree(csave) != 0) {
			return 1;
		}
	}

	if (!csave.End_Chunk()) {
		return 1;
	}

	return 0;
}

/***********************************************************************************************
 * MeshSaveClass::write_header -- write a mesh header chunk into a wtm file                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *    csave - chunk save object                    														  *
 * 																														  *
 * OUTPUT:																												  *
 *    0 if nothing went wrong, Non-Zero otherwise															  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int MeshSaveClass::write_header(ChunkSaveClass & csave)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_MESH_HEADER3)) {
		return 1;
	}

	if (csave.Write(&Header,sizeof(W3dMeshHeader3Struct)) != sizeof(W3dMeshHeader3Struct)) {
		return 1;
	}

	if (!csave.End_Chunk()) {
		return 1;
	}

	return 0;
}

/***********************************************************************************************
 * MeshSaveClass::write_user_text -- write the user text chunk                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/20/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int MeshSaveClass::write_user_text(ChunkSaveClass & csave)
{
	// If there's no user text, just don't write the chunk
	if (UserText == nullptr) {
		return 0;
	}

	if (!csave.Begin_Chunk(W3D_CHUNK_MESH_USER_TEXT)) {
		return 1;
	}

	// write the user text buffer (writing one extra byte to include the null terminator)
	if (csave.Write(UserText,strlen(UserText) + 1) != strlen(UserText) + 1) {
		return 1;
	}

	if (!csave.End_Chunk()) {
		return 1;
	}

	return 0;
}

/***********************************************************************************************
 * MeshSaveClass::write_verts -- write the vertex chunk into a wtm file                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *    csave - chunk save object                    														  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *    0 if nothing went wrong, Non-Zero otherwise															  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int MeshSaveClass::write_verts(ChunkSaveClass & csave)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_VERTICES)) {
		return 1;
	}

	assert(Builder.Get_Vertex_Count() > 0);
	assert(Builder.Get_Vertex_Count() == (int)Header.NumVertices);

	for (int i=0; i<Builder.Get_Vertex_Count(); i++) {

		const MeshBuilderClass::VertClass & vert = Builder.Get_Vertex(i);

		if ((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) != W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN) {

			assert(vert.Position.X <= Header.Max.X);
			assert(vert.Position.Y <= Header.Max.Y);
			assert(vert.Position.Z <= Header.Max.Z);

			assert(vert.Position.X >= Header.Min.X);
			assert(vert.Position.Y >= Header.Min.Y);
			assert(vert.Position.Z >= Header.Min.Z);

		}

		W3dVectorStruct w3dvert;
		w3dvert.X = vert.Position.X;
		w3dvert.Y = vert.Position.Y;
		w3dvert.Z = vert.Position.Z;

		if (csave.Write(&(w3dvert),sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct)) {
			return 1;
		}
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}


/***********************************************************************************************
 * MeshSaveClass::write_vert_normals -- Writes the surrender normal chunk into a wtm file      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *    csave - chunk save object                    														  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *    0 if nothing went wrong, Non-Zero otherwise															  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int MeshSaveClass::write_vert_normals(ChunkSaveClass & csave)
{
	if (!(Header.VertexChannels & W3D_VERTEX_CHANNEL_NORMAL)) return 0;

	if (!csave.Begin_Chunk(W3D_CHUNK_VERTEX_NORMALS)) {
		return 1;
	}

	for (int i=0; i < Builder.Get_Vertex_Count(); i++) {

		const MeshBuilderClass::VertClass & vert = Builder.Get_Vertex(i);

		W3dVectorStruct norm;

		if (ExportOptions.Is_ZNormals_Enabled()) {
			norm.X = 0.0f;
			norm.Y = 0.0f;
			norm.Z = 1.0f;
		} else {
			norm.X = vert.Normal.X;
			norm.Y = vert.Normal.Y;
			norm.Z = vert.Normal.Z;
		}

		if (csave.Write(&(norm),sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct)) {
			return 1;
		}
	}

	if (!csave.End_Chunk()) {
		return 1;
	}

	return 0;
}

/***********************************************************************************************
 * MeshSaveClass::write_vert_influences -- skins will have this chunk that binds verts to bones*
 *                                                                                             *
 * INPUT:                                                                                      *
 *    csave - chunk save object                 															  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *    0 if nothing went wrong, Non-Zero otherwise															  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int MeshSaveClass::write_vert_influences(ChunkSaveClass & csave)
{
	if (((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK) != W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN) ||
		 !(Header.VertexChannels & W3D_VERTEX_CHANNEL_BONEID) ||
		 (VertInfluences == nullptr)) {
		return 0;
	}

	if (!csave.Begin_Chunk(W3D_CHUNK_VERTEX_INFLUENCES)) {
		return 1;
	}

	int count = Builder.Get_Vertex_Count();
	if (csave.Write(VertInfluences,count * sizeof(W3dVertInfStruct)) != count * sizeof(W3dVertInfStruct)) {
		return 1;
	}

	if (!csave.End_Chunk()) {
		return 1;
	}

	return 0;
}


/***********************************************************************************************
 * MeshSaveClass::write_triangles -- write the triangles chunk                                 *
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
int MeshSaveClass::write_triangles(ChunkSaveClass & csave)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_TRIANGLES)) {
		return 1;
	}

	assert(Builder.Get_Face_Count() == (int)Header.NumTris);
	for (int i=0; i<Builder.Get_Face_Count(); i++) {

		const MeshBuilderClass::FaceClass & face = Builder.Get_Face(i);

		W3dTriStruct tri;
		memset(&tri,0,sizeof(W3dTriStruct));

		// convert each triangle into surrender format
		tri.Vindex[0] =			face.VertIdx[0];
		tri.Vindex[1] = 			face.VertIdx[1];
		tri.Vindex[2] = 			face.VertIdx[2];

		tri.Attributes =			face.SurfaceType;
		tri.Normal.X = 			face.Normal.X;
		tri.Normal.Y = 			face.Normal.Y;
		tri.Normal.Z = 			face.Normal.Z;
		tri.Dist =					face.Dist;

		// write each triangle
		if (csave.Write(&tri,sizeof(W3dTriStruct)) != sizeof(W3dTriStruct)) {
			return 1;
		}
	}

	if (!csave.End_Chunk()) {
		return 1;
	}

	return 0;
}

int MeshSaveClass::write_vert_shade_indices(ChunkSaveClass & csave)
{
	if (!(Header.VertexChannels & W3D_VERTEX_CHANNEL_NORMAL)) return 0;

	if (!csave.Begin_Chunk(W3D_CHUNK_VERTEX_SHADE_INDICES)) {
		return 1;
	}

	for (int i=0; i < Builder.Get_Vertex_Count(); i++) {

		const MeshBuilderClass::VertClass & vert = Builder.Get_Vertex(i);

		uint32 shade_index = vert.ShadeIndex;

		if (csave.Write(&(shade_index),sizeof(shade_index)) != sizeof(shade_index)) {
			return 1;
		}
	}

	if (!csave.End_Chunk()) {
		return 1;
	}

	return 0;
}

int MeshSaveClass::write_material_info(ChunkSaveClass & csave)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_MATERIAL_INFO)) {
		return 1;
	}

	W3dMaterialInfoStruct matinfo;
	matinfo.PassCount = MaterialDesc.Pass_Count();
	matinfo.VertexMaterialCount = MaterialDesc.Vertex_Material_Count();
	matinfo.ShaderCount = MaterialDesc.Shader_Count();
	matinfo.TextureCount = MaterialDesc.Texture_Count();

	if (csave.Write(&matinfo,sizeof(matinfo)) != sizeof(matinfo)) {
		return 1;
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}

int MeshSaveClass::write_vertex_materials(ChunkSaveClass & csave)
{
	if (MaterialDesc.Vertex_Material_Count() <= 0) return 0;

	// open a chunk which wraps all of the vertex materials
	if (!csave.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIALS)) {
		return 1;
	}

	for (int i=0; i<MaterialDesc.Vertex_Material_Count(); i++) {

		csave.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL);

		// write the filename
		const char * name = MaterialDesc.Get_Vertex_Material_Name(i);
		if (name != nullptr) {
			csave.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL_NAME);
			if (csave.Write(name,strlen(name) + 1) != strlen(name) + 1) {
				return 1;
			}
			csave.End_Chunk();
		}

		// write the struct
		W3dVertexMaterialStruct * vmat = MaterialDesc.Get_Vertex_Material(i);
		csave.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL_INFO);
		if (csave.Write(vmat,sizeof(W3dVertexMaterialStruct)) != sizeof(W3dVertexMaterialStruct)) {
			return 1;
		}
		csave.End_Chunk();

		// write the mapper args
		const char * args = MaterialDesc.Get_Mapper_Args(i, 0);
		if (args != nullptr) {
			csave.Begin_Chunk(W3D_CHUNK_VERTEX_MAPPER_ARGS0);
			if (csave.Write(args,strlen(args) + 1) != strlen(args) + 1) {
				return 1;
			}
			csave.End_Chunk();
		}
		args = MaterialDesc.Get_Mapper_Args(i, 1);
		if (args != nullptr) {
			csave.Begin_Chunk(W3D_CHUNK_VERTEX_MAPPER_ARGS1);
			if (csave.Write(args,strlen(args) + 1) != strlen(args) + 1) {
				return 1;
			}
			csave.End_Chunk();
		}

		csave.End_Chunk();
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}

int MeshSaveClass::write_shaders(ChunkSaveClass & csave)
{
	assert(MaterialDesc.Shader_Count() > 0);

	if (PS2Material == TRUE) {

		// Make the PC shader as close to the PS2 shader as possible.
		// This will allow for viewing in W3D View.
		setup_PC_shaders_from_PS2_shaders();
	}

	if (!csave.Begin_Chunk(W3D_CHUNK_SHADERS)) {
		return 1;
	}

	W3dShaderStruct * shader;

	for (int i=0; i<MaterialDesc.Shader_Count(); i++) {
		shader = MaterialDesc.Get_Shader(i);
		if (csave.Write(shader,sizeof(W3dShaderStruct)) != sizeof(W3dShaderStruct)) {
			return 1;
		}
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}


/***********************************************************************************************
 * MeshSaveClass::write_ps2_shaders -- Write shaders specific to the PS2 in their own chunk.   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/1999MLL: Created.                                                                   *
 *=============================================================================================*/
int MeshSaveClass::write_ps2_shaders(ChunkSaveClass & csave)
{

	if (!csave.Begin_Chunk(W3D_CHUNK_PS2_SHADERS)) {
		return 1;
	}

	W3dShaderStruct *shader;
	W3dPS2ShaderStruct ps2shader;

	for (int i=0; i<MaterialDesc.Shader_Count(); i++) {
		shader = MaterialDesc.Get_Shader(i);

		ps2shader.AlphaTest = W3d_Shader_Get_Alpha_Test(shader);
		ps2shader.AParam = W3d_Shader_Get_PS2_Param_A(shader);
		ps2shader.BParam = W3d_Shader_Get_PS2_Param_B(shader);
		ps2shader.CParam = W3d_Shader_Get_PS2_Param_C(shader);
		ps2shader.DParam = W3d_Shader_Get_PS2_Param_D(shader);

		// Write out the PS2 native form.
		switch (W3d_Shader_Get_Depth_Compare(shader))
		{
			case PSS_DEPTHCOMPARE_PASS_NEVER:
				ps2shader.DepthCompare = 0;
				break;
			case PSS_DEPTHCOMPARE_PASS_LESS:
				ps2shader.DepthCompare = 3;
				break;
			case PSS_DEPTHCOMPARE_PASS_ALWAYS:
				ps2shader.DepthCompare = 1;
				break;
			case PSS_DEPTHCOMPARE_PASS_LEQUAL:
				ps2shader.DepthCompare = 2;
				break;
		}

		ps2shader.DepthMask = !W3d_Shader_Get_Depth_Mask(shader);

		switch (W3d_Shader_Get_Pri_Gradient(shader))
		{
			case PSS_PRIGRADIENT_MODULATE:
				ps2shader.PriGradient = PSS_PS2_PRIGRADIENT_MODULATE;
				break;

			case PSS_PRIGRADIENT_HIGHLIGHT:
				ps2shader.PriGradient = PSS_PS2_PRIGRADIENT_HIGHLIGHT;
				break;

			case PSS_PRIGRADIENT_HIGHLIGHT2:
				ps2shader.PriGradient = PSS_PS2_PRIGRADIENT_HIGHLIGHT2;
				break;

			case PSS_PRIGRADIENT_DECAL:
				ps2shader.PriGradient = PSS_PS2_PRIGRADIENT_DECAL;
				break;

		}

		ps2shader.Texturing = W3d_Shader_Get_Texturing(shader);

		if (csave.Write(&ps2shader,sizeof(W3dPS2ShaderStruct)) != sizeof(W3dPS2ShaderStruct)) {
			return 1;
		}
	}

	if (!csave.End_Chunk()) {
		return 1;
	}

	return (0);
}

void MeshSaveClass::setup_PC_shaders_from_PS2_shaders()
{

	W3dShaderStruct *shader;

	for (int i=0; i<MaterialDesc.Shader_Count(); i++) {
		shader = MaterialDesc.Get_Shader(i);

		// DepthCompare on the PS2 doesn't have as many options as on the PC.
		switch (W3d_Shader_Get_Depth_Compare(shader))
		{
			case PSS_DEPTHCOMPARE_PASS_NEVER:
				W3d_Shader_Set_Depth_Compare(shader, W3DSHADER_DEPTHCOMPARE_PASS_NEVER);
				break;
			case PSS_DEPTHCOMPARE_PASS_LESS:
				W3d_Shader_Set_Depth_Compare(shader, W3DSHADER_DEPTHCOMPARE_PASS_LESS);
				break;
			case PSS_DEPTHCOMPARE_PASS_LEQUAL:
				W3d_Shader_Set_Depth_Compare(shader, W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL);
				break;
			case PSS_DEPTHCOMPARE_PASS_ALWAYS:
				W3d_Shader_Set_Depth_Compare(shader, W3DSHADER_DEPTHCOMPARE_PASS_ALWAYS);
				break;
		}

		// The PS2 has more gradient functions than the PC.
		switch (W3d_Shader_Get_Pri_Gradient(shader))
		{
			case PSS_PRIGRADIENT_MODULATE:
				W3d_Shader_Set_Pri_Gradient(shader, W3DSHADER_PRIGRADIENT_MODULATE);
				break;

			// No PC equivalent.  Set to default: modulate.
			case PSS_PRIGRADIENT_HIGHLIGHT:
			case PSS_PRIGRADIENT_HIGHLIGHT2:
			case PSS_PRIGRADIENT_DECAL:
				W3d_Shader_Set_Pri_Gradient(shader, W3DSHADER_PRIGRADIENT_MODULATE);
				break;
		}


	}

}

int MeshSaveClass::write_textures(ChunkSaveClass & csave)
{
	if (MaterialDesc.Texture_Count() <= 0) return 0;

	// open a chunk which wraps all textures
	if (!csave.Begin_Chunk(W3D_CHUNK_TEXTURES)) {
		return 1;
	}

	for (int i=0; i<MaterialDesc.Texture_Count(); i++) {

		W3dMapClass * map = MaterialDesc.Get_Texture(i);

		csave.Begin_Chunk(W3D_CHUNK_TEXTURE);

		// write the filename
		csave.Begin_Chunk(W3D_CHUNK_TEXTURE_NAME);
		if (csave.Write(map->Filename,strlen(map->Filename) + 1) != strlen(map->Filename) + 1) return 1;
		csave.End_Chunk();

		// optionally write an animation info chunk
		if (map->AnimInfo != nullptr) {
			csave.Begin_Chunk(W3D_CHUNK_TEXTURE_INFO);
			if (csave.Write(map->AnimInfo,sizeof(W3dTextureInfoStruct)) != sizeof(W3dTextureInfoStruct)) return 1;
			csave.End_Chunk();
		}

		csave.End_Chunk();
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}

int MeshSaveClass::write_pass(ChunkSaveClass & csave,int pass)
{
	assert(pass >= 0);
	assert(pass < MaterialDesc.Pass_Count());

	if (!csave.Begin_Chunk(W3D_CHUNK_MATERIAL_PASS)) {
		return 1;
	}

	const MeshBuilderClass::MeshStatsStruct & stats = Builder.Get_Mesh_Stats();

	if (stats.HasVertexMaterial[pass]) {
		write_vertex_material_ids(csave,pass);
	}

	if (stats.HasShader[pass]) {
		write_shader_ids(csave,pass);
	}

	if (stats.HasDiffuseColor[pass] || DeformSave.Does_Deformer_Modify_DCG ()) {
		write_dcg(csave,pass);
	}

	for (int stage = 0; stage < MeshBuilderClass::MAX_STAGES; stage++) {
		write_texture_stage(csave,pass,stage);
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}

int MeshSaveClass::write_vertex_material_ids(ChunkSaveClass & csave,int pass)
{
	const MeshBuilderClass::MeshStatsStruct & stats = Builder.Get_Mesh_Stats();

	if (!csave.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL_IDS)) {
		return 1;
	}

	uint32 matid;

	if (stats.HasPerVertexMaterial[pass]) {
		for (int i=0; i<Builder.Get_Vertex_Count(); i++) {
			matid = Builder.Get_Vertex(i).VertexMaterialIndex[pass];
			if (csave.Write(&matid,sizeof(uint32)) != sizeof(uint32)) return 1;
		}
	} else {
		matid = Builder.Get_Vertex(0).VertexMaterialIndex[pass];
		if (csave.Write(&matid,sizeof(uint32)) != sizeof(uint32)) return 1;
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}

int MeshSaveClass::write_shader_ids(ChunkSaveClass & csave,int pass)
{
	const MeshBuilderClass::MeshStatsStruct & stats = Builder.Get_Mesh_Stats();

	if (!csave.Begin_Chunk(W3D_CHUNK_SHADER_IDS)) {
		return 1;
	}

	uint32 shaderid;
	if (stats.HasPerPolyShader[pass]) {
		for (int i=0; i<Builder.Get_Face_Count(); i++) {
			shaderid = Builder.Get_Face(i).ShaderIndex[pass];
			if (csave.Write(&shaderid,sizeof(uint32)) != sizeof(uint32)) return 1;
		}
	} else {
		shaderid = Builder.Get_Face(0).ShaderIndex[pass];
		if (csave.Write(&shaderid,sizeof(uint32)) != sizeof(uint32)) return 1;
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}

int MeshSaveClass::write_dcg(ChunkSaveClass & csave,int pass)
{
	const MeshBuilderClass::MeshStatsStruct & stats = Builder.Get_Mesh_Stats();

	if (!csave.Begin_Chunk(W3D_CHUNK_DCG)) {
		return 1;
	}

	for (int i=0; i<Builder.Get_Vertex_Count(); i++) {
		Vector3 vcolor = Builder.Get_Vertex(i).DiffuseColor[pass];
		W3dRGBAStruct color;
		color.R = (uint8)(255.0f * vcolor.X);
		color.G = (uint8)(255.0f * vcolor.Y);
		color.B = (uint8)(255.0f * vcolor.Z);

		float a = Builder.Get_Vertex(i).Alpha[pass];
		color.A = (uint8)(255.0f * a);

		if (csave.Write(&(color),sizeof(W3dRGBAStruct)) != sizeof(W3dRGBAStruct)) {
			return 1;
		}
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}

int MeshSaveClass::write_texture_stage(ChunkSaveClass & csave,int pass,int stage)
{
	const MeshBuilderClass::MeshStatsStruct & stats = Builder.Get_Mesh_Stats();
	if (!stats.HasTexture[pass][stage]) return 0;

	if (!csave.Begin_Chunk(W3D_CHUNK_TEXTURE_STAGE)) {
		return 1;
	}

	write_texture_ids(csave,pass,stage);
	write_texture_coords(csave,pass,stage);

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}


int MeshSaveClass::write_texture_ids(ChunkSaveClass & csave,int pass,int stage)
{
	const MeshBuilderClass::MeshStatsStruct & stats = Builder.Get_Mesh_Stats();

	if (!csave.Begin_Chunk(W3D_CHUNK_TEXTURE_IDS)) {
		return 1;
	}

	uint32 texid;
	if (stats.HasPerPolyTexture[pass][stage]) {
		for (int i=0; i<Builder.Get_Face_Count(); i++) {
			texid = Builder.Get_Face(i).TextureIndex[pass][stage];
			if (csave.Write(&texid,sizeof(uint32)) != sizeof(uint32)) return 1;
		}
	} else {
		texid = Builder.Get_Face(0).TextureIndex[pass][stage];
		if (csave.Write(&texid,sizeof(uint32)) != sizeof(uint32)) return 1;
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}

int MeshSaveClass::write_texture_coords(ChunkSaveClass & csave,int pass,int stage)
{
	const MeshBuilderClass::MeshStatsStruct & stats = Builder.Get_Mesh_Stats();
	if (stats.HasTexCoords[pass][stage] == false) return 0;

	if (!csave.Begin_Chunk(W3D_CHUNK_STAGE_TEXCOORDS)) {
		return 1;
	}

	for (int i=0; i<Builder.Get_Vertex_Count(); i++) {
		W3dTexCoordStruct tex;
		tex.U = Builder.Get_Vertex(i).TexCoord[pass][stage].X;
		tex.V = Builder.Get_Vertex(i).TexCoord[pass][stage].Y;

		if (csave.Write(&(tex),sizeof(W3dTexCoordStruct)) != sizeof(W3dTexCoordStruct)) {
			return 1;
		}
	}

	if (!csave.End_Chunk()) {
		return 1;
	}
	return 0;
}

int MeshSaveClass::write_aabtree(ChunkSaveClass & csave)
{
	/*
	** Don't bother with an AABTree if this not a normal mesh or if it has very few polygons
	*/
	if (	(Builder.Get_Face_Count() >= MIN_AABTREE_POLYGONS) &&
			((Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK)  == W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL)
		)
	{

		/*
		** Build temporary array representations of the mesh
		*/
		int vertcount = Builder.Get_Vertex_Count();
		int polycount = Builder.Get_Face_Count();
		Vector3 * verts = new Vector3[vertcount];
		Vector3i * polys = new Vector3i[polycount];

		for (int vi=0; vi<vertcount; vi++) {
			verts[vi] = Builder.Get_Vertex(vi).Position;
		}

		for (int pi=0; pi<polycount; pi++) {
			polys[pi].I = Builder.Get_Face(pi).VertIdx[0];
			polys[pi].J = Builder.Get_Face(pi).VertIdx[1];
			polys[pi].K = Builder.Get_Face(pi).VertIdx[2];
		}

		/*
		** Build the AABTree and save it
		*/
		AABTreeBuilderClass aabtree_builder;
		aabtree_builder.Build_AABTree(polycount,polys,vertcount,verts);
		aabtree_builder.Export(csave);

		/*
		** Release the allocated resources
		*/
		delete[] verts;
		delete[] polys;
	}

	return 0;
}

int MeshSaveClass::scan_used_materials(Mesh & mesh,Mtl * nodemtl)
{
	int face_index;
	int mat_index;

	if ((nodemtl == nullptr) || (nodemtl->NumSubMtls() <= 1)) {

		MaterialRemapTable = new int[1];
		MaterialRemapTable[0] = 0;
		return 1;

	} else {

		int sub_mtl_count = nodemtl->NumSubMtls();
		MaterialRemapTable = new int[sub_mtl_count];

		// Initialize each remap to -1 (indicates that the material is un-used)
		for (mat_index=0; mat_index<sub_mtl_count; mat_index++) {
			MaterialRemapTable[mat_index] = -1;
		}

		// Set a 1 for each material actually referenced by the mesh
		for (face_index=0; face_index<mesh.getNumFaces(); face_index++) {
			int max_mat_index = mesh.faces[face_index].getMatID();
			int mat_index = (max_mat_index % sub_mtl_count);
			MaterialRemapTable[mat_index] = 1;
		}

		// Replace each 1 entry with an incrementing material index
		int matcount = 0;
		for (mat_index=0; mat_index<sub_mtl_count; mat_index++) {
			if (MaterialRemapTable[mat_index] == 1) {
				MaterialRemapTable[mat_index] = matcount;
				matcount++;
			}
		}

		assert(matcount > 0);
		return matcount;
	}
}

//Check if material is solid, non-textured.
int isTexturedMaterial(Mtl * mtl)
{
	Texmap * tmap;

	tmap = mtl->GetSubTexmap(ID_DI);
	if (mtl->ClassID() == GameMaterialClassID)
	{
		GameMtl * gamemtl=(GameMtl *)mtl;

		for (int pass=0;pass<gamemtl->Get_Pass_Count(); pass++)
		{
			for (int stage=0; stage < W3dMaterialClass::MAX_STAGES; stage++)
			{	if (gamemtl->Get_Texture_Enable(pass,stage) && gamemtl->Get_Texture(pass,stage))
					return 1;	//using a texture
			}
		}
		return 0;
	}
	else
	{
		return (tmap && tmap->ClassID() == Class_ID(BMTEX_CLASS_ID,0));
	}
}

//count number of used solid materials (no texture)
int MeshSaveClass::getNumSolidMaterials(Mtl * nodemtl)
{
	int mat_index;
	int numSolid=0;

	if ((nodemtl == nullptr) || (nodemtl->NumSubMtls() <= 1))
	{	//Check if diffuse texture present
		if (isTexturedMaterial(nodemtl))
			return 00;
		return 1;
	}
	else
	{
		int sub_mtl_count = nodemtl->NumSubMtls();

		for (mat_index=0; mat_index<sub_mtl_count; mat_index++)
		{
			if (MaterialRemapTable[mat_index] != -1)
			{	//material is used on some faces
				if (isTexturedMaterial(nodemtl->GetSubMtl(mat_index)))
					continue;
				numSolid++;
			}
		}
		return numSolid;
	}
}

//Cancels out the material colors by setting them to white.
//Also changes prefix on texture names to use DIFFUSE_HOUSECOLOR_TEXTURE_PREFIX
//if house color is used.
void MeshSaveClass::fix_diffuse_materials(bool isHouseColor)
{
	uint8 color=255;
	char	materialColorFilename[_MAX_FNAME + 1];

	for (int mat_index=0; mat_index<MaterialDesc.Material_Count(); mat_index++)
	{
		for (int pass=0; pass<MaterialDesc.Pass_Count(); pass++)
		{
			W3dVertexMaterialStruct *vmat=MaterialDesc.Get_Vertex_Material(mat_index,pass);

			for (int stage=0; stage<W3dMaterialClass::MAX_STAGES; stage++)
			{
				W3dMapClass *map3d=MaterialDesc.Get_Texture(mat_index,pass,stage);
				if (map3d && map3d->Filename && (*((unsigned int *)map3d->Filename) & 0xffff00ff) == DIFFUSE_COLOR_TEXTURE_MASK)	//check for 'TXC^' prefix
				{	//found a material which had its material color replaced by a texture
					//set the material color to white so it's not being used.
					vmat->Diffuse.Set(color,color,color);
					vmat->Ambient.Set(color,color,color);

					if (isHouseColor && *(map3d->Filename+1) != 'H')
					{	//our material texture contains house colors, adjust the name
						//by adding H prefix.
						sprintf(materialColorFilename,"%s%s","ZHCD",map3d->Filename+4);
						map3d->Set_Filename(materialColorFilename);
					}
				}
			}
		}
	}
}

/***********************************************************************************************
 * MeshSaveClass::create_materials -- create the materials for this mesh                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *   2/8/99     GTH : modified to use the MaterialRemapTable                                   *
 *=============================================================================================*/
void MeshSaveClass::create_materials(Mtl * nodemtl,DWORD wirecolor, char *materialColorTexture)
{
	bool domaps = !use_simple_rendering(Header.Attributes);

	//////////////////////////////////////////////////////////////////////
	// Create materials
	// Four cases:
	// - Creating a collision object: use a hard-coded material
	// - The material is null: use wire color
	// - The material is a simple material: create one material
	// - The material is a multi-material: create n materials
	//////////////////////////////////////////////////////////////////////
	int geo_type = Header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;
	if ((geo_type == W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX) ||
			(geo_type == W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX))
	{
		Header.NumMaterials = 1;

		W3dVertexMaterialStruct vmat;
		W3dShaderStruct shader;
		W3dMaterialClass material;

		Color diffuse;
		if (Header.Attributes & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL) {
			diffuse.r = 0.0f;
			diffuse.g = 0.0f;
			diffuse.b = 1.0f;
		} else {
			diffuse.r = 0.4f;
			diffuse.g = 1.0f;
			diffuse.b = 0.4f;
		}

		W3d_Shader_Reset(&shader);
		W3d_Shader_Set_Dest_Blend_Func(&shader,W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA);
		W3d_Shader_Set_Src_Blend_Func(&shader,W3DSHADER_SRCBLENDFUNC_SRC_ALPHA);

		W3d_Vertex_Material_Reset(&vmat);
		vmat.Opacity = 0.5f;

		// add material 0
		vmat.Diffuse.R = (uint8)(diffuse.r * 255.0f);
		vmat.Diffuse.G = (uint8)(diffuse.g * 255.0f);
		vmat.Diffuse.B = (uint8)(diffuse.b * 255.0f);
		material.Set_Pass_Count(1);
		material.Set_Vertex_Material(vmat,0);
		material.Set_Shader(shader,0);
		MaterialDesc.Add_Material(material);

		assert(MaterialDesc.Material_Count() == 1);

	} else if (!nodemtl) {

		// Create a single material using the wire color
		Header.NumMaterials = 1;

		W3dVertexMaterialStruct vmat;
		W3dShaderStruct shader;
		W3dMaterialClass material;

		W3d_Vertex_Material_Reset(&vmat);
		W3d_Shader_Reset(&shader);

		vmat.Diffuse.R = GetRValue(wirecolor);
		vmat.Diffuse.G = GetGValue(wirecolor);
		vmat.Diffuse.B = GetBValue(wirecolor);

		material.Set_Pass_Count(1);
		material.Set_Vertex_Material(vmat,0);
		material.Set_Shader(shader,0);

		MaterialDesc.Add_Material(material,"WireColor");
		assert(MaterialDesc.Material_Count() == 1);

	} else if (!nodemtl->IsMultiMtl()) {

		Header.NumMaterials = 1;
		W3dMaterialClass mat;

		if (isTexturedMaterial(nodemtl) == 0)
			mat.Init(nodemtl,materialColorTexture);
		else
			mat.Init(nodemtl,nullptr);

		W3dMaterialDescClass::ErrorType err;
		err = MaterialDesc.Add_Material(mat,nodemtl->GetName());
		if (err == W3dMaterialDescClass::MULTIPASS_TRANSPARENT) {
			sprintf(_string1,"Exporting Materials for Mesh: %s\nMaterial %s is multi-pass and transparent\nMulti-pass transparent materials are not allowed.\n",
					Header.MeshName,
					nodemtl->GetName());
			ExportLog::printf(_string1);
			throw ErrorClass(_string1);
		}
		assert(MaterialDesc.Material_Count() == 1);

	} else {

		Header.NumMaterials = nodemtl->NumSubMtls();
		W3dMaterialClass mat;

		for (unsigned mi = 0; mi < Header.NumMaterials; mi++) {

			// only process materials that were found to be used in the scan_used_materials call
			if (MaterialRemapTable[mi] != -1) {

				if (isTexturedMaterial(nodemtl->GetSubMtl(mi)) == 0)
					mat.Init(nodemtl->GetSubMtl(mi),materialColorTexture);
				else
					mat.Init(nodemtl->GetSubMtl(mi),nullptr);

				char * name;
				W3dMaterialDescClass::ErrorType err;

				name = nodemtl->GetSubMtl(mi)->GetName();
				err = MaterialDesc.Add_Material(mat,name);

				if (err == W3dMaterialDescClass::INCONSISTENT_PASSES) {
					sprintf(_string1,"Exporting Materials for Mesh: %s\nMaterial %s has %d passes.\nThe other materials have %d passes.\nAll Materials must have the same number of passes.\n",
							Header.MeshName,
							nodemtl->GetSubMtl(mi)->GetName(),
							mat.Get_Pass_Count(),
							MaterialDesc.Pass_Count());
					ExportLog::printf(_string1);
					throw ErrorClass(_string1);
				}
				if (err == W3dMaterialDescClass::MULTIPASS_TRANSPARENT) {
					sprintf(_string1,"Exporting Materials for Mesh: %s\nMaterial %s is multi-pass and transparent\nMulti-pass transparent materials are not allowed.\n",
							Header.MeshName,
							nodemtl->GetSubMtl(mi)->GetName());
					throw ErrorClass(_string1);
				}
				if (err == W3dMaterialDescClass::INCONSISTENT_SORT_LEVEL) {
					sprintf(_string1,"Exporting Materials for Mesh: %s\nMaterial %s does not have the same Static Sort Level as other materials used on the mesh.\nAll materials for a mesh must use the same Static Sort Level value.\n",
							Header.MeshName,
							nodemtl->GetSubMtl(mi)->GetName());
					ExportLog::printf(_string1);
					throw ErrorClass(_string1);
				}
			}
		}
	}

	// Store the material's sort level in the mesh header.
	Header.SortLevel = MaterialDesc.Get_Sort_Level();
}

/***********************************************************************************************
 * MeshSaveClass::compute_bounding_volumes -- computes a bounding box and bounding sphere for  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/01/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void MeshSaveClass::compute_bounding_volumes(void)
{
	Vector3 min,max,center;
	float radius;

	Builder.Compute_Bounding_Box(&min,&max);
	Builder.Compute_Bounding_Sphere(&center,&radius);

	Header.SphCenter.X = center.X;
	Header.SphCenter.Y = center.Y;
	Header.SphCenter.Z = center.Z;
	Header.SphRadius = radius;

	Header.Min.X = min.X;
	Header.Min.Y = min.Y;
	Header.Min.Z = min.Z;
	Header.Max.X = max.X;
	Header.Max.Y = max.Y;
	Header.Max.Z = max.Z;
}


/***********************************************************************************************
 * MeshSaveClass::compute_physical_properties -- computes the volume and moment of inertia     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/01/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void MeshSaveClass::compute_physical_constants
(
	INode * inode,
	Progress_Meter_Class & meter,
	bool voxelize
)
{

#if 0 // IF we need this again, move the data to a physics chunk (header doesn't have it anymore)

	if (voxelize) {

		// Create an INodeList object for this mesh
		AnyINodeFilter nodefilt;
		INodeListClass meshlist(CurTime,&nodefilt);
		meshlist.Insert(inode);

		// Create a Voxel object for this mesh
		VoxelClass * voxel = new VoxelClass
 		(
 			meshlist,
 			VOXEL_RESOLUTION,
 			ExportSpace,
 			CurTime,
 			meter
 		);

#if DEBUG_VOXELS
		VoxelDebugWindowClass dbgwin(voxel);
 		dbgwin.Display_Window();
#endif

		double vol[1];
		double cm[3];
		double inertia[9];

		voxel->Compute_Physical_Properties(vol,cm,inertia);

		Header.Volume = (float)vol[0];

		Header.MassCenter.X = (float)cm[0];
		Header.MassCenter.Y = (float)cm[1];
		Header.MassCenter.Z = (float)cm[2];

		Header.Inertia[0] = (float)inertia[0];
		Header.Inertia[1] = (float)inertia[1];
		Header.Inertia[2] = (float)inertia[2];
		Header.Inertia[3] = (float)inertia[3];
		Header.Inertia[4] = (float)inertia[4];
		Header.Inertia[5] = (float)inertia[5];
		Header.Inertia[6] = (float)inertia[6];
		Header.Inertia[7] = (float)inertia[7];
		Header.Inertia[8] = (float)inertia[8];

	} else {

		// Set mass center to the center of the bounding box
		Header.MassCenter.X = (Header.Max.X + Header.Min.X) / 2.0f;
		Header.MassCenter.Y = (Header.Max.Y + Header.Min.Y) / 2.0f;
		Header.MassCenter.Z = (Header.Max.Z + Header.Min.Z) / 2.0f;

		// Set inertia tensor to inertia tensor of the bounding box
		// (gth) !!!! DO THIS !!!!
		Header.Inertia[0] = 1.0f;
		Header.Inertia[1] = 0.0f;
		Header.Inertia[2] = 0.0f;
		Header.Inertia[3] = 0.0f;
		Header.Inertia[4] = 1.0f;
		Header.Inertia[5] = 0.0f;
		Header.Inertia[6] = 0.0f;
		Header.Inertia[7] = 0.0f;
		Header.Inertia[8] = 1.0f;

		Header.Volume = (Header.Max.X - Header.Min.X) * (Header.Max.Y - Header.Min.Y) * (Header.Max.Z - Header.Min.Z);
	}

#endif
}


/***********************************************************************************************
 * MeshSaveClass::prep_mesh -- pre-transform the MAX mesh by a specified matrix                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/01/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void MeshSaveClass::prep_mesh(Mesh & mesh,Matrix3 & objoff)
{
	int vert_index;

	// Transform the mesh's vertices so that they are relative to the coordinate
	// system that we want to use with the mesh
	for (vert_index = 0; vert_index < mesh.getNumVerts (); vert_index++) {
		mesh.verts[vert_index] = mesh.verts[vert_index] * objoff;
	}

	// Re-Build the normals.
	mesh.buildNormals();
}


