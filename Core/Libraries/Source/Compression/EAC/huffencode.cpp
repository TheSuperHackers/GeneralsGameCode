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

#ifndef __HUFWRITE
#define __HUFWRITE 1

#include <string.h>
#include "codex.h"
#include "huffcodex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

struct HUFFMemStruct
{
	char	*ptr;
	int	len;
};

#define HUFFBIGNUM					32000
#define HUFFTREESIZE				520
#define HUFFCODES					256
#define HUFFMAXBITS					16
#define HUFFREPTBL					252

struct HuffEncodeContext
{
    char            qleapcode[HUFFCODES];
    unsigned int	count[768];
    unsigned int	bitnum[HUFFMAXBITS+1];
    unsigned int	repbits[HUFFREPTBL];
    unsigned int	repbase[HUFFREPTBL];
    unsigned int	tree_left[HUFFTREESIZE];
    unsigned int	tree_right[HUFFTREESIZE];
    unsigned int	bitsarray[HUFFCODES];
    unsigned int	patternarray[HUFFCODES];
    unsigned int	masks[17];
    unsigned int	packbits;
    unsigned int	workpattern;
    unsigned char	*buffer;
    unsigned char	*bufptr;
    int			flen;
    unsigned int	csum;
    unsigned int	mostbits;
    unsigned int	codes;
    unsigned int	chainused;
    unsigned int	clue;
    unsigned int	dclue;
    unsigned int	clues;
    unsigned int	dclues;
    int				mindelta;
    int				maxdelta;
    unsigned int	plen;
    unsigned int	ulen;
    unsigned int	sortptr[HUFFCODES];
};

static void HUFF_deltabytes(const void *source,void *dest,int len)
{
    const unsigned char *s = (const unsigned char *) source;
    unsigned char *d = (unsigned char *) dest;
	unsigned char c;
	unsigned char c1;
	const unsigned char *send;

	c = '\0';
	send = s+len;
	while (s<send)
	{
		c1 = *s++;
		*d++ = (unsigned char)(c1-c);
		c = c1;
	}
}


static void HUFF_writebits(struct HuffEncodeContext *EC,
                    struct HUFFMemStruct *dest,
                    unsigned int	  bitpattern,
                    unsigned int	  len)
{
	if (len > 16)
	{
		HUFF_writebits(EC,dest,(unsigned int) (bitpattern>>16), len-16);
		HUFF_writebits(EC,dest,(unsigned int) bitpattern, 16);
	}
	else
	{
		EC->packbits += len;
		EC->workpattern += (bitpattern & EC->masks[len]) << (24-EC->packbits);
		while (EC->packbits > 7)
		{
			*(dest->ptr+dest->len) = (unsigned char) (EC->workpattern >> 16);
			++dest->len;

			EC->workpattern = EC->workpattern << 8;
			EC->packbits -= 8;
			++EC->plen;
		}
	}
}

static void HUFF_treechase(struct HuffEncodeContext *EC,
                    unsigned int node,
                    unsigned int bits)
{
	if (node < HUFFCODES)
		EC->bitsarray[node] = bits;
	else
	{	HUFF_treechase(EC,EC->tree_left[node], bits+1);
		HUFF_treechase(EC,EC->tree_right[node], bits+1);
	}
}

static void HUFF_maketree(struct HuffEncodeContext *EC)
{
	unsigned int	i, i1;
	unsigned int	ptr1;
	unsigned int	val1, val2;

	unsigned int			ptr2, nodes;

	unsigned int			list_count[HUFFCODES+2];
	unsigned int			list_ptr[HUFFCODES+2];

/* initialize tree */

/* registers vars usage
    i  - code
    i1 - code index (number of unjoined codes)
    i2 -
    i3 -
*/

	nodes = HUFFCODES;
	i1 = 0;
	list_count[i1++] = 0L;
	for (i=0; i<HUFFCODES; ++i)
	{	EC->bitsarray[i] = 99;
		if (EC->count[i])
		{	list_count[i1] = (unsigned int) EC->count[i];
			list_ptr[i1++] = i;
		}
	}
	EC->codes = i1-1;

/* make tree */

/* registers vars usage
    i  - code index/temp
    i1 - number of unjoined codes
    i2 -
    i3 -
*/

	if (i1>2)
	{
		while (i1>2)
		{
			/* get 2 smallest node counts */

			i = i1;
			val1 = list_count[--i];			/* initialize 2 values */
			ptr1 = i;
			val2 = list_count[--i];
			ptr2 = i;
			if (val1 < val2)
			{	val2 = val1;
				ptr2 = ptr1;
				val1 = list_count[i];
				ptr1 = i;
			}

			while (i)
			{	--i;
				while (list_count[i] > val1)
					--i;

				if (i)
				{	val1 = list_count[i];
					ptr1 = i;
					if (val1 <= val2)
					{	val1 = val2;
						ptr1 = ptr2;
						val2 = list_count[i];
						ptr2 = i;
					}
				}
			}

			EC->tree_left[nodes] = list_ptr[ptr1];
			EC->tree_right[nodes] = list_ptr[ptr2];
			list_count[ptr1] = val1 + val2;
			list_ptr[ptr1] = nodes;
			list_count[ptr2] = list_count[--i1];
			list_ptr[ptr2] = list_ptr[i1];
			++nodes;
		}

		/* traverse tree */

		HUFF_treechase(EC,nodes-1, 0);
	}
	else
	{
		/* traverse tree */

		HUFF_treechase(EC,list_ptr[EC->codes], 1);
	}
}

static int HUFF_minrep(struct HuffEncodeContext *EC,
                unsigned int remaining,
                unsigned int r)
{
	int	min, min1, use, newremaining;

	if (r)
	{	min = HUFF_minrep(EC,remaining, r-1);
		if (EC->count[EC->clue+r])
		{	use = remaining/r;
			newremaining = remaining-(use*r);
			min1 = HUFF_minrep(EC,newremaining, r-1)+EC->bitsarray[EC->clue+r]*use;
			if (min1<min)
				min = min1;
		}
	}
	else
	{	min = 0;
		if (remaining)
		{	min = 20;
			if (remaining<HUFFREPTBL)
				min = EC->bitsarray[EC->clue]+3+EC->repbits[remaining]*2;
		}
	}
	return(min);
}

static void HUFF_writenum(struct HuffEncodeContext *EC,
                   struct HUFFMemStruct	*dest,
                   unsigned int		num)
{
	unsigned int	dphuf;
	unsigned int	dbase;

	if (num<HUFFREPTBL)
	{	dphuf = EC->repbits[(unsigned int) num];
		dbase = (unsigned int) EC->repbase[(unsigned int) num];
	}
	else
	{	if (num<508L)
		{	dphuf = 6;
			dbase = 252L;
		}
		else if (num<1020L)
		{	dphuf = 7;
			dbase = 508L;
		}
		else if (num<2044L)
		{	dphuf = 8;
			dbase = 1020L;
		}
		else if (num<4092L)
		{	dphuf = 9;
			dbase = 2044L;
		}
		else if (num<8188L)
		{	dphuf = 10;
			dbase = 4092L;
		}
		else if (num<16380L)
		{	dphuf = 11;
			dbase = 8188L;
		}
		else if (num<32764L)
		{	dphuf = 12;
			dbase = 16380L;
		}
		else if (num<65532L)
		{	dphuf = 13;
			dbase = 32764L;
		}
		else if (num<131068L)
		{	dphuf = 14;
			dbase = 65532L;
		}
		else if (num<262140L)
		{	dphuf = 15;
			dbase = 131068L;
		}
		else if (num<524288L)
		{	dphuf = 16;
			dbase = 262140L;
		}
		else if (num<1048576L)
		{	dphuf = 17;
			dbase = 524288L;
		}
		else
		{	dphuf = 18;
			dbase = 1048576L;
		}
	}
	HUFF_writebits(EC,dest,(unsigned int) 0x00000001, dphuf+1);
	HUFF_writebits(EC,dest,(unsigned int) (num - dbase), dphuf+2);
}

/* write explicite byte ([clue] 0gn [0] [byte]) */

static void HUFF_writeexp(struct HuffEncodeContext *EC,
                  struct HUFFMemStruct *dest,
                  unsigned int code)
{
	HUFF_writebits(EC,dest,EC->patternarray[EC->clue], EC->bitsarray[EC->clue]);
	HUFF_writenum(EC,dest,0L);
	HUFF_writebits(EC,dest,(unsigned int) code, 9);
}

static void HUFF_writecode(struct HuffEncodeContext *EC,
                    struct HUFFMemStruct *dest,
                    unsigned int code)
{
	if (code==EC->clue)
		HUFF_writeexp(EC,dest,code);
	else
		HUFF_writebits(EC,dest,EC->patternarray[code], EC->bitsarray[code]);
}

static void HUFF_init(struct HuffEncodeContext *EC)
{
	unsigned int	i;

/* precalculate repeat field lengths */

/* registers vars usage
    i  - num
    i1 -
    i2 -
    i3 -
*/

	i = 0;
	while (i<4)
	{	EC->repbits[i] = 0;
		EC->repbase[i++] = 0L;
	}
	while (i<12)
	{	EC->repbits[i] = 1;
		EC->repbase[i++] = 4L;
	}
	while (i<28)
	{	EC->repbits[i] = 2;
		EC->repbase[i++] = 12L;
	}
	while (i<60)
	{	EC->repbits[i] = 3;
		EC->repbase[i++] = 28L;
	}
	while (i<124)
	{	EC->repbits[i] = 4;
		EC->repbase[i++] = 60L;
	}
	while (i<252)
	{	EC->repbits[i] = 5;
		EC->repbase[i++] = 124L;
	}
}


static void HUFF_analysis(struct HuffEncodeContext *EC,
                   unsigned int opt,
                   unsigned int chainsaw)
{
	unsigned char			*bptr1;
	unsigned char			*bptr2;
	unsigned int			i;
	unsigned int			i1;
	unsigned int			i2=0;
	unsigned int			i3;

	unsigned int			thres=0;
	int						di;
	unsigned int			pattern;
	unsigned int			rep1;
	unsigned int			repn;
	unsigned int			ncode;
	unsigned int			irep;
	unsigned int			remaining;

	unsigned int			count2[HUFFCODES];
	unsigned int			dcount[HUFFCODES];

/* count file (pass 1) */

/* registers vars usage
    i  - current byte
    i1 - previous byte
    i2 - rep
    i3 - checksum
*/

	for (i=0; i<768; ++i)
		EC->count[i] = 0;

	bptr1 = EC->buffer;
	i1 = 256;
	i3 = 0;
	while (bptr1<EC->bufptr)
	{	i = (unsigned int) *bptr1++;
		i3 = i3 + i;
		if (i == i1)
		{
			i2 = 0;
			bptr2 = bptr1+30000;
			if (bptr2>(EC->bufptr))
				bptr2 = EC->bufptr;

			while ((i == i1) && (bptr1 < bptr2))
			{	++i2;
				i = (unsigned int) *bptr1++;
				i3 = i3 + i;
			}

			if (i2 < 255)
				++EC->count[512+i2];
			else
				++EC->count[512];
		}
		++EC->count[i];
		++EC->count[((i+256-i1)&255)+256];
		i1 = i;
	}
	EC->csum = i3;
	if (!EC->count[512])
		++EC->count[512];

/* find clue bytes */

/* registers vars usage
    i  - code
    i1 - cluebyte
    i2 - clues
    i3 - best forced clue
*/

	EC->clues = 0;
	EC->dclues = 0;
	i3 = 0;
	for (i=0;i<HUFFCODES; ++i)
	{	i1 = i;
		i2 = 0;
		if (EC->count[i]<EC->count[i3])
			i3 = i;
		while (!EC->count[i] && (i<256))
		{	++i2;
			++i;
		}
		if (i2 >= EC->dclues)
		{	EC->dclue = i1;
			EC->dclues = i2;
			if (EC->dclues >= EC->clues)
			{	EC->dclue = EC->clue;
				EC->dclues = EC->clues;
				EC->clue = i1;
				EC->clues = i2;
			}
		}
	}

/* force a clue byte */

	if (opt & 32)
	{	if (!EC->clues)
		{	EC->clues = 1;
			EC->clue = i3;
		}
	}

/* disable & split clue bytes */

	if ((~opt) & 2)
	{	if (EC->clues>1)
			EC->clues = 1;
		if ((~opt) & 1)
			EC->clues = 0;
	}
	if ((~opt) & 4)
		EC->dclues = 0;
	else
	{	if (EC->dclues > 10)
		{	i1 = EC->clue;
			i2 = EC->clues;
			EC->clue = EC->dclue;
			EC->clues = EC->dclues;
			EC->dclue = i1;
			EC->dclues = i2;
		}

		if ((EC->clues*4) < EC->dclues)
		{	EC->clues = EC->dclues/4;
			EC->dclues = EC->dclues-EC->clues;
			EC->clue = EC->dclue+EC->dclues;
		}
	}


/* copy delta clue bytes */

	if (EC->dclues)
	{
		EC->mindelta = -((int)EC->dclues/2);
		EC->maxdelta = EC->dclues+EC->mindelta;
		thres = (int) (EC->ulen/25);

		for (i=1;i<=(unsigned int)EC->maxdelta;++i)
			if (EC->count[256+i] > thres)
				EC->count[EC->dclue+(i-1)*2] = EC->count[256+i];
		for (i=1;i<=(unsigned int)(-EC->mindelta);++i)
			if (EC->count[512-i] > thres)
				EC->count[EC->dclue+(i-1)*2+1] = EC->count[512-i];

/* adjust delta clues */

		for (i=0, i2=0;i<EC->dclues;++i)
			if (EC->count[EC->dclue+i])
				i2 = i;
		di = EC->dclues-i2-1;
		EC->dclues -= di;
		if (EC->clue == (EC->dclue+EC->dclues+di))
		{	EC->clue -= di;
			EC->clues += di;
		}
		EC->mindelta = -((int)EC->dclues/2);
		EC->maxdelta = EC->dclues+EC->mindelta;
	}

/* copy rep clue bytes */

	if (EC->clues)
	{
		for (i=0;i<EC->clues;++i)
			EC->count[EC->clue+i] = EC->count[512+i];
	}

/* make a first approximation tree */

	HUFF_maketree(EC);



/* remove implied rep clues */

/* registers vars usage
    i  - clues
    i1 - minrep
    i2 -
    i3 -
*/

	if (EC->clues>1)
	{
		for (i=1; i<EC->clues; ++i)
		{	i1 = i-1;
			if (i1>8)
				i1 = 8;
			if (EC->count[EC->clue+i])
			{	i1 = HUFF_minrep(EC,i,i1);
				if ((i1 <= EC->bitsarray[EC->clue+i])
					 || (EC->count[EC->clue+i]*(i1-EC->bitsarray[EC->clue+i])<(i/2)))
				{	EC->count[EC->clue+i] = 0;
				}
			}
		}
	}

/* count file (pass 2) */

/* registers vars usage
    i  - current byte
    i1 - previous byte
    i2 - rep
    i3 - rep2
*/

	for (i=0; i<HUFFCODES; ++i)
	{	count2[i] = EC->count[i];
		dcount[i] = 0;
		EC->count[i] = 0;
		EC->count[256+i] = 0;
		EC->count[512+i] = 0;
	}

	i = 1;
	i1 = 256;
	bptr1 = EC->buffer;
	while (bptr1<EC->bufptr)
	{	i = (unsigned int) *bptr1++;
		if (i == i1)
		{
			i2 = 0;
			bptr2 = bptr1+30000;
			if (bptr2>(EC->bufptr))
				bptr2 = EC->bufptr;

			while ((i == i1) && (bptr1 < bptr2))
			{	i = (unsigned int) *bptr1++;
				++i2;
			}

			repn = HUFFBIGNUM;
			irep = HUFFBIGNUM;
			ncode = i2*EC->bitsarray[i1];
			if (EC->clues)
			{
				if (count2[EC->clue])
				{	repn = 20;
					if (i2 < HUFFREPTBL)
						repn = EC->bitsarray[EC->clue]+3+EC->repbits[i2]*2;
				}
				if (EC->clues>1)
				{
					remaining = i2;
					irep = 0;
					i3=EC->clues-1;
					while (i3)
					{	if (count2[EC->clue+i3])
						{	rep1 = remaining/i3;
							irep = irep+rep1*EC->bitsarray[EC->clue+i3];
							remaining = remaining-rep1*i3;
						}
						--i3;
					}
					if (remaining)
						irep=HUFFBIGNUM;
				}
			}

			if ((ncode<=repn) && (ncode<=irep))
				EC->count[i1] += i2;
			else
			{	if (repn < irep)
					++EC->count[EC->clue];
				else
				{
					remaining = i2;
					irep = 0;
					i3=EC->clues-1;
					while (i3)
					{	if (count2[EC->clue+i3])
						{	rep1 = remaining/i3;
							irep = irep+rep1*EC->bitsarray[EC->clue+i3];
							remaining = remaining-rep1*i3;
							EC->count[EC->clue+i3] += rep1;
						}
						--i3;
					}
				}
			}
		}
		if (EC->dclues)
		{	i3 = 0;
			di = i-i1;
			if (di <= EC->maxdelta)
			{	if (di >= EC->mindelta)
				{	di = (i-i1-1)*2+EC->dclue;
					if (i<i1)
						di = (i1-i-1)*2+EC->dclue+1;
					if (count2[di]>thres)
					{	if (count2[i]<4)
							++i3;
						if (EC->bitsarray[di] < EC->bitsarray[i])
							++i3;
						if (EC->bitsarray[di] == EC->bitsarray[i])
							if (EC->count[di] > EC->count[i])
								++i3;
					}
				}
			}
			if (i3)
				++EC->count[di];
			else
				++EC->count[i];
		}
		else
			++EC->count[i];
		i1 = i;
	}

/* force a clue byte */

	if (opt & 32)
		++EC->count[EC->clue];

/* make a second approximation tree */

	HUFF_maketree(EC);

/* chainsaw IV branch clipping algorithm */

/* - maintains perfect tree

   - find intest code
   - find intest branch thats shorter than maximum bits
   - graft one branch to the shorter branch
   - shorten the other code by 1
*/

/* registers vars usage
    i  - code
    i1 - codes lengths
    i2 - int code1 ptr
    i3 - int code2 ptr
*/

	EC->chainused = 0;
	i1 = 99;
	while (i1>chainsaw)
	{	i1 = 0;
		for (i=0; i<HUFFCODES; ++i)					/* find intest code */
		{	if (EC->count[i])
			{	if (EC->bitsarray[i]>=i1)
				{	i3 = i2;
					i2 = i;
					i1 = EC->bitsarray[i];
				}
			}
		}

		if (i1>chainsaw)
		{
			i1 = 0;
			while (i1<HUFFCODES)
			{	if (EC->count[i1])
				{
					if (EC->bitsarray[i1]<chainsaw)
						break;
				}
				++i1;
			}
			for (i=i1; i<HUFFCODES; ++i)				/* find code to graft to */
			{	if (EC->count[i])
				{
					if ((EC->bitsarray[i]<chainsaw) && (EC->bitsarray[i]>EC->bitsarray[i1]))
						i1 = i;
				}
			}

			i = EC->bitsarray[i1]+1;				/* graft to short code */
			EC->bitsarray[i1] = i;
			EC->bitsarray[i2] = i;
			EC->bitsarray[i3] = EC->bitsarray[i3]-1;/* shorten other code by 1 */

			EC->chainused = chainsaw;
			i1 = 99;
		}
	}

/* if huffman inhibited make all codes 8 bits */

/* registers vars usage
    i  - code
    i1 - 8
    i2 -
    i3 -
*/

	if ((~opt) & 8)
	{
		i1 = 8;
		for (i=0; i<HUFFCODES; ++i)
			EC->bitsarray[i] = i1;
	}

/* count bitnums */

/* registers vars usage
    i  - code/bits
    i1 -
    i2 -
    i3 -
*/

	for (i=0; i<=HUFFMAXBITS; ++i)
		EC->bitnum[i] = 0;
	for (i=0; i<HUFFCODES; ++i)
	{	if (EC->bitsarray[i]<=HUFFMAXBITS)
			++EC->bitnum[EC->bitsarray[i]];
	}

/* sort codes */

/* registers vars usage
    i  - next sorted ptr
    i1 - code
    i2 - bits
    i3 - EC->mostbits
*/

	i=0;
	i3=0;
	for (i2=1; i2<=HUFFMAXBITS; ++i2)
	{	if (EC->bitnum[i2])
		{	for (i1=0; i1<HUFFCODES; ++i1)
				if (EC->bitsarray[i1] == i2)
					EC->sortptr[i++] = i1;
			i3 = i2;
		}
	}
	EC->mostbits = i3;

/* assign bit patterns */

/* registers vars usage
    i  - code index
    i1 - code
    i2 - bits
    i3 -
*/

	pattern = 0L;
	i2 = 0;
	for (i=0; i<EC->codes; ++i)
	{	i1 = EC->sortptr[i];
		while (i2 < EC->bitsarray[i1])
		{	++i2;
			pattern = pattern << 1;
		}
		EC->patternarray[i1] = pattern;
		++pattern;
	}
}


static void HUFF_pack(struct HuffEncodeContext *EC,
               struct HUFFMemStruct *dest,
               unsigned int	opt)
{
	unsigned char			*bptr1;
	unsigned char			*bptr2;
	unsigned int			i;
	unsigned int			i1;
	unsigned int			i2;
	unsigned int			i3;
	int						uptype;
	unsigned int			hlen, ibits, rladjust;
	int						di, firstcode, firstbits;
	unsigned int			rep1, repn, ncode, irep, remaining,curpc;

/* write header */

	curpc = 0L;

	uptype = 38;
	rladjust = 1;
	if (uptype==38)
	{
		if (uptype==34)
		{
			HUFF_writenum(EC,dest,(unsigned int) EC->ulen);

			ibits = 0;
			if ((opt & 16) && (!EC->chainused))
				ibits = 1;

			i = 0;									/* write options field */
			if (EC->clues)
				i = 1;
			if (ibits)
				i += 2;
			if (EC->dclues)
				i += 4;
			HUFF_writenum(EC,dest,(unsigned int) i);

			if (EC->clues)
			{	HUFF_writenum(EC,dest,(unsigned int) EC->clue);
				HUFF_writenum(EC,dest,(unsigned int) EC->clues);
			}
			if (EC->dclues)
			{	HUFF_writenum(EC,dest,(unsigned int) EC->dclue);
				HUFF_writenum(EC,dest,(unsigned int) EC->dclues);
			}

			if (!ibits)
				HUFF_writenum(EC,dest,(unsigned int) EC->mostbits);
		}
		else
		{
			HUFF_writebits(EC,dest,(unsigned int) EC->clue, 8);	/* clue */
			rladjust = 0;
		}

		for (i=1; i <= EC->mostbits; ++i)
			HUFF_writenum(EC,dest,(unsigned int) EC->bitnum[i]);

		for (i=0; i<HUFFCODES; ++i)
			EC->qleapcode[i] = 0;

		i = 0;
		i2 = 255;
		firstbits = 0;
		firstcode = -1;
		while (i<EC->codes)
		{
			i1 = EC->sortptr[i];
#if 0
			if (EC->bitsarray[i1]!=firstbits)
			{
				i2 = firstcode;
				firstcode = i1;
				firstbits = EC->bitsarray[i2];
			}
#endif

/* calculate leapfrog delta */

			di = -1;
			do
			{
				i2 = (i2+1)&255;
				if (!EC->qleapcode[i2])
					++di;
			} while (i1!=i2);
			EC->qleapcode[i2] = 1;
			HUFF_writenum(EC,dest,(unsigned int) di);
			++i;
		}
	}
	hlen = EC->plen+1;

	if (!EC->clues)
		EC->clue = HUFFBIGNUM;

/* write packed file */

/* registers vars usage
    i  - current byte
    i1 - previous byte
    i2 - rep
    i3 - rep2
*/

	i = 1;
	i1 = 256;
	bptr1 = EC->buffer;
	while (bptr1<EC->bufptr)
	{	i = (unsigned int) *bptr1++;

		if (i == i1)
		{
			i2 = 0;
			bptr2 = bptr1+30000;
			if (bptr2>(EC->bufptr))
				bptr2 = EC->bufptr;

			while ((i == i1) && (bptr1 < bptr2))
			{	i = (unsigned int) *bptr1++;
				++i2;
			}

			repn = HUFFBIGNUM;
			irep = HUFFBIGNUM;
			ncode = i2*EC->bitsarray[i1];
			if (EC->clues)
			{
				if (EC->count[EC->clue])
				{	repn = 20;
					if (i2 < HUFFREPTBL)
						repn = EC->bitsarray[EC->clue]+3+EC->repbits[i2]*2;
				}
				if (EC->clues>1)
				{
					remaining = i2;
					irep = 0;
					i3=EC->clues-1;
					while (i3)
					{	if (EC->count[EC->clue+i3])
						{	rep1 = remaining/i3;
							irep = irep+rep1*EC->bitsarray[EC->clue+i3];
							remaining = remaining-rep1*i3;
						}
						--i3;
					}
					if (remaining)
						irep = HUFFBIGNUM;
				}
			}

			if ((ncode<=repn) && (ncode<=irep))
			{
				while (i2--)
					HUFF_writecode(EC,dest,i1);
			}
			else
			{	if (repn < irep)
				{
					HUFF_writebits(EC,dest,EC->patternarray[EC->clue], EC->bitsarray[EC->clue]);
					HUFF_writenum(EC,dest,(unsigned int) (i2-rladjust));
				}
				else
				{
					remaining = i2;
					irep = 0;
					i3=EC->clues-1;
					while (i3)
					{	if (EC->count[EC->clue+i3])
						{	rep1 = remaining/i3;
							irep = irep+rep1*EC->bitsarray[EC->clue+i3];
							remaining = remaining-rep1*i3;
							while (rep1--)
								HUFF_writecode(EC,dest,EC->clue+i3);
						}
						--i3;
					}
				}
			}
		}
		i3 = 0;
		if (EC->dclues)
		{
			di = i-i1;
			if ((di <= EC->maxdelta) && (di >= EC->mindelta))
			{	di = (i-i1-1)*2+EC->dclue;
				if (i<i1)
					di = (i1-i-1)*2+EC->dclue+1;
				if (EC->bitsarray[di] < EC->bitsarray[i])
				{	HUFF_writebits(EC,dest,EC->patternarray[di], EC->bitsarray[di]);
					++i3;
				}
			}
		}
		i1 = i;
		if (!i3)
			HUFF_writecode(EC,dest,i);

		if (((long) bptr1- (long) EC->buffer) >= (long)(EC->plen+curpc))
			curpc = (long) bptr1 - (long) EC->buffer - EC->plen;
	}

	/* write EOF ([clue] 0gn [10]) */

	HUFF_writebits(EC,dest,EC->patternarray[EC->clue], EC->bitsarray[EC->clue]);
	HUFF_writenum(EC,dest,0L);
	HUFF_writebits(EC,dest,(unsigned int) 2, 2);

	/* flush bits */

	HUFF_writebits(EC,dest,(unsigned int) 0,7);

	curpc += 2;
}

static int HUFF_packfile(struct HuffEncodeContext *EC,
                   struct HUFFMemStruct	*infile,
                   struct HUFFMemStruct	*outfile,
                   int	ulen,
                   int	deltaed)
{
	unsigned int i;
	unsigned int uptype=0;
	unsigned int chainsaw;
	unsigned int opt;

/* set defaults */

	EC->packbits = 0;
	EC->workpattern = 0L;
	chainsaw = 15;
	EC->masks[0] = 0;
	for (i=1;i<17;++i)
		EC->masks[i] = (EC->masks[i-1] << 1) + 1;

/* initialize huffman vars */

	HUFF_init(EC);

/* read in a source file */

	EC->buffer = (unsigned char *) (infile->ptr);
	EC->flen = infile->len;

	EC->ulen = EC->flen;
	EC->bufptr = EC->buffer + EC->flen;

/* pack a file */

	outfile->ptr = outfile->ptr;
	outfile->len = 0L;

	EC->packbits = 0;
	EC->workpattern = 0L;
	EC->plen = 0L;

	opt = 57 | 49;

	HUFF_analysis(EC,opt, chainsaw);

/* write standard header stuff (type/signature/ulen/adjust) */

    if (ulen>0xffffff)  // 32 bit header required
    {
    	/* simple fb6 header */

    	if (ulen==infile->len)
    	{
    		if (deltaed==0) 		uptype = 0xb0fb;
    		else if (deltaed==1)	uptype = 0xb2fb;
    		else if (deltaed==2)	uptype = 0xb4fb;
    		HUFF_writebits(EC,outfile,(unsigned int) uptype, 16);
    		HUFF_writebits(EC,outfile,(unsigned int) infile->len, 32);
    	}

    	/* composite fb4 header */

    	else
    	{
    		if (deltaed==0) 		uptype = 0xb1fb;
    		else if (deltaed==1)	uptype = 0xb3fb;
    		else if (deltaed==2)	uptype = 0xb5fb;
    		HUFF_writebits(EC,outfile,(unsigned int) uptype, 16);
    		HUFF_writebits(EC,outfile,(unsigned int) ulen, 32);
    		HUFF_writebits(EC,outfile,(unsigned int) infile->len, 32);
    	}
    }
    else
    {
    	/* simple fb6 header */


    	if (ulen==infile->len)
    	{
    		if (deltaed==0) 		uptype = 0x30fb;
    		else if (deltaed==1)	uptype = 0x32fb;
    		else if (deltaed==2)	uptype = 0x34fb;
    		HUFF_writebits(EC,outfile,(unsigned int) uptype, 16);
    		HUFF_writebits(EC,outfile,(unsigned int) infile->len, 24);
    	}

    	/* composite fb4 header */

    	else
    	{
    		if (deltaed==0) 		uptype = 0x31fb;
    		else if (deltaed==1)	uptype = 0x33fb;
    		else if (deltaed==2)	uptype = 0x35fb;
    		HUFF_writebits(EC,outfile,(unsigned int) uptype, 16);
    		HUFF_writebits(EC,outfile,(unsigned int) ulen, 24);
    		HUFF_writebits(EC,outfile,(unsigned int) infile->len, 24);
    	}
    }

	HUFF_pack(EC,outfile, opt);

    return(outfile->len);
}


/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/

int GCALL HUFF_encode(void *compresseddata, const void *source, int sourcesize, int *opts)
{
    int   plen=0;
    struct HUFFMemStruct infile;
    struct HUFFMemStruct outfile;
    struct HuffEncodeContext *EC=nullptr;
    void *deltabuf=nullptr;
    int opt=0;
    if (opts)
        opt = opts[0];

    EC = (struct HuffEncodeContext *)galloc(sizeof(struct HuffEncodeContext));
    if (EC)
    {
        switch (opt)
        {
            default:
            case 0:
                infile.ptr = (char *)source;
                break;

            case 1:
                deltabuf = galloc(sourcesize);
    			HUFF_deltabytes(source,deltabuf,sourcesize);
                infile.ptr = (char *) deltabuf;
                break;

            case 2:
                deltabuf = galloc(sourcesize);
    			HUFF_deltabytes(source,deltabuf,sourcesize);
    			HUFF_deltabytes(deltabuf,deltabuf,sourcesize);
                infile.ptr = (char *) deltabuf;
                break;
        }

        infile.len = sourcesize;
        outfile.ptr = (char *)compresseddata;
        outfile.len = sourcesize;

        plen = HUFF_packfile(EC,&infile, &outfile, sourcesize, opt);

        if (deltabuf) gfree(deltabuf);
        gfree(EC);
    }
    return(plen);
}

#endif

