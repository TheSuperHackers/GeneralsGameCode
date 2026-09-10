module;

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

export module Assets.Particles;

import Assets.Math;

namespace Assets
{

export enum class EmitterRandomizerKind : std::uint8_t
{
	Unknown,
	SolidBox,
	SolidSphere,
	HollowSphere,
	SolidCylinder
};

export struct EmitterRandomizerDesc final
{
	EmitterRandomizerKind kind = EmitterRandomizerKind::SolidBox;
	// SolidBox uses x/y/z; spheres use x as radius; SolidCylinder uses x as
	// height and y as radius.
	Vector3f dimensions{};
};

export enum class EmitterGeometryMode : std::uint8_t
{
	SpriteTriangles,
	SpriteQuads,
	Line,
	LineGroupTetra,
	LineGroupPrism,
	Unknown
};

export struct EmitterAtlasDesc final
{
	std::uint32_t columns = 1;
	std::uint32_t rows = 1;
};

export enum class EmitterDepthCompare : std::uint8_t
{
	Never,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	Always,
	Unknown
};

export enum class EmitterBlendFactor : std::uint8_t
{
	Zero,
	One,
	SourceColor,
	OneMinusSourceColor,
	SourceAlpha,
	OneMinusSourceAlpha,
	SourceColorPreFog,
	Unknown
};

export enum class EmitterPrimaryGradient : std::uint8_t
{
	Disabled,
	Modulate,
	Add,
	BumpEnvironmentMap,
	BumpEnvironmentMapLuminance,
	Modulate2X,
	Unknown
};

export enum class EmitterSecondaryGradient : std::uint8_t
{
	Disabled,
	Enabled,
	Unknown
};

export enum class EmitterDetailColorFunction : std::uint8_t
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
	ModulateAlphaAddColor,
	Unknown
};

export enum class EmitterDetailAlphaFunction : std::uint8_t
{
	Disabled,
	Detail,
	Scale,
	InverseScale,
	Unknown
};

export struct EmitterShaderDesc final
{
	EmitterDepthCompare depth_compare = EmitterDepthCompare::LessEqual;
	bool depth_write = false;
	EmitterBlendFactor destination_blend = EmitterBlendFactor::One;
	EmitterPrimaryGradient primary_gradient = EmitterPrimaryGradient::Disabled;
	EmitterSecondaryGradient secondary_gradient = EmitterSecondaryGradient::Disabled;
	EmitterBlendFactor source_blend = EmitterBlendFactor::One;
	bool texturing = true;
	EmitterDetailColorFunction detail_color_function = EmitterDetailColorFunction::Disabled;
	EmitterDetailAlphaFunction detail_alpha_function = EmitterDetailAlphaFunction::Disabled;
	bool alpha_test = false;
	EmitterDetailColorFunction post_detail_color_function = EmitterDetailColorFunction::Disabled;
	EmitterDetailAlphaFunction post_detail_alpha_function = EmitterDetailAlphaFunction::Disabled;
};

export enum class EmitterTextureBlendPolicy : std::uint8_t
{
	Authored,
	AdditiveSprite,
	AlphaSpriteWhenTextureHasAlpha
};

export enum class EmitterLineTextureMapping : std::uint8_t
{
	UniformWidth,
	UniformLength,
	Tiled,
	Unknown
};

export struct EmitterLinePropertiesDesc final
{
	EmitterLineTextureMapping texture_mapping = EmitterLineTextureMapping::UniformWidth;
	bool merge_intersections = false;
	bool freeze_random = false;
	bool disable_sorting = false;
	bool end_caps = false;
	std::uint32_t subdivision_level = 0;
	float noise_amplitude = 0.0f;
	float merge_abort_factor = 0.0f;
	float texture_tile_factor = 0.0f;
	Vector2f uv_offset_rate{};
};

export struct EmitterColorKeyframe final
{
	float time = 0.0f;
	Color4f value{0.0f, 0.0f, 0.0f, 0.0f};
};

export struct EmitterFloatKeyframe final
{
	float time = 0.0f;
	float value = 0.0f;
};

export struct EmitterColorTrack final
{
	float start_time = 0.0f;
	Color4f start{0.0f, 0.0f, 0.0f, 0.0f};
	Color4f random{0.0f, 0.0f, 0.0f, 0.0f};
	std::vector<EmitterColorKeyframe> keys;
};

export struct EmitterFloatTrack final
{
	float start_time = 0.0f;
	float start = 0.0f;
	float random = 0.0f;
	std::vector<EmitterFloatKeyframe> keys;
};

export struct EmitterAssetDesc final
{
	std::string name;
	std::string texture_name;

	float lifetime = 0.0f;
	float emission_rate = 0.0f;
	float max_emissions = 0.0f;
	Vector3f velocity{};
	Vector3f acceleration{};
	float gravity = 0.0f;
	float elasticity = 0.0f;

	std::uint32_t burst_size = 1;
	EmitterRandomizerDesc creation_volume{};
	EmitterRandomizerDesc velocity_random{};
	float outward_velocity = 0.0f;
	float velocity_inheritance = 0.0f;
	EmitterShaderDesc shader{};
	EmitterTextureBlendPolicy texture_blend_policy = EmitterTextureBlendPolicy::Authored;
	EmitterGeometryMode geometry_mode = EmitterGeometryMode::SpriteTriangles;
	EmitterAtlasDesc atlas{};

	EmitterColorTrack color;
	EmitterFloatTrack opacity;
	EmitterFloatTrack size;
	EmitterFloatTrack rotation;
	EmitterFloatTrack frame;
	EmitterFloatTrack blur_time;
	float initial_orientation_random = 0.0f;
	EmitterLinePropertiesDesc line_properties;
	float future_start_time = 0.0f;
};

export class EmitterAsset final
{
public:
	EmitterAsset() = default;
	explicit EmitterAsset(EmitterAssetDesc description)
		: m_description(std::move(description))
	{
	}

	const EmitterAssetDesc &Description() const noexcept { return m_description; }
	const std::string &Name() const noexcept { return m_description.name; }
	const std::string &Texture_Name() const noexcept { return m_description.texture_name; }

private:
	EmitterAssetDesc m_description;
};

}
