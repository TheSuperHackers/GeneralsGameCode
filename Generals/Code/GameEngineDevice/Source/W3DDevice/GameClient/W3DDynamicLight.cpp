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

// W3DDynamicLight.cpp
// Class to handle dynamic lights.
// Author: John Ahlquist, April 2001
#include <stdlib.h>

#include "W3DDevice/GameClient/W3DDynamicLight.h"

namespace
{
Graphics::RenderLight Make_Modern_Light(const W3DDynamicLight &light) noexcept
{
	const Vector3 position = light.Get_Position();
	Vector3 diffuse;
	light.Get_Diffuse(&diffuse);
	return {
		Graphics::RenderLightType::Point,
		light.isEnabled() ? Graphics::RenderLightFlags::Enabled : Graphics::RenderLightFlags::None,
		{position.X, position.Y, position.Z},
		{0.0f, 0.0f, -1.0f},
		{diffuse.X, diffuse.Y, diffuse.Z},
		light.Get_Intensity(),
		light.Get_Attenuation_Range(),
		0.0f,
		0.0f
	};
}
}

W3DDynamicLight::W3DDynamicLight():
LightClass(LightClass::POINT)
{
	m_priorEnable = false;
	m_processMe = false;
	m_prevMinX = 0;
	m_prevMinY = 0;
	m_prevMaxX = 0;
	m_prevMaxY = 0;
	m_minX = 0;
	m_minY = 0;
	m_maxX = 0;
	m_maxY = 0;
	m_enabled = true;
	m_decayRange = false;
	m_decayColor = false;
	m_curDecayFrameCount = 0;
	m_curIncreaseFrameCount = 0;
	m_decayFrameCount = 0;
	m_increaseFrameCount = 0;
	m_targetRange = 0.0f;
	m_targetAmbient = {};
	m_targetDiffuse = {};
	m_modernLight = Graphics::CreatePointLight(Make_Modern_Light(*this));
}

W3DDynamicLight::~W3DDynamicLight()
{
	Graphics::DestroyPointLight(m_modernLight);
}

void W3DDynamicLight::setEnabled(Bool enabled)
{
	m_enabled = enabled;
	m_decayRange = false;
	m_decayFrameCount = 0;
	m_decayColor = false;
	m_increaseFrameCount = 0;
	Graphics::UpdatePointLight(m_modernLight, Make_Modern_Light(*this));
}

void W3DDynamicLight::On_Frame_Update()
{
	if (!m_enabled) {
		Graphics::UpdatePointLight(m_modernLight, Make_Modern_Light(*this));
		return;
	}
	Real factor = 1.0f;
	if (m_curIncreaseFrameCount>0 && m_increaseFrameCount>0) {
		// increasing
		m_curIncreaseFrameCount--;
		factor = (m_increaseFrameCount-m_curIncreaseFrameCount)/(Real)m_increaseFrameCount;

	}	else if (m_decayFrameCount==0) {
		factor = 1.0;  // never decays,
	}	else {
		m_curDecayFrameCount--;
		if (m_curDecayFrameCount == 0) {
			m_enabled = false;
			Graphics::UpdatePointLight(m_modernLight, Make_Modern_Light(*this));
			return;
		}
		factor = m_curDecayFrameCount/(Real)m_decayFrameCount;
	}
	if (m_decayRange) {
		this->FarAttenEnd = factor*m_targetRange;
		if (FarAttenEnd < FarAttenStart) {
			FarAttenEnd = FarAttenStart;
		}
	}
	if (m_decayColor) {
		this->Ambient = m_targetAmbient*factor;
		this->Diffuse = m_targetDiffuse*factor;
	}
	Graphics::UpdatePointLight(m_modernLight, Make_Modern_Light(*this));
}

void W3DDynamicLight::setFrameFade(UnsignedInt frameIncreaseTime, UnsignedInt decayFrameTime)
{
	m_decayFrameCount = decayFrameTime;
	m_curDecayFrameCount = decayFrameTime;
	m_curIncreaseFrameCount = frameIncreaseTime;
	m_increaseFrameCount = frameIncreaseTime;
	m_targetAmbient = Ambient;
	m_targetDiffuse = Diffuse;
	m_targetRange = FarAttenEnd;
	Graphics::UpdatePointLight(m_modernLight, Make_Modern_Light(*this));
}
