module;
#define BOOST_TEST_MODULE GeneralsPropSkinningTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <vector>
#include <span>
export module Graphics.Tests.PropSkinning;
import Graphics.Scene.Props.Skinning;
import Graphics.Scene.RenderScene;

BOOST_AUTO_TEST_CASE(weighted_pose_changes_geometry_and_preserves_surface_attributes)
{
    using namespace Graphics;
    PropVertex vertex; vertex.position={1,0,0};vertex.normal={0,0,1};vertex.tangent={1,0,0,1};vertex.uv={.2f,.7f};
    PropSkinInfluences skin{{0,1,0,0},{.25f,.75f,0,0}};
    RenderTransform first{{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};
    RenderTransform second=first;second.matrix[3]=4;
    std::array bones{first,second}; std::vector<PropVertex> output;
    BOOST_REQUIRE(Pose_Prop_Vertices(std::span(&vertex,1),std::span(&skin,1),bones,output));
    BOOST_TEST(output[0].position[0]==4);
    BOOST_TEST(output[0].uv[0]==.2f); BOOST_TEST(output[0].uv[1]==.7f);
    BOOST_TEST(output[0].normal[2]==1); BOOST_TEST(output[0].tangent[0]==1);
    skin.indices[1]=9;
    BOOST_TEST(!Pose_Prop_Vertices(std::span(&vertex,1),std::span(&skin,1),bones,output));
    BOOST_TEST(output[0].position[0]==4);
}
