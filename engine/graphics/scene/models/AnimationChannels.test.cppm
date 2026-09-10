module;
#define BOOST_TEST_MODULE AnimationChannelTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <span>
#include <vector>
export module Graphics.Scene.Models.AnimationChannels.Tests;
import Graphics.Scene.Models.AnimationChannels;
import Graphics.Scene.Models.AssetPose;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
import Assets.ModelRig;
import Assets.Animations;
import Assets.Adapters.W3D.Rig;
import Assets.Cache.Skeletons;
using namespace Graphics;
using namespace Assets;

namespace {
std::vector<std::byte> DrawingPoseAnimation() {
    using Bytes=std::vector<std::byte>;
    const auto u32=[](Bytes& bytes,std::uint32_t value) {
        for(unsigned i=0;i<4;++i)bytes.push_back(std::byte(value>>(i*8)));
    };
    const auto name=[](Bytes& bytes,const char* value) {
        for(unsigned i=0;i<16;++i) { bytes.push_back(std::byte(*value));if(*value)++value; }
    };
    const auto chunk=[&](Bytes& bytes,unsigned id,const Bytes& payload) {
        u32(bytes,id);u32(bytes,unsigned(payload.size()));bytes.insert(bytes.end(),payload.begin(),payload.end());
    };
    Bytes header,body;u32(header,1);name(header,"FACE");name(header,"RIG");
    u32(header,9);u32(header,std::bit_cast<unsigned>(30.f));u32(header,2);chunk(body,0x2c1,header);
    for(unsigned index=0;index<2;++index) {
        Bytes channel,reference,keys;name(reference,"RIG.MOVE");
        u32(keys,0);u32(keys,index?0:4);u32(keys,8);u32(keys,4);
        chunk(channel,0x2c3,reference);chunk(channel,0x2c4,keys);chunk(body,0x2c2,channel);
    }
    Bytes mapping;u32(mapping,0);u32(mapping,1);chunk(body,0x2c5,mapping);return body;
}
std::vector<std::byte> DrawingHierarchy(bool insert_root) {
    using Bytes=std::vector<std::byte>;
    const auto u32=[](Bytes& bytes,std::uint32_t value) {
        for(unsigned i=0;i<4;++i)bytes.push_back(std::byte(value>>(i*8)));
    };
    const auto name=[](Bytes& bytes,const char* value) {
        for(unsigned i=0;i<16;++i) {
            bytes.push_back(std::byte(*value));
            if(*value)++value;
        }
    };
    const auto chunk=[&](Bytes& bytes,unsigned id,const Bytes& payload) {
        u32(bytes,id);u32(bytes,unsigned(payload.size()));bytes.insert(bytes.end(),payload.begin(),payload.end());
    };
    Bytes header,pivots,body;u32(header,insert_root?0x20000:0x40001);name(header,"RIG");
    u32(header,insert_root?1:2);for(unsigned i=0;i<3;++i)u32(header,0);
    for(unsigned i=insert_root?1:0;i<2;++i) {
        name(pivots,i?"DRAW":"ROOT");u32(pivots,i&&!insert_root?0:0xffffffffu);
        u32(pivots,std::bit_cast<std::uint32_t>(i?.125f:0.f));
        for(unsigned j=0;j<8;++j)u32(pivots,0);u32(pivots,std::bit_cast<std::uint32_t>(1.f));
    }
    chunk(body,0x101,header);chunk(body,0x102,pivots);return body;
}
}

BOOST_AUTO_TEST_CASE(raw_defaults_and_fractional_frames_preserve_wrap_inputs) {
    ModelAnimationChannel channel{0,ModelChannelComponent::TranslationX,2,true,{{4,0,0,0},{8,0,0,0}}};
    BOOST_CHECK_EQUAL(Sample_Animation_Scalar(channel,1.5f),2);
    BOOST_CHECK_EQUAL(Sample_Animation_Scalar(channel,2.5f),6);
    BOOST_CHECK_EQUAL(Sample_Animation_Scalar(channel,3.5f),4);
    BOOST_CHECK_EQUAL(Sample_Animation_Scalar(channel,4),0);
    channel.hold_endpoints=true;
    BOOST_CHECK_EQUAL(Sample_Animation_Scalar(channel,-100),4);
    BOOST_CHECK_EQUAL(Sample_Animation_Scalar(channel,100),8);
    channel.component=ModelChannelComponent::Rotation;channel.hold_endpoints=false;
    BOOST_CHECK(Read_Animation_Frame(channel,-1)==(std::array<float,4>{0,0,0,1}));
    std::array<float,5> destination{9,9,9,9,9};
    Copy_Animation_Frame(channel,100,destination.data());
    BOOST_CHECK_EQUAL(destination[3],1);BOOST_CHECK_EQUAL(destination[4],9);
    BOOST_CHECK(Select_Animation_Interval(channel,std::numeric_limits<float>::infinity()).first==(std::array<float,4>{0,0,0,1}));
}
BOOST_AUTO_TEST_CASE(pose_keys_preserve_sparse_times_integer_poses_and_reverse_queries) {
    const std::array<ModelPoseKey,3> keys{{{2,7},{6,1},{10,16777217}}};
    for(float frame:{8.f,2.f,-1.f,4.f,10.f,6.f,100.f,3.f,0.f,1.f}) {
        const auto interval=Select_Pose_Interval(keys,frame);
        if(frame<0) { BOOST_CHECK_EQUAL(interval.first,7);BOOST_CHECK_EQUAL(interval.fraction,0); }
        else if(frame>=10) { BOOST_CHECK_EQUAL(interval.first,16777217u);BOOST_CHECK_EQUAL(interval.second,16777217u); }
        else if(frame<6) {
            BOOST_CHECK_EQUAL(interval.first,7);BOOST_CHECK_EQUAL(interval.second,1);
            BOOST_CHECK_EQUAL(interval.fraction,(frame-2)/4);
        } else {
            BOOST_CHECK_EQUAL(interval.first,1);BOOST_CHECK_EQUAL(interval.second,16777217u);
            BOOST_CHECK_EQUAL(interval.fraction,(frame-6)/4);
        }
    }
    BOOST_CHECK_EQUAL(Select_Pose_Interval({},2).first,0);
    BOOST_CHECK_EQUAL(Select_Pose_Interval(keys,std::numeric_limits<float>::quiet_NaN()).fraction,0);
    BOOST_CHECK_EQUAL(Select_Pose_Interval(std::span(keys).first(1),4).first,7);
}
BOOST_AUTO_TEST_CASE(timed_keys_preserve_steps_endpoints_and_order_independent_sampling) {
    ModelAnimationChannel channel{0,ModelChannelComponent::TranslationX,0,true,{{2,0,0,0},{6,0,0,0},{10,0,0,0}}};
    channel.key_frames={0,4,8};channel.step_into_key={0,1,0};channel.hold_endpoints=true;
    struct Case { float frame,value; };
    for(const auto& sample:std::array<Case,9>{{{7,9},{0,2},{3.9f,2},{4,6},{6,8},{8,10},{100,10},{2,2},{-1,2}}})
        BOOST_CHECK_EQUAL(Sample_Animation_Scalar(channel,sample.frame),sample.value);
    channel.component=ModelChannelComponent::Visibility;
    channel.samples={{1,0,0,0},{0,0,0,0},{1,0,0,0}};
    BOOST_CHECK(Sample_Animation_Visibility(channel,3.99f));
    BOOST_CHECK(!Sample_Animation_Visibility(channel,4));
    BOOST_CHECK(!Sample_Animation_Visibility(channel,7.99f));
    BOOST_CHECK(Sample_Animation_Visibility(channel,8));
    BOOST_CHECK(Sample_Animation_Visibility(channel,1));
}
BOOST_AUTO_TEST_CASE(sparse_rotations_interpolate_across_keys_and_keep_authored_steps) {
    ModelRigDesc rig;rig.skeleton_name="RIG";rig.bones={{"ROOT"},{"TIP",0,{1,0,0}}};
    ModelAnimationDesc clip;clip.name="TURN";clip.skeleton_name="RIG";clip.frame_count=5;clip.frame_rate=30;
    ModelAnimationChannel rotation{0,ModelChannelComponent::Rotation,0,true,{{0,0,0,1},{0,0,1,0}}};
    rotation.key_frames={0,4};rotation.hold_endpoints=true;clip.channels={rotation};rig.animations={clip};
    ModelAssetPose pose;std::string error;BOOST_REQUIRE(pose.Initialize(rig,error));
    BOOST_REQUIRE(pose.Evaluate(0,2));RenderTransform tip;BOOST_REQUIRE(pose.Bone_Transform(1,tip));
    BOOST_CHECK_SMALL(tip.matrix[3],.0001f);BOOST_CHECK_CLOSE(tip.matrix[7],1.f,.01f);
    rig.animations[0].channels[0].step_into_key={0,1};
    BOOST_REQUIRE(pose.Initialize(rig,error));BOOST_REQUIRE(pose.Evaluate(0,3.5f));
    BOOST_REQUIRE(pose.Bone_Transform(1,tip));BOOST_CHECK_CLOSE(tip.matrix[3],1.f,.01f);
    BOOST_REQUIRE(pose.Evaluate(0,4));BOOST_REQUIRE(pose.Bone_Transform(1,tip));
    BOOST_CHECK_CLOSE(tip.matrix[3],-1.f,.01f);
}
BOOST_AUTO_TEST_CASE(animated_pose_pixels_and_visibility_survive_reverse_sampling_and_target_resize) {
    for(bool insert_root:{false,true}) {
    ModelRigDesc rig;std::string error;
    BOOST_REQUIRE_MESSAGE(W3D::W3DRead_Hierarchy(DrawingHierarchy(insert_root),rig,error),error);
    SkeletonCache skeletons;
    const auto skeleton=skeletons.Publish(rig,error);
    BOOST_REQUIRE(skeleton.Is_Valid());
    BOOST_REQUIRE(skeletons.Resolve(skeleton));
    rig=*skeletons.Resolve(skeleton);
    ModelAnimationDesc clip;clip.name="MOVE";clip.skeleton_name="RIG";clip.frame_count=5;clip.frame_rate=30;
    ModelAnimationChannel translation{1,ModelChannelComponent::TranslationX,0,true,{{-.625f,0,0,0},{.375f,0,0,0}}};
    translation.key_frames={0,4};translation.hold_endpoints=true;
    ModelAnimationChannel visibility{1,ModelChannelComponent::Visibility,0,true,{{1,0,0,0},{0,0,0,0},{1,0,0,0}}};
    visibility.key_frames={0,3,4};visibility.hold_endpoints=true;
    clip.channels={translation,visibility};
    AnimationAsset prepared;
    BOOST_REQUIRE(prepared.Initialize(std::move(clip),static_cast<unsigned>(rig.bones.size()),error));
    BOOST_REQUIRE(prepared.Channel(1,ModelChannelComponent::Visibility));
    rig.animations={prepared.Description()};
    ModelAssetPose pose;BOOST_REQUIRE(pose.Initialize(rig,error));
    prepared={};rig.animations.clear();
    // A prepared instance owns its pose data across selective level unloading.
    skeletons.Clear();
    BOOST_CHECK(!skeletons.Resolve(skeleton));
    ModelPoseAnimationDesc pose_clip;
    BOOST_REQUIRE(W3D::W3DRead_Pose_Animation(DrawingPoseAnimation(),pose_clip,error));
    const auto& pose_keys=pose_clip.channels[pose_clip.bone_channels[1]].keys;
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
            for(unsigned frame:{4u,0u,2u,3u,0u}) {
                const auto interval=Select_Pose_Interval(pose_keys,float(frame)*2);
                const float sampled_frame=float(interval.first)+(float(interval.second)-float(interval.first))*interval.fraction;
                BOOST_REQUIRE(pose.Evaluate(0,sampled_frame));RenderTransform transform;
                BOOST_REQUIRE(pose.Bone_Transform(1,transform));
                BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
                if(pose.Visible(1)) {
                    std::array<PropVertex,3> vertices{};
                    vertices[0].position={-.15f,-.5f,.5f};vertices[1].position={.15f,-.5f,.5f};vertices[2].position={0,.5f,.5f};
                    for(auto& vertex:vertices) { vertex.position[0]+=transform.matrix[3];vertex.color={1,0,0,1}; }
                    const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,3>{0,1,2});
                    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));renderer.Destroy_Mesh(mesh);
                }
                std::vector<std::byte> pixels(width*16*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
                for(unsigned column:{width/4,width/2,width*3/4}) {
                    const auto expected_column=frame==0?width/4:frame==2?width/2:width*3/4;
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+column)*4]),frame!=3&&column==expected_column?255u:0u);
                }
            }
            device.Destroy_Texture(target);device.Destroy_Texture(depth);
        }
        renderer.Shutdown();
    }
    }
}
