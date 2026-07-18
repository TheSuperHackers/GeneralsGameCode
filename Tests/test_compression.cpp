// Opt-in CTest + doctest unit test harness.
// Round-trips real data through the Core Compression library's public API
// (Core/Libraries/Source/Compression/Compression.h) to prove compression and
// decompression are inverses of one another.

#include <doctest/doctest.h>

#include "Compression.h"

#include <cstring>
#include <string>
#include <vector>

TEST_CASE("CompressionManager zlib round-trip preserves original data")
{
    // Build a reasonably large, non-trivial buffer so zlib actually compresses it,
    // rather than a tiny buffer that might legitimately fail to shrink.
    std::string pattern =
        "The quick brown fox jumps over the lazy dog. "
        "openSAGE / GeneralsGameCode Compression round-trip test. ";
    std::vector<UnsignedByte> original;
    for (int i = 0; i < 200; ++i)
    {
        original.insert(original.end(), pattern.begin(), pattern.end());
    }

    const Int srcLen = static_cast<Int>(original.size());
    REQUIRE(srcLen > 0);

    const CompressionType compType = COMPRESSION_ZLIB5;

    const Int maxCompressedSize = CompressionManager::getMaxCompressedSize(srcLen, compType);
    REQUIRE(maxCompressedSize > 0);

    std::vector<UnsignedByte> compressed(static_cast<size_t>(maxCompressedSize), 0);

    const Int compressedLen = CompressionManager::compressData(
        compType, original.data(), srcLen, compressed.data(), maxCompressedSize);

    REQUIRE(compressedLen > 0);
    CHECK(CompressionManager::isDataCompressed(compressed.data(), compressedLen));
    CHECK(CompressionManager::getCompressionType(compressed.data(), compressedLen) == compType);
    CHECK(CompressionManager::getUncompressedSize(compressed.data(), compressedLen) == srcLen);

    std::vector<UnsignedByte> decompressed(static_cast<size_t>(srcLen), 0);

    const Int decompressedLen = CompressionManager::decompressData(
        compressed.data(), compressedLen, decompressed.data(), srcLen);

    REQUIRE(decompressedLen == srcLen);
    CHECK(std::memcmp(original.data(), decompressed.data(), static_cast<size_t>(srcLen)) == 0);
}
