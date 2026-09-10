module;
#define BOOST_TEST_MODULE ModelRigTests
#include <boost/test/included/unit_test.hpp>
#include <limits>
#include <string>
export module Assets.ModelRig.Tests;
import Assets.ModelRig;
using namespace Assets;
BOOST_AUTO_TEST_CASE(validates_parent_order_attachments_and_external_skeletons) {
    ModelRigDesc rig; std::string error;
    BOOST_REQUIRE(Validate_Model_Rig(rig,error));
    rig.skeleton_name="DOORS"; rig.bones={{"ROOT"},{"HINGE",0,{2,3,4}}};
    rig.attachments={{"DOORS.PANEL",1}};
    BOOST_REQUIRE(Validate_Model_Rig(rig,error));
    rig.bones[1].name.clear(); BOOST_REQUIRE(Validate_Model_Rig(rig,error));
    rig.bones[1].parent=1; BOOST_TEST(!Validate_Model_Rig(rig,error)); rig.bones[1].parent=0;
    rig.attachments[0].bone=2; BOOST_TEST(!Validate_Model_Rig(rig,error));
    rig.bones.clear(); BOOST_TEST(Validate_Model_Rig(rig,error)); // explicit external skeleton identity
    rig.skeleton_name.clear(); BOOST_TEST(!Validate_Model_Rig(rig,error));
}
BOOST_AUTO_TEST_CASE(rejects_bad_animation_ranges_duplicates_and_nonfinite_values) {
    ModelRigDesc rig; rig.skeleton_name="DOORS"; rig.bones={{"ROOT"},{"HINGE",0}};
    ModelAnimationDesc animation; animation.name="OPEN"; animation.skeleton_name="DOORS"; animation.frame_count=41; animation.frame_rate=30;
    animation.channels.push_back({1,ModelChannelComponent::Rotation,40,true,{{0,0,0,1}}});
    rig.animations.push_back(animation); std::string error;
    BOOST_REQUIRE(Validate_Model_Rig(rig,error));
    rig.animations[0].channels[0].first_frame=41; BOOST_TEST(!Validate_Model_Rig(rig,error));
    rig.animations[0]=animation; rig.animations[0].channels.push_back(animation.channels[0]);
    BOOST_TEST(!Validate_Model_Rig(rig,error));
    rig.animations[0]=animation; rig.animations[0].channels[0].samples[0]={0,0,0,0};
    BOOST_TEST(!Validate_Model_Rig(rig,error));
    rig.animations[0]=animation; rig.bones[0].translation.x=std::numeric_limits<float>::infinity();
    BOOST_TEST(!Validate_Model_Rig(rig,error));
}
BOOST_AUTO_TEST_CASE(validates_sparse_key_counts_order_steps_and_clip_bounds) {
    ModelRigDesc rig;rig.skeleton_name="RIG";rig.bones={{"ROOT"}};
    ModelAnimationDesc clip;clip.name="MOVE";clip.skeleton_name="RIG";clip.frame_count=5;clip.frame_rate=30;
    ModelAnimationChannel channel{0,ModelChannelComponent::TranslationX,0,true,{{1,0,0,0},{2,0,0,0}}};
    channel.key_frames={0,4};channel.step_into_key={0,1};clip.channels={channel};rig.animations={clip};
    std::string error;BOOST_REQUIRE(Validate_Model_Rig(rig,error));
    auto& value=rig.animations[0].channels[0];
    value.key_frames={4,0};BOOST_CHECK(!Validate_Model_Rig(rig,error));
    value.key_frames={0,0};BOOST_CHECK(!Validate_Model_Rig(rig,error));
    value.key_frames={0,5};BOOST_CHECK(!Validate_Model_Rig(rig,error));
    value.key_frames={0};BOOST_CHECK(!Validate_Model_Rig(rig,error));
    value.key_frames={0,4};value.step_into_key={1};BOOST_CHECK(!Validate_Model_Rig(rig,error));
    value.key_frames.clear();value.step_into_key={0,1};BOOST_CHECK(!Validate_Model_Rig(rig,error));
}

BOOST_AUTO_TEST_CASE(pose_clips_validate_references_keys_mapping_and_playback_metadata) {
    ModelPoseAnimationDesc clip;clip.name="FACE";clip.skeleton_name="RIG";
    clip.frame_count=9;clip.frame_rate=30;
    clip.channels={{"RIG.EXPRESSION",{{0,7},{8,1}}},{"RIG.MOUTH",{{0,3}}}};
    clip.bone_channels={0,1,0};std::string error;
    BOOST_REQUIRE(Validate_Pose_Animation(clip,error));
    clip.bone_channels[1]=2;BOOST_CHECK(!Validate_Pose_Animation(clip,error));
    clip.bone_channels.clear();BOOST_CHECK(Validate_Pose_Animation(clip,error));
    clip.channels[0].keys[1].frame=0;BOOST_CHECK(!Validate_Pose_Animation(clip,error));
    clip.channels[0].keys[1].frame=8;clip.channels[1].animation_name.clear();
    BOOST_CHECK(!Validate_Pose_Animation(clip,error));
    clip.channels[1].animation_name="RIG.MOUTH";clip.frame_rate=std::numeric_limits<float>::infinity();
    BOOST_CHECK(!Validate_Pose_Animation(clip,error));
    clip.frame_rate=30;clip.frame_count=0;BOOST_CHECK(!Validate_Pose_Animation(clip,error));
}
