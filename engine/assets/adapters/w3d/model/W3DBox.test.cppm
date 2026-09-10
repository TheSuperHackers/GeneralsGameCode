module;

#define BOOST_TEST_MODULE W3DBoxTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

export module Assets.Adapters.W3D.Box.Tests;

import Assets.Adapters.W3D.Box;

namespace
{

using Bytes = std::array<std::byte, Assets::W3D::W3DBoxPayloadSize>;

void Put_U32(Bytes &bytes, std::size_t offset, std::uint32_t value)
{
	bytes[offset] = std::byte(value & 0xffu);
	bytes[offset + 1] = std::byte((value >> 8) & 0xffu);
	bytes[offset + 2] = std::byte((value >> 16) & 0xffu);
	bytes[offset + 3] = std::byte((value >> 24) & 0xffu);
}

void Put_F32(Bytes &bytes, std::size_t offset, float value)
{
	Put_U32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

Bytes Box_Bytes(std::uint32_t version = Assets::W3D::W3DBoxCurrentVersion)
{
	Bytes bytes{};
	Put_U32(bytes, 0, version);
	Put_U32(bytes, 4, Assets::W3D::W3DBoxAttributeOriented
		| Assets::W3D::W3DBoxAttributeAligned | (0x2u << Assets::W3D::W3DBoxAttributeCollisionTypeShift));
	const std::string name = "BOUNDINGBOX";
	for (std::size_t index = 0; index < name.size(); ++index)
		bytes[8 + index] = std::byte(static_cast<unsigned char>(name[index]));
	bytes[40] = std::byte{17};
	bytes[41] = std::byte{129};
	bytes[42] = std::byte{241};
	Put_F32(bytes, 44, -4.5f);
	Put_F32(bytes, 48, 2.0f);
	Put_F32(bytes, 52, 7.25f);
	Put_F32(bytes, 56, 1.5f);
	Put_F32(bytes, 60, 3.0f);
	Put_F32(bytes, 64, 0.75f);
	return bytes;
}

}

BOOST_AUTO_TEST_CASE(box_decoder_retains_wire_fields_and_explicit_collision_mapping)
{
	Assets::W3D::W3DBoxDescription result;
	std::string error;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DRead_Box(Box_Bytes(), result, error), error);
	BOOST_CHECK_EQUAL(result.version, Assets::W3D::W3DBoxCurrentVersion);
	BOOST_CHECK(result.Is_Oriented());
	BOOST_CHECK(result.Is_Aligned());
	BOOST_CHECK_EQUAL(result.name, "BOUNDINGBOX");
	BOOST_CHECK_EQUAL(result.color[0], 17);
	BOOST_CHECK_EQUAL(result.color[1], 129);
	BOOST_CHECK_EQUAL(result.color[2], 241);
	BOOST_CHECK_EQUAL(result.Collision_Type(), 4);
	BOOST_CHECK_EQUAL(result.center.x, -4.5f);
	BOOST_CHECK_EQUAL(result.center.y, 2.0f);
	BOOST_CHECK_EQUAL(result.center.z, 7.25f);
	BOOST_CHECK_EQUAL(result.extent.x, 1.5f);
	BOOST_CHECK_EQUAL(result.extent.y, 3.0f);
	BOOST_CHECK_EQUAL(result.extent.z, 0.75f);
}

BOOST_AUTO_TEST_CASE(box_decoder_is_atomic_for_truncation)
{
	Assets::W3D::W3DBoxDescription result;
	result.name = "UNCHANGED";
	result.center = {9, 8, 7};
	std::string error;
	const auto bytes = Box_Bytes();
	for (std::size_t size = 0; size < bytes.size(); ++size) {
		BOOST_CHECK(!Assets::W3D::W3DRead_Box(std::span(bytes).first(size), result, error));
		BOOST_CHECK_EQUAL(result.name, "UNCHANGED");
		BOOST_CHECK_EQUAL(result.center.x, 9);
	}
}

BOOST_AUTO_TEST_CASE(box_decoder_preserves_versions_for_the_fixed_wire_layout)
{
	Assets::W3D::W3DBoxDescription result;
	std::string error;
	const auto bytes = Box_Bytes(0x00020000u);
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DRead_Box(bytes, result, error), error);
	BOOST_CHECK_EQUAL(result.version, 0x00020000u);
	BOOST_CHECK_EQUAL(result.name, "BOUNDINGBOX");
	BOOST_CHECK_EQUAL(result.color[0], 17);
	BOOST_CHECK_EQUAL(result.center.x, -4.5f);
	BOOST_CHECK_EQUAL(result.extent.z, .75f);
}

BOOST_AUTO_TEST_CASE(box_decoder_accepts_the_legacy_empty_name)
{
	auto bytes = Box_Bytes();
	for (std::size_t index = 0; index < Assets::W3D::W3DBoxNameSize; ++index)
		bytes[8 + index] = std::byte{};
	Assets::W3D::W3DBoxDescription result;
	std::string error;
	BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DRead_Box(bytes, result, error), error);
	BOOST_CHECK(result.name.empty());
}
