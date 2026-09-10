module;

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

export module Assets.Adapters.W3D.Null;

import Assets.Adapters.W3D.Chunks;

namespace Assets::W3D
{

export inline constexpr std::uint32_t W3DChunkNullObject = 0x00000750u;
export inline constexpr std::size_t W3DNullPayloadSize = 48u;
export inline constexpr std::size_t W3DNullNameOffset = 16u;
export inline constexpr std::size_t W3DNullNameSize = 32u;

export struct W3DNullDescription final
{
	std::uint32_t version = 0;
	std::uint32_t attributes = 0;
	std::string name;
};

namespace NullDetail
{

bool Fail(std::string &error, std::string_view message)
{
	error.assign(message.data(), message.size());
	return false;
}

bool Read_Name(W3DByteSpan bytes, std::string &name)
{
	if (bytes.size() < W3DNullNameOffset + W3DNullNameSize)
		return false;

	// W3dNullObjectStruct::Name is a fixed-size C string. Require the terminator
	// inside the field before using the bounded chunk-string helper; this keeps
	// malformed payloads from causing an unbounded read while preserving empty
	// names as a valid, terminated field.
	const auto field = bytes.subspan(W3DNullNameOffset, W3DNullNameSize);
	for (const std::byte value : field) {
		if (value == std::byte{0}) {
			name = W3DRead_String(field);
			return true;
		}
	}
	return false;
}

}

// Reads one W3D_CHUNK_NULL_OBJECT payload. The legacy loader consumed the
// first 48 bytes and ignored later bytes, so payloads with a trailing extension
// remain accepted here. Version, attributes, and padding are not used to
// create the no-draw object, but the two meaningful header fields are retained
// for callers that need to inspect the source record.
export bool W3DRead_Null(W3DByteSpan bytes, W3DNullDescription &result, std::string &error)
{
	error.clear();
	if (bytes.size() < W3DNullPayloadSize)
		return NullDetail::Fail(error, "W3D null object definition is truncated");

	W3DNullDescription next;
	if (!W3DRead_U32(bytes, 0, next.version)
		|| !W3DRead_U32(bytes, 4, next.attributes)
		|| !NullDetail::Read_Name(bytes, next.name))
		return NullDetail::Fail(error, "W3D null object definition has an invalid name");

	result = std::move(next);
	return true;
}

}
