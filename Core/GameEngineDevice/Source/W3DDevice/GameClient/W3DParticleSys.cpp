#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DSmudge.h"
#include "W3DDevice/GameClient/W3DSnow.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string_view>
#include <vector>

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GENERALS_GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GENERALS_GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif

import Graphics.Scene.Particles.Renderer;
import Assets.Runtime;
import Assets.Cache;
import Assets.Textures;
import Graphics.Scene.Beams;
import Graphics.Scene.Screen.Distortion;

namespace
{

bool Build_Graphics_Particle_Texture(const char *texture_name, Graphics::Texture &description, std::vector<std::byte> &pixels)
{
    GENERALS_GRAPHICS_PROFILE_SCOPE("Build_Graphics_Particle_Texture");
    if (texture_name==nullptr || *texture_name=='\0') return false;
    auto* cache=Assets::Try_Get_Asset_Cache();
    if (!cache) return false;
    const auto handle=cache->Request_Texture(texture_name);
    cache->Wait(handle);
    const auto* texture=cache->Try_Get_Texture(handle);
    if (!texture || !texture->Has_Pixels()) return false;
    const auto source=texture->Pixels();
    pixels.assign(source.begin(),source.end());
    description.width=texture->Width();
    description.height=texture->Height();
    description.depth=1;
    description.mip_count=1;
    description.format=Graphics::TextureFormat::RGBA8_UNorm;
    description.usage=Graphics::TextureUsage::Sampled;
    description.row_pitch=texture->Row_Pitch();
    description.pixel_data=pixels;
    return true;
}

}

W3DParticleSystemManager::W3DParticleSystemManager()
{
	m_graphicsEmitters.reserve(1024);
	m_graphicsStreaks.reserve(256);
	m_graphicsMaterials.reserve(256);
	m_readyToRender = false;
	m_graphicsParticlesPrepared = false;
	m_onScreenParticleCount = 0;
}

W3DParticleSystemManager::~W3DParticleSystemManager()
{
	Reset_Graphics_Particle_Bindings();
}

void W3DParticleSystemManager::queueParticleRender()
{
	m_readyToRender = true;
}

void DoParticles(W3DRenderContext &rinfo)
{
	if (TheParticleSystemManager)
		TheParticleSystemManager->doParticles(rinfo);
}

void W3DParticleSystemManager::doParticles(W3DRenderContext &rinfo)
{
	GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::doParticles");
	if (!m_readyToRender)
		return;

	m_readyToRender = false;
	m_onScreenParticleCount = 0;
	m_fieldParticleCount = 0;
	m_terrainBoundsValid = false;
	m_weatherParticlesReady = false;
	Matrix3D legacy_view;
	Matrix4x4 legacy_projection;
	rinfo.Camera.Get_View_Matrix(&legacy_view);
	rinfo.Camera.Get_Backend_Projection_Matrix(&legacy_projection);
	Graphics::Matrix4x4 graphics_view;
	Graphics::Matrix4x4 graphics_projection;
	for (std::size_t row = 0; row < 4; ++row) {
		for (std::size_t column = 0; column < 4; ++column) {
			graphics_view.values[row * 4 + column] = (row < 3 ? legacy_view[row][column] : (column == 3 ? 1.0f : 0.0f));
			graphics_projection.values[row * 4 + column] = legacy_projection[row][column];
		}
	}
	const Vector3 camera_position = rinfo.Camera.Get_Position();
	Set_Graphics_Particle_View(Graphics::View(
		graphics_view,
		graphics_projection,
		{camera_position.X, camera_position.Y, camera_position.Z},
		m_graphicsView.viewport));
	if (TheTerrainRenderObject != nullptr) {
		AABoxClass bounds;
		TheTerrainRenderObject->getMaximumVisibleBox(rinfo.Camera.Get_Frustum(), &bounds, TRUE);
		m_terrainCenterX = bounds.Center.X;
		m_terrainCenterY = bounds.Center.Y;
		m_terrainCenterZ = bounds.Center.Z;
		m_terrainExtentX = bounds.Extent.X;
		m_terrainExtentY = bounds.Extent.Y;
		m_terrainExtentZ = bounds.Extent.Z;
		m_terrainBoundsValid = true;
	}
	Prepare_Graphics_Particles();
	TheParticleSystemManager->setOnScreenParticleCount(m_onScreenParticleCount);
}

void W3DParticleSystemManager::Reset_Graphics_Particle_Bindings() noexcept
{
	Graphics::ParticleRenderer &renderer = Graphics::GetParticleRenderer();
	for (const GraphicsEmitterBinding &binding : m_graphicsEmitters)
		if (renderer.Is_Initialized())
			renderer.Destroy_Emitter(binding.graphics_emitter);
	for (GraphicsStreakBinding &binding : m_graphicsStreaks)
		if (Graphics::GetBeamRenderer().Is_Initialized()) {
			for (Graphics::BeamHandle beam : binding.beams)
				Graphics::GetBeamRenderer().Destroy(beam);
			if (binding.material.Is_Valid())
				Graphics::GetBeamRenderer().Destroy_Material(binding.material);
			if (binding.texture.Is_Valid())
				Graphics::GetBeamRenderer().Destroy_Texture(binding.texture);
		}
	if (TheSnowManager != nullptr)
		static_cast<W3DSnowManager *>(TheSnowManager)->Release_Weather_Particles(renderer);
	for (const GraphicsMaterialBinding &binding : m_graphicsMaterials) {
		if (renderer.Is_Initialized()) {
			renderer.Destroy_Material(binding.material);
			renderer.Destroy_Texture(binding.texture);
		}
	}
	m_graphicsEmitters.clear();
	m_graphicsStreaks.clear();
	m_graphicsMaterials.clear();
	m_graphicsSyncStamp = 0;
	m_graphicsParticlesPrepared = false;
	m_weatherParticlesReady = false;
}

bool W3DParticleSystemManager::Set_Graphics_Particle_View(const Graphics::View &view) noexcept
{
	m_graphicsView = view;
	m_graphicsViewValid = true;
	return Graphics::GetParticleRenderer().Set_View(view);
}

bool W3DParticleSystemManager::Render_Graphics_Particles(Graphics::CommandList &commands, const Graphics::FrameTargets &targets) noexcept
{
	if (!m_graphicsParticlesPrepared)
		return true;

	if (!Graphics::GetParticleRenderer().Render(
		commands,
		targets.backbuffer.texture,
		targets.depth.texture,
		{0, 0, targets.backbuffer.width, targets.backbuffer.height, 0.0f, 1.0f}))
		return false;

	return Graphics::GetScreenDistortionRenderer().Render(
			commands,
			targets.backbuffer.texture,
			targets.depth.texture,
			{0, 0, targets.backbuffer.width, targets.backbuffer.height, 0.0f, 1.0f},
			{
				std::span<const float>(m_graphicsSmudgePositionX.data(), m_graphicsSmudgeCount),
				std::span<const float>(m_graphicsSmudgePositionY.data(), m_graphicsSmudgeCount),
				std::span<const float>(m_graphicsSmudgePositionZ.data(), m_graphicsSmudgeCount),
				std::span<const float>(m_graphicsSmudgeOffsetX.data(), m_graphicsSmudgeCount),
				std::span<const float>(m_graphicsSmudgeOffsetY.data(), m_graphicsSmudgeCount),
				std::span<const float>(m_graphicsSmudgeSizes.data(), m_graphicsSmudgeCount),
				std::span<const float>(m_graphicsSmudgeOpacities.data(), m_graphicsSmudgeCount),
                {1.0f,238.0f/255.0f,221.0f/255.0f}
			});
}

void W3DParticleSystemManager::Prepare_Graphics_Particles()
{
	GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Prepare_Graphics_Particles");
	Graphics::ParticleRenderer &renderer = Graphics::GetParticleRenderer();
	m_graphicsParticlesPrepared = renderer.Is_Initialized();
	if (!m_graphicsParticlesPrepared)
		return;

	++m_graphicsSyncStamp;
	if (m_graphicsSyncStamp == 0)
		m_graphicsSyncStamp = 1;
	{
		GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Reset_Graphics_Particles");
		renderer.Reset_Particles();
	}
	m_graphicsSmudgeCount = 0;
	{
		GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Prepare_Weather_Snow");
		m_weatherParticlesReady = Prepare_Weather_Snow();
	}
	if (TheSmudgeManager != nullptr && TheGlobalData != nullptr && TheGlobalData->m_useHeatEffects) {
		static_cast<W3DSmudgeManager *>(TheSmudgeManager)->resetDraw();
	}

	ParticleSystemManager::ParticleSystemList &systems = TheParticleSystemManager->getAllParticleSystems();
	for (ParticleSystem *system : systems) {
		GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Prepare_Graphics_System");
		if (system == nullptr || system->isUsingDrawables())
			continue;
		if (system->isUsingSmudge()) {
			if (TheSmudgeManager != nullptr && TheGlobalData != nullptr && TheGlobalData->m_useHeatEffects) {
				for (Particle *particle = system->getFirstParticle(); particle != nullptr; particle = particle->m_systemNext) {
					const Coord3D *position = particle->getPosition();
					if (position != nullptr && Passes_Terrain_Bounds(position->x, position->y, position->z, std::max(0.0f, particle->getSize()))) {
						if (Smudge *smudge = TheSmudgeManager->findSmudge(particle))
							smudge->m_draw = true;
					}
				}
			}
			continue;
		}
		if (system->isUsingStreak()) {
			GraphicsStreakBinding *streak = Ensure_Graphics_Streak(*system);
			if (streak == nullptr)
				continue;
			streak->sync_stamp = m_graphicsSyncStamp;
			Update_Graphics_Streak(*system, *streak);
			continue;
		}
		if (!Is_Graphics_Particle_System(*system))
			continue;

		const Graphics::ParticleEmitterHandle emitter_handle = Ensure_Graphics_Emitter(*system);
		if (!emitter_handle.Is_Valid())
			continue;

		GraphicsEmitterBinding *binding = nullptr;
		for (GraphicsEmitterBinding &candidate : m_graphicsEmitters) {
			if (candidate.legacy_system == system) {
				binding = &candidate;
				break;
			}
		}
		if (binding == nullptr)
			continue;
		binding->sync_stamp = m_graphicsSyncStamp;

		Coord3D system_position;
		system->getPosition(&system_position);
		const Coord3D *drift = system->getDriftVelocity();
		Graphics::ParticleEmitter emitter;
		emitter.position = {system_position.x, system_position.y, system_position.z};
		emitter.velocity = drift != nullptr ? Graphics::Vector3{drift->x, drift->y, drift->z} : Graphics::Vector3{};
		emitter.material = Ensure_Graphics_Material(system->getParticleTypeName().str());
		emitter.flags = Graphics_Particle_Flags(*system);
		emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
		emitter.max_particles = static_cast<std::uint32_t>(MAX_PARTICLES_PER_SYSTEM);
		if (!renderer.Update_Emitter(emitter_handle, emitter))
			continue;

		const std::size_t layer_count = system->isUsingVolumeParticles()
			? std::clamp<std::size_t>(system->getVolumeParticleDepth(), 1, 16)
			: 1;
		const float layer_scale = layer_count > 1 ? 0.1f / static_cast<float>(layer_count) : 0.0f;
		const bool billboard = Graphics::Has_Particle_Emitter_Flag(emitter.flags, Graphics::ParticleEmitterFlags::Billboard);
		std::size_t source_count = 0;
		std::size_t count = 0;
		{
			GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Prepare_Graphics_Particle_Data");
			for (Particle *particle = system->getFirstParticle(); particle != nullptr && source_count < MAX_PARTICLES_PER_SYSTEM; particle = particle->m_systemNext) {
			const Coord3D *position = particle->getPosition();
			const RGBColor *color = particle->getColor();
			if (position == nullptr || color == nullptr)
				continue;

			const float size = std::max(0.0f, particle->getSize());
			if (!Passes_Terrain_Bounds(position->x, position->y, position->z, size))
				continue;
			++source_count;
			const float to_camera_x = m_graphicsView.position.x - position->x;
			const float to_camera_y = m_graphicsView.position.y - position->y;
			const float to_camera_z = m_graphicsView.position.z - position->z;
			const float distance_squared = to_camera_x * to_camera_x + to_camera_y * to_camera_y + to_camera_z * to_camera_z;
			const float inverse_distance = distance_squared > 1.0e-12f ? 1.0f / std::sqrt(distance_squared) : 0.0f;
			for (std::size_t layer = 0; layer < layer_count && count < MAX_VOLUME_PARTICLES_PER_SYSTEM; ++layer) {
				const float shift = billboard ? static_cast<float>(layer) * size * layer_scale : 0.0f;
				m_graphicsPositionX[count] = position->x + to_camera_x * inverse_distance * shift;
				m_graphicsPositionY[count] = position->y + to_camera_y * inverse_distance * shift;
				m_graphicsPositionZ[count] = position->z + to_camera_z * inverse_distance * shift;
				m_graphicsVelocityX[count] = 0.0f;
				m_graphicsVelocityY[count] = 0.0f;
				m_graphicsVelocityZ[count] = 0.0f;
				m_graphicsLifetimes[count] = 1.0f;
				// Authored billboard size is its full width; ground effects use
				// that value as a half extent in the original point-group geometry.
				m_graphicsSizes[count] = billboard ? size * 0.5f : size;
				m_graphicsColorR[count] = color->red;
				m_graphicsColorG[count] = color->green;
				m_graphicsColorB[count] = color->blue;
				m_graphicsColorA[count] = particle->getAlpha();
				m_graphicsAngles[count] = particle->getAngle();
				m_graphicsParticleMaterials[count] = emitter.material;
				m_graphicsEmitterFlags[count] = emitter.flags;
				m_graphicsPipelines[count] = emitter.pipeline;
				++count;
			}
			}
		}
		m_fieldParticleCount += system->getPriority() == AREA_EFFECT && system->m_isGroundAligned != FALSE ? static_cast<Int>(source_count) : 0;

		const Graphics::ParticleData data{
			std::span<const float>(m_graphicsPositionX.data(), count),
			std::span<const float>(m_graphicsPositionY.data(), count),
			std::span<const float>(m_graphicsPositionZ.data(), count),
			std::span<const float>(m_graphicsVelocityX.data(), count),
			std::span<const float>(m_graphicsVelocityY.data(), count),
			std::span<const float>(m_graphicsVelocityZ.data(), count),
			std::span<const float>(m_graphicsLifetimes.data(), count),
			std::span<const float>(m_graphicsSizes.data(), count),
			std::span<const float>(m_graphicsColorR.data(), count),
			std::span<const float>(m_graphicsColorG.data(), count),
			std::span<const float>(m_graphicsColorB.data(), count),
			std::span<const float>(m_graphicsColorA.data(), count),
			std::span<const float>(m_graphicsAngles.data(), count),
			std::span<const Graphics::MaterialHandle>(m_graphicsParticleMaterials.data(), count),
			std::span<const Graphics::ParticleEmitterFlags>(m_graphicsEmitterFlags.data(), count),
			{},
			std::span<const Graphics::PipelineHandle>(m_graphicsPipelines.data(), count)
		};
		if (renderer.Append_Particles(emitter_handle, data))
			m_onScreenParticleCount += static_cast<Int>(source_count);
	}
	{
		GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Prepare_Graphics_Smudges");
		Prepare_Graphics_Smudges();
	}

	for (std::size_t index = 0; index < m_graphicsEmitters.size();) {
		GraphicsEmitterBinding &binding = m_graphicsEmitters[index];
		if (binding.sync_stamp == m_graphicsSyncStamp) {
			++index;
			continue;
		}
		renderer.Destroy_Emitter(binding.graphics_emitter);
		m_graphicsEmitters[index] = m_graphicsEmitters.back();
		m_graphicsEmitters.pop_back();
	}

	for (std::size_t index = 0; index < m_graphicsStreaks.size();) {
		GraphicsStreakBinding &binding = m_graphicsStreaks[index];
		if (binding.sync_stamp == m_graphicsSyncStamp) {
			++index;
			continue;
		}
		for (Graphics::BeamHandle beam : binding.beams)
			Graphics::GetBeamRenderer().Destroy(beam);
		if (binding.material.Is_Valid())
			Graphics::GetBeamRenderer().Destroy_Material(binding.material);
		if (binding.texture.Is_Valid())
			Graphics::GetBeamRenderer().Destroy_Texture(binding.texture);
		m_graphicsStreaks[index] = std::move(m_graphicsStreaks.back());
		m_graphicsStreaks.pop_back();
	}
}

bool W3DParticleSystemManager::Prepare_Weather_Snow()
{
	if (TheSnowManager == nullptr || !Graphics::GetParticleRenderer().Is_Initialized())
		return false;

	Graphics::ParticleRenderer &renderer = Graphics::GetParticleRenderer();
	W3DSnowManager *snow = static_cast<W3DSnowManager *>(TheSnowManager);
	const Graphics::MaterialHandle material = TheWeatherSetting != nullptr
		? Ensure_Graphics_Material(TheWeatherSetting->m_snowTexture.str())
		: renderer.Default_Material();
	const Graphics::WeatherParticleCullingBounds bounds{
		m_terrainBoundsValid,
		{m_terrainCenterX, m_terrainCenterY, m_terrainCenterZ},
		{m_terrainExtentX, m_terrainExtentY, m_terrainExtentZ}
	};
	const bool prepared = snow->Prepare_Weather_Particles(renderer, m_graphicsView, material, bounds);
	if (prepared)
		m_onScreenParticleCount += static_cast<Int>(snow->Weather_Particle_Count());
	return prepared;
}

void W3DParticleSystemManager::Prepare_Graphics_Smudges()
{
	m_graphicsSmudgeCount = 0;
	if (TheSmudgeManager == nullptr || TheGlobalData == nullptr || !TheGlobalData->m_useHeatEffects)
		return;
	m_graphicsSmudgeCount = static_cast<W3DSmudgeManager *>(TheSmudgeManager)->Collect_Graphics_Smudges(
		std::span<float>(m_graphicsSmudgePositionX),
		std::span<float>(m_graphicsSmudgePositionY),
		std::span<float>(m_graphicsSmudgePositionZ),
		std::span<float>(m_graphicsSmudgeOffsetX),
		std::span<float>(m_graphicsSmudgeOffsetY),
		std::span<float>(m_graphicsSmudgeSizes),
		std::span<float>(m_graphicsSmudgeOpacities));
}

bool W3DParticleSystemManager::Is_Graphics_Particle_System(const ParticleSystem &system) const noexcept
{
	return system.m_particleType == ParticleSystemInfo::PARTICLE || system.m_particleType == ParticleSystemInfo::VOLUME_PARTICLE;
}

Graphics::ParticleEmitterHandle W3DParticleSystemManager::Find_Graphics_Emitter(ParticleSystem *system) const noexcept
{
	GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Find_Graphics_Emitter");
	for (const GraphicsEmitterBinding &binding : m_graphicsEmitters)
		if (binding.legacy_system == system)
			return binding.graphics_emitter;
	return {};
}

Graphics::ParticleEmitterHandle W3DParticleSystemManager::Ensure_Graphics_Emitter(ParticleSystem &system)
{
	GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Ensure_Graphics_Emitter");
	const Graphics::ParticleEmitterHandle existing = Find_Graphics_Emitter(&system);
	if (existing.Is_Valid())
		return existing;

	Graphics::ParticleEmitter emitter;
	emitter.material = Ensure_Graphics_Material(system.getParticleTypeName().str());
	emitter.flags = Graphics_Particle_Flags(system);
	emitter.pipeline = Graphics::GetParticleRenderer().Pipeline_For_Flags(emitter.flags);
	emitter.max_particles = static_cast<std::uint32_t>(MAX_PARTICLES_PER_SYSTEM);
	const Graphics::ParticleEmitterHandle handle = Graphics::GetParticleRenderer().Create_Emitter(emitter);
	if (!handle.Is_Valid())
		return {};

	m_graphicsEmitters.push_back({&system, handle, 0});
	return handle;
}

Graphics::MaterialHandle W3DParticleSystemManager::Ensure_Graphics_Material(const char *texture_name)
{
	GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Ensure_Graphics_Material");
	if (texture_name == nullptr || *texture_name == '\0')
		return Graphics::GetParticleRenderer().Default_Material();

	for (const GraphicsMaterialBinding &binding : m_graphicsMaterials)
		if (binding.texture_name == texture_name)
			return binding.material;

	Graphics::ParticleRenderer &renderer = Graphics::GetParticleRenderer();
	const Graphics::MaterialHandle unavailable_material{};
	Graphics::Texture texture_description;
	std::vector<std::byte> pixels;
	if (!Build_Graphics_Particle_Texture(texture_name, texture_description, pixels)) {
		m_graphicsMaterials.push_back({texture_name, {}, unavailable_material});
		return unavailable_material;
	}

	const Graphics::TextureHandle texture = renderer.Create_Texture(texture_description, texture_description.pixel_data);
	if (!texture.Is_Valid()) {
		m_graphicsMaterials.push_back({texture_name, {}, unavailable_material});
		return unavailable_material;
	}

	Graphics::Material material;
	material.shader = renderer.Particle_Shader();
	material.textures[0] = texture;
	material.parameters.values[0] = 1.0f;
	material.parameters.values[1] = 1.0f;
	material.parameters.values[2] = 1.0f;
	material.parameters.values[3] = 1.0f;
	const Graphics::MaterialHandle material_handle = renderer.Create_Material(material);
	if (!material_handle.Is_Valid()) {
		renderer.Destroy_Texture(texture);
		m_graphicsMaterials.push_back({texture_name, {}, unavailable_material});
		return unavailable_material;
	}

	m_graphicsMaterials.push_back({texture_name, texture, material_handle});
	return material_handle;
}

Graphics::ParticleEmitterFlags W3DParticleSystemManager::Graphics_Particle_Flags(const ParticleSystem &system) const noexcept
{
	Graphics::ParticleEmitterFlags flags = Graphics::ParticleEmitterFlags::Enabled;
	if (system.m_isGroundAligned == FALSE)
		flags = flags | Graphics::ParticleEmitterFlags::Billboard;

	switch (system.m_shaderType) {
	case ParticleSystemInfo::ADDITIVE:
		flags = flags | Graphics::ParticleEmitterFlags::Additive;
		break;
	case ParticleSystemInfo::ALPHA_TEST:
		flags = flags | Graphics::ParticleEmitterFlags::AlphaTest;
		break;
	case ParticleSystemInfo::MULTIPLY:
		flags = flags | Graphics::ParticleEmitterFlags::Multiply;
		break;
	case ParticleSystemInfo::ALPHA:
	case ParticleSystemInfo::INVALID_SHADER:
	case ParticleSystemInfo::PARTICLE_SHADER_TYPE_COUNT:
		break;
	}
	return flags;
}

W3DParticleSystemManager::GraphicsStreakBinding *W3DParticleSystemManager::Find_Graphics_Streak(ParticleSystem *system) noexcept
{
	GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Find_Graphics_Streak");
	for (GraphicsStreakBinding &binding : m_graphicsStreaks)
		if (binding.legacy_system == system)
			return &binding;
	return nullptr;
}

W3DParticleSystemManager::GraphicsStreakBinding *W3DParticleSystemManager::Ensure_Graphics_Streak(ParticleSystem &system)
{
	GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Ensure_Graphics_Streak");
	if (GraphicsStreakBinding *existing = Find_Graphics_Streak(&system))
		return existing;

	GraphicsStreakBinding binding;
	binding.legacy_system = &system;
	binding.texture_name = system.getParticleTypeName().str();
	Graphics::BeamRenderer &renderer = Graphics::GetBeamRenderer();
	Graphics::Texture texture_description;
	std::vector<std::byte> pixels;
	if (Build_Graphics_Particle_Texture(binding.texture_name.c_str(), texture_description, pixels)) {
		binding.texture = renderer.Create_Texture(texture_description, texture_description.pixel_data);
		if (binding.texture.Is_Valid()) {
			Graphics::Material material;
			material.shader = renderer.Beam_Shader();
			material.textures[0] = binding.texture;
			binding.material = renderer.Create_Material(material);
			if (!binding.material.Is_Valid()) {
				renderer.Destroy_Texture(binding.texture);
				binding.texture = {};
			}
		}
	}
	if (!binding.material.Is_Valid())
		binding.material = renderer.Default_Material();
	binding.beams.reserve(MAX_PARTICLES_PER_SYSTEM - 1);
	m_graphicsStreaks.push_back(std::move(binding));
	return &m_graphicsStreaks.back();
}

Graphics::BeamFlags W3DParticleSystemManager::Graphics_Streak_Flags(const ParticleSystem &system) const noexcept
{
	Graphics::BeamFlags flags = Graphics::BeamFlags::Enabled;
	switch (system.m_shaderType) {
	case ParticleSystemInfo::ADDITIVE:
		flags = flags | Graphics::BeamFlags::Additive;
		break;
	case ParticleSystemInfo::ALPHA_TEST:
		flags = flags | Graphics::BeamFlags::AlphaTest;
		break;
	case ParticleSystemInfo::MULTIPLY:
		flags = flags | Graphics::BeamFlags::Multiply;
		break;
	case ParticleSystemInfo::ALPHA:
	case ParticleSystemInfo::INVALID_SHADER:
	case ParticleSystemInfo::PARTICLE_SHADER_TYPE_COUNT:
		break;
	}
	return flags;
}

void W3DParticleSystemManager::Update_Graphics_Streak(ParticleSystem &system, GraphicsStreakBinding &binding) noexcept
{
	GENERALS_GRAPHICS_PROFILE_SCOPE("W3DParticleSystemManager::Update_Graphics_Streak");
	Graphics::BeamRenderer &renderer = Graphics::GetBeamRenderer();
	std::array<Particle *, MAX_PARTICLES_PER_SYSTEM> points{};
	std::size_t point_count = 0;
	for (Particle *particle = system.getFirstParticle(); particle != nullptr && point_count < MAX_PARTICLES_PER_SYSTEM; particle = particle->m_systemNext) {
		const Coord3D *position = particle->getPosition();
		if (position != nullptr && Passes_Terrain_Bounds(position->x, position->y, position->z, std::max(0.0f, particle->getSize())))
			points[point_count++] = particle;
	}
	const std::size_t segment_count = point_count > 1 ? point_count - 1 : 0;
	while (binding.beams.size() < segment_count) {
		Graphics::BeamDescription description;
		description.material = binding.material;
		description.flags = Graphics_Streak_Flags(system);
		const Graphics::BeamHandle beam = renderer.Create(description);
		if (!beam.Is_Valid())
			return;
		binding.beams.push_back(beam);
	}
	while (binding.beams.size() > segment_count) {
		renderer.Destroy(binding.beams.back());
		binding.beams.pop_back();
	}

	const Graphics::BeamFlags flags = Graphics_Streak_Flags(system);
	for (std::size_t segment = 0; segment < segment_count; ++segment) {
		const Coord3D *start = points[segment]->getPosition();
		const Coord3D *end = points[segment + 1]->getPosition();
		const RGBColor *start_color = points[segment]->getColor();
		const RGBColor *end_color = points[segment + 1]->getColor();
		if (start == nullptr || end == nullptr || start_color == nullptr || end_color == nullptr)
			continue;
		Graphics::BeamDescription description;
		description.start = {start->x, start->y, start->z};
		description.end = {end->x, end->y, end->z};
		description.width = 2.0f * std::max(0.0f, points[segment + 1]->getSize());
		description.color = {end_color->red, end_color->green, end_color->blue, 1.0f};
		description.opacity = 1.0f;
		description.start_color = segment == 0
			? Graphics::Color4{}
			: Graphics::Color4{start_color->red, start_color->green, start_color->blue, points[segment]->getAlpha()};
		description.end_color = {end_color->red, end_color->green, end_color->blue, points[segment + 1]->getAlpha()};
		description.color_gradient = true;
		description.uv_offset = 0.0f;
		description.uv_scale = 0.0f;
		description.material = binding.material;
		description.flags = flags;
		renderer.Update(binding.beams[segment], description);
	}
	m_onScreenParticleCount += static_cast<Int>(point_count);
}

bool W3DParticleSystemManager::Passes_Terrain_Bounds(float x, float y, float z, float radius) const noexcept
{
	return !m_terrainBoundsValid
		|| std::fabs(x - m_terrainCenterX) <= m_terrainExtentX + radius
		&& std::fabs(y - m_terrainCenterY) <= m_terrainExtentY + radius
		&& std::fabs(z - m_terrainCenterZ) <= m_terrainExtentZ + radius;
}
