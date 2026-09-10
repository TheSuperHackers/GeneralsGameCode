module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

export module Graphics.Scene.Ring.Runtime;

import Assets.Math;
import Assets.Rings;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Primitives.Geometry;

namespace Graphics
{

// The legacy ring has one null LOD followed by twenty generated annuli.  The
// values are kept here rather than in the status-circle renderer because the
// two ring responsibilities have different geometry policies.
export inline constexpr std::uint32_t AuthoredRingLODCount = 20;
export inline constexpr std::uint32_t AuthoredRingLowestLOD = 10;
export inline constexpr std::uint32_t AuthoredRingHighestLOD = 50;
export inline constexpr std::size_t AuthoredRingValueCount = AuthoredRingLODCount + 2;
export inline constexpr std::size_t AuthoredRingCostCount = AuthoredRingLODCount + 1;

export struct AuthoredRingBounds final
{
	Assets::Vector3f center{};
	Assets::Vector3f extent{};
};

export struct AuthoredRingRuntimeState final
{
	Assets::Color4f color{0.75f, 0.75f, 0.75f, 1.0f};
	float alpha = 1.0f;
	Assets::Vector2f inner_scale{1.0f, 1.0f};
	Assets::Vector2f outer_scale{1.0f, 1.0f};
	float animation_time = 0.0f;
	// Visibility controlled by authored hidden and animation-hidden state.
	// Camera culling is tracked independently by Is_Visible().
	bool visible = true;
	bool animating = false;
};

export struct AuthoredRingGeometryView final
{
	std::span<const PropVertex> vertices{};
	std::span<const std::uint32_t> indices{};
	std::uint32_t segments = 0;
	Assets::Vector2f inner_extent{};
	Assets::Vector2f outer_extent{};
	Assets::Vector2f inner_scale{1.0f, 1.0f};
	Assets::Vector2f outer_scale{1.0f, 1.0f};
	float texture_tiles = 0.0f;
};

// This is the complete CPU data needed by the scene adapter for one ring
// draw.  Resource handles and command submission remain outside this module;
// the adapter supplies those to the generic prop renderer.
export struct AuthoredRingDrawData final
{
	AuthoredRingGeometryView geometry{};
	AuthoredRingBounds bounds{};
	Assets::Color4f vertex_color{1.0f, 1.0f, 1.0f, 1.0f};
	Assets::RingMaterialDesc material{};
	bool textured = false;
	bool camera_aligned = false;
	bool sort_required = false;
	std::uint32_t ordered_layer = 0;
};

namespace AuthoredRingDetail
{

template <typename Key, typename Value, typename Lerp>
Value Evaluate(const std::vector<Key> &keys, float time, Value fallback, Lerp &&lerp)
{
	if (keys.empty())
		return fallback;
	if (keys.size() == 1 || time >= keys.back().time)
		return keys.back().value;

	std::size_t first = 0;
	// This deliberately includes the interval before the first key.  The
	// original LERP channel extrapolates from that first interval instead of
	// clamping to the first value.
	while (first + 1 < keys.size() && !(time < keys[first + 1].time))
		++first;
	if (first + 1 >= keys.size())
		return keys.back().value;

	const float denominator = keys[first + 1].time - keys[first].time;
	if (denominator == 0.0f)
		return keys[first].value;
	const float fraction = (time - keys[first].time) / denominator;
	return lerp(keys[first].value, keys[first + 1].value, fraction);
}

Assets::Color4f Lerp_Color(Assets::Color4f left, Assets::Color4f right, float fraction) noexcept
{
	return {
		left.r + (right.r - left.r) * fraction,
		left.g + (right.g - left.g) * fraction,
		left.b + (right.b - left.b) * fraction,
		left.a + (right.a - left.a) * fraction};
}

Assets::Vector2f Lerp_Scale(Assets::Vector2f left, Assets::Vector2f right, float fraction) noexcept
{
	return {
		left.x + (right.x - left.x) * fraction,
		left.y + (right.y - left.y) * fraction};
}

float Lerp_Scalar(float left, float right, float fraction) noexcept
{
	return left + (right - left) * fraction;
}

bool Is_Finite(Assets::Vector2f value) noexcept
{
	return std::isfinite(value.x) && std::isfinite(value.y);
}

bool Is_Finite(Assets::Vector3f value) noexcept
{
	return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool Is_Finite(Assets::Color4f value) noexcept
{
	return std::isfinite(value.r) && std::isfinite(value.g) &&
		std::isfinite(value.b) && std::isfinite(value.a);
}

Assets::Color4f Quantized_Color(const AuthoredRingRuntimeState &state,
	Assets::RingBlendFactor destination) noexcept
{
	const Assets::Color4f source = destination == Assets::RingBlendFactor::One
		? Assets::Color4f{state.color.r * state.alpha, state.color.g * state.alpha,
			state.color.b * state.alpha, 1.0f}
		: Assets::Color4f{state.color.r, state.color.g, state.color.b, state.alpha};
	return Assets::Color_From_ARGB(Assets::Color_To_ARGB(source));
}

std::uint32_t Ordered_Layer(const Assets::RingMaterialDesc &material) noexcept
{
	const bool alpha_test_category = material.alpha_test &&
		(material.destination_blend == Assets::RingBlendFactor::Zero ||
			(material.source_blend == Assets::RingBlendFactor::SourceAlpha &&
				material.destination_blend == Assets::RingBlendFactor::InverseSourceAlpha));
	if ((!material.alpha_test && material.destination_blend == Assets::RingBlendFactor::Zero) ||
		alpha_test_category)
		return 0;
	if (material.source_blend == Assets::RingBlendFactor::One &&
		material.destination_blend == Assets::RingBlendFactor::One)
		return 10;
	if (material.source_blend == Assets::RingBlendFactor::One &&
		material.destination_blend == Assets::RingBlendFactor::InverseSourceColor)
		return 15;
	return 20;
}

bool Depth_Sort_Eligible(const Assets::RingMaterialDesc &material) noexcept
{
	return material.destination_blend != Assets::RingBlendFactor::Zero && !material.alpha_test;
}

std::uint32_t Segments_For_LOD(int lod) noexcept
{
	if (lod <= 0)
		return 0;
	return AuthoredRingLowestLOD + static_cast<std::uint32_t>(lod - 1) *
		((AuthoredRingHighestLOD - AuthoredRingLowestLOD) / AuthoredRingLODCount);
}

}

export class AuthoredRingRuntime final
{
public:
	AuthoredRingRuntime()
	{
		Calculate_Value_Array(1.0f, m_values);
	}

	explicit AuthoredRingRuntime(const Assets::RingAssetDesc &description)
		: m_description(description),
		  m_state{description.default_color, description.default_alpha,
			 description.default_inner_scale, description.default_outer_scale, 0.0f, true, false},
		  m_center(description.center),
		  m_inner_extent(description.inner_extent),
		  m_outer_extent(description.outer_extent),
		  m_extent(description.extent)
	{
		Calculate_Value_Array(1.0f, m_values);
	}

	AuthoredRingRuntime(const AuthoredRingRuntime &) = default;
	AuthoredRingRuntime &operator=(const AuthoredRingRuntime &) = default;
	AuthoredRingRuntime(AuthoredRingRuntime &&) noexcept = default;
	AuthoredRingRuntime &operator=(AuthoredRingRuntime &&) noexcept = default;

	const Assets::RingAssetDesc &Description() const noexcept { return m_description; }
	const AuthoredRingRuntimeState &State() const noexcept { return m_state; }
	AuthoredRingBounds Bounds() const noexcept { return {m_center, m_extent}; }

	void Start_Animating() noexcept
	{
		m_state.animating = true;
		m_state.animation_time = 0.0f;
	}

	void Stop_Animating() noexcept
	{
		m_state.animating = false;
		m_state.animation_time = 0.0f;
	}

	void Restart_Animation() noexcept
	{
		m_state.animation_time = 0.0f;
	}

	bool Is_Animating() const noexcept { return m_state.animating; }

	void Set_Visible(bool visible) noexcept
	{
		// This is the scene/camera visibility bit. It must not stop animation or
		// make draw-data extraction unavailable to a caller doing its own cull.
		m_camera_visible = visible;
	}

	bool Is_Visible() const noexcept { return m_camera_visible; }
	bool Is_Authored_Visible() const noexcept { return m_state.visible; }

	void Set_Hidden(bool hidden) noexcept
	{
		m_hidden = hidden;
		Update_On_Visibility();
	}

	void Set_Animation_Hidden(bool hidden) noexcept
	{
		m_animation_hidden = hidden;
		Update_On_Visibility();
	}

	void Set_Force_Visible(bool force_visible) noexcept
	{
		m_force_visible = force_visible;
	}

	void Advance(float delta_seconds) noexcept
	{
		if (!m_state.animating || m_description.color_track.keys.empty() &&
			m_description.alpha_track.keys.empty() && m_description.inner_scale_track.keys.empty() &&
			m_description.outer_scale_track.keys.empty())
			return;
		if (!std::isfinite(delta_seconds))
			return;

		if (m_description.animation_duration > 0.0f)
			m_state.animation_time += delta_seconds / m_description.animation_duration;
		else
			m_state.animation_time = 1.0f;

		// Keep the source's one-step loop behavior.  In particular, an exact
		// one is held for the final key and a large delta is reduced once.
		if (m_description.animation_loop && m_state.animation_time > 1.0f)
			m_state.animation_time -= 1.0f;

		if (!m_description.color_track.keys.empty())
			m_state.color = AuthoredRingDetail::Evaluate(m_description.color_track.keys, m_state.animation_time,
				m_state.color, AuthoredRingDetail::Lerp_Color);
		if (!m_description.alpha_track.keys.empty())
			m_state.alpha = AuthoredRingDetail::Evaluate(m_description.alpha_track.keys, m_state.animation_time,
				m_state.alpha, AuthoredRingDetail::Lerp_Scalar);
		if (!m_description.inner_scale_track.keys.empty())
			m_state.inner_scale = AuthoredRingDetail::Evaluate(m_description.inner_scale_track.keys,
				m_state.animation_time, m_state.inner_scale, AuthoredRingDetail::Lerp_Scale);
		if (!m_description.outer_scale_track.keys.empty()) {
			m_state.outer_scale = AuthoredRingDetail::Evaluate(m_description.outer_scale_track.keys,
				m_state.animation_time, m_state.outer_scale, AuthoredRingDetail::Lerp_Scale);
			Update_Outer_Bounds();
		}
	}

	void Set_Color(Assets::Color4f color) noexcept
	{
		if (AuthoredRingDetail::Is_Finite(color))
			m_state.color = color;
	}

	void Set_Alpha(float alpha) noexcept
	{
		if (std::isfinite(alpha))
			m_state.alpha = alpha;
	}

	void Set_Inner_Scale(Assets::Vector2f scale) noexcept
	{
		if (AuthoredRingDetail::Is_Finite(scale))
			m_state.inner_scale = scale;
	}

	void Set_Outer_Scale(Assets::Vector2f scale) noexcept
	{
		if (AuthoredRingDetail::Is_Finite(scale))
			m_state.outer_scale = scale;
	}

	Assets::Color4f Get_Default_Color() const noexcept
	{
		return m_description.color_track.keys.empty() ? m_state.color :
			m_description.color_track.keys.front().value;
	}

	float Get_Default_Alpha() const noexcept
	{
		return m_description.alpha_track.keys.empty() ? m_state.alpha : m_description.alpha_track.keys.front().value;
	}

	Assets::Vector2f Get_Default_Inner_Scale() const noexcept
	{
		return m_description.inner_scale_track.keys.empty() ? m_state.inner_scale :
			m_description.inner_scale_track.keys.front().value;
	}

	Assets::Vector2f Get_Default_Outer_Scale() const noexcept
	{
		return m_description.outer_scale_track.keys.empty() ? m_state.outer_scale :
			m_description.outer_scale_track.keys.front().value;
	}

	void Set_Inner_Extent(Assets::Vector2f extent) noexcept
	{
		if (AuthoredRingDetail::Is_Finite(extent))
			m_inner_extent = extent;
	}

	void Set_Outer_Extent(Assets::Vector2f extent) noexcept
	{
		if (AuthoredRingDetail::Is_Finite(extent)) {
			m_outer_extent = extent;
			m_extent = {extent.x, extent.y, 0.0f};
		}
	}

	Assets::Vector2f Inner_Extent() const noexcept { return m_inner_extent; }
	Assets::Vector2f Outer_Extent() const noexcept { return m_outer_extent; }

	void Set_Local_Center_Extent(Assets::Vector3f center, Assets::Vector3f extent) noexcept
	{
		if (AuthoredRingDetail::Is_Finite(center) && AuthoredRingDetail::Is_Finite(extent)) {
			m_center = center;
			m_extent = extent;
		}
	}

	void Set_Local_Min_Max(Assets::Vector3f minimum, Assets::Vector3f maximum) noexcept
	{
		if (!std::isfinite(minimum.x) || !std::isfinite(minimum.y) || !std::isfinite(minimum.z) ||
			!std::isfinite(maximum.x) || !std::isfinite(maximum.y) || !std::isfinite(maximum.z))
			return;
		m_center = {(maximum.x + minimum.x) * 0.5f, (maximum.y + minimum.y) * 0.5f,
			(maximum.z + minimum.z) * 0.5f};
		m_extent = {(maximum.x - minimum.x) * 0.5f, (maximum.y - minimum.y) * 0.5f,
			(maximum.z - minimum.z) * 0.5f};
	}

	void Set_Animation_Duration(float duration) noexcept
	{
		if (std::isfinite(duration)) {
			m_description.animation_duration = duration;
			m_state.animation_time = 0.0f;
		}
	}

	float Animation_Duration() const noexcept { return m_description.animation_duration; }

	void Set_Animation_Loop(bool loop) noexcept { m_description.animation_loop = loop; }
	bool Animation_Loop() const noexcept { return m_description.animation_loop; }
	void Set_Camera_Aligned(bool aligned) noexcept { m_description.camera_aligned = aligned; }
	bool Camera_Aligned() const noexcept { return m_description.camera_aligned; }

	void Set_Texture_Tiling(int count) noexcept { m_description.texture_tile_count = count; }
	int Texture_Tiling() const noexcept { return m_description.texture_tile_count; }

	void Scale(float scale) noexcept { Scale(scale, scale, scale); }

	void Scale(float scale_x, float scale_y, float /*scale_z*/) noexcept
	{
		if (!std::isfinite(scale_x) || !std::isfinite(scale_y))
			return;
		m_state.inner_scale.x *= scale_x;
		m_state.inner_scale.y *= scale_y;
		m_state.outer_scale.x *= scale_x;
		m_state.outer_scale.y *= scale_y;
		for (auto &key : m_description.inner_scale_track.keys) {
			key.value.x *= scale_x;
			key.value.y *= scale_y;
		}
		for (auto &key : m_description.outer_scale_track.keys) {
			key.value.x *= scale_x;
			key.value.y *= scale_y;
		}
	}

	void Set_LOD_Bias(float bias) noexcept
	{
		if (std::isfinite(bias))
			m_lod_bias = std::max(bias, 0.0f);
	}

	float LOD_Bias() const noexcept { return m_lod_bias; }

	void Prepare_LOD(float screen_area) noexcept
	{
		if (m_state.visible && std::isfinite(screen_area))
			Calculate_Value_Array(screen_area, m_values);
	}

	void Increment_LOD() noexcept
	{
		if (m_lod < static_cast<int>(AuthoredRingLODCount))
			++m_lod;
	}

	void Decrement_LOD() noexcept
	{
		if (m_lod > 0)
			--m_lod;
	}

	void Set_LOD_Level(int lod) noexcept
	{
		m_lod = std::clamp(lod, 0, static_cast<int>(AuthoredRingLODCount));
	}

	int LOD_Level() const noexcept { return m_lod; }
	int LOD_Count() const noexcept { return static_cast<int>(AuthoredRingLODCount + 1); }

	std::uint32_t Num_Polys() const noexcept
	{
		return m_lod == 0 ? 0 : AuthoredRingDetail::Segments_For_LOD(m_lod) * 2;
	}

	float Cost() const noexcept { return static_cast<float>(Num_Polys()); }
	float Value() const noexcept { return m_values[static_cast<std::size_t>(m_lod)]; }
	float Post_Increment_Value() const noexcept { return m_values[static_cast<std::size_t>(m_lod) + 1]; }

	bool Calculate_Cost_Value_Arrays(float screen_area, std::span<float> values,
		std::span<float> costs) const noexcept
	{
		if (values.size() < AuthoredRingValueCount || costs.size() < AuthoredRingCostCount ||
			!std::isfinite(screen_area))
			return false;
		Calculate_Value_Array(screen_area, values);
		for (std::size_t lod = 0; lod < AuthoredRingCostCount; ++lod)
			costs[lod] = lod == 0 ? 0.000001f : static_cast<float>(AuthoredRingDetail::Segments_For_LOD(static_cast<int>(lod)) * 2);
		return true;
	}

	// Rebuild the annulus after animation or state changes and expose the
	// resulting draw data without transferring ownership of its CPU storage.
	bool Build_Draw_Data(AuthoredRingDrawData &result)
	{
		result = {};
		if (!m_state.visible || m_lod == 0 || !AuthoredRingDetail::Is_Finite(m_inner_extent) ||
			!AuthoredRingDetail::Is_Finite(m_outer_extent) || !AuthoredRingDetail::Is_Finite(m_state.inner_scale) ||
			!AuthoredRingDetail::Is_Finite(m_state.outer_scale) || !AuthoredRingDetail::Is_Finite(m_state.color) ||
			!AuthoredRingDetail::Is_Finite(m_center) || !AuthoredRingDetail::Is_Finite(m_extent) ||
			!std::isfinite(m_state.alpha))
			return false;

		const std::uint32_t segments = AuthoredRingDetail::Segments_For_LOD(m_lod);
		if (m_geometry_segments != segments) {
			if (!m_geometry.Generate(segments))
				return false;
			m_geometry_segments = segments;
		}
		const Assets::Vector2f inner{
			m_inner_extent.x * m_state.inner_scale.x,
			m_inner_extent.y * m_state.inner_scale.y};
		const Assets::Vector2f outer{
			m_outer_extent.x * m_state.outer_scale.x,
			m_outer_extent.y * m_state.outer_scale.y};
		if (!m_geometry.Scale({inner.x, inner.y}, {outer.x, outer.y}))
			return false;

		const Assets::Color4f color = AuthoredRingDetail::Quantized_Color(m_state,
			m_description.material.destination_blend);
		m_geometry.Set_Color({color.r, color.g, color.b, color.a});
		const bool textured = !m_description.texture_name.empty();
		const float tiles = textured ? static_cast<float>(m_description.texture_tile_count) : 0.0f;
		if (!m_geometry.Set_Tiling(tiles))
			return false;

		result.geometry = {
			m_geometry.Vertices(), m_geometry.Indices(), segments,
			m_inner_extent, m_outer_extent, m_state.inner_scale, m_state.outer_scale, tiles};
		result.bounds = {m_center, m_extent};
		result.vertex_color = color;
		result.material = m_description.material;
		result.textured = textured;
		result.camera_aligned = m_description.camera_aligned;
		result.ordered_layer = AuthoredRingDetail::Ordered_Layer(m_description.material);
		result.sort_required = AuthoredRingDetail::Depth_Sort_Eligible(m_description.material);
		return true;
	}

	static Assets::Color4f Quantize_Color(const AuthoredRingRuntimeState &state,
		Assets::RingBlendFactor destination) noexcept
	{
		return AuthoredRingDetail::Quantized_Color(state, destination);
	}

	static std::uint32_t Ordered_Layer(const Assets::RingMaterialDesc &material) noexcept
	{
		return AuthoredRingDetail::Ordered_Layer(material);
	}

private:
	void Update_On_Visibility() noexcept
	{
		const bool animation_visible = !m_hidden && !m_animation_hidden;
		m_state.visible = animation_visible;
		if (animation_visible && !m_state.animating)
			Start_Animating();
		else if (!animation_visible && m_state.animating)
			Stop_Animating();
	}

	void Update_Outer_Bounds() noexcept
	{
		m_extent = {m_outer_extent.x * m_state.outer_scale.x,
			m_outer_extent.y * m_state.outer_scale.y, 0.0f};
	}

	void Calculate_Value_Array(float screen_area, std::span<float> values) const noexcept
	{
		if (values.size() < AuthoredRingValueCount)
			return;
		values[0] = (std::numeric_limits<float>::max)();
		for (std::size_t lod = 1; lod <= AuthoredRingLODCount; ++lod) {
			const float polycount = static_cast<float>(AuthoredRingDetail::Segments_For_LOD(static_cast<int>(lod)) * 2);
			const float benefit_factor = 1.0f - 0.5f / (polycount * polycount);
			values[lod] = (benefit_factor * screen_area * m_lod_bias) / polycount;
		}
		values[AuthoredRingLODCount + 1] = -1.0f;
	}

	Assets::RingAssetDesc m_description{};
	AuthoredRingRuntimeState m_state{};
	Assets::Vector3f m_center{};
	Assets::Vector2f m_inner_extent{0.5f, 0.5f};
	Assets::Vector2f m_outer_extent{1.0f, 1.0f};
	Assets::Vector3f m_extent{1.0f, 1.0f, 1.0f};
	AnnulusGeometry m_geometry;
	std::uint32_t m_geometry_segments = 0;
	std::array<float, AuthoredRingValueCount> m_values{};
	float m_lod_bias = 1.0f;
	int m_lod = static_cast<int>(AuthoredRingLODCount);
	bool m_hidden = false;
	bool m_animation_hidden = false;
	bool m_force_visible = false;
	bool m_camera_visible = true;
};

}
