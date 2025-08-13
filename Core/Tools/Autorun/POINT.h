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
 *                 Project Name: Command & Conquer                                             *
 *                                                                                             *
 *                      Archive: /Sun/Point.h                                                  *
 *                                                                                             *
 *                       Author: Joe_b                                                         *
 *                                                                                             *
 *                      Modtime: 2/02/98 10:09a                                                *
 *                                                                                             *
 *                     Revision: 24                                                            *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#pragma once

#ifndef POINT_H
#define POINT_H

#include <math.h>
//#include "always.h"
//#include "cctypes.h"

//#ifdef __cplusplus
//extern "C"{
//#endif

//#pragma pack(1)

typedef struct Point2DStruct
{
	int X;
	int Y;
}	Point2DStruct;

//#pragma pack()

//#ifdef __cplusplus
//}
//#endif



template<class T> class TRect;

/***********************************************************************************************
**	This class describes a point in 2 dimensional space using arbitrary
**	components. The interpretation of which is outside the scope
**	of this class. This class is the successor to the old style COORDINATE
**	and CELL types but also serves anywhere an X and Y value are treated
**	as a logical object (e.g., pixel location).
*/
template<class T>
class TPoint2D {
	public:
		TPoint2D(void) {}		// Default constructor does nothing by design.
		TPoint2D(T x, T y) : X(x), Y(y) {}

		// Equality comparison operators.
		bool operator == (TPoint2D<T> const & rvalue) const {return(X==rvalue.X && Y==rvalue.Y);}
		bool operator != (TPoint2D<T> const & rvalue) const {return(X!=rvalue.X || Y!=rvalue.Y);}

		// Addition and subtraction operators.
		TPoint2D<T> const & operator += (TPoint2D<T> const & rvalue) {X += rvalue.X;Y += rvalue.Y;return(*this);}
		TPoint2D<T> const & operator -= (TPoint2D<T> const & rvalue) {X -= rvalue.X;Y -= rvalue.Y;return(*this);}
		TPoint2D<T> const operator - (TPoint2D<T> const & rvalue) const {return(TPoint2D<T>(T(X - rvalue.X), T(Y - rvalue.Y)));}
		TPoint2D<T> const operator + (TPoint2D<T> const & rvalue) const {return(TPoint2D<T>(T(X + rvalue.X), T(Y + rvalue.Y)));}

		// Scalar multiplication and division.
		TPoint2D<T> const operator * (T rvalue) const {return(TPoint2D<T>(T(X * rvalue), T(Y * rvalue)));}
		TPoint2D<T> const & operator *= (T rvalue) {X *= rvalue; Y *= rvalue;return(*this);}
		TPoint2D<T> const operator / (T rvalue) const {if (rvalue == T(0)) return(TPoint2D<T>(0,0));return(TPoint2D<T>(T(X / rvalue), T(Y / rvalue)));}
		TPoint2D<T> const & operator /= (T rvalue) {if (rvalue != T(0)) {X /= rvalue;Y /= rvalue;}return(*this);}

		// Dot and cross product.
		TPoint2D<T> const operator * (TPoint2D<T> const & rvalue) const {return(TPoint2D<T>(T(X * rvalue.X), T(Y * rvalue.Y)));}
		T Dot_Product(TPoint2D<T> const & rvalue) const {return((T(X * rvalue.X + Y * rvalue.Y)));}
		TPoint2D<T> const Cross_Product(TPoint2D<T> const & rvalue) const {return(TPoint2D<T>(T(Y - rvalue.Y), T(rvalue.X - X)));}

		// Negation operator -- simple and effective
		TPoint2D<T> const operator - (void) const {return(TPoint2D<T>(T(-X), T(-Y)));}

		// Vector support functions.
		T Length(void) const {return(T(sqrt(double(X*X + Y*Y))));}
		TPoint2D<T> const Normalize(void) const {
			double len = sqrt(X*X + Y*Y);
			if (len != 0.0) {
				return(TPoint2D<T>((T)(X / len), (T)(Y / len)));
			} else {
				return(*this);
			}
		}

		// Find distance between points.
		T Distance_To(TPoint2D<T> const & point) const {return((*this - point).Length());}

	public:
		T X;
		T Y;
};


/***********************************************************************************************
**	This typedef provides an uncluttered type name for use by simple integer points.
*/
class Point2D : public TPoint2D<int>
{
	public:
		Point2D(void) {}		// Default constructor does nothing by design.
		Point2D(int x, int y) : TPoint2D<int>(x, y) {}
		Point2D(Point2DStruct const & rvalue) : TPoint2D<int>(rvalue.X, rvalue.Y) {}
		Point2D(TPoint2D<int> const & rvalue) : TPoint2D<int>(rvalue) {}

		operator Point2DStruct (void) const {Point2DStruct pt;pt.X = X;pt.Y = Y;return(pt);}

		Point2D const & operator += (Point2D const & rvalue) {X += rvalue.X;Y += rvalue.Y;return(*this);}
		Point2D const & operator -= (Point2D const & rvalue) {X -= rvalue.X;Y -= rvalue.Y;return(*this);}
		Point2D const operator - (Point2D const & rvalue) const {return(Point2D(int(X - rvalue.X), int(Y - rvalue.Y)));}
		Point2D const operator + (Point2D const & rvalue) const {return(Point2D(int(X + rvalue.X), int(Y + rvalue.Y)));}
};


template<class T>
T Distance(TPoint2D<T> const & point1, TPoint2D<T> const & point2)
{
	return((point1 - point2).Length());
}


template<class T>
TPoint2D<T> const Cross_Product(TPoint2D<T> const & lvalue, TPoint2D<T> const & rvalue)
{
	return(lvalue.Cross_Product(rvalue));
}


/***********************************************************************************************
**	This describes a point in 3 dimensional space using arbitrary
**	components. This is the successor to the COORDINATE type for those
**	times when height (Z axis) needs to be tracked.
**
**	Notice that it is NOT implemented as a virtually derived class. This
**	is for efficiency reasons. This class chooses to be smaller and faster at the
**	expense of polymorphism. However, since it is publicly derived, inheritance is
**	the next best thing.
*/
template<class T>
class TPoint3D : public TPoint2D<T> {
		typedef TPoint2D<T> BASECLASS;

	public:
		using BASECLASS::X;
		using BASECLASS::Y;

	public:
		TPoint3D(void) {}		// Default constructor does nothing by design.
		TPoint3D(T x, T y, T z) : BASECLASS(x, y), Z(z) {}
		TPoint3D(BASECLASS const & rvalue, T z /*= 0*/) : BASECLASS(rvalue), Z(z) {}

		// Equality comparison operators.
		bool operator == (TPoint3D<T> const & rvalue) const {return(X==rvalue.X && Y==rvalue.Y && Z==rvalue.Z);}
		bool operator != (TPoint3D<T> const & rvalue) const {return(X!=rvalue.X || Y!=rvalue.Y || Z!=rvalue.Z);}

		// Addition and subtraction operators.
		TPoint3D<T> const & operator += (TPoint3D<T> const & rvalue) {X += rvalue.X;Y += rvalue.Y;Z += rvalue.Z;return(*this);}
		TPoint2D<T> const & operator += (TPoint2D<T> const & rvalue) {BASECLASS::operator += (rvalue);return(*this);}
		TPoint3D<T> const & operator -= (TPoint3D<T> const & rvalue) {X -= rvalue.X;Y -= rvalue.Y;Z -= rvalue.Z;return(*this);}
		TPoint2D<T> const & operator -= (TPoint2D<T> const & rvalue) {BASECLASS::operator -= (rvalue);return(*this);}
		TPoint3D<T> const operator - (TPoint3D<T> const & rvalue) const {return(TPoint3D<T>(X - rvalue.X, Y - rvalue.Y, Z - rvalue.Z));}
		TPoint3D<T> const operator - (TPoint2D<T> const & rvalue) const {return(TPoint3D<T>(X - rvalue.X, Y - rvalue.Y, Z));}
		TPoint3D<T> const operator + (TPoint3D<T> const & rvalue) const {return(TPoint3D<T>(X + rvalue.X, Y + rvalue.Y, Z + rvalue.Z));}
		TPoint3D<T> const operator + (TPoint2D<T> const & rvalue) const {return(TPoint3D<T>(X + rvalue.X, Y + rvalue.Y, Z));}

		// Scalar multiplication and division.
		TPoint3D<T> const operator * (T rvalue) const {return(TPoint3D<T>(X * rvalue, Y * rvalue, Z * rvalue));}
		TPoint3D<T> const & operator *= (T rvalue) {X *= rvalue;Y *= rvalue;Z *= rvalue;return(*this);}
		TPoint3D<T> const operator / (T rvalue) const {if (rvalue == T(0)) return(TPoint3D<T>(0,0,0));return(TPoint3D<T>(X / rvalue, Y / rvalue, Z / rvalue));}
		TPoint3D<T> const & operator /= (T rvalue) {if (rvalue != T(0)) {X /= rvalue;Y /= rvalue;Z /= rvalue;}return(*this);}

		// Dot and cross product.
		TPoint3D<T> const operator * (TPoint3D<T> const & rvalue) const {return(TPoint3D<T>(X * rvalue.X, Y * rvalue.Y, Z * rvalue.Z));}
		T Dot_Product(TPoint3D<T> const & rvalue) const {return(T(X * rvalue.X + Y * rvalue.Y + Z * rvalue.Z));}
		TPoint3D<T> const Cross_Product(TPoint3D<T> const & rvalue) const {return TPoint3D<T>(Y * rvalue.Z - Z * rvalue.Y, Z * rvalue.X - X * rvalue.Z, X * rvalue.Y - Y * rvalue.X);}

		// Negation operator -- simple and effective
		TPoint3D<T> const operator - (void) const {return(TPoint3D<T>(-X, -Y, -Z));}

		// Vector support functions.
		T Length(void) const {return(T(sqrt(double(X*X + Y*Y + Z*Z))));}
		TPoint3D<T> const Normalize(void) const {
			double len = sqrt(X*X + Y*Y + Z*Z);
			if (len != 0.0) {
				return(TPoint3D<T>((T)(X / len), (T)(Y / len), (T)(Z / len)));
			} else {
				return(*this);
			}
		}

	public:

		/*
		**	The Z component of this point.
		*/
		T Z;
};


/***********************************************************************************************
**	This typedef provides a simple uncluttered type name for use by
**	integer 3D points.
*/
typedef TPoint3D<int> Point3D;

template<class T>
TPoint3D<T> const Cross_Product(TPoint3D<T> const & lvalue, TPoint3D<T> const & rvalue)
{
	return(lvalue.Cross_Product(rvalue));
}


#endif
