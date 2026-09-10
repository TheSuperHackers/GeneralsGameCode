module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <span>
#include <type_traits>

export module Graphics.Scene.Models.AnimationBlend;

export import Graphics.Scene.Models.Animation;

namespace Graphics
{

namespace
{
struct Vec3 final
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

struct Quaternion final
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;
};

struct TransformComponents final
{
	Vec3 translation{};
	Vec3 scale{1.0f, 1.0f, 1.0f};
	Quaternion rotation{};
};

float Dot(const Vec3 &left, const Vec3 &right) noexcept
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

Vec3 Cross(const Vec3 &left, const Vec3 &right) noexcept
{
	return {
		left.y * right.z - left.z * right.y,
		left.z * right.x - left.x * right.z,
		left.x * right.y - left.y * right.x
	};
}

float Length(const Vec3 &value) noexcept
{
	return std::sqrt(Dot(value, value));
}

Vec3 Normalize(const Vec3 &value, Vec3 fallback) noexcept
{
	const float length = Length(value);
	if (!std::isfinite(length) || length <= 1.0e-8f)
		return fallback;
	return {value.x / length, value.y / length, value.z / length};
}

Quaternion Normalize(Quaternion value) noexcept
{
	const float length = std::sqrt(value.x * value.x + value.y * value.y
		+ value.z * value.z + value.w * value.w);
	if (!std::isfinite(length) || length <= 1.0e-8f)
		return {};
	value.x /= length;
	value.y /= length;
	value.z /= length;
	value.w /= length;
	return value;
}

bool Decompose(const RenderTransform &transform, TransformComponents &components) noexcept
{
	for (const float value : transform.matrix)
		if (!std::isfinite(value))
			return false;

	components.translation = {transform.matrix[3], transform.matrix[7], transform.matrix[11]};
	const Vec3 column_x{transform.matrix[0], transform.matrix[4], transform.matrix[8]};
	const Vec3 column_y{transform.matrix[1], transform.matrix[5], transform.matrix[9]};
	const Vec3 column_z{transform.matrix[2], transform.matrix[6], transform.matrix[10]};
	components.scale = {Length(column_x), Length(column_y), Length(column_z)};

	Vec3 rotation_x = Normalize(column_x, {1.0f, 0.0f, 0.0f});
	Vec3 rotation_y = Normalize(column_y, {0.0f, 1.0f, 0.0f});
	Vec3 rotation_z = Normalize(column_z, {0.0f, 0.0f, 1.0f});
	const float determinant = Dot(rotation_x, Cross(rotation_y, rotation_z));
	if (std::isfinite(determinant) && determinant < 0.0f) {
		components.scale.z = -components.scale.z;
		rotation_z = {-rotation_z.x, -rotation_z.y, -rotation_z.z};
	}

	const float trace = rotation_x.x + rotation_y.y + rotation_z.z;
	Quaternion rotation;
	if (trace > 0.0f) {
		const float root = std::sqrt(trace + 1.0f) * 2.0f;
		rotation.w = 0.25f * root;
		rotation.x = (rotation_y.z - rotation_z.y) / root;
		rotation.y = (rotation_z.x - rotation_x.z) / root;
		rotation.z = (rotation_x.y - rotation_y.x) / root;
	} else if (rotation_x.x > rotation_y.y && rotation_x.x > rotation_z.z) {
		const float root = std::sqrt(std::max(0.0f, 1.0f + rotation_x.x - rotation_y.y - rotation_z.z)) * 2.0f;
		rotation.w = (rotation_y.z - rotation_z.y) / root;
		rotation.x = 0.25f * root;
		rotation.y = (rotation_y.x + rotation_x.y) / root;
		rotation.z = (rotation_z.x + rotation_x.z) / root;
	} else if (rotation_y.y > rotation_z.z) {
		const float root = std::sqrt(std::max(0.0f, 1.0f - rotation_x.x + rotation_y.y - rotation_z.z)) * 2.0f;
		rotation.w = (rotation_z.x - rotation_x.z) / root;
		rotation.x = (rotation_y.x + rotation_x.y) / root;
		rotation.y = 0.25f * root;
		rotation.z = (rotation_z.y + rotation_y.z) / root;
	} else {
		const float root = std::sqrt(std::max(0.0f, 1.0f - rotation_x.x - rotation_y.y + rotation_z.z)) * 2.0f;
		rotation.w = (rotation_x.y - rotation_y.x) / root;
		rotation.x = (rotation_z.x + rotation_x.z) / root;
		rotation.y = (rotation_z.y + rotation_y.z) / root;
		rotation.z = 0.25f * root;
	}
	components.rotation = Normalize(rotation);
	return true;
}

Quaternion Slerp(Quaternion left, Quaternion right, float weight) noexcept
{
	float dot = left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
	if (dot < 0.0f) {
		right.x = -right.x;
		right.y = -right.y;
		right.z = -right.z;
		right.w = -right.w;
		dot = -dot;
	}
	dot = std::clamp(dot, -1.0f, 1.0f);
	if (dot > 0.9995f)
		return Normalize({
			left.x + (right.x - left.x) * weight,
			left.y + (right.y - left.y) * weight,
			left.z + (right.z - left.z) * weight,
			left.w + (right.w - left.w) * weight
		});

	const float angle = std::acos(dot);
	const float sine = std::sin(angle);
	if (!std::isfinite(sine) || std::abs(sine) <= 1.0e-8f)
		return left;
	const float first_weight = std::sin((1.0f - weight) * angle) / sine;
	const float second_weight = std::sin(weight * angle) / sine;
	return Normalize({
		left.x * first_weight + right.x * second_weight,
		left.y * first_weight + right.y * second_weight,
		left.z * first_weight + right.z * second_weight,
		left.w * first_weight + right.w * second_weight
	});
}

RenderTransform Compose(const TransformComponents &components) noexcept
{
	const Quaternion &q = components.rotation;
	const float xx = q.x * q.x;
	const float yy = q.y * q.y;
	const float zz = q.z * q.z;
	const float xy = q.x * q.y;
	const float xz = q.x * q.z;
	const float yz = q.y * q.z;
	const float wx = q.w * q.x;
	const float wy = q.w * q.y;
	const float wz = q.w * q.z;

	RenderTransform result;
	result.matrix = {
		(1.0f - 2.0f * (yy + zz)) * components.scale.x,
		(2.0f * (xy - wz)) * components.scale.y,
		(2.0f * (xz + wy)) * components.scale.z,
		components.translation.x,
		(2.0f * (xy + wz)) * components.scale.x,
		(1.0f - 2.0f * (xx + zz)) * components.scale.y,
		(2.0f * (yz - wx)) * components.scale.z,
		components.translation.y,
		(2.0f * (xz - wy)) * components.scale.x,
		(2.0f * (yz + wx)) * components.scale.y,
		(1.0f - 2.0f * (xx + yy)) * components.scale.z,
		components.translation.z,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};
	return result;
}
}

export bool Blend_Transforms(const RenderTransform &first, const RenderTransform &second,
	float weight, RenderTransform &output) noexcept
{
	if (!std::isfinite(weight))
		return false;
	weight = std::clamp(weight, 0.0f, 1.0f);
	if (weight == 0.0f) {
		output = first;
		return true;
	}
	if (weight == 1.0f) {
		output = second;
		return true;
	}

	TransformComponents first_components;
	TransformComponents second_components;
	if (!Decompose(first, first_components) || !Decompose(second, second_components))
		return false;

	const TransformComponents blended{
		{
			first_components.translation.x + (second_components.translation.x - first_components.translation.x) * weight,
			first_components.translation.y + (second_components.translation.y - first_components.translation.y) * weight,
			first_components.translation.z + (second_components.translation.z - first_components.translation.z) * weight
		},
		{
			first_components.scale.x + (second_components.scale.x - first_components.scale.x) * weight,
			first_components.scale.y + (second_components.scale.y - first_components.scale.y) * weight,
			first_components.scale.z + (second_components.scale.z - first_components.scale.z) * weight
		},
		Slerp(first_components.rotation, second_components.rotation, weight)
	};
	output = Compose(blended);
	return true;
}

export bool Blend_Local_Poses(std::span<const RenderTransform> first,
	std::span<const RenderTransform> second, float weight,
	std::span<RenderTransform> output) noexcept
{
	if (first.size() != second.size() || output.size() < first.size())
		return false;
	for (std::size_t bone = 0; bone < first.size(); ++bone)
		if (!Blend_Transforms(first[bone], second[bone], weight, output[bone]))
			return false;
	return true;
}

export bool Blend_Poses(const Skeleton &skeleton, std::span<const RenderTransform> first,
	std::span<const RenderTransform> second, float weight, Pose &output) noexcept
{
	if (!skeleton.Is_Valid() || first.size() != skeleton.Bone_Count()
		|| second.size() != skeleton.Bone_Count() || !output.Is_Valid()
		|| output.Local_Transforms().size() != skeleton.Bone_Count())
		return false;
	if (!Blend_Local_Poses(first, second, weight, output.Local_Transforms()))
		return false;
	return skeleton.Evaluate_Pose(output.Local_Transforms(), output.World_Transforms());
}

}
