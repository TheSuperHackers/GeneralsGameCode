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
#include <string>

#include "Utility/stringex.h"

// The source text is created at runtime, so that the compiler cannot precompute its length and the copy.
static const std::string SourceText("The quick brown fox jumps over the lazy dog");

static void BM_strlcpy_t(benchmark::State &state)
{
	char buffer[16];
	for (auto _ : state)
	{
		strlcpy_t(buffer, SourceText.c_str());
		benchmark::DoNotOptimize(buffer);
	}
}
BENCHMARK(BM_strlcpy_t);

// The truncating copy that strlcpy_t replaces.
static void BM_strncpy(benchmark::State &state)
{
	char buffer[16];
	for (auto _ : state)
	{
		strncpy(buffer, SourceText.c_str(), sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';
		benchmark::DoNotOptimize(buffer);
	}
}
BENCHMARK(BM_strncpy);
