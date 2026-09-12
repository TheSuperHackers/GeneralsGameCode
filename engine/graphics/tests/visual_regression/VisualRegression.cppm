module;

#include <algorithm>
#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

export module Graphics.Testing.VisualRegression;

export import Graphics.RHI;

namespace Graphics
{

export struct RGBAImage final
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::uint8_t> pixels;

	bool Is_Valid() const noexcept
	{
		return width != 0 && height != 0 && pixels.size() == static_cast<std::size_t>(width) * height * 4;
	}

	std::span<const std::uint8_t> Data() const noexcept
	{
		return pixels;
	}
};

export struct VisualRegressionConfig final
{
	std::uint32_t width = 128;
	std::uint32_t height = 72;
	std::uint8_t channel_tolerance = 1;
	std::filesystem::path reference_directory;
	std::filesystem::path failure_directory;
};

export struct VisualComparisonResult final
{
	bool matched = false;
	bool expected_loaded = false;
	std::uint32_t differing_pixels = 0;
	std::uint8_t maximum_channel_error = 0;
};

export bool Load_Binary_File(const std::filesystem::path &path, std::vector<std::byte> &data)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
		return false;

	const std::streampos end = file.tellg();
	if (end <= 0)
		return false;

	std::vector<std::byte> loaded(static_cast<std::size_t>(end));
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char *>(loaded.data()), static_cast<std::streamsize>(loaded.size()));
	if (!file.good() && !file.eof())
		return false;

	data = std::move(loaded);
	return true;
}

static std::uint32_t Read_U32_Big_Endian(std::span<const std::uint8_t> data) noexcept
{
	return (static_cast<std::uint32_t>(data[0]) << 24) | (static_cast<std::uint32_t>(data[1]) << 16) | (static_cast<std::uint32_t>(data[2]) << 8) | static_cast<std::uint32_t>(data[3]);
}

static std::uint32_t Update_CRC32(std::uint32_t crc, std::span<const std::uint8_t> data) noexcept
{
	for (std::uint8_t value : data) {
		crc ^= value;
		for (std::uint32_t bit = 0; bit < 8; ++bit)
			crc = (crc & 1u) != 0 ? (crc >> 1) ^ 0xedb88320u : crc >> 1;
	}
	return crc;
}

static std::uint32_t CRC32(std::span<const std::uint8_t> type, std::span<const std::uint8_t> data) noexcept
{
	std::uint32_t crc = Update_CRC32(0xffffffffu, type);
	return ~Update_CRC32(crc, data);
}

static void Append_U32_Big_Endian(std::vector<std::uint8_t> &data, std::uint32_t value)
	{
	data.push_back(static_cast<std::uint8_t>(value >> 24));
	data.push_back(static_cast<std::uint8_t>(value >> 16));
	data.push_back(static_cast<std::uint8_t>(value >> 8));
	data.push_back(static_cast<std::uint8_t>(value));
}

static void Append_PNG_Chunk(std::vector<std::uint8_t> &png, std::array<std::uint8_t, 4> type, std::span<const std::uint8_t> data)
	{
	Append_U32_Big_Endian(png, static_cast<std::uint32_t>(data.size()));
	png.insert(png.end(), type.begin(), type.end());
	png.insert(png.end(), data.begin(), data.end());
	Append_U32_Big_Endian(png, CRC32(type, data));
}

static std::uint32_t Adler32(std::span<const std::uint8_t> data) noexcept
{
	std::uint32_t lower = 1;
	std::uint32_t upper = 0;
	for (std::uint8_t value : data) {
		lower = (lower + value) % 65521u;
		upper = (upper + lower) % 65521u;
	}
	return (upper << 16) | lower;
}

static bool Inflate_Stored_Zlib(std::span<const std::uint8_t> compressed, std::size_t expected_size, std::vector<std::uint8_t> &raw)
{
	if (compressed.size() < 6 || compressed[0] != 0x78 || (compressed[0] & 0x0f) != 8 || (static_cast<std::uint32_t>(compressed[0]) << 8 | compressed[1]) % 31 != 0)
		return false;

	const std::size_t deflate_end = compressed.size() - 4;
	std::size_t compressed_offset = 2;
	std::size_t raw_offset = 0;
	bool final_block = false;
	raw.resize(expected_size);
	while (!final_block) {
		if (compressed_offset >= deflate_end)
			return false;

		const std::uint8_t block_header = compressed[compressed_offset++];
		final_block = (block_header & 1u) != 0;
		if (((block_header >> 1) & 3u) != 0 || compressed_offset + 4 > deflate_end)
			return false;

		const std::uint16_t length = static_cast<std::uint16_t>(compressed[compressed_offset]) | static_cast<std::uint16_t>(compressed[compressed_offset + 1] << 8);
		const std::uint16_t inverse_length = static_cast<std::uint16_t>(compressed[compressed_offset + 2]) | static_cast<std::uint16_t>(compressed[compressed_offset + 3] << 8);
		compressed_offset += 4;
		if (static_cast<std::uint16_t>(length ^ inverse_length) != 0xffffu || length > deflate_end - compressed_offset || length > expected_size - raw_offset)
			return false;

		for (std::uint16_t index = 0; index < length; ++index)
			raw[raw_offset + index] = compressed[compressed_offset + index];
		compressed_offset += length;
		raw_offset += length;
	}

	if (raw_offset != expected_size || compressed_offset != deflate_end || Read_U32_Big_Endian(compressed.subspan(deflate_end, 4)) != Adler32(raw))
		return false;

	return true;
}

static std::uint8_t Paeth(std::uint8_t left, std::uint8_t above, std::uint8_t upper_left) noexcept
	{
	const int prediction = static_cast<int>(left) + static_cast<int>(above) - static_cast<int>(upper_left);
	const int left_distance = prediction > left ? prediction - left : left - prediction;
	const int above_distance = prediction > above ? prediction - above : above - prediction;
	const int upper_left_distance = prediction > upper_left ? prediction - upper_left : upper_left - prediction;
	if (left_distance <= above_distance && left_distance <= upper_left_distance)
		return left;
	return above_distance <= upper_left_distance ? above : upper_left;
}

export bool Load_RGBA8_PNG(const std::filesystem::path &path, RGBAImage &image)
{
	std::vector<std::byte> file_data;
	if (!Load_Binary_File(path, file_data))
		return false;

	const std::span<const std::uint8_t> bytes(reinterpret_cast<const std::uint8_t *>(file_data.data()), file_data.size());
	constexpr std::array<std::uint8_t, 8> signature = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
	if (bytes.size() < signature.size() || !std::equal(signature.begin(), signature.end(), bytes.begin()))
		return false;

	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::vector<std::uint8_t> compressed;
	bool header_found = false;
	bool end_found = false;
	std::size_t offset = signature.size();
	while (offset + 12 <= bytes.size()) {
		const std::uint32_t chunk_size = Read_U32_Big_Endian(bytes.subspan(offset, 4));
		if (chunk_size > bytes.size() - offset - 12)
			return false;
		const std::span<const std::uint8_t> type = bytes.subspan(offset + 4, 4);
		const std::span<const std::uint8_t> chunk = bytes.subspan(offset + 8, chunk_size);
		const std::uint32_t stored_crc = Read_U32_Big_Endian(bytes.subspan(offset + 8 + chunk_size, 4));
		if (stored_crc != CRC32(type, chunk))
			return false;

		if (type[0] == 'I' && type[1] == 'H' && type[2] == 'D' && type[3] == 'R') {
			if (header_found || chunk.size() != 13 || chunk[8] != 8 || chunk[9] != 6 || chunk[10] != 0 || chunk[11] != 0 || chunk[12] != 0)
				return false;
			width = Read_U32_Big_Endian(chunk);
			height = Read_U32_Big_Endian(chunk.subspan(4, 4));
			header_found = width != 0 && height != 0;
		} else if (type[0] == 'I' && type[1] == 'D' && type[2] == 'A' && type[3] == 'T') {
			compressed.insert(compressed.end(), chunk.begin(), chunk.end());
		} else if (type[0] == 'I' && type[1] == 'E' && type[2] == 'N' && type[3] == 'D') {
			if (!chunk.empty())
				return false;
			end_found = true;
			break;
		}

		offset += static_cast<std::size_t>(chunk_size) + 12;
	}

	if (!header_found || !end_found || compressed.empty() || width > (std::numeric_limits<std::size_t>::max() - 1) / 4)
		return false;
	const std::size_t row_size = static_cast<std::size_t>(width) * 4;
	const std::size_t scanline_size = row_size + 1;
	if (height > std::numeric_limits<std::size_t>::max() / scanline_size)
		return false;
	const std::size_t raw_size = static_cast<std::size_t>(height) * scanline_size;
	std::vector<std::uint8_t> raw;
	if (!Inflate_Stored_Zlib(compressed, raw_size, raw))
		return false;

	RGBAImage loaded;
	loaded.width = width;
	loaded.height = height;
	loaded.pixels.resize(static_cast<std::size_t>(width) * height * 4);
	for (std::uint32_t row = 0; row < height; ++row) {
		const std::size_t scanline = static_cast<std::size_t>(row) * scanline_size;
		if (raw[scanline] > 4)
			return false;
		for (std::size_t byte = 0; byte < row_size; ++byte) {
			const std::size_t current = scanline + 1 + byte;
			const std::uint8_t left = byte >= 4 ? raw[current - 4] : 0;
			const std::uint8_t above = row != 0 ? raw[current - scanline_size] : 0;
			const std::uint8_t upper_left = row != 0 && byte >= 4 ? raw[current - scanline_size - 4] : 0;
			switch (raw[scanline]) {
			case 0:
				break;
			case 1:
				raw[current] = static_cast<std::uint8_t>(raw[current] + left);
				break;
			case 2:
				raw[current] = static_cast<std::uint8_t>(raw[current] + above);
				break;
			case 3:
				raw[current] = static_cast<std::uint8_t>(raw[current] + static_cast<std::uint8_t>((static_cast<std::uint16_t>(left) + above) / 2));
				break;
			case 4:
				raw[current] = static_cast<std::uint8_t>(raw[current] + Paeth(left, above, upper_left));
				break;
			}
			loaded.pixels[static_cast<std::size_t>(row) * row_size + byte] = raw[current];
		}
	}

	image = std::move(loaded);
	return true;
}

export bool Save_RGBA8_PNG(const std::filesystem::path &path, const RGBAImage &image)
{
	if (!image.Is_Valid() || image.width > (std::numeric_limits<std::size_t>::max() - 1) / 4)
		return false;

	const std::size_t row_size = static_cast<std::size_t>(image.width) * 4;
	const std::size_t scanline_size = row_size + 1;
	if (image.height > std::numeric_limits<std::size_t>::max() / scanline_size)
		return false;
	const std::size_t raw_size = static_cast<std::size_t>(image.height) * scanline_size;
	if (raw_size > std::numeric_limits<std::uint32_t>::max() - 32)
		return false;

	std::vector<std::uint8_t> raw(raw_size);
	for (std::uint32_t row = 0; row < image.height; ++row) {
		const std::size_t source = static_cast<std::size_t>(row) * row_size;
		const std::size_t destination = static_cast<std::size_t>(row) * scanline_size;
		for (std::size_t byte = 0; byte < row_size; ++byte)
			raw[destination + 1 + byte] = image.pixels[source + byte];
	}

	std::vector<std::uint8_t> compressed;
	compressed.reserve(raw.size() + 16);
	compressed.push_back(0x78);
	compressed.push_back(0x01);
	std::size_t raw_offset = 0;
	while (raw_offset < raw.size()) {
		const std::size_t block_size = std::min<std::size_t>(raw.size() - raw_offset, 65535);
		const bool final_block = raw_offset + block_size == raw.size();
		compressed.push_back(final_block ? 1 : 0);
		const auto length = static_cast<std::uint16_t>(block_size);
		const auto inverse_length = static_cast<std::uint16_t>(~length);
		compressed.push_back(static_cast<std::uint8_t>(length));
		compressed.push_back(static_cast<std::uint8_t>(length >> 8));
		compressed.push_back(static_cast<std::uint8_t>(inverse_length));
		compressed.push_back(static_cast<std::uint8_t>(inverse_length >> 8));
		compressed.insert(compressed.end(), raw.begin() + static_cast<std::ptrdiff_t>(raw_offset), raw.begin() + static_cast<std::ptrdiff_t>(raw_offset + block_size));
		raw_offset += block_size;
	}
	Append_U32_Big_Endian(compressed, Adler32(raw));

	std::vector<std::uint8_t> png = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
	std::array<std::uint8_t, 13> header{};
	header[0] = static_cast<std::uint8_t>(image.width >> 24);
	header[1] = static_cast<std::uint8_t>(image.width >> 16);
	header[2] = static_cast<std::uint8_t>(image.width >> 8);
	header[3] = static_cast<std::uint8_t>(image.width);
	header[4] = static_cast<std::uint8_t>(image.height >> 24);
	header[5] = static_cast<std::uint8_t>(image.height >> 16);
	header[6] = static_cast<std::uint8_t>(image.height >> 8);
	header[7] = static_cast<std::uint8_t>(image.height);
	header[8] = 8;
	header[9] = 6;
	Append_PNG_Chunk(png, {'I', 'H', 'D', 'R'}, header);
	Append_PNG_Chunk(png, {'I', 'D', 'A', 'T'}, compressed);
	Append_PNG_Chunk(png, {'I', 'E', 'N', 'D'}, {});

	std::error_code error;
	if (!path.parent_path().empty())
		std::filesystem::create_directories(path.parent_path(), error);
	if (error)
		return false;

	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	if (!file)
		return false;
	file.write(reinterpret_cast<const char *>(png.data()), static_cast<std::streamsize>(png.size()));
	return static_cast<bool>(file);
}

export VisualComparisonResult Compare_RGBA8(const RGBAImage &actual, const RGBAImage &expected, std::uint8_t channel_tolerance) noexcept
{
	VisualComparisonResult result;
	result.expected_loaded = expected.Is_Valid();
	if (!actual.Is_Valid() || !expected.Is_Valid() || actual.width != expected.width || actual.height != expected.height)
		return result;

	const std::size_t pixel_count = static_cast<std::size_t>(actual.width) * actual.height;
	for (std::size_t pixel = 0; pixel < pixel_count; ++pixel) {
		bool different = false;
		for (std::size_t channel = 0; channel < 4; ++channel) {
			const std::uint8_t left = actual.pixels[pixel * 4 + channel];
			const std::uint8_t right = expected.pixels[pixel * 4 + channel];
			const std::uint8_t error = left > right ? static_cast<std::uint8_t>(left - right) : static_cast<std::uint8_t>(right - left);
			if (error > result.maximum_channel_error)
				result.maximum_channel_error = error;
			if (error > channel_tolerance)
				different = true;
		}
		if (different)
			++result.differing_pixels;
	}

	result.matched = result.differing_pixels == 0;
	return result;
}

export RGBAImage Make_Diff_Image(const RGBAImage &actual, const RGBAImage &expected) noexcept
{
	RGBAImage diff;
	if (!actual.Is_Valid() || !expected.Is_Valid() || actual.width != expected.width || actual.height != expected.height)
		return diff;

	diff.width = actual.width;
	diff.height = actual.height;
	diff.pixels.resize(actual.pixels.size());
	for (std::size_t index = 0; index < actual.pixels.size(); index += 4) {
		const std::uint8_t red_error = actual.pixels[index + 0] > expected.pixels[index + 0] ? static_cast<std::uint8_t>(actual.pixels[index + 0] - expected.pixels[index + 0]) : static_cast<std::uint8_t>(expected.pixels[index + 0] - actual.pixels[index + 0]);
		const std::uint8_t green_error = actual.pixels[index + 1] > expected.pixels[index + 1] ? static_cast<std::uint8_t>(actual.pixels[index + 1] - expected.pixels[index + 1]) : static_cast<std::uint8_t>(expected.pixels[index + 1] - actual.pixels[index + 1]);
		const std::uint8_t blue_error = actual.pixels[index + 2] > expected.pixels[index + 2] ? static_cast<std::uint8_t>(actual.pixels[index + 2] - expected.pixels[index + 2]) : static_cast<std::uint8_t>(expected.pixels[index + 2] - actual.pixels[index + 2]);
		diff.pixels[index + 0] = red_error;
		diff.pixels[index + 1] = green_error;
		diff.pixels[index + 2] = blue_error;
		diff.pixels[index + 3] = 255;
	}
	return diff;
}

export using VisualRenderFunction = bool (*)(Device &, CommandList &, RHITextureHandle, RHITextureHandle, RHIViewport, void *) noexcept;

export class VisualRegressionHarness final
{
public:
	explicit VisualRegressionHarness(VisualRegressionConfig config) noexcept
		: m_config(std::move(config))
	{
		const char *suffix = std::getenv("GRAPHICS_TEST_FAILURE_SUFFIX");
		if (suffix != nullptr && *suffix != '\0' && !m_config.failure_directory.empty())
			m_config.failure_directory /= suffix;
	}

	bool Render_Offscreen(Device &device, VisualRenderFunction render, void *context, RGBAImage &image)
	{
		if (!device.Is_Valid() || render == nullptr || m_config.width == 0 || m_config.height == 0)
			return false;

		const RHITextureHandle color_target = device.Create_Texture({
			m_config.width,
			m_config.height,
			1,
			RHITextureFormat::RGBA8_UNorm,
			static_cast<std::uint32_t>(RHITextureUsage::RenderTarget),
			1
		});
		const RHITextureHandle depth_target = device.Create_Texture({
			m_config.width,
			m_config.height,
			1,
			RHITextureFormat::D32_Float,
			static_cast<std::uint32_t>(RHITextureUsage::DepthStencil),
			1
		});
		if (!color_target.Is_Valid() || !depth_target.Is_Valid()) {
			if (color_target.Is_Valid())
				device.Destroy_Texture(color_target);
			if (depth_target.Is_Valid())
				device.Destroy_Texture(depth_target);
			return false;
		}

		const RHIViewport viewport{0, 0, m_config.width, m_config.height, 0.0f, 1.0f};
		const bool rendered = render(device, device.Immediate_Command_List(), color_target, depth_target, viewport, context);
		image.width = m_config.width;
		image.height = m_config.height;
		image.pixels.resize(static_cast<std::size_t>(m_config.width) * m_config.height * 4);
		const bool read = rendered && device.Readback_Texture(color_target, std::as_writable_bytes(std::span<std::uint8_t>(image.pixels)), m_config.width * 4);
		const bool destroyed = device.Destroy_Texture(depth_target) && device.Destroy_Texture(color_target);
		if (!read || !destroyed) {
			image = {};
			return false;
		}

		return true;
	}

	VisualComparisonResult Run(Device &device, std::string_view scene_name, VisualRenderFunction render, void *context)
	{
		RGBAImage actual;
		const bool rendered = Render_Offscreen(device, render, context, actual);
		RGBAImage expected;
		const std::filesystem::path expected_path = m_config.reference_directory / (std::string(scene_name) + ".png");
		const char *update_references = std::getenv("GRAPHICS_TEST_UPDATE_REFERENCES");
		if (rendered && update_references != nullptr && std::string_view(update_references) == "1") {
			const bool saved = Save_RGBA8_PNG(expected_path, actual);
			return {saved, saved, 0, 0};
		}
		const bool expected_loaded = Load_RGBA8_PNG(expected_path, expected);
		VisualComparisonResult result = rendered && expected_loaded
			? Compare_RGBA8(actual, expected, m_config.channel_tolerance)
			: VisualComparisonResult{false, expected_loaded, 0, 0};
		if (!result.matched)
			Write_Failure_Artifacts(scene_name, actual, expected, expected_loaded);
		return result;
	}

private:
	void Write_Failure_Artifacts(std::string_view scene_name, const RGBAImage &actual, const RGBAImage &expected, bool expected_loaded) const
	{
		if (m_config.failure_directory.empty())
			return;

		const std::string name(scene_name);
		Save_RGBA8_PNG(m_config.failure_directory / (name + ".actual.png"), actual);
		if (expected_loaded)
			Save_RGBA8_PNG(m_config.failure_directory / (name + ".expected.png"), expected);
		Save_RGBA8_PNG(m_config.failure_directory / (name + ".diff.png"), Make_Diff_Image(actual, expected));
	}

	VisualRegressionConfig m_config;
};

}
