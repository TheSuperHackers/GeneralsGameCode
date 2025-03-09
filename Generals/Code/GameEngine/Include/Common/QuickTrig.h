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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: QuickTrig.h ////////////////////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen (adapted by srj)
// Desc:   fast trig
///////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#ifndef __QUICKTRIG_H_
#define __QUICKTRIG_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------

extern Real TheQuickSinTable[];
extern Real TheQuickTanTable[];

// yes, Real. No, really.
extern Real TheQuickSinTableCount;
extern Real TheQuickTanTableCount;

//-----------------------------------------------------------------------------

#define QUARTER_CIRCLE (PI/2)

//-----------------------------------------------------------------------------
// quick magnitude estimation, without the square root
// NOTE, this is a very rough estimate, and may be off by 10% or more, so
// use it only when you don't need accuracy
//-----------------------------------------------------------------------------
inline Real QMag(Real x, Real y, Real z)
{
   Real sTempV;

   Real sMaxV = fabs(x);
   Real sMedV = fabs(y);
   Real sMinV = fabs(z);

   if (sMaxV < sMedV)
   {
      sTempV = sMaxV;
      sMaxV = sMedV;
      sMedV = sTempV;
   }
   
   if (sMaxV < sMinV)
   {
      sTempV = sMaxV;
      sMaxV = sMinV;
      sMinV = sTempV;
   }
   
   sMedV += sMinV;
   sMaxV += (sMedV*0.25f); 
   
   return sMaxV;
}

//-----------------------------------------------------------------------------
// table based trig functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
inline Real QSin(Real a)
{  
   Real angle = a;
   long sgn = 1;

   if (angle < 0)       // DO POSITIVE MATH AND PRESERVE SIGN
   {
      sgn = -1;
      angle = -angle;
   }
   
   while ( angle > (PI)) // MODULATE ANGLE INTO RANGE OF PI
   {
      angle -= PI;
      sgn = -sgn;   
   }
    
   if (angle > PI/2)
   {
      angle = PI - angle; // FLIP
   }

   int index = REAL_TO_INT((angle/QUARTER_CIRCLE) * TheQuickTanTableCount);
   Real x = TheQuickSinTable[index];

   return x * sgn;

  /*
   Real remainder = node - index;
   Real next = TheQuickSinTable[index + 1];
   Real range = next - x;
   Real scalar = remainder / SIN_TABLE_BRACKET;
   Real finetune = scalar * range;
   x += finetune;
   */

}

//-----------------------------------------------------------------------------
inline Real QCos(Real angle)
{  
   return QSin((QUARTER_CIRCLE) - angle);
}

//-----------------------------------------------------------------------------
inline Real QTan(Real angle)
{  
   return TheQuickTanTable[REAL_TO_INT(angle * TheQuickSinTableCount)];
}

//-----------------------------------------------------------------------------
inline Real QCsc(Real angle)
{  
   return 1.0f / QSin(angle);
}

//-----------------------------------------------------------------------------
inline Real QSec(Real angle)
{  
   return 1.0f / QCos(angle);
}

//-----------------------------------------------------------------------------
inline Real QCot(Real angle)
{  
   return 1.0f / QTan(angle);
}

#endif

