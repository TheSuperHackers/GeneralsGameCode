module;
#define BOOST_TEST_MODULE AnimationAssetTests
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <utility>
export module Assets.Animations.Tests;
import Assets.Animations;
using namespace Assets;

BOOST_AUTO_TEST_CASE(prepared_channels_own_data_and_bind_by_bone_and_component) {
    ModelAnimationDesc source;source.name="MOVE";source.skeleton_name="RIG";
    source.frame_count=2;source.frame_rate=30;
    source.channels={{3,ModelChannelComponent::TranslationX,0,true,{{7,0,0,0},{9,0,0,0}}},
        {0,ModelChannelComponent::Rotation,0,true,{{0,0,0,1}}},
        {9,ModelChannelComponent::Visibility,0,false,{{0,0,0,0}}}};
    AnimationAsset asset;std::string error;
    BOOST_REQUIRE(asset.Initialize(source,4,error));source.channels.clear();
    BOOST_CHECK_EQUAL(asset.Bone_Count(),4);
    BOOST_REQUIRE(asset.Channel(3,ModelChannelComponent::TranslationX));
    BOOST_CHECK_EQUAL(asset.Channel(3,ModelChannelComponent::TranslationX)->samples[1][0],9);
    BOOST_CHECK(!asset.Channel(3,ModelChannelComponent::Rotation));
    BOOST_CHECK(!asset.Channel(9,ModelChannelComponent::Visibility));
    BOOST_CHECK_EQUAL(asset.Description().channels.size(),3);
    auto copied=asset;asset={};auto moved=std::move(copied);
    BOOST_CHECK_EQUAL(moved.Channel(3,ModelChannelComponent::TranslationX)->samples[0][0],7);
    BOOST_CHECK_EQUAL(moved.Channel(0,ModelChannelComponent::Rotation)->samples[0][3],1);
}
BOOST_AUTO_TEST_CASE(binding_storage_grows_and_failed_preparation_preserves_previous_clip) {
    ModelAnimationDesc source;source.name="LARGE";source.skeleton_name="RIG";
    source.frame_count=1;source.frame_rate=30;
    for(unsigned bone=0;bone<17001;++bone)
        source.channels.push_back({bone,ModelChannelComponent::TranslationZ,0,true,{{float(bone),0,0,0}}});
    AnimationAsset asset;std::string error;BOOST_REQUIRE(asset.Initialize(source,17001,error));
    for(unsigned bone=0;bone<17001;++bone) {
        BOOST_REQUIRE(asset.Channel(bone,ModelChannelComponent::TranslationZ));
        BOOST_CHECK_EQUAL(asset.Channel(bone,ModelChannelComponent::TranslationZ)->samples[0][0],float(bone));
    }
    source.channels.push_back(source.channels.front());
    BOOST_CHECK(!asset.Initialize(std::move(source),17001,error));
    BOOST_CHECK_EQUAL(asset.Bone_Count(),17001);
    BOOST_CHECK_EQUAL(asset.Channel(17000,ModelChannelComponent::TranslationZ)->samples[0][0],17000);
}
