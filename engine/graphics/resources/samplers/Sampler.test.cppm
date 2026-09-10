module;

#define BOOST_TEST_MODULE GraphicsSamplerTests

#include <boost/test/included/unit_test.hpp>

#include <type_traits>

export module Graphics.Resources.Samplers.Sampler.Tests;

import Graphics.Resources.Samplers.Sampler;

using namespace Graphics;

static_assert(std::is_nothrow_move_constructible_v<Sampler>);
static_assert(std::is_nothrow_move_assignable_v<Sampler>);

BOOST_AUTO_TEST_CASE(sampler_modes_are_preserved)
{
	const Sampler sampler{
		SamplerFilter::Nearest,
		SamplerFilter::Linear,
		SamplerFilter::Nearest,
		SamplerAddressMode::ClampToEdge,
		SamplerAddressMode::MirroredRepeat,
		SamplerAddressMode::ClampToBorder
	};

	BOOST_CHECK(sampler.min_filter == SamplerFilter::Nearest);
	BOOST_CHECK(sampler.mag_filter == SamplerFilter::Linear);
	BOOST_CHECK(sampler.mip_filter == SamplerFilter::Nearest);
	BOOST_CHECK(sampler.address_u == SamplerAddressMode::ClampToEdge);
	BOOST_CHECK(sampler.address_v == SamplerAddressMode::MirroredRepeat);
	BOOST_CHECK(sampler.address_w == SamplerAddressMode::ClampToBorder);
}

BOOST_AUTO_TEST_CASE(sampler_pool_uses_sampler_handles)
{
	SamplerPool pool;
	const SamplerHandle handle = pool.Create(Sampler{
		SamplerFilter::Linear,
		SamplerFilter::Linear,
		SamplerFilter::Linear,
		SamplerAddressMode::Repeat,
		SamplerAddressMode::Repeat,
		SamplerAddressMode::Repeat
	});

	BOOST_REQUIRE(pool.Resolve(handle) != nullptr);
	BOOST_CHECK(pool.Resolve(handle)->min_filter == SamplerFilter::Linear);
	BOOST_CHECK(pool.Resolve(handle)->address_u == SamplerAddressMode::Repeat);
}
