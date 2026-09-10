module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <vector>

export module Graphics.Scene.Sphere;

import Assets.Spheres;
import Assets.Math;
import Graphics.RHI;
import Graphics.Resources.Textures.Sampling;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Models.AnimationRotation;
import Graphics.Scene.Primitives.Geometry;
import Graphics.Scene.Props.Material;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;

namespace Graphics
{

export inline constexpr std::uint32_t SphereLODCount = 10;
export inline constexpr std::uint32_t SphereLowestLOD = 7;
export inline constexpr std::uint32_t SphereHighestLOD = 17;
export inline constexpr std::uint32_t SphereLODValueCount = SphereLODCount + 2;
export inline constexpr float SphereNullLODCost = 0.000001f;

export struct SphereRuntimeState final
{
	Assets::Vector3f color{0.75f, 0.75f, 0.75f};
	float alpha = 1.0f;
	Assets::Vector3f scale{1.0f, 1.0f, 1.0f};
	std::array<float, 4> vector_rotation{0.0f, 0.0f, 0.0f, 1.0f};
	float vector_intensity = 1.0f;
	float animation_time = 0.0f;
	bool visible = true;
	bool animation_hidden = false;
	bool force_visible = false;
	bool animating = true;
	std::uint32_t lod = SphereLODCount;
	float lod_bias = 1.0f;
};

export struct SphereBounds final
{
	Assets::Vector3f center{};
	Assets::Vector3f extent{};
	float radius = 0.0f;
};

export struct SphereDrawInput final
{
	std::array<float, 16> view_projection{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1};
	std::array<float, 16> projection{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1};
	std::array<float, 16> view{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1};
	std::array<float, 16> world{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1};
	std::array<float, 4> camera_position{};
	TextureSampling texture_sampling{};
	SceneDrawParameters scene{};
	float screen_area = 1.0f;
	bool sorting_enabled = false;
	bool front_counter_clockwise = false;
};

export struct SphereDrawState final
{
	PropMaterial material{};
	PropStyle style{};
	PropParameters parameters{};
	bool additive = false;
	bool sort_required = false;
	float sort_depth = 0.0f;
};

namespace SphereDetail
{

bool Finite(Assets::Vector3f value) noexcept
{
	return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool Finite(std::array<float, 4> value) noexcept
{
	for (float component : value)
		if (!std::isfinite(component))
			return false;
	return true;
}

Assets::Vector3f Add(Assets::Vector3f left, Assets::Vector3f right) noexcept
{
	return {left.x + right.x, left.y + right.y, left.z + right.z};
}

Assets::Vector3f Multiply(Assets::Vector3f left, Assets::Vector3f right) noexcept
{
	return {left.x * right.x, left.y * right.y, left.z * right.z};
}

Assets::Vector3f Lerp(Assets::Vector3f first, Assets::Vector3f second, float weight) noexcept
{
	return {first.x + (second.x - first.x) * weight,
		first.y + (second.y - first.y) * weight,
		first.z + (second.z - first.z) * weight};
}

template <typename Key, typename Value, typename ReadValue, typename Interpolate>
Value Sample_Track(const std::vector<Key> &keys, float time, Value fallback,
	ReadValue &&read_value, Interpolate &&interpolate) noexcept
{
	if (keys.empty())
		return fallback;
	if (keys.size() == 1 || !(time < keys.back().time))
		return read_value(keys.back());

	// The source channel extrapolates before the first key using the first
	// interval. This is intentional for authored sphere animations.
	std::size_t next = 1;
	while (next < keys.size() && !(time < keys[next].time))
		++next;
	if (next == keys.size())
		return read_value(keys.back());
	const Key &first = keys[next - 1];
	const Key &second = keys[next];
	const float denominator = second.time - first.time;
	if (denominator == 0.0f)
		return read_value(first);
	return interpolate(read_value(first), read_value(second), (time - first.time) / denominator);
}

Assets::Vector3f Rotate_X(std::array<float, 4> quaternion) noexcept
{
	const float x = quaternion[0];
	const float y = quaternion[1];
	const float z = quaternion[2];
	const float w = quaternion[3];
	// This is the direct quaternion-vector product used by the source object.
	return {
		w * w + x * x - y * y - z * z,
		2.0f * (x * y + w * z),
		2.0f * (x * z - w * y)};
}

std::array<float, 16> Identity() noexcept
{
	return {1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1};
}

Assets::Vector3f Transform_Point(const std::array<float, 16> &matrix,
	Assets::Vector3f point) noexcept
{
	return {
		matrix[0] * point.x + matrix[1] * point.y + matrix[2] * point.z + matrix[3],
		matrix[4] * point.x + matrix[5] * point.y + matrix[6] * point.z + matrix[7],
		matrix[8] * point.x + matrix[9] * point.y + matrix[10] * point.z + matrix[11]};
}

std::array<float, 16> Scale_World(const std::array<float, 16> &world,
	Assets::Vector3f scale) noexcept
{
	std::array<float, 16> result = world;
	// Matrix3D::Scale scales basis columns and keeps the affine translation in
	// world space. Preserve that contract for rotated and translated objects.
	result[0] *= scale.x;
	result[4] *= scale.x;
	result[8] *= scale.x;
	result[1] *= scale.y;
	result[5] *= scale.y;
	result[9] *= scale.y;
	result[2] *= scale.z;
	result[6] *= scale.z;
	result[10] *= scale.z;
	return result;
}

RHIComparison Map_Depth_Compare(Assets::SphereDepthCompare compare) noexcept
{
	return static_cast<RHIComparison>(compare);
}

MaterialFogMode Map_Fog(Assets::SphereFogMode mode) noexcept
{
	switch (mode) {
	case Assets::SphereFogMode::Enabled: return MaterialFogMode::Scene;
	case Assets::SphereFogMode::ScaleFragment: return MaterialFogMode::Black;
	case Assets::SphereFogMode::White: return MaterialFogMode::White;
	case Assets::SphereFogMode::Disabled: break;
	}
	return MaterialFogMode::Disabled;
}

RHIBlendFactor Map_Blend(Assets::SphereBlendFactor factor) noexcept
{
	switch (factor) {
	case Assets::SphereBlendFactor::Zero: return RHIBlendFactor::Zero;
	case Assets::SphereBlendFactor::One: return RHIBlendFactor::One;
	case Assets::SphereBlendFactor::SourceColor: return RHIBlendFactor::SourceColor;
	case Assets::SphereBlendFactor::InverseSourceColor: return RHIBlendFactor::InverseSourceColor;
	case Assets::SphereBlendFactor::SourceAlpha: return RHIBlendFactor::SourceAlpha;
	case Assets::SphereBlendFactor::InverseSourceAlpha: return RHIBlendFactor::InverseSourceAlpha;
	case Assets::SphereBlendFactor::SourceColorPreFog: return RHIBlendFactor::SourceColor;
	}
	return RHIBlendFactor::One;
}

bool Valid_Runtime_State(const SphereRuntimeState &state) noexcept
{
	return state.lod != 0 && state.lod <= SphereLODCount
		&& SphereDetail::Finite(state.color) && std::isfinite(state.alpha)
		&& SphereDetail::Finite(state.scale) && SphereDetail::Finite(state.vector_rotation)
		&& std::isfinite(state.vector_intensity);
}

bool Build_Sphere_Geometry_From(const Assets::SphereAssetDesc &asset,
	const SphereRuntimeState &state, SphereGeometry &geometry,
	std::vector<PropVertex> &vertices, std::vector<std::uint32_t> &indices) noexcept
{
	vertices.clear();
	indices.clear();
	if (!Valid_Runtime_State(state))
		return false;

	const bool additive = asset.material.destination_blend == Assets::SphereBlendFactor::One;
	const bool use_alpha_vector = (asset.attributes & Assets::SphereAttributeUseAlphaVector) != 0;
	const Assets::Vector3f direction = SphereDetail::Rotate_X(state.vector_rotation);
	const std::array<float, 3> opacity_direction{direction.x, direction.y, direction.z};
	if (!geometry.Set_Directional_Opacity(opacity_direction,
		use_alpha_vector ? state.vector_intensity : 0.0f,
		use_alpha_vector && (asset.attributes & Assets::SphereAttributeInverseAlpha) != 0,
		use_alpha_vector && additive))
		return false;
	vertices.assign(geometry.Vertices().begin(), geometry.Vertices().end());
	indices.assign(geometry.Indices().begin(), geometry.Indices().end());

	// The source sphere keeps the default material opacity for additive draws;
	// its current alpha only scales emissive colour in that path. Directional
	// opacity remains in the generated vertex channel.
	PropMaterial material;
	material.ambient = {0.0f, 0.0f, 0.0f};
	material.diffuse = {0.0f, 0.0f, 0.0f};
	material.specular = {0.0f, 0.0f, 0.0f};
	material.emissive = additive
		? std::array<float, 3>{state.color.x * state.alpha,
			state.color.y * state.alpha, state.color.z * state.alpha}
		: std::array<float, 3>{state.color.x, state.color.y, state.color.z};
	material.opacity = additive ? 0.25f : state.alpha;
	material.shininess = 0.0f;
	material.lighting = true;
	for (auto &vertex : vertices)
		Apply_Prop_Material(vertex, material);
	return true;
}

class SphereGeometryCache final
{
public:
	bool Initialize() noexcept
	{
		if (m_initialized)
			return true;
		for (std::uint32_t lod = 1; lod <= SphereLODCount; ++lod) {
			const std::uint32_t size = SphereLowestLOD + lod - 1;
			if (!m_geometry[lod - 1].Generate(1.0f, size, size))
				return false;
		}
		m_initialized = true;
		return true;
	}

	bool Build(const Assets::SphereAssetDesc &asset, const SphereRuntimeState &state,
		std::vector<PropVertex> &vertices, std::vector<std::uint32_t> &indices) noexcept
	{
		if (!Initialize() || !Valid_Runtime_State(state))
			return false;
		return Build_Sphere_Geometry_From(asset, state, m_geometry[state.lod - 1], vertices, indices);
	}

private:
	std::array<SphereGeometry, SphereLODCount> m_geometry{};
	bool m_initialized = false;
};

}

export SphereRuntimeState Make_Sphere_Runtime_State(const Assets::SphereAssetDesc &asset) noexcept
{
	SphereRuntimeState state;
	state.color = asset.default_color;
	state.alpha = asset.default_alpha;
	state.scale = asset.default_scale;
	state.vector_rotation = asset.default_vector_rotation;
	state.vector_intensity = asset.default_vector_intensity;
	state.animating = true;
	state.visible = true;
	state.animation_hidden = false;
	state.force_visible = false;
	state.lod = SphereLODCount;
	state.lod_bias = 1.0f;
	return state;
}

export bool Evaluate_Sphere_Animation(const Assets::SphereAssetDesc &asset,
	float animation_time, SphereRuntimeState &state) noexcept
{
	if (!std::isfinite(animation_time))
		return false;
	state.animation_time = animation_time;
	state.color = SphereDetail::Sample_Track(asset.color_track.keys, animation_time,
		state.color, [](const Assets::SphereColorKeyframe &key) { return key.value; },
		[](Assets::Vector3f first, Assets::Vector3f second, float weight) {
			return SphereDetail::Lerp(first, second, weight);
		});
	state.alpha = SphereDetail::Sample_Track(asset.alpha_track.keys, animation_time,
		state.alpha, [](const Assets::SphereAlphaKeyframe &key) { return key.value; },
		[](float first, float second, float weight) {
			return first + (second - first) * weight;
		});
	state.scale = SphereDetail::Sample_Track(asset.scale_track.keys, animation_time,
		state.scale, [](const Assets::SphereScaleKeyframe &key) { return key.value; },
		[](Assets::Vector3f first, Assets::Vector3f second, float weight) {
			return SphereDetail::Lerp(first, second, weight);
		});
	if (!asset.vector_track.keys.empty()) {
		const auto value = SphereDetail::Sample_Track(asset.vector_track.keys, animation_time,
			Assets::SphereVectorKeyframe{0.0f, asset.default_vector_rotation,
				asset.default_vector_intensity},
			[](const Assets::SphereVectorKeyframe &key) { return key; },
			[](const Assets::SphereVectorKeyframe &first,
				const Assets::SphereVectorKeyframe &second, float weight) {
				Assets::SphereVectorKeyframe result;
				result.rotation = Interpolate_Animation_Rotation(first.rotation, second.rotation, weight);
				result.intensity = first.intensity + (second.intensity - first.intensity) * weight;
				return result;
			});
		state.vector_rotation = value.rotation;
		state.vector_intensity = value.intensity;
	} else {
		// A caller may have set the current vector explicitly; an absent channel
		// leaves that state untouched, matching the source update cadence.
	}
	return true;
}

export bool Advance_Sphere_Animation(const Assets::SphereAssetDesc &asset,
	float logic_frame_seconds, SphereRuntimeState &state) noexcept
{
	if (!std::isfinite(logic_frame_seconds) || !state.animating)
		return false;
	const bool has_channels = !asset.color_track.keys.empty()
		|| !asset.alpha_track.keys.empty() || !asset.scale_track.keys.empty()
		|| !asset.vector_track.keys.empty();
	if (!has_channels)
		return true;
	if (asset.animation_duration > 0.0f)
		state.animation_time += logic_frame_seconds / asset.animation_duration;
	else
		state.animation_time = 1.0f;
	if ((asset.attributes & Assets::SphereAttributeAnimationLoop) != 0
		&& state.animation_time > 1.0f)
		state.animation_time -= 1.0f;
	return Evaluate_Sphere_Animation(asset, state.animation_time, state);
}

export void Update_Sphere_Visibility(bool not_hidden, SphereRuntimeState &state) noexcept
{
	if (not_hidden && !state.animating) {
		state.animating = true;
		state.animation_time = 0.0f;
	} else if (!not_hidden && state.animating) {
		state.animating = false;
		state.animation_time = 0.0f;
	}
	state.visible = not_hidden;
}

export float Sphere_LOD_Cost(std::uint32_t lod) noexcept
{
	if (lod == 0)
		return SphereNullLODCost;
	if (lod > SphereLODCount)
		return 0.0f;
	const float size = static_cast<float>(SphereLowestLOD + lod - 1);
	return size * size * 2.0f;
}

export bool Calculate_Sphere_LOD_Values(float screen_area, float lod_bias,
	std::span<float> values, std::span<float> costs) noexcept
{
	if (!std::isfinite(screen_area) || !std::isfinite(lod_bias) || lod_bias < 0.0f
		|| values.size() < SphereLODValueCount || costs.size() < SphereLODCount + 1)
		return false;
	// Match W3DRenderObject's LOD sentinels: minimum LOD is a large positive
	// value and the post-maximum sentinel is -1.
	values[0] = (std::numeric_limits<float>::max)();
	costs[0] = SphereNullLODCost;
	for (std::uint32_t lod = 1; lod <= SphereLODCount; ++lod) {
		const float cost = Sphere_LOD_Cost(lod);
		costs[lod] = cost;
		const float benefit = 1.0f - 0.5f / (cost * cost);
		values[lod] = benefit * screen_area * lod_bias / cost;
	}
	values[SphereLODCount + 1] = -1.0f;
	return true;
}

export bool Build_Sphere_Geometry(const Assets::SphereAssetDesc &asset,
	const SphereRuntimeState &state, std::vector<PropVertex> &vertices,
	std::vector<std::uint32_t> &indices) noexcept
{
	if (!Assets::Is_Valid_Sphere_Asset(asset) || !SphereDetail::Valid_Runtime_State(state)) {
		vertices.clear();
		indices.clear();
		return false;
	}

	const std::uint32_t size = SphereLowestLOD + state.lod - 1;
	SphereGeometry geometry;
	if (!geometry.Generate(1.0f, size, size))
		return false;
	return SphereDetail::Build_Sphere_Geometry_From(asset, state, geometry, vertices, indices);
}

bool Build_Sphere_Draw_State_Unchecked(const Assets::SphereAssetDesc &asset,
	const SphereRuntimeState &state, const SphereDrawInput &input,
	SphereDrawState &output) noexcept
{
	if (!SphereDetail::Valid_Runtime_State(state) || !std::isfinite(input.screen_area))
		return false;
	output = {};
	const auto &source = asset.material;
	output.additive = source.destination_blend == Assets::SphereBlendFactor::One;
	PropStyle authored_style;
	authored_style.depth_write = source.depth_write;
	authored_style.depth_test = true;
	authored_style.source_blend = SphereDetail::Map_Blend(source.source_blend);
	authored_style.destination_blend = SphereDetail::Map_Blend(source.destination_blend);
	authored_style.depth_comparison = SphereDetail::Map_Depth_Compare(source.depth_compare);
	authored_style.cull = source.cull_enabled ? RHICullMode::Back : RHICullMode::None;
	authored_style.front_counter_clockwise = input.front_counter_clockwise;
	authored_style.color_write_mask = source.color_write ? 15 : 0;
	if (authored_style.source_blend == RHIBlendFactor::One
		&& authored_style.destination_blend == RHIBlendFactor::Zero)
		authored_style.blend = RHIBlendMode::Disabled;
	else if (output.additive)
		authored_style.blend = RHIBlendMode::Additive;
	else
		authored_style.blend = RHIBlendMode::Alpha;
	authored_style.samplers[0] = Resolve_Texture_Sampling(
		input.texture_sampling, Get_Texture_Sampling_Settings());
	output.style = Resolve_Prop_Style(authored_style, input.scene);

	const Assets::Vector3f real_scale = SphereDetail::Multiply(asset.extent, state.scale);
	if ((asset.attributes & Assets::SphereAttributeCameraAligned) != 0) {
		const Assets::Vector3f camera_position = SphereDetail::Transform_Point(input.view,
			{input.world[3], input.world[7], input.world[11]});
		// The source billboard is expressed directly in camera space. Using the
		// supplied projection and an identity view keeps the generic prop shader
		// on the same camera-space path without an inverse-matrix dependency.
		output.parameters.view_projection = input.projection;
		output.parameters.view = SphereDetail::Identity();
		output.parameters.world = {
			0.0f, real_scale.y, 0.0f, camera_position.x,
			0.0f, 0.0f, real_scale.z, camera_position.y,
			real_scale.x, 0.0f, 0.0f, camera_position.z,
			0.0f, 0.0f, 0.0f, 1.0f};
		output.parameters.camera_position = {camera_position.x, camera_position.y, camera_position.z, 1.0f};
	} else {
		output.parameters.view_projection = input.view_projection;
		output.parameters.view = input.view;
		output.parameters.world = SphereDetail::Scale_World(input.world, real_scale);
		output.parameters.camera_position = input.camera_position;
	}
	output.parameters.textured = !asset.texture_name.empty() ? 1.0f : 0.0f;
	output.parameters.primary_gradient = static_cast<float>(source.primary_gradient);
	output.parameters.secondary_gradient = static_cast<float>(source.secondary_gradient);
	output.parameters.detail_color = static_cast<float>(source.post_detail_color_function);
	output.parameters.detail_alpha = static_cast<float>(source.post_detail_alpha_function);
	output.parameters.alpha_cutoff = source.alpha_test ? 96.0f / 255.0f : 0.0f;
	output.parameters.opacity = 1.0f;
	output.parameters.normal_in_world_space = 1.0f;
	const MaterialFog fog = Resolve_Material_Fog(input.scene.fog,
		SphereDetail::Map_Fog(source.fog));
	output.parameters.fog_state = fog.state;
	output.parameters.fog_color = fog.color;
	output.material.ambient = {0.0f, 0.0f, 0.0f};
	output.material.diffuse = {0.0f, 0.0f, 0.0f};
	output.material.specular = {0.0f, 0.0f, 0.0f};
	output.material.emissive = output.additive
		? std::array<float, 3>{state.color.x * state.alpha,
			state.color.y * state.alpha, state.color.z * state.alpha}
		: std::array<float, 3>{state.color.x, state.color.y, state.color.z};
	output.material.opacity = output.additive ? 0.25f : state.alpha;
	output.material.shininess = 0.0f;
	output.material.lighting = true;
	output.sort_required = input.sorting_enabled
		&& source.destination_blend != Assets::SphereBlendFactor::Zero
		&& !source.alpha_test;
	output.sort_depth = output.parameters.view[8] * output.parameters.world[3]
		+ output.parameters.view[9] * output.parameters.world[7]
		+ output.parameters.view[10] * output.parameters.world[11]
		+ output.parameters.view[11];
	return true;
}

export bool Build_Sphere_Draw_State(const Assets::SphereAssetDesc &asset,
	const SphereRuntimeState &state, const SphereDrawInput &input,
	SphereDrawState &output) noexcept
{
	if (!Assets::Is_Valid_Sphere_Asset(asset))
		return false;
	return Build_Sphere_Draw_State_Unchecked(asset, state, input, output);
}

// Static scene ordering is used while the legacy renderer has sorting
// disabled. Keep the authored categories at the graphics boundary so callers
// do not need to reconstruct material state just to enqueue a sphere.
export std::uint32_t Sphere_Ordered_Layer(
	const Assets::SphereMaterialDesc &material) noexcept
{
	const bool alpha_test_category = material.alpha_test &&
		(material.destination_blend == Assets::SphereBlendFactor::Zero ||
			(material.source_blend == Assets::SphereBlendFactor::SourceAlpha &&
				material.destination_blend == Assets::SphereBlendFactor::InverseSourceAlpha));
	if ((!material.alpha_test && material.destination_blend == Assets::SphereBlendFactor::Zero) ||
		alpha_test_category)
		return 0;
	if (material.source_blend == Assets::SphereBlendFactor::One &&
		material.destination_blend == Assets::SphereBlendFactor::One)
		return 10;
	if (material.source_blend == Assets::SphereBlendFactor::One &&
		material.destination_blend == Assets::SphereBlendFactor::InverseSourceColor)
		return 15;
	return 20;
}

// Thin owner used by game scene adapters. It owns source data and mutable
// animation/visibility state while all geometry and submission stay in this
// focused sphere component.
export class SphereSceneObject final
{
public:
	explicit SphereSceneObject(Assets::SphereAssetDesc asset)
		: m_asset(std::move(asset)), m_state(Make_Sphere_Runtime_State(m_asset))
	{
		Calculate_Sphere_LOD_Values(1.0f, m_state.lod_bias, m_values, m_costs);
	}

	const Assets::SphereAssetDesc &Asset() const noexcept { return m_asset; }
	const SphereRuntimeState &State() const noexcept { return m_state; }
	SphereRuntimeState &State() noexcept { return m_state; }

	std::uint32_t Flags() const noexcept { return m_asset.attributes; }
	void Set_Flags(std::uint32_t flags) noexcept { m_asset.attributes = flags; }
	void Set_Flag(std::uint32_t flag, bool enabled) noexcept
	{
		m_asset.attributes = enabled ? m_asset.attributes | flag : m_asset.attributes & ~flag;
	}
	bool Has_Flag(std::uint32_t flag) const noexcept { return (m_asset.attributes & flag) != 0; }
	void Set_Camera_Aligned(bool aligned) noexcept { Set_Flag(Assets::SphereAttributeCameraAligned, aligned); }
	bool Camera_Aligned() const noexcept { return Has_Flag(Assets::SphereAttributeCameraAligned); }
	void Set_Animation_Loop(bool loop) noexcept { Set_Flag(Assets::SphereAttributeAnimationLoop, loop); }
	bool Animation_Loop() const noexcept { return Has_Flag(Assets::SphereAttributeAnimationLoop); }

	void Set_Hidden(bool hidden) noexcept
	{
		m_hidden = hidden;
		m_state.animation_hidden = m_animation_hidden;
		Update_Sphere_Visibility(!m_hidden && !m_animation_hidden, m_state);
	}
	void Set_Animation_Hidden(bool hidden) noexcept
	{
		m_animation_hidden = hidden;
		m_state.animation_hidden = hidden;
		Update_Sphere_Visibility(!m_hidden && !m_animation_hidden, m_state);
	}
	// IS_VISIBLE is camera-owned in the source object. It is intentionally
	// separate from Is_Not_Hidden_At_All(), which only combines authored hidden
	// and animation-hidden bits and is what SphereW3DRenderObject::Render tests.
	void Set_Visible(bool visible) noexcept { m_visible_requested = visible; }
	bool Is_Visible() const noexcept { return m_visible_requested; }
	bool Is_Not_Hidden_At_All() const noexcept { return m_state.visible; }
	bool Is_Hidden() const noexcept { return m_hidden; }
	bool Is_Animation_Hidden() const noexcept { return m_animation_hidden; }
	void Set_Force_Visible(bool force_visible) noexcept
	{
		m_force_visible = force_visible;
		m_state.force_visible = force_visible;
	}
	bool Is_Force_Visible() const noexcept { return m_force_visible; }
	bool Is_Drawable() const noexcept { return m_state.visible; }

	bool Is_Animating() const noexcept { return m_state.animating; }
	bool Update(float logic_frame_seconds) noexcept
	{
		return Advance_Sphere_Animation(m_asset, logic_frame_seconds, m_state);
	}

	void Scale(float scale) noexcept { Scale(scale, scale, scale); }
	void Scale(float scale_x, float scale_y, float scale_z) noexcept
	{
		if (!std::isfinite(scale_x) || !std::isfinite(scale_y) || !std::isfinite(scale_z))
			return;
		m_state.scale.x *= scale_x;
		m_state.scale.y *= scale_y;
		m_state.scale.z *= scale_z;
		for (auto &key : m_asset.scale_track.keys) {
			key.value.x *= scale_x;
			key.value.y *= scale_y;
			key.value.z *= scale_z;
		}
	}

	void Prepare_LOD(float screen_area) noexcept
	{
		if (std::isfinite(screen_area))
			Calculate_Sphere_LOD_Values(screen_area, m_state.lod_bias, m_values, m_costs);
	}
	void Increment_LOD() noexcept
	{
		if (m_state.lod < SphereLODCount)
			++m_state.lod;
	}
	void Decrement_LOD() noexcept
	{
		if (m_state.lod > 0)
			--m_state.lod;
	}
	void Set_LOD_Level(std::uint32_t lod) noexcept { m_state.lod = std::min(lod, SphereLODCount); }
	std::uint32_t LOD_Level() const noexcept { return m_state.lod; }
	std::uint32_t LOD_Count() const noexcept { return SphereLODCount + 1; }
	std::uint32_t Num_Polys() const noexcept { return static_cast<std::uint32_t>(Sphere_LOD_Cost(m_state.lod)); }
	float Cost() const noexcept { return Sphere_LOD_Cost(m_state.lod); }
	float Value() const noexcept { return m_values[m_state.lod]; }
	float Post_Increment_Value() const noexcept { return m_values[m_state.lod + 1]; }
	void Set_LOD_Bias(float bias) noexcept
	{
		if (std::isfinite(bias))
			m_state.lod_bias = std::max(bias, 0.0f);
	}
	float LOD_Bias() const noexcept { return m_state.lod_bias; }
	bool Calculate_Cost_Value_Arrays(float screen_area, std::span<float> values,
		std::span<float> costs) const noexcept
	{
		if (!Calculate_Sphere_LOD_Values(screen_area, m_state.lod_bias, values, costs))
			return false;
		for (std::uint32_t lod = 0; lod <= SphereLODCount; ++lod)
			costs[lod] = Sphere_LOD_Cost(lod);
		return true;
	}
	SphereBounds Bounds() const noexcept
	{
		const Assets::Vector3f extent = SphereDetail::Multiply(m_asset.extent, m_state.scale);
		return {m_asset.center, extent,
			std::sqrt(extent.x * extent.x + extent.y * extent.y + extent.z * extent.z)};
	}

private:
	Assets::SphereAssetDesc m_asset;
	SphereRuntimeState m_state;
	std::array<float, SphereLODValueCount> m_values{};
	std::array<float, SphereLODCount + 1> m_costs{};
	bool m_hidden = false;
	bool m_animation_hidden = false;
	bool m_force_visible = false;
	bool m_visible_requested = true;
};

// Owns the mutable prop mesh for one sphere instance. CPU LOD geometry and
// versioned GPU meshes live here so a deferred transparent draw can retain an
// immutable mesh while the next animation frame publishes a replacement.
export class SphereRenderer final
{
public:
	SphereRenderer() = default;
	SphereRenderer(const SphereRenderer &) = delete;
	SphereRenderer &operator=(const SphereRenderer &) = delete;

	~SphereRenderer()
	{
		if (m_owner != nullptr && m_mesh.Is_Valid())
			m_owner->Destroy_Mesh(m_mesh);
	}

	bool Submit(PropRenderer &renderer, PropSubmission &submission,
		const SphereSceneObject &sphere, const SphereDrawInput &input,
		std::span<const RHITextureHandle> textures) noexcept
	{
		if (textures.size() > 1 || !sphere.Is_Drawable() ||
			sphere.State().lod == 0 || !Assets::Is_Valid_Sphere_Asset(sphere.Asset()))
			return false;
		SphereDrawState draw_state;
		if (!Build_Sphere_Draw_State_Unchecked(sphere.Asset(), sphere.State(), input, draw_state))
			return false;
		// SphereW3DRenderObject selects texturing from its live texture pointer;
		// Set_Texture can attach one even when the serialized asset name is empty.
		// The supplied handle is the authoritative presence check at submission.
		draw_state.parameters.textured =
			!textures.empty() && textures.front().Is_Valid() ? 1.0f : 0.0f;

		std::vector<PropVertex> vertices;
		std::vector<std::uint32_t> indices;
		if (!Ensure_Mesh(renderer, sphere.Asset(), sphere.State(), vertices, indices))
			return false;
		const PropDrawPhase phase = draw_state.sort_required
			? PropDrawPhase::Transparent : PropDrawPhase::Immediate;
		const std::array<float, 4> camera_depth{
			draw_state.parameters.view[8], draw_state.parameters.view[9],
			draw_state.parameters.view[10], draw_state.parameters.view[11]};
		return submission.Submit(m_mesh, draw_state.style, draw_state.parameters,
			textures, phase, camera_depth);
	}

	void Release(PropRenderer &renderer) noexcept
	{
		if (m_mesh.Is_Valid()) {
			if (m_owner != nullptr)
				m_owner->Destroy_Mesh(m_mesh);
			else
				renderer.Destroy_Mesh(m_mesh);
		}
		m_mesh = {};
		m_owner = nullptr;
	}

	PropMeshHandle Mesh() const noexcept { return m_mesh; }

private:
	bool Ensure_Mesh(PropRenderer &renderer, const Assets::SphereAssetDesc &asset,
		const SphereRuntimeState &state, std::vector<PropVertex> &vertices,
		std::vector<std::uint32_t> &indices)
	{
		if (m_owner != &renderer && m_mesh.Is_Valid()) {
			if (m_owner != nullptr)
				m_owner->Destroy_Mesh(m_mesh);
			m_mesh = {};
		}
		m_owner = &renderer;
		if (!m_geometry_cache.Build(asset, state, vertices, indices))
			return false;
		if (m_mesh.Is_Valid()) {
			const PropGeometry *geometry = renderer.Mesh_Geometry(m_mesh);
			if (geometry != nullptr && geometry->Matches(vertices, indices))
				return true;
			if (renderer.Update_Mesh(m_mesh, vertices, indices))
				return true;
			// A deferred draw may retain this version. Publish a replacement so
			// its geometry remains immutable until the queue consumes it.
			const PropMeshHandle replacement = renderer.Create_Mesh(vertices, indices);
			if (!replacement.Is_Valid())
				return false;
			const PropMeshHandle previous = m_mesh;
			m_mesh = replacement;
			renderer.Destroy_Mesh(previous);
			return true;
		}
		m_mesh = renderer.Create_Mesh(vertices, indices);
		return m_mesh.Is_Valid();
	}

	PropRenderer *m_owner = nullptr;
	PropMeshHandle m_mesh{};
	SphereDetail::SphereGeometryCache m_geometry_cache;
};

}
