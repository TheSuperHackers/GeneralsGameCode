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

#ifndef __BTRREAD
#define __BTRREAD 1

#include <string.h>
#include "codex.h"
#include "btreecodex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

struct BTreeDecodeContext
{
    signed char    cluetbl[256];
    unsigned char  left[256];
    unsigned char  right[256];
    unsigned char  *d;
};

static void BTREE_chase(struct BTreeDecodeContext *DC, unsigned char node)
{
    if (DC->cluetbl[node])
    {
        BTREE_chase(DC,DC->left[node]);
        BTREE_chase(DC,DC->right[node]);
        return;
    }
    *DC->d++ = node;
}

static int BTREE_decompress(unsigned char *packbuf,unsigned char *unpackbuf)
{
    int  node;
    int  i;
    int  nodes;
    int  clue;
    int ulen;
    unsigned char *s;
    signed char c;
    unsigned int type;
    struct BTreeDecodeContext DC;

    s = packbuf;
    DC.d = unpackbuf;
    ulen = 0L;

    if (s)
    {
        type = ggetm(s,2);
        s += 2;

        /* (skip nothing for 0x46fb) */
        if (type==0x47fb)                       /* skip ulen */
            s += 3;

        ulen = ggetm(s,3);
        s += 3;

        for (i=0;i<256;++i)                     /* 0 means a code is a leaf */
            DC.cluetbl[i] = 0;

        clue = *s++;
        DC.cluetbl[clue] = 1;                   /* mark clue as special */

        nodes = *s++;
        for (i=0;i<nodes;++i)
        {   node = *s++;
            DC.left[node]  = *s++;
            DC.right[node] = *s++;
            DC.cluetbl[node] = (signed char)-1;
        }

        for (;;)
        {
            node = (int) *s++;
            c=DC.cluetbl[node];
            if (!c)
            {
                *DC.d++ = (unsigned char) node;
                continue;
            }
            if (c<0)
            {
                BTREE_chase(&DC,DC.left[node]);
                BTREE_chase(&DC,DC.right[node]);
                continue;
            }
            node = (int) *s++;
            if (node)
            {
                *DC.d++ = (char) node;
                continue;
            }
            break;
        }
    }
    return(ulen);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

/* check for reasonable header: */
/* 46fb header */

bool GCALL BTREE_is(const void *compresseddata)
{
    bool ok=false;

    if (ggetm(compresseddata,2)==0x46fb
     || ggetm(compresseddata,2)==0x47fb)
        ok = true;

    return(ok);
}


/****************************************************************/
/*  Decode Functions                                            */
/****************************************************************/

int GCALL BTREE_size(const void *compresseddata)
{
    int len=0;

    if (ggetm(compresseddata,2)==0x46fb)
    {
        len = ggetm((char *)compresseddata+2,3);
    }
    else
    {
        len = ggetm((char *)compresseddata+2+3,3);
    }

    return(len);
}

int GCALL BTREE_decode(void *dest, const void *compresseddata, int *compressedsize)
{
    return(BTREE_decompress((unsigned char *)compresseddata,(unsigned char *)dest));
}

#endif

