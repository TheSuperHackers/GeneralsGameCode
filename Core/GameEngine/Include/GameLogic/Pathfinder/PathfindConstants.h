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

#pragma once

constexpr const Int MAX_WALL_PIECES = 128;
constexpr const Int PATHFIND_QUEUE_LEN = 512;

// how close a unit has to be in z to interact with the layer.
constexpr const Real LAYER_Z_CLOSE_ENOUGH_F = 10.0f;

constexpr const UnsignedInt PATHFIND_CELL_SIZE = 10;
constexpr const Real PATHFIND_CELL_SIZE_F = 10.0f;

constexpr const UnsignedInt ZONE_UPDATE_FREQUENCY = 300;
constexpr const UnsignedInt MAX_CELL_COUNT = 500;
constexpr const UnsignedInt MAX_ADJUSTMENT_CELL_COUNT = 400;
constexpr const UnsignedInt MAX_SAFE_PATH_CELL_COUNT = 2000;

// Number of cells we will search pathfinding per frame.
constexpr const UnsignedInt PATHFIND_CELLS_PER_FRAME = 5000;

constexpr const Int COST_ORTHOGONAL = 10;
constexpr const Int COST_DIAGONAL = 14;
constexpr const Real COST_TO_DISTANCE_FACTOR = 1.0f / 10.0f;
constexpr const Real COST_TO_DISTANCE_FACTOR_SQR = COST_TO_DISTANCE_FACTOR * COST_TO_DISTANCE_FACTOR;

#if RETAIL_COMPATIBLE_PATHFINDING
// TheSuperHackers @info This variable is here so the code will run down the retail compatible path till a failure mode is hit
// The pathfinding will then switch over to the corrected pathfinding code for SH clients
extern Bool s_useFixedPathfinding;
extern Bool s_forceCleanCells;
#endif
