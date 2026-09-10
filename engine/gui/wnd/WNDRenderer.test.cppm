module;

#define BOOST_TEST_MODULE EngineUIWNDRendererTests

#include <boost/test/included/unit_test.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <span>
#include <string>
#include <utility>
#include <vector>

export module Engine.UI.WND.Tests;

import Engine.UI.WND;
import Engine.UI.WND.Layout;

#if defined(ENGINE_UI_WND_VISUAL_REGRESSION)
import Graphics.Testing.VisualRegression;
#endif

using namespace Engine::UI::WND;

namespace
{
struct LayoutWindow
{
	LayoutBounds bounds;
	bool preserve = false;
	LayoutWindow *next = nullptr;
	LayoutWindow *child = nullptr;
	int applications = 0;
};

const LayoutSource layout_source{
	nullptr,
	[](void *, void *p) noexcept -> void * { return static_cast<LayoutWindow *>(p)->next; },
	[](void *, void *p) noexcept -> void * { return static_cast<LayoutWindow *>(p)->child; },
	[](void *, void *p) noexcept { return static_cast<LayoutWindow *>(p)->bounds; },
	[](void *, void *p) noexcept { return static_cast<LayoutWindow *>(p)->preserve; },
	[](void *, void *p, LayoutBounds bounds) noexcept {
		auto &window = *static_cast<LayoutWindow *>(p);
		window.bounds = bounds;
		++window.applications;
	}};
}

BOOST_AUTO_TEST_CASE(wnd_layout_preserves_cards_and_stretches_decoration)
{
	LayoutWindow second{{70, 10, 40, 40}, true};
	LayoutWindow first{{10, 10, 40, 40}, true, &second};
	LayoutWindow decoration{{0, 0, 800, 600}, false, nullptr, &first};
	Layout layout;
	layout.Register(&first, 800, 600);
	layout.Register(&second, 800, 600);
	layout.Apply(&decoration, true, 800, 600, 1600, 900, layout_source);
	BOOST_TEST(decoration.bounds.width == 1600);
	BOOST_TEST(decoration.bounds.height == 900);
	BOOST_TEST(first.bounds.width == 60);
	BOOST_TEST(first.bounds.height == 60);
	BOOST_TEST(first.bounds.x == 45);
	BOOST_TEST(second.bounds.x == 135);
	BOOST_TEST(first.bounds.y == 15);
	BOOST_TEST(first.applications == 1);
	BOOST_TEST(second.applications == 1);
	BOOST_TEST(decoration.applications == 1);
}

BOOST_AUTO_TEST_CASE(wnd_layout_round_trips_without_resize_drift)
{
	LayoutWindow child{{3, 7, 11, 19}, true};
	LayoutWindow root{{0, 0, 800, 600}, false, nullptr, &child};
	Layout layout;
	layout.Register(&child, 800, 600);
	layout.Apply(&root, false, 800, 600, 800, 600, layout_source);
	for (int width = 801; width <= 1920; ++width)
		layout.Apply(&root, false, width - 1, 600, width, 600, layout_source);
	layout.Apply(&root, false, 1920, 600, 3840, 2160, layout_source);
	BOOST_TEST(child.bounds.width == 40);
	BOOST_TEST(child.bounds.height == 68);
	layout.Apply(&root, false, 3840, 2160, 800, 600, layout_source);
	BOOST_TEST(child.bounds.x == 3);
	BOOST_TEST(child.bounds.y == 7);
	BOOST_TEST(child.bounds.width == 11);
	BOOST_TEST(child.bounds.height == 19);
}

BOOST_AUTO_TEST_CASE(wnd_layout_handles_nested_controls_and_authored_widescreen)
{
	LayoutWindow inside{{5, 5, 10, 10}, true};
	LayoutWindow card{{40, 20, 40, 40}, true, nullptr, &inside};
	LayoutWindow root{{0, 0, 800, 600}, false, nullptr, &card};
	Layout layout;
	layout.Register(&card, 800, 600);
	layout.Register(&inside, 800, 600);
	layout.Apply(&root, false, 800, 600, 1600, 900, layout_source);
	BOOST_TEST(inside.bounds.width == 15);
	BOOST_TEST(inside.bounds.height == 15);
	BOOST_TEST(inside.bounds.x == 8);
	layout.Forget(&card);
	layout.Forget(&inside);
	layout.Forget(&root);
	card.bounds = {40, 20, 40, 40};
	card.child = nullptr;
	root.bounds = {0, 0, 1600, 900};
	layout.Register(&card, 1600, 900);
	layout.Apply(&root, false, 1600, 900, 3200, 1800, layout_source);
	BOOST_TEST(card.bounds.width == 80);
	BOOST_TEST(card.bounds.height == 80);
}

BOOST_AUTO_TEST_CASE(wnd_layout_captures_siblings_once_and_retains_game_moves)
{
	LayoutWindow second{{400, 0, 400, 600}};
	LayoutWindow first{{0, 0, 400, 600}, false, &second};
	Layout layout;
	layout.Apply(&first, true, 800, 600, 800, 600, layout_source);
	first.bounds.x = 20;
	layout.Apply(&first, true, 800, 600, 1600, 1200, layout_source);
	BOOST_TEST(first.bounds.x == 40);
	BOOST_TEST(first.applications == 2);
	BOOST_TEST(second.applications == 2);
}

BOOST_AUTO_TEST_CASE(wnd_panels_and_generated_controls_share_the_authored_scale)
{
	LayoutWindow button{{10, 10, 80, 30}, true};
	LayoutWindow panel{{100, 100, 600, 400}, true, nullptr, &button};
	LayoutWindow root{{0, 0, 800, 600}, true, nullptr, &panel};
	Layout layout;
	layout.Register(&root, 800, 600);
	layout.Register(&panel, 800, 600);
	layout.Apply(&root, false, 800, 600, 1600, 900, layout_source);
	BOOST_TEST(root.bounds.width == 1600);
	BOOST_TEST(panel.bounds.width == 900);
	BOOST_TEST(panel.bounds.height == 600);
	BOOST_TEST(button.bounds.width == 120);
	BOOST_TEST(button.bounds.height == 45);
	BOOST_TEST(layout.Scale(&panel, 1600, 900) == 1.5);
	layout.Apply(&root, false, 1600, 900, 800, 600, layout_source);
	BOOST_TEST(panel.bounds.x == 100);
	BOOST_TEST(button.bounds.x == 10);
}

BOOST_AUTO_TEST_CASE(wnd_widescreen_hud_keeps_its_bottom_anchor_and_artwork_transform)
{
	LayoutWindow hud{{0, 488, 1280, 232}, true};
	Layout layout;
	layout.Register(&hud, 3840, 2160);
	layout.Apply(&hud, false, 1280, 720, 800, 600, layout_source);
	BOOST_TEST(hud.bounds.y == 455);
	BOOST_TEST(hud.bounds.height == 145);
	const auto artwork = Fit_Viewport(3840, 2160, 800, 600, LayoutAnchor::Center, LayoutAnchor::End);
	BOOST_TEST(artwork.x == 0.0);
	BOOST_TEST(artwork.y == 150.0);
	BOOST_TEST(1464 * artwork.scale + artwork.y == 455.0);
	int x = 0, y = 0;
	BOOST_REQUIRE(layout.Default_Position(&hud, 800, 600, x, y));
	BOOST_TEST(y == 455);
}

BOOST_AUTO_TEST_CASE(wnd_explicit_anchor_keeps_menu_margin_through_resize)
{
	LayoutWindow button{{540, 108, 208, 30}, true};
	LayoutWindow root{{0, 0, 800, 600}, false, nullptr, &button};
	Layout layout;
	BOOST_REQUIRE(layout.Register(&root, 800, 600, " RIGHT TOP "));
	BOOST_REQUIRE(layout.Register(&button, 800, 600));
	layout.Apply(&root, false, 800, 600, 800, 600, layout_source);
	BOOST_TEST(root.bounds.x == 0);
	BOOST_TEST(button.bounds.x == 540);
	layout.Apply(&root, false, 800, 600, 1920, 1080, layout_source);
	BOOST_TEST(root.bounds.x == 480);
	BOOST_TEST(root.bounds.width == 1440);
	BOOST_TEST(button.bounds.x == 972);
	BOOST_TEST(button.bounds.width == 374);
	BOOST_TEST(1920 - root.bounds.x - button.bounds.x - button.bounds.width == 94);
	int x = -1, y = -1;
	BOOST_REQUIRE(layout.Default_Position(&root, 1920, 1080, x, y));
	BOOST_TEST(x == 480);
	BOOST_TEST(y == 0);
	layout.Apply(&root, false, 1920, 1080, 3440, 1440, layout_source);
	BOOST_TEST(root.bounds.x == 1520);
	BOOST_TEST(3440 - root.bounds.x - button.bounds.x - button.bounds.width == 125);
	layout.Apply(&root, false, 3440, 1440, 800, 600, layout_source);
	BOOST_TEST(root.bounds.x == 0);
	BOOST_TEST(root.bounds.width == 800);
	BOOST_TEST(button.bounds.x == 540);
	BOOST_TEST(button.bounds.width == 208);
	BOOST_TEST(!layout.Register(&root, 800, 600, "RIGHT"));
	BOOST_TEST(!layout.Register(&root, 800, 600, "RIGHT SIDE"));
	BOOST_TEST(!layout.Register(&root, 800, 600, "RIGHT TOP EXTRA"));
}

BOOST_AUTO_TEST_CASE(wnd_slider_cells_and_thumb_follow_the_panel_scale)
{
	HorizontalSliderImageVisual visual;
	Layout_Horizontal_Slider_Images(visual, {0, 0, 240, 24}, 10, 1.5f, 0, 100, 50);
	BOOST_TEST(visual.box_width == 15);
	BOOST_TEST(visual.box_padding == 3);
	BOOST_TEST(visual.box_count == 13);
	const auto thumb = Layout_Slider_Thumb(240, 24, 1.5f, 13, 10, 0, 100, 50);
	BOOST_TEST(thumb.bounds.width == 20);
	BOOST_TEST(thumb.bounds.height == 24);
	BOOST_TEST(thumb.bounds.x == 110);
	BOOST_TEST(thumb.bounds.y == 15);
	const auto combo = Layout_Combo_Box_Children(216, 36, 1.5f);
	BOOST_TEST(combo.button.width == 32);
	BOOST_TEST(combo.entry.width == 184);
	BOOST_TEST(combo.entry.height == 36);
}

BOOST_AUTO_TEST_CASE(wnd_resize_keeps_the_child_geometry_computed_by_its_gadget)
{
	LayoutWindow entry{{0, 0, 123, 24}};
	LayoutWindow combo{{100, 100, 144, 24}, true, nullptr, &entry};
	LayoutWindow root{{0, 0, 800, 600}, true, nullptr, &combo};
	LayoutSource source = layout_source;
	source.context = &entry;
	source.managed_by_parent = [](void *context, void *window) noexcept { return context == window; };
	source.apply = [](void *, void *pointer, LayoutBounds bounds) noexcept {
		auto &window = *static_cast<LayoutWindow *>(pointer);
		window.bounds = bounds;
		++window.applications;
		if (window.preserve && window.child)
			window.child->bounds = {0, 0, bounds.width - 32, bounds.height};
	};
	Layout layout;
	layout.Register(&root, 800, 600);
	layout.Register(&combo, 800, 600);
	layout.Apply(&root, false, 800, 600, 1600, 900, source);
	BOOST_TEST(entry.bounds.width == 184);
	BOOST_TEST(entry.bounds.height == 36);
	BOOST_TEST(entry.applications == 0);
}

namespace
{

struct RecordingSink final
{
	std::vector<int> events;
	std::vector<Graphics::Renderer2D::ClipState> draw_clips;
	Graphics::Renderer2D *renderer = nullptr;

	static void Record_Draw(RecordingSink *recording, int event) noexcept
	{
		recording->events.push_back(event);
		if (recording->renderer != nullptr)
			recording->draw_clips.push_back(recording->renderer->Get_Clip());
	}

	static bool Draw(void *context, void *, void *, DrawList &) noexcept
	{
		Record_Draw(static_cast<RecordingSink *>(context), 1);
		return true;
	}

	static bool DrawChild(void *context, void *, void *, DrawList &) noexcept
	{
		Record_Draw(static_cast<RecordingSink *>(context), 2);
		return true;
	}

	static bool DrawBorder(void *context, void *, void *, DrawList &) noexcept
	{
		Record_Draw(static_cast<RecordingSink *>(context), 3);
		return true;
	}

};

struct TestWindow final
{
	TestWindow *next = nullptr;
	TestWindow *child = nullptr;
	int id = 0;
};

#if defined(ENGINE_UI_WND_GAME_DATA_ROOT)

}

namespace RealWNDTest
{

class RecordingCommandList final : public Graphics::CommandList
{
public:
	bool Bind_Pipeline(Graphics::RHIPipelineHandle pipeline) noexcept override
	{
		return pipeline.Is_Valid();
	}

	bool Set_Bindless_Resources(
		std::span<const Graphics::RHIBindlessResource> resources) noexcept override
	{
		bindless_count = resources.size();
		return true;
	}

	bool Set_Render_Targets(
		Graphics::RHITextureHandle color,
		Graphics::RHITextureHandle depth) noexcept override
	{
		return color.Is_Valid() && depth.Is_Valid();
	}

	bool Set_Depth_Target(Graphics::RHITextureHandle target) noexcept override
	{
		return target.Is_Valid();
	}

	bool Clear(const std::array<float, 4> &, float) noexcept override { return true; }
	bool Clear_Depth(float) noexcept override { return true; }

	bool Set_Viewport(Graphics::RHIViewport viewport) noexcept override
	{
		return viewport.width != 0 && viewport.height != 0;
	}

	bool Set_Scissor(Graphics::RHIScissorRect scissor) noexcept override
	{
		current_scissor = scissor;
		scissors.push_back(scissor);
		return scissor.width != 0 && scissor.height != 0;
	}

	bool Set_Vertex_Buffer(
		std::uint32_t,
		Graphics::RHIBufferHandle buffer,
		std::uint32_t stride,
		std::uint32_t) noexcept override
	{
		return buffer.Is_Valid() && stride == sizeof(Graphics::Renderer2DVertex);
	}

	bool Set_Index_Buffer(
		Graphics::RHIBufferHandle buffer,
		Graphics::RHIIndexFormat,
		std::uint32_t) noexcept override
	{
		return buffer.Is_Valid();
	}

	bool Draw(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept override
	{
		return false;
	}

	bool Draw_Indexed(
		std::uint32_t index_count,
		std::uint32_t first_index,
		std::int32_t = 0,
		std::uint32_t = 0,
		std::uint32_t = 0) noexcept override
	{
		if (index_count == 0)
			return false;
		draws.push_back({index_count, first_index, current_scissor});
		return true;
	}

	struct DrawRecord final
	{
		std::uint32_t index_count = 0;
		std::uint32_t first_index = 0;
		Graphics::RHIScissorRect scissor{};
	};

	std::size_t bindless_count = 0;
	std::vector<Graphics::RHIScissorRect> scissors;
	std::vector<DrawRecord> draws;
	Graphics::RHIScissorRect current_scissor{};
};

class TestSwapChain final : public Graphics::SwapChain
{
public:
	bool Is_Valid() const noexcept override { return true; }
	Graphics::RHIBackbuffer Backbuffer() const noexcept override { return {}; }
	Graphics::RHIDepthTarget Depth_Target() const noexcept override { return {}; }
	bool Resize(std::uint32_t, std::uint32_t) override { return true; }
	bool Present() noexcept override { return true; }
};

class TestDevice final : public Graphics::Device
{
public:
	bool Is_Valid() const noexcept override { return true; }

	Graphics::RHIBufferHandle Create_Buffer(const Graphics::RHIBuffer &description) override
	{
		const Graphics::RHIBufferHandle handle(++next_buffer, 1);
		if (description.usage == Graphics::RHIBufferUsage::Vertex)
			vertex_buffer = handle;
		else if (description.usage == Graphics::RHIBufferUsage::Index)
			index_buffer = handle;
		return handle;
	}

	Graphics::RHITextureHandle Create_Texture(const Graphics::RHITexture &) override
	{
		return Graphics::RHITextureHandle(++next_texture, 1);
	}

	Graphics::RHIPipelineHandle Create_Pipeline(const Graphics::RHIPipeline &) override
	{
		return Graphics::RHIPipelineHandle(++next_pipeline, 1);
	}

	Graphics::RHIPipelineHandle Create_Pipeline(
		const Graphics::RHIPipeline &,
		Graphics::RHIShaderBytecode vertex_shader,
		Graphics::RHIShaderBytecode fragment_shader) override
	{
		return vertex_shader.data.empty() || fragment_shader.data.empty()
			? Graphics::RHIPipelineHandle{}
			: Graphics::RHIPipelineHandle(++next_pipeline, 1);
	}

	Graphics::RHITextureHandle Create_Texture_Initialized(
		const Graphics::RHITexture &,
		const Graphics::RHITextureUpload &upload) override
	{
		return upload.data.empty()
			? Graphics::RHITextureHandle{}
			: Graphics::RHITextureHandle(++next_texture, 1);
	}

	bool Update_Buffer(
		Graphics::RHIBufferHandle buffer,
		std::uint32_t offset,
		std::span<const std::byte> data) noexcept override
	{
		if (!buffer.Is_Valid() || data.empty())
			return false;
		std::vector<std::byte> *destination = nullptr;
		if (buffer == vertex_buffer)
			destination = &vertex_data;
		else if (buffer == index_buffer)
			destination = &index_data;
		if (destination == nullptr)
			return false;
		if (destination->size() < offset + data.size())
			destination->resize(offset + data.size());
		std::memcpy(destination->data() + offset, data.data(), data.size());
		return true;
	}

	bool Readback_Texture(
		Graphics::RHITextureHandle texture,
		std::span<std::byte> destination,
		std::uint32_t row_pitch) noexcept override
	{
		if (!texture.Is_Valid() || row_pitch == 0 || destination.size() < row_pitch
			|| destination.size() % row_pitch != 0)
			return false;
		std::fill(destination.begin(), destination.end(), std::byte{0});
		if (vertex_data.size() % sizeof(Graphics::Renderer2DVertex) != 0
			|| index_data.size() % sizeof(std::uint32_t) != 0)
			return false;

		const std::uint32_t width = row_pitch / 4;
		const std::uint32_t height = static_cast<std::uint32_t>(destination.size() / row_pitch);
		if (width == 0 || height == 0)
			return false;
		const std::size_t vertex_count = vertex_data.size() / sizeof(Graphics::Renderer2DVertex);
		const std::size_t index_count = index_data.size() / sizeof(std::uint32_t);
		const auto read_vertex = [&](std::uint32_t index) {
			Graphics::Renderer2DVertex vertex;
			std::memcpy(&vertex,
				vertex_data.data() + static_cast<std::size_t>(index) * sizeof(vertex),
				sizeof(vertex));
			return vertex;
		};
		const auto read_index = [&](std::uint32_t index) {
			std::uint32_t value = 0;
			std::memcpy(&value,
				index_data.data() + static_cast<std::size_t>(index) * sizeof(value),
				sizeof(value));
			return value;
		};
		const auto to_x = [width](float value) {
			return (value + 1.0f) * static_cast<float>(width) * 0.5f;
		};
		const auto to_y = [height](float value) {
			return (1.0f - value) * static_cast<float>(height) * 0.5f;
		};
		for (const RecordingCommandList::DrawRecord &draw : command_list.draws) {
			if (draw.first_index > index_count
				|| draw.index_count > index_count - draw.first_index)
				return false;
			const std::uint32_t left_clip = (std::min)(draw.scissor.x, width);
			const std::uint32_t top_clip = (std::min)(draw.scissor.y, height);
			const std::uint32_t right_clip = (std::min)(
				width, left_clip + (std::min)(draw.scissor.width, width - left_clip));
			const std::uint32_t bottom_clip = (std::min)(
				height, top_clip + (std::min)(draw.scissor.height, height - top_clip));
			for (std::uint32_t offset = 0; offset + 2 < draw.index_count; offset += 3) {
				const std::uint32_t first = read_index(draw.first_index + offset);
				const std::uint32_t second = read_index(draw.first_index + offset + 1);
				const std::uint32_t third = read_index(draw.first_index + offset + 2);
				if (first >= vertex_count || second >= vertex_count || third >= vertex_count)
					return false;
				const Graphics::Renderer2DVertex vertices[3] = {
					read_vertex(first), read_vertex(second), read_vertex(third)};
				const float min_x = (std::min)({to_x(vertices[0].position[0]),
					to_x(vertices[1].position[0]), to_x(vertices[2].position[0])});
				const float max_x = (std::max)({to_x(vertices[0].position[0]),
					to_x(vertices[1].position[0]), to_x(vertices[2].position[0])});
				const float min_y = (std::min)({to_y(vertices[0].position[1]),
					to_y(vertices[1].position[1]), to_y(vertices[2].position[1])});
				const float max_y = (std::max)({to_y(vertices[0].position[1]),
					to_y(vertices[1].position[1]), to_y(vertices[2].position[1])});
				const std::uint32_t left = std::max<std::uint32_t>(
					left_clip, min_x <= 0.0f ? 0 : static_cast<std::uint32_t>(min_x));
				const std::uint32_t top = std::max<std::uint32_t>(
					top_clip, min_y <= 0.0f ? 0 : static_cast<std::uint32_t>(min_y));
				const std::uint32_t right = std::min<std::uint32_t>(
					right_clip, max_x >= width ? width : static_cast<std::uint32_t>(max_x + 0.999f));
				const std::uint32_t bottom = std::min<std::uint32_t>(
					bottom_clip, max_y >= height ? height : static_cast<std::uint32_t>(max_y + 0.999f));
				for (std::uint32_t y = top; y < bottom; ++y) {
					for (std::uint32_t x = left; x < right; ++x) {
						std::uint8_t *pixel = reinterpret_cast<std::uint8_t *>(
							destination.data() + static_cast<std::size_t>(y) * row_pitch + x * 4);
						const float alpha = std::clamp(vertices[0].color[3], 0.0f, 1.0f);
						for (std::size_t channel = 0; channel < 3; ++channel) {
							const float source = std::clamp(vertices[0].color[channel], 0.0f, 1.0f) * 255.0f;
							const float result = source * alpha + pixel[channel] * (1.0f - alpha);
							pixel[channel] = static_cast<std::uint8_t>(std::clamp(result, 0.0f, 255.0f));
						}
						const float destination_alpha = pixel[3] / 255.0f;
						pixel[3] = static_cast<std::uint8_t>(std::clamp(
							(alpha + destination_alpha * (1.0f - alpha)) * 255.0f, 0.0f, 255.0f));
					}
				}
			}
		}
		return true;
	}

	bool Destroy_Buffer(Graphics::RHIBufferHandle) noexcept override { return true; }
	bool Destroy_Texture(Graphics::RHITextureHandle) noexcept override { return true; }
	bool Destroy_Pipeline(Graphics::RHIPipelineHandle) noexcept override { return true; }
	Graphics::CommandList &Immediate_Command_List() noexcept override { return command_list; }
	Graphics::SwapChain &Get_Swap_Chain() noexcept override { return swap_chain; }
	bool Begin_Frame() noexcept override { return true; }
	bool End_Frame() noexcept override { return true; }

	RecordingCommandList command_list;
	Graphics::RHIBufferHandle vertex_buffer{};
	Graphics::RHIBufferHandle index_buffer{};
	std::vector<std::byte> vertex_data;
	std::vector<std::byte> index_data;

private:
	std::uint32_t next_buffer = 0;
	std::uint32_t next_texture = 100;
	std::uint32_t next_pipeline = 200;
	TestSwapChain swap_chain;
};

bool WriteShaderStubs(const std::filesystem::path &directory)
{
	std::error_code error;
	std::filesystem::create_directories(directory, error);
	if (error)
		return false;
	for (const char *name : {"ui_2d.vso", "ui_2d.pso"}) {
		std::ofstream file(directory / name, std::ios::binary | std::ios::trunc);
		if (!file)
			return false;
		file.write("ui2d", 4);
		if (!file)
			return false;
	}
	return true;
}

constexpr std::size_t InvalidRealWindow = static_cast<std::size_t>(-1);

struct RealWNDWindow final
{
	RealWNDWindow *next = nullptr;
	RealWNDWindow *child = nullptr;
	std::size_t next_index = InvalidRealWindow;
	std::size_t child_index = InvalidRealWindow;
	std::size_t last_child_index = InvalidRealWindow;
	Rect screen_region{};
	Graphics::Color2D draw_color{0.12f, 0.20f, 0.40f, 0.35f};
	std::string draw_callback;
	bool hidden = false;
};

struct RealWNDSource final
{
	std::vector<RealWNDWindow> windows;
	std::size_t visible_windows = 0;
	std::size_t callback_windows = 0;
	int creation_width = 800;
	int creation_height = 600;
};

std::size_t CountVisibleRealWND(
	const RealWNDWindow *window,
	bool hidden_ancestor = false) noexcept
{
	std::size_t count = 0;
	for (const RealWNDWindow *current = window; current != nullptr;
		current = current->next) {
		const bool hidden = hidden_ancestor || current->hidden;
		if (!hidden)
			++count;
		if (!hidden)
			count += CountVisibleRealWND(current->child);
	}
	return count;
}

std::string TrimRealWNDLine(std::string line)
{
	while (!line.empty() && (line.back() == '\r' || line.back() == ';'
		|| line.back() == ' ' || line.back() == '\t'))
		line.pop_back();
	std::size_t first = 0;
	while (first < line.size() && (line[first] == ' ' || line[first] == '\t'))
		++first;
	return line.substr(first);
}

bool ParseRealWNDPoint(const std::string &line, const char *marker, int &x, int &y)
{
	const std::size_t marker_position = line.find(marker);
	if (marker_position == std::string::npos)
		return false;
	std::string values = line.substr(marker_position + std::string(marker).size());
	for (char &character : values) {
		if (character == ',')
			character = ' ';
	}
	std::istringstream stream(values);
	return static_cast<bool>(stream >> x >> y);
}

bool ParseRealWNDColor(
	const std::string &line,
	const char *marker,
	Graphics::Color2D &color)
{
	const std::size_t marker_position = line.find(marker);
	if (marker_position == std::string::npos)
		return false;
	std::string values = line.substr(marker_position + std::string(marker).size());
	for (char &character : values) {
		if (character == ',')
			character = ' ';
	}
	std::istringstream stream(values);
	int red = 0;
	int green = 0;
	int blue = 0;
	int alpha = 0;
	if (!(stream >> red >> green >> blue >> alpha))
		return false;
	color = {
		static_cast<float>(red) / 255.0f,
		static_cast<float>(green) / 255.0f,
		static_cast<float>(blue) / 255.0f,
		static_cast<float>(alpha) / 255.0f};
	return true;
}

bool LoadRealWND(const std::filesystem::path &path, RealWNDSource &source)
{
	std::ifstream file(path);
	if (!file)
		return false;

	std::vector<std::size_t> parents;
	std::size_t last_root = InvalidRealWindow;
	std::string line;
	while (std::getline(file, line)) {
		line = TrimRealWNDLine(std::move(line));
		if (line == "WINDOW") {
			const std::size_t index = source.windows.size();
			source.windows.emplace_back();
			if (parents.empty()) {
				if (last_root != InvalidRealWindow)
					source.windows[last_root].next_index = index;
				last_root = index;
			} else {
				RealWNDWindow &parent = source.windows[parents.back()];
				if (parent.child_index == InvalidRealWindow)
					parent.child_index = index;
				else
					source.windows[parent.last_child_index].next_index = index;
				parent.last_child_index = index;
			}
			parents.push_back(index);
			continue;
		}
		if (line == "END") {
			if (parents.empty())
				return false;
			parents.pop_back();
			continue;
		}
		if (parents.empty())
			continue;

		RealWNDWindow &window = source.windows[parents.back()];
		int creation_width = 0;
		int creation_height = 0;
		if (ParseRealWNDPoint(line, "CREATIONRESOLUTION:", creation_width, creation_height)) {
			source.creation_width = creation_width;
			source.creation_height = creation_height;
		}
		if (line.rfind("DRAWCALLBACK", 0) == 0) {
			const std::size_t equals = line.find('=');
			if (equals != std::string::npos) {
				window.draw_callback = TrimRealWNDLine(line.substr(equals + 1));
				if (!window.draw_callback.empty() && window.draw_callback.front() == '"')
					window.draw_callback.erase(window.draw_callback.begin());
				if (!window.draw_callback.empty() && window.draw_callback.back() == '"')
					window.draw_callback.pop_back();
			}
		} else if (line.rfind("STATUS", 0) == 0) {
			window.hidden = line.find("HIDDEN") != std::string::npos;
		} else if (line.rfind("ENABLEDDRAWDATA", 0) == 0) {
			ParseRealWNDColor(line, "COLOR:", window.draw_color);
		} else {
			int x = 0;
			int y = 0;
			if (ParseRealWNDPoint(line, "UPPERLEFT:", x, y)) {
				window.screen_region.left = x;
				window.screen_region.top = y;
			} else if (ParseRealWNDPoint(line, "BOTTOMRIGHT:", x, y)) {
				window.screen_region.right = x;
				window.screen_region.bottom = y;
			}
		}
	}

	if (!parents.empty() || source.windows.empty())
		return false;

	for (RealWNDWindow &window : source.windows) {
		window.next = window.next_index == InvalidRealWindow
			? nullptr : &source.windows[window.next_index];
		window.child = window.child_index == InvalidRealWindow
			? nullptr : &source.windows[window.child_index];
		if (!window.draw_callback.empty())
			++source.callback_windows;
	}
	source.visible_windows = CountVisibleRealWND(source.windows.data());
	return true;
}

struct RealWNDRenderContext final
{
	std::size_t extracted_windows = 0;
	Graphics::Renderer2D *renderer = nullptr;
	float scale_x = 1.0f;
	float scale_y = 1.0f;
};

bool ExtractRealWNDWindow(void *context, void *window_pointer, void *, DrawList &draw_list) noexcept
{
	RealWNDRenderContext *render_context = static_cast<RealWNDRenderContext *>(context);
	++render_context->extracted_windows;
	const RealWNDWindow &window = *static_cast<const RealWNDWindow *>(window_pointer);
	const Rect rectangle = window.screen_region;
	return draw_list.Add_Rect(
		{static_cast<float>(rectangle.left) * render_context->scale_x,
			static_cast<float>(rectangle.top) * render_context->scale_y,
			static_cast<float>(rectangle.right) * render_context->scale_x,
			static_cast<float>(rectangle.bottom) * render_context->scale_y},
		window.draw_color);
}

bool DescribeRealWNDWindow(void *context, void *window_pointer, RenderNode &node) noexcept
{
	const RealWNDWindow &window = *static_cast<RealWNDWindow *>(window_pointer);
	node.extract = &ExtractRealWNDWindow;
	node.extract_context = context;
	node.screen_region = window.screen_region;
	if (window.hidden)
		node.flags |= static_cast<std::uint32_t>(WindowFlag::Hidden);
	return true;
}

void *NextRealWNDWindow(void *, void *window) noexcept
{
	return static_cast<RealWNDWindow *>(window)->next;
}

void *ChildRealWNDWindow(void *, void *window) noexcept
{
	return static_cast<RealWNDWindow *>(window)->child;
}

void RunRealWNDLayout(const std::filesystem::path &path, const char *name)
{
	RealWNDSource source;
	BOOST_REQUIRE_MESSAGE(LoadRealWND(path, source),
		"unable to load configured WND layout " << path.string());
	BOOST_REQUIRE_MESSAGE(source.windows.size() > 8,
		name << " did not contain a representative WND hierarchy");
	BOOST_REQUIRE_MESSAGE(source.callback_windows > 0,
		name << " did not contain draw callbacks");

	std::error_code error;
	const std::filesystem::path shader_directory =
		std::filesystem::temp_directory_path(error) / "generals_ui_wnd_test";
	BOOST_REQUIRE(!error);
	BOOST_REQUIRE(WriteShaderStubs(shader_directory));
	TestDevice device;
	Graphics::Renderer2D graphics_renderer;
	BOOST_REQUIRE(graphics_renderer.Initialize(device, shader_directory, 4096, 6144, 128));
	graphics_renderer.Begin(3840, 2160);
	RealWNDRenderContext context{0, &graphics_renderer};
	RenderList list(source.windows.size());
	BOOST_REQUIRE(Build_Render_List(list, source.windows.data(),
		{&context, &NextRealWNDWindow, &ChildRealWNDWindow, &DescribeRealWNDWindow}));
	BOOST_CHECK_EQUAL(list.Size(), source.windows.size());

	Renderer renderer;
	BOOST_REQUIRE(renderer.Render(list, graphics_renderer));
	BOOST_CHECK(graphics_renderer.Has_Draws());
	BOOST_CHECK(graphics_renderer.Vertex_Count() > 0);
	BOOST_CHECK(graphics_renderer.Batch_Count() > 0);
	BOOST_REQUIRE(graphics_renderer.Execute(
		device,
		device.command_list,
		Graphics::RHITextureHandle(900, 1),
		Graphics::RHITextureHandle(901, 1),
		{0, 0, 3840, 2160, 0.0f, 1.0f}));
	BOOST_CHECK(!device.command_list.scissors.empty());
	graphics_renderer.Shutdown();
	BOOST_TEST_MESSAGE(name << " windows=" << source.windows.size()
		<< " visible=" << source.visible_windows
		<< " extracted=" << context.extracted_windows);
	BOOST_CHECK_EQUAL(context.extracted_windows, source.visible_windows);
}

#if defined(ENGINE_UI_WND_VISUAL_REGRESSION)

struct RealWNDVisualContext final
{
	RenderList list;
	Renderer wnd_renderer;
	Graphics::Renderer2D graphics_renderer;
};

bool RenderRealWNDVisual(
	Graphics::Device &device,
	Graphics::CommandList &commands,
	Graphics::RHITextureHandle color_target,
	Graphics::RHITextureHandle depth_target,
	Graphics::RHIViewport viewport,
	void *context) noexcept
{
	RealWNDVisualContext &visual = *static_cast<RealWNDVisualContext *>(context);
	visual.graphics_renderer.Begin(viewport.width, viewport.height);
	return visual.wnd_renderer.Render(visual.list, visual.graphics_renderer)
		&& visual.graphics_renderer.Execute(
			device, commands, color_target, depth_target, viewport);
}

void RunRealWNDGolden(
	const std::filesystem::path &path,
	const char *scene_name,
	const char *description)
{
	RealWNDSource source;
	BOOST_REQUIRE_MESSAGE(LoadRealWND(path, source),
		"unable to load configured WND layout " << path.string());
	BOOST_REQUIRE(source.creation_width > 0);
	BOOST_REQUIRE(source.creation_height > 0);

	std::error_code error;
	const std::filesystem::path shader_directory =
		std::filesystem::path(ENGINE_UI_WND_SHADER_DIRECTORY);
	#if defined(ENGINE_UI_WND_TEST_SHADER_STUBS)
	BOOST_REQUIRE(WriteShaderStubs(shader_directory));
	#else
	BOOST_REQUIRE_MESSAGE(
		std::filesystem::exists(shader_directory / "ui_2d.vso", error)
			&& !error,
		"missing UI shader directory " << shader_directory.string());
	#endif

	TestDevice device;
	RealWNDRenderContext extraction_context{
		0,
		nullptr,
		800.0f / static_cast<float>(source.creation_width),
		600.0f / static_cast<float>(source.creation_height)};
	RealWNDVisualContext visual;
	visual.list = RenderList(source.windows.size());
	BOOST_REQUIRE(visual.graphics_renderer.Initialize(
		device, shader_directory, 16384, 24576, 256));
	BOOST_REQUIRE(Build_Render_List(
		visual.list,
		source.windows.data(),
		{&extraction_context, &NextRealWNDWindow, &ChildRealWNDWindow,
			&DescribeRealWNDWindow}));

	Graphics::VisualRegressionHarness harness({
		800,
		600,
		1,
		std::filesystem::path(ENGINE_UI_WND_VISUAL_REFERENCE_DIRECTORY),
		std::filesystem::path(ENGINE_UI_WND_VISUAL_FAILURE_DIRECTORY)});
	const Graphics::VisualComparisonResult result = harness.Run(
		device,
		scene_name,
		&RenderRealWNDVisual,
		&visual);
	BOOST_CHECK_MESSAGE(result.expected_loaded,
		"missing " << description << " WND golden image");
	BOOST_CHECK_MESSAGE(result.matched,
		description << " WND golden image mismatch");
	visual.graphics_renderer.Shutdown();
}

BOOST_AUTO_TEST_CASE(renders_real_generals_main_menu_wnd_to_a_golden_image)
{
	RunRealWNDGolden(
		std::filesystem::path(ENGINE_UI_WND_GAME_DATA_ROOT)
			/ "Window" / "Menus" / "MainMenu.wnd",
		"generals_main_menu_wnd",
		"Generals MainMenu");
}

BOOST_AUTO_TEST_CASE(renders_real_generals_single_player_wnd_to_a_golden_image)
{
	RunRealWNDGolden(
		std::filesystem::path(ENGINE_UI_WND_GAME_DATA_ROOT)
			/ "Window" / "Menus" / "SinglePlayerMenu.wnd",
		"generals_single_player_menu_wnd",
		"Generals SinglePlayerMenu");
}

BOOST_AUTO_TEST_CASE(renders_real_generals_skirmish_options_wnd_to_a_golden_image)
{
	RunRealWNDGolden(
		std::filesystem::path(ENGINE_UI_WND_GAME_DATA_ROOT)
			/ "Window" / "Menus" / "SkirmishGameOptionsMenu.wnd",
		"generals_skirmish_options_menu_wnd",
		"Generals SkirmishGameOptionsMenu");
}

#endif

}

#endif

#if defined(ENGINE_UI_WND_GAME_DATA_ROOT)
namespace
{
#endif

bool DrawTreeNode(void *context, void *window, void *, DrawList &) noexcept
{
	static_cast<RecordingSink *>(context)->events.push_back(
		static_cast<TestWindow *>(window)->id);
	return true;
}

void *NextTestWindow(void *, void *window) noexcept
{
	return static_cast<TestWindow *>(window)->next;
}

void *ChildTestWindow(void *, void *window) noexcept
{
	return static_cast<TestWindow *>(window)->child;
}

bool DescribeTestWindow(void *context, void *window, RenderNode &node) noexcept
{
	node.extract = &DrawTreeNode;
	node.extract_context = context;
	return true;
}

}

BOOST_AUTO_TEST_CASE(builds_a_render_list_from_an_opaque_wnd_tree)
{
	TestWindow first{nullptr, nullptr, 1};
	TestWindow child{nullptr, nullptr, 2};
	TestWindow second{nullptr, &child, 3};
	first.next = &second;

	RenderList list(3);
	RecordingSink recording;
	BOOST_REQUIRE(Build_Render_List(list, &first,
		{&recording, &NextTestWindow, &ChildTestWindow, &DescribeTestWindow}));
	BOOST_CHECK_EQUAL(list.Size(), 3);
	BOOST_CHECK(list.Root_Head() != Invalid_Node);
	BOOST_CHECK(list.Root_Tail() != Invalid_Node);

	Renderer renderer;
	BOOST_REQUIRE(renderer.Render(list, Graphics::Get_Renderer2D()));
	BOOST_REQUIRE_EQUAL(recording.events.size(), 3);
	BOOST_CHECK(recording.events[0] == 3);
	BOOST_CHECK(recording.events[1] == 2);
	BOOST_CHECK(recording.events[2] == 1);
}

BOOST_AUTO_TEST_CASE(preserves_wnd_parent_first_and_reverse_sibling_order)
{
	RenderList list(3);
	RecordingSink recording;
	RenderNode root;
	root.window = &recording;
	root.extract = &RecordingSink::Draw;
	root.extract_context = &recording;
	root.extract_border = &RecordingSink::DrawBorder;
	root.extract_context = &recording;
	root.flags = WindowFlag::Border | WindowFlag::BorderBeforeChildren;

	NodeIndex root_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(root, root_index));

	RenderNode first;
	first.window = &recording;
	first.extract = &RecordingSink::DrawChild;
	first.extract_context = &recording;
	NodeIndex first_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(first, first_index));

	RenderNode second = first;
	NodeIndex second_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(second, second_index));

	list.Get_Node(root_index)->first_child = first_index;
	list.Get_Node(root_index)->last_child = second_index;
	list.Get_Node(first_index)->next_sibling = second_index;
	list.Get_Node(second_index)->previous_sibling = first_index;
	list.Set_Roots(root_index, root_index);

	Renderer renderer;
	BOOST_REQUIRE(renderer.Render(list, Graphics::Get_Renderer2D()));
	BOOST_REQUIRE_EQUAL(recording.events.size(), 4);
	BOOST_CHECK(recording.events[0] == 1);
	BOOST_CHECK(recording.events[1] == 3);
	BOOST_CHECK(recording.events[2] == 2);
	BOOST_CHECK(recording.events[3] == 2);
}

BOOST_AUTO_TEST_CASE(emits_each_layer_once_without_repeating_descendants)
{
	RenderList list(3);
	RecordingSink recording;

	RenderNode below;
	below.window = &recording;
	below.extract = &RecordingSink::Draw;
	below.extract_context = &recording;
	below.layer = Layer::Below;
	NodeIndex below_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(below, below_index));

	RenderNode normal;
	normal.window = &recording;
	normal.extract = &RecordingSink::DrawChild;
	normal.extract_context = &recording;
	normal.layer = Layer::Normal;
	NodeIndex normal_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(normal, normal_index));

	RenderNode above;
	above.window = &recording;
	above.extract = &RecordingSink::DrawBorder;
	above.extract_context = &recording;
	above.layer = Layer::Above;
	NodeIndex above_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(above, above_index));

	list.Get_Node(below_index)->next_sibling = normal_index;
	list.Get_Node(normal_index)->previous_sibling = below_index;
	list.Get_Node(normal_index)->next_sibling = above_index;
	list.Get_Node(above_index)->previous_sibling = normal_index;
	list.Set_Roots(below_index, above_index);

	Renderer renderer;
	BOOST_REQUIRE(renderer.Render(list, Graphics::Get_Renderer2D()));
	BOOST_REQUIRE_EQUAL(recording.events.size(), 3);
	BOOST_CHECK(recording.events[0] == 1);
	BOOST_CHECK(recording.events[1] == 2);
	BOOST_CHECK(recording.events[2] == 3);
}

BOOST_AUTO_TEST_CASE(hidden_parent_suppresses_its_children)
{
	RenderList list(3);
	RecordingSink recording;
	RenderNode parent;
	parent.window = &recording;
	parent.extract = &RecordingSink::Draw;
	parent.extract_context = &recording;
	parent.flags = static_cast<std::uint32_t>(WindowFlag::Hidden);
	NodeIndex parent_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(parent, parent_index));

	RenderNode child;
	child.window = &recording;
	child.extract = &RecordingSink::DrawChild;
	child.extract_context = &recording;
	NodeIndex child_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(child, child_index));
	list.Get_Node(parent_index)->first_child = child_index;
	list.Get_Node(parent_index)->last_child = child_index;
	list.Set_Roots(parent_index, parent_index);

	Renderer renderer;
	BOOST_REQUIRE(renderer.Render(list, Graphics::Get_Renderer2D()));
	BOOST_CHECK(recording.events.empty());
}

BOOST_AUTO_TEST_CASE(intersects_nested_window_clip_rectangles)
{
	RenderList list(3);
	RecordingSink recording;
	recording.renderer = &Graphics::Get_Renderer2D();
	RenderNode root;
	root.window = &recording;
	root.extract = &RecordingSink::Draw;
	root.extract_context = &recording;
	root.flags = static_cast<std::uint32_t>(WindowFlag::ClipChildren);
	root.screen_region = {10, 20, 110, 120};
	NodeIndex root_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(root, root_index));

	RenderNode child;
	child.window = &recording;
	child.extract = &RecordingSink::DrawChild;
	child.extract_context = &recording;
	child.flags = static_cast<std::uint32_t>(WindowFlag::ClipChildren);
	child.screen_region = {50, 0, 150, 80};
	NodeIndex child_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(child, child_index));

	RenderNode grandchild;
	grandchild.window = &recording;
	grandchild.extract = &RecordingSink::DrawBorder;
	grandchild.extract_context = &recording;
	NodeIndex grandchild_index = Invalid_Node;
	BOOST_REQUIRE(list.Add_Node(grandchild, grandchild_index));
	list.Get_Node(root_index)->first_child = child_index;
	list.Get_Node(root_index)->last_child = child_index;
	list.Get_Node(child_index)->first_child = grandchild_index;
	list.Get_Node(child_index)->last_child = grandchild_index;
	list.Set_Roots(root_index, root_index);

	Renderer renderer;
	BOOST_REQUIRE(renderer.Render(list, Graphics::Get_Renderer2D()));
	BOOST_REQUIRE_EQUAL(recording.draw_clips.size(), 3);
	BOOST_CHECK(!recording.draw_clips[0].enabled);
	BOOST_CHECK(recording.draw_clips[1].enabled);
	BOOST_CHECK(recording.draw_clips[1].rectangle.left == 10.0f
		&& recording.draw_clips[1].rectangle.top == 20.0f
		&& recording.draw_clips[1].rectangle.right == 110.0f
		&& recording.draw_clips[1].rectangle.bottom == 120.0f);
	BOOST_CHECK(recording.draw_clips[2].enabled);
	BOOST_CHECK(recording.draw_clips[2].rectangle.left == 50.0f
		&& recording.draw_clips[2].rectangle.top == 20.0f
		&& recording.draw_clips[2].rectangle.right == 110.0f
		&& recording.draw_clips[2].rectangle.bottom == 80.0f);
	BOOST_CHECK(!Graphics::Get_Renderer2D().Get_Clip().enabled);
}

BOOST_AUTO_TEST_CASE(resolves_visual_state_with_selection_priority)
{
	BOOST_CHECK(Resolve_Visual_State(false, true, true) == VisualState::Disabled);
	BOOST_CHECK(Resolve_Visual_State(true, true, true) == VisualState::Selected);
	BOOST_CHECK(Resolve_Visual_State(true, true, false) == VisualState::Highlighted);
	BOOST_CHECK(Resolve_Visual_State(true, false, false) == VisualState::Normal);
}

BOOST_AUTO_TEST_CASE(builds_contiguous_draw_data_in_submission_order)
{
	DrawList draw_list(4);
	BOOST_REQUIRE(draw_list.Add_Rect({1.0f, 2.0f, 20.0f, 30.0f}, {1.0f, 0.0f, 0.0f, 0.5f}));
	BOOST_REQUIRE(draw_list.Add_Line({2.0f, 3.0f}, {18.0f, 28.0f}, 2.0f,
		{0.0f, 1.0f, 0.0f, 1.0f}));
	BOOST_REQUIRE(draw_list.Add_Clock({3.0f, 4.0f, 19.0f, 29.0f}, 50,
		{0.0f, 0.0f, 1.0f, 1.0f}, true));
	const ImageRef image{Assets::TextureAssetHandle(27, 3), {0.1f, 0.2f, 0.8f, 0.9f}};
	BOOST_REQUIRE(draw_list.Add_Image(image, {4.0f, 5.0f, 18.0f, 28.0f}));
	BOOST_CHECK_EQUAL(draw_list.Size(), 4);
	BOOST_CHECK(draw_list.Commands()[0].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK(draw_list.Commands()[1].kind == DrawCommandKind::Line);
	BOOST_CHECK(draw_list.Commands()[2].kind == DrawCommandKind::Clock);
	BOOST_CHECK(draw_list.Commands()[3].kind == DrawCommandKind::Image);
	BOOST_CHECK(draw_list.Commands()[3].image.texture == image.texture);
	BOOST_CHECK(draw_list.Commands()[3].image.uv.left == image.uv.left);
	BOOST_CHECK_CLOSE(draw_list.Commands()[0].color.alpha, 0.5f, 0.001f);
	BOOST_CHECK_CLOSE(draw_list.Commands()[1].color.green, 1.0f, 0.001f);
	BOOST_CHECK_CLOSE(draw_list.Commands()[3].color.red, 1.0f, 0.001f);
	BOOST_CHECK(!draw_list.Add_Rect({}, {}));
}

BOOST_AUTO_TEST_CASE(builds_push_button_layers_in_visual_order)
{
	DrawList draw_list(16);
	PushButtonVisual button;
	button.rectangle = {10.0f, 20.0f, 110.0f, 60.0f};
	button.has_border = true;
	button.border_color = {1.0f, 0.0f, 0.0f, 1.0f};
	button.has_fill = true;
	button.fill_color = {0.0f, 1.0f, 0.0f, 1.0f};
	button.has_overlay = true;
	button.overlay_image = {Assets::TextureAssetHandle(8, 1), {0.0f, 0.0f, 1.0f, 1.0f}};
	button.has_clock = true;
	button.clock_percent = 50;
	button.has_extra_border = true;
	button.extra_border = {9.0f, 19.0f, 111.0f, 61.0f};
	button.extra_border_color = {0.0f, 0.0f, 1.0f, 1.0f};

	BOOST_REQUIRE(Add_Push_Button_Background(draw_list, button));
	BOOST_REQUIRE(Add_Push_Button_Overlays(draw_list, button));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 5);
	BOOST_CHECK(draw_list.Commands()[0].kind == DrawCommandKind::Outline);
	BOOST_CHECK(draw_list.Commands()[1].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK(draw_list.Commands()[2].kind == DrawCommandKind::Image);
	BOOST_CHECK(draw_list.Commands()[3].kind == DrawCommandKind::Clock);
	BOOST_CHECK(draw_list.Commands()[4].kind == DrawCommandKind::Outline);
}

BOOST_AUTO_TEST_CASE(builds_checkbox_layers_and_checked_mark_in_order)
{
	CheckBoxVisual visual;
	visual.rectangle = {10.0f, 20.0f, 110.0f, 60.0f};
	visual.box_rectangle = {14.0f, 30.0f, 27.0f, 50.0f};
	visual.has_background_fill = true;
	visual.background_fill = {0.1f, 0.2f, 0.3f, 1.0f};
	visual.has_background_border = true;
	visual.background_border = {0.8f, 0.7f, 0.6f, 1.0f};
	visual.has_box_fill = true;
	visual.box_fill = {0.3f, 0.4f, 0.5f, 1.0f};
	visual.has_box_border = true;
	visual.box_border = {0.9f, 0.8f, 0.7f, 1.0f};
	visual.checked = true;
	visual.check_color = {1.0f, 1.0f, 1.0f, 1.0f};

	DrawList draw_list(8);
	BOOST_REQUIRE(Add_Check_Box_Visual(draw_list, visual));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 6);
	BOOST_CHECK(draw_list.Commands()[0].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK(draw_list.Commands()[1].kind == DrawCommandKind::Outline);
	BOOST_CHECK(draw_list.Commands()[2].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK(draw_list.Commands()[3].kind == DrawCommandKind::Outline);
	BOOST_CHECK(draw_list.Commands()[4].kind == DrawCommandKind::Line);
	BOOST_CHECK(draw_list.Commands()[5].kind == DrawCommandKind::Line);
}

BOOST_AUTO_TEST_CASE(builds_radio_button_panels_in_order)
{
	RadioButtonVisual visual;
	visual.rectangle = {10.0f, 20.0f, 110.0f, 60.0f};
	visual.has_background_fill = true;
	visual.background_fill = {0.1f, 0.2f, 0.3f, 1.0f};
	visual.has_background_border = true;
	visual.background_border = {0.8f, 0.7f, 0.6f, 1.0f};
	visual.left_box_rectangle = {11.0f, 21.0f, 49.0f, 59.0f};
	visual.right_box_rectangle = {71.0f, 21.0f, 109.0f, 59.0f};
	visual.has_box_fill = true;
	visual.box_fill = {0.3f, 0.4f, 0.5f, 1.0f};

	DrawList draw_list(8);
	BOOST_REQUIRE(Add_Radio_Button_Visual(draw_list, visual));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 6);
	BOOST_CHECK(draw_list.Commands()[0].kind == DrawCommandKind::Outline);
	BOOST_CHECK(draw_list.Commands()[1].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK(draw_list.Commands()[2].kind == DrawCommandKind::Line);
	BOOST_CHECK(draw_list.Commands()[3].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK(draw_list.Commands()[4].kind == DrawCommandKind::Line);
	BOOST_CHECK(draw_list.Commands()[5].kind == DrawCommandKind::Rectangle);
}

BOOST_AUTO_TEST_CASE(repeats_radio_button_middle_image_with_a_partial_uv)
{
	RadioButtonVisual visual;
	visual.segmented_images = true;
	visual.left_image = {Assets::TextureAssetHandle(1, 1), {0.0f, 0.0f, 0.25f, 1.0f}};
	visual.middle_image = {Assets::TextureAssetHandle(2, 1), {0.25f, 0.0f, 0.75f, 1.0f}};
	visual.right_image = {Assets::TextureAssetHandle(3, 1), {0.75f, 0.0f, 1.0f, 1.0f}};
	visual.left_image_rectangle = {0.0f, 0.0f, 10.0f, 20.0f};
	visual.middle_image_rectangle = {10.0f, 0.0f, 30.0f, 20.0f};
	visual.right_image_rectangle = {30.0f, 0.0f, 40.0f, 20.0f};
	visual.middle_width = 8.0f;

	DrawList draw_list(8);
	BOOST_REQUIRE(Add_Radio_Button_Visual(draw_list, visual));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 5);
	BOOST_CHECK(draw_list.Commands()[0].kind == DrawCommandKind::Image);
	BOOST_CHECK_EQUAL(draw_list.Commands()[0].rectangle.right, 18.0f);
	BOOST_CHECK_EQUAL(draw_list.Commands()[2].rectangle.right, 30.0f);
	BOOST_CHECK_CLOSE(draw_list.Commands()[2].image.uv.right, 0.5f, 0.001f);
	BOOST_CHECK(draw_list.Commands()[3].image.texture == visual.left_image.texture);
	BOOST_CHECK(draw_list.Commands()[4].image.texture == visual.right_image.texture);
}

BOOST_AUTO_TEST_CASE(builds_progress_bar_layers_and_progress_highlights)
{
	ProgressBarVisual visual;
	visual.rectangle = {0.0f, 0.0f, 100.0f, 20.0f};
	visual.progress = 50;
	visual.has_background_border = true;
	visual.background_border = {0.1f, 0.1f, 0.1f, 1.0f};
	visual.has_background_fill = true;
	visual.background_fill = {0.2f, 0.2f, 0.2f, 1.0f};
	visual.has_bar_border = true;
	visual.bar_border = {0.3f, 0.3f, 0.3f, 1.0f};
	visual.has_bar_fill = true;
	visual.bar_fill = {0.4f, 0.4f, 0.4f, 1.0f};

	DrawList draw_list(8);
	BOOST_REQUIRE(Add_Progress_Bar_Visual(draw_list, visual));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 6);
	BOOST_CHECK(draw_list.Commands()[0].kind == DrawCommandKind::Outline);
	BOOST_CHECK(draw_list.Commands()[1].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK(draw_list.Commands()[2].kind == DrawCommandKind::Outline);
	BOOST_CHECK(draw_list.Commands()[3].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK(draw_list.Commands()[4].kind == DrawCommandKind::Line);
	BOOST_CHECK(draw_list.Commands()[5].kind == DrawCommandKind::Line);
}

BOOST_AUTO_TEST_CASE(builds_slider_background_layers)
{
	SliderVisual visual;
	visual.rectangle = {2.0f, 3.0f, 22.0f, 13.0f};
	visual.has_background_border = true;
	visual.background_border = {1.0f, 0.0f, 0.0f, 1.0f};
	visual.has_background_fill = true;
	visual.background_fill = {0.0f, 1.0f, 0.0f, 1.0f};

	DrawList draw_list(2);
	BOOST_REQUIRE(Add_Slider_Visual(draw_list, visual));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 2);
	BOOST_CHECK(draw_list.Commands()[0].kind == DrawCommandKind::Outline);
	BOOST_CHECK(draw_list.Commands()[1].kind == DrawCommandKind::Rectangle);
}

BOOST_AUTO_TEST_CASE(builds_horizontal_slider_boxes_in_order)
{
	HorizontalSliderImageVisual visual;
	visual.highlighted_image = {Assets::TextureAssetHandle(1, 1), {0.0f, 0.0f, 1.0f, 1.0f}};
	visual.selected_image = {Assets::TextureAssetHandle(2, 1), {0.0f, 0.0f, 1.0f, 1.0f}};
	visual.unselected_image = {Assets::TextureAssetHandle(3, 1), {0.0f, 0.0f, 1.0f, 1.0f}};
	visual.origin = {10.0f, 20.0f};
	visual.box_width = 4.0f;
	visual.box_count = 3;
	visual.selected_box_count = 2;
	visual.highlighted = true;

	DrawList draw_list(8);
	BOOST_REQUIRE(Add_Horizontal_Slider_Image_Visual(draw_list, visual));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 7);
	BOOST_CHECK(draw_list.Commands()[0].image.texture == visual.highlighted_image.texture);
	BOOST_CHECK(draw_list.Commands()[4].image.texture == visual.selected_image.texture);
	BOOST_CHECK(draw_list.Commands()[6].image.texture == visual.unselected_image.texture);
}

BOOST_AUTO_TEST_CASE(places_static_text_and_emits_its_background_in_order)
{
	StaticTextVisual visual;
	visual.rectangle = {10.0f, 20.0f, 110.0f, 60.0f};
	visual.has_border = true;
	visual.border_color = {1.0f, 0.0f, 0.0f, 1.0f};
	visual.has_fill = true;
	visual.fill_color = {0.0f, 1.0f, 0.0f, 1.0f};
	visual.centered = true;
	visual.centered_vertically = true;
	visual.text_width = 20;
	visual.text_height = 10;

	const Graphics::Point2D position = Get_Static_Text_Position(visual);
	BOOST_CHECK_EQUAL(position.x, 50.0f);
	BOOST_CHECK_EQUAL(position.y, 35.0f);

	DrawList draw_list(4);
	BOOST_REQUIRE(Add_Static_Text_Background(draw_list, visual));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 2);
	BOOST_CHECK(draw_list.Commands()[0].kind == DrawCommandKind::Outline);
	BOOST_CHECK(draw_list.Commands()[1].kind == DrawCommandKind::Rectangle);
}

BOOST_AUTO_TEST_CASE(retains_text_clip_in_draw_data)
{
	FontFace font;
	const std::u16string text = u"text";
	DrawList draw_list(1);
	BOOST_REQUIRE(draw_list.Add_Text(
		&font,
		nullptr,
		reinterpret_cast<const std::uint16_t *>(text.c_str()),
		4.0f,
		5.0f,
		{},
		{},
		true,
		{1.0f, 2.0f, 30.0f, 40.0f}));
	BOOST_CHECK(draw_list.Commands()[0].text_clip);
	BOOST_CHECK_EQUAL(draw_list.Commands()[0].text_clip_rectangle.right, 30.0f);
}

namespace
{

struct TestFontMetrics final
{
	int spacing = 5;
	int height = 10;
	int overlap = 1;
};

int TestSpacing(const void *context, std::uint16_t) noexcept
{
	return static_cast<const TestFontMetrics *>(context)->spacing;
}

int TestHeight(const void *context) noexcept
{
	return static_cast<const TestFontMetrics *>(context)->height;
}

int TestOverlap(const void *context) noexcept
{
	return static_cast<const TestFontMetrics *>(context)->overlap;
}

}

BOOST_AUTO_TEST_CASE(layouts_wrapped_text_and_marks_the_requested_hotkey)
{
	const TestFontMetrics metrics;
	const TextMetrics source{&metrics, &TestSpacing, &TestHeight, &TestOverlap};
	const std::u16string text = u"AB &CD";
	TextLayout layout;
	BOOST_REQUIRE(layout.Build(source, reinterpret_cast<const std::uint16_t *>(text.c_str()),
		{15, false, true, static_cast<std::uint16_t>('C'), true}));
	BOOST_REQUIRE_EQUAL(layout.Lines().size(), 2);
	BOOST_REQUIRE_EQUAL(layout.Placements().size(), 4);
	BOOST_CHECK(layout.Placements()[2].character == static_cast<std::uint16_t>('C'));
	BOOST_CHECK(layout.Placements()[2].hotkey);
	BOOST_CHECK_EQUAL(layout.Height(), 20);
}

namespace
{

struct ListBoxTestContext final
{
	int cells = 0;
};

bool QueryListBoxRow(
	void *, std::uint32_t row, ListBoxRowVisual &visual) noexcept
{
	visual.rectangle = {0.0f, static_cast<float>(row * 10), 100.0f, static_cast<float>(row * 10 + 10)};
	visual.selected = row == 0;
	return true;
}

bool QuerySmallListBoxRow(
	void *, std::uint32_t, ListBoxRowVisual &visual) noexcept
{
	visual.rectangle = {0.0f, 0.0f, 20.0f, 10.0f};
	visual.selected = true;
	return true;
}

bool EmitListBoxCell(
	void *context, DrawList &draw_list, std::uint32_t,
	std::uint32_t, Graphics::Rect2D row, Graphics::Rect2D clip) noexcept
{
	++static_cast<ListBoxTestContext *>(context)->cells;
	return draw_list.Add_Rect(
		{clip.left, row.top, clip.right, clip.bottom}, {1.0f, 1.0f, 1.0f, 1.0f});
}

}

BOOST_AUTO_TEST_CASE(extracts_list_box_rows_with_selection_before_cells)
{
	ListBoxTestContext context;
	ListBoxVisual visual;
	visual.clip_rectangle = {0.0f, 0.0f, 100.0f, 15.0f};
	visual.row_count = 2;
	visual.column_count = 1;
	visual.selection.has_border = true;
	visual.selection.border = {1.0f, 0.0f, 0.0f, 1.0f};
	visual.selection.has_fill = true;
	visual.selection.fill = {0.0f, 1.0f, 0.0f, 1.0f};
	visual.context = &context;
	visual.query_row = &QueryListBoxRow;
	visual.emit_cell = &EmitListBoxCell;

	DrawList draw_list(8);
	BOOST_REQUIRE(Add_List_Box_Visual(draw_list, visual));
	BOOST_REQUIRE_EQUAL(context.cells, 2);
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 4);
	BOOST_CHECK(draw_list.Commands()[0].kind == DrawCommandKind::Outline);
	BOOST_CHECK(draw_list.Commands()[1].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK(draw_list.Commands()[2].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK(draw_list.Commands()[3].kind == DrawCommandKind::Rectangle);
	BOOST_CHECK_EQUAL(draw_list.Commands()[3].rectangle.bottom, 15.0f);
}

BOOST_AUTO_TEST_CASE(builds_text_entry_background_and_clipped_text_data)
{
	TextEntryVisual background;
	background.rectangle = {10.0f, 20.0f, 110.0f, 50.0f};
	background.has_border = true;
	background.border = {1.0f, 0.0f, 0.0f, 1.0f};
	background.has_fill = true;
	background.fill = {0.0f, 1.0f, 0.0f, 1.0f};
	DrawList draw_list(4);
	BOOST_REQUIRE(Add_Text_Entry_Background(draw_list, background));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 2);

	FontFace font;
	const std::u16string text = u"entry";
	TextEntryTextVisual text_visual;
	text_visual.font = &font;
	text_visual.text = reinterpret_cast<const std::uint16_t *>(text.c_str());
	text_visual.clip_rectangle = {10.0f, 20.0f, 100.0f, 40.0f};
	text_visual.x = 15.0f;
	text_visual.y = 22.0f;
	text_visual.visible_width = 85.0f;
	text_visual.text_width = 25;
	BOOST_REQUIRE(Add_Text_Entry_Text(draw_list, text_visual));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 3);
	BOOST_CHECK(draw_list.Commands()[2].kind == DrawCommandKind::Text);
	BOOST_CHECK(draw_list.Commands()[2].text_clip);
}

BOOST_AUTO_TEST_CASE(crops_image_uvs_when_a_cell_is_clipped)
{
	const ImageRef image{Assets::TextureAssetHandle(4, 1), {0.0f, 0.0f, 1.0f, 1.0f}};
	DrawList draw_list(1);
	BOOST_REQUIRE(Add_Clipped_Image(
		draw_list, image, {0.0f, 0.0f, 100.0f, 100.0f}, {25.0f, 10.0f, 75.0f, 90.0f}));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 1);
	BOOST_CHECK_CLOSE(draw_list.Commands()[0].image.uv.left, 0.25f, 0.001f);
	BOOST_CHECK_CLOSE(draw_list.Commands()[0].image.uv.right, 0.75f, 0.001f);
	BOOST_CHECK_CLOSE(draw_list.Commands()[0].image.uv.top, 0.1f, 0.001f);
	BOOST_CHECK_CLOSE(draw_list.Commands()[0].image.uv.bottom, 0.9f, 0.001f);
}

BOOST_AUTO_TEST_CASE(crops_segmented_list_selection_to_the_visible_row)
{
	ListBoxTestContext context;
	ListBoxVisual visual;
	visual.clip_rectangle = {0.0f, 0.0f, 20.0f, 5.0f};
	visual.row_count = 1;
	visual.column_count = 0;
	visual.selection.segmented_image = true;
	visual.selection.left_image = {Assets::TextureAssetHandle(1, 1), {0.0f, 0.0f, 1.0f, 1.0f}};
	visual.selection.center_image = {Assets::TextureAssetHandle(2, 1), {0.0f, 0.0f, 1.0f, 1.0f}};
	visual.selection.small_center_image = visual.selection.center_image;
	visual.selection.right_image = {Assets::TextureAssetHandle(3, 1), {0.0f, 0.0f, 1.0f, 1.0f}};
	visual.selection.left_width = 5.0f;
	visual.selection.right_width = 5.0f;
	visual.selection.center_width = 10.0f;
	visual.selection.small_center_width = 2.0f;
	visual.context = &context;
	visual.query_row = &QuerySmallListBoxRow;
	visual.emit_cell = &EmitListBoxCell;

	DrawList draw_list(8);
	BOOST_REQUIRE(Add_List_Box_Visual(draw_list, visual));
	BOOST_REQUIRE_EQUAL(draw_list.Size(), 3);
	BOOST_CHECK_CLOSE(draw_list.Commands()[0].image.uv.bottom, 0.5f, 0.001f);
	BOOST_CHECK_CLOSE(draw_list.Commands()[2].image.uv.bottom, 0.5f, 0.001f);
}

#if defined(ENGINE_UI_WND_GAME_DATA_ROOT)

BOOST_AUTO_TEST_CASE(renders_real_generals_menu_and_hud_wnd_trees)
{
	const std::filesystem::path data_root(ENGINE_UI_WND_GAME_DATA_ROOT);
	RealWNDTest::RunRealWNDLayout(
		data_root / "Window" / "Menus" / "MainMenu.wnd", "MainMenu.wnd");
	RealWNDTest::RunRealWNDLayout(
		data_root / "Window" / "ControlBar.wnd", "ControlBar.wnd");
}

#endif

BOOST_AUTO_TEST_CASE(packs_large_fonts_on_multiple_pages_and_retains_space_advance)
{
    std::vector<Assets::FontGlyphAsset> glyphs;
    glyphs.push_back({32,0,17,{}});
    glyphs.push_back({65,600,601,std::vector<std::uint8_t>(600*600,255)});
    glyphs.push_back({66,600,601,std::vector<std::uint8_t>(600*600,128)});
    const Assets::FontAsset source("Test",600,false,600,0,std::move(glyphs));
    FontFace face;
    BOOST_REQUIRE(face.Build(source));
    BOOST_REQUIRE(face.Get_Glyph(65));
    BOOST_REQUIRE(face.Get_Glyph(66));
    BOOST_CHECK_NE(face.Get_Glyph(65)->page,face.Get_Glyph(66)->page);
    BOOST_CHECK(face.Get_Glyph(32)==nullptr);
    TextRenderer text;
    std::uint32_t width=0,height=0;
    const std::array<std::uint16_t,4> value{65,32,66,0};
    BOOST_REQUIRE(text.Measure(face,value.data(),{},width,height));
    BOOST_CHECK_EQUAL(width,1219);
    BOOST_CHECK_EQUAL(height,600);
}

BOOST_AUTO_TEST_CASE(no_draw_callback_does_not_replace_a_parent_with_a_background)
{
    RecordingSink recording;
    RenderList list(2);
    RenderNode parent;
    parent.window=&recording;
    parent.extract=[](void*,void*,void*,DrawList&) noexcept { return true; };
    NodeIndex parent_index=Invalid_Node;
    BOOST_REQUIRE(list.Add_Node(parent,parent_index));
    RenderNode child;
    child.window=&recording;
    child.extract_context=&recording;
    child.extract=&RecordingSink::DrawChild;
    NodeIndex child_index=Invalid_Node;
    BOOST_REQUIRE(list.Add_Node(child,child_index));
    list.Get_Node(parent_index)->first_child=child_index;
    list.Get_Node(parent_index)->last_child=child_index;
    list.Set_Roots(parent_index,parent_index);
    Renderer renderer;
    BOOST_REQUIRE(renderer.Render(list,Graphics::Get_Renderer2D()));
    BOOST_REQUIRE_EQUAL(recording.events.size(),1);
    BOOST_CHECK_EQUAL(recording.events[0],2);
}

BOOST_AUTO_TEST_CASE(opening_dialogs_grows_the_window_list_and_preserves_child_order)
{
    std::vector<TestWindow> windows(800);
    for (std::size_t i = 0; i < windows.size(); ++i) {
        windows[i].id = static_cast<int>(i);
        if (i + 1 < windows.size()) windows[i].next = &windows[i + 1];
    }
    TestWindow parent{nullptr, windows.data(), 800};
    RenderList list;
    RecordingSink recording;
    BOOST_REQUIRE(Build_Render_List(list, &parent,
        {&recording, &NextTestWindow, &ChildTestWindow, &DescribeTestWindow}));
    BOOST_CHECK_EQUAL(list.Size(), 801);
    Renderer renderer;
    BOOST_REQUIRE(renderer.Render(list, Graphics::Get_Renderer2D()));
    BOOST_REQUIRE_EQUAL(recording.events.size(), 801);
    for (int i = 0; i <= 800; ++i) BOOST_CHECK_EQUAL(recording.events[i], 800 - i);
}

BOOST_AUTO_TEST_CASE(cyclic_window_siblings_are_rejected_without_a_capacity_limit)
{
    TestWindow first, second;
    first.next = &second;
    second.next = &first;
    RenderList list;
    RecordingSink recording;
    BOOST_CHECK(!Build_Render_List(list, &first,
        {&recording, &NextTestWindow, &ChildTestWindow, &DescribeTestWindow}));
    TestWindow parent{nullptr, &first, 1};
    BOOST_CHECK(!Build_Render_List(list, &parent,
        {&recording, &NextTestWindow, &ChildTestWindow, &DescribeTestWindow}));
}
