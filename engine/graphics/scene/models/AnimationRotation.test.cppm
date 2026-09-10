module;
#define BOOST_TEST_MODULE AnimationRotationTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <vector>
#if GRAPHICS_COMPARE_GAME_MATH
#include "AnimationRotation.test-reference.h"
#endif
export module Graphics.Scene.Models.AnimationRotation.Tests;
import Graphics.Scene.Models.AnimationRotation;
import Graphics.Scene.AffineTransform;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(interpolation_preserves_endpoints_magnitude_and_opposite_hemispheres) {
    const std::array<float,4> first{0,0,0,1},opposite{0,0,0,-1};
    for(float weight:{-1.f,0.f,.5f,1.f,2.f}) {
        const auto result=Interpolate_Animation_Rotation(first,opposite,weight);
        BOOST_CHECK_EQUAL(result[3],1);
        BOOST_CHECK_EQUAL(result[0],0);
    }
    const std::array<float,4> authored{0,0,0,2};
    BOOST_CHECK_EQUAL(Interpolate_Animation_Rotation(authored,authored,.5f)[3],2);
}
BOOST_AUTO_TEST_CASE(sampled_rotation_draws_at_expected_positions_after_resize_and_reverse_sampling) {
    for(bool warp:{true,false}) {
        GraphicsTestDevice device({warp});if(!warp&&!device.Is_Valid())continue;
        PropRenderer renderer;BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        auto& commands=device.Immediate_Command_List();
        PropStyle style;style.depth_test=false;style.depth_write=false;
        PropParameters parameters;parameters.textured=0;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        for(unsigned size:{32u,64u,32u}) {
            const auto target=device.Create_Texture({size,size,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
            const auto depth=device.Create_Texture({size,size,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));BOOST_REQUIRE(commands.Set_Viewport({0,0,size,size}));
            for(float weight:{1.f,0.f,.5f,0.f,1.f}) {
                const auto transform=Quaternion_Affine(Interpolate_Animation_Rotation({0,0,0,1},{0,0,1,0},weight));
                const std::array<std::array<float,2>,4> points{{{-.6f,-.1f},{-.4f,-.1f},{-.4f,.1f},{-.6f,.1f}}};
                std::array<PropVertex,4> vertices{};
                for(unsigned i=0;i<4;++i) {
                    const auto& m=transform.matrix;const auto& point=points[i];
                    vertices[i].position={m[0]*point[0]+m[1]*point[1],m[4]*point[0]+m[5]*point[1],.5f};
                    vertices[i].color={0,1,0,1};
                }
                BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
                const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,6>{0,1,2,0,2,3});
                BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));renderer.Destroy_Mesh(mesh);
                std::vector<std::byte> pixels(size*size*4);
                BOOST_REQUIRE(device.Readback_Texture(target,pixels,size*4));
                const std::array<std::array<unsigned,2>,3> probes{{{size/4,size/2},{size/2,size*3/4},{size*3/4,size/2}}};
                for(unsigned i=0;i<3;++i) {
                    const auto expected=weight==0?0u:weight==.5f?1u:2u;
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(probes[i][1]*size+probes[i][0])*4+1]),i==expected?255u:0u);
                }
            }
            device.Destroy_Texture(target);device.Destroy_Texture(depth);
        }
        renderer.Shutdown();
    }
}
#if GRAPHICS_COMPARE_GAME_MATH
BOOST_AUTO_TEST_CASE(rendering_rotation_retains_existing_sampler_bits) {
    for(unsigned sample=0;sample<512;++sample) {
        const float angle=float(sample)*.0061f;
        std::array<float,4> first{std::sin(angle),0,0,std::cos(angle)};
        std::array<float,4> second{0,std::sin(angle*.7f),0,std::cos(angle*.7f)};
        if(sample%3==0)for(auto& value:second)value=-value;
        for(float weight:{-.5f,0.f,.125f,.5f,.999f,1.f,1.25f}) {
            BOOST_TEST_CONTEXT("sample="<<sample<<", weight="<<weight) {
            std::array<float,4> expected;
            Reference_Animation_Rotation(first.data(),second.data(),weight,expected.data());
            const auto actual=Interpolate_Animation_Rotation(first,second,weight);
            for(unsigned i=0;i<4;++i)
                BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(actual[i]),std::bit_cast<std::uint32_t>(expected[i]));
            }
        }
    }
}
#endif
