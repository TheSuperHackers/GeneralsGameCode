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

#ifndef __REFWRITE
#define __REFWRITE 1

#include <string.h>
#include "codex.h"
#include "refcodex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static unsigned int matchlen(unsigned char *s,unsigned char *d, unsigned int maxmatch)
{
    unsigned int current;

    for (current=0; current<maxmatch && *s++==*d++; ++current)
        ;

    return(current);
}

#define HASH(cptr) (int)((((unsigned int)(unsigned char)cptr[0]<<8) | ((unsigned int)(unsigned char)cptr[2])) ^ ((unsigned int)(unsigned char)cptr[1]<<4))

static int refcompress(unsigned char *from, int len, unsigned char *dest, int maxback, int quick)
{
    unsigned int tlen;
    unsigned int tcost;
//    unsigned int ccost;    // context cost
    unsigned int run;
    unsigned int toffset;
    unsigned int boffset;
    unsigned int blen;
    unsigned int bcost;
    unsigned int mlen;
    unsigned char *tptr;
    unsigned char *cptr;
    unsigned char *to;
    unsigned char *rptr;
    int countliterals=0;
    int countshort=0;
    int countint=0;
    int countvint=0;
    int hash;
    int hoffset;
    int minhoffset;
    int i;
    int *link;
    int *hashtbl;

    to = dest;
    run = 0;
    cptr = rptr = from;

    if ((unsigned int)maxback > (unsigned int)131071)
        maxback = 131071;

	hashtbl = (int *) galloc(65536L*sizeof(int));
	if (!hashtbl)
        return(0);
	link = (int *) galloc(131072L*sizeof(int));
	if (!link)
        return(0);

    memset(hashtbl,-1,65536L*sizeof(int));

    len -= 4;
    while (len>=0)
    {
        boffset = 0;
        blen = 2;
        bcost = 2;
//        ccost = 0;
        mlen = qmin(len,1028);
        tptr=cptr-1;
        hash = HASH(cptr);
        hoffset = hashtbl[hash];
        minhoffset = qmax(cptr-from-131071,0);


        if (hoffset>=minhoffset)
        {
            do
            {
                tptr = from+hoffset;
                if (cptr[blen]==tptr[blen])
                {
                    tlen = matchlen(cptr,tptr,mlen);
                    if (tlen > blen)
                    {
                        toffset = (cptr-1)-tptr;
                        if (toffset<1024 && tlen<=10)       /* two byte int form */
                            tcost = 2;
                        else if (toffset<16384 && tlen<=67) /* three byte int form */
                            tcost = 3;
                        else                                /* four byte very int form */
                            tcost = 4;

                        if (tlen-tcost+4 > blen-bcost+4)
                        {
                            blen = tlen;
                            bcost = tcost;
                            boffset = toffset;
                            if (blen>=1028) break;
                        }
                    }
                }
            } while ((hoffset = link[hoffset&131071]) >= minhoffset);
        }

//        ccost = 0;
//        if ((run<4) && ((run+blen)>=4))
//            ccost = 1;  // extra packet cost to switch out of literal into reference

//        if (bcost>blen || (blen<=2 && bcost==blen && !ccost) || (len<4))
        if (bcost>=blen || len<4)
        {
            hoffset = (cptr-from);
            link[hoffset&131071] = hashtbl[hash];
            hashtbl[hash] = hoffset;

            ++run;
            ++cptr;
            --len;
        }
        else
        {
            while (run>3)                   /* literal block of data */
            {
                tlen = qmin(112,run&~3);
                run -= tlen;
                *to++ = (unsigned char) (0xe0+(tlen>>2)-1);
                memcpy(to,rptr,tlen);
                rptr += tlen;
                to += tlen;
                ++countliterals;
            }
            if (bcost==2)                   /* two byte int form */
            {
                *to++ = (unsigned char) (((boffset>>8)<<5) + ((blen-3)<<2) + run);
                *to++ = (unsigned char) boffset;
                ++countshort;
            }
            else if (bcost==3)              /* three byte int form */
            {
                *to++ = (unsigned char) (0x80 + (blen-4));
                *to++ = (unsigned char) ((run<<6) + (boffset>>8));
                *to++ = (unsigned char) boffset;
                ++countint;
            }
            else                            /* four byte very int form */
            {
                *to++ = (unsigned char) (0xc0 + ((boffset>>16)<<4) + (((blen-5)>>8)<<2) + run);
                *to++ = (unsigned char) (boffset>>8);
                *to++ = (unsigned char) (boffset);
                *to++ = (unsigned char) (blen-5);
                ++countvint;
            }
            if (run)
            {
                memcpy(to, rptr, run);
                to += run;
                run = 0;
            }

            if (quick)
            {
                hoffset = (cptr-from);
                link[hoffset&131071] = hashtbl[hash];
                hashtbl[hash] = hoffset;
                cptr += blen;
            }
            else
            {
                for (i=0; i < (int)blen; ++i)
                {
                    hash = HASH(cptr);
                    hoffset = (cptr-from);
                    link[hoffset&131071] = hashtbl[hash];
                    hashtbl[hash] = hoffset;
                    ++cptr;
                }
            }

            rptr = cptr;
            len -= blen;
        }
    }
    len += 4;
    run += len;
    while (run>3)                       /* no match at end, use literal */
    {
        tlen = qmin(112,run&~3);
        run -= tlen;
        *to++ = (unsigned char) (0xe0+(tlen>>2)-1);
        memcpy(to,rptr,tlen);
        rptr += tlen;
        to += tlen;
    }

    *to++ = (unsigned char) (0xfc+run); /* end of stream command + 0..3 literal */
    if (run)
    {
        memcpy(to,rptr,run);
        to += run;
    }

	gfree(link);
	gfree(hashtbl);
    return(to-dest);
}


/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/

int GCALL REF_encode(void *compresseddata, const void *source, int sourcesize, int *opts)
{
    int    maxback=131072;
    int     quick=0;
    int    plen;
    int    hlen;


    /* simple fb6 header */

    if (sourcesize>0xffffff)  // 32 bit header required
    {
        gputm(compresseddata,   (unsigned int) 0x90fb, 2);
        gputm((char *)compresseddata+2, (unsigned int) sourcesize, 4);
        hlen = 6L;
    }
    else
    {
        gputm(compresseddata,   (unsigned int) 0x10fb, 2);
        gputm((char *)compresseddata+2, (unsigned int) sourcesize, 3);
        hlen = 5L;
    }
    plen = hlen+refcompress((unsigned char *)source, sourcesize, (unsigned char *)compresseddata+hlen, maxback, quick);
    return(plen);
}

#endif

