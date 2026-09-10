module;

#define BOOST_TEST_MODULE GraphicsResourceHandleTests

#include <boost/test/included/unit_test.hpp>

#include <type_traits>

export module Graphics.Resources.Handles.ResourceHandle.Tests;

import Graphics.Resources.Handles.ResourceHandle;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(default_handle_is_invalid)
{
	const MeshHandle handle;

	BOOST_CHECK(!handle.Is_Valid());
	BOOST_CHECK(handle == nullptr);
}

BOOST_AUTO_TEST_CASE(constructed_handle_is_valid)
{
	const MeshHandle handle(12, 34);

	BOOST_CHECK(handle.Is_Valid());
	BOOST_CHECK(handle != nullptr);
}

BOOST_AUTO_TEST_CASE(handle_values_are_preserved)
{
	const MeshHandle handle(12, 34);

	BOOST_CHECK(handle.Get_Index() == 12);
	BOOST_CHECK(handle.Get_Generation() == 34);
}

BOOST_AUTO_TEST_CASE(handle_equality_uses_index_and_generation)
{
	const MeshHandle handle(12, 34);

	BOOST_CHECK(handle == MeshHandle(12, 34));
	BOOST_CHECK(handle != MeshHandle(13, 34));
	BOOST_CHECK(handle != MeshHandle(12, 35));
}

BOOST_AUTO_TEST_CASE(handles_are_strongly_typed)
{
	BOOST_CHECK((!std::is_convertible_v<MeshHandle, TextureHandle>));
	BOOST_CHECK((!std::is_constructible_v<TextureHandle, MeshHandle>));
	BOOST_CHECK((!std::is_convertible_v<SamplerHandle, TextureHandle>));
	BOOST_CHECK((!std::is_constructible_v<TextureHandle, SamplerHandle>));
	BOOST_CHECK((!std::is_convertible_v<ShaderHandle, MaterialHandle>));
	BOOST_CHECK((!std::is_constructible_v<MaterialHandle, ShaderHandle>));
}
