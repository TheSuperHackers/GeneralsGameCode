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

/* Copyright (C) Electronic Arts Canada Inc. 1995-2002.  All rights reserved. */


#ifdef __cplusplus
extern "C" {
#endif

#include "codex.h"

/****************************************************************/
/*  HUF Codex                                                   */
/****************************************************************/

/* Information Functions */

CODEXABOUT *GCALL HUFF_about(void);
bool        GCALL HUFF_is(const void *compresseddata);

/* Decode Functions */

int        GCALL HUFF_size(const void *compresseddata);
#ifdef __cplusplus
int        GCALL HUFF_decode(void *dest, const void *compresseddata, int *compressedsize=0);
#else
int        GCALL HUFF_decode(void *dest, const void *compresseddata, int *compressedsize);
#endif

/* Encode Functions */

#ifdef __cplusplus
int        GCALL HUFF_encode(void *compresseddata, const void *source, int sourcesize, int *opts=0);
#else
int        GCALL HUFF_encode(void *compresseddata, const void *source, int sourcesize, int *opts);
#endif

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#ifndef qmin
#define qmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef qmax
#define qmax(a,b) ((a)>(b)?(a):(b))
#endif

#ifdef __cplusplus
}
#endif

