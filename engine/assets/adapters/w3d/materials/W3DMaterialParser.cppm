module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

export module Assets.Adapters.W3D.Materials;

import Assets.Adapters.W3D.Chunks;
export import Assets.Adapters.W3D.ShaderMaterials;
import Assets.Math;
import Assets.Models;
import Assets.Materials.TextureMapping;
import Assets.Adapters.W3D.TextureMapping;

namespace Assets::W3D
{

export inline constexpr std::uint32_t W3DChunkTextureInfo = 0x00000033u;

export inline constexpr std::uint16_t W3DTextureAttributeNoLod = 0x0004u;
export inline constexpr std::uint16_t W3DTextureAttributeClampU = 0x0008u;
export inline constexpr std::uint16_t W3DTextureAttributeClampV = 0x0010u;
export inline constexpr std::uint16_t W3DTextureAttributeMipLevelsMask = 0x00C0u;
export inline constexpr std::uint16_t W3DTextureAttributeMipLevelsAll = 0x0000u;
export inline constexpr std::uint16_t W3DTextureAttributeMipLevels2 = 0x0040u;
export inline constexpr std::uint16_t W3DTextureAttributeMipLevels3 = 0x0080u;
export inline constexpr std::uint16_t W3DTextureAttributeMipLevels4 = 0x00C0u;
export inline constexpr std::uint16_t W3DTextureAttributeTypeMask = 0x1000u;
export inline constexpr std::uint16_t W3DTextureAttributeTypeColorMap = 0x0000u;
export inline constexpr std::uint16_t W3DTextureAttributeTypeBumpMap = 0x1000u;

export struct W3DMaterialPass final
{
	std::uint32_t vertex_material_index = W3DInvalidIndex;
	std::uint32_t shader_index = W3DInvalidIndex;
	std::uint32_t texture_index = W3DInvalidIndex;
	std::vector<Vector2f> texcoords;
	bool uses_shader_material = false;
};

export struct W3DVertexMaterialData final
{
	ModelMaterialDesc material;
	std::array<std::string, 2> mapper_arguments;
	std::array<std::optional<TextureMappingDescription>,2> mappings;
	bool has_name = false;
};

export bool W3DRead_Vertex_Material(W3DByteSpan bytes, W3DVertexMaterialData &result);

export enum class W3DMaterial3MapKind : std::uint8_t
{
	DiffuseColor,
	DiffuseIllumination,
	SpecularColor,
	SpecularIllumination
};

export struct W3DMaterial3MapData final
{
	W3DMaterial3MapKind kind = W3DMaterial3MapKind::DiffuseColor;
	std::string filename;
	std::uint16_t mapping_type = 0;
	std::uint16_t frame_count = 0;
	float frame_rate = 0.0f;
};

export struct W3DMaterial3Data final
{
	ModelMaterialDesc material;
	std::uint32_t attributes = 0;
	float fog_coefficient = 1.0f;
	std::vector<W3DMaterial3MapData> maps;
};

export bool W3DRead_Material3(W3DByteSpan bytes,W3DMaterial3Data& result);
export bool W3DRead_Material3_Container(W3DByteSpan bytes,std::vector<W3DMaterial3Data>& result);

export struct W3DShaderSettings final
{
	std::uint8_t depth_compare = 3;
	std::uint8_t depth_mask = 1;
	std::uint8_t destination_blend = 0;
	std::uint8_t primary_gradient = 1;
	std::uint8_t secondary_gradient = 0;
	std::uint8_t source_blend = 1;
	std::uint8_t texturing = 0;
	std::uint8_t detail_alpha = 0;
	std::uint8_t alpha_test = 0;
	std::uint8_t detail_color = 0;
	std::uint8_t color_mask = 1;
	std::uint8_t fog_function = 0;
	std::uint8_t shader_preset = 0;
	std::uint8_t post_detail_color = 0;
	std::uint8_t post_detail_alpha = 0;
};

export struct W3DTextureData final
{
	std::string name;
	std::uint16_t attributes = 0;
	std::uint16_t animation_type = 0;
	std::uint32_t frame_count = 0;
	float frame_rate = 0;
	bool has_info = false;
};

export bool W3DRead_Texture(W3DByteSpan bytes, W3DTextureData &result);
export bool W3DRead_Shader(W3DByteSpan bytes, W3DShaderSettings &result);
export bool W3DRead_Shaders(W3DByteSpan bytes, std::vector<W3DShaderSettings> &result);

export struct W3DMaterialInfo final
{
	std::uint32_t pass_count = 0;
	std::uint32_t vertex_material_count = 0;
	std::uint32_t shader_count = 0;
	std::uint32_t texture_count = 0;
};

export struct W3DMaterialData final
{
	std::vector<W3DVertexMaterialData> vertex_materials;
	std::vector<W3DTextureData> textures;
	std::vector<W3DShaderSettings> shaders;
	std::vector<W3DShaderMaterial> shader_materials;
	std::vector<W3DMaterialPass> passes;
	W3DMaterialInfo info;
	bool has_info = false;
};

namespace MaterialDetail
{

bool Read_UV_Array(W3DByteSpan bytes, std::uint32_t count, std::vector<Vector2f> &values)
{
	if (count > bytes.size() / 8 || bytes.size() != static_cast<std::size_t>(count) * 8)
		return false;

	values.resize(count);
	for (std::uint32_t index = 0; index < count; ++index) {
		const std::size_t offset = static_cast<std::size_t>(index) * 8;
		if (!W3DRead_F32(bytes, offset, values[index].x) || !W3DRead_F32(bytes, offset + 4, values[index].y))
			return false;
		if (!std::isfinite(values[index].x))
			values[index].x = 0.0f;
		if (!std::isfinite(values[index].y))
			values[index].y = 0.0f;
		// W3D stores V from the opposite origin used by generic model data.
		values[index].y = 1.0f - values[index].y;
	}
	return true;
}

Color4f Read_Material_Color(W3DByteSpan bytes, std::size_t offset) noexcept
{
	return {std::to_integer<unsigned>(bytes[offset]) / 255.0f,
		std::to_integer<unsigned>(bytes[offset + 1]) / 255.0f,
		std::to_integer<unsigned>(bytes[offset + 2]) / 255.0f, 1.0f};
}

bool Parse_Vertex_Material(W3DByteSpan bytes, W3DVertexMaterialData &result)
{
	bool has_info = false;
	std::array<bool, 2> has_arguments{};
	auto &material = result.material;
	const bool valid = W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
		switch (chunk.id) {
			case W3DChunkVertexMaterialName:
				if (result.has_name) return false;
				material.name = W3DRead_String(chunk.payload);
				result.has_name = true;
				return true;
			case 0x2E:
			case 0x2F: {
				const auto stage = chunk.id - 0x2E;
				if (has_arguments[stage]) return false;
				has_arguments[stage] = true;
				result.mapper_arguments[stage] = W3DRead_String(chunk.payload);
				return true;
			}
			case W3DChunkVertexMaterialInfo: {
				if (has_info || chunk.payload.size() < 32)
					return false;
				std::uint32_t attributes = 0;
				if (!W3DRead_U32(chunk.payload, 0, attributes))
					return false;
				material.source_attributes = attributes;
				material.ambient_color = Read_Material_Color(chunk.payload, 4);
				material.base_color = Read_Material_Color(chunk.payload, 8);
				material.specular_color = Read_Material_Color(chunk.payload, 12);
				material.emissive_color = Read_Material_Color(chunk.payload, 16);
				return W3DRead_F32(chunk.payload, 20, material.shininess) &&
					W3DRead_F32(chunk.payload, 24, material.opacity) &&
					W3DRead_F32(chunk.payload, 28, material.translucency) &&
					std::isfinite(material.shininess) && std::isfinite(material.opacity) &&
					std::isfinite(material.translucency) && (has_info = true);
			}
			default:
				return true;
		}
	});
	return valid && has_info;
}

bool Read_U16(W3DByteSpan bytes, std::size_t offset, std::uint16_t &value) noexcept
{
	if (offset > bytes.size() || bytes.size() - offset < sizeof(std::uint16_t))
		return false;
	const auto *data = reinterpret_cast<const std::uint8_t *>(bytes.data() + offset);
	value = static_cast<std::uint16_t>(data[0]) |
		(static_cast<std::uint16_t>(data[1]) << 8);
	return true;
}

bool Read_Null_Terminated_String(W3DByteSpan bytes, std::string &value, bool allow_empty)
{
	if (bytes.empty())
		return false;
	const auto *data = reinterpret_cast<const char *>(bytes.data());
	std::size_t length = 0;
	while (length < bytes.size() && data[length] != '\0')
		++length;
	if (length == bytes.size() || (!allow_empty && length == 0))
		return false;
	value.assign(data, length);
	return true;
}

bool Read_Material3_Map(W3DByteSpan bytes, W3DMaterial3MapData &result)
{
	W3DMaterial3MapData parsed;
	bool has_filename = false;
	bool has_info = false;
	unsigned required_chunk = 0;
	if (!W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
		switch (chunk.id) {
			case W3DChunkMap3Filename:
				if (required_chunk != 0 || chunk.contains_children ||
					!Read_Null_Terminated_String(chunk.payload, parsed.filename, false))
					return false;
				has_filename = true;
				required_chunk = 1;
				return true;
			case W3DChunkMap3Info:
				if (required_chunk != 1 || has_info || chunk.contains_children || chunk.payload.size() < 8 ||
					!Read_U16(chunk.payload, 0, parsed.mapping_type) ||
					!Read_U16(chunk.payload, 2, parsed.frame_count) ||
					!W3DRead_F32(chunk.payload, 4, parsed.frame_rate) ||
					!std::isfinite(parsed.frame_rate))
					return false;
				has_info = true;
				required_chunk = 2;
				return true;
			default:
				return required_chunk == 2;
		}
	}))
		return false;
	if (!has_filename || !has_info)
		return false;
	result = std::move(parsed);
	return true;
}

bool Read_Material3_Record(W3DByteSpan bytes, W3DMaterial3Data &result)
{
	W3DMaterial3Data parsed;
	bool has_name = false;
	bool has_info = false;
	unsigned required_chunk = 0;
	if (!W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
		switch (chunk.id) {
			case W3DChunkMaterial3Name:
				if (required_chunk != 0 || chunk.contains_children ||
					!Read_Null_Terminated_String(chunk.payload, parsed.material.name, true))
					return false;
				has_name = true;
				required_chunk = 1;
				return true;
			case W3DChunkMaterial3Info:
				if (required_chunk != 1 || has_info || chunk.contains_children)
					return false;
				{
					W3DMaterial3Data info;
					// The wire record is 44 bytes; some exporters pad the chunk.
					if (chunk.payload.size() < 44 ||
						!W3DRead_Material3(chunk.payload.first(44), info))
						return false;
					std::string name = std::move(parsed.material.name);
					parsed = std::move(info);
					parsed.material.name = std::move(name);
				}
				has_info = true;
				required_chunk = 2;
				return true;
			case W3DChunkMaterial3DiffuseColorMap:
			case W3DChunkMaterial3DiffuseIlluminationMap:
			case W3DChunkMaterial3SpecularColorMap:
			case W3DChunkMaterial3SpecularIlluminationMap: {
				if (!has_info || !chunk.contains_children)
					return false;
				W3DMaterial3MapData map;
				if (!Read_Material3_Map(chunk.payload, map))
					return false;
				switch (chunk.id) {
					case W3DChunkMaterial3DiffuseColorMap:
						map.kind = W3DMaterial3MapKind::DiffuseColor;
						break;
					case W3DChunkMaterial3DiffuseIlluminationMap:
						map.kind = W3DMaterial3MapKind::DiffuseIllumination;
						break;
					case W3DChunkMaterial3SpecularColorMap:
						map.kind = W3DMaterial3MapKind::SpecularColor;
						break;
					default:
						map.kind = W3DMaterial3MapKind::SpecularIllumination;
						break;
				}
				parsed.maps.push_back(std::move(map));
				return true;
			}
			default:
				return required_chunk == 2;
		}
	}))
		return false;
	if (!has_name || !has_info)
		return false;
	result = std::move(parsed);
	return true;
}

bool Parse_Vertex_Materials(W3DByteSpan bytes, std::vector<W3DVertexMaterialData> &materials)
{
	return W3DVisit_Chunks(bytes, [&materials](const W3DChunkView &chunk) {
		if (chunk.id != W3DChunkVertexMaterial)
			return false;
		W3DVertexMaterialData material;
		if (!chunk.contains_children || !W3DRead_Vertex_Material(chunk.payload, material) || !material.has_name)
			return false;
		materials.push_back(std::move(material));
		return true;
	});
}

bool Parse_Textures(W3DByteSpan bytes, std::vector<W3DTextureData> &textures)
{
	return W3DVisit_Chunks(bytes, [&textures](const W3DChunkView &chunk) {
		if (chunk.id != W3DChunkTexture || !chunk.contains_children)
			return false;
		W3DTextureData texture;
		if (!W3DRead_Texture(chunk.payload, texture))
			return false;
		textures.push_back(std::move(texture));
		return true;
	});
}

bool Parse_Shaders(W3DByteSpan bytes, std::vector<W3DShaderSettings> &shaders)
{
	if (bytes.size() % 16 != 0)
		return false;

	shaders.reserve(shaders.size() + bytes.size() / 16);
	for (std::size_t offset = 0; offset < bytes.size(); offset += 16) {
		W3DShaderSettings shader;
		W3DRead_Shader(bytes.subspan(offset, 16), shader);
		shaders.push_back(shader);
	}
	return true;
}

bool Parse_Texture_Stage(W3DByteSpan bytes, std::uint32_t vertex_count, W3DMaterialPass &pass)
{
	return W3DVisit_Chunks(bytes, [&pass, vertex_count](const W3DChunkView &chunk) {
		switch (chunk.id) {
			case W3DChunkTextureIds:
				if (chunk.payload.size() < 4 || chunk.payload.size() % 4 != 0)
					return false;
				return W3DRead_U32(chunk.payload, 0, pass.texture_index);
			case W3DChunkStageTextureCoords:
				return Read_UV_Array(chunk.payload, vertex_count, pass.texcoords);
			case W3DChunkPerFaceTextureCoordIds:
				return chunk.payload.size() % 12 == 0;
			default:
				return true;
		}
	});
}

bool Parse_Material_Pass(W3DByteSpan bytes, std::uint32_t vertex_count, W3DMaterialPass &pass)
{
	if (!W3DVisit_Chunks(bytes, [&pass](const W3DChunkView &chunk) {
		if (chunk.id == W3DChunkShaderMaterialIds) {
			if (pass.uses_shader_material || chunk.payload.empty() || chunk.payload.size() % 4 != 0)
				return false;
			pass.uses_shader_material = true;
		}
		return true;
	})) return false;
	return W3DVisit_Chunks(bytes, [&pass, vertex_count](const W3DChunkView &chunk) {
		switch (chunk.id) {
			case W3DChunkVertexMaterialIds:
				if (chunk.payload.size() < 4 || chunk.payload.size() % 4 != 0)
					return false;
				return W3DRead_U32(chunk.payload, 0, pass.vertex_material_index);
			case W3DChunkShaderIds:
				if (chunk.payload.size() < 4 || chunk.payload.size() % 4 != 0)
					return false;
				return W3DRead_U32(chunk.payload, 0, pass.shader_index);
			case W3DChunkTextureStage:
				// Full indexed UVs for shader passes are decoded by PassBindings
				// once the mesh's face count is known.
				if (pass.uses_shader_material) return chunk.contains_children;
				return chunk.contains_children && Parse_Texture_Stage(chunk.payload, vertex_count, pass);
			default:
				return true;
		}
	});
}

}

export bool W3DRead_Vertex_Material(W3DByteSpan bytes, W3DVertexMaterialData &result)
{
	result = {};
	W3DVertexMaterialData parsed;
	if (!W3DValidate_Chunk_Tree(bytes) || !MaterialDetail::Parse_Vertex_Material(bytes, parsed))
		return false;
	for (unsigned stage=0;stage<parsed.mappings.size();++stage)
		parsed.mappings[stage]=W3DRead_Texture_Mapping(parsed.material.source_attributes,stage,parsed.mapper_arguments[stage]);
	result = std::move(parsed);
	return true;
}

export bool W3DRead_Material3(W3DByteSpan bytes,W3DMaterial3Data& result)
{
	result = {};
	if (bytes.size() != 44)
		return false;
	W3DMaterial3Data parsed;
	auto &material = parsed.material;
	if (!W3DRead_U32(bytes, 0, parsed.attributes) ||
		!W3DRead_F32(bytes, 28, material.shininess) ||
		!W3DRead_F32(bytes, 32, material.opacity) ||
		!W3DRead_F32(bytes, 36, material.translucency) ||
		!W3DRead_F32(bytes, 40, parsed.fog_coefficient) ||
		!std::isfinite(material.shininess) || !std::isfinite(material.opacity) ||
		!std::isfinite(material.translucency) || !std::isfinite(parsed.fog_coefficient))
		return false;
	material.source_attributes = parsed.attributes;
	const auto diffuse = MaterialDetail::Read_Material_Color(bytes, 4);
	const auto specular = MaterialDetail::Read_Material_Color(bytes, 8);
	const auto diffuse_coefficient = MaterialDetail::Read_Material_Color(bytes, 20);
	const auto specular_coefficient = MaterialDetail::Read_Material_Color(bytes, 24);
	material.base_color = {diffuse.r * diffuse_coefficient.r,
		diffuse.g * diffuse_coefficient.g, diffuse.b * diffuse_coefficient.b, 1.0f};
	material.specular_color = {specular.r * specular_coefficient.r,
		specular.g * specular_coefficient.g, specular.b * specular_coefficient.b, 1.0f};
	material.emissive_color = MaterialDetail::Read_Material_Color(bytes, 12);
	material.ambient_color = MaterialDetail::Read_Material_Color(bytes, 16);
	result = std::move(parsed);
	return true;
}

export bool W3DRead_Material3_Container(W3DByteSpan bytes, std::vector<W3DMaterial3Data> &result)
{
	result = {};
	std::vector<W3DMaterial3Data> parsed;
	if (!W3DValidate_Chunk_Tree(bytes) || !W3DVisit_Chunks(bytes, [&parsed](const W3DChunkView &chunk) {
		if (chunk.id != W3DChunkMaterial3 || !chunk.contains_children)
			return false;
		W3DMaterial3Data material;
		if (!MaterialDetail::Read_Material3_Record(chunk.payload, material))
			return false;
		parsed.push_back(std::move(material));
		return true;
	}))
		return false;
	result = std::move(parsed);
	return true;
}

export bool W3DRead_Texture(W3DByteSpan bytes, W3DTextureData &result)
{
	result = {};
	W3DTextureData parsed;
	bool has_name = false;
	if (!W3DValidate_Chunk_Tree(bytes)) return false;
	const bool valid = W3DVisit_Chunks(bytes, [&](const W3DChunkView &chunk) {
		if (chunk.id == W3DChunkTextureName) {
			if (has_name) return false;
			has_name = true;
			parsed.name = W3DRead_String(chunk.payload);
		} else if (chunk.id == W3DChunkTextureInfo) {
			std::uint32_t flags;
			if (parsed.has_info || chunk.payload.size() < 12
				|| !W3DRead_U32(chunk.payload,0,flags)
				|| !W3DRead_U32(chunk.payload,4,parsed.frame_count)
				|| !W3DRead_F32(chunk.payload,8,parsed.frame_rate)
				|| !std::isfinite(parsed.frame_rate)) return false;
			parsed.attributes = static_cast<std::uint16_t>(flags);
			parsed.animation_type = static_cast<std::uint16_t>(flags >> 16);
			parsed.has_info = true;
		}
		return true;
	});
	if (!valid || parsed.name.empty()) return false;
	result = std::move(parsed);
	return true;
}

export bool W3DRead_Shader(W3DByteSpan bytes, W3DShaderSettings &result)
{
	result = {};
	if (bytes.size() != 16) return false;
	const auto *data = reinterpret_cast<const std::uint8_t *>(bytes.data());
	result = {data[0], data[1], data[3], data[5], data[6], data[7], data[8],
		data[10], data[12], data[9], data[2], data[4], data[11], data[13], data[14]};
	return true;
}

export bool W3DRead_Shaders(W3DByteSpan bytes, std::vector<W3DShaderSettings> &result)
{
	result.clear();
	return MaterialDetail::Parse_Shaders(bytes, result);
}

export bool W3DParse_Materials(W3DByteSpan bytes, std::uint32_t vertex_count, W3DMaterialData &materials)
{
	materials = {};
	if (!W3DValidate_Chunk_Tree(bytes))
		return false;

	bool has_shader_materials = false;
	if (!W3DVisit_Chunks(bytes, [&materials, vertex_count, &has_shader_materials](const W3DChunkView &chunk) {
		switch (chunk.id) {
			case W3DChunkMaterialInfo:
				if (chunk.payload.size() < 16 ||
					!W3DRead_U32(chunk.payload, 0, materials.info.pass_count) ||
					!W3DRead_U32(chunk.payload, 4, materials.info.vertex_material_count) ||
					!W3DRead_U32(chunk.payload, 8, materials.info.shader_count) ||
					!W3DRead_U32(chunk.payload, 12, materials.info.texture_count))
					return false;
				materials.has_info = true;
				return true;
			case W3DChunkVertexMaterials:
				return chunk.contains_children && MaterialDetail::Parse_Vertex_Materials(
					chunk.payload,
					materials.vertex_materials);
			case W3DChunkShaders:
				return MaterialDetail::Parse_Shaders(chunk.payload, materials.shaders);
			case W3DChunkShaderMaterials:
				if (has_shader_materials || !chunk.contains_children)
					return false;
				has_shader_materials = true;
				return W3DRead_Shader_Materials(chunk.payload, materials.shader_materials);
			case W3DChunkTextures:
				return chunk.contains_children && MaterialDetail::Parse_Textures(chunk.payload, materials.textures);
			case W3DChunkMaterialPass: {
				if (!chunk.contains_children)
					return false;
				W3DMaterialPass pass;
				if (!MaterialDetail::Parse_Material_Pass(chunk.payload, vertex_count, pass))
					return false;
				materials.passes.push_back(std::move(pass));
				return true;
			}
			default:
				return true;
		}
	}))
		return false;

	if (materials.has_info &&
		(materials.info.vertex_material_count != materials.vertex_materials.size() ||
			materials.info.texture_count != materials.textures.size() ||
			materials.info.pass_count != materials.passes.size()))
		return false;

	return true;
}

}
