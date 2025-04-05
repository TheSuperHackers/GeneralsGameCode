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
/*             Huffman - Huffman with Runlength Codex               */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 950108 FB based on huff and ref codex                    */
/*    1.01 950316 FB delta huff and delta delta huff                */
/*    1.02 950317 FB quick table version (used in wing3)            */
/*    1.03 950626 FB allocate context instead of on stack           */
/*    1.04 010608 ID forgot to undelta big size buffer fix          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Reentrant                                                        */
/* Files: hufread.c hufwrite.c hufcodex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/*      *30fb  fb6      huff 6.1     EOF only huff                  */
/*      *31fb  fb6      huff 6.1      '    '   '   composite        */
/*      *32fb  fb6      huff 6.1     EOF only speed                 */
/*      *33fb  fb6      huff 6.1      '    '   '   composite        */
/*      *34fb  fb6      huff 6.1     EOF only acceleration          */
/*      *35fb  fb6      huff 6.1      '    '   '   composite        */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "codex.h"
#include "huffcodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

CODEXABOUT *GCALL HUFF_about(void)
{
    CODEXABOUT *info;

    info = (CODEXABOUT *) galloc(sizeof(CODEXABOUT));
    if (info)
    {
        memset(info, 0, sizeof(CODEXABOUT));

        info->signature       = QMAKEID('H','U','F','F');
        info->size            = sizeof(CODEXABOUT);
        info->version         = 200;    /* codex version number (200) */
        info->decode          = 1;      /* supports decoding */
        info->encode          = 1;      /* supports encoding */
        info->size32          = 0;      /* supports 32 bit size field */
        strcpy(info->versionstr,    "1.04");     /* version # */
        strcpy(info->shorttypestr,  "huff");     /* type */
        strcpy(info->longtypestr,   "Huffman");  /* longtype */
    }
    return(info);
}

