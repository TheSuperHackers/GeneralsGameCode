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

#ifndef WW3D2_STRIP_OPTIMIZER_H__
#define WW3D2_STRIP_OPTIMIZER_H__

#include "always.h"


// strip data =
//
// len = number of vertices in strip
// [vertex indices]
// len
// [vertex indices]
// ..

class StripOptimizerClass
{
public:
	static int* Stripify(const int* tris, int tri_count); // Outputs a set of strips
	static int* Combine_Strips(const int* strips, int strip_count);
	static void Optimize_Strip_Order(int* strips, int strip_count); // Sorts strips for optimal access order
	static void Optimize_Triangle_Order(int* tris, int triangle_count); // Sorts triangles (three indices each) into near-optimal access order

	static int Get_Strip_Index_Count(const int* strips, int strips_count);
};

#endif // WW3D2_STRIP_OPTIMIZER_H__
