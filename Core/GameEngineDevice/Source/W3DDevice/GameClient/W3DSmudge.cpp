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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// W3DSmudge.cpp ////////////////////////////////////////////////////////////////////////////////
// Smudge System implementation
// Author: Mark Wilczynski, June 2003
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Lib/BaseType.h"
#include "always.h"
#include "W3DDevice/GameClient/W3DSmudge.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "Common/GameMemory.h"
#include "GameClient/View.h"
#include "GameClient/Display.h"
#include "WW3D2/texture.h"
#include "WW3D2/dx8indexbuffer.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/rinfo.h"
#include "WW3D2/camera.h"
#include "WW3D2/sortingrenderer.h"


SmudgeManager *TheSmudgeManager=NULL;

W3DSmudgeManager::W3DSmudgeManager(void)
{
}

W3DSmudgeManager::~W3DSmudgeManager()
{
	ReleaseResources();
}

void W3DSmudgeManager::init(void)
{
	SmudgeManager::init();
	ReAcquireResources();
	testHardwareSupport();
}

void W3DSmudgeManager::reset (void)
{
	SmudgeManager::reset();	//base
}

void W3DSmudgeManager::ReleaseResources(void)
{
#ifdef USE_COPY_RECTS
	REF_PTR_RELEASE(m_backgroundTexture);
#endif
	REF_PTR_RELEASE(m_indexBuffer);
}


#define SMUDGE_DRAW_SIZE	500	//draw at most 50 smudges per call. Tweak value to improve CPU/GPU parallelism.

static_assert(SMUDGE_DRAW_SIZE * 5 < 0x10000, "Vertex index exceeds 16-bit limit");


void W3DSmudgeManager::ReAcquireResources(void)
{
	ReleaseResources();

	SurfaceClass *surface=DX8Wrapper::_Get_DX8_Back_Buffer();
	SurfaceClass::SurfaceDescription surface_desc;

	surface->Get_Description(surface_desc);
	REF_PTR_RELEASE(surface);

	#ifdef USE_COPY_RECTS
	m_backgroundTexture = MSGNEW("TextureClass") TextureClass(TheTacticalView->getWidth(),TheTacticalView->getHeight(),surface_desc.Format,MIP_LEVELS_1,TextureClass::POOL_DEFAULT, true);
	#endif

	m_backBufferWidth = surface_desc.Width;
	m_backBufferHeight = surface_desc.Height;

	m_indexBuffer=NEW_REF(DX8IndexBufferClass,(SMUDGE_DRAW_SIZE*4*3));	//allocate 4 triangles per smudge, each with 3 indices.

	// Fill up the IB with static vertex indices that will be used for all smudges.
	{
		DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexBuffer);
		UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
		//quad of 4 triangles:
		//	0-----3
		//  |\   /|
		//  |  4  |
		//	|/   \|
		//  1-----2
		Int vbCount=0;
		for (Int i=0; i<SMUDGE_DRAW_SIZE; i++)
		{
			//Top
			ib[0]=vbCount;
			ib[1]=vbCount+4;
			ib[2]=vbCount+3;
			//Right
			ib[3]=vbCount+3;
			ib[4]=vbCount+4;
			ib[5]=vbCount+2;
			//Bottom
			ib[6]=vbCount+2;
			ib[7]=vbCount+4;
			ib[8]=vbCount+1;
			//Left
			ib[9]=vbCount+1;
			ib[10]=vbCount+4;
			ib[11]=vbCount+0;

			vbCount += 5;
			ib+=12;
		}
	}
}

Bool W3DSmudgeManager::testHardwareSupport(void)
{
	// Return cached result if already checked
	if (m_hardwareSupportStatus != SMUDGE_SUPPORT_UNKNOWN)
	{
		return m_hardwareSupportStatus == SMUDGE_SUPPORT_YES;
	}

	LPDIRECT3DDEVICE8 d3d8Device = DX8Wrapper::_Get_D3D_Device8();
	LPDIRECT3D8 d3d8Interface = DX8Wrapper::_Get_D3D8();
	if (!d3d8Device || !d3d8Interface)
	{
		m_hardwareSupportStatus = SMUDGE_SUPPORT_NO;
		return false;
	}

	D3DCAPS8 caps;
	d3d8Device->GetDeviceCaps(&caps);

	// DX8 requires dynamic textures to efficiently update smudge texture
	if (!(caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES))
	{
		m_hardwareSupportStatus = SMUDGE_SUPPORT_NO;
		return false;
	}

	IDirect3DTexture8 *backTexture = W3DShaderManager::getRenderTexture();
	if (!backTexture)
	{
		m_hardwareSupportStatus = SMUDGE_SUPPORT_NO;
		return FALSE;
	}

	IDirect3DSurface8* surface;
	if (FAILED(backTexture->GetSurfaceLevel(0, &surface)))
	{
		m_hardwareSupportStatus = SMUDGE_SUPPORT_NO;
		return false;
	}

	D3DSURFACE_DESC desc;
	surface->GetDesc(&desc);
	surface->Release();

	// Check if the device supports render-to-texture for this format
	HRESULT hr = d3d8Interface->CheckDeviceFormat(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
		DX8Wrapper::_Get_D3D_Back_Buffer_Format(),
		D3DUSAGE_RENDERTARGET,
		D3DRTYPE_TEXTURE,
		desc.Format
	);

	if (hr != D3D_OK)
	{
		m_hardwareSupportStatus = SMUDGE_SUPPORT_NO;
		return false;
	}

	m_hardwareSupportStatus = SMUDGE_SUPPORT_YES;
	return true;
}

void W3DSmudgeManager::render(RenderInfoClass &rinfo)
{
	//Verify that the card supports the effect.
	if (m_hardwareSupportStatus == SMUDGE_SUPPORT_NO)
		return;

	CameraClass &camera=rinfo.Camera;
	Vector3 vsVert;
	Vector4 ssVert;
	Real uvSpanX,uvSpanY;
	Vector3 vertex_offsets[4] = {
		Vector3(-0.5f, 0.5f, 0.0f),
		Vector3(-0.5f, -0.5f, 0.0f),
		Vector3(0.5f, -0.5f, 0.0f),
		Vector3(0.5f, 0.5f, 0.0f)
	};

#define THE_COLOR (0x00ffeedd)

	UnsignedInt vertexDiffuse[5]={THE_COLOR,THE_COLOR,THE_COLOR,THE_COLOR,THE_COLOR};

	Matrix4x4 proj;
	Matrix3D view;

	camera.Get_View_Matrix(&view);
	camera.Get_Projection_Matrix(&proj);

	SurfaceClass::SurfaceDescription surface_desc;
#ifdef USE_COPY_RECTS
	SurfaceClass *background=m_backgroundTexture->Get_Surface_Level();
	background->Get_Description(surface_desc);
#else
	D3DSURFACE_DESC D3DDesc;

	IDirect3DTexture8 *backTexture=W3DShaderManager::getRenderTexture();
	if (!backTexture || !W3DShaderManager::isRenderingToTexture())
		return;	//this card doesn't support render targets.

	backTexture->GetLevelDesc(0,&D3DDesc);

	surface_desc.Width = D3DDesc.Width;
	surface_desc.Height = D3DDesc.Height;
#endif

	Real texClampX = (Real)TheTacticalView->getWidth()/(Real)surface_desc.Width;
	Real texClampY = (Real)TheTacticalView->getHeight()/(Real)surface_desc.Height;

	Real texScaleX = texClampX*0.5f;
	Real texScaleY = texClampY*0.5f;

	//Do a first pass over the smudges to determine how many are visible
	//and to fill in their world-space positions and screen uv coordinates.
	//TODO: Optimize out this extra pass!
	//TODO: Find size of screen rectangle that actually needs copying.

	SmudgeSet *set=m_usedSmudgeSetList.Head();	//first set that didn't fit into render batch.
	Int count = 0;

	if (set)
	{	//there are possibly some smudges to render, so make sure background particles have finished drawing.
		SortingRendererClass::Flush();	//draw sorted translucent polys like particles.
	}

	while (set)
	{
		Smudge *smudge=set->getUsedSmudgeList().Head();

		while (smudge)
		{
			//Get view-space center
			Matrix3D::Transform_Vector(view,smudge->m_pos,&vsVert);

			//Get 5 view-space vertices
			Smudge::smudgeVertex *verts=smudge->m_verts;

			//Do center vertex outside 'for' loop since it's different.
			verts[4].pos = vsVert;

			for (Int i=0; i<4; i++)
			{
				verts[i].pos = vsVert + vertex_offsets[i] * smudge->m_size;
				//Ge uv coordinates for each vertex
				ssVert = proj * verts[i].pos;
				Real oow = 1.0f/ssVert.W;
				ssVert *= oow;	//returned in camera space which is -1,-1 (bottom-left) to 1,1 (top-right)
				//convert camera space to uv space: 0,0 (top-left), 1,1 (bottom-right)
				verts[i].uv.Set((ssVert.X+1.0f)*texScaleX,(1.0f-ssVert.Y)*texScaleY);

				Vector2 &thisUV=verts[i].uv;

				//Clamp coordinates so we're not referencing texels outside the view.
				WWMath::Clamp(thisUV.X, 0, texClampX);
				WWMath::Clamp(thisUV.Y, 0, texClampY);
			}

			//Finish center vertex
			//Ge uv coordinates by interpolating corner uv coordinates and applying desired offset.
			uvSpanX=verts[3].uv.X - verts[0].uv.X;
			uvSpanY=verts[1].uv.Y - verts[0].uv.Y;
			verts[4].uv.X=verts[0].uv.X+uvSpanX*(0.5f+smudge->m_offset.X);
			verts[4].uv.Y=verts[0].uv.Y+uvSpanY*(0.5f+smudge->m_offset.Y);

			count++;	//increment visible smudge count.
			smudge=smudge->Succ();
		}

		set=set->Succ();	//advance to next node.
	}

	if (!count)
	{
#ifdef USE_COPY_RECTS
		REF_PTR_RELEASE(background);
#endif
		return;	//nothing to render.
	}

#ifdef USE_COPY_RECTS
	SurfaceClass *backBuffer=DX8Wrapper::_Get_DX8_Back_Buffer();

	backBuffer->Get_Description(surface_desc);

	//Copy the area of backbuffer occupied by smudges into an alternate buffer.
	background->Copy(0,0,0,0,surface_desc.Width,surface_desc.Height,backBuffer);

	REF_PTR_RELEASE(background);
	REF_PTR_RELEASE(backBuffer);
#endif

	Matrix4x4 identity(true);
	DX8Wrapper::Set_Transform(D3DTS_WORLD,identity);
	DX8Wrapper::Set_Transform(D3DTS_VIEW,identity);

	DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
	//DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueSpriteShader);

	DX8Wrapper::Set_Shader(ShaderClass::_PresetAlphaShader);
#ifdef USE_COPY_RECTS
	DX8Wrapper::Set_Texture(0,m_backgroundTexture);
#else

	IDirect3DTexture8* renderTextureCopy = NULL;
	DX8Wrapper::_Get_D3D_Device8()->CreateTexture(
		D3DDesc.Width,
		D3DDesc.Height,
		1,
		0,
		D3DDesc.Format,
		D3DPOOL_DEFAULT,
		&renderTextureCopy);

	IDirect3DSurface8* srcRT = nullptr;
	IDirect3DSurface8* dstRT = nullptr;

	backTexture->GetSurfaceLevel(0, &srcRT);
	renderTextureCopy->GetSurfaceLevel(0, &dstRT);

	DX8Wrapper::_Get_D3D_Device8()->CopyRects(srcRT, nullptr, 0, dstRT, nullptr);

	srcRT->Release();
	dstRT->Release();

	DX8Wrapper::Set_DX8_Texture(0,renderTextureCopy);
	//Need these states in case texture is non-power-of-2
	DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
	DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
#endif
	VertexMaterialClass *vmat=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
	DX8Wrapper::Set_Material(vmat);
	REF_PTR_RELEASE(vmat);

	//Disable reading texture alpha since it's undefined.
	//DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2);

	Int smudgesBatchCount=0;
	Int smudgesRemaining=count;
	set=m_usedSmudgeSetList.Head();	//first smudge set that needs rendering.
	Smudge	*remainingSmudgeStart=set->getUsedSmudgeList().Head();	//first smudge that needs rendering.

	while (smudgesRemaining)	//keep drawing smudges until we run out.
	{
		//Now that we know how many smudges need rendering, allocate vertex buffer space and copy verts.
		count=smudgesRemaining;

		if (count > SMUDGE_DRAW_SIZE)
			count = SMUDGE_DRAW_SIZE;

		Int smudgesInRenderBatch=0;

		DynamicVBAccessClass vb_access(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,count*5);	//allocate 5 verts per smudge.
		{
			DynamicVBAccessClass::WriteLockClass lock(&vb_access);
			VertexFormatXYZNDUV2* verts=lock.Get_Formatted_Vertex_Array();

			while (set)
			{
				Smudge *smudge=remainingSmudgeStart;

				while (smudge)
				{
					Smudge::smudgeVertex *smVerts = smudge->m_verts;

					//Check if we exceeded maximum number of smudges allowed per draw call.
					if (smudgesInRenderBatch >= count)
					{
						remainingSmudgeStart = smudge;
						goto flushSmudges;
					}

					//Set center vertex opacity.
					vertexDiffuse[4] = ((Int)(smudge->m_opacity * 255.0f) << 24) | THE_COLOR;

					for (Int i=0; i<5; i++)
					{
						verts->x=smVerts->pos.X;
						verts->y=smVerts->pos.Y;
						verts->z=smVerts->pos.Z;
						verts->nx=0;	//keep AGP write-combining active
						verts->ny=0;
						verts->nz=0;
						verts->diffuse=vertexDiffuse[i];	//set to transparent
						verts->u1=smVerts->uv.X;
						verts->v1=smVerts->uv.Y;
						verts->u2=0;	//keep AGP write-combining active
						verts->v2=0;
						verts++;
						smVerts++;
					}

					smudgesInRenderBatch++;
					smudge=smudge->Succ();
				}

				set=set->Succ();	//advance to next node.

				if (set)	//start next batch at beginning of set.
					remainingSmudgeStart = set->getUsedSmudgeList().Head();
			}
		}
flushSmudges:
		++smudgesBatchCount;
		DX8Wrapper::Set_Vertex_Buffer(vb_access);

		DX8Wrapper::Draw_Triangles(	0,smudgesInRenderBatch*4, 0, smudgesInRenderBatch*5);

//Debug Code which draws outline around smudge
/*		DX8Wrapper::_Get_D3D_Device8()->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
		DX8Wrapper::_Get_D3D_Device8()->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLOROP,D3DTOP_SELECTARG2);
		DX8Wrapper::Draw_Triangles(	0,smudgesInRenderBatch*4, 0, smudgesInRenderBatch*5);
		DX8Wrapper::_Get_D3D_Device8()->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
		DX8Wrapper::_Get_D3D_Device8()->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
*/
		smudgesRemaining -= smudgesInRenderBatch;
	}

	// TheSuperHackers @bugfix xezon 15/06/2025 Draw a dummy point with the last vertex buffer
	// to force the GPU to flush all current pipeline state and commit the previous draw call.
	// This is required for some AMD models and drivers that refuse to flush a single draw call
	// for the smudges. This draw call is invisible and harmless.
	//if (smudgesBatchCount == 1)
	//{
	//	DX8Wrapper::_Get_D3D_Device8()->DrawPrimitive(D3DPT_POINTLIST, 0, 1);
	//}

	DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);

	renderTextureCopy->Release();
}
