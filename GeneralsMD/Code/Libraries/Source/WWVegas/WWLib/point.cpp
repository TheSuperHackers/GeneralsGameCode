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
 *                     $Archive:: /Commando/Library/Point.CPP                                 $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 1                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   Point2D::Bias_To -- Bias a point into a rectangle.                                        * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef NEVER


#include	"always.h"
#include	"Point.h"
#include	"rect.h"


/*********************************************************************************************** 
 * Point2D::Bias_To -- Bias a point into a rectangle.                                          * 
 *                                                                                             * 
 *    It is often necessary to take a point that is relative to a rectangle and derive a       * 
 *    point that is no longer relative to the rectangle coordiates, yet still have it refer    * 
 *    to the same location.                                                                    * 
 *                                                                                             * 
 * INPUT:   rect  -- The rectangle to bias this point against.                                 * 
 *                                                                                             * 
 * OUTPUT:  Returns with a point in the rectangles coordinate space but still referring to the * 
 *          same location.                                                                     * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/26/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
Point2D const Point2D::Bias_To(Rect const & rect) const
{
	return(Point2D(X + rect.X, Y + rect.Y));
}

#endif
