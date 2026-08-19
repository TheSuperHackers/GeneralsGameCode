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

#include "PreRTS.h"

#include "Common/Diagnostic/SimulationMathCrc.h"
#include "Common/XferCRC.h"
#include "WWMath/matrix3d.h"
#include "WWMath/wwmath.h"
#include "GameLogic/FPUControl.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

static void appendSimulationMathCrc_Deterministic(XferCRC &xfer)
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

    xfer.xferMatrix3D(&matrix);
}

static void appendSimulationMathCrc_Native(XferCRC &xfer)
{
    Matrix3D matrix;
    Matrix3D factorsMatrix;

    matrix.Set(
        4.1f, 1.2f, 0.3f, 0.4f,
        0.5f, 3.6f, 0.7f, 0.8f,
        0.9f, 1.0f, 2.1f, 1.2f);

    factorsMatrix.Set(
        (float)(::sin(0.7) * ::log10(2.3)),
        (float)(::cos(1.1) * ::pow(1.1, 2.0)),
        (float)::tan(0.3),
        (float)::asin(0.967302263),
        (float)::acos(0.967302263),
        (float)(::atan(0.967302263) * ::pow(1.1, 2.0)),
        (float)::atan2(0.4, 1.3),
        (float)::sinh(0.2),
        (float)(::cosh(0.4) * ::tanh(0.5)),
        (float)::sqrt(55788.84375),
        (float)(::exp(0.1) * ::log10(2.3)),
        (float)::log(1.4));

    Matrix3D::Multiply(matrix, factorsMatrix, &matrix);
    matrix.Get_Inverse(matrix);

    xfer.xferMatrix3D(&matrix);
}

UnsignedInt SimulationMathCrc::calculate()
{
    XferCRC xfer;
    xfer.open("SimulationMathCrc");

    setFPMode();

    appendSimulationMathCrc_Deterministic(xfer);

    _fpreset();

    xfer.close();

    return xfer.getCRC();
}

void SimulationMathCrc::runBenchmark(int iterations)
{
    int i;
    clock_t startDet = clock();
    UnsignedInt crcDet = 0;
    
    setFPMode();

    for (i = 0; i < iterations; ++i)
    {
        XferCRC xfer;
        xfer.open("SimMathDet");
        appendSimulationMathCrc_Deterministic(xfer);
        xfer.close();
		if (i == 0)
			crcDet = xfer.getCRC();
    }
    _fpreset();
    clock_t endDet = clock();
    double timeDetMs = (double)(endDet - startDet) / CLOCKS_PER_SEC * 1000.0;

    clock_t startNat = clock();
    UnsignedInt crcNat = 0;
    
    setFPMode();

    for (i = 0; i < iterations; ++i)
    {
        XferCRC xfer;
        xfer.open("SimMathNat");
        appendSimulationMathCrc_Native(xfer);
        xfer.close();
		if (i == 0)
			crcNat = xfer.getCRC();
    }
    _fpreset();
    clock_t endNat = clock();
    double timeNatMs = (double)(endNat - startNat) / CLOCKS_PER_SEC * 1000.0;

    printf("\n================ MATH BENCHMARK (%d iterations) ================\n", iterations);
    printf("Deterministic (WWMath): CRC = %08X, Time = %.2f ms\n", crcDet, timeDetMs);
    printf("Native (system math):   CRC = %08X, Time = %.2f ms\n", crcNat, timeNatMs);
    printf("===========================================================\n\n");
}
