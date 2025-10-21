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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/coltype.h                              $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 07/01/02 12:45p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * 07/01/02 KM Coltype enum change to avoid MAX conflicts									   *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once


#ifndef COLTYPE_H
#define COLTYPE_H


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Collision 'Types'
//
// This enum defines the collision type bit-field that is used in render object
// collision detection.
//
// The collision type field in a collision or intersection test is used as a
// low-level collision mask.  It will be 'AND'ed with the collision type of
// the render object and will ignore the object unless the result is
// non-zero.  In Commando, we use this to implement separate collision
// representations for "physical" collisions versus "projectile"
// collisions.  I.e. we use a very simple mesh for the character's
// physical collision and a more complex set of meshes for checking whether
// a bullet hits a person.  This masking system is not meant to be a general
// "collision grouping" system.  You should use a higher level system for doing
// things like making bullets ignore each other, etc.
//
// One more wrinkle to the system:  The collision type in the render obj
// will always have the LSB set (COLL_TYPE_ALL) so that you can always
// do queries against every piece of geometry in a render obj if desired.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	COLL_TYPE_ALL	= 0x01,		// perform this test against *EVERYTHING*
	COLL_TYPE_0		= 0x02,		// perform this test against type 0 collision objects
	COLL_TYPE_1		= 0x04,		// perform this test against type 1 collision objects
	COLL_TYPE_2		= 0x08,
	COLL_TYPE_3		= 0x10,
	COLL_TYPE_4		= 0x20,
	COLL_TYPE_5		= 0x40,
	COLL_TYPE_6		= 0x80,

	COLL_TYPE_PHYSICAL	=		COLL_TYPE_0,		// physics collisions
	COLL_TYPE_PROJECTILE =	COLL_TYPE_1,		// projectile collisions
	COLL_TYPE_VIS			=				COLL_TYPE_2,		// "vis node" detection
	COLL_TYPE_CAMERA		=			COLL_TYPE_3,		// camera collision (99% should match physical setting)
	COLL_TYPE_VEHICLE		=		COLL_TYPE_4,		// vehicles will collide with physical and this.
};



#endif

