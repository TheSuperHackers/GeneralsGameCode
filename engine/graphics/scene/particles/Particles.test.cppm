module;

#define BOOST_TEST_MODULE GraphicsParticlesTests

#include <boost/test/included/unit_test.hpp>

#include <type_traits>
#include <array>
#include <span>

export module Graphics.Scene.Particles.Tests;

import Graphics.Scene.Particles;

using namespace Graphics;

static_assert(!std::is_convertible_v<ParticleEmitterHandle, MaterialHandle>);
static_assert(std::is_nothrow_move_constructible_v<ParticleEmitter>);
static_assert(std::is_nothrow_move_assignable_v<ParticleEmitter>);
static_assert(std::is_nothrow_move_constructible_v<GPUParticleData>);

BOOST_AUTO_TEST_CASE(texture_regions_survive_source_release_compaction_and_default_spawning)
{
	ParticleSystem source, particles;
	source.Reserve(1, 2);
	particles.Reserve(1, 2);
	const auto source_handle = source.Create_Emitter();
	const auto handle = particles.Create_Emitter();
	BOOST_REQUIRE(source.Spawn(source_handle, 2));
	auto data = source.Particles();
	std::array<ParticleTextureRegion, 2> regions{{{0, 0, .5f, .5f}, {.5f, .5f, 1, 1}}};
	const std::array<float, 2> lifetimes{.25f, 2};
	data.lifetimes = lifetimes;
	data.texture_regions = std::span(regions).first(1);
	BOOST_CHECK(!particles.Append_Particles(handle, data));
	BOOST_CHECK_EQUAL(particles.Particle_Count(), 0);
	data.texture_regions = regions;
	BOOST_REQUIRE(particles.Append_Particles(handle, data));
	regions = {};
	source.Clear();
	const ParticleTextureRegion second{.5f, .5f, 1, 1};
	BOOST_CHECK(particles.Particles().texture_regions[1] == second);
	BOOST_REQUIRE(particles.Update(.5f));
	BOOST_CHECK_EQUAL(particles.Particle_Count(), 1);
	BOOST_CHECK(particles.Particles().texture_regions[0] == second);
	BOOST_CHECK(Pack_GPU_Particle(particles.Particles(), 0, 0).texture_region == second);
	particles.Clear_Particles();
	BOOST_REQUIRE(particles.Spawn(handle, 1));
	const ParticleTextureRegion full{0, 0, 1, 1};
	BOOST_CHECK(particles.Particles().texture_regions[0] == full);
	auto default_data = particles.Particles();
	default_data.texture_regions = {};
	BOOST_CHECK(Pack_GPU_Particle(default_data, 0, 0).texture_region == full);
	ParticleSystem copied;
	copied.Reserve(1, 1);
	const auto copied_handle = copied.Create_Emitter();
	BOOST_REQUIRE(copied.Append_Particles(copied_handle, default_data));
	BOOST_CHECK(copied.Particles().texture_regions[0] == full);
}

BOOST_AUTO_TEST_CASE(particles_spawn_into_soa_storage_and_simulate)
{
	ParticleSystem particles;
	particles.Reserve(1, 4);

	ParticleEmitter emitter;
	emitter.position = {1.0f, 2.0f, 3.0f};
	emitter.velocity = {0.0f, 0.0f, 4.0f};
	emitter.particle_lifetime = 1.0f;
	emitter.particle_size = 2.0f;
	emitter.color = {0.25f, 0.5f, 0.75f, 1.0f};
	emitter.material = MaterialHandle(6, 1);
	emitter.max_particles = 4;
	const ParticleEmitterHandle handle = particles.Create_Emitter(emitter);

	BOOST_REQUIRE(particles.Spawn(handle, 2));
	BOOST_CHECK(particles.Particle_Count() == 2);
	BOOST_CHECK(particles.Particle_Count(handle) == 2);
	const ParticleData before_update = particles.Particles();
	BOOST_CHECK(before_update.position_z[0] == 3.0f);
	BOOST_CHECK(before_update.velocity_z[0] == 4.0f);
	BOOST_CHECK(before_update.lifetimes[0] == 1.0f);
	BOOST_CHECK(before_update.sizes[0] == 2.0f);
	BOOST_CHECK(before_update.color_g[0] == 0.5f);
	BOOST_CHECK(before_update.materials[0] == emitter.material);

	BOOST_REQUIRE(particles.Update(0.25f));
	const ParticleData after_update = particles.Particles();
	BOOST_CHECK(after_update.position_z[0] == 4.0f);
	BOOST_CHECK(after_update.lifetimes[0] == 0.75f);

	const GPUParticleData gpu_particle = Pack_GPU_Particle(after_update, 0, 11);
	BOOST_CHECK(gpu_particle.position_lifetime[2] == 4.0f);
	BOOST_CHECK(gpu_particle.position_lifetime[3] == 0.75f);
	BOOST_CHECK(gpu_particle.velocity_size[3] == 2.0f);
	BOOST_CHECK(gpu_particle.material_index == 11);
}

BOOST_AUTO_TEST_CASE(expired_particles_are_removed_and_emitters_reject_stale_handles)
{
	ParticleSystem particles;
	particles.Reserve(1, 2);
	ParticleEmitter emitter;
	emitter.particle_lifetime = 0.5f;
	emitter.max_particles = 2;
	const ParticleEmitterHandle old_handle = particles.Create_Emitter(emitter);
	BOOST_REQUIRE(particles.Spawn(old_handle, 2));
	BOOST_REQUIRE(particles.Update(0.5f));
	BOOST_CHECK(particles.Particle_Count() == 0);
	BOOST_CHECK(particles.Particle_Count(old_handle) == 0);

	BOOST_REQUIRE(particles.Spawn(old_handle, 1));
	BOOST_REQUIRE(particles.Destroy_Emitter(old_handle));
	BOOST_CHECK(!particles.Is_Emitter_Valid(old_handle));
	BOOST_CHECK(!particles.Spawn(old_handle, 1));

	const ParticleEmitterHandle new_handle = particles.Create_Emitter(emitter);
	BOOST_CHECK(new_handle.Get_Index() == old_handle.Get_Index());
	BOOST_CHECK(new_handle != old_handle);
	BOOST_CHECK(particles.Emitter_Count() == 1);
	BOOST_CHECK(particles.Particle_Count() == 0);
}

BOOST_AUTO_TEST_CASE(emitter_updates_propagate_render_state_to_live_particles)
{
	ParticleSystem particles;
	particles.Reserve(1, 1);
	const ParticleEmitterHandle handle = particles.Create_Emitter();
	BOOST_REQUIRE(particles.Spawn(handle, 1));

	ParticleEmitter updated;
	updated.material = MaterialHandle(12, 1);
	updated.flags = ParticleEmitterFlags::None;
	BOOST_REQUIRE(particles.Update_Emitter(handle, updated));

	const ParticleData data = particles.Particles();
	BOOST_CHECK(data.materials[0] == updated.material);
	BOOST_CHECK(data.emitter_flags[0] == ParticleEmitterFlags::None);
	BOOST_CHECK(!particles.Update(-1.0f));
}

BOOST_AUTO_TEST_CASE(external_soa_particle_updates_preserve_render_attributes)
{
	ParticleSystem particles;
	particles.Reserve(1, 2);
	ParticleEmitter emitter;
	emitter.max_particles = 2;
	const ParticleEmitterHandle handle = particles.Create_Emitter(emitter);

	const std::array<float, 1> position_x{1.0f};
	const std::array<float, 1> position_y{2.0f};
	const std::array<float, 1> position_z{3.0f};
	const std::array<float, 1> velocity_x{4.0f};
	const std::array<float, 1> velocity_y{5.0f};
	const std::array<float, 1> velocity_z{6.0f};
	const std::array<float, 1> lifetimes{7.0f};
	const std::array<float, 1> sizes{8.0f};
	const std::array<float, 1> color_r{0.1f};
	const std::array<float, 1> color_g{0.2f};
	const std::array<float, 1> color_b{0.3f};
	const std::array<float, 1> color_a{0.4f};
	const std::array<float, 1> angles{0.5f};
	const std::array<MaterialHandle, 1> materials{MaterialHandle(2, 1)};
	const std::array<ParticleEmitterFlags, 1> flags{ParticleEmitterFlags::Enabled};
	const ParticleData source{
		position_x, position_y, position_z,
		velocity_x, velocity_y, velocity_z,
		lifetimes, sizes,
		color_r, color_g, color_b, color_a, angles,
		materials, flags, {}
	};

	BOOST_REQUIRE(particles.Append_Particles(handle, source));
	const ParticleData data = particles.Particles();
	BOOST_CHECK(data.position_z[0] == 3.0f);
	BOOST_CHECK(data.velocity_x[0] == 4.0f);
	BOOST_CHECK(data.angles[0] == 0.5f);
	BOOST_CHECK(data.materials[0] == materials[0]);
	BOOST_CHECK(data.emitters[0] == handle);
}
