module;

#define BOOST_TEST_MODULE MeshMaterialBindingsTests
#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <memory>

export module Graphics.Scene.Models.MeshMaterialBindings.Tests;

import Assets.Math;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.State;
import Graphics.Scene.Models.MeshMaterialBindings;
import Graphics.Scene.Models.MeshMaterialPreparation;
import Graphics.Scene.Props.Material;

namespace {
using Bindings = Graphics::MeshMaterialBindings<std::shared_ptr<int>, std::array<float, 2>>;
}

BOOST_AUTO_TEST_CASE(copy_preserves_vertex_color_sources)
{
    Bindings source;
    source.Reset(3, 2, 2);
    source.Set_DCG_Source(0, Graphics::PropColorSource::PrimaryColor);
    source.Set_DIG_Source(0, Graphics::PropColorSource::SecondaryColor);
    source.Set_DCG_Source(1, Graphics::PropColorSource::SecondaryColor);
    source.Set_DIG_Source(1, Graphics::PropColorSource::PrimaryColor);

    const Bindings copy = source;
    BOOST_CHECK(copy.Get_DCG_Source(0) == Graphics::PropColorSource::PrimaryColor);
    BOOST_CHECK(copy.Get_DIG_Source(0) == Graphics::PropColorSource::SecondaryColor);
    BOOST_CHECK(copy.Get_DCG_Source(1) == Graphics::PropColorSource::SecondaryColor);
    BOOST_CHECK(copy.Get_DIG_Source(1) == Graphics::PropColorSource::PrimaryColor);
}

BOOST_AUTO_TEST_CASE(controlled_color_changes_preserve_aliases_and_raw_pointer_fallback)
{
    Bindings source; source.Reset(1, 3, 1);
    source.Allocate_Color_Array(0);
    source.Set_DCG_Source(0, Graphics::PropColorSource::PrimaryColor);
    source.Set_Color(0, 0, 0xff112233u);
    Bindings copy = source;
    const auto revision = source.DCG_Revision(0);
    BOOST_CHECK_NE(revision, 0u);
    BOOST_CHECK_EQUAL(copy.DCG_Revision(0), revision);
    const auto* colors = source.Peek_DCG_Array(0);
    BOOST_CHECK_EQUAL(source.DCG_Revision(0), revision);
    copy.Set_Color(0, 0, 0xff445566u);
    BOOST_CHECK_EQUAL(colors[0], 0xff445566u);
    BOOST_CHECK_NE(source.DCG_Revision(0), revision);
    BOOST_CHECK_EQUAL(source.DCG_Revision(0), copy.DCG_Revision(0));
    auto* writable = copy.Get_DCG_Array(0);
    BOOST_CHECK_EQUAL(source.DCG_Revision(0), 0u);
    source.Set_Color(0, 0, 0xff778899u);
    BOOST_CHECK_EQUAL(writable[0], 0xff778899u);
    BOOST_CHECK_EQUAL(source.DCG_Revision(0), 0u);
    writable[0] = 0xffaabbccu;
    BOOST_CHECK_EQUAL(source.Peek_DCG_Array(0)[0], 0xffaabbccu);
    source.Make_Color_Array_Unique(0);
    source.Set_Color(0, 0, 0xffffffffu);
    BOOST_CHECK_EQUAL(copy.Peek_DCG_Array(0)[0], 0xffaabbccu);
}

BOOST_AUTO_TEST_CASE(grouping_versions_follow_shared_slots_and_writable_shader_storage)
{
    Bindings source; source.Reset(2, 3, 1);
    const auto shader = Graphics::MaterialState::Opaque();
    source.Set_Shader(0, shader); source.Set_Shader(1, shader);
    source.Set_Texture(0, std::make_shared<int>(1));
    source.Set_Texture(1, std::make_shared<int>(2));
    const auto versions = source.Grouping_Revisions(0);
    BOOST_REQUIRE(versions);
    Bindings clone = source;
    BOOST_REQUIRE(clone.Grouping_Revisions(0));
    BOOST_CHECK(*clone.Grouping_Revisions(0) != *versions);
    auto slots = *source.Get_Texture_Array(0, 0);
    slots.Set(1, std::make_shared<int>(3));
    BOOST_CHECK(*source.Grouping_Revisions(0) != *versions);
    BOOST_CHECK_EQUAL(*source.Peek_Texture(1), 3);
    BOOST_CHECK_EQUAL(*clone.Peek_Texture(1), 2);
    auto* writable = slots.Peek(1);
    BOOST_CHECK(!source.Grouping_Revisions(0));
    *writable = std::make_shared<int>(4);
    BOOST_CHECK_EQUAL(*source.Peek_Texture(1), 4);
    auto* shaders = clone.Get_Shader_Array(0);
    BOOST_CHECK(!clone.Grouping_Revisions(0));
    shaders[0] = Graphics::MaterialState{0};
    clone.Set_Shader(1, shader);
    BOOST_CHECK(!clone.Grouping_Revisions(0));
    BOOST_CHECK_EQUAL(clone.Get_Shader(0).Get_Bits(), 0u);
}

BOOST_AUTO_TEST_CASE(two_sided_and_fog_update_single_and_array_shader_state)
{
    Bindings bindings;
    bindings.Reset(1, 1, 1);
    bindings.Set_Single_Shader(Graphics::MaterialState::Opaque(), 0);
    bindings.Set_Shader(0, Graphics::MaterialState::Opaque(), 0);

    bindings.Set_Two_Sided();
    BOOST_CHECK(bindings.Get_Single_Shader(0).Get_Cull_Mode()
        == Graphics::MaterialState::CULL_MODE_DISABLE);
    BOOST_CHECK(bindings.Get_Shader(0, 0).Get_Cull_Mode()
        == Graphics::MaterialState::CULL_MODE_DISABLE);

    Graphics::Apply_Mesh_Material_Fog(bindings);
    BOOST_CHECK(bindings.Get_Single_Shader(0).Get_Fog_Func()
        == Graphics::MaterialState::FOG_ENABLE);
    BOOST_CHECK(bindings.Get_Shader(0, 0).Get_Fog_Func()
        == Graphics::MaterialState::FOG_ENABLE);
}

BOOST_AUTO_TEST_CASE(ambient_conversion_uses_last_scanned_color_and_each_opacity)
{
    Bindings bindings;
    bindings.Reset(1, 2, 1);
    auto first = std::make_shared<Graphics::MeshMaterial>();
    auto second = std::make_shared<Graphics::MeshMaterial>();
    first->parameters.diffuse = {};
    first->parameters.ambient = {0.25f, 0.25f, 0.25f};
    first->parameters.opacity = 0.5f;
    second->parameters.diffuse = {};
    second->parameters.ambient = {0.75f, 0.75f, 0.75f};
    second->parameters.opacity = 0.75f;
    bindings.Set_Material(0, first);
    bindings.Set_Material(1, second);
    bindings.Set_DCG_Source(0, Graphics::PropColorSource::PrimaryColor);
    auto* colors = bindings.Get_Color_Array(0);
    colors[0] = Assets::Color_To_ARGB({1, 1, 1, 1});
    colors[1] = Assets::Color_To_ARGB({1, 1, 1, 1});

    Graphics::Prepare_Mesh_Materials(bindings);

    const auto first_color = Assets::Color_From_ARGB(colors[0]);
    const auto second_color = Assets::Color_From_ARGB(colors[1]);
    BOOST_CHECK_SMALL(first_color.r - 0.75f, 0.005f);
    BOOST_CHECK_SMALL(first_color.a - 0.5f, 0.005f);
    BOOST_CHECK_SMALL(second_color.r - 0.75f, 0.005f);
    BOOST_CHECK_SMALL(second_color.a - 0.75f, 0.005f);
}
