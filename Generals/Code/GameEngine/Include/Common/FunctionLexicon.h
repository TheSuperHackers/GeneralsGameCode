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

// FILE: FunctionLexicon.h ////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Collection of function pointers to help us in managing
//						 and assign callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/SubsystemInterface.h"
#include "Common/NameKeyGenerator.h"
#include "Common/GameMemory.h"
#include "GameClient/GameWindow.h"
#include "GameClient/WindowLayout.h"

//-------------------------------------------------------------------------------------------------
/** Collection of function pointers to help us in managing callbacks */
//-------------------------------------------------------------------------------------------------
class FunctionLexicon : public SubsystemInterface
{

public:

	struct TableEntry
	{
		NameKeyType key;
		const char *name;
		void *func;
	};

	enum TableIndex
	{
		TABLE_ANY = -1,					///< use this when searching to search any table

		TABLE_GAME_WIN_SYSTEM = 0,
		TABLE_GAME_WIN_INPUT,
		TABLE_GAME_WIN_TOOLTIP,
		TABLE_GAME_WIN_DEVICEDRAW,
		TABLE_GAME_WIN_DRAW,
		TABLE_WIN_LAYOUT_INIT,
		TABLE_WIN_LAYOUT_DEVICEINIT,
		TABLE_WIN_LAYOUT_UPDATE,
		TABLE_WIN_LAYOUT_SHUTDOWN,

		MAX_FUNCTION_TABLES
	};

public:

	FunctionLexicon( void );
	virtual ~FunctionLexicon( void );

	virtual void init( void );
	virtual void reset( void );
	virtual void update( void );

	/// validate the tables and make sure all entries are unique
	Bool validate( void );

	/// get internal function table
	TableEntry *getTable( TableIndex index );

	//
	// !NOTE! We do NOT have a functionToName() method because we assume
	// that functions in the tables are unique and that there is a 1 to 1
	// mapping of a symbol to a function address.  However, when compiling
	// in release, functions that have the same arguments and the same
	// body (mainly empty stub functions) get optimized to the
	// SAME ADDRESS.  That destroys our 1 to 1 mapping so it is something
	// that we must avoid
	//
	// translate a function pointer to its symbolic name
	// char *functionToName( void *func );

	// Game window functions ------------------------------------------------------------------------
	GameWinSystemFunc		gameWinSystemFunc( NameKeyType key, TableIndex = TABLE_GAME_WIN_SYSTEM );
	GameWinInputFunc		gameWinInputFunc( NameKeyType key, TableIndex = TABLE_GAME_WIN_INPUT  );
	GameWinTooltipFunc  gameWinTooltipFunc( NameKeyType key, TableIndex = TABLE_GAME_WIN_TOOLTIP );
	GameWinDrawFunc			gameWinDrawFunc( NameKeyType key, TableIndex = TABLE_ANY  );

	// Window layout functions ----------------------------------------------------------------------
	WindowLayoutInitFunc			winLayoutInitFunc( NameKeyType key, TableIndex = TABLE_ANY );
	WindowLayoutUpdateFunc		winLayoutUpdateFunc( NameKeyType key, TableIndex = TABLE_WIN_LAYOUT_UPDATE );
	WindowLayoutShutdownFunc	winLayoutShutdownFunc( NameKeyType key, TableIndex = TABLE_WIN_LAYOUT_SHUTDOWN );

protected:

	/// load a lookup table with run time values needed and save in table list
	void loadTable( TableEntry *table, TableIndex tableIndex );

	/** given a key find the function, the index parameter can limit the search
	to a single table or to ANY of the tables */
	void *findFunction( NameKeyType key, TableIndex index );

#ifdef NOT_IN_USE
	const char *funcToName( void *func, TableEntry *table );  ///< internal searching
#endif
	void *keyToFunc( NameKeyType key, TableEntry *table );  ///< internal searching

	TableEntry *m_tables[ MAX_FUNCTION_TABLES ];  ///< the lookup tables

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// INLINING
///////////////////////////////////////////////////////////////////////////////////////////////////
inline FunctionLexicon::TableEntry *FunctionLexicon::getTable( TableIndex index )
	{ return m_tables[ index ]; }

inline GameWinSystemFunc FunctionLexicon::gameWinSystemFunc( NameKeyType key, TableIndex index )
	{ return (GameWinSystemFunc)findFunction( key, index ); }
inline GameWinInputFunc FunctionLexicon::gameWinInputFunc( NameKeyType key, TableIndex index )
	{ return (GameWinInputFunc)findFunction( key, index ); }
inline GameWinTooltipFunc FunctionLexicon::gameWinTooltipFunc( NameKeyType key, TableIndex index )
	{ return (GameWinTooltipFunc)findFunction( key, index ); }

inline WindowLayoutUpdateFunc FunctionLexicon::winLayoutUpdateFunc( NameKeyType key, TableIndex index )
	{ return (WindowLayoutUpdateFunc)findFunction( key, index ); }
inline WindowLayoutShutdownFunc FunctionLexicon::winLayoutShutdownFunc( NameKeyType key, TableIndex index )
	{ return (WindowLayoutShutdownFunc)findFunction( key, index ); }

///////////////////////////////////////////////////////////////////////////////////////////////////
// EXTERNALS
///////////////////////////////////////////////////////////////////////////////////////////////////
extern FunctionLexicon *TheFunctionLexicon;  ///< function dictionary external
