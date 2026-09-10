module;

#define BOOST_TEST_MODULE GraphicsSphereTests

#include <boost/test/included/unit_test.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <filesystem>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Scene.Sphere.Tests;

import Assets.Math;
import Assets.Spheres;
import Graphics.Tests.Device;
import Graphics.RHI;
import Graphics.Resources.Textures.References;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Shadows.DirectionalRenderer;
import Graphics.Scene.Sphere;

namespace
{

Assets::SphereAssetDesc Make_Asset()
{
	Assets::SphereAssetDesc asset;
	asset.name = "test.sphere";
	asset.extent = {1.0f, 1.0f, 1.0f};
	asset.default_color = {0.2f, 0.4f, 0.6f};
	asset.default_alpha = 0.75f;
	asset.default_scale = {1.0f, 1.0f, 1.0f};
	asset.default_vector_rotation = {0.0f, 0.0f, 0.0f, 1.0f};
	asset.default_vector_intensity = 1.0f;
	asset.animation_duration = 2.0f;
	asset.color_track.keys = {
		{0.0f, {0.0f, 0.0f, 0.0f}},
		{1.0f, {2.0f, 4.0f, 6.0f}}};
	asset.alpha_track.keys = {{0.0f, 0.25f}, {1.0f, 0.75f}};
	asset.scale_track.keys = {
		{0.0f, {1.0f, 2.0f, 3.0f}},
		{1.0f, {3.0f, 4.0f, 5.0f}}};
	asset.vector_track.keys = {
		{0.0f, {0.0f, 0.0f, 0.0f, 1.0f}, 0.5f},
		{1.0f, {0.0f, 0.0f, 0.70710677f, 0.70710677f}, 1.5f}};
	return asset;
}

bool Close(float actual, float expected, float epsilon = 0.0001f)
{
	return std::abs(actual - expected) <= epsilon;
}

void Check_RGBA_Bounds(const std::array<std::byte, 32 * 32 * 4> &pixels,
	unsigned x, unsigned y, std::array<unsigned, 4> expected, unsigned tolerance)
{
	const std::size_t offset = (static_cast<std::size_t>(y) * 32u + x) * 4u;
	for (unsigned channel = 0; channel < expected.size(); ++channel) {
		const unsigned actual = std::to_integer<unsigned>(pixels[offset + channel]);
		const unsigned lower = expected[channel] > tolerance ? expected[channel] - tolerance : 0u;
		const unsigned upper = (std::min)(255u, expected[channel] + tolerance);
		BOOST_CHECK(actual >= lower);
		BOOST_CHECK(actual <= upper);
	}
}

}

BOOST_AUTO_TEST_CASE(animation_extrapolates_holds_and_preserves_single_subtraction_loop)
{
	Assets::SphereAssetDesc asset = Make_Asset();
	Graphics::SphereRuntimeState state = Graphics::Make_Sphere_Runtime_State(asset);

	BOOST_REQUIRE(Graphics::Evaluate_Sphere_Animation(asset, -0.5f, state));
	BOOST_CHECK(Close(state.color.x, -1.0f));
	BOOST_CHECK(Close(state.color.y, -2.0f));
	BOOST_CHECK(Close(state.alpha, 0.0f));
	BOOST_CHECK(Close(state.scale.z, 2.0f));

	BOOST_REQUIRE(Graphics::Evaluate_Sphere_Animation(asset, 0.5f, state));
	BOOST_CHECK(Close(state.color.x, 1.0f));
	BOOST_CHECK(Close(state.alpha, 0.5f));
	BOOST_CHECK(Close(state.scale.y, 3.0f));
	BOOST_CHECK(Close(state.vector_intensity, 1.0f));

	BOOST_REQUIRE(Graphics::Evaluate_Sphere_Animation(asset, 4.0f, state));
	BOOST_CHECK(Close(state.color.z, 6.0f));
	BOOST_CHECK(Close(state.alpha, 0.75f));
	BOOST_CHECK(Close(state.scale.x, 3.0f));

	state = Graphics::Make_Sphere_Runtime_State(asset);
	asset.attributes |= Assets::SphereAttributeAnimationLoop;
	BOOST_REQUIRE(Graphics::Advance_Sphere_Animation(asset, 6.0f, state));
	// 6 / 2 adds 3; the source subtracts one once, leaving 2 instead of
	// applying a remainder operation. The final key therefore remains held.
	BOOST_CHECK(Close(state.animation_time, 2.0f));
	BOOST_CHECK(Close(state.color.x, 2.0f));
	BOOST_CHECK(Close(state.alpha, 0.75f));
}

BOOST_AUTO_TEST_CASE(visibility_restarts_animation_at_the_authored_boundary)
{
	Assets::SphereAssetDesc asset = Make_Asset();
	asset.center = {1.0f, -2.5f, 3.0f};
	Graphics::SphereSceneObject sphere(asset);
	BOOST_REQUIRE(sphere.Update(0.5f));
	BOOST_CHECK(sphere.State().animation_time > 0.0f);

	sphere.Set_Animation_Hidden(true);
	BOOST_CHECK(!sphere.Is_Drawable());
	BOOST_CHECK(!sphere.State().animating);
	BOOST_CHECK(Close(sphere.State().animation_time, 0.0f));
	sphere.Set_Animation_Hidden(false);
	BOOST_CHECK(sphere.Is_Drawable());
	BOOST_CHECK(sphere.State().animating);
	BOOST_CHECK(Close(sphere.State().animation_time, 0.0f));
	BOOST_REQUIRE(sphere.Update(0.25f));
	BOOST_CHECK(Close(sphere.State().animation_time, 0.125f));

	sphere.Set_Hidden(true);
	BOOST_CHECK(!sphere.Is_Drawable());
	sphere.Set_Hidden(false);
	BOOST_CHECK(sphere.Is_Drawable());
	BOOST_CHECK(Close(sphere.State().animation_time, 0.0f));

	// Camera culling owns IS_VISIBLE and force-visible. Sphere rendering and
	// animation use Is_Not_Hidden_At_All(), so these flags do not stop them.
	sphere.Set_Visible(false);
	BOOST_CHECK(!sphere.Is_Visible());
	BOOST_CHECK(sphere.Is_Drawable());
	sphere.Set_Force_Visible(true);
	BOOST_CHECK(sphere.Is_Force_Visible());
	BOOST_CHECK(sphere.Is_Drawable());
	sphere.Set_Visible(true);
}

BOOST_AUTO_TEST_CASE(lod_costs_bounds_and_directional_material_data_are_stable)
{
	Assets::SphereAssetDesc asset = Make_Asset();
	asset.center = {1.0f, -2.5f, 3.0f};
	Graphics::SphereSceneObject sphere(asset);
	sphere.State().scale = {2.0f, 3.0f, 4.0f};
	sphere.Set_LOD_Level(1);
	const Graphics::SphereBounds bounds = sphere.Bounds();
	BOOST_CHECK(Close(bounds.center.x, 1.0f));
	BOOST_CHECK(Close(bounds.center.y, -2.5f));
	BOOST_CHECK(Close(bounds.center.z, 3.0f));
	BOOST_CHECK(Close(bounds.extent.x, 2.0f));
	BOOST_CHECK(Close(bounds.extent.y, 3.0f));
	BOOST_CHECK(Close(bounds.extent.z, 4.0f));
	BOOST_CHECK(Close(bounds.radius, std::sqrt(29.0f)));
	BOOST_CHECK(Close(Graphics::Sphere_LOD_Cost(0), Graphics::SphereNullLODCost));
	BOOST_CHECK(Close(Graphics::Sphere_LOD_Cost(1), 98.0f));
	BOOST_CHECK(Close(Graphics::Sphere_LOD_Cost(10), 512.0f));

	std::vector<Graphics::PropVertex> vertices;
	std::vector<std::uint32_t> indices;
	Graphics::SphereRuntimeState state = Graphics::Make_Sphere_Runtime_State(asset);
	state.lod = 1;
	state.color = {0.25f, 0.5f, 0.75f};
	state.alpha = 0.5f;
	BOOST_REQUIRE(Graphics::Build_Sphere_Geometry(asset, state, vertices, indices));
	BOOST_REQUIRE_EQUAL(vertices.size(), 58u);
	BOOST_REQUIRE_EQUAL(indices.size(), 98u * 3u);
	bool directional_alpha_seen = false;
	for (const auto &vertex : vertices) {
		BOOST_CHECK(vertex.material_emissive[0] == state.color.x);
		BOOST_CHECK(vertex.material_emissive[1] == state.color.y);
		BOOST_CHECK(vertex.material_emissive[2] == state.color.z);
		BOOST_CHECK(vertex.material_diffuse[3] == state.alpha);
		if (vertex.color[3] != 1.0f)
			directional_alpha_seen = true;
	}
	BOOST_CHECK(directional_alpha_seen);

	Assets::SphereAssetDesc additive = asset;
	additive.material.destination_blend = Assets::SphereBlendFactor::One;
	state.color = {0.8f, 0.4f, 0.2f};
	state.alpha = 0.5f;
	BOOST_REQUIRE(Graphics::Build_Sphere_Geometry(additive, state, vertices, indices));
	float strongest = 0.0f;
	bool directional_color_seen = false;
	for (const auto &vertex : vertices)
	{
		strongest = (std::max)(strongest, vertex.color[0]);
		BOOST_CHECK(vertex.color[3] == 0.0f);
		BOOST_CHECK(vertex.material_diffuse[3] == 0.25f);
		BOOST_CHECK(vertex.material_emissive[0] == state.color.x * state.alpha);
		BOOST_CHECK(vertex.material_emissive[1] == state.color.y * state.alpha);
		BOOST_CHECK(vertex.material_emissive[2] == state.color.z * state.alpha);
		if (vertex.color[0] != 0.0f)
			directional_color_seen = true;
	}
	BOOST_CHECK(strongest > 0.9f);
	BOOST_CHECK(directional_color_seen);

	sphere.Scale(2.0f, 3.0f, 4.0f);
	BOOST_CHECK(Close(sphere.State().scale.x, 4.0f));
	BOOST_CHECK(Close(sphere.State().scale.y, 9.0f));
	BOOST_CHECK(Close(sphere.State().scale.z, 16.0f));
	BOOST_CHECK(Close(sphere.Asset().scale_track.keys.front().value.x, 2.0f));
	BOOST_CHECK(Close(sphere.Asset().scale_track.keys.front().value.y, 6.0f));
	BOOST_CHECK(Close(sphere.Asset().scale_track.keys.front().value.z, 12.0f));

	sphere.Set_LOD_Level(0);
	BOOST_CHECK(sphere.Num_Polys() == 0u);
	sphere.Increment_LOD();
	BOOST_CHECK(sphere.LOD_Level() == 1u);
	sphere.Decrement_LOD();
	BOOST_CHECK(sphere.LOD_Level() == 0u);
	sphere.Set_LOD_Level(Graphics::SphereLODCount);
	sphere.Prepare_LOD(2.0f);
	BOOST_CHECK(sphere.Value() > sphere.Post_Increment_Value());
}

BOOST_AUTO_TEST_CASE(shader_state_maps_effective_details_and_camera_alignment)
{
	Assets::SphereAssetDesc asset = Make_Asset();
	asset.material.destination_blend = Assets::SphereBlendFactor::One;
	asset.material.source_blend = Assets::SphereBlendFactor::SourceAlpha;
	// A retained texture handle determines texturing at draw time. The authored
	// shader bit alone must not disable a sphere that has texture data.
	asset.material.texturing = false;
	asset.texture_name = "sphere.tga";
	asset.material.post_detail_color_function = Assets::SphereDetailColorFunction::Add;
	asset.material.post_detail_alpha_function = Assets::SphereDetailAlphaFunction::Scale;
	asset.attributes |= Assets::SphereAttributeCameraAligned;
	Graphics::SphereRuntimeState state = Graphics::Make_Sphere_Runtime_State(asset);
	state.scale = {2.0f, 3.0f, 4.0f};
	Graphics::SphereDrawInput input;
	input.world[3] = 2.0f;
	input.world[7] = 3.0f;
	input.world[11] = 4.0f;
	input.view[3] = -1.0f;
	input.view[7] = -2.0f;
	input.view[11] = -3.0f;
	Graphics::SphereDrawState draw;
	BOOST_REQUIRE(Graphics::Build_Sphere_Draw_State(asset, state, input, draw));
	BOOST_CHECK(draw.additive);
	BOOST_CHECK(draw.style.source_blend == Graphics::RHIBlendFactor::SourceAlpha);
	BOOST_CHECK(draw.style.destination_blend == Graphics::RHIBlendFactor::One);
	BOOST_CHECK(draw.material.opacity == 0.25f);
	BOOST_CHECK(draw.material.emissive[0] == state.color.x * state.alpha);
	BOOST_CHECK(draw.material.emissive[1] == state.color.y * state.alpha);
	BOOST_CHECK(draw.material.emissive[2] == state.color.z * state.alpha);
	BOOST_CHECK(draw.parameters.textured == 1.0f);
	BOOST_CHECK(draw.parameters.detail_color == static_cast<float>(Assets::SphereDetailColorFunction::Add));
	BOOST_CHECK(draw.parameters.detail_alpha == static_cast<float>(Assets::SphereDetailAlphaFunction::Scale));
	asset.material.alpha_test = true;
	BOOST_REQUIRE(Graphics::Build_Sphere_Draw_State(asset, state, input, draw));
	BOOST_CHECK(Close(draw.parameters.alpha_cutoff, 96.0f / 255.0f));
	asset.material.alpha_test = false;
	BOOST_CHECK(draw.parameters.world[3] == 1.0f);
	BOOST_CHECK(draw.parameters.world[7] == 1.0f);
	BOOST_CHECK(draw.parameters.world[11] == 1.0f);
	BOOST_CHECK(draw.parameters.world[1] == 3.0f);
	BOOST_CHECK(draw.parameters.world[6] == 4.0f);
	BOOST_CHECK(draw.parameters.world[8] == 2.0f);

	// Matrix3D::Scale scales basis columns while preserving translation. Use a
	// rotated, translated transform so row/column confusion is observable.
	asset.attributes &= ~Assets::SphereAttributeCameraAligned;
	input.world = {
		0.0f, -1.0f, 0.0f, 10.0f,
		1.0f, 0.0f, 0.0f, 20.0f,
		0.0f, 0.0f, 1.0f, 30.0f,
		0.0f, 0.0f, 0.0f, 1.0f};
	BOOST_REQUIRE(Graphics::Build_Sphere_Draw_State(asset, state, input, draw));
	BOOST_CHECK(draw.parameters.world[0] == 0.0f);
	BOOST_CHECK(draw.parameters.world[4] == 2.0f);
	BOOST_CHECK(draw.parameters.world[1] == -3.0f);
	BOOST_CHECK(draw.parameters.world[5] == 0.0f);
	BOOST_CHECK(draw.parameters.world[10] == 4.0f);
	BOOST_CHECK(draw.parameters.world[3] == 10.0f);
	BOOST_CHECK(draw.parameters.world[7] == 20.0f);
	BOOST_CHECK(draw.parameters.world[11] == 30.0f);
}

BOOST_AUTO_TEST_CASE(camera_aligned_sphere_uses_source_permutation_with_rotated_view)
{
	Assets::SphereAssetDesc asset = Make_Asset();
	asset.attributes |= Assets::SphereAttributeCameraAligned;
	asset.extent = {1.5f, 0.75f, 2.0f};
	Graphics::SphereRuntimeState state = Graphics::Make_Sphere_Runtime_State(asset);
	state.scale = {2.0f, 3.0f, 4.0f};
	Graphics::SphereDrawInput input;
	input.world = {
		0.0f, 0.0f, 1.0f, 10.0f,
		0.0f, 1.0f, 0.0f, 20.0f,
		-1.0f, 0.0f, 0.0f, 30.0f,
		0.0f, 0.0f, 0.0f, 1.0f};
	input.view = {
		0.0f, -1.0f, 0.0f, 5.0f,
		1.0f, 0.0f, 0.0f, -6.0f,
		0.0f, 0.0f, 1.0f, 7.0f,
		0.0f, 0.0f, 0.0f, 1.0f};
	Graphics::SphereDrawState draw;
	BOOST_REQUIRE(Graphics::Build_Sphere_Draw_State(asset, state, input, draw));
	// real_scale = {3, 2.25, 8}; camera_position = view * {10,20,30,1}.
	BOOST_CHECK(Close(draw.parameters.world[1], 2.25f));
	BOOST_CHECK(Close(draw.parameters.world[6], 8.0f));
	BOOST_CHECK(Close(draw.parameters.world[8], 3.0f));
	BOOST_CHECK(Close(draw.parameters.world[3], -15.0f));
	BOOST_CHECK(Close(draw.parameters.world[7], 4.0f));
	BOOST_CHECK(Close(draw.parameters.world[11], 37.0f));
	BOOST_CHECK(Close(draw.parameters.camera_position[0], -15.0f));
	BOOST_CHECK(Close(draw.parameters.camera_position[1], 4.0f));
	BOOST_CHECK(Close(draw.parameters.camera_position[2], 37.0f));
}

BOOST_AUTO_TEST_CASE(lod_value_sentinels_match_render_object_contract)
{
	std::array<float, Graphics::SphereLODValueCount> values{};
	std::array<float, Graphics::SphereLODCount + 1> costs{};
	BOOST_REQUIRE(Graphics::Calculate_Sphere_LOD_Values(1.0f, 1.0f, values, costs));
	BOOST_CHECK(values.front() == (std::numeric_limits<float>::max)());
	BOOST_CHECK(values.back() == -1.0f);
	BOOST_CHECK(costs.front() == Graphics::SphereNullLODCost);
}

BOOST_AUTO_TEST_CASE(sphere_geometry_draws_after_source_release_and_target_recreation)
{
	Graphics::GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());
	Graphics::PropRenderer renderer;
	const auto shader_directory = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	Graphics::DirectionalShadowRenderer shadows;
	BOOST_REQUIRE(shadows.Initialize(device, shader_directory));
	Graphics::PropSubmission submission;
	submission.Initialize(device, renderer, shadows);

	Graphics::SphereSceneObject sphere([] {
		Assets::SphereAssetDesc value;
		value.name = "draw.sphere";
		value.default_color = {0.8f, 0.2f, 0.1f};
		value.default_alpha = 0.5f;
		value.attributes = 0;
		value.material.texturing = false;
		return value;
	}());
	sphere.Set_LOD_Level(1);
	Graphics::SphereDrawInput input;
	// D3D11 clips depth to [0,w].  The unit sphere is authored around zero,
	// so map its Z range into that interval before rasterizing it.
	input.view_projection = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.25f, 0.5f, 0, 0, 0, 1};
	input.view = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	Graphics::SphereRenderer sphere_renderer;

	using namespace Graphics;
	auto target = device.Create_Texture({32, 32, 1, RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(RHITextureUsage::RenderTarget)});
	auto depth = device.Create_Texture({32, 32, 1, RHITextureFormat::D32_Float,
		static_cast<unsigned>(RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
	auto draw_once = [&](RHITextureHandle color_target, RHITextureHandle depth_target) {
		auto &commands = device.Immediate_Command_List();
		return commands.Set_Render_Targets(color_target, depth_target)
			&& commands.Set_Viewport({0, 0, 32, 32})
			&& commands.Clear({0, 0, 0, 0}, 1.0f)
			&& sphere_renderer.Submit(renderer, submission, sphere, input,
				std::span<const RHITextureHandle>{});
	};
	BOOST_REQUIRE(draw_once(target, depth));
	std::array<std::byte, 32 * 32 * 4> pixels{};
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32 * 4));
	Check_RGBA_Bounds(pixels, 16, 16, {204u, 51u, 26u, 128u}, 4u);
	Check_RGBA_Bounds(pixels, 0, 0, {0u, 0u, 0u, 0u}, 0u);

	// A no-draw result leaves the caller's retained texture reference intact.
	// This exercises the ownership contract without requiring a mesh upload.
	Graphics::SphereSceneObject hidden_sphere([] {
		Assets::SphereAssetDesc value;
		value.name = "hidden.sphere";
		value.texture_name = "hidden.tga";
		return value;
	}());
	hidden_sphere.Set_Animation_Hidden(true);
	const std::array<std::byte, 4> hidden_texel{
		std::byte{17}, std::byte{29}, std::byte{43}, std::byte{61}};
	const auto hidden_texture = device.Create_Texture_Initialized({1, 1},
		{std::as_bytes(std::span(hidden_texel)), 4});
	BOOST_REQUIRE(hidden_texture.Is_Valid());
	Graphics::SphereRenderer hidden_renderer;
	BOOST_CHECK(!hidden_renderer.Submit(renderer, submission, hidden_sphere, input,
		std::array{hidden_texture}));
	std::array<std::byte, 4> hidden_readback{};
	BOOST_REQUIRE(device.Readback_Texture(hidden_texture, hidden_readback, 4));
	BOOST_CHECK(hidden_readback == hidden_texel);
	BOOST_REQUIRE(device.Destroy_Texture(hidden_texture));

	// A sorted sphere uses the same production submission boundary and is
	// consumed by the transparent queue before its mesh reference is released.
	Graphics::SphereSceneObject sorted_sphere([] {
		Assets::SphereAssetDesc value;
		value.name = "sorted.sphere";
		value.default_color = {0.1f, 0.3f, 0.7f};
		value.default_alpha = 0.75f;
		value.attributes = 0;
		value.material.destination_blend = Assets::SphereBlendFactor::One;
		return value;
	}());
	sorted_sphere.Set_LOD_Level(1);
	Graphics::SphereDrawInput sorted_input = input;
	sorted_input.world = {
		0.0f, -1.0f, 0.0f, 0.1f,
		1.0f, 0.0f, 0.0f, -0.1f,
		0.0f, 0.0f, 1.0f, 0.25f,
		0.0f, 0.0f, 0.0f, 1.0f};
	sorted_input.view = {
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.1f,
		0.0f, 0.0f, 0.0f, 1.0f};
	// Keep the view and projection composed.  With the old identity matrix,
	// the front and back pole fans were clipped at the projected center.
		sorted_input.view_projection = {
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.25f, 0.525f,
		0.0f, 0.0f, 0.0f, 1.0f};
	sorted_input.sorting_enabled = true;
	Graphics::SphereRenderer sorted_renderer;
	auto &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1.0f));
	BOOST_REQUIRE(sorted_renderer.Submit(renderer, submission, sorted_sphere, sorted_input,
		std::span<const RHITextureHandle>{}));
	BOOST_REQUIRE(submission.Flush_Transparent());
	pixels.fill(std::byte{0});
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32 * 4));
	// Additive spheres retain the source alpha in emissive RGB but use the
	// source material's authored opacity of .25 for the output alpha.
	Check_RGBA_Bounds(pixels, 16, 16, {19u, 57u, 134u, 64u}, 6u);
	Check_RGBA_Bounds(pixels, 0, 0, {0u, 0u, 0u, 0u}, 0u);
	// This single deferred draw verifies extraction and lifetime. A
	// non-commutative overlapping ordering case remains outside this fixture.
	sorted_renderer.Release(renderer);

	sphere_renderer.Release(renderer);
	hidden_renderer.Release(renderer);
	submission.Shutdown();
	shadows.Shutdown();
	renderer.Shutdown();
	device.Destroy_Texture(target);
	device.Destroy_Texture(depth);
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	BOOST_REQUIRE(shadows.Initialize(device, shader_directory));
	submission.Initialize(device, renderer, shadows);
	target = device.Create_Texture({32, 32, 1, RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(RHITextureUsage::RenderTarget)});
	depth = device.Create_Texture({32, 32, 1, RHITextureFormat::D32_Float,
		static_cast<unsigned>(RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(draw_once(target, depth));
	pixels.fill(std::byte{0});
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32 * 4));
	Check_RGBA_Bounds(pixels, 16, 16, {204u, 51u, 26u, 128u}, 4u);
	Check_RGBA_Bounds(pixels, 0, 0, {0u, 0u, 0u, 0u}, 0u);

	sphere_renderer.Release(renderer);
	submission.Shutdown();
	shadows.Shutdown();
	renderer.Shutdown();
	device.Destroy_Texture(target);
	device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(sphere_texture_transfer_preserves_cached_source_across_repeated_and_deferred_draws)
{
	Graphics::GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());
	const auto shader_directory = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
	Graphics::PropRenderer renderer;
	Graphics::DirectionalShadowRenderer shadows;
	Graphics::PropSubmission submission;
	BOOST_REQUIRE(renderer.Initialize(device, shader_directory));
	BOOST_REQUIRE(shadows.Initialize(device, shader_directory));
	submission.Initialize(device, renderer, shadows);
	Graphics::TextureReferences references;

	const auto target = device.Create_Texture({32, 32, 1, Graphics::RHITextureFormat::RGBA8_UNorm,
		static_cast<unsigned>(Graphics::RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({32, 32, 1, Graphics::RHITextureFormat::D32_Float,
		static_cast<unsigned>(Graphics::RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());

	const std::array<std::uint8_t, 4> texel{255, 0, 0, 255};
	const auto source = device.Create_Texture_Initialized({1, 1},
		{std::as_bytes(std::span(texel)), 4});
	BOOST_REQUIRE(source.Is_Valid());
	const auto borrowed = references.Retain(device, source);
	BOOST_REQUIRE(borrowed.Is_Valid());
	// The cache owns the only remaining source reference after this release.
	BOOST_REQUIRE(device.Destroy_Texture(source));

	Assets::SphereAssetDesc asset;
	asset.name = "unnamed-textured-sphere";
	asset.default_color = {1.0f, 1.0f, 1.0f};
	asset.default_alpha = 1.0f;
	asset.material.texturing = false;
	asset.material.source_blend = Assets::SphereBlendFactor::One;
	asset.material.destination_blend = Assets::SphereBlendFactor::Zero;
	Graphics::SphereSceneObject sphere(asset);
	sphere.Set_LOD_Level(1);
	Graphics::SphereRenderer sphere_renderer;
	Graphics::SphereDrawInput input;
	input.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 0.25f, 0.5f, 0, 0, 0, 1};
	input.view = {1, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 1};

	auto &commands = device.Immediate_Command_List();
	std::array<std::byte, 32 * 32 * 4> pixels{};
	std::array<std::byte, 4> source_readback{};
	for (unsigned draw = 0; draw < 2; ++draw) {
		BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
		BOOST_REQUIRE(commands.Set_Viewport({0, 0, 32, 32}));
		BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1.0f));
		// PropSubmission consumes this transfer reference on success. The
		// cache-owned reference must survive both immediate draws.
		BOOST_REQUIRE(device.Retain_Texture(borrowed));
		BOOST_REQUIRE(sphere_renderer.Submit(renderer, submission, sphere, input,
			std::array{borrowed}));
		BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32 * 4));
		Check_RGBA_Bounds(pixels, 16, 16, {255u, 0u, 0u, 255u}, 4u);
		BOOST_REQUIRE(device.Readback_Texture(borrowed, source_readback, 4));
		BOOST_CHECK((source_readback == std::array<std::byte, 4>{
			std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255}}));
	}

	// An unsupported two-texture span leaves the transfer reference with its
	// caller; release that reference while retaining the cache entry.
	BOOST_REQUIRE(device.Retain_Texture(borrowed));
	BOOST_CHECK(!sphere_renderer.Submit(renderer, submission, sphere, input,
		std::array{borrowed, Graphics::RHITextureHandle{}}));
	BOOST_REQUIRE(device.Destroy_Texture(borrowed));
	BOOST_REQUIRE(device.Readback_Texture(borrowed, source_readback, 4));

	// A transparent draw retains the transfer until the queue flushes. The
	// original source reference was already released above, so this also
	// covers the deferred lifetime supplied solely by the cache and queue.
	Assets::SphereAssetDesc deferred_asset = asset;
	deferred_asset.material.destination_blend = Assets::SphereBlendFactor::InverseSourceAlpha;
	Graphics::SphereSceneObject deferred_sphere(deferred_asset);
	deferred_sphere.Set_LOD_Level(1);
	Graphics::SphereDrawInput deferred_input = input;
	deferred_input.sorting_enabled = true;
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1.0f));
	BOOST_REQUIRE(device.Retain_Texture(borrowed));
	BOOST_REQUIRE(sphere_renderer.Submit(renderer, submission, deferred_sphere,
		deferred_input, std::array{borrowed}));
	// The queue owns the transfer reference independently of the cache. Clear
	// the cache before extraction to prove the deferred draw keeps its source
	// alive until the submission is consumed.
	references.Clear();
	BOOST_REQUIRE(submission.Flush_Transparent());
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32 * 4));
	Check_RGBA_Bounds(pixels, 16, 16, {255u, 0u, 0u, 255u}, 4u);
	BOOST_CHECK(!device.Retain_Texture(borrowed));

	sphere_renderer.Release(renderer);
	submission.Shutdown();
	shadows.Shutdown();
	renderer.Shutdown();
	references.Clear();
	BOOST_CHECK(!device.Retain_Texture(borrowed));
	BOOST_REQUIRE(device.Destroy_Texture(target));
	BOOST_REQUIRE(device.Destroy_Texture(depth));
}
