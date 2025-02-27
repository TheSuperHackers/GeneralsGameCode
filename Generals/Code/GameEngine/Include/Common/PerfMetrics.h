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

// PerfMetrics.h //////////////////////////////////////////////////////////////////////////////////
// Author: John McDonald, Jr August 2002
// Do Not Distribute

#pragma once

// For load timings
enum { PERFMETRICS_LOAD_STARTFRAME = 0 };
enum { PERFMETRICS_LOAD_STOPFRAME = 1 };

// For engine timings
enum { PERFMETRICS_LOGIC_STARTFRAME = 1 };
enum { PERFMETRICS_LOGIC_STOPFRAME = 1000 };


// For showing metrics
enum { PERFMETRICS_BETWEEN_METRICS = 150 };
