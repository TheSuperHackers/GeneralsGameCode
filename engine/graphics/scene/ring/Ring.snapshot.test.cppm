module;

#define BOOST_TEST_MODULE GraphicsRingSnapshotTests

#include <boost/test/included/unit_test.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>

export module Graphics.Scene.Ring.Snapshot.Tests;

import Graphics.Tests.Device;
import Graphics.RHI;
import Graphics.Scene.Ring;
import Graphics.Scene.Screen.FullscreenOverlay;
import Graphics.Testing.VisualRegression;

using namespace Graphics;

#ifndef GRAPHICS_RING_SNAPSHOT_REFERENCE_DIRECTORY
#define GRAPHICS_RING_SNAPSHOT_REFERENCE_DIRECTORY "."
#endif

#ifndef GRAPHICS_RING_SNAPSHOT_FAILURE_DIRECTORY
#define GRAPHICS_RING_SNAPSHOT_FAILURE_DIRECTORY "."
#endif

#ifndef GRAPHICS_RING_SNAPSHOT_SHADER_DIRECTORY
#define GRAPHICS_RING_SNAPSHOT_SHADER_DIRECTORY "."
#endif

namespace
{
struct RingSnapshotContext final
{
	RingRenderer *ring = nullptr;
	FullscreenOverlayRenderer *overlay = nullptr;
	std::uint32_t mode = 0;
};

static bool Render_Ring_Overlay(Device &, CommandList &commands, RHITextureHandle color_target,
	RHITextureHandle depth_target, RHIViewport viewport, void *context) noexcept
{
	const RingSnapshotContext &snapshot = *static_cast<const RingSnapshotContext *>(context);
	if (!commands.Set_Render_Targets(color_target, depth_target)
		|| !commands.Clear({0.02f, 0.02f, 0.03f, 1.0f}, 1.0f))
		return false;

	if (!snapshot.ring->Set_Ring({-0.20f, 0.05f, 0.0f, 0.0f, 0.30f, {0.10f, 0.75f, 1.0f, 0.55f}, RingSegmentCount}))
		return false;

	FullscreenOverlayDescription overlay;
	overlay.color = {0.22f, 0.18f, 0.12f, 1.0f};
	switch (snapshot.mode) {
	case 1:
		overlay.blend_operation = RHIBlendOperation::ReverseSubtract;
		break;
	case 2:
		overlay.blend_mode = RHIBlendMode::ColorMultiply;
		overlay.draw_count = 2;
		break;
	case 3:
		overlay.blend_mode = RHIBlendMode::Multiply;
		break;
	default:
		break;
	}
	if (!snapshot.overlay->Set_Overlay(overlay))
		return false;

	return snapshot.ring->Render(commands, color_target, viewport)
		&& snapshot.overlay->Render(commands, color_target, viewport);
}

static void Run_Ring_Snapshot(std::uint32_t mode, const char *name)
{
	GraphicsTestDevice device({true});
	BOOST_REQUIRE(device.Is_Valid());

	RingRenderer ring;
	FullscreenOverlayRenderer overlay;
	BOOST_REQUIRE(ring.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_RING_SNAPSHOT_SHADER_DIRECTORY)));
	BOOST_REQUIRE(overlay.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_RING_SNAPSHOT_SHADER_DIRECTORY)));

	const VisualRegressionConfig config{
		128,
		72,
		2,
		std::filesystem::path(GRAPHICS_RING_SNAPSHOT_REFERENCE_DIRECTORY),
		std::filesystem::path(GRAPHICS_RING_SNAPSHOT_FAILURE_DIRECTORY)
	};
	VisualRegressionHarness harness(config);
	RingSnapshotContext context{&ring, &overlay, mode};
	const VisualComparisonResult result = harness.Run(device, name, Render_Ring_Overlay, &context);
	BOOST_CHECK_MESSAGE(result.expected_loaded, "missing colocated ring snapshot");
	BOOST_CHECK_MESSAGE(result.matched, "ring rendering snapshot mismatch");

	overlay.Shutdown();
	ring.Shutdown();
}
}

BOOST_AUTO_TEST_CASE(ring_add_matches_colocated_snapshot)
{
	Run_Ring_Snapshot(0, "ring_overlay_add");
}

BOOST_AUTO_TEST_CASE(ring_reverse_subtract_matches_colocated_snapshot)
{
	Run_Ring_Snapshot(1, "ring_overlay_reverse_subtract");
}

BOOST_AUTO_TEST_CASE(ring_color_multiply_matches_colocated_snapshot)
{
	Run_Ring_Snapshot(2, "ring_overlay_color_multiply");
}

BOOST_AUTO_TEST_CASE(ring_multiply_matches_colocated_snapshot)
{
	Run_Ring_Snapshot(3, "ring_overlay_multiply");
}
