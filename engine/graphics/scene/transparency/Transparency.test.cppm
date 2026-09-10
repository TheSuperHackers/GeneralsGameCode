module;

#define BOOST_TEST_MODULE GraphicsTransparencyTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <type_traits>

export module Graphics.Scene.Transparency.Tests;

import Graphics.Scene.Transparency;

using namespace Graphics;

static Mesh Make_Mesh() noexcept
{
	Mesh mesh;
	mesh.vertex_count = 3;
	mesh.index_count = 3;
	mesh.vertex_stride = 32;
	mesh.index_format = MeshIndexFormat::UInt32;
	return mesh;
}

static RenderInstance Make_Instance(MeshHandle mesh, MaterialHandle material, float depth) noexcept
{
	RenderInstance instance;
	instance.transform.matrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, -depth,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	instance.bounds.radius = 0.25f;
	instance.mesh = mesh;
	instance.material = material;
	return instance;
}

static View Make_View() noexcept
{
	Matrix4x4 projection{};
	projection.values[0] = 1.0f;
	projection.values[5] = 1.0f;
	projection.values[10] = -101.0f / 99.0f;
	projection.values[11] = -200.0f / 99.0f;
	projection.values[14] = -1.0f;
	return {Matrix4x4::Identity(), projection, {}, {0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f}};
}

static_assert(std::is_nothrow_move_constructible_v<TransparentDrawSet>);

BOOST_AUTO_TEST_CASE(transparent_material_classification_is_explicit)
{
	Material opaque;
	Material transparent;
	transparent.flags = MaterialFlags::Transparent;

	BOOST_CHECK(!Is_Transparent_Material(opaque));
	BOOST_CHECK(Is_Transparent_Material(transparent));
	BOOST_CHECK(Is_Transparent_Material(MaterialFlags::Transparent | MaterialFlags::DoubleSided));
}

BOOST_AUTO_TEST_CASE(transparent_draws_are_filtered_and_sorted_back_to_front)
{
	MeshPool meshes;
	MaterialPool materials;
	const MeshHandle mesh = meshes.Create(Make_Mesh());
	const MaterialHandle transparent_material = materials.Create();
	const MaterialHandle opaque_material = materials.Create();
	materials.Resolve(transparent_material)->flags = MaterialFlags::Transparent;

	RenderScene scene;
	const InstanceHandle near_instance = scene.Create(Make_Instance(mesh, transparent_material, 5.0f));
	scene.Create(Make_Instance(mesh, opaque_material, 7.0f));
	const InstanceHandle far_instance = scene.Create(Make_Instance(mesh, transparent_material, 20.0f));

	std::array<InstanceHandle, 3> visible_storage{};
	std::array<LODSelection, 3> lod_storage{};
	VisibleSet visible_set(visible_storage);
	LODSet lod_set(lod_storage);
	const View view = Make_View();
	BOOST_REQUIRE(Build_Visible_Set(scene, view, visible_set));
	BOOST_REQUIRE(Build_LOD_Set(scene, meshes, visible_set, view, lod_set));

	TexturePool textures;
	SamplerPool samplers;
	GPUScene gpu_scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));

	std::array<DrawData, 3> draw_storage{};
	TransparentDrawSet draw_set(draw_storage);
	const PipelineHandle pipeline(7, 1);
	BOOST_REQUIRE(Build_Transparent_Draw_Data(scene, materials, lod_set, view, gpu_scene, {0, pipeline, 0}, draw_set));
	BOOST_REQUIRE(draw_set.Size() == 2);
	BOOST_CHECK(draw_set.Records()[0].instance_index == gpu_scene.Instance_Index(far_instance));
	BOOST_CHECK(draw_set.Records()[1].instance_index == gpu_scene.Instance_Index(near_instance));
	BOOST_CHECK(draw_set.Records()[0].sort_key < draw_set.Records()[1].sort_key);
}

BOOST_AUTO_TEST_CASE(transparent_pipeline_state_is_depth_tested_blended_and_write_disabled)
{
	PipelineDesc base;
	base.vertex_shader = 4;
	base.fragment_shader = 5;
	const PipelineDesc transparent = Make_Transparent_Pipeline(base);

	BOOST_CHECK(transparent.depth_test);
	BOOST_CHECK(!transparent.depth_write);
	BOOST_CHECK(transparent.blend_mode == RHIBlendMode::Alpha);
	BOOST_CHECK(base.Key() != transparent.Key());
}
