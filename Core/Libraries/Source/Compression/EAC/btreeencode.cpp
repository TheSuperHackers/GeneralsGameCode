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

#ifndef __BTRWRITE
#define __BTRWRITE 1

#include <string.h>
#include "codex.h"
#include "btreecodex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

#define BTREEWORD	 short
#define BTREECODES	 256
#define BTREEBIGNUM	 32000
#define	BTREESLOPAGE 16384

struct BTREEMemStruct
{
	char	*ptr;
	int	len;
};

struct BTreeEncodeContext
{
    unsigned int	packbits;
    unsigned int	workpattern;
    unsigned char  *bufptr;
    unsigned int	ulen;
    unsigned int	masks[17];
    unsigned char	clueq[BTREECODES];
    unsigned char	right[BTREECODES];
    unsigned char	join[BTREECODES];
    unsigned int	plen;
    unsigned char  *bufbase;
    unsigned char  *bufend;
    unsigned char  *buffer;
    unsigned char  *buf1;
    unsigned char  *buf2;
};

static void BTREE_writebits(struct BTreeEncodeContext *EC,
                            struct BTREEMemStruct *dest,
                            unsigned int bitpattern,
                            unsigned int len)
{
	if (len > 16)
	{
		BTREE_writebits(EC,dest,(unsigned int) (bitpattern>>16), len-16);
		BTREE_writebits(EC,dest,(unsigned int) bitpattern, 16);
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


static void BTREE_adjcount(unsigned char *s, unsigned char *bend, BTREEWORD *count)
{

#ifdef __WATCOMC__

    union
    {
        unsigned char           b[4];
        int                     w;
    } i;

    #define	COUNTADJ(j)	i.b[1] = i.b[0]; i.b[0] = *(s+j); ++count[i.w];

    i.w = 0;
	i.b[0] = (unsigned BTREEWORD) *s++;

#else

	unsigned BTREEWORD		i;

    #define COUNTADJ(j)	i = (BTREEWORD)(((i<<8) | *(s+j)));	++*(count+(int)i);

	i = (unsigned BTREEWORD) *s++;

#endif

	bend -= 16;

	if (s<bend)
	{
		do
		{
			COUNTADJ(0);
			COUNTADJ(1);
			COUNTADJ(2);
			COUNTADJ(3);

			COUNTADJ(4);
			COUNTADJ(5);
			COUNTADJ(6);
			COUNTADJ(7);

			COUNTADJ(8);
			COUNTADJ(9);
			COUNTADJ(10);
			COUNTADJ(11);

			COUNTADJ(12);
			COUNTADJ(13);
			COUNTADJ(14);
			COUNTADJ(15);
            s += 16;

		} while (s<bend);
	}

	bend += 16;

	if (s<bend)
	{
		do
		{
			COUNTADJ(0);
            ++s;

		} while (s<bend);
	}
}

/* Clear 128 ints (256 words). Done as ints for speed. */

static void BTREE_clearcount(unsigned char *tryq,BTREEWORD *countbuf)
{
	int		zero=0;
	int					i;
	int					j;
	int				*intptr;

	intptr = (int *) countbuf;
	j = 256;
	do
	{
		if (*tryq)
		{
			*tryq++ = 1;

			i = 8;
			do
			{
				*(intptr+0) = zero;
				*(intptr+1) = zero;
				*(intptr+2) = zero;
				*(intptr+3) = zero;

				*(intptr+4) = zero;
				*(intptr+5) = zero;
				*(intptr+6) = zero;
				*(intptr+7) = zero;

				*(intptr+8) = zero;
				*(intptr+9) = zero;
				*(intptr+10) = zero;
				*(intptr+11) = zero;

				*(intptr+12) = zero;
				*(intptr+13) = zero;
				*(intptr+14) = zero;
				*(intptr+15) = zero;
                intptr += 16;

			} while (--i);
		}
		else
		{
			++tryq;
			intptr += 128;
		}
	} while (--j);
}

static void BTREE_joinnodes(struct BTreeEncodeContext *EC,
                            unsigned char *cluep,
                            unsigned char *rightp,
                            unsigned char *joinp,
                            unsigned int  clue)
{
	unsigned char			*s;
	unsigned char			*d;
	unsigned char			*bend;
	unsigned int			i;

    /* pack file */

    s = EC->bufbase;
	if (s==EC->buf1)
	{
		d = EC->buf2;
	}
	else
    {
		d = EC->buf1;
	}
	EC->bufbase = d;
	bend = EC->bufend;

	*bend = (unsigned char) clue;
	while (s<=bend)
	{
		while (!cluep[(unsigned int) (*d++ = *s++)])
			;

		i = (unsigned int) *(s-1);

		if (cluep[i]==1)
		{	if (*s==rightp[i])
			{	*(d-1) = joinp[i];
				++s;
			}
		}
		else
		{	if (cluep[i]==3)
			{	*(d-1) = (unsigned char) clue;
				*d++ = *(s-1);
			}
			else
				*d++ = *s++;
		}
	}
	EC->bufend = d-2;
}


/* find 48 most common nodes */

static unsigned int	BTREE_findbest(BTREEWORD     *countptr,
                                   unsigned char *tryq,
                                   unsigned int  *bestn,
                                   unsigned int  *bestval,
                                   int			  ratio)
{
	unsigned int				i;
	unsigned int				i1;
	unsigned int				i2;
	unsigned int				bestsize;

	unsigned int				i3;
	unsigned int				l;

	bestsize = 1;
	i = 3;
	l = 0;
	for (i2=0;i2<256;++i2)
	{
		if (tryq[i2])
		{	for (i1=0;i1<256;++i1)
			{
				if (*countptr++>(BTREEWORD)i)
				{
					if (tryq[i1])
					{	i = *(countptr-1);
						i3=bestsize;
						while (bestval[i3-1]<i)
						{
							bestn[i3] = bestn[i3-1];
							bestval[i3] = bestval[i3-1];
							--i3;
						}
						bestn[i3] = l+i1;
						bestval[i3] = i;
						if (bestsize<48)
							++bestsize;
						while (bestval[bestsize-1]<(bestval[1]/ratio))
							--bestsize;
						if (bestsize<48)
							i = bestval[1]/ratio;
						else
							i = bestval[bestsize-1];
					}
				}
			}
		}
		else
			countptr += 256;
		l += 256;
	}
	return(bestsize);
}

static void BTREE_treepack(struct BTreeEncodeContext *EC,
                           struct BTREEMemStruct *dest,
                           unsigned int     passes,
                           unsigned int     multimax,
                           unsigned int     quick,
                          int				 zerosuppress)
{

	unsigned char	*bend;
	unsigned char	*ptr1;
	unsigned char	*treebuf;
	BTREEWORD		*count;
	unsigned int	i;
	unsigned int	i1;

	int				joinnode, leftnode, rightnode;
	int				ratio;
	unsigned int	hlen;
	unsigned int	domore;
	unsigned int	cost, save;
	unsigned int	tcost, tsave;
    unsigned int	clue;

	unsigned int	freeptr;
	unsigned int	count2[BTREECODES];
	unsigned char	tryq[BTREECODES];
	unsigned char	freeq[BTREECODES];

	unsigned int	bestsize;
	unsigned char	bestjoin[BTREECODES];
	unsigned int	bestn[BTREECODES];
	unsigned int	bestval[BTREECODES];

	unsigned int	bt_size;
	unsigned int	bt_node[BTREECODES];
	unsigned int	bt_left[BTREECODES];
	unsigned int	bt_right[BTREECODES];
    unsigned int	sortptr[BTREECODES];

	int			treebufsize;
	int			buf1size;
	int			buf2size;

    // 3/2 allows for worst case, where 2nd most popular
	treebufsize = 65536L*sizeof(BTREEWORD);  /* 131K */
	buf1size = EC->ulen*3/2+(int)BTREESLOPAGE;
	buf2size = EC->ulen*3/2+(int)BTREESLOPAGE;

	treebuf =	(unsigned char *) galloc(treebufsize);
	if (!treebuf)
        return; /* failure Insufficient memory for work buffer */

	EC->buf1 =	(unsigned char *) galloc(buf1size);
	if (!EC->buf1)
        return; /* failure Insufficient memory for work buffer */

	EC->buf2 =	(unsigned char *) galloc(buf2size);
	if (!EC->buf2)
        return; /* failure Insufficient memory for work buffer */

    memcpy(EC->buf1, EC->buffer, EC->ulen); /* copy to scratch buffer */

	EC->buffer = EC->buf1;
	EC->bufptr = EC->buf1+EC->ulen;

	EC->bufbase = EC->buffer;
	EC->bufend = EC->bufptr;

	count = (BTREEWORD *) treebuf;

	if (quick)	ratio = quick;
	else		ratio = 2;

/*** count file ***/

	for (i=0; i<BTREECODES; ++i)
		count2[i] = 0;

	i1 = 0;
	bend = EC->bufptr;
	ptr1 = EC->buffer;
	while (ptr1<bend)
	{	i = (unsigned int) *ptr1++;
		i1 = i1 + i;
		++count2[i];
	}

/* init clue array */

	for (i=0;i<BTREECODES;++i)
		EC->clueq[i] = 0;

/* get a clue byte (forced) */

	for(i=0; i<BTREECODES; ++i)
	{	freeq[i] = 1;
		if (count2[i]>3)
			tryq[i] = 1;
		else
			tryq[i] = 0;
	}

/* don't use 0 for clue or node (so that [clue][0] is reserved for EOF) */

	count2[0] = BTREEBIGNUM;		/* don't use for clue or node */

/* asciiz btree suppress packing of 0..31 */

	if (zerosuppress)
	{
		for (i=0; i<32; ++i)
		{
			count2[i] = BTREEBIGNUM;		/* don't use for nodes */
			tryq[i] = 0;			/* don't try to pack */
			freeq[i] = 0;			/* don't try to pack */
		}
	}


/* sort codes with least used codes first */
/* primary key is count, secondary key is higher code */

/* registers vars usage
    i  - code
    i1 - done flag/swap temp
*/

	for (i=0;i<BTREECODES;++i)
		sortptr[i] = i;

	i1=1;
	while (i1)
	{
		i1 = 0;
		for (i=1; i<BTREECODES; ++i)
		{
			if (count2[sortptr[i]]<count2[sortptr[i-1]] || ((count2[sortptr[i]]==count2[sortptr[i-1]]) && (sortptr[i]>sortptr[i-1])))
			{
				i1 = sortptr[i];
				sortptr[i] = sortptr[i-1];
				sortptr[i-1] = i1;
				i1 = 1;
			}
		}
	}

	freeptr = 0;
	i = sortptr[freeptr++];
	clue = (unsigned char) i;
	freeq[i] = 0;
	tryq[i] = 0;
	EC->clueq[i] = 3;

	if (count2[i])
		BTREE_joinnodes(EC,EC->clueq, EC->right, EC->join, clue);

	EC->clueq[i] = 2;

/* init best array */

	bestval[0] = (unsigned int) -1;


/* count file (pass 2) */

	bt_size = 0;
	domore = passes;
	while (domore)
	{

/* clear count array */

		BTREE_clearcount(tryq,count);

/* do an adjacency count */

		ptr1 = EC->bufbase;
		bend = EC->bufend;

		BTREE_adjcount(ptr1, bend, count);

/* find most common nodes */

		bestsize = BTREE_findbest(count,tryq,bestn,bestval,ratio);

/* decide which nodes should be joined to what */

		domore = 0;
		if (bestsize>1)
		{
			tcost = tsave = 0;
			i = i1 = domore = 1;
			while (domore)
			{
				leftnode = (bestn[i]>>8)&255;
				rightnode = bestn[i]&255;

				if ((tryq[leftnode]==1) && (tryq[rightnode]==1))
				{
					domore = 0;
					while ((freeptr<BTREECODES) && (!freeq[sortptr[freeptr]]))
						++freeptr;

					if (freeptr<BTREECODES)
					{
						joinnode = sortptr[freeptr];

						cost = 3+count2[joinnode];
						save = bestval[i];

						if (cost<save)
						{
							tcost += cost;
							tsave += save;

							bestjoin[i1] = (unsigned char) joinnode;
							bestn[i1] = bestn[i];
							++i1;

							freeq[joinnode] = 0;
							tryq[joinnode] = 2;
							EC->clueq[joinnode] = 3;

							freeq[leftnode] = 0;
							tryq[leftnode] = 2;
							EC->clueq[leftnode] = 1;
							EC->right[leftnode] = (unsigned char) rightnode;
							EC->join[leftnode] =  (unsigned char) joinnode;

							freeq[rightnode] = 0;
							tryq[rightnode] = 2;

							bt_node[bt_size] = joinnode;
							bt_left[bt_size] = leftnode;
							bt_right[bt_size] = rightnode;
							++bt_size;

							if (i1<=multimax)
								domore = 1;
						}
					}
				}

				++i;
				if (i>=bestsize)
					domore = 0;
			}

			bestsize = i1;

			if (bestsize>1)
			{
				/* multijoin nodes */

				BTREE_joinnodes(EC,EC->clueq, EC->right, EC->join, clue);

				/* restore clue table */

				for(i=1;i<bestsize;++i)
				{	leftnode = (bestn[i]>>8)&255;
					joinnode = bestjoin[i];
					EC->clueq[leftnode] = EC->clueq[joinnode] = 0;
				}

				domore = --passes;
			}

		}

	}
	EC->bufptr = EC->bufend;

/* write header */

	BTREE_writebits(EC,dest,(unsigned int) clue, 8);	/* clue byte */
	BTREE_writebits(EC,dest,(unsigned int) bt_size, 8);	/* tree size */

	for (i=0; i<bt_size;++i)
	{	BTREE_writebits(EC,dest,(unsigned int) bt_node[i], 8);
		BTREE_writebits(EC,dest,(unsigned int) bt_left[i], 8);
		BTREE_writebits(EC,dest,(unsigned int) bt_right[i], 8);
	}

	hlen = EC->plen;

/*** write packed file ***/

	ptr1 = EC->bufbase;
	bend = EC->bufend;

	while (ptr1<bend)
		BTREE_writebits(EC,dest,(unsigned int) *ptr1++, 8);

	BTREE_writebits(EC,dest,(unsigned int) clue, 8);
	BTREE_writebits(EC,dest,(unsigned int) 0, 8);

	BTREE_writebits(EC,dest,0L,7);	/* flush bits */

	gfree(EC->buf2);
	gfree(EC->buf1);
	gfree(treebuf);
}

static int BTREE_compressfile(struct BTreeEncodeContext *EC,
                               struct BTREEMemStruct *infile,
                               struct BTREEMemStruct *outfile,
                               int			 ulen,
                               int				 zerosuppress)
{
	unsigned int	i;

	unsigned int	passes;
	unsigned int	multimax;
    int			flen;

/* set defaults */

	EC->packbits = 0;

	EC->workpattern = 0L;
	passes = 256;
	multimax = 32;
	EC->masks[0] = 0;
	for (i=1;i<17;++i)
		EC->masks[i] = (EC->masks[i-1] << 1) + 1;

/* read in a source file */

	EC->buffer = (unsigned char *) (infile->ptr);
	flen = infile->len;

	EC->ulen = flen;
	EC->bufptr = EC->buffer + flen;

/* pack a file */

	outfile->ptr = outfile->ptr;
	outfile->len = 0L;

	EC->packbits = 0;
	EC->workpattern = 0L;
	EC->plen = 0L;

/* write standard header stuff (type/signature/ulen/adjust) */

	/* simple fb6 header */

	if (ulen==infile->len)
	{
		BTREE_writebits(EC,outfile,(unsigned int) 0x46fb, 16);
		BTREE_writebits(EC,outfile,(unsigned int) infile->len, 24);
	}

	/* composite fb6 header */

	else
	{
		BTREE_writebits(EC,outfile,(unsigned int) 0x47fb, 16);
		BTREE_writebits(EC,outfile,(unsigned int) ulen, 24);
		BTREE_writebits(EC,outfile,(unsigned int) infile->len, 24);
	}

	BTREE_treepack(EC,outfile,passes, multimax, 0, zerosuppress);

    return(outfile->len);
}



/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/

int GCALL BTREE_encode(void *compresseddata, const void *source, int sourcesize, int *opts)
{
    int   plen;
    struct BTREEMemStruct infile;
    struct BTREEMemStruct outfile;
    struct BTreeEncodeContext EC;
    int opt=0;
    if (opts)
        opt = opts[0];

    infile.ptr = (char *)source;
    infile.len = sourcesize;
    outfile.ptr = (char *)compresseddata;
    outfile.len = sourcesize;

    plen = BTREE_compressfile(&EC,&infile, &outfile, sourcesize, opt);

    return(plen);
}

#endif

