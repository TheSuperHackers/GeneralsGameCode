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

#pragma once

#include "Common/MessageStream.h"
#include "Common/GameMemory.h"

//----------------------------------------------------------------------------
class GameMessageParserArgumentType : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(GameMessageParserArgumentType, "GameMessageParserArgumentType")
public:
	GameMessageParserArgumentType(GameMessageArgumentDataType type, Int argCount);
	//virtual ~GameMessageParserArgumentType();

	GameMessageParserArgumentType *getNext();
	void setNext(GameMessageParserArgumentType *next);
	Int getArgCount();
	GameMessageArgumentDataType getType();

protected:
	GameMessageParserArgumentType*	m_next;
	GameMessageArgumentDataType			m_type;
	Int															m_argCount;
};

//----------------------------------------------------------------------------
inline GameMessageParserArgumentType * GameMessageParserArgumentType::getNext()
{
	return m_next;
}

//----------------------------------------------------------------------------
inline void GameMessageParserArgumentType::setNext(GameMessageParserArgumentType *next)
{
	m_next = next;
}

//----------------------------------------------------------------------------
inline GameMessageArgumentDataType GameMessageParserArgumentType::getType()
{
	return m_type;
}

//----------------------------------------------------------------------------
inline Int GameMessageParserArgumentType::getArgCount()
{
	return m_argCount;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class GameMessageParser : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(GameMessageParser, "GameMessageParser")
public:
	GameMessageParser();
	GameMessageParser(GameMessage *msg);
	//virtual ~GameMessageParser();

	GameMessageParserArgumentType *getFirstArgumentType();
	void addArgType(GameMessageArgumentDataType type, Int argCount);
	Int getNumTypes();

protected:
	GameMessageParserArgumentType *m_first, *m_last;
	Int m_argTypeCount;
};

//----------------------------------------------------------------------------
inline GameMessageParserArgumentType * GameMessageParser::getFirstArgumentType()
{
	return m_first;
}

//----------------------------------------------------------------------------
inline Int GameMessageParser::getNumTypes()
{
	return m_argTypeCount;
}

