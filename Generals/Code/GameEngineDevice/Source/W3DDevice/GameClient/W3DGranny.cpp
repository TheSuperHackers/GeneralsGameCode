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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DGranny.cpp ////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: W3DGranny.cpp
//
// Created:   Mark Wilczynski, Sep 2001
//
// Desc:      Methods for using Granny models within the W3D engine.
//-----------------------------------------------------------------------------

#ifdef INCLUDE_GRANNY_IN_BUILD

#include "W3DDevice/GameClient/W3DGranny.h"
#include "Common/GlobalData.h"
#include "texture.h"
#include "colmath.h"
#include "coltest.h"
#include "rinfo.h"
#include "camera.h"
#include "assetmgr.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/scene.h"

#pragma comment( lib, "granny2" )

static granny_pnt332_vertex g_blendingBuffer[4096];	///<temporary workspace for granny (all models < 4096 verts).

/** Local function used to find animation matching the given model */
static int FindTrackGroupFor(granny_animation *Animation, granny_model_instance *ModelInstance)
{
    if(Animation && ModelInstance)
    {        
        {for(Int TrackGroupIndex = 0;
             TrackGroupIndex < Animation->TrackGroupCount;
             ++TrackGroupIndex)
        {
            if(strcmp(Animation->TrackGroups[TrackGroupIndex]->Name,
                      GrannyGetSourceModel(ModelInstance)->Name) == 0)
            {
                return(TrackGroupIndex);
            }
        }}
    }
    
    return(-1);
}

/** Local function used to modify the original granny data - flip coordintes, scale, etc. */
static void	TransformFile(granny_file_info *FileInfo)
{
    float Origin[] = {0, 0, 0};
    float RightVector[] = {1, 0, 0}; //x
    float UpVector[] = {0, 0, 1};	//y
    float BackVector[] = {0, -1, 0};//z
            
	return;

    float Affine3[3];
    float Linear3x3[3][3];
    float InverseLinear3x3[3][3];
    GrannyAutoComputeBasisConversion(FileInfo, 1.0f,
                                     Origin,
                                     RightVector,
                                     UpVector,
                                     BackVector,
                                     Affine3, (float *)Linear3x3,
                                     (float *)InverseLinear3x3);

    GrannyAutoTransformFile(FileInfo, Affine3,
                            (float *)Linear3x3,
                            (float *)InverseLinear3x3);
}

//=============================================================================
// GrannyRenderObjClass::~GrannyRenderObjClass
//=============================================================================
/** Destructor. Releases w3d/granny assets. */
//=============================================================================
GrannyRenderObjClass::~GrannyRenderObjClass(void)
{
	if (m_animationControl)
		GrannyFreeControl(m_animationControl);
    if (m_modelInstance)
		GrannyFreeModelInstance(m_modelInstance);
	freeResources();
}

//=============================================================================
// GrannyRenderObjClass::GrannyRenderObjClass
//=============================================================================
/** Constructor. Creates an instance of the prototype. */
//=============================================================================
GrannyRenderObjClass::GrannyRenderObjClass(const GrannyPrototypeClass &proto)
:m_prototype(proto)
{
	granny_file_info *fileInfo = GrannyGetFileInfo((granny_file *)proto.m_file);
	m_boundingBox = proto.m_boundingBox;
	m_boundingSphere = proto.m_boundingSphere;
	m_vertexCount = proto.m_vertexCount;
	m_animationControl = NULL;	//no animation playing by default

	if (fileInfo)
	{
		//Find the skinned model
		for (Int modelIndex=0; modelIndex<fileInfo->ModelCount; modelIndex++)
		{
			granny_model *sourceModel =  fileInfo->Models[modelIndex];
			//ignore bounding boxes since they are never rendered
			if (stricmp(sourceModel->Name,"AABOX") != 0)
				m_modelInstance =  GrannyInstantiateModel(fileInfo->Models[modelIndex]);
		}

		if(m_modelInstance)
		{
			//assign textures to the model
			GrannyAutoBindModel(m_modelInstance, _GrannyLoader.Get_Material_Library(), 1);
	//        GrannyAddModelToScene(Global.Scene, ModelInstance);	///@todo: Create a granny scene for quicker updates.

			float *RootTransform = GrannyGetModelRootTransform(m_modelInstance);

			RootTransform[12] = 0;//x
			RootTransform[13] = 0;//y
			RootTransform[14] = 0;//z
		}//modelinstance*/
	}
}

/** Set scaling factor applied to prototype during rendering */ 
void GrannyRenderObjClass::Set_ObjectScale(float scale)
{
	ObjectScale=scale;

	//adjust bounding volumes we copied from non-scaled prototype.

	m_boundingBox.Center *= scale;
	m_boundingBox.Extent *= scale;

	///@todo: Remove earlier hacks in W3D to get instance matrix scaling!
	///After the W3D hacks are gone, we should also scale the radius here.
	m_boundingSphere.Center *= scale;
}

//=============================================================================
// GrannyRenderObjClass::Get_Obj_Space_Bounding_Sphere
//=============================================================================
/** WW3D method that returns object bounding sphere used in frustum culling*/
//=============================================================================
void GrannyRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{	sphere=m_boundingSphere;
}

//=============================================================================
// GrannyRenderObjClass::Get_Obj_Space_Bounding_Box
//=============================================================================
/** WW3D method that returns object bounding box used in collision detection*/
//=============================================================================
void GrannyRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box=m_boundingBox;
}

//=============================================================================
// GrannyRenderObjClass::Cast_Ray
//=============================================================================
/** WW3D method that returns intersection point between ray and model.
	We will only return results of a simple 'pick box' - not full triangle level
	detail.
*/
//=============================================================================
Bool GrannyRenderObjClass::Cast_Ray(RayCollisionTestClass & raytest)
{

	if ((Get_Collision_Type() & raytest.CollisionType) == 0) return false;

	AABoxClass hbox=m_boundingBox;

	hbox.Transform(Get_Transform());

	if (CollisionMath::Collide(raytest.Ray,hbox,raytest.Result)) {
		raytest.CollidedRenderObj = this;
		return true;
	}
	return false;
}

//=============================================================================
// GrannyRenderObjClass::Class_ID
//=============================================================================
/** returns the class id, so the scene can tell what kind of render object it has. */
//=============================================================================
Int GrannyRenderObjClass::Class_ID(void) const
{
	return RenderObjClass::CLASSID_UNKNOWN;
}

//=============================================================================
// GrannyRenderObjClass::Clone
//=============================================================================
/** Not used, but required virtual method. */
//=============================================================================
RenderObjClass *	 GrannyRenderObjClass::Clone(void) const
{
	assert(false);
	return NULL;
}

//=============================================================================
// GrannyRenderObjClass::freeResources
//=============================================================================
/** Free any W3D resources associated with this object */
//=============================================================================
Int GrannyRenderObjClass::freeResources(void)
{
//	REF_PTR_RELEASE(m_stageZeroTexture);

	return 0;
}

/** W3D Method used to control the animation assocated with this render object. */
void GrannyRenderObjClass::Set_Animation( HAnimClass * motion,	float frame, int anim_mode)
{
	GrannyAnimClass *gAnim=(GrannyAnimClass*)motion;

	granny_animation *Animation=gAnim->Animation;

    if(Animation)
	{
		Int trackIndex=-1;
		//Check if this animation works with this model
		trackIndex=FindTrackGroupFor(Animation,m_modelInstance);

        if(trackIndex != -1)
		{
			//stop previous animations
			if (m_animationControl)
			{	GrannyFreeControl(m_animationControl);
				m_animationControl=NULL;
			}

			m_animationControl = GrannyPlayControlledAnimation(frame / gAnim->FrameRate, Animation, trackIndex, m_modelInstance, 0);
			if (m_animationControl)
			{
				GrannySetControlSpeed(m_animationControl, 1.0f);	//play at normal speed
				if (anim_mode == ANIM_MODE_LOOP)
					GrannySetControlLooping(m_animationControl, true);
				else
					GrannySetControlLooping(m_animationControl, false);
			}
		}
	}
}

//=============================================================================
// GrannyRenderObjClass::Render
//=============================================================================
/** Draws this render object to the screen.
*/
//=============================================================================
void GrannyRenderObjClass::Render(RenderInfoClass & rinfo)
{
	TheGrannyRenderObjSystem->AddRenderObject(rinfo, this);
	return;

	//update the model
    GrannySetModelClock(m_modelInstance, LastSyncTime);
	LastSyncTime = WW3D::Get_Sync_Time()*0.001f;	//convert to seconds

    GrannySampleModelAnimations(m_modelInstance, 0, GrannyGetModelBoneCount(m_modelInstance),
                                    GrannyGetModelLocalPose(m_modelInstance));
    GrannyBuildModelWorldTransforms(m_modelInstance, 0,
                                    GrannyGetModelBoneCount(m_modelInstance),
                                    GrannyGetModelLocalPose(m_modelInstance),
                                    GrannyGetModelRootTransform(m_modelInstance),
                                    GrannyGetModelWorldPose(m_modelInstance));

    Real *RootTransform = GrannyGetModelRootTransform(m_modelInstance);

    RootTransform[12] = 0; //X
    RootTransform[13] = 0; //Y
    RootTransform[13] = 0; //Z
    RootTransform[0] = ObjectScale;
    RootTransform[5] = ObjectScale;
    RootTransform[10] = ObjectScale;

    Int MeshCount = GrannyGetModelMeshCount(m_modelInstance);
    for(Int MeshIndex = 0; MeshIndex < MeshCount; ++MeshIndex)
    {
        granny_mesh_instance *Mesh = (granny_mesh_instance *)GrannyGetModelMeshCookie(m_modelInstance, MeshIndex);
        granny_mesh_model_binding *Binding = GrannyGetMeshModelBinding(Mesh);

        granny_pnt332_vertex *Vertices = 0;
        if(GrannyMeshIsRigid(Mesh))
        {	assert(0);	//have not coded support for rigid meshes yet - only soft skinned supported.
//            granny_matrix_4x4 TempBuffer;
//            glMultMatrixf(GrannyGetRigidMeshTransform(
//                Binding, GrannyGetModelWorldPose(m_modelInstance),
//                (granny_real32 *)TempBuffer));
            Vertices = (granny_pnt332_vertex *)GrannyGetMeshVertices(Mesh);
        }
        else
        {
            GrannyDeformMesh(Binding, GrannyGetModelWorldPose(m_modelInstance),
                             0, g_blendingBuffer);
            Vertices = g_blendingBuffer;
        }

        int GroupCount = GrannyGetMeshTriangleGroupCount(Mesh);        
        {for(Int GroupIndex = 0;
             GroupIndex < GroupCount;
             ++GroupIndex)
        {
            granny_material_instance *Material = (granny_material_instance *)
                GrannyGetMeshMaterialCookie(
                    Mesh, GrannyGetMeshTriangleGroupMaterialIndex(
                        Mesh, GroupIndex));
		    if(Material)
			{
				Int TextureIndex;
				if(GrannyGetMaterialTextureIndexByType(
					Material, GrannyDiffuseColorTexture, &TextureIndex))
				{
					granny_texture_instance *TextureInstance =
						(granny_texture_instance *)GrannyGetMaterialTextureCookie(
							Material, TextureIndex);

					if(TextureInstance)
					{
						DX8Wrapper::Set_Texture(0, (TextureClass *)GrannyGetTextureCookie(TextureInstance));
					}
					else
						DX8Wrapper::Set_Texture(0, NULL);
				}
			}

			DynamicVBAccessClass vb_access(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,m_vertexCount);
			{
				DynamicVBAccessClass::WriteLockClass lock(&vb_access);
				VertexFormatXYZNDUV2* vb=lock.Get_Formatted_Vertex_Array();
				if(vb)
				{
					for (Int i=0; i<m_vertexCount; i++)
					{
						vb->x=Vertices->Position[0];
						vb->y=Vertices->Position[1];
						vb->z=Vertices->Position[2];
						vb->nx=Vertices->Normal[0];
						vb->ny=Vertices->Normal[1];
						vb->nz=Vertices->Normal[2];
						vb->diffuse=0xffffffff;
						vb->u1=Vertices->UV[0];
						vb->v1=Vertices->UV[1];
						vb++;
						Vertices++;
					}
				}
			}

            Int indexCount=GrannyGetMeshTriangleGroupIndexCount(Mesh, GroupIndex);
			UnsignedInt *indices=(UnsignedInt*)GrannyGetMeshTriangleGroupIndices(Mesh, GroupIndex);

			DynamicIBAccessClass ib_access(BUFFER_TYPE_DYNAMIC_DX8,indexCount);
			{
				DynamicIBAccessClass::WriteLockClass lock(&ib_access);
				unsigned short* ib=lock.Get_Index_Array();

				if(ib)
				{
					for (Int i=0; i<indexCount; i++)
						ib[i]=(UnsignedShort)indices[i];
				}
			}

			
			Matrix3D tm(Transform);
			DX8Wrapper::Set_Light_Environment(rinfo.light_environment);
			DX8Wrapper::Set_Material(m_prototype.m_vertexMaterial);
			DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueShader);
			DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
			DX8Wrapper::Set_Index_Buffer(ib_access,0);
			DX8Wrapper::Set_Vertex_Buffer(vb_access);
			DX8Wrapper::Draw_Triangles(	0,indexCount/3, 0,	m_vertexCount);	//draw a quad, 2 triangles, 4 verts
        }}
    }
}

/**
GrannyLoaderClass::Load -- reads in a granny model and creates a prototype for it
*/
PrototypeClass * GrannyLoaderClass::Load_W3D(const char *filename)
{
    char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];

    granny_file *File = GrannyReadEntireFile(filename);
	granny_file_info *fileInfo;
	AABoxClass box;
	Int vertexCount=0;
	Int indexCount=0;
	Int meshCount=0;
	GrannyPrototypeClass::grannyMeshDesc meshData[GRANNY_MAX_MESHES_PER_MODEL];

    if(File)
    {
        fileInfo = GrannyGetFileInfo(File);
		if (fileInfo)
		{

            TransformFile(fileInfo);
            
            for(int TextureIndex = 0; TextureIndex < fileInfo->TextureCount; ++TextureIndex)
            {
				_splitpath(fileInfo->Textures[TextureIndex]->FromFileName, drive, dir, fname, ext );
				TextureClass *TextureHandle=WW3DAssetManager::Get_Instance()->Get_Texture(strncat(fname,".tga",4));
			    granny_texture_instance *TextureInstance =  GrannyInstantiateTexture(fileInfo->Textures[TextureIndex]);
				GrannySetTextureCookie(TextureInstance, (granny_uint32)TextureHandle);
                GrannyAddTextureToLibrary(TextureLibrary,TextureInstance);
            }            

			//Calculate bounding box and find maximum number of vertices needed to render mesh.
			for (Int modelIndex=0; modelIndex<fileInfo->ModelCount; modelIndex++)
			{
				granny_model *sourceModel =  fileInfo->Models[modelIndex];
				if (stricmp(sourceModel->Name,"AABOX") == 0)
				{	//found a collision box, copy out data
					int MeshCount = sourceModel->MeshBindingCount;
					if (MeshCount==1)
					{
						granny_mesh *sourceMesh = sourceModel->MeshBindings[0].Mesh;
						granny_pn33_vertex *Vertices = (granny_pn33_vertex *)sourceMesh->PrimaryVertexData->Vertices;
						Vector3 points[24];

						assert (sourceMesh->PrimaryVertexData->VertexCount <= 24);

						for (Int boxVertex=0; boxVertex<sourceMesh->PrimaryVertexData->VertexCount; boxVertex++)
						{	points[boxVertex].Set(Vertices[boxVertex].Position[0],
												  Vertices[boxVertex].Position[1],
												  Vertices[boxVertex].Position[2]);
						}
						box.Init(points,sourceMesh->PrimaryVertexData->VertexCount);
					}
				}
				else
				{	//mesh is part of model
					meshCount = sourceModel->MeshBindingCount;

					for (Int meshIndex=0; meshIndex<meshCount; meshIndex++)
					{
						granny_mesh *sourceMesh = sourceModel->MeshBindings[meshIndex].Mesh;
						meshData[meshIndex].vertex=NULL;
						meshData[meshIndex].vertexCount=0;
						if (sourceMesh->PrimaryVertexData)
						{	vertexCount+=sourceMesh->PrimaryVertexData->VertexCount;
							meshData[meshIndex].vertex=(granny_pnt332_vertex *)sourceMesh->PrimaryVertexData->Vertices;
							meshData[meshIndex].vertexCount=sourceMesh->PrimaryVertexData->VertexCount;
						}
						meshData[meshIndex].index=NULL;
						meshData[meshIndex].indexCount=NULL;
						if (sourceMesh->PrimaryTopology)
						{
							assert (sourceMesh->PrimaryTopology->GroupCount == 1);	//should only have 1 material per mesh!
							granny_tri_material_group &sourceGroup = sourceMesh->PrimaryTopology->Groups[0];

							// This is the texture index (relative to the array we just
							// built) for this group of triangles
							// SourceGroup.MaterialIndex;

							// This is the number of indices
							if(sourceMesh->PrimaryTopology->IndexCount)
							{
								meshData[meshIndex].index=(const UnsignedInt*)&sourceMesh->PrimaryTopology->Indices[3*sourceGroup.TriFirst];
								// These are the indices for this group
								meshData[meshIndex].indexCount=3*sourceGroup.TriCount;
								indexCount += meshData[meshIndex].indexCount;	//keep track of total indices in this model
							}
						}
					}
				}
			}
			  ///@todo: Convert granny materials into W3D/D3D materials - now using default.
//            for(int MaterialIndex = 0; MaterialIndex < fileInfo->MaterialCount; ++MaterialIndex)
//            {
//			    granny_material_instance *MaterialInstance = GrannyInstantiateMaterial(fileInfo->Materials[MaterialIndex]);
//				GrannyAutoBindAllMaterialTextures(MaterialInstance,TextureLibrary);
//                GrannyAddMaterialToLibrary(MaterialLibrary,MaterialInstance);
//            }
			

            GrannyAutoBindAllMaterials(MaterialLibrary, fileInfo,TextureLibrary);

        } //fileInfo

		if (fileInfo == NULL) {
			return NULL;
		}

		// ok, accept this model! 
		GrannyPrototypeClass * hproto = NEW GrannyPrototypeClass(File);
		_splitpath(filename, drive, dir, fname, ext );
		hproto->Set_Name(strcat(fname,ext));
		hproto->setBoundingBox(box);
		hproto->setBoundingSphere(SphereClass(box.Center,box.Extent.Length()));
		hproto->setVertexCount(vertexCount);
		hproto->setIndexCount(indexCount);
		hproto->setMeshCount(meshCount);
		for (Int i=0; i<meshCount; i++)
			hproto->setMeshData(meshData[i],i);
		return hproto;
	}//FILE

	return NULL;
}

GrannyLoaderClass::GrannyLoaderClass(void)
{
    TextureLibrary = GrannyNewTextureLibrary(1 << 12);
	MaterialLibrary = GrannyNewMaterialLibrary(1 << 12);
}

GrannyLoaderClass::~GrannyLoaderClass(void)
{
    GrannyFreeTextureLibrary(TextureLibrary);
    GrannyFreeMaterialLibrary(MaterialLibrary);

	TextureLibrary=NULL;
	MaterialLibrary=NULL;
}

GrannyAnimManagerClass::GrannyAnimManagerClass(void) 
{
	// Create the hash tables
	AnimPtrTable = NEW HashTableClass( 2048 );
	MissingAnimTable = NEW HashTableClass( 2048 );
}

GrannyAnimManagerClass::~GrannyAnimManagerClass(void)
{
	Free_All_Anims();

	delete AnimPtrTable;
	AnimPtrTable = NULL;

	delete MissingAnimTable;
	MissingAnimTable = NULL;
}

/** Release all loaded animations */
void GrannyAnimManagerClass::Free_All_Anims(void)
{
	// Make an iterator, and release all ptrs
	GrannyAnimManagerIterator it( *this );
	for( it.First(); !it.Is_Done(); it.Next() ) {
		GrannyAnimClass *anim = it.Get_Current_Anim();
		anim->Release_Ref();
	}

	// Then clear the table
	AnimPtrTable->Reset();
}

/** Find animation in cache */
GrannyAnimClass * GrannyAnimManagerClass::Peek_Anim(const char * name)
{
	return (GrannyAnimClass*)AnimPtrTable->Find( name );
}

/** Get animation from cache and increment its reference count */
GrannyAnimClass * GrannyAnimManagerClass::Get_Anim(const char * name)
{	
	GrannyAnimClass * anim = Peek_Anim( name );
	if ( anim != NULL ) {
		anim->Add_Ref();
	}
	return anim;
}

/** Add animation to cache */
Bool GrannyAnimManagerClass::Add_Anim(GrannyAnimClass *new_anim)
{
	WWASSERT (new_anim != NULL);

	// Increment the refcount on the new animation and add it to our table.
	new_anim->Add_Ref ();
	AnimPtrTable->Add( new_anim );

	return true;
}

/*
** Missing Anims
**
** The idea here, allow the system to register which anims are determined to be missing
** so that if they are asked for again, we can quickly return NULL, without searching the
** disk again.
*/
void	GrannyAnimManagerClass::Register_Missing( const char * name )
{
	MissingAnimTable->Add( NEW MissingAnimClass( name ) );
}

Bool	GrannyAnimManagerClass::Is_Missing( const char * name )
{
	return ( MissingAnimTable->Find( name ) != NULL );
}

/** Load an animation from disk and add to cache */
int GrannyAnimManagerClass::Load_Anim(const char * name)
{
	GrannyAnimClass * newanim = NEW GrannyAnimClass;

	if (newanim == NULL) {
		goto Error;
	}

	SET_REF_OWNER( newanim );

	if (newanim->Load_W3D(name) != GrannyAnimClass::OK)
	{	// load failed!
		newanim->Release_Ref();
		goto Error;
	} else if (Peek_Anim(newanim->Get_Name()) != NULL)
	{	// duplicate exists!
		newanim->Release_Ref();	// Release the one we just loaded
		goto Error;
	} else
	{	Add_Anim( newanim );
		newanim->Release_Ref();
	}

	return 0;

Error:
	return 1;
}

/*
** Iterator converter from HashableClass to GrannyAnimClass
*/
GrannyAnimClass * GrannyAnimManagerIterator::Get_Current_Anim( void )	
{ 
	return (GrannyAnimClass *)Get_Current(); 
}


GrannyAnimClass::GrannyAnimClass(void) :
	NumFrames(0),
	FrameRate(0),
	File(0),
	Animation(0)
{
	Name[0]='\0';
}


/** GrannyAnimClass::~GrannyAnimClass -- Destructor */ 
GrannyAnimClass::~GrannyAnimClass(void)
{
	GrannyFreeFile(File);
}

/** Read granny data from disk */
int GrannyAnimClass::Load_W3D(const char *name)
{
	granny_file *file=NULL;

    file = GrannyReadEntireFile(name);

	if (file)
	{
		granny_file_info *fileInfo;
		fileInfo = GrannyGetFileInfo(file);
		if (fileInfo && fileInfo->AnimationCount)
		{	Animation = fileInfo->Animations[0];
			File = file;
			strcpy(Name,name);
			FrameRate = 1.0f/Animation->TimeStep;
			NumFrames = Animation->Duration / Animation->TimeStep;
			return OK;
		}
		else
			GrannyFreeFile(File);	//no animations found
	}
	return LOAD_ERROR;
}

GrannyLoaderClass	_GrannyLoader;
GrannyRenderObjSystem	*TheGrannyRenderObjSystem=NULL;

/** Adds render object to a queue with other objects using the same material.  These queues will be
	flushed at the end of the frame.
*/
void GrannyRenderObjSystem::AddRenderObject(RenderInfoClass & rinfo, GrannyRenderObjClass *robj)
{
	if (m_renderObjectCount >= MAX_VISIBLE_GRANNY_MODELS)
	{	assert (m_renderObjectCount < MAX_VISIBLE_GRANNY_MODELS);
		return;	//can't add any more granny models this frame.
	}

	for (Int i=0; i<m_renderStateCount; i++)
	{	//search for other objects with same render state
		//right now we can just sort by prototype since prototypes have fixed materials.
		if (m_renderStateModelList[i].list->m_prototype.m_file == robj->m_prototype.m_file)
		{	//found model with same prototype.  Add new model to same list.
			robj->m_nextSystem=m_renderStateModelList[i].list;
			m_renderStateModelList[i].list=robj;
			if (rinfo.light_environment)
				m_renderLocalLightEnv[m_renderObjectCount]=*rinfo.light_environment;
			m_renderObjectCount++;
			return;
		}
	}

	//Found a new render state

	assert (m_renderStateCount < MAX_GRANNY_RENDERSTATES);

	if (m_renderStateCount > MAX_GRANNY_RENDERSTATES)
		return;	//reached maximum number of render states

	//store this objects lighting information for use later during drawing.
	if (rinfo.light_environment)
		m_renderLocalLightEnv[m_renderObjectCount]=*rinfo.light_environment;

	m_renderStateModelList[m_renderStateCount++].list=robj;
	robj->m_nextSystem=NULL;

	m_renderObjectCount++;
}

/** Draws all the granny render objects that were rendered in the last frame.
	Drawing is done in render state order to reduce overhead.
*/
void GrannyRenderObjSystem::Flush(void)
{
	GrannyRenderObjClass *robj;
    granny_model_instance *modelInstance;
	Bool setMaterial;
	Int modelCount=0;
	Int indexCount;	//per model index count

	for (Int i=0; i<m_renderStateCount; i++)
	{
		robj=m_renderStateModelList[i].list;	//set to start of objects using this material
		setMaterial=true;	//force rendering code to apply material

		while (robj)
		{
			modelInstance=robj->m_modelInstance;

			//update the model
			GrannySetModelClock(modelInstance,robj->LastSyncTime);
			robj->LastSyncTime = WW3D::Get_Sync_Time()*0.001f;	//convert to seconds

			GrannySampleModelAnimations(modelInstance, 0, GrannyGetModelBoneCount(modelInstance),
											GrannyGetModelLocalPose(modelInstance));
			GrannyBuildModelWorldTransforms(modelInstance, 0,
											GrannyGetModelBoneCount(modelInstance),
											GrannyGetModelLocalPose(modelInstance),
											GrannyGetModelRootTransform(modelInstance),
											GrannyGetModelWorldPose(modelInstance));

			Real *RootTransform = GrannyGetModelRootTransform(modelInstance);

			RootTransform[12] = 0; //X
			RootTransform[13] = 0; //Y
			RootTransform[13] = 0; //Z
			RootTransform[0] = robj->ObjectScale;
			RootTransform[5] = robj->ObjectScale;
			RootTransform[10] = robj->ObjectScale;

			Int MeshCount = GrannyGetModelMeshCount(modelInstance);
			for(Int MeshIndex = 0; MeshIndex < MeshCount; ++MeshIndex)
			{
				granny_mesh_instance *Mesh = (granny_mesh_instance *)GrannyGetModelMeshCookie(modelInstance, MeshIndex);
				granny_mesh_model_binding *Binding = GrannyGetMeshModelBinding(Mesh);

				granny_pnt332_vertex *Vertices = 0;
				if(GrannyMeshIsRigid(Mesh))
				{	assert(0);	//have not coded support for rigid meshes yet - only soft skinned supported.
		//            granny_matrix_4x4 TempBuffer;
		//            glMultMatrixf(GrannyGetRigidMeshTransform(
		//                Binding, GrannyGetModelWorldPose(modelInstance),
		//                (granny_real32 *)TempBuffer));
					Vertices = (granny_pnt332_vertex *)GrannyGetMeshVertices(Mesh);
				}
				else
				{
					GrannyDeformMesh(Binding, GrannyGetModelWorldPose(modelInstance),
									 0, g_blendingBuffer);
					Vertices = g_blendingBuffer;
				}

				int GroupCount = GrannyGetMeshTriangleGroupCount(Mesh);        
				for(Int GroupIndex = 0;	 GroupIndex < GroupCount; ++GroupIndex)
				{
					granny_material_instance *Material = (granny_material_instance *)
						GrannyGetMeshMaterialCookie(
							Mesh, GrannyGetMeshTriangleGroupMaterialIndex(
								Mesh, GroupIndex));
					if(setMaterial && Material)
					{
						Int TextureIndex;
						if(GrannyGetMaterialTextureIndexByType(
							Material, GrannyDiffuseColorTexture, &TextureIndex))
						{
							granny_texture_instance *TextureInstance =
								(granny_texture_instance *)GrannyGetMaterialTextureCookie(
									Material, TextureIndex);

							if(TextureInstance)
							{
								DX8Wrapper::Set_Texture(0, (TextureClass *)GrannyGetTextureCookie(TextureInstance));
							}
							else
								DX8Wrapper::Set_Texture(0, NULL);
						}
					}

					DynamicVBAccessClass vb_access(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,robj->m_vertexCount);
					{
						DynamicVBAccessClass::WriteLockClass lock(&vb_access);
						VertexFormatXYZNDUV2* vb=lock.Get_Formatted_Vertex_Array();
						if(vb)
						{
							for (Int i=0; i<robj->m_vertexCount; i++)
							{
								vb->x=Vertices->Position[0];
								vb->y=Vertices->Position[1];
								vb->z=Vertices->Position[2];
								vb->nx=Vertices->Normal[0];
								vb->ny=Vertices->Normal[1];
								vb->nz=Vertices->Normal[2];
								vb->diffuse=0xffffffff;
								vb->u1=Vertices->UV[0];
								vb->v1=Vertices->UV[1];
								vb++;
								Vertices++;
							}
						}
					}

					if (setMaterial)
					{	//we started using a new material queue - really a new model prototype
						//so reset all relevant data.
						indexCount=GrannyGetMeshTriangleGroupIndexCount(Mesh, GroupIndex);
						UnsignedInt *indices=(UnsignedInt*)GrannyGetMeshTriangleGroupIndices(Mesh, GroupIndex);

						DynamicIBAccessClass ib_access(BUFFER_TYPE_DYNAMIC_DX8,indexCount);
						{
							DynamicIBAccessClass::WriteLockClass lock(&ib_access);
							unsigned short* ib=lock.Get_Index_Array();

							if(ib)
							{
								for (Int i=0; i<indexCount; i++)
									ib[i]=(UnsignedShort)indices[i];
							}
						}
						DX8Wrapper::Set_Material(robj->m_prototype.m_vertexMaterial);
						DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueShader);
						setMaterial=false;	//don't set material again unless it changes.
						DX8Wrapper::Set_Index_Buffer(ib_access,0);
					}
					
					Matrix3D tm(robj->Transform);
					DX8Wrapper::Set_Light_Environment(&m_renderLocalLightEnv[modelCount]);
					DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
					DX8Wrapper::Set_Vertex_Buffer(vb_access);
					DX8Wrapper::Draw_Triangles(	0,indexCount/3, 0,	robj->m_vertexCount);	//draw a quad, 2 triangles, 4 verts
				}
			}
			robj=robj->m_nextSystem;	//move to next object using this material
			modelCount++;
		}//while

	}

	//reset for next frame
	m_renderObjectCount=0;
	m_renderStateCount=0;
}

#if 0	///@todo: Will have to implement an optimized granny rendering system
//=============================================================================
// GrannyRenderObjClassSystem::GrannyRenderObjClassSystem
//=============================================================================
/** Constructor. Just nulls out some variables. */
//=============================================================================
GrannyRenderObjClassSystem::GrannyRenderObjClassSystem()
{
	m_usedModules = NULL;
	m_freeModules = NULL;
	m_indexBuffer = NULL;
	m_vertexMaterialClass = NULL;
	m_vertexBuffer = NULL;
}

//=============================================================================
// GrannyRenderObjClassSystem::~GrannyRenderObjClassSystem
//=============================================================================
/** Destructor.  Free all pre-allocated track laying render objects*/
//=============================================================================
GrannyRenderObjClassSystem::~GrannyRenderObjClassSystem( void )
{

	// free all data
	shutdown();

	m_vertexMaterialClass=NULL;

}

//=============================================================================
// GrannyRenderObjClassSystem::ReAcquireResources
//=============================================================================
/** (Re)allocates all W3D assets after a reset.. */
//=============================================================================
void GrannyRenderObjClassSystem::ReAcquireResources(void)
{
	// just for paranoia's sake.
	REF_PTR_RELEASE(m_indexBuffer);
	REF_PTR_RELEASE(m_vertexBuffer);

	//Create static index buffers.  These will index the vertex buffers holding the map.
	m_indexBuffer=NEW_REF(DX8IndexBufferClass,(32*6));

	// Fill up the IB
	{
		DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexBuffer);
		UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	}

	///@todo: Allocating double sized buffer than really needed... but things go bad otherwise. Figure out why!

	m_vertexBuffer=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,1*2,DX8VertexBufferClass::USAGE_DYNAMIC));

}

//=============================================================================
// GrannyRenderObjClassSystem::ReleaseResources
//=============================================================================
/** (Re)allocates all W3D assets after a reset.. */
//=============================================================================
void GrannyRenderObjClassSystem::ReleaseResources(void)
{
	REF_PTR_RELEASE(m_indexBuffer);
	REF_PTR_RELEASE(m_vertexBuffer);
	// Note - it is ok to not release the material, as it is a w3d object that
	// has no dx8 resources. jba.
}

//=============================================================================
// GrannyRenderObjClassSystem::init
//=============================================================================
/**  initialize the system, allocate all the render objects we will need */
//=============================================================================
void GrannyRenderObjClassSystem::init()
{
	const Int numModules=TheGlobalData->m_maxTerrainTracks;

	Int i;
	GrannyRenderObjClass *mod;

	ReAcquireResources();
	//go with a preset material for now.
	m_vertexMaterialClass=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);

	//use a multi-texture shader: (text1*diffuse)*text2.
	m_shaderClass = ShaderClass::_PresetAlphaShader;//_PresetATestSpriteShader;//_PresetOpaqueShader;

	// we cannot initialize a system that is already initialized
	if( m_freeModules || m_usedModules )
	{

		// system already online!
		assert( 0 );
		return;

	}  // end if

	// allocate our modules for this system
	for( i = 0; i < numModules; i++ )
	{

		mod = NEW_REF( GrannyRenderObjClass, () );

		if( mod == NULL )
		{

			// unable to allocate modules needed
			assert( 0 );
			return;

		}  // end if

		mod->m_prevSystem = NULL;
		mod->m_nextSystem = m_freeModules;
		if( m_freeModules )
			m_freeModules->m_prevSystem = mod;
		m_freeModules = mod;

	}  // end for i

}  // end init

//=============================================================================
// GrannyRenderObjClassSystem::shutdown
//=============================================================================
/** Shutdown and free all memory for this system */
//=============================================================================
void GrannyRenderObjClassSystem::shutdown( void )
{

	REF_PTR_RELEASE(m_indexBuffer);
	REF_PTR_RELEASE(m_vertexMaterialClass);
	REF_PTR_RELEASE(m_vertexBuffer);

}  // end shutdown

//=============================================================================
// GrannyRenderObjClassSystem::update
//=============================================================================
/** Update the state of all active track marks - fade, expire, etc. */
//=============================================================================
void GrannyRenderObjClassSystem::update()
{

	Int		iTime=timeGetTime();
}


//=============================================================================
// GrannyRenderObjClassSystem::flush
//=============================================================================
/** Draw all active track marks for this frame */
//=============================================================================
void GrannyRenderObjClassSystem::flush()
{
	Int	diffuseLight;
	GrannyRenderObjClass *mod=0;

	// adjust shading for time of day.
	Real shadeR, shadeG, shadeB;
	shadeR = TheGlobalData->m_terrainAmbientRed;
	shadeG = TheGlobalData->m_terrainAmbientGreen;
	shadeB = TheGlobalData->m_terrainAmbientBlue;
	shadeR += TheGlobalData->m_terrainDiffuseRed/2;
	shadeG += TheGlobalData->m_terrainDiffuseGreen/2;
	shadeB += TheGlobalData->m_terrainDiffuseBlue/2;
	shadeR*=255.0f;
	shadeG*=255.0f;
	shadeB*=255.0f;

	diffuseLight=(int)shadeB | ((int)shadeG << 8) | ((int)shadeR << 16);

	//check if there is anything to draw and fill vertex buffer
	{
		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexBuffer);
		VertexFormatXYZDUV1 *verts = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();

	}//edges to flush

	//draw the filled vertex buffers
	{
		ShaderClass::Invalidate();
		DX8Wrapper::Set_Material(m_vertexMaterialClass);
		DX8Wrapper::Set_Shader(m_shaderClass);
		DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
		DX8Wrapper::Set_Vertex_Buffer(m_vertexBuffer);

		Matrix3D tm(mod->Transform);
		DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
		DX8Wrapper::Set_Index_Buffer_Index_Offset(0);
		DX8Wrapper::Draw_Triangles(	0,2, 0, 1*2);

	}	//there are some edges to render in pool.
}

//=============================================================================
// GrannyRenderObjClassSystem::createRenderObj
//=============================================================================
/** Creates an instance of a W3D render object using the specified granny	 */
/*  model.  If the model doesn't exist yet, it will be loaded from disk.	 */
//=============================================================================
RenderObjClass *GrannyRenderObjClassSystem::createRenderObj(const char * name)
{

	return NULL;
}

GrannyRenderObjClassSystem *TheGrannyRenderObjClassSystem=NULL;	///< singleton for track drawing system.

#endif	//end of GrannyRenderObjClassSystem

#endif //INCLUDE_GRANNY_IN_BUILD