/*
**	Command & Conquer Generals(tm)
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

/* Copyright (C) Electronic Arts Canada Inc. 1994-1998.  All rights reserved. */

/*------------------------------------------------------------------*/
/*                                                                  */
/*           ANSI Standard File and Memory Interface v1.04          */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                     Code Module - May 24, 1995                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* This modules makes an easy basis for new Gimex low level         */
/* functions.  You will need to make the following changes:         */
/*                                                                  */
/* function              what to change                             */
/* --------              --------------                             */
/* galloc/free           put in your memory manager                 */
/* LIBHANDLE.handle      handle for your file system                */
/* gopen/gwopen          fopen, fseek, ftell (find file size)       */
/* gclose                fclose                                     */
/* gread                 fread and possibly memcpy                  */
/* gwrite                fwrite                                     */
/* gseek                 fseek                                      */
/*                                                                  */
/* The other routines should not need changing.                     */
/*                                                                  */
/*------------------------------------------------------------------*/

/* And increase stream buffer */
/*
 setvbuf(f, NULL, _IOFBF, FILE_BUFFER_SIZE);
*/

#define __NOINLINE__ 1
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gimex.h"
#include "Wnd_File.h"

/* Memory Functions */

int galloccount=0;

void * GCALL galloc(long size)
{
    ++galloccount;
    return(malloc((size_t)size));
}

int GCALL gfree(void *memptr)
{
    --galloccount;
    free(memptr);
    return(1);
}

/* get motorola memory */

unsigned long ggetm(void *src, int bytes)
{
    unsigned char *s = (unsigned char *) src;
    unsigned long value;

    value = 0L;
    while (bytes--)
    {
        value = (value<<8) + ((*s++));
    }
    return(value);
}

/* get intel memory */

unsigned long ggeti(void *src, int bytes)
{
    unsigned char *s = (unsigned char *) src;
    int            i = 0;
    unsigned long  value;

    value = 0L;
    while (bytes--)
    {
        value += ((*s++)) << (i);
        i += 8;
    }
    return(value);
}

/* put motorolla memory */

void gputm(void *dst, unsigned long data, int bytes)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned long       pval;

    data <<= (4-bytes)*8;
    while (bytes)
    {
        pval = data >>  24;
        *d++  = (unsigned char) pval;
        data <<= 8;
        --bytes;
    }
}

/* put intel memory */

void gputi(void *dst, unsigned long data, int bytes)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned long   pval;

    while (bytes)
    {
        pval = data;
        *d++  = (unsigned char) pval;
        data >>= 8;
        --bytes;
    }
}

/* File Functions */

GSTREAM * GCALL gopen(const char *filename)
{
    FILE *handle;

	Msg( __LINE__, __FILE__, "gopen:: %s.", filename );

    handle = fopen( filename, "r+b" );

	Msg( __LINE__, __FILE__, "gopen:: handle = %d", (( handle != NULL )? 1 : 0 ));

    if ( !handle ) {

        handle = fopen( filename, "rb" );

		Msg( __LINE__, __FILE__, "gopen:: handle = %d", (( handle != NULL )? 1 : 0 ));
	}
    return((GSTREAM *) handle);
}

GSTREAM * GCALL gwopen(const char *filename)
{
    FILE *handle;

    handle = fopen(filename,"w+b");
    if (!handle)
        handle = fopen(filename,"wb");

    return((GSTREAM *) handle);
}

int GCALL gclose(GSTREAM *g)
{
    int ok=1;
    if (g)
        ok = !fclose((FILE*) g);
    return(ok);
}

int GCALL gread(GSTREAM *g, void *buf, long size)
{
    return(fread(buf, (size_t) 1, (size_t) size, (FILE *) g));
}

int GCALL gwrite(GSTREAM *g, void *buf, long size)
{
    return(fwrite(buf, (size_t)1, (size_t)size, (FILE *) g));
}

int GCALL gseek(GSTREAM *g, long offset)
{
    return(!fseek((FILE *) g, offset, SEEK_SET));
}

long GCALL glen(GSTREAM *g)
{
    long len;
    long oldpos = gtell(g);
    fseek((FILE *)g, 0, SEEK_END);
    len = gtell(g);
    fseek((FILE *)g, oldpos, SEEK_SET);
    return(len);
}

long GCALL gtell(GSTREAM *g)
{
    return(ftell((FILE *) g));
}

