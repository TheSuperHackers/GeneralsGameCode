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


#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameNetwork/GameMessageParser.h"

//----------------------------------------------------------------------------
GameMessageParser::GameMessageParser()
{
	m_first = nullptr;
	m_argTypeCount = 0;
}

//----------------------------------------------------------------------------
GameMessageParser::GameMessageParser(GameMessage *msg)
{
	m_first = nullptr;
	m_argTypeCount = 0;

	UnsignedByte argCount = msg->getArgumentCount();
	GameMessageArgumentDataType lasttype = ARGUMENTDATATYPE_UNKNOWN;
	Int thisTypeCount = 0;

	for (UnsignedByte i = 0; i < argCount; ++i) {
		GameMessageArgumentDataType type = msg->getArgumentDataType(i);
		if (type != lasttype) {
			if (thisTypeCount > 0) {
				addArgType(lasttype, thisTypeCount);
				++m_argTypeCount;
			}
			lasttype = type;
			thisTypeCount = 0;
		}
		++thisTypeCount;
	}
	if (thisTypeCount > 0) {
		addArgType(lasttype, thisTypeCount);
		++m_argTypeCount;
	}
}

//----------------------------------------------------------------------------
GameMessageParser::~GameMessageParser()
{
	GameMessageParserArgumentType *temp = nullptr;
	while (m_first != nullptr) {
		temp = m_first->getNext();
		deleteInstance(m_first);
		m_first = temp;
	}
}

//----------------------------------------------------------------------------
void GameMessageParser::addArgType(GameMessageArgumentDataType type, Int argCount)
{
	if (m_first == nullptr) {
		m_first = newInstance(GameMessageParserArgumentType)(type, argCount);
		m_last = m_first;
		return;
	}

	m_last->setNext(newInstance(GameMessageParserArgumentType)(type, argCount));
	m_last = m_last->getNext();
}

//----------------------------------------------------------------------------
GameMessageParserArgumentType::GameMessageParserArgumentType(GameMessageArgumentDataType type, Int argCount)
{
	m_next = nullptr;
	m_type = type;
	m_argCount = argCount;
}

//----------------------------------------------------------------------------
GameMessageParserArgumentType::~GameMessageParserArgumentType()
{
}

