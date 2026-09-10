module;
#define BOOST_TEST_MODULE W3DMeshDataTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
export module Assets.Adapters.W3D.MeshData.Tests;
import Assets.Adapters.W3D.MeshData;
import Assets.Adapters.W3D.Materials;
import Assets.Adapters.W3D.Geometry;
import Assets.Math;
using namespace Assets::W3D;
namespace {
using Bytes = std::vector<std::byte>;
void Word(Bytes& bytes, std::uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) bytes.push_back(std::byte(value >> (i * 8)));
}
void Put(Bytes& bytes, std::size_t offset, std::uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) bytes[offset + i] = std::byte(value >> (i * 8));
}
void Chunk(Bytes& bytes, std::uint32_t id, const Bytes& payload, bool nested = false) {
    Word(bytes, id); Word(bytes, static_cast<std::uint32_t>(payload.size()) | (nested ? 0x80000000u : 0u));
    bytes.insert(bytes.end(), payload.begin(), payload.end());
}
Bytes Geometry(std::uint32_t vertices = 3, std::uint32_t flags = 0, std::uint32_t version = 0x40001) {
    Bytes bytes, header(116);
    Put(header, 0, version); Put(header, 4, flags); header[8] = std::byte{'M'};
    Put(header, 40, 1); Put(header, 44, vertices);
    Chunk(bytes, 0x1f, header);
    Bytes positions(vertices * 12);
    Put(positions, 12, std::bit_cast<std::uint32_t>(1.f));
    Put(positions, 28, std::bit_cast<std::uint32_t>(1.f));
    Chunk(bytes, 2, positions);
    Bytes triangle; for (unsigned value : {0u, 1u, 2u, 17u, 0u, 0u, 0x3f800000u, 0u}) Word(triangle, value);
    Chunk(bytes, 0x20, triangle);
    return bytes;
}
}
BOOST_AUTO_TEST_CASE(ordered_prelighting_text_and_old_bone_indices_survive_source_release) {
    auto bytes = Geometry(3, 0x0f000000, 0x20000);
    Chunk(bytes, 0xc, Bytes{std::byte{'A'}, std::byte{0}});
    Chunk(bytes, 0xc, Bytes{std::byte{'B'}, std::byte{0}});
    Bytes bones(24); bones[0] = std::byte{2}; bones[8] = std::byte{3}; bones[16] = std::byte{4};
    Chunk(bytes, 0xe, bones);
    for (unsigned id : {0x23u, 0x24u, 0x25u, 0x26u}) {
        Bytes info(16); Put(info, 0, id - 0x22);
        Bytes wrapper; Chunk(wrapper, 0x28, info); Chunk(bytes, id, wrapper, true);
    }
    W3DMeshData mesh; std::string error;
    BOOST_REQUIRE(W3DRead_Mesh_Data(bytes, W3DMeshPrelighting::Vertex, mesh, error));
    BOOST_CHECK_EQUAL(mesh.prelit_chunk, 0x24);
    BOOST_REQUIRE_EQUAL(mesh.materials.size(), 1);
    BOOST_CHECK_EQUAL(std::get<W3DMaterialInfo>(mesh.materials[0]).pass_count, 2);
    BOOST_REQUIRE(mesh.bone_indices); BOOST_CHECK_EQUAL((*mesh.bone_indices)[2], 5);
    bytes.clear();
    BOOST_REQUIRE(mesh.user_text); BOOST_CHECK_EQUAL(*mesh.user_text, std::string("A\0", 2));
    BOOST_CHECK_EQUAL(mesh.triangles[0].surface_type, 17);
    BOOST_CHECK_EQUAL(mesh.positions[2].y, 1.f);
}
BOOST_AUTO_TEST_CASE(format_indices_are_not_limited_by_a_callers_vertex_storage) {
    const auto bytes = Geometry(65537);
    W3DMeshData mesh; std::string error;
    BOOST_REQUIRE(W3DRead_Mesh_Data(bytes, W3DMeshPrelighting::Unlit, mesh, error));
    BOOST_CHECK_EQUAL(mesh.positions.size(), 65537);
}
BOOST_AUTO_TEST_CASE(malformed_and_out_of_range_geometry_do_not_replace_a_published_mesh) {
    const auto bytes = Geometry(); W3DMeshData mesh; std::string error;
    BOOST_REQUIRE(W3DRead_Mesh_Data(bytes, W3DMeshPrelighting::Unlit, mesh, error));
    for (unsigned failure = 0; failure < 4; ++failure) {
        auto invalid = bytes;
        if (failure == 0) invalid.pop_back();
        if (failure == 1) Put(invalid, 176, 3); // First triangle index, beyond three vertices.
        if (failure == 2) Put(invalid, 188, 256); // Surface type cannot be represented by the authored table.
        if (failure == 3) Put(invalid, 52, 4); // Header vertex count disagrees with its vector chunk.
        BOOST_CHECK(!W3DRead_Mesh_Data(invalid, W3DMeshPrelighting::Unlit, mesh, error));
        BOOST_CHECK_EQUAL(mesh.positions.size(), 3); BOOST_CHECK_EQUAL(mesh.triangles[0].indices[0], 0);
    }
}
