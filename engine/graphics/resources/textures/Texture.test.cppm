module;

#define BOOST_TEST_MODULE GraphicsTextureTests

#include <boost/test/included/unit_test.hpp>

#include <type_traits>

export module Graphics.Resources.Textures.Texture.Tests;

import Graphics.Resources.Textures.Texture;

using namespace Graphics;

static_assert(std::is_nothrow_move_constructible_v<Texture>);
static_assert(std::is_nothrow_move_assignable_v<Texture>);

BOOST_AUTO_TEST_CASE(texture_metadata_is_preserved)
{
	const TextureUsage usage = TextureUsage::Sampled | TextureUsage::RenderTarget;
	const Texture texture{
		1920,
		1080,
		1,
		12,
		TextureFormat::RGBA8_UNorm,
		usage
	};

	BOOST_CHECK(texture.width == 1920);
	BOOST_CHECK(texture.height == 1080);
	BOOST_CHECK(texture.depth == 1);
	BOOST_CHECK(texture.mip_count == 12);
	BOOST_CHECK(texture.format == TextureFormat::RGBA8_UNorm);
	BOOST_CHECK(Has_Texture_Usage(texture.usage, TextureUsage::Sampled));
	BOOST_CHECK(Has_Texture_Usage(texture.usage, TextureUsage::RenderTarget));
}

BOOST_AUTO_TEST_CASE(texture_pool_uses_texture_handles)
{
	TexturePool pool;
	const TextureHandle handle = pool.Create(Texture{256, 256, 1, 1, TextureFormat::RGBA8_UNorm, TextureUsage::Sampled});

	BOOST_REQUIRE(pool.Resolve(handle) != nullptr);
	BOOST_CHECK(pool.Resolve(handle)->width == 256);
	BOOST_CHECK(pool.Resolve(handle)->height == 256);
}
