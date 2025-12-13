/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 TheSuperHackers
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

#pragma once

#ifdef RTS_HAS_JSON_CHUNK

#include "Common/GameMemory.h"
#include "Common/Dict.h"
#include <nlohmann/json.hpp>
#include <string>

typedef unsigned short DataChunkVersionType;

class JSONOutputChunk : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(JSONOutputChunk, "JSONOutputChunk")
public:
	JSONOutputChunk* next;
	AsciiString label;
	nlohmann::json* data;
};
EMPTY_DTOR(JSONOutputChunk)

class JSONChunkOutput
{
protected:
	nlohmann::json m_root;
	nlohmann::json m_toc;
	JSONOutputChunk* m_chunkStack;
	UnsignedInt m_nextID;

public:
	JSONChunkOutput( void );
	~JSONChunkOutput();

	void openDataChunk( const char *name, DataChunkVersionType ver );
	void closeDataChunk( void );

	void writeReal(Real r);
	void writeInt(Int i);
	void writeByte(Byte b);
	void writeAsciiString(const AsciiString& string);
	void writeUnicodeString(UnicodeString string);
	void writeArrayOfBytes(char *ptr, Int len);
	void writeDict(const Dict& d);
	void writeNameKey(const NameKeyType key);

	std::string getJSONString( void );
};

#endif // RTS_HAS_JSON_CHUNK
