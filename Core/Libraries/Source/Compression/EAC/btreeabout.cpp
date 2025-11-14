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
/*                    BTree - Binary Tree Codex                     */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 950108 FB BTree codex based on hufftree and ref codex    */
/*    1.01 970117 FB encode check index before going off array      */
/*    1.02 020716 FB allocate percentage more buffer for large files*/
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Reentrant                                                        */
/* Files: btrread.c btrwrite.c btrcodex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* BTree is an EA proprietary compression scheme by Frank Barchard. */
/* Each byte is either a raw byte (leaf) or node that points to     */
/* 2 other nodes.  Each node is either a simple byte or 2 nodes.    */
/* The stream is simple bytes and uses bytes for nodes that werent  */
/* used in the original file.                                       */
/*                                                                  */
/* BTREE (fb6) header format:                                       */
/* --------------------------                                       */
/*                                                                  */
/*             offset  bytes   notes                                */
/* id          0       2       id is 46fb                           */
/* ulen        2       3       total unpacked len                   */
/* ilen*       2/5     3       unpacked len for this file           */
/* clue        5/8     1                                            */
/* nodes       6/9     1       number of nodes                      */
/* {                                                                */
/*     node    7/10+3n 1                                            */
/*     left    8/11+3n 1                                            */
/*     right   9/12+3n 1                                            */
/* }                                                                */
/*                                                                  */
/* [packed data]                                                    */
/* [explicitely packed last byte]                                   */
/*                                                                  */
/* * present if composite packed                                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "codex.h"
#include "btreecodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

CODEXABOUT *GCALL BTREE_about(void)
{
    CODEXABOUT* info;

    info = (CODEXABOUT*) galloc(sizeof(CODEXABOUT));
    if (info)
    {
        memset(info, 0, sizeof(CODEXABOUT));

        info->signature       = QMAKEID('B','T','R','E');
        info->size            = sizeof(CODEXABOUT);
        info->version         = 200;    /* codex version number (200) */
        info->decode          = 1;      /* supports decoding */
        info->encode          = 1;      /* supports encoding */
        info->size32          = 0;      /* supports 32 bit size field */
        strcpy(info->versionstr,    "1.02");     /* version # */
        strcpy(info->shorttypestr,  "btr");      /* type */
        strcpy(info->longtypestr,   "BTree");    /* longtype */
    }
    return(info);
}

