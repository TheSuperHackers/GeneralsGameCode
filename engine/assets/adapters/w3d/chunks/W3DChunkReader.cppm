module;

#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

export module Assets.Adapters.W3D.Chunks;

import Assets.Math;

namespace Assets::W3D
{

export using W3DByteSpan = std::span<const std::byte>;
export inline constexpr std::size_t W3DNameLength = 16u;

export inline constexpr std::uint32_t W3DChunkMesh = 0x00000000;
export inline constexpr std::uint32_t W3DChunkVertices = 0x00000002;
export inline constexpr std::uint32_t W3DChunkVertexNormals = 0x00000003;
export inline constexpr std::uint32_t W3DChunkSurrenderNormals = 0x00000004;
export inline constexpr std::uint32_t W3DChunkTextureCoords = 0x00000005;
export inline constexpr std::uint32_t W3DChunkLegacyMaterials = 0x00000006;
export inline constexpr std::uint32_t W3DChunkSurrenderTriangles = 0x00000009;
export inline constexpr std::uint32_t W3DChunkMeshUserText = 0x0000000C;
export inline constexpr std::uint32_t W3DChunkVertexColors = 0x0000000D;
export inline constexpr std::uint32_t W3DChunkVertexInfluences = 0x0000000E;
export inline constexpr std::uint32_t W3DChunkDamage = 0x0000000F;
export inline constexpr std::uint32_t W3DChunkLegacyMaterials2 = 0x00000014;
export inline constexpr std::uint32_t W3DChunkMaterials3 = 0x00000015;
export inline constexpr std::uint32_t W3DChunkMaterial3 = 0x00000016;
export inline constexpr std::uint32_t W3DChunkMaterial3Name = 0x00000017;
export inline constexpr std::uint32_t W3DChunkMaterial3Info = 0x00000018;
export inline constexpr std::uint32_t W3DChunkMaterial3DiffuseColorMap = 0x00000019;
export inline constexpr std::uint32_t W3DChunkMap3Filename = 0x0000001A;
export inline constexpr std::uint32_t W3DChunkMap3Info = 0x0000001B;
export inline constexpr std::uint32_t W3DChunkMaterial3DiffuseIlluminationMap = 0x0000001C;
export inline constexpr std::uint32_t W3DChunkMaterial3SpecularColorMap = 0x0000001D;
export inline constexpr std::uint32_t W3DChunkMaterial3SpecularIlluminationMap = 0x0000001E;
export inline constexpr std::uint32_t W3DChunkMeshHeader3 = 0x0000001F;
export inline constexpr std::uint32_t W3DChunkTriangles = 0x00000020;
export inline constexpr std::uint32_t W3DChunkPerTriMaterials = 0x00000021;
export inline constexpr std::uint32_t W3DChunkVertexShadeIndices = 0x00000022;
export inline constexpr std::uint32_t W3DChunkPrelitUnlit = 0x00000023;
export inline constexpr std::uint32_t W3DChunkPrelitVertex = 0x00000024;
export inline constexpr std::uint32_t W3DChunkPrelitLightmapMultiPass = 0x00000025;
export inline constexpr std::uint32_t W3DChunkPrelitLightmapMultiTexture = 0x00000026;
export inline constexpr std::uint32_t W3DChunkMaterialInfo = 0x00000028;
export inline constexpr std::uint32_t W3DChunkShaders = 0x00000029;
export inline constexpr std::uint32_t W3DChunkVertexMaterials = 0x0000002A;
export inline constexpr std::uint32_t W3DChunkVertexMaterial = 0x0000002B;
export inline constexpr std::uint32_t W3DChunkVertexMaterialName = 0x0000002C;
export inline constexpr std::uint32_t W3DChunkVertexMaterialInfo = 0x0000002D;
export inline constexpr std::uint32_t W3DChunkTextures = 0x00000030;
export inline constexpr std::uint32_t W3DChunkTexture = 0x00000031;
export inline constexpr std::uint32_t W3DChunkTextureName = 0x00000032;
export inline constexpr std::uint32_t W3DChunkMaterialPass = 0x00000038;
export inline constexpr std::uint32_t W3DChunkVertexMaterialIds = 0x00000039;
export inline constexpr std::uint32_t W3DChunkShaderIds = 0x0000003A;
export inline constexpr std::uint32_t W3DChunkShaderMaterialIds = 0x0000003F;
export inline constexpr std::uint32_t W3DChunkTextureStage = 0x00000048;
export inline constexpr std::uint32_t W3DChunkTextureIds = 0x00000049;
export inline constexpr std::uint32_t W3DChunkStageTextureCoords = 0x0000004A;
export inline constexpr std::uint32_t W3DChunkPerFaceTextureCoordIds = 0x0000004B;
export inline constexpr std::uint32_t W3DChunkShaderMaterials = 0x00000050;
export inline constexpr std::uint32_t W3DChunkShaderMaterial = 0x00000051;
export inline constexpr std::uint32_t W3DChunkShaderMaterialHeader = 0x00000052;
export inline constexpr std::uint32_t W3DChunkShaderMaterialProperty = 0x00000053;
export inline constexpr std::uint32_t W3DChunkTangents = 0x00000060;
export inline constexpr std::uint32_t W3DChunkBitangents = 0x00000061;
export inline constexpr std::uint32_t W3DChunkHierarchy = 0x00000100;
export inline constexpr std::uint32_t W3DChunkHierarchyHeader = 0x00000101;
export inline constexpr std::uint32_t W3DChunkPivots = 0x00000102;
export inline constexpr std::uint32_t W3DChunkAnimation = 0x00000200;
export inline constexpr std::uint32_t W3DChunkAnimationHeader = 0x00000201;
export inline constexpr std::uint32_t W3DChunkCompressedAnimation = 0x00000280;
export inline constexpr std::uint32_t W3DChunkMorphAnimation = 0x000002C0;
export inline constexpr std::uint32_t W3DChunkSizeMask = 0x7FFFFFFF;
export inline constexpr std::uint32_t W3DChunkContainsChildren = 0x80000000;
export inline constexpr std::uint32_t W3DInvalidIndex = 0xFFFFFFFF;
export inline constexpr std::uint32_t W3DMaterialUseAlpha = 0x00000001;
export inline constexpr std::uint32_t W3DMaterialUseSorting = 0x00000002;

export struct W3DChunkView final
{
	std::uint32_t id = 0;
	bool contains_children = false;
	W3DByteSpan payload;
};

export bool W3DRead_U32(W3DByteSpan bytes, std::size_t offset, std::uint32_t &value) noexcept
{
	if (offset > bytes.size() || bytes.size() - offset < sizeof(std::uint32_t))
		return false;

	const auto *data = reinterpret_cast<const std::uint8_t *>(bytes.data() + offset);
	value = static_cast<std::uint32_t>(data[0]) |
		(static_cast<std::uint32_t>(data[1]) << 8) |
		(static_cast<std::uint32_t>(data[2]) << 16) |
		(static_cast<std::uint32_t>(data[3]) << 24);
	return true;
}

export bool W3DRead_F32(W3DByteSpan bytes, std::size_t offset, float &value) noexcept
{
	std::uint32_t bits = 0;
	if (!W3DRead_U32(bytes, offset, bits))
		return false;
	value = std::bit_cast<float>(bits);
	return true;
}

export bool W3DRead_Vector3(W3DByteSpan bytes, std::size_t offset, Vector3f &value) noexcept
{
	return W3DRead_F32(bytes, offset, value.x) &&
		W3DRead_F32(bytes, offset + sizeof(float), value.y) &&
		W3DRead_F32(bytes, offset + sizeof(float) * 2, value.z);
}

export std::string W3DRead_String(W3DByteSpan bytes)
{
	if (bytes.empty())
		return {};
	const auto *data = reinterpret_cast<const char *>(bytes.data());
	std::size_t length = 0;
	while (length < bytes.size() && data[length] != '\0')
		++length;
	return std::string(data, length);
}

export std::string W3DRead_Fixed_String(W3DByteSpan bytes, std::size_t offset, std::size_t length)
{
	if (offset > bytes.size() || bytes.size() - offset < length)
		return {};
	return W3DRead_String(bytes.subspan(offset, length));
}

export template <typename Callback>
bool W3DVisit_Chunks(W3DByteSpan bytes, Callback &&callback)
{
	std::size_t offset = 0;
	while (offset < bytes.size()) {
		if (bytes.size() - offset < 8)
			return false;

		std::uint32_t id = 0;
		std::uint32_t encoded_size = 0;
		if (!W3DRead_U32(bytes, offset, id) || !W3DRead_U32(bytes, offset + 4, encoded_size))
			return false;

		const std::size_t payload_size = encoded_size & W3DChunkSizeMask;
		if (payload_size > bytes.size() - offset - 8)
			return false;

		const W3DChunkView chunk{
			id,
			(encoded_size & W3DChunkContainsChildren) != 0,
			bytes.subspan(offset + 8, payload_size)};
		if (!callback(chunk))
			return false;
		offset += 8 + payload_size;
	}
	return offset == bytes.size();
}

export bool W3DValidate_Chunk_Tree(W3DByteSpan bytes, std::size_t depth = 0)
{
	if (depth > 64)
		return false;
	return W3DVisit_Chunks(bytes, [depth](const W3DChunkView &chunk) {
		return !chunk.contains_children || W3DValidate_Chunk_Tree(chunk.payload, depth + 1);
	});
}

}
