module;
// MSVC needs Tracy's static helpers when instantiating instrumented module templates.
#include "../../profiling/Tracy.h"
#include "../../profiling/Tracy.h"
#define BOOST_TEST_MODULE ModelMeshDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <vector>
export module Graphics.Scene.Models.MeshDrawing.Tests;
import Graphics.RHI;
import Graphics.Tests.Device;
import Graphics.Materials.State;
import Graphics.Materials.ProceduralPass;
import Graphics.Materials.MeshMaterial;
import Graphics.Resources.Textures.Sampling;
import Graphics.Scene.Models.GeometrySource;
import Graphics.Scene.Props.Instances;
import Graphics.Scene.Props.SkinPalettes;
import Graphics.Scene.Models.Hierarchy;
import Graphics.Scene.AffineTransform;
import Graphics.Scene.Models.SourceRevision;
import Graphics.Scene.Models.MeshMaterialBindings;
import Graphics.Scene.Models.MeshDrawing;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Extraction;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.Material;
import Graphics.Scene.Props.MeshSet;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Shadows.DirectionalRenderer;
import Graphics.Scene.Surfaces.Geometry;
using namespace Graphics;
namespace {
using Position = std::array<float, 3>;
struct CountedPosition final {
    std::array<float,3> values;
    inline static unsigned reads = 0;
    float operator[](std::size_t index) const { ++reads; return values[index]; }
};
struct CountedTriangle final {
    std::array<std::uint32_t, 3> values;
    inline static unsigned reads = 0;
    std::uint32_t operator[](std::size_t index) const { ++reads; return values[index]; }
};
using Triangle = std::array<std::uint32_t, 3>;
using Source = ModelGeometrySource<Position, Triangle, std::array<float, 4>>;
using TextureOwner = std::shared_ptr<RHITextureHandle>;
using Bindings = MeshMaterialBindings<TextureOwner, std::array<float, 2>>;
constexpr std::array<float, 16> Identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
TextureOwner Create_Texture_Owner(GraphicsTestDevice& device, std::array<std::uint8_t, 4> color) {
    const auto handle = device.Create_Texture_Initialized({1, 1}, {std::as_bytes(std::span(color)), 4});
    BOOST_REQUIRE(handle.Is_Valid());
    return TextureOwner(new RHITextureHandle(handle), [&device](auto* value) { device.Destroy_Texture(*value); delete value; });
}
std::optional<PropMaterialTexture> Resolve(const RHITextureHandle* source, bool) {
    return PropMaterialTexture{*source, {}};
}
struct Target final {
    GraphicsTestDevice& device;
    unsigned width;
    RHITextureHandle color, depth;
    Target(GraphicsTestDevice& source, unsigned size) : device(source), width(size) {
        color = device.Create_Texture({width,32,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        depth = device.Create_Texture({width,32,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(color.Is_Valid()); BOOST_REQUIRE(depth.Is_Valid());
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(color, depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,width,32}));
        Clear();
    }
    ~Target() { device.Destroy_Texture(color); device.Destroy_Texture(depth); }
    void Clear() { BOOST_REQUIRE(device.Immediate_Command_List().Clear({0,0,0,0}, 1)); }
    void Pixel(unsigned x, unsigned y, std::array<int, 4> expected) {
        std::vector<std::byte> pixels(width*32*4);
        BOOST_REQUIRE(device.Readback_Texture(color,pixels,width*4));
        for (unsigned c = 0; c < 4; ++c) {
            BOOST_TEST_CONTEXT("pixel " << x << "," << y << " channel " << c) {
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*width+x)*4+c])-expected[c], 2);
            }
        }
    }
};
}
BOOST_AUTO_TEST_CASE(material_batches_draw_both_triangles_after_source_release_opacity_change_and_renderer_recreation) {
    for (const bool warp : {true, false}) {
        GraphicsTestDevice device({warp}); if (!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer; PropExtractionCache extraction; ModelMeshState<TextureOwner> state;
        DirectionalShadowRenderer shadows; PropSubmission submission;
        Source authored; authored.Reset(2, 4);
        *authored.positions = {{-.8f,-.8f,.5f},{.8f,-.8f,.5f},{.8f,.8f,.5f},{-.8f,.8f,.5f}};
        *authored.triangles = {{0,1,2},{0,2,3}};
        auto geometry = authored; authored.Reset(0,0);
        auto red = Create_Texture_Owner(device, {255,0,0,255}), green = Create_Texture_Owner(device, {0,255,0,255});
        Bindings bindings; bindings.Reset(2,4,1);
        bindings.Set_Single_Material(std::make_shared<MeshMaterial>());
        auto shader = MaterialState::Opaque(); shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
        shader.Set_Texturing(MaterialState::TEXTURING_ENABLE); bindings.Set_Single_Shader(shader);
        bindings.Set_Texture(0,red); bindings.Set_Texture(1,green); red.reset(); green.reset();
        for (const unsigned width : {32u, 64u}) {
            BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
            submission.Initialize(device, renderer, shadows);
            Target target(device, width);
            for (const float opacity : {1.f, .5f, 1.f}) {
                target.Clear();
                ModelMeshDrawContext context; context.parameters.view_projection = Identity; context.projection = Identity;
                context.overrides.opacity = opacity;
                ModelMeshDrawing drawing(std::span<const Position>(*geometry.positions), std::span<const Position>(*geometry.normals),
                    std::span<const Triangle>(*geometry.triangles), geometry.revision.Token(), bindings, state, renderer, extraction, context);
                unsigned draws = 0;
                BOOST_REQUIRE(drawing.Draw_Base([&](auto vertices, auto indices, auto draw_shader, auto textures, auto parameters, auto overrides) {
                    ++draws; BOOST_CHECK_EQUAL(indices.size(), 3);
                    return Submit_Prop_Material(device,renderer,submission,vertices,indices,draw_shader,textures,Resolve,parameters,{},overrides);
                }));
                BOOST_CHECK_EQUAL(draws, 2);
                const int rgb = opacity == 1 ? 255 : 128;
                const int alpha = opacity == 1 ? 255 : 64;
                target.Pixel(width*3/4,22,{rgb,0,0,alpha});
                target.Pixel(width/4,10,{0,rgb,0,alpha});
                target.Pixel(1,1,{0,0,0,0});
            }
            submission.Shutdown(); renderer.Shutdown();
        }
        state.base.Clear(); state.additional.Clear();
    }
}
BOOST_AUTO_TEST_CASE(material_submission_retains_only_transferred_textures_and_preserves_deferred_visibility) {
    for (const bool warp : {true, false}) {
        GraphicsTestDevice device({warp}); if (!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer; BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        DirectionalShadowRenderer shadows; PropSubmission submission; submission.Initialize(device,renderer,shadows);
        Target target(device,32);
        std::array<SurfaceVertex,4> vertices;
        vertices[0].position={-.8f,-.8f,.5f}; vertices[1].position={.8f,-.8f,.5f};
        vertices[2].position={.8f,.8f,.5f}; vertices[3].position={-.8f,.8f,.5f};
        for(auto& vertex:vertices) vertex.color={1,1,1,1};
        const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
        auto texture = Create_Texture_Owner(device,{255,0,0,255});
        PropParameters parameters; parameters.view_projection=Identity;
        auto shader=MaterialState::Opaque(); shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
        shader.Set_Texturing(MaterialState::TEXTURING_ENABLE);
        PropSubmission rejected;
        BOOST_CHECK(!Submit_Prop_Surface(device,renderer,rejected,std::span<const SurfaceVertex>(vertices),
            std::span<const std::uint32_t>(indices),shader,std::array<const RHITextureHandle*,2>{texture.get(),nullptr},Resolve,parameters,{}));
        // Rejection must leave the borrowed source live and usable repeatedly.
        for(unsigned i=0;i<2;++i) {
            target.Clear();
            BOOST_REQUIRE(Submit_Prop_Surface(device,renderer,submission,std::span<const SurfaceVertex>(vertices),
                std::span<const std::uint32_t>(indices),shader,std::array<const RHITextureHandle*,2>{texture.get(),nullptr},Resolve,parameters,{}));
            target.Pixel(24,22,{255,0,0,255}); target.Pixel(8,10,{255,0,0,255});
        }
        target.Clear(); PropMaterialDrawContext context; context.sorting_depth={0,0,1,0};
        const auto handle=*texture;
        BOOST_REQUIRE(Submit_Prop_Surface(device,renderer,submission,std::span<const SurfaceVertex>(vertices),
            std::span<const std::uint32_t>(indices),shader,std::array<const RHITextureHandle*,2>{texture.get(),nullptr},Resolve,parameters,context));
        texture.reset();
        target.Pixel(24,22,{0,0,0,0});
        BOOST_REQUIRE(device.Retain_Texture(handle)); device.Destroy_Texture(handle);
        BOOST_REQUIRE(submission.Flush_Transparent());
        target.Pixel(24,22,{255,0,0,255}); target.Pixel(8,10,{255,0,0,255}); target.Pixel(1,1,{0,0,0,0});
        BOOST_CHECK(!device.Retain_Texture(handle));
        submission.Shutdown(); renderer.Shutdown();
    }
}

BOOST_AUTO_TEST_CASE(additional_pass_keeps_selection_world_coordinates_color_mask_and_queued_lifetimes) {
    GraphicsTestDevice device({true}); BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer; BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    PropExtractionCache extraction; ModelMeshState<TextureOwner> state;
    DirectionalShadowRenderer shadows; PropSubmission submission; submission.Initialize(device,renderer,shadows);
    Target target(device,32);
    const std::array<Position,4> positions{{{-.8f,-.8f,.5f},{.8f,-.8f,.5f},{.8f,.8f,.5f},{-.8f,.8f,.5f}}};
    const std::array<Triangle,2> triangles{{{0,1,2},{0,2,3}}};
    Bindings bindings; bindings.Reset(2,4,1);
    ModelMeshDrawContext context; context.parameters.view_projection=context.projection=Identity;
    context.additional_only=true; context.translucent=true;
    context.overrides.pass_opacity=.5f; context.overrides.pass_emissive=.25f;
    ModelMeshDrawing drawing(std::span<const Position>(positions),std::span<const Position>{},
        std::span<const Triangle>(triangles),0,bindings,state,renderer,extraction,context);
    BOOST_CHECK(!drawing.Accepts_Additional_Pass(false)); BOOST_CHECK(drawing.Accepts_Additional_Pass(true));
    unsigned base_calls=0;
    BOOST_REQUIRE(drawing.Draw_Base([&](auto,auto,auto,auto,auto,auto) { ++base_calls; return true; }));
    BOOST_CHECK_EQUAL(base_calls,0);
    ProceduralMaterialPass<TextureOwner,int>::Description description;
    description.shader=MaterialState::Opaque(); description.shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
    description.shader.Set_Texturing(MaterialState::TEXTURING_ENABLE);
    auto texture=Create_Texture_Owner(device,{0,255,0,255}); description.textures[0]=texture.get();
    auto material=std::make_shared<MeshMaterial>(); material->parameters.emissive={.4f,.8f,1.f};
    description.material=material.get(); description.world_coordinates=true;
    description.world_texture_transform[3]=.25f; description.color_write_mask=7;
    const std::array<std::uint32_t,1> selected{1};
    BOOST_REQUIRE(drawing.Draw_Additional(description,std::span<const std::uint32_t>(selected),0,
        [&](auto vertices,auto indices,auto,auto,auto parameters,auto overrides) {
            BOOST_CHECK_EQUAL(indices.size(),3); BOOST_CHECK_EQUAL(parameters.uv_sources[0],4.f);
            BOOST_CHECK_EQUAL(parameters.uv_transform[0][3],.25f); BOOST_CHECK(overrides.deferred_pass);
            for(const auto& vertex:vertices) {
                BOOST_CHECK_EQUAL(vertex.material_diffuse[3],.5f);
                BOOST_CHECK_SMALL(vertex.material_emissive[0]-.1f,1e-6f);
                BOOST_CHECK_SMALL(vertex.material_emissive[1]-.2f,1e-6f);
                BOOST_CHECK_EQUAL(vertex.material_emissive[2],.25f);
            }
            return true;
        }));
    material->parameters.emissive={};
    BOOST_REQUIRE(drawing.Draw_Additional(description,std::span<const std::uint32_t>(selected),0,
        [&](auto vertices,auto indices,auto shader,auto textures,auto parameters,auto overrides) {
            return Submit_Prop_Material(device,renderer,submission,vertices,indices,shader,textures,Resolve,parameters,{},overrides);
        }));
    texture.reset(); material.reset(); state.additional.Clear();
    target.Pixel(8,10,{0,0,0,0});
    BOOST_REQUIRE(submission.Flush_Materials());
    target.Pixel(8,10,{0,255,0,0}); target.Pixel(24,22,{0,0,0,0}); target.Pixel(1,1,{0,0,0,0});
    submission.Shutdown(); renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(additional_pass_reuses_prepared_geometry_and_invalidates_only_changed_inputs) {
    GraphicsTestDevice device({true});
    PropRenderer renderer; PropExtractionCache extraction; ModelMeshState<TextureOwner> state;
    DirectionalShadowRenderer shadows; PropSubmission submission;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    submission.Initialize(device,renderer,shadows);
    Target target(device,32);
    std::array<CountedPosition,4> positions{{{{-.8f,-.8f,.5f}},{{.8f,-.8f,.5f}},{{.8f,.8f,.5f}},{{-.8f,.8f,.5f}}}};
    const std::array<Triangle,2> triangles{{{0,1,2},{0,2,3}}};
    std::array<std::uint32_t,1> selected{1};
    std::span<const std::uint32_t> polygons = selected;
    std::uint64_t polygon_revision = 0;
    Bindings bindings; bindings.Reset(2,4,1);
    ModelMeshDrawContext context; context.parameters.view_projection=context.projection=Identity;
    ProceduralMaterialPass<TextureOwner,int>::Description description;
    description.shader = MaterialState::Opaque();
    description.shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
    MeshMaterial material; description.material = &material;
    std::uint64_t revision = 1;
    PropMeshHandle submitted;
    const auto draw = [&] {
        target.Clear(); CountedPosition::reads = 0;
        ModelMeshDrawing drawing(std::span<const CountedPosition>(positions),std::span<const CountedPosition>{},
            std::span<const Triangle>(triangles),revision,bindings,state,renderer,extraction,context);
        BOOST_REQUIRE(drawing.Draw_Additional(description,polygons,0,
            [&](auto vertices,auto indices,auto shader,auto textures,auto parameters,auto overrides) {
                submitted = overrides.mesh;
                return Submit_Prop_Material(device,renderer,submission,vertices,indices,shader,textures,Resolve,parameters,{},overrides);
            },polygon_revision));
    };
    draw(); const auto first = submitted;
    BOOST_CHECK_GT(CountedPosition::reads,0u);
    target.Pixel(8,10,{255,255,255,255}); target.Pixel(24,22,{0,0,0,0});
    draw(); BOOST_CHECK(submitted == first); BOOST_CHECK_EQUAL(CountedPosition::reads,0u);
    context.parameters.world[3] = 2;
    draw(); BOOST_CHECK(submitted == first); BOOST_CHECK_EQUAL(CountedPosition::reads,0u);
    target.Pixel(8,10,{0,0,0,0});
    context.parameters.world[3] = 0;
    selected[0] = 0;
    draw(); BOOST_CHECK(submitted != first); BOOST_CHECK_GT(CountedPosition::reads,0u);
    target.Pixel(8,10,{0,0,0,0}); target.Pixel(24,22,{255,255,255,255});
    material.parameters.diffuse = {1,0,0};
    draw(); BOOST_CHECK_GT(CountedPosition::reads,0u);
    target.Pixel(24,22,{255,0,0,255});
    const auto red = submitted;
    submission.Shutdown(); renderer.Shutdown();
    BOOST_REQUIRE(renderer.Initialize(device,shaders)); submission.Initialize(device,renderer,shadows);
    draw(); BOOST_CHECK(submitted == red); BOOST_CHECK_EQUAL(CountedPosition::reads,0u);
    target.Pixel(24,22,{255,0,0,255});
    positions[0].values[0] = -.7f; ++revision;
    draw(); BOOST_CHECK(submitted != red); BOOST_CHECK_GT(CountedPosition::reads,0u);
    target.Pixel(24,22,{255,0,0,255});
    polygons = state.Complete_Polygons(triangles.size());
    polygon_revision = state.Complete_Polygon_Revision();
    BOOST_REQUIRE_EQUAL(polygons.size(),2u);
    BOOST_CHECK_EQUAL(polygons[0],0u); BOOST_CHECK_EQUAL(polygons[1],1u);
    draw(); BOOST_CHECK_GT(CountedPosition::reads,0u);
    target.Pixel(8,10,{255,0,0,255}); target.Pixel(24,22,{255,0,0,255});
    const auto complete = submitted;
    BOOST_CHECK(state.Complete_Polygons(triangles.size()).data() == polygons.data());
    BOOST_CHECK_EQUAL(state.Complete_Polygon_Revision(),polygon_revision);
    draw(); BOOST_CHECK(submitted == complete); BOOST_CHECK_EQUAL(CountedPosition::reads,0u);
    polygons = state.Complete_Polygons(1);
    BOOST_CHECK_NE(state.Complete_Polygon_Revision(),polygon_revision);
    polygon_revision = state.Complete_Polygon_Revision();
    draw(); BOOST_CHECK_GT(CountedPosition::reads,0u);
    target.Pixel(8,10,{0,0,0,0}); target.Pixel(24,22,{255,0,0,255});
    state.additional.Clear(); submission.Shutdown(); renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(skinned_instances_share_bind_geometry_and_move_without_reading_source_vertices) {
    GraphicsTestDevice device({true}); BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer; BOOST_REQUIRE(renderer.Initialize(device, Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    PropExtractionCache extraction;
    DirectionalShadowRenderer shadows; PropSubmission submission; submission.Initialize(device, renderer, shadows);
    Target target(device, 32);
    const std::array<CountedPosition, 3> positions{{{{-.2f,-.3f,.5f}},{{.2f,-.3f,.5f}},{{0,.3f,.5f}}}};
    std::array<std::uint16_t, 3> bones{};
    const std::array<Triangle, 1> triangles{{{0,1,2}}};
    SourceRevision source;
    std::array<PropSkinOwner, 2> skins;
    std::array<PropInstanceOwner, 2> instances;
    std::array<PropMeshHandle, 2> meshes;
    std::array<PropSkinPaletteHandle, 2> palettes;
    ModelMeshState<TextureOwner> state;
    Bindings bindings; bindings.Reset(1,3,1);
    bindings.Set_Single_Material(std::make_shared<MeshMaterial>());
    auto shader = MaterialState::Opaque(); shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
    shader.Set_Texturing(MaterialState::TEXTURING_ENABLE); bindings.Set_Single_Shader(shader);
    auto texture = Create_Texture_Owner(device, {255,255,255,255}); bindings.Set_Single_Texture(texture);
    bindings.Allocate_Color_Array(0); bindings.Set_DCG_Source(0, PropColorSource::PrimaryColor);
    for (unsigned vertex = 0; vertex < 3; ++vertex) bindings.Set_Color(0, vertex, 0xffff0000u);
    const auto color_revision = bindings.DCG_Revision(0);
    const auto draw = [&](unsigned instance, float x, bool additional=false) {
        const std::array<PropBoneTransform,2> pose{{
            {1,0,0,x,0,1,0,0,0,0,1,0}, {1,0,0,x,0,1,0,.5f,0,0,1,0}}};
        palettes[instance]=skins[instance].Update(renderer.Instances().Palettes(),pose.size(),
            [&](std::size_t bone) -> const auto& { return pose[bone]; });
        ModelMeshDrawContext context; context.parameters.view_projection = Identity; context.projection = Identity;
        context.parameters.normal_in_world_space=1; context.bone_links=bones;
        ModelMeshDrawing drawing(std::span<const CountedPosition>(positions), std::span<const CountedPosition>{},
            std::span<const Triangle>(triangles),source.Token(),bindings,state,renderer,extraction,context);
        const auto submit = [&](auto vertices, auto indices, auto draw_shader, auto textures, auto parameters, auto overrides) {
            meshes[instance]=overrides.mesh;
            overrides.instance=&instances[instance]; overrides.skin=palettes[instance];
            PropMaterialDrawContext submission_context; submission_context.batchable=true;
            return Submit_Prop_Material(device,renderer,submission,vertices,indices,draw_shader,textures,
                Resolve,parameters,submission_context,overrides);
        };
        if (additional) {
            ProceduralMaterialPass<TextureOwner,int>::Description description;
            description.shader=shader; description.material=bindings.Peek_Single_Material();
            description.textures[0]=texture.get();
            const std::array<std::uint32_t,1> polygons{0};
            BOOST_REQUIRE(drawing.Draw_Additional(description,std::span<const std::uint32_t>(polygons),0,submit));
        } else BOOST_REQUIRE(drawing.Draw_Base(submit));
    };
    submission.Begin_Batching();
    const auto before=device.Immediate_Command_List().Submission_Counts();
    draw(0,-.5f); draw(1,.5f);
    BOOST_REQUIRE(submission.End_Batching());
    BOOST_CHECK_EQUAL(device.Immediate_Command_List().Submission_Counts().draw_calls-before.draw_calls,1u);
    BOOST_CHECK(meshes[0]==meshes[1]);
    const auto bind_mesh=meshes[0];
    target.Pixel(8,16,{255,0,0,255}); target.Pixel(24,16,{255,0,0,255});
    const auto first_palette=palettes[0];
    target.Clear(); CountedPosition::reads=0;
    draw(0,-.5f); draw(1,0);
    BOOST_CHECK(palettes[0]==first_palette);
    BOOST_CHECK(meshes[0]==bind_mesh && meshes[1]==bind_mesh);
    BOOST_CHECK_EQUAL(CountedPosition::reads,0u);
    target.Pixel(8,16,{255,0,0,255}); target.Pixel(16,16,{255,0,0,255}); target.Pixel(24,16,{0,0,0,0});
    BOOST_CHECK_EQUAL(bindings.DCG_Revision(0),color_revision);
    target.Clear();
    for (unsigned vertex=0; vertex<3; ++vertex) bindings.Set_Color(0,vertex,0xff00ff00u);
    draw(0,-.5f); target.Pixel(8,16,{0,255,0,255});
    BOOST_CHECK_NE(bindings.DCG_Revision(0),color_revision);
    auto* writable=bindings.Get_DCG_Array(0);
    draw(0,-.5f);
    target.Clear();
    for (unsigned vertex=0; vertex<3; ++vertex) writable[vertex]=0xff0000ffu;
    draw(0,-.5f); target.Pixel(8,16,{0,0,255,255});
    // Bone links have escaped their source revision domain. Both base and
    // additional geometry must see their mutation while retaining bind positions.
    target.Clear(); bones.fill(1);
    draw(0,-.5f); target.Pixel(8,8,{0,0,255,255}); target.Pixel(8,16,{0,0,0,0});
    target.Clear(); draw(0,.5f,true);
    target.Pixel(24,8,{0,0,255,255}); target.Pixel(24,16,{0,0,0,0});
    target.Clear(); bones.fill(0); draw(0,.5f,true);
    target.Pixel(24,16,{0,0,255,255}); target.Pixel(24,8,{0,0,0,0});
    submission.Shutdown(); renderer.Shutdown();
}
BOOST_AUTO_TEST_CASE(retained_packets_skip_source_reads_and_material_preparation_until_inputs_change) {
    GraphicsTestDevice device({true}); BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer; BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    PropExtractionCache extraction; ModelMeshState<TextureOwner> state;
    DirectionalShadowRenderer shadows; PropSubmission submission; submission.Initialize(device, renderer, shadows);
    Target target(device, 32);
    std::array<CountedPosition, 4> positions{{{{-.8f,-.8f,.5f}},{{.8f,-.8f,.5f}},{{.8f,.8f,.5f}},{{-.8f,.8f,.5f}}}};
    std::array<CountedTriangle, 2> triangles{{{{0,1,2}},{{0,2,3}}}};
    SourceRevision revision, topology_revision;
    auto texture = Create_Texture_Owner(device, {255,0,0,255});
    auto material = std::make_shared<MeshMaterial>();
    Bindings bindings; bindings.Reset(2,4,1);
    bindings.Set_Single_Material(material); bindings.Set_Single_Texture(texture);
    auto shader = MaterialState::Opaque(); shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
    shader.Set_Texturing(MaterialState::TEXTURING_ENABLE); bindings.Set_Single_Shader(shader);
    ModelMeshDrawContext context; context.parameters.view_projection = context.projection = Identity;
    PropMaterialDrawContext submission_context;
    PropMeshHandle submitted;
    unsigned resolutions = 0;
    const auto draw = [&] {
        target.Clear(); CountedPosition::reads = CountedTriangle::reads = 0;
        ModelMeshDrawing drawing(std::span<const CountedPosition>(positions), std::span<const CountedPosition>{},
            std::span<const CountedTriangle>(triangles), revision.Token(), bindings, state, renderer, extraction, context,
            topology_revision.Token());
        BOOST_REQUIRE(drawing.Draw_Base([&](auto vertices, auto indices, auto draw_shader, auto textures, auto parameters, auto overrides) {
            submitted = overrides.mesh;
            return Submit_Prop_Material(device, renderer, submission, vertices, indices, draw_shader, textures,
                [&](auto source, bool load) { ++resolutions; return Resolve(source, load); }, parameters, submission_context, overrides);
        }));
    };
    draw(); const auto original_mesh = submitted;
    BOOST_CHECK_GT(CountedPosition::reads, 0u); BOOST_CHECK_GT(CountedTriangle::reads, 0u);
    BOOST_REQUIRE_EQUAL(state.groups[0].packets.size(), 1u);
    BOOST_CHECK_EQUAL(state.groups[0].packets[0].preparation.Preparation_Count(), 1u);
    target.Pixel(24,22,{255,0,0,255}); target.Pixel(8,10,{255,0,0,255});
    // A second instance/view changes dynamic inputs without reading source
    // triangles/vertices or resolving the material and samplers again.
    context.parameters.world[3] = .05f; context.milliseconds = 100;
    draw(); BOOST_CHECK(submitted == original_mesh);
    BOOST_CHECK_EQUAL(CountedPosition::reads, 0u); BOOST_CHECK_EQUAL(CountedTriangle::reads, 0u);
    BOOST_CHECK_EQUAL(state.groups[0].packets[0].preparation.Preparation_Count(), 1u);
    target.Pixel(24,22,{255,0,0,255}); target.Pixel(8,10,{255,0,0,255}); target.Pixel(1,1,{0,0,0,0});
    // Public material fields have no revision contract. Their actual values
    // must invalidate geometry even at the same material address.
    material->parameters.opacity = .5f;
    draw(); BOOST_CHECK(submitted != original_mesh); BOOST_CHECK_GT(CountedPosition::reads, 0u);
    target.Pixel(24,22,{255,0,0,128}); target.Pixel(8,10,{255,0,0,128});
    const auto translucent_mesh = submitted;
    draw(); BOOST_CHECK(submitted == translucent_mesh); BOOST_CHECK_EQUAL(CountedPosition::reads, 0u);
    BOOST_CHECK_EQUAL(CountedTriangle::reads, 0u);
    // Publishing a new GPU generation through the same logical owner must
    // sample the new resource while keeping the unchanged mesh and style.
    auto green = Create_Texture_Owner(device, {0,255,0,255});
    std::swap(*texture, *green);
    draw(); BOOST_CHECK(submitted == translucent_mesh);
    BOOST_CHECK_EQUAL(state.groups[0].packets[0].preparation.Preparation_Count(), 1u);
    target.Pixel(24,22,{0,255,0,128}); target.Pixel(8,10,{0,255,0,128});
    BOOST_CHECK_EQUAL(resolutions, 5u);
    // Scene state belongs to the current view, including a zero write mask.
    submission_context.scene.color_write_mask = 0;
    draw(); target.Pixel(24,22,{0,0,0,0});
    BOOST_CHECK_EQUAL(state.groups[0].packets[0].preparation.Preparation_Count(), 2u);
    submission_context.scene.color_write_mask = 15;
    draw(); target.Pixel(8,10,{0,255,0,128});
    BOOST_CHECK_EQUAL(state.groups[0].packets[0].preparation.Preparation_Count(), 3u);
    // Recreating the renderer keeps CPU mesh ownership valid for first use.
    submission.Shutdown(); renderer.Shutdown();
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    submission.Initialize(device, renderer, shadows);
    draw(); target.Pixel(24,22,{0,255,0,128}); target.Pixel(8,10,{0,255,0,128});
    // A changed pose reads the six indexed corners needed for extraction,
    // without another topology read to recover material ownership/ranges.
    positions[2].values[0] = .7f; revision.Invalidate();
    draw(); BOOST_CHECK_EQUAL(CountedTriangle::reads, 6u);
    BOOST_CHECK_GT(CountedPosition::reads, 0u);
    target.Pixel(24,22,{0,255,0,128}); target.Pixel(8,10,{0,255,0,128});
    std::swap(triangles[0], triangles[1]); topology_revision.Invalidate();
    draw(); BOOST_CHECK_EQUAL(CountedTriangle::reads, 7u);
    target.Pixel(24,22,{0,255,0,128}); target.Pixel(8,10,{0,255,0,128});
    submission.Shutdown(); renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(retained_material_sampling_and_texturing_follow_authored_changes) {
    GraphicsTestDevice device({true}); BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer; BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    DirectionalShadowRenderer shadows; PropSubmission submission; submission.Initialize(device, renderer, shadows);
    Target target(device, 32);
    const std::array<std::uint8_t, 8> texels{255,0,0,255, 0,255,0,128};
    const auto texture = device.Create_Texture_Initialized({2,1}, {std::as_bytes(std::span(texels)),8});
    BOOST_REQUIRE(texture.Is_Valid());
    std::array<PropVertex,4> vertices;
    vertices[0].position={-.8f,-.8f,.5f}; vertices[1].position={.8f,-.8f,.5f};
    vertices[2].position={.8f,.8f,.5f}; vertices[3].position={-.8f,.8f,.5f};
    for (auto& vertex : vertices) vertex.uv={1.25f,.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices); BOOST_REQUIRE(mesh.Is_Valid());
    PropMaterialPreparation preparation;
    PropMaterialDrawOverrides overrides; overrides.mesh=mesh; overrides.preparation=&preparation;
    PropParameters parameters; parameters.view_projection=Identity;
    auto shader=MaterialState::Opaque(); shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
    shader.Set_Texturing(MaterialState::TEXTURING_ENABLE);
    TextureSampling sampling;
    const auto draw = [&] {
        target.Clear();
        BOOST_REQUIRE(Submit_Prop_Material(device,renderer,submission,std::span<const PropVertex>(vertices),
            std::span<const std::uint32_t>(indices),shader,std::array<const RHITextureHandle*,2>{&texture,nullptr},
            [&](auto, bool) -> std::optional<PropMaterialTexture> { return PropMaterialTexture{texture,sampling}; },
            parameters,{},overrides));
    };
    draw(); target.Pixel(24,22,{255,0,0,255}); target.Pixel(8,10,{255,0,0,255});
    draw(); BOOST_CHECK_EQUAL(preparation.Preparation_Count(),1u);
    sampling.address[0]=RHISamplerAddress::Clamp;
    draw(); target.Pixel(24,22,{0,255,0,128}); target.Pixel(8,10,{0,255,0,128});
    BOOST_CHECK_EQUAL(preparation.Preparation_Count(),2u);
    shader.Set_Texturing(MaterialState::TEXTURING_DISABLE);
    draw(); target.Pixel(24,22,{255,255,255,255}); target.Pixel(8,10,{255,255,255,255});
    BOOST_CHECK_EQUAL(preparation.Preparation_Count(),3u);
    shader.Set_Texturing(MaterialState::TEXTURING_ENABLE);
    draw(); target.Pixel(24,22,{0,255,0,128}); target.Pixel(8,10,{0,255,0,128});
    target.Pixel(1,1,{0,0,0,0});
    renderer.Destroy_Mesh(mesh); device.Destroy_Texture(texture);
    submission.Shutdown(); renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(material_group_ranges_follow_changes_and_keep_polygon_order) {
    GraphicsTestDevice device({true}); BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer; BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    PropExtractionCache extraction; ModelMeshState<TextureOwner> state;
    DirectionalShadowRenderer shadows; PropSubmission submission; submission.Initialize(device, renderer, shadows);
    Target target(device, 32);
    const std::array<Position,4> positions{{{-.8f,-.8f,.5f},{.8f,-.8f,.5f},{.8f,.8f,.5f},{-.8f,.8f,.5f}}};
    std::array<Triangle,2> triangles{{{0,1,2},{0,2,3}}};
    SourceRevision revision;
    auto red = Create_Texture_Owner(device, {255,0,0,255}), green = Create_Texture_Owner(device, {0,255,0,255});
    Bindings bindings; bindings.Reset(2,4,1);
    bindings.Set_Single_Material(std::make_shared<MeshMaterial>());
    auto shader = MaterialState::Opaque(); shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
    shader.Set_Texturing(MaterialState::TEXTURING_ENABLE); bindings.Set_Single_Shader(shader);
    bindings.Set_Texture(0, red); bindings.Set_Texture(1, green);
    const auto draw = [&] {
        ModelMeshDrawContext context; context.parameters.view_projection = Identity; context.projection = Identity;
        ModelMeshDrawing drawing(std::span<const Position>(positions), std::span<const Position>{},
            std::span<const Triangle>(triangles), revision.Token(), bindings, state, renderer, extraction, context);
        std::vector<const RHITextureHandle*> order;
        BOOST_REQUIRE(drawing.Draw_Base([&](auto vertices, auto indices, auto shader, auto textures, auto parameters, auto overrides) {
            order.push_back(textures[0]);
            return Submit_Prop_Material(device, renderer, submission, vertices, indices, shader, textures,
                Resolve, parameters, {}, overrides);
        }));
        return order;
    };
    BOOST_CHECK((draw() == std::vector<const RHITextureHandle*>{red.get(),green.get()}));
    BOOST_REQUIRE(state.groups[0].valid);
    BOOST_CHECK((state.groups[0].packets.size() == 2 && state.groups[0].packets[0].end == 1 && state.groups[0].packets[1].end == 2));
    const auto* ranges = state.groups[0].packets.data();
    target.Clear(); draw();
    BOOST_CHECK(state.groups[0].packets.data() == ranges);
    target.Pixel(24,22,{255,0,0,255}); target.Pixel(8,10,{0,255,0,255});
    bindings.Set_Texture(1,red); target.Clear();
    BOOST_CHECK((draw() == std::vector<const RHITextureHandle*>{red.get()}));
    BOOST_CHECK((state.groups[0].packets.size() == 1 && state.groups[0].packets[0].end == 2));
    target.Pixel(8,10,{255,0,0,255});
    auto* slot = bindings.Get_Texture_Array(0,0)->Peek(1);
    *slot = green; target.Clear(); draw();
    BOOST_CHECK(!state.groups[0].valid);
    target.Pixel(8,10,{0,255,0,255});
    *slot = red; target.Clear(); draw();
    target.Pixel(8,10,{255,0,0,255});
    // Allocated-but-empty arrays fall back to single resources. Retained
    // ownership must follow Peek's selection, including these missing slots.
    bindings.Set_Single_Texture(green);
    bindings.Get_Texture_Array(0,0)->Allocate(0);
    bindings.Get_Material_Array(0,true)->Allocate(0);
    bindings.Get_Single_Material()->parameters.opacity = .5f;
    target.Clear();
    BOOST_CHECK((draw() == std::vector<const RHITextureHandle*>{green.get()}));
    target.Pixel(24,22,{0,255,0,128}); target.Pixel(8,10,{0,255,0,128});
    submission.Shutdown(); renderer.Shutdown();
}
