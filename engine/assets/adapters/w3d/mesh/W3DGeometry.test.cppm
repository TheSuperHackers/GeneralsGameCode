module;
#define BOOST_TEST_MODULE W3DGeometryTests
#include <boost/test/included/unit_test.hpp>
#include <bit>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <vector>
export module Assets.Adapters.W3D.Geometry.Tests;
import Assets.Adapters.W3D.Geometry;
import Assets.Adapters.W3D.Chunks;
import Assets.Math;
import Assets.MeshBoundsTree;
using namespace Assets;
using namespace Assets::W3D;
using Bytes=std::vector<std::byte>;
void U32(Bytes& bytes,std::uint32_t value) { for(unsigned i=0;i<4;++i)bytes.push_back(std::byte(value>>(i*8))); }
void F32(Bytes& bytes,float value) { U32(bytes,std::bit_cast<std::uint32_t>(value)); }

BOOST_AUTO_TEST_CASE(authored_tree_order_bounds_and_invalid_references)
{
    Bytes bytes;
    const auto chunk=[&](unsigned id,const Bytes& payload) {
        U32(bytes,id);U32(bytes,static_cast<unsigned>(payload.size()));
        bytes.insert(bytes.end(),payload.begin(),payload.end());
    };
    Bytes header;U32(header,3);U32(header,2);header.resize(32);chunk(0x91,header);
    Bytes polygons;U32(polygons,1);U32(polygons,0);chunk(0x92,polygons);
    Bytes nodes;
    for(unsigned i=0;i<3;++i) {
        for(float value:{-1.f,-1.f,-0.f,1.f,1.f,.5f})F32(nodes,value);
        U32(nodes,i==0 ? 2u : 0x80000000u+(i-1));U32(nodes,1);
    }
    chunk(0x93,nodes);
    MeshBoundsTree tree;
    BOOST_REQUIRE(W3DRead_Mesh_Bounds_Tree(bytes,2,tree));
    BOOST_REQUIRE_EQUAL(tree.nodes.size(),3);
    BOOST_CHECK_EQUAL(tree.nodes[0].first,2);BOOST_CHECK_EQUAL(tree.nodes[0].second,1);
    BOOST_CHECK_EQUAL(tree.polygon_indices[0],1);BOOST_CHECK_EQUAL(tree.polygon_indices[1],0);
    BOOST_CHECK_EQUAL(tree.nodes[2].first,1);BOOST_CHECK_EQUAL(tree.nodes[2].second,1);
    BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(tree.nodes[0].bounds.minimum.z),0x80000000u);
    for(std::size_t size=0;size<bytes.size();++size) {
        BOOST_CHECK(!W3DRead_Mesh_Bounds_Tree(W3DByteSpan(bytes).first(size),2,tree));
        BOOST_REQUIRE_EQUAL(tree.nodes.size(),3);BOOST_CHECK_EQUAL(tree.nodes[0].first,2);
    }
    const auto invalid=[&](std::size_t offset,unsigned value) {
        auto bad=bytes;for(unsigned i=0;i<4;++i)bad[offset+i]=std::byte(value>>(i*8));
        BOOST_CHECK(!W3DRead_Mesh_Bounds_Tree(bad,2,tree));
        BOOST_CHECK_EQUAL(tree.nodes[0].first,2);
    };
    invalid(88,0); // Root points back to itself.
    invalid(88,3); // Child outside the node array.
    invalid(120,0x80000002u); // Leaf range exceeds polygon table.
    invalid(48,2); // Polygon index exceeds mesh triangle count.
    invalid(64,0x7f800000u); // Nonfinite bounds.
    invalid(8,0xffffffffu); // Count rejected before allocation.
    bytes.clear();BOOST_CHECK_EQUAL(tree.nodes[2].bounds.maximum.z,.5f);
}

BOOST_AUTO_TEST_CASE(mesh_header_retains_authored_bounds_names_flags_and_version)
{
    Bytes bytes(116);
    const auto word=[&](std::size_t offset,std::uint32_t value) {
        for(unsigned i=0;i<4;++i)bytes[offset+i]=std::byte(value>>(i*8));
    };
    const auto scalar=[&](std::size_t offset,float value) { word(offset,std::bit_cast<std::uint32_t>(value)); };
    word(0,0x40001);word(4,0x12345678);word(40,17);word(44,29);word(48,3);
    word(56,std::bit_cast<std::uint32_t>(std::int32_t{-7}));word(68,0xabcdef);word(72,0x123);
    for(unsigned i=0;i<16;++i) { bytes[8+i]=std::byte('A'+i);bytes[24+i]=std::byte('a'+i); }
    for(unsigned i=0;i<3;++i) { scalar(76+i*4,-1.0f);scalar(88+i*4,2.0f); }
    scalar(100,9);scalar(104,-3);scalar(108,2);scalar(112,4);
    W3DMeshHeader header;
    BOOST_REQUIRE(W3DRead_Mesh_Header(bytes,header));
    BOOST_CHECK_EQUAL(header.version,0x40001u);BOOST_CHECK_EQUAL(header.attributes,0x12345678u);
    BOOST_CHECK_EQUAL(header.name,"ABCDEFGHIJKLMNOP");BOOST_CHECK_EQUAL(header.container_name,"abcdefghijklmnop");
    BOOST_CHECK_EQUAL(header.triangle_count,17);BOOST_CHECK_EQUAL(header.vertex_count,29);
    BOOST_CHECK_EQUAL(header.material_count,3);BOOST_CHECK_EQUAL(header.sort_level,-7);
    BOOST_CHECK_EQUAL(header.vertex_channels,0xabcdefu);BOOST_CHECK_EQUAL(header.face_channels,0x123u);
    BOOST_CHECK_EQUAL(header.bounds.minimum.x,-1);BOOST_CHECK_EQUAL(header.bounds.maximum.z,2);
    BOOST_CHECK_EQUAL(header.sphere_center.x,9);BOOST_CHECK_EQUAL(header.sphere_center.y,-3);
    BOOST_CHECK_EQUAL(header.sphere_center.z,2);BOOST_CHECK_EQUAL(header.sphere_radius,4);
    for(std::size_t size=0;size<116;++size) {
        BOOST_CHECK(!W3DRead_Mesh_Header(W3DByteSpan(bytes).first(size),header));
        BOOST_CHECK_EQUAL(header.name,"ABCDEFGHIJKLMNOP");
    }
    scalar(112,-1);BOOST_CHECK(!W3DRead_Mesh_Header(bytes,header));
    BOOST_CHECK_EQUAL(header.sphere_radius,4);
    bytes.clear();BOOST_CHECK_EQUAL(header.container_name,"abcdefghijklmnop");
}

BOOST_AUTO_TEST_CASE(triangle_fields_and_float_bits_survive_source_release)
{
    Bytes bytes;
    for(auto index:{69999u,2u,1u,255u})U32(bytes,index);
    for(float value:{.25f,-.5f,-0.0f,17.0f})F32(bytes,value);
    std::vector<W3DTriangleRecord> values;
    BOOST_REQUIRE(W3DRead_Geometry_Triangles(bytes,1,values));
    for(std::size_t size=0;size<bytes.size();++size) {
        BOOST_CHECK(!W3DRead_Geometry_Triangles(W3DByteSpan(bytes).first(size),1,values));
        BOOST_REQUIRE_EQUAL(values.size(),1);
        BOOST_CHECK_EQUAL(values[0].indices[0],69999);
    }
    bytes.clear();
    BOOST_CHECK_EQUAL(values[0].indices[2],1);
    BOOST_CHECK_EQUAL(values[0].surface_type,255);
    BOOST_CHECK_EQUAL(values[0].normal.x,.25f);
    BOOST_CHECK_EQUAL(values[0].normal.y,-.5f);
    BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(values[0].normal.z),0x80000000u);
    BOOST_CHECK_EQUAL(values[0].distance,17);
}

BOOST_AUTO_TEST_CASE(vector_and_index_decoding_is_atomic_and_ignores_influence_padding)
{
    Bytes bytes;for(float value:{-0.0f,2.25f,-7.0f})F32(bytes,value);
    std::vector<Vector3f> vectors;
    BOOST_REQUIRE(W3DRead_Geometry_Vectors(bytes,1,vectors));
    BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(vectors[0].x),0x80000000u);
    BOOST_CHECK_EQUAL(vectors[0].y,2.25f);
    BOOST_CHECK(!W3DRead_Geometry_Vectors(bytes,2,vectors));
    BOOST_CHECK(!W3DRead_Geometry_Vectors(bytes,(std::numeric_limits<std::size_t>::max)(),vectors));
    BOOST_CHECK_EQUAL(vectors[0].z,-7);
    bytes.clear();U32(bytes,0xffff1234);U32(bytes,0xffffffff);
    std::vector<std::uint16_t> bones;
    BOOST_REQUIRE(W3DRead_Geometry_Bone_Links(bytes,1,bones));
    BOOST_CHECK_EQUAL(bones[0],0x1234);
    std::vector<std::uint32_t> shades;
    BOOST_REQUIRE(W3DRead_Geometry_Shade_Indices(bytes,2,shades));
    BOOST_CHECK_EQUAL(shades[0],0xffff1234u);BOOST_CHECK_EQUAL(shades[1],0xffffffffu);
    bytes.pop_back();
    BOOST_CHECK(!W3DRead_Geometry_Bone_Links(bytes,1,bones));
    BOOST_CHECK(!W3DRead_Geometry_Shade_Indices(bytes,2,shades));
    BOOST_CHECK_EQUAL(shades[1],0xffffffffu);
}

BOOST_AUTO_TEST_CASE(retail_mesh_geometry_uses_the_runtime_readers)
{
    const auto* directory=std::getenv("GENERALS_W3D_ANIMATION_TEST_DIRECTORY");
    if(!directory) { BOOST_TEST_MESSAGE("Retail geometry corpus not configured");return; }
    unsigned meshes=0,arrays=0,trees=0;
    for(const auto& entry:std::filesystem::directory_iterator(directory)) {
        const auto extension=entry.path().extension().string();
        if(!entry.is_regular_file() || (extension!=".w3d" && extension!=".W3D"))continue;
        std::ifstream input(entry.path(),std::ios::binary|std::ios::ate);
        const auto size=input.tellg();BOOST_REQUIRE(size>=0);input.seekg(0);
        Bytes bytes(static_cast<std::size_t>(size));input.read(reinterpret_cast<char*>(bytes.data()),size);
        BOOST_REQUIRE(input.good());
        const bool valid=W3DVisit_Chunks(bytes,[&](const W3DChunkView& top) {
            if(top.id!=W3DChunkMesh)return true;
            std::uint32_t vertices=0,triangles=0;bool header=false;
            BOOST_REQUIRE(W3DVisit_Chunks(top.payload,[&](const W3DChunkView& chunk) {
                if(chunk.id!=W3DChunkMeshHeader3)return true;
                W3DMeshHeader decoded;
                header=W3DRead_Mesh_Header(chunk.payload,decoded);
                triangles=decoded.triangle_count;vertices=decoded.vertex_count;
                return header;
            }));
            BOOST_REQUIRE(header);++meshes;
            return W3DVisit_Chunks(top.payload,[&](const W3DChunkView& chunk) {
                bool decoded=true;
                switch(chunk.id) {
                    case 0x90: {
                        MeshBoundsTree tree;
                        decoded=W3DRead_Mesh_Bounds_Tree(chunk.payload,triangles,tree);++trees;break;
                    }
                    case W3DChunkVertices:case W3DChunkVertexNormals: {
                        std::vector<Vector3f> values;decoded=W3DRead_Geometry_Vectors(chunk.payload,vertices,values);break;
                    }
                    case W3DChunkTriangles: {
                        std::vector<W3DTriangleRecord> values;decoded=W3DRead_Geometry_Triangles(chunk.payload,triangles,values);break;
                    }
                    case W3DChunkVertexInfluences: {
                        std::vector<std::uint16_t> values;decoded=W3DRead_Geometry_Bone_Links(chunk.payload,vertices,values);break;
                    }
                    case W3DChunkVertexShadeIndices: {
                        std::vector<std::uint32_t> values;decoded=W3DRead_Geometry_Shade_Indices(chunk.payload,vertices,values);break;
                    }
                    default:return true;
                }
                BOOST_REQUIRE_MESSAGE(decoded,entry.path().string()+" chunk "+std::to_string(chunk.id));
                ++arrays;return decoded;
            });
        });
        if(!valid) {
            const auto name=entry.path().filename();
            BOOST_REQUIRE_MESSAGE(name=="UISabotr_idel.w3d" || name=="UISabotr_Jump.w3d"
                || name=="UISabotr_Left.w3d" || name=="UISabotr_Right.w3d" || name=="UISabotr_Up.w3d",entry.path().string());
        }
    }
    BOOST_CHECK(meshes>0);BOOST_CHECK(arrays>=meshes);
    BOOST_TEST_MESSAGE("Decoded meshes="<<meshes<<", geometry arrays="<<arrays<<", bounds trees="<<trees);
}
