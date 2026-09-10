module;

#define BOOST_TEST_MODULE GraphicsResourcePoolTests

#include <boost/test/included/unit_test.hpp>

export module Graphics.Resources.Pools.ResourcePool.Tests;

import Graphics.Resources.Handles.ResourceHandle;
import Graphics.Resources.Pools.ResourcePool;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(create_and_resolve)
{
	ResourcePool<int, MeshHandle> pool;
	const MeshHandle handle = pool.Create(42);

	BOOST_REQUIRE(handle.Is_Valid());
	BOOST_REQUIRE(pool.Resolve(handle) != nullptr);
	BOOST_CHECK(*pool.Resolve(handle) == 42);
}

BOOST_AUTO_TEST_CASE(destroy_invalidates_old_handle)
{
	ResourcePool<int, MeshHandle> pool;
	const MeshHandle handle = pool.Create(42);

	BOOST_REQUIRE(pool.Destroy(handle));
	BOOST_CHECK(pool.Resolve(handle) == nullptr);
	BOOST_CHECK(!pool.Destroy(handle));
}

BOOST_AUTO_TEST_CASE(slot_reuse_increments_generation)
{
	ResourcePool<int, MeshHandle> pool;
	const MeshHandle old_handle = pool.Create(42);
	const MeshHandle::Generation old_generation = old_handle.Get_Generation();

	BOOST_REQUIRE(pool.Destroy(old_handle));

	const MeshHandle new_handle = pool.Create(84);
	BOOST_CHECK(new_handle.Get_Index() == old_handle.Get_Index());
	BOOST_CHECK(new_handle.Get_Generation() == old_generation + 1);
}

BOOST_AUTO_TEST_CASE(stale_handles_cannot_access_new_resources)
{
	ResourcePool<int, MeshHandle> pool;
	const MeshHandle old_handle = pool.Create(42);

	BOOST_REQUIRE(pool.Destroy(old_handle));
	const MeshHandle new_handle = pool.Create(84);

	BOOST_CHECK(pool.Resolve(old_handle) == nullptr);
	BOOST_CHECK(!pool.Destroy(old_handle));
	BOOST_REQUIRE(pool.Resolve(new_handle) != nullptr);
	BOOST_CHECK(*pool.Resolve(new_handle) == 84);
}

BOOST_AUTO_TEST_CASE(multiple_resources_work_correctly)
{
	ResourcePool<int, MeshHandle> pool;
	const MeshHandle first = pool.Create(10);
	const MeshHandle second = pool.Create(20);
	const MeshHandle third = pool.Create(30);

	BOOST_REQUIRE(pool.Destroy(second));
	BOOST_CHECK(pool.Size() == 2);
	BOOST_REQUIRE(pool.Resolve(first) != nullptr);
	BOOST_REQUIRE(pool.Resolve(third) != nullptr);
	BOOST_CHECK(*pool.Resolve(first) == 10);
	BOOST_CHECK(*pool.Resolve(third) == 30);
}
