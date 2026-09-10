module;

#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

export module Assets.Spheres;

import Assets.Math;

namespace Assets
{

// Sphere attributes are kept as source-compatible bits.  The asset layer
// exposes the bits so scene code can make the visibility/material decisions
// without depending on the W3D record layout.
export inline constexpr std::uint32_t SphereAttributeUseAlphaVector = 0x00000001u;
export inline constexpr std::uint32_t SphereAttributeCameraAligned = 0x00000002u;
export inline constexpr std::uint32_t SphereAttributeInverseAlpha = 0x00000004u;
export inline constexpr std::uint32_t SphereAttributeAnimationLoop = 0x00000008u;

export enum class SphereDepthCompare : std::uint8_t
{
	Never,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	Always
};

export enum class SphereBlendFactor : std::uint8_t
{
	Zero,
	One,
	SourceColor,
	InverseSourceColor,
	SourceAlpha,
	InverseSourceAlpha,
	SourceColorPreFog
};

export enum class SpherePrimaryGradient : std::uint8_t
{
	Disabled,
	Modulate,
	Add,
	BumpEnvironmentMap,
	BumpEnvironmentMapLuminance,
	Modulate2X
};

export enum class SphereSecondaryGradient : std::uint8_t
{
	Disabled,
	Enabled
};

export enum class SphereDetailColorFunction : std::uint8_t
{
	Disabled,
	Detail,
	Scale,
	InverseScale,
	Add,
	Subtract,
	ReverseSubtract,
	Blend,
	DetailBlend,
	AddSigned,
	AddSigned2X,
	Scale2X,
	ModulateAlphaAddColor
};

export enum class SphereDetailAlphaFunction : std::uint8_t
{
	Disabled,
	Detail,
	Scale,
	InverseScale
};

export enum class SphereFogMode : std::uint8_t
{
	Disabled,
	Enabled,
	ScaleFragment,
	White
};

export struct SphereMaterialDesc final
{
	SphereDepthCompare depth_compare = SphereDepthCompare::LessEqual;
	bool depth_write = true;
	bool color_write = true;
	SphereBlendFactor destination_blend = SphereBlendFactor::Zero;
	SphereFogMode fog = SphereFogMode::Disabled;
	SpherePrimaryGradient primary_gradient = SpherePrimaryGradient::Modulate;
	SphereSecondaryGradient secondary_gradient = SphereSecondaryGradient::Disabled;
	SphereBlendFactor source_blend = SphereBlendFactor::One;
	bool texturing = false;
	SphereDetailColorFunction detail_color_function = SphereDetailColorFunction::Disabled;
	SphereDetailAlphaFunction detail_alpha_function = SphereDetailAlphaFunction::Disabled;
	bool alpha_test = false;
	SphereDetailColorFunction post_detail_color_function = SphereDetailColorFunction::Disabled;
	SphereDetailAlphaFunction post_detail_alpha_function = SphereDetailAlphaFunction::Disabled;
	// Sphere normals are authored with the opposite winding in the original
	// data. Keep both sides visible until content is corrected.
	bool cull_enabled = false;
};

export struct SphereColorKeyframe final
{
	float time = 0.0f;
	Vector3f value{0.0f, 0.0f, 0.0f};
};

export struct SphereAlphaKeyframe final
{
	float time = 0.0f;
	float value = 1.0f;
};

export struct SphereScaleKeyframe final
{
	float time = 0.0f;
	Vector3f value{1.0f, 1.0f, 1.0f};
};

export struct SphereVectorKeyframe final
{
	float time = 0.0f;
	std::array<float, 4> rotation{0.0f, 0.0f, 0.0f, 1.0f};
	float intensity = 1.0f;
};

export struct SphereColorTrack final
{
	std::vector<SphereColorKeyframe> keys;
};

export struct SphereAlphaTrack final
{
	std::vector<SphereAlphaKeyframe> keys;
};

export struct SphereScaleTrack final
{
	std::vector<SphereScaleKeyframe> keys;
};

export struct SphereVectorTrack final
{
	std::vector<SphereVectorKeyframe> keys;
};

export struct SphereAssetDesc final
{
	std::uint32_t version = 0;
	std::uint32_t attributes = SphereAttributeUseAlphaVector;
	std::string name;
	Vector3f center{};
	Vector3f extent{1.0f, 1.0f, 1.0f};
	float animation_duration = 0.0f;
	Vector3f default_color{0.75f, 0.75f, 0.75f};
	float default_alpha = 1.0f;
	Vector3f default_scale{1.0f, 1.0f, 1.0f};
	std::array<float, 4> default_vector_rotation{0.0f, 0.0f, 0.0f, 1.0f};
	float default_vector_intensity = 1.0f;
	std::string texture_name;
	SphereMaterialDesc material{};
	SphereColorTrack color_track;
	SphereAlphaTrack alpha_track;
	SphereScaleTrack scale_track;
	SphereVectorTrack vector_track;
};

namespace SphereDetail
{

template <typename Key>
bool Is_Track_Valid(const std::vector<Key> &keys) noexcept
{
	float previous = 0.0f;
	bool first = true;
	for (const Key &key : keys) {
		if (!std::isfinite(key.time) || (!first && !(key.time > previous)))
			return false;
		previous = key.time;
		first = false;
	}
	return true;
}

bool Finite(Vector3f value) noexcept
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

}

export bool Is_Valid_Sphere_Asset(const SphereAssetDesc &asset) noexcept
{
	if (asset.name.empty() || !SphereDetail::Finite(asset.center) || !SphereDetail::Finite(asset.extent)
		|| !std::isfinite(asset.animation_duration) || !SphereDetail::Finite(asset.default_color)
		|| !std::isfinite(asset.default_alpha) || !SphereDetail::Finite(asset.default_scale)
		|| !SphereDetail::Finite(asset.default_vector_rotation) || !std::isfinite(asset.default_vector_intensity))
		return false;
	for (const SphereColorKeyframe &key : asset.color_track.keys)
		if (!SphereDetail::Finite(key.value))
			return false;
	for (const SphereScaleKeyframe &key : asset.scale_track.keys)
		if (!SphereDetail::Finite(key.value))
			return false;
	for (const SphereVectorKeyframe &key : asset.vector_track.keys)
		if (!SphereDetail::Finite(key.rotation) || !std::isfinite(key.intensity))
			return false;
	for (const SphereAlphaKeyframe &key : asset.alpha_track.keys)
		if (!std::isfinite(key.value))
			return false;
	return SphereDetail::Is_Track_Valid(asset.color_track.keys)
		&& SphereDetail::Is_Track_Valid(asset.alpha_track.keys)
		&& SphereDetail::Is_Track_Valid(asset.scale_track.keys)
		&& SphereDetail::Is_Track_Valid(asset.vector_track.keys);
}

export class SphereAsset final
{
public:
	SphereAsset() = default;
	explicit SphereAsset(SphereAssetDesc description)
		: m_description(std::move(description))
	{
	}

	const SphereAssetDesc &Description() const noexcept { return m_description; }
	const std::string &Name() const noexcept { return m_description.name; }
	const std::string &Texture_Name() const noexcept { return m_description.texture_name; }

private:
	SphereAssetDesc m_description;
};

}
