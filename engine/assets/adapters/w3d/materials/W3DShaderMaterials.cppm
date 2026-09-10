module;

#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

export module Assets.Adapters.W3D.ShaderMaterials;

import Assets.Adapters.W3D.Chunks;

namespace Assets::W3D
{

export enum class W3DShaderPropertyType : std::uint32_t
{
    String = 1,
    Float = 2,
    Vector2 = 3,
    Vector3 = 4,
    Vector4 = 5,
    Integer = 6,
    Boolean = 7
};

// Source data only. Consumers translate recognised properties to engine
// materials; a shader filename is never an instruction to load executable code.
export struct W3DShaderProperty final
{
    std::string name;
    W3DShaderPropertyType type{};
    std::string texture;
    std::array<float, 4> values{};
    std::int32_t integer = 0;
    bool boolean = false;
    std::vector<std::byte> unknown_value;
};

export struct W3DShaderMaterial final
{
    std::uint8_t version = 0;
    std::string shader_name;
    std::uint32_t technique = 0;
    std::vector<W3DShaderProperty> properties;
};

namespace ShaderMaterialDetail
{

bool Read_Counted_String(W3DByteSpan bytes, std::size_t &offset, std::string &value)
{
    std::uint32_t length = 0;
    if (!W3DRead_U32(bytes, offset, length))
        return false;
    offset += 4;
    if (length == 0 || length > bytes.size() - offset || bytes[offset + length - 1] != std::byte{0})
        return false;
    const auto payload = bytes.subspan(offset, length);
    value = W3DRead_String(payload);
    if (value.size() != length - 1)
        return false;
    offset += length;
    return true;
}

bool Read_Property(W3DByteSpan bytes, W3DShaderProperty &property)
{
    std::uint32_t type = 0;
    std::size_t offset = 4;
    if (!W3DRead_U32(bytes, 0, type) || !Read_Counted_String(bytes, offset, property.name) || property.name.empty())
        return false;
    property.type = static_cast<W3DShaderPropertyType>(type);
    const auto value = bytes.subspan(offset);
    switch (property.type) {
    case W3DShaderPropertyType::String:
        return Read_Counted_String(bytes, offset, property.texture) && offset == bytes.size();
    case W3DShaderPropertyType::Float:
    case W3DShaderPropertyType::Vector2:
    case W3DShaderPropertyType::Vector3:
    case W3DShaderPropertyType::Vector4: {
        const std::size_t count = type - 1;
        if (value.size() != count * 4)
            return false;
        for (std::size_t index = 0; index < count; ++index)
            if (!W3DRead_F32(value, index * 4, property.values[index]) || !std::isfinite(property.values[index]))
                return false;
        return true;
    }
    case W3DShaderPropertyType::Integer: {
        std::uint32_t bits = 0;
        if (value.size() != 4 || !W3DRead_U32(value, 0, bits))
            return false;
        property.integer = std::bit_cast<std::int32_t>(bits);
        return true;
    }
    case W3DShaderPropertyType::Boolean:
        if (value.size() != 1 || std::to_integer<unsigned>(value[0]) > 1)
            return false;
        property.boolean = value[0] != std::byte{0};
        return true;
    default:
        // Unknown optional property types remain bounded and available for
        // diagnostics. They must not be mistaken for a supported texture.
        property.unknown_value.assign(value.begin(), value.end());
        return true;
    }
}

}

export bool W3DRead_Shader_Material(W3DByteSpan bytes, W3DShaderMaterial &result)
{
    result = {};
    if (!W3DValidate_Chunk_Tree(bytes))
        return false;
    W3DShaderMaterial parsed;
    bool has_header = false;
    const bool valid = W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
        switch (chunk.id) {
        case W3DChunkShaderMaterialHeader:
            if (has_header || chunk.contains_children || chunk.payload.size() != 37)
                return false;
            parsed.version = std::to_integer<std::uint8_t>(chunk.payload[0]);
            parsed.shader_name = W3DRead_Fixed_String(chunk.payload, 1, 32);
            has_header = !parsed.shader_name.empty() && W3DRead_U32(chunk.payload, 33, parsed.technique);
            return has_header;
        case W3DChunkShaderMaterialProperty: {
            W3DShaderProperty property;
            if (chunk.contains_children || !ShaderMaterialDetail::Read_Property(chunk.payload, property))
                return false;
            for (const auto &existing : parsed.properties)
                if (existing.name == property.name)
                    return false;
            parsed.properties.push_back(std::move(property));
            return true;
        }
        default:
            return true;
        }
    });
    if (!valid || !has_header)
        return false;
    result = std::move(parsed);
    return true;
}

// Payload of W3D_CHUNK_SHADER_MATERIALS, preserving its index order.
export bool W3DRead_Shader_Materials(W3DByteSpan bytes, std::vector<W3DShaderMaterial> &result)
{
    result.clear();
    if (!W3DValidate_Chunk_Tree(bytes))
        return false;
    std::vector<W3DShaderMaterial> parsed;
    const bool valid = W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
        if (chunk.id != W3DChunkShaderMaterial)
            return true;
        W3DShaderMaterial material;
        if (!chunk.contains_children || !W3DRead_Shader_Material(chunk.payload, material))
            return false;
        parsed.push_back(std::move(material));
        return true;
    });
    if (valid)
        result = std::move(parsed);
    return valid;
}

}
