module;

#define BOOST_TEST_MODULE GraphicsParticleVisibilityTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.Scene.ParticleVisibility.Tests;

import Graphics.Scene.ParticleVisibility;

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

View Make_View() noexcept
{
	return {Matrix4x4::Identity(), Make_Perspective(1.0f, 100.0f), {}, {0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f}};
}
}

BOOST_AUTO_TEST_CASE(particle_visibility_culls_disabled_and_outside_particles_deterministically)
{
	ParticleSystem particles;
	particles.Reserve(3, 3);

	ParticleEmitter visible;
	visible.position = {0.0f, 0.0f, -5.0f};
	visible.particle_lifetime = 10.0f;
	const ParticleEmitterHandle visible_emitter = particles.Create_Emitter(visible);

	ParticleEmitter outside = visible;
	outside.position.x = 200.0f;
	const ParticleEmitterHandle outside_emitter = particles.Create_Emitter(outside);

	ParticleEmitter disabled = visible;
	disabled.flags = ParticleEmitterFlags::None;
	const ParticleEmitterHandle disabled_emitter = particles.Create_Emitter(disabled);

	BOOST_REQUIRE(particles.Spawn(visible_emitter, 1));
	BOOST_REQUIRE(particles.Spawn(outside_emitter, 1));
	BOOST_REQUIRE(particles.Spawn(disabled_emitter, 1));

	std::array<std::uint32_t, 3> storage{};
	VisibleParticleSet visible_particles(storage);
	const View view = Make_View();
	BOOST_REQUIRE(Build_Visible_Particles(particles, view, visible_particles));
	BOOST_REQUIRE(visible_particles.Size() == 1);
	BOOST_CHECK(visible_particles.Indices()[0] == 0);

	BOOST_REQUIRE(Build_Visible_Particles(particles, view, visible_particles));
	BOOST_CHECK(visible_particles.Size() == 1);
	BOOST_CHECK(visible_particles.Indices()[0] == 0);
}

BOOST_AUTO_TEST_CASE(particle_visibility_rejects_insufficient_output_storage)
{
	ParticleSystem particles;
	particles.Reserve(1, 1);
	ParticleEmitter emitter;
	emitter.position = {0.0f, 0.0f, -5.0f};
	emitter.particle_lifetime = 1.0f;
	const ParticleEmitterHandle handle = particles.Create_Emitter(emitter);
	BOOST_REQUIRE(particles.Spawn(handle, 1));

	std::array<std::uint32_t, 0> storage{};
	VisibleParticleSet visible_particles(storage);
	BOOST_CHECK(!Build_Visible_Particles(particles, Make_View(), visible_particles));
	BOOST_CHECK(visible_particles.Size() == 0);
}

BOOST_AUTO_TEST_CASE(rotated_particle_corners_remain_visible_at_the_camera_edge)
{
    ParticleSystem particles;
    particles.Reserve(1,1);
    ParticleEmitter emitter;
    emitter.position = {1.6f,0,0.5f};
    emitter.particle_size = 0.5f;
    emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard;
    const auto handle = particles.Create_Emitter(emitter);
    BOOST_REQUIRE(particles.Spawn(handle,1));
    std::array<std::uint32_t,1> storage{};
    VisibleParticleSet visible(storage);
    const View view{Matrix4x4::Identity(),Matrix4x4::Identity(),{}, {0,0,64,64,0,1}};
    BOOST_REQUIRE(Build_Visible_Particles(particles,view,visible));
    BOOST_CHECK_EQUAL(visible.Size(),1);
    emitter.position.x = 1.8f;
    particles.Clear_Particles();
    BOOST_REQUIRE(particles.Update_Emitter(handle,emitter));
    BOOST_REQUIRE(particles.Spawn(handle,1));
    BOOST_REQUIRE(Build_Visible_Particles(particles,view,visible));
    BOOST_CHECK_EQUAL(visible.Size(),0);
}
