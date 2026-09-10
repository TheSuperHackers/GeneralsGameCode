module;

#define BOOST_TEST_MODULE GraphicsMeshTests

#include <boost/test/included/unit_test.hpp>

#include <type_traits>

export module Graphics.Resources.Meshes.Mesh.Tests;

import Graphics.Resources.Meshes.Mesh;

using namespace Graphics;

static_assert(std::is_nothrow_move_constructible_v<Mesh>);
static_assert(std::is_nothrow_move_assignable_v<Mesh>);

BOOST_AUTO_TEST_CASE(mesh_metadata_is_preserved)
{
	const Mesh mesh{
		128,
		384,
		24,
		MeshIndexFormat::UInt32
	};

	BOOST_CHECK(mesh.vertex_count == 128);
	BOOST_CHECK(mesh.index_count == 384);
	BOOST_CHECK(mesh.vertex_stride == 24);
	BOOST_CHECK(mesh.index_format == MeshIndexFormat::UInt32);
}

BOOST_AUTO_TEST_CASE(mesh_pool_uses_mesh_handles)
{
	MeshPool pool;
	const MeshHandle handle = pool.Create(Mesh{64, 96, 16, MeshIndexFormat::UInt16});

	BOOST_REQUIRE(pool.Resolve(handle) != nullptr);
	BOOST_CHECK(pool.Resolve(handle)->vertex_count == 64);
	BOOST_CHECK(pool.Resolve(handle)->index_count == 96);
}
