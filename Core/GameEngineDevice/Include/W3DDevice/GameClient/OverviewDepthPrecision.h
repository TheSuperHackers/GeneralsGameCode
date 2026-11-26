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

#pragma once

#include <algorithm>
#include <cmath>

namespace OverviewDepthPrecision
{
// Distances are in camera-forward world units. A view-plane ray at z=-1
// advances by one such unit, and maxDown is its largest downward Z component.
// Keep a large clearance from the highest terrain and a conservative 4x cap.
// The old near plane remains the floor, so this never alters ordinary zoom.
inline float nearPlane(float legacyNear, float farPlane, float excessHeight,
    float terrainClearance, float minDown, float maxDown)
{
    if (!std::isfinite(legacyNear) || !std::isfinite(farPlane) ||
        !std::isfinite(excessHeight) || !std::isfinite(terrainClearance) ||
        !std::isfinite(minDown) || !std::isfinite(maxDown) ||
        legacyNear <= 0.0f || farPlane <= legacyNear ||
        excessHeight <= 0.0f || terrainClearance <= 0.0f ||
        minDown <= 0.0f || maxDown < minDown)
        return legacyNear;

    // Fade in continuously above the legacy zoom ceiling and away from
    // near-horizontal views. Never move the far plane or change FOV.
    const float pitchWeight = std::min(minDown / 0.25f, 1.0f);
    const float desired = legacyNear + pitchWeight *
        std::min(3.0f * legacyNear, excessHeight * 0.125f);
    const float terrainLimit = terrainClearance / (16.0f * maxDown);
    const float rangeLimit = farPlane / 64.0f;
    return std::max(legacyNear, std::min({desired, terrainLimit, rangeLimit}));
}
}
