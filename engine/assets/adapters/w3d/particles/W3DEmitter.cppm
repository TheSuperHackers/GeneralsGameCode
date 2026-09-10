module;

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>

export module Assets.Adapters.W3D.Particles;

import Assets.Adapters.W3D.Chunks;
import Assets.Math;
export import Assets.Particles;

namespace Assets::W3D
{

export inline constexpr std::uint32_t W3DChunkEmitter = 0x00000500;
export inline constexpr std::uint32_t W3DChunkEmitterHeader = 0x00000501;
export inline constexpr std::uint32_t W3DChunkEmitterUserData = 0x00000502;
export inline constexpr std::uint32_t W3DChunkEmitterInfo = 0x00000503;
export inline constexpr std::uint32_t W3DChunkEmitterInfoV2 = 0x00000504;
export inline constexpr std::uint32_t W3DChunkEmitterProperties = 0x00000505;
export inline constexpr std::uint32_t W3DChunkEmitterLineProperties = 0x00000509;
export inline constexpr std::uint32_t W3DChunkEmitterRotationKeyframes = 0x0000050A;
export inline constexpr std::uint32_t W3DChunkEmitterFrameKeyframes = 0x0000050B;
export inline constexpr std::uint32_t W3DChunkEmitterBlurTimeKeyframes = 0x0000050C;
export inline constexpr std::uint32_t W3DChunkEmitterExtraInfo = 0x0000050D;

namespace EmitterDetail
{

constexpr std::size_t HeaderSize = 20;
constexpr std::size_t UserHeaderSize = 12;
constexpr std::size_t InfoSize = 332;
constexpr std::size_t InfoV2Size = 124;
constexpr std::size_t PropertiesHeaderSize = 40;
constexpr std::size_t RotationHeaderSize = 16;
constexpr std::size_t FrameHeaderSize = 16;
constexpr std::size_t BlurHeaderSize = 12;
constexpr std::size_t LinePropertiesSize = 64;
constexpr std::size_t ExtraInfoSize = 40;
constexpr std::size_t KeyframeSize = 8;
constexpr std::uint32_t VersionOneLimit = 0x00010000;
constexpr std::uint32_t MaxKeyframes = 1u << 20;

bool Fail(std::string &error, std::string_view message)
{
	error.assign(message.data(), message.size());
	return false;
}

bool Read_Byte(W3DByteSpan bytes, std::size_t offset, std::uint8_t &value)
{
	if (offset >= bytes.size())
		return false;
	value = std::to_integer<std::uint8_t>(bytes[offset]);
	return true;
}

bool Read_Finite_F32(W3DByteSpan bytes, std::size_t offset, float &value)
{
	return W3DRead_F32(bytes, offset, value) && std::isfinite(value);
}

bool Read_Finite_Vector3(W3DByteSpan bytes, std::size_t offset, Vector3f &value)
{
	return W3DRead_Vector3(bytes, offset, value) && std::isfinite(value.x) &&
		std::isfinite(value.y) && std::isfinite(value.z);
}

bool Read_Color(W3DByteSpan bytes, std::size_t offset, Color4f &value)
{
	if (offset > bytes.size() || bytes.size() - offset < 4)
		return false;
	value = {
		static_cast<float>(std::to_integer<std::uint8_t>(bytes[offset])) / 255.0f,
		static_cast<float>(std::to_integer<std::uint8_t>(bytes[offset + 1])) / 255.0f,
		static_cast<float>(std::to_integer<std::uint8_t>(bytes[offset + 2])) / 255.0f,
		static_cast<float>(std::to_integer<std::uint8_t>(bytes[offset + 3])) / 255.0f};
	return true;
}

bool Read_Fixed_String(W3DByteSpan bytes, std::size_t offset, std::size_t length,
	std::string &value)
{
	if (offset > bytes.size() || bytes.size() - offset < length)
		return false;
	value = W3DRead_Fixed_String(bytes, offset, length);
	return true;
}

std::string Normalize_Texture_Name(std::string_view authored)
{
	const std::size_t slash = authored.find_last_of("\\/");
	if (slash == std::string_view::npos)
		return std::string(authored);
	return std::string(authored.substr(slash + 1));
}

struct SourceHeader final
{
	std::uint32_t version = 0;
	std::string name;
};

struct SourceUserData final
{
	std::uint32_t type = 0;
	std::string_view value;
};

struct SourceInfo final
{
	std::string texture_name;
	float start_size = 0.0f;
	float end_size = 0.0f;
	float lifetime = 0.0f;
	float emission_rate = 0.0f;
	float max_emissions = 0.0f;
	float velocity_random = 0.0f;
	float position_random = 0.0f;
	float fade_time = 0.0f;
	float gravity = 0.0f;
	float elasticity = 0.0f;
	Vector3f velocity{};
	Vector3f acceleration{};
	Color4f start_color{};
	Color4f end_color{};
};

struct CapturedChunks final
{
	W3DByteSpan header;
	W3DByteSpan user_data;
	W3DByteSpan info;
	W3DByteSpan info_v2;
	W3DByteSpan properties;
	W3DByteSpan line_properties;
	W3DByteSpan rotation;
	W3DByteSpan frame;
	W3DByteSpan blur;
	W3DByteSpan extra;
	bool has_header = false;
	bool has_user_data = false;
	bool has_info = false;
	bool has_info_v2 = false;
	bool has_properties = false;
	bool has_line_properties = false;
	bool has_rotation = false;
	bool has_frame = false;
	bool has_blur = false;
	bool has_extra = false;
};

bool Capture_Chunk(W3DByteSpan &destination, bool &present,
	const W3DChunkView &chunk, std::string &error, std::string_view label)
{
	if (present || chunk.contains_children)
		return Fail(error, label);
	present = true;
	destination = chunk.payload;
	return true;
}

bool Parse_Header(W3DByteSpan bytes, SourceHeader &header, std::string &error)
{
	if (bytes.size() < HeaderSize)
		return Fail(error, "emitter header is truncated");
	if (!W3DRead_U32(bytes, 0, header.version) ||
		!Read_Fixed_String(bytes, 4, 16, header.name) || header.name.empty())
		return Fail(error, "emitter header is invalid");
	return true;
}

bool Parse_User_Data(W3DByteSpan bytes, SourceUserData &user, std::string &error)
{
	if (bytes.size() < UserHeaderSize || !W3DRead_U32(bytes, 0, user.type))
		return Fail(error, "emitter user data header is truncated");
	std::uint32_t string_size = 0;
	if (!W3DRead_U32(bytes, 4, string_size) ||
		string_size > bytes.size() - UserHeaderSize)
		return Fail(error, "emitter user data string is truncated");
	user.value = std::string_view(reinterpret_cast<const char *>(bytes.data() + UserHeaderSize),
		string_size);
	return true;
}

bool Parse_Info(W3DByteSpan bytes, SourceInfo &info, std::string &error)
{
	if (bytes.size() < InfoSize || !Read_Fixed_String(bytes, 0, 260, info.texture_name) ||
		!Read_Finite_F32(bytes, 260, info.start_size) ||
		!Read_Finite_F32(bytes, 264, info.end_size) ||
		!Read_Finite_F32(bytes, 268, info.lifetime) ||
		!Read_Finite_F32(bytes, 272, info.emission_rate) ||
		!Read_Finite_F32(bytes, 276, info.max_emissions) ||
		!Read_Finite_F32(bytes, 280, info.velocity_random) ||
		!Read_Finite_F32(bytes, 284, info.position_random) ||
		!Read_Finite_F32(bytes, 288, info.fade_time) ||
		!Read_Finite_F32(bytes, 292, info.gravity) ||
		!Read_Finite_F32(bytes, 296, info.elasticity) ||
		!Read_Finite_Vector3(bytes, 300, info.velocity) ||
		!Read_Finite_Vector3(bytes, 312, info.acceleration) ||
		!Read_Color(bytes, 324, info.start_color) || !Read_Color(bytes, 328, info.end_color))
		return Fail(error, "emitter info is truncated or contains a non-finite value");
	return true;
}

bool Parse_Randomizer(W3DByteSpan bytes, std::size_t offset,
	EmitterRandomizerDesc &randomizer, std::string &error)
{
	std::uint32_t class_id = 0;
	float value1 = 0.0f;
	float value2 = 0.0f;
	float value3 = 0.0f;
	if (!W3DRead_U32(bytes, offset, class_id) ||
		!Read_Finite_F32(bytes, offset + 4, value1) ||
		!Read_Finite_F32(bytes, offset + 8, value2) ||
		!Read_Finite_F32(bytes, offset + 12, value3))
		return Fail(error, "emitter randomizer is truncated or contains a non-finite value");

	switch (class_id) {
	case 0:
		randomizer.kind = EmitterRandomizerKind::SolidBox;
		randomizer.dimensions = {value1, value2, value3};
		break;
	case 1:
		randomizer.kind = EmitterRandomizerKind::SolidSphere;
		randomizer.dimensions = {value1, 0.0f, 0.0f};
		break;
	case 2:
		randomizer.kind = EmitterRandomizerKind::HollowSphere;
		randomizer.dimensions = {value1, 0.0f, 0.0f};
		break;
	case 3:
		randomizer.kind = EmitterRandomizerKind::SolidCylinder;
		randomizer.dimensions = {value1, value2, 0.0f};
		break;
	default:
		return Fail(error, "emitter randomizer class is unsupported");
	}
	return true;
}

bool Map_Depth_Compare(std::uint8_t value, EmitterDepthCompare &mapped)
{
	if (value > 7)
		return false;
	mapped = static_cast<EmitterDepthCompare>(value);
	return true;
}

bool Map_Destination_Blend(std::uint8_t value, EmitterBlendFactor &mapped)
{
	switch (value) {
	case 0: mapped = EmitterBlendFactor::Zero; return true;
	case 1: mapped = EmitterBlendFactor::One; return true;
	case 2: mapped = EmitterBlendFactor::SourceColor; return true;
	case 3: mapped = EmitterBlendFactor::OneMinusSourceColor; return true;
	case 4: mapped = EmitterBlendFactor::SourceAlpha; return true;
	case 5: mapped = EmitterBlendFactor::OneMinusSourceAlpha; return true;
	case 6: mapped = EmitterBlendFactor::SourceColorPreFog; return true;
	default: return false;
	}
}

bool Map_Source_Blend(std::uint8_t value, EmitterBlendFactor &mapped)
{
	switch (value) {
	case 0: mapped = EmitterBlendFactor::Zero; return true;
	case 1: mapped = EmitterBlendFactor::One; return true;
	case 2: mapped = EmitterBlendFactor::SourceAlpha; return true;
	case 3: mapped = EmitterBlendFactor::OneMinusSourceAlpha; return true;
	default: return false;
	}
}

bool Map_Primary_Gradient(std::uint8_t value, EmitterPrimaryGradient &mapped)
{
	if (value > 5)
		return false;
	mapped = static_cast<EmitterPrimaryGradient>(value);
	return true;
}

bool Map_Secondary_Gradient(std::uint8_t value, EmitterSecondaryGradient &mapped)
{
	if (value > 1)
		return false;
	mapped = static_cast<EmitterSecondaryGradient>(value);
	return true;
}

bool Map_Detail_Color(std::uint8_t value, EmitterDetailColorFunction &mapped)
{
	if (value > 12)
		return false;
	mapped = static_cast<EmitterDetailColorFunction>(value);
	return true;
}

bool Map_Detail_Alpha(std::uint8_t value, EmitterDetailAlphaFunction &mapped)
{
	if (value > 3)
		return false;
	mapped = static_cast<EmitterDetailAlphaFunction>(value);
	return true;
}

bool Parse_Shader(W3DByteSpan bytes, EmitterShaderDesc &shader, std::string &error)
{
	if (bytes.size() < 16)
		return Fail(error, "emitter shader is truncated");

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
	std::uint8_t post_detail_color = 0;
	std::uint8_t post_detail_alpha = 0;
	if (!Read_Byte(bytes, 0, depth_compare) || !Read_Byte(bytes, 1, depth_mask) ||
		!Read_Byte(bytes, 3, destination_blend) || !Read_Byte(bytes, 5, primary_gradient) ||
		!Read_Byte(bytes, 6, secondary_gradient) || !Read_Byte(bytes, 7, source_blend) ||
		!Read_Byte(bytes, 8, texturing) || !Read_Byte(bytes, 9, detail_color) ||
		!Read_Byte(bytes, 10, detail_alpha) || !Read_Byte(bytes, 12, alpha_test) ||
		!Read_Byte(bytes, 13, post_detail_color) || !Read_Byte(bytes, 14, post_detail_alpha) ||
		depth_mask > 1 || texturing > 1 || alpha_test > 1 ||
		!Map_Depth_Compare(depth_compare, shader.depth_compare) ||
		!Map_Destination_Blend(destination_blend, shader.destination_blend) ||
		!Map_Primary_Gradient(primary_gradient, shader.primary_gradient) ||
		!Map_Secondary_Gradient(secondary_gradient, shader.secondary_gradient) ||
		!Map_Source_Blend(source_blend, shader.source_blend) ||
		!Map_Detail_Color(detail_color, shader.detail_color_function) ||
		!Map_Detail_Alpha(detail_alpha, shader.detail_alpha_function) ||
		!Map_Detail_Color(post_detail_color, shader.post_detail_color_function) ||
		!Map_Detail_Alpha(post_detail_alpha, shader.post_detail_alpha_function))
		return Fail(error, "emitter shader contains an unsupported setting");

	shader.depth_write = depth_mask != 0;
	shader.texturing = texturing != 0;
	shader.alpha_test = alpha_test != 0;
	return true;
}

bool Parse_Geometry_Mode(std::uint32_t value, EmitterGeometryMode &mode)
{
	switch (value) {
	case 0: mode = EmitterGeometryMode::SpriteTriangles; return true;
	case 1: mode = EmitterGeometryMode::SpriteQuads; return true;
	case 2: mode = EmitterGeometryMode::Line; return true;
	case 3: mode = EmitterGeometryMode::LineGroupTetra; return true;
	case 4: mode = EmitterGeometryMode::LineGroupPrism; return true;
	default: return false;
	}
}

bool Parse_Atlas_Mode(std::uint32_t value, EmitterAtlasDesc &atlas)
{
	if (value > 4)
		return false;
	const std::uint32_t side = 1u << value;
	atlas.columns = side;
	atlas.rows = side;
	return true;
}

bool Parse_Info_V2(W3DByteSpan bytes, EmitterAssetDesc &description, std::string &error)
{
	if (bytes.size() < InfoV2Size)
		return Fail(error, "emitter version 2 info is truncated");
	std::uint32_t burst_size = 0;
	std::uint32_t render_mode = 0;
	std::uint32_t frame_mode = 0;
	if (!W3DRead_U32(bytes, 0, burst_size) ||
		!Parse_Randomizer(bytes, 4, description.creation_volume, error) ||
		!Parse_Randomizer(bytes, 36, description.velocity_random, error) ||
		!Read_Finite_F32(bytes, 68, description.outward_velocity) ||
		!Read_Finite_F32(bytes, 72, description.velocity_inheritance) ||
		!Parse_Shader(bytes.subspan(76, 16), description.shader, error) ||
		!W3DRead_U32(bytes, 92, render_mode) || !Parse_Geometry_Mode(render_mode, description.geometry_mode) ||
		!W3DRead_U32(bytes, 96, frame_mode) || !Parse_Atlas_Mode(frame_mode, description.atlas))
		return error.empty() ? Fail(error, "emitter version 2 info is invalid") : false;

	// The runtime treats a zero burst as the default single-particle burst.
	description.burst_size = burst_size == 0 ? 1 : burst_size;
	return true;
}

bool Read_Color_Record(W3DByteSpan bytes, std::size_t offset,
	float &time, Color4f &value)
{
	return Read_Finite_F32(bytes, offset, time) && Read_Color(bytes, offset + 4, value);
}

bool Read_Float_Record(W3DByteSpan bytes, std::size_t offset,
	float &time, float &value)
{
	return Read_Finite_F32(bytes, offset, time) && Read_Finite_F32(bytes, offset + 4, value);
}

bool Parse_Properties(W3DByteSpan bytes, EmitterAssetDesc &description, std::string &error)
{
	if (bytes.size() < PropertiesHeaderSize)
		return Fail(error, "emitter properties header is truncated");
	std::uint32_t color_count = 0;
	std::uint32_t opacity_count = 0;
	std::uint32_t size_count = 0;
	if (!W3DRead_U32(bytes, 0, color_count) || !W3DRead_U32(bytes, 4, opacity_count) ||
		!W3DRead_U32(bytes, 8, size_count) || color_count == 0 || opacity_count == 0 ||
		size_count == 0 || color_count > MaxKeyframes || opacity_count > MaxKeyframes ||
		size_count > MaxKeyframes || !Read_Color(bytes, 12, description.color.random) ||
		!Read_Finite_F32(bytes, 16, description.opacity.random) ||
		!Read_Finite_F32(bytes, 20, description.size.random))
		return Fail(error, "emitter properties header is invalid");

	std::size_t remaining = bytes.size() - PropertiesHeaderSize;
	if (color_count > remaining / KeyframeSize)
		return Fail(error, "emitter color keyframes are truncated");
	const std::size_t color_bytes = static_cast<std::size_t>(color_count) * KeyframeSize;
	remaining -= color_bytes;
	if (opacity_count > remaining / KeyframeSize)
		return Fail(error, "emitter opacity keyframes are truncated");
	const std::size_t opacity_bytes = static_cast<std::size_t>(opacity_count) * KeyframeSize;
	remaining -= opacity_bytes;
	if (size_count > remaining / KeyframeSize)
		return Fail(error, "emitter size keyframes are truncated");

	description.color.keys.clear();
	description.opacity.keys.clear();
	description.size.keys.clear();
	description.color.keys.reserve(color_count - 1);
	description.opacity.keys.reserve(opacity_count - 1);
	description.size.keys.reserve(size_count - 1);

	std::size_t offset = PropertiesHeaderSize;
	float previous_time = 0.0f;
	if (!Read_Color_Record(bytes, offset, description.color.start_time, description.color.start))
		return Fail(error, "emitter color start keyframe is invalid");
	previous_time = description.color.start_time;
	for (std::uint32_t index = 1; index < color_count; ++index) {
		EmitterColorKeyframe key;
		if (!Read_Color_Record(bytes, offset + static_cast<std::size_t>(index) * KeyframeSize,
			key.time, key.value) || !(key.time > previous_time))
			return Fail(error, "emitter color keyframe times are invalid");
		previous_time = key.time;
		description.color.keys.push_back(key);
	}

	offset += color_bytes;
	if (!Read_Float_Record(bytes, offset, description.opacity.start_time, description.opacity.start))
		return Fail(error, "emitter opacity start keyframe is invalid");
	previous_time = description.opacity.start_time;
	for (std::uint32_t index = 1; index < opacity_count; ++index) {
		EmitterFloatKeyframe key;
		if (!Read_Float_Record(bytes, offset + static_cast<std::size_t>(index) * KeyframeSize,
			key.time, key.value) || !(key.time > previous_time))
			return Fail(error, "emitter opacity keyframe times are invalid");
		previous_time = key.time;
		description.opacity.keys.push_back(key);
	}

	offset += opacity_bytes;
	if (!Read_Float_Record(bytes, offset, description.size.start_time, description.size.start))
		return Fail(error, "emitter size start keyframe is invalid");
	previous_time = description.size.start_time;
	for (std::uint32_t index = 1; index < size_count; ++index) {
		EmitterFloatKeyframe key;
		if (!Read_Float_Record(bytes, offset + static_cast<std::size_t>(index) * KeyframeSize,
			key.time, key.value) || !(key.time > previous_time))
			return Fail(error, "emitter size keyframe times are invalid");
		previous_time = key.time;
		description.size.keys.push_back(key);
	}
	return true;
}

bool Parse_Optional_Float_Track(W3DByteSpan bytes, std::size_t header_size,
	EmitterFloatTrack &track, float *auxiliary, std::string &error,
	std::string_view label)
{
	if (bytes.size() < header_size || header_size < 8)
		return Fail(error, label);
	std::uint32_t key_count = 0;
	if (!W3DRead_U32(bytes, 0, key_count) || key_count > MaxKeyframes ||
		!Read_Finite_F32(bytes, 4, track.random))
		return Fail(error, label);
	if (auxiliary != nullptr && !Read_Finite_F32(bytes, 8, *auxiliary))
		return Fail(error, label);

	const std::size_t available = bytes.size() - header_size;
	if (available < KeyframeSize || static_cast<std::size_t>(key_count) + 1 > available / KeyframeSize)
		return Fail(error, label);
	track.keys.clear();
	track.keys.reserve(key_count);
	if (!Read_Float_Record(bytes, header_size, track.start_time, track.start))
		return Fail(error, label);
	float previous_time = track.start_time;
	for (std::uint32_t index = 1; index <= key_count; ++index) {
		EmitterFloatKeyframe key;
		if (!Read_Float_Record(bytes, header_size + static_cast<std::size_t>(index) * KeyframeSize,
			key.time, key.value) || !(key.time > previous_time))
			return Fail(error, label);
		previous_time = key.time;
		track.keys.push_back(key);
	}
	return true;
}

bool Parse_Line_Properties(W3DByteSpan bytes, EmitterLinePropertiesDesc &line,
	std::string &error)
{
	if (bytes.size() < LinePropertiesSize)
		return Fail(error, "emitter line properties are truncated");
	std::uint32_t flags = 0;
	if (!W3DRead_U32(bytes, 0, flags) || !W3DRead_U32(bytes, 4, line.subdivision_level) ||
		!Read_Finite_F32(bytes, 8, line.noise_amplitude) ||
		!Read_Finite_F32(bytes, 12, line.merge_abort_factor) ||
		!Read_Finite_F32(bytes, 16, line.texture_tile_factor) ||
		!Read_Finite_F32(bytes, 20, line.uv_offset_rate.x) ||
		!Read_Finite_F32(bytes, 24, line.uv_offset_rate.y))
		return Fail(error, "emitter line properties are invalid");

	const std::uint32_t mapping = (flags >> 24) & 0xff;
	switch (mapping) {
	case 0: line.texture_mapping = EmitterLineTextureMapping::UniformWidth; break;
	case 1: line.texture_mapping = EmitterLineTextureMapping::UniformLength; break;
	case 2: line.texture_mapping = EmitterLineTextureMapping::Tiled; break;
	default: return Fail(error, "emitter line texture mapping is unsupported");
	}
	line.merge_intersections = (flags & 0x1) != 0;
	line.freeze_random = (flags & 0x2) != 0;
	line.disable_sorting = (flags & 0x4) != 0;
	line.end_caps = (flags & 0x8) != 0;
	return true;
}

bool Parse_Extra_Info(W3DByteSpan bytes, float &future_start_time, std::string &error)
{
	if (bytes.size() < ExtraInfoSize || !Read_Finite_F32(bytes, 0, future_start_time))
		return Fail(error, "emitter extra info is invalid");
	return true;
}

bool Unwrap_Root(W3DByteSpan source, W3DByteSpan &payload, std::string &error)
{
	if (source.empty())
		return Fail(error, "emitter source is empty");
	std::uint32_t first_id = 0;
	if (!W3DRead_U32(source, 0, first_id))
		return Fail(error, "emitter source has no chunk header");
	if (first_id != W3DChunkEmitter) {
		payload = source;
		return true;
	}

	W3DChunkView root;
	std::size_t count = 0;
	if (!W3DVisit_Chunks(source, [&root, &count](const W3DChunkView &chunk) {
		root = chunk;
		++count;
		return true;
	}) || count != 1 || !root.contains_children)
		return Fail(error, "emitter root chunk is invalid");
	payload = root.payload;
	return true;
}

bool Capture_Chunks(W3DByteSpan bytes, CapturedChunks &chunks, std::string &error)
{
	return W3DVisit_Chunks(bytes, [&chunks, &error](const W3DChunkView &chunk) {
		switch (chunk.id) {
		case W3DChunkEmitterHeader:
			return Capture_Chunk(chunks.header, chunks.has_header, chunk, error,
				"duplicate or nested emitter header");
		case W3DChunkEmitterUserData:
			return Capture_Chunk(chunks.user_data, chunks.has_user_data, chunk, error,
				"duplicate or nested emitter user data");
		case W3DChunkEmitterInfo:
			return Capture_Chunk(chunks.info, chunks.has_info, chunk, error,
				"duplicate or nested emitter info");
		case W3DChunkEmitterInfoV2:
			return Capture_Chunk(chunks.info_v2, chunks.has_info_v2, chunk, error,
				"duplicate or nested emitter version 2 info");
		case W3DChunkEmitterProperties:
			return Capture_Chunk(chunks.properties, chunks.has_properties, chunk, error,
				"duplicate or nested emitter properties");
		case W3DChunkEmitterLineProperties:
			return Capture_Chunk(chunks.line_properties, chunks.has_line_properties, chunk, error,
				"duplicate or nested emitter line properties");
		case W3DChunkEmitterRotationKeyframes:
			return Capture_Chunk(chunks.rotation, chunks.has_rotation, chunk, error,
				"duplicate or nested emitter rotation keyframes");
		case W3DChunkEmitterFrameKeyframes:
			return Capture_Chunk(chunks.frame, chunks.has_frame, chunk, error,
				"duplicate or nested emitter frame keyframes");
		case W3DChunkEmitterBlurTimeKeyframes:
			return Capture_Chunk(chunks.blur, chunks.has_blur, chunk, error,
				"duplicate or nested emitter blur keyframes");
		case W3DChunkEmitterExtraInfo:
			return Capture_Chunk(chunks.extra, chunks.has_extra, chunk, error,
				"duplicate or nested emitter extra info");
		default:
			return true;
		}
	});
}

bool Convert_Version_One(const SourceInfo &info, EmitterAssetDesc &description)
{
	const float position_extent = info.position_random / 1000.0f;
	description.burst_size = 1;
	description.creation_volume = {
		EmitterRandomizerKind::SolidBox,
		{position_extent, position_extent, position_extent}};
	description.velocity_random = {
		EmitterRandomizerKind::SolidBox,
		{info.velocity_random, info.velocity_random, info.velocity_random}};
	description.outward_velocity = 0.0f;
	description.velocity_inheritance = 0.0f;
	description.shader = {};
	description.texture_blend_policy = EmitterTextureBlendPolicy::AlphaSpriteWhenTextureHasAlpha;
	description.geometry_mode = EmitterGeometryMode::SpriteTriangles;
	description.atlas = {};

	description.color.start_time = 0.0f;
	description.color.start = info.start_color;
	description.color.random = {};
	description.color.keys = {{info.fade_time, info.end_color}};
	description.opacity.start_time = 0.0f;
	description.opacity.start = info.start_color.a;
	description.opacity.random = 0.0f;
	description.opacity.keys = {{info.fade_time, info.end_color.a}};
	description.size.start_time = 0.0f;
	description.size.start = info.start_size;
	description.size.random = 0.0f;
	description.size.keys.clear();
	return true;
}

bool Parse_Emitter(W3DByteSpan source, EmitterAssetDesc &result, std::string &error)
{
	error.clear();
	if (!W3DValidate_Chunk_Tree(source))
		return Fail(error, "emitter chunk tree is truncated or invalid");

	W3DByteSpan payload;
	if (!Unwrap_Root(source, payload, error))
		return false;
	CapturedChunks chunks;
	if (!Capture_Chunks(payload, chunks, error))
		return false;
	if (!chunks.has_header || !chunks.has_user_data || !chunks.has_info)
		return Fail(error, "emitter is missing a required source chunk");

	SourceHeader header;
	SourceUserData user;
	SourceInfo info;
	if (!Parse_Header(chunks.header, header, error) ||
		!Parse_User_Data(chunks.user_data, user, error) ||
		!Parse_Info(chunks.info, info, error))
		return false;
	(void)user;

	EmitterAssetDesc next;
	next.name = std::move(header.name);
	next.texture_name = Normalize_Texture_Name(info.texture_name);
	next.lifetime = info.lifetime;
	next.emission_rate = info.emission_rate;
	next.max_emissions = info.max_emissions;
	next.velocity = info.velocity;
	next.acceleration = info.acceleration;
	next.gravity = info.gravity;
	next.elasticity = info.elasticity;

	if (header.version > VersionOneLimit) {
		if (!chunks.has_info_v2 || !chunks.has_properties)
			return Fail(error, "version 2 emitter is missing info or properties");
		if (!Parse_Info_V2(chunks.info_v2, next, error) ||
			!Parse_Properties(chunks.properties, next, error))
			return false;
	} else {
		Convert_Version_One(info, next);
	}

	if (chunks.has_line_properties &&
		!Parse_Line_Properties(chunks.line_properties, next.line_properties, error))
		return false;
	if (chunks.has_rotation &&
		!Parse_Optional_Float_Track(chunks.rotation, RotationHeaderSize, next.rotation,
			&next.initial_orientation_random, error, "emitter rotation keyframes are invalid"))
		return false;
	if (chunks.has_frame &&
		!Parse_Optional_Float_Track(chunks.frame, FrameHeaderSize, next.frame,
			nullptr, error, "emitter frame keyframes are invalid"))
		return false;
	if (chunks.has_blur &&
		!Parse_Optional_Float_Track(chunks.blur, BlurHeaderSize, next.blur_time,
			nullptr, error, "emitter blur keyframes are invalid"))
		return false;
	if (chunks.has_extra && !Parse_Extra_Info(chunks.extra, next.future_start_time, error))
		return false;

	result = std::move(next);
	return true;
}

}

export bool W3DRead_Emitter(W3DByteSpan source, EmitterAssetDesc &result, std::string &error)
{
	return EmitterDetail::Parse_Emitter(source, result, error);
}

}
