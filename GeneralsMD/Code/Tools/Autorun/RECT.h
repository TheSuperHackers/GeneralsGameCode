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
 *                      Archive : /Sun/RECT.H                                                  * 
 *                                                                                             * 
 *                       Author : Joe_b                                                        *
 *                                                                                             * 
 *                      Modtime : 11/21/97 4:40p                                               *
 *                                                                                             * 
 *                     Revision : 20                                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   Union -- Combines two rectangles into one larger one.                                     * 
 *   Intersect -- Find the intersection between two rectangles.                                * 
 *   Intersect -- Simple intersect between two rectangles.                                     * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#pragma once

#ifndef RECT_H
#define RECT_H

#include	<stddef.h>
#include	"POINT.h"


/*
**	This class manages a rectangle. Typically, this is used for tracking regions on a surface
**	and for clipping operations. This is a lightweight class in that it defines few support
**	functions and exposes the member variables for direct access.
*/
template<class T>
class TRect
{
	public:
		TRect(void) {}		// Default constructor does nothing by design.
		TRect(T x, T y, T w, T h) : X(x), Y(y), Width(w), Height(h) {}
		TRect(TPoint2D<T> const & point, T w, T h) : X(point.X), Y(point.Y), Width(w), Height(h) {}

		// Equality comparison operators.
		bool operator == (TRect<T> const & rvalue) const {return(X==rvalue.X && Y==rvalue.Y && Width==rvalue.Width && Height==rvalue.Height);}
		bool operator != (TRect<T> const & rvalue) const {return(X!=rvalue.X || Y!=rvalue.Y || Width!=rvalue.Width || Height!=rvalue.Height);}

		// Addition and subtraction operators.
		TRect<T> const & operator += (TPoint2D<T> const & point) {X += point.X;Y += point.Y;return(*this);}
		TRect<T> const & operator -= (TPoint2D<T> const & point) {X -= point.X;Y -= point.Y;return(*this);}
		TRect<T> const operator + (TPoint2D<T> const & point) {return(TRect<T>(Top_Left() + point, Width, Height));}
		TRect<T> const operator - (TPoint2D<T> const & point) {return(TRect<T>(Top_Left() - point, Width, Height));}

		/*
		**	Bias this rectangle within another.
		*/
		TRect<T> const Bias_To(TRect<T> const & rect) const {return(TRect<T>(Top_Left() + rect.Top_Left(), Width, Height));}

		// Assign values
		void Set(T x, T y, T w, T h) {X = x; Y = y; Width = w; Height = h;}

		/*
		**	Determine if two rectangles overlap.
		*/
		bool Is_Overlapping(TRect<T> const & rect) const {return(X < rect.X+rect.Width && Y < rect.Y+rect.Height && X+Width > rect.X && Y+Height > rect.Y);}

		/*
		**	Determine is rectangle is valid.
		*/
		bool Is_Valid(void) const {return(Width > 0 && Height > 0);}
		__declspec(property(get=Is_Valid)) bool IsValid;

		/*
		**	Returns size of rectangle if each discrete location within it is presumed
		**	to be of size 1.
		*/
		int Size(void) const {return(int(Width) * int(Height));}

		/*
		**	Fetch points of rectangle (used as a convenience for the programmer).
		*/
		TPoint2D<T> Top_Left(void) const {return(TPoint2D<T>(X, Y));}
		__declspec(property(get=Top_Left)) TPoint2D<T> TopLeft;

		TPoint2D<T> Top_Right(void) const {return(TPoint2D<T>(T(X + Width - 1), Y));}
		__declspec(property(get=Top_Right)) TPoint2D<T> TopRight;
		
		TPoint2D<T> Bottom_Left(void) const {return(TPoint2D<T>(X, T(Y + Height - 1)));}
		__declspec(property(get=Bottom_Left)) TPoint2D<T> BottomLeft;
		
		TPoint2D<T> Bottom_Right(void) const {return(TPoint2D<T>(T(X + Width - 1), T(Y + Height - 1)));}
		__declspec(property(get=Bottom_Right)) TPoint2D<T> BottomRight;


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
TPoint2D<T> const Bias_To(TPoint2D<T> const & point, TRect<T> const & rect)
{
	return(TPoint2D<T>(T(point.X + rect.X), T(point.Y + rect.Y)));
}


/*********************************************************************************************** 
 * Union -- Combines two rectangles into one larger one.                                       * 
 *                                                                                             * 
 *    This routine will combine the two specified rectangles such that a larger one is         * 
 *    returned that encompasses both rectangles.                                               * 
 *                                                                                             * 
 * INPUT:   rect1 -- One rectangle to combine.                                                 * 
 *          rect2 -- The other rectangle to combine.                                           * 
 *                                                                                             * 
 * OUTPUT:  Returns with the smallest rectangle that encompasses both specified rectangles.    * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/04/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
template<class T>
TRect<T> const Union(TRect<T> const & rect1, TRect<T> const & rect2)
{
	if (rect1.Is_Valid()) {
		if (rect2.Is_Valid()) {
			TRect<T> result = rect1;

			if (result.X > rect2.X) {
				result.Width += T(result.X-rect2.X);
				result.X = rect2.X;
			}
			if (result.Y > rect2.Y) {
				result.Height += T(result.Y-rect2.Y);
				result.Y = rect2.Y;
			}
			if (result.X+result.Width < rect2.X+rect2.Width) {
				result.Width = T(((rect2.X+rect2.Width)-result.X)+1);
			}
			if (result.Y+result.Height < rect2.Y+rect2.Height) {
				result.Height = T(((rect2.Y+rect2.Height)-result.Y)+1);
			}
			return(result);
		}
		return(rect1);
	}
	return(rect2);
}


/*********************************************************************************************** 
 * Intersect -- Find the intersection between two rectangles.                                  * 
 *                                                                                             * 
 *    This routine will take two rectangles and return the intersecting rectangle. It also     * 
 *    tracks how much on rectangle was clipped off of the top and left edges and returns       * 
 *    these values. It can be handy to use these returned clipping values for blit operations  * 
 *    between rectangles.                                                                      * 
 *                                                                                             * 
 * INPUT:   bounding_rect  -- The rectangle of the bounding box (clipping rectangle).          * 
 *                                                                                             * 
 *          draw_rect      -- The rectangle that will be clipped into the bounding rectangle.  * 
 *                                                                                             * 
 *          x,y            -- Place to store the clipping offset performed on the draw_rect.   * 
 *                            If this offset is applied to a subsiquent blit operation from    * 
 *                            the draw_rect source, it will appear to be properly clipped      * 
 *                            against the clipping rectangle rather than offset to the         * 
 *                            clipping rectangle.                                              * 
 *                                                                                             * 
 * OUTPUT:  Returns with the rectangle that is the intersection of the two rectangles.         * 
 *                                                                                             * 
 * WARNINGS:   The returned rectangle may be clipped into nothingness. Check for Is_Valid      * 
 *             to catch this case.                                                             * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/04/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
template<class T>
TRect<T> const Intersect(TRect<T> const & bounding_rect, TRect<T> const & draw_rect, T * x, T * y)
{
	TRect<T> bad_rect(0, 0, 0, 0);			// Dummy (illegal) draw_rect.
	TRect<T> new_draw_rect = draw_rect;		// Working draw_rect.

	/*
	**	Both draw_rects must be valid or else no intersection can occur. In such
	**	a case, return an illegal draw_rect.
	*/
	if (!bounding_rect.Is_Valid() || !draw_rect.Is_Valid()) return(bad_rect);

	/*
	**	The draw_rect spills past the left edge.
	*/
	if (new_draw_rect.X < bounding_rect.X) {
		new_draw_rect.Width -= T(bounding_rect.X - new_draw_rect.X);
		new_draw_rect.X = bounding_rect.X;
	}
	if (new_draw_rect.Width < 1) return(bad_rect);

	/*
	**	The draw_rect spills past top edge.
	*/
	if (new_draw_rect.Y < bounding_rect.Y) {
		new_draw_rect.Height -= T(bounding_rect.Y - new_draw_rect.Y);
		new_draw_rect.Y = bounding_rect.Y;
	}
	if (new_draw_rect.Height < 1) return(bad_rect);

	/*
	**	The draw_rect spills past the right edge.
	*/
	if (new_draw_rect.X + new_draw_rect.Width > bounding_rect.X + bounding_rect.Width) {
		new_draw_rect.Width -= T((new_draw_rect.X + new_draw_rect.Width) - (bounding_rect.X + bounding_rect.Width));
	}
	if (new_draw_rect.Width < 1) return(bad_rect);

	/*
	**	The draw_rect spills past the bottom edge.
	*/
	if (new_draw_rect.Y + new_draw_rect.Height > bounding_rect.Y + bounding_rect.Height) {
		new_draw_rect.Height -= T((new_draw_rect.Y + new_draw_rect.Height) - (bounding_rect.Y + bounding_rect.Height));
	}
	if (new_draw_rect.Height < 1) return(bad_rect);

	/*
	**	Adjust Height relative draw position according to Height new draw_rect
	**	union.
	*/
	if (x != NULL) {
		*x -= T(new_draw_rect.X - draw_rect.X);
	}
	if (y != NULL) {
		*y -= T(new_draw_rect.Y - draw_rect.Y);
	}

	return(new_draw_rect);
}


/*********************************************************************************************** 
 * Intersect -- Simple intersect between two rectangles.                                       * 
 *                                                                                             * 
 *    This will return with the rectangle that represents the intersection of the two          * 
 *    rectangles specified.                                                                    * 
 *                                                                                             * 
 * INPUT:   rect1    -- The first rectangle.                                                   * 
 *                                                                                             * 
 *          rect2    -- The second rectangle.                                                  * 
 *                                                                                             * 
 * OUTPUT:  Returns with the intersecting rectangle between the two rectangles specified.      * 
 *                                                                                             * 
 * WARNINGS:   If there is no valid intersection between the two rectangles, then a rectangle  * 
 *             of illegal value is returned. Check for this case by using the Is_Valid()       * 
 *             function.                                                                       * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/04/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
template<class T>
TRect<T> const Intersect(TRect<T> const & rect1, TRect<T> const & rect2)
{
	return(Intersect(rect1, rect2, (T*)NULL, (T*)NULL));
}


/*
**	This typedef provides an uncluttered type name for a rectangle that
**	is composed of integers.
*/
typedef TRect<int> Rect;

const Rect RECT_NONE(0,0,0,0);

#endif

