module;
#define BOOST_TEST_MODULE TerrainVisibilityTests
#include <boost/test/included/unit_test.hpp>
#include <array>
export module Graphics.Scene.Terrain.Visibility.Tests;
import Graphics.Scene.Terrain.Visibility;
using namespace Graphics;
BOOST_AUTO_TEST_CASE(batch_bounds_are_conservative_at_clip_planes)
{
    std::array<float, 16> matrix{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    TerrainGeometryBatch batch{{-0.5f,-0.5f,0.25f}, {0.5f,0.5f,0.75f}, 0, 6};
    BOOST_CHECK(Is_Terrain_Batch_Visible(batch, matrix));
    matrix[3] = 1.5f;
    BOOST_CHECK(Is_Terrain_Batch_Visible(batch, matrix));
    matrix[3] = 1.6f;
    BOOST_CHECK(!Is_Terrain_Batch_Visible(batch, matrix));
    matrix[3] = 0;
    matrix[11] = -1;
    BOOST_CHECK(!Is_Terrain_Batch_Visible(batch, matrix));
    matrix[11] = 1;
    BOOST_CHECK(!Is_Terrain_Batch_Visible(batch, matrix));
}
