/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#pragma once

#include <vector>

#include "WWMath/matrix3d.h"

// Neutral water geometry supplied by the game-world layer to the renderer.
// This deliberately contains no scene, resource, or PolygonTrigger types.
struct WaterGeometryPoint
{
	float x;
	float y;
	float z;
};

struct WaterSurfacePolygon
{
	bool river = false;
	int river_start = 0;
	std::vector<WaterGeometryPoint> points;
};

struct WaterGeometry
{
	std::vector<WaterSurfacePolygon> polygons;
};

// Read-only render snapshot of the gameplay water-grid state.  The renderer
// consumes this value and never reaches back into the simulation object.
struct WaterGridRenderData
{
	bool enabled = false;
	bool surface_override = false;
	int cells_x = 0;
	int cells_y = 0;
	float cell_size = 0.0f;
	Matrix3D transform;
	std::vector<float> heights;
};
