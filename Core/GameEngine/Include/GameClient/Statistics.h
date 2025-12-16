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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Statistics.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  Statistics.h                                                  */
/* Created:    John K. McDonald, Jr., 4/2/2002                               */
/* Desc:       Common statistical functions                                  */
/* Revision History:                                                         */
/*		4/2/2002 : Initial creation                                            */
/*---------------------------------------------------------------------------*/

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"

// DEFINES ////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS ///////////////////////////////////////////////////////


// valueToRun is the value (between 0 and maxValueForVal) to test.
// mu is designates the steepness of the curve.
// The return value is a value in [-1, 1]
extern Real MuLaw(Real valueToRun, Real maxValueForVal, Real mu);

// valueToNormalize is a value in minRange..maxRange
// minRange is the smallest value the range contains
// maxRange is the largest value the range contains
// the return is a value in [0, 1].
extern Real Normalize(Real valueToNormalize, Real minRange, Real maxRange);

// same as Normalize, except that output will be in the range [outRangeMin, outRangeMax]
extern Real NormalizeToRange(Real valueToNormalize, Real minRange, Real maxRange, Real outRangeMin, Real outRangeMax);
