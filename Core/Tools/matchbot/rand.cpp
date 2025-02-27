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

#include "rand.h"
#include <cmath>

static const double theMultFactor = 1.0 / (pow(2, 8 * sizeof(unsigned int)) - 1.0);

RandClass::RandClass(int start)
{
	seed[0] = 0xf22d0e56L;
	seed[1] = 0x883126e9L;
	seed[2] = 0xc624dd2fL;
	seed[3] = 0x702c49cL;
	seed[4] = 0x9e353f7dL;
	seed[5] = 0x6fdf3b64L;

	unsigned int ax;

	ax = start;                     /* mov     eax,SEED                     */
	ax += 0xf22d0e56;               /* add     eax,0f22d0e56h               */
	seed[0] = ax;                   /* mov     seed,eax                     */
	ax += 0x883126e9 - 0xf22d0e56;  /* add     eax,0883126e9h-0f22d0e56h    */
	seed[1] = ax;                   /* mov     seed+4,eax                   */
	ax += 0xc624dd2f - 0x883126e9;  /* add     eax,0c624dd2fh-0883126e9h    */
	seed[2] = ax;                   /* mov     seed+8,eax                   */
	ax += 0x0702c49c - 0xc624dd2f;  /* add     eax,00702c49ch-0c624dd2fh    */
	seed[3] = ax;                   /* mov     seed+12,eax                  */
	ax += 0x9e353f7d - 0x0702c49c;  /* add     eax,09e353f7dh-00702c49ch    */
	seed[4] = ax;                   /* mov     seed+16,eax                  */
	ax += 0x6fdf3b64 - 0x9e353f7d;  /* add     eax,06fdf3b64h-09e353f7dh    */
	seed[5] = ax;                   /* mov     seed+20,eax                  */
}

// Add with carry. SUM is replaced with A + B + C, C is replaced with 1  if there was a carry, 0 if there wasn't. A carry occurred if the sum is  less than one of the inputs. This is addition, so carry can never be  more than one.
#define ADC(SUM, A, B, C)   SUM = (A) + (B) + (C); C = ((SUM < (A)) || (SUM < (B)))

unsigned int RandClass::randomValue( void )
{
	unsigned int ax;
	unsigned int c = 0;


	ADC(ax, seed[5], seed[4], c);   /*  mov     ax,seed+20  */
	/*  add     ax,seed+16  */
	seed[4] = ax;                   /*  mov     seed+8,ax   */

	ADC(ax, ax, seed[3], c);        /*  adc     ax,seed+12  */
	seed[3] = ax;                   /*  mov     seed+12,ax  */

	ADC(ax, ax, seed[2], c);        /*  adc     ax,seed+8   */
	seed[2] = ax;                   /*  mov     seed+8,ax   */

	ADC(ax, ax, seed[1], c);        /*  adc     ax,seed+4   */
	seed[1] = ax;                   /*  mov     seed+4,ax   */

	ADC(ax, ax, seed[0], c);        /*  adc     ax,seed+0   */
	seed[0] = ax;                   /*  mov     seed+0,ax   */

	/* Increment seed array, bubbling up the carries. */
	if (!++seed[5])
	{
		if (!++seed[4])
		{
			if (!++seed[3])
			{
				if (!++seed[2])
				{
					if (!++seed[1])
					{
						++seed[0];
						++ax;
					}
				}
			}
		}
	}
	return(ax);
}


int RandClass::Int(void)
{
	return (int)randomValue();
}

int RandClass::Int(int low, int high)
{
	unsigned int delta = high - low + 1;
	int rval;

	if (delta == 0)
		return high;

	rval = ((int)(randomValue()%delta)) + low;

	return rval;
}

double RandClass::Double(void)
{
	return Double(0.0, 1.0);
}

double RandClass::Double(double low, double high)
{
	double delta = high - low;
	double rval;

	if (delta <= 0.0)
		return high;

	rval = ((double)(randomValue()) * theMultFactor) * delta + low;
	return rval;
}


