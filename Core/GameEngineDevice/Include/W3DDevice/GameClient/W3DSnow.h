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

// FILE: W3DSnow.h /////////////////////////////////////////////////////////

#pragma once

#include <cstddef>

#include "GameClient/Snow.h"

import Graphics.Scene.WeatherParticles;

class W3DRenderContext;
class W3DTextureHandle;

class W3DSnowManager : public SnowManager
{
  public :

	W3DSnowManager();
	virtual ~W3DSnowManager() override;

	virtual void init() override;
	virtual void reset() override;
	virtual void update () override;
	virtual void updateIniSettings() override;

	bool Prepare_Weather_Particles(Graphics::ParticleRenderer &renderer, const Graphics::View &view,
		Graphics::MaterialHandle material, const Graphics::WeatherParticleCullingBounds &bounds) noexcept;
	void Release_Weather_Particles(Graphics::ParticleRenderer &renderer) noexcept;
	std::size_t Weather_Particle_Count() const noexcept;
	void	ReleaseResources();
	Bool	ReAcquireResources();

 private:
	Graphics::WeatherParticles m_weatherParticles;
};
