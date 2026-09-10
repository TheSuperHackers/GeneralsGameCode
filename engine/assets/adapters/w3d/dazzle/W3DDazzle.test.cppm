module;
#define BOOST_TEST_MODULE W3DDazzleTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
export module Assets.Adapters.W3D.Dazzle.Tests;
import Assets.Dazzles;
import Assets.Adapters.W3D.Dazzle;
namespace {
struct Reader {
    std::map<std::string,std::vector<std::string>> lists;
    std::map<std::string,std::string> values;
    std::vector<std::string> List(const std::string& name) const { const auto i=lists.find(name); return i==lists.end() ? std::vector<std::string>{} : i->second; }
    std::string String(const std::string& name,const std::string& key) const { const auto i=values.find(name+"/"+key); return i==values.end() ? "" : i->second; }
    float Float(const std::string& name,const std::string& key,float fallback) const { const auto s=String(name,key); return s.empty() ? fallback : std::stof(s); }
    int Integer(const std::string& name,const std::string& key,int fallback) const { const auto s=String(name,key); return s.empty() ? fallback : std::stoi(s); }
    std::array<float,3> Vector3(const std::string& name,const std::string& key,std::array<float,3> fallback) const {
        auto s=String(name,key); std::array<float,3> result;
        return std::sscanf(s.c_str(),"%f,%f,%f",&result[0],&result[1],&result[2])==3 ? result : fallback;
    }
    std::array<float,4> Vector4(const std::string& name,const std::string& key,std::array<float,4> fallback) const {
        auto s=String(name,key); std::array<float,4> result;
        return std::sscanf(s.c_str(),"%f,%f,%f,%f",&result[0],&result[1],&result[2],&result[3])==4 ? result : fallback;
    }
};
void Word(std::vector<std::byte>& bytes,std::uint32_t word) { for(unsigned i=0;i<4;++i) bytes.push_back(std::byte((word>>(8*i))&255)); }
void Chunk(std::vector<std::byte>& bytes,std::uint32_t id,const std::string& value) {
    Word(bytes,id); Word(bytes,static_cast<std::uint32_t>(value.size()+1));
    for(const auto c:value) bytes.push_back(std::byte(c)); bytes.push_back(std::byte{});
}
}
BOOST_AUTO_TEST_CASE(binary_names_follow_chunk_order_ignore_extensions_and_reject_truncation_atomically) {
    std::vector<std::byte> bytes;
    Chunk(bytes,0x901,"old"); Chunk(bytes,0x999,"extension"); Chunk(bytes,0x902,"Lamp"); Chunk(bytes,0x901,"MODEL.GLOW");
    Assets::W3D::W3DDazzleReference result;
    BOOST_REQUIRE(Assets::W3D::W3DRead_Dazzle(bytes,result));
    BOOST_CHECK_EQUAL(result.name,"MODEL.GLOW"); BOOST_CHECK_EQUAL(result.type,"Lamp");
    bytes.pop_back();
    BOOST_CHECK(!Assets::W3D::W3DRead_Dazzle(bytes,result)); BOOST_CHECK_EQUAL(result.name,"MODEL.GLOW");
    bytes.back()=std::byte{'x'}; bytes.push_back(std::byte{'x'});
    BOOST_CHECK(!Assets::W3D::W3DRead_Dazzle(bytes,result)); BOOST_CHECK_EQUAL(result.type,"Lamp");
}
BOOST_AUTO_TEST_CASE(ini_defaults_and_authored_fields_preserve_order_direction_magnitude_and_independent_uvs) {
    Reader reader;
    reader.lists={{"Lensflares_List",{"Flare"}},{"Dazzles_List",{"Lamp","Default"}}};
    reader.values={{"Flare/TextureName","atlas.tga"},{"Flare/FlareCount","2"},{"Flare/FlareLocation1","-0.5"},
        {"Flare/FlareSize1","0.125"},{"Flare/FlareColor1","0.25,0.5,0.75"},{"Flare/FlareUV1","0.1,0.2,0.3,0.4"},
        {"Lamp/DazzleDirection","0,3,4"},{"Lamp/DazzleTextureName","glare.tga"},{"Lamp/HaloTextureName","halo.tga"},
        {"Lamp/LensflareName","Flare"},{"Lamp/UseCameraTranslation","0"},{"Lamp/DazzleScaleX","0.1"},
        {"Lamp/DazzleScaleY","0.2"},{"Lamp/HaloScaleX","0.3"},{"Lamp/HaloScaleY","0.4"},
        {"Lamp/HaloIntensity","0.6"},{"Lamp/HaloIntensityPow","2"},{"Lamp/DazzleIntensity","0.8"},
        {"Lamp/DazzleIntensityPow","3"},{"Lamp/DazzleSizePow","4"},{"Lamp/DazzleArea","0.7"},
        {"Lamp/DazzleDirectionArea","0.9"},{"Lamp/FadeoutStart","12"},{"Lamp/FadeoutEnd","24"},
        {"Lamp/HistoryWeight","0.97"},{"Lamp/Radius","3"},{"Lamp/BlinkPeriod","2"},{"Lamp/BlinkOnTime","0.5"},
        {"Lamp/DazzleColor","0.1,0.2,0.3"},{"Lamp/HaloColor","0.4,0.5,0.6"}};
    Assets::DazzleDefinitions result;
    BOOST_REQUIRE(Assets::W3D::W3DRead_Dazzle_Definitions(reader,result));
    reader={};
    BOOST_REQUIRE_EQUAL(result.dazzles.size(),2); BOOST_REQUIRE_EQUAL(result.lens_flares.size(),1);
    const auto& lamp=result.dazzles[0];
    BOOST_CHECK_EQUAL(lamp.name,"Lamp"); BOOST_CHECK_EQUAL(lamp.direction[1],3); BOOST_CHECK_EQUAL(lamp.direction[2],4);
    BOOST_CHECK_EQUAL(lamp.primary_texture,"glare.tga"); BOOST_CHECK_EQUAL(lamp.halo_texture,"halo.tga"); BOOST_CHECK_EQUAL(lamp.lens_flare,"Flare");
    BOOST_CHECK(!lamp.use_camera_translation);
    BOOST_CHECK_EQUAL(lamp.scale[0],.1f); BOOST_CHECK_EQUAL(lamp.scale[1],.2f); BOOST_CHECK_EQUAL(lamp.halo_scale[0],.3f); BOOST_CHECK_EQUAL(lamp.halo_scale[1],.4f);
    BOOST_CHECK_EQUAL(lamp.halo_intensity,.6f); BOOST_CHECK_EQUAL(lamp.halo_intensity_power,2); BOOST_CHECK_EQUAL(lamp.intensity,.8f);
    BOOST_CHECK_EQUAL(lamp.intensity_power,3); BOOST_CHECK_EQUAL(lamp.size_power,4); BOOST_CHECK_EQUAL(lamp.area,.7f); BOOST_CHECK_EQUAL(lamp.direction_area,.9f);
    BOOST_CHECK_EQUAL(lamp.fade_start,12); BOOST_CHECK_EQUAL(lamp.fade_end,24); BOOST_CHECK_EQUAL(lamp.history_weight,.97f);
    BOOST_CHECK_EQUAL(lamp.radius,3); BOOST_CHECK_EQUAL(lamp.blink_period,2); BOOST_CHECK_EQUAL(lamp.blink_on_time,.5f);
    BOOST_CHECK_EQUAL(lamp.color[2],.3f); BOOST_CHECK_EQUAL(lamp.halo_color[0],.4f);
    const auto& defaults=result.dazzles[1]; BOOST_CHECK_EQUAL(defaults.name,"Default");
    BOOST_CHECK_EQUAL(defaults.halo_intensity,.95f); BOOST_CHECK_EQUAL(defaults.scale[0],100); BOOST_CHECK_EQUAL(defaults.scale[1],25);
    BOOST_CHECK_EQUAL(defaults.direction_area,.5f); BOOST_CHECK_EQUAL(defaults.fade_start,25); BOOST_CHECK_EQUAL(defaults.fade_end,50);
    BOOST_CHECK(defaults.use_camera_translation); BOOST_CHECK_EQUAL(defaults.direction[0],0);
    const auto& sprites=result.lens_flares.front().sprites; BOOST_REQUIRE_EQUAL(sprites.size(),2);
    BOOST_CHECK_EQUAL(sprites[0].location,-.5f); BOOST_CHECK_EQUAL(sprites[0].size,.125f);
    BOOST_CHECK_EQUAL(sprites[0].color[2],.75f); BOOST_CHECK_EQUAL(sprites[0].uv[3],.4f);
    BOOST_CHECK_EQUAL(sprites[1].size,1); BOOST_CHECK_EQUAL(sprites[1].uv[2],1); BOOST_CHECK_EQUAL(sprites[1].color[0],1);
}
BOOST_AUTO_TEST_CASE(invalid_definition_does_not_replace_the_published_catalog) {
    Reader reader; reader.lists={{"Lensflares_List",{"Bad"}}}; reader.values={{"Bad/FlareCount","-1"}};
    Assets::DazzleDefinitions result; result.dazzles.emplace_back().name="Retained";
    BOOST_CHECK(!Assets::W3D::W3DRead_Dazzle_Definitions(reader,result));
    BOOST_CHECK_EQUAL(result.dazzles.front().name,"Retained");
    reader={}; reader.lists={{"Dazzles_List",{"Bad"}}}; reader.values={{"Bad/DazzleArea","0"}};
    BOOST_CHECK(!Assets::W3D::W3DRead_Dazzle_Definitions(reader,result));
    BOOST_CHECK_EQUAL(result.dazzles.front().name,"Retained");
    reader.values={{"Bad/HaloIntensity","nan"}};
    BOOST_CHECK(!Assets::W3D::W3DRead_Dazzle_Definitions(reader,result));
    BOOST_CHECK_EQUAL(result.dazzles.front().name,"Retained");
}
