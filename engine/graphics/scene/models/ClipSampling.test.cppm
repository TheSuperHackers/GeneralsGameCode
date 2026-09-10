module;
#define BOOST_TEST_MODULE ClipSamplingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
export module Graphics.Scene.Models.ClipSampling.Tests;
import Graphics.Scene.Models.ClipSampling;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;
using namespace Assets;
namespace {
ModelAnimationDesc MovingClip() {
    ModelAnimationDesc clip;clip.name="MOVE";clip.skeleton_name="RIG";
    clip.frame_count=3;clip.frame_rate=30;
    clip.channels={{1,ModelChannelComponent::TranslationX,0,true,{{-.5f,0,0,0},{0,0,0,0},{.5f,0,0,0}}},
        {1,ModelChannelComponent::Rotation,0,true,{{0,0,0,1},{0,0,1,0},{0,0,0,1}}},
        {1,ModelChannelComponent::Visibility,0,true,{{1,0,0,0},{0,0,0,0},{1,0,0,0}}}};
    return clip;
}
}
BOOST_AUTO_TEST_CASE(consecutive_clips_retain_fractional_bias_wrap_defaults_and_visibility) {
    AnimationCache cache;std::string error;
    const auto handle=cache.Publish(MovingClip(),2,AnimationSampling::Consecutive,error);
    BOOST_REQUIRE(handle);
    struct Case { float frame,x;bool visible; };
    for(const auto expected:std::array<Case,7>{{{0,-.5f,true},{.25f,-.375f,true},{.75f,-.125f,true},
        {1,0,false},{1.75f,.375f,false},{2,.5f,true},{2.75f,-.25f,true}}}) {
        BOOST_CHECK_EQUAL(Sample_Clip_Translation(cache,handle,1,expected.frame)[0],expected.x);
        BOOST_CHECK_EQUAL(Sample_Clip_Visibility(cache,handle,1,expected.frame),expected.visible);
        BOOST_CHECK_EQUAL(Sample_Clip_Transform(cache,handle,1,expected.frame).matrix[3],expected.x);
    }
    BOOST_CHECK_EQUAL(Sample_Clip_Rotation(cache,handle,0,1)[3],1);
    BOOST_CHECK_EQUAL(Sample_Clip_Translation(cache,handle,8,1)[0],0);
    BOOST_CHECK_EQUAL(Sample_Clip_Rotation(cache,handle,1,1)[2],1);
}
BOOST_AUTO_TEST_CASE(pose_clips_interpolate_referenced_poses_and_keep_their_visibility_contract) {
    AnimationCache cache;std::string error;
    auto description=MovingClip();
    for(auto& channel:description.channels)channel.hold_endpoints=true;
    const auto source=cache.Publish(description,2,AnimationSampling::Keyed,error);
    BOOST_REQUIRE(source);
    ModelPoseAnimationDesc pose;pose.name="POSE";pose.skeleton_name="RIG";
    pose.frame_count=5;pose.frame_rate=30;pose.channels={{"RIG.MOVE",{{0,0},{4,2}}}};
    const auto handle=cache.Publish_Pose(pose,2,{source},error);BOOST_REQUIRE(handle);
    BOOST_REQUIRE(cache.Retain(handle));cache.Clear();
    for(float frame:{4.f,0.f,2.f,1.f,4.f}) {
        const auto translation=Sample_Clip_Translation(cache,handle,1,frame);
        BOOST_CHECK_EQUAL(translation[0],-.5f+.25f*frame);
        BOOST_CHECK(Sample_Clip_Visibility(cache,handle,1,frame));
    }
    BOOST_CHECK(cache.Release(handle));BOOST_CHECK(!cache.Resolve(source));
}
BOOST_AUTO_TEST_CASE(retained_clip_pixels_survive_cache_clear_reverse_sampling_and_resize) {
    for(bool pose_mode:{false,true}) {
    AnimationCache cache;std::string error;
    auto description=MovingClip();
    const auto source=cache.Publish(description,2,AnimationSampling::Consecutive,error);
    BOOST_REQUIRE(source);
    auto handle=source;
    if(pose_mode) {
        ModelPoseAnimationDesc pose;pose.name="MAPPED";pose.skeleton_name="RIG";
        pose.frame_count=3;pose.frame_rate=30;
        pose.channels={{"RIG.MOVE",{{0,2},{2,2}}},{"RIG.MOVE",{{0,0},{2,2}}}};
        pose.bone_channels={0,1};
        handle=cache.Publish_Pose(pose,2,{source,source},error);
    }
    BOOST_REQUIRE(handle);BOOST_REQUIRE(cache.Retain(handle));cache.Clear();
    for(bool warp:{true,false}) {
        GraphicsTestDevice device({warp});if(!warp&&!device.Is_Valid())continue;
        PropRenderer renderer;BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        auto& commands=device.Immediate_Command_List();
        PropStyle style;style.depth_test=false;style.depth_write=false;
        PropParameters parameters;parameters.textured=0;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        for(unsigned width:{32u,64u,32u}) {
            const auto target=device.Create_Texture({width,16,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
            const auto depth=device.Create_Texture({width,16,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));BOOST_REQUIRE(commands.Set_Viewport({0,0,width,16}));
            for(float frame:{2.f,0.f,1.f,.75f,0.f}) {
                const auto transform=Sample_Clip_Transform(cache,handle,1,frame);
                BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
                if(Sample_Clip_Visibility(cache,handle,1,frame)) {
                    std::array<PropVertex,3> vertices{};
                    vertices[0].position={-.14f,-.7f,.5f};vertices[1].position={.14f,-.7f,.5f};vertices[2].position={0,.7f,.5f};
                    for(auto& vertex:vertices) { vertex.position[0]+=transform.matrix[3];vertex.color={1,0,0,1}; }
                    const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,3>{0,1,2});
                    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));renderer.Destroy_Mesh(mesh);
                }
                std::vector<std::byte> pixels(width*16*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
                const unsigned expected=frame==2?width*3/4:frame==1?width/2:frame==.75f?width*7/16:width/4;
                for(unsigned column:{width/4,width*7/16,width/2,width*3/4}) {
                    BOOST_TEST_CONTEXT("pose="<<pose_mode<<" width="<<width<<" frame="<<frame<<" column="<<column) {
                        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+column)*4]),(pose_mode||frame!=1)&&column==expected?255u:0u);
                    }
                }
            }
            device.Destroy_Texture(target);device.Destroy_Texture(depth);
        }
        renderer.Shutdown();
    }
    BOOST_CHECK(cache.Release(handle));BOOST_CHECK(!cache.Resolve(handle));
    BOOST_CHECK(!cache.Resolve(source));
    }
}
