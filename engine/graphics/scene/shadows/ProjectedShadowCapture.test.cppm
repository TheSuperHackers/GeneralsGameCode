module;
#define BOOST_TEST_MODULE ProjectedShadowCaptureTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

export module Graphics.Scene.Shadows.ProjectedCapture.Tests;
import Graphics.Scene.Shadows.Projected;
import Graphics.Scene.Shadows.ProjectedCapture;
import Graphics.Frame.AttachmentBindings;
import Graphics.Resources.Textures.Resource;
import Graphics.Tests.Device;
import Graphics.RHI;
import Graphics.Scene.Props.Renderer;
import Assets.Images.PixelEncoding;

namespace
{
std::unique_ptr<Graphics::TextureResource> Make_Target(Graphics::Device &device, unsigned size)
{
	return std::unique_ptr<Graphics::TextureResource>(Graphics::TextureResource::Create(
		&device, {size, size, 1, Graphics::RHITextureFormat::RGBA8_UNorm,
			static_cast<unsigned>(Graphics::RHITextureUsage::RenderTarget)
				| static_cast<unsigned>(Graphics::RHITextureUsage::ShaderResource)},
		Assets::PixelEncoding::RGBA8, Graphics::RHITextureFormat::D24_UNorm_S8));
}

Graphics::PropMeshHandle Make_Quad(Graphics::PropRenderer &renderer, float minimum,
	float maximum, std::array<float, 4> color)
{
	std::array<Graphics::PropVertex, 4> vertices{};
	vertices[0].position = {minimum, minimum, 0.5f};
	vertices[1].position = {maximum, minimum, 0.5f};
	vertices[2].position = {maximum, maximum, 0.5f};
	vertices[3].position = {minimum, maximum, 0.5f};
	vertices[0].uv = {0.0f, 1.0f};
	vertices[1].uv = {1.0f, 1.0f};
	vertices[2].uv = {1.0f, 0.0f};
	vertices[3].uv = {0.0f, 0.0f};
	for (auto &vertex : vertices) vertex.color = color;
	const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
	return renderer.Create_Mesh(vertices, indices);
}

std::array<std::byte, 8 * 8 * 4> Read_Capture(Graphics::Device &device,
	Graphics::RHITextureHandle texture)
{
	std::array<std::byte, 8 * 8 * 4> pixels{};
	BOOST_REQUIRE(device.Readback_Texture(texture, pixels, 8 * 4));
	return pixels;
}

void Check_Caster_Pixels(const std::array<std::byte, 8 * 8 * 4> &pixels)
{
	const auto check = [&](unsigned x, unsigned y, unsigned expected_red,
		unsigned expected_alpha) {
		const auto offset = (y * 8 + x) * 4;
		BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset]) - static_cast<int>(expected_red), 2);
		BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset + 1]) - static_cast<int>(expected_red), 2);
		BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset + 2]) - static_cast<int>(expected_red), 2);
		BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset + 3]) - static_cast<int>(expected_alpha), 2);
	};
	check(0, 0, 255, 0);
	check(4, 4, 0, 255);
}

std::array<std::byte, 16 * 16 * 4> Read_Receiver(Graphics::Device &device,
	Graphics::RHITextureHandle texture)
{
	std::array<std::byte, 16 * 16 * 4> pixels{};
	BOOST_REQUIRE(device.Readback_Texture(texture, pixels, 16 * 4));
	return pixels;
}

void Check_Receiver_Pixels(const std::array<std::byte, 16 * 16 * 4> &pixels)
{
	const auto check = [&](unsigned x, unsigned y, unsigned expected_red,
		unsigned expected_green, unsigned expected_blue) {
		const auto offset = (y * 16 + x) * 4;
		BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset]) - static_cast<int>(expected_red), 2);
		BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset + 1]) - static_cast<int>(expected_green), 2);
		BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset + 2]) - static_cast<int>(expected_blue), 2);
		BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset + 3]) - 255, 2);
	};
	check(0, 0, 204, 153, 102);
	check(8, 8, 0, 0, 0);
}
}

BOOST_AUTO_TEST_CASE(capture_clears_transparent_white_and_restores_the_previous_pass)
{
	Graphics::GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());
	Graphics::PropRenderer renderer;
	const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
	BOOST_REQUIRE(renderer.Initialize(device, shaders));

	// Keep the default screen and generated texture at different dimensions.
	auto main_target = Make_Target(device, 16);
	auto capture_target = Make_Target(device, 8);
	BOOST_REQUIRE(main_target && capture_target);
	const auto main_handle = main_target->Handle();
	const auto first_capture_handle = capture_target->Handle();
	BOOST_REQUIRE(device.Retain_Texture(first_capture_handle));

	Graphics::AttachmentBindings attachments;
	BOOST_REQUIRE(attachments.Initialize(device,
		{main_handle, main_target->Depth_Attachment(), {0, 0, 16, 16, 0, 1}}));
	attachments.Clear(true, true, {0.25f, 0.5f, 0.75f, 0.25f});

	const auto caster = Make_Quad(renderer, -0.5f, 0.5f, {0, 0, 0, 1});
	const auto receiver = Make_Quad(renderer, -1.0f, 1.0f, {1, 1, 1, 1});
	BOOST_REQUIRE(caster.Is_Valid() && receiver.Is_Valid());
	Graphics::PropStyle caster_style;
	caster_style.blend = Graphics::RHIBlendMode::Disabled;
	caster_style.depth_test = true;
	caster_style.depth_write = true;
	caster_style.cull = Graphics::RHICullMode::None;
	Graphics::PropParameters caster_parameters;
	caster_parameters.textured = 0.0f;
	caster_parameters.view_projection = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	Graphics::PropParameters receiver_parameters;
	receiver_parameters.alpha_cutoff = 0.5f;
	receiver_parameters.view_projection = caster_parameters.view_projection;

	const bool captured = Graphics::Capture_Projected_Texture(
		attachments, capture_target.get(), renderer,
		[&](Graphics::PropRenderer &value) {
			// The camera adapter converts the normalized camera viewport
			// against the 16x16 screen target. Simulate that result before the
			// capture helper restores the generated texture's pixel viewport.
			BOOST_REQUIRE(attachments.Set_Viewport({0, 0, 16, 16, 0, 1}));
			const auto viewport = Graphics::Projected_Texture_Viewport(8, 8);
			BOOST_REQUIRE(attachments.Set_Viewport(viewport));
			BOOST_CHECK_EQUAL(viewport.x, 1u);
			BOOST_CHECK_EQUAL(viewport.y, 1u);
			BOOST_CHECK_EQUAL(viewport.width, 6u);
			BOOST_CHECK_EQUAL(viewport.height, 6u);
			const bool drawn = value.Draw(device.Immediate_Command_List(), caster,
				caster_style, caster_parameters, {});
			// AttachmentScope retains the GPU generation through this callback.
			capture_target.reset();
			return drawn;
		},
		[](Graphics::PropRenderer &) { return true; });
	BOOST_REQUIRE(captured);
	BOOST_CHECK(!attachments.Offscreen());
	BOOST_CHECK(attachments.Current().color == main_handle);
	Check_Caster_Pixels(Read_Capture(device, first_capture_handle));

	// A failed draw must still restore the caller's attachments and must not
	// invoke the post-capture callback.
	auto failed_target = Make_Target(device, 8);
	BOOST_REQUIRE(failed_target);
	bool flushed = false;
	const bool failed = Graphics::Capture_Projected_Texture(
		attachments, failed_target.get(), renderer,
		[](Graphics::PropRenderer &) { return false; },
		[&](Graphics::PropRenderer &) {
			flushed = true;
			return true;
		});
	BOOST_CHECK(!failed);
	BOOST_CHECK(!flushed);
	BOOST_CHECK(!attachments.Offscreen());
	BOOST_CHECK(attachments.Current().color == main_handle);

	failed_target.reset();

	attachments.Clear(true, true, {0.8f, 0.6f, 0.4f, 1.0f});
	BOOST_REQUIRE(Graphics::Draw_Projected_Shadow(renderer, device.Immediate_Command_List(), receiver,
		receiver_parameters, std::array{first_capture_handle}));
	Check_Receiver_Pixels(Read_Receiver(device, main_handle));

	// Recreate the generated source and draw it again. The old GPU generation
	// remains usable through its explicit retain while the owner is gone.
	renderer.Shutdown();
	BOOST_REQUIRE(renderer.Initialize(device, shaders));
	capture_target = Make_Target(device, 8);
	BOOST_REQUIRE(capture_target);
	const auto recreated_capture_handle = capture_target->Handle();
	const bool recaptured = Graphics::Capture_Projected_Texture(
		attachments, capture_target.get(), renderer,
		[&](Graphics::PropRenderer &value) {
			return value.Draw(device.Immediate_Command_List(), caster, caster_style,
				caster_parameters, {});
		},
		[](Graphics::PropRenderer &) { return true; });
	BOOST_REQUIRE(recaptured);
	Check_Caster_Pixels(Read_Capture(device, recreated_capture_handle));

	attachments.Clear(true, true, {0.8f, 0.6f, 0.4f, 1.0f});
	BOOST_REQUIRE(Graphics::Draw_Projected_Shadow(renderer, device.Immediate_Command_List(), receiver,
		receiver_parameters, std::array{recreated_capture_handle}));
	Check_Receiver_Pixels(Read_Receiver(device, main_handle));

	attachments.Reset();
	BOOST_REQUIRE(device.Destroy_Texture(first_capture_handle));
	renderer.Destroy_Mesh(caster);
	renderer.Destroy_Mesh(receiver);
	renderer.Shutdown();
}
