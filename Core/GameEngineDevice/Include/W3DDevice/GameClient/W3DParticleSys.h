#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "GameClient/ParticleSys.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"

import Graphics.Scene.Particles.Renderer;
import Graphics.Scene.Beams;
import Graphics.FrameTargets;

class W3DParticleSystemManager : public ParticleSystemManager
{
public:
	W3DParticleSystemManager();
	virtual ~W3DParticleSystemManager() override;

	virtual void doParticles(W3DRenderContext &rinfo) override;
	virtual void queueParticleRender() override;
	virtual Int getOnScreenParticleCount() override { return m_onScreenParticleCount; }

	void Reset_Graphics_Particle_Bindings() noexcept;
	bool Set_Graphics_Particle_View(const Graphics::View &view) noexcept;
	bool Render_Graphics_Particles(Graphics::CommandList &commands, const Graphics::FrameTargets &targets) noexcept;

private:
	static constexpr std::size_t MAX_PARTICLES_PER_SYSTEM = 512;
	static constexpr std::size_t MAX_VOLUME_PARTICLES_PER_SYSTEM = MAX_PARTICLES_PER_SYSTEM * 16;
	static constexpr std::size_t MAX_GRAPHICS_SMUDGES = 512;

	struct GraphicsEmitterBinding final
	{
		ParticleSystem *legacy_system = nullptr;
		Graphics::ParticleEmitterHandle graphics_emitter{};
		std::uint32_t sync_stamp = 0;
	};

	struct GraphicsStreakBinding final
	{
		ParticleSystem *legacy_system = nullptr;
		std::string texture_name;
		Graphics::TextureHandle texture{};
		Graphics::MaterialHandle material{};
		std::vector<Graphics::BeamHandle> beams;
		std::uint32_t sync_stamp = 0;
	};

	struct GraphicsMaterialBinding final
	{
		std::string texture_name;
		Graphics::TextureHandle texture{};
		Graphics::MaterialHandle material{};
	};

	void Prepare_Graphics_Particles();
	bool Is_Graphics_Particle_System(const ParticleSystem &system) const noexcept;
	Graphics::ParticleEmitterHandle Find_Graphics_Emitter(ParticleSystem *system) const noexcept;
	Graphics::ParticleEmitterHandle Ensure_Graphics_Emitter(ParticleSystem &system);
	GraphicsStreakBinding *Find_Graphics_Streak(ParticleSystem *system) noexcept;
	GraphicsStreakBinding *Ensure_Graphics_Streak(ParticleSystem &system);
	Graphics::BeamFlags Graphics_Streak_Flags(const ParticleSystem &system) const noexcept;
	void Update_Graphics_Streak(ParticleSystem &system, GraphicsStreakBinding &binding) noexcept;
	Graphics::MaterialHandle Ensure_Graphics_Material(const char *texture_name);
	Graphics::ParticleEmitterFlags Graphics_Particle_Flags(const ParticleSystem &system) const noexcept;
	bool Passes_Terrain_Bounds(float x, float y, float z, float radius) const noexcept;
	bool Prepare_Weather_Snow();
	void Prepare_Graphics_Smudges();

	std::vector<GraphicsEmitterBinding> m_graphicsEmitters;
	std::vector<GraphicsStreakBinding> m_graphicsStreaks;
	std::vector<GraphicsMaterialBinding> m_graphicsMaterials;
	std::uint32_t m_graphicsSyncStamp = 0;
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsPositionX{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsPositionY{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsPositionZ{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsVelocityX{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsVelocityY{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsVelocityZ{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsLifetimes{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsSizes{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsColorR{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsColorG{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsColorB{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsColorA{};
	std::array<float, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsAngles{};
	std::array<Graphics::MaterialHandle, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsParticleMaterials{};
	std::array<Graphics::ParticleEmitterFlags, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsEmitterFlags{};
	std::array<Graphics::PipelineHandle, MAX_VOLUME_PARTICLES_PER_SYSTEM> m_graphicsPipelines{};
	std::array<float, MAX_GRAPHICS_SMUDGES> m_graphicsSmudgePositionX{};
	std::array<float, MAX_GRAPHICS_SMUDGES> m_graphicsSmudgePositionY{};
	std::array<float, MAX_GRAPHICS_SMUDGES> m_graphicsSmudgePositionZ{};
	std::array<float, MAX_GRAPHICS_SMUDGES> m_graphicsSmudgeOffsetX{};
	std::array<float, MAX_GRAPHICS_SMUDGES> m_graphicsSmudgeOffsetY{};
	std::array<float, MAX_GRAPHICS_SMUDGES> m_graphicsSmudgeSizes{};
	std::array<float, MAX_GRAPHICS_SMUDGES> m_graphicsSmudgeOpacities{};
	std::size_t m_graphicsSmudgeCount = 0;
	Graphics::View m_graphicsView{};
	bool m_graphicsViewValid = false;
	float m_terrainCenterX = 0.0f;
	float m_terrainCenterY = 0.0f;
	float m_terrainCenterZ = 0.0f;
	float m_terrainExtentX = 0.0f;
	float m_terrainExtentY = 0.0f;
	float m_terrainExtentZ = 0.0f;
	bool m_terrainBoundsValid = false;
	Int m_onScreenParticleCount = 0;
	Bool m_readyToRender = false;
	Bool m_graphicsParticlesPrepared = false;
	bool m_weatherParticlesReady = false;
};
