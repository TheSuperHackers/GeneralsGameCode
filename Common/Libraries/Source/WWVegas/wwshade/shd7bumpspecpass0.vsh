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

// DX7 bump specular mask vertex shader pass 0
// Kenny Mitchell - Westwood Studios EA 2002

vs.1.1

#include "shdhw_constants.h"

#include "shd7bumpspec_constants.h"

// In:
//	v0 - object space vertex position
//	v1 - object space normal
//	v2 - color
//	v3 - texture coords
//	v4 - S basis
//	v5 - T basis
//	v6 - SxT

// object space vertex position -> screen (early as possible for view clipping)
dp4 oPos.x, V_POSITION, c[CV_WORLD_VIEW_PROJECTION_0]
dp4 oPos.y, V_POSITION, c[CV_WORLD_VIEW_PROJECTION_1]
dp4 oPos.z, V_POSITION, c[CV_WORLD_VIEW_PROJECTION_2]
dp4 oPos.w, V_POSITION, c[CV_WORLD_VIEW_PROJECTION_3]

// Transform basis vectors to world space
dp3 S_WORLD.x, V_S, c[CV_WORLD_0]
dp3 S_WORLD.y, V_S, c[CV_WORLD_1]
dp3 S_WORLD.z, V_S, c[CV_WORLD_2]

dp3 T_WORLD.x, V_T, c[CV_WORLD_0]
dp3 T_WORLD.y, V_T, c[CV_WORLD_1]
dp3 T_WORLD.z, V_T, c[CV_WORLD_2]

dp3 SxT_WORLD.x, V_SxT, c[CV_WORLD_0]
dp3 SxT_WORLD.y, V_SxT, c[CV_WORLD_1]
dp3 SxT_WORLD.z, V_SxT, c[CV_WORLD_2]

// transform light 0 by basis vectors to put it into texture space
dp3 LIGHT_LOCAL.x, S_WORLD.xyz, c[CV_LIGHT_DIRECTION_0]
dp3 LIGHT_LOCAL.y, T_WORLD.xyz, c[CV_LIGHT_DIRECTION_0]
dp3 LIGHT_LOCAL.z, SxT_WORLD.xyz, c[CV_LIGHT_DIRECTION_0]

// Normalize the light vector
dp3 LIGHT_LOCAL.w, LIGHT_LOCAL, LIGHT_LOCAL
rsq LIGHT_LOCAL.w, LIGHT_LOCAL.w
mul LIGHT_LOCAL, LIGHT_LOCAL, LIGHT_LOCAL.w

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
add oD1, COL, c[CV_AMBIENT]


// Scale to 0-1
add LIGHT_LOCAL, LIGHT_LOCAL, c[CV_CONST].yyy
mul LIGHT_LOCAL, LIGHT_LOCAL, c[CV_CONST].zzz

// apply bump scale and bias controls
mul LIGHT_LOCAL, LIGHT_LOCAL, c[CV_BUMPINESS].xxx
add oD0, LIGHT_LOCAL, c[CV_BUMPINESS].yyy

// place negated bump light intensity in alpha channel
add oD0.w, c[CV_CONST].y, -LIGHT_0.w


mov oT0, V_TEXTURE
mov oT1, V_TEXTURE


