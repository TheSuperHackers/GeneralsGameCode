module;

#define BOOST_TEST_MODULE AssetsW3DRingTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <string>
#include <vector>

export module Assets.Tests.Adapters.W3D.Ring;

import Assets.Adapters.W3D.Chunks;
import Assets.Rings;
import Assets.Adapters.W3D.Ring;

namespace
{

using Byte = std::byte;

void Append_U32(std::vector<Byte> &bytes, std::uint32_t value)
{
	bytes.push_back(static_cast<Byte>(value & 0xffu));
	bytes.push_back(static_cast<Byte>((value >> 8) & 0xffu));
	bytes.push_back(static_cast<Byte>((value >> 16) & 0xffu));
	bytes.push_back(static_cast<Byte>((value >> 24) & 0xffu));
}

void Store_U32(std::vector<Byte> &bytes, std::size_t offset, std::uint32_t value)
{
	BOOST_REQUIRE(offset <= bytes.size() && bytes.size() - offset >= 4);
	bytes[offset] = static_cast<Byte>(value & 0xffu);
	bytes[offset + 1] = static_cast<Byte>((value >> 8) & 0xffu);
	bytes[offset + 2] = static_cast<Byte>((value >> 16) & 0xffu);
	bytes[offset + 3] = static_cast<Byte>((value >> 24) & 0xffu);
}

void Store_F32(std::vector<Byte> &bytes, std::size_t offset, float value)
{
	Store_U32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void Store_Vector2(std::vector<Byte> &bytes, std::size_t offset, float x, float y)
{
	Store_F32(bytes, offset, x);
	Store_F32(bytes, offset + 4, y);
}

void Store_Vector3(std::vector<Byte> &bytes, std::size_t offset, float x, float y, float z)
{
	Store_F32(bytes, offset, x);
	Store_F32(bytes, offset + 4, y);
	Store_F32(bytes, offset + 8, z);
}

void Store_Fixed(std::vector<Byte> &bytes, std::size_t offset, std::size_t length, std::string value)
{
	BOOST_REQUIRE(offset <= bytes.size() && bytes.size() - offset >= length);
	const std::size_t count = value.size() < length ? value.size() : length - 1;
	for (std::size_t index = 0; index < count; ++index)
		bytes[offset + index] = static_cast<Byte>(value[index]);
	bytes[offset + count] = Byte{0};
}

void Append_Chunk(std::vector<Byte> &bytes, std::uint32_t id,
	const std::vector<Byte> &payload, bool contains_children)
{
	Append_U32(bytes, id);
	Append_U32(bytes, static_cast<std::uint32_t>(payload.size()) |
		(contains_children ? Assets::W3D::W3DChunkContainsChildren : 0u));
	bytes.insert(bytes.end(), payload.begin(), payload.end());
}

void Append_Micro(std::vector<Byte> &bytes, const std::vector<Byte> &payload,
	std::uint8_t id = 1)
{
	BOOST_REQUIRE(payload.size() <= (std::numeric_limits<std::uint8_t>::max)());
	bytes.push_back(static_cast<Byte>(id));
	bytes.push_back(static_cast<Byte>(payload.size()));
	bytes.insert(bytes.end(), payload.begin(), payload.end());
}

std::vector<Byte> Make_Definition()
{
	std::vector<Byte> definition(168, Byte{0});
	Store_U32(definition, 0, 0x02000001u);
	Store_U32(definition, 4, 0x00000003u);
	Store_Fixed(definition, 8, 32, "TestRing");
	Store_Vector3(definition, 40, 1.25f, -2.5f, 3.75f);
	Store_Vector3(definition, 52, 4.0f, 5.0f, 6.0f);
	Store_F32(definition, 64, 2.5f);
	Store_F32(definition, 68, 0.125f);
	Store_F32(definition, 72, 0.375f);
	Store_F32(definition, 76, 0.875f);
	Store_F32(definition, 80, 0.625f);
	Store_Vector2(definition, 84, 0.75f, 1.25f);
	Store_Vector2(definition, 92, 1.5f, 1.75f);
	Store_Vector2(definition, 100, 0.25f, 0.5f);
	Store_Vector2(definition, 108, 3.0f, 4.0f);
	Store_Fixed(definition, 116, 32, "Textures\\Ring.tga");

	const std::uint8_t shader[16] = {
		7, 1, 1, 6, 3, 5, 1, 2, 1, 12, 3, 0, 1, 11, 3, 0};
	for (std::size_t index = 0; index < 16; ++index)
		definition[148 + index] = static_cast<Byte>(shader[index]);
	Store_U32(definition, 164, 9);
	return definition;
}

std::vector<Byte> Make_Color_Channel()
{
	std::vector<Byte> variables;
	std::vector<Byte> first(16, Byte{0});
	Store_F32(first, 0, 0.125f);
	Store_F32(first, 4, 0.25f);
	Store_F32(first, 8, 0.5f);
	Store_F32(first, 12, -0.25f);
	Append_Micro(variables, first);
	std::vector<Byte> second(16, Byte{0});
	Store_F32(second, 0, 0.875f);
	Store_F32(second, 4, 0.625f);
	Store_F32(second, 8, 0.375f);
	Store_F32(second, 12, 1.75f);
	Append_Micro(variables, second);
	std::vector<Byte> channel;
	Append_Chunk(channel, 0x03150809u, variables, false);
	return channel;
}

std::vector<Byte> Make_Alpha_Channel()
{
	std::vector<Byte> variables;
	std::vector<Byte> first(8, Byte{0});
	Store_F32(first, 0, 0.2f);
	Store_F32(first, 4, 0.0f);
	Append_Micro(variables, first);
	std::vector<Byte> second(8, Byte{0});
	Store_F32(second, 0, 0.9f);
	Store_F32(second, 4, 1.0f);
	Append_Micro(variables, second);
	std::vector<Byte> channel;
	Append_Chunk(channel, 0x03150809u, variables, false);
	return channel;
}

std::vector<Byte> Make_Scale_Channel(float first_x, float first_y, float second_x, float second_y)
{
	std::vector<Byte> variables;
	std::vector<Byte> first(12, Byte{0});
	Store_Vector2(first, 0, first_x, first_y);
	Store_F32(first, 8, -1.0f);
	Append_Micro(variables, first);
	std::vector<Byte> second(12, Byte{0});
	Store_Vector2(second, 0, second_x, second_y);
	Store_F32(second, 8, 3.0f);
	Append_Micro(variables, second);
	std::vector<Byte> channel;
	Append_Chunk(channel, 0x03150809u, variables, false);
	return channel;
}

std::vector<Byte> Make_Ring(bool with_root = true)
{
	std::vector<Byte> children;
	Append_Chunk(children, 1, Make_Definition(), false);
	Append_Chunk(children, 2, Make_Color_Channel(), true);
	Append_Chunk(children, 3, Make_Alpha_Channel(), true);
	Append_Chunk(children, 4, Make_Scale_Channel(0.5f, 0.75f, 1.25f, 1.5f), true);
	Append_Chunk(children, 5, Make_Scale_Channel(0.25f, 0.375f, 2.0f, 2.5f), true);
	if (!with_root)
		return children;
	std::vector<Byte> root;
	Append_Chunk(root, Assets::W3D::W3DChunkRing, children, true);
	return root;
}

}

BOOST_AUTO_TEST_CASE(valid_ring_decodes_semantics_and_all_channels)
{
	using namespace Assets;
	using namespace Assets::W3D;
	std::vector<Byte> bytes = Make_Ring();
	Assets::RingAssetDesc result;
	std::string error;
	BOOST_REQUIRE(W3DRead_Ring(bytes, result, error));
	BOOST_CHECK(error.empty());
	BOOST_CHECK(result.name == "TestRing");
	BOOST_CHECK(result.texture_name == "Textures\\Ring.tga");
	BOOST_CHECK(result.center.x == 1.25f);
	BOOST_CHECK(result.center.y == -2.5f);
	BOOST_CHECK(result.center.z == 3.75f);
	BOOST_CHECK(result.extent.x == 4.0f);
	BOOST_CHECK(result.extent.y == 5.0f);
	BOOST_CHECK(result.extent.z == 6.0f);
	BOOST_CHECK(result.animation_duration == 2.5f);
	BOOST_CHECK(result.default_color.r == 0.125f);
	BOOST_CHECK(result.default_color.g == 0.375f);
	BOOST_CHECK(result.default_color.b == 0.875f);
	BOOST_CHECK(result.default_color.a == 1.0f);
	BOOST_CHECK(result.default_alpha == 0.625f);
	BOOST_CHECK(result.default_inner_scale.x == 0.75f);
	BOOST_CHECK(result.default_inner_scale.y == 1.25f);
	BOOST_CHECK(result.default_outer_scale.x == 1.5f);
	BOOST_CHECK(result.default_outer_scale.y == 1.75f);
	BOOST_CHECK(result.inner_extent.x == 0.25f);
	BOOST_CHECK(result.inner_extent.y == 0.5f);
	BOOST_CHECK(result.outer_extent.x == 3.0f);
	BOOST_CHECK(result.outer_extent.y == 4.0f);
	BOOST_CHECK(result.texture_tile_count == 9);
	BOOST_CHECK(result.camera_aligned);
	BOOST_CHECK(result.animation_loop);

	const RingMaterialDesc &material = result.material;
	BOOST_CHECK(material.depth_compare == RingDepthCompare::Always);
	BOOST_CHECK(material.depth_write);
	BOOST_CHECK(material.color_write);
	BOOST_CHECK(material.destination_blend == RingBlendFactor::SourceColorPreFog);
	BOOST_CHECK(material.fog == RingFogMode::Disabled);
	BOOST_CHECK(material.primary_gradient == RingPrimaryGradient::Modulate2X);
	BOOST_CHECK(material.secondary_gradient == RingSecondaryGradient::Enabled);
	BOOST_CHECK(material.source_blend == RingBlendFactor::SourceAlpha);
	BOOST_CHECK(material.texturing);
	BOOST_CHECK(material.detail_color_function == RingDetailColorFunction::Disabled);
	BOOST_CHECK(material.detail_alpha_function == RingDetailAlphaFunction::Disabled);
	BOOST_CHECK(material.alpha_test);
	BOOST_CHECK(material.post_detail_color_function == RingDetailColorFunction::ModulateAlphaAddColor);
	BOOST_CHECK(material.post_detail_alpha_function == RingDetailAlphaFunction::InverseScale);

	BOOST_REQUIRE_EQUAL(result.color_track.keys.size(), 2u);
	BOOST_CHECK(result.color_track.keys[0].time == -0.25f);
	BOOST_CHECK(result.color_track.keys[0].value.r == 0.125f);
	BOOST_CHECK(result.color_track.keys[0].value.g == 0.25f);
	BOOST_CHECK(result.color_track.keys[0].value.b == 0.5f);
	BOOST_CHECK(result.color_track.keys[1].time == 1.75f);
	BOOST_CHECK(result.color_track.keys[1].value.b == 0.375f);
	BOOST_REQUIRE_EQUAL(result.alpha_track.keys.size(), 2u);
	BOOST_CHECK(result.alpha_track.keys[0].time == 0.0f);
	BOOST_CHECK(result.alpha_track.keys[0].value == 0.2f);
	BOOST_CHECK(result.alpha_track.keys[1].time == 1.0f);
	BOOST_CHECK(result.alpha_track.keys[1].value == 0.9f);
	BOOST_REQUIRE_EQUAL(result.inner_scale_track.keys.size(), 2u);
	BOOST_CHECK(result.inner_scale_track.keys[0].value.x == 0.5f);
	BOOST_CHECK(result.inner_scale_track.keys[1].value.y == 1.5f);
	BOOST_REQUIRE_EQUAL(result.outer_scale_track.keys.size(), 2u);
	BOOST_CHECK(result.outer_scale_track.keys[0].value.y == 0.375f);
	BOOST_CHECK(result.outer_scale_track.keys[1].value.x == 2.0f);
}

BOOST_AUTO_TEST_CASE(child_sequence_is_accepted_and_result_owns_source_data)
{
	using namespace Assets::W3D;
	std::vector<Byte> bytes = Make_Ring(false);
	Assets::RingAssetDesc result;
	std::string error;
	BOOST_REQUIRE(W3DRead_Ring(bytes, result, error));
	bytes.clear();
	BOOST_CHECK(result.name == "TestRing");
	BOOST_CHECK(result.texture_name == "Textures\\Ring.tga");
	BOOST_CHECK(result.color_track.keys.size() == 2u);
}

BOOST_AUTO_TEST_CASE(truncation_and_invalid_values_fail_atomically)
{
	using namespace Assets::W3D;
	const std::vector<Byte> original = Make_Ring();
	for (std::size_t size = 0; size < original.size(); ++size) {
		Assets::RingAssetDesc sentinel;
		sentinel.name = "unchanged";
		std::string error = "stale";
		BOOST_CHECK(!W3DRead_Ring(std::span<const Byte>(original.data(), size), sentinel, error));
		BOOST_CHECK(sentinel.name == "unchanged");
		BOOST_CHECK(!error.empty());
	}

	std::vector<Byte> invalid = original;
	// The first shader field is depth compare; eight is outside the semantic enum.
	invalid[16 + 148] = static_cast<Byte>(8);
	Assets::RingAssetDesc sentinel;
	sentinel.name = "unchanged";
	std::string error;
	BOOST_CHECK(!W3DRead_Ring(invalid, sentinel, error));
	BOOST_CHECK(sentinel.name == "unchanged");

	invalid = original;
	// Make the second color key time equal to the first key time.
	Store_F32(invalid, 232, -0.25f);
	sentinel.name = "unchanged";
	BOOST_CHECK(!W3DRead_Ring(invalid, sentinel, error));
	BOOST_CHECK(sentinel.name == "unchanged");
}

BOOST_AUTO_TEST_CASE(malformed_micro_chunks_and_duplicate_definitions_are_rejected)
{
	using namespace Assets::W3D;
	std::vector<Byte> malformed;
	std::vector<Byte> definition = Make_Definition();
	Append_Chunk(malformed, 1, definition, false);
	std::vector<Byte> bad_variables{static_cast<Byte>(1), static_cast<Byte>(16), static_cast<Byte>(0)};
	std::vector<Byte> bad_channel;
	Append_Chunk(bad_channel, 0x03150809u, bad_variables, false);
	Append_Chunk(malformed, 2, bad_channel, true);
	Assets::RingAssetDesc result;
	std::string error;
	BOOST_CHECK(!W3DRead_Ring(malformed, result, error));

	std::vector<Byte> duplicate;
	Append_Chunk(duplicate, 1, definition, false);
	Append_Chunk(duplicate, 1, definition, false);
	BOOST_CHECK(!W3DRead_Ring(duplicate, result, error));
}

BOOST_AUTO_TEST_CASE(source_and_destination_blends_use_their_wire_specific_encodings)
{
	using namespace Assets;
	using namespace Assets::W3D;
	const std::array<RingBlendFactor, 7> destination{{
		RingBlendFactor::Zero,
		RingBlendFactor::One,
		RingBlendFactor::SourceColor,
		RingBlendFactor::InverseSourceColor,
		RingBlendFactor::SourceAlpha,
		RingBlendFactor::InverseSourceAlpha,
		RingBlendFactor::SourceColorPreFog}};
	const std::array<RingBlendFactor, 4> source{{
		RingBlendFactor::Zero,
		RingBlendFactor::One,
		RingBlendFactor::SourceAlpha,
		RingBlendFactor::InverseSourceAlpha}};

	for (std::uint8_t destination_value = 0; destination_value < destination.size(); ++destination_value) {
		for (std::uint8_t source_value = 0; source_value < source.size(); ++source_value) {
			std::vector<Byte> bytes = Make_Ring();
			bytes[16 + 148 + 3] = static_cast<Byte>(destination_value);
			bytes[16 + 148 + 7] = static_cast<Byte>(source_value);
			Assets::RingAssetDesc result;
			std::string error;
			BOOST_REQUIRE(W3DRead_Ring(bytes, result, error));
			BOOST_CHECK(result.material.destination_blend == destination[destination_value]);
			BOOST_CHECK(result.material.source_blend == source[source_value]);
		}
	}

	std::vector<Byte> invalid = Make_Ring();
	invalid[16 + 148 + 7] = static_cast<Byte>(4);
	Assets::RingAssetDesc result;
	std::string error;
	BOOST_CHECK(!W3DRead_Ring(invalid, result, error));
}

BOOST_AUTO_TEST_CASE(obsolete_shader_fields_are_normalized_and_texture_name_controls_sampling)
{
	using namespace Assets::W3D;
	std::vector<Byte> bytes = Make_Ring();
	// ColorMask, FogFunc, and the post-detail bytes are obsolete. The source
	// detail bytes are the effective generic-pass detail functions.
	bytes[16 + 148 + 2] = static_cast<Byte>(0);
	bytes[16 + 148 + 4] = static_cast<Byte>(0xff);
	bytes[16 + 148 + 8] = static_cast<Byte>(0);
	bytes[16 + 148 + 13] = static_cast<Byte>(0xff);
	bytes[16 + 148 + 14] = static_cast<Byte>(0xff);

	Assets::RingAssetDesc result;
	std::string error;
	BOOST_REQUIRE(W3DRead_Ring(bytes, result, error));
	BOOST_CHECK(result.material.color_write);
	BOOST_CHECK(result.material.fog == Assets::RingFogMode::Disabled);
	BOOST_CHECK(result.material.texturing);
	BOOST_CHECK(result.material.detail_color_function == Assets::RingDetailColorFunction::Disabled);
	BOOST_CHECK(result.material.detail_alpha_function == Assets::RingDetailAlphaFunction::Disabled);
	BOOST_CHECK(result.material.post_detail_color_function == Assets::RingDetailColorFunction::ModulateAlphaAddColor);
	BOOST_CHECK(result.material.post_detail_alpha_function == Assets::RingDetailAlphaFunction::InverseScale);

	Store_Fixed(bytes, 16 + 116, 32, "");
	BOOST_REQUIRE(W3DRead_Ring(bytes, result, error));
	BOOST_CHECK(!result.material.texturing);
}
