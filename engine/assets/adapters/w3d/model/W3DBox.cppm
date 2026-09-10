module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

export module Assets.Adapters.W3D.Box;

import Assets.Adapters.W3D.Chunks;
import Assets.Math;

namespace Assets::W3D
{

export inline constexpr std::uint32_t W3DChunkBox = 0x00000740u;
export inline constexpr std::uint32_t W3DBoxCurrentVersion = 0x00010000u;

export inline constexpr std::uint32_t W3DBoxAttributeOriented = 0x00000001u;
export inline constexpr std::uint32_t W3DBoxAttributeAligned = 0x00000002u;
export inline constexpr std::uint32_t W3DBoxAttributeCollisionTypeMask = 0x00000FF0u;
export inline constexpr std::uint32_t W3DBoxAttributeCollisionTypeShift = 4u;

// The on-disk box definition is a fixed-size leaf payload. Keeping the raw
// colour bytes avoids changing the old /255 conversion at the game boundary.
export inline constexpr std::size_t W3DBoxPayloadSize = 68u;
export inline constexpr std::size_t W3DBoxNameSize = 32u;

export struct W3DBoxDescription final
{
	std::uint32_t version = W3DBoxCurrentVersion;
	std::uint32_t attributes = 0;
	std::string name;
	std::array<std::uint8_t, 3> color{255, 255, 255};
	Vector3f center{};
	Vector3f extent{1, 1, 1};

	bool Is_Oriented() const noexcept { return (attributes & W3DBoxAttributeOriented) != 0; }
	bool Is_Aligned() const noexcept { return (attributes & W3DBoxAttributeAligned) != 0; }
	std::uint32_t Collision_Type() const noexcept
	{
		return ((attributes & W3DBoxAttributeCollisionTypeMask)
			>> W3DBoxAttributeCollisionTypeShift) << 1;
	}
};

namespace BoxDetail
{

bool Fail(std::string &error, std::string_view message)
{
	error.assign(message.data(), message.size());
	return false;
}

bool Read_Finite_Vector3(W3DByteSpan bytes, std::size_t offset, Vector3f &value) noexcept
{
	return W3DRead_Vector3(bytes, offset, value)
		&& std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool Read_Name(W3DByteSpan bytes, std::string &name) noexcept
{
	if (bytes.size() < 8u + W3DBoxNameSize)
		return false;
	name = W3DRead_Fixed_String(bytes, 8, W3DBoxNameSize);
	return true;
}

bool Read_Color(W3DByteSpan bytes, std::array<std::uint8_t, 3> &color) noexcept
{
	if (bytes.size() < 43)
		return false;
	color[0] = std::to_integer<std::uint8_t>(bytes[40]);
	color[1] = std::to_integer<std::uint8_t>(bytes[41]);
	color[2] = std::to_integer<std::uint8_t>(bytes[42]);
	return true;
}

}

// Reads the payload of one W3D_CHUNK_BOX. The result is committed only after
// every field has been read, so callers retain their previous description on a
// malformed or truncated chunk.
export bool W3DRead_Box(W3DByteSpan bytes, W3DBoxDescription &result, std::string &error)
{
	error.clear();
	if (bytes.size() != W3DBoxPayloadSize)
		return BoxDetail::Fail(error, "W3D box definition has an invalid size");

	W3DBoxDescription next;
	if (!W3DRead_U32(bytes, 0, next.version)
		|| !W3DRead_U32(bytes, 4, next.attributes)
		|| !BoxDetail::Read_Name(bytes, next.name)
		|| !BoxDetail::Read_Color(bytes, next.color)
		|| !BoxDetail::Read_Finite_Vector3(bytes, 44, next.center)
		|| !BoxDetail::Read_Finite_Vector3(bytes, 56, next.extent))
		return BoxDetail::Fail(error, "W3D box definition is invalid");

	result = std::move(next);
	return true;
}

}
