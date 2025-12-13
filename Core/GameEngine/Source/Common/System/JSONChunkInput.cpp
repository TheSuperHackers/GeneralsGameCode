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

#include "Common/JSONChunkInput.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Errors.h"
#include "Common/UnicodeString.h"
#include "Common/AsciiString.h"
#ifdef WIN32
#include <windows.h>
#endif

JSONChunkInput::JSONChunkInput( const char* jsonData, size_t jsonSize ) :
	m_parserList(NULL),
	m_chunkStack(NULL),
	m_currentChunkIndex(0),
	m_currentObject(NULL),
	m_userData(NULL)
{
	try {
		m_root = nlohmann::json::parse(jsonData, jsonData + jsonSize);
		if (m_root.contains("chunks") && m_root["chunks"].is_array()) {
			for (auto& chunk : m_root["chunks"]) {
				m_chunkArray.push_back(&chunk);
			}
		}
	} catch (...) {
		m_root = nlohmann::json();
	}
}

JSONChunkInput::~JSONChunkInput()
{
	clearChunkStack();

	JSONParser *p, *next;
	for (p=m_parserList; p; p=next) {
		next = p->next;
		deleteInstance(p);
	}
}

void JSONChunkInput::clearChunkStack( void )
{
	JSONInputChunk *c, *next;

	for (c=m_chunkStack; c; c=next) {
		next = c->next;
		deleteInstance(c);
	}

	m_chunkStack = NULL;
}

Bool JSONChunkInput::isValidFileType(void)
{
	return m_root.contains("chunks") && m_root["chunks"].is_array();
}

void JSONChunkInput::registerParser( const AsciiString& label, const AsciiString& parentLabel, JSONChunkParserPtr parser, void *userData )
{
	JSONParser *p = newInstance(JSONParser);
	p->parser = parser;
	p->label = label;
	p->parentLabel = parentLabel;
	p->userData = userData;
	p->next = m_parserList;
	m_parserList = p;
}

AsciiString JSONChunkInput::openDataChunk(DataChunkVersionType *ver )
{
	// If we're inside a chunk, read nested chunks from the current chunk's data array
	if (m_chunkStack && m_chunkStack->data && m_chunkStack->data->is_array()) {
		// Look for nested chunks in the current chunk's data array
		// Scan from current dataIndex to find chunk objects (don't modify dataIndex - it's used by read functions)
		for (size_t i = m_chunkStack->dataIndex; i < m_chunkStack->data->size(); i++) {
			nlohmann::json& item = (*m_chunkStack->data)[i];

			// Check if this item is a chunk (has label, version, data)
			if (item.is_object() && item.contains("label") && item.contains("version") && item.contains("data")) {
				if (item["data"].is_array()) {
					// Found a nested chunk - advance dataIndex past it so read functions skip it
					m_chunkStack->dataIndex = i + 1;

					JSONInputChunk *c = newInstance(JSONInputChunk);
					c->label = AsciiString(item["label"].get<std::string>().c_str());
					c->version = item["version"].get<DataChunkVersionType>();
					c->data = &(item["data"]);
					c->dataIndex = 0;
					c->next = m_chunkStack;
					m_chunkStack = c;

					*ver = c->version;
					return c->label;
				}
			}
		}
		// No more nested chunks in current chunk
		*ver = 0;
		return AsciiString::TheEmptyString;
	}

	// Top-level chunk - read from m_chunkArray
	if (m_currentChunkIndex >= m_chunkArray.size()) {
		*ver = 0;
		return AsciiString::TheEmptyString;
	}

	nlohmann::json* chunkJson = m_chunkArray[m_currentChunkIndex++];

	if (!chunkJson->contains("label") || !chunkJson->contains("version") || !chunkJson->contains("data")) {
		*ver = 0;
		return AsciiString::TheEmptyString;
	}

	if (!(*chunkJson)["data"].is_array()) {
		*ver = 0;
		return AsciiString::TheEmptyString;
	}

	JSONInputChunk *c = newInstance(JSONInputChunk);
	c->label = AsciiString((*chunkJson)["label"].get<std::string>().c_str());
	c->version = (*chunkJson)["version"].get<DataChunkVersionType>();
	c->data = &((*chunkJson)["data"]);
	c->dataIndex = 0;
	c->next = m_chunkStack;
	m_chunkStack = c;

	*ver = c->version;
	return c->label;
}

void JSONChunkInput::closeDataChunk( void )
{
	if (m_chunkStack == NULL) {
		return;
	}

	JSONInputChunk *c = m_chunkStack;
	m_chunkStack = m_chunkStack->next;
	deleteInstance(c);
}

Bool JSONChunkInput::atEndOfFile( void )
{
	return m_currentChunkIndex >= m_chunkArray.size() && m_chunkStack == NULL;
}

Bool JSONChunkInput::atEndOfChunk( void )
{
	if (m_chunkStack == NULL) {
		return true;
	}

	if (!m_chunkStack->data->is_array()) {
		return true;
	}

	return m_chunkStack->dataIndex >= m_chunkStack->data->size();
}

void JSONChunkInput::reset( void )
{
	clearChunkStack();
	m_currentChunkIndex = 0;
}

AsciiString JSONChunkInput::getChunkLabel( void )
{
	if (m_chunkStack == NULL) {
		DEBUG_CRASH(("Bad."));
		return AsciiString::TheEmptyString;
	}

	return m_chunkStack->label;
}

DataChunkVersionType JSONChunkInput::getChunkVersion( void )
{
	if (m_chunkStack == NULL) {
		DEBUG_CRASH(("Bad."));
		return 0;
	}

	return m_chunkStack->version;
}

unsigned int JSONChunkInput::getChunkDataSize( void )
{
	if (m_chunkStack == NULL) {
		DEBUG_CRASH(("Bad."));
		return 0;
	}

	if (!m_chunkStack->data->is_array()) {
		return 0;
	}

	return m_chunkStack->data->size();
}

unsigned int JSONChunkInput::getChunkDataSizeLeft( void )
{
	if (m_chunkStack == NULL) {
		DEBUG_CRASH(("Bad."));
		return 0;
	}

	if (!m_chunkStack->data->is_array()) {
		return 0;
	}

	return m_chunkStack->data->size() - m_chunkStack->dataIndex;
}

Bool JSONChunkInput::parse( void *userData )
{
	m_userData = userData;

	while (!atEndOfFile()) {
		if (m_chunkStack && atEndOfChunk()) {
			break;
		}

		DataChunkVersionType ver;
		AsciiString label = openDataChunk(&ver);
		if (atEndOfFile()) {
			break;
		}

		AsciiString parentLabel = AsciiString::TheEmptyString;
		if (m_chunkStack && m_chunkStack->next) {
			parentLabel = m_chunkStack->next->label;
		}

		Bool scopeOK = false;
		JSONParser* parser;
		for (parser=m_parserList; parser; parser=parser->next) {
			if (parser->label == label) {
				scopeOK = true;
				if (parentLabel != parser->parentLabel) {
					scopeOK = false;
				}

				if (scopeOK) {
					JSONChunkInfo info;
					info.label = label;
					info.parentLabel = parentLabel;
					info.version = ver;
					info.dataSize = getChunkDataSize();

					if (parser->parser(*this, &info, userData) == false) {
						return false;
					}
					break;
				}
			}
		}

		closeDataChunk();
	}

	return true;
}

Real JSONChunkInput::readReal(void)
{
	if (m_chunkStack == NULL || !m_chunkStack->data->is_array()) {
		DEBUG_CRASH(("Bad."));
		return 0.0f;
	}

	if (m_chunkStack->dataIndex >= m_chunkStack->data->size()) {
		DEBUG_CRASH(("Read past end of chunk."));
		return 0.0f;
	}

	Real r = (*m_chunkStack->data)[m_chunkStack->dataIndex++].get<Real>();
	return r;
}

Int JSONChunkInput::readInt(void)
{
	if (m_chunkStack == NULL || !m_chunkStack->data->is_array()) {
		DEBUG_CRASH(("Bad."));
		return 0;
	}

	if (m_chunkStack->dataIndex >= m_chunkStack->data->size()) {
		DEBUG_CRASH(("Read past end of chunk."));
		return 0;
	}

	Int i = (*m_chunkStack->data)[m_chunkStack->dataIndex++].get<Int>();
	return i;
}

Byte JSONChunkInput::readByte(void)
{
	if (m_chunkStack == NULL || !m_chunkStack->data->is_array()) {
		DEBUG_CRASH(("Bad."));
		return 0;
	}

	if (m_chunkStack->dataIndex >= m_chunkStack->data->size()) {
		DEBUG_CRASH(("Read past end of chunk."));
		return 0;
	}

	Byte b = (*m_chunkStack->data)[m_chunkStack->dataIndex++].get<Byte>();
	return b;
}

void JSONChunkInput::readArrayOfBytes(char *ptr, Int len)
{
	if (m_chunkStack == NULL || !m_chunkStack->data->is_array()) {
		DEBUG_CRASH(("Bad."));
		return;
	}

	if (m_chunkStack->dataIndex >= m_chunkStack->data->size()) {
		DEBUG_CRASH(("Read past end of chunk."));
		return;
	}

	std::string bytesStr = (*m_chunkStack->data)[m_chunkStack->dataIndex++].get<std::string>();

	if (bytesStr.length() < (size_t)len) {
		DEBUG_CRASH(("Not enough bytes."));
		return;
	}

	memcpy(ptr, bytesStr.c_str(), len);
}

AsciiString JSONChunkInput::readAsciiString(void)
{
	if (m_chunkStack == NULL || !m_chunkStack->data->is_array()) {
		DEBUG_CRASH(("Bad."));
		return AsciiString::TheEmptyString;
	}

	if (m_chunkStack->dataIndex >= m_chunkStack->data->size()) {
		DEBUG_CRASH(("Read past end of chunk."));
		return AsciiString::TheEmptyString;
	}

	std::string str = (*m_chunkStack->data)[m_chunkStack->dataIndex++].get<std::string>();
	return AsciiString(str.c_str());
}

UnicodeString JSONChunkInput::readUnicodeString(void)
{
	if (m_chunkStack == NULL || !m_chunkStack->data->is_array()) {
		DEBUG_CRASH(("Bad."));
		return UnicodeString::TheEmptyString;
	}

	if (m_chunkStack->dataIndex >= m_chunkStack->data->size()) {
		DEBUG_CRASH(("Read past end of chunk."));
		return UnicodeString::TheEmptyString;
	}

	std::string str = (*m_chunkStack->data)[m_chunkStack->dataIndex++].get<std::string>();
	if (str.empty()) {
		return UnicodeString::TheEmptyString;
	}

#ifdef WIN32
	int wideLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	if (wideLen <= 0) {
		DEBUG_CRASH(("Failed to convert UTF-8 string to wide string."));
		return UnicodeString::TheEmptyString;
	}
	WideChar* wideStr = (WideChar*)TheDynamicMemoryAllocator->allocateBytes(wideLen * sizeof(WideChar), "JSONChunkInput::readUnicodeString");
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wideStr, wideLen);
	UnicodeString result(wideStr);
	TheDynamicMemoryAllocator->freeBytes(wideStr);
	return result;
#else
	AsciiString asciiStr(str.c_str());
	UnicodeString result;
	result.translate(asciiStr);
	return result;
#endif
}

NameKeyType JSONChunkInput::readNameKey(void)
{
	Int keyAndType = readInt();
#ifdef DEBUG_CRASHING
	Dict::DataType t = (Dict::DataType)(keyAndType & 0xff);
	DEBUG_ASSERTCRASH(t==Dict::DICT_ASCIISTRING,("Invalid key data."));
#endif
	keyAndType >>= 8;

	std::string knameStr;
	if (m_root.contains("toc") && m_root["toc"].is_object()) {
		std::string idStr = std::to_string(keyAndType);
		if (m_root["toc"].contains(idStr)) {
			knameStr = m_root["toc"][idStr].get<std::string>();
		}
	}

	if (knameStr.empty()) {
		knameStr = std::to_string(keyAndType);
	}

	AsciiString kname(knameStr.c_str());
	NameKeyType k = TheNameKeyGenerator->nameToKey(kname);
	return k;
}

Dict JSONChunkInput::readDict()
{
	UnsignedShort len = readInt();
	Dict d(len);

	for (int i = 0; i < len; i++)
	{
		Int keyAndType = readInt();
		Dict::DataType t = (Dict::DataType)(keyAndType & 0xff);
		keyAndType >>= 8;

		std::string knameStr;
		if (m_root.contains("toc") && m_root["toc"].is_object()) {
			std::string idStr = std::to_string(keyAndType);
			if (m_root["toc"].contains(idStr)) {
				knameStr = m_root["toc"][idStr].get<std::string>();
			}
		}

		if (knameStr.empty()) {
			knameStr = std::to_string(keyAndType);
		}

		AsciiString kname(knameStr.c_str());
		NameKeyType k = TheNameKeyGenerator->nameToKey(kname);

		switch(t)
		{
			case Dict::DICT_BOOL:
				d.setBool(k, readByte() ? true : false);
				break;
			case Dict::DICT_INT:
				d.setInt(k, readInt());
				break;
			case Dict::DICT_REAL:
				d.setReal(k, readReal());
				break;
			case Dict::DICT_ASCIISTRING:
				d.setAsciiString(k, readAsciiString());
				break;
			case Dict::DICT_UNICODESTRING:
				d.setUnicodeString(k, readUnicodeString());
				break;
			default:
				throw ERROR_CORRUPT_FILE_FORMAT;
				break;
		}
	}

	return d;
}
