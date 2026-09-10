module;
#define BOOST_TEST_MODULE ModelAssemblyDrawingTests
#include <boost/test/included/unit_test.hpp>
#include "../../../assets/adapters/w3d/model/W3DAssembly.test-data.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <span>
export module Graphics.Scene.Models.AssemblyDrawing.Tests;
import Assets.Adapters.W3D.Assembly;
import Assets.Adapters.W3D.Collection;
import Assets.Adapters.W3D.LevelSet;
import Assets.Adapters.W3D.Aggregate;
import Assets.Adapters.W3D.Retention;
import Assets.Adapters.W3D.Geometry;
import Assets.MeshBoundsTree;
import Graphics.Scene.Models.BoundsTree;
import Graphics.Scene.Models.VertexChannels;
import Graphics.Scene.Models.MaterialSlots;
import Graphics.Frame.SubmissionStatistics;
import Graphics.Scene.Models.Hierarchy;
import Graphics.Scene.Models.FactoryStore;
import Graphics.Scene.Models.Factory;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Assets;
using namespace Graphics;
BOOST_AUTO_TEST_CASE(material_slots_retain_selected_textures_across_shared_edits_and_resize)
{
    for(bool warp:{true,false}) {
        GraphicsTestDevice device({warp});if(!warp&&!device.Is_Valid())continue;
        PropRenderer renderer;BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        auto make_texture=[&](std::array<std::uint8_t,4> pixel) {
            const auto handle=device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(pixel)),4});
            BOOST_REQUIRE(handle.Is_Valid());
            return std::shared_ptr<RHITextureHandle>(new RHITextureHandle(handle),[&device](auto* value) {
                device.Destroy_Texture(*value);delete value;
            });
        };
        auto red=make_texture({255,0,0,255});
        auto green=make_texture({0,255,0,255});
        std::weak_ptr<RHITextureHandle> red_lifetime=red,green_lifetime=green;
        MaterialSlots<std::shared_ptr<RHITextureHandle>> original;
        original.Allocate(1);original.Set(0,red);
        auto alternate=original;
        auto clone=original.Clone();
        alternate.Set(0,green);
        BOOST_CHECK(original.Get(0)==green);
        BOOST_CHECK(clone.Get(0)==red);
        original.Reset();red.reset();green.reset();
        BOOST_CHECK(!red_lifetime.expired());BOOST_CHECK(!green_lifetime.expired());

        std::array<PropVertex,4> vertices{};
        vertices[0].position={-.8f,-.8f,.5f};vertices[1].position={.8f,-.8f,.5f};
        vertices[2].position={.8f,.8f,.5f};vertices[3].position={-.8f,.8f,.5f};
        for(auto& vertex:vertices) { vertex.uv={.5f,.5f};vertex.color={1,1,1,1}; }
        const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,6>{0,1,2,0,2,3});
        PropStyle style;style.depth_test=false;style.depth_write=false;
        PropParameters parameters;parameters.textured=1;parameters.primary_gradient=0;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        auto& commands=device.Immediate_Command_List();
        FrameSubmissionStatistics statistics;
        for(unsigned width:{32u,64u,32u}) {
            BOOST_REQUIRE(statistics.Begin(&commands,commands.Submission_Counts()));
            const auto target=device.Create_Texture({width,16,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
            const auto depth=device.Create_Texture({width,16,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));BOOST_REQUIRE(commands.Set_Viewport({0,0,width,16}));
            for(unsigned variant:{0u,1u,0u}) {
                auto selection=variant ? alternate : clone;
                const auto retained=selection.Get(0);selection.Reset();
                BOOST_REQUIRE(retained);
                BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
                BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{*retained}));
                std::vector<std::byte> pixels(width*16*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
                const auto offset=(8*width+width/2)*4;
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset]),variant?0u:255u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+1]),variant?255u:0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+2]),0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+3]),255u);
            }
            BOOST_REQUIRE(statistics.Complete(&commands,commands.Submission_Counts()));
            BOOST_CHECK_EQUAL(statistics.Last_Frame().draw_calls,3);
            BOOST_CHECK_EQUAL(statistics.Last_Frame().triangles,6);
            BOOST_CHECK_EQUAL(statistics.Last_Frame().vertex_invocations,18);
            device.Destroy_Texture(target);device.Destroy_Texture(depth);
        }
        renderer.Destroy_Mesh(mesh);renderer.Shutdown();
        alternate.Reset();clone.Reset();
        BOOST_CHECK(red_lifetime.expired());BOOST_CHECK(green_lifetime.expired());
    }
}

BOOST_AUTO_TEST_CASE(vertex_channels_grow_share_and_detach_without_aliasing_edits)
{
    using UV=std::array<float,2>;
    VertexChannels<UV> channels;
    for(unsigned i=0;i<16;++i) {
        const std::array<UV,1> values{{{float(i),.5f}}};
        BOOST_CHECK_EQUAL(channels.Install(values),i);
        BOOST_CHECK_EQUAL(channels.Count(),i+1);
    }
    VertexChannels<UV> alternate;
    BOOST_CHECK_EQUAL(alternate.Import(channels,15),0);
    BOOST_CHECK(alternate.Get(0)==channels.Get(15));
    auto copy=alternate;copy.Make_Unique(0);copy.Get(0)[0][0]=99;
    BOOST_CHECK_EQUAL(alternate.Get(0)[0][0],15);
    BOOST_CHECK_EQUAL(copy.Get(0)[0][0],99);
    channels.Clear();BOOST_CHECK_EQUAL(alternate.Get(0)[0][0],15);
    const std::array<UV,1> positive{{{0,.5f}}},negative{{{-0.f,.5f}}};
    const auto positive_index=alternate.Install(positive);
    const auto negative_index=alternate.Install(negative);
    BOOST_CHECK_NE(positive_index,negative_index);
    BOOST_CHECK_EQUAL(alternate.Install(positive),1);
    VertexChannels<UV> sparse;sparse.Create(8,1)[0]={1,2};
    BOOST_CHECK_EQUAL(sparse.Count(),0);BOOST_CHECK(!sparse.Empty());
    BOOST_CHECK(sparse.Get(7)==nullptr);sparse.Share(0,sparse,8);
    BOOST_CHECK_EQUAL(sparse.Count(),1);BOOST_CHECK(sparse.Get(0)==sparse.Get(8));
}

BOOST_AUTO_TEST_CASE(shared_uv_channels_draw_distinct_textures_after_edit_and_resize)
{
    using UV=std::array<float,2>;
    const std::array<UV,4> coordinates{{{.25f,.5f},{.25f,.5f},{.25f,.5f},{.25f,.5f}}};
    VertexChannels<UV> original;original.Install(coordinates);
    auto edited=original;edited.Make_Unique(0);
    for(unsigned i=0;i<4;++i)edited.Get(0)[i][0]=.75f;
    for(bool warp:{true,false}) {
        GraphicsTestDevice device({warp});if(!warp&&!device.Is_Valid())continue;
        PropRenderer renderer;BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const std::array<std::uint8_t,8> colors{255,0,0,255,0,255,0,255};
        const auto texture=device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(colors)),8});
        BOOST_REQUIRE(texture.Is_Valid());
        auto& commands=device.Immediate_Command_List();
        PropStyle style;style.depth_test=false;style.depth_write=false;style.samplers[0].Set_Filter(RHISamplerFilter::Point);
        PropParameters parameters;parameters.textured=1;parameters.primary_gradient=0;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        for(unsigned width:{32u,64u,32u}) {
            const auto target=device.Create_Texture({width,16,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
            const auto depth=device.Create_Texture({width,16,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));BOOST_REQUIRE(commands.Set_Viewport({0,0,width,16}));
            for(unsigned variant:{0u,1u,0u}) {
                auto source=variant ? edited : original;
                std::array<PropVertex,4> vertices{};
                vertices[0].position={-.8f,-.8f,.5f};vertices[1].position={.8f,-.8f,.5f};
                vertices[2].position={.8f,.8f,.5f};vertices[3].position={-.8f,.8f,.5f};
                for(unsigned i=0;i<4;++i) { vertices[i].uv=source.Get(0)[i];vertices[i].color={1,1,1,1}; }
                const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,6>{0,1,2,0,2,3});source.Clear();
                BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
                BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));renderer.Destroy_Mesh(mesh);
                std::vector<std::byte> pixels(width*16*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
                const auto offset=(8*width+width/2)*4;
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset]),variant?0u:255u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+1]),variant?255u:0u);
            }
            device.Destroy_Texture(target);device.Destroy_Texture(depth);
        }
        device.Destroy_Texture(texture);renderer.Shutdown();
    }
}
BOOST_AUTO_TEST_CASE(tree_cast_preserves_surface_selection_and_leaf_local_exits)
{
    MeshBoundsTree source;
    source.nodes={{{{-1,-1,0},{1,1,1}},1,2,false},
        {{{-1,-1,0},{0,1,1}},0,2,true},{{{0,-1,0},{1,1,1}},2,2,true}};
    source.polygon_indices={0,1,2,3};
    ModelBoundsTree tree(std::move(source));
    const auto cull=[](const Bounds3f&) { return false; };
    std::vector<unsigned> calls,surfaces;
    BOOST_CHECK(tree.Cast(cull,[&](unsigned polygon) { calls.push_back(polygon);return true; },
        [] { return false; },[&](unsigned polygon) { surfaces.push_back(polygon); }));
    const std::vector<unsigned> all{0,1,2,3},last_per_leaf{1,3};
    BOOST_CHECK_EQUAL_COLLECTIONS(calls.begin(),calls.end(),all.begin(),all.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(surfaces.begin(),surfaces.end(),last_per_leaf.begin(),last_per_leaf.end());
    calls.clear();surfaces.clear();bool start_bad=false;
    BOOST_CHECK(tree.Cast(cull,[&](unsigned polygon) { calls.push_back(polygon);start_bad=true;return true; },
        [&] { return start_bad; },[&](unsigned polygon) { surfaces.push_back(polygon); }));
    const std::vector<unsigned> first_per_leaf{0,2};
    BOOST_CHECK_EQUAL_COLLECTIONS(calls.begin(),calls.end(),first_per_leaf.begin(),first_per_leaf.end());
    BOOST_CHECK(surfaces.empty());
    calls.clear();
    BOOST_CHECK(tree.Intersects(cull,[&](unsigned polygon) { calls.push_back(polygon);return true; }));
    BOOST_CHECK_EQUAL_COLLECTIONS(calls.begin(),calls.end(),first_per_leaf.begin(),first_per_leaf.end());
    BOOST_CHECK(!tree.Cast([](const Bounds3f&) { return true; },[](unsigned) { BOOST_FAIL("Culled polygon");return true; },
        [] { BOOST_FAIL("Culled start-state check");return false; },[](unsigned) { BOOST_FAIL("Culled surface"); }));
}
BOOST_AUTO_TEST_CASE(runtime_tree_preserves_leaf_order_cull_timing_and_copy_independence)
{
    MeshBoundsTree source;
    source.nodes={{{{-3,-1,0},{3,1,1}},2,1,false},
        {{{1,-1,0},{3,1,1}},0,2,true},{{{-2,-1,0},{0,1,1}},2,2,true}};
    source.polygon_indices={9,7,5,3};
    ModelBoundsTree tree(source);source={};
    auto copy=tree;copy.Scale(2);
    std::vector<unsigned> visits;
    std::vector<float> bounds;
    tree.Visit([&](const Bounds3f& value) { bounds.push_back(value.minimum.x);return false; },
        [&](std::span<const std::uint32_t> polygons) {
            visits.insert(visits.end(),polygons.begin(),polygons.end());
            return true; // A leaf's result must not short-circuit the next leaf.
        });
    const std::vector<unsigned> expected{5,3,9,7};
    BOOST_CHECK_EQUAL_COLLECTIONS(visits.begin(),visits.end(),expected.begin(),expected.end());
    const std::vector<float> expected_bounds{-3,-2,1};
    BOOST_CHECK_EQUAL_COLLECTIONS(bounds.begin(),bounds.end(),expected_bounds.begin(),expected_bounds.end());
    unsigned leaves=0,culls=0;
    copy.Visit([&](const Bounds3f& value) {
        ++culls;
        if(culls==1)BOOST_CHECK_EQUAL(value.minimum.x,-6);
        return leaves!=0; // Query state changes in the front leaf before culling the back.
    },[&](std::span<const std::uint32_t> polygons) { ++leaves;BOOST_CHECK_EQUAL(polygons[0],5); });
    BOOST_CHECK_EQUAL(leaves,1);BOOST_CHECK_EQUAL(culls,3);
    copy.Scale(-1);
    copy.Visit([&](const Bounds3f& value) {
        BOOST_CHECK_EQUAL(value.minimum.x,6);BOOST_CHECK_EQUAL(value.maximum.x,-6);return true;
    },[](std::span<const std::uint32_t>) { BOOST_FAIL("Culled root visited a leaf"); });
    ModelBoundsTree empty;
    empty.Visit([](const Bounds3f&) { BOOST_FAIL("Empty tree culled a node");return false; },
        [](std::span<const std::uint32_t>) { BOOST_FAIL("Empty tree visited a leaf"); });
}
BOOST_AUTO_TEST_CASE(bounds_tree_polygon_order_draws_every_leaf_after_resize)
{
    std::vector<Vector3f> positions;
    std::vector<std::array<std::uint32_t,3>> triangles;
    std::vector<PropVertex> vertices;
    for(unsigned i=0;i<8;++i) {
        const float x=-.875f+.25f*i;
        for(const auto point:std::array<Vector3f,3>{{{x-.09f,-.6f,.5f},{x+.09f,-.6f,.5f},{x,.6f,.5f}}}) {
            positions.push_back(point);
            PropVertex vertex{};vertex.position={point.x,point.y,point.z};vertex.color={1,0,0,1};
            vertices.push_back(vertex);
        }
        triangles.push_back({i*3,i*3+1,i*3+2});
    }
    MeshBoundsTree tree;unsigned samples=0;
    BOOST_REQUIRE(Build_Mesh_Bounds_Tree(positions,triangles,[&] { return samples++%3==0 ? 3u : 0u; },tree));
    BOOST_REQUIRE(tree.nodes.size()>1);
    AssemblyTestData::Bytes tree_bytes,header,polygon_bytes,node_bytes;
    AssemblyTestData::U32(header,static_cast<unsigned>(tree.nodes.size()));
    AssemblyTestData::U32(header,static_cast<unsigned>(tree.polygon_indices.size()));header.resize(32);
    AssemblyTestData::Chunk(tree_bytes,0x91,header);
    for(const auto index:tree.polygon_indices)AssemblyTestData::U32(polygon_bytes,index);
    AssemblyTestData::Chunk(tree_bytes,0x92,polygon_bytes);
    for(const auto& node:tree.nodes) {
        for(float value:{node.bounds.minimum.x,node.bounds.minimum.y,node.bounds.minimum.z,
            node.bounds.maximum.x,node.bounds.maximum.y,node.bounds.maximum.z})
            AssemblyTestData::U32(node_bytes,std::bit_cast<std::uint32_t>(value));
        AssemblyTestData::U32(node_bytes,node.first|(node.leaf?0x80000000u:0u));
        AssemblyTestData::U32(node_bytes,node.second);
    }
    AssemblyTestData::Chunk(tree_bytes,0x93,node_bytes);tree={};
    BOOST_REQUIRE(W3D::W3DRead_Mesh_Bounds_Tree(tree_bytes,static_cast<unsigned>(triangles.size()),tree));
    tree_bytes.clear();
    ModelBoundsTree runtime(std::move(tree));
    std::vector<std::uint32_t> indices;
    runtime.Visit([](const Bounds3f&) { return false; },[&](std::span<const std::uint32_t> polygons) {
        for(const auto polygon:polygons) {
            const auto& triangle=triangles[polygon];
            indices.insert(indices.end(),triangle.begin(),triangle.end());
        }
    });
    positions.clear();triangles.clear();runtime={};
    for(bool warp:{true,false}) {
        GraphicsTestDevice device({warp});if(!warp&&!device.Is_Valid())continue;
        PropRenderer renderer;BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const auto mesh=renderer.Create_Mesh(vertices,indices);
        auto& commands=device.Immediate_Command_List();
        PropStyle style;style.depth_test=false;style.depth_write=false;
        PropParameters parameters;parameters.textured=0;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        for(unsigned width:{128u,256u,128u}) {
            const auto target=device.Create_Texture({width,32,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
            const auto depth=device.Create_Texture({width,32,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
            BOOST_REQUIRE(commands.Set_Viewport({0,0,width,32}));BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
            std::vector<std::byte> pixels(width*32*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
            for(unsigned i=0;i<8;++i)
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(16*width+(2*i+1)*width/16)*4]),255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(16*width+width/8)*4]),0u);
            device.Destroy_Texture(target);device.Destroy_Texture(depth);
        }
        renderer.Destroy_Mesh(mesh);renderer.Shutdown();
    }
}
BOOST_AUTO_TEST_CASE(decoded_aggregate_attachments_draw_on_named_bones_after_resize)
{
    W3D::W3DAggregateDescription source;std::string error;
    BOOST_REQUIRE(W3D::W3DRead_Model_Aggregate(AssemblyTestData::Aggregate(),source,error));
    const auto instance=source.model;source={};
    ModelRigDesc rig;rig.skeleton_name="RIG";
    rig.bones={{"ROOT"},{"LEFT",0,{-.5f,0,0}},{"RIGHT",0,{.5f,0,0}}};
    ModelHierarchy hierarchy(rig);hierarchy.Evaluate_Rest(Affine_Identity());
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
            BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            for(const auto& attachment:instance.attachments) {
                const int bone=hierarchy.Bone_Index(attachment.bone_name);BOOST_REQUIRE(bone>0);
                const auto transform=hierarchy.World_Transform(bone);
                const bool left=attachment.model_name=="Left";
                std::array<PropVertex,3> vertices{};
                vertices[0].position={-.14f,-.7f,.5f};vertices[1].position={.14f,-.7f,.5f};vertices[2].position={0,.7f,.5f};
                for(auto& vertex:vertices) {
                    vertex.position[0]+=transform.matrix[3];vertex.color={left?1.f:0.f,0,left?0.f:1.f,1};
                }
                const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,3>{0,1,2});
                BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));renderer.Destroy_Mesh(mesh);
            }
            std::vector<std::byte> pixels(width*16*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+width/4)*4]),255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+width*3/4)*4+2]),255u);
            device.Destroy_Texture(target);device.Destroy_Texture(depth);
        }
        renderer.Shutdown();
    }
}
BOOST_AUTO_TEST_CASE(decoded_model_level_selection_draws_distinct_details_after_resize)
{
    ModelLevelSetDesc source;std::string error;
    BOOST_REQUIRE(W3D::W3DRead_Model_Level_Set(AssemblyTestData::LevelSet(),source,error));
    const auto instance=source;source={};
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
            for(unsigned selected:{1u,0u,1u}) {
                BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
                const bool high=instance.levels[selected].name=="HIGH";
                std::array<PropVertex,3> vertices{};
                vertices[0].position={-.5f,-.8f,.5f};vertices[1].position={.5f,-.8f,.5f};vertices[2].position={0,.8f,.5f};
                for(auto& vertex:vertices)vertex.color={high?1.f:0.f,0,high?0.f:1.f,1};
                const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,3>{0,1,2});
                BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));renderer.Destroy_Mesh(mesh);
                std::vector<std::byte> pixels(width*16*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+width/2)*4]),selected==0?255u:0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+width/2)*4+2]),selected==1?255u:0u);
            }
            device.Destroy_Texture(target);device.Destroy_Texture(depth);
        }
        renderer.Shutdown();
    }
}
BOOST_AUTO_TEST_CASE(collection_proxy_transform_draws_after_source_release_and_resize)
{
    ModelCollectionDesc source;std::string error;
    BOOST_REQUIRE(W3D::W3DRead_Model_Collection(AssemblyTestData::Collection(),source,error));
    ModelFactoryStore<ModelFactory<ModelCollectionDesc>> factories;
    factories.Insert(source.name,std::make_unique<ModelFactory<ModelCollectionDesc>>(source.name,1,
        [description=source] { return new ModelCollectionDesc(description); }));
    for(const auto* name:{"OTHER", "COLLECTION#Variant"})
        factories.Insert(name,std::make_unique<ModelFactory<ModelCollectionDesc>>(name,1,
            [description=source] { return new ModelCollectionDesc(description); }));
    factories.Erase_If([](const auto& candidate) {
        const auto key=W3D::W3D_Model_Retention_Key(candidate.name);
        return !key || *key!="COLLECTION";
    });
    BOOST_CHECK_EQUAL(factories.Size(),1);
    const auto* factory=factories.Find("collection");BOOST_REQUIRE(factory);
    const std::unique_ptr<ModelCollectionDesc> created(factory->Instantiate());BOOST_REQUIRE(created);
    const auto& instance=*created;source={};factories.Clear();
    BOOST_CHECK_EQUAL(factories.Size(),0);
    AssemblyTestData::Bytes geometry_bytes;
    for(float value:{-.3f,-.1f,.5f,.3f,-.1f,.5f,.3f,.1f,.5f,-.3f,.1f,.5f})
        AssemblyTestData::U32(geometry_bytes,std::bit_cast<std::uint32_t>(value));
    std::vector<Vector3f> points;
    BOOST_REQUIRE(W3D::W3DRead_Geometry_Vectors(geometry_bytes,4,points));
    geometry_bytes.clear();

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
            BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
            BOOST_REQUIRE(commands.Set_Viewport({0,0,width,16}));
            BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            std::array<PropVertex,4> vertices{};
            const auto& transform=instance.proxies[0].transform;
            for(std::size_t i=0;i<vertices.size();++i) {
                for(unsigned row=0;row<3;++row)
                    vertices[i].position[row]=transform[row*4]*points[i].x+transform[row*4+1]*points[i].y
                        +transform[row*4+2]*points[i].z+transform[row*4+3];
                vertices[i].color={1,0,0,1};
            }
            const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,6>{0,1,2,0,2,3});
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));renderer.Destroy_Mesh(mesh);
            std::vector<std::byte> pixels(width*16*4);
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+width/4)*4]),255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(6*width+width/4)*4]),255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+width*3/8)*4]),0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+width/2)*4]),0u);
            device.Destroy_Texture(target);device.Destroy_Texture(depth);
        }
        renderer.Shutdown();
    }
}
BOOST_AUTO_TEST_CASE(decoded_assembly_levels_and_bone_attachments_draw_after_source_release_and_resize) {
    for(unsigned format:{0u,1u,2u}) {
        ModelAssemblyDesc source;std::string error;
        const auto bytes=format==0?AssemblyTestData::Hlod():AssemblyTestData::Hmodel(format==2);
        BOOST_REQUIRE(W3D::W3DRead_Model_Assembly(bytes,format==0,source,error));
        ModelRigDesc rig;rig.skeleton_name="RIG";
        rig.bones={{"ROOT"},{"LEFT",0,{-.5f,0,0}},{"RIGHT",0,{.5f,0,0}}};
        ModelHierarchy hierarchy(rig);
        hierarchy.Evaluate_Rest(Affine_Identity());
        const auto instance=source;source={};
        if(!instance.proxies.empty()) {
            BOOST_CHECK_EQUAL(instance.proxies.front().object_name,"SOCKET");
            BOOST_CHECK_EQUAL(hierarchy.World_Transform(instance.proxies.front().bone).matrix[3],.5f);
        }
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
                for(std::size_t level:{instance.levels.size()-1,std::size_t(0),instance.levels.size()-1}) {
                    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
                    const auto draw=[&](const ModelAttachmentDesc& child) {
                        const bool left=child.object_name=="MODEL.Left",root=child.object_name=="MODEL.Root";
                        const auto transform=hierarchy.World_Transform(child.bone);
                        std::array<PropVertex,3> vertices{};
                        vertices[0].position={-.14f,-.7f,.5f};vertices[1].position={.14f,-.7f,.5f};vertices[2].position={0,.7f,.5f};
                        for(auto& vertex:vertices) {
                            vertex.position[0]+=transform.matrix[3];
                            vertex.color={left?1.f:0.f,root?1.f:0.f,(!left&&!root)?1.f:0.f,1};
                        }
                        const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,3>{0,1,2});
                        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));renderer.Destroy_Mesh(mesh);
                    };
                    for(const auto& child:instance.levels[level].children)draw(child);
                    for(const auto& child:instance.aggregates)draw(child);
                    std::vector<std::byte> pixels(width*16*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+width/4)*4]),level==0?255u:0u);
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+width/2)*4+1]),255u);
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8*width+width*3/4)*4+2]),255u);
                }
                device.Destroy_Texture(target);device.Destroy_Texture(depth);
            }
            renderer.Shutdown();
        }
    }
}
