module;

#define BOOST_TEST_MODULE GraphicsAttachmentGraphTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <type_traits>

export module Graphics.Scene.Attachments.Tests;

import Graphics.Scene.Attachments;

#if defined(_WIN32)
import Graphics.Tests.Device;
import Graphics.Scene.StaticMeshes;
import Graphics.Testing.VisualRegression;
#endif

using namespace Graphics;

static RenderTransform Translation(float x, float y, float z) noexcept
{
	RenderTransform transform = Identity_Render_Transform();
	transform.matrix[3] = x;
	transform.matrix[7] = y;
	transform.matrix[11] = z;
	return transform;
}

static RenderInstance Make_Instance(MeshHandle mesh, MaterialHandle material, RenderTransform transform) noexcept
{
	RenderInstance instance;
	instance.transform = transform;
	instance.bounds.radius = 1.0f;
	instance.mesh = mesh;
	instance.material = material;
	instance.flags = RenderInstanceFlags::CastsShadow | RenderInstanceFlags::ReceivesShadow;
	return instance;
}

static float Translation_X(const RenderTransform &transform) noexcept
{
	return transform.matrix[3];
}

BOOST_AUTO_TEST_CASE(parent_transform_propagates_to_child_without_recreating_it)
{
	RenderScene scene;
	const InstanceHandle parent = scene.Create(Make_Instance(MeshHandle(1, 1), MaterialHandle(1, 1), Translation(10.0f, 2.0f, 3.0f)));
	const InstanceHandle child = scene.Create(Make_Instance(MeshHandle(2, 1), MaterialHandle(2, 1), Translation(0.0f, 0.0f, 0.0f)));
	AttachmentGraph graph;
	graph.Reserve(1);

	AttachmentTarget target;
	target.local_transform = Translation(1.0f, 0.0f, 0.0f);
	const AttachmentLinkHandle link = graph.Attach(scene, child, parent, target, Translation(0.0f, 2.0f, 0.0f));
	BOOST_REQUIRE(link.Is_Valid());
	BOOST_REQUIRE(graph.Update(scene));

	RenderTransform child_transform;
	BOOST_REQUIRE(scene.Get_Transform(child, child_transform));
	BOOST_CHECK(Translation_X(child_transform) == 11.0f);
	BOOST_CHECK(child_transform.matrix[7] == 4.0f);
	BOOST_CHECK(graph.Changed_Children().size() == 1);

	bool preserved = false;
	BOOST_REQUIRE(scene.Visit(child, [&](InstanceHandle handle, const RenderInstanceView &view) noexcept {
		preserved = handle == child && view.mesh == MeshHandle(2, 1)
			&& view.material == MaterialHandle(2, 1)
			&& view.flags == (RenderInstanceFlags::CastsShadow | RenderInstanceFlags::ReceivesShadow);
	}));
	BOOST_CHECK(preserved);
}

BOOST_AUTO_TEST_CASE(bone_and_socket_targets_use_pre_resolved_local_transforms)
{
	RenderScene scene;
	const InstanceHandle parent = scene.Create(Make_Instance(MeshHandle(1, 1), MaterialHandle(1, 1), Translation(4.0f, 0.0f, 0.0f)));
	const InstanceHandle bone_child = scene.Create(Make_Instance(MeshHandle(2, 1), MaterialHandle(2, 1), {}));
	const InstanceHandle socket_child = scene.Create(Make_Instance(MeshHandle(3, 1), MaterialHandle(3, 1), {}));
	AttachmentGraph graph;
	graph.Reserve(2);

	AttachmentTarget bone_target;
	bone_target.kind = AttachmentTargetKind::Bone;
	bone_target.bone = BoneHandle(4, 2);
	bone_target.local_transform = Translation(2.0f, 0.0f, 0.0f);
	BOOST_REQUIRE(graph.Attach(scene, bone_child, parent, bone_target).Is_Valid());

	AttachmentTarget socket_target;
	socket_target.kind = AttachmentTargetKind::Socket;
	socket_target.socket = AttachmentHandle(7, 3);
	socket_target.local_transform = Translation(0.0f, 3.0f, 0.0f);
	BOOST_REQUIRE(graph.Attach(scene, socket_child, parent, socket_target).Is_Valid());
	BOOST_REQUIRE(graph.Update(scene));

	RenderTransform transform;
	BOOST_REQUIRE(scene.Get_Transform(bone_child, transform));
	BOOST_CHECK(Translation_X(transform) == 6.0f);
	BOOST_REQUIRE(scene.Get_Transform(socket_child, transform));
	BOOST_CHECK(transform.matrix[7] == 3.0f);
}

BOOST_AUTO_TEST_CASE(bone_and_socket_targets_follow_the_parent_pose)
{
	const std::array<SkeletonBone, 2> bones = {{
		{Invalid_Bone_Index, Identity_Render_Transform()},
		{0, Translation(1.0f, 0.0f, 0.0f)}
	}};
	const std::array<SkeletonAttachment, 1> sockets = {{
		{BoneHandle(1, 1), Translation(2.0f, 0.0f, 0.0f)}
	}};
	SkeletonPool skeletons;
	skeletons.Reserve(1);
	const SkeletonHandle skeleton = Create_Skeleton(skeletons, {bones, sockets});
	BOOST_REQUIRE(skeleton.Is_Valid());

	std::array<RenderTransform, 2> pose_matrices = {{
		Identity_Render_Transform(),
		Translation(3.0f, 0.0f, 0.0f)
	}};
	BoneMatrixTable poses;
	poses.Reserve(1, 2);
	const PoseHandle pose = poses.Create(pose_matrices);
	BOOST_REQUIRE(pose.Is_Valid());

	RenderScene scene;
	RenderInstance parent_instance = Make_Instance(MeshHandle(1, 1), MaterialHandle(1, 1), Translation(4.0f, 0.0f, 0.0f));
	parent_instance.skeleton = skeleton;
	parent_instance.pose = pose;
	const InstanceHandle parent = scene.Create(parent_instance);
	const RenderInstance child_instance = Make_Instance(MeshHandle(2, 1), MaterialHandle(2, 1), {});
	const InstanceHandle bone_child = scene.Create(child_instance);
	const InstanceHandle socket_child = scene.Create(Make_Instance(MeshHandle(3, 1), MaterialHandle(3, 1), {}));

	AttachmentGraph graph;
	graph.Reserve(2);
	AttachmentTarget bone_target;
	bone_target.kind = AttachmentTargetKind::Bone;
	bone_target.bone = BoneHandle(1, 1);
	BOOST_REQUIRE(graph.Attach(scene, bone_child, parent, bone_target).Is_Valid());
	AttachmentTarget socket_target;
	socket_target.kind = AttachmentTargetKind::Socket;
	socket_target.socket = AttachmentHandle(0, 1);
	BOOST_REQUIRE(graph.Attach(scene, socket_child, parent, socket_target).Is_Valid());

	BOOST_REQUIRE(graph.Update(scene, skeletons, poses));
	RenderTransform transform;
	BOOST_REQUIRE(scene.Get_Transform(bone_child, transform));
	BOOST_CHECK(Translation_X(transform) == 7.0f);
	BOOST_REQUIRE(scene.Get_Transform(socket_child, transform));
	BOOST_CHECK(Translation_X(transform) == 9.0f);

	pose_matrices[1] = Translation(5.0f, 0.0f, 0.0f);
	BOOST_REQUIRE(poses.Update(pose, pose_matrices));
	BOOST_REQUIRE(graph.Update(scene, skeletons, poses));
	BOOST_REQUIRE(scene.Get_Transform(bone_child, transform));
	BOOST_CHECK(Translation_X(transform) == 9.0f);
	BOOST_REQUIRE(scene.Get_Transform(socket_child, transform));
	BOOST_CHECK(Translation_X(transform) == 11.0f);
}

BOOST_AUTO_TEST_CASE(detach_and_reattach_reuse_the_child_instance)
{
	RenderScene scene;
	const InstanceHandle parent = scene.Create(Make_Instance(MeshHandle(1, 1), MaterialHandle(1, 1), Translation(5.0f, 0.0f, 0.0f)));
	const InstanceHandle child = scene.Create(Make_Instance(MeshHandle(2, 1), MaterialHandle(2, 1), {}));
	AttachmentGraph graph;
	graph.Reserve(1);

	const AttachmentLinkHandle first = graph.Attach(scene, child, parent, {});
	BOOST_REQUIRE(first.Is_Valid());
	BOOST_REQUIRE(graph.Update(scene));
	BOOST_REQUIRE(graph.Detach(first));
	BOOST_CHECK(!graph.Detach(first));

	AttachmentTarget replacement;
	replacement.local_transform = Translation(-2.0f, 0.0f, 0.0f);
	const AttachmentLinkHandle second = graph.Attach(scene, child, parent, replacement);
	BOOST_REQUIRE(second.Is_Valid());
	BOOST_CHECK(second != first);
	BOOST_REQUIRE(graph.Update(scene));

	RenderTransform child_transform;
	BOOST_REQUIRE(scene.Get_Transform(child, child_transform));
	BOOST_CHECK(Translation_X(child_transform) == 3.0f);
	BOOST_CHECK(scene.Dense_Index(child) != Invalid_Render_Scene_Index);
}

BOOST_AUTO_TEST_CASE(invalid_handles_and_cycles_are_rejected)
{
	RenderScene scene;
	const InstanceHandle first = scene.Create(Make_Instance(MeshHandle(1, 1), MaterialHandle(1, 1), {}));
	const InstanceHandle second = scene.Create(Make_Instance(MeshHandle(2, 1), MaterialHandle(2, 1), {}));
	const InstanceHandle third = scene.Create(Make_Instance(MeshHandle(3, 1), MaterialHandle(3, 1), {}));
	AttachmentGraph graph;
	graph.Reserve(3);

	BOOST_CHECK(!graph.Attach(scene, InstanceHandle{}, first, {}).Is_Valid());
	BOOST_CHECK(!graph.Attach(scene, second, InstanceHandle{}, {}).Is_Valid());
	AttachmentTarget invalid_bone;
	invalid_bone.kind = AttachmentTargetKind::Bone;
	BOOST_CHECK(!graph.Attach(scene, second, first, invalid_bone).Is_Valid());

	BOOST_REQUIRE(graph.Attach(scene, second, first, {}).Is_Valid());
	BOOST_REQUIRE(graph.Attach(scene, third, second, {}).Is_Valid());
	BOOST_CHECK(!graph.Attach(scene, first, third, {}).Is_Valid());

	const InstanceHandle stale = second;
	BOOST_REQUIRE(scene.Destroy(stale));
	BOOST_CHECK(!graph.Update(scene));
	BOOST_CHECK(!graph.Attach(scene, stale, first, {}).Is_Valid());
	graph.Remove_Instance(stale);
	BOOST_CHECK(graph.Size() == 0);
}

BOOST_AUTO_TEST_CASE(updated_attachment_data_preserves_child_state)
{
	RenderScene scene;
	RenderInstance parent_instance = Make_Instance(MeshHandle(1, 1), MaterialHandle(1, 1), Translation(2.0f, 0.0f, 0.0f));
	RenderInstance child_instance = Make_Instance(MeshHandle(9, 1), MaterialHandle(8, 1), Translation(0.0f, 4.0f, 0.0f));
	child_instance.visibility_mask = 0x5u;
	const InstanceHandle parent = scene.Create(parent_instance);
	const InstanceHandle child = scene.Create(child_instance);
	AttachmentGraph graph;
	graph.Reserve(1);
	const AttachmentLinkHandle link = graph.Attach(scene, child, parent, {});
	BOOST_REQUIRE(link.Is_Valid());
	BOOST_REQUIRE(graph.Update(scene));

	AttachmentTarget updated_target;
	updated_target.kind = AttachmentTargetKind::Socket;
	updated_target.socket = AttachmentHandle(1, 1);
	updated_target.local_transform = Translation(0.0f, 1.0f, 0.0f);
	BOOST_REQUIRE(graph.Update_Target(link, updated_target));
	BOOST_REQUIRE(graph.Update(scene));

	bool preserved = false;
	BOOST_REQUIRE(scene.Visit(child, [&](InstanceHandle, const RenderInstanceView &view) noexcept {
		preserved = view.mesh == child_instance.mesh && view.material == child_instance.material
			&& view.visibility_mask == child_instance.visibility_mask;
	}));
	BOOST_CHECK(preserved);
}

#if defined(_WIN32)

#ifndef GRAPHICS_ATTACHMENT_REFERENCE_DIRECTORY
#define GRAPHICS_ATTACHMENT_REFERENCE_DIRECTORY "."
#endif

#ifndef GRAPHICS_ATTACHMENT_FAILURE_DIRECTORY
#define GRAPHICS_ATTACHMENT_FAILURE_DIRECTORY "."
#endif

#ifndef GRAPHICS_ATTACHMENT_SHADER_DIRECTORY
#define GRAPHICS_ATTACHMENT_SHADER_DIRECTORY "."
#endif

namespace
{
struct AttachmentVisualScene final
{
	StaticMeshRenderer renderer;
};

bool Render_Attachment_Scene(Device &, CommandList &commands, RHITextureHandle color_target,
	RHITextureHandle depth_target, RHIViewport viewport, void *context) noexcept
{
	AttachmentVisualScene &scene = *static_cast<AttachmentVisualScene *>(context);
	return scene.renderer.Render(commands,
		{{color_target, viewport.width, viewport.height}, {depth_target, viewport.width, viewport.height}});
}

RenderTransform Make_Visual_Transform(float x) noexcept
{
	return Translation(x, 0.0f, 0.35f);
}
}

BOOST_AUTO_TEST_CASE(attached_models_match_colocated_snapshot)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());
	AttachmentVisualScene scene;
	BOOST_REQUIRE(scene.renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_ATTACHMENT_SHADER_DIRECTORY), 2, 2));

	const std::array<StaticMeshVertex, 3> parent_vertices = {{
		{{-0.35f, -0.35f, 0.35f}, {0.95f, 0.15f, 0.08f, 1.0f}, {0.0f, 1.0f}},
		{{0.0f, 0.35f, 0.35f}, {1.0f, 0.25f, 0.08f, 1.0f}, {0.5f, 0.0f}},
		{{0.35f, -0.35f, 0.35f}, {0.95f, 0.08f, 0.12f, 1.0f}, {1.0f, 1.0f}}
	}};
	const std::array<StaticMeshVertex, 3> child_vertices = {{
		{{-0.35f, -0.35f, 0.35f}, {0.08f, 0.25f, 0.95f, 1.0f}, {0.0f, 1.0f}},
		{{0.0f, 0.35f, 0.35f}, {0.08f, 0.45f, 1.0f, 1.0f}, {0.5f, 0.0f}},
		{{0.35f, -0.35f, 0.35f}, {0.12f, 0.08f, 0.95f, 1.0f}, {1.0f, 1.0f}}
	}};
	const std::array<std::uint16_t, 3> indices = {0, 1, 2};
	const auto make_source = [&](std::span<const StaticMeshVertex> vertices) {
		return StaticMeshSource{
			3,
			3,
			static_cast<std::uint32_t>(sizeof(StaticMeshVertex)),
			MeshIndexFormat::UInt16,
			std::as_bytes(vertices),
			std::as_bytes(std::span<const std::uint16_t>(indices)),
			{0.0f, 0.0f, 0.35f},
			1.0f
		};
	};
	const MeshHandle parent_mesh = scene.renderer.Create_Mesh(make_source(parent_vertices));
	const MeshHandle child_mesh = scene.renderer.Create_Mesh(make_source(child_vertices));
	BOOST_REQUIRE(parent_mesh.Is_Valid());
	BOOST_REQUIRE(child_mesh.Is_Valid());

	const MaterialHandle material = scene.renderer.Default_Material();
	const std::array<SkeletonBone, 1> bones = {{{Invalid_Bone_Index, Identity_Render_Transform()}}};
	const SkeletonHandle skeleton = scene.renderer.Create_Skeleton(bones);
	BOOST_REQUIRE(skeleton.Is_Valid());

	RenderInstance parent_instance;
	parent_instance.transform = Make_Visual_Transform(-0.45f);
	parent_instance.bounds = {{0.0f, 0.0f, 0.35f}, 1.0f};
	parent_instance.mesh = parent_mesh;
	parent_instance.material = material;
	parent_instance.skeleton = skeleton;
	parent_instance.flags = RenderInstanceFlags::None;
	const InstanceHandle parent = scene.renderer.Create_Instance(parent_instance);

	RenderInstance child_instance;
	child_instance.transform = Make_Visual_Transform(0.0f);
	child_instance.bounds = {{0.0f, 0.0f, 0.35f}, 1.0f};
	child_instance.mesh = child_mesh;
	child_instance.material = material;
	child_instance.flags = RenderInstanceFlags::None;
	const InstanceHandle child = scene.renderer.Create_Instance(child_instance);
	BOOST_REQUIRE(parent.Is_Valid());
	BOOST_REQUIRE(child.Is_Valid());

	AttachmentTarget target;
	target.kind = AttachmentTargetKind::Bone;
	target.bone = BoneHandle(0, 1);
	BOOST_REQUIRE(scene.renderer.Attach_Instance(child, parent, target, Translation(0.9f, 0.0f, 0.0f)).Is_Valid());
	scene.renderer.Set_View({Matrix4x4::Identity(), Matrix4x4::Identity(), {}, {0.0f, 0.0f, 128.0f, 72.0f, 0.0f, 1.0f}});

	const VisualRegressionConfig config{
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_ATTACHMENT_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_ATTACHMENT_FAILURE_DIRECTORY)
	};
	VisualRegressionHarness harness(config);
	const VisualComparisonResult result = harness.Run(device, "AttachmentGraph", Render_Attachment_Scene, &scene);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated attachment reference image");
	BOOST_CHECK_MESSAGE(result.matched, "attachment visual regression mismatch");

	scene.renderer.Shutdown();
}

#endif
