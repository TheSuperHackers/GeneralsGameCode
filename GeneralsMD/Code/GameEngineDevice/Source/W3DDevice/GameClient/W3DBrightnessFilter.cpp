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
//  (c) 2025																																//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DBrightnessFilter.cpp ///////////////////////////////////////////////
//
// Brightness adjustment post-processing filter implementation
//
// Author: @bobtista, November 2025
//
///////////////////////////////////////////////////////////////////////////////

#include "W3DDevice/GameClient/W3DBrightnessFilter.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "GameClient/Display.h"
#include "GameClient/View.h"
#include "Common/GameLOD.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/dx8caps.h"
#include "WW3D2/shader.h"
#include "WW3D2/vertmaterial.h"
#include <d3dx8.h>

/*===========================================================================================*/
/*=========      Pixel Shader Version														=============*/
/*===========================================================================================*/

DWORD ScreenBrightnessFilter::m_pixelShaderHandle = NULL;
Int ScreenBrightnessFilter::m_brightnessValue = 0;
Bool ScreenBrightnessFilter::m_isEnabled = false;

Int ScreenBrightnessFilter::init(void)
{
	m_pixelShaderHandle = NULL;
	m_brightnessValue = 0;
	m_isEnabled = false;

	if (!W3DShaderManager::canRenderToTexture()) {
		// Have to be able to render to texture.
		return false;
	}

	Int chipset = W3DShaderManager::getChipset();
	if (chipset >= DC_GENERIC_PIXEL_SHADER_1_1)
	{
		//shader declaration
		DWORD Declaration[] =
		{
			(D3DVSD_STREAM(0)),
			(D3DVSD_REG(0, D3DVSDT_FLOAT3)), // Position
			(D3DVSD_REG(1, D3DVSDT_D3DCOLOR)), // Diffuse
			(D3DVSD_REG(2, D3DVSDT_FLOAT2)), //  Texture Coordinates
			(D3DVSD_END())
		};

		//Brightness adjustment pixel shader
		HRESULT hr = W3DShaderManager::LoadAndCreateD3DShader("shaders\\brightness.pso", &Declaration[0], 0, false, &m_pixelShaderHandle);
		if (FAILED(hr))
			return FALSE;

		return TRUE;
	}
	return FALSE;
}

Int ScreenBrightnessFilter::shutdown(void)
{
	if (m_pixelShaderHandle)
	{
		DX8Wrapper::_Get_D3D_Device8()->DeletePixelShader(m_pixelShaderHandle);
		m_pixelShaderHandle = NULL;
	}
	return TRUE;
}

void ScreenBrightnessFilter::reset(void)
{
	// Reset to default state
	ShaderClass::Invalidate();
	DX8Wrapper::_Get_D3D_Device8()->SetTexture(0, NULL);
	DX8Wrapper::_Get_D3D_Device8()->SetPixelShader(0);
}

Bool ScreenBrightnessFilter::preRender(Bool &skipRender, CustomScenePassModes &scenePassMode)
{
	// Only enable if brightness is non-zero
	if (m_brightnessValue == 0)
	{
		skipRender = false;
		return false;
	}

	skipRender = false;
	W3DShaderManager::startRenderToTexture();
	return true;
}

Bool ScreenBrightnessFilter::postRender(FilterModes mode, Coord2D &scrollDelta, Bool &doExtraRender)
{
	// If brightness is zero, no adjustment needed
	if (m_brightnessValue == 0)
	{
		return false;
	}

	IDirect3DTexture8 * tex = W3DShaderManager::endRenderToTexture();
	DEBUG_ASSERTCRASH(tex, ("Require rendered texture."));
	if (!tex) return false;
	if (!set(mode)) return false;

	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();

	struct _TRANS_LIT_TEX_VERTEX {
		D3DXVECTOR4 p;
		DWORD color;   // diffuse color
		float u;
		float v;
	} v[4];

	Int xpos, ypos, width, height;

	pDev->SetTexture(0, tex);  // previously rendered frame inside this texture
	TheTacticalView->getOrigin(&xpos, &ypos);
	width = TheTacticalView->getWidth();
	height = TheTacticalView->getHeight();

	// Create fullscreen quad
	// bottom right
	v[0].p = D3DXVECTOR4(xpos + width - 0.5f, ypos + height - 0.5f, 0.0f, 1.0f);
	v[0].u = (Real)(xpos + width) / (Real)TheDisplay->getWidth();	v[0].v = (Real)(ypos + height) / (Real)TheDisplay->getHeight();
	// top right
	v[1].p = D3DXVECTOR4(xpos + width - 0.5f, ypos - 0.5f, 0.0f, 1.0f);
	v[1].u = (Real)(xpos + width) / (Real)TheDisplay->getWidth();	v[1].v = (Real)(ypos) / (Real)TheDisplay->getHeight();
	// bottom left
	v[2].p = D3DXVECTOR4(xpos - 0.5f, ypos + height - 0.5f, 0.0f, 1.0f);
	v[2].u = (Real)(xpos) / (Real)TheDisplay->getWidth();	v[2].v = (Real)(ypos + height) / (Real)TheDisplay->getHeight();
	// top left
	v[3].p = D3DXVECTOR4(xpos - 0.5f, ypos - 0.5f, 0.0f, 1.0f);
	v[3].u = (Real)(xpos) / (Real)TheDisplay->getWidth();	v[3].v = (Real)(ypos) / (Real)TheDisplay->getHeight();
	v[0].color = 0xffffffff;
	v[1].color = 0xffffffff;
	v[2].color = 0xffffffff;
	v[3].color = 0xffffffff;

	pDev->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(_TRANS_LIT_TEX_VERTEX));

	reset();
	return true;
}

Int ScreenBrightnessFilter::set(FilterModes mode)
{
	HRESULT hr;

	// Set up render states for fullscreen quad
	VertexMaterialClass *vmat = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
	DX8Wrapper::Set_Material(vmat);
	REF_PTR_RELEASE(vmat);
	DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueShader);
	DX8Wrapper::Set_Texture(0, NULL);
	DX8Wrapper::Apply_Render_State_Changes();

	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
	DX8Wrapper::Apply_Render_State_Changes();

	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();
	hr = pDev->SetPixelShader(m_pixelShaderHandle);

	// Calculate brightness parameters based on GenTool's algorithm
	// Range: -128 to +256
	// -128 to 0: darken (lift black level down, reduce brightness)
	// 0: neutral (no adjustment)
	// 1 to 128: lift black level linearly
	// 129 to 256: lift brightness while preserving black level (gamma adjustment)

	float blackLift = 0.0f;
	float gammaAdjust = 1.0f;
	float brightnessAdd = 0.0f;

	if (m_brightnessValue < 0)
	{
		// Darken: negative black lift
		blackLift = (float)m_brightnessValue / 128.0f;  // Range: -1.0 to 0.0
	}
	else if (m_brightnessValue <= 128)
	{
		// Lift black level linearly
		blackLift = (float)m_brightnessValue / 128.0f;  // Range: 0.0 to 1.0
	}
	else
	{
		// Lift brightness while preserving black level (gamma/brightness boost)
		// Range 129-256 maps to brightness increase
		float adjustAmount = (float)(m_brightnessValue - 128) / 128.0f;  // 0.0 to 1.0
		gammaAdjust = 1.0f / (1.0f + adjustAmount);  // Inverse gamma for brightening
	}

	// Set pixel shader constants
	// c0: (blackLift, gammaAdjust, brightnessAdd, unused)
	DX8Wrapper::_Get_D3D_Device8()->SetPixelShaderConstant(0, D3DXVECTOR4(blackLift, gammaAdjust, brightnessAdd, 1.0f), 1);

	return TRUE;
}

Bool ScreenBrightnessFilter::setup(FilterModes mode)
{
	// Brightness filter doesn't need special setup
	return true;
}

void ScreenBrightnessFilter::setBrightnessValue(Int value)
{
	// Clamp to GenTool's range
	if (value < -128) value = -128;
	if (value > 256) value = 256;

	m_brightnessValue = value;
	m_isEnabled = (value != 0);
}

Int ScreenBrightnessFilter::getBrightnessValue()
{
	return m_brightnessValue;
}

/*===========================================================================================*/
/*=========      Fixed-Function Version													=============*/
/*===========================================================================================*/

Int ScreenBrightnessFilterFixedFunction::m_brightnessValue = 0;
Bool ScreenBrightnessFilterFixedFunction::m_isEnabled = false;

Int ScreenBrightnessFilterFixedFunction::init(void)
{
	m_brightnessValue = 0;
	m_isEnabled = false;

	if (!W3DShaderManager::canRenderToTexture()) {
		return false;
	}

	// Fixed-function always supported on DirectX 8 hardware
	return TRUE;
}

Int ScreenBrightnessFilterFixedFunction::shutdown(void)
{
	return TRUE;
}

void ScreenBrightnessFilterFixedFunction::reset(void)
{
	ShaderClass::Invalidate();
	DX8Wrapper::_Get_D3D_Device8()->SetTexture(0, NULL);
}

Bool ScreenBrightnessFilterFixedFunction::preRender(Bool &skipRender, CustomScenePassModes &scenePassMode)
{
	if (m_brightnessValue == 0)
	{
		skipRender = false;
		return false;
	}

	skipRender = false;
	W3DShaderManager::startRenderToTexture();
	return true;
}

Bool ScreenBrightnessFilterFixedFunction::postRender(FilterModes mode, Coord2D &scrollDelta, Bool &doExtraRender)
{
	if (m_brightnessValue == 0)
	{
		return false;
	}

	IDirect3DTexture8 * tex = W3DShaderManager::endRenderToTexture();
	DEBUG_ASSERTCRASH(tex, ("Require rendered texture."));
	if (!tex) return false;
	if (!set(mode)) return false;

	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();

	struct _TRANS_LIT_TEX_VERTEX {
		D3DXVECTOR4 p;
		DWORD color;
		float u;
		float v;
	} v[4];

	Int xpos, ypos, width, height;

	pDev->SetTexture(0, tex);
	TheTacticalView->getOrigin(&xpos, &ypos);
	width = TheTacticalView->getWidth();
	height = TheTacticalView->getHeight();

	// Create fullscreen quad
	v[0].p = D3DXVECTOR4(xpos + width - 0.5f, ypos + height - 0.5f, 0.0f, 1.0f);
	v[0].u = (Real)(xpos + width) / (Real)TheDisplay->getWidth();	v[0].v = (Real)(ypos + height) / (Real)TheDisplay->getHeight();
	v[1].p = D3DXVECTOR4(xpos + width - 0.5f, ypos - 0.5f, 0.0f, 1.0f);
	v[1].u = (Real)(xpos + width) / (Real)TheDisplay->getWidth();	v[1].v = (Real)(ypos) / (Real)TheDisplay->getHeight();
	v[2].p = D3DXVECTOR4(xpos - 0.5f, ypos + height - 0.5f, 0.0f, 1.0f);
	v[2].u = (Real)(xpos) / (Real)TheDisplay->getWidth();	v[2].v = (Real)(ypos + height) / (Real)TheDisplay->getHeight();
	v[3].p = D3DXVECTOR4(xpos - 0.5f, ypos - 0.5f, 0.0f, 1.0f);
	v[3].u = (Real)(xpos) / (Real)TheDisplay->getWidth();	v[3].v = (Real)(ypos) / (Real)TheDisplay->getHeight();

	// Calculate brightness modulation via vertex color
	// This is a simpler approximation for cards without pixel shaders
	DWORD colorMod = 0xff808080;  // Default neutral gray
	if (m_brightnessValue < 0)
	{
		// Darken
		int val = 128 + m_brightnessValue;  // 0-128
		colorMod = D3DCOLOR_RGBA(val, val, val, 255);
	}
	else if (m_brightnessValue <= 128)
	{
		// Brighten via additive
		int val = 128 + (m_brightnessValue / 2);  // 128-192
		colorMod = D3DCOLOR_RGBA(val, val, val, 255);
	}
	else
	{
		// Maximum brightness
		colorMod = 0xffffffff;
	}

	v[0].color = colorMod;
	v[1].color = colorMod;
	v[2].color = colorMod;
	v[3].color = colorMod;

	pDev->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(_TRANS_LIT_TEX_VERTEX));

	reset();
	return true;
}

Int ScreenBrightnessFilterFixedFunction::set(FilterModes mode)
{
	// Set up render states for fullscreen quad
	VertexMaterialClass *vmat = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
	DX8Wrapper::Set_Material(vmat);
	REF_PTR_RELEASE(vmat);
	DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueShader);
	DX8Wrapper::Set_Texture(0, NULL);
	DX8Wrapper::Apply_Render_State_Changes();

	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
	DX8Wrapper::Apply_Render_State_Changes();

	// Use texture stage states to modulate brightness
	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();
	pDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	return TRUE;
}

Bool ScreenBrightnessFilterFixedFunction::setup(FilterModes mode)
{
	return true;
}

void ScreenBrightnessFilterFixedFunction::setBrightnessValue(Int value)
{
	if (value < -128) value = -128;
	if (value > 256) value = 256;

	m_brightnessValue = value;
	m_isEnabled = (value != 0);
}

Int ScreenBrightnessFilterFixedFunction::getBrightnessValue()
{
	return m_brightnessValue;
}


// TheSuperHackers @feature bobtista 11/02/2025 Helper function to set brightness on both filter implementations
// This is called from OptionsMenu to update brightness for whichever filter is active
void setBrightnessFilterValue(Int value)
{
	ScreenBrightnessFilter::setBrightnessValue(value);
	ScreenBrightnessFilterFixedFunction::setBrightnessValue(value);
}

