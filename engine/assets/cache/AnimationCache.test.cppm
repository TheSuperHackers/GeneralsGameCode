module;
#define BOOST_TEST_MODULE AnimationCacheTests
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <vector>
export module Assets.Cache.Animations.Tests;
import Assets.Cache.Animations;
using namespace Assets;
namespace {
ModelAnimationDesc Clip(std::string name) {
    ModelAnimationDesc clip;clip.name=std::move(name);clip.skeleton_name="RIG";
    clip.frame_count=5;clip.frame_rate=30;
    clip.channels={{1,ModelChannelComponent::TranslationX,0,true,{{1,0,0,0},{3,0,0,0}}}};
    return clip;
}
ModelPoseAnimationDesc Pose(std::string name,std::string source) {
    ModelPoseAnimationDesc pose;pose.name=std::move(name);pose.skeleton_name="RIG";
    pose.frame_count=5;pose.frame_rate=30;
    pose.channels={{std::move(source),{{0,0},{4,1}}}};
    return pose;
}
}
BOOST_AUTO_TEST_CASE(retained_clips_survive_name_clear_and_stale_handles_never_revive) {
    AnimationCache cache;std::string error;
    const auto clip=cache.Publish(Clip("MOVE"),2,AnimationSampling::Consecutive,error);
    BOOST_REQUIRE(clip);const auto* data=cache.Resolve(clip);BOOST_REQUIRE(data);
    BOOST_CHECK(cache.Acquire("rig.move")==clip);
    cache.Register_Missing("rig.absent");BOOST_CHECK(cache.Is_Missing("RIG.ABSENT"));
    cache.Clear_Named();BOOST_TEST(cache.Size()==0u);BOOST_CHECK(cache.Is_Missing("rig.absent"));
    cache.Reset_Missing();BOOST_CHECK(!cache.Is_Missing("rig.absent"));
    BOOST_CHECK(!cache.Find("RIG.MOVE"));BOOST_CHECK(cache.Resolve(clip)==data);
    const auto replacement=cache.Publish(Clip("MOVE"),2,AnimationSampling::Keyed,error);
    BOOST_REQUIRE(replacement);BOOST_CHECK(replacement!=clip);
    BOOST_CHECK(cache.Release(clip));BOOST_CHECK(!cache.Resolve(clip));
    BOOST_CHECK(!cache.Retain(clip));BOOST_CHECK(!cache.Release(clip));
    cache.Clear();BOOST_CHECK(!cache.Resolve(replacement));
    const auto reused=cache.Publish(Clip("NEW"),2,AnimationSampling::Consecutive,error);
    BOOST_REQUIRE(reused);BOOST_CHECK(reused!=replacement);BOOST_CHECK(!cache.Resolve(clip));
}
BOOST_AUTO_TEST_CASE(pose_dependencies_survive_clear_until_the_last_instance_releases_them) {
    AnimationCache cache;std::string error;
    const auto source=cache.Publish(Clip("SOURCE"),2,AnimationSampling::Keyed,error);
    BOOST_REQUIRE(source);
    const auto pose=cache.Publish_Pose(Pose("POSE","rig.source"),2,{source},error);
    BOOST_REQUIRE_MESSAGE(pose,error);BOOST_TEST(cache.Reference_Count(source)==1u);
    const auto nested=cache.Publish_Pose(Pose("NESTED","RIG.POSE"),2,{pose},error);
    BOOST_REQUIRE(nested);BOOST_REQUIRE(cache.Retain(nested));
    cache.Clear();BOOST_REQUIRE(cache.Resolve(nested));BOOST_REQUIRE(cache.Resolve(pose));
    BOOST_REQUIRE(cache.Resolve(source));BOOST_TEST(cache.Resolve(pose)->pose.bone_channels.size()==2u);
    BOOST_CHECK(cache.Release(nested));BOOST_CHECK(!cache.Resolve(nested));
    BOOST_CHECK(!cache.Resolve(pose));BOOST_CHECK(!cache.Resolve(source));
}
BOOST_AUTO_TEST_CASE(rejected_publication_preserves_dependency_counts_and_cache_identity) {
    AnimationCache cache;std::string error;
    const auto source=cache.Publish(Clip("SOURCE"),2,AnimationSampling::Keyed,error);
    BOOST_REQUIRE(source);
    BOOST_CHECK(!cache.Publish(Clip("source"),2,AnimationSampling::Consecutive,error));
    BOOST_CHECK(!cache.Publish(Clip("BAD"),0,AnimationSampling::Keyed,error));
    BOOST_CHECK(!cache.Publish_Pose(Pose("BAD","RIG.SOURCE"),2,{},error));
    BOOST_CHECK(!cache.Publish_Pose(Pose("BAD","RIG.MISSING"),2,{source},error));
    auto bad=Pose("BAD","RIG.SOURCE");bad.bone_channels={1,0};
    BOOST_CHECK(!cache.Publish_Pose(bad,2,{source},error));
    BOOST_TEST(cache.Reference_Count(source)==0u);BOOST_TEST(cache.Size()==1u);
    const auto pose=cache.Publish_Pose(Pose("POSE","RIG.SOURCE"),2,{source},error);
    BOOST_REQUIRE(pose);
    BOOST_CHECK(!cache.Publish_Pose(Pose("pose","RIG.SOURCE"),2,{source},error));
    BOOST_TEST(cache.Reference_Count(source)==1u);BOOST_TEST(cache.Size()==2u);
    cache.Remove_Unused_If([](const AnimationClip& clip) { return clip.name!="RIG.POSE"; });
    BOOST_CHECK(cache.Resolve(source));BOOST_CHECK(cache.Resolve(pose));
    cache.Remove_Unused_If([](const AnimationClip&) { return true; });
    cache.Remove_Unused_If([](const AnimationClip&) { return true; });
    BOOST_TEST(cache.Size()==0u);BOOST_CHECK(!cache.Resolve(source));BOOST_CHECK(!cache.Resolve(pose));
}
BOOST_AUTO_TEST_CASE(growth_preserves_borrowed_data_and_unloading_preserves_external_users) {
    AnimationCache cache;std::string error;
    std::vector<AnimationAssetHandle> handles;
    const AnimationClip* first=nullptr;
    for(unsigned i=0;i<17001;++i) {
        const auto handle=cache.Publish(Clip(std::to_string(i)),2,AnimationSampling::Consecutive,error);
        BOOST_REQUIRE(handle);handles.push_back(handle);if(!i)first=cache.Resolve(handle);
    }
    BOOST_CHECK(cache.Resolve(handles.front())==first);
    BOOST_REQUIRE(cache.Retain(handles.front()));
    cache.Remove_Unused_If([](const AnimationClip&) { return true; });
    BOOST_TEST(cache.Size()==1u);BOOST_CHECK(cache.Resolve(handles.front())==first);
    for(std::size_t i=1;i<handles.size();++i)BOOST_CHECK(!cache.Resolve(handles[i]));
    BOOST_CHECK(cache.Release(handles.front()));
    cache.Remove_Unused_If([](const AnimationClip&) { return true; });BOOST_TEST(cache.Size()==0u);
}
