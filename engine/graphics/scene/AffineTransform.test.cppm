module;
#define BOOST_TEST_MODULE AffineTransformTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <bit>
#include <cstdint>
#if GRAPHICS_COMPARE_GAME_MATH
#include <Utility/CppMacros.h>
#include "WWMath/matrix3d.h"
#include "WWMath/quat.h"
#endif
export module Graphics.Scene.AffineTransform.Tests;
import Graphics.Scene.AffineTransform;
import Graphics.Scene.Models.Hierarchy;
import Assets.ModelRig;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(row_conversion_preserves_all_affine_components) {
    using Rows=std::array<std::array<float,4>,3>;
    const Rows source{{{1,2,3,4},{5,6,7,8},{9,10,11,12}}};
    const auto transform=Import_Affine_Transform(source);
    BOOST_CHECK(Export_Affine_Transform<Rows>(transform)==source);
    BOOST_TEST(transform.matrix[12]==0.f);BOOST_TEST(transform.matrix[13]==0.f);
    BOOST_TEST(transform.matrix[14]==0.f);BOOST_TEST(transform.matrix[15]==1.f);
}

BOOST_AUTO_TEST_CASE(parent_rotation_and_local_translation_preserve_composition_order) {
    auto parent=Quaternion_Affine({0,0,1,0});parent.matrix[3]=4;
    auto child=Affine_Identity();child.matrix[3]=2;child.matrix[7]=3;
    const auto result=Multiply_Affine(parent,child);
    BOOST_TEST(result.matrix[3]==2.f);BOOST_TEST(result.matrix[7]==-3.f);
    Translate_Affine(parent,{2,3,0});
    BOOST_CHECK(parent.matrix==result.matrix);
    const auto authored=Quaternion_Affine({0,0,2,0});
    BOOST_TEST(authored.matrix[0]==-7.f);BOOST_TEST(authored.matrix[5]==-7.f);
}

#if GRAPHICS_COMPARE_GAME_MATH
BOOST_AUTO_TEST_CASE(prepared_hierarchy_and_controlled_queries_retain_game_matrix_results) {
    for(unsigned sample=0;sample<32;++sample) {
        Assets::ModelRigDesc rig;rig.skeleton_name="COMPARE";rig.bones={{"ROOT"}};
        for(unsigned bone=1;bone<5;++bone)rig.bones.push_back({std::to_string(bone),bone-1,
            {float(bone)*.12345f,-.23456f,float(sample)*.003f},{.12f,-.23f,.34f,.9f}});
        ModelHierarchy hierarchy(rig);hierarchy.Scale(1.25f);
        ::Matrix3D root(true);root.Translate({12.345f,-23.456f,34.567f});
        ::Matrix3D rotation;Build_Matrix3D(::Quaternion(.11f,.22f,-.33f,.91f),rotation);root.postMul(rotation);
        hierarchy.Capture(2);::Matrix3D control(true);control.Translate({.37f,-.48f,.59f});
        hierarchy.Control(2,Import_Affine_Transform(control),sample%2!=0);
        hierarchy.Evaluate(Import_Affine_Transform(root),[&](int bone) {
            BoneMotion motion;motion.translate=motion.rotate=true;
            motion.translation={float(bone)*.135f,-.246f,float(sample)*.017f};
            motion.orientation={-.14f,.25f,.36f,.87f};return motion;
        });
        std::array<::Matrix3D,5> world;world[0]=root;
        for(unsigned bone=1;bone<5;++bone) {
            ::Matrix3D rest(true);const auto& description=rig.bones[bone];
            rest.Translate({description.translation.x,description.translation.y,description.translation.z});
            Build_Matrix3D(::Quaternion(.12f,-.23f,.34f,.9f),rotation);rest.postMul(rotation);
            rest.Set_Translation(rest.Get_Translation()*1.25f);
            world[bone].mul(world[bone-1],rest);
            world[bone].Translate(::Vector3(float(bone)*.135f,-.246f,float(sample)*.017f)*1.25f);
            Build_Matrix3D(::Quaternion(-.14f,.25f,.36f,.87f),rotation);world[bone].postMul(rotation);
            if(bone==2) {
                if(sample%2)world[bone].Adjust_Translation(control.Get_Translation());
                else world[bone].postMul(control);
            }
            const auto& actual=hierarchy.World_Transform(bone);
            for(unsigned row=0;row<3;++row)for(unsigned column=0;column<4;++column)
                BOOST_TEST(std::bit_cast<std::uint32_t>(actual.matrix[row*4+column])==std::bit_cast<std::uint32_t>(world[bone][row][column]));
        }
    }
}

BOOST_AUTO_TEST_CASE(game_matrix_queries_retain_bit_exact_affine_results) {
    constexpr std::array<float,8> rounding_values{0.0f,std::bit_cast<float>(0x80000000u),
        std::bit_cast<float>(1u),std::bit_cast<float>(0x80000001u),1.0e-20f,-1.0e-20f,.125f,-.23456f};
    for(unsigned sample=0;sample<256;++sample) {
        Matrix3D left,right;
        for(unsigned row=0;row<3;++row)for(unsigned column=0;column<4;++column) {
            left[row][column]=float(int((sample*7+row*11+column*3)%37)-18)*.12345f;
            right[row][column]=float(int((sample*13+row*7+column*5)%41)-20)*.23456f;
            if(sample>=128) {
                left[row][column]=rounding_values[(sample*7+row*11+column*3)%rounding_values.size()];
                right[row][column]=rounding_values[(sample*13+row*7+column*5)%rounding_values.size()];
            }
        }
        Matrix3D expected;expected.mul(left,right);
        const auto actual=Multiply_Affine(Import_Affine_Transform(left),Import_Affine_Transform(right));
        auto assigned_left=Import_Affine_Transform(left);
        assigned_left=Multiply_Affine(assigned_left,Import_Affine_Transform(right));
        auto assigned_right=Import_Affine_Transform(right);
        assigned_right=Multiply_Affine(Import_Affine_Transform(left),assigned_right);
        for(unsigned row=0;row<3;++row)for(unsigned column=0;column<4;++column) {
            BOOST_TEST(std::bit_cast<std::uint32_t>(actual.matrix[row*4+column])==std::bit_cast<std::uint32_t>(expected[row][column]));
            BOOST_TEST(std::bit_cast<std::uint32_t>(assigned_left.matrix[row*4+column])==std::bit_cast<std::uint32_t>(expected[row][column]));
            BOOST_TEST(std::bit_cast<std::uint32_t>(assigned_right.matrix[row*4+column])==std::bit_cast<std::uint32_t>(expected[row][column]));
        }
        for(unsigned column=0;column<4;++column)
            BOOST_TEST(std::bit_cast<std::uint32_t>(actual.matrix[12+column])==(column==3 ? 0x3f800000u : 0u));
        const std::array<float,4> q{float(sample)*.003f,-.123f,.321f,.876f};
        Matrix3D rotation;Build_Matrix3D(Quaternion(q[0],q[1],q[2],q[3]),rotation);
        const auto converted=Quaternion_Affine(q);
        for(unsigned row=0;row<3;++row)for(unsigned column=0;column<4;++column)
            BOOST_TEST(std::bit_cast<std::uint32_t>(converted.matrix[row*4+column])==std::bit_cast<std::uint32_t>(rotation[row][column]));
        const ::Vector3 translation(.125f,-.234f,.987f);
        auto translated=Import_Affine_Transform(left);left.Translate(translation);
        Translate_Affine(translated,{translation.X,translation.Y,translation.Z});
        for(unsigned row=0;row<3;++row)
            BOOST_TEST(std::bit_cast<std::uint32_t>(translated.matrix[row*4+3])==std::bit_cast<std::uint32_t>(left[row][3]));
    }
}
#endif
