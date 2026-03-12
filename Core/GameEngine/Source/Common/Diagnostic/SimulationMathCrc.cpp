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
#ifndef _WIN32
#include <fenv.h>
#endif

static void appendSimulationMathCrc(XferCRC &xfer)
{
    Matrix3D matrix;
    Matrix3D factorsMatrix;

    matrix.Set(
        4.1f, 1.2f, 0.3f, 0.4f,
        0.5f, 3.6f, 0.7f, 0.8f,
        0.9f, 1.0f, 2.1f, 1.2f);

    factorsMatrix.Set(
        WWMath::Sin(0.7f) * log10f(2.3f),
        WWMath::Cos(1.1f) * powf(1.1f, 2.0f),
        tanf(0.3f),
        asinf(0.967302263f),
        acosf(0.967302263f),
        atanf(0.967302263f) * powf(1.1f, 2.0f),
        atan2f(0.4f, 1.3f),
        sinhf(0.2f),
        coshf(0.4f) * tanhf(0.5f),
        sqrtf(55788.84375f),
        expf(0.1f) * log10f(2.3f),
        logf(1.4f));

    Matrix3D::Multiply(matrix, factorsMatrix, &matrix);
    matrix.Get_Inverse(matrix);

    xfer.xferMatrix3D(&matrix);
}

UnsignedInt SimulationMathCrc::calculate()
{
    XferCRC xfer;
    xfer.open("SimulationMathCrc");

    setFPMode();

    appendSimulationMathCrc(xfer);

    // GeneralsX @build BenderAI 12/03/2026 Restore the default FP environment portably after CRC sampling.
#ifdef _WIN32
    _fpreset();
#else
    fesetenv(FE_DFL_ENV);
    feclearexcept(FE_ALL_EXCEPT);
#endif

    xfer.close();

    return xfer.getCRC();
}
