module;

#define BOOST_TEST_MODULE CameraStateTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>

export module Graphics.Scene.Views.CameraState.Tests;

import Graphics.Scene.Views.CameraState;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;

using namespace Graphics;

namespace
{

RenderTransform Translation(float x, float y, float z) noexcept
{
	RenderTransform transform = Affine_Identity();
	transform.matrix[3] = x;
	transform.matrix[7] = y;
	transform.matrix[11] = z;
	return transform;
}

}

static_assert(std::is_nothrow_copy_constructible_v<CameraState>);
static_assert(std::is_nothrow_copy_assignable_v<CameraState>);

BOOST_AUTO_TEST_CASE(default_camera_uses_perspective_view_plane)
{
	CameraState camera;
	Assets::Vector2f minimum{};
	Assets::Vector2f maximum{};
	camera.Get_View_Plane(minimum, maximum);

	const float expected_width_half = std::tan(0.87266463f * 0.5f);
	const float expected_height_half = expected_width_half / (4.0f / 3.0f);
	BOOST_CHECK(camera.Get_Projection_Type() == CameraProjectionType::Perspective);
	BOOST_CHECK_CLOSE_FRACTION(camera.Get_Depth(), 1000.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(minimum.x, -expected_width_half, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(maximum.x, expected_width_half, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(minimum.y, -expected_height_half, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(maximum.y, expected_height_half, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(camera.Get_Aspect_Ratio(), 4.0f / 3.0f, 0.000001f);
}

BOOST_AUTO_TEST_CASE(perspective_projection_reports_near_far_and_side_results)
{
	CameraState camera;
	camera.Set_Clip_Planes(1.0f, 10.0f);
	camera.Set_View_Plane({-1.0f, -1.0f}, {1.0f, 1.0f});

	Vector3 projected{};
	BOOST_CHECK(camera.Project(projected, {0.0f, 0.0f, -5.0f})
		== CameraProjectionResult::InsideFrustum);
	BOOST_CHECK_SMALL(projected.x, 0.000001f);
	BOOST_CHECK_SMALL(projected.y, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(projected.z, 7.0f / 9.0f, 0.000001f);

	BOOST_CHECK(camera.Project(projected, {6.0f, 0.0f, -5.0f})
		== CameraProjectionResult::OutsideFrustum);
	BOOST_CHECK_CLOSE_FRACTION(projected.x, 1.2f, 0.000001f);

	BOOST_CHECK(camera.Project(projected, {0.0f, 0.0f, -0.999f})
		== CameraProjectionResult::OutsideNearClip);
	BOOST_CHECK_SMALL(projected.x, 0.000001f);
	BOOST_CHECK_SMALL(projected.y, 0.000001f);
	BOOST_CHECK_SMALL(projected.z, 0.000001f);

	BOOST_CHECK(camera.Project(projected, {0.0f, 0.0f, -11.0f})
		== CameraProjectionResult::OutsideFarClip);

	// Project uses the strict near-plane check, while the camera-space entry
	// point retains the legacy epsilon used by render-object callers.
	BOOST_CHECK(camera.Project(projected, {0.0f, 0.0f, -0.99995f})
		== CameraProjectionResult::OutsideNearClip);
	BOOST_CHECK(camera.Project_Camera_Space_Point(projected, {0.0f, 0.0f, -0.99995f})
		== CameraProjectionResult::InsideFrustum);
	BOOST_CHECK(camera.Project_Camera_Space_Point(projected, {0.0f, 0.0f, -0.9998f})
		== CameraProjectionResult::OutsideNearClip);
	BOOST_CHECK_SMALL(projected.x, 0.000001f);
	BOOST_CHECK_SMALL(projected.y, 0.000001f);
	BOOST_CHECK_SMALL(projected.z, 0.000001f);
}

BOOST_AUTO_TEST_CASE(perspective_projection_keeps_view_plane_at_unit_depth)
{
	CameraState camera;
	camera.Set_Clip_Planes(2.0f, 10.0f);
	camera.Set_View_Plane({-1.0f, -1.0f}, {1.0f, 1.0f});

	const Matrix4x4 &projection = camera.Get_Projection_Matrix();
	BOOST_CHECK_CLOSE_FRACTION(projection.values[0], 1.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(projection.values[5], 1.0f, 0.000001f);

	Vector3 projected{};
	BOOST_CHECK(camera.Project_Camera_Space_Point(projected, {2.0f, 0.0f, -2.0f})
		== CameraProjectionResult::InsideFrustum);
	BOOST_CHECK_CLOSE_FRACTION(projected.x, 1.0f, 0.000001f);
	BOOST_CHECK(camera.Project_Camera_Space_Point(projected, {2.1f, 0.0f, -2.0f})
		== CameraProjectionResult::OutsideFrustum);
}

BOOST_AUTO_TEST_CASE(orthographic_projection_does_not_scale_view_plane_by_near_clip)
{
	CameraState camera;
	camera.Set_Projection_Type(CameraProjectionType::Ortho);
	camera.Set_Clip_Planes(2.0f, 10.0f);
	camera.Set_View_Plane({-2.0f, -1.0f}, {2.0f, 1.0f});

	const Matrix4x4 &projection = camera.Get_Projection_Matrix();
	BOOST_CHECK_CLOSE_FRACTION(projection.values[0], 0.5f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(projection.values[5], 1.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(projection.values[10], -0.25f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(projection.values[11], -1.5f, 0.000001f);

	Vector3 projected{};
	BOOST_CHECK(camera.Project_Camera_Space_Point(projected, {0.0f, 0.0f, -6.0f})
		== CameraProjectionResult::InsideFrustum);
	BOOST_CHECK_SMALL(projected.z, 0.000001f);
	BOOST_CHECK(camera.Project_Camera_Space_Point(projected, {3.0f, 0.0f, -6.0f})
		== CameraProjectionResult::OutsideFrustum);
}

BOOST_AUTO_TEST_CASE(transform_projection_and_unprojection_follow_camera_transform)
{
	CameraState camera;
	camera.Set_Clip_Planes(1.0f, 10.0f);
	camera.Set_View_Plane({-1.0f, -1.0f}, {1.0f, 1.0f});
	camera.Set_Transform(Translation(10.0f, 20.0f, 30.0f));

	Vector3 view{};
	camera.Transform_To_View_Space(view, {10.0f, 20.0f, 25.0f});
	BOOST_CHECK_SMALL(view.x, 0.000001f);
	BOOST_CHECK_SMALL(view.y, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(view.z, -5.0f, 0.000001f);

	Vector3 projected{};
	BOOST_CHECK(camera.Project(projected, {10.0f, 20.0f, 25.0f})
		== CameraProjectionResult::InsideFrustum);

	Vector3 world{};
	camera.Un_Project(world, {0.0f, 0.0f});
	BOOST_CHECK_CLOSE_FRACTION(world.x, 10.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(world.y, 20.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(world.z, 29.0f, 0.000001f);

	const RenderTransform &inverse = camera.Get_Inverse_Transform();
	BOOST_CHECK_CLOSE_FRACTION(inverse.matrix[3], -10.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(inverse.matrix[7], -20.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(inverse.matrix[11], -30.0f, 0.000001f);
}

BOOST_AUTO_TEST_CASE(frustum_culls_depth_and_side_volumes)
{
	CameraState camera;
	camera.Set_Clip_Planes(1.0f, 10.0f);
	camera.Set_View_Plane({-1.0f, -1.0f}, {1.0f, 1.0f});

	BOOST_CHECK(!camera.Cull_Sphere({{0.0f, 0.0f, -5.0f}, 0.1f}));
	BOOST_CHECK(camera.Cull_Sphere({{6.0f, 0.0f, -5.0f}, 0.1f}));
	BOOST_CHECK(camera.Cull_Sphere({{0.0f, 0.0f, -0.5f}, 0.1f}));
	BOOST_CHECK(camera.Cull_Sphere({{0.0f, 0.0f, -11.0f}, 0.1f}));
	BOOST_CHECK(!camera.Cull_Sphere_On_Frustum_Sides({{0.0f, 0.0f, -0.5f}, 0.1f}));
	BOOST_CHECK(camera.Cull_Sphere_On_Frustum_Sides({{6.0f, 0.0f, -5.0f}, 0.1f}));

	BOOST_CHECK(!camera.Cull_Box({{0.0f, 0.0f, -5.0f}, {0.2f, 0.2f, 0.2f}}));
	BOOST_CHECK(camera.Cull_Box({{6.0f, 0.0f, -5.0f}, {0.1f, 0.1f, 0.1f}}));

	CameraOrientedBox oriented{};
	oriented.center = {6.0f, 0.0f, -5.0f};
	oriented.extent = {0.1f, 0.1f, 0.1f};
	oriented.basis = {
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f};
	BOOST_CHECK(camera.Cull_Oriented_Box(oriented));
}

BOOST_AUTO_TEST_CASE(near_clip_box_and_view_space_frustum_match_camera_space)
{
	CameraState camera;
	camera.Set_Clip_Planes(2.0f, 10.0f);
	camera.Set_View_Plane({-2.0f, -1.0f}, {2.0f, 1.0f});

	const CameraFrustum &frustum = camera.Get_View_Space_Frustum();
	BOOST_CHECK_CLOSE_FRACTION(frustum.corners[0].x, 4.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(frustum.corners[0].y, -2.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(frustum.corners[0].z, -2.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(frustum.corners[4].x, 20.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(frustum.corners[4].y, -10.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(frustum.corners[4].z, -10.0f, 0.000001f);

	const CameraOrientedBox &near_box = camera.Get_Near_Clip_Bounding_Box();
	BOOST_CHECK_CLOSE_FRACTION(near_box.center.z, -2.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(near_box.extent.x, 4.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(near_box.extent.y, 2.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(near_box.extent.z, 0.01f, 0.000001f);
}

BOOST_AUTO_TEST_CASE(inverse_transform_handles_rotation_and_nonuniform_scale)
{
	CameraState camera;
	RenderTransform transform = Affine_Identity();
	transform.matrix = {
		0.0f, -2.0f, 0.0f, 3.0f,
		3.0f, 0.0f, 0.0f, 4.0f,
		0.0f, 0.0f, 4.0f, 5.0f,
		0.0f, 0.0f, 0.0f, 1.0f};
	camera.Set_Transform(transform);

	Vector3 view{};
	camera.Transform_To_View_Space(view, {1.0f, 7.0f, 1.0f});
	BOOST_CHECK_CLOSE_FRACTION(view.x, 1.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(view.y, 1.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(view.z, -1.0f, 0.000001f);

	const RenderTransform &inverse = camera.Get_Inverse_Transform();
	BOOST_CHECK_CLOSE_FRACTION(inverse.matrix[0], 0.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(inverse.matrix[1], 1.0f / 3.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(inverse.matrix[4], -0.5f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(inverse.matrix[5], 0.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(inverse.matrix[3], -4.0f / 3.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(inverse.matrix[7], 1.5f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(inverse.matrix[11], -1.25f, 0.000001f);
}

BOOST_AUTO_TEST_CASE(viewport_device_conversion_and_camera_publication_are_consistent)
{
	CameraState camera;
	camera.Set_View_Plane({-1.0f, -1.0f}, {1.0f, 1.0f});
	camera.Set_Viewport({0.25f, 0.25f}, {0.75f, 0.75f});

	Vector3 view{};
	camera.Device_To_View_Space({50.0f, 50.0f}, 100.0f, 100.0f, view);
	BOOST_CHECK_SMALL(view.x, 0.000001f);
	BOOST_CHECK_SMALL(view.y, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(view.z, -1.0f, 0.000001f);
	camera.Device_To_View_Space({25.0f, 25.0f}, 100.0f, 100.0f, view);
	BOOST_CHECK_CLOSE_FRACTION(view.x, -1.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(view.y, 1.0f, 0.000001f);

	Vector3 world{};
	camera.Device_To_World_Space({75.0f, 75.0f}, 100.0f, 100.0f, world);
	BOOST_CHECK_CLOSE_FRACTION(world.x, 1.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(world.y, -1.0f, 0.000001f);
	BOOST_CHECK_CLOSE_FRACTION(world.z, -1.0f, 0.000001f);

	CameraMatrices &published = Get_Camera_Matrices();
	const CameraMatrices saved = published;
	camera.Set_Transform(Translation(10.0f, 20.0f, 30.0f));
	camera.Publish_Camera_Matrices();
	const Matrix4x4 &view_matrix = camera.Get_View_Matrix();
	const Matrix4x4 &projection_matrix = camera.Get_Backend_Projection_Matrix();
	BOOST_CHECK(published.view.values == view_matrix.values);
	BOOST_CHECK(published.projection.values == projection_matrix.values);
	published = saved;
}

BOOST_AUTO_TEST_CASE(camera_projection_and_view_transform_reach_gpu_prop_drawing)
{
	for (const bool software : {true, false}) {
		GraphicsTestDevice device({software});
		BOOST_REQUIRE(device.Is_Valid());
		PropRenderer renderer;
		BOOST_REQUIRE(renderer.Initialize(device,
			Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));

		constexpr std::uint32_t target_size = 64;
		const auto target = device.Create_Texture({target_size, target_size, 1,
			RHITextureFormat::RGBA8_UNorm,
			static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
		const auto depth = device.Create_Texture({target_size, target_size, 1,
			RHITextureFormat::D32_Float,
			static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
		BOOST_REQUIRE(target.Is_Valid());
		BOOST_REQUIRE(depth.Is_Valid());

		std::array<PropVertex, 4> vertices{};
		vertices[0].position = {-0.5f, -0.5f, 0.0f};
		vertices[1].position = {0.5f, -0.5f, 0.0f};
		vertices[2].position = {0.5f, 0.5f, 0.0f};
		vertices[3].position = {-0.5f, 0.5f, 0.0f};
		for (auto &vertex : vertices) {
			vertex.color = {1.0f, 0.0f, 0.0f, 1.0f};
			vertex.normal = {0.0f, 0.0f, 1.0f};
			vertex.material_diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
		}
		const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
		const auto mesh = renderer.Create_Mesh(vertices, indices);
		BOOST_REQUIRE(mesh.Is_Valid());

		CameraState camera;
		camera.Set_Projection_Type(CameraProjectionType::Ortho);
		camera.Set_Clip_Planes(1.0f, 10.0f);
		camera.Set_View_Plane({-2.0f, -1.0f}, {2.0f, 1.0f});
		RenderTransform transform = Affine_Identity();
		transform.matrix[3] = 1.0f;
		transform.matrix[11] = 5.0f;
		camera.Set_Transform(transform);

		const Matrix4x4 &view = camera.Get_View_Matrix();
		const Matrix4x4 &projection = camera.Get_Backend_Projection_Matrix();
		PropParameters parameters;
		parameters.view_projection = Compose_Matrices(projection, view).values;
		parameters.view = view.values;
		parameters.camera_position = {transform.matrix[3], transform.matrix[7],
			transform.matrix[11], 1.0f};
		parameters.textured = 0.0f;
		parameters.primary_gradient = 0.0f;

		PropStyle style;
		style.blend = RHIBlendMode::Disabled;
		style.depth_test = false;
		style.depth_write = false;
		style.cull = RHICullMode::None;
		auto &commands = device.Immediate_Command_List();
		BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
		BOOST_REQUIRE(commands.Set_Viewport({0, 0, target_size, target_size}));
		BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1.0f));
		BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters, {}));

		std::array<std::byte, target_size * target_size * 4> pixels{};
		BOOST_REQUIRE(device.Readback_Texture(target, pixels, target_size * 4));
		const auto check_pixel = [&](std::uint32_t x, std::uint32_t y,
			std::array<int, 4> expected) {
			const std::size_t offset = (static_cast<std::size_t>(y) * target_size + x) * 4;
			for (std::size_t channel = 0; channel < expected.size(); ++channel)
				BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset + channel]) - expected[channel], 2);
		};
		// The camera translates the quad one world unit to the left. Its center
		// therefore lands at x=16; an identity view would leave it at x=32.
		check_pixel(16, 32, {255, 0, 0, 255});
		check_pixel(32, 32, {0, 0, 0, 0});

		renderer.Destroy_Mesh(mesh);
		renderer.Shutdown();
		device.Destroy_Texture(target);
		device.Destroy_Texture(depth);
	}
}
