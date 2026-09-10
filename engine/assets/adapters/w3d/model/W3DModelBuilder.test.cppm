module;

#define BOOST_TEST_MODULE GeneralsAssetsW3DModelBuilderTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <cmath>
#include <string>

export module Assets.Tests.W3DModelBuilder;

import Assets.Adapters.W3D.Mesh;
import Assets.Adapters.W3D.Model;
import Assets.Models;
import Assets.Materials;
import Assets.Adapters.W3D.Materials;
import Assets.Adapters.W3D.PassBindings;

BOOST_AUTO_TEST_CASE(model_builder_keeps_optional_skin_weights)
{
    Assets::W3D::W3DParsedMesh mesh;
    mesh.header.bounds = {{0,0,0},{1,1,0}};
    mesh.positions = {{0,0,0},{1,0,0},{0,1,0}};
    mesh.normals.assign(3,{0,0,1}); mesh.stage_texcoords.assign(3,{}); mesh.colors.assign(3,{});
    mesh.bone_indices = {1,1,1};
    mesh.skin_indices.assign(3,{1,7,0,0});
    mesh.skin_weights.assign(3,{0.4f,0.6f,0,0});
    mesh.triangles = {{0,1,2}};
    Assets::ModelAssetDesc description;
    Assets::W3D::W3DAppend_Mesh(description,mesh);
    BOOST_TEST(description.vertices[0].bone_indices[1] == 7u);
    BOOST_TEST(description.vertices[0].bone_weights[0] == 0.4f);
    BOOST_TEST(description.vertices[0].bone_weights[1] == 0.6f);
    BOOST_TEST(description.skin_bone_count == 8u);
}

BOOST_AUTO_TEST_CASE(model_builder_creates_generic_submesh_and_dependencies)
{
	Assets::W3D::W3DParsedMesh mesh;
	mesh.header.name = "body";
	mesh.header.container_name = "unit";
	mesh.header.attributes = 4;
	mesh.header.sort_level = 3;
	mesh.header.bounds = {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
	mesh.positions = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
	mesh.normals.assign(3, {0.0f, 0.0f, 1.0f});
	mesh.stage_texcoords.assign(3, {});
	mesh.colors.assign(3, {});
	mesh.triangles.push_back({0, 1, 2});
	mesh.materials.vertex_materials.push_back({{"body_material", "Body.TGA"}});
	mesh.materials.textures.push_back({"Body.TGA"});
	mesh.materials.passes.push_back({0, 0, {}});

	Assets::ModelAssetDesc description;
	Assets::W3D::W3DAppend_Mesh(description, mesh);

	BOOST_REQUIRE_EQUAL(description.vertices.size(), 3);
	BOOST_REQUIRE_EQUAL(description.indices.size(), 3);
	BOOST_REQUIRE_EQUAL(description.submeshes.size(), 1);
	BOOST_REQUIRE_EQUAL(description.materials.size(), 1);
	BOOST_CHECK(description.submeshes[0].material_index == 0);
	BOOST_CHECK(description.materials[0].primary_texture == "Body.TGA");
	BOOST_CHECK(description.bounds.Is_Valid());
	BOOST_CHECK(description.sort_level == 3);
	BOOST_REQUIRE_EQUAL(description.dependencies.size(), 2);
}

BOOST_AUTO_TEST_CASE(surface_pass_preserves_face_material_order_uv_seams_and_skin_attachment)
{
	using namespace Assets;
	using namespace Assets::W3D;
	W3DParsedMesh mesh;
	mesh.header.name = "panels";
	mesh.header.bounds = {{0, 0, 0}, {1, 1, 0}};
	mesh.positions = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
	mesh.normals.assign(3, {0, 0, 1});
	mesh.stage_texcoords.assign(3, {});
	mesh.colors.assign(3, {});
	mesh.bone_indices = {2, 3, 4};
	mesh.triangles = {{0, 1, 2}, {2, 1, 0}, {0, 2, 1}};
	W3DMaterialPass pass; pass.uses_shader_material = true;
	mesh.materials.passes.push_back(pass);
	mesh.surface_materials = {{"red", "red.dds"}, {"blue", "blue.dds"}};
	mesh.surface_materials[0].surface.shading_model = MaterialShadingModel::SpecularGlossiness;
	mesh.surface_materials[0].surface_textures[static_cast<std::size_t>(MaterialTextureRole::Normal)] = "nrm.dds";
	W3DPassBindings bindings;
	bindings.shader_material_ids = {0, 1, 0};
	bindings.stages.resize(1);
	bindings.stages[0].texcoords = {{0, 0}, {1, 0}, {0, 1}, {.5f, .25f}};
	bindings.stages[0].face_texcoord_ids = {{3, 1, 2}, {2, 1, 0}, {0, 2, 1}};
	mesh.shader_pass_bindings.push_back(bindings);
	ModelAssetDesc description;
	W3DAppend_Mesh(description, mesh);
	BOOST_REQUIRE_EQUAL(description.submeshes.size(), 3u);
	BOOST_REQUIRE_EQUAL(description.materials.size(), 2u);
	BOOST_REQUIRE_EQUAL(description.indices.size(), 9u);
	for (std::size_t face = 0; face < 3; ++face) {
		BOOST_TEST(description.submeshes[face].material_index == (face == 1 ? 1u : 0u));
		BOOST_TEST(description.submeshes[face].first_index == face * 3);
		BOOST_TEST(description.submeshes[face].index_count == 3u);
	}
	const auto &corner = description.vertices[description.indices[0]];
	BOOST_TEST(corner.texcoord.x == .5f);
	BOOST_TEST(corner.texcoord.y == .75f);
	BOOST_TEST(corner.bone_indices[0] == 2u);
	BOOST_TEST(corner.bone_weights[0] == 1.0f);
	BOOST_TEST(std::isfinite(corner.tangent.x));
	BOOST_TEST(corner.tangent.x * corner.tangent.x + corner.tangent.y * corner.tangent.y
		+ corner.tangent.z * corner.tangent.z == 1.0f, boost::test_tools::tolerance(.0001f));
	BOOST_TEST(description.vertices[description.indices[6]].texcoord.x == 0.0f);
	BOOST_TEST(description.skin_bone_count == 5u);
	bool normal_dependency = false;
	for (const auto &dependency : description.dependencies)
		if (dependency.name == "nrm.dds") normal_dependency = true;
	BOOST_TEST(normal_dependency);
}

BOOST_AUTO_TEST_CASE(authored_tangent_handedness_follows_the_w3d_v_axis_conversion)
{
	Assets::W3D::W3DParsedMesh mesh;
	mesh.positions = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
	mesh.normals.assign(3, {0, 0, 1});
	mesh.tangents.assign(3, {2, 0, 0});
	mesh.bitangents.assign(3, {0, 3, 0});
	mesh.stage_texcoords.assign(3, {});
	mesh.colors.assign(3, {});
	mesh.triangles = {{0, 1, 2}};
	Assets::ModelAssetDesc description;
	Assets::W3D::W3DAppend_Mesh(description, mesh);
	BOOST_TEST(description.vertices[0].tangent.x == 1.0f);
	BOOST_TEST(description.vertices[0].tangent_sign == -1.0f);
	mesh.bitangents.assign(3, {0, -3, 0});
	Assets::ModelAssetDesc mirrored;
	Assets::W3D::W3DAppend_Mesh(mirrored, mesh);
	BOOST_TEST(mirrored.vertices[0].tangent_sign == 1.0f);
}
