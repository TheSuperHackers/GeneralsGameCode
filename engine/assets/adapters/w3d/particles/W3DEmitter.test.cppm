module;

#define BOOST_TEST_MODULE GeneralsAssetsW3DEmitterTests

#include <boost/test/included/unit_test.hpp>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

export module Assets.Tests.W3DEmitter;

import Assets.Adapters.W3D.Particles;
import Assets.Adapters.W3D.Chunks;

namespace
{

using Byte = std::byte;
using Bytes = std::vector<Byte>;
using namespace Assets::W3D;

constexpr std::size_t HeaderSize = 20;
constexpr std::size_t InfoSize = 332;
constexpr std::size_t InfoV2Size = 124;
constexpr std::size_t PropertiesHeaderSize = 40;
constexpr std::size_t RotationHeaderSize = 16;
constexpr std::size_t FrameHeaderSize = 16;
constexpr std::size_t BlurHeaderSize = 12;
constexpr std::size_t LinePropertiesSize = 64;
constexpr std::size_t ExtraInfoSize = 40;
constexpr std::size_t KeyframeSize = 8;

void Append_U32(Bytes &bytes, std::uint32_t value)
{
	bytes.push_back(static_cast<Byte>(value & 0xff));
	bytes.push_back(static_cast<Byte>((value >> 8) & 0xff));
	bytes.push_back(static_cast<Byte>((value >> 16) & 0xff));
	bytes.push_back(static_cast<Byte>((value >> 24) & 0xff));
}

void Append_F32(Bytes &bytes, float value)
{
	Append_U32(bytes, std::bit_cast<std::uint32_t>(value));
}

void Put_U32(Bytes &bytes, std::size_t offset, std::uint32_t value)
{
	BOOST_REQUIRE(offset <= bytes.size() && bytes.size() - offset >= 4);
	bytes[offset] = static_cast<Byte>(value & 0xff);
	bytes[offset + 1] = static_cast<Byte>((value >> 8) & 0xff);
	bytes[offset + 2] = static_cast<Byte>((value >> 16) & 0xff);
	bytes[offset + 3] = static_cast<Byte>((value >> 24) & 0xff);
}

void Put_F32(Bytes &bytes, std::size_t offset, float value)
{
	Put_U32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void Put_String(Bytes &bytes, std::size_t offset, std::size_t width, std::string_view value)
{
	BOOST_REQUIRE(offset <= bytes.size() && bytes.size() - offset >= width);
	const std::size_t count = value.size() < width ? value.size() : width - 1;
	for (std::size_t index = 0; index < count; ++index)
		bytes[offset + index] = static_cast<Byte>(value[index]);
	bytes[offset + count] = Byte{0};
}

void Put_Color(Bytes &bytes, std::size_t offset, std::uint8_t r, std::uint8_t g,
	std::uint8_t b, std::uint8_t a)
{
	BOOST_REQUIRE(offset <= bytes.size() && bytes.size() - offset >= 4);
	bytes[offset] = static_cast<Byte>(r);
	bytes[offset + 1] = static_cast<Byte>(g);
	bytes[offset + 2] = static_cast<Byte>(b);
	bytes[offset + 3] = static_cast<Byte>(a);
}

void Append_Color_Record(Bytes &bytes, float time, std::uint8_t r, std::uint8_t g,
	std::uint8_t b, std::uint8_t a)
{
	Append_F32(bytes, time);
	bytes.push_back(static_cast<Byte>(r));
	bytes.push_back(static_cast<Byte>(g));
	bytes.push_back(static_cast<Byte>(b));
	bytes.push_back(static_cast<Byte>(a));
}

void Append_Float_Record(Bytes &bytes, float time, float value)
{
	Append_F32(bytes, time);
	Append_F32(bytes, value);
}

void Append_Chunk(Bytes &bytes, std::uint32_t id, const Bytes &payload, bool contains_children = false)
{
	Append_U32(bytes, id);
	Append_U32(bytes, static_cast<std::uint32_t>(payload.size()) |
		(contains_children ? 0x80000000u : 0));
	bytes.insert(bytes.end(), payload.begin(), payload.end());
}

Bytes Make_Header(std::uint32_t version)
{
	Bytes bytes(HeaderSize, Byte{0});
	Put_U32(bytes, 0, version);
	Put_String(bytes, 4, 16, "EmitterFixture");
	return bytes;
}

Bytes Make_User_Data()
{
	Bytes bytes(12, Byte{0});
	Put_U32(bytes, 0, 7);
	Put_U32(bytes, 4, 6);
	const std::string_view value = "fixture";
	for (std::size_t index = 0; index != 6; ++index)
		bytes.push_back(static_cast<Byte>(value[index]));
	return bytes;
}

Bytes Make_Info()
{
	Bytes bytes(InfoSize, Byte{0});
	Put_String(bytes, 0, 260, "textures\\emitters\\fixture.tga");
	Put_F32(bytes, 260, 1.25f);
	Put_F32(bytes, 264, 4.5f);
	Put_F32(bytes, 268, 2.5f);
	Put_F32(bytes, 272, 33.0f);
	Put_F32(bytes, 276, 90.0f);
	Put_F32(bytes, 280, 2.75f);
	Put_F32(bytes, 284, 12.0f);
	Put_F32(bytes, 288, 0.75f);
	Put_F32(bytes, 292, -8.0f);
	Put_F32(bytes, 296, 0.4f);
	Put_F32(bytes, 300, 10.0f);
	Put_F32(bytes, 304, 20.0f);
	Put_F32(bytes, 308, 30.0f);
	Put_F32(bytes, 312, -1.0f);
	Put_F32(bytes, 316, -2.0f);
	Put_F32(bytes, 320, -3.0f);
	Put_Color(bytes, 324, 32, 64, 96, 128);
	Put_Color(bytes, 328, 192, 160, 128, 64);
	return bytes;
}

void Put_Randomizer(Bytes &bytes, std::size_t offset, std::uint32_t class_id,
	float value1, float value2, float value3)
{
	Put_U32(bytes, offset, class_id);
	Put_F32(bytes, offset + 4, value1);
	Put_F32(bytes, offset + 8, value2);
	Put_F32(bytes, offset + 12, value3);
}

Bytes Make_Info_V2(std::uint32_t creation_class = 0, std::uint32_t velocity_class = 3)
{
	Bytes bytes(InfoV2Size, Byte{0});
	Put_U32(bytes, 0, 5);
	Put_Randomizer(bytes, 4, creation_class, 1.0f, 2.0f, 3.0f);
	Put_Randomizer(bytes, 36, velocity_class, 4.0f, 5.0f, 6.0f);
	Put_F32(bytes, 68, 7.5f);
	Put_F32(bytes, 72, 0.25f);

	// Depth LEQUAL, depth writes, destination source alpha, primary add,
	// secondary enabled, source inverse alpha, texturing, detail scale,
	// detail alpha inverse scale, alpha test, and post detail functions.
	bytes[76] = static_cast<Byte>(3);
	bytes[77] = static_cast<Byte>(1);
	bytes[79] = static_cast<Byte>(4);
	bytes[81] = static_cast<Byte>(2);
	bytes[82] = static_cast<Byte>(1);
	bytes[83] = static_cast<Byte>(3);
	bytes[84] = static_cast<Byte>(1);
	bytes[85] = static_cast<Byte>(2);
	bytes[86] = static_cast<Byte>(3);
	bytes[88] = static_cast<Byte>(1);
	bytes[89] = static_cast<Byte>(11);
	bytes[90] = static_cast<Byte>(2);
	Put_U32(bytes, 92, 4);
	Put_U32(bytes, 96, 3);
	return bytes;
}

Bytes Make_Properties()
{
	Bytes bytes(40, Byte{0});
	Put_U32(bytes, 0, 2);
	Put_U32(bytes, 4, 2);
	Put_U32(bytes, 8, 3);
	Put_Color(bytes, 12, 8, 16, 24, 32);
	Put_F32(bytes, 16, 0.125f);
	Put_F32(bytes, 20, 0.75f);
	Append_Color_Record(bytes, 0.0f, 16, 32, 48, 64);
	Append_Color_Record(bytes, 0.25f, 80, 96, 112, 128);
	Append_Float_Record(bytes, 0.0f, 0.5f);
	Append_Float_Record(bytes, 0.5f, 0.25f);
	Append_Float_Record(bytes, 0.0f, 1.0f);
	Append_Float_Record(bytes, 0.25f, 2.0f);
	Append_Float_Record(bytes, 0.75f, 3.0f);
	return bytes;
}

Bytes Make_Optional_Tracks()
{
	Bytes bytes;
	Bytes rotation(RotationHeaderSize, Byte{0});
	Put_U32(rotation, 0, 2);
	Put_F32(rotation, 4, 0.125f);
	Put_F32(rotation, 8, 0.5f);
	Append_Float_Record(rotation, 0.0f, 1.0f);
	Append_Float_Record(rotation, 0.25f, 2.0f);
	Append_Float_Record(rotation, 0.75f, 3.0f);
	Append_Chunk(bytes, W3DChunkEmitterRotationKeyframes, rotation);

	Bytes frame(FrameHeaderSize, Byte{0});
	Put_U32(frame, 0, 1);
	Put_F32(frame, 4, 0.5f);
	Append_Float_Record(frame, 0.0f, 0.0f);
	Append_Float_Record(frame, 0.5f, 2.0f);
	Append_Chunk(bytes, W3DChunkEmitterFrameKeyframes, frame);

	Bytes blur(BlurHeaderSize, Byte{0});
	Put_U32(blur, 0, 1);
	Put_F32(blur, 4, 0.25f);
	Append_Float_Record(blur, 0.0f, 0.1f);
	Append_Float_Record(blur, 0.5f, 0.6f);
	Append_Chunk(bytes, W3DChunkEmitterBlurTimeKeyframes, blur);
	return bytes;
}

Bytes Make_Line_Properties()
{
	Bytes bytes(LinePropertiesSize, Byte{0});
	Put_U32(bytes, 0, 0x0200000Fu);
	Put_U32(bytes, 4, 3);
	Put_F32(bytes, 8, 0.2f);
	Put_F32(bytes, 12, 0.3f);
	Put_F32(bytes, 16, 4.0f);
	Put_F32(bytes, 20, 1.5f);
	Put_F32(bytes, 24, -0.5f);
	return bytes;
}

Bytes Make_Emitter(std::uint32_t version = 0x00020000, bool include_version_two = true,
	std::uint32_t creation_class = 0, std::uint32_t velocity_class = 3)
{
	Bytes payload;
	Append_Chunk(payload, W3DChunkEmitterInfo, Make_Info());
	Append_Chunk(payload, W3DChunkEmitterHeader, Make_Header(version));
	Append_Chunk(payload, W3DChunkEmitterUserData, Make_User_Data());
	if (include_version_two) {
		Append_Chunk(payload, W3DChunkEmitterInfoV2, Make_Info_V2(creation_class, velocity_class));
		Append_Chunk(payload, W3DChunkEmitterProperties, Make_Properties());
	}
	Append_Chunk(payload, W3DChunkEmitterLineProperties, Make_Line_Properties());
	const Bytes optional = Make_Optional_Tracks();
	payload.insert(payload.end(), optional.begin(), optional.end());
	Bytes extra(ExtraInfoSize, Byte{0});
	Put_F32(extra, 0, 1.75f);
	Append_Chunk(payload, W3DChunkEmitterExtraInfo, extra);
	Bytes unknown{Byte{0x2a}};
	Append_Chunk(payload, 0x00000999, unknown);

	Bytes root;
	Append_Chunk(root, W3DChunkEmitter, payload, true);
	return root;
}

std::size_t Find_Chunk_Payload(const Bytes &bytes, std::uint32_t id, unsigned occurrence = 0)
{
	if (bytes.size() < 8)
		return bytes.size();
	std::uint32_t first_id = 0;
	if (!Assets::W3D::W3DRead_U32(std::span(bytes), 0, first_id))
		return bytes.size();
	std::size_t offset = 0;
	std::size_t end = bytes.size();
	if (first_id == W3DChunkEmitter) {
		std::uint32_t encoded_size = 0;
		if (!Assets::W3D::W3DRead_U32(std::span(bytes), 4, encoded_size))
			return bytes.size();
		offset = 8;
		end = 8 + (encoded_size & 0x7fffffffu);
	}
	while (offset + 8 <= end) {
		std::uint32_t chunk_id = 0;
		std::uint32_t encoded_size = 0;
		if (!Assets::W3D::W3DRead_U32(std::span(bytes), offset, chunk_id) ||
			!Assets::W3D::W3DRead_U32(std::span(bytes), offset + 4, encoded_size))
			return bytes.size();
		const std::size_t payload_size = encoded_size & 0x7fffffffu;
		if (payload_size > end - offset - 8)
			return bytes.size();
		if (chunk_id == id) {
			if (occurrence == 0)
				return offset + 8;
			--occurrence;
		}
		offset += 8 + payload_size;
	}
	return bytes.size();
}

void Append_To_Root(Bytes &bytes, const Bytes &chunk)
{
	BOOST_REQUIRE(bytes.size() >= 8);
	std::uint32_t root_size = 0;
	BOOST_REQUIRE(Assets::W3D::W3DRead_U32(std::span(bytes), 4, root_size));
	bytes.insert(bytes.end(), chunk.begin(), chunk.end());
	Put_U32(bytes, 4, (root_size & 0x80000000u) |
		(static_cast<std::uint32_t>(bytes.size() - 8)));
}

void Remove_From_Root(Bytes &bytes, std::uint32_t id)
{
	BOOST_REQUIRE(bytes.size() >= 8);
	std::uint32_t root_size = 0;
	BOOST_REQUIRE(Assets::W3D::W3DRead_U32(std::span(bytes), 4, root_size));
	const std::size_t payload_end = 8 + (root_size & 0x7fffffffu);
	std::size_t offset = 8;
	while (offset + 8 <= payload_end) {
		std::uint32_t chunk_id = 0;
		std::uint32_t encoded_size = 0;
		BOOST_REQUIRE(Assets::W3D::W3DRead_U32(std::span(bytes), offset, chunk_id));
		BOOST_REQUIRE(Assets::W3D::W3DRead_U32(std::span(bytes), offset + 4, encoded_size));
		const std::size_t payload_size = encoded_size & 0x7fffffffu;
		if (chunk_id == id) {
			bytes.erase(bytes.begin() + offset, bytes.begin() + offset + 8 + payload_size);
			Put_U32(bytes, 4, static_cast<std::uint32_t>(bytes.size() - 8) | 0x80000000u);
			return;
		}
		offset += 8 + payload_size;
	}
	BOOST_FAIL("fixture chunk was not found");
}

Bytes Read_File(const std::filesystem::path &path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file)
		return {};
	std::vector<char> raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	Bytes bytes;
	bytes.reserve(raw.size());
	for (const char value : raw)
		bytes.push_back(static_cast<Byte>(static_cast<unsigned char>(value)));
	return bytes;
}

}

BOOST_AUTO_TEST_CASE(emitter_decodes_semantic_version_two_description)
{
	for (std::uint32_t creation_class = 0; creation_class != 4; ++creation_class) {
		const auto bytes = Make_Emitter(0x00020000, true, creation_class, (creation_class + 1) % 4);
		Assets::EmitterAssetDesc result;
		std::string error;
		BOOST_REQUIRE_MESSAGE(W3DRead_Emitter(bytes, result, error), error);
		BOOST_CHECK_EQUAL(result.name, "EmitterFixture");
		BOOST_CHECK_EQUAL(result.texture_name, "fixture.tga");
		BOOST_CHECK_CLOSE(result.lifetime, 2.5f, 0.001f);
		BOOST_CHECK_CLOSE(result.emission_rate, 33.0f, 0.001f);
		BOOST_CHECK_CLOSE(result.max_emissions, 90.0f, 0.001f);
		BOOST_CHECK_CLOSE(result.velocity.z, 30.0f, 0.001f);
		BOOST_CHECK_CLOSE(result.acceleration.x, -1.0f, 0.001f);
		BOOST_CHECK_CLOSE(result.gravity, -8.0f, 0.001f);
		BOOST_CHECK_EQUAL(result.burst_size, 5u);
		BOOST_CHECK(result.creation_volume.kind ==
			static_cast<Assets::EmitterRandomizerKind>(creation_class + 1));
		BOOST_CHECK_CLOSE(result.creation_volume.dimensions.x, 1.0f, 0.001f);
		BOOST_CHECK(result.velocity_random.kind ==
			static_cast<Assets::EmitterRandomizerKind>(((creation_class + 1) % 4) + 1));
		BOOST_CHECK_CLOSE(result.outward_velocity, 7.5f, 0.001f);
		BOOST_CHECK_CLOSE(result.velocity_inheritance, 0.25f, 0.001f);
		BOOST_CHECK(result.shader.depth_write);
		BOOST_CHECK(result.shader.depth_compare == Assets::EmitterDepthCompare::LessEqual);
		BOOST_CHECK(result.shader.destination_blend == Assets::EmitterBlendFactor::SourceAlpha);
		BOOST_CHECK(result.shader.source_blend == Assets::EmitterBlendFactor::OneMinusSourceAlpha);
		BOOST_CHECK(result.shader.primary_gradient == Assets::EmitterPrimaryGradient::Add);
		BOOST_CHECK(result.shader.secondary_gradient == Assets::EmitterSecondaryGradient::Enabled);
		BOOST_CHECK(result.shader.detail_color_function == Assets::EmitterDetailColorFunction::Scale);
		BOOST_CHECK(result.shader.detail_alpha_function == Assets::EmitterDetailAlphaFunction::InverseScale);
		BOOST_CHECK(result.shader.alpha_test);
		BOOST_CHECK(result.geometry_mode == Assets::EmitterGeometryMode::LineGroupPrism);
		BOOST_CHECK_EQUAL(result.atlas.columns, 8u);
		BOOST_CHECK_EQUAL(result.atlas.rows, 8u);
		BOOST_CHECK(result.texture_blend_policy == Assets::EmitterTextureBlendPolicy::Authored);

		BOOST_REQUIRE_EQUAL(result.color.keys.size(), 1u);
		BOOST_CHECK_CLOSE(result.color.start.r, 16.0f / 255.0f, 0.001f);
		BOOST_CHECK_CLOSE(result.color.start.a, 64.0f / 255.0f, 0.001f);
		BOOST_CHECK_CLOSE(result.color.random.a, 32.0f / 255.0f, 0.001f);
		BOOST_CHECK_CLOSE(result.color.keys[0].time, 0.25f, 0.001f);
		BOOST_REQUIRE_EQUAL(result.opacity.keys.size(), 1u);
		BOOST_CHECK_CLOSE(result.opacity.random, 0.125f, 0.001f);
		BOOST_REQUIRE_EQUAL(result.size.keys.size(), 2u);
		BOOST_CHECK_CLOSE(result.size.keys[1].value, 3.0f, 0.001f);
		BOOST_REQUIRE_EQUAL(result.rotation.keys.size(), 2u);
		BOOST_CHECK_CLOSE(result.rotation.random, 0.125f, 0.001f);
		BOOST_CHECK_CLOSE(result.initial_orientation_random, 0.5f, 0.001f);
		BOOST_REQUIRE_EQUAL(result.frame.keys.size(), 1u);
		BOOST_CHECK_CLOSE(result.frame.keys[0].value, 2.0f, 0.001f);
		BOOST_REQUIRE_EQUAL(result.blur_time.keys.size(), 1u);
		BOOST_CHECK_CLOSE(result.blur_time.keys[0].value, 0.6f, 0.001f);
		BOOST_CHECK(result.line_properties.texture_mapping == Assets::EmitterLineTextureMapping::Tiled);
		BOOST_CHECK(result.line_properties.merge_intersections);
		BOOST_CHECK(result.line_properties.freeze_random);
		BOOST_CHECK(result.line_properties.disable_sorting);
		BOOST_CHECK(result.line_properties.end_caps);
		BOOST_CHECK_EQUAL(result.line_properties.subdivision_level, 3u);
		BOOST_CHECK_CLOSE(result.line_properties.uv_offset_rate.y, -0.5f, 0.001f);
		BOOST_CHECK_CLOSE(result.future_start_time, 1.75f, 0.001f);

		const auto copy = result;
		result = {};
		BOOST_CHECK_EQUAL(copy.name, "EmitterFixture");
		BOOST_CHECK_EQUAL(copy.color.keys.size(), 1u);
	}
}

BOOST_AUTO_TEST_CASE(emitter_converts_version_one_defaults_and_texture_policy)
{
	const auto bytes = Make_Emitter(0x00010000, false);
	Assets::EmitterAssetDesc result;
	std::string error;
	BOOST_REQUIRE_MESSAGE(W3DRead_Emitter(bytes, result, error), error);
	BOOST_CHECK_EQUAL(result.burst_size, 1u);
	BOOST_CHECK(result.creation_volume.kind == Assets::EmitterRandomizerKind::SolidBox);
	BOOST_CHECK_CLOSE(result.creation_volume.dimensions.x, 0.012f, 0.001f);
	BOOST_CHECK_CLOSE(result.creation_volume.dimensions.y, 0.012f, 0.001f);
	BOOST_CHECK_CLOSE(result.velocity_random.dimensions.z, 2.75f, 0.001f);
	BOOST_CHECK_CLOSE(result.outward_velocity, 0.0f, 0.001f);
	BOOST_CHECK_CLOSE(result.velocity_inheritance, 0.0f, 0.001f);
	BOOST_CHECK(result.texture_blend_policy == Assets::EmitterTextureBlendPolicy::AlphaSpriteWhenTextureHasAlpha);
	BOOST_CHECK(result.geometry_mode == Assets::EmitterGeometryMode::SpriteTriangles);
	BOOST_CHECK_EQUAL(result.atlas.columns, 1u);
	BOOST_CHECK_EQUAL(result.atlas.rows, 1u);
	BOOST_CHECK(result.shader.source_blend == Assets::EmitterBlendFactor::One);
	BOOST_CHECK(result.shader.destination_blend == Assets::EmitterBlendFactor::One);
	BOOST_REQUIRE_EQUAL(result.color.keys.size(), 1u);
	BOOST_CHECK_CLOSE(result.color.start.g, 64.0f / 255.0f, 0.001f);
	BOOST_CHECK_CLOSE(result.color.keys[0].value.b, 128.0f / 255.0f, 0.001f);
	BOOST_REQUIRE_EQUAL(result.opacity.keys.size(), 1u);
	BOOST_CHECK_CLOSE(result.opacity.start, 128.0f / 255.0f, 0.001f);
	BOOST_CHECK_CLOSE(result.opacity.keys[0].value, 64.0f / 255.0f, 0.001f);
	BOOST_CHECK_CLOSE(result.size.start, 1.25f, 0.001f);
	BOOST_CHECK(result.size.keys.empty());
}

BOOST_AUTO_TEST_CASE(emitter_rejects_truncation_without_partial_publication)
{
	const auto bytes = Make_Emitter();
	for (std::size_t size = 0; size < bytes.size(); ++size) {
		Assets::EmitterAssetDesc result;
		result.name = "unchanged";
		result.texture_name = "sentinel.tga";
		std::string error = "previous";
		BOOST_CHECK(!W3DRead_Emitter(std::span(bytes).first(size), result, error));
		BOOST_CHECK_EQUAL(result.name, "unchanged");
		BOOST_CHECK_EQUAL(result.texture_name, "sentinel.tga");
	}
}

BOOST_AUTO_TEST_CASE(emitter_rejects_invalid_counts_modes_and_numeric_values)
{
	Assets::EmitterAssetDesc result;
	result.name = "unchanged";
	std::string error;

	auto duplicate_header = Make_Emitter();
	Append_To_Root(duplicate_header, [&] {
		Bytes chunk;
		Append_Chunk(chunk, W3DChunkEmitterHeader, Make_Header(0x00020000));
		return chunk;
	}());
	BOOST_CHECK(!W3DRead_Emitter(duplicate_header, result, error));
	BOOST_CHECK_EQUAL(result.name, "unchanged");

	auto bad_props = Make_Emitter();
	const std::size_t props = Find_Chunk_Payload(bad_props, W3DChunkEmitterProperties);
	BOOST_REQUIRE(props < bad_props.size());
	Put_U32(bad_props, props, 0xffffffffu);
	BOOST_CHECK(!W3DRead_Emitter(bad_props, result, error));
	BOOST_CHECK_EQUAL(result.name, "unchanged");

	auto bad_float = Make_Emitter();
	const std::size_t info = Find_Chunk_Payload(bad_float, W3DChunkEmitterInfo);
	BOOST_REQUIRE(info < bad_float.size());
	Put_F32(bad_float, info + 260, std::numeric_limits<float>::quiet_NaN());
	BOOST_CHECK(!W3DRead_Emitter(bad_float, result, error));

	auto bad_randomizer = Make_Emitter();
	const std::size_t info_v2 = Find_Chunk_Payload(bad_randomizer, W3DChunkEmitterInfoV2);
	BOOST_REQUIRE(info_v2 < bad_randomizer.size());
	Put_U32(bad_randomizer, info_v2 + 4, 99);
	BOOST_CHECK(!W3DRead_Emitter(bad_randomizer, result, error));

	auto bad_shader = Make_Emitter();
	const std::size_t shader_info = Find_Chunk_Payload(bad_shader, W3DChunkEmitterInfoV2);
	BOOST_REQUIRE(shader_info < bad_shader.size());
	bad_shader[shader_info + 76 + 3] = static_cast<Byte>(99);
	BOOST_CHECK(!W3DRead_Emitter(bad_shader, result, error));

	auto bad_mode = Make_Emitter();
	const std::size_t mode_info = Find_Chunk_Payload(bad_mode, W3DChunkEmitterInfoV2);
	BOOST_REQUIRE(mode_info < bad_mode.size());
	Put_U32(bad_mode, mode_info + 92, 99);
	BOOST_CHECK(!W3DRead_Emitter(bad_mode, result, error));

	auto bad_line = Make_Emitter();
	const std::size_t line = Find_Chunk_Payload(bad_line, W3DChunkEmitterLineProperties);
	BOOST_REQUIRE(line < bad_line.size());
	Put_U32(bad_line, line, 0xff000000u);
	BOOST_CHECK(!W3DRead_Emitter(bad_line, result, error));

	auto duplicate_rotation = Make_Emitter();
	Bytes rotation;
	Append_Chunk(rotation, W3DChunkEmitterRotationKeyframes,
		Bytes(RotationHeaderSize + KeyframeSize, Byte{0}));
	Append_To_Root(duplicate_rotation, rotation);
	BOOST_CHECK(!W3DRead_Emitter(duplicate_rotation, result, error));

	auto missing_props = Make_Emitter();
	Remove_From_Root(missing_props, W3DChunkEmitterProperties);
	BOOST_CHECK(!W3DRead_Emitter(missing_props, result, error));
	BOOST_CHECK_EQUAL(result.name, "unchanged");
}

BOOST_AUTO_TEST_CASE(emitter_reads_optional_retail_corpus_when_available)
{
	const char *configured = std::getenv("GENERALS_W3D_EMITTER_DIRECTORY");
	if (configured == nullptr || configured[0] == '\0') {
		BOOST_TEST_MESSAGE("retail emitter corpus skipped; GENERALS_W3D_EMITTER_DIRECTORY is unset");
		return;
	}
	const std::filesystem::path directory = configured;
	BOOST_REQUIRE_MESSAGE(std::filesystem::is_directory(directory), directory.string());

	for (const std::string_view name : {"EXHydrant.w3d", "EXDrtExp01.w3d", "EXLeafFall.w3d"}) {
		const auto path = directory / name;
		BOOST_REQUIRE_MESSAGE(std::filesystem::is_regular_file(path), path.string());
		const auto bytes = Read_File(path);
		Assets::EmitterAssetDesc result;
		std::string error;
		BOOST_REQUIRE_MESSAGE(!bytes.empty(), path.string());
		BOOST_REQUIRE_MESSAGE(W3DRead_Emitter(bytes, result, error), path.string() << ": " << error);
		BOOST_CHECK(!result.name.empty());
		BOOST_CHECK(!result.texture_name.empty());
		BOOST_CHECK(!result.color.keys.empty() || result.color.start_time == 0.0f);
		BOOST_CHECK(!result.opacity.keys.empty() || result.opacity.start_time == 0.0f);
		BOOST_CHECK(!result.size.keys.empty() || result.size.start_time == 0.0f);
		if (name == "EXHydrant.w3d") {
			BOOST_CHECK(result.geometry_mode == Assets::EmitterGeometryMode::SpriteQuads);
			BOOST_CHECK_EQUAL(result.atlas.columns, 1u);
			BOOST_CHECK_EQUAL(result.atlas.rows, 1u);
			BOOST_CHECK_EQUAL(result.burst_size, 5u);
			BOOST_CHECK_CLOSE(result.lifetime, 1.5f, 0.001f);
			BOOST_CHECK_CLOSE(result.velocity.z, 80.0f, 0.001f);
			BOOST_CHECK_CLOSE(result.acceleration.z, -100.0f, 0.001f);
		}
	}
}
