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
 *                     $Archive:: /Commando/Code/wwlib/trect.h                                $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/28/00 2:37p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#pragma once

#ifndef TRECT_H
#define TRECT_H

#include	"Point.h"


/*
**	This class manages a rectangle.
*/
template<class T>
class TRect
{
	public:
		TRect(T x=0, T y=0, T w=0, T h=0) : X(x), Y(y), Width(w), Height(h) {}
		TRect(TPoint2D<T> const & point, T w, T h) : X(point.X), Y(point.Y), Width(w), Height(h) {}

		// Equality comparison operators.
		bool operator == (TRect<T> const & rvalue) const {return(X==rvalue.X && Y==rvalue.Y && Width==rvalue.Width && Height==rvalue.Height);}
		bool operator != (TRect<T> const & rvalue) const {return(X!=rvalue.X || Y!=rvalue.Y || Width!=rvalue.Width || Height!=rvalue.Height);}

		// Addition and subtraction operators.
		TRect<T> const & operator += (TPoint2D<T> const & point) {X += point.X;Y += point.Y;return(*this);}
		TRect<T> const & operator -= (TPoint2D<T> const & point) {X -= point.X;Y -= point.Y;return(*this);}
		TRect<T> const operator + (TPoint2D<T> const & point) {return(TRect<T>(X + point.X, Y + point.Y, Width, Height));}
		TRect<T> const operator - (TPoint2D<T> const & point) {return(TRect<T>(X - point.X, Y - point.Y, Width, Height));}

		TRect<T> const Intersect(TRect<T> const & rectangle, T * x=NULL, T * y=NULL) const;
		TRect<T> const Union(TRect<T> const & rect2) const;

		/*
		**	Bias this rectangle within another.
		*/
		TRect<T> const Bias_To(TRect<T> const & rect) const {return(TRect<T>(X + rect.X, Y + rect.Y, Width, Height));}

		/*
		**	Determine if two rectangles overlap.
		*/
		bool Is_Overlapping(TRect<T> const & rect) const {return(X < rect.X+rect.Width && Y < rect.Y+rect.Height && X+Width > rect.X && Y+Height > rect.Y);}

		/*
		**	Determine is rectangle is valid.
		*/
		bool Is_Valid(void) const {return(Width > 0 && Height > 0);}

		/*
		**	Returns size of rectangle if each discrete location within it is presumed
		**	to be of size 1.
		*/
		int Size(void) const {return(int(Width) * int(Height));}

		/*
		**	Fetch points of rectangle (used as a convenience for the programmer).
		*/
		TPoint2D<T> Top_Left(void) const {return(TPoint2D<T>(X, Y));}
		TPoint2D<T> Top_Right(void) const {return(TPoint2D<T>(X + Width - 1, Y));}
		TPoint2D<T> Bottom_Left(void) const {return(TPoint2D<T>(X, Y + Height - 1));}
		TPoint2D<T> Bottom_Right(void) const {return(TPoint2D<T>(X + Width - 1, Y + Height - 1));}

		/*
		**	Determine if a point lies within the rectangle.
		*/
		bool Is_Point_Within(TPoint2D<T> const & point) const {return(point.X >= X && point.X < X+Width && point.Y >= Y && point.Y < Y+Height);}

	public:

		/*
		**	Coordinate of upper left corner of rectangle.
		*/
		T X;
		T Y;

		/*
		**	Dimensions of rectangle. If the width or height is less than or equal to
		**	zero, then the rectangle is in an invalid state.
		*/
		T Width;
		T Height;
};


template<class T>
TRect<T> const TRect<T>::Intersect(TRect<T> const & rectangle, T * x, T * y) const
{
	TRect<T> rect(0, 0, 0, 0);			// Dummy (illegal) rectangle.
	TRect<T> r = rectangle;				// Working rectangle.

	/*
	**	Both rectangles must be valid or else no intersection can occur. In such
	**	a case, return an illegal rectangle.
	*/
	if (!Is_Valid() || !rectangle.Is_Valid()) return(rect);

	/*
	**	The rectangle spills past the left edge.
	*/
	if (r.X < X) {
		r.Width -= X - r.X;
		r.X = X;
	}
	if (r.Width < 1) return(rect);

	/*
	**	The rectangle spills past top edge.
	*/
	if (r.Y < Y) {
		r.Height -= Y - r.Y;
		r.Y = Y;
	}
	if (r.Height < 1) return(rect);

	/*
	**	The rectangle spills past the right edge.
	*/
	if (r.X + r.Width > X + Width) {
		r.Width -= (r.X + r.Width) - (X + Width);
	}
	if (r.Width < 1) return(rect);

	/*
	**	The rectangle spills past the bottom edge.
	*/
	if (r.Y + r.Height > Y + Height) {
		r.Height -= (r.Y + r.Height) - (Y + Height);
	}
	if (r.Height < 1) return(rect);

	/*
	**	Adjust Height relative draw position according to Height new rectangle
	**	union.
	*/
	if (x != NULL) {
		*x -= (r.X-X);
	}
	if (y != NULL) {
		*y -= (r.Y-Y);
	}

	return(r);
}


template<class T>
TRect<T> const TRect<T>::Union(TRect<T> const & rect2) const
{
	if (Is_Valid()) {
		if (rect2.Is_Valid()) {
			TRect<T> result = *this;

			if (result.X > rect2.X) {
				result.Width += result.X-rect2.X;
				result.X = rect2.X;
			}
			if (result.Y > rect2.Y) {
				result.Height += result.Y-rect2.Y;
				result.Y = rect2.Y;
			}
			if (result.X+result.Width < rect2.X+rect2.Width) {
				result.Width = ((rect2.X+rect2.Width)-result.X)+1;
			}
			if (result.Y+result.Height < rect2.Y+rect2.Height) {
				result.Height = ((rect2.Y+rect2.Height)-result.Y)+1;
			}
			return(result);
		}
		return(*this);
	}
	return(rect2);
}


template<class T>
TPoint2D<T> const TPoint2D<T>::Bias_To(TRect<T> const & rect) const
{
	return(TPoint2D<T>(X + rect.X, Y + rect.Y));
}


/*
**	This typedef provides an uncluttered type name for a rectangle that
**	is composed of integers.
*/
typedef TRect<int> Rect;


#endif

