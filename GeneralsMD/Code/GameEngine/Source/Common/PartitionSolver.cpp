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

// FILE: PartitionSolver.cpp //////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  PartitionSolver.cpp                                           */
/* Created:    John K. McDonald, Jr., 4/2/2002                               */
/* Desc:       This contains a general-purpose Partition solver							 */
/* Revision History:                                                         */
/*		4/12/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/
/**************************************************************************************************
Some info about partioning problems:

	This problem is contained in a very interesting class of problems known as NP complete. The 
	basic problem is that there is no way to tell whether you have an optimal solution or not. 
	Worst case, you try out every possible solution and still don't find the optimal solution: 
	this takes 2^n time to find, where N is the number of elements you are attempting to place.
	For this reason, a value near PREFER_FAST_SOLUTION should almost always be chosen. We will use
	a flat multiply to determine how many solutions to attempt before giving up and returning our 
	best attempt. If you want more info, this site contains info on the problem:
	http://odysseus.nat.uni-magdeburg.de/~mertens/npp/index.shtml
**************************************************************************************************/

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/PartitionSolver.h"

static Bool greater_than(PairObjectIDAndUInt a, PairObjectIDAndUInt b)
{
	return a.second > b.second;
}

PartitionSolver::PartitionSolver(const EntriesVec& elements, const SpacesVec& spaces, SolutionType solveHow)
{
	m_data = elements;
	m_spacesForData = spaces;
	m_howToSolve = solveHow;
	//Added By Sadullah Nader
	//Initializations inserted
	m_currentSolutionLeftovers = 0;
	//
}

void PartitionSolver::solve(void)
{
	m_bestSolution.clear();
	m_currentSolution.clear();
	m_currentSolutionLeftovers = 0x7fffffff;
	
	Int minSizeForAllData = 0;
	Int slotsAllotted = 0;
	size_t i, j;

	// first, determine whether there is an actual solution, or we're going to have to fudge it.
	for (i = 0; i < m_data.size(); ++i) {
		minSizeForAllData += m_data[i].second;
	}

	for (i = 0; i < m_spacesForData.size(); ++i) {
		slotsAllotted += m_spacesForData[i].second;
	}

	// we want to attempt to place the largest things first. This allows us to throw
	// out whole classes of solutions

	std::sort(m_data.begin(), m_data.end(), greater_than);
	
	// Also make the largest partition first.
	std::sort(m_spacesForData.begin(), m_spacesForData.end(), greater_than);

	// work in our temporary vector.
	SpacesVec spacesStillAvailable = m_spacesForData;
	
	if (m_howToSolve == PREFER_FAST_SOLUTION) 
	{
		// we prefer the fast, but not necessarily correct solution
		// simply start placing the stuff. Skip things you can't place.
		for (i = 0; i < m_data.size(); ++i) 
		{
			for (j = 0; j < spacesStillAvailable.size(); ++j) 
			{
				if (m_data[i].second <= spacesStillAvailable[j].second) 
				{
					spacesStillAvailable[j].second -= m_data[i].second;
					m_bestSolution.push_back(std::make_pair(m_data[i].first, spacesStillAvailable[j].first));
					break;
				}
			}
		}
	} else {
		DEBUG_CRASH(("PREFER_CORRECT_SOLUTION @todo impl"));
	}
}

const SolutionVec& PartitionSolver::getSolution( void ) const
{
	return m_bestSolution;
}