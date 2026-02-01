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

#include "Common/SimulationMathCrc.h"
#include "Common/XferCRC.h"
#include "WWMath/matrix3d.h"
#include "WWMath/wwmath.h"

#include <math.h>

static void appendMatrixCrc(XferCRC &xfer)
{
    Matrix3D matrix;
    Matrix3D factors_matrix;

    matrix.Set(
        4.1f, 1.2f, 0.3f, 0.4f,
        0.5f, 3.6f, 0.7f, 0.8f,
        0.9f, 1.0f, 2.1f, 1.2f);

    factors_matrix.Set(
        WWMath::Sin(0.7f) * log10f(2.3f),
        WWMath::Cos(1.1f) * powf(1.1f, 2.0f),
        tanf(0.3f),
        asinf(0.9673022627830505),
        acosf(0.9673022627830505),
        atanf(0.9673022627830505) * powf(1.1f, 2.0f),
        atan2f(0.4f, 1.3f),
        sinhf(0.2f),
        coshf(0.4f) * tanhf(0.5f),
        sqrtf(55788.84375),
        expf(0.1f) * log10f(2.3f),
        logf(1.4f));

    Matrix3D::Multiply(matrix, factors_matrix, &matrix);
    matrix.Get_Inverse(matrix);

    xfer.xferMatrix3D(&matrix);
}

UnsignedInt SimulationMathCrc::calculate()
{
    XferCRC xfer;
    xfer.open("SimulationMathCrc");

    _fpreset();
    // TheSuperHackers @info stephanmeesters 29/01/2026 Use the same rounding mode as used in the game. This affects vc6 only.
    _controlfp(0x000A001F, _MCW_RC | _MCW_PC | _MCW_EM);

    appendMatrixCrc(xfer);

    _fpreset();

    xfer.close();

    return xfer.getCRC();
}
