module;

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

export module Video.Frame;

namespace Engine::Video
{

export enum class PixelFormat : std::uint8_t
{
	Unknown,
	RGB8,
	RGBA8,
	BGRA8,
	BGRX8,
	RGB565,
	RGB555
};

export constexpr std::uint32_t Bytes_Per_Pixel(PixelFormat format) noexcept
{
	switch (format) {
	case PixelFormat::RGB8:
		return 3;
	case PixelFormat::RGBA8:
	case PixelFormat::BGRA8:
	case PixelFormat::BGRX8:
		return 4;
	case PixelFormat::RGB565:
	case PixelFormat::RGB555:
		return 2;
	case PixelFormat::Unknown:
		return 0;
	}

	return 0;
}

export struct DecodedVideoFrame final
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t row_pitch = 0;
	PixelFormat format = PixelFormat::Unknown;
	std::uint64_t frame_index = 0;
	std::uint64_t presentation_time_us = 0;
	std::span<const std::byte> pixels{};

	bool Is_Valid() const noexcept
	{
		const std::uint32_t bytes_per_pixel = Bytes_Per_Pixel(format);
		if (width == 0 || height == 0 || bytes_per_pixel == 0)
			return false;

		const std::uint64_t minimum_row_pitch = static_cast<std::uint64_t>(width) * bytes_per_pixel;
		const std::uint64_t required_size = static_cast<std::uint64_t>(row_pitch) * height;
		return minimum_row_pitch <= std::numeric_limits<std::uint32_t>::max()
			&& row_pitch >= minimum_row_pitch
			&& required_size <= pixels.size();
	}
};

}
