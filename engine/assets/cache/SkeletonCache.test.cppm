module;
#define BOOST_TEST_MODULE SkeletonCacheTests
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <utility>
#include <vector>
export module Assets.Cache.Skeletons.Tests;
import Assets.Cache.Skeletons;

namespace {
Assets::ModelRigDesc Skeleton(std::string name) {
    Assets::ModelRigDesc result;
    result.skeleton_name=std::move(name);
    result.bones={{"ROOT"},{"TURRET",0,{2,3,4}}};
    return result;
}
}

BOOST_AUTO_TEST_CASE(publishes_immutable_skeletons_and_rejects_invalid_or_duplicate_identity) {
    Assets::SkeletonCache cache;std::string error;
    auto description=Skeleton("Tank");
    const auto handle=cache.Publish(description,error);
    BOOST_REQUIRE(handle.Is_Valid());
    description.bones[1].translation.x=99;
    const auto* skeleton=cache.Resolve(handle);
    BOOST_REQUIRE(skeleton);
    BOOST_TEST(skeleton->bones[1].translation.x==2.f);
    BOOST_CHECK(cache.Find("TANK")==handle);
    BOOST_CHECK(cache.Find("tank")==handle);
    BOOST_TEST(!cache.Publish(Skeleton("tank"),error).Is_Valid());
    auto invalid=Skeleton("Broken");invalid.bones[1].parent=1;
    BOOST_TEST(!cache.Publish(invalid,error).Is_Valid());
    BOOST_TEST(cache.Size()==1u);
    BOOST_CHECK(cache.Resolve(handle)==skeleton);
    BOOST_TEST(!cache.Find("Broken").Is_Valid());
}

BOOST_AUTO_TEST_CASE(selective_unloading_preserves_live_handles_and_invalidates_reused_slots) {
    Assets::SkeletonCache cache;std::string error;
    const auto first=cache.Publish(Skeleton("first"),error);
    const auto keep=cache.Publish(Skeleton("KEEP"),error);
    const auto last=cache.Publish(Skeleton("last"),error);
    const auto* retained=cache.Resolve(keep);
    BOOST_REQUIRE(retained);
    cache.Remove_If([](const Assets::ModelRigDesc& rig) { return rig.skeleton_name!="KEEP"; });
    BOOST_TEST(cache.Size()==1u);
    BOOST_CHECK(cache.Resolve(keep)==retained);
    BOOST_CHECK(!cache.Resolve(first));BOOST_CHECK(!cache.Resolve(last));
    BOOST_TEST(!cache.Find("first").Is_Valid());BOOST_TEST(!cache.Find("last").Is_Valid());
    const auto replacement=cache.Publish(Skeleton("last"),error);
    BOOST_REQUIRE(replacement.Is_Valid());
    BOOST_TEST(replacement.Get_Index()==last.Get_Index());
    BOOST_TEST(replacement.Get_Generation()!=last.Get_Generation());
    BOOST_CHECK(!cache.Resolve(last));
    cache.Clear();cache.Clear();
    BOOST_TEST(cache.Size()==0u);
    BOOST_CHECK(!cache.Resolve(keep));BOOST_CHECK(!cache.Resolve(replacement));
    BOOST_REQUIRE(cache.Publish(Skeleton("new level"),error).Is_Valid());
    BOOST_CHECK(!cache.Resolve(keep));BOOST_CHECK(!cache.Resolve(replacement));
}

BOOST_AUTO_TEST_CASE(grows_beyond_retired_tree_capacity_without_moving_published_assets) {
    Assets::SkeletonCache cache;std::string error;
    std::vector<Assets::SkeletonAssetHandle> handles;
    const Assets::ModelRigDesc* first=nullptr;
    for(unsigned i=0;i<17001;++i) {
        const auto handle=cache.Publish(Skeleton(std::to_string(i)),error);
        BOOST_REQUIRE(handle.Is_Valid());handles.push_back(handle);
        if(!i)first=cache.Resolve(handle);
    }
    BOOST_CHECK(cache.Resolve(handles.front())==first);
    BOOST_TEST(cache.Size()==handles.size());
    for(unsigned i=0;i<handles.size();++i) {
        const auto* skeleton=cache.Resolve(handles[i]);
        BOOST_REQUIRE(skeleton);BOOST_TEST(skeleton->skeleton_name==std::to_string(i));
    }
    cache.Clear();
    for(const auto handle:handles)BOOST_CHECK(!cache.Resolve(handle));
}
