import Graphics.Frame.RenderClock;
#include <array>
#include <span>

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

#include "W3DDevice/GameClient/W3DSnow.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"

#include <cmath>
#include <cstddef>
#include <algorithm>
#include <span>



namespace
{
constexpr std::size_t WEATHER_PARTICLE_CAPACITY = 65536;
}

W3DSnowManager::W3DSnowManager()
{
}

W3DSnowManager::~W3DSnowManager()
{
	ReleaseResources();
}

void W3DSnowManager::init()
{
	SnowManager::init();
	m_weatherParticles.Initialize(WEATHER_PARTICLE_CAPACITY);
	ReAcquireResources();
}

/** Releases all renderer resources before a reset. */
void W3DSnowManager::ReleaseResources()
{
    m_weatherParticles.Reset();
}

/** (Re)allocates all renderer resources after a reset. */
Bool W3DSnowManager::ReAcquireResources()
{
    return TRUE;
}

void W3DSnowManager::updateIniSettings()
{
	//Call base class
	SnowManager::updateIniSettings();

}

void W3DSnowManager::reset()
{
	SnowManager::reset();
	m_weatherParticles.Reset();
}

void W3DSnowManager::update()
{
	// TheSuperHackers @tweak The snow render update is now decoupled from the logic step.
	m_time += Graphics::Get_Render_Clock().Logic_Frame_Time_Seconds();

	//find current time offset, adjusting for overflow
	m_time=fmod(m_time,m_fullTimePeriod);
}

#define MAXIMUM_CAMERA_DISTANCE 100000	//maximum distance of camera position from world origin.
#define ISPOW2(x)  (x && (x & (x-1)) == 0)	//is a number a power of 2?
#define MODPOW2(x,y) ((x) & (y-1))		//mod '%' operator for powers of 2.

bool W3DSnowManager::Prepare_Weather_Particles(Graphics::ParticleRenderer &renderer, const Graphics::View &view,
	Graphics::MaterialHandle material, const Graphics::WeatherParticleCullingBounds &bounds) noexcept
{
	if (!m_weatherParticles.Is_Initialized() || !renderer.Is_Initialized())
		return false;
	if (TheWeatherSetting == nullptr || !TheWeatherSetting->m_snowEnabled || !m_isVisible || m_startingHeights == nullptr) {
		m_weatherParticles.Reset();
		return true;
	}

	const bool point_sprites = TheWeatherSetting->m_usePointSprites != FALSE;
	Graphics::ParticleEmitterFlags flags = Graphics::ParticleEmitterFlags::Enabled | Graphics::ParticleEmitterFlags::Billboard;
	if (point_sprites)
		flags = flags | Graphics::ParticleEmitterFlags::PointSprite;
	const float particle_size = point_sprites
		? std::clamp(m_pointSize, m_minPointSize, m_maxPointSize)
		: 0.5f * std::max(0.0f, m_quadSize);
	const Graphics::WeatherParticleFieldDescription description{
		std::span<const float>(m_startingHeights, SnowManager::SNOW_NOISE_X * SnowManager::SNOW_NOISE_Y),
		SnowManager::SNOW_NOISE_X,
		SnowManager::SNOW_NOISE_Y,
		m_boxDimensions,
		m_emitterSpacing,
		m_velocity,
		m_frequencyScaleX,
		m_frequencyScaleY,
		m_amplitude,
		particle_size,
		std::max(0.0f, m_amplitude) + std::max(0.0f, m_quadSize),
		{1.0f, 1.0f, 1.0f, 1.0f},
		material.Is_Valid() ? material : renderer.Default_Material(),
		flags,
		renderer.Pipeline_For_Flags(flags)
	};
	if (!m_weatherParticles.Configure(description)
		|| !m_weatherParticles.Bind(renderer)
		|| !m_weatherParticles.Update(description, view.position, m_time, view, bounds)
		|| !m_weatherParticles.Append())
		return false;
	return true;
}

void W3DSnowManager::Release_Weather_Particles(Graphics::ParticleRenderer &renderer) noexcept
{
	m_weatherParticles.Unbind(renderer);
	m_weatherParticles.Reset();
}

std::size_t W3DSnowManager::Weather_Particle_Count() const noexcept
{
	return m_weatherParticles.Particle_Count();
}
