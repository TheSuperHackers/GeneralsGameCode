module;

#define BOOST_TEST_MODULE GraphicsAlignedAllocatorTests

#include <boost/test/included/unit_test.hpp>

#include <cstdint>

export module Graphics.Memory.AlignedAllocator.Tests;

import Graphics.Memory.AlignedAllocator;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(aligned_vector_has_contiguous_aligned_storage)
{
	AlignedVector<float> values;
	values.reserve(8);
	values.resize(8);

	BOOST_CHECK(values.data() != nullptr);
	BOOST_CHECK((reinterpret_cast<std::uintptr_t>(values.data()) % 16u) == 0u);
	BOOST_CHECK(values.data() + 7 == &values[7]);
}
