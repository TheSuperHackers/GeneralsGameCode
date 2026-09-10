module;

#define BOOST_TEST_MODULE GraphicsParticleDrawTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.Scene.ParticleDraw.Tests;

import Graphics.Scene.ParticleDraw;

using namespace Graphics;

namespace
{
Matrix4x4 Make_Perspective(float near_clip, float far_clip) noexcept
{
	Matrix4x4 projection{};
	projection.values[0] = 1.0f;
	projection.values[5] = 1.0f;
	projection.values[10] = -(far_clip + near_clip) / (far_clip - near_clip);
	projection.values[11] = -2.0f * far_clip * near_clip / (far_clip - near_clip);
	projection.values[14] = -1.0f;
	return projection;
}
}

BOOST_AUTO_TEST_CASE(particle_draws_resolve_material_indices_and_sort_back_to_front)
{
	MaterialPool materials;
	const MaterialHandle material = materials.Create();
	ParticleSystem particles;
	particles.Reserve(2, 2);

	ParticleEmitter near_emitter;
	near_emitter.position = {0.0f, 0.0f, -5.0f};
	near_emitter.particle_lifetime = 10.0f;
	near_emitter.material = material;
	const ParticleEmitterHandle near_handle = particles.Create_Emitter(near_emitter);

	ParticleEmitter far_emitter = near_emitter;
	far_emitter.position.z = -10.0f;
	const ParticleEmitterHandle far_handle = particles.Create_Emitter(far_emitter);
	BOOST_REQUIRE(particles.Spawn(near_handle, 1));
	BOOST_REQUIRE(particles.Spawn(far_handle, 1));

	const View view{Matrix4x4::Identity(), Make_Perspective(1.0f, 100.0f), {}, {0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f}};
	std::array<std::uint32_t, 2> visible_storage{};
	VisibleParticleSet visible_particles(visible_storage);
	BOOST_REQUIRE(Build_Visible_Particles(particles, view, visible_particles));

	MeshPool meshes;
	TexturePool textures;
	SamplerPool samplers;
	GPUScene gpu_scene;
	RenderScene scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));

	std::array<ParticleDrawData, 2> draw_storage{};
	ParticleDrawSet draw_set(draw_storage);
	const DrawPass pass{2, PipelineHandle(5, 1), 0};
	BOOST_REQUIRE(Build_Particle_Draw_Data(visible_particles, particles, view, gpu_scene, pass, draw_set));
	BOOST_REQUIRE(draw_set.Size() == 2);
	BOOST_CHECK(draw_set.Records()[0].particle_index == 1);
	BOOST_CHECK(draw_set.Records()[1].particle_index == 0);
	BOOST_CHECK(draw_set.Records()[0].material_index == gpu_scene.Material_Index(material));
	BOOST_CHECK(draw_set.Records()[0].pipeline == pass.pipeline);

	std::array<ParticleDrawData, 2> repeated_storage{};
	ParticleDrawSet repeated_set(repeated_storage);
	BOOST_REQUIRE(Build_Particle_Draw_Data(visible_particles, particles, view, gpu_scene, pass, repeated_set));
	BOOST_CHECK(repeated_set.Records()[0].particle_index == draw_set.Records()[0].particle_index);
	BOOST_CHECK(repeated_set.Records()[1].particle_index == draw_set.Records()[1].particle_index);
	BOOST_CHECK(far_handle != near_handle);
}

BOOST_AUTO_TEST_CASE(particle_draw_generation_skips_expired_indices_and_rejects_missing_pipeline)
{
	MaterialPool materials;
	const MaterialHandle material = materials.Create();
	ParticleSystem particles;
	particles.Reserve(1, 1);
	ParticleEmitter emitter;
	emitter.position = {0.0f, 0.0f, -5.0f};
	emitter.particle_lifetime = 0.5f;
	emitter.material = material;
	const ParticleEmitterHandle handle = particles.Create_Emitter(emitter);
	BOOST_REQUIRE(particles.Spawn(handle, 1));

	const View view{Matrix4x4::Identity(), Make_Perspective(1.0f, 100.0f), {}, {0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f}};
	std::array<std::uint32_t, 1> visible_storage{};
	VisibleParticleSet visible_particles(visible_storage);
	BOOST_REQUIRE(Build_Visible_Particles(particles, view, visible_particles));
	BOOST_REQUIRE(particles.Update(0.5f));

	MeshPool meshes;
	TexturePool textures;
	SamplerPool samplers;
	GPUScene gpu_scene;
	RenderScene scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));
	std::array<ParticleDrawData, 1> draw_storage{};
	ParticleDrawSet draw_set(draw_storage);
	BOOST_REQUIRE(Build_Particle_Draw_Data(visible_particles, particles, view, gpu_scene, {0, PipelineHandle(3, 1), 0}, draw_set));
	BOOST_CHECK(draw_set.Size() == 0);
	BOOST_CHECK(!Build_Particle_Draw_Data(visible_particles, particles, view, gpu_scene, {0, {}, 0}, draw_set));
}
