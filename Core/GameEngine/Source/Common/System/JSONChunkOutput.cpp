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

#include "PreRTS.h"

#include "Common/JSONChunkOutput.h"
#include "Common/NameKeyGenerator.h"

JSONChunkOutput::JSONChunkOutput( void ) :
	m_chunkStack(NULL),
	m_nextID(1)
{
	m_root["chunks"] = nlohmann::json::array();
	m_toc = nlohmann::json::object();
}

JSONChunkOutput::~JSONChunkOutput()
{
	while (m_chunkStack) {
		JSONOutputChunk *c = m_chunkStack;
		m_chunkStack = m_chunkStack->next;
		deleteInstance(c);
	}

	if (!m_toc.empty()) {
		m_root["toc"] = m_toc;
	}
}

void JSONChunkOutput::openDataChunk( const char *name, DataChunkVersionType ver )
{
	AsciiString nameStr(name);
	UnsignedInt id = m_nextID++;
	m_toc[std::to_string(id)] = nameStr.str();

	nlohmann::json chunk;
	chunk["label"] = nameStr.str();
	chunk["version"] = ver;
	chunk["data"] = nlohmann::json::array();

	// If we're inside a chunk (nested), add to parent's data array, otherwise add to root chunks
	if (m_chunkStack && m_chunkStack->data) {
		// Nested chunk - add to parent's data array
		m_chunkStack->data->push_back(chunk);
	} else {
		// Top-level chunk - add to root chunks array
		m_root["chunks"].push_back(chunk);
	}

	JSONOutputChunk *c = newInstance(JSONOutputChunk);
	c->label = nameStr;
	// Set data pointer to the chunk's data array we just created
	if (m_chunkStack && m_chunkStack->data) {
		c->data = &(m_chunkStack->data->back()["data"]);
	} else {
		c->data = &(m_root["chunks"].back()["data"]);
	}
	c->next = m_chunkStack;
	m_chunkStack = c;
}

void JSONChunkOutput::closeDataChunk( void )
{
	if (m_chunkStack == NULL) {
		return;
	}

	JSONOutputChunk *c = m_chunkStack;
	m_chunkStack = m_chunkStack->next;
	deleteInstance(c);
}

void JSONChunkOutput::writeReal( Real r )
{
	if (m_chunkStack && m_chunkStack->data) {
		m_chunkStack->data->push_back(r);
	}
}

void JSONChunkOutput::writeInt( Int i )
{
	if (m_chunkStack && m_chunkStack->data) {
		m_chunkStack->data->push_back(i);
	}
}

void JSONChunkOutput::writeByte( Byte b )
{
	if (m_chunkStack && m_chunkStack->data) {
		m_chunkStack->data->push_back(b);
	}
}

void JSONChunkOutput::writeArrayOfBytes(char *ptr, Int len)
{
	if (m_chunkStack && m_chunkStack->data) {
		std::string bytesStr(ptr, len);
		m_chunkStack->data->push_back(bytesStr);
	}
}

void JSONChunkOutput::writeAsciiString( const AsciiString& theString )
{
	if (m_chunkStack && m_chunkStack->data) {
		m_chunkStack->data->push_back(theString.str());
	}
}

void JSONChunkOutput::writeUnicodeString( UnicodeString theString )
{
	if (m_chunkStack && m_chunkStack->data) {
		m_chunkStack->data->push_back(theString.str());
	}
}

void JSONChunkOutput::writeNameKey( const NameKeyType key )
{
	AsciiString kname = TheNameKeyGenerator->keyToName(key);
	UnsignedInt id = m_nextID++;
	m_toc[std::to_string(id)] = kname.str();
	Int keyAndType = id;
	keyAndType <<= 8;
	Dict::DataType t = Dict::DICT_ASCIISTRING;
	keyAndType |= (t & 0xff);
	writeInt(keyAndType);
}

void JSONChunkOutput::writeDict( const Dict& d )
{
	UnsignedShort len = d.getPairCount();
	writeInt(len);
	for (int i = 0; i < len; i++)
	{
		NameKeyType k = d.getNthKey(i);
		AsciiString kname = TheNameKeyGenerator->keyToName(k);

		UnsignedInt id = m_nextID++;
		m_toc[std::to_string(id)] = kname.str();
		Int keyAndType = id;
		keyAndType <<= 8;
		Dict::DataType t = d.getNthType(i);
		keyAndType |= (t & 0xff);
		writeInt(keyAndType);

		switch(t)
		{
			case Dict::DICT_BOOL:
				writeByte(d.getNthBool(i)?1:0);
				break;
			case Dict::DICT_INT:
				writeInt(d.getNthInt(i));
				break;
			case Dict::DICT_REAL:
				writeReal(d.getNthReal(i));
				break;
			case Dict::DICT_ASCIISTRING:
				writeAsciiString(d.getNthAsciiString(i));
				break;
			case Dict::DICT_UNICODESTRING:
				writeUnicodeString(d.getNthUnicodeString(i));
				break;
			default:
				DEBUG_CRASH(("impossible"));
				break;
		}
	}
}

std::string JSONChunkOutput::getJSONString( void )
{
	if (!m_toc.empty()) {
		m_root["toc"] = m_toc;
	}
	return m_root.dump(2);
}
