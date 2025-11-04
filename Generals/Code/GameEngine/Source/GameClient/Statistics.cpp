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

// FILE: Statistics.cpp
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  Statistics.cpp                                                */
/* Created:    John K. McDonald, Jr., 4/2/2002                               */
/* Desc:       Statistical functions should live here                        */
/* Revision History:                                                         */
/*		4/2/2002 : Initial creation                                            */
/*---------------------------------------------------------------------------*/

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/Statistics.h"

// Solution taken from http://www.epanorama.net/documents/telecom/ulaw_alaw.html
Real MuLaw(Real valueToRun, Real maxValueForVal, Real mu)
{
	Real testVal = (valueToRun - maxValueForVal / 2) / (maxValueForVal / 2);
	return (sign(testVal) * log(1 + mu * fabs(testVal)) /
														 log(1 + mu));
}

// from my head. jkmcd
Real Normalize(Real valueToNormalize, Real minRange, Real maxRange)
{
	return ((valueToNormalize - minRange) / (maxRange - minRange));
}

// from my head again. jkmcd
Real NormalizeToRange(Real valueToNormalize, Real minRange, Real maxRange, Real outRangeMin, Real outRangeMax)
{
	return (Normalize(valueToNormalize, minRange, maxRange) * (outRangeMax - outRangeMin)) + outRangeMin;
}
