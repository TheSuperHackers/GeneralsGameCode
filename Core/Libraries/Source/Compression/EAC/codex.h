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

/*------------------------------------------------------------------*/
/*                                                                  */
/*         Compression Decompression Exchange(CODEX) v2.00          */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                    Header Module - Oct 15, 2001                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 990824 FB codex API seperated from huff tool             */
/*    1.01 010427 FB fb6 32 bit size header                         */
/*    1.02 011011 FB c++ defaults                                   */
/*    2.00 011015 FB bool, dest/source, new about struct,no QPUBLIC */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef __CODEX_H
#define __CODEX_H 1

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define CODEX_VERSION 200

/****************************************************************/
/*  Data Types                                                  */
/****************************************************************/

/* Info structure describing bitmaps */

typedef struct
{
    int signature;              /* signature of codex ie 'tga ' (optional) */
    int size;                   /* size of CODEXABOUT structure */
    int version;                /* version number of CODEXABOUT structure (200) */

    unsigned int decode    :1;  /* supports decoding */
    unsigned int encode    :1;  /* supports encoding */
    unsigned int size32    :1;  /* support 32 bit size field */
    unsigned int pad       :29;

    char versionstr[8];          /* version number of codex module ie 1.00 */
    char shorttypestr[8];        /* 3 or 4 character type string ie ref */
    char longtypestr[16];        /* full name of data format ie Refpack */
} CODEXABOUT;

#define QMAKEID(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

#if !defined(GCALL)
#if defined(_MSC_VER) && !defined(_XBOX)
#define GCALL __stdcall
#else
#define GCALL
#endif
#endif


typedef struct QFUNCTIONS
{
    CODEXABOUT * (GCALL * CODEX_about)(void);
    bool         (GCALL * CODEX_is)(const void *compressed);
    int          (GCALL * CODEX_size)(const void *compressed);
    int          (GCALL * CODEX_decode)(void *dest, const void *source, int *sourcesizeptr);
    int          (GCALL * CODEX_encode)(void *dest, const void *source, int sourcesize, int *opts);
} QFUNCTIONS;

extern struct QFUNCTIONS qfunctions[];

/****************************************************************/
/*  Codex Module Example Prototypes                             */
/****************************************************************/

#include "gimex.h" /* for memory IO */

/* Information Functions */

CODEXABOUT *GCALL CODEX_about(void);
bool        GCALL CODEX_is(const void *source);
int         GCALL CODEX_size(const void *source);

/* Decode/Encode Functions */

#ifdef __cplusplus
int GCALL CODEX_decode(void *dest, const void *source, int *sourcesizeptr=0);
int GCALL CODEX_encode(void *dest, const void *source, int sourcesize, int *opts=0);
#else
int GCALL CODEX_decode(void *dest, const void *source, int *sourcesizeptr);
int GCALL CODEX_encode(void *dest, const void *source, int sourcesize, int *opts);
#endif

#ifdef __cplusplus
}
#endif
#endif

