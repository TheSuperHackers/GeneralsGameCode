module;

#define BOOST_TEST_MODULE GraphicsModelVisibilityTests

#include <boost/test/included/unit_test.hpp>

#include <cstdint>

export module Graphics.Scene.Models.ModelVisibility.Tests;

import Graphics.Scene.Models.ModelVisibility;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(model_visibility_defaults_to_all_parts)
{
	BOOST_CHECK(Is_Submesh_Visible(All_Submeshes_Visible, 0));
	BOOST_CHECK(Is_Submesh_Visible(All_Submeshes_Visible, Max_Model_Part_Count - 1));
	BOOST_CHECK(!Is_Submesh_Visible(All_Submeshes_Visible, Max_Model_Part_Count));
}

BOOST_AUTO_TEST_CASE(model_visibility_toggles_parts_without_affecting_other_parts)
{
	SubmeshVisibilityMask visibility = All_Submeshes_Visible;
	visibility = Set_Submesh_Visible(visibility, 3, false);
	BOOST_CHECK(!Is_Submesh_Visible(visibility, 3));
	BOOST_CHECK(Is_Submesh_Visible(visibility, 2));
	visibility = Set_Submesh_Visible(visibility, 3, true);
	BOOST_CHECK(Is_Submesh_Visible(visibility, 3));
}

BOOST_AUTO_TEST_CASE(model_visibility_rejects_out_of_range_parts_without_mutation)
{
	constexpr SubmeshVisibilityMask visibility = 0x12345678u;
	BOOST_CHECK(Set_Submesh_Visible(visibility, Max_Model_Part_Count, false) == visibility);
	BOOST_CHECK(!Is_Submesh_Visible(visibility, Max_Model_Part_Count));
}
