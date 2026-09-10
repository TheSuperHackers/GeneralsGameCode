module;
#define BOOST_TEST_MODULE MeshTextureMappingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstdint>
export module Graphics.Materials.MeshTextureMapping.Tests;
import Assets.Math;
import Assets.Materials.TextureMapping;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.TextureMapping;
import Graphics.Materials.MeshTextureMapping;
import Graphics.Scene.Props.Renderer;
BOOST_AUTO_TEST_CASE(stage_zero_override_restores_scroll_state_while_other_stages_advance) {
    Graphics::MeshMaterial material;
    const Assets::TextureScrollMapping scroll{{1,1},{.5f,-.25f},{.75f,.25f}};
    material.mappings[0] = Graphics::TextureMapping::Create(scroll, 100);
    material.mappings[1] = Graphics::TextureMapping::Create(scroll, 100);
    Graphics::PropParameters parameters;
    constexpr std::array<float,16> identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    Graphics::Extract_Mesh_Texture_Mappings(parameters,&material,1100,identity,identity,std::array<float,2>{.125f,.875f});
    BOOST_CHECK_EQUAL(parameters.uv_transform[0][3], .125f);
    BOOST_CHECK_EQUAL(parameters.uv_transform[0][7], .875f);
    BOOST_CHECK_EQUAL(material.mappings[0]->Linear_Scroll()->offset.x, .75f);
    BOOST_CHECK_EQUAL(material.mappings[0]->Linear_Scroll()->last_time, 100);
    BOOST_CHECK_SMALL(parameters.uv_transform[1][3]-.25f, 1e-6f);
    BOOST_CHECK_SMALL(parameters.uv_transform[1][7]-.5f, 1e-6f);
    Graphics::Extract_Mesh_Texture_Mappings(parameters,&material,1100,identity,identity);
    BOOST_CHECK_SMALL(parameters.uv_transform[0][3]-.25f, 1e-6f);
    BOOST_CHECK_SMALL(parameters.uv_transform[0][7]-.5f, 1e-6f);
    BOOST_CHECK_EQUAL(material.mappings[0]->Linear_Scroll()->last_time, 1100);
}
