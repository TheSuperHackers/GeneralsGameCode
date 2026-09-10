module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.Scene.Debug.CollisionBox;

import Assets.Math;
import Graphics.RHI;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;

namespace Graphics
{

export inline constexpr std::size_t CollisionBoxVertexCount = 8u;
export inline constexpr std::size_t CollisionBoxIndexCount = 36u;

export struct CollisionBoxDrawData final
{
	std::array<float, 3> center{};
	std::array<float, 3> extent{1, 1, 1};
	std::array<float, 4> color{1, 1, 1, .25f};
	std::array<float, 16> view_projection{1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1};
	std::array<float, 16> world{1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1};
	std::uint32_t collision_type = 0;
	bool front_counter_clockwise = false;
};

export struct CollisionBoxGeometry final
{
	std::array<PropVertex, CollisionBoxVertexCount> vertices{};
	std::array<std::uint32_t, CollisionBoxIndexCount> indices{};
};

namespace CollisionBoxDetail
{

constexpr std::array<std::array<float, 3>, CollisionBoxVertexCount> Corners{{
	{{1, 1, 1}}, {{-1, 1, 1}}, {{-1, -1, 1}}, {{1, -1, 1}},
	{{1, 1, -1}}, {{-1, 1, -1}}, {{-1, -1, -1}}, {{1, -1, -1}}
}};

constexpr std::array<std::uint32_t, CollisionBoxIndexCount> Topology{{
	0, 1, 2, 0, 2, 3,
	4, 7, 6, 4, 6, 5,
	0, 3, 7, 0, 7, 4,
	1, 5, 6, 1, 6, 2,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7
}};

bool Finite(const std::array<float, 3> &value) noexcept
{
	for (const float component : value)
		if (!std::isfinite(component))
			return false;
	return true;
}

bool Finite(const std::array<float, 4> &value) noexcept
{
	for (const float component : value)
		if (!std::isfinite(component))
			return false;
	return true;
}

}

// Builds the same eight-corner, twelve-triangle topology used by W3D box
// assets. Color quantization intentionally goes through the shared asset
// boundary so CPU and GPU paths see the same packed ARGB values.
export bool Build_Collision_Box_Geometry(const CollisionBoxDrawData &data,
	CollisionBoxGeometry &result) noexcept
{
	if (!CollisionBoxDetail::Finite(data.center)
		|| !CollisionBoxDetail::Finite(data.extent)
		|| !CollisionBoxDetail::Finite(data.color))
		return false;

	CollisionBoxGeometry next;
	const auto packed = Assets::Color_To_ARGB({data.color[0], data.color[1], data.color[2], data.color[3]});
	const auto color = Assets::Color_From_ARGB(packed);
	for (std::size_t index = 0; index < CollisionBoxVertexCount; ++index) {
		next.vertices[index].position = {
			data.center[0] + CollisionBoxDetail::Corners[index][0] * data.extent[0],
			data.center[1] + CollisionBoxDetail::Corners[index][1] * data.extent[1],
			data.center[2] + CollisionBoxDetail::Corners[index][2] * data.extent[2]};
		next.vertices[index].color = {color.r, color.g, color.b, color.a};
	}
	next.indices = CollisionBoxDetail::Topology;
	result = next;
	return true;
}

namespace
{
std::uint32_t g_collision_box_display_mask = 0;
}

export void Set_Collision_Box_Display_Mask(int mask) noexcept
{
	g_collision_box_display_mask = static_cast<std::uint32_t>(mask);
}

export int Get_Collision_Box_Display_Mask() noexcept
{
	return static_cast<int>(g_collision_box_display_mask);
}

// Owns one persistent prop mesh for a box instance. World and camera data are
// submitted per draw; only local geometry and its packed color are retained.
export class CollisionBoxRenderer final
{
public:
	CollisionBoxRenderer() = default;
	CollisionBoxRenderer(const CollisionBoxRenderer &) = delete;
	CollisionBoxRenderer &operator=(const CollisionBoxRenderer &) = delete;
	~CollisionBoxRenderer()
	{
		if (m_owner != nullptr && m_mesh.Is_Valid())
			m_owner->Destroy_Mesh(m_mesh);
	}

	bool Submit(PropRenderer &renderer, PropSubmission &submission,
		const CollisionBoxDrawData &data)
	{
		if ((g_collision_box_display_mask & data.collision_type) == 0)
			return false;
		if (!Ensure_Geometry(renderer, data))
			return false;

		PropParameters parameters;
		parameters.view_projection = data.view_projection;
		parameters.world = data.world;
		parameters.textured = 0;
		parameters.primary_gradient = 1;
		parameters.opacity = 1;
		parameters.normal_in_world_space = 1;

		PropStyle style;
		style.blend = RHIBlendMode::Alpha;
		style.depth_write = false;
		style.depth_test = true;
		style.source_blend = RHIBlendFactor::SourceAlpha;
		style.destination_blend = RHIBlendFactor::InverseSourceAlpha;
		style.depth_comparison = RHIComparison::LessEqual;
		style.cull = RHICullMode::Back;
		style.front_counter_clockwise = data.front_counter_clockwise;
		style.color_write_mask = 15;
		return submission.Submit(m_mesh, style, parameters,
			std::span<const RHITextureHandle>{}, PropDrawPhase::Immediate);
	}

	void Release(PropRenderer &renderer) noexcept
	{
		if (m_mesh.Is_Valid())
			renderer.Destroy_Mesh(m_mesh);
		m_mesh = {};
		m_owner = nullptr;
		m_has_geometry = false;
	}

	PropMeshHandle Mesh() const noexcept { return m_mesh; }

private:
	bool Ensure_Geometry(PropRenderer &renderer, const CollisionBoxDrawData &data)
	{
		const auto packed = Assets::Color_To_ARGB({data.color[0], data.color[1], data.color[2], data.color[3]});
		const auto color = Assets::Color_From_ARGB(packed);
		const std::array<float, 4> quantized_color{color.r, color.g, color.b, color.a};
		if (m_owner == &renderer && m_has_geometry
			&& m_center == data.center && m_extent == data.extent && m_color == quantized_color)
			return true;

		CollisionBoxGeometry geometry;
		if (!Build_Collision_Box_Geometry(data, geometry))
			return false;
		if (m_owner != &renderer && m_mesh.Is_Valid()) {
			if (m_owner != nullptr)
				m_owner->Destroy_Mesh(m_mesh);
			m_mesh = {};
			m_has_geometry = false;
		}
		if (!m_mesh.Is_Valid()) {
			m_mesh = renderer.Create_Mesh(geometry.vertices, geometry.indices);
			if (!m_mesh.Is_Valid())
				return false;
		} else if (!renderer.Update_Mesh(m_mesh, geometry.vertices, geometry.indices)) {
			return false;
		}
		m_owner = &renderer;
		m_center = data.center;
		m_extent = data.extent;
		m_color = quantized_color;
		m_has_geometry = true;
		return true;
	}

	PropRenderer *m_owner = nullptr;
	PropMeshHandle m_mesh{};
	std::array<float, 3> m_center{};
	std::array<float, 3> m_extent{};
	std::array<float, 4> m_color{};
	bool m_has_geometry = false;
};

}
