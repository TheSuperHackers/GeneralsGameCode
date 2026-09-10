module;
#define BOOST_TEST_MODULE W3DShaderMaterialTests
#include <boost/test/included/unit_test.hpp>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string_view>
#include <vector>

export module Assets.Tests.W3DShaderMaterials;
import Assets.Adapters.W3D.Chunks;
import Assets.Adapters.W3D.ShaderMaterials;

namespace
{
using Bytes = std::vector<std::byte>;
using namespace Assets::W3D;
void U32(Bytes &bytes, std::uint32_t value)
{
    for (unsigned shift = 0; shift != 32; shift += 8)
        bytes.push_back(static_cast<std::byte>((value >> shift) & 255));
}
void Chunk(Bytes &bytes, std::uint32_t id, const Bytes &payload, bool children = false)
{
    U32(bytes, id); U32(bytes, static_cast<std::uint32_t>(payload.size()) | (children ? 0x80000000u : 0));
    bytes.insert(bytes.end(), payload.begin(), payload.end());
}
void String(Bytes &bytes, std::string_view value)
{
    U32(bytes, static_cast<std::uint32_t>(value.size() + 1));
    for (char c : value) bytes.push_back(static_cast<std::byte>(c));
    bytes.push_back(std::byte{0});
}
Bytes Header(std::uint8_t version = 1)
{
    Bytes bytes(37); bytes[0] = static_cast<std::byte>(version);
    const std::string_view shader = "objectsallied.fx";
    for (std::size_t i = 0; i < shader.size(); ++i) bytes[i + 1] = static_cast<std::byte>(shader[i]);
    bytes[33] = std::byte{2};
    return bytes;
}
void Property(Bytes &material, std::uint32_t type, std::string_view name, const Bytes &value)
{
    Bytes bytes; U32(bytes, type); String(bytes, name);
    bytes.insert(bytes.end(), value.begin(), value.end());
    Chunk(material, W3DChunkShaderMaterialProperty, bytes);
}
Bytes Material()
{
    Bytes bytes; Chunk(bytes, W3DChunkShaderMaterialHeader, Header());
    return bytes;
}
}

BOOST_AUTO_TEST_CASE(decodes_authored_shader_parameters_without_executing_shader_names)
{
    auto bytes = Material();
    Bytes texture; String(texture, "GE_airfield_nrm.dds"); Property(bytes, 1, "NormalMap", texture);
    Bytes scale; U32(scale, std::bit_cast<std::uint32_t>(0.75f)); Property(bytes, 2, "BumpScale", scale);
    Bytes color; for (float f : {0.2f, 0.4f, 0.6f, 1.0f}) U32(color, std::bit_cast<std::uint32_t>(f));
    Property(bytes, 5, "ColorDiffuse", color);
    Bytes integer; U32(integer, 0xFFFFFFFFu); Property(bytes, 6, "SignedSetting", integer);
    Property(bytes, 7, "AlphaTestEnable", {std::byte{1}});
    W3DShaderMaterial result;
    BOOST_REQUIRE(W3DRead_Shader_Material(bytes, result));
    BOOST_TEST(result.shader_name == "objectsallied.fx");
    BOOST_TEST(result.version == 1u);
    BOOST_TEST(result.technique == 2u);
    BOOST_REQUIRE_EQUAL(result.properties.size(), 5u);
    BOOST_TEST(result.properties[0].texture == "GE_airfield_nrm.dds");
    BOOST_TEST(result.properties[1].values[0] == 0.75f);
    BOOST_TEST(result.properties[2].values[2] == 0.6f);
    BOOST_TEST(result.properties[3].integer == -1);
    BOOST_TEST(result.properties[4].boolean);
}

BOOST_AUTO_TEST_CASE(preserves_table_order_empty_tables_and_unknown_optional_properties)
{
    auto first = Material();
    Property(first, 100, "FutureParameter", {std::byte{23}, std::byte{42}});
    Bytes second; Chunk(second, W3DChunkShaderMaterialHeader, Header(2));
    Bytes table; Chunk(table, W3DChunkShaderMaterial, first, true);
    Chunk(table, 0x12345678, {std::byte{19}});
    Chunk(table, W3DChunkShaderMaterial, second, true);
    std::vector<W3DShaderMaterial> result;
    BOOST_REQUIRE(W3DRead_Shader_Materials(table, result));
    BOOST_REQUIRE_EQUAL(result.size(), 2u);
    BOOST_TEST(result[0].version == 1u);
    BOOST_TEST(result[1].version == 2u);
    BOOST_REQUIRE_EQUAL(result[0].properties[0].unknown_value.size(), 2u);
    BOOST_TEST(result[0].properties[0].texture.empty());
    BOOST_REQUIRE(W3DRead_Shader_Materials({}, result));
    BOOST_TEST(result.empty());
}

BOOST_AUTO_TEST_CASE(rejects_truncated_duplicate_and_nonfinite_data_without_partial_publication)
{
    const auto original = Material();
    W3DShaderMaterial result;
    for (std::size_t size = 0; size < original.size(); ++size) {
        result.shader_name = "stale";
        BOOST_CHECK(!W3DRead_Shader_Material(W3DByteSpan(original).first(size), result));
        BOOST_TEST(result.shader_name.empty());
    }
    auto duplicate = original; Chunk(duplicate, W3DChunkShaderMaterialHeader, Header());
    BOOST_CHECK(!W3DRead_Shader_Material(duplicate, result));
    for (float value : {std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN()}) {
        auto bytes = original; Bytes scalar; U32(scalar, std::bit_cast<std::uint32_t>(value));
        Property(bytes, 2, "BumpScale", scalar);
        BOOST_CHECK(!W3DRead_Shader_Material(bytes, result));
        BOOST_TEST(result.properties.empty());
    }
    auto bytes = original;
    Property(bytes, 7, "AlphaTestEnable", {std::byte{2}});
    BOOST_CHECK(!W3DRead_Shader_Material(bytes, result));
    bytes = original;
    Property(bytes, 7, "AlphaTestEnable", {std::byte{1}});
    Property(bytes, 7, "AlphaTestEnable", {std::byte{0}});
    BOOST_CHECK(!W3DRead_Shader_Material(bytes, result));
}

BOOST_AUTO_TEST_CASE(validates_counted_strings_and_vector_lengths)
{
    for (std::uint32_t type = 3; type <= 4; ++type) {
        auto bytes = Material(); Bytes vector;
        for (std::uint32_t i = 0; i < type - 1; ++i) U32(vector, std::bit_cast<std::uint32_t>(float(i + 1)));
        Property(bytes, type, "Vector", vector);
        W3DShaderMaterial result;
        BOOST_REQUIRE(W3DRead_Shader_Material(bytes, result));
        BOOST_TEST(result.properties[0].values[type - 2] == float(type - 1));
        bytes = Material(); vector.pop_back(); Property(bytes, type, "Vector", vector);
        BOOST_CHECK(!W3DRead_Shader_Material(bytes, result));
    }
    for (const Bytes value : {Bytes{std::byte{255},std::byte{255},std::byte{255},std::byte{255}},
        Bytes{std::byte{1},std::byte{0},std::byte{0},std::byte{0},std::byte{'x'}}}) {
        auto bytes = Material(); Property(bytes, 1, "DiffuseTexture", value);
        W3DShaderMaterial result;
        BOOST_CHECK(!W3DRead_Shader_Material(bytes, result));
    }
    Bytes table; Chunk(table, W3DChunkShaderMaterial, Material(), true);
    auto invalid = Material(); Property(invalid, 7, "AlphaTestEnable", {});
    Chunk(table, W3DChunkShaderMaterial, invalid, true);
    std::vector<W3DShaderMaterial> result;
    BOOST_CHECK(!W3DRead_Shader_Materials(table, result));
    BOOST_TEST(result.empty());
}

BOOST_AUTO_TEST_CASE(decodes_external_converted_shader_corpus)
{
    const char *directory = std::getenv("GENERALS_W3D_SHADER_DIRECTORY");
    if (!directory) {
        BOOST_TEST_MESSAGE("Set GENERALS_W3D_SHADER_DIRECTORY for Evolution shader conversion fixtures.");
        return;
    }
    std::size_t files = 0, properties = 0, normal_maps = 0, specular_maps = 0;
    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() != ".bin") continue;
        std::ifstream stream(entry.path(), std::ios::binary | std::ios::ate);
        BOOST_REQUIRE(stream.good());
        const auto size = stream.tellg();
        BOOST_REQUIRE(size > 0);
        Bytes bytes(static_cast<std::size_t>(size)); stream.seekg(0);
        BOOST_REQUIRE(stream.read(reinterpret_cast<char *>(bytes.data()), size).good());
        std::vector<W3DShaderMaterial> materials;
        BOOST_TEST_CONTEXT(entry.path().filename().string()) {
            BOOST_REQUIRE(W3DRead_Shader_Materials(bytes, materials));
            BOOST_REQUIRE_EQUAL(materials.size(), 1u);
            for (const auto &property : materials[0].properties) {
                ++properties;
                if (property.name == "NormalMap") {
                    BOOST_CHECK(property.type == W3DShaderPropertyType::String);
                    BOOST_TEST(!property.texture.empty()); ++normal_maps;
                }
                if (property.name == "SpecMap") {
                    BOOST_CHECK(property.type == W3DShaderPropertyType::String);
                    BOOST_TEST(!property.texture.empty()); ++specular_maps;
                }
            }
            if (entry.path().stem() == "ABAIRFIELD_SKN.USA_AIRFIELD") {
                BOOST_TEST(materials[0].shader_name == "objectsallied.fx");
                bool diffuse = false, normal = false, specular = false;
                for (const auto &property : materials[0].properties) {
                    if (property.name == "DiffuseTexture") diffuse = property.texture == "ABNBUB_AllStructuresTextureAtlas";
                    if (property.name == "NormalMap") normal = property.texture == "ABNBUB_AllStructuresTextureAtlas_NRM";
                    if (property.name == "SpecMap") specular = property.texture == "ABNBUB_AllStructuresTextureAtlas_SPM";
                }
                BOOST_TEST(diffuse); BOOST_TEST(normal); BOOST_TEST(specular);
            }
        }
        ++files;
    }
    BOOST_REQUIRE_GT(files, 0u); BOOST_REQUIRE_GT(normal_maps, 0u); BOOST_REQUIRE_GT(specular_maps, 0u);
    BOOST_TEST_MESSAGE("Decoded " << files << " shader fixtures, " << properties << " properties, "
        << normal_maps << " normal maps and " << specular_maps << " specular maps.");
}
