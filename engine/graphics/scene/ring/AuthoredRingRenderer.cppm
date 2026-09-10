module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.Scene.Ring.Renderer;

import Assets.Rings;
import Graphics.RHI;
import Graphics.Resources.Textures.Sampling;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Ring.Runtime;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;

namespace Graphics
{

export struct AuthoredRingDrawInput final
{
	std::array<float, 16> view_projection{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f};
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
	// Camera world-space +Z, supplied by the camera adapter for authored
	// camera-aligned rings.  The ring component owns the source Look_At policy.
	std::array<float, 3> camera_z{0.0f, 0.0f, 1.0f};
	TextureSampling texture_sampling{};
	SceneDrawParameters scene{};
	bool sorting_enabled = false;
	bool front_counter_clockwise = false;
};

namespace AuthoredRingRendererDetail
{

RHIBlendFactor Map_Blend(Assets::RingBlendFactor factor) noexcept
{
	switch (factor) {
	case Assets::RingBlendFactor::Zero: return RHIBlendFactor::Zero;
	case Assets::RingBlendFactor::One: return RHIBlendFactor::One;
	case Assets::RingBlendFactor::SourceColor: return RHIBlendFactor::SourceColor;
	case Assets::RingBlendFactor::InverseSourceColor: return RHIBlendFactor::InverseSourceColor;
	case Assets::RingBlendFactor::SourceAlpha: return RHIBlendFactor::SourceAlpha;
	case Assets::RingBlendFactor::InverseSourceAlpha: return RHIBlendFactor::InverseSourceAlpha;
	case Assets::RingBlendFactor::SourceColorPreFog: return RHIBlendFactor::SourceColor;
	}
	return RHIBlendFactor::One;
}

MaterialFogMode Map_Fog(Assets::RingFogMode mode) noexcept
{
	switch (mode) {
	case Assets::RingFogMode::Enabled: return MaterialFogMode::Scene;
	case Assets::RingFogMode::ScaleFragment: return MaterialFogMode::Black;
	case Assets::RingFogMode::White: return MaterialFogMode::White;
	case Assets::RingFogMode::Disabled: break;
	}
	return MaterialFogMode::Disabled;
}

RHIComparison Map_Depth_Compare(Assets::RingDepthCompare compare) noexcept
{
	return static_cast<RHIComparison>(compare);
}

PropStyle Make_Style(const Assets::RingMaterialDesc &material,
	bool front_counter_clockwise, const TextureSampling &texture_sampling) noexcept
{
	PropStyle style;
	style.depth_write = material.depth_write;
	style.depth_test = true;
	style.source_blend = Map_Blend(material.source_blend);
	style.destination_blend = Map_Blend(material.destination_blend);
	style.depth_comparison = Map_Depth_Compare(material.depth_compare);
	style.cull = material.cull_enabled ? RHICullMode::Back : RHICullMode::None;
	style.front_counter_clockwise = front_counter_clockwise;
	style.color_write_mask = material.color_write ? 0x0f : 0;
	style.samplers[0] = Resolve_Texture_Sampling(
		texture_sampling, Get_Texture_Sampling_Settings());
	if (style.source_blend == RHIBlendFactor::One &&
		style.destination_blend == RHIBlendFactor::Zero)
		style.blend = RHIBlendMode::Disabled;
	else if (style.source_blend == RHIBlendFactor::One &&
		style.destination_blend == RHIBlendFactor::One)
		style.blend = RHIBlendMode::Additive;
	else
		style.blend = RHIBlendMode::Alpha;
	return style;
}

bool Finite(const std::array<float, 16> &values) noexcept
{
	for (const float value : values)
		if (!std::isfinite(value))
			return false;
	return true;
}

bool Finite(const std::array<float, 3> &values) noexcept
{
	for (const float value : values)
		if (!std::isfinite(value))
			return false;
	return true;
}

// Matrix3D::Look_At(p, p + direction, 0) points local -Z along direction.
// Keep its XY-pole fallback and axis convention here so camera-aligned rings
// retain the authored billboard orientation without importing game math.
bool Make_Camera_Aligned_World(const std::array<float, 3> &position,
	const std::array<float, 3> &direction, std::array<float, 16> &result) noexcept
{
	if (!Finite(position) || !Finite(direction))
		return false;

	float dx = direction[0];
	float dy = direction[1];
	float dz = direction[2];
	const float length_squared = dx * dx + dy * dy + dz * dz;
	if (length_squared != 0.0f) {
		const float inverse_length = 1.0f / std::sqrt(length_squared);
		dx *= inverse_length;
		dy *= inverse_length;
		dz *= inverse_length;
	}

	const float xy_length = std::sqrt(dx * dx + dy * dy);
	const float sin_pitch = dz;
	const float cos_pitch = xy_length;
	float sin_yaw = 0.0f;
	float cos_yaw = 1.0f;
	if (xy_length != 0.0f) {
		sin_yaw = dy / xy_length;
		cos_yaw = dx / xy_length;
	}

	result = {
		sin_yaw, -cos_yaw * sin_pitch, -cos_yaw * cos_pitch, position[0],
		-cos_yaw, -sin_yaw * sin_pitch, -sin_yaw * cos_pitch, position[1],
		0.0f, cos_pitch, -sin_pitch, position[2],
		0.0f, 0.0f, 0.0f, 1.0f};
	return Finite(result);
}

}

// Exposed for camera-adapter tests and callers that need the same authored
// orientation before assembling a draw input.
export bool Build_Authored_Ring_Camera_Aligned_World(
	const std::array<float, 3> &position, const std::array<float, 3> &direction,
	std::array<float, 16> &result) noexcept
{
	return AuthoredRingRendererDetail::Make_Camera_Aligned_World(
		position, direction, result);
}

// Owns one versioned prop mesh for a ring instance. Authored animation and
// bounds remain in AuthoredRingRuntime; this class only converts its draw data
// to a generic prop submission and keeps geometry alive between draws.
export class AuthoredRingRenderer final
{
public:
	AuthoredRingRenderer() = default;
	AuthoredRingRenderer(const AuthoredRingRenderer &) = delete;
	AuthoredRingRenderer &operator=(const AuthoredRingRenderer &) = delete;

	~AuthoredRingRenderer()
	{
		if (m_owner != nullptr && m_mesh.Is_Valid())
			m_owner->Destroy_Mesh(m_mesh);
	}

	bool Submit(PropRenderer &renderer, PropSubmission &submission,
		AuthoredRingRuntime &runtime, const AuthoredRingDrawInput &input,
		std::span<const RHITextureHandle> textures)
	{
		if (textures.size() > 1)
			return false;
		if (!AuthoredRingRendererDetail::Finite(input.view_projection) ||
			!AuthoredRingRendererDetail::Finite(input.view) ||
			!AuthoredRingRendererDetail::Finite(input.world) ||
			(runtime.Description().camera_aligned &&
				!AuthoredRingRendererDetail::Finite(input.camera_z)))
			return false;

		AuthoredRingDrawData data;
		if (!runtime.Build_Draw_Data(data) || data.geometry.vertices.empty() ||
			data.geometry.indices.empty())
			return false;
		if (!Ensure_Mesh(renderer, data.geometry.vertices, data.geometry.indices))
			return false;

		std::array<float, 16> world = input.world;
		if (data.camera_aligned) {
			const std::array<float, 3> position{
				input.world[3], input.world[7], input.world[11]};
			if (!Build_Authored_Ring_Camera_Aligned_World(
				position, input.camera_z, world))
				return false;
		}

		PropParameters parameters;
		parameters.view_projection = input.view_projection;
		parameters.view = input.view;
		parameters.world = world;
		parameters.textured = !textures.empty() && textures.front().Is_Valid() ? 1.0f : 0.0f;
		parameters.primary_gradient = static_cast<float>(data.material.primary_gradient);
		parameters.secondary_gradient = static_cast<float>(data.material.secondary_gradient);
		parameters.detail_color = static_cast<float>(data.material.post_detail_color_function);
		parameters.detail_alpha = static_cast<float>(data.material.post_detail_alpha_function);
		parameters.alpha_cutoff = data.material.alpha_test ? 96.0f / 255.0f : 0.0f;
		parameters.opacity = 1.0f;
		parameters.normal_in_world_space = 1.0f;

		const PropStyle authored_style = AuthoredRingRendererDetail::Make_Style(
			data.material, input.front_counter_clockwise, input.texture_sampling);
		const PropStyle style = Resolve_Prop_Style(authored_style, input.scene);
		const MaterialFog fog = Resolve_Material_Fog(
			input.scene.fog, AuthoredRingRendererDetail::Map_Fog(data.material.fog));
		parameters.fog_state = fog.state;
		parameters.fog_color = fog.color;
		const PropDrawPhase phase = input.sorting_enabled && data.sort_required
			? PropDrawPhase::Transparent : PropDrawPhase::Immediate;
		const std::array<float, 4> camera_depth{
			input.view[8], input.view[9], input.view[10], input.view[11]};
		return submission.Submit(m_mesh, style, parameters, textures,
			phase, camera_depth);
	}

	void Release(PropRenderer &renderer) noexcept
	{
		if (m_mesh.Is_Valid())
			renderer.Destroy_Mesh(m_mesh);
		m_mesh = {};
		m_owner = nullptr;
	}

	PropMeshHandle Mesh() const noexcept { return m_mesh; }

private:
	bool Ensure_Mesh(PropRenderer &renderer,
		std::span<const PropVertex> vertices,
		std::span<const std::uint32_t> indices)
	{
		if (m_owner != &renderer && m_mesh.Is_Valid()) {
			if (m_owner != nullptr)
				m_owner->Destroy_Mesh(m_mesh);
			m_mesh = {};
		}
		m_owner = &renderer;

		if (m_mesh.Is_Valid()) {
			const PropGeometry *geometry = renderer.Mesh_Geometry(m_mesh);
			if (geometry != nullptr && geometry->Matches(vertices, indices))
				return true;
			if (renderer.Update_Mesh(m_mesh, vertices, indices))
				return true;
			// A deferred transparent submission may retain the old mesh. Publish
			// a replacement version so its geometry remains immutable until flush.
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
};

}
