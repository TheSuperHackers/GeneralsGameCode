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

#include <benchmark/benchmark.h>
#include <vector>

#include "Compression.h"

// The game compresses map and save data chunks and transferred map previews with the preferred compression.

static std::vector<unsigned char> makeSource()
{
	std::vector<unsigned char> source(64 * 1024);
	for (size_t i = 0; i < source.size(); ++i)
	{
		source[i] = static_cast<unsigned char>((i * 7) % 251);
	}
	return source;
}

static void BM_CompressData(benchmark::State &state)
{
	std::vector<unsigned char> source = makeSource();
	const Int sourceSize = static_cast<Int>(source.size());
	const CompressionType type = CompressionManager::getPreferredCompression();
	std::vector<unsigned char> compressed(CompressionManager::getMaxCompressedSize(sourceSize, type));

	for (auto _ : state)
	{
		benchmark::DoNotOptimize(CompressionManager::compressData(type, source.data(), sourceSize, compressed.data(), static_cast<Int>(compressed.size())));
	}
	state.SetBytesProcessed(state.iterations() * sourceSize);
}
BENCHMARK(BM_CompressData);

static void BM_DecompressData(benchmark::State &state)
{
	std::vector<unsigned char> source = makeSource();
	const Int sourceSize = static_cast<Int>(source.size());
	const CompressionType type = CompressionManager::getPreferredCompression();
	std::vector<unsigned char> compressed(CompressionManager::getMaxCompressedSize(sourceSize, type));
	const Int compressedSize = CompressionManager::compressData(type, source.data(), sourceSize, compressed.data(), static_cast<Int>(compressed.size()));
	std::vector<unsigned char> decompressed(source.size());

	for (auto _ : state)
	{
		benchmark::DoNotOptimize(CompressionManager::decompressData(compressed.data(), compressedSize, decompressed.data(), sourceSize));
	}
	state.SetBytesProcessed(state.iterations() * sourceSize);
}
BENCHMARK(BM_DecompressData);
