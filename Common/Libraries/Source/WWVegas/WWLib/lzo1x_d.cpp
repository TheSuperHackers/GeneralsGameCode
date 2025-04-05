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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Library/LZO1X_D.CPP                               $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* $Header: /Commando/Library/LZO1X_D.CPP 1     7/22/97 12:00p Greg_h $ */
/* lzo1x_d.c -- standalone LZO1X decompressor

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the LZO library; see the file COPYING.LIB.
   If not, write to the Free Software Foundation, Inc.,
   675 Mass Ave, Cambridge, MA 02139, USA.

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */

#include	"always.h"
#include	"lzo1x.h"
#include	<assert.h>

#if !defined(LZO1X) && !defined(LZO1Y)
#  define LZO1X
#endif

#if 1
#  define TEST_IP				1
#else
#  define TEST_IP				(ip < ip_end)
#endif


/***********************************************************************
// decompress a block of data.
************************************************************************/

int lzo1x_decompress     ( const lzo_byte * in, lzo_uint  in_len,
                                 lzo_byte * out, lzo_uint * out_len,
                                 lzo_voidp )
{
	register lzo_byte *op;
	register const lzo_byte *ip;
	register lzo_uint t;
	register const lzo_byte *m_pos;
	const lzo_byte * const ip_end = in + in_len;

	*out_len = 0;

	op = out;
	ip = in;

	if (*ip > 17) {
		t = *ip++ - 17;
		goto first_literal_run;
	}

	for (;;) {
//	while (TEST_IP) {
		t = *ip++;
		if (t >= 16)
			goto match;
		/* a literal run */
		if (t == 0) {
			t = 15;
			while (*ip == 0) {
				t += 255, ip++;
			}
			t += *ip++;
		}
		/* copy literals */
		*op++ = *ip++; *op++ = *ip++; *op++ = *ip++;
first_literal_run:
		do *op++ = *ip++; while (--t > 0);


		t = *ip++;

		if (t >= 16) {
			goto match;
		}
#if defined(LZO1X)
		m_pos = op - 1 - 0x800;
#elif defined(LZO1Y)
		m_pos = op - 1 - 0x400;
#endif
		m_pos -= t >> 2;
		m_pos -= *ip++ << 2;
		*op++ = *m_pos++;
		*op++ = *m_pos++;
		*op++ = *m_pos;
//		*op++ = *m_pos++;
		goto match_done;


		/* handle matches */
		for (;;) {
//		while (TEST_IP) {
			if (t < 16) {						/* a M1 match */
				m_pos = op - 1;
				m_pos -= t >> 2;
				m_pos -= *ip++ << 2;
				*op++ = *m_pos++;
				*op++ = *m_pos;
//				*op++ = *m_pos++;
			} else {
match:
				if (t >= 64) {				/* a M2 match */
					m_pos = op - 1;
#if defined(LZO1X)
					m_pos -= (t >> 2) & 7;
					m_pos -= *ip++ << 3;
					t = (t >> 5) - 1;
#elif defined(LZO1Y)
					m_pos -= (t >> 2) & 3;
					m_pos -= *ip++ << 2;
					t = (t >> 4) - 3;
#endif
				} else {
					if (t >= 32) {			/* a M3 match */
						t &= 31;
						if (t == 0) {
							t = 31;
							while (*ip == 0) {
								t += 255, ip++;
							}
							t += *ip++;
						}
						m_pos = op - 1;
						m_pos -= *ip++ >> 2;
						m_pos -= *ip++ << 6;
					} else {						/* a M4 match */
						m_pos = op;
						m_pos -= (t & 8) << 11;
						t &= 7;
						if (t == 0) {
							t = 7;
							while (*ip == 0) {
								t += 255, ip++;
							}
							t += *ip++;
						}
						m_pos -= *ip++ >> 2;
						m_pos -= *ip++ << 6;
						if (m_pos == op) {
							goto eof_found;
						}
						m_pos -= 0x4000;
					}
				}
				*op++ = *m_pos++; *op++ = *m_pos++;
				do *op++ = *m_pos++; while (--t > 0);
			}

match_done:
			t = ip[-2] & 3;
			if (t == 0)
				break;
			/* copy literals */
			do *op++ = *ip++; while (--t > 0);
			t = *ip++;
		}
	}

	/* ip == ip_end and no EOF code was found */

	//Unreachable - ST 9/5/96 5:07PM
	//*out_len = op - out;
	//return (ip == ip_end ? LZO_E_EOF_NOT_FOUND : LZO_E_ERROR);

eof_found:
	assert(t == 1);
	*out_len = op - out;
	return (ip == ip_end ? LZO_E_OK : LZO_E_ERROR);
}


/*
vi:ts=4
*/
