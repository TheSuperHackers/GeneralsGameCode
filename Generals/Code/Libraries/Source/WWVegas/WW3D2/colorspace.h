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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Colorspace                                                   *
 *                                                                                             *
 *                     $Archive::                                                             $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                      $Author::                                                             $*
 *                                                                                             *
 *                     $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision::                                                             $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef COLORSPACE_H
#define COLORSPACE_H

#include "dx8wrapper.h"
#include <wwmath.h>

void RGB_To_HSV(Vector3 &hsv,const Vector3 &rgb);
void HSV_To_RGB(Vector3 &rgb, const Vector3 &hsv);
void Recolor(Vector3 &rgb, const Vector3 &hsv_shift);

//---------------------------------------------------------------------
// Color Conversions
//---------------------------------------------------------------------

inline void RGB_To_HSV(Vector3 &hsv,const Vector3 &rgb)
// modified from Foley et al. page 592
// converts rgb[0..1] to h [0,360), s and v in [0,1]
// negative h values are to signify undefined
{
	float max=WWMath::Max(rgb.X,rgb.Y);
	max=WWMath::Max(max,rgb.Z);
	float min=WWMath::Min(rgb.X,rgb.Y);
	min=WWMath::Min(min,rgb.Z);

	// value
	hsv.Z=max;

	// saturation
	hsv.Y=(max!=0.0f)?((max-min)/max):0.0f;
	if (hsv.Y==0.0f) hsv.X=-1.0f;
	else
	{
		float delta=max-min;
		if (rgb.X==max)
			hsv.X=(rgb.Y-rgb.Z)/delta;
		else if (rgb.Y==max)
			hsv.X=2.0f+ (rgb.Z-rgb.X)/delta;
		else if (rgb.Z==max)
			hsv.X=4.0f+ (rgb.X-rgb.Y)/delta;
		hsv.X*=60.0f;
		if (hsv.X<0.0f) hsv.X+=360.0f;
	}
}

inline void HSV_To_RGB(Vector3 &rgb, const Vector3 &hsv)
{
	float h=hsv.X;
	float s=hsv.Y;
	float v=hsv.Z;	

	if (hsv.Y==0.0f)	{
		rgb.Set(v,v,v);
	} else	{
		float f,p,q,t;		

		int i;

		if (h==360.0f) h=0.0f;

		h/=60.0f;
		i=WWMath::Floor(h);
		f=h-i;
		p=v*(1.0f-s);
		q=v*(1.0f-(s*f));
		t=v*(1.0f-(s*(1.0f-f)));
		switch (i)	{
		case 0:
			rgb.Set(v,t,p);
			break;
		case 1:
			rgb.Set(q,v,p);
			break;
		case 2:
			rgb.Set(p,v,t);
			break;
		case 3:
			rgb.Set(p,q,v);
			break;
		case 4:
			rgb.Set(t,p,v);
			break;
		case 5:
			rgb.Set(v,p,q);
			break;
		}
	}
}

inline void Recolor(Vector3 &rgb, const Vector3 &hsv_shift)
{
	Vector3 hsv;
	RGB_To_HSV(hsv,rgb);

	// If the Hue has the "undefined flag" (a negative value), this means that the color is pure
	// monochrome. In this case do not shift the hue (it is undefined) or the saturation (it is 0
	// so it cannot be decreased, and increasing it would cause the undefined hue to actually
	// become visible). In this case, we only modify the value.
	if (hsv.X<0.0f) hsv+=Vector3(0.0f,0.0f,hsv_shift.Z);
	else	hsv+=hsv_shift;

	// angular mod
	if (hsv.X<0.0f) hsv.X+=360.0f;
	if (hsv.X>360.0f) hsv.X-=360.0f;
	// clamp saturation and value
	hsv.Y=WWMath::Clamp(hsv.Y,0.0f,1.0f);
	hsv.Z=WWMath::Clamp(hsv.Z,0.0f,1.0f);	
	HSV_To_RGB(rgb,hsv);
}

inline void Recolor(unsigned& rgba, const Vector3 &hsv_shift)
{
	Vector4 rgba_v = DX8Wrapper::Convert_Color(rgba);
	Recolor((Vector3&)rgba_v, hsv_shift);
	rgba = DX8Wrapper::Convert_Color(rgba_v);
}


#endif

