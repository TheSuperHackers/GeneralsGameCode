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
 *                     $Archive:: /Commando/Code/ww3d2/shdrenderer.cpp                           $*
 *
 *                    Org Author:: Jani_p
 *                                                                                             *
 *                       $Author:: Kenny_m
 *																																	
 *								$Modtime:: 7/29/02 1:50p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * 6/02/02 5:59p KM Added render info and light environment support                                              $*
 * 7/29/02 1:50p KM Added VB and IB usage flags for software vertex shaders
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "shdrenderer.h"
#include "shdforcelinks.h"
#include "shdmesh.h"
#include "shdsubmesh.h"
#include "shddef.h"
#include "shddefmanager.h"
#include "shdclassids.h"
#include "wwdebug.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "rinfo.h"
#include "camera.h"
#include "texture.h"
#include "ww3dformat.h"
#include "wwprofile.h"
#include "sortingrenderer.h"
#include "meshmatdesc.h"

static DynamicVectorClass<Vector3>				_TempVertexBuffer;
static DynamicVectorClass<Vector3>				_TempNormalBuffer;

ShdRendererClass* ShdRendererClass::ShdRenderer=NULL;

ShdRendererClass::ShdRendererClass()
{
}

ShdRendererClass::~ShdRendererClass()
{
}

/////////////////////////////////////////////////////////////////////////////////////
// Initialize the rendering system. This is the place to create different versions
// (such as DX8, DX9, OGL) of the rendering system, if needed.
/////////////////////////////////////////////////////////////////////////////////////
void ShdRendererClass::Init()
{
	SHD_Force_Links();

	WWASSERT(!ShdRenderer);

	ShdRenderer=new ShdDX8RendererClass();

	Init_Shaders();
}

void ShdRendererClass::Init_Shaders()
{
	ShdDefClass* def;
	for (int i=SHDDEF_CLASSID_DUMMY+1; i<SHDDEF_CLASSID_LAST; i++)
	{
		def=ShdDefManagerClass::Create_ShdDefClass_Instance(i);
		if (def != NULL) {
			def->Init();
			REF_PTR_RELEASE(def);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Release the renderer system. At this point, all the meshes must have been
// unregistered.
/////////////////////////////////////////////////////////////////////////////////////
void ShdRendererClass::Shutdown()
{
	Shutdown_Shaders();

	WWASSERT(ShdRenderer);

	delete ShdRenderer;
	ShdRenderer=0;
}

void ShdRendererClass::Shutdown_Shaders()
{
	ShdDefClass* def;
	for (int i=SHDDEF_CLASSID_DUMMY+1; i<SHDDEF_CLASSID_LAST; i++)
	{
		def=ShdDefManagerClass::Create_ShdDefClass_Instance(i);
		if (def) {
			def->Shutdown();
			REF_PTR_RELEASE(def);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// DirectX8 renderer utility class declaration
/////////////////////////////////////////////////////////////////////////////////////

RendererListContainerClass::RendererListContainerClass(int pass) : Pass(pass)
{
}

RendererListContainerClass::~RendererListContainerClass()
{
	Unregister_All();
}

void RendererListContainerClass::Register_Renderer(ShdRendererNodeClass* node)
{
	WWASSERT(node);
	node->Add_Ref();
	node->Set_Renderer_List_Container(this,Pass);
	LinkedNodes.Add_Tail(node);
}

void RendererListContainerClass::Unregister_All()
{
	while (!LinkedNodes.Is_Empty()) {
		ShdRendererNodeClass* node=LinkedNodes.Remove_Head();
//		REF_PTR_RELEASE(node);
		delete node;
	}
}

void RendererListContainerClass::Flush()
{
	ShdRendererNodeClass* prev_node=NULL;
	while (!VisibleNodes.Is_Empty()) 
	{
		ShdRendererNodeClass* node=VisibleNodes.Remove_Head();
		node->Apply_Shared_Shader_Settings(prev_node,Pass);
		node->Flush(Pass);
		prev_node=node;
	}

}


class ShdDX8RendererClass::MeshContainerClass : public RefCountClass
{
public:
	MeshContainerClass();
	virtual ~MeshContainerClass();

	ShdRendererNodeClass* Register_Mesh(ShdMeshClass* mesh,ShdSubMeshClass* sub_mesh);
//	void Add_Visible_Node(ShdRendererNodeClass* node,int pass);
	void Flush();

private:

//	ShdRendererNodeList VisibleNodeList[SHD_MAX_PASSES];
	RendererListContainerList RendererListContainers[SHD_MAX_PASSES];
};

/**
** ShdDX8RendererNode
*/
class ShdDX8RendererNodeClass : public ShdRendererNodeClass
{
public:
	ShdDX8RendererNodeClass(ShdDX8RendererClass::MeshContainerClass* container, ShdMeshClass* mesh,ShdSubMeshClass* sub_mesh);
	virtual ~ShdDX8RendererNodeClass();

	virtual void Render(RenderInfoClass& rinfo);
	virtual void Flush(int pass);
	virtual void Apply_Shared_Shader_Settings(ShdRendererNodeClass* prev,int pass);

	virtual bool Greater_Than(const ShdRendererNodeClass&, int pass) const;
	virtual bool Similar_Enough(const ShdRendererNodeClass&, int pass) const;

private:

	ShdDX8RendererClass::MeshContainerClass*	Container;
	ShdMeshClass*										Mesh;
	ShdSubMeshClass*									SubMesh;
	VertexBufferClass**							VertexBuffers;
	IndexBufferClass*								IndexBuffer;
	LightEnvironmentClass							LightEnvironment;	// todo KJM optimize for output lights
	RenderInfoClass*									RenderInfo;
};

/////////////////////////////////////////////////////////////////////////////////////
// DirectX8 renderer implementation
/////////////////////////////////////////////////////////////////////////////////////
ShdDX8RendererClass::ShdDX8RendererClass()
{
	MeshCategories=new MeshContainerClass*[SHDDEF_CLASSID_LAST];
	for (int i=0;i<SHDDEF_CLASSID_LAST;++i) 
	{
		MeshCategories[i]=0;
	}
}

ShdDX8RendererClass::~ShdDX8RendererClass()
{
	for (int i=0;i<SHDDEF_CLASSID_LAST;++i) 
	{
		if (MeshCategories[i]) 
		{
			REF_PTR_RELEASE(MeshCategories[i]);
			delete MeshCategories[i];
			MeshCategories[i]=0;
		}
	}
	delete[] MeshCategories;
	MeshCategories=0;
}

bool ShdDX8RendererNodeClass::Greater_Than(const ShdRendererNodeClass& s, int pass) const
{
	// Danger! Assuming ShdDX8RendererNodeClass!
	const ShdDX8RendererNodeClass* src=static_cast<const ShdDX8RendererNodeClass*>(&s);

	WWASSERT(SubMesh && src->SubMesh);
	return SubMesh->Peek_Shader()->Greater_Than(*src->SubMesh->Peek_Shader(),pass);
}

bool ShdDX8RendererNodeClass::Similar_Enough(const ShdRendererNodeClass& s, int pass) const
{
	// Danger! Assuming ShdDX8RendererNodeClass!
	const ShdDX8RendererNodeClass* src=static_cast<const ShdDX8RendererNodeClass*>(&s);

	WWASSERT(SubMesh && src->SubMesh);
	return SubMesh->Peek_Shader()->Similar_Enough(*src->SubMesh->Peek_Shader(),pass);
}


/////////////////////////////////////////////////////////////////////////////////////
// Register mesh to the rendering system
/////////////////////////////////////////////////////////////////////////////////////
ShdRendererNodeClass* ShdDX8RendererClass::Register_Mesh
(
	ShdMeshClass* mesh,
	ShdSubMeshClass* sub_mesh
)
{
	ShdInterfaceClass* shd=sub_mesh->Peek_Shader();
	const ShdDefClass* def=shd->Peek_Definition();
	uint32 class_id=def->Get_Class_ID();
	WWASSERT(class_id<SHDDEF_CLASSID_LAST);
	if (!MeshCategories[class_id]) 
	{
		MeshCategories[class_id]=new MeshContainerClass();
	}
	return MeshCategories[class_id]->Register_Mesh(mesh,sub_mesh);
}

/////////////////////////////////////////////////////////////////////////////////////
// Flush the list of visible meshes
/////////////////////////////////////////////////////////////////////////////////////
void ShdDX8RendererClass::Flush()
{
	WWPROFILE("ShdDX8RendererClass::Flush");

	DX8Wrapper::Apply_Render_State_Changes();
	DX8Wrapper::Invalidate_Cached_Render_States();
	DX8Wrapper::Apply_Default_State();

	SNAPSHOT_SAY(("ShdDX8RendererClass::Flush()\n"));
	for (int i=0;i<SHDDEF_CLASSID_LAST;++i) 
	{
		if (MeshCategories[i]) 
		{
			MeshCategories[i]->Flush();
		}
	}
	DX8Wrapper::Invalidate_Cached_Render_States();
	DX8Wrapper::Apply_Default_State();
}



/////////////////////////////////////////////////////////////////////////////////////
// MeshContiner is used for each shader type. Meshes are assigned to a container
// when they're first rendered and they link themselves to container's visible list
// each frame they're visible.
/////////////////////////////////////////////////////////////////////////////////////
ShdDX8RendererClass::MeshContainerClass::MeshContainerClass()
{
}

ShdDX8RendererClass::MeshContainerClass::~MeshContainerClass()
{
	for (int pass=0; pass<SHD_MAX_PASSES; pass++) 
	{
		while (!RendererListContainers[pass].Is_Empty()) {
			RendererListContainerClass* cont=RendererListContainers[pass].Remove_Head();
			cont->Unregister_All();
			REF_PTR_RELEASE(cont);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// RendererNodeClass acts as a link between ShdMeshClass, ShdSubMeshClass and the
// rendering system.
/////////////////////////////////////////////////////////////////////////////////////
ShdRendererNodeClass* ShdDX8RendererClass::MeshContainerClass::Register_Mesh
(
	ShdMeshClass* mesh,
	ShdSubMeshClass* sub_mesh
)
{
	ShdDX8RendererNodeClass* node=new ShdDX8RendererNodeClass(this,mesh,sub_mesh);
	ShdInterfaceClass* shdi=sub_mesh->Peek_Shader();
	WWASSERT(shdi);
	for (int pass=0; pass<shdi->Get_Pass_Count(); pass++) 
	{
		RendererListContainerIterator ite(&RendererListContainers[pass]);
		ite.Last();
		while (!ite.Is_Done()) {
			RendererListContainerClass* cont=ite.Peek_Obj();
			if (cont) {
				ShdRendererNodeListIterator ite2(&cont->Peek_Linked_Nodes());
				ite2.First();
				// Container should delete itself when the last node is removed, so we must
				// never have an empty node.
				WWASSERT(!ite2.Is_Done());
				
				ShdRendererNodeClass* obj=ite2.Peek_Obj();
				if (obj) {
					if (node->Greater_Than(*obj,pass)) {
						// If similar enough, add to the same renderer container, otherwise create a new one
						if (node->Similar_Enough(*obj,pass)) {
							cont->Register_Renderer(node);
						}
						else {
							RendererListContainerClass* new_cont=new RendererListContainerClass(pass);
							new_cont->Register_Renderer(node);
							RendererListContainers[pass].Add_After(new_cont,new_cont);
						}
						break;
					}
				}
			}
			ite.Prev();
		}
		if (ite.Is_Done()) {
			RendererListContainerClass* new_cont=new RendererListContainerClass(pass);
			new_cont->Register_Renderer(node);
			RendererListContainers[pass].Add(new_cont);
		}
	}

	return node;
}

/////////////////////////////////////////////////////////////////////////////////////
// Render all visible nodes and clear the visible list.
/////////////////////////////////////////////////////////////////////////////////////
void ShdDX8RendererClass::MeshContainerClass::Flush()
{
	SNAPSHOT_SAY(("ShdDX8RendererClass::MeshContainerClass::Flush()\n"));
	
	for (int pass=0; pass<SHD_MAX_PASSES; pass++) 
	{
		RendererListContainerIterator ite(&RendererListContainers[pass]);
		ite.First();
		while (!ite.Is_Done()) {
			RendererListContainerClass* cont=ite.Peek_Obj();
			cont->Flush();
			ite.Next();
		}
	}
}


/***********************************************************************************************
 * ShdDX8RendererNodeClass::ShdDX8RendererNodeClass -- 
 * Init the mesh for rendering... this node is used for all subsequent rendering of
 * the particular mesh.
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *	  4/20/2002  jp  : Created
 *   5/26/2002  kjm : Updated stream handling for per-pixel shaders                                                                 *
 *   6/02/2002  kjm : Added light environment handling
 *   7/29/2002  kjm : Added VB and IB usage flags for software vertex shaders
 *=============================================================================================*/
ShdDX8RendererNodeClass::ShdDX8RendererNodeClass
(
	ShdDX8RendererClass::MeshContainerClass* container,
	ShdMeshClass* mesh,
	ShdSubMeshClass* sub_mesh
)
:	Container(container),
	Mesh(NULL),
	SubMesh(NULL),
	VertexBuffers(NULL),
	IndexBuffer(NULL)
{
	REF_PTR_SET(Container,container);
	REF_PTR_SET(Mesh,mesh);
	REF_PTR_SET(SubMesh,sub_mesh);

	// create usage depending on associated shader's processing behaviour KM
	DX8IndexBufferClass::UsageType ib_usage=DX8IndexBufferClass::USAGE_DEFAULT;

	if (!SubMesh->Peek_Shader()->Use_HW_Vertex_Processing())
	{
		ib_usage=DX8IndexBufferClass::USAGE_SOFTWAREPROCESSING;
	}

	int count=SubMesh->Get_Visible_Polygon_Count();
	if (!count) count=SubMesh->Get_Polygon_Count();
	unsigned index_count=count*3;
	if (SubMesh->Is_Sorting()) {
		IndexBuffer=new SortingIndexBufferClass(index_count);
	}
	else {
		IndexBuffer=new DX8IndexBufferClass(index_count,ib_usage);
	}
	IndexBufferClass::WriteLockClass ilock(IndexBuffer);
	const TriIndex* indices=SubMesh->Get_Polygon_Array();
	int i;
	int j=0;
	for (i=SubMesh->Get_First_Visible_Polygon();i<count+SubMesh->Get_First_Visible_Polygon();++i) 
	{
		ilock.Get_Index_Array()[j++]=indices[i][0];
		ilock.Get_Index_Array()[j++]=indices[i][1];
		ilock.Get_Index_Array()[j++]=indices[i][2];
	}

	// Don't use static vertex buffers if skin
	if (SubMesh->Get_Flag(MeshGeometryClass::SKIN)) {
		return;
	}

	// Compose the vertex and index buffers
	VertexStreamStruct vss;
	vss.Locations=SubMesh->Get_Vertex_Array();
	vss.Normals=SubMesh->Get_Vertex_Normal_Array();
	for (unsigned stage=0;stage<MAX_TEXTURE_STAGES;++stage) 
	{
		vss.UV[stage]=SubMesh->Get_UV_Array(stage);
	}
	vss.DiffuseInt=SubMesh->Get_Diffuse_Array();
	vss.S=SubMesh->Get_Tangent_Basis_S_Array();
	vss.T=SubMesh->Get_Tangent_Basis_T_Array();
	vss.SxT=SubMesh->Get_Tangent_Basis_SxT_Array();

	unsigned vertex_count=SubMesh->Get_Vertex_Count();
	unsigned stream_count=SubMesh->Peek_Shader()->Get_Vertex_Stream_Count();
	WWASSERT(stream_count>0);

	// create usage depending on associated shader's processing behaviour KM
	DX8VertexBufferClass::UsageType vb_usage=DX8VertexBufferClass::USAGE_DEFAULT;

	if (!SubMesh->Peek_Shader()->Use_HW_Vertex_Processing())
	{
		vb_usage=DX8VertexBufferClass::USAGE_SOFTWAREPROCESSING;
	}

	VertexBuffers=new VertexBufferClass*[stream_count];
	unsigned n;
	for (n=0;n<stream_count;++n) 
	{
		unsigned vertex_size=SubMesh->Peek_Shader()->Get_Vertex_Size(n);

		if (SubMesh->Is_Sorting()) {
			VertexBuffers[n]=
				new SortingVertexBufferClass(vertex_count);
		}
		else {
			VertexBuffers[n]=
				new DX8VertexBufferClass
				(
					0,
					vertex_count,
					vb_usage,
					vertex_size
				);
		}

		VertexBufferClass::WriteLockClass vlock(VertexBuffers[n]);
		SubMesh->Peek_Shader()->Copy_Vertex_Stream
		(
			n,
			vlock.Get_Vertex_Array(),
			vss,
			SubMesh->Get_Vertex_Count()
		);
	}
}

ShdDX8RendererNodeClass::~ShdDX8RendererNodeClass()
{
	unsigned stream_count=SubMesh->Peek_Shader()->Get_Vertex_Stream_Count();
	unsigned n;
	for (n=0;n<stream_count;++n) 
	{
		REF_PTR_RELEASE(VertexBuffers[n]);
	}
	delete[] VertexBuffers;
	VertexBuffers=NULL;

	REF_PTR_RELEASE(IndexBuffer);
	REF_PTR_RELEASE(Mesh);
	REF_PTR_RELEASE(SubMesh);
	REF_PTR_RELEASE(Container);
}

/***********************************************************************************************
 * ShdDX8RendererNodeClass::Render -- Render node
 * 
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *	  4/20/2002  jp  : Created
 *   6/02/2002  kjm : Added light environment handling
 *=============================================================================================*/
void ShdDX8RendererNodeClass::Render(RenderInfoClass& rinfo)
{
	RenderInfo=&rinfo;
	if (rinfo.light_environment)
	{
		memcpy(&LightEnvironment,rinfo.light_environment,sizeof(LightEnvironmentClass));
	}
	else {
		LightEnvironment.Reset(Vector3(0.0f,0.0f,0.0f),Vector3(0.0f,0.0f,0.0f));
	}

	Connect();
}

bool enable_rnd=true;
void ShdDX8RendererNodeClass::Flush(int cur_pass)
{
	WWASSERT(RenderInfo);

	// pass selection for this instance
	if (!SubMesh->Peek_Shader()->Pass_Selection(Mesh,RenderInfo,cur_pass))
	{
		return;
	}

	SNAPSHOT_SAY(("ShdDX8RendererNodeClass::Flush(%d) - %s\n",cur_pass,SubMesh->Get_Name()));

	// BEGIN OF SKIN CODE
	if (SubMesh->Get_Flag(MeshGeometryClass::SKIN)) {
		DX8Wrapper::Set_Vertex_Buffer(NULL);	// Free up the reference to the current vertex buffer
															// (in case it is the dynamic, which may have to be resized)

		unsigned vertex_count=SubMesh->Get_Vertex_Count();
		unsigned stream_count=SubMesh->Peek_Shader()->Get_Vertex_Stream_Count();
		WWASSERT(stream_count>0);

		// For now there are restrictions in vertex buffer format with skinning
		FVFInfoClass fi(dynamic_fvf_type);
		WWASSERT(SubMesh->Peek_Shader()->Get_Vertex_Size(0)==fi.Get_FVF_Size());

		DynamicVBAccessClass vb(
			SubMesh->Is_Sorting() ? BUFFER_TYPE_DYNAMIC_SORTING : BUFFER_TYPE_DYNAMIC_DX8,
			dynamic_fvf_type,
			vertex_count);
		SNAPSHOT_SAY(("DynamicVBAccess - %s - %d vertices\n",SubMesh->Is_Sorting() ? "sorting" : "non-sorting",vertex_count));

		if (_TempVertexBuffer.Length() < (int)vertex_count) _TempVertexBuffer.Resize(vertex_count); 
		if (_TempNormalBuffer.Length() < (int)vertex_count) _TempNormalBuffer.Resize(vertex_count);
		Vector3* loc=&(_TempVertexBuffer[0]);
		Vector3* norm=&(_TempNormalBuffer[0]);
		WWASSERT(Mesh->Get_Container());
		SubMesh->Get_Deformed_Vertices(loc,norm,Mesh->Get_Container()->Get_HTree());

		// Compose the vertex and index buffers
		VertexStreamStruct vss;
		vss.Locations=loc;
		vss.Normals=norm;
		for (unsigned stage=0;stage<MAX_TEXTURE_STAGES;++stage) 
		{
			vss.UV[stage]=SubMesh->Get_UV_Array(stage);
		}
		vss.DiffuseInt=SubMesh->Get_Diffuse_Array();
		vss.S=SubMesh->Get_Tangent_Basis_S_Array();
		vss.T=SubMesh->Get_Tangent_Basis_T_Array();
		vss.SxT=SubMesh->Get_Tangent_Basis_SxT_Array();

		WWASSERT(stream_count==1);
		{
			DynamicVBAccessClass::WriteLockClass vlock(&vb);
			SubMesh->Peek_Shader()->Copy_Vertex_Stream
			(
				0,//stream
				vlock.Get_Formatted_Vertex_Array(),
				vss,
				SubMesh->Get_Vertex_Count()
			);
		}
		DX8Wrapper::Set_Vertex_Buffer(vb);//stream 0

		DX8Wrapper::Set_Index_Buffer(IndexBuffer,0);
		DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix3D(true));//Mesh->Get_Transform());

		DX8Wrapper::Set_Light_Environment(&LightEnvironment);
		SubMesh->Peek_Shader()->Apply_Instance(cur_pass, *RenderInfo);

		if (SubMesh->Is_Sorting()) {
			SortingRendererClass::Insert_Triangles
			(
				0,
				SubMesh->Get_Visible_Polygon_Count() ? SubMesh->Get_Visible_Polygon_Count() : SubMesh->Get_Polygon_Count(),
				0,
				SubMesh->Get_Vertex_Count()
			);
		}
		else {
			DX8Wrapper::Draw_Triangles
			(
				0,
				SubMesh->Get_Visible_Polygon_Count() ? SubMesh->Get_Visible_Polygon_Count() : SubMesh->Get_Polygon_Count(),
				0,
				SubMesh->Get_Vertex_Count()
			);
		}

		return;
	}

	// END OF SKIN CODE


	for (unsigned n=0;n<SubMesh->Peek_Shader()->Get_Vertex_Stream_Count();++n) 
	{
		DX8Wrapper::Set_Vertex_Buffer(VertexBuffers[n],n);
	}

	DX8Wrapper::Set_Index_Buffer(IndexBuffer,0);
	DX8Wrapper::Set_Transform(D3DTS_WORLD,Mesh->Get_Transform());

	DX8Wrapper::Set_Light_Environment(&LightEnvironment);
	SubMesh->Peek_Shader()->Apply_Instance(cur_pass, *RenderInfo);

	if (SubMesh->Is_Sorting()) {
		SortingRendererClass::Insert_Triangles
		(
			0,
			SubMesh->Get_Visible_Polygon_Count() ? SubMesh->Get_Visible_Polygon_Count() : SubMesh->Get_Polygon_Count(),
			0,
			SubMesh->Get_Vertex_Count()
		);
	}
	else {
		DX8Wrapper::Draw_Triangles
		(
			0,
			SubMesh->Get_Visible_Polygon_Count() ? SubMesh->Get_Visible_Polygon_Count() : SubMesh->Get_Polygon_Count(),
			0,
			SubMesh->Get_Vertex_Count()
		);
	}

}

void ShdDX8RendererNodeClass::Apply_Shared_Shader_Settings(ShdRendererNodeClass* prev_node, int pass)
{
	// If the previously set shader is not the same type as this, we need to set
	// all render states to default state.
	if (prev_node) {
		ShdDX8RendererNodeClass* prev_dx8_node=static_cast<ShdDX8RendererNodeClass*>(prev_node); // Illegal to pass another type!
		ShdInterfaceClass* shd=prev_dx8_node->SubMesh->Peek_Shader();
		if (shd->Get_Class_ID()!=SubMesh->Peek_Shader()->Get_Class_ID()) {
			DX8Wrapper::Apply_Default_State();
		}
	}
	SubMesh->Peek_Shader()->Apply_Shared(pass, *RenderInfo);
}

