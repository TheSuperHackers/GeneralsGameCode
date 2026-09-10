module;
#define BOOST_TEST_MODULE W3DAssemblyTests
#include <boost/test/included/unit_test.hpp>
#include "W3DAssembly.test-data.h"
#include <string>
#include <span>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>
export module Assets.Adapters.W3D.Assembly.Tests;
import Assets.Adapters.W3D.Assembly;
import Assets.Adapters.W3D.Collection;
import Assets.Adapters.W3D.LevelSet;
import Assets.Adapters.W3D.Aggregate;
import Assets.Adapters.W3D.Chunks;
using namespace Assets;
using namespace AssemblyTestData;
BOOST_AUTO_TEST_CASE(decodes_levels_aggregates_and_proxies_without_flattening_their_roles) {
    ModelAssemblyDesc description;std::string error;
    BOOST_REQUIRE_MESSAGE(W3D::W3DRead_Model_Assembly(Hlod(),true,description,error),error);
    BOOST_CHECK_EQUAL(description.name,"MODEL");BOOST_CHECK_EQUAL(description.skeleton_name,"RIG");
    BOOST_REQUIRE_EQUAL(description.levels.size(),2);
    BOOST_REQUIRE_EQUAL(description.levels[0].children.size(),2);
    BOOST_CHECK_EQUAL(description.levels[0].children[0].bone,1);
    BOOST_CHECK_EQUAL(description.levels[1].children[0].object_name,"MODEL.Low");
    BOOST_CHECK_EQUAL(description.levels[1].maximum_screen_size,200);
    BOOST_REQUIRE_EQUAL(description.aggregates.size(),1);BOOST_CHECK_EQUAL(description.aggregates[0].bone,0);
    BOOST_REQUIRE_EQUAL(description.proxies.size(),1);BOOST_CHECK_EQUAL(description.proxies[0].object_name,"SOCKET");
    BOOST_CHECK_EQUAL(description.proxies[0].bone,2);
}
BOOST_AUTO_TEST_CASE(hierarchy_models_preserve_all_connection_kinds_old_root_mapping_and_points) {
    for(bool old:{false,true}) {
        ModelAssemblyDesc description;std::string error;
        BOOST_REQUIRE_MESSAGE(W3D::W3DRead_Model_Assembly(Hmodel(old),false,description,error),error);
        BOOST_REQUIRE_EQUAL(description.levels.size(),1);BOOST_REQUIRE_EQUAL(description.levels[0].children.size(),3);
        BOOST_CHECK_EQUAL(description.levels[0].children[0].bone,1);
        BOOST_CHECK_EQUAL(description.levels[0].children[1].bone,2);
        BOOST_CHECK_EQUAL(description.levels[0].children[2].bone,0);
        BOOST_CHECK_EQUAL(description.levels[0].children[0].object_name,"MODEL.Left");
        BOOST_REQUIRE_EQUAL(description.snap_points.size(),1);BOOST_CHECK_EQUAL(description.snap_points[0].z,3);
    }
}
BOOST_AUTO_TEST_CASE(rejected_truncation_and_counts_do_not_replace_the_previous_description) {
    for(bool lod:{false,true}) {
        const auto bytes=lod?Hlod():Hmodel(false);std::string error;
        ModelAssemblyDesc description;description.name="UNCHANGED";
        for(std::size_t length=0;length<bytes.size();++length) {
            if((lod&&(length==228||length==296))||(!lod&&length==126))continue;
            BOOST_TEST_CONTEXT("lod="<<lod<<" length="<<length) {
                BOOST_CHECK(!W3D::W3DRead_Model_Assembly(std::span(bytes).first(length),lod,description,error));
                BOOST_CHECK_EQUAL(description.name,"UNCHANGED");
            }
        }
        auto bad=bytes;bad[lod?12:44]=std::byte(4);
        BOOST_CHECK(!W3D::W3DRead_Model_Assembly(bad,lod,description,error));
        BOOST_CHECK_EQUAL(description.name,"UNCHANGED");
    }
}

BOOST_AUTO_TEST_CASE(optional_retail_model_assembly_corpus) {
    const auto* directory=std::getenv("GENERALS_W3D_ASSEMBLY_TEST_DIRECTORY");
    if(!directory || !*directory)return;
    unsigned hlods=0,hmodels=0,collections=0,level_sets=0,aggregate_models=0,children=0,proxies=0,aggregates=0;
    for(const auto& entry:std::filesystem::directory_iterator(directory)) {
        if(!entry.is_regular_file())continue;
        auto extension=entry.path().extension().string();
        std::transform(extension.begin(),extension.end(),extension.begin(),
            [](unsigned char value){return static_cast<char>(std::tolower(value));});
        if(extension!=".w3d")continue;
        std::ifstream input(entry.path(),std::ios::binary|std::ios::ate);
        const auto size=input.tellg();BOOST_REQUIRE(size>=0);
        Bytes bytes(static_cast<std::size_t>(size));input.seekg(0);
        BOOST_REQUIRE(input.read(reinterpret_cast<char*>(bytes.data()),size));
        const bool valid=W3D::W3DVisit_Chunks(bytes,[&](const W3D::W3DChunkView& chunk) {
            if(chunk.id==0x600) {
                W3D::W3DAggregateDescription description;std::string error;
                BOOST_REQUIRE_MESSAGE(W3D::W3DRead_Model_Aggregate(chunk.payload,description,error),entry.path().string()+": "+error);
                ++aggregate_models;
                return true;
            }
            if(chunk.id==0x400) {
                ModelLevelSetDesc description;std::string error;
                BOOST_REQUIRE_MESSAGE(W3D::W3DRead_Model_Level_Set(chunk.payload,description,error),entry.path().string()+": "+error);
                ++level_sets;
                return true;
            }
            if(chunk.id==0x420) {
                ModelCollectionDesc description;std::string error;
                BOOST_REQUIRE_MESSAGE(W3D::W3DRead_Model_Collection(chunk.payload,description,error),entry.path().string()+": "+error);
                ++collections;
                return true;
            }
            if(chunk.id!=0x300 && chunk.id!=0x700)return true;
            ModelAssemblyDesc description;std::string error;
            BOOST_REQUIRE_MESSAGE(W3D::W3DRead_Model_Assembly(chunk.payload,chunk.id==0x700,description,error),entry.path().string()+": "+error);
            if(chunk.id==0x700)++hlods;else ++hmodels;
            for(const auto& level:description.levels)children+=static_cast<unsigned>(level.children.size());
            proxies+=static_cast<unsigned>(description.proxies.size());aggregates+=static_cast<unsigned>(description.aggregates.size());
            return true;
        });
        if(!valid) {
            const auto name=entry.path().filename().string();
            BOOST_REQUIRE_MESSAGE(name=="UISabotr_idel.w3d" || name=="UISabotr_Jump.w3d" || name=="UISabotr_Left.w3d"
                || name=="UISabotr_Right.w3d" || name=="UISabotr_Up.w3d",entry.path().string()+": invalid container");
        }
    }
    BOOST_CHECK(hlods+hmodels>0);BOOST_CHECK(children>0);
    BOOST_TEST_MESSAGE("Assemblies HLOD="<<hlods<<" HMODEL="<<hmodels<<" collections="<<collections<<" level_sets="<<level_sets<<" aggregate_models="<<aggregate_models<<" children="<<children<<" proxies="<<proxies<<" aggregates="<<aggregates);
}
