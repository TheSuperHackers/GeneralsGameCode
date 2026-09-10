module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

export module Graphics.Scene.Beams.Laser;

export import Graphics.Scene.Beams;
export import Graphics.RHI;
export import Graphics.Resources.Handles.ResourceHandle;
import Graphics.Resources.Pools.ResourcePool;
import Graphics.Resources.Textures.References;
import Graphics.Resources.Textures.Snapshot;
import Graphics.Scene.Beams.RibbonTextureCoordinates;
import Graphics.Shaders.Library;

namespace Graphics
{

export struct LaserHandleTag
{
};

export using LaserHandle = ResourceHandle<LaserHandleTag>;

export struct LaserDescription final
{
	Vec3 start{};
	Vec3 end{};
	float width = 0.0f;
	Color4 color{};
	float uv_scale = 1.0f;
	float uv_offset = 0.0f;
	float scroll_rate = 0.0f;
	float distortion = 1.0f;
	RHITextureHandle texture{};
	bool enabled = true;
};

export struct LaserView final
{
	std::array<float, 16> view_projection{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f};
	std::array<float, 3> camera_right{1.0f, 0.0f, 0.0f};
	std::array<float, 3> camera_forward{0.0f, 0.0f, -1.0f};
	std::array<float, 4> shroud_projection{};
	std::uint32_t time_milliseconds = 0;
};

export struct LaserVertex final
{
	float position[3]{};
	float color[4]{};
	float uv[2]{};
	float detail_uv[2]{};
	float normal[3]{};
	float distortion[2]{};
};

static_assert(sizeof(LaserVertex) == 64);

namespace
{

Vec3 operator-(Vec3 left, Vec3 right) noexcept
{
	return {left.x - right.x, left.y - right.y, left.z - right.z};
}

Vec3 operator+(Vec3 left, Vec3 right) noexcept
{
	return {left.x + right.x, left.y + right.y, left.z + right.z};
}

Vec3 operator*(Vec3 value, float scalar) noexcept
{
	return {value.x * scalar, value.y * scalar, value.z * scalar};
}

float Dot(Vec3 left, Vec3 right) noexcept
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

Vec3 Cross(Vec3 left, Vec3 right) noexcept
{
	return {
		left.y * right.z - left.z * right.y,
		left.z * right.x - left.x * right.z,
		left.x * right.y - left.y * right.x};
}

Vec3 Normalize(Vec3 value) noexcept
{
	const float length_squared = Dot(value, value);
	if (!(length_squared > 1.0e-12f) || !std::isfinite(length_squared))
		return {};
	return value * (1.0f / std::sqrt(length_squared));
}

void Write_Vertex(LaserVertex &vertex, Vec3 position, Color4 color,
	float u, float v, float detail_v, Vec3 normal, float distortion) noexcept
{
	vertex.position[0] = position.x;
	vertex.position[1] = position.y;
	vertex.position[2] = position.z;
	vertex.color[0] = color.r;
	vertex.color[1] = color.g;
	vertex.color[2] = color.b;
	vertex.color[3] = color.a;
	vertex.uv[0] = u;
	vertex.uv[1] = v;
	vertex.detail_uv[0] = u;
	vertex.detail_uv[1] = detail_v;
	vertex.normal[0] = normal.x;
	vertex.normal[1] = normal.y;
	vertex.normal[2] = normal.z;
	vertex.distortion[0] = std::max(0.0f, distortion);
	vertex.distortion[1] = 0.0f;
}

}

// Builds a camera-facing laser quad in world space. The caller supplies a
// destination large enough for six vertices. Degenerate and disabled lasers
// produce no vertices so one draw can contain a sparse set of records.
export std::size_t Build_Laser_Vertices(const LaserDescription &description,
	const LaserView &view, float uv_offset, std::span<LaserVertex> output) noexcept
{
	if (output.size() < 6 || !description.enabled || !(description.width > 0.0f)
		|| !std::isfinite(description.width) || !std::isfinite(uv_offset))
		return 0;

	const Vec3 direction = Normalize(description.end - description.start);
	if (Dot(direction, direction) == 0.0f)
		return 0;

	Vec3 side = Normalize(Cross(direction,
		{view.camera_forward[0], view.camera_forward[1], view.camera_forward[2]}));
	if (Dot(side, side) == 0.0f)
		side = Normalize({view.camera_right[0], view.camera_right[1], view.camera_right[2]});
	if (Dot(side, side) == 0.0f)
		return 0;

	const Vec3 offset = side * (description.width * 0.5f);
	const Vec3 start_left = description.start - offset;
	const Vec3 start_right = description.start + offset;
	const Vec3 end_left = description.end - offset;
	const Vec3 end_right = description.end + offset;
	const float start_v = uv_offset;
	const float end_v = uv_offset + description.uv_scale;
	const float start_detail_v = uv_offset;
	const float end_detail_v = uv_offset + description.uv_scale;
	Write_Vertex(output[0], start_left, description.color, 0.0f, start_v, start_detail_v, side, description.distortion);
	Write_Vertex(output[1], start_right, description.color, 1.0f, start_v, start_detail_v, side, description.distortion);
	Write_Vertex(output[2], end_left, description.color, 0.0f, end_v, end_detail_v, side, description.distortion);
	Write_Vertex(output[3], end_left, description.color, 0.0f, end_v, end_detail_v, side, description.distortion);
	Write_Vertex(output[4], start_right, description.color, 1.0f, start_v, start_detail_v, side, description.distortion);
	Write_Vertex(output[5], end_right, description.color, 1.0f, end_v, end_detail_v, side, description.distortion);
	return 6;
}

struct LaserRecord final
{
	LaserDescription description{};
	RibbonTextureCoordinates texture_coordinates{};
	bool texture_time_initialized = false;
	bool fully_obscured = false;
};

static_assert(std::is_nothrow_move_constructible_v<LaserRecord>);
static_assert(std::is_nothrow_move_assignable_v<LaserRecord>);

export class LaserRenderer final
{
public:
	LaserRenderer() = default;
	LaserRenderer(const LaserRenderer &) = delete;
	LaserRenderer &operator=(const LaserRenderer &) = delete;
	~LaserRenderer() { Shutdown(); }

	bool Initialize(Device &device, const std::filesystem::path &shader_directory,
		std::size_t max_lasers = 4096)
	{
		if (m_device == &device && m_vertex_buffer.Is_Valid())
			return true;
		Shutdown();
		if (max_lasers == 0 || max_lasers > (std::numeric_limits<std::uint32_t>::max)() / 6u)
			return false;

		m_shaders = ShaderLibrary{};
		ShaderPrecompiledDesc core_shader;
		core_shader.program.vertex_shader = 50;
		core_shader.program.fragment_shader = 50;
		core_shader.program.source_key = 0x4c41534552434f52ull;
		core_shader.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
		core_shader.vertex_path = shader_directory / "laser_core.vso";
		core_shader.fragment_path = shader_directory / "laser_core.pso";
		m_core_shader = m_shaders.Load_Precompiled(core_shader);

		ShaderPrecompiledDesc distortion_shader;
		distortion_shader.program.vertex_shader = 51;
		distortion_shader.program.fragment_shader = 51;
		distortion_shader.program.source_key = 0x4c41534552444953ull;
		distortion_shader.program.stages = ShaderStageMask::Vertex | ShaderStageMask::Pixel;
		distortion_shader.vertex_path = shader_directory / "laser_distortion.vso";
		distortion_shader.fragment_path = shader_directory / "laser_distortion.pso";
		m_distortion_shader = m_shaders.Load_Precompiled(distortion_shader);
		if (!m_shaders.Is_Loaded(m_core_shader) || !m_shaders.Is_Loaded(m_distortion_shader)) {
			Shutdown();
			return false;
		}

		m_device = &device;
		m_constants = device.Create_Buffer({sizeof(LaserParameters), RHIBufferUsage::Constant});
		if (!m_constants.Is_Valid() || !Ensure_Vertex_Capacity(max_lasers)) {
			Shutdown();
			return false;
		}

		const std::array<std::uint8_t, 4> white_pixels{255, 255, 255, 255};
		// The fallback has enough spatial variation to preserve both the soft
		// detail pattern and the two-sample normal field when an asset has no
		// source texture.  RGB is grayscale so it cannot tint the beam.
		const std::array<std::uint8_t, 16> detail_pixels{
			96,  96,  96, 255,  192, 192, 192, 224,
			160, 160, 160, 240,  64,  64,  64, 192};
		m_white_texture = device.Create_Texture_Initialized(
			{1, 1, 1, RHITextureFormat::RGBA8_UNorm,
				static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)},
			{std::as_bytes(std::span(white_pixels)), 4});
		m_detail_texture = device.Create_Texture_Initialized(
			{2, 2, 1, RHITextureFormat::RGBA8_UNorm,
				static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)},
			{std::as_bytes(std::span(detail_pixels)), 8});
		if (!m_white_texture.Is_Valid() || !m_detail_texture.Is_Valid()) {
			Shutdown();
			return false;
		}

		RHIPipeline core_pipeline;
		core_pipeline.vertex_format = RHIVertexFormat::Position3Color4UV2UV2Normal3;
		core_pipeline.vertex_element_count = 6;
		core_pipeline.vertex_elements[0] = {RHIVertexSemantic::Position, 0,
			RHIVertexElementFormat::Float3, offsetof(LaserVertex, position)};
		core_pipeline.vertex_elements[1] = {RHIVertexSemantic::Color, 0,
			RHIVertexElementFormat::Float4, offsetof(LaserVertex, color)};
		core_pipeline.vertex_elements[2] = {RHIVertexSemantic::TexCoord, 0,
			RHIVertexElementFormat::Float2, offsetof(LaserVertex, uv)};
		core_pipeline.vertex_elements[3] = {RHIVertexSemantic::TexCoord, 1,
			RHIVertexElementFormat::Float2, offsetof(LaserVertex, detail_uv)};
		core_pipeline.vertex_elements[4] = {RHIVertexSemantic::Normal, 0,
			RHIVertexElementFormat::Float3, offsetof(LaserVertex, normal)};
		core_pipeline.vertex_elements[5] = {RHIVertexSemantic::TexCoord, 2,
			RHIVertexElementFormat::Float2, offsetof(LaserVertex, distortion)};
		core_pipeline.depth_test = true;
		core_pipeline.depth_write = false;
		core_pipeline.blend_mode = RHIBlendMode::Additive;
		core_pipeline.cull_mode = RHICullMode::None;
		core_pipeline.custom_blend_factors = true;
		core_pipeline.source_blend = RHIBlendFactor::SourceAlpha;
		core_pipeline.destination_blend = RHIBlendFactor::One;
		core_pipeline.sampler_count = 4;
		core_pipeline.samplers[0].address.fill(RHISamplerAddress::Wrap);
		core_pipeline.samplers[1].address.fill(RHISamplerAddress::Wrap);
		core_pipeline.samplers[2].address.fill(RHISamplerAddress::Clamp);
		core_pipeline.samplers[3].address.fill(RHISamplerAddress::Clamp);
		m_core_pipeline = device.Create_Pipeline(core_pipeline,
			{m_shaders.Bytecode(m_core_shader, ShaderStage::Vertex)},
			{m_shaders.Bytecode(m_core_shader, ShaderStage::Pixel)});

		RHIPipeline distortion_pipeline = core_pipeline;
		distortion_pipeline.blend_mode = RHIBlendMode::Alpha;
		distortion_pipeline.custom_blend_factors = false;
		distortion_pipeline.color_write_mask = 7;
		m_distortion_pipeline = device.Create_Pipeline(distortion_pipeline,
			{m_shaders.Bytecode(m_distortion_shader, ShaderStage::Vertex)},
			{m_shaders.Bytecode(m_distortion_shader, ShaderStage::Pixel)});
		if (!m_core_pipeline.Is_Valid() || !m_distortion_pipeline.Is_Valid()) {
			Shutdown();
			return false;
		}
		m_vertex_capacity = max_lasers;
		return true;
	}

	void Shutdown() noexcept
	{
		if (m_device != nullptr) {
			if (m_core_pipeline.Is_Valid())
				m_device->Destroy_Pipeline(m_core_pipeline);
			if (m_distortion_pipeline.Is_Valid())
				m_device->Destroy_Pipeline(m_distortion_pipeline);
			if (m_vertex_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_vertex_buffer);
			if (m_constants.Is_Valid())
				m_device->Destroy_Buffer(m_constants);
			if (m_white_texture.Is_Valid())
				m_device->Destroy_Texture(m_white_texture);
			if (m_detail_texture.Is_Valid())
				m_device->Destroy_Texture(m_detail_texture);
		}
		m_references.Clear();
		m_snapshot.Shutdown();
		m_core_pipeline = {};
		m_distortion_pipeline = {};
		m_vertex_buffer = {};
		m_constants = {};
		m_white_texture = {};
		m_detail_texture = {};
		m_vertex_capacity = 0;
		m_device = nullptr;
		m_shaders.Destroy(m_core_shader);
		m_shaders.Destroy(m_distortion_shader);
		m_core_shader = {};
		m_distortion_shader = {};
		m_vertices.clear();
	}

	bool Is_Initialized() const noexcept
	{
		return m_device != nullptr && m_core_pipeline.Is_Valid() && m_distortion_pipeline.Is_Valid()
			&& m_vertex_buffer.Is_Valid() && m_constants.Is_Valid();
	}

	LaserHandle Create(const LaserDescription &description = {},
		std::uint32_t creation_time_milliseconds = 0)
	{
		LaserRecord record;
		record.description = description;
		record.texture_coordinates.SetRate({0.0f, description.scroll_rate});
		record.texture_coordinates.Reset(creation_time_milliseconds);
		record.texture_time_initialized = true;
		return m_lasers.Create(std::move(record));
	}

	bool Update(LaserHandle handle, const LaserDescription &description) noexcept
	{
		LaserRecord *record = m_lasers.Resolve(handle);
		if (record == nullptr)
			return false;
		record->description = description;
		record->texture_coordinates.SetRate({0.0f, description.scroll_rate});
		return true;
	}

	bool Destroy(LaserHandle handle) noexcept
	{
		return m_lasers.Destroy(handle);
	}

	bool Set_Fully_Obscured(LaserHandle handle, bool fully_obscured) noexcept
	{
		LaserRecord *record = m_lasers.Resolve(handle);
		if (record == nullptr)
			return false;
		record->fully_obscured = fully_obscured;
		return true;
	}

	bool Set_View(const LaserView &view) noexcept
	{
		if (!Is_Initialized())
			return false;
		m_view = view;
		return true;
	}

	const LaserView &View() const noexcept
	{
		return m_view;
	}

	std::size_t Size() const noexcept
	{
		return m_lasers.Size();
	}

	bool Render(CommandList &commands, RHITextureHandle color_target, RHITextureHandle depth_target,
		RHIViewport viewport, RHITextureHandle shroud_texture = {}, bool enable_distortion = true,
		RHITextureFormat color_format = RHITextureFormat::BGRA8_UNorm) noexcept
	{
		if (!Is_Initialized() || !color_target.Is_Valid() || !depth_target.Is_Valid()
			|| viewport.width == 0 || viewport.height == 0)
			return false;
		if (m_lasers.Size() == 0)
			return true;

		if (m_lasers.Size() > (std::numeric_limits<std::size_t>::max)() / 6
			|| !Ensure_Vertex_Capacity(m_lasers.Size()))
			return false;

		const LaserParameters parameters = Make_Parameters();
		if (!m_device->Update_Buffer(m_constants, 0,
			std::as_bytes(std::span(&parameters, 1))))
			return false;

		std::size_t vertex_count = 0;
		bool has_distortion = false;
		m_ranges.clear();
		// The pool exposes const iteration for stable traversal. Advance each
		// record in a second pass so a repeated view at the same logic time does
		// not change its UV phase.
		m_lasers.For_Each([&](LaserHandle handle, const LaserRecord &) {
			LaserRecord *record = m_lasers.Resolve(handle);
			if (record == nullptr || record->fully_obscured || !record->description.enabled)
				return;
			if (!record->texture_time_initialized) {
				record->texture_coordinates.Reset(m_view.time_milliseconds);
				record->texture_time_initialized = true;
			}
			const auto offset = record->texture_coordinates.Advance(m_view.time_milliseconds);
			LaserDescription description = record->description;
			description.uv_offset += offset[1];
			const std::size_t written = Build_Laser_Vertices(description, m_view,
				 description.uv_offset, std::span(m_vertices).subspan(vertex_count));
			if (written == 6) {
				const RHITextureHandle source = record->description.texture.Is_Valid()
					? m_references.Retain(*m_device, record->description.texture) : RHITextureHandle{};
				LaserDrawRange range;
				range.texture = source.Is_Valid() ? source : m_white_texture;
				range.first_vertex = static_cast<std::uint32_t>(vertex_count);
				range.vertex_count = static_cast<std::uint32_t>(written);
				m_ranges.push_back(range);
				vertex_count += written;
				has_distortion = has_distortion || description.distortion > 0.0f;
			}
		});
		if (vertex_count == 0)
			return true;
		if (!m_device->Update_Buffer(m_vertex_buffer, 0,
			std::as_bytes(std::span<const LaserVertex>(m_vertices.data(), vertex_count))))
			return false;

		const RHITextureHandle retained_shroud = m_references.Retain(*m_device, shroud_texture);
		const RHITextureHandle shroud = retained_shroud.Is_Valid() ? retained_shroud : m_white_texture;
		const bool draw_distortion = enable_distortion && has_distortion;
		if (draw_distortion) {
			if (!commands.Reset_State()
				|| !m_snapshot.Capture(*m_device, commands, color_target,
					viewport.width, viewport.height, color_format))
				return false;
			if (!Draw_Ranges(commands, m_distortion_pipeline, color_target, depth_target,
				viewport, shroud, m_snapshot.Texture()))
				return false;
		}
		if (!Draw_Ranges(commands, m_core_pipeline, color_target, depth_target,
			viewport, shroud, m_white_texture))
			return false;
		return commands.Reset_State()
			&& commands.Set_Render_Targets(color_target, depth_target)
			&& commands.Set_Viewport(viewport);
	}

	private:
	struct LaserParameters final
	{
		std::array<float, 16> view_projection{};
		std::array<float, 4> shroud_projection{};
	};
	static_assert(sizeof(LaserParameters) == 80);

	LaserParameters Make_Parameters() const noexcept
	{
		LaserParameters parameters;
		parameters.view_projection = m_view.view_projection;
		parameters.shroud_projection = m_view.shroud_projection;
		return parameters;
	}

	bool Ensure_Vertex_Capacity(std::size_t laser_capacity)
	{
		if (laser_capacity == 0 || laser_capacity > (std::numeric_limits<std::uint32_t>::max)() / (6u * sizeof(LaserVertex)))
			return false;
		if (laser_capacity <= m_vertex_capacity && m_vertex_buffer.Is_Valid()) {
			m_vertices.resize(laser_capacity * 6);
			return true;
		}
		if (m_device == nullptr)
			return false;
		const std::size_t vertex_count = laser_capacity * 6;
		if (vertex_count > (std::numeric_limits<std::uint32_t>::max)() / sizeof(LaserVertex))
			return false;
		const RHIBufferHandle replacement = m_device->Create_Buffer({
			static_cast<std::uint32_t>(vertex_count * sizeof(LaserVertex)),
			RHIBufferUsage::Vertex, sizeof(LaserVertex)});
		if (!replacement.Is_Valid())
			return false;
		if (m_vertex_buffer.Is_Valid())
			m_device->Destroy_Buffer(m_vertex_buffer);
		m_vertex_buffer = replacement;
		m_vertex_capacity = laser_capacity;
		m_vertices.resize(vertex_count);
		return true;
	}

	struct LaserDrawRange final
	{
		RHITextureHandle texture{};
		std::uint32_t first_vertex = 0;
		std::uint32_t vertex_count = 0;
	};

	bool Draw_Ranges(CommandList &commands, RHIPipelineHandle pipeline, RHITextureHandle color_target,
		RHITextureHandle depth_target, RHIViewport viewport, RHITextureHandle shroud,
		RHITextureHandle background) noexcept
	{
		if (!pipeline.Is_Valid() || !shroud.Is_Valid() || !background.Is_Valid())
			return false;
		for (const LaserDrawRange &range : m_ranges) {
			if (!range.texture.Is_Valid() || range.vertex_count == 0)
				continue;
			std::array<RHIBindlessResource, 5> bindings{};
			bindings[0].type = RHIResourceType::Material;
			bindings[0].buffer = m_constants;
			const std::array<RHITextureHandle, 4> textures{range.texture, m_detail_texture, shroud, background};
			for (std::size_t slot = 0; slot < textures.size(); ++slot) {
				bindings[slot + 1].type = RHIResourceType::Texture;
				bindings[slot + 1].index = ResourceIndex{static_cast<std::uint32_t>(slot), 1};
				bindings[slot + 1].texture = textures[slot];
			}
			if (!commands.Set_Render_Targets(color_target, depth_target)
				|| !commands.Set_Viewport(viewport)
				|| !commands.Bind_Pipeline(pipeline)
				|| !commands.Set_Bindless_Resources(bindings)
				|| !commands.Set_Vertex_Buffer(0, m_vertex_buffer, sizeof(LaserVertex), 0)
				|| !commands.Draw(range.vertex_count, range.first_vertex))
				return false;
		}
		return true;
	}

	Device *m_device = nullptr;
	ResourcePool<LaserRecord, LaserHandle> m_lasers;
	LaserView m_view{};
	std::vector<LaserVertex> m_vertices;
	std::vector<LaserDrawRange> m_ranges;
	std::size_t m_vertex_capacity = 0;
	ShaderLibrary m_shaders;
	ShaderHandle m_core_shader{};
	ShaderHandle m_distortion_shader{};
	RHIPipelineHandle m_core_pipeline{};
	RHIPipelineHandle m_distortion_pipeline{};
	RHIBufferHandle m_vertex_buffer{};
	RHIBufferHandle m_constants{};
	RHITextureHandle m_white_texture{};
	RHITextureHandle m_detail_texture{};
	TextureReferences m_references;
	TextureSnapshot m_snapshot;
};

namespace
{
LaserRenderer g_laser_renderer;
}

export LaserRenderer &GetLaserRenderer() noexcept
{
	return g_laser_renderer;
}

export LaserHandle CreateLaser(const LaserDescription &description,
	std::uint32_t creation_time_milliseconds = 0)
{
	return g_laser_renderer.Create(description, creation_time_milliseconds);
}

export bool UpdateLaser(LaserHandle handle, const LaserDescription &description) noexcept
{
	return g_laser_renderer.Update(handle, description);
}

export void DestroyLaser(LaserHandle handle) noexcept
{
	g_laser_renderer.Destroy(handle);
}

}
