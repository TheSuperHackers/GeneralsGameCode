module;

#include <cmath>
#include <cstdint>

export module Assets.Lights;

import Assets.Math;

namespace Assets
{

// The values describe authored light state.  Position and world transform
// remain consumer data because W3D light chunks carry only the light source
// parameters.
export enum class LightType : std::uint8_t
{
	Point,
	Directional,
	Spot
};

export struct LightAssetDesc final
{
	LightType type = LightType::Point;
	bool cast_shadows = false;
	float intensity = 1.0f;
	Vector3f ambient{1.0f, 1.0f, 1.0f};
	Vector3f diffuse{1.0f, 1.0f, 1.0f};
	Vector3f specular{1.0f, 1.0f, 1.0f};

	bool near_attenuation_enabled = false;
	float near_attenuation_start = 0.0f;
	float near_attenuation_end = 0.0f;
	bool far_attenuation_enabled = false;
	float far_attenuation_start = 50.0f;
	float far_attenuation_end = 100.0f;

	Vector3f spot_direction{0.0f, 0.0f, 1.0f};
	float spot_angle = 0.7853981633974483f;
	float spot_exponent = 1.0f;
};

namespace LightDetail
{

bool Finite(Vector3f value) noexcept
{
	return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool Normalized(Vector3f value) noexcept
{
	return Finite(value) && value.x >= 0.0f && value.x <= 1.0f
		&& value.y >= 0.0f && value.y <= 1.0f
		&& value.z >= 0.0f && value.z <= 1.0f;
}

bool Type_Is_Valid(LightType type) noexcept
{
	return type == LightType::Point || type == LightType::Directional || type == LightType::Spot;
}

}

export bool Is_Valid_Light_Asset(const LightAssetDesc &asset) noexcept
{
	return LightDetail::Type_Is_Valid(asset.type) && std::isfinite(asset.intensity)
		&& LightDetail::Normalized(asset.ambient) && LightDetail::Normalized(asset.diffuse)
		&& LightDetail::Normalized(asset.specular) && std::isfinite(asset.near_attenuation_start)
		&& std::isfinite(asset.near_attenuation_end) && std::isfinite(asset.far_attenuation_start)
		&& std::isfinite(asset.far_attenuation_end) && LightDetail::Finite(asset.spot_direction)
		&& std::isfinite(asset.spot_angle) && std::isfinite(asset.spot_exponent);
}

}
