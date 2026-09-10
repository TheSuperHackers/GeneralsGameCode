module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.Scene.Lines.Tracer;

export import Graphics.Scene.Props.Geometry;
export import Graphics.Scene.Props.Renderer;
export import Graphics.Scene.Props.Submission;
import Assets.Math;

namespace Graphics
{

namespace
{

std::array<float, 4> Quantize_Color(const std::array<float, 4> &color) noexcept
{
	// The prelit path submitted an 8-bit packed colour. Keep that boundary
	// before the float vertex stream so channel values remain identical.
	const auto packed = Assets::Color_To_ARGB({color[0], color[1], color[2], color[3]});
	const auto unpacked = Assets::Color_From_ARGB(packed);
	return {unpacked.r, unpacked.g, unpacked.b, unpacked.a};
}

float Quantize_Opacity(float opacity) noexcept
{
	return Quantize_Color({0.0f, 0.0f, 0.0f, opacity})[3];
}

}

export inline constexpr std::size_t TracerVertexCount = 8;
export inline constexpr std::size_t TracerIndexCount = 36;
// Alpha tracers retain the authored layer used by the original scene path.
// The scene queue owns the ordering mechanics; this value keeps the policy
// with the graphics feature instead of leaking it into the game adapter.
export inline constexpr std::uint32_t TracerAuthoredLayer = 1;

// A tracer is a small rectangular volume whose local X axis is its length.
// Colour is kept in the mesh because PropParameters has no per-draw colour
// slot; transforms and opacity remain per-draw state.
export struct TracerDescription final
{
	float length = 1.0f;
	float width = 1.0f;
	std::array<float, 4> color{1.0f, 1.0f, 1.0f, 1.0f};
};

export struct TracerDrawData final
{
	std::array<float, 16> view_projection{};
	std::array<float, 16> view{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f};
	std::array<float, 16> world{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f};
	std::array<float, 4> camera_position{};
	std::array<float, 4> camera_depth{};
	float opacity = 1.0f;
	bool front_counter_clockwise = true;
};

export bool Is_Valid_Tracer(const TracerDescription &description) noexcept
{
	if (!std::isfinite(description.length) || !std::isfinite(description.width)
		|| description.length < 0.0f || description.width < 0.0f)
		return false;
	for (const float component : description.color)
		if (!std::isfinite(component))
			return false;
	return true;
}

export std::size_t Build_Tracer_Geometry(const TracerDescription &description,
	std::span<PropVertex> vertices, std::span<std::uint32_t> indices) noexcept
{
	if (!Is_Valid_Tracer(description) || vertices.size() < TracerVertexCount
		|| indices.size() < TracerIndexCount)
		return 0;

	const float half_width = description.width * 0.5f;
	const std::array<std::array<float, 3>, TracerVertexCount> positions{{
		{0.0f, -half_width, -half_width},
		{0.0f, half_width, -half_width},
		{0.0f, -half_width, half_width},
		{0.0f, half_width, half_width},
		{description.length, -half_width, -half_width},
		{description.length, half_width, -half_width},
		{description.length, -half_width, half_width},
		{description.length, half_width, half_width}}};

	constexpr std::array<std::uint32_t, TracerIndexCount> topology{
		3, 5, 1,
		7, 5, 3,
		1, 5, 0,
		5, 4, 0,
		4, 2, 0,
		4, 6, 2,
		7, 3, 2,
		6, 7, 2,
		7, 6, 5,
		5, 6, 4,
		2, 3, 1,
		2, 1, 0};

	const std::array<float, 4> packed_color = Quantize_Color(description.color);
	for (std::size_t vertex_index = 0; vertex_index < TracerVertexCount; ++vertex_index) {
		PropVertex &vertex = vertices[vertex_index];
		vertex = {};
		vertex.position = positions[vertex_index];
		vertex.color = packed_color;
		vertex.material_diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
		vertex.material_specular = {0.0f, 0.0f, 0.0f, 1.0f};
		vertex.tangent = {0.0f, 0.0f, 0.0f, 1.0f};
	}
	for (std::size_t index = 0; index < TracerIndexCount; ++index)
		indices[index] = topology[index];
	return TracerVertexCount;
}

export class TracerRenderer final
{
public:
	TracerRenderer() = default;
	TracerRenderer(const TracerRenderer &) = delete;
	TracerRenderer &operator=(const TracerRenderer &) = delete;

	~TracerRenderer() = default;

	bool Set_Description(PropRenderer &renderer, const TracerDescription &description)
	{
		if (!Is_Valid_Tracer(description))
			return false;
		if (m_mesh.Is_Valid() && description.length == m_description.length
			&& description.width == m_description.width && description.color == m_description.color)
			return true;

		std::array<PropVertex, TracerVertexCount> vertices{};
		std::array<std::uint32_t, TracerIndexCount> indices{};
		if (Build_Tracer_Geometry(description, vertices, indices) == 0)
			return false;

		// Publish a replacement so geometry is never mutated after it has been
		// handed to another graphics consumer.
		const PropMeshHandle replacement = renderer.Create_Mesh(vertices, indices);
		if (!replacement.Is_Valid())
			return false;
		const PropMeshHandle previous = m_mesh;
		m_mesh = replacement;
		m_description = description;
		if (previous.Is_Valid())
			renderer.Destroy_Mesh(previous);
		return true;
	}

	bool Draw(PropRenderer &renderer, CommandList &commands, const TracerDrawData &data) const
	{
		if (!m_mesh.Is_Valid() || !Is_Valid_Draw_Data(data))
			return false;
		return renderer.Draw(commands, m_mesh, Make_Style(data), Make_Parameters(data), {});
	}

	bool Submit(PropRenderer &renderer, PropSubmission &submission, const TracerDrawData &data) const
	{
		if (!m_mesh.Is_Valid() || renderer.Mesh_Geometry(m_mesh) == nullptr || !Is_Valid_Draw_Data(data))
			return false;
		// The legacy prelit draw submitted directly to the immediate command
		// list. The authored queue controls when this call occurs; it must not
		// move tracers into material or transparent batching.
		return submission.Submit(m_mesh, Make_Style(data), Make_Parameters(data), {},
			PropDrawPhase::Immediate);
	}

	void Release(PropRenderer &renderer) noexcept
	{
		if (m_mesh.Is_Valid())
			renderer.Destroy_Mesh(m_mesh);
		m_mesh = {};
		m_description = {};
	}

	PropMeshHandle Mesh() const noexcept { return m_mesh; }
	const TracerDescription &Description() const noexcept { return m_description; }

private:
	static bool Is_Valid_Draw_Data(const TracerDrawData &data) noexcept
	{
		for (const float value : data.view_projection)
			if (!std::isfinite(value))
				return false;
		for (const float value : data.view)
			if (!std::isfinite(value))
				return false;
		for (const float value : data.world)
			if (!std::isfinite(value))
				return false;
		for (const float value : data.camera_position)
			if (!std::isfinite(value))
				return false;
		for (const float value : data.camera_depth)
			if (!std::isfinite(value))
				return false;
		return std::isfinite(data.opacity);
	}

	static PropStyle Make_Style(const TracerDrawData &data) noexcept
	{
		PropStyle style;
		const bool transparent = data.opacity < 1.0f;
		style.blend = transparent ? RHIBlendMode::Alpha : RHIBlendMode::Disabled;
		style.depth_write = !transparent;
		style.depth_test = true;
		style.source_blend = transparent ? RHIBlendFactor::SourceAlpha : RHIBlendFactor::One;
		style.destination_blend = transparent ? RHIBlendFactor::InverseSourceAlpha : RHIBlendFactor::Zero;
		style.cull = RHICullMode::Back;
		style.front_counter_clockwise = data.front_counter_clockwise;
		return style;
	}

	static PropParameters Make_Parameters(const TracerDrawData &data) noexcept
	{
		PropParameters parameters;
		parameters.view_projection = data.view_projection;
		parameters.view = data.view;
		parameters.world = data.world;
		parameters.camera_position = data.camera_position;
		parameters.textured = 0.0f;
		parameters.primary_gradient = 1.0f;
		parameters.opacity = Quantize_Opacity(data.opacity);
		parameters.normal_in_world_space = 1.0f;
		return parameters;
	}

	PropMeshHandle m_mesh{};
	TracerDescription m_description{};
};

}
