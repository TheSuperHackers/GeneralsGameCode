module;
#define BOOST_TEST_MODULE ModelHierarchyTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
export module Graphics.Scene.Models.Hierarchy.Tests;
import Graphics.Scene.Models.Hierarchy;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
import Assets.ModelRig;
using namespace Graphics;

namespace {
Assets::ModelRigDesc Rig() {
    Assets::ModelRigDesc rig;rig.skeleton_name="MODEL";
    rig.bones={{"ROOT"},{"TURRET",0,{2,3,0},{0,0,1,0}},{"MUZZLE",1,{1,0,0}}};
    return rig;
}
BoneMotion MovingTurret(int bone) {
    BoneMotion result;result.set_visibility=true;result.visible=false;
    if(bone==1) { result.translate=true;result.translation={1,0,0}; }
    return result;
}
}

BOOST_AUTO_TEST_CASE(rest_motion_capture_and_world_translation_retain_child_composition) {
    auto rig=Rig();rig.bones[0].translation={99,98,97};
    ModelHierarchy hierarchy(rig);
    auto root=Affine_Identity();root.matrix[3]=10;
    hierarchy.Evaluate_Rest(root);
    BOOST_TEST(hierarchy.World_Transform(2).matrix[3]==11.f);
    BOOST_TEST(hierarchy.World_Transform(2).matrix[7]==3.f);
    BOOST_TEST(hierarchy.Bone_Index("turret")==1);BOOST_TEST(hierarchy.Bone_Index("missing")==0);
    BOOST_TEST(hierarchy.Parent_Index(2)==1);
    hierarchy.Evaluate(root,MovingTurret);
    BOOST_TEST(hierarchy.World_Transform(2).matrix[3]==10.f);
    BOOST_TEST(!hierarchy.Visible(1));BOOST_TEST(!hierarchy.Visible(2));
    hierarchy.Capture(1);auto control=Affine_Identity();control.matrix[7]=2;
    hierarchy.Control(1,control);hierarchy.Evaluate(root,MovingTurret);
    BOOST_TEST(hierarchy.World_Transform(2).matrix[7]==1.f);
    BOOST_TEST(hierarchy.Visible(1));BOOST_TEST(!hierarchy.Visible(2));
    hierarchy.Control(1,control,true);hierarchy.Evaluate(root,MovingTurret);
    BOOST_TEST(hierarchy.World_Transform(2).matrix[7]==5.f);
    auto clone=hierarchy;clone.Release(1);clone.Evaluate(root,MovingTurret);
    BOOST_TEST(clone.World_Transform(2).matrix[7]==3.f);
    BOOST_TEST(!clone.Visible(1));BOOST_TEST(hierarchy.Is_Captured(1));
    hierarchy.Release(1);hierarchy.Capture(1);hierarchy.Evaluate(root,MovingTurret);
    BOOST_TEST(hierarchy.World_Transform(2).matrix[7]==5.f);
    hierarchy.Capture(0);hierarchy.Control(0,control);hierarchy.Evaluate(root,MovingTurret);
    BOOST_CHECK(hierarchy.World_Transform(0).matrix==root.matrix);
}

BOOST_AUTO_TEST_CASE(queries_preserve_published_pose_and_scale_rest_and_motion_without_capture) {
    ModelHierarchy hierarchy(Rig());hierarchy.Scale(2);
    auto root=Affine_Identity();root.matrix[3]=10;
    hierarchy.Evaluate_Rest(root);
    BOOST_TEST(hierarchy.World_Transform(2).matrix[3]==12.f);
    hierarchy.Capture(1);auto control=Affine_Identity();control.matrix[7]=2;
    hierarchy.Control(1,control,true);hierarchy.Evaluate(root,MovingTurret);
    BOOST_TEST(hierarchy.World_Transform(2).matrix[3]==10.f);
    BOOST_TEST(hierarchy.World_Transform(2).matrix[7]==8.f);
    const auto published=hierarchy.World_Transform(2);
    RenderTransform query;
    BOOST_REQUIRE(hierarchy.Evaluate_Bone(2,root,[](int bone) {
        auto motion=Affine_Identity();if(bone==1)motion.matrix[3]=1;return motion;
    },query));
    BOOST_TEST(query.matrix[3]==10.f);BOOST_TEST(query.matrix[7]==6.f);
    BOOST_CHECK(hierarchy.World_Transform(2).matrix==published.matrix);
    BOOST_TEST(!hierarchy.Evaluate_Bone(-1,root,[](int) { return Affine_Identity(); },query));
    auto invalid=Rig();invalid.bones[2].parent=2;std::string error;
    BOOST_TEST(!hierarchy.Initialize(invalid,error));
    BOOST_CHECK(hierarchy.World_Transform(2).matrix==published.matrix);
}

BOOST_AUTO_TEST_CASE(flat_parent_indices_and_aligned_pose_storage_survive_large_hierarchy_copies) {
    auto rig=Rig();rig.bones.resize(2049);
    for(unsigned i=1;i<rig.bones.size();++i)rig.bones[i]={std::to_string(i),i-1,{.25f,0,0}};
    ModelHierarchy hierarchy(rig);rig.bones.clear();
    hierarchy.Evaluate_Rest(Affine_Identity());
    BOOST_TEST(hierarchy.World_Transform(2048).matrix[3]==512.f);
    hierarchy.Capture(1024);auto offset=Affine_Identity();offset.matrix[7]=7;
    hierarchy.Control(1024,offset,true);auto clone=hierarchy;
    hierarchy.Initialize_Default();clone.Evaluate_Rest(Affine_Identity());
    BOOST_TEST(clone.World_Transform(2048).matrix[7]==7.f);
    for(int i=0;i<clone.Bone_Count();++i)
        BOOST_TEST(reinterpret_cast<std::uintptr_t>(&clone.World_Transform(i))%16==0u);
}

BOOST_AUTO_TEST_CASE(captured_and_hidden_attachments_draw_after_source_release_and_resize) {
    Assets::ModelRigDesc rig;rig.skeleton_name="DRAW";rig.bones={{"ROOT"},{"DRAW",0,{.125f,0,0}}};
    ModelHierarchy hierarchy(rig);rig.bones.clear();
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
            struct Step { unsigned frame;bool capture; };
            for(const auto step:std::array<Step,6>{{{4,false},{0,false},{3,false},{3,true},{2,false},{0,false}}}) {
                hierarchy.Release(1);
                if(step.capture) {
                    hierarchy.Capture(1);auto offset=Affine_Identity();offset.matrix[3]=-.25f;hierarchy.Control(1,offset);
                }
                hierarchy.Evaluate(Affine_Identity(),[&](int) {
                    BoneMotion sample;sample.translate=sample.set_visibility=true;
                    sample.translation[0]=-.625f+float(step.frame)*.25f;sample.visible=step.frame!=3;return sample;
                });
                BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
                if(hierarchy.Visible(1)) {
                    std::array<PropVertex,3> vertices{};
                    vertices[0].position={-.15f,-.5f,.5f};vertices[1].position={.15f,-.5f,.5f};vertices[2].position={0,.5f,.5f};
                    for(auto& vertex:vertices) { vertex.position[0]+=hierarchy.World_Transform(1).matrix[3];vertex.color={1,0,0,1}; }
                    const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,3>{0,1,2});
                    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));renderer.Destroy_Mesh(mesh);
                }
                std::vector<std::byte> pixels(width*16*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
                const auto expected_column=step.frame==0?width/4:step.frame==4?width*3/4:width/2;
                for(unsigned column:{width/4,width/2,width*3/4})
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+column)*4]),
                        (step.frame!=3||step.capture)&&column==expected_column?255u:0u);
            }
            device.Destroy_Texture(target);device.Destroy_Texture(depth);
        }
        renderer.Shutdown();
    }
}
