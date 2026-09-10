module;

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module Assets.Adapters.W3D.Ring;

import Assets.Adapters.W3D.Chunks;
import Assets.Math;
export import Assets.Rings;

namespace Assets::W3D
{

export inline constexpr std::uint32_t W3DChunkRing = 0x00000742;

namespace RingDetail
{

constexpr std::size_t RingDefinitionSize = 168;
constexpr std::size_t ShaderRecordSize = 16;
constexpr std::uint32_t RingDefinitionChunk = 1;
constexpr std::uint32_t ColorChannelChunk = 2;
constexpr std::uint32_t AlphaChannelChunk = 3;
constexpr std::uint32_t InnerScaleChannelChunk = 4;
constexpr std::uint32_t OuterScaleChannelChunk = 5;
constexpr std::uint32_t VariablesChunk = 0x03150809;
constexpr std::uint8_t KeyMicroChunk = 1;
constexpr std::size_t ColorKeySize = 16;
constexpr std::size_t AlphaKeySize = 8;
constexpr std::size_t ScaleKeySize = 12;
constexpr std::size_t MaximumKeyCount = 1u << 20;

bool Fail(std::string &error, std::string_view message)
{
	error.assign(message.data(), message.size());
	return false;
}

bool Read_Byte(W3DByteSpan bytes, std::size_t offset, std::uint8_t &value) noexcept
{
	if (offset >= bytes.size())
		return false;
	value = std::to_integer<std::uint8_t>(bytes[offset]);
	return true;
}

bool Read_Finite_F32(W3DByteSpan bytes, std::size_t offset, float &value) noexcept
{
	return W3DRead_F32(bytes, offset, value) && std::isfinite(value);
}

bool Read_Finite_Vector2(W3DByteSpan bytes, std::size_t offset, Vector2f &value) noexcept
{
	return Read_Finite_F32(bytes, offset, value.x) && Read_Finite_F32(bytes, offset + 4, value.y);
}

bool Read_Finite_Vector3(W3DByteSpan bytes, std::size_t offset, Vector3f &value) noexcept
{
	return Read_Finite_F32(bytes, offset, value.x) &&
		Read_Finite_F32(bytes, offset + 4, value.y) &&
		Read_Finite_F32(bytes, offset + 8, value.z);
}

bool Read_Fixed_Name(W3DByteSpan bytes, std::size_t offset, std::size_t length,
	std::string &value) noexcept
{
	if (offset > bytes.size() || bytes.size() - offset < length)
		return false;
	value = W3DRead_Fixed_String(bytes, offset, length);
	return true;
}

bool Map_Depth_Compare(std::uint8_t value, RingDepthCompare &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(RingDepthCompare::Always))
		return false;
	mapped = static_cast<RingDepthCompare>(value);
	return true;
}

bool Map_Destination_Blend(std::uint8_t value, RingBlendFactor &mapped) noexcept
{
	switch (value) {
	case 0: mapped = RingBlendFactor::Zero; return true;
	case 1: mapped = RingBlendFactor::One; return true;
	case 2: mapped = RingBlendFactor::SourceColor; return true;
	case 3: mapped = RingBlendFactor::InverseSourceColor; return true;
	case 4: mapped = RingBlendFactor::SourceAlpha; return true;
	case 5: mapped = RingBlendFactor::InverseSourceAlpha; return true;
	case 6: mapped = RingBlendFactor::SourceColorPreFog; return true;
	default: return false;
	}
}

bool Map_Source_Blend(std::uint8_t value, RingBlendFactor &mapped) noexcept
{
	switch (value) {
	case 0: mapped = RingBlendFactor::Zero; return true;
	case 1: mapped = RingBlendFactor::One; return true;
	case 2: mapped = RingBlendFactor::SourceAlpha; return true;
	case 3: mapped = RingBlendFactor::InverseSourceAlpha; return true;
	default: return false;
	}
}

bool Map_Primary_Gradient(std::uint8_t value, RingPrimaryGradient &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(RingPrimaryGradient::Modulate2X))
		return false;
	mapped = static_cast<RingPrimaryGradient>(value);
	return true;
}

bool Map_Secondary_Gradient(std::uint8_t value, RingSecondaryGradient &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(RingSecondaryGradient::Enabled))
		return false;
	mapped = static_cast<RingSecondaryGradient>(value);
	return true;
}

bool Map_Detail_Color(std::uint8_t value, RingDetailColorFunction &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(RingDetailColorFunction::ModulateAlphaAddColor))
		return false;
	mapped = static_cast<RingDetailColorFunction>(value);
	return true;
}

bool Map_Detail_Alpha(std::uint8_t value, RingDetailAlphaFunction &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(RingDetailAlphaFunction::InverseScale))
		return false;
	mapped = static_cast<RingDetailAlphaFunction>(value);
	return true;
}

bool Map_Fog(std::uint8_t value, RingFogMode &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(RingFogMode::White))
		return false;
	mapped = static_cast<RingFogMode>(value);
	return true;
}

bool Parse_Shader(W3DByteSpan bytes, RingMaterialDesc &material, std::string &error)
{
	if (bytes.size() < ShaderRecordSize)
		return Fail(error, "ring shader record is truncated");

	std::uint8_t depth_compare = 0;
	std::uint8_t depth_mask = 0;
	std::uint8_t color_mask = 0;
	std::uint8_t destination_blend = 0;
	std::uint8_t fog = 0;
	std::uint8_t primary_gradient = 0;
	std::uint8_t secondary_gradient = 0;
	std::uint8_t source_blend = 0;
	std::uint8_t texturing = 0;
	std::uint8_t detail_color = 0;
	std::uint8_t detail_alpha = 0;
	std::uint8_t alpha_test = 0;
	if (!Read_Byte(bytes, 0, depth_compare) || !Read_Byte(bytes, 1, depth_mask) ||
		!Read_Byte(bytes, 2, color_mask) || !Read_Byte(bytes, 3, destination_blend) ||
		!Read_Byte(bytes, 4, fog) || !Read_Byte(bytes, 5, primary_gradient) ||
		!Read_Byte(bytes, 6, secondary_gradient) || !Read_Byte(bytes, 7, source_blend) ||
		!Read_Byte(bytes, 8, texturing) || !Read_Byte(bytes, 9, detail_color) ||
		!Read_Byte(bytes, 10, detail_alpha) || !Read_Byte(bytes, 12, alpha_test) ||
		depth_mask > 1 || color_mask > 1 || texturing > 1 || alpha_test > 1 ||
		!Map_Depth_Compare(depth_compare, material.depth_compare) ||
		!Map_Destination_Blend(destination_blend, material.destination_blend) ||
		!Map_Primary_Gradient(primary_gradient, material.primary_gradient) ||
		!Map_Secondary_Gradient(secondary_gradient, material.secondary_gradient) ||
		!Map_Source_Blend(source_blend, material.source_blend) ||
		!Map_Detail_Color(detail_color, material.post_detail_color_function) ||
		!Map_Detail_Alpha(detail_alpha, material.post_detail_alpha_function))
		return Fail(error, "ring shader record contains an unsupported setting");

	material.depth_write = depth_mask != 0;
	// ColorMask and FogFunc are obsolete W3D fields. Their effective ring
	// state is owned by the generic prop pass and is always writable/unfogged.
	material.color_write = true;
	material.fog = RingFogMode::Disabled;
	material.texturing = texturing != 0;
	material.detail_color_function = RingDetailColorFunction::Disabled;
	material.detail_alpha_function = RingDetailAlphaFunction::Disabled;
	material.alpha_test = alpha_test != 0;
	return true;
}

bool Parse_Definition(W3DByteSpan bytes, RingAssetDesc &description, std::string &error)
{
	if (bytes.size() < RingDefinitionSize)
		return Fail(error, "ring definition is truncated");

	std::uint32_t attributes = 0;
	std::uint32_t texture_tile_count = 0;
	if (!W3DRead_U32(bytes, 4, attributes) ||
		!Read_Fixed_Name(bytes, 8, 32, description.name) || description.name.empty() ||
		!Read_Finite_Vector3(bytes, 40, description.center) ||
		!Read_Finite_Vector3(bytes, 52, description.extent) ||
		!Read_Finite_F32(bytes, 64, description.animation_duration) ||
		!Read_Finite_F32(bytes, 68, description.default_color.r) ||
		!Read_Finite_F32(bytes, 72, description.default_color.g) ||
		!Read_Finite_F32(bytes, 76, description.default_color.b) ||
		!Read_Finite_F32(bytes, 80, description.default_alpha) ||
		!Read_Finite_Vector2(bytes, 84, description.default_inner_scale) ||
		!Read_Finite_Vector2(bytes, 92, description.default_outer_scale) ||
		!Read_Finite_Vector2(bytes, 100, description.inner_extent) ||
		!Read_Finite_Vector2(bytes, 108, description.outer_extent) ||
		!Read_Fixed_Name(bytes, 116, 32, description.texture_name) ||
		!Parse_Shader(bytes.subspan(148, ShaderRecordSize), description.material, error) ||
		!W3DRead_U32(bytes, 164, texture_tile_count))
		return error.empty() ? Fail(error, "ring definition is invalid") : false;

	description.default_color.a = 1.0f;
	description.texture_tile_count = static_cast<std::int32_t>(texture_tile_count);
	// The texture name is the authoritative source of whether this pass can
	// sample a texture. Some W3D writers leave the obsolete Texturing bit off.
	description.material.texturing = !description.texture_name.empty();
	description.camera_aligned = (attributes & 0x00000001u) != 0;
	description.animation_loop = (attributes & 0x00000002u) != 0;
	return true;
}

template <typename Key, typename ValueReader>
bool Parse_Track(W3DByteSpan bytes, std::size_t key_size, std::vector<Key> &keys,
	ValueReader &&read_value, std::string &error, std::string_view label)
{
	std::vector<Key> parsed;
	std::size_t offset = 0;
	while (offset < bytes.size()) {
		if (bytes.size() - offset < 2)
			return Fail(error, label);
		const std::uint8_t micro_id = std::to_integer<std::uint8_t>(bytes[offset]);
		const std::size_t micro_size = std::to_integer<std::uint8_t>(bytes[offset + 1]);
		offset += 2;
		if (micro_size > bytes.size() - offset)
			return Fail(error, label);

		if (micro_id == KeyMicroChunk) {
			if (micro_size != key_size || parsed.size() >= MaximumKeyCount)
				return Fail(error, label);
			Key key{};
			if (!read_value(bytes.subspan(offset, micro_size), key) ||
				(!parsed.empty() && !(key.time > parsed.back().time)))
				return Fail(error, label);
			parsed.push_back(key);
		}
		offset += micro_size;
	}

	keys = std::move(parsed);
	return true;
}

bool Parse_Channel(W3DByteSpan bytes, std::size_t key_size,
	RingColorTrack *color, RingAlphaTrack *alpha, RingScaleTrack *scale,
	std::string &error, std::string_view label)
{
	if (color == nullptr && alpha == nullptr && scale == nullptr)
		return false;

	bool found_variables = false;
	if (!W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
		if (chunk.id != VariablesChunk)
			return true;
		if (found_variables || chunk.contains_children)
			return false;
		found_variables = true;
		if (color != nullptr)
			return Parse_Track<RingColorKeyframe>(chunk.payload, key_size, color->keys,
				[](W3DByteSpan record, RingColorKeyframe &key) {
					return W3DRead_F32(record, 0, key.value.r) &&
						W3DRead_F32(record, 4, key.value.g) &&
						W3DRead_F32(record, 8, key.value.b) &&
						W3DRead_F32(record, 12, key.time) &&
						std::isfinite(key.value.r) && std::isfinite(key.value.g) &&
						std::isfinite(key.value.b) && std::isfinite(key.time);
				}, error, label);
		if (alpha != nullptr)
			return Parse_Track<RingAlphaKeyframe>(chunk.payload, key_size, alpha->keys,
				[](W3DByteSpan record, RingAlphaKeyframe &key) {
					return W3DRead_F32(record, 0, key.value) && W3DRead_F32(record, 4, key.time) &&
						std::isfinite(key.value) && std::isfinite(key.time);
				}, error, label);
		return Parse_Track<RingScaleKeyframe>(chunk.payload, key_size, scale->keys,
			[](W3DByteSpan record, RingScaleKeyframe &key) {
				return W3DRead_F32(record, 0, key.value.x) && W3DRead_F32(record, 4, key.value.y) &&
					W3DRead_F32(record, 8, key.time) && std::isfinite(key.value.x) &&
					std::isfinite(key.value.y) && std::isfinite(key.time);
			}, error, label);
	}))
		return error.empty() ? Fail(error, label) : false;
	return true;
}

bool Read_Ring_Children(W3DByteSpan bytes, RingAssetDesc &description, std::string &error)
{
	bool has_definition = false;
	bool has_color = false;
	bool has_alpha = false;
	bool has_inner_scale = false;
	bool has_outer_scale = false;
	if (!W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
		switch (chunk.id) {
		case RingDefinitionChunk:
			if (has_definition || chunk.contains_children)
				return false;
			has_definition = true;
			return Parse_Definition(chunk.payload, description, error);
		case ColorChannelChunk:
			if (has_color || !chunk.contains_children)
				return false;
			has_color = true;
			return Parse_Channel(chunk.payload, ColorKeySize, &description.color_track, nullptr, nullptr,
				error, "ring color channel is invalid");
		case AlphaChannelChunk:
			if (has_alpha || !chunk.contains_children)
				return false;
			has_alpha = true;
			return Parse_Channel(chunk.payload, AlphaKeySize, nullptr, &description.alpha_track, nullptr,
				error, "ring alpha channel is invalid");
		case InnerScaleChannelChunk:
			if (has_inner_scale || !chunk.contains_children)
				return false;
			has_inner_scale = true;
			return Parse_Channel(chunk.payload, ScaleKeySize, nullptr, nullptr,
				&description.inner_scale_track, error, "ring inner scale channel is invalid");
		case OuterScaleChannelChunk:
			if (has_outer_scale || !chunk.contains_children)
				return false;
			has_outer_scale = true;
			return Parse_Channel(chunk.payload, ScaleKeySize, nullptr, nullptr,
				&description.outer_scale_track, error, "ring outer scale channel is invalid");
		default:
			return true;
		}
	}))
		return error.empty() ? Fail(error, "ring child chunks are malformed") : false;
	if (!has_definition)
		return Fail(error, "ring definition chunk is missing");
	return true;
}

bool Unwrap_Ring_Root(W3DByteSpan source, W3DByteSpan &children, std::string &error)
{
	children = source;
	if (source.size() < 8)
		return true;

	std::uint32_t id = 0;
	std::uint32_t encoded_size = 0;
	if (!W3DRead_U32(source, 0, id) || !W3DRead_U32(source, 4, encoded_size))
		return Fail(error, "ring source header is truncated");
	if (id != W3DChunkRing)
		return true;
	if ((encoded_size & W3DChunkContainsChildren) == 0)
		return Fail(error, "ring root does not contain child chunks");
	const std::size_t payload_size = encoded_size & W3DChunkSizeMask;
	if (payload_size != source.size() - 8)
		return Fail(error, "ring root has trailing data");
	children = source.subspan(8, payload_size);
	return true;
}

}

export bool W3DRead_Ring(W3DByteSpan source, RingAssetDesc &result, std::string &error)
{
	error.clear();
	if (source.empty() || !W3DValidate_Chunk_Tree(source))
		return RingDetail::Fail(error, "ring source is empty or has malformed chunks");

	W3DByteSpan children;
	if (!RingDetail::Unwrap_Ring_Root(source, children, error))
		return false;
	RingAssetDesc parsed;
	if (!RingDetail::Read_Ring_Children(children, parsed, error))
		return false;
	result = std::move(parsed);
	return true;
}

}
