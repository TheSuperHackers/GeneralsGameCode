/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#include "W3DDevice/GameClient/W3DRiver.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/dx8vertexbuffer.h"
#include "WW3D2/dx8indexbuffer.h"
#include "WW3D2/texture.h"
#include "WW3D2/vertmaterial.h"
#include "WW3D2/Renderer.h"
#include "Common/GlobalData.h"
#include "Common/GameLOD.h"
#include "GameClient/Color.h"
#include "GameClient/Water.h"
#include "GameLogic/PolygonTrigger.h"
#include "d3dx8math.h"

#define NOISE_REPEAT_FACTOR ((float)(1.0f/(16.0f)))
#define BUMP_SIZE (50.f)
#define REFLECTION_FACTOR 0.1f

static Bool wireframeForDebug = 0;

static Int getRiverVertexDiffuse(W3DShroud* shroud, Real x, Real y, Real shadeR, Real shadeG, Real shadeB, Int diffuse)
{
	if (!shroud)
		return diffuse;

	Int cellX = (Int)(x / shroud->getCellWidth());
	Int cellY = (Int)(y / shroud->getCellHeight());
	W3DShroudLevel level = shroud->getShroudLevel(cellX, cellY);
	Real shroudScale = (Real)level / 255.0f;
	return GameMakeColor(
		(Int)(shadeR * shroudScale),
		(Int)(shadeG * shroudScale),
		(Int)(shadeB * shroudScale),
		((diffuse >> 24) & 0xff) * shroudScale);
}

W3DRiver::W3DRiver() :
	m_texture(nullptr),
	m_alphaEdge(nullptr),
	m_pixelShader(0)
{
}

W3DRiver::~W3DRiver()
{
	DEBUG_ASSERTCRASH(m_pixelShader == 0, ("Release river device resources before destruction."));
	REF_PTR_RELEASE(m_texture);
	REF_PTR_RELEASE(m_alphaEdge);
}

void W3DRiver::init()
{
	if (!m_texture)
		m_texture = WW3DAssetManager::Get_Instance()->Get_Texture(TheWaterTransparency->m_standingWaterTexture.str());
	if (!m_alphaEdge)
		m_alphaEdge = WW3DAssetManager::Get_Instance()->Get_Texture("TWAlphaEdge.tga");
}

void W3DRiver::updateMapOverrides()
{
	if (m_texture && TheWaterTransparency->m_standingWaterTexture.compareNoCase(m_texture->Get_Texture_Name()) != 0)
	{
		REF_PTR_RELEASE(m_texture);
		m_texture = WW3DAssetManager::Get_Instance()->Get_Texture(TheWaterTransparency->m_standingWaterTexture.str());
	}
}

void W3DRiver::ReleaseResources(IDirect3DDevice8 *device)
{
	if (m_pixelShader)
		device->DeletePixelShader(m_pixelShader);
	m_pixelShader = 0;
}

void W3DRiver::ReAcquireResources(IDirect3DDevice8 *device)
{
	ReleaseResources(device);
	if (W3DShaderManager::getChipset() >= DC_GENERIC_PIXEL_SHADER_1_1)
		m_pixelShader = createPixelShader(device);

	// W3D invalidates textures on device loss; restore textures accessed directly while drawing.
	if (m_texture && !m_texture->Is_Initialized())
		m_texture->Init();
	if (m_alphaEdge && !m_alphaEdge->Is_Initialized())
		m_alphaEdge->Init();
}

UnsignedInt W3DRiver::createPixelShader(IDirect3DDevice8 *device)
{
	static const DWORD riverWaterShader[] =
	{
		D3DPS_VERSION(1, 1),
		0x00000042, 0xb00f0000, // tex t0
		0x00000042, 0xb00f0001, // tex t1
		0x00000042, 0xb00f0002, // tex t2
		0x00000042, 0xb00f0003, // tex t3
		0x00000005, 0x80070000, 0x90e40000, 0xb0e40000, // mul r0.rgb, v0, t0 ; blend vertex color into t0.
		0x00000001, 0x80080000, 0xb0e40000, // mov r0.a, t0 ; keep vertex alpha from fading the base water.
		0x00000005, 0x800f0001, 0xb0e40001, 0xb0e40002, // mul r1, t1, t2
		0x00000002, 0x80070001, 0x80e40001, 0xb0e40003, // add r1.rgb, r1, t3
		0x00000005, 0x80070001, 0x80e40001, 0x90ff0000, // mul r1.rgb, r1, v0.a
		0x40000005, 0x80080000, 0x80e40000, 0xb0e40003, // +mul r0.a, r0, t3
		0x00000002, 0x80070000, 0x80e40000, 0x80e40001, // add r0.rgb, r0, r1
		D3DPS_END(),
	};
	DWORD shader = 0;
	device->CreatePixelShader(riverWaterShader, &shader);
	return shader;
}

void W3DRiver::setupShader(IDirect3DDevice8 *device, const RenderSettings& settings) const
{
	if (!TheWaterTransparency->m_additiveBlend)
		DX8Wrapper::Set_Shader(ShaderClass::_PresetAlphaShader);
	else
		DX8Wrapper::Set_Shader(ShaderClass::_PresetAdditiveShader);

	VertexMaterialClass* vmat = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
	DX8Wrapper::Set_Material(vmat);
	REF_PTR_RELEASE(vmat);
	m_texture->Get_Filter().Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_BEST);
	m_texture->Get_Filter().Set_Min_Filter(TextureFilterClass::FILTER_TYPE_BEST);
	m_texture->Get_Filter().Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_BEST);

	DX8Wrapper::Apply_Render_State_Changes(); //force update of view and projection matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP, D3DTOP_ADD);
	if (!m_alphaEdge->Is_Initialized())
		m_alphaEdge->Init();
	device->SetTexture(3, m_alphaEdge->Peek_D3D_Texture());
	DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
	DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 0);
	DX8Wrapper::Set_DX8_Texture_Stage_State(3, D3DTSS_TEXCOORDINDEX, 1);

	Bool doSparkles = true;

	if (m_pixelShader && doSparkles)
	{
		if (!settings.sparkles->Is_Initialized())
			settings.sparkles->Init();
		device->SetTexture(1, settings.sparkles->Peek_D3D_Texture());

		if (!settings.noise->Is_Initialized())
			settings.noise->Init();
		device->SetTexture(2, settings.noise->Peek_D3D_Texture());

		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

		DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
		// Two output coordinates are used.
		DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		DX8Wrapper::Set_DX8_Texture_Stage_State(2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

		D3DXMATRIX curView;
		DX8Wrapper::_Get_DX8_Transform(D3DTS_VIEW, curView);
		D3DXMATRIX inv;
		float det;
		D3DXMatrixInverse(&inv, &det, &curView);
		D3DXMATRIX scale;
		D3DXMatrixScaling(&scale, NOISE_REPEAT_FACTOR, NOISE_REPEAT_FACTOR, 1);
		D3DXMATRIX destMatrix = inv * scale;
		D3DXMatrixTranslation(&scale, settings.phase, settings.phase, 0);
		destMatrix = destMatrix * scale;
		DX8Wrapper::_Set_DX8_Transform(D3DTS_TEXTURE2, destMatrix);
	}
	device->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	device->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	device->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	device->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	device->SetTextureStageState(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	device->SetTextureStageState(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	device->SetTextureStageState(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	device->SetTextureStageState(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	if (m_pixelShader)
	{
		device->SetPixelShaderConstant(0, D3DXVECTOR4(REFLECTION_FACTOR, REFLECTION_FACTOR, REFLECTION_FACTOR, 1.0f), 1);
		device->SetPixelShader(m_pixelShader);
	}
}

Bool W3DRiver::draw(IDirect3DDevice8 *device, PolygonTrigger* pTrig, const RenderSettings& settings) const
{
	Int rectangleCount = pTrig->getNumPoints() / 2;
	rectangleCount--;

	Real bumpFactor = 5;
	static Bool doWobble = true;

	//allocate 2 triangles per side with 3 indices per triangle
	const Int indexCount = (rectangleCount + 1) * 2 * 3;
	// TheSuperHackers @bugfix arcticdolphin 12/10/2026 Skip rivers that do not fit the 16 bit dynamic buffers.
	DEBUG_ASSERTCRASH(indexCount <= DynamicIBAccessClass::MAX_INDEX_COUNT, ("River has too many points to draw"));
	if (indexCount > DynamicIBAccessClass::MAX_INDEX_COUNT)
		return false;

	DynamicIBAccessClass ib_access(BUFFER_TYPE_DYNAMIC_DX8, indexCount);
	{
		DynamicIBAccessClass::WriteLockClass lockib(&ib_access);
		UnsignedShort* curIb = lockib.Get_Index_Array();
		for (Int i = 0; i < rectangleCount; i++)
		{
			//triangle 1
			curIb[0] = i * 2;
			curIb[1] = i * 2 + 1;
			curIb[2] = i * 2 + 3;

			//triangle 2
			curIb[3] = i * 2;
			curIb[4] = i * 2 + 3;
			curIb[5] = i * 2 + 2;

			curIb += 6; //skip the 6 indices we just added.
		}
	}

	Real shadeR = TheWaterTransparency->m_standingWaterColor.red;
	Real shadeG = TheWaterTransparency->m_standingWaterColor.green;
	Real shadeB = TheWaterTransparency->m_standingWaterColor.blue;

	//If the water color is not overridden, use legacy lighting code.
	if (shadeR == 1.0f && shadeG == 1.0f && shadeB == 1.0f)
	{
		shadeR = TheGlobalData->m_terrainAmbient[0].red;
		shadeG = TheGlobalData->m_terrainAmbient[0].green;
		shadeB = TheGlobalData->m_terrainAmbient[0].blue;

		//Add in diffuse lighting from each terrain light
		for (Int lightIndex = 0; lightIndex < TheGlobalData->m_numGlobalLights; lightIndex++)
		{
			if (-TheGlobalData->m_terrainLightPos[lightIndex].z > 0)
			{
				shadeR += -TheGlobalData->m_terrainLightPos[lightIndex].z * TheGlobalData->m_terrainDiffuse[lightIndex].red;
				shadeG += -TheGlobalData->m_terrainLightPos[lightIndex].z * TheGlobalData->m_terrainDiffuse[lightIndex].green;
				shadeB += -TheGlobalData->m_terrainLightPos[lightIndex].z * TheGlobalData->m_terrainDiffuse[lightIndex].blue;
			}
		}

		//Get water material colors
		Real waterShadeR = (settings.waterDiffuse & 0xff) / 255.0f;
		Real waterShadeG = ((settings.waterDiffuse >> 8) & 0xff) / 255.0f;
		Real waterShadeB = ((settings.waterDiffuse >> 16) & 0xff) / 255.0f;

		shadeR = shadeR * waterShadeR * 255.0f;
		shadeG = shadeG * waterShadeG * 255.0f;
		shadeB = shadeB * waterShadeB * 255.0f;
	}
	else
	{
		shadeR = shadeR * 255.0f;
		shadeG = shadeG * 255.0f;
		shadeB = shadeB * 255.0f;

		if (shadeR == 0 && shadeG == 0 && shadeB == 0)
		{
			//special case where we disable lighting
			shadeR = 255;
			shadeG = 255;
			shadeB = 255;
		}
	}

	Int diffuse = REAL_TO_INT(shadeB) | (REAL_TO_INT(shadeG) << 8) | (REAL_TO_INT(shadeR) << 16);

	//Keep diffuse from lighting calculations but substitute custom alpha
	diffuse |= settings.waterDiffuse & 0xff000000; //copy alpha/opacity from ini setting

	Int innerNdx = pTrig->getRiverStart();
	Int outerNdx = innerNdx + 1;

	Real endLen = 0;
	Real totalLen = 0;
	Int i;
	for (i = 0; i < pTrig->getNumPoints() - 1; i++)
	{
		ICoord3D innerPt = *pTrig->getPoint(i);
		ICoord3D outerPt = *pTrig->getPoint(i + 1);
		Real dx = innerPt.x - outerPt.x;
		Real dy = innerPt.y - outerPt.y;
		Real curLen = sqrt(dx * dx + dy * dy);
		totalLen += curLen;
		if (i == innerNdx) { endLen = curLen; }
	}
	bumpFactor = endLen / BUMP_SIZE;

	Real lengthOfRiver = (totalLen / 2) - endLen;
	Real repeatCount = lengthOfRiver / (endLen);

	Real vScale = (Real)repeatCount / (Real)rectangleCount;

#define HEIGHT_TO_USE (0.5f)
	if (innerNdx >= pTrig->getNumPoints() - 1)
		return true;
	//allocate 2 vertices per side
	DynamicVBAccessClass vb_access(BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, (rectangleCount + 1) * 2);
	{
		DynamicVBAccessClass::WriteLockClass lock(&vb_access);
		VertexFormatXYZNDUV2* vb = lock.Get_Formatted_Vertex_Array();

		Real constA = 3 * settings.phase;

		// TheSuperHackers @bugfix afc-afc0 14/04/2026 Apply shroud per-vertex to avoid double-darkening
		// at river borders.
		W3DShroud* shroud = TheTerrainRenderObject ? TheTerrainRenderObject->getShroud() : nullptr;

		for (i = 0; i < (pTrig->getNumPoints() / 2); i++)
		{
			Real x, y;
			ICoord3D innerPt = *pTrig->getPoint(outerNdx);
			ICoord3D outerPt = *pTrig->getPoint(innerNdx);
			outerNdx++;
			innerNdx--;
			if (innerNdx < 0) { innerNdx = pTrig->getNumPoints() - 1; }
			if (outerNdx >= pTrig->getNumPoints()) { outerNdx = 0; }
			x = innerPt.x;
			y = innerPt.y;

			vb->x = x;
			vb->y = y;

			vb->z = innerPt.z;

			vb->diffuse = getRiverVertexDiffuse(shroud, x, y, shadeR, shadeG, shadeB, diffuse);

			Real wobbleConst = -settings.phase + vScale * (Real)i + WWMath::Fast_Sin(2 * PI * (vScale * (Real)i) - constA) / 22.0f;
			//old slower version
			//vb->v1=-settings.phase+vScale*(Real)i + wobble(vScale*i, settings.phase, doWobble);
			vb->v1 = wobbleConst;
			vb->u1 = HEIGHT_TO_USE;
			//old slower version
			//vb->v2 = -settings.phase+vScale*(Real)i + wobble(vScale*i, settings.phase, doWobble);
			vb->v2 = wobbleConst;
			vb->u2 = 1.0f;
			vb->nx = 0;
			vb->ny = 0;
			vb->nz = 1.0f;
			vb++;

			x = outerPt.x;
			y = outerPt.y;

			vb->x = x;
			vb->y = y;
			vb->z = outerPt.z;

			vb->diffuse = getRiverVertexDiffuse(shroud, x, y, shadeR, shadeG, shadeB, diffuse);
			//old slower version
			//vb->v1=-settings.phase+vScale*(Real)i + wobble(vScale*i, settings.phase, doWobble);
			vb->v1 = wobbleConst;
			vb->u1 = 0;
			//old slower version
			//vb->v2 = -settings.phase+vScale*(Real)i + wobble(vScale*i, settings.phase, doWobble);
			vb->v2 = wobbleConst;
			vb->u2 = 0;
			vb->nx = 0;
			vb->ny = 0;
			vb->nz = 1.0f;
			vb++;
		}
	}

	Matrix3D tm(1);

	Renderer::Set_Transform(RenderTransform_World, tm); //position the water surface
	DX8Wrapper::Set_Index_Buffer(ib_access, 0);
	DX8Wrapper::Set_Vertex_Buffer(vb_access);
	DX8Wrapper::Set_Texture(0, m_texture); //set to blue

	setupShader(device, settings);

	//In additive blending we need to use the alpha at the edges of river to darken
	//rgb instead.
	if (TheWaterTransparency->m_additiveBlend)
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);

	if (m_pixelShader)
		device->SetPixelShader(m_pixelShader);
	DWORD cull;
	device->GetRenderState(D3DRS_CULLMODE, &cull);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	if (wireframeForDebug) { device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME); }
	DX8Wrapper::Draw_Triangles(0, rectangleCount * 2, 0, (rectangleCount + 1) * 2);
	if (wireframeForDebug) { device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID); }

	if (m_pixelShader)
		device->SetPixelShader(0);

	//restore blend mode to what W3D expects.
	if (TheWaterTransparency->m_additiveBlend)
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_ONE);

	device->SetRenderState(D3DRS_CULLMODE, cull);

	return true;
}