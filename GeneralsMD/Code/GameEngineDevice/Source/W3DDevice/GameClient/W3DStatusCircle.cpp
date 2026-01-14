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

#include "W3DDevice/GameClient/W3DStatusCircle.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"

#include <stdlib.h>
#include <assetmgr.h>
#include <texture.h>
#include <tri.h>
#include <colmath.h>
#include <coltest.h>
#include <rinfo.h>
#include <camera.h>
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/shader.h"
#include "Common/GlobalData.h"
#include "Common/MapObject.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/ScriptEngine.h"

#define SC_DETAIL_BLEND ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_ENABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::DETAILCOLOR_SCALE, ShaderClass::DETAILALPHA_DISABLE, ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_ENABLE, \
	ShaderClass::DETAILCOLOR_SCALE, ShaderClass::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA ( SHADE_CNST(ShaderClass::PASS_ALWAYS, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_ENABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA_Z ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE, ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_ENABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, no gradient, add src to dest.
#define SC_ADD ( SHADE_CNST(ShaderClass::PASS_ALWAYS, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ONE, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_DISABLE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_ENABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )



static ShaderClass detailOpaqueShader(SC_ALPHA);
Bool W3DStatusCircle::m_needUpdate;
Int W3DStatusCircle::m_diffuse=255; // blue.

W3DStatusCircle::~W3DStatusCircle(void)
{
	freeMapResources();
}

W3DStatusCircle::W3DStatusCircle(void)
{
	m_indexBuffer=nullptr;
	m_vertexMaterialClass=nullptr;
	m_vertexBufferCircle=nullptr;
	m_vertexBufferScreen=nullptr;
}


bool W3DStatusCircle::Cast_Ray(RayCollisionTestClass & raytest)
{

	return false;

}


//@todo: MW Handle both of these properly!!
W3DStatusCircle::W3DStatusCircle(const W3DStatusCircle & src)
{
	*this = src;
}

W3DStatusCircle & W3DStatusCircle::operator = (const W3DStatusCircle & that)
{
	assert(false);
	return *this;
}

void W3DStatusCircle::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	Vector3	ObjSpaceCenter((float)1000*0.5f,(float)1000*0.5f,(float)0);
	float length = ObjSpaceCenter.Length();

	sphere.Init(ObjSpaceCenter, length);
}

void W3DStatusCircle::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	Vector3	minPt(0,0,0);
	Vector3	maxPt((float)1000,(float)1000,(float)1000);
	box.Init(minPt,maxPt);
}

Int W3DStatusCircle::Class_ID(void) const
{
	return RenderObjClass::CLASSID_UNKNOWN;
}

RenderObjClass * W3DStatusCircle::Clone(void) const
{
	return NEW W3DStatusCircle(*this);
}


Int W3DStatusCircle::freeMapResources(void)
{

	REF_PTR_RELEASE(m_indexBuffer);
	REF_PTR_RELEASE(m_vertexBufferScreen);
	REF_PTR_RELEASE(m_vertexBufferCircle);
	REF_PTR_RELEASE(m_vertexMaterialClass);
	return 0;
}

#define NUM_TRI 20
//Allocate a heightmap of x by y vertices.
//data must be an array matching this size.
Int W3DStatusCircle::initData(void)
{
	Int i;

	m_needUpdate = true;
	freeMapResources();	//free old data and ib/vb

	m_numTriangles = NUM_TRI;
	m_indexBuffer=NEW_REF(DX8IndexBufferClass,(m_numTriangles*3));

	// Fill up the IB
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexBuffer);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();

	for (i=0; i<3*m_numTriangles; i+=3)
	{
		ib[0]=i;
		ib[1]=i+1;
		ib[2]=i+2;

		ib+=3;	//skip the 3 indices we just filled
	}

	m_vertexBufferCircle=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,m_numTriangles*3,DX8VertexBufferClass::USAGE_DEFAULT));
	m_vertexBufferScreen=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,2*3,DX8VertexBufferClass::USAGE_DEFAULT));

	//go with a preset material for now.
	m_vertexMaterialClass=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);

	m_shaderClass = ShaderClass(SC_ALPHA);// _PresetOpaque2DShader;//; //_PresetOpaqueShader;


	return 0;
}


/** updateCircleVB puts a circle with a team color vertex buffer. */

Int W3DStatusCircle::updateCircleVB(void)
{
	Int i, k;
	Real shade;
	DX8VertexBufferClass	*pVB = m_vertexBufferCircle;
	if (m_vertexBufferCircle )
	{
		m_needUpdate = false;
		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(pVB);
		VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();

		const Real theZ = 0.0f;
		const Real theRadius = 0.02f;
		const Int theAlpha = 127;
	  Int diffuse = m_diffuse + (theAlpha<<24);	 // b g<<8 r<<16 a<<24.
		Int limit = m_numTriangles;
		float curAngle = 0;
		float deltaAngle = 2*PI/limit;
		for (i=0; i<limit; i++)
		{

			shade=0.7f*255.0f;
			for (k=0; k<3; k++) {
				vb->z=  theZ;
				if (k==0) {
					vb->x=	0;
					vb->y=	0;
				} else if (k==1) {
					Vector3 vec(theRadius,0,theZ);
					vec.Rotate_Z(curAngle);
					vb->x=	vec.X;
					vb->y=	vec.Y;
				} else if (k==2) {
					Real angle = curAngle+deltaAngle;
					if (i==limit-1) {
						angle = 0;
					}
					Vector3 vec(theRadius,0,theZ);
					vec.Rotate_Z(angle);
					vb->x=	vec.X;
					vb->y=	vec.Y;
				}
				vb->diffuse = diffuse;
				vb->u1=0;
				vb->v1=0;
				vb++;
			}
			curAngle += deltaAngle;

		}
		return 0; //success.
	}
	return -1;
}

/** updateCircleVB puts a circle with a team color vertex buffer. */

Int W3DStatusCircle::updateScreenVB(Int diffuse)
{
	DX8VertexBufferClass	*pVB = m_vertexBufferScreen;
	if (m_vertexBufferScreen )
	{
		m_needUpdate = false;
		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(pVB);
		VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();

		vb->x =	-1;
		vb->y =	-1;
		vb->z = 0;
		vb->diffuse = diffuse;
		vb->u1=0;
		vb->v1=0;
		vb++;

		vb->x =	1;
		vb->y =	1;
		vb->z = 0;
		vb->diffuse = diffuse;
		vb->u1=0;
		vb->v1=0;
		vb++;

		vb->x =	-1;
		vb->y =	1;
		vb->z = 0;
		vb->diffuse = diffuse;
		vb->u1=0;
		vb->v1=0;
		vb++;

		vb->x =	-1;
		vb->y =	-1;
		vb->z = 0;
		vb->diffuse = diffuse;
		vb->u1=0;
		vb->v1=0;
		vb++;

		vb->x =	1;
		vb->y =	-1;
		vb->z = 0;
		vb->diffuse = diffuse;
		vb->u1=0;
		vb->v1=0;
		vb++;

		vb->x =	1;
		vb->y =	1;
		vb->z = 0;
		vb->diffuse = diffuse;
		vb->u1=0;
		vb->v1=0;
		vb++;
		return 0; //success.
	}
	return -1;
}

void W3DStatusCircle::Render(RenderInfoClass & rinfo)
{
	if (!TheGameLogic->isInGame() || TheGameLogic->getGameMode() == GAME_SHELL)
		return;

	if (m_indexBuffer == nullptr) {
		initData();
	}
	if (m_indexBuffer == nullptr) {
		return;
	}
	Bool setIndex = false;
	Matrix3D tm(true);
	if( TheGlobalData->m_showTeamDot )
	{
		if (m_needUpdate) {
			updateCircleVB();
		}
		//Apply the shader and material
		DX8Wrapper::Set_Material(m_vertexMaterialClass);
		DX8Wrapper::Set_Shader(m_shaderClass);
		DX8Wrapper::Set_Texture(0, nullptr);
		DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
		DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferCircle);
		setIndex = true;

		Vector3 vec(0.95f, 0.67f, 0);
		Matrix3x3 rot(true);

		tm.Set_Translation(vec);

		DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
		DX8Wrapper::Draw_Triangles(	0,NUM_TRI, 0,	(m_numTriangles*3));
	}


	ScriptEngine::TFade fade = TheScriptEngine->getFade();
	if (fade == ScriptEngine::FADE_NONE) {
		return;
	}

	if (!setIndex) {
		DX8Wrapper::Set_Material(m_vertexMaterialClass);
		DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
		DX8Wrapper::Set_Texture(0, nullptr);
	}

	tm.Make_Identity();
	Real intensity = TheScriptEngine->getFadeValue();
	Int clr = 255*intensity;
	Int diffuse = (0xff<<24)|(clr<<16)|(clr<<8)|clr;	 // b g<<8 r<<16 a<<24.
	updateScreenVB(diffuse);
	DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
	DX8Wrapper::Set_Shader(ShaderClass(SC_ADD));
	DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferScreen);
	DX8Wrapper::Apply_Render_State_Changes();
	switch (fade) {
		default:
		case ScriptEngine::FADE_ADD:
			DX8Wrapper::Draw_Triangles(	0,2, 0,	(2*3));
			break;
		case ScriptEngine::FADE_SUBTRACT:
			DX8Wrapper::Set_DX8_Render_State(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT );
			DX8Wrapper::Draw_Triangles(	0,2, 0,	(2*3));
			DX8Wrapper::Set_DX8_Render_State(D3DRS_BLENDOP, D3DBLENDOP_ADD );
			break;
		case ScriptEngine::FADE_SATURATE:
			// 4x multiply
			DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR);
			DX8Wrapper::Draw_Triangles(	0,2, 0,	(2*3));
			DX8Wrapper::Draw_Triangles(	0,2, 0,	(2*3));
			break;
		case ScriptEngine::FADE_MULTIPLY:
			// Straight multiply
			DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND,D3DBLEND_ZERO);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR);
			DX8Wrapper::Draw_Triangles(	0,2, 0,	(2*3));
			break;
	}
	ShaderClass::Invalidate();
}
