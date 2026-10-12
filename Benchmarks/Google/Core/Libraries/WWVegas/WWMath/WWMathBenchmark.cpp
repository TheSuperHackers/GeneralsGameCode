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
#include <math.h>

#include "WWMath/wwmath.h"

// Each WWMath function is timed next to the CRT function it replaces.

template <typename T, T (*Function)(T)>
static void BM_Unary(benchmark::State &state)
{
	T x = static_cast<T>(0.5);
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(x);
		benchmark::DoNotOptimize(Function(x));
	}
}

template <typename T, T (*Function)(T, T)>
static void BM_Binary(benchmark::State &state)
{
	T x = static_cast<T>(0.4);
	T y = static_cast<T>(1.3);
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(x);
		benchmark::DoNotOptimize(y);
		benchmark::DoNotOptimize(Function(x, y));
	}
}

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Sinf);
BENCHMARK_TEMPLATE(BM_Unary, float, sinf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Sin);
BENCHMARK_TEMPLATE(BM_Unary, double, sin);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Cosf);
BENCHMARK_TEMPLATE(BM_Unary, float, cosf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Cos);
BENCHMARK_TEMPLATE(BM_Unary, double, cos);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Tanf);
BENCHMARK_TEMPLATE(BM_Unary, float, tanf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Tan);
BENCHMARK_TEMPLATE(BM_Unary, double, tan);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Asinf);
BENCHMARK_TEMPLATE(BM_Unary, float, asinf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Asin);
BENCHMARK_TEMPLATE(BM_Unary, double, asin);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Acosf);
BENCHMARK_TEMPLATE(BM_Unary, float, acosf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Acos);
BENCHMARK_TEMPLATE(BM_Unary, double, acos);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Atanf);
BENCHMARK_TEMPLATE(BM_Unary, float, atanf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Atan);
BENCHMARK_TEMPLATE(BM_Unary, double, atan);

BENCHMARK_TEMPLATE(BM_Binary, float, WWMath::Atan2f);
BENCHMARK_TEMPLATE(BM_Binary, float, atan2f);
BENCHMARK_TEMPLATE(BM_Binary, double, WWMath::Atan2);
BENCHMARK_TEMPLATE(BM_Binary, double, atan2);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Sinhf);
BENCHMARK_TEMPLATE(BM_Unary, float, sinhf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Sinh);
BENCHMARK_TEMPLATE(BM_Unary, double, sinh);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Coshf);
BENCHMARK_TEMPLATE(BM_Unary, float, coshf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Cosh);
BENCHMARK_TEMPLATE(BM_Unary, double, cosh);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Tanhf);
BENCHMARK_TEMPLATE(BM_Unary, float, tanhf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Tanh);
BENCHMARK_TEMPLATE(BM_Unary, double, tanh);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Expf);
BENCHMARK_TEMPLATE(BM_Unary, float, expf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Exp);
BENCHMARK_TEMPLATE(BM_Unary, double, exp);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Logf);
BENCHMARK_TEMPLATE(BM_Unary, float, logf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Log);
BENCHMARK_TEMPLATE(BM_Unary, double, log);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Log10f);
BENCHMARK_TEMPLATE(BM_Unary, float, log10f);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Log10);
BENCHMARK_TEMPLATE(BM_Unary, double, log10);

BENCHMARK_TEMPLATE(BM_Binary, float, WWMath::Powf);
BENCHMARK_TEMPLATE(BM_Binary, float, powf);
BENCHMARK_TEMPLATE(BM_Binary, double, WWMath::Pow);
BENCHMARK_TEMPLATE(BM_Binary, double, pow);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Sqrtf);
BENCHMARK_TEMPLATE(BM_Unary, float, sqrtf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Sqrt);
BENCHMARK_TEMPLATE(BM_Unary, double, sqrt);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Fabsf);
BENCHMARK_TEMPLATE(BM_Unary, float, fabsf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Fabs);
BENCHMARK_TEMPLATE(BM_Unary, double, fabs);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Ceilf);
BENCHMARK_TEMPLATE(BM_Unary, float, ceilf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Ceil);
BENCHMARK_TEMPLATE(BM_Unary, double, ceil);

BENCHMARK_TEMPLATE(BM_Unary, float, WWMath::Floorf);
BENCHMARK_TEMPLATE(BM_Unary, float, floorf);
BENCHMARK_TEMPLATE(BM_Unary, double, WWMath::Floor);
BENCHMARK_TEMPLATE(BM_Unary, double, floor);
