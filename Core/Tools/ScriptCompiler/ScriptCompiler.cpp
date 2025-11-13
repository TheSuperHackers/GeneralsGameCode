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

// TheSuperHackers @feature bobtista 14/11/2025 Bidirectional SCB to JSON compiler for AI scripts.

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdint>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static void DebugLog(const char* format, ...)
{
	char buffer[1024];
	buffer[0] = 0;
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);
	printf("%s\n", buffer);
}
#define DEBUG_LOG(x) DebugLog x

struct ChunkHeader
{
	uint32_t chunkID;
	uint16_t version;
	uint32_t dataSize;
};

class DataBuffer
{
public:
	DataBuffer(const uint8_t* data, size_t size) : m_data(data), m_size(size), m_pos(0) {}
	DataBuffer(const std::vector<uint8_t>& vec) : m_data(vec.data()), m_size(vec.size()), m_pos(0) {}

	bool readString(std::string& str)
	{
		if (m_pos + 2 > m_size) return false;
		uint16_t len = m_data[m_pos] | (m_data[m_pos+1] << 8);
		m_pos += 2;
		if (m_pos + len > m_size) return false;
		str = std::string((char*)&m_data[m_pos], len);
		m_pos += len;
		return true;
	}

	bool readByte(uint8_t& val)
	{
		if (m_pos >= m_size) return false;
		val = m_data[m_pos++];
		return true;
	}

	bool readInt(int32_t& val)
	{
		if (m_pos + 4 > m_size) return false;
		val = m_data[m_pos] | (m_data[m_pos+1] << 8) |
		      (m_data[m_pos+2] << 16) | (m_data[m_pos+3] << 24);
		m_pos += 4;
		return true;
	}

	bool readUInt(uint32_t& val)
	{
		if (m_pos + 4 > m_size) return false;
		val = m_data[m_pos] | (m_data[m_pos+1] << 8) |
		      (m_data[m_pos+2] << 16) | (m_data[m_pos+3] << 24);
		m_pos += 4;
		return true;
	}

	bool readShort(uint16_t& val)
	{
		if (m_pos + 2 > m_size) return false;
		val = m_data[m_pos] | (m_data[m_pos+1] << 8);
		m_pos += 2;
		return true;
	}

	bool readChunkHeader(ChunkHeader& header)
	{
		if (m_pos + 10 > m_size) return false;
		if (!readUInt(header.chunkID)) return false;
		if (!readShort(header.version)) return false;
		if (!readUInt(header.dataSize)) return false;
		return true;
	}

	size_t getPosition() const { return m_pos; }
	size_t remaining() const { return m_size - m_pos; }
	bool atEnd() const { return m_pos >= m_size; }

private:
	const uint8_t* m_data;
	size_t m_size;
	size_t m_pos;
};

class BinaryChunkReader
{
public:
	BinaryChunkReader(FILE* fp) : m_file(fp), m_pos(0)
	{
		fseek(m_file, 0, SEEK_END);
		m_size = ftell(m_file);
		fseek(m_file, 0, SEEK_SET);
	}

	bool readFileHeader(std::vector<std::string>& stringTable)
	{
		char magic[4];
		if (fread(magic, 1, 4, m_file) != 4) return false;
		m_pos += 4;

		if (memcmp(magic, "CkMp", 4) != 0)
		{
			fseek(m_file, 0, SEEK_SET);
			m_pos = 0;
			return true;
		}

		uint32_t numStrings;
		if (fread(&numStrings, 4, 1, m_file) != 1) return false;
		m_pos += 4;

		DEBUG_LOG(("File header: CkMp, %u strings in table", numStrings));

		for (uint32_t i = 0; i < numStrings; i++)
		{
			uint8_t length;
			if (fread(&length, 1, 1, m_file) != 1) return false;
			m_pos += 1;

			std::vector<char> buffer(length);
			if (length > 0)
			{
				if (fread(buffer.data(), 1, length, m_file) != length) return false;
				m_pos += length;
			}

			uint32_t stringID;
			if (fread(&stringID, 4, 1, m_file) != 1) return false;
			m_pos += 4;

			std::string str(buffer.begin(), buffer.end());

			if (stringID >= stringTable.size())
				stringTable.resize(stringID + 1);

			stringTable[stringID] = str;
			DEBUG_LOG(("  String ID %u: '%s'", stringID, str.c_str()));
		}

		return true;
	}

	bool readChunkHeader(ChunkHeader& header)
	{
		if (m_pos + 10 > m_size)
			return false;

		if (fread(&header.chunkID, 4, 1, m_file) != 1) return false;
		if (fread(&header.version, 2, 1, m_file) != 1) return false;
		if (fread(&header.dataSize, 4, 1, m_file) != 1) return false;

		m_pos += 10;
		return true;
	}

	bool readString(std::string& str)
	{
		uint16_t length;
		if (fread(&length, 2, 1, m_file) != 1) return false;
		m_pos += 2;

		if (length > 0)
		{
			std::vector<char> buffer(length);
			if (fread(buffer.data(), 1, length, m_file) != length) return false;
			m_pos += length;
			str = std::string(buffer.begin(), buffer.end());
		}
		else
		{
			str.clear();
		}
		return true;
	}

	bool readByte(uint8_t& val)
	{
		if (fread(&val, 1, 1, m_file) != 1) return false;
		m_pos += 1;
		return true;
	}

	bool readInt(int32_t& val)
	{
		if (fread(&val, 4, 1, m_file) != 4) return false;
		m_pos += 4;
		return true;
	}

	bool readReal(float& val)
	{
		if (fread(&val, 4, 1, m_file) != 1) return false;
		m_pos += 4;
		return true;
	}

	bool skipBytes(size_t count)
	{
		if (fseek(m_file, count, SEEK_CUR) != 0) return false;
		m_pos += count;
		return true;
	}

	size_t getPosition() const { return m_pos; }
	size_t getSize() const { return m_size; }
	bool atEnd() const { return m_pos >= m_size; }

private:
	FILE* m_file;
	size_t m_pos;
	size_t m_size;
};

void dumpHelp(const char *exe)
{
	DEBUG_LOG(("Usage:"));
	DEBUG_LOG(("  To convert SCB to JSON: %s -in script.scb -out script.json", exe));
	DEBUG_LOG(("  To convert JSON to SCB: %s -in script.json -out script.scb", exe));
	DEBUG_LOG((""));
	DEBUG_LOG(("The tool auto-detects the format based on file extension."));
}

std::string chunkIDToString(uint32_t id)
{
	char buf[5];
	buf[0] = (id >> 0) & 0xFF;
	buf[1] = (id >> 8) & 0xFF;
	buf[2] = (id >> 16) & 0xFF;
	buf[3] = (id >> 24) & 0xFF;
	buf[4] = 0;

	for (int i = 0; i < 4; i++)
	{
		if (!isprint(buf[i]))
			return std::to_string(id);
	}

	return std::string(buf);
}

uint32_t stringToChunkID(const std::string& str)
{
	if (str.length() != 4)
		return 0;

	uint32_t id = 0;
	id |= (uint8_t)str[0] << 0;
	id |= (uint8_t)str[1] << 8;
	id |= (uint8_t)str[2] << 16;
	id |= (uint8_t)str[3] << 24;
	return id;
}

json parseChunkRecursive(DataBuffer& buffer, const std::vector<std::string>& stringTable, int depth = 0);

json parseScriptChunk(DataBuffer& buffer, const std::vector<std::string>& stringTable)
{
	json parsed;

	std::string scriptName, comment, conditionComment, actionComment;
	buffer.readString(scriptName);
	buffer.readString(comment);
	buffer.readString(conditionComment);
	buffer.readString(actionComment);

	uint8_t isActive, isOneShot, easy, normal, hard, isSubroutine;
	buffer.readByte(isActive);
	buffer.readByte(isOneShot);
	buffer.readByte(easy);
	buffer.readByte(normal);
	buffer.readByte(hard);
	buffer.readByte(isSubroutine);

	int32_t delaySeconds;
	buffer.readInt(delaySeconds);

	parsed["scriptName"] = scriptName;
	parsed["comment"] = comment;
	parsed["conditionComment"] = conditionComment;
	parsed["actionComment"] = actionComment;
	parsed["isActive"] = isActive != 0;
	parsed["isOneShot"] = isOneShot != 0;
	parsed["isSubroutine"] = isSubroutine != 0;
	parsed["delayEvaluationSeconds"] = delaySeconds;

	json difficulty = json::array();
	if (easy) difficulty.push_back("easy");
	if (normal) difficulty.push_back("normal");
	if (hard) difficulty.push_back("hard");
	parsed["difficulty"] = difficulty;

	json children = json::array();
	while (!buffer.atEnd())
	{
		json child = parseChunkRecursive(buffer, stringTable, 1);
		if (child.is_null()) break;
		children.push_back(child);
	}
	if (!children.empty())
		parsed["children"] = children;

	return parsed;
}

json parseParameter(DataBuffer& buffer, const std::vector<std::string>& stringTable)
{
	json param;

	int32_t paramType;
	if (!buffer.readInt(paramType)) return json();

	param["paramType"] = paramType;

	if (paramType == 32)
	{
		float x, y, z;
		uint32_t xi, yi, zi;
		buffer.readUInt(xi);
		buffer.readUInt(yi);
		buffer.readUInt(zi);
		memcpy(&x, &xi, 4);
		memcpy(&y, &yi, 4);
		memcpy(&z, &zi, 4);
		param["coord"] = {x, y, z};
	}
	else
	{
		int32_t intVal;
		uint32_t realBits;
		std::string strVal;

		buffer.readInt(intVal);
		buffer.readUInt(realBits);

		float realVal;
		memcpy(&realVal, &realBits, 4);

		buffer.readString(strVal);

		param["int"] = intVal;
		param["real"] = realVal;
		param["string"] = strVal;
	}

	return param;
}

json parseConditionChunk(DataBuffer& buffer, const std::vector<std::string>& stringTable, uint16_t version)
{
	json parsed;

	int32_t conditionType;
	buffer.readInt(conditionType);

	parsed["conditionType"] = conditionType;

	if (conditionType < (int32_t)stringTable.size() && !stringTable[conditionType].empty())
		parsed["conditionTypeName"] = stringTable[conditionType];
	else
		parsed["conditionTypeName"] = "UNKNOWN_" + std::to_string(conditionType);

	if (version >= 4)
	{
		uint32_t nameKey;
		buffer.readUInt(nameKey);
		parsed["nameKey"] = nameKey;
	}

	int32_t numParams;
	buffer.readInt(numParams);
	parsed["numParameters"] = numParams;

	json params = json::array();
	for (int i = 0; i < numParams && !buffer.atEnd(); i++)
	{
		json param = parseParameter(buffer, stringTable);
		if (!param.is_null())
			params.push_back(param);
	}
	parsed["parameters"] = params;

	return parsed;
}

json parseActionChunk(DataBuffer& buffer, const std::vector<std::string>& stringTable, uint16_t version)
{
	json parsed;

	int32_t actionType;
	buffer.readInt(actionType);

	parsed["actionType"] = actionType;

	if (actionType < (int32_t)stringTable.size() && !stringTable[actionType].empty())
		parsed["actionTypeName"] = stringTable[actionType];
	else
		parsed["actionTypeName"] = "UNKNOWN_" + std::to_string(actionType);

	if (version >= 2)
	{
		uint32_t nameKey;
		buffer.readUInt(nameKey);
		parsed["nameKey"] = nameKey;
	}

	int32_t numParams;
	buffer.readInt(numParams);
	parsed["numParameters"] = numParams;

	json params = json::array();
	for (int i = 0; i < numParams && !buffer.atEnd(); i++)
	{
		json param = parseParameter(buffer, stringTable);
		if (!param.is_null())
			params.push_back(param);
	}
	parsed["parameters"] = params;

	return parsed;
}

json parseChunkRecursive(DataBuffer& buffer, const std::vector<std::string>& stringTable, int depth)
{
	if (buffer.atEnd() || depth > 20)
		return json();

	ChunkHeader header;
	size_t headerStart = buffer.getPosition();
	if (!buffer.readChunkHeader(header))
		return json();

	std::string chunkTypeName = "UNKNOWN";
	if (header.chunkID < stringTable.size() && !stringTable[header.chunkID].empty())
		chunkTypeName = stringTable[header.chunkID];

	size_t dataStart = buffer.getPosition();
	size_t dataEnd = dataStart + header.dataSize;

	if (dataEnd > buffer.getPosition() + buffer.remaining())
		return json();

	std::vector<uint8_t> chunkData(header.dataSize);
	for (size_t i = 0; i < header.dataSize && !buffer.atEnd(); i++)
		buffer.readByte(chunkData[i]);

	DataBuffer chunkBuffer(chunkData);
	json parsed;

	DEBUG_LOG(("%*sChunk: %s (ID=%u) v%d size=%u", depth*2, "",
		chunkTypeName.c_str(), header.chunkID, header.version, header.dataSize));

	if (chunkTypeName == "Script")
	{
		parsed = parseScriptChunk(chunkBuffer, stringTable);
		parsed["type"] = "Script";
	}
	else if (chunkTypeName == "ScriptAction" || chunkTypeName == "ScriptActionFalse")
	{
		parsed = parseActionChunk(chunkBuffer, stringTable, header.version);
		parsed["type"] = chunkTypeName;
	}
	else if (chunkTypeName == "Condition")
	{
		parsed = parseConditionChunk(chunkBuffer, stringTable, header.version);
		parsed["type"] = "Condition";
	}
	else if (chunkTypeName == "ScriptGroup")
	{
		std::string groupName;
		uint8_t isActive, isSubroutine = 0;

		chunkBuffer.readString(groupName);
		chunkBuffer.readByte(isActive);
		if (header.version >= 2)
			chunkBuffer.readByte(isSubroutine);

		parsed["type"] = "ScriptGroup";
		parsed["groupName"] = groupName;
		parsed["isActive"] = isActive != 0;
		parsed["isSubroutine"] = isSubroutine != 0;

		json children = json::array();
		while (!chunkBuffer.atEnd())
		{
			json child = parseChunkRecursive(chunkBuffer, stringTable, depth + 1);
			if (child.is_null()) break;
			children.push_back(child);
		}
		if (!children.empty())
			parsed["children"] = children;
	}
	else if (chunkTypeName == "PlayerScriptsList" || chunkTypeName == "ScriptList" ||
	         chunkTypeName == "OrCondition")
	{
		parsed["type"] = chunkTypeName;
		json children = json::array();
		while (!chunkBuffer.atEnd())
		{
			json child = parseChunkRecursive(chunkBuffer, stringTable, depth + 1);
			if (child.is_null()) break;
			children.push_back(child);
		}
		if (!children.empty())
			parsed["children"] = children;
	}
	else
	{
		parsed["type"] = chunkTypeName;
		parsed["rawData"] = chunkData;
	}

	parsed["_meta"] = {
		{"chunkID", header.chunkID},
		{"version", header.version},
		{"dataSize", header.dataSize}
	};

	return parsed;
}

bool readBinaryToJson(const std::string& inFile, json& output)
{
	DEBUG_LOG(("Reading binary SCB file: %s", inFile.c_str()));

	FILE* fp = fopen(inFile.c_str(), "rb");
	if (!fp)
	{
		DEBUG_LOG(("Failed to open file: %s", inFile.c_str()));
		return false;
	}

	BinaryChunkReader reader(fp);
	std::vector<std::string> stringTable;

	if (!reader.readFileHeader(stringTable))
	{
		DEBUG_LOG(("Failed to read file header"));
		fclose(fp);
		return false;
	}

	DEBUG_LOG(("String table has %zu entries", stringTable.size()));

	json chunks = json::array();

	while (!reader.atEnd())
	{
		ChunkHeader header;
		size_t chunkStart = reader.getPosition();

		if (!reader.readChunkHeader(header))
			break;

		json chunk;
		std::string chunkTypeName = "UNKNOWN";

		if (header.chunkID < stringTable.size() && !stringTable[header.chunkID].empty())
		{
			chunkTypeName = stringTable[header.chunkID];
		}
		else
		{
			chunkTypeName = chunkIDToString(header.chunkID);
		}

		chunk["id"] = header.chunkID;
		chunk["type"] = chunkTypeName;
		chunk["version"] = header.version;
		chunk["dataSize"] = header.dataSize;
		chunk["fileOffset"] = chunkStart;

		DEBUG_LOG(("Chunk: %s (ID=%u) v%d size=%d at offset %zu",
			chunkTypeName.c_str(), header.chunkID, header.version, header.dataSize, chunkStart));

		std::vector<uint8_t> rawData(header.dataSize);

		FILE* dataFp = fopen(inFile.c_str(), "rb");
		fseek(dataFp, reader.getPosition(), SEEK_SET);
		fread(rawData.data(), 1, header.dataSize, dataFp);
		fclose(dataFp);

		DataBuffer chunkBuffer(rawData);
		json parsed = parseChunkRecursive(chunkBuffer, stringTable, 0);

		if (!parsed.is_null())
			chunk["parsed"] = parsed;
		else
			chunk["rawData"] = rawData;

		reader.skipBytes(header.dataSize);

		chunks.push_back(chunk);
	}

	fclose(fp);

	output["format"] = "SCB";
	output["version"] = 1;
	output["stringTable"] = stringTable;
	output["chunks"] = chunks;

	DEBUG_LOG(("Successfully read %zu chunks", chunks.size()));
	return true;
}

class BinaryChunkWriter
{
public:
	BinaryChunkWriter(FILE* fp) : m_file(fp) {}

	void writeFileHeader(const std::vector<std::string>& stringTable)
	{
		fwrite("CkMp", 1, 4, m_file);

		uint32_t numStrings = stringTable.size();
		fwrite(&numStrings, 4, 1, m_file);

		for (uint32_t i = 0; i < stringTable.size(); i++)
		{
			const std::string& str = stringTable[i];
			uint8_t length = str.length();
			fwrite(&length, 1, 1, m_file);
			if (length > 0)
				fwrite(str.c_str(), 1, length, m_file);
			fwrite(&i, 4, 1, m_file);
		}
	}

	void writeChunkHeader(uint32_t id, uint16_t version, uint32_t dataSize)
	{
		fwrite(&id, 4, 1, m_file);
		fwrite(&version, 2, 1, m_file);
		fwrite(&dataSize, 4, 1, m_file);
	}

	void writeString(const std::string& str)
	{
		uint16_t len = str.length();
		fwrite(&len, 2, 1, m_file);
		if (len > 0)
			fwrite(str.c_str(), 1, len, m_file);
	}

	void writeByte(uint8_t val)
	{
		fwrite(&val, 1, 1, m_file);
	}

	void writeInt(int32_t val)
	{
		fwrite(&val, 4, 1, m_file);
	}

	void writeUInt(uint32_t val)
	{
		fwrite(&val, 4, 1, m_file);
	}

	void writeReal(float val)
	{
		fwrite(&val, 4, 1, m_file);
	}

private:
	FILE* m_file;
};

std::vector<uint8_t> serializeParameter(const json& param)
{
	std::vector<uint8_t> data;

	int32_t paramType = param.value("paramType", 0);
	data.resize(4);
	memcpy(data.data(), &paramType, 4);

	if (paramType == 32 && param.contains("coord"))
	{
		auto coord = param["coord"];
		float x = coord[0].get<float>();
		float y = coord[1].get<float>();
		float z = coord[2].get<float>();

		size_t offset = data.size();
		data.resize(offset + 12);
		memcpy(&data[offset], &x, 4);
		memcpy(&data[offset + 4], &y, 4);
		memcpy(&data[offset + 8], &z, 4);
	}
	else
	{
		int32_t intVal = param.value("int", 0);
		float realVal = param.value("real", 0.0f);
		std::string strVal = param.value("string", "");

		size_t offset = data.size();
		data.resize(offset + 4);
		memcpy(&data[offset], &intVal, 4);

		offset = data.size();
		data.resize(offset + 4);
		memcpy(&data[offset], &realVal, 4);

		uint16_t strLen = strVal.length();
		offset = data.size();
		data.resize(offset + 2 + strLen);
		memcpy(&data[offset], &strLen, 2);
		if (strLen > 0)
			memcpy(&data[offset + 2], strVal.c_str(), strLen);
	}

	return data;
}

std::vector<uint8_t> serializeChunk(const json& chunk, const std::vector<std::string>& stringTable);

std::vector<uint8_t> serializeCondition(const json& cond, const std::vector<std::string>& stringTable)
{
	std::vector<uint8_t> data;

	int32_t condType = cond.value("conditionType", 0);
	data.resize(4);
	memcpy(data.data(), &condType, 4);

	uint16_t version = cond["_meta"].value("version", 4);
	if (version >= 4)
	{
		uint32_t nameKey = cond.value("nameKey", 0);
		size_t offset = data.size();
		data.resize(offset + 4);
		memcpy(&data[offset], &nameKey, 4);
	}

	int32_t numParams = cond.value("numParameters", 0);
	size_t offset = data.size();
	data.resize(offset + 4);
	memcpy(&data[offset], &numParams, 4);

	if (cond.contains("parameters"))
	{
		for (const auto& param : cond["parameters"])
		{
			auto paramData = serializeParameter(param);
			data.insert(data.end(), paramData.begin(), paramData.end());
		}
	}

	return data;
}

std::vector<uint8_t> serializeAction(const json& action, const std::vector<std::string>& stringTable)
{
	std::vector<uint8_t> data;

	int32_t actionType = action.value("actionType", 0);
	data.resize(4);
	memcpy(data.data(), &actionType, 4);

	uint16_t version = action["_meta"].value("version", 2);
	if (version >= 2)
	{
		uint32_t nameKey = action.value("nameKey", 0);
		size_t offset = data.size();
		data.resize(offset + 4);
		memcpy(&data[offset], &nameKey, 4);
	}

	int32_t numParams = action.value("numParameters", 0);
	size_t offset = data.size();
	data.resize(offset + 4);
	memcpy(&data[offset], &numParams, 4);

	if (action.contains("parameters"))
	{
		for (const auto& param : action["parameters"])
		{
			auto paramData = serializeParameter(param);
			data.insert(data.end(), paramData.begin(), paramData.end());
		}
	}

	return data;
}

std::vector<uint8_t> serializeScript(const json& script, const std::vector<std::string>& stringTable)
{
	std::vector<uint8_t> data;

	std::string scriptName = script.value("scriptName", "");
	std::string comment = script.value("comment", "");
	std::string condComment = script.value("conditionComment", "");
	std::string actComment = script.value("actionComment", "");

	auto writeStr = [&](const std::string& s) {
		uint16_t len = s.length();
		size_t offset = data.size();
		data.resize(offset + 2 + len);
		memcpy(&data[offset], &len, 2);
		if (len > 0)
			memcpy(&data[offset + 2], s.c_str(), len);
	};

	writeStr(scriptName);
	writeStr(comment);
	writeStr(condComment);
	writeStr(actComment);

	uint8_t isActive = script.value("isActive", true) ? 1 : 0;
	uint8_t isOneShot = script.value("isOneShot", false) ? 1 : 0;
	uint8_t easy = script.value("easy", true) ? 1 : 0;
	uint8_t normal = script.value("normal", true) ? 1 : 0;
	uint8_t hard = script.value("hard", true) ? 1 : 0;
	uint8_t isSubroutine = script.value("isSubroutine", false) ? 1 : 0;

	data.push_back(isActive);
	data.push_back(isOneShot);
	data.push_back(easy);
	data.push_back(normal);
	data.push_back(hard);
	data.push_back(isSubroutine);

	int32_t delaySeconds = script.value("delayEvaluationSeconds", 0);
	size_t offset = data.size();
	data.resize(offset + 4);
	memcpy(&data[offset], &delaySeconds, 4);

	if (script.contains("children"))
	{
		for (const auto& child : script["children"])
		{
			auto childData = serializeChunk(child, stringTable);
			data.insert(data.end(), childData.begin(), childData.end());
		}
	}

	return data;
}

std::vector<uint8_t> serializeScriptGroup(const json& group, const std::vector<std::string>& stringTable)
{
	std::vector<uint8_t> data;

	std::string groupName = group.value("groupName", "");
	uint16_t len = groupName.length();
	data.resize(2 + len);
	memcpy(data.data(), &len, 2);
	if (len > 0)
		memcpy(&data[2], groupName.c_str(), len);

	uint8_t isActive = group.value("isActive", true) ? 1 : 0;
	uint8_t isSubroutine = group.value("isSubroutine", false) ? 1 : 0;
	data.push_back(isActive);
	data.push_back(isSubroutine);

	if (group.contains("children"))
	{
		for (const auto& child : group["children"])
		{
			auto childData = serializeChunk(child, stringTable);
			data.insert(data.end(), childData.begin(), childData.end());
		}
	}

	return data;
}

std::vector<uint8_t> serializeChunk(const json& chunk, const std::vector<std::string>& stringTable)
{
	std::vector<uint8_t> result;
	std::vector<uint8_t> chunkData;

	std::string chunkType = chunk.value("type", "");
	uint32_t chunkID = chunk["_meta"].value("chunkID", 0);
	uint16_t version = chunk["_meta"].value("version", 1);

	if (chunkType == "Script")
	{
		chunkData = serializeScript(chunk, stringTable);
	}
	else if (chunkType == "ScriptAction" || chunkType == "ScriptActionFalse")
	{
		chunkData = serializeAction(chunk, stringTable);
	}
	else if (chunkType == "Condition")
	{
		chunkData = serializeCondition(chunk, stringTable);
	}
	else if (chunkType == "ScriptGroup")
	{
		chunkData = serializeScriptGroup(chunk, stringTable);
	}
	else if (chunk.contains("children"))
	{
		for (const auto& child : chunk["children"])
		{
			auto childData = serializeChunk(child, stringTable);
			chunkData.insert(chunkData.end(), childData.begin(), childData.end());
		}
	}
	else if (chunk.contains("rawData"))
	{
		chunkData = chunk["rawData"].get<std::vector<uint8_t>>();
	}

	uint32_t dataSize = chunkData.size();
	result.resize(10 + dataSize);

	memcpy(&result[0], &chunkID, 4);
	memcpy(&result[4], &version, 2);
	memcpy(&result[6], &dataSize, 4);
	if (dataSize > 0)
		memcpy(&result[10], chunkData.data(), dataSize);

	return result;
}

bool writeJsonToBinary(const json& input, const std::string& outFile)
{
	DEBUG_LOG(("Writing binary SCB file: %s", outFile.c_str()));

	FILE* fp = fopen(outFile.c_str(), "wb");
	if (!fp)
	{
		DEBUG_LOG(("Failed to create file: %s", outFile.c_str()));
		return false;
	}

	BinaryChunkWriter writer(fp);

	if (input.contains("stringTable"))
	{
		std::vector<std::string> stringTable = input["stringTable"].get<std::vector<std::string>>();
		writer.writeFileHeader(stringTable);

		if (input.contains("chunks"))
		{
			for (const auto& chunkEntry : input["chunks"])
			{
				uint32_t outerChunkID = chunkEntry.value("id", 0);
				uint16_t outerVersion = chunkEntry.value("version", 1);

				std::vector<uint8_t> innerData;

				if (chunkEntry.contains("parsed"))
				{
					innerData = serializeChunk(chunkEntry["parsed"], stringTable);
				}
				else if (chunkEntry.contains("rawData"))
				{
					innerData = chunkEntry["rawData"].get<std::vector<uint8_t>>();
				}

				writer.writeChunkHeader(outerChunkID, outerVersion, innerData.size());
				if (!innerData.empty())
					fwrite(innerData.data(), 1, innerData.size(), fp);
			}
		}
	}

	fclose(fp);
	DEBUG_LOG(("Successfully wrote binary SCB file"));
	return true;
}

bool readJsonFile(const std::string& inFile, json& output)
{
	FILE* fp = fopen(inFile.c_str(), "r");
	if (!fp)
	{
		DEBUG_LOG(("Cannot open JSON file: %s", inFile.c_str()));
		return false;
	}

	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::vector<char> buffer(size + 1);
	size_t numRead = fread(buffer.data(), 1, size, fp);
	fclose(fp);

	if (numRead != size)
	{
		DEBUG_LOG(("Failed to read JSON file: %s", inFile.c_str()));
		return false;
	}

	buffer[size] = 0;

	try
	{
		output = json::parse(buffer.data());
		return true;
	}
	catch (const json::exception& e)
	{
		DEBUG_LOG(("JSON parse error: %s", e.what()));
		return false;
	}
}

bool writeJsonFile(const json& data, const std::string& outFile)
{
	FILE* fp = fopen(outFile.c_str(), "w");
	if (!fp)
	{
		DEBUG_LOG(("Cannot create JSON file: %s", outFile.c_str()));
		return false;
	}

	std::string jsonStr = data.dump(2);
	size_t written = fwrite(jsonStr.c_str(), 1, jsonStr.length(), fp);
	fclose(fp);

	if (written != jsonStr.length())
	{
		DEBUG_LOG(("Failed to write JSON file: %s", outFile.c_str()));
		return false;
	}

	return true;
}

bool endsWithIgnoreCase(const std::string& str, const std::string& suffix)
{
	if (str.length() < suffix.length())
		return false;

	size_t start = str.length() - suffix.length();
	for (size_t i = 0; i < suffix.length(); ++i)
	{
		char c1 = tolower(str[start + i]);
		char c2 = tolower(suffix[i]);
		if (c1 != c2)
			return false;
	}
	return true;
}

int main(int argc, char **argv)
{
	std::string inFile;
	std::string outFile;

	for (int i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-help") || !strcmp(argv[i], "--help"))
		{
			dumpHelp(argv[0]);
			return EXIT_SUCCESS;
		}

		if (!strcmp(argv[i], "-in"))
		{
			++i;
			if (i < argc)
				inFile = argv[i];
		}

		if (!strcmp(argv[i], "-out"))
		{
			++i;
			if (i < argc)
				outFile = argv[i];
		}
	}

	if (inFile.empty() || outFile.empty())
	{
		dumpHelp(argv[0]);
		return EXIT_FAILURE;
	}

	bool inIsScb = endsWithIgnoreCase(inFile, ".scb");
	bool outIsScb = endsWithIgnoreCase(outFile, ".scb");
	bool inIsJson = endsWithIgnoreCase(inFile, ".json");
	bool outIsJson = endsWithIgnoreCase(outFile, ".json");

	if (inIsScb && outIsJson)
	{
		json data;
		if (!readBinaryToJson(inFile, data))
			return EXIT_FAILURE;

		if (!writeJsonFile(data, outFile))
			return EXIT_FAILURE;

		DEBUG_LOG(("Successfully converted %s to %s", inFile.c_str(), outFile.c_str()));
		return EXIT_SUCCESS;
	}
	else if (inIsJson && outIsScb)
	{
		json data;
		if (!readJsonFile(inFile, data))
			return EXIT_FAILURE;

		if (!writeJsonToBinary(data, outFile))
			return EXIT_FAILURE;

		DEBUG_LOG(("Successfully converted %s to %s", inFile.c_str(), outFile.c_str()));
		return EXIT_SUCCESS;
	}
	else
	{
		DEBUG_LOG(("ERROR: Cannot determine conversion direction from file extensions"));
		DEBUG_LOG(("Input: %s, Output: %s", inFile.c_str(), outFile.c_str()));
		dumpHelp(argv[0]);
		return EXIT_FAILURE;
	}
}

