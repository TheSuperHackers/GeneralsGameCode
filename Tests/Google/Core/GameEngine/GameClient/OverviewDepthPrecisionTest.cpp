/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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
#include <limits>
#include "W3DDevice/GameClient/OverviewDepthPrecision.h"

using OverviewDepthPrecision::nearPlane;

TEST(OverviewDepthPrecisionTest, LegacyAndLowClearanceRemainUnchanged)
{
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, -1, 2000, .5f, 1.5f), 10);
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, 0, 2000, .5f, 1.5f), 10);
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, 400, 100, .5f, 1.5f), 10);
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, 400, 2000, 0, 1.5f), 10);
}

TEST(OverviewDepthPrecisionTest, SmoothHeightAndPitchRamp)
{
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, 8, 2000, .5f, 1.5f), 11);
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, 80, 2000, .5f, 1.5f), 20);
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, 80, 2000, .125f, 1.5f), 15);
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, 400, 2000, .5f, 1.5f), 40);
}

TEST(OverviewDepthPrecisionTest, TerrainAndFarRangeBoundThePlane)
{
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, 400, 480, .5f, 1.5f), 20);
    EXPECT_FLOAT_EQ(nearPlane(10, 1280, 400, 2000, .5f, 1.5f), 20);
    EXPECT_FLOAT_EQ(nearPlane(10, 11, 400, 2000, .5f, 1.5f), 10);
}

TEST(OverviewDepthPrecisionTest, InvalidInputsFallBackToLegacy)
{
    const float nan = std::numeric_limits<float>::quiet_NaN();
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, nan, 2000, .5f, 1.5f), 10);
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, 400, nan, .5f, 1.5f), 10);
    EXPECT_FLOAT_EQ(nearPlane(10, 4000, 400, 2000, .5f, 0), 10);
    EXPECT_FLOAT_EQ(nearPlane(10, 0, 400, 2000, .5f, 1.5f), 10);
}
