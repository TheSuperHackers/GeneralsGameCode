module;

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <array>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module Assets.Adapters.W3D.Sphere;

import Assets.Adapters.W3D.Chunks;
import Assets.Math;
export import Assets.Spheres;

namespace Assets::W3D
{

export inline constexpr std::uint32_t W3DChunkSphere = 0x00000741u;

namespace SphereDetail
{

constexpr std::size_t SphereDefinitionSize = 164;
constexpr std::size_t ShaderRecordSize = 16;
constexpr std::size_t SphereNameSize = 32;
constexpr std::uint32_t DefinitionChunk = 1;
constexpr std::uint32_t ColorChannelChunk = 2;
constexpr std::uint32_t AlphaChannelChunk = 3;
constexpr std::uint32_t ScaleChannelChunk = 4;
constexpr std::uint32_t VectorChannelChunk = 5;
constexpr std::uint32_t VariablesChunk = 0x03150809u;
constexpr std::uint8_t KeyMicroChunk = 1;
constexpr std::size_t ColorKeySize = 16;
constexpr std::size_t AlphaKeySize = 8;
constexpr std::size_t ScaleKeySize = 16;
constexpr std::size_t VectorKeySize = 24;
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

bool Read_Finite_Vector3(W3DByteSpan bytes, std::size_t offset,
	Assets::Vector3f &value) noexcept
{
	return Read_Finite_F32(bytes, offset, value.x)
		&& Read_Finite_F32(bytes, offset + 4, value.y)
		&& Read_Finite_F32(bytes, offset + 8, value.z);
}

bool Read_Finite_Rotation(W3DByteSpan bytes, std::size_t offset,
	std::array<float, 4> &value) noexcept
{
	for (std::size_t component = 0; component < value.size(); ++component)
		if (!Read_Finite_F32(bytes, offset + component * sizeof(float), value[component]))
			return false;
	return true;
}

bool Read_Fixed_Name(W3DByteSpan bytes, std::size_t offset, std::size_t length,
	std::string &value) noexcept
{
	if (offset > bytes.size() || bytes.size() - offset < length)
		return false;
	value = W3DRead_Fixed_String(bytes, offset, length);
	return true;
}

bool Map_Depth_Compare(std::uint8_t value, SphereDepthCompare &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(SphereDepthCompare::Always))
		return false;
	mapped = static_cast<SphereDepthCompare>(value);
	return true;
}

// Destination and source records use different wire enumerations. The
// destination's values 2 and 3 refer to source color, while the source's
// values 2 and 3 refer to source alpha.
bool Map_Destination_Blend(std::uint8_t value, SphereBlendFactor &mapped) noexcept
{
	switch (value) {
	case 0: mapped = SphereBlendFactor::Zero; return true;
	case 1: mapped = SphereBlendFactor::One; return true;
	case 2: mapped = SphereBlendFactor::SourceColor; return true;
	case 3: mapped = SphereBlendFactor::InverseSourceColor; return true;
	case 4: mapped = SphereBlendFactor::SourceAlpha; return true;
	case 5: mapped = SphereBlendFactor::InverseSourceAlpha; return true;
	case 6: mapped = SphereBlendFactor::SourceColorPreFog; return true;
	default: return false;
	}
}

bool Map_Source_Blend(std::uint8_t value, SphereBlendFactor &mapped) noexcept
{
	switch (value) {
	case 0: mapped = SphereBlendFactor::Zero; return true;
	case 1: mapped = SphereBlendFactor::One; return true;
	case 2: mapped = SphereBlendFactor::SourceAlpha; return true;
	case 3: mapped = SphereBlendFactor::InverseSourceAlpha; return true;
	default: return false;
	}
}

bool Map_Primary_Gradient(std::uint8_t value, SpherePrimaryGradient &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(SpherePrimaryGradient::Modulate2X))
		return false;
	mapped = static_cast<SpherePrimaryGradient>(value);
	return true;
}

bool Map_Secondary_Gradient(std::uint8_t value, SphereSecondaryGradient &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(SphereSecondaryGradient::Enabled))
		return false;
	mapped = static_cast<SphereSecondaryGradient>(value);
	return true;
}

bool Map_Detail_Color(std::uint8_t value, SphereDetailColorFunction &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(SphereDetailColorFunction::ModulateAlphaAddColor))
		return false;
	mapped = static_cast<SphereDetailColorFunction>(value);
	return true;
}

bool Map_Detail_Alpha(std::uint8_t value, SphereDetailAlphaFunction &mapped) noexcept
{
	if (value > static_cast<std::uint8_t>(SphereDetailAlphaFunction::InverseScale))
		return false;
	mapped = static_cast<SphereDetailAlphaFunction>(value);
	return true;
}

bool Parse_Shader(W3DByteSpan bytes, SphereMaterialDesc &material, std::string &error)
{
	if (bytes.size() < ShaderRecordSize)
		return Fail(error, "sphere shader record is truncated");

	std::uint8_t depth_compare = 0;
	std::uint8_t depth_mask = 0;
	std::uint8_t destination_blend = 0;
	std::uint8_t primary_gradient = 0;
	std::uint8_t secondary_gradient = 0;
	std::uint8_t source_blend = 0;
	std::uint8_t texturing = 0;
	std::uint8_t detail_color = 0;
	std::uint8_t detail_alpha = 0;
	std::uint8_t alpha_test = 0;
	if (!Read_Byte(bytes, 0, depth_compare) || !Read_Byte(bytes, 1, depth_mask)
		|| !Read_Byte(bytes, 3, destination_blend)
		|| !Read_Byte(bytes, 5, primary_gradient)
		|| !Read_Byte(bytes, 6, secondary_gradient) || !Read_Byte(bytes, 7, source_blend)
		|| !Read_Byte(bytes, 8, texturing) || !Read_Byte(bytes, 9, detail_color)
		|| !Read_Byte(bytes, 10, detail_alpha) || !Read_Byte(bytes, 12, alpha_test)
		|| depth_mask > 1 || texturing > 1 || alpha_test > 1
		|| !Map_Depth_Compare(depth_compare, material.depth_compare)
		|| !Map_Destination_Blend(destination_blend, material.destination_blend)
		|| !Map_Primary_Gradient(primary_gradient, material.primary_gradient)
		|| !Map_Secondary_Gradient(secondary_gradient, material.secondary_gradient)
		|| !Map_Source_Blend(source_blend, material.source_blend)
		|| !Map_Detail_Color(detail_color, material.detail_color_function)
		|| !Map_Detail_Alpha(detail_alpha, material.detail_alpha_function))
		return Fail(error, "sphere shader record contains an unsupported setting");

	material.depth_write = depth_mask != 0;
	// Material record translation forces color writes on and
	// disables fog. These obsolete wire fields are therefore not decoded.
	material.color_write = true;
	material.fog = SphereFogMode::Disabled;
	material.texturing = texturing != 0;
	material.alpha_test = alpha_test != 0;
	// The loader uses the authored detail functions as the post-detail pass;
	// raw post-detail bytes are ignored by the effective shader state.
	material.post_detail_color_function = material.detail_color_function;
	material.post_detail_alpha_function = material.detail_alpha_function;
	return true;
}

bool Parse_Definition(W3DByteSpan bytes, SphereAssetDesc &description, std::string &error)
{
	if (bytes.size() < SphereDefinitionSize)
		return Fail(error, "sphere definition is truncated");

	if (!W3DRead_U32(bytes, 0, description.version)
		|| !W3DRead_U32(bytes, 4, description.attributes)
		|| !Read_Fixed_Name(bytes, 8, SphereNameSize, description.name)
		|| description.name.empty()
		|| !Read_Finite_Vector3(bytes, 40, description.center)
		|| !Read_Finite_Vector3(bytes, 52, description.extent)
		|| !Read_Finite_F32(bytes, 64, description.animation_duration)
		|| !Read_Finite_Vector3(bytes, 68, description.default_color)
		|| !Read_Finite_F32(bytes, 80, description.default_alpha)
		|| !Read_Finite_Vector3(bytes, 84, description.default_scale)
		|| !Read_Finite_Rotation(bytes, 96, description.default_vector_rotation)
		|| !Read_Finite_F32(bytes, 112, description.default_vector_intensity)
		|| !Read_Fixed_Name(bytes, 116, SphereNameSize, description.texture_name)
		|| !Parse_Shader(bytes.subspan(148, ShaderRecordSize), description.material, error))
		return error.empty() ? Fail(error, "sphere definition is invalid") : false;
	// The render object enables texturing from the resolved texture, so the
	// authored name is the asset-level source of whether this pass can sample.
	description.material.texturing = !description.texture_name.empty();
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
			if (!read_value(bytes.subspan(offset, micro_size), key)
				|| (!parsed.empty() && !(key.time > parsed.back().time)))
				return Fail(error, label);
			parsed.push_back(key);
		}
		offset += micro_size;
	}
	keys = std::move(parsed);
	return true;
}

bool Parse_Channel(W3DByteSpan bytes, std::size_t key_size,
	SphereColorTrack *color, SphereAlphaTrack *alpha, SphereScaleTrack *scale,
	SphereVectorTrack *vector, std::string &error, std::string_view label)
{
	if (color == nullptr && alpha == nullptr && scale == nullptr && vector == nullptr)
		return false;

	bool found_variables = false;
	if (!W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
		if (chunk.id != VariablesChunk)
			return true;
		if (found_variables || chunk.contains_children)
			return false;
		found_variables = true;
		if (color != nullptr)
			return Parse_Track<SphereColorKeyframe>(chunk.payload, key_size, color->keys,
				[](W3DByteSpan record, SphereColorKeyframe &key) {
					return W3DRead_F32(record, 0, key.value.x)
						&& W3DRead_F32(record, 4, key.value.y)
						&& W3DRead_F32(record, 8, key.value.z)
						&& W3DRead_F32(record, 12, key.time)
						&& std::isfinite(key.value.x) && std::isfinite(key.value.y)
						&& std::isfinite(key.value.z) && std::isfinite(key.time);
				}, error, label);
		if (alpha != nullptr)
			return Parse_Track<SphereAlphaKeyframe>(chunk.payload, key_size, alpha->keys,
				[](W3DByteSpan record, SphereAlphaKeyframe &key) {
					return W3DRead_F32(record, 0, key.value) && W3DRead_F32(record, 4, key.time)
						&& std::isfinite(key.value) && std::isfinite(key.time);
				}, error, label);
		if (scale != nullptr)
			return Parse_Track<SphereScaleKeyframe>(chunk.payload, key_size, scale->keys,
				[](W3DByteSpan record, SphereScaleKeyframe &key) {
					return W3DRead_F32(record, 0, key.value.x)
						&& W3DRead_F32(record, 4, key.value.y)
						&& W3DRead_F32(record, 8, key.value.z)
						&& W3DRead_F32(record, 12, key.time)
						&& std::isfinite(key.value.x) && std::isfinite(key.value.y)
						&& std::isfinite(key.value.z) && std::isfinite(key.time);
				}, error, label);
		return Parse_Track<SphereVectorKeyframe>(chunk.payload, key_size, vector->keys,
			[](W3DByteSpan record, SphereVectorKeyframe &key) {
				return Read_Finite_Rotation(record, 0, key.rotation)
					&& Read_Finite_F32(record, 16, key.intensity)
					&& Read_Finite_F32(record, 20, key.time);
			}, error, label);
	}))
		return error.empty() ? Fail(error, label) : false;
	return true;
}

bool Read_Sphere_Children(W3DByteSpan bytes, SphereAssetDesc &description, std::string &error)
{
	bool has_definition = false;
	bool has_color = false;
	bool has_alpha = false;
	bool has_scale = false;
	bool has_vector = false;
	if (!W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
		switch (chunk.id) {
		case DefinitionChunk:
			if (has_definition || chunk.contains_children)
				return false;
			has_definition = true;
			return Parse_Definition(chunk.payload, description, error);
		case ColorChannelChunk:
			if (has_color || !chunk.contains_children)
				return false;
			has_color = true;
			return Parse_Channel(chunk.payload, ColorKeySize, &description.color_track,
				nullptr, nullptr, nullptr, error, "sphere color channel is invalid");
		case AlphaChannelChunk:
			if (has_alpha || !chunk.contains_children)
				return false;
			has_alpha = true;
			return Parse_Channel(chunk.payload, AlphaKeySize, nullptr,
				&description.alpha_track, nullptr, nullptr, error, "sphere alpha channel is invalid");
		case ScaleChannelChunk:
			if (has_scale || !chunk.contains_children)
				return false;
			has_scale = true;
			return Parse_Channel(chunk.payload, ScaleKeySize, nullptr, nullptr,
				&description.scale_track, nullptr, error, "sphere scale channel is invalid");
		case VectorChannelChunk:
			if (has_vector || !chunk.contains_children)
				return false;
			has_vector = true;
			return Parse_Channel(chunk.payload, VectorKeySize, nullptr, nullptr,
				nullptr, &description.vector_track, error, "sphere vector channel is invalid");
		default:
			return true;
		}
	}))
		return error.empty() ? Fail(error, "sphere child chunks are malformed") : false;
	if (!has_definition)
		return Fail(error, "sphere definition chunk is missing");
	return Is_Valid_Sphere_Asset(description)
		|| Fail(error, "sphere asset contains invalid values");
}

bool Unwrap_Sphere_Root(W3DByteSpan source, W3DByteSpan &children, std::string &error)
{
	children = source;
	if (source.size() < 8)
		return true;

	std::uint32_t id = 0;
	std::uint32_t encoded_size = 0;
	if (!W3DRead_U32(source, 0, id) || !W3DRead_U32(source, 4, encoded_size))
		return Fail(error, "sphere source header is truncated");
	if (id != W3DChunkSphere)
		return true;
	if ((encoded_size & W3DChunkContainsChildren) == 0)
		return Fail(error, "sphere root does not contain child chunks");
	const std::size_t payload_size = encoded_size & W3DChunkSizeMask;
	if (payload_size != source.size() - 8)
		return Fail(error, "sphere root has trailing data");
	children = source.subspan(8, payload_size);
	return true;
}

}

export bool W3DRead_Sphere(W3DByteSpan source, SphereAssetDesc &result, std::string &error)
{
	error.clear();
	if (source.empty() || !W3DValidate_Chunk_Tree(source))
		return SphereDetail::Fail(error, "sphere source is empty or has malformed chunks");

	W3DByteSpan children;
	if (!SphereDetail::Unwrap_Sphere_Root(source, children, error))
		return false;
	SphereAssetDesc parsed;
	if (!SphereDetail::Read_Sphere_Children(children, parsed, error))
		return false;
	result = std::move(parsed);
	return true;
}

}
