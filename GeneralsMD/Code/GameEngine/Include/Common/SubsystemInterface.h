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

// FILE: SubsystemInterface.h /////////////////////////////////////////////////////////////////////
// Author: Colin Day, October 2001
// Description: Framework for subsystems singletons of the game engine
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SUBSYSTEMINTERFACE_H_
#define __SUBSYSTEMINTERFACE_H_

#include "Common/INI.h"
#include "Common/STLTypedefs.h"

class Xfer;

//-------------------------------------------------------------------------------------------------
/** This is the abstract base class from which all game engine subsytems should derive from.
	* In order to provide consistent behaviors across all these systems, any implementation
	* must obey the rules defined in here
	*
	* Nothing about the subsystems is automatic, this interface does not wrap up automated
	* functions, it is only here to provide a basic interface and rules for behavior
	* all subsystems
	*/
class SubsystemInterface
{

public:

	//-----------------------------------------------------------------------------------------------
	/** - Constructors should initialize any data to a valid state.  That DOES NOT mean
		*		the data has default values (something done in the init() method), only that
		*		nothing is left pointing to garbage, un-initialized memory.  In most cases
		*		this probably means just setting members to zero or NULL.
		*/
	SubsystemInterface();

	//-----------------------------------------------------------------------------------------------
	/** - Free any resources allocated for this class.
		*
		* - DO NOT throw exceptions during any destruction ever, and do not call other
		*		methods that have the possibility of throwing exceptions.  Try to keep it
		*		simple and keep as much work actually inside the destructor as possible.
		*/
	virtual ~SubsystemInterface();

	//-----------------------------------------------------------------------------------------------
	/** - Assign any default values to data required for the class
		*
		* - Allocate any memory and resources needed throughout the lifetime of the class
		*/
	virtual void init() = 0;

	//-----------------------------------------------------------------------------------------------
	/** - Called for all subsystems after all other Subsystems are inited.
		* (allows for initializing inter-system dependencies)
		*/
	virtual void postProcessLoad() { }

	//-----------------------------------------------------------------------------------------------
	/** - Any system should be able to reset all data and go back to an empty state
		*		that is ready to accept a completely new set of data.  Reset() can expect
		*		to be used in the context of resetting the engine in order to start or
		*		load a new game.
		*
		*	- Do NOT free and re-allocate resources needed, where possible reorganize and
		*		re-initialize the resources already allocated.
		*
		*	- After a reset, the system does not need to be in EXACTLY the same state as a
		*		fresh instantiation.  If there are persistent state information for the
		*		system make sure you maintain it while restoring or re-initializing other
		*		transient parts.
		*/
	virtual void reset() = 0;

	//-----------------------------------------------------------------------------------------------
	/** - Update methods are the place to do system per frame processing.  You
		*		should call the system update once each time through the game loop
		*		to service the system.
		*
		*	- Note that currently the GameClient and GameLogic will be updating
		*		at different rates where the logic is running real time, and the
		*		client will adjust how many loops can be done during one server
		*		time slice in order to improve performance on low end machines.
		*/
	virtual void update() = 0;


	virtual void draw( void ){DEBUG_CRASH(("Shouldn't call base class.  jba."));}

#ifdef DUMP_PERF_STATS
	void UPDATE(void);
	void DRAW(void);
	Real getUpdateTime(void) {return m_curUpdateTime;}
	Real getDrawTime(void) {return m_curDrawTime;}
	Bool doDumpUpdate(void) {return m_dumpUpdate;}
	Bool doDumpDraw(void) {return m_dumpDraw;}
	static Real getTotalTime(void) {return s_msConsumed;}
	static void clearTotalTime(void) {s_msConsumed = 0;}
protected:
	static Real s_msConsumed;
	Real m_startTimeConsumed;
	Real m_curUpdateTime;

	Real m_startDrawTimeConsumed;
	Real m_curDrawTime;
	Bool m_dumpUpdate;
	Bool m_dumpDraw;
#else
	inline void UPDATE(void) {update();}
	inline void DRAW(void) {draw();}
#endif
protected:
	AsciiString m_name;
public:
	AsciiString getName(void) {return m_name;}
	void setName(AsciiString name) {m_name = name;}

};

//-------------------------------------------------------------------------------------------------
class SubsystemInterfaceList
{
public:

	SubsystemInterfaceList();
	~SubsystemInterfaceList();

	void initSubsystem(SubsystemInterface* sys, const char* path1, const char* path2, Xfer *pXfer, AsciiString name="");
	void addSubsystem(SubsystemInterface* sys);
	void removeSubsystem(SubsystemInterface* sys);
	void postProcessLoadAll();
	void resetAll();
	void shutdownAll();
#ifdef DUMP_PERF_STATS
 	AsciiString dumpTimesForAll();
#endif

private:

	typedef std::vector<SubsystemInterface*> SubsystemList;
	SubsystemList m_subsystems;
	SubsystemList m_allSubsystems;

};

extern SubsystemInterfaceList* TheSubsystemList;

#endif // __SUBSYSTEMINTERFACE_H_

