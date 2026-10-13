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
#include <cstdio>
#include <type_traits>

#include "Common/INI.h"

enum ScanParser
{
	ScanParser_Current,
	ScanParser_Legacy,
};

// Ordinary inputs use the fast path. Overflow and underflow (Real only) use the range fallback.
enum ScanInput
{
	ScanInput_Ordinary,
	ScanInput_Overflow,
	ScanInput_Underflow,
};

// Keep the original conversion and failure check, with a call boundary like the production scanners.
// Out-of-range sscanf results characterize the local CRT, not portable C behavior.
template <typename Type>
NOINLINE static Type scanLegacy(const char* token)
{
	Type value;
	const char* format = std::is_same_v<Type, Real> ? "%f" : std::is_signed_v<Type> ? "%d" : "%u";
	if (std::sscanf(token, format, &value) != 1)
		throw INI_INVALID_DATA;
	return value;
}

// Each iteration parses four fixed tokens. These are microbenchmarks, not a distribution of game INI inputs.
template <typename Type, ScanParser Parser, ScanInput Input>
static void BM_INIScan(benchmark::State &state)
{
	const char* const integerTokens[][4] = {
		{"0",                   "+42",                  "-1000",                "2147483647"},
		{"9999999999999999999", "+9999999999999999999", "-9999999999999999999", "18446744073709551616"}
	};
	const char* const realTokens[][4] = {
		{"0.0",    "+1.25",   "-1000.5", "3.14159265"},
		{"1e100",  "+1e100",  "-1e100",  "3.5e38"},
		{"1e-100", "+1e-100", "-1e-100", "1e-46"}
	};
	const auto& tokens = std::is_same_v<Type, Real> ? realTokens[Input] : integerTokens[Input];

	for (auto _ : state)
	{
		for (const char* token : tokens)
		{
			benchmark::DoNotOptimize(token);
			Type value;
			if constexpr (Parser == ScanParser_Legacy)
				value = scanLegacy<Type>(token);
			else if constexpr (std::is_same_v<Type, Real>)
				value = INI::scanReal(token);
			else if constexpr (std::is_signed_v<Type>)
				value = INI::scanInt(token);
			else
				value = INI::scanUnsignedInt(token);
			benchmark::DoNotOptimize(value);
		}
	}
	state.SetItemsProcessed(state.iterations() * 4);
}

BENCHMARK_TEMPLATE(BM_INIScan, Int, ScanParser_Current, ScanInput_Ordinary);
BENCHMARK_TEMPLATE(BM_INIScan, Int, ScanParser_Legacy, ScanInput_Ordinary);
BENCHMARK_TEMPLATE(BM_INIScan, Int, ScanParser_Current, ScanInput_Overflow);
BENCHMARK_TEMPLATE(BM_INIScan, Int, ScanParser_Legacy, ScanInput_Overflow);
BENCHMARK_TEMPLATE(BM_INIScan, UnsignedInt, ScanParser_Current, ScanInput_Ordinary);
BENCHMARK_TEMPLATE(BM_INIScan, UnsignedInt, ScanParser_Legacy, ScanInput_Ordinary);
BENCHMARK_TEMPLATE(BM_INIScan, UnsignedInt, ScanParser_Current, ScanInput_Overflow);
BENCHMARK_TEMPLATE(BM_INIScan, UnsignedInt, ScanParser_Legacy, ScanInput_Overflow);
BENCHMARK_TEMPLATE(BM_INIScan, Real, ScanParser_Current, ScanInput_Ordinary);
BENCHMARK_TEMPLATE(BM_INIScan, Real, ScanParser_Legacy, ScanInput_Ordinary);
BENCHMARK_TEMPLATE(BM_INIScan, Real, ScanParser_Current, ScanInput_Overflow);
BENCHMARK_TEMPLATE(BM_INIScan, Real, ScanParser_Legacy, ScanInput_Overflow);
BENCHMARK_TEMPLATE(BM_INIScan, Real, ScanParser_Current, ScanInput_Underflow);
BENCHMARK_TEMPLATE(BM_INIScan, Real, ScanParser_Legacy, ScanInput_Underflow);
