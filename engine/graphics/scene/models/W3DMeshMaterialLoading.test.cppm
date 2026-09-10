module;
#define BOOST_TEST_MODULE W3DMeshMaterialLoadingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
export module Graphics.Scene.Models.W3DMaterialLoading.Tests;
import Assets.Math;
import Assets.Models;
import Assets.Adapters.W3D.MeshData;
import Assets.Adapters.W3D.Materials;
import Assets.Adapters.W3D.PassBindings;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.State;
import Graphics.Scene.Models.Materials;
import Graphics.Scene.Models.MeshMaterialBindings;
import Graphics.Scene.Models.W3DMaterialLoading;
import Graphics.Scene.Models.MeshMaterialPreparation;
import Graphics.Scene.Props.Material;
namespace {
using Owner = std::shared_ptr<std::string>;
using Bindings = Graphics::MeshMaterialBindings<Owner, std::array<float, 2>>;
Owner Resolve(const Assets::W3D::W3DTextureData& source) { return std::make_shared<std::string>(source.name); }
const char* Name(const Owner& owner) { return owner->c_str(); }
}
BOOST_AUTO_TEST_CASE(ordered_color_records_preserve_multiply_then_authored_alpha_and_resource_lifetime) {
    using namespace Assets::W3D;
    W3DMeshData mesh; mesh.header.vertex_count = 3; mesh.header.triangle_count = 1;
    mesh.header.attributes = 0x2000; mesh.prelit_chunk = 0x24;
    mesh.materials.emplace_back(W3DMaterialInfo{1, 1, 1, 1});
    W3DVertexMaterialData material; material.material.base_color = {1, 1, 1, 1};
    material.material.ambient_color = {1, 1, 1, 1}; material.material.opacity = 1;
    mesh.materials.emplace_back(std::vector<W3DVertexMaterialData>{material});
    W3DShaderSettings shader; shader.destination_blend = 1; shader.source_blend = 1; shader.texturing = 1;
    mesh.materials.emplace_back(std::vector<W3DShaderSettings>{shader});
    mesh.materials.emplace_back(std::vector<W3DTextureData>{{"tile"}});
    W3DPassBindings pass; pass.vertex_material_ids = {0}; pass.shader_ids = {0};
    pass.diffuse_illumination.assign(3, {.5f, .25f, 1.f, 1.f});
    pass.diffuse_colors.assign(3, {0, 0, 0, .5f});
    pass.color_order = {W3DPassColorSource::Illumination, W3DPassColorSource::Diffuse};
    pass.stages.push_back({{0}, {{.1f, .2f}, {.3f, .4f}, {.5f, .6f}}});
    mesh.materials.emplace_back(pass);
    Bindings defaults; defaults.Reset(1, 3, 1);
    Graphics::W3DMeshMaterialLoadOptions options; options.assign_sort_level = true; options.overbright = true;
    Graphics::W3DMeshMaterialLoading loading(mesh, defaults, options, Resolve, Name);
    BOOST_REQUIRE(loading.Read());
    // Illumination becomes the default set; a following diffuse record is the
    // alternate set, retaining authored alpha independently of illumination.
    BOOST_CHECK_EQUAL(defaults.Peek_Color_Array(0)[0], 0xff8040ffu);
    BOOST_CHECK_SMALL(defaults.Peek_UV_Array(0, 0)[0][1] - .8f, 1e-6f);
    std::unique_ptr<Bindings> alternate; Graphics::ModelMaterials<Owner> resources;
    const auto loaded = loading.Finish(alternate, resources);
    BOOST_REQUIRE(loaded.success); BOOST_CHECK(loaded.sorted); BOOST_CHECK_EQUAL(loaded.sort_level, 10);
    BOOST_REQUIRE(alternate);
    BOOST_CHECK_NE(defaults.DCG_Revision(0), 0u);
    BOOST_CHECK_NE(alternate->DCG_Revision(0), 0u);
    BOOST_CHECK_EQUAL(alternate->Peek_Color_Array(0)[0], 0x80000000u);
    BOOST_CHECK(defaults.Get_Single_Shader().Get_Cull_Mode() == Graphics::MaterialState::CULL_MODE_DISABLE);
    BOOST_CHECK(defaults.Get_Single_Shader().Get_Primary_Gradient() == Graphics::MaterialState::GRADIENT_MODULATE2X);
    BOOST_CHECK(alternate->Get_Single_Shader().Get_Primary_Gradient() == Graphics::MaterialState::GRADIENT_MODULATE);
    mesh.materials.clear(); resources.Reset();
    BOOST_CHECK_EQUAL(*defaults.Get_Single_Texture(), "tile");
    BOOST_CHECK_EQUAL(*alternate->Get_Single_Texture(), "tile");
}
BOOST_AUTO_TEST_CASE(resource_ids_are_rejected_before_the_pass_changes_bindings) {
    using namespace Assets::W3D;
    W3DMeshData mesh; mesh.header.vertex_count = 3; mesh.header.triangle_count = 1;
    mesh.materials.emplace_back(W3DMaterialInfo{1, 0, 0, 0});
    W3DPassBindings pass; pass.shader_ids = {99}; mesh.materials.emplace_back(pass);
    Bindings defaults; defaults.Reset(1, 3, 1);
    const auto shader = defaults.Get_Single_Shader().Get_Bits();
    Graphics::W3DMeshMaterialLoading loading(mesh, defaults, {}, Resolve, Name);
    BOOST_CHECK(!loading.Read()); BOOST_CHECK_EQUAL(defaults.Get_Single_Shader().Get_Bits(), shader);
    BOOST_CHECK(!defaults.Has_Shader_Array(0));
}
BOOST_AUTO_TEST_CASE(material_switching_reuses_sort_and_overbright_rules) {
    Bindings bindings; bindings.Reset(1, 3, 2);
    bindings.Set_Single_Shader(Graphics::MaterialState::Opaque(), 0);
    auto additive = Graphics::MaterialState::Opaque();
    additive.Set_Src_Blend_Func(Graphics::MaterialState::SRCBLEND_ONE);
    additive.Set_Dst_Blend_Func(Graphics::MaterialState::DSTBLEND_ONE);
    bindings.Set_Single_Shader(additive, 1);
    BOOST_CHECK_EQUAL(Graphics::Mesh_Material_Sort_Level(bindings), 0);
    bindings.Set_Single_Shader(additive, 0);
    BOOST_CHECK_EQUAL(Graphics::Mesh_Material_Sort_Level(bindings), 10);
    Graphics::Apply_Mesh_Material_Overbright(bindings);
    BOOST_CHECK(bindings.Get_Single_Shader(1).Get_Primary_Gradient() == Graphics::MaterialState::GRADIENT_MODULATE2X);
}
