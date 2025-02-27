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

// Copyright (C) Electronic Arts Canada Inc. 1995-2002.  All rights reserved.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               RefPack - Backward Reference Codex                 */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    0.10 941010 FB First codex prototype module                   */
/*    0.90 941019 FB #includable                                    */
/*    1.00 950108 FB frozen API at 1.00                             */
/*    1.01 010426 FB 32 size field                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* used hash table and link table for speed                         */
/* Reentrant                                                        */
/* Files: refread.c refwrite.c refcodex.h (refmatch.asm pc)         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* refpack is a sliding window (131k) lzss method, with byte        */
/* oriented coding.                                                 */
/*                                                                  */
/* huff fb6 style header:                                           */
/*      *10fb  fb6      refpack 1.0  reference pack                 */
/*      *90fb  fb6      refpack 1.01 32 bit reference pack          */
/*                                                                  */
/*                                                                  */
/* header:                                                          */
/* [10fb] [totalunpacksize] [nextunpacksize]                        */
/*   2         3                  3                                 */
/* [90fb] [totalunpacksize] [nextunpacksize]                        */
/*   2         4                  4                                 */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* format is:                                                       */
/* ----------                                                       */
/* 0ffnnndd_ffffffff          short ref, f=0..1023,n=3..10,d=0..3   */
/* 10nnnnnn_ddffffff_ffffffff int ref, f=0..16384,n=4..67,d=0..3   */
/* 110fnndd_f.._f.._nnnnnnnn  very int,f=0..131071,n=5..1028,d=0..3*/
/* 111ddddd                   literal, d=4..112                     */
/* 111111dd                   eof, d=0..3                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "codex.h"
#include "refcodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

CODEXABOUT *GCALL REF_about(void)
{
    CODEXABOUT *info;

    info = (CODEXABOUT *) galloc(sizeof(CODEXABOUT));
    if (info)
    {
        memset(info, 0, sizeof(CODEXABOUT));

        info->signature       = QMAKEID(0,'R','E','F');
        info->size            = sizeof(CODEXABOUT);
        info->version         = 200;    /* codex version number (200) */
        info->decode          = 1;      /* supports decoding */
        info->encode          = 1;      /* supports encoding */
        info->size32          = 1;      /* supports 32 bit size field */
        strcpy(info->versionstr,    "1.01");     /* version # */
        strcpy(info->shorttypestr,  "ref");      /* type */
        strcpy(info->longtypestr,   "Refpack");    /* longtype */
    }
    return(info);
}

