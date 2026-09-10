module;

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module Assets.Adapters.W3D.Light;

import Assets.Adapters.W3D.Chunks;
import Assets.Math;
export import Assets.Lights;

namespace Assets::W3D
{

export inline constexpr std::uint32_t W3DChunkLight = 0x00000460u;
export inline constexpr std::uint32_t W3DChunkLightInfo = 0x00000461u;
export inline constexpr std::uint32_t W3DChunkSpotLightInfo = 0x00000462u;
export inline constexpr std::uint32_t W3DChunkNearAttenuation = 0x00000463u;
export inline constexpr std::uint32_t W3DChunkFarAttenuation = 0x00000464u;

export inline constexpr std::uint32_t W3DLightAttributeTypeMask = 0x000000FFu;
export inline constexpr std::uint32_t W3DLightAttributePoint = 0x00000001u;
export inline constexpr std::uint32_t W3DLightAttributeDirectional = 0x00000002u;
export inline constexpr std::uint32_t W3DLightAttributeSpot = 0x00000003u;
export inline constexpr std::uint32_t W3DLightAttributeCastShadows = 0x00000100u;

// These are the byte sizes of the authored records, independent of host
// structure packing.  The decoder accepts trailing bytes for forward
// extensions, as the original chunk reader did.
export inline constexpr std::size_t W3DLightInfoPayloadSize = 24u;
export inline constexpr std::size_t W3DSpotLightInfoPayloadSize = 20u;
export inline constexpr std::size_t W3DLightAttenuationPayloadSize = 8u;

export struct W3DLightDecodeMetadata final
{
	bool spot_info_present = false;
};

namespace LightDetail
{

bool Fail(std::string &error, std::string_view message)
{
	error.assign(message.data(), message.size());
	return false;
}

bool Read_Finite_F32(W3DByteSpan bytes, std::size_t offset, float &value) noexcept
{
	return W3DRead_F32(bytes, offset, value) && std::isfinite(value);
}

bool Read_Finite_Vector3(W3DByteSpan bytes, std::size_t offset, Vector3f &value) noexcept
{
	return Read_Finite_F32(bytes, offset, value.x)
		&& Read_Finite_F32(bytes, offset + sizeof(float), value.y)
		&& Read_Finite_F32(bytes, offset + sizeof(float) * 2, value.z);
}

Vector3f Read_RGB(W3DByteSpan bytes, std::size_t offset) noexcept
{
	return {
		static_cast<float>(std::to_integer<std::uint8_t>(bytes[offset])) / 255.0f,
		static_cast<float>(std::to_integer<std::uint8_t>(bytes[offset + 1])) / 255.0f,
		static_cast<float>(std::to_integer<std::uint8_t>(bytes[offset + 2])) / 255.0f};
}

bool Read_Info(W3DByteSpan bytes, LightAssetDesc &light, std::string &error)
{
	if (bytes.size() < W3DLightInfoPayloadSize)
		return Fail(error, "W3D light info is truncated");

	std::uint32_t attributes = 0;
	if (!W3DRead_U32(bytes, 0, attributes))
		return Fail(error, "W3D light attributes are truncated");

	switch (attributes & W3DLightAttributeTypeMask) {
	case W3DLightAttributePoint:
		light.type = LightType::Point;
		break;
	case W3DLightAttributeDirectional:
		light.type = LightType::Directional;
		break;
	case W3DLightAttributeSpot:
		light.type = LightType::Spot;
		break;
	default:
		return Fail(error, "W3D light has an unsupported type");
	}

	light.cast_shadows = (attributes & W3DLightAttributeCastShadows) != 0;
	light.ambient = Read_RGB(bytes, 8);
	light.diffuse = Read_RGB(bytes, 12);
	light.specular = Read_RGB(bytes, 16);
	if (!Read_Finite_F32(bytes, 20, light.intensity))
		return Fail(error, "W3D light intensity is invalid");
	return true;
}

bool Read_Spot(W3DByteSpan bytes, LightAssetDesc &light, std::string &error)
{
	if (bytes.size() < W3DSpotLightInfoPayloadSize
		|| !Read_Finite_Vector3(bytes, 0, light.spot_direction)
		|| !Read_Finite_F32(bytes, 12, light.spot_angle)
		|| !Read_Finite_F32(bytes, 16, light.spot_exponent))
		return Fail(error, "W3D spot light info is invalid");
	return true;
}

bool Read_Attenuation(W3DByteSpan bytes, float &start, float &end, std::string &error,
	std::string_view label)
{
	if (bytes.size() < W3DLightAttenuationPayloadSize
		|| !Read_Finite_F32(bytes, 0, start)
		|| !Read_Finite_F32(bytes, 4, end))
		return Fail(error, label);
	return true;
}

bool Read_Children(W3DByteSpan bytes, LightAssetDesc &light, bool &spot_info_present,
	std::string &error)
{
	bool found_info = false;
	const bool valid = W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
		switch (chunk.id) {
		case W3DChunkLightInfo:
			// The legacy loader consumes the first info record and ignores any
			// later record encountered among optional children.
			if (found_info)
				return true;
			if (chunk.contains_children)
				return Fail(error, "W3D light info unexpectedly contains children");
			found_info = true;
			return Read_Info(chunk.payload, light, error);
		case W3DChunkSpotLightInfo:
			if (chunk.contains_children)
				return Fail(error, "W3D spot light info unexpectedly contains children");
			spot_info_present = true;
			return Read_Spot(chunk.payload, light, error);
		case W3DChunkNearAttenuation:
			if (chunk.contains_children
				|| !Read_Attenuation(chunk.payload, light.near_attenuation_start,
					light.near_attenuation_end, error, "W3D near attenuation is invalid"))
				return false;
			light.near_attenuation_enabled = true;
			return true;
		case W3DChunkFarAttenuation:
			if (chunk.contains_children
				|| !Read_Attenuation(chunk.payload, light.far_attenuation_start,
					light.far_attenuation_end, error, "W3D far attenuation is invalid"))
				return false;
			light.far_attenuation_enabled = true;
			return true;
		default:
			// Unknown light extensions are intentionally ignored, including
			// their payload bytes, to retain the old chunk reader behavior.
			return true;
		}
	});
	if (!valid)
		return error.empty() ? Fail(error, "W3D light child chunks are malformed") : false;
	if (!found_info)
		return Fail(error, "W3D light info chunk is missing");
	return true;
}

bool Unwrap_Root(W3DByteSpan source, W3DByteSpan &children, std::string &error)
{
	children = source;
	if (source.size() < 8)
		return true;

	std::uint32_t id = 0;
	std::uint32_t encoded_size = 0;
	if (!W3DRead_U32(source, 0, id) || !W3DRead_U32(source, 4, encoded_size))
		return Fail(error, "W3D light root header is truncated");
	if (id != W3DChunkLight)
		return true;
	if ((encoded_size & W3DChunkContainsChildren) == 0)
		return Fail(error, "W3D light root does not contain child chunks");
	const std::size_t payload_size = encoded_size & W3DChunkSizeMask;
	if (payload_size != source.size() - 8)
		return Fail(error, "W3D light root has trailing data");
	children = source.subspan(8, payload_size);
	return true;
}

std::uint8_t Encode_Color(float value) noexcept
{
	// Match LightClass::Save_W3D exactly for normalized authored colors:
	// conversion truncates instead of rounding or saturating.
	return static_cast<std::uint8_t>(255.0f * value);
}

void Append_U32(std::vector<std::byte> &bytes, std::uint32_t value)
{
	bytes.push_back(static_cast<std::byte>(value & 0xffu));
	bytes.push_back(static_cast<std::byte>((value >> 8) & 0xffu));
	bytes.push_back(static_cast<std::byte>((value >> 16) & 0xffu));
	bytes.push_back(static_cast<std::byte>((value >> 24) & 0xffu));
}

void Append_Chunk(std::vector<std::byte> &bytes, std::uint32_t id,
	std::span<const std::byte> payload, bool contains_children = false)
{
	Append_U32(bytes, id);
	Append_U32(bytes, static_cast<std::uint32_t>(payload.size())
		| (contains_children ? W3DChunkContainsChildren : 0u));
	bytes.insert(bytes.end(), payload.begin(), payload.end());
}

std::array<std::byte, W3DLightInfoPayloadSize> Encode_Info(const LightAssetDesc &light)
{
	std::array<std::byte, W3DLightInfoPayloadSize> bytes{};
	const auto put_u32 = [&bytes](std::size_t offset, std::uint32_t value) {
		bytes[offset] = static_cast<std::byte>(value & 0xffu);
		bytes[offset + 1] = static_cast<std::byte>((value >> 8) & 0xffu);
		bytes[offset + 2] = static_cast<std::byte>((value >> 16) & 0xffu);
		bytes[offset + 3] = static_cast<std::byte>((value >> 24) & 0xffu);
	};
	const auto put_f32 = [&put_u32](std::size_t offset, float value) {
		put_u32(offset, std::bit_cast<std::uint32_t>(value));
	};
	const auto put_rgb = [&bytes](std::size_t offset, Vector3f value) {
		bytes[offset] = static_cast<std::byte>(Encode_Color(value.x));
		bytes[offset + 1] = static_cast<std::byte>(Encode_Color(value.y));
		bytes[offset + 2] = static_cast<std::byte>(Encode_Color(value.z));
		bytes[offset + 3] = std::byte{};
	};

	const std::uint32_t type = light.type == LightType::Point ? W3DLightAttributePoint
		: light.type == LightType::Directional ? W3DLightAttributeDirectional
		: W3DLightAttributeSpot;
	put_u32(0, type | (light.cast_shadows ? W3DLightAttributeCastShadows : 0u));
	put_rgb(8, light.ambient);
	put_rgb(12, light.diffuse);
	put_rgb(16, light.specular);
	put_f32(20, light.intensity);
	return bytes;
}

std::array<std::byte, W3DSpotLightInfoPayloadSize> Encode_Spot(const LightAssetDesc &light)
{
	std::array<std::byte, W3DSpotLightInfoPayloadSize> bytes{};
	const auto put_u32 = [&bytes](std::size_t offset, std::uint32_t value) {
		bytes[offset] = static_cast<std::byte>(value & 0xffu);
		bytes[offset + 1] = static_cast<std::byte>((value >> 8) & 0xffu);
		bytes[offset + 2] = static_cast<std::byte>((value >> 16) & 0xffu);
		bytes[offset + 3] = static_cast<std::byte>((value >> 24) & 0xffu);
	};
	const auto put_f32 = [&put_u32](std::size_t offset, float value) {
		put_u32(offset, std::bit_cast<std::uint32_t>(value));
	};
	put_f32(0, light.spot_direction.x);
	put_f32(4, light.spot_direction.y);
	put_f32(8, light.spot_direction.z);
	put_f32(12, light.spot_angle);
	put_f32(16, light.spot_exponent);
	return bytes;
}

std::array<std::byte, W3DLightAttenuationPayloadSize> Encode_Attenuation(float start, float end)
{
	std::array<std::byte, W3DLightAttenuationPayloadSize> bytes{};
	const auto put_u32 = [&bytes](std::size_t offset, std::uint32_t value) {
		bytes[offset] = static_cast<std::byte>(value & 0xffu);
		bytes[offset + 1] = static_cast<std::byte>((value >> 8) & 0xffu);
		bytes[offset + 2] = static_cast<std::byte>((value >> 16) & 0xffu);
		bytes[offset + 3] = static_cast<std::byte>((value >> 24) & 0xffu);
	};
	put_u32(0, std::bit_cast<std::uint32_t>(start));
	put_u32(4, std::bit_cast<std::uint32_t>(end));
	return bytes;
}

}

// Reads either the child payload of one W3D_CHUNK_LIGHT or a complete light
// root chunk. No W3D or graphics runtime object is retained by the result.
export bool W3DRead_Light(W3DByteSpan source, LightAssetDesc &result, std::string &error,
	W3DLightDecodeMetadata *metadata = nullptr)
{
	error.clear();
	if (source.empty() || !W3DValidate_Chunk_Tree(source))
		return LightDetail::Fail(error, "W3D light source is empty or malformed");

	W3DByteSpan children;
	if (!LightDetail::Unwrap_Root(source, children, error))
		return false;
	// The original LightClass loader starts from the destination object's
	// defaults and only replaces fields represented by the source chunks. Keep
	// that behavior for optional spot and attenuation records while parsing
	// into a private copy for atomic publication.
	LightAssetDesc parsed = result;
	W3DLightDecodeMetadata parsed_metadata;
	if (!LightDetail::Read_Children(children, parsed, parsed_metadata.spot_info_present, error))
		return false;
	if (!Is_Valid_Light_Asset(parsed))
		return LightDetail::Fail(error, "W3D light values are invalid");
	result = std::move(parsed);
	if (metadata != nullptr)
		*metadata = parsed_metadata;
	return true;
}

export bool W3DWrite_Light(const LightAssetDesc &source, std::vector<std::byte> &result,
	std::string &error)
{
	error.clear();
	if (!Is_Valid_Light_Asset(source))
		return LightDetail::Fail(error, "W3D light values are invalid");

	std::vector<std::byte> children;
	const auto info = LightDetail::Encode_Info(source);
	LightDetail::Append_Chunk(children, W3DChunkLightInfo, info);
	if (source.type == LightType::Spot) {
		const auto spot = LightDetail::Encode_Spot(source);
		LightDetail::Append_Chunk(children, W3DChunkSpotLightInfo, spot);
	}
	if (source.near_attenuation_enabled) {
		const auto near_attenuation = LightDetail::Encode_Attenuation(
			source.near_attenuation_start, source.near_attenuation_end);
		LightDetail::Append_Chunk(children, W3DChunkNearAttenuation, near_attenuation);
	}
	if (source.far_attenuation_enabled) {
		const auto far_attenuation = LightDetail::Encode_Attenuation(
			source.far_attenuation_start, source.far_attenuation_end);
		LightDetail::Append_Chunk(children, W3DChunkFarAttenuation, far_attenuation);
	}

	std::vector<std::byte> encoded;
	LightDetail::Append_Chunk(encoded, W3DChunkLight, children, true);
	result = std::move(encoded);
	return true;
}

}
