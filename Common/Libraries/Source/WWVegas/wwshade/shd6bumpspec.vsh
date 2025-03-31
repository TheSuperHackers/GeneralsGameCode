//
//	Command & Conquer Generals Zero Hour(tm)
//	Copyright 2025 Electronic Arts Inc.
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// DX6 specular mask vertex shader (no bump)
// Kenny Mitchell - Westwood Studios EA 2002

vs.1.1

#include "shdhw_constants.h"

#include "shd6bumpspec_constants.h"

// In:
//	v0 - object space vertex position
//	v1 - object space normal
//	v2 - color
//	v3 - texture coords

// object space vertex position -> screen (early as possible for view clipping)
dp4 oPos.x, V_POSITION, c[CV_WORLD_VIEW_PROJECTION_0]
dp4 oPos.y, V_POSITION, c[CV_WORLD_VIEW_PROJECTION_1]
dp4 oPos.z, V_POSITION, c[CV_WORLD_VIEW_PROJECTION_2]
dp4 oPos.w, V_POSITION, c[CV_WORLD_VIEW_PROJECTION_3]

dp3 WORLD_NORMAL.x, V_NORMAL, c[CV_WORLD_0]
dp3 WORLD_NORMAL.y, V_NORMAL, c[CV_WORLD_1]
dp3 WORLD_NORMAL.z, V_NORMAL, c[CV_WORLD_2]

// Normalize the world normal vector
dp3 WORLD_NORMAL.w, WORLD_NORMAL, WORLD_NORMAL
rsq WORLD_NORMAL.w, WORLD_NORMAL.w
mul WORLD_NORMAL, WORLD_NORMAL, WORLD_NORMAL.w

// calculate light 0 factor
dp3 LIGHT_0.w, WORLD_NORMAL, c[CV_LIGHT_DIRECTION_0]	// L.N
max LIGHT_0.w, LIGHT_0.w, c[CV_CONST].x				// clamp 0-1
mul LIGHT_0.w, LIGHT_0.w, c[CV_LIGHT_COLOR_0].w		// light attentuation factor

// calculate light 1 factor
dp3 LIGHT_1.w, WORLD_NORMAL, c[CV_LIGHT_DIRECTION_1]	// L.N
max LIGHT_1.w, LIGHT_1.w, c[CV_CONST].x				// clamp 0-1
mul LIGHT_1.w, LIGHT_1.w, c[CV_LIGHT_COLOR_1].w		// light attentuation factor

// calculate light 2 factor
dp3 LIGHT_2.w, WORLD_NORMAL, c[CV_LIGHT_DIRECTION_2]	// L.N
max LIGHT_2.w, LIGHT_2.w, c[CV_CONST].x				// clamp 0-1
mul LIGHT_2.w, LIGHT_2.w, c[CV_LIGHT_COLOR_2].w		// light attentuation factor

// calculate light 3 factor
dp3 LIGHT_3.w, WORLD_NORMAL, c[CV_LIGHT_DIRECTION_3]	// L.N
max LIGHT_3.w, LIGHT_3.w, c[CV_CONST].x				// clamp 0-1
mul LIGHT_3.w, LIGHT_3.w, c[CV_LIGHT_COLOR_3].w		// light attentuation factor

// accumulate light colors
mul COL, c[CV_LIGHT_COLOR_0], LIGHT_0.w
mad COL, c[CV_LIGHT_COLOR_1], LIGHT_1.w, COL
mad COL, c[CV_LIGHT_COLOR_2], LIGHT_2.w, COL
mad COL, c[CV_LIGHT_COLOR_3], LIGHT_3.w, COL

// apply vertex color and diffuse and ambient material terms
mul COL, COL, V_DIFFUSE
mul COL, COL, c[CV_DIFFUSE]
add oD0, COL, c[CV_AMBIENT]


// calculate half angle
// transform vertex position to world space
//  to calculate V, vector to viewer in world
//  space.
dp4 WORLD_VERTEX.x, V_POSITION, c[CV_WORLD_0]
dp4 WORLD_VERTEX.y, V_POSITION, c[CV_WORLD_1]
dp4 WORLD_VERTEX.z, V_POSITION, c[CV_WORLD_2]
//dp4 WORLD_VERTEX.w, V_POSITION, c[CV_WORLD_3]

// Half angle vector is  (L+V)/||L+V||  or Normalize( L+V )
//  ||a|| is magnitude of a
//  L = vec to light from vertex point
//  V = vec to viewer from vertex point

// vertex position - eye position
// eye position - vertex position
add EYE_VECTOR, c[CV_EYE_WORLD], -WORLD_VERTEX.xyz

// Normalize the eye vector
dp3 EYE_VECTOR.w, EYE_VECTOR, EYE_VECTOR
rsq EYE_VECTOR.w, EYE_VECTOR.w
mul EYE_VECTOR, EYE_VECTOR, EYE_VECTOR.w

// Add them to average & create half angle vector
add HALF_ANGLE, c[CV_LIGHT_DIRECTION_0], EYE_VECTOR


// Normalize the half angle vector
dp3 HALF_ANGLE.w, HALF_ANGLE, HALF_ANGLE
rsq HALF_ANGLE.w, HALF_ANGLE.w
mul HALF_ANGLE, HALF_ANGLE, HALF_ANGLE.w

// calculate light 0 factor
dp3 WORLD_NORMAL.x, V_NORMAL, c[CV_WORLD_0]
dp3 WORLD_NORMAL.y, V_NORMAL, c[CV_WORLD_1]
dp3 WORLD_NORMAL.z, V_NORMAL, c[CV_WORLD_2]

// Normalize the world normal vector
dp3 WORLD_NORMAL.w, WORLD_NORMAL, WORLD_NORMAL
rsq WORLD_NORMAL.w, WORLD_NORMAL.w
mul WORLD_NORMAL, WORLD_NORMAL, WORLD_NORMAL.w

dp3 LIGHT_0.w, HALF_ANGLE, WORLD_NORMAL	// N.H
max LIGHT_0.w, LIGHT_0.w, c[CV_CONST].x				// clamp 0-1
mul LIGHT_0.w, LIGHT_0.w, c[CV_LIGHT_COLOR_0].w		// light attentuation factor

// raise to 8th power
mul LIGHT_0.w, LIGHT_0, LIGHT_0
mul LIGHT_0.w, LIGHT_0, LIGHT_0
mul LIGHT_0.w, LIGHT_0, LIGHT_0

// accumulate light colors
mul COL, c[CV_LIGHT_COLOR_0], LIGHT_0.w
 
mul oD1, COL, c[CV_SPECULAR]


mov oT0, V_TEXTURE


