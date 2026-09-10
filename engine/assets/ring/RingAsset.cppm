module;

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

export module Assets.Rings;

import Assets.Math;

namespace Assets
{

// These values describe the effective state consumed by a ring draw pass.  They
// intentionally do not expose the byte layout of the source W3D shader record.
export enum class RingDepthCompare : std::uint8_t
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

export enum class RingBlendFactor : std::uint8_t
{
	Zero,
	One,
	SourceColor,
	InverseSourceColor,
	SourceAlpha,
	InverseSourceAlpha,
	SourceColorPreFog
};

export enum class RingPrimaryGradient : std::uint8_t
{
	Disabled,
	Modulate,
	Add,
	BumpEnvironmentMap,
	BumpEnvironmentMapLuminance,
	Modulate2X
};

export enum class RingSecondaryGradient : std::uint8_t
{
	Disabled,
	Enabled
};

export enum class RingDetailColorFunction : std::uint8_t
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

export enum class RingDetailAlphaFunction : std::uint8_t
{
	Disabled,
	Detail,
	Scale,
	InverseScale
};

export enum class RingFogMode : std::uint8_t
{
	Disabled,
	Enabled,
	ScaleFragment,
	White
};

export struct RingMaterialDesc final
{
	RingDepthCompare depth_compare = RingDepthCompare::LessEqual;
	bool depth_write = false;
	bool color_write = true;
	RingBlendFactor destination_blend = RingBlendFactor::InverseSourceAlpha;
	RingFogMode fog = RingFogMode::Disabled;
	RingPrimaryGradient primary_gradient = RingPrimaryGradient::Modulate;
	RingSecondaryGradient secondary_gradient = RingSecondaryGradient::Disabled;
	RingBlendFactor source_blend = RingBlendFactor::SourceAlpha;
	bool texturing = true;
	RingDetailColorFunction detail_color_function = RingDetailColorFunction::Disabled;
	RingDetailAlphaFunction detail_alpha_function = RingDetailAlphaFunction::Disabled;
	bool alpha_test = false;
	RingDetailColorFunction post_detail_color_function = RingDetailColorFunction::Disabled;
	RingDetailAlphaFunction post_detail_alpha_function = RingDetailAlphaFunction::Disabled;
	bool cull_enabled = true;
};

export struct RingColorKeyframe final
{
	float time = 0.0f;
	Color4f value{0.0f, 0.0f, 0.0f, 1.0f};
};

export struct RingAlphaKeyframe final
{
	float time = 0.0f;
	float value = 1.0f;
};

export struct RingScaleKeyframe final
{
	float time = 0.0f;
	Vector2f value{1.0f, 1.0f};
};

export struct RingColorTrack final
{
	std::vector<RingColorKeyframe> keys;
};

export struct RingAlphaTrack final
{
	std::vector<RingAlphaKeyframe> keys;
};

export struct RingScaleTrack final
{
	std::vector<RingScaleKeyframe> keys;
};

export struct RingAssetDesc final
{
	std::string name;
	Vector3f center{};
	Vector3f extent{1.0f, 1.0f, 1.0f};
	float animation_duration = 0.0f;
	Color4f default_color{0.75f, 0.75f, 0.75f, 1.0f};
	float default_alpha = 1.0f;
	Vector2f default_inner_scale{1.0f, 1.0f};
	Vector2f default_outer_scale{1.0f, 1.0f};
	Vector2f inner_extent{0.5f, 0.5f};
	Vector2f outer_extent{1.0f, 1.0f};
	std::string texture_name;
	RingMaterialDesc material{};
	int texture_tile_count = 5;
	bool camera_aligned = false;
	bool animation_loop = false;
	RingColorTrack color_track;
	RingAlphaTrack alpha_track;
	RingScaleTrack inner_scale_track;
	RingScaleTrack outer_scale_track;
};

export class RingAsset final
{
public:
	RingAsset() = default;
	explicit RingAsset(RingAssetDesc description)
		: m_description(std::move(description))
	{
	}

	const RingAssetDesc &Description() const noexcept { return m_description; }
	const std::string &Name() const noexcept { return m_description.name; }
	const std::string &Texture_Name() const noexcept { return m_description.texture_name; }

private:
	RingAssetDesc m_description;
};

}
