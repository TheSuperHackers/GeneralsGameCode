module;

#define BOOST_TEST_MODULE GeneralsAssetsW3DMeshTests

#include <boost/test/included/unit_test.hpp>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

export module Assets.Tests.W3DMeshParser;

import Assets.Adapters.W3D.Chunks;
import Assets.Adapters.W3D.Mesh;
import Assets.Materials;

namespace
{

using Byte = std::byte;

void Append_U32(std::vector<Byte> &bytes, std::uint32_t value)
{
	bytes.push_back(static_cast<Byte>(value & 0xFF));
	bytes.push_back(static_cast<Byte>((value >> 8) & 0xFF));
	bytes.push_back(static_cast<Byte>((value >> 16) & 0xFF));
	bytes.push_back(static_cast<Byte>((value >> 24) & 0xFF));
}

void Append_F32(std::vector<Byte> &bytes, float value)
{
	Append_U32(bytes, std::bit_cast<std::uint32_t>(value));
}

void Write_U32(std::vector<Byte> &bytes, std::size_t offset, std::uint32_t value)
{
	bytes[offset + 0] = static_cast<Byte>(value & 0xFF);
	bytes[offset + 1] = static_cast<Byte>((value >> 8) & 0xFF);
	bytes[offset + 2] = static_cast<Byte>((value >> 16) & 0xFF);
	bytes[offset + 3] = static_cast<Byte>((value >> 24) & 0xFF);
}

void Write_F32(std::vector<Byte> &bytes, std::size_t offset, float value)
{
	Write_U32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void Write_Fixed_String(std::vector<Byte> &bytes, std::size_t offset, std::size_t length, std::string_view value)
{
	for (std::size_t index = 0; index < length; ++index)
		bytes[offset + index] = index < value.size() ? static_cast<Byte>(value[index]) : Byte{0};
}

void Append_Vector3(std::vector<Byte> &bytes, float x, float y, float z)
{
	Append_F32(bytes, x);
	Append_F32(bytes, y);
	Append_F32(bytes, z);
}

void Append_Chunk(std::vector<Byte> &bytes, std::uint32_t id, const std::vector<Byte> &payload, bool children)
{
	Append_U32(bytes, id);
	Append_U32(bytes, static_cast<std::uint32_t>(payload.size()) | (children ? 0x80000000u : 0u));
	bytes.insert(bytes.end(), payload.begin(), payload.end());
}

std::vector<Byte> Make_Mesh(bool invalid_index)
{
	std::vector<Byte> header(116, Byte{0});
	Write_U32(header, 0, 0x00030000);
	Write_Fixed_String(header, 8, 16, "mesh");
	Write_Fixed_String(header, 24, 16, "container");
	Write_U32(header, 40, 1);
	Write_U32(header, 44, 3);
	Write_U32(header, 68, 7);
	Write_U32(header, 72, 1);
	Write_F32(header, 76, 0.0f);
	Write_F32(header, 80, 0.0f);
	Write_F32(header, 84, 0.0f);
	Write_F32(header, 88, 1.0f);
	Write_F32(header, 92, 1.0f);
	Write_F32(header, 96, 0.0f);
	Write_F32(header, 100, 0.333f);
	Write_F32(header, 104, 0.333f);
	Write_F32(header, 112, 1.0f);

	std::vector<Byte> mesh;
	Append_Chunk(mesh, Assets::W3D::W3DChunkMeshHeader3, header, false);
	std::vector<Byte> vertices;
	Append_Vector3(vertices, 0.0f, 0.0f, 0.0f);
	Append_Vector3(vertices, 1.0f, 0.0f, 0.0f);
	Append_Vector3(vertices, 0.0f, 1.0f, 0.0f);
	Append_Chunk(mesh, Assets::W3D::W3DChunkVertices, vertices, false);
	std::vector<Byte> triangles;
	Append_U32(triangles, 0);
	Append_U32(triangles, 1);
	Append_U32(triangles, invalid_index ? 4 : 2);
	Append_U32(triangles, 0);
	Append_Vector3(triangles, 0.0f, 0.0f, 1.0f);
	Append_F32(triangles, 0.0f);
	Append_Chunk(mesh, Assets::W3D::W3DChunkTriangles, triangles, false);
	return mesh;
}

}

BOOST_AUTO_TEST_CASE(optional_skin_bindings_preserve_weights_and_reject_malformed_payloads)
{
    using namespace Assets::W3D;
    for (int scenario = 0; scenario < 6; ++scenario) {
        auto bytes = Make_Mesh(false);
        Write_U32(bytes, 12, 0x20000);
        std::vector<Byte> payload;
        Append_U32(payload, scenario == 1 ? 2 : 1); Append_U32(payload, 3);
        for (int i = 0; i < 3; ++i) {
            Append_U32(payload, 2u | (5u << 16)); Append_U32(payload, 0);
            Append_F32(payload, scenario == 2 ? -0.4f : 0.4f);
            Append_F32(payload, scenario == 3 ? 0.5f : 0.6f);
            Append_F32(payload, 0); Append_F32(payload, 0);
        }
        if (scenario == 4) payload.pop_back();
        Append_Chunk(bytes, W3DChunkSkinBindings, payload, false);
        if (scenario == 5) Append_Chunk(bytes, W3DChunkSkinBindings, payload, false);
        W3DParsedMesh mesh; std::string error;
        const bool parsed = W3DParse_Mesh(bytes, mesh, error);
        BOOST_TEST(parsed == (scenario == 0));
        if (parsed) {
            BOOST_REQUIRE_EQUAL(mesh.skin_indices.size(), 3u);
            BOOST_TEST(mesh.skin_indices[0][1] == 5u);
            BOOST_TEST(mesh.skin_weights[0][0] == 0.4f);
            BOOST_TEST(mesh.skin_weights[0][1] == 0.6f);
        }
    }
}

BOOST_AUTO_TEST_CASE(legacy_influence_padding_does_not_enable_weighted_skin)
{
    using namespace Assets::W3D;
    auto bytes = Make_Mesh(false);
    std::vector<Byte> payload;
    for (int i = 0; i < 3; ++i) {
        Append_U32(payload, 2u | (5u << 16));
        Append_U32(payload, 40u | (60u << 16));
    }
    Append_Chunk(bytes, W3DChunkVertexInfluences, payload, false);
    W3DParsedMesh mesh; std::string error;
    BOOST_REQUIRE(W3DParse_Mesh(bytes, mesh, error));
    BOOST_TEST(mesh.bone_indices[0] == 2u);
    BOOST_TEST(mesh.skin_indices.empty());
    BOOST_TEST(mesh.skin_weights.empty());
}

BOOST_AUTO_TEST_CASE(mesh_parser_extracts_geometry_and_bounds)
{
	Assets::W3D::W3DParsedMesh mesh;
	std::string error;
	BOOST_REQUIRE(Assets::W3D::W3DParse_Mesh(Make_Mesh(false), mesh, error));
	BOOST_REQUIRE_EQUAL(mesh.positions.size(), 3);
	BOOST_REQUIRE_EQUAL(mesh.triangles.size(), 1);
	BOOST_CHECK(mesh.triangles[0][2] == 2);
	BOOST_CHECK(mesh.header.bounds.Is_Valid());
	BOOST_CHECK(mesh.header.name == "mesh");
}

BOOST_AUTO_TEST_CASE(mesh_parser_rejects_out_of_range_indices)
{
	Assets::W3D::W3DParsedMesh mesh;
	std::string error;
	BOOST_CHECK(!Assets::W3D::W3DParse_Mesh(Make_Mesh(true), mesh, error));
	BOOST_CHECK(!error.empty());
}

namespace
{
void Counted_String(std::vector<Byte> &bytes, std::string_view value)
{
	Append_U32(bytes, static_cast<std::uint32_t>(value.size() + 1));
	for (char c : value) bytes.push_back(static_cast<Byte>(c));
	bytes.push_back(Byte{0});
}

std::vector<Byte> Surface_Mesh(std::uint32_t material_id, bool extra_id = false, bool bad_uv = false,
	bool auxiliary_uv = false, bool auxiliary_texture = false)
{
	using namespace Assets::W3D;
	auto mesh = Make_Mesh(false);
	std::vector<Byte> header(37);
	header[0] = Byte{1};
	Write_Fixed_String(header, 1, 32, "objectsallied.fx");
	std::vector<Byte> material, table;
	Append_Chunk(material, W3DChunkShaderMaterialHeader, header, false);
	for (auto name : {"DiffuseTexture", "NormalMap", "SpecMap"}) {
		std::vector<Byte> property;
		Append_U32(property, 1); Counted_String(property, name); Counted_String(property, std::string(name) + ".dds");
		Append_Chunk(material, W3DChunkShaderMaterialProperty, property, false);
	}
	Append_Chunk(table, W3DChunkShaderMaterial, material, true);
	Append_Chunk(mesh, W3DChunkShaderMaterials, table, true);
	std::vector<Byte> pass, ids;
	Append_U32(ids, material_id);
	if (extra_id) Append_U32(ids, material_id);
	Append_Chunk(pass, W3DChunkShaderMaterialIds, ids, false);
	std::vector<Byte> stage, uvs, uv_ids;
	for (float coordinate : {0.f, 0.f, 1.f, 0.f, 0.f, 1.f, .5f, .5f}) Append_F32(uvs, coordinate);
	Append_U32(uv_ids, 3); Append_U32(uv_ids, 1); Append_U32(uv_ids, bad_uv ? 4 : 2);
	Append_Chunk(stage, W3DChunkStageTextureCoords, uvs, false);
	Append_Chunk(stage, W3DChunkPerFaceTextureCoordIds, uv_ids, false);
	Append_Chunk(pass, W3DChunkTextureStage, stage, true);
	if (auxiliary_uv) {
		if (auxiliary_texture) {
			std::vector<Byte> texture; Append_U32(texture, 0);
			Append_Chunk(stage, W3DChunkTextureIds, texture, false);
		}
		Append_Chunk(pass, W3DChunkTextureStage, stage, true);
	}
	Append_Chunk(mesh, W3DChunkMaterialPass, pass, true);
	return mesh;
}
}

BOOST_AUTO_TEST_CASE(shader_only_pass_resolves_material_and_retains_indexed_uvs)
{
	Assets::W3D::W3DParsedMesh mesh;
	std::string error;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DParse_Mesh(Surface_Mesh(0), mesh, error), error);
	BOOST_REQUIRE_EQUAL(mesh.surface_materials.size(), 1u);
	BOOST_CHECK(mesh.surface_materials[0].surface.shading_model == Assets::MaterialShadingModel::SpecularGlossiness);
	BOOST_TEST(mesh.materials.passes[0].uses_shader_material);
	BOOST_REQUIRE_EQUAL(mesh.shader_pass_bindings.size(), 1u);
	BOOST_TEST(mesh.shader_pass_bindings[0].stages[0].texcoords.size() == 4u);
	BOOST_TEST(mesh.shader_pass_bindings[0].stages[0].face_texcoord_ids[0][0] == 3u);
}

BOOST_AUTO_TEST_CASE(object_shader_retains_valid_auxiliary_uvs_without_enabling_an_extra_texture_stage)
{
	Assets::W3D::W3DParsedMesh mesh;
	std::string error;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DParse_Mesh(Surface_Mesh(0,false,false,true),mesh,error),error);
	BOOST_REQUIRE_EQUAL(mesh.shader_pass_bindings[0].stages.size(),2u);
	BOOST_TEST(mesh.shader_pass_bindings[0].stages[1].texcoords[3].x==.5f);
	BOOST_TEST(!Assets::W3D::W3DParse_Mesh(Surface_Mesh(0,false,false,true,true),mesh,error));
	BOOST_TEST(error.find("auxiliary texture binding")!=std::string::npos);
}

BOOST_AUTO_TEST_CASE(shader_pass_rejects_out_of_range_material_and_invalid_face_or_uv_counts)
{
	for (const auto &bytes : {Surface_Mesh(1), Surface_Mesh(0, true), Surface_Mesh(0, false, true)}) {
		Assets::W3D::W3DParsedMesh mesh;
		std::string error;
		BOOST_TEST(!Assets::W3D::W3DParse_Mesh(bytes, mesh, error));
		BOOST_TEST(!error.empty());
	}
}

BOOST_AUTO_TEST_CASE(mesh_tangent_arrays_preserve_authored_vectors_and_reject_corruption)
{
	using namespace Assets::W3D;
	for (int scenario = 0; scenario < 4; ++scenario) {
		auto bytes = Surface_Mesh(0);
		std::vector<Byte> tangents, bitangents;
		for (int vertex = 0; vertex < 3; ++vertex) {
			Append_Vector3(tangents, scenario == 1 ? std::numeric_limits<float>::infinity() : 1, 0, 0);
			Append_Vector3(bitangents, 0, 1, 0);
		}
		if (scenario == 2) tangents.pop_back();
		Append_Chunk(bytes, W3DChunkTangents, tangents, false);
		Append_Chunk(bytes, W3DChunkBitangents, bitangents, false);
		if (scenario == 3) Append_Chunk(bytes, W3DChunkTangents, tangents, false);
		W3DParsedMesh mesh; std::string error;
		const bool parsed = W3DParse_Mesh(bytes, mesh, error);
		BOOST_TEST(parsed == (scenario == 0));
		if (parsed) {
			BOOST_REQUIRE_EQUAL(mesh.tangents.size(), 3u);
			BOOST_TEST(mesh.tangents[0].x == 1.0f);
			BOOST_TEST(mesh.bitangents[0].y == 1.0f);
		}
	}
}
