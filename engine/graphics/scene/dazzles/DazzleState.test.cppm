module;
#define BOOST_TEST_MODULE DazzleStateTests
#include <boost/test/included/unit_test.hpp>
#include <array>
export module Graphics.Scene.Dazzles.State.Tests;
import Assets.Dazzles;
import Graphics.Scene.Dazzles.State;
using namespace Graphics;
namespace {
constexpr std::array<float,16> Identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
Assets::DazzleDefinition Definition() {
    Assets::DazzleDefinition result;
    result.direction_area=0; result.intensity=.8f; result.halo_intensity=.6f;
    return result;
}
}
BOOST_AUTO_TEST_CASE(distance_fade_keeps_early_halo_return_and_camera_relative_exemption) {
    auto definition=Definition(); definition.fade_start=10; definition.fade_end=20;
    auto sample=Calculate_Dazzle_Intensity(definition,{0,0,-1},{0,0,1},{0,0,1},225);
    BOOST_CHECK_SMALL(sample.intensity-.4f,1e-6f); BOOST_CHECK_SMALL(sample.halo-.3f,1e-6f); BOOST_CHECK_EQUAL(sample.size,1);
    sample=Calculate_Dazzle_Intensity(definition,{0,0,-1},{0,0,1},{0,0,1},400);
    BOOST_CHECK_EQUAL(sample.intensity,0); BOOST_CHECK_EQUAL(sample.halo,0);
    sample=Calculate_Dazzle_Intensity(definition,{0,0,-1},{0,0,1},{0,0,1},401);
    BOOST_CHECK_EQUAL(sample.intensity,0); BOOST_CHECK_EQUAL(sample.halo,1);
    definition.use_camera_translation=false;
    sample=Calculate_Dazzle_Intensity(definition,{0,0,-1},{0,0,1},{0,0,1},401);
    BOOST_CHECK_SMALL(sample.intensity-.8f,1e-6f); BOOST_CHECK_SMALL(sample.halo-.6f,1e-6f);
}
BOOST_AUTO_TEST_CASE(angular_envelopes_keep_authored_direction_magnitude_and_independent_halo_power) {
    auto definition=Definition(); definition.area=1; definition.size_power=2;
    definition.intensity_power=3; definition.halo_intensity_power=2;
    auto sample=Calculate_Dazzle_Intensity(definition,{.8f,0,-.6f},{0,0,1},{0,0,1},1);
    BOOST_CHECK_SMALL(sample.intensity-.1728f,1e-6f); BOOST_CHECK_SMALL(sample.size-.36f,1e-6f); BOOST_CHECK_SMALL(sample.halo-.216f,1e-6f);
    definition.direction_area=.5f;
    sample=Calculate_Dazzle_Intensity(definition,{.8f,0,-.6f},{0,0,.4f},{0,0,1},1);
    BOOST_CHECK_EQUAL(sample.intensity,0); BOOST_CHECK_SMALL(sample.halo-.216f,1e-6f);
    sample=Calculate_Dazzle_Intensity(definition,{.8f,0,-.6f},{0,0,2},{0,0,1},1);
    BOOST_CHECK_SMALL(sample.intensity-.1728f,1e-6f);
}
BOOST_AUTO_TEST_CASE(visibility_fades_only_glare_and_preserves_blink_boundary_and_unsigned_clock_wrap) {
    auto definition=Definition(); definition.history_weight=.5f;
    DazzleState state; DazzleView view; view.view=view.projection=Identity;
    view.camera_position={0,0,1}; view.frame_milliseconds=2;
    unsigned queries=0;
    BOOST_CHECK(Prepare_Dazzle(state,definition,{0,0,.5f},view,[&] { ++queries; return .5f; }));
    BOOST_CHECK_SMALL(state.intensity-.4f,1e-6f); BOOST_CHECK_EQUAL(state.size,1); BOOST_CHECK_EQUAL(state.halo,.6f);
    BOOST_CHECK(Prepare_Dazzle(state,definition,{0,0,.5f},view,[&] { ++queries; return 0.f; }));
    BOOST_CHECK_SMALL(state.intensity-.1f,1e-6f); BOOST_CHECK_EQUAL(state.halo,.6f);
    BOOST_CHECK(Prepare_Dazzle(state,definition,{0,0,.5f},view,[&] { ++queries; return 0.f; }));
    BOOST_CHECK_EQUAL(state.intensity,0); BOOST_CHECK_EQUAL(queries,3);
    definition.blink_period=.1f; definition.blink_on_time=.032f;
    state.creation_time=0xfffffff0u; view.milliseconds=16;
    BOOST_CHECK(Prepare_Dazzle(state,definition,{0,0,.5f},view,[&] { ++queries; return 1.f; }));
    BOOST_CHECK_SMALL(state.intensity-.8f,1e-6f); BOOST_CHECK_EQUAL(queries,4);
    view.milliseconds=17;
    BOOST_CHECK(Prepare_Dazzle(state,definition,{9,9,9},view,[&] { ++queries; return 0.f; }));
    BOOST_CHECK_EQUAL(queries,4); BOOST_CHECK_EQUAL(state.visibility,0); BOOST_CHECK_EQUAL(state.screen_position[0],0);
    BOOST_CHECK_SMALL(state.intensity-.8f,1e-6f); BOOST_CHECK_EQUAL(state.halo,.6f);
}
BOOST_AUTO_TEST_CASE(projection_and_direction_use_full_affine_transforms_without_normalizing_the_authored_direction) {
    auto definition=Definition(); definition.direction={2,0,1}; definition.use_camera_translation=false;
    DazzleState state;
    const std::array<float,16> transform{0,-3,0,10,2,0,0,20,0,0,4,30,0,0,0,1};
    Set_Dazzle_Direction(state,definition,transform);
    BOOST_CHECK((state.direction==std::array<float,3>{0,4,4}));
    DazzleView view; view.view={2,0,0,3,0,4,0,5,0,0,1,1,0,0,0,1}; view.projection=Identity;
    view.projection[15]=2; view.camera_position={1,2,4};
    BOOST_CHECK(Prepare_Dazzle(state,definition,{1,2,3},view,[] { return 1.f; }));
    BOOST_CHECK((state.screen_position==std::array<float,3>{2.5f,6.5f,2}));
}
