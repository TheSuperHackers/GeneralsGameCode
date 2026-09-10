module;

#define BOOST_TEST_MODULE W3DLightTests

#include <boost/test/included/unit_test.hpp>

#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <vector>

export module Assets.Tests.Adapters.W3D.Light;

import Assets.Adapters.W3D.Light;

namespace
{

using Byte = std::byte;

void Put_U32(std::vector<Byte> &bytes, std::size_t offset, std::uint32_t value)
{
	bytes[offset] = Byte(value & 0xffu);
	bytes[offset + 1] = Byte((value >> 8) & 0xffu);
	bytes[offset + 2] = Byte((value >> 16) & 0xffu);
	bytes[offset + 3] = Byte((value >> 24) & 0xffu);
}

void Put_F32(std::vector<Byte> &bytes, std::size_t offset, float value)
{
	Put_U32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void Append_U32(std::vector<Byte> &bytes, std::uint32_t value)
{
	bytes.push_back(Byte(value & 0xffu));
	bytes.push_back(Byte((value >> 8) & 0xffu));
	bytes.push_back(Byte((value >> 16) & 0xffu));
	bytes.push_back(Byte((value >> 24) & 0xffu));
}

void Append_Chunk(std::vector<Byte> &bytes, std::uint32_t id,
	std::span<const Byte> payload, bool contains_children = false)
{
	Append_U32(bytes, id);
	Append_U32(bytes, static_cast<std::uint32_t>(payload.size())
		| (contains_children ? 0x80000000u : 0u));
	bytes.insert(bytes.end(), payload.begin(), payload.end());
}

std::uint32_t Read_U32(const std::vector<Byte> &bytes, std::size_t offset)
{
	return std::to_integer<std::uint32_t>(bytes[offset])
		| (std::to_integer<std::uint32_t>(bytes[offset + 1]) << 8)
		| (std::to_integer<std::uint32_t>(bytes[offset + 2]) << 16)
		| (std::to_integer<std::uint32_t>(bytes[offset + 3]) << 24);
}

std::vector<Byte> Info()
{
	// Independent literal W3dLightStruct layout: 24 bytes.
	std::vector<Byte> bytes(24, Byte{});
	Put_U32(bytes, 0, 0x00000103u);
	Put_U32(bytes, 4, 0xA5A5F00Du);
	bytes[8] = Byte{17};
	bytes[9] = Byte{129};
	bytes[10] = Byte{241};
	bytes[12] = Byte{63};
	bytes[13] = Byte{127};
	bytes[14] = Byte{191};
	bytes[16] = Byte{191};
	bytes[17] = Byte{127};
	bytes[18] = Byte{63};
	Put_F32(bytes, 20, 1.25f);
	return bytes;
}

std::vector<Byte> Spot()
{
	// Independent literal W3dSpotLightStruct layout: 20 bytes.
	std::vector<Byte> bytes(20, Byte{});
	Put_F32(bytes, 0, -0.25f);
	Put_F32(bytes, 4, 0.5f);
	Put_F32(bytes, 8, -1.0f);
	Put_F32(bytes, 12, 0.75f);
	Put_F32(bytes, 16, 2.5f);
	return bytes;
}

std::vector<Byte> Attenuation(float start, float end)
{
	// Independent literal W3dLightAttenuationStruct layout: two float32s.
	std::vector<Byte> bytes(8, Byte{});
	Put_F32(bytes, 0, start);
	Put_F32(bytes, 4, end);
	return bytes;
}

std::vector<Byte> Children(bool include_optional = true)
{
	std::vector<Byte> bytes;
	const std::vector<Byte> unknown{Byte{0x44}, Byte{0x33}, Byte{0x22}};
	Append_Chunk(bytes, 0xDEADBEEFu, unknown);
	const auto info = Info();
	Append_Chunk(bytes, 0x00000461u, info);
	if (include_optional) {
		const auto spot = Spot();
		Append_Chunk(bytes, 0x00000462u, spot);
		const auto near_attenuation = Attenuation(2.0f, 5.0f);
		Append_Chunk(bytes, 0x00000463u, near_attenuation);
		const auto far_attenuation = Attenuation(10.0f, 20.0f);
		Append_Chunk(bytes, 0x00000464u, far_attenuation);
	}
	return bytes;
}

std::vector<Byte> Root(std::vector<Byte> children)
{
	std::vector<Byte> bytes;
	Append_Chunk(bytes, 0x00000460u, children, true);
	return bytes;
}

}

BOOST_AUTO_TEST_CASE(light_decoder_reads_literal_wire_offsets_and_ignores_unknown_chunks)
{
	const auto bytes = Root(Children());
	Assets::LightAssetDesc result;
	Assets::W3D::W3DLightDecodeMetadata metadata;
	std::string error;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DRead_Light(bytes, result, error, &metadata), error);
	BOOST_CHECK(error.empty());
	BOOST_CHECK(metadata.spot_info_present);
	BOOST_CHECK(result.type == Assets::LightType::Spot);
	BOOST_CHECK(result.cast_shadows);
	BOOST_CHECK_CLOSE(result.intensity, 1.25f, 0.001);
	BOOST_CHECK_CLOSE(result.ambient.x, 17.0f / 255.0f, 0.001);
	BOOST_CHECK_CLOSE(result.ambient.y, 129.0f / 255.0f, 0.001);
	BOOST_CHECK_CLOSE(result.diffuse.z, 191.0f / 255.0f, 0.001);
	BOOST_CHECK_CLOSE(result.specular.x, 191.0f / 255.0f, 0.001);
	BOOST_CHECK_CLOSE(result.spot_direction.x, -0.25f, 0.001);
	BOOST_CHECK_CLOSE(result.spot_direction.z, -1.0f, 0.001);
	BOOST_CHECK_CLOSE(result.spot_angle, 0.75f, 0.001);
	BOOST_CHECK_CLOSE(result.spot_exponent, 2.5f, 0.001);
	BOOST_CHECK(result.near_attenuation_enabled);
	BOOST_CHECK_CLOSE(result.near_attenuation_start, 2.0f, 0.001);
	BOOST_CHECK_CLOSE(result.near_attenuation_end, 5.0f, 0.001);
	BOOST_CHECK(result.far_attenuation_enabled);
	BOOST_CHECK_CLOSE(result.far_attenuation_start, 10.0f, 0.001);
	BOOST_CHECK_CLOSE(result.far_attenuation_end, 20.0f, 0.001);
}

BOOST_AUTO_TEST_CASE(light_decoder_accepts_unwrapped_children_and_preserves_defaults)
{
	const auto bytes = Children(false);
	Assets::LightAssetDesc result;
	Assets::W3D::W3DLightDecodeMetadata metadata;
	std::string error;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DRead_Light(bytes, result, error, &metadata), error);
	BOOST_CHECK(!metadata.spot_info_present);
	BOOST_CHECK(result.type == Assets::LightType::Spot);
	BOOST_CHECK(!result.near_attenuation_enabled);
	BOOST_CHECK_EQUAL(result.near_attenuation_start, 0.0f);
	BOOST_CHECK_EQUAL(result.near_attenuation_end, 0.0f);
	BOOST_CHECK(!result.far_attenuation_enabled);
	BOOST_CHECK_EQUAL(result.far_attenuation_start, 50.0f);
	BOOST_CHECK_EQUAL(result.far_attenuation_end, 100.0f);
	BOOST_CHECK_EQUAL(result.spot_direction.x, 0.0f);
	BOOST_CHECK_EQUAL(result.spot_direction.z, 1.0f);
}

BOOST_AUTO_TEST_CASE(light_decoder_preserves_seeded_optional_values_when_chunks_are_absent)
{
	const auto bytes = Children(false);
	Assets::LightAssetDesc result;
	result.near_attenuation_enabled = true;
	result.near_attenuation_start = 3.0f;
	result.near_attenuation_end = 4.0f;
	result.far_attenuation_enabled = true;
	result.far_attenuation_start = 30.0f;
	result.far_attenuation_end = 40.0f;
	result.spot_direction = {1.0f, 2.0f, 3.0f};
	result.spot_angle = 0.25f;
	result.spot_exponent = 4.0f;
	std::string error;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DRead_Light(bytes, result, error), error);
	BOOST_CHECK(result.near_attenuation_enabled);
	BOOST_CHECK_EQUAL(result.near_attenuation_start, 3.0f);
	BOOST_CHECK_EQUAL(result.near_attenuation_end, 4.0f);
	BOOST_CHECK(result.far_attenuation_enabled);
	BOOST_CHECK_EQUAL(result.far_attenuation_start, 30.0f);
	BOOST_CHECK_EQUAL(result.far_attenuation_end, 40.0f);
	BOOST_CHECK_EQUAL(result.spot_direction.x, 1.0f);
	BOOST_CHECK_EQUAL(result.spot_direction.y, 2.0f);
	BOOST_CHECK_EQUAL(result.spot_direction.z, 3.0f);
	BOOST_CHECK_EQUAL(result.spot_angle, 0.25f);
	BOOST_CHECK_EQUAL(result.spot_exponent, 4.0f);
}

BOOST_AUTO_TEST_CASE(light_decoder_is_atomic_for_every_truncated_prefix)
{
	const auto bytes = Root(Children());
	for (std::size_t size = 0; size < bytes.size(); ++size) {
		Assets::LightAssetDesc result;
		result.type = Assets::LightType::Directional;
		result.intensity = 7.0f;
		result.ambient = {9.0f, 8.0f, 7.0f};
		result.far_attenuation_enabled = true;
		result.far_attenuation_end = 123.0f;
		Assets::W3D::W3DLightDecodeMetadata metadata;
		metadata.spot_info_present = true;
		std::string error = "stale";
		BOOST_CHECK(!Assets::W3D::W3DRead_Light(
			std::span<const Byte>(bytes).first(size), result, error, &metadata));
		BOOST_CHECK(result.type == Assets::LightType::Directional);
		BOOST_CHECK_EQUAL(result.intensity, 7.0f);
		BOOST_CHECK_EQUAL(result.ambient.x, 9.0f);
		BOOST_CHECK(result.far_attenuation_enabled);
		BOOST_CHECK_EQUAL(result.far_attenuation_end, 123.0f);
		BOOST_CHECK(metadata.spot_info_present);
		BOOST_CHECK(!error.empty());
	}
}

BOOST_AUTO_TEST_CASE(light_decoder_rejects_invalid_known_records_atomically)
{
	auto bytes = Root(Children());
	Assets::LightAssetDesc result;
	result.type = Assets::LightType::Directional;
	result.intensity = 6.0f;
	std::string error;

	// The info record starts at the root's child header (8), followed by the
	// unknown extension chunk (11 bytes), then the 0x461 header (8).
	const std::size_t info_payload = 8u + 11u + 8u;
	Put_F32(bytes, info_payload + 20u, std::numeric_limits<float>::infinity());
	BOOST_CHECK(!Assets::W3D::W3DRead_Light(bytes, result, error));
	BOOST_CHECK(result.type == Assets::LightType::Directional);
	BOOST_CHECK_EQUAL(result.intensity, 6.0f);

	bytes = Children(false);
	// Keep the preceding chunks valid and give the known spot child a
	// one-byte-short payload so its own fixed-record check is exercised.
	const auto spot = Spot();
	Append_Chunk(bytes, 0x00000462u,
		std::span<const Byte>(spot).first(19u));
	result.intensity = 6.0f;
	error.clear();
	BOOST_CHECK(!Assets::W3D::W3DRead_Light(bytes, result, error));
	BOOST_CHECK_EQUAL(result.intensity, 6.0f);
}

BOOST_AUTO_TEST_CASE(light_decoder_rejects_unsupported_type_without_publishing)
{
	auto bytes = Root(Children(false));
	const std::size_t info_payload = 8u + 11u + 8u;
	Put_U32(bytes, info_payload, 0x00000009u);
	Assets::LightAssetDesc result;
	result.type = Assets::LightType::Directional;
	std::string error;
	BOOST_CHECK(!Assets::W3D::W3DRead_Light(bytes, result, error));
	BOOST_CHECK(result.type == Assets::LightType::Directional);
	BOOST_CHECK(!error.empty());
}

BOOST_AUTO_TEST_CASE(light_encoder_writes_literal_wire_layout_with_truncating_colors)
{
	Assets::LightAssetDesc source;
	source.type = Assets::LightType::Spot;
	source.cast_shadows = true;
	source.intensity = 1.25f;
	source.ambient = {0.5f, 0.25f, 0.75f};
	source.diffuse = {0.25f, 0.5f, 0.75f};
	source.specular = {0.75f, 0.5f, 0.25f};
	source.spot_direction = {-0.25f, 0.5f, -1.0f};
	source.spot_angle = 0.75f;
	source.spot_exponent = 2.5f;
	source.near_attenuation_enabled = false;
	source.far_attenuation_enabled = true;
	source.far_attenuation_start = 10.0f;
	source.far_attenuation_end = 20.0f;

	std::vector<Byte> bytes;
	std::string error;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DWrite_Light(source, bytes, error), error);
	BOOST_REQUIRE_EQUAL(Read_U32(bytes, 0), 0x00000460u);
	BOOST_CHECK((Read_U32(bytes, 4) & 0x80000000u) != 0);
	BOOST_REQUIRE_EQUAL(Read_U32(bytes, 8), 0x00000461u);
	BOOST_REQUIRE_EQUAL(Read_U32(bytes, 12), 24u);
	BOOST_CHECK_EQUAL(Read_U32(bytes, 16), 0x00000103u);
	// Root header (8), info header (8), then W3dLightStruct's RGB fields.
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[24]), 127u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[25]), 63u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[26]), 191u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[27]), 0u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[28]), 63u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[29]), 127u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[30]), 191u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[31]), 0u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[32]), 191u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[33]), 127u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[34]), 63u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[35]), 0u);
	BOOST_REQUIRE_EQUAL(Read_U32(bytes, 40), 0x00000462u);
	BOOST_REQUIRE_EQUAL(Read_U32(bytes, 44), 20u);
	// The far record follows the 20-byte spot record.
	const std::size_t far_header = 8u + 8u + 24u + 8u + 20u;
	BOOST_REQUIRE_EQUAL(Read_U32(bytes, far_header), 0x00000464u);
	BOOST_REQUIRE_EQUAL(Read_U32(bytes, far_header + 4u), 8u);
	BOOST_CHECK(error.empty());
}

BOOST_AUTO_TEST_CASE(light_encoder_preserves_color_boundaries_without_saturation_or_rounding)
{
	Assets::LightAssetDesc source;
	source.ambient = {0.0f, 1.0f, 0.5f};
	source.diffuse = {std::nextafter(2.0f / 255.0f, 0.0f), 0.0f, 1.0f};
	std::vector<Byte> bytes;
	std::string error;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DWrite_Light(source, bytes, error), error);
	// Info payload begins at byte 16, and the three padded RGB records begin
	// at payload offsets 8, 12, and 16 respectively.
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[24]), 0u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[25]), 255u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[26]), 127u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[28]), 1u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[29]), 0u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[30]), 255u);
}

BOOST_AUTO_TEST_CASE(light_encoder_respects_near_enabled_and_roundtrips)
{
	Assets::LightAssetDesc source;
	source.type = Assets::LightType::Point;
	std::vector<Byte> bytes;
	std::string error;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DWrite_Light(source, bytes, error), error);
	// The native defaults have no near/far flags and a point light has no spot
	// record, so the complete root contains only the 24-byte info record.
	BOOST_CHECK_EQUAL(bytes.size(), 40u);

	source.near_attenuation_enabled = true;
	source.near_attenuation_start = 2.0f;
	source.near_attenuation_end = 5.0f;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DWrite_Light(source, bytes, error), error);
	Assets::LightAssetDesc decoded;
	Assets::W3D::W3DLightDecodeMetadata metadata;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DRead_Light(bytes, decoded, error, &metadata), error);
	BOOST_CHECK(decoded.type == Assets::LightType::Point);
	BOOST_CHECK(decoded.near_attenuation_enabled);
	BOOST_CHECK_EQUAL(decoded.near_attenuation_start, 2.0f);
	BOOST_CHECK_EQUAL(decoded.near_attenuation_end, 5.0f);
	BOOST_CHECK(!metadata.spot_info_present);
	// A point light does not receive a spot child even if its seed carries
	// spot values.
	BOOST_CHECK_EQUAL(Read_U32(bytes, 8u + 8u + 24u), 0x00000463u);
}

BOOST_AUTO_TEST_CASE(light_encoder_rejects_invalid_values_without_replacing_output)
{
	Assets::LightAssetDesc source;
	source.intensity = std::numeric_limits<float>::quiet_NaN();
	std::vector<Byte> bytes{Byte{0x42}, Byte{0x24}};
	const auto retained = bytes;
	std::string error;
	BOOST_CHECK(!Assets::W3D::W3DWrite_Light(source, bytes, error));
	BOOST_CHECK(bytes == retained);
	BOOST_CHECK(!error.empty());
}
