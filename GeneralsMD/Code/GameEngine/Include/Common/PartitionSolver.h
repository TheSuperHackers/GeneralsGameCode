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
/* File name:  PartitionSolver.h                                             */
/* Created:    John K. McDonald, Jr., 4/2/2002                               */
/* Desc:       General purpose partition problem-solver											 */
/* Revision History:                                                         */
/*		4/12/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/

#pragma once
#ifndef _H_PARTITIONSOLVER_
#define _H_PARTITIONSOLVER_

// INCLUDES ///////////////////////////////////////////////////////////////////

// DEFINES ////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

typedef std::pair<ObjectID, UnsignedInt> PairObjectIDAndUInt;
typedef std::pair<ObjectID, ObjectID> PairObjectID;

// the first unsigned int is the identifying nugget, while the second is the size of the nugget
typedef std::vector<PairObjectIDAndUInt> EntriesVec;

// the first unsigned int is the identifying nugget, while the second is the size of the hole we get to fill
typedef std::vector<PairObjectIDAndUInt> SpacesVec;

// the first ObjectID is the id of the entry, while the second is the id of the hole
typedef std::vector<PairObjectID> SolutionVec;

enum SolutionType CPP_11(: Int)
{
	PREFER_FAST_SOLUTION = 0,
	PREFER_CORRECT_SOLUTION = 0x7FFFFFFF
};

class PartitionSolver
{
	protected:
		SolutionType m_howToSolve;
		EntriesVec m_data;
		SpacesVec m_spacesForData;

		SolutionVec m_currentSolution;
		UnsignedInt m_currentSolutionLeftovers;
		SolutionVec m_bestSolution;

	public:
		PartitionSolver(const EntriesVec& elements, const SpacesVec& spaces, SolutionType solveHow);
		
		// Solve could potentially take a LONG TIME (as in NEVER complete). This problem is NP-complete.
		void solve(void);
		const SolutionVec& getSolution( void ) const;
};

#endif /* _H_PARTITIONSOLVER_ */
