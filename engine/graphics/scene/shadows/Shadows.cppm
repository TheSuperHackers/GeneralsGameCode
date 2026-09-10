module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

export module Graphics.Scene.Shadows;

export import Graphics.RHI;
export import Graphics.RenderGraph;
export import Graphics.Scene.Lighting;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export inline constexpr std::size_t Max_Shadow_Cascades = 4;

export struct ShadowSettings final
{
	std::uint32_t cascade_count = 4;
	float near_clip = 1.0f;
	float far_clip = 1000.0f;
	float split_lambda = 0.5f;
	float depth_padding = 50.0f;
	std::uint32_t map_size = 1024;
};

export struct ShadowView final
{
	Matrix4x4 view_matrix{};
	Matrix4x4 projection_matrix{};
	Matrix4x4 view_projection{};
	Viewport viewport{};
	float split_near = 0.0f;
	float split_far = 0.0f;
};

export struct alignas(16) GPUShadowData final
{
	std::array<Matrix4x4, Max_Shadow_Cascades> view_projections{};
	std::array<float, Max_Shadow_Cascades> split_distances{};
	std::array<std::uint32_t, Max_Shadow_Cascades> shadow_map_indices{};
	std::uint32_t cascade_count = 0;
	std::uint32_t light_index = Invalid_Shadow_Data_Index;
	std::uint32_t reserved0 = 0;
	std::uint32_t reserved1 = 0;
};

static_assert(sizeof(GPUShadowData) == 304);

export class ShadowCascades final
{
public:
	LightHandle light{};
	std::uint32_t count = 0;
	std::array<ShadowView, Max_Shadow_Cascades> views{};
	std::array<ResourceIndex, Max_Shadow_Cascades> shadow_map_indices{};

	void Clear() noexcept
	{
		light = {};
		count = 0;
		views = {};
		shadow_map_indices = {};
	}

	bool Set_Shadow_Map_Index(std::uint32_t cascade_index, ResourceIndex index) noexcept
	{
		if (cascade_index >= count)
			return false;

		shadow_map_indices[cascade_index] = index;
		return true;
	}

	GPUShadowData Pack_GPU_Data(std::uint32_t gpu_light_index) const noexcept
	{
		GPUShadowData data;
		data.shadow_map_indices.fill(Invalid_Shadow_Data_Index);
		data.cascade_count = count;
		data.light_index = gpu_light_index;
		for (std::uint32_t cascade_index = 0; cascade_index < count && cascade_index < Max_Shadow_Cascades; ++cascade_index) {
			data.view_projections[cascade_index] = views[cascade_index].view_projection;
			data.split_distances[cascade_index] = views[cascade_index].split_far;
			if (shadow_map_indices[cascade_index].Is_Valid())
				data.shadow_map_indices[cascade_index] = shadow_map_indices[cascade_index].Get_Index();
		}
		return data;
	}
};

export bool Calculate_Cascade_Splits(float near_clip, float far_clip, float split_lambda, std::span<float> splits) noexcept;

export bool Build_Shadow_Cascades(
	const View &view,
	LightHandle light_handle,
	const RenderLight &light,
	const ShadowSettings &settings,
	ShadowCascades &cascades) noexcept;

export class ShadowMapResources final
{
public:
	ShadowMapResources() = default;
	ShadowMapResources(const ShadowMapResources &) = delete;
	ShadowMapResources &operator=(const ShadowMapResources &) = delete;

	bool Initialize(Device &device, RenderGraph &graph, std::uint32_t cascade_count, std::uint32_t map_size);

	void Shutdown(Device &device) noexcept;

	bool Is_Valid() const noexcept
	{
		return m_count != 0;
	}

	std::uint32_t Count() const noexcept
	{
		return m_count;
	}

	std::uint32_t Map_Size() const noexcept
	{
		return m_map_size;
	}

	RHITextureHandle Texture(std::uint32_t cascade_index) const noexcept
	{
		return cascade_index < m_count ? m_textures[cascade_index] : RHITextureHandle{};
	}

	GraphResourceHandle Target(std::uint32_t cascade_index) const noexcept
	{
		return cascade_index < m_count ? m_targets[cascade_index] : GraphResourceHandle{};
	}

private:
	std::array<RHITextureHandle, Max_Shadow_Cascades> m_textures{};
	std::array<GraphResourceHandle, Max_Shadow_Cascades> m_targets{};
	std::uint32_t m_count = 0;
	std::uint32_t m_map_size = 0;
};

namespace
{
constexpr float Shadow_Epsilon = 0.0001f;

float Dot(Vector3 left, Vector3 right) noexcept
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

Vector3 Subtract(Vector3 left, Vector3 right) noexcept
{
	return {left.x - right.x, left.y - right.y, left.z - right.z};
}

Vector3 Add(Vector3 left, Vector3 right) noexcept
{
	return {left.x + right.x, left.y + right.y, left.z + right.z};
}

Vector3 Multiply(Vector3 value, float scalar) noexcept
{
	return {value.x * scalar, value.y * scalar, value.z * scalar};
}

Vector3 Cross(Vector3 left, Vector3 right) noexcept
{
	return {
		left.y * right.z - left.z * right.y,
		left.z * right.x - left.x * right.z,
		left.x * right.y - left.y * right.x
	};
}

bool Normalize(Vector3 value, Vector3 &normalized) noexcept
{
	const float length_squared = Dot(value, value);
	if (!std::isfinite(length_squared) || length_squared <= Shadow_Epsilon * Shadow_Epsilon)
		return false;

	const float inverse_length = 1.0f / std::sqrt(length_squared);
	normalized = Multiply(value, inverse_length);
	return true;
}

Matrix4x4 Multiply_Matrices(const Matrix4x4 &left, const Matrix4x4 &right) noexcept
{
	Matrix4x4 result{};
	for (std::size_t row = 0; row < 4; ++row) {
		for (std::size_t column = 0; column < 4; ++column) {
			for (std::size_t element = 0; element < 4; ++element)
				result.values[row * 4 + column] += left.values[row * 4 + element] * right.values[element * 4 + column];
		}
	}
	return result;
}

bool Invert(const Matrix4x4 &input, Matrix4x4 &output) noexcept
{
	float augmented[4][8]{};
	for (std::size_t row = 0; row < 4; ++row) {
		for (std::size_t column = 0; column < 4; ++column)
			augmented[row][column] = input.values[row * 4 + column];
		augmented[row][row + 4] = 1.0f;
	}

	for (std::size_t column = 0; column < 4; ++column) {
		std::size_t pivot_row = column;
		float pivot_size = std::abs(augmented[pivot_row][column]);
		for (std::size_t row = column + 1; row < 4; ++row) {
			const float candidate_size = std::abs(augmented[row][column]);
			if (candidate_size > pivot_size) {
				pivot_row = row;
				pivot_size = candidate_size;
			}
		}

		if (!std::isfinite(pivot_size) || pivot_size <= Shadow_Epsilon)
			return false;

		if (pivot_row != column) {
			for (std::size_t value = 0; value < 8; ++value) {
				const float temporary = augmented[column][value];
				augmented[column][value] = augmented[pivot_row][value];
				augmented[pivot_row][value] = temporary;
			}
		}

		const float inverse_pivot = 1.0f / augmented[column][column];
		for (std::size_t value = 0; value < 8; ++value)
			augmented[column][value] *= inverse_pivot;

		for (std::size_t row = 0; row < 4; ++row) {
			if (row == column)
				continue;

			const float factor = augmented[row][column];
			for (std::size_t value = 0; value < 8; ++value)
				augmented[row][value] -= factor * augmented[column][value];
		}
	}

	for (std::size_t row = 0; row < 4; ++row) {
		for (std::size_t column = 0; column < 4; ++column)
			output.values[row * 4 + column] = augmented[row][column + 4];
	}
	return true;
}

struct Vector4 final
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;
};

Vector4 Transform(const Matrix4x4 &matrix, Vector4 value) noexcept
{
	return {
		matrix.values[0] * value.x + matrix.values[1] * value.y + matrix.values[2] * value.z + matrix.values[3] * value.w,
		matrix.values[4] * value.x + matrix.values[5] * value.y + matrix.values[6] * value.z + matrix.values[7] * value.w,
		matrix.values[8] * value.x + matrix.values[9] * value.y + matrix.values[10] * value.z + matrix.values[11] * value.w,
		matrix.values[12] * value.x + matrix.values[13] * value.y + matrix.values[14] * value.z + matrix.values[15] * value.w
	};
}

bool Unproject(const Matrix4x4 &inverse_view_projection, float x, float y, float z, Vector3 &world_position) noexcept
{
	const Vector4 transformed = Transform(inverse_view_projection, {x, y, z, 1.0f});
	if (!std::isfinite(transformed.w) || std::abs(transformed.w) <= Shadow_Epsilon)
		return false;

	const float inverse_w = 1.0f / transformed.w;
	world_position = {transformed.x * inverse_w, transformed.y * inverse_w, transformed.z * inverse_w};
	return std::isfinite(world_position.x) && std::isfinite(world_position.y) && std::isfinite(world_position.z);
}

bool Projection_Depth(const Matrix4x4 &projection, float view_z, float &ndc_z) noexcept
{
	const float clip_z = projection.values[10] * view_z + projection.values[11];
	const float clip_w = projection.values[14] * view_z + projection.values[15];
	if (!std::isfinite(clip_w) || std::abs(clip_w) <= Shadow_Epsilon)
		return false;

	ndc_z = clip_z / clip_w;
	return std::isfinite(ndc_z);
}

Matrix4x4 Make_Light_View(Vector3 right, Vector3 up, Vector3 forward, Vector3 origin) noexcept
{
	Matrix4x4 result{};
	result.values[0] = right.x;
	result.values[1] = right.y;
	result.values[2] = right.z;
	result.values[3] = -Dot(right, origin);
	result.values[4] = up.x;
	result.values[5] = up.y;
	result.values[6] = up.z;
	result.values[7] = -Dot(up, origin);
	result.values[8] = -forward.x;
	result.values[9] = -forward.y;
	result.values[10] = -forward.z;
	result.values[11] = Dot(forward, origin);
	result.values[15] = 1.0f;
	return result;
}

Matrix4x4 Make_Orthographic(float left, float right, float bottom, float top, float near_clip, float far_clip) noexcept
{
	Matrix4x4 result = Matrix4x4::Identity();
	result.values[0] = 2.0f / (right - left);
	result.values[3] = -(right + left) / (right - left);
	result.values[5] = 2.0f / (top - bottom);
	result.values[7] = -(top + bottom) / (top - bottom);
	// Graphics depth textures use the [0,1] clip-depth interval.
	result.values[10] = -1.0f / (far_clip - near_clip);
	result.values[11] = -near_clip / (far_clip - near_clip);
	return result;
}

bool Build_Cascade_View(
	const std::array<Vector3, 8> &corners,
	Vector3 forward,
	float split_near,
	float split_far,
	float depth_padding,
	std::uint32_t map_size,
	ShadowView &shadow_view) noexcept
{
	Vector3 center{};
	for (const Vector3 corner : corners)
		center = Add(center, corner);
	center = Multiply(center, 1.0f / static_cast<float>(corners.size()));

	Vector3 reference_up{0.0f, 0.0f, 1.0f};
	if (std::abs(Dot(forward, reference_up)) > 0.99f)
		reference_up = {0.0f, 1.0f, 0.0f};

	Vector3 right{};
	if (!Normalize(Cross(forward, reference_up), right))
		return false;

	Vector3 up{};
	if (!Normalize(Cross(right, forward), up))
		return false;

	float radius_squared = 0;
	float minimum_z = std::numeric_limits<float>::max();
	float maximum_z = std::numeric_limits<float>::lowest();
	for (const Vector3 corner : corners) {
		const Vector3 relative = Subtract(corner, center);
		const float z = -Dot(forward, relative);
		const float distance_squared = Dot(relative,relative);
		radius_squared = distance_squared > radius_squared ? distance_squared : radius_squared;
		minimum_z = z < minimum_z ? z : minimum_z;
		maximum_z = z > maximum_z ? z : maximum_z;
	}

	const float padding = depth_padding > 0.0f ? depth_padding : 0.0f;
	if (!std::isfinite(radius_squared) || radius_squared <= 0)
		return false;

	// A sphere keeps XY scale invariant under camera rotation. Quantize its
	// radius to suppress roundoff and reserve a texel for center snapping.
	const float radius = std::ceil(std::sqrt(radius_squared)*16.0f)/16.0f;
	const float extent = radius * (1.0f + 2.0f/static_cast<float>(map_size));
	const float texel_size = 2.0f*extent/static_cast<float>(map_size);
	const float center_x = Dot(right,center);
	const float center_y = Dot(up,center);
	const float snapped_x = std::round(center_x/texel_size)*texel_size;
	const float snapped_y = std::round(center_y/texel_size)*texel_size;
	Vector3 origin = Add(center, Multiply(forward, -maximum_z - padding));
	origin = Add(origin,Multiply(right,snapped_x-center_x));
	origin = Add(origin,Multiply(up,snapped_y-center_y));
	// Keep upstream casters in the padded interval. Starting at `padding`
	// would discard precisely the geometry for which the camera was moved.
	const float near_plane = 0.0f;
	const float far_plane = maximum_z - minimum_z + 2.0f * padding;
	if (!std::isfinite(far_plane) || far_plane <= near_plane)
		return false;

	shadow_view.view_matrix = Make_Light_View(right, up, forward, origin);
	shadow_view.projection_matrix = Make_Orthographic(-extent, extent, -extent, extent, near_plane, far_plane);
	shadow_view.view_projection = Multiply_Matrices(shadow_view.projection_matrix, shadow_view.view_matrix);
	shadow_view.viewport = {0.0f, 0.0f, static_cast<float>(map_size), static_cast<float>(map_size), 0.0f, 1.0f};
	shadow_view.split_near = split_near;
	shadow_view.split_far = split_far;
	return true;
}
}

export bool Calculate_Cascade_Splits(float near_clip, float far_clip, float split_lambda, std::span<float> splits) noexcept
{
	if (splits.empty() || splits.size() > Max_Shadow_Cascades
		|| !std::isfinite(near_clip) || !std::isfinite(far_clip) || near_clip <= 0.0f || far_clip <= near_clip)
		return false;

	if (!std::isfinite(split_lambda))
		return false;

	const float lambda = split_lambda < 0.0f ? 0.0f : split_lambda > 1.0f ? 1.0f : split_lambda;
	const float logarithmic_ratio = far_clip / near_clip;
	for (std::size_t index = 0; index < splits.size(); ++index) {
		const float fraction = static_cast<float>(index + 1) / static_cast<float>(splits.size());
		const float logarithmic = near_clip * std::pow(logarithmic_ratio, fraction);
		const float linear = near_clip + (far_clip - near_clip) * fraction;
		splits[index] = logarithmic * lambda + linear * (1.0f - lambda);
	}

	return true;
}

export bool Build_Shadow_Cascades(
	const View &view,
	LightHandle light_handle,
	const RenderLight &light,
	const ShadowSettings &settings,
	ShadowCascades &cascades) noexcept
{
	if (!light_handle.Is_Valid() || light.type != RenderLightType::Directional || !Has_Render_Light_Flag(light.flags, RenderLightFlags::Enabled)
		|| settings.cascade_count == 0 || settings.cascade_count > Max_Shadow_Cascades || settings.map_size == 0
		|| !std::isfinite(settings.near_clip) || !std::isfinite(settings.far_clip) || settings.near_clip <= 0.0f || settings.far_clip <= settings.near_clip
		|| !std::isfinite(settings.depth_padding) || settings.depth_padding < 0.0f)
		return false;

	Vector3 forward{};
	if (!Normalize(light.direction, forward))
		return false;

	const Matrix4x4 view_projection = Multiply_Matrices(view.projection_matrix, view.view_matrix);
	Matrix4x4 inverse_view_projection{};
	if (!Invert(view_projection, inverse_view_projection))
		return false;

	std::array<float, Max_Shadow_Cascades> splits{};
	if (!Calculate_Cascade_Splits(settings.near_clip, settings.far_clip, settings.split_lambda, {splits.data(), settings.cascade_count}))
		return false;

	ShadowCascades candidate;
	candidate.light = light_handle;
	candidate.count = settings.cascade_count;
	float split_near = settings.near_clip;
	for (std::uint32_t cascade_index = 0; cascade_index < settings.cascade_count; ++cascade_index) {
		const float split_far = splits[cascade_index];
		float near_ndc = 0.0f;
		float far_ndc = 0.0f;
		if (!Projection_Depth(view.projection_matrix, -split_near, near_ndc)
			|| !Projection_Depth(view.projection_matrix, -split_far, far_ndc))
			return false;

		std::array<Vector3, 8> corners{};
		std::size_t corner_index = 0;
		for (const float depth : {near_ndc, far_ndc}) {
			for (const float y : {-1.0f, 1.0f}) {
				for (const float x : {-1.0f, 1.0f}) {
					if (!Unproject(inverse_view_projection, x, y, depth, corners[corner_index++]))
						return false;
				}
			}
		}

		if (!Build_Cascade_View(corners, forward, split_near, split_far, settings.depth_padding, settings.map_size, candidate.views[cascade_index]))
			return false;

		split_near = split_far;
	}

	cascades = candidate;
	return true;
}

bool ShadowMapResources::Initialize(Device &device, RenderGraph &graph, std::uint32_t cascade_count, std::uint32_t map_size)
{
	if (!device.Is_Valid() || cascade_count == 0 || cascade_count > Max_Shadow_Cascades || map_size == 0)
		return false;

	Shutdown(device);
	std::array<RHITextureHandle, Max_Shadow_Cascades> textures{};
	std::array<GraphResourceHandle, Max_Shadow_Cascades> targets{};
	const RHITexture description{
		map_size,
		map_size,
		1,
		RHITextureFormat::D32_Float,
		static_cast<std::uint32_t>(RHITextureUsage::ShaderResource) | static_cast<std::uint32_t>(RHITextureUsage::DepthStencil),
		1
	};

	for (std::uint32_t cascade_index = 0; cascade_index < cascade_count; ++cascade_index) {
		textures[cascade_index] = device.Create_Texture(description);
		if (!textures[cascade_index].Is_Valid()) {
			for (std::uint32_t created = 0; created < cascade_index; ++created)
				device.Destroy_Texture(textures[created]);
			return false;
		}

		targets[cascade_index] = graph.Create_Resource({GraphResourceKind::Texture});
		if (!targets[cascade_index].Is_Valid()) {
			device.Destroy_Texture(textures[cascade_index]);
			for (std::uint32_t created = 0; created < cascade_index; ++created)
				device.Destroy_Texture(textures[created]);
			return false;
		}
	}

	m_textures = textures;
	m_targets = targets;
	m_count = cascade_count;
	m_map_size = map_size;
	return true;
}

void ShadowMapResources::Shutdown(Device &device) noexcept
{
	for (std::uint32_t cascade_index = 0; cascade_index < m_count; ++cascade_index) {
		if (m_textures[cascade_index].Is_Valid())
			device.Destroy_Texture(m_textures[cascade_index]);
		m_textures[cascade_index] = {};
		m_targets[cascade_index] = {};
	}
	m_count = 0;
	m_map_size = 0;
}

}
