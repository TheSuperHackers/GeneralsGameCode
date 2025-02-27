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
/*  BTR Codex                                                   */
/****************************************************************/

/* Information Functions */

CODEXABOUT *GCALL BTREE_about(void);
bool        GCALL BTREE_is(const void *compresseddata);

/* Decode Functions */

int        GCALL BTREE_size(const void *compresseddata);
#ifdef __cplusplus
int        GCALL BTREE_decode(void *dest, const void *compresseddata, int *compressedsize=0);
#else
int        GCALL BTREE_decode(void *dest, const void *compresseddata, int *compressedsize);
#endif

/* Encode Functions */

#ifdef __cplusplus
int        GCALL BTREE_encode(void *compresseddata, const void *source, int sourcesize, int *opts=0);
#else
int        GCALL BTREE_encode(void *compresseddata, const void *source, int sourcesize, int *opts);
#endif

#ifdef __cplusplus
}
#endif

