module;
#define BOOST_TEST_MODULE ModelAssetPoseTests
#include <boost/test/included/unit_test.hpp>
#include <cmath>
#include <string>
export module Graphics.Scene.Models.AssetPose.Tests;
import Graphics.Scene.Models.AssetPose;
import Assets.ModelRig;
using namespace Graphics;
BOOST_AUTO_TEST_CASE(samples_rest_relative_rotation_without_shrinking_at_half_frames) {
    Assets::ModelRigDesc rig;rig.skeleton_name="RIG";
    rig.bones={{"ROOT"},{"HINGE",0,{2,3,4}},{"TIP",1,{1,0,0}}};
    Assets::ModelAnimationDesc animation;animation.name="OPEN";animation.skeleton_name="RIG";animation.frame_count=2;animation.frame_rate=30;
    animation.channels={{1,Assets::ModelChannelComponent::Rotation,0,true,{{0,0,0,1},{0,0,1,0}}},
                        {1,Assets::ModelChannelComponent::TranslationZ,0,true,{{0,0,0,0},{4,0,0,0}}}};
    rig.animations={animation};ModelAssetPose pose;std::string error;
    BOOST_REQUIRE(pose.Initialize(rig,error));BOOST_REQUIRE(pose.Evaluate(0,.5f));
    RenderTransform tip;BOOST_REQUIRE(pose.Bone_Transform(2,tip));
    BOOST_TEST(tip.matrix[3]==2.f,boost::test_tools::tolerance(.0001f));
    BOOST_TEST(tip.matrix[7]==4.f,boost::test_tools::tolerance(.0001f));
    BOOST_TEST(tip.matrix[11]==6.f,boost::test_tools::tolerance(.0001f));
    BOOST_TEST(std::sqrt(tip.matrix[0]*tip.matrix[0]+tip.matrix[4]*tip.matrix[4]+tip.matrix[8]*tip.matrix[8])==1.f,boost::test_tools::tolerance(.0001f));
    BOOST_REQUIRE(pose.Evaluate(0,-.5f,true));BOOST_REQUIRE(pose.Bone_Transform(2,tip));
    BOOST_TEST(tip.matrix[11]==6.f,boost::test_tools::tolerance(.0001f));
}
BOOST_AUTO_TEST_CASE(uses_identity_motion_and_visibility_defaults_outside_sparse_ranges) {
    Assets::ModelRigDesc rig;rig.skeleton_name="RIG";rig.bones={{"ROOT"},{"HINGE",0}};
    Assets::ModelAnimationDesc animation;animation.name="OPEN";animation.skeleton_name="RIG";animation.frame_count=5;animation.frame_rate=30;
    animation.channels={{1,Assets::ModelChannelComponent::TranslationX,2,true,{{5,0,0,0},{7,0,0,0}}},
                        {1,Assets::ModelChannelComponent::Visibility,2,true,{{0,0,0,0}}}};
    rig.animations={animation};ModelAssetPose pose;std::string error;
    BOOST_REQUIRE(pose.Initialize(rig,error));BOOST_REQUIRE(pose.Evaluate(0,0));
    RenderTransform matrix;BOOST_REQUIRE(pose.Bone_Transform(1,matrix));BOOST_TEST(matrix.matrix[3]==0.f);BOOST_TEST(pose.Visible(1));
    BOOST_REQUIRE(pose.Evaluate(0,1.5f));BOOST_REQUIRE(pose.Bone_Transform(1,matrix));BOOST_TEST(matrix.matrix[3]==2.5f);
    BOOST_REQUIRE(pose.Evaluate(0,2));BOOST_TEST(!pose.Visible(1));
    BOOST_REQUIRE(pose.Evaluate(0,8));BOOST_REQUIRE(pose.Bone_Transform(1,matrix));BOOST_TEST(matrix.matrix[3]==0.f);BOOST_TEST(pose.Visible(1));
    rig.animations[0].channels_available=false;BOOST_REQUIRE(pose.Initialize(rig,error));BOOST_TEST(!pose.Evaluate(0,2));
}
