module;

#define BOOST_TEST_MODULE AssetsW3DSphereTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

export module Assets.Tests.Adapters.W3D.Sphere;

import Assets.Adapters.W3D.Chunks;
import Assets.Math;
import Assets.Spheres;
import Assets.Adapters.W3D.Sphere;

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

void Store_Vector3(std::vector<Byte> &bytes, std::size_t offset, float x, float y, float z)
{
	Store_F32(bytes, offset, x);
	Store_F32(bytes, offset + 4, y);
	Store_F32(bytes, offset + 8, z);
}

void Store_Fixed(std::vector<Byte> &bytes, std::size_t offset, std::size_t length,
	std::string value)
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
	Append_U32(bytes, static_cast<std::uint32_t>(payload.size())
		| (contains_children ? Assets::W3D::W3DChunkContainsChildren : 0u));
	bytes.insert(bytes.end(), payload.begin(), payload.end());
}

void Append_Micro(std::vector<Byte> &bytes, const std::vector<Byte> &payload,
	std::uint8_t id = 1)
{
	bytes.push_back(static_cast<Byte>(id));
	bytes.push_back(static_cast<Byte>(payload.size()));
	bytes.insert(bytes.end(), payload.begin(), payload.end());
}

std::vector<Byte> Make_Definition()
{
	std::vector<Byte> definition(164, Byte{0});
	Store_U32(definition, 0, 0x02000001u);
	Store_U32(definition, 4, Assets::SphereAttributeUseAlphaVector
		| Assets::SphereAttributeCameraAligned | Assets::SphereAttributeInverseAlpha
		| Assets::SphereAttributeAnimationLoop);
	Store_Fixed(definition, 8, 32, "TestSphere");
	Store_Vector3(definition, 40, 1.25f, -2.5f, 3.75f);
	Store_Vector3(definition, 52, 4.0f, 5.0f, 6.0f);
	Store_F32(definition, 64, 2.5f);
	Store_Vector3(definition, 68, 0.125f, 0.375f, 0.875f);
	Store_F32(definition, 80, 0.625f);
	Store_Vector3(definition, 84, 0.75f, 1.25f, 1.5f);
	Store_F32(definition, 96, 0.0f);
	Store_F32(definition, 100, 0.25f);
	Store_F32(definition, 104, 0.5f);
	Store_F32(definition, 108, 0.75f);
	Store_F32(definition, 112, 1.25f);
	Store_Fixed(definition, 116, 32, "Textures\\Sphere.tga");
	const std::uint8_t shader[16] = {
		7, 1, 1, 6, 3, 5, 1, 2, 1, 12, 3, 0, 1, 11, 3, 0};
	for (std::size_t index = 0; index < 16; ++index)
		definition[148 + index] = static_cast<Byte>(shader[index]);
	return definition;
}

std::vector<Byte> Make_Color_Channel()
{
	std::vector<Byte> variables;
	std::vector<Byte> first(16, Byte{0});
	Store_Vector3(first, 0, 0.125f, 0.25f, 0.5f);
	Store_F32(first, 12, -0.25f);
	Append_Micro(variables, first);
	std::vector<Byte> second(16, Byte{0});
	Store_Vector3(second, 0, 0.875f, 0.625f, 0.375f);
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

std::vector<Byte> Make_Scale_Channel()
{
	std::vector<Byte> variables;
	std::vector<Byte> first(16, Byte{0});
	Store_Vector3(first, 0, 0.5f, 0.75f, 1.0f);
	Store_F32(first, 12, -1.0f);
	Append_Micro(variables, first);
	std::vector<Byte> second(16, Byte{0});
	Store_Vector3(second, 0, 1.25f, 1.5f, 1.75f);
	Store_F32(second, 12, 3.0f);
	Append_Micro(variables, second);
	std::vector<Byte> channel;
	Append_Chunk(channel, 0x03150809u, variables, false);
	return channel;
}

std::vector<Byte> Make_Vector_Channel()
{
	std::vector<Byte> variables;
	std::vector<Byte> first(24, Byte{0});
	Store_Vector3(first, 0, 0.0f, 0.0f, 0.0f);
	Store_F32(first, 12, 1.0f);
	Store_F32(first, 16, 0.75f);
	Store_F32(first, 20, 0.0f);
	Append_Micro(variables, first);
	std::vector<Byte> second(24, Byte{0});
	Store_Vector3(second, 0, 0.0f, 0.0f, 0.70710677f);
	Store_F32(second, 12, 0.70710677f);
	Store_F32(second, 16, 1.25f);
	Store_F32(second, 20, 1.0f);
	Append_Micro(variables, second);
	std::vector<Byte> channel;
	Append_Chunk(channel, 0x03150809u, variables, false);
	return channel;
}

std::vector<Byte> Make_Sphere(bool with_root = true)
{
	std::vector<Byte> children;
	Append_Chunk(children, 1, Make_Definition(), false);
	Append_Chunk(children, 2, Make_Color_Channel(), true);
	Append_Chunk(children, 3, Make_Alpha_Channel(), true);
	Append_Chunk(children, 4, Make_Scale_Channel(), true);
	Append_Chunk(children, 5, Make_Vector_Channel(), true);
	if (!with_root)
		return children;
	std::vector<Byte> root;
	Append_Chunk(root, Assets::W3D::W3DChunkSphere, children, true);
	return root;
}

}

BOOST_AUTO_TEST_CASE(valid_sphere_decodes_semantics_and_all_channels)
{
	std::vector<Byte> bytes = Make_Sphere();
	Assets::SphereAssetDesc result;
	std::string error;
	BOOST_REQUIRE(Assets::W3D::W3DRead_Sphere(bytes, result, error));
	BOOST_CHECK(error.empty());
	BOOST_CHECK(result.version == 0x02000001u);
	BOOST_CHECK(result.name == "TestSphere");
	BOOST_CHECK(result.texture_name == "Textures\\Sphere.tga");
	BOOST_CHECK(result.center.x == 1.25f);
	BOOST_CHECK(result.center.y == -2.5f);
	BOOST_CHECK(result.center.z == 3.75f);
	BOOST_CHECK(result.extent.z == 6.0f);
	BOOST_CHECK(result.animation_duration == 2.5f);
	BOOST_CHECK(result.default_color.x == 0.125f);
	BOOST_CHECK(result.default_color.y == 0.375f);
	BOOST_CHECK(result.default_color.z == 0.875f);
	BOOST_CHECK(result.default_alpha == 0.625f);
	BOOST_CHECK(result.default_scale.x == 0.75f);
	BOOST_CHECK(result.default_scale.z == 1.5f);
	BOOST_CHECK(result.default_vector_rotation[2] == 0.5f);
	BOOST_CHECK(result.default_vector_intensity == 1.25f);

	const Assets::SphereMaterialDesc &material = result.material;
	BOOST_CHECK(material.depth_compare == Assets::SphereDepthCompare::Always);
	BOOST_CHECK(material.depth_write);
	BOOST_CHECK(material.color_write);
	BOOST_CHECK(material.destination_blend == Assets::SphereBlendFactor::SourceColorPreFog);
	BOOST_CHECK(material.fog == Assets::SphereFogMode::Disabled);
	BOOST_CHECK(material.primary_gradient == Assets::SpherePrimaryGradient::Modulate2X);
	BOOST_CHECK(material.secondary_gradient == Assets::SphereSecondaryGradient::Enabled);
	BOOST_CHECK(material.source_blend == Assets::SphereBlendFactor::SourceAlpha);
	BOOST_CHECK(material.texturing);
	BOOST_CHECK(material.detail_color_function == Assets::SphereDetailColorFunction::ModulateAlphaAddColor);
	BOOST_CHECK(material.detail_alpha_function == Assets::SphereDetailAlphaFunction::InverseScale);
	BOOST_CHECK(material.alpha_test);
	BOOST_CHECK(material.post_detail_color_function == Assets::SphereDetailColorFunction::ModulateAlphaAddColor);
	BOOST_CHECK(material.post_detail_alpha_function == Assets::SphereDetailAlphaFunction::InverseScale);

	BOOST_REQUIRE_EQUAL(result.color_track.keys.size(), 2u);
	BOOST_CHECK(result.color_track.keys[0].time == -0.25f);
	BOOST_CHECK(result.color_track.keys[0].value.y == 0.25f);
	BOOST_CHECK(result.color_track.keys[1].time == 1.75f);
	BOOST_REQUIRE_EQUAL(result.alpha_track.keys.size(), 2u);
	BOOST_CHECK(result.alpha_track.keys[0].value == 0.2f);
	BOOST_CHECK(result.alpha_track.keys[1].time == 1.0f);
	BOOST_REQUIRE_EQUAL(result.scale_track.keys.size(), 2u);
	BOOST_CHECK(result.scale_track.keys[0].value.x == 0.5f);
	BOOST_CHECK(result.scale_track.keys[1].value.z == 1.75f);
	BOOST_REQUIRE_EQUAL(result.vector_track.keys.size(), 2u);
	BOOST_CHECK(result.vector_track.keys[0].intensity == 0.75f);
	BOOST_CHECK(result.vector_track.keys[1].rotation[2] == 0.70710677f);
}

BOOST_AUTO_TEST_CASE(result_owns_source_without_a_root_wrapper)
{
	std::vector<Byte> bytes = Make_Sphere(false);
	Assets::SphereAssetDesc result;
	std::string error;
	BOOST_REQUIRE(Assets::W3D::W3DRead_Sphere(bytes, result, error));
	bytes.clear();
	BOOST_CHECK(result.name == "TestSphere");
	BOOST_CHECK(result.vector_track.keys.size() == 2u);
}

BOOST_AUTO_TEST_CASE(truncation_and_invalid_values_fail_atomically)
{
	const std::vector<Byte> original = Make_Sphere();
	for (std::size_t size = 0; size < original.size(); ++size) {
		Assets::SphereAssetDesc sentinel;
		sentinel.name = "unchanged";
		std::string error = "stale";
		BOOST_CHECK(!Assets::W3D::W3DRead_Sphere(
			std::span<const Byte>(original.data(), size), sentinel, error));
		BOOST_CHECK(sentinel.name == "unchanged");
		BOOST_CHECK(!error.empty());
	}

	std::vector<Byte> invalid = original;
	// Root header (8) plus definition header (8), then shader offset 148.
	invalid[16 + 148] = static_cast<Byte>(8);
	Assets::SphereAssetDesc sentinel;
	sentinel.name = "unchanged";
	std::string error;
	BOOST_CHECK(!Assets::W3D::W3DRead_Sphere(invalid, sentinel, error));
	BOOST_CHECK(sentinel.name == "unchanged");

	invalid = original;
	// The second vector key time is at root 8 + definition 172 + channel
	// header 8 + variables header 8 + first micro 26 + key time offset 20.
	const std::size_t second_vector_time = 8 + 8 + 164 + 8 + 8 + 2 + 24 + 2 + 20;
	BOOST_REQUIRE(second_vector_time + sizeof(float) <= invalid.size());
	Store_F32(invalid, second_vector_time, 0.0f);
	sentinel.name = "unchanged";
	BOOST_CHECK(!Assets::W3D::W3DRead_Sphere(invalid, sentinel, error));
	BOOST_CHECK(sentinel.name == "unchanged");
}

BOOST_AUTO_TEST_CASE(source_and_destination_blends_use_their_wire_specific_encodings)
{
	const std::array<Assets::SphereBlendFactor, 7> destination{{
		Assets::SphereBlendFactor::Zero,
		Assets::SphereBlendFactor::One,
		Assets::SphereBlendFactor::SourceColor,
		Assets::SphereBlendFactor::InverseSourceColor,
		Assets::SphereBlendFactor::SourceAlpha,
		Assets::SphereBlendFactor::InverseSourceAlpha,
		Assets::SphereBlendFactor::SourceColorPreFog}};
	const std::array<Assets::SphereBlendFactor, 4> source{{
		Assets::SphereBlendFactor::Zero,
		Assets::SphereBlendFactor::One,
		Assets::SphereBlendFactor::SourceAlpha,
		Assets::SphereBlendFactor::InverseSourceAlpha}};

	for (std::uint8_t destination_value = 0; destination_value < destination.size(); ++destination_value) {
		for (std::uint8_t source_value = 0; source_value < source.size(); ++source_value) {
			std::vector<Byte> bytes = Make_Sphere();
			bytes[16 + 148 + 3] = static_cast<Byte>(destination_value);
			bytes[16 + 148 + 7] = static_cast<Byte>(source_value);
			Assets::SphereAssetDesc result;
			std::string error;
			BOOST_REQUIRE(Assets::W3D::W3DRead_Sphere(bytes, result, error));
			BOOST_CHECK(result.material.destination_blend == destination[destination_value]);
			BOOST_CHECK(result.material.source_blend == source[source_value]);
		}
	}

	std::vector<Byte> invalid = Make_Sphere();
	invalid[16 + 148 + 7] = static_cast<Byte>(4);
	Assets::SphereAssetDesc result;
	std::string error;
	BOOST_CHECK(!Assets::W3D::W3DRead_Sphere(invalid, result, error));
}

BOOST_AUTO_TEST_CASE(loader_keeps_effective_context_for_obsolete_shader_fields)
{
	std::vector<Byte> bytes = Make_Sphere();
	// Definition starts at root offset 8 + child header 8. Set obsolete wire
	// fields to unrelated values while changing the effective detail fields.
	const std::size_t shader = 16 + 148;
	bytes[shader + 2] = static_cast<Byte>(0);    // obsolete color mask
	bytes[shader + 4] = static_cast<Byte>(255);  // ignored fog byte
	bytes[shader + 9] = static_cast<Byte>(4);    // effective post color
	bytes[shader + 10] = static_cast<Byte>(2);   // effective post alpha
	bytes[shader + 11] = static_cast<Byte>(255); // ignored shader preset
	bytes[shader + 13] = static_cast<Byte>(255); // ignored raw post color
	bytes[shader + 14] = static_cast<Byte>(255); // ignored raw post alpha
	bytes[shader + 15] = static_cast<Byte>(255); // ignored padding
	bytes[shader + 8] = static_cast<Byte>(0);   // texturing bit follows name

	Assets::SphereAssetDesc result;
	std::string error;
	BOOST_REQUIRE(Assets::W3D::W3DRead_Sphere(bytes, result, error));
	BOOST_CHECK(result.material.color_write);
	BOOST_CHECK(result.material.fog == Assets::SphereFogMode::Disabled);
	BOOST_CHECK(result.material.detail_color_function == Assets::SphereDetailColorFunction::Add);
	BOOST_CHECK(result.material.detail_alpha_function == Assets::SphereDetailAlphaFunction::Scale);
	BOOST_CHECK(result.material.post_detail_color_function == Assets::SphereDetailColorFunction::Add);
	BOOST_CHECK(result.material.post_detail_alpha_function == Assets::SphereDetailAlphaFunction::Scale);
	BOOST_CHECK(result.material.texturing);

	// A texture bit without an authored name cannot produce a texture sample.
	Store_Fixed(bytes, 16 + 116, 32, "");
	bytes[shader + 8] = static_cast<Byte>(1);
	BOOST_REQUIRE(Assets::W3D::W3DRead_Sphere(bytes, result, error));
	BOOST_CHECK(!result.material.texturing);
}

BOOST_AUTO_TEST_CASE(malformed_channels_and_duplicate_definitions_are_rejected)
{
	std::vector<Byte> malformed;
	Append_Chunk(malformed, 1, Make_Definition(), false);
	std::vector<Byte> bad_variables{static_cast<Byte>(1), static_cast<Byte>(16), static_cast<Byte>(0)};
	std::vector<Byte> bad_channel;
	Append_Chunk(bad_channel, 0x03150809u, bad_variables, false);
	Append_Chunk(malformed, 2, bad_channel, true);
	Assets::SphereAssetDesc result;
	std::string error;
	BOOST_CHECK(!Assets::W3D::W3DRead_Sphere(malformed, result, error));

	std::vector<Byte> duplicate;
	Append_Chunk(duplicate, 1, Make_Definition(), false);
	Append_Chunk(duplicate, 1, Make_Definition(), false);
	BOOST_CHECK(!Assets::W3D::W3DRead_Sphere(duplicate, result, error));
}
