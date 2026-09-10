module;

#include "../profiling/Tracy.h"

#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <span>
#include <utility>
#include <vector>

export module Graphics.Renderer2D;

export import Graphics.RenderGraph.Execution;
export import Graphics.Resources.Bindless.BindlessResourceTable;
export import Graphics.FrameTargets;
export import Graphics.RHI;

namespace Graphics
{

export struct Rect2D final
{
	float left = 0.0f;
	float top = 0.0f;
	float right = 0.0f;
	float bottom = 0.0f;
};

export struct Point2D final
{
	float x = 0.0f;
	float y = 0.0f;
};

export struct Color2D final
{
	float red = 1.0f;
	float green = 1.0f;
	float blue = 1.0f;
	float alpha = 1.0f;

	static constexpr Color2D From_ARGB(std::uint32_t color) noexcept
	{
		return {float((color >> 16) & 255) / 255.0f,
			float((color >> 8) & 255) / 255.0f, float(color & 255) / 255.0f,
			float((color >> 24) & 255) / 255.0f};
	}
};

export enum class Renderer2DBlendMode : std::uint8_t
{
	Solid,
	Alpha,
	Additive
};

export struct Renderer2DTexture final
{
	TextureHandle owner{};
	RHITextureHandle texture{};
	ResourceIndex index{};
};

export struct Renderer2DGlyph final
{
	Rect2D screen{};
	Rect2D uv{};
	Color2D color{};
};

export struct Renderer2DTextureSource final
{
	TextureHandle owner{};
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t row_pitch = 0;
	std::uint32_t revision = 1;
	std::span<const std::byte> pixels{};
};

export struct Renderer2DVertex final
{
	float position[3]{};
	float color[4]{};
	float uv[2]{};
	std::uint32_t texture_index = 0;
};

static_assert(sizeof(Renderer2DVertex) == 40);

export class Renderer2D final
{
public:
	bool Initialize(
		Device &device,
		const std::filesystem::path &shader_directory,
		std::size_t vertex_capacity = 131072,
		std::size_t index_capacity = 196608,
		std::size_t batch_capacity = 8192)
	{
		if (m_initialized || !device.Is_Valid() || vertex_capacity == 0 || index_capacity == 0 || batch_capacity == 0
			|| vertex_capacity > std::numeric_limits<std::uint32_t>::max() / sizeof(Renderer2DVertex)
			|| index_capacity > std::numeric_limits<std::uint32_t>::max() / sizeof(std::uint32_t))
			return false;

		std::vector<std::byte> vertex_shader;
		std::vector<std::byte> fragment_shader;
		if (!Load_Binary(shader_directory / "ui_2d.vso", vertex_shader)
			|| !Load_Binary(shader_directory / "ui_2d.pso", fragment_shader))
			return false;

		m_device = &device;
		m_vertices.reserve(vertex_capacity);
		m_indices.reserve(index_capacity);
		m_batches.reserve(batch_capacity);
		m_vertex_capacity = vertex_capacity;
		m_index_capacity = index_capacity;
		m_width = 1;
		m_height = 1;

		m_vertex_buffer = device.Create_Buffer({
			static_cast<std::uint32_t>(vertex_capacity * sizeof(Renderer2DVertex)),
			RHIBufferUsage::Vertex,
			static_cast<std::uint32_t>(sizeof(Renderer2DVertex))});
		m_index_buffer = device.Create_Buffer({
			static_cast<std::uint32_t>(index_capacity * sizeof(std::uint32_t)),
			RHIBufferUsage::Index,
			static_cast<std::uint32_t>(sizeof(std::uint32_t))});
		if (!m_vertex_buffer.Is_Valid() || !m_index_buffer.Is_Valid()) {
			Shutdown();
			return false;
		}

		constexpr std::array<std::uint8_t, 4> white_pixels = {255, 255, 255, 255};
		m_white_texture = device.Create_Texture_Initialized(
			{1, 1, 1, RHITextureFormat::RGBA8_UNorm, static_cast<std::uint32_t>(RHITextureUsage::ShaderResource), 1},
			{std::as_bytes(std::span<const std::uint8_t>(white_pixels)), 4});
		if (!m_white_texture.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_textures.Reserve(2048);
		m_owned_textures.reserve(2048);
		m_white_binding = Register_Texture(TextureHandle(0, 1), m_white_texture);
		if (!m_white_binding.index.Is_Valid()) {
			Shutdown();
			return false;
		}

		for (std::size_t index = 0; index < m_pipelines.size(); ++index) {
			const RHIBlendMode blend = index == static_cast<std::size_t>(Renderer2DBlendMode::Solid)
				? RHIBlendMode::Disabled
				: index == static_cast<std::size_t>(Renderer2DBlendMode::Additive)
					? RHIBlendMode::Additive
					: RHIBlendMode::Alpha;
			RHIPipeline description{
				0x3255440000000000ull + static_cast<std::uint64_t>(index),
				false,
				false,
				RHIPrimitiveTopology::TriangleList,
				RHIVertexFormat::Position3Color4UV2ResourceIndex,
				blend,
				RHICullMode::None,
				RHIBlendOperation::Add,
				true};
			description.samplers[0].address.fill(RHISamplerAddress::Clamp);
			m_pipelines[index] = device.Create_Pipeline(description, {vertex_shader}, {fragment_shader});
			if (!m_pipelines[index].Is_Valid()) {
				Shutdown();
				return false;
			}
		}

		m_graph.Reserve(2, 1, 2);
		m_color_resource = m_graph.Create_Resource({GraphResourceKind::Texture});
		m_depth_resource = m_graph.Create_Resource({GraphResourceKind::Texture});
		const std::array<GraphResourceUse, 2> uses = {
			GraphResourceUse::Write(m_color_resource),
			GraphResourceUse::Write(m_depth_resource)};
		m_pass = m_graph.Add_Pass({0}, uses);
		if (!m_color_resource.Is_Valid() || !m_depth_resource.Is_Valid() || !m_pass.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_initialized = true;
		return true;
	}

	void Shutdown() noexcept
	{
		if (m_device != nullptr) {
			for (TextureEntry &entry : m_owned_textures) {
				if (entry.texture.Is_Valid())
					m_device->Destroy_Texture(entry.texture);
			}
			for (RHIPipelineHandle &pipeline : m_pipelines) {
				if (pipeline.Is_Valid())
					m_device->Destroy_Pipeline(pipeline);
				pipeline = {};
			}
			if (m_white_texture.Is_Valid())
				m_device->Destroy_Texture(m_white_texture);
			if (m_vertex_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_vertex_buffer);
			if (m_index_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_index_buffer);
		}

		m_texture_pages.clear();
		m_texture_slots.clear();
		m_page_epoch = 0;
		m_textures.Clear();
		m_owned_textures.clear();
		m_white_texture = {};
		m_white_binding = {};
		m_vertex_buffer = {};
		m_index_buffer = {};
		m_vertices.clear();
		m_indices.clear();
		m_batches.clear();
		m_graph = {};
		m_execution_plan = {};
		m_color_resource = {};
		m_depth_resource = {};
		m_pass = {};
		m_bindings = {};
		m_device = nullptr;
		m_initialized = false;
		m_plan_compiled = false;
		m_recording = false;
	}

	bool Is_Initialized() const noexcept
	{
		return m_initialized;
	}

	Renderer2DTexture Register_Texture(TextureHandle owner, RHITextureHandle texture)
	{
		if (m_device == nullptr)
			return {};
		if (!owner.Is_Valid() || !texture.Is_Valid())
			return {};
		return {owner, texture, m_textures.Register_Texture(owner, texture)};
	}

	Renderer2DTexture Register_Texture(Renderer2DTextureSource source)
	{
		if (m_device == nullptr || !source.owner.Is_Valid() || source.width == 0 || source.height == 0
			|| source.row_pitch < source.width * 4 || source.pixels.empty())
			return {};

		for (TextureEntry &entry : m_owned_textures) {
			if (entry.owner != source.owner)
				continue;
			if (entry.revision == source.revision && entry.texture.Is_Valid())
				return entry.binding;

			const RHITextureUpload upload{source.pixels, source.row_pitch};
			if (entry.texture.Is_Valid() && entry.width == source.width && entry.height == source.height
				&& m_device->Update_Texture(entry.texture, upload)) {
				entry.revision = source.revision;
				return entry.binding;
			}

			const RHITextureHandle replacement = m_device->Create_Texture_Initialized(
				{source.width, source.height, 1, RHITextureFormat::RGBA8_UNorm,
					static_cast<std::uint32_t>(RHITextureUsage::ShaderResource), 1},
				upload);
			if (!replacement.Is_Valid())
				return {};
			if (entry.texture.Is_Valid())
				m_device->Destroy_Texture(entry.texture);
			entry.texture = replacement;
			entry.width = source.width;
			entry.height = source.height;
			entry.revision = source.revision;
			if (!m_textures.Update_Texture(source.owner, replacement))
				return {};
			entry.binding = {source.owner, replacement, m_textures.Texture_Index(source.owner)};
			return entry.binding;
		}

		const RHITextureHandle texture = m_device->Create_Texture_Initialized(
			{source.width, source.height, 1, RHITextureFormat::RGBA8_UNorm,
				static_cast<std::uint32_t>(RHITextureUsage::ShaderResource), 1},
			{source.pixels, source.row_pitch});
		if (!texture.Is_Valid())
			return {};
		const ResourceIndex index = m_textures.Register_Texture(source.owner, texture);
		if (!index.Is_Valid()) {
			m_device->Destroy_Texture(texture);
			return {};
		}
		m_owned_textures.push_back({source.owner, texture, {source.owner, texture, index},
			source.width, source.height, source.revision});
		return m_owned_textures.back().binding;
	}

	Renderer2DTexture White_Texture() const noexcept
	{
		return m_white_binding;
	}

	void Begin(std::uint32_t width, std::uint32_t height) noexcept
	{
		m_width = width;
		m_height = height;
		m_vertices.clear();
		m_indices.clear();
		m_batches.clear();
		m_texture_pages.clear();
		Begin_Texture_Page();
		m_clip_enabled = false;
		m_clip = {};
		m_recording = m_initialized && width != 0 && height != 0;
	}

	void Discard() noexcept
	{
		m_recording = false;
		m_vertices.clear();
		m_indices.clear();
		m_batches.clear();
	}

	struct ClipState final
	{
		bool enabled = false;
		Rect2D rectangle{};
	};

	ClipState Get_Clip() const noexcept
	{
		return {m_clip_enabled, m_clip};
	}

	void Set_Clip(bool enabled, Rect2D clip) noexcept
	{
		m_clip_enabled = enabled;
		m_clip = clip;
	}

	bool Add_Rect(Rect2D rect, Color2D color, Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha, bool grayscale = false) noexcept
	{
		return Add_Quad(rect, {0.0f, 0.0f, 1.0f, 1.0f}, m_white_binding, color, blend, grayscale);
	}

	bool Add_Outline(Rect2D rect, float width, Color2D color, Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha) noexcept
	{
		if (width <= 0.0f)
			return true;
		return Add_Line({rect.left + 1.0f, rect.bottom}, {rect.left + 1.0f, rect.top + 1.0f}, width, color, blend)
			&& Add_Line({rect.left, rect.top + 1.0f}, {rect.right - 1.0f, rect.top + 1.0f}, width, color, blend)
			&& Add_Line({rect.right, rect.top}, {rect.right, rect.bottom - 1.0f}, width, color, blend)
			&& Add_Line({rect.right, rect.bottom}, {rect.left + 1.0f, rect.bottom}, width, color, blend);
	}

	bool Add_Line(Point2D start, Point2D end, float width, Color2D color, Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha) noexcept
	{
		const float dx = start.x - end.x;
		const float dy = start.y - end.y;
		const float length_squared = dx * dx + dy * dy;
		if (width <= 0.0f || length_squared <= 0.0f)
			return true;
		const float inverse_length = 1.0f / std::sqrt(length_squared);
		const float offset_x = dy * inverse_length * width * 0.5f;
		const float offset_y = -dx * inverse_length * width * 0.5f;
		const std::array<Point2D, 4> positions = {{
			{start.x - offset_x, start.y - offset_y},
			{start.x + offset_x, start.y + offset_y},
			{end.x - offset_x, end.y - offset_y},
			{end.x + offset_x, end.y + offset_y}}};
		return Add_Quad(
			positions,
			Rect2D{0.0f, 0.0f, 1.0f, 1.0f},
			m_white_binding,
			color,
			blend,
			false);
	}

	bool Add_Gradient_Line(
		Point2D start,
		Point2D end,
		float width,
		std::array<Color2D, 4> colors,
		Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha) noexcept
	{
		const float dx = start.x - end.x;
		const float dy = start.y - end.y;
		const float length_squared = dx * dx + dy * dy;
		if (width <= 0.0f || length_squared <= 0.0f)
			return true;
		const float inverse_length = 1.0f / std::sqrt(length_squared);
		const float offset_x = dy * inverse_length * width * 0.5f;
		const float offset_y = -dx * inverse_length * width * 0.5f;
		const std::array<Point2D, 4> positions = {{
			{start.x - offset_x, start.y - offset_y},
			{start.x + offset_x, start.y + offset_y},
			{end.x - offset_x, end.y - offset_y},
			{end.x + offset_x, end.y + offset_y}}};
		const std::array<Point2D, 4> uvs = {{
			{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}}};
		const Rect2D screen{
			std::min({positions[0].x, positions[1].x, positions[2].x, positions[3].x}),
			std::min({positions[0].y, positions[1].y, positions[2].y, positions[3].y}),
			std::max({positions[0].x, positions[1].x, positions[2].x, positions[3].x}),
			std::max({positions[0].y, positions[1].y, positions[2].y, positions[3].y})};
		return Add_Quad_Expanded(screen, uvs, m_white_binding, colors, blend, false, positions);
	}

	bool Add_Line(
		Point2D start,
		Point2D end,
		float width,
		std::array<Color2D, 4> colors,
		Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha) noexcept
	{
		const float dx = start.x - end.x;
		const float dy = start.y - end.y;
		const float length_squared = dx * dx + dy * dy;
		if (width <= 0.0f || length_squared <= 0.0f)
			return true;
		const float inverse_length = 1.0f / std::sqrt(length_squared);
		const float offset_x = dy * inverse_length * width * 0.5f;
		const float offset_y = -dx * inverse_length * width * 0.5f;
		const std::array<Point2D, 4> positions = {{
			{start.x - offset_x, start.y - offset_y},
			{start.x + offset_x, start.y + offset_y},
			{end.x - offset_x, end.y - offset_y},
			{end.x + offset_x, end.y + offset_y}}};
		const std::array<Point2D, 4> uvs = {{
			{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}}};
		const Rect2D screen{
			std::min({positions[0].x, positions[1].x, positions[2].x, positions[3].x}),
			std::min({positions[0].y, positions[1].y, positions[2].y, positions[3].y}),
			std::max({positions[0].x, positions[1].x, positions[2].x, positions[3].x}),
			std::max({positions[0].y, positions[1].y, positions[2].y, positions[3].y})};
		return Add_Quad_Expanded(screen, uvs, m_white_binding, colors, blend, false, positions);
	}

	bool Add_Triangle(Point2D first, Point2D second, Point2D third, Color2D color, Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha) noexcept
	{
		if (!Can_Add(3, 3))
			return false;
		const ResourceIndex texture_index = Page_Texture(m_white_binding.index);
		if (!texture_index.Is_Valid()) return false;
		const std::uint32_t first_vertex = static_cast<std::uint32_t>(m_vertices.size());
		Append_Vertex(first, {0.0f, 0.0f}, texture_index, color);
		Append_Vertex(second, {0.0f, 0.0f}, texture_index, color);
		Append_Vertex(third, {0.0f, 0.0f}, texture_index, color);
		Append_Indices(first_vertex, {0, 1, 2}, blend);
		return true;
	}

	bool Add_Rect_Clock(
		Rect2D rectangle,
		int percent,
		Color2D color,
		bool remaining,
		Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha) noexcept
	{
		if (percent < 0 || percent > 100 || rectangle.right <= rectangle.left || rectangle.bottom <= rectangle.top)
			return true;
		if ((!remaining && percent == 100) || (remaining && percent == 0))
			return Add_Rect(rectangle, color, blend);

		const Point2D center{
			(rectangle.left + rectangle.right) * 0.5f,
			(rectangle.top + rectangle.bottom) * 0.5f};
		const std::array<Point2D, 7> perimeter = {{
			{center.x, rectangle.top},
			{rectangle.right, rectangle.top},
			{rectangle.right, rectangle.bottom},
			{center.x, rectangle.bottom},
			{rectangle.left, rectangle.bottom},
			{rectangle.left, rectangle.top},
			{center.x, rectangle.top}}};
		const float start = remaining ? percent * 6.0f / 100.0f : 0.0f;
		const float end = remaining ? 6.0f : percent * 6.0f / 100.0f;
		const auto point_on_perimeter = [&](float position) {
			if (position >= 6.0f)
				return perimeter[6];
			const int segment = static_cast<int>(position);
			const float fraction = position - segment;
			return Point2D{
				perimeter[segment].x + (perimeter[segment + 1].x - perimeter[segment].x) * fraction,
				perimeter[segment].y + (perimeter[segment + 1].y - perimeter[segment].y) * fraction};
		};

		Point2D previous = point_on_perimeter(start);
		for (int boundary = static_cast<int>(std::floor(start)) + 1; boundary < static_cast<int>(std::ceil(end)); ++boundary) {
			if (!Add_Triangle(center, previous, perimeter[boundary], color, blend))
				return false;
			previous = perimeter[boundary];
		}
		const Point2D final_point = point_on_perimeter(end);
		if (final_point.x != previous.x || final_point.y != previous.y)
			return Add_Triangle(center, previous, final_point, color, blend);
		return true;
	}

	bool Add_Quad(
		Rect2D screen,
		Rect2D uv,
		Renderer2DTexture texture,
		Color2D color,
		Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha,
		bool grayscale = false) noexcept
	{
		const std::array<Point2D, 4> positions = {{
			{screen.left, screen.top},
			{screen.left, screen.bottom},
			{screen.right, screen.top},
			{screen.right, screen.bottom}}};
		const std::array<Point2D, 4> uvs = {{
			{uv.left, uv.top}, {uv.left, uv.bottom}, {uv.right, uv.top}, {uv.right, uv.bottom}}};
		const std::array<Color2D, 4> colors = {{color, color, color, color}};
		return Add_Quad_Expanded(screen, uvs, texture, colors, blend, grayscale, positions);
	}

	bool Add_Quad(
		std::array<Point2D, 4> positions,
		Rect2D uv,
		Renderer2DTexture texture,
		Color2D color,
		Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha,
		bool grayscale = false) noexcept
	{
		const Rect2D screen{
			std::min({positions[0].x, positions[1].x, positions[2].x, positions[3].x}),
			std::min({positions[0].y, positions[1].y, positions[2].y, positions[3].y}),
			std::max({positions[0].x, positions[1].x, positions[2].x, positions[3].x}),
			std::max({positions[0].y, positions[1].y, positions[2].y, positions[3].y})};
		const std::array<Point2D, 4> uvs = {{
			{uv.left, uv.top}, {uv.left, uv.bottom}, {uv.right, uv.top}, {uv.right, uv.bottom}}};
		const std::array<Color2D, 4> colors = {{color, color, color, color}};
		return Add_Quad_Expanded(screen, uvs, texture, colors, blend, grayscale, positions);
	}

	bool Add_Quad(
		std::array<Point2D, 4> positions,
		std::array<Point2D, 4> uvs,
		Renderer2DTexture texture,
		Color2D color,
		Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha,
		bool grayscale = false) noexcept
	{
		const Rect2D screen{
			std::min({positions[0].x, positions[1].x, positions[2].x, positions[3].x}),
			std::min({positions[0].y, positions[1].y, positions[2].y, positions[3].y}),
			std::max({positions[0].x, positions[1].x, positions[2].x, positions[3].x}),
			std::max({positions[0].y, positions[1].y, positions[2].y, positions[3].y})};
		const std::array<Color2D, 4> colors = {{color, color, color, color}};
		return Add_Quad_Expanded(screen, uvs, texture, colors, blend, grayscale, positions);
	}

	bool Add_Text_Glyphs(
		std::span<const Renderer2DGlyph> glyphs,
		Renderer2DTexture texture,
		Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha) noexcept
	{
		for (const Renderer2DGlyph &glyph : glyphs) {
			if (!Add_Quad(glyph.screen, glyph.uv, texture, glyph.color, blend))
				return false;
		}
		return true;
	}

	bool Execute(Device &device, CommandList &command_list, RHITextureHandle color_target, RHITextureHandle depth_target, RHIViewport viewport) noexcept
	{
		GRAPHICS_PROFILE_SCOPE("Graphics.Renderer2D.Execute");
		if (!m_initialized || m_device != &device || !color_target.Is_Valid() || !depth_target.Is_Valid() || viewport.width == 0 || viewport.height == 0)
			return false;

		m_bindings[0] = GraphResourceBinding::Texture(m_color_resource, color_target);
		m_bindings[1] = GraphResourceBinding::Texture(m_depth_resource, depth_target);
		if (!m_plan_compiled) {
			if (!m_execution_plan.Compile(m_graph, m_bindings))
				return false;
			m_plan_compiled = true;
		}

		return m_execution_plan.Execute(m_graph, command_list,
			[this, viewport](GraphPassHandle, CommandList &commands, const PassResources &resources) noexcept {
				return Execute_Pass(commands, resources, viewport);
			});
	}

	std::size_t Vertex_Count() const noexcept
	{
		return m_vertices.size();
	}

	std::size_t Index_Count() const noexcept
	{
		return m_indices.size();
	}

	std::size_t Batch_Count() const noexcept
	{
		return m_batches.size();
	}

	bool Has_Draws() const noexcept
	{
		return !m_indices.empty();
	}

private:
	struct Batch final
	{
		std::uint32_t first_index = 0;
		std::uint32_t index_count = 0;
		Renderer2DBlendMode blend = Renderer2DBlendMode::Alpha;
		RHIScissorRect scissor{};
		std::size_t texture_page = 0;
	};

    static constexpr std::uint32_t Texture_Page_Size = 128;
    struct TexturePage final
    {
        std::array<RHIBindlessResource, Texture_Page_Size> resources{};
        std::array<ResourceIndex, Texture_Page_Size> logical_indices{};
        std::uint32_t count = 0;
    };
    struct TextureSlot final
    {
        std::uint64_t epoch = 0;
        RHITextureHandle texture{};
        ResourceIndex index{};
    };

    void Begin_Texture_Page()
    {
        m_texture_pages.emplace_back();
        if (++m_page_epoch == 0) {
            for (auto& slot : m_texture_slots) slot.epoch = 0;
            m_page_epoch = 1;
        }
    }

    ResourceIndex Page_Texture(ResourceIndex logical_index)
    {
        const auto resource = m_textures.Resolve(logical_index);
        if (resource.type != RHIResourceType::Texture || !resource.texture.Is_Valid()) return {};
        const auto slot_index = logical_index.Get_Index();
        if (slot_index >= m_texture_slots.size()) m_texture_slots.resize(std::size_t(slot_index) + 1);
        auto& slot = m_texture_slots[slot_index];
        if (slot.epoch == m_page_epoch && slot.texture == resource.texture) return slot.index;
        if (m_texture_pages.back().count == Texture_Page_Size) Begin_Texture_Page();
        auto& page = m_texture_pages.back();
        const ResourceIndex index(page.count,1);
        page.logical_indices[page.count] = logical_index;
        page.resources[page.count] = resource;
        page.resources[page.count++].index = index;
        slot = {m_page_epoch,resource.texture,index};
        return index;
    }

	struct TextureEntry final
	{
		TextureHandle owner{};
		RHITextureHandle texture{};
		Renderer2DTexture binding{};
		std::uint32_t width = 0;
		std::uint32_t height = 0;
		std::uint32_t revision = 0;
	};

	static bool Load_Binary(const std::filesystem::path &path, std::vector<std::byte> &data)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)
			return false;
		const std::streampos end = file.tellg();
		if (end <= 0)
			return false;
		data.resize(static_cast<std::size_t>(end));
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
		return file.good() || file.eof();
	}

	bool Can_Add(std::size_t vertices, std::size_t indices) const noexcept
	{
		return m_recording && vertices <= m_vertex_capacity - m_vertices.size() && indices <= m_index_capacity - m_indices.size();
	}

	static float To_Clip_X(float x, std::uint32_t width) noexcept
	{
		return x * (2.0f / static_cast<float>(width)) - 1.0f;
	}

	static float To_Clip_Y(float y, std::uint32_t height) noexcept
	{
		return 1.0f - y * (2.0f / static_cast<float>(height));
	}

	void Append_Vertex(Point2D position, Point2D uv, ResourceIndex texture_index, Color2D color) noexcept
	{
		Renderer2DVertex &vertex = m_vertices.emplace_back();
		vertex.position[0] = To_Clip_X(position.x, m_width);
		vertex.position[1] = To_Clip_Y(position.y, m_height);
		vertex.position[2] = 0.0f;
		vertex.color[0] = color.red;
		vertex.color[1] = color.green;
		vertex.color[2] = color.blue;
		vertex.color[3] = color.alpha;
		vertex.uv[0] = uv.x;
		vertex.uv[1] = uv.y;
		vertex.texture_index = texture_index.Get_Index();
	}

	bool Add_Quad_Expanded(
		Rect2D screen,
		std::array<Point2D, 4> uvs,
		Renderer2DTexture texture,
		std::array<Color2D, 4> colors,
		Renderer2DBlendMode blend,
		bool grayscale,
		std::array<Point2D, 4> positions) noexcept
	{
		if (screen.right <= screen.left || screen.bottom <= screen.top)
			return true;
		if (!texture.index.Is_Valid() || !Can_Add(4, 6))
			return false;

		const std::uint32_t first_vertex = static_cast<std::uint32_t>(m_vertices.size());
		const ResourceIndex page_index = Page_Texture(texture.index);
		if (!page_index.Is_Valid()) return false;
		const std::uint32_t texture_index = page_index.Get_Index() | (grayscale ? 0x80000000u : 0u);
		for (std::size_t index = 0; index < positions.size(); ++index) {
			Renderer2DVertex &vertex = m_vertices.emplace_back();
			vertex.position[0] = To_Clip_X(positions[index].x, m_width);
			vertex.position[1] = To_Clip_Y(positions[index].y, m_height);
			vertex.position[2] = 0.0f;
			vertex.color[0] = colors[index].red;
			vertex.color[1] = colors[index].green;
			vertex.color[2] = colors[index].blue;
			vertex.color[3] = colors[index].alpha;
			vertex.uv[0] = uvs[index].x;
			vertex.uv[1] = uvs[index].y;
			vertex.texture_index = texture_index;
		}
		Append_Indices(first_vertex, {0, 1, 2, 2, 1, 3}, blend);
		return true;
	}

	void Append_Indices(std::uint32_t first_vertex, std::initializer_list<std::uint32_t> indices, Renderer2DBlendMode blend) noexcept
	{
		const std::uint32_t first_index = static_cast<std::uint32_t>(m_indices.size());
		for (const std::uint32_t index : indices)
			m_indices.push_back(first_vertex + index);

		const RHIScissorRect scissor = Make_Scissor();
		if (!m_batches.empty() && m_batches.back().texture_page == m_texture_pages.size() - 1
			&& m_batches.back().blend == blend
			&& m_batches.back().scissor.x == scissor.x
			&& m_batches.back().scissor.y == scissor.y
			&& m_batches.back().scissor.width == scissor.width
			&& m_batches.back().scissor.height == scissor.height
			&& m_batches.back().first_index + m_batches.back().index_count == first_index) {
			m_batches.back().index_count += static_cast<std::uint32_t>(indices.size());
			return;
		}

		m_batches.push_back({first_index, static_cast<std::uint32_t>(indices.size()), blend, scissor, m_texture_pages.size() - 1});
	}

	RHIScissorRect Make_Scissor() const noexcept
	{
		const float left = m_clip_enabled ? m_clip.left : 0.0f;
		const float top = m_clip_enabled ? m_clip.top : 0.0f;
		const float right = m_clip_enabled ? m_clip.right : static_cast<float>(m_width);
		const float bottom = m_clip_enabled ? m_clip.bottom : static_cast<float>(m_height);
		const float clamped_left = left < 0.0f ? 0.0f : left > m_width ? static_cast<float>(m_width) : left;
		const float clamped_top = top < 0.0f ? 0.0f : top > m_height ? static_cast<float>(m_height) : top;
		const float clamped_right = right < clamped_left ? clamped_left : right > m_width ? static_cast<float>(m_width) : right;
		const float clamped_bottom = bottom < clamped_top ? clamped_top : bottom > m_height ? static_cast<float>(m_height) : bottom;
		return {
			static_cast<std::uint32_t>(clamped_left),
			static_cast<std::uint32_t>(clamped_top),
			static_cast<std::uint32_t>(clamped_right - clamped_left),
			static_cast<std::uint32_t>(clamped_bottom - clamped_top)};
	}

	bool Execute_Pass(CommandList &commands, const PassResources &resources, RHIViewport viewport) noexcept
	{
		const RHITextureHandle color_target = resources.Texture(m_color_resource);
		const RHITextureHandle depth_target = resources.Texture(m_depth_resource);
		if (!color_target.Is_Valid() || !depth_target.Is_Valid()
			|| !commands.Set_Render_Targets(color_target, depth_target)
			|| !commands.Set_Viewport(viewport))
			return false;
		if (m_indices.empty())
			return true;

		if (!m_device->Update_Buffer(m_vertex_buffer, 0, std::as_bytes(std::span<const Renderer2DVertex>(m_vertices)))
			|| !m_device->Update_Buffer(m_index_buffer, 0, std::as_bytes(std::span<const std::uint32_t>(m_indices)))
			|| !commands.Set_Vertex_Buffer(0, m_vertex_buffer, sizeof(Renderer2DVertex), 0)
			|| !commands.Set_Index_Buffer(m_index_buffer, RHIIndexFormat::UInt32, 0))
			return false;

		std::size_t bound_page = m_texture_pages.size();
		for (const Batch &batch : m_batches) {
			if (batch.scissor.width == 0 || batch.scissor.height == 0)
				continue;
            if (bound_page != batch.texture_page) {
                auto& page = m_texture_pages[batch.texture_page];
                for (std::uint32_t slot = 0; slot < page.count; ++slot) {
                    auto resource = m_textures.Resolve(page.logical_indices[slot]);
                    if (resource.type != RHIResourceType::Texture || !resource.texture.Is_Valid()) return false;
                    resource.index = ResourceIndex(slot,1);
                    page.resources[slot] = resource;
                }
                if (!commands.Set_Bindless_Resources({page.resources.data(),page.count})) return false;
                bound_page = batch.texture_page;
            }
			const std::size_t pipeline_index = static_cast<std::size_t>(batch.blend);
			if (pipeline_index >= m_pipelines.size() || !commands.Bind_Pipeline(m_pipelines[pipeline_index])
				|| !commands.Set_Scissor(batch.scissor)
				|| !commands.Draw_Indexed(batch.index_count, batch.first_index))
				return false;
		}
		return true;
	}

	Device *m_device = nullptr;
	bool m_initialized = false;
	bool m_recording = false;
	bool m_plan_compiled = false;
	std::uint32_t m_width = 1;
	std::uint32_t m_height = 1;
	std::size_t m_vertex_capacity = 0;
	std::size_t m_index_capacity = 0;
	bool m_clip_enabled = false;
	Rect2D m_clip{};
	std::vector<Renderer2DVertex> m_vertices;
	std::vector<std::uint32_t> m_indices;
	std::vector<Batch> m_batches;
	RHITextureHandle m_white_texture{};
	Renderer2DTexture m_white_binding{};
	RHIBufferHandle m_vertex_buffer{};
	RHIBufferHandle m_index_buffer{};
	std::array<RHIPipelineHandle, 3> m_pipelines{};
	BindlessResourceTable m_textures;
    std::vector<TexturePage> m_texture_pages;
    std::vector<TextureSlot> m_texture_slots;
    std::uint64_t m_page_epoch = 0;
	std::vector<TextureEntry> m_owned_textures;
	RenderGraph m_graph;
	ExecutionPlan m_execution_plan;
	std::array<GraphResourceBinding, 2> m_bindings{};
	GraphResourceHandle m_color_resource{};
	GraphResourceHandle m_depth_resource{};
	GraphPassHandle m_pass{};
};

export Renderer2D &Get_Renderer2D() noexcept;

namespace
{
Renderer2D g_renderer;
}

Renderer2D &Get_Renderer2D() noexcept
{
	return g_renderer;
}

}
