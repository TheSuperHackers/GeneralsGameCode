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
#include <float.h>
#include <math.h>

#include "GameLogic/FPUControl.h"
#include "WWMath/matrix3d.h"
#include "WWMath/wwmath.h"

static Matrix3D computeSimulationMathWithWWMath()
{
	Matrix3D matrix;
	Matrix3D factorsMatrix;

	matrix.Set(
		4.1f, 1.2f, 0.3f, 0.4f,
		0.5f, 3.6f, 0.7f, 0.8f,
		0.9f, 1.0f, 2.1f, 1.2f);

	factorsMatrix.Set(
		WWMath::Sinf(0.7f) * WWMath::Log10f(2.3f),
		WWMath::Cosf(1.1f) * WWMath::Powf(1.1f, 2.0f),
		WWMath::Tanf(0.3f),
		WWMath::Asinf(0.967302263f),
		WWMath::Acosf(0.967302263f),
		WWMath::Atanf(0.967302263f) * WWMath::Powf(1.1f, 2.0f),
		WWMath::Atan2f(0.4f, 1.3f),
		WWMath::Sinhf(0.2f),
		WWMath::Coshf(0.4f) * WWMath::Tanhf(0.5f),
		WWMath::Sqrtf(55788.84375f),
		WWMath::Expf(0.1f) * WWMath::Log10f(2.3f),
		WWMath::Logf(1.4f));

	Matrix3D::Multiply(matrix, factorsMatrix, &matrix);
	matrix.Get_Inverse(matrix);

	return matrix;
}

static Matrix3D computeSimulationMathWithSystemMath()
{
	Matrix3D matrix;
	Matrix3D factorsMatrix;

	matrix.Set(
		4.1f, 1.2f, 0.3f, 0.4f,
		0.5f, 3.6f, 0.7f, 0.8f,
		0.9f, 1.0f, 2.1f, 1.2f);

	factorsMatrix.Set(
		::sinf(0.7f) * ::log10f(2.3f),
		::cosf(1.1f) * ::powf(1.1f, 2.0f),
		::tanf(0.3f),
		::asinf(0.967302263f),
		::acosf(0.967302263f),
		::atanf(0.967302263f) * ::powf(1.1f, 2.0f),
		::atan2f(0.4f, 1.3f),
		::sinhf(0.2f),
		::coshf(0.4f) * ::tanhf(0.5f),
		::sqrtf(55788.84375f),
		::expf(0.1f) * ::log10f(2.3f),
		::logf(1.4f));

	Matrix3D::Multiply(matrix, factorsMatrix, &matrix);
	matrix.Get_Inverse(matrix);

	return matrix;
}

static void BM_SimulationMath(benchmark::State &state)
{
	setFPMode();
	for (auto _ : state)
	{
		Matrix3D matrix = computeSimulationMathWithWWMath();
		benchmark::DoNotOptimize(matrix);
	}
	_fpreset();
}
BENCHMARK(BM_SimulationMath);

static void BM_SimulationMathWithSystemMath(benchmark::State &state)
{
	setFPMode();
	for (auto _ : state)
	{
		Matrix3D matrix = computeSimulationMathWithSystemMath();
		benchmark::DoNotOptimize(matrix);
	}
	_fpreset();
}
BENCHMARK(BM_SimulationMathWithSystemMath);
