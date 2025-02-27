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

#ifndef __REFREAD
#define __REFREAD 1

#include <string.h>
#include "codex.h"
#include "refcodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

/* check for reasonable header: */
/* 10fb header */

bool GCALL REF_is(const void *compresseddata)
{
    bool ok=false;
    int packtype=ggetm(compresseddata,2);

    if (packtype==0x10fb
     || packtype==0x11fb
     || packtype==0x90fb
     || packtype==0x91fb)
        ok = true;

    return(ok);
}


/****************************************************************/
/*  Decode Functions                                            */
/****************************************************************/

int GCALL REF_size(const void *compresseddata)
{
    int len=0;
    int packtype=ggetm(compresseddata,2);
    int ssize=(packtype&0x8000)?4:3;

    if (packtype&0x100)     /* 11fb */
    {
        len = ggetm((char *)compresseddata+2+ssize,ssize);
    }
    else                    /* 10fb */
    {
        len = ggetm((char *)compresseddata+2,ssize);
    }

    return(len);
}


int GCALL REF_decode(void *dest, const void *compresseddata, int *compressedsize)
{
    unsigned char *s;
    unsigned char *ref;
    unsigned char *d;
    unsigned char first;
    unsigned char second;
    unsigned char third;
    unsigned char forth;
    unsigned int  run;
    unsigned int  type;
    int          ulen;

    s = (unsigned char *) compresseddata;
    d = (unsigned char *) dest;
    ulen = 0L;

    if (s)
    {
        type = *s++;
        type = (type<<8) + *s++;

        if (type&0x8000) /* 4 byte size field */
        {
            if (type&0x100)                       /* skip ulen */
                s += 4;

            ulen = *s++;
            ulen = (ulen<<8) + *s++;
            ulen = (ulen<<8) + *s++;
            ulen = (ulen<<8) + *s++;
        }
        else
        {
            if (type&0x100)                       /* skip ulen */
                s += 3;

            ulen = *s++;
            ulen = (ulen<<8) + *s++;
            ulen = (ulen<<8) + *s++;
        }

        for (;;)
        {
            first = *s++;
            if (!(first&0x80))          /* short form */
            {
                second = *s++;
                run = first&3;
                while (run--)
                    *d++ = *s++;
                ref = d-1 - (((first&0x60)<<3) + second);
                run = ((first&0x1c)>>2)+3-1;
                do
                {
                    *d++ = *ref++;
                } while (run--);
                continue;
            }
            if (!(first&0x40))          /* int form */
            {
                second = *s++;
                third = *s++;
                run = second>>6;
                while (run--)
                    *d++ = *s++;

                ref = d-1 - (((second&0x3f)<<8) + third);

                run = (first&0x3f)+4-1;
                do
                {
                    *d++ = *ref++;
                } while (run--);
                continue;
            }
            if (!(first&0x20))          /* very int form */
            {
                second = *s++;
                third = *s++;
                forth = *s++;
                run = first&3;
                while (run--)
                    *d++ = *s++;

                ref = d-1 - (((first&0x10)>>4<<16) +  (second<<8) + third);

                run = ((first&0x0c)>>2<<8) + forth + 5-1;
                do
                {
                    *d++ = *ref++;
                } while (run--);
                continue;
            }
            run = ((first&0x1f)<<2)+4;  /* literal */
            if (run<=112)
            {
                while (run--)
                    *d++ = *s++;
                continue;
            }
            run = first&3;              /* eof (+0..3 literal) */
            while (run--)
                *d++ = *s++;
            break;
        }
    }
    if (compressedsize)
        *compressedsize = (int)((char *)s-(char *)compresseddata);
    return(ulen);
}

#endif

