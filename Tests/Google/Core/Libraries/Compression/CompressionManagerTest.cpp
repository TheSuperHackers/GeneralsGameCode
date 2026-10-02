/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>
#include <vector>

#include "Compression.h"

// The game compresses map and save data chunks and transferred map previews with the preferred compression.
TEST(CompressionManager, PreferredCompressionRoundTrips)
{
	std::vector<unsigned char> source(4096);
	for (size_t i = 0; i < source.size(); ++i)
	{
		source[i] = static_cast<unsigned char>(i % 61);
	}
	const Int sourceSize = static_cast<Int>(source.size());

	const CompressionType type = CompressionManager::getPreferredCompression();
	std::vector<unsigned char> compressed(CompressionManager::getMaxCompressedSize(sourceSize, type));
	const Int compressedSize = CompressionManager::compressData(type, source.data(), sourceSize, compressed.data(), static_cast<Int>(compressed.size()));
	ASSERT_GT(compressedSize, 0);
	EXPECT_LT(compressedSize, sourceSize);
	EXPECT_TRUE(CompressionManager::isDataCompressed(compressed.data(), compressedSize));
	EXPECT_EQ(CompressionManager::getUncompressedSize(compressed.data(), compressedSize), sourceSize);

	std::vector<unsigned char> decompressed(source.size());
	EXPECT_EQ(CompressionManager::decompressData(compressed.data(), compressedSize, decompressed.data(), sourceSize), sourceSize);
	EXPECT_EQ(decompressed, source);
}
