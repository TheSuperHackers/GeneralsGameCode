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
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/vector2i.h                            $*
 *                                                                                             *
 *                       Author:: Eric Cosky                                                   *
 *                                                                                             *
 *                     $Modtime:: 5/10/01 11:37p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef VECTOR2I_H
#define VECTOR2I_H

#include "always.h"

class Vector2i
{
public:

	int		I;
	int		J;

	WWINLINE Vector2i(void);
	WWINLINE Vector2i(int i,int j);
	
	WWINLINE void Set(int i, int j);

	WWINLINE void Swap(Vector2i & other);

	WWINLINE bool			operator== (const Vector2i & v) const;
	WWINLINE bool			operator!= (const Vector2i& v) const;
	WWINLINE const	int&	operator[] (int n) const;
	WWINLINE int&			operator[] (int n);
};


WWINLINE Vector2i::Vector2i(void)
{
}

WWINLINE Vector2i::Vector2i(int i,int j) 
{ 
	I = i; J = j; 
}

WWINLINE bool Vector2i::operator == (const Vector2i & v) const
{ 
	return (I == v.I && J == v.J );	
}

WWINLINE bool Vector2i::operator !=	(const Vector2i& v) const
{ 
	return !(I == v.I && J == v.J);	
}

WWINLINE const int& Vector2i::operator[] (int n) const				
{ 
	return ((int*)this)[n]; 
}

WWINLINE int& Vector2i::operator[] (int n)
{ 
	return ((int*)this)[n]; 
}

WWINLINE void Vector2i::Set(int i, int j) { I = i; J = j; }

WWINLINE void Vector2i::Swap(Vector2i & other)
{
	// this could use MMX..

	I ^= other.I;
	other.I ^= I;
	I ^= other.I;

	J ^= other.J;
	other.J ^= J;
	J ^= other.J;
}


#endif