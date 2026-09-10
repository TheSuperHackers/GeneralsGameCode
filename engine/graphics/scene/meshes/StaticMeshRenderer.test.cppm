module;

#define BOOST_TEST_MODULE GraphicsStaticMeshRendererTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#if defined(_WIN32)
#include <filesystem>
#endif

export module Graphics.Scene.StaticMeshes.Tests;

import Graphics.Scene.StaticMeshes;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(static_mesh_source_contract)
{
	const std::array<StaticMeshVertex, 3> vertices = {{
		{{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.5f, 0.0f}},
		{{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	}};
	const std::array<std::uint16_t, 3> indices = {0, 1, 2};
	const StaticMeshSource source{
		3,
		3,
		static_cast<std::uint32_t>(sizeof(StaticMeshVertex)),
		MeshIndexFormat::UInt16,
		std::as_bytes(std::span<const StaticMeshVertex>(vertices)),
		std::as_bytes(std::span<const std::uint16_t>(indices)),
		{0.0f, 0.0f, 0.0f},
		1.0f
	};

	BOOST_TEST(Validate_Static_Mesh_Source(source));
	BOOST_TEST(source.vertex_count == 3u);
	BOOST_TEST(source.index_count == 3u);
	BOOST_TEST(MeshHandle(0, 1).Is_Valid());
}

BOOST_AUTO_TEST_CASE(static_mesh_source_rejects_incompatible_layout)
{
	StaticMeshSource source;
	source.vertex_count = 3;
	source.index_count = 3;
	source.vertex_stride = 16;
	source.index_format = MeshIndexFormat::UInt16;
	BOOST_TEST(!Validate_Static_Mesh_Source(source));
}

#if defined(_WIN32)

import Graphics.Tests.Device;
import Graphics.Testing.VisualRegression;

#ifndef GRAPHICS_STATIC_MESH_REFERENCE_DIRECTORY
#define GRAPHICS_STATIC_MESH_REFERENCE_DIRECTORY "."
#endif

#ifndef GRAPHICS_STATIC_MESH_FAILURE_DIRECTORY
#define GRAPHICS_STATIC_MESH_FAILURE_DIRECTORY "."
#endif

#ifndef GRAPHICS_STATIC_MESH_SHADER_DIRECTORY
#define GRAPHICS_STATIC_MESH_SHADER_DIRECTORY "."
#endif

namespace
{
bool Render_Static_Mesh(Device &, CommandList &commands, RHITextureHandle color_target, RHITextureHandle depth_target, RHIViewport viewport, void *context) noexcept
{
	StaticMeshRenderer &renderer = *static_cast<StaticMeshRenderer *>(context);
	return renderer.Render(commands, {{color_target, viewport.width, viewport.height}, {depth_target, viewport.width, viewport.height}});
}

RenderTransform Make_LOD_Transform(float scale) noexcept
{
	RenderTransform transform;
	transform.matrix = Matrix4x4::Identity().values;
	transform.matrix[0] = scale;
	transform.matrix[5] = scale;
	transform.matrix[10] = scale;
	return transform;
}
}

BOOST_AUTO_TEST_CASE(static_mesh_variant_switch_keeps_instance_handle)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	StaticMeshRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_STATIC_MESH_SHADER_DIRECTORY), 2, 1));

	const std::array<StaticMeshVertex, 3> vertices = {{
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	}};
	const std::array<std::uint16_t, 3> indices = {0, 1, 2};
	const StaticMeshSource source{
		3,
		3,
		static_cast<std::uint32_t>(sizeof(StaticMeshVertex)),
		MeshIndexFormat::UInt16,
		std::as_bytes(std::span<const StaticMeshVertex>(vertices)),
		std::as_bytes(std::span<const std::uint16_t>(indices)),
		{0.0f, 0.0f, 0.0f},
		1.0f
	};
	const MeshHandle first_mesh = renderer.Create_Mesh(source);
	const MeshHandle second_mesh = renderer.Create_Mesh(source);
	BOOST_REQUIRE(first_mesh.Is_Valid());
	BOOST_REQUIRE(second_mesh.Is_Valid());

	StaticMeshBinding binding;
	RenderTransform transform;
	transform.matrix = Matrix4x4::Identity().values;
	const RenderBounds bounds{{0.0f, 0.0f, 0.0f}, 1.0f};
	const RenderInstanceFlags flags = RenderInstanceFlags::CastsShadow;
	const StaticMeshSource invalid_source{};
	BOOST_CHECK(!binding.Replace(renderer, invalid_source, transform, bounds, renderer.Default_Material(), flags));
	BOOST_CHECK(!binding.Is_Active());

	BOOST_REQUIRE(binding.Replace(renderer, source, transform, bounds, renderer.Default_Material(), flags));
	BOOST_REQUIRE(binding.Is_Active());
	BOOST_REQUIRE(binding.Instance().Is_Valid());
	const InstanceHandle stable_instance = binding.Instance();
	const MeshHandle stable_mesh = binding.Mesh();
	BOOST_CHECK(!binding.Replace(renderer, invalid_source, transform, bounds, renderer.Default_Material(), flags));
	BOOST_CHECK(binding.Is_Active());
	BOOST_CHECK(binding.Instance() == stable_instance);
	BOOST_CHECK(binding.Mesh() == stable_mesh);
	BOOST_REQUIRE(binding.Replace(renderer, source, transform, bounds, renderer.Default_Material(), flags));
	BOOST_CHECK(binding.Instance() == stable_instance);
	BOOST_CHECK(binding.Mesh() != first_mesh);
	BOOST_CHECK(binding.Mesh() != second_mesh);

	BOOST_REQUIRE(binding.Suspend(renderer, transform));
	BOOST_CHECK(!binding.Is_Active());
	BOOST_CHECK(binding.Instance() == stable_instance);
	BOOST_CHECK(!binding.Replace(renderer, invalid_source, transform, bounds, renderer.Default_Material(), flags));
	BOOST_CHECK(binding.Instance() == stable_instance);

	binding.Destroy(renderer);
	BOOST_CHECK(!binding.Has_Instance());
	BOOST_CHECK(!binding.Is_Active());
	BOOST_CHECK(!binding.Instance().Is_Valid());

	BOOST_CHECK(renderer.Destroy_Mesh(second_mesh));
	BOOST_CHECK(renderer.Destroy_Mesh(first_mesh));
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(static_mesh_lod_switch_uses_screen_space_selection)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	StaticMeshRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_STATIC_MESH_SHADER_DIRECTORY), 2, 1));

	const std::array<std::uint16_t, 3> indices = {0, 1, 2};
	const std::array<StaticMeshVertex, 3> high_detail_vertices = {{
		{{-0.72f, -0.62f, 0.35f}, {1.0f, 0.10f, 0.10f, 1.0f}, {0.0f, 1.0f}},
		{{0.0f, 0.78f, 0.35f}, {1.0f, 0.10f, 0.10f, 1.0f}, {0.5f, 0.0f}},
		{{0.72f, -0.62f, 0.35f}, {1.0f, 0.10f, 0.10f, 1.0f}, {1.0f, 1.0f}}
	}};
	const std::array<StaticMeshVertex, 3> low_detail_vertices = {{
		{{-0.42f, -0.36f, 0.35f}, {0.10f, 0.20f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{0.0f, 0.46f, 0.35f}, {0.10f, 0.20f, 1.0f, 1.0f}, {0.5f, 0.0f}},
		{{0.42f, -0.36f, 0.35f}, {0.10f, 0.20f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	}};
	const StaticMeshSource high_detail_source{
		3,
		3,
		static_cast<std::uint32_t>(sizeof(StaticMeshVertex)),
		MeshIndexFormat::UInt16,
		std::as_bytes(std::span<const StaticMeshVertex>(high_detail_vertices)),
		std::as_bytes(std::span<const std::uint16_t>(indices)),
		{0.0f, 0.0f, 0.35f},
		1.0f
	};
	const StaticMeshSource low_detail_source{
		3,
		3,
		static_cast<std::uint32_t>(sizeof(StaticMeshVertex)),
		MeshIndexFormat::UInt16,
		std::as_bytes(std::span<const StaticMeshVertex>(low_detail_vertices)),
		std::as_bytes(std::span<const std::uint16_t>(indices)),
		{0.0f, 0.0f, 0.35f},
		1.0f
	};
	const std::array<StaticMeshLODSource, 2> lod_sources = {{
		{high_detail_source, 0.0f},
		{low_detail_source, 0.75f}
	}};

	StaticMeshBinding binding;
	BOOST_REQUIRE(binding.Replace_LODs(renderer, lod_sources, Make_LOD_Transform(0.5f),
		{{0.0f, 0.0f, 0.35f}, 1.0f}, renderer.Default_Material(), RenderInstanceFlags::None));
	BOOST_REQUIRE(binding.Mesh_LODs().size() == 2);
	renderer.Set_View({Matrix4x4::Identity(), Matrix4x4::Identity(), {}, {0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}});

	VisualRegressionHarness harness({
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_STATIC_MESH_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_STATIC_MESH_FAILURE_DIRECTORY)
	});
	const VisualComparisonResult near_result = harness.Run(device, "StaticMeshRenderer.LOD.Near", Render_Static_Mesh, &renderer);
	BOOST_CHECK_MESSAGE(near_result.expected_loaded, "missing colocated near LOD reference image");
	BOOST_CHECK_MESSAGE(near_result.matched, "near LOD visual regression mismatch");

	BOOST_REQUIRE(binding.Update(renderer, Make_LOD_Transform(0.25f), RenderInstanceFlags::None));
	const VisualComparisonResult far_result = harness.Run(device, "StaticMeshRenderer.LOD.Far", Render_Static_Mesh, &renderer);
	BOOST_CHECK_MESSAGE(far_result.expected_loaded, "missing colocated far LOD reference image");
	BOOST_CHECK_MESSAGE(far_result.matched, "far LOD visual regression mismatch");

	binding.Destroy(renderer);
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(static_mesh_binding_queries_static_bone_and_attachment_transforms)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	StaticMeshRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_STATIC_MESH_SHADER_DIRECTORY), 1, 1));

	const std::array<StaticMeshVertex, 3> vertices = {{
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	}};
	const std::array<std::uint16_t, 3> indices = {0, 1, 2};
	const StaticMeshSource source{
		3,
		3,
		static_cast<std::uint32_t>(sizeof(StaticMeshVertex)),
		MeshIndexFormat::UInt16,
		std::as_bytes(std::span<const StaticMeshVertex>(vertices)),
		std::as_bytes(std::span<const std::uint16_t>(indices)),
		{0.0f, 0.0f, 0.0f},
		1.0f
	};

	const std::array<SkeletonBone, 1> bones = {{
		{Invalid_Bone_Index, RenderTransform{Matrix4x4::Identity().values}}
	}};
	const std::array<SkeletonAttachment, 1> attachments = {{
		{BoneHandle(0, 1), RenderTransform{Matrix4x4::Identity().values}}
	}};
	const SkeletonHandle skeleton = renderer.Create_Skeleton(bones, attachments);
	BOOST_REQUIRE(skeleton.Is_Valid());

	RenderTransform model_transform;
	model_transform.matrix = Matrix4x4::Identity().values;
	model_transform.matrix[3] = 10.0f;
	model_transform.matrix[7] = 20.0f;
	model_transform.matrix[11] = 30.0f;
	const RenderBounds bounds{{0.0f, 0.0f, 0.0f}, 1.0f};
	StaticMeshBinding binding;
	BOOST_REQUIRE(binding.Replace(renderer, source, model_transform, bounds, renderer.Default_Material(),
		RenderInstanceFlags::None, All_Submeshes_Visible, skeleton));

	RenderTransform result;
	BOOST_REQUIRE(binding.Get_Bone_Transform(renderer, BoneHandle(0, 1), result));
	BOOST_CHECK(result.matrix[3] == 10.0f);
	BOOST_CHECK(result.matrix[7] == 20.0f);
	BOOST_CHECK(result.matrix[11] == 30.0f);
	BOOST_REQUIRE(binding.Get_Attachment_Transform(renderer, AttachmentHandle(0, 1), result));
	BOOST_CHECK(result.matrix[3] == 10.0f);
	BOOST_CHECK(result.matrix[7] == 20.0f);
	BOOST_CHECK(result.matrix[11] == 30.0f);
	BOOST_CHECK(!binding.Get_Bone_Transform(renderer, BoneHandle(0, 2), result));
	BOOST_CHECK(!binding.Get_Attachment_Transform(renderer, AttachmentHandle(0, 2), result));

	binding.Destroy(renderer);
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(static_mesh_submesh_visibility_preserves_instance_state)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	StaticMeshRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_STATIC_MESH_SHADER_DIRECTORY), 1, 1));

	const std::array<StaticMeshVertex, 6> vertices = {{
		{{0.0f, 0.78f, 0.35f}, {1.0f, 0.20f, 0.10f, 1.0f}, {0.5f, 0.0f}},
		{{-0.72f, -0.62f, 0.35f}, {0.10f, 0.85f, 0.20f, 1.0f}, {0.0f, 1.0f}},
		{{0.72f, -0.62f, 0.35f}, {0.10f, 0.20f, 0.95f, 1.0f}, {1.0f, 1.0f}},
		{{-0.25f, 0.25f, 0.35f}, {1.0f, 1.0f, 0.10f, 1.0f}, {0.0f, 0.0f}},
		{{-0.65f, -0.25f, 0.35f}, {0.95f, 0.10f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{0.15f, -0.25f, 0.35f}, {0.10f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	}};
	const std::array<std::uint16_t, 6> indices = {0, 2, 1, 3, 5, 4};
	const std::array<MeshPart, 2> parts = {{{0, 3, 0}, {3, 3, 0}}};
	const StaticMeshSource source{
		6,
		6,
		static_cast<std::uint32_t>(sizeof(StaticMeshVertex)),
		MeshIndexFormat::UInt16,
		std::as_bytes(std::span<const StaticMeshVertex>(vertices)),
		std::as_bytes(std::span<const std::uint16_t>(indices)),
		{0.0f, 0.0f, 0.35f},
		1.0f,
		parts
	};

	StaticMeshBinding binding;
	RenderTransform transform;
	transform.matrix = Matrix4x4::Identity().values;
	const RenderBounds bounds{{0.0f, 0.0f, 0.35f}, 1.0f};
	const RenderInstanceFlags flags = RenderInstanceFlags::CastsShadow;
	BOOST_REQUIRE(binding.Replace(renderer, source, transform, bounds, renderer.Default_Material(), flags));
	const InstanceHandle stable_instance = binding.Instance();
	BOOST_REQUIRE(binding.Set_Submesh_Visible(renderer, 1, false));
	BOOST_CHECK(binding.Instance() == stable_instance);
	BOOST_CHECK(!Is_Submesh_Visible(binding.Visibility(), 1));
	BOOST_CHECK(Is_Submesh_Visible(binding.Visibility(), 0));
	BOOST_CHECK(binding.Mesh().Is_Valid());
	BOOST_CHECK(!binding.Set_Submesh_Visible(renderer, 2, false));

	renderer.Set_View({
		Matrix4x4::Identity(),
		Matrix4x4::Identity(),
		{},
		{0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}
	});
	VisualRegressionHarness harness({
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_STATIC_MESH_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_STATIC_MESH_FAILURE_DIRECTORY)
	});
	const VisualComparisonResult result = harness.Run(device, "StaticMeshRenderer.SubmeshVisibility", Render_Static_Mesh, &renderer);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated static mesh submesh reference image");
	BOOST_CHECK_MESSAGE(result.matched, "static mesh submesh visibility visual regression mismatch");

	binding.Destroy(renderer);
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(static_mesh_shadow_setting_preserves_instance_handle)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	StaticMeshRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_STATIC_MESH_SHADER_DIRECTORY), 1, 1));
	const std::array<StaticMeshVertex, 3> vertices = {{
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	}};
	const std::array<std::uint16_t, 3> indices = {0, 1, 2};
	const StaticMeshSource source{
		3,
		3,
		static_cast<std::uint32_t>(sizeof(StaticMeshVertex)),
		MeshIndexFormat::UInt16,
		std::as_bytes(std::span<const StaticMeshVertex>(vertices)),
		std::as_bytes(std::span<const std::uint16_t>(indices)),
		{0.0f, 0.0f, 0.0f},
		1.0f
	};

	StaticMeshBinding binding;
	BOOST_REQUIRE(binding.Replace(renderer, source, Identity_Render_Transform(), {{}, 1.0f}, renderer.Default_Material(),
		RenderInstanceFlags::CastsShadow));
	const InstanceHandle instance = binding.Instance();
	BOOST_CHECK(binding.Casts_Shadow());
	BOOST_REQUIRE(binding.Set_Casts_Shadow(renderer, false));
	BOOST_CHECK(!binding.Casts_Shadow());
	BOOST_CHECK(binding.Instance() == instance);
	BOOST_REQUIRE(binding.Set_Casts_Shadow(renderer, true));
	BOOST_CHECK(binding.Casts_Shadow());
	BOOST_CHECK(binding.Instance() == instance);

	binding.Destroy(renderer);
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(static_mesh_renderer_matches_colocated_production_golden_image)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	StaticMeshRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_STATIC_MESH_SHADER_DIRECTORY), 1, 1));
	const std::array<StaticMeshVertex, 3> vertices = {{
		{{0.0f, 0.78f, 0.35f}, {1.0f, 0.20f, 0.10f, 1.0f}, {0.5f, 0.0f}},
		{{-0.72f, -0.62f, 0.35f}, {0.10f, 0.85f, 0.20f, 1.0f}, {0.0f, 1.0f}},
		{{0.72f, -0.62f, 0.35f}, {0.10f, 0.20f, 0.95f, 1.0f}, {1.0f, 1.0f}}
	}};
	const std::array<std::uint16_t, 3> indices = {0, 2, 1};
	const MeshHandle mesh = renderer.Create_Mesh({
		3,
		3,
		static_cast<std::uint32_t>(sizeof(StaticMeshVertex)),
		MeshIndexFormat::UInt16,
		std::as_bytes(std::span<const StaticMeshVertex>(vertices)),
		std::as_bytes(std::span<const std::uint16_t>(indices)),
		{0.0f, 0.0f, 0.35f},
		1.0f
	});
	BOOST_REQUIRE(mesh.Is_Valid());

	RenderInstance instance;
	instance.transform.matrix = Matrix4x4::Identity().values;
	instance.bounds = {{0.0f, 0.0f, 0.35f}, 1.0f};
	instance.mesh = mesh;
	instance.material = renderer.Default_Material();
	instance.flags = RenderInstanceFlags::CastsShadow;
	const InstanceHandle instance_handle = renderer.Create_Instance(instance);
	BOOST_REQUIRE(instance_handle.Is_Valid());
	renderer.Set_View({
		Matrix4x4::Identity(),
		Matrix4x4::Identity(),
		{},
		{0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}
	});

	VisualRegressionHarness harness({
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_STATIC_MESH_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_STATIC_MESH_FAILURE_DIRECTORY)
	});
	const VisualComparisonResult result = harness.Run(device, "StaticMeshRenderer", Render_Static_Mesh, &renderer);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated static mesh renderer reference image");
	BOOST_CHECK_MESSAGE(result.matched, "static mesh renderer visual regression mismatch");

	BOOST_REQUIRE(renderer.Destroy_Instance(instance_handle));
	BOOST_REQUIRE(renderer.Destroy_Mesh(mesh));
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(skinned_mesh_renderer_matches_colocated_production_golden_image)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	StaticMeshRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_STATIC_MESH_SHADER_DIRECTORY), 1, 1, 4));

	std::array<SkinnedMeshVertex, 3> vertices{};
	vertices[0].position[0] = -0.72f;
	vertices[0].position[1] = -0.62f;
	vertices[0].position[2] = 0.35f;
	vertices[0].color[0] = 1.0f;
	vertices[0].color[1] = 0.20f;
	vertices[0].color[2] = 0.10f;
	vertices[0].color[3] = 1.0f;
	vertices[0].uv[0] = 0.0f;
	vertices[0].uv[1] = 1.0f;
	vertices[0].skinning.bone_indices[0] = 0;
	vertices[0].skinning.bone_weights[0] = 1.0f;

	vertices[1].position[0] = 0.0f;
	vertices[1].position[1] = 0.78f;
	vertices[1].position[2] = 0.35f;
	vertices[1].color[0] = 0.10f;
	vertices[1].color[1] = 0.85f;
	vertices[1].color[2] = 0.20f;
	vertices[1].color[3] = 1.0f;
	vertices[1].uv[0] = 0.5f;
	vertices[1].skinning.bone_indices[0] = 1;
	vertices[1].skinning.bone_weights[0] = 1.0f;

	vertices[2].position[0] = 0.72f;
	vertices[2].position[1] = -0.62f;
	vertices[2].position[2] = 0.35f;
	vertices[2].color[0] = 0.10f;
	vertices[2].color[1] = 0.20f;
	vertices[2].color[2] = 0.95f;
	vertices[2].color[3] = 1.0f;
	vertices[2].uv[0] = 1.0f;
	vertices[2].uv[1] = 1.0f;
	vertices[2].skinning.bone_indices[0] = 0;
	vertices[2].skinning.bone_weights[0] = 1.0f;

	const std::array<std::uint16_t, 3> indices = {0, 1, 2};
	RenderTransform child_transform;
	child_transform.matrix = Matrix4x4::Identity().values;
	child_transform.matrix[3] = 0.20f;
	const std::array<SkeletonBone, 2> bones = {{
		{Invalid_Bone_Index, RenderTransform{Matrix4x4::Identity().values}},
		{0, child_transform}
	}};
	const SkeletonHandle skeleton = renderer.Create_Skeleton(bones);
	BOOST_REQUIRE(skeleton.Is_Valid());

	StaticMeshSource source;
	source.vertex_count = static_cast<std::uint32_t>(vertices.size());
	source.index_count = static_cast<std::uint32_t>(indices.size());
	source.vertex_stride = sizeof(SkinnedMeshVertex);
	source.index_format = MeshIndexFormat::UInt16;
	source.vertex_data = std::as_bytes(std::span<const SkinnedMeshVertex>(vertices));
	source.index_data = std::as_bytes(std::span<const std::uint16_t>(indices));
	source.bounds_center = {0.10f, 0.0f, 0.35f};
	source.bounds_radius = 1.0f;
	source.vertex_format = MeshVertexFormat::Position3Color4UV2Skinned;
	source.skin_bone_count = 2;

	RenderTransform transform;
	transform.matrix = Matrix4x4::Identity().values;
	StaticMeshBinding binding;
	BOOST_REQUIRE(binding.Replace(renderer, source, transform, {{0.10f, 0.0f, 0.35f}, 1.0f},
		renderer.Default_Material(), RenderInstanceFlags::None, All_Submeshes_Visible, skeleton));
	BOOST_REQUIRE(binding.Pose().Is_Valid());

	renderer.Set_View({
		Matrix4x4::Identity(),
		Matrix4x4::Identity(),
		{},
		{0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}
	});
	VisualRegressionHarness harness({
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_STATIC_MESH_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_STATIC_MESH_FAILURE_DIRECTORY)
	});
	const VisualComparisonResult result = harness.Run(device, "StaticMeshRenderer.Skinned", Render_Static_Mesh, &renderer);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated skinned mesh reference image");
	BOOST_CHECK_MESSAGE(result.matched, "skinned mesh renderer visual regression mismatch");

	binding.Destroy(renderer);
	renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(blended_skinned_mesh_renderer_matches_colocated_production_golden_image)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	StaticMeshRenderer renderer;
	BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_STATIC_MESH_SHADER_DIRECTORY), 1, 1, 4));

	std::array<SkinnedMeshVertex, 3> vertices{};
	vertices[0].position[0] = -0.72f;
	vertices[0].position[1] = -0.62f;
	vertices[0].position[2] = 0.35f;
	vertices[0].color[0] = 1.0f;
	vertices[0].color[1] = 0.20f;
	vertices[0].color[3] = 1.0f;
	vertices[0].skinning.bone_weights[0] = 1.0f;
	vertices[1].position[1] = 0.78f;
	vertices[1].position[2] = 0.35f;
	vertices[1].color[1] = 0.85f;
	vertices[1].color[3] = 1.0f;
	vertices[1].skinning.bone_indices[0] = 1;
	vertices[1].skinning.bone_weights[0] = 1.0f;
	vertices[2].position[0] = 0.72f;
	vertices[2].position[1] = -0.62f;
	vertices[2].position[2] = 0.35f;
	vertices[2].color[2] = 0.95f;
	vertices[2].color[3] = 1.0f;
	vertices[2].skinning.bone_weights[0] = 1.0f;
	const std::array<std::uint16_t, 3> indices = {0, 1, 2};

	const std::array<SkeletonBone, 2> bones = {{
		{Invalid_Bone_Index, RenderTransform{Matrix4x4::Identity().values}},
		{0, RenderTransform{Matrix4x4::Identity().values}}
	}};
	const SkeletonHandle skeleton = renderer.Create_Skeleton(bones);
	BOOST_REQUIRE(skeleton.Is_Valid());

	const std::array<RenderTransform, 2> first_pose = {
		RenderTransform{Matrix4x4::Identity().values},
		RenderTransform{Matrix4x4::Identity().values}
	};
	RenderTransform moved_bone;
	moved_bone.matrix = Matrix4x4::Identity().values;
	moved_bone.matrix[3] = 0.40f;
	const std::array<RenderTransform, 2> second_pose = {
		RenderTransform{Matrix4x4::Identity().values}, moved_bone
	};
	const AnimationClipHandle first_animation = renderer.Create_Animation_Clip({2, 1, 1.0f, first_pose});
	const AnimationClipHandle second_animation = renderer.Create_Animation_Clip({2, 1, 1.0f, second_pose});
	BOOST_REQUIRE(first_animation.Is_Valid());
	BOOST_REQUIRE(second_animation.Is_Valid());

	StaticMeshSource source;
	source.vertex_count = static_cast<std::uint32_t>(vertices.size());
	source.index_count = static_cast<std::uint32_t>(indices.size());
	source.vertex_stride = sizeof(SkinnedMeshVertex);
	source.index_format = MeshIndexFormat::UInt16;
	source.vertex_data = std::as_bytes(std::span<const SkinnedMeshVertex>(vertices));
	source.index_data = std::as_bytes(std::span<const std::uint16_t>(indices));
	source.bounds_center = {0.10f, 0.0f, 0.35f};
	source.bounds_radius = 1.0f;
	source.vertex_format = MeshVertexFormat::Position3Color4UV2Skinned;
	source.skin_bone_count = 2;

	RenderTransform transform;
	transform.matrix = Matrix4x4::Identity().values;
	StaticMeshBinding binding;
	BOOST_REQUIRE(binding.Replace(renderer, source, transform, {{0.10f, 0.0f, 0.35f}, 1.0f},
		renderer.Default_Material(), RenderInstanceFlags::None, All_Submeshes_Visible, skeleton,
		first_animation, AnimationPlaybackMode::Once, 0.0f));
	BOOST_REQUIRE(binding.Set_Animation_Blend(renderer, first_animation, AnimationPlaybackMode::Once, 0.0f,
		second_animation, AnimationPlaybackMode::Once, 0.0f, 0.5f));
	BOOST_CHECK(binding.Secondary_Animation() == second_animation);
	BOOST_CHECK_CLOSE(binding.Blend_Weight(), 0.5f, 0.001);

	renderer.Set_View({
		Matrix4x4::Identity(),
		Matrix4x4::Identity(),
		{},
		{0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}
	});
	VisualRegressionHarness harness({
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_STATIC_MESH_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_STATIC_MESH_FAILURE_DIRECTORY)
	});
	const VisualComparisonResult result = harness.Run(device, "StaticMeshRenderer.Blended", Render_Static_Mesh, &renderer);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated blended mesh reference image");
	BOOST_CHECK_MESSAGE(result.matched, "blended mesh renderer visual regression mismatch");

	binding.Destroy(renderer);
	renderer.Shutdown();
}

#endif
