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

#include "Common/DataChunk.h"
#include "Common/JSONChunkInput.h"
#include "Common/JSONChunkOutput.h"
#include "GameLogic/Scripts.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/GameMemory.h"
#include "GameClient/GameText.h"
#include "Common/Science.h"
#include "Common/MultiplayerSettings.h"
#include "Common/TerrainTypes.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Errors.h"
#include "Common/GameState.h"
#include "Common/KindOf.h"
#include "Common/Radar.h"
#include "Common/Player.h"
#include "Common/MapReaderWriterInfo.h"
#include "GameLogic/AI.h"
#include "GameLogic/Object.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/SidesList.h"
#include "GameClient/ShellHooks.h"
#include "Common/SubsystemInterface.h"
#include "Common/LocalFileSystem.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/INI.h"
#include "Win32Device/Common/Win32LocalFileSystem.h"
#include "Win32Device/Common/Win32BIGFileSystem.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <map>

HINSTANCE ApplicationHInstance = NULL;
HWND ApplicationHWnd = NULL;
const char *gAppPrefix = "dc_";
const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";

static SubsystemInterfaceList TheSubsystemListRecord;

template<class SUBSYSTEM>
void initSubsystem(SUBSYSTEM*& sysref, SUBSYSTEM* sys, const char* path1 = NULL, const char* path2 = NULL)
{
	sysref = sys;
	TheSubsystemListRecord.initSubsystem(sys, path1, path2, NULL);
}

template<class SUBSYSTEM>
void initSubsystemSafe(SUBSYSTEM*& sysref, SUBSYSTEM* sys, const char* defaultPath, const char* standardPath) {
	sysref = sys;
	sys->setName(NULL);
	sys->init();

	INI ini;
	UnsignedInt filesRead = 0;

	if (defaultPath) {
		try {
			filesRead += ini.loadFileDirectory(defaultPath, INI_LOAD_OVERWRITE, NULL);
		} catch (...) {
		}
	}
	if (standardPath) {
		try {
			filesRead += ini.loadFileDirectory(standardPath, INI_LOAD_OVERWRITE, NULL);
		} catch (...) {
		}
	}

	TheSubsystemListRecord.addSubsystem(sys);
}



class SimpleFileOutputStream : public OutputStream
{
	FILE* m_file;
public:
	SimpleFileOutputStream(FILE* file) : m_file(file) {}
	Int write(const void *pData, Int numBytes) override
	{
		return fwrite(pData, 1, numBytes, m_file);
	}
};

struct RawChunk
{
	AsciiString label;
	DataChunkVersionType version;
	std::vector<unsigned char> data;

	RawChunk() : version(0) {}
	RawChunk(const AsciiString& lbl, DataChunkVersionType ver, const unsigned char* buf, size_t len)
		: label(lbl), version(ver), data(buf, buf + len) {}
};

static const char base64_chars[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64_encode(const unsigned char* bytes_to_encode, size_t in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i) {
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while((i++ < 3))
			ret += '=';
	}

	return ret;
}

static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

static int base64_char_index(unsigned char c) {
	if (c >= 'A' && c <= 'Z') return c - 'A';
	if (c >= 'a' && c <= 'z') return 26 + (c - 'a');
	if (c >= '0' && c <= '9') return 52 + (c - '0');
	if (c == '+') return 62;
	if (c == '/') return 63;
	return -1;
}

static std::vector<unsigned char> base64_decode(const std::string& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::vector<unsigned char> ret;

	while (in_len-- && (encoded_string[in] != '=') && is_base64(encoded_string[in])) {
		char_array_4[i++] = encoded_string[in]; in++;
		if (i == 4) {
			for (i = 0; i < 4; i++) {
				int idx = base64_char_index(char_array_4[i]);
				if (idx < 0) break;
				char_array_4[i] = idx;
			}

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++) {
			int idx = base64_char_index(char_array_4[j]);
			if (idx >= 0) char_array_4[j] = idx;
		}

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
	}

	return ret;
}

static void extractChunkIDsFromRawData(const unsigned char* data, size_t dataSize, std::set<UnsignedInt>& chunkIDs) {
	size_t pos = 0;
	while (pos + 10 <= dataSize) {
		UnsignedInt id;
		memcpy(&id, data + pos, sizeof(UnsignedInt));
		pos += sizeof(UnsignedInt);

		DataChunkVersionType version;
		memcpy(&version, data + pos, sizeof(DataChunkVersionType));
		pos += sizeof(DataChunkVersionType);

		Int chunkSize;
		memcpy(&chunkSize, data + pos, sizeof(Int));
		pos += sizeof(Int);

		if (chunkSize < 0 || (size_t)chunkSize > dataSize - pos) {
			break;
		}

		chunkIDs.insert(id);

		if (chunkSize > 0) {
			extractChunkIDsFromRawData(data + pos, chunkSize, chunkIDs);
		}

		pos += chunkSize;
	}
}

static void printUsage(const char* exeName)
{
	printf("Usage: %s -in <input_file> -out <output_file>\n", exeName);
	printf("Converts SCB files to JSON and vice versa.\n");
	printf("File format is auto-detected based on extension (.scb or .json)\n");
}


static Bool convertSCBToJSON(const char* inputPath, const char* outputPath)
{
	CachedFileInputStream inputStream;
	if (!inputStream.open(AsciiString(inputPath))) {
		printf("Error: Could not open input file: %s\n", inputPath);
		return false;
	}

	ChunkInputStream *pStrm = &inputStream;
	pStrm->absoluteSeek(0);

	// Read TOC manually to preserve exact order and IDs
	struct TocEntry {
		AsciiString name;
		UnsignedInt id;
	};
	std::vector<TocEntry> tocEntries;

	Byte tag[4];
	pStrm->read((char*)tag, sizeof(tag));
	if (tag[0] != 'C' || tag[1] != 'k' || tag[2] != 'M' || tag[3] != 'p') {
		printf("Error: Invalid SCB file format\n");
		inputStream.close();
		return false;
	}

	Int count;
	pStrm->read((char*)&count, sizeof(Int));

	// Read TOC entries in file order. The file stores entries in the order write() outputs them
	// (which is list order). Since read() prepends entries, the list after reading is in reverse
	// of file order. But the file order IS the write() output order, so we store entries in file order.
	for (Int i = 0; i < count; i++) {
		unsigned char len;
		pStrm->read((char*)&len, sizeof(unsigned char));

		char* nameBuf = new char[len + 1];
		pStrm->read(nameBuf, len);
		nameBuf[len] = '\0';

		UnsignedInt id;
		pStrm->read((char*)&id, sizeof(UnsignedInt));

		TocEntry entry;
		entry.name = AsciiString(nameBuf);
		entry.id = id;
		tocEntries.push_back(entry);

		delete[] nameBuf;
	}

	pStrm->absoluteSeek(0);
	DataChunkInput file(pStrm);

	if (!file.isValidFileType()) {
		printf("Error: Invalid SCB file format\n");
		inputStream.close();
		return false;
	}

	struct ChunkInfo {
		enum Type { PARSED, RAW };
		Type type;
		AsciiString label;
		DataChunkVersionType version;
		ScriptList* scripts[MAX_PLAYER_COUNT];
		Int numScripts;
		RawChunk rawChunk;

		ChunkInfo() : type(RAW), version(0), numScripts(0) {
			for (Int i = 0; i < MAX_PLAYER_COUNT; i++) {
				scripts[i] = NULL;
			}
		}
	};
	std::vector<ChunkInfo> chunks;

	ScriptList *scripts[MAX_PLAYER_COUNT];
	for (Int i = 0; i < MAX_PLAYER_COUNT; i++) {
		scripts[i] = NULL;
	}

	file.registerParser("PlayerScriptsList", AsciiString::TheEmptyString, ScriptList::ParseScriptsDataChunk);

	for (;;) {
		DataChunkVersionType ver;
		AsciiString label = file.openDataChunk(&ver);

		if (label == AsciiString::TheEmptyString) {
			break;
		}

		ChunkInfo chunkInfo;
		chunkInfo.label = label;
		chunkInfo.version = ver;

		if (label == "PlayerScriptsList") {
			chunkInfo.type = ChunkInfo::PARSED;

			for (Int i = 0; i < MAX_PLAYER_COUNT; i++) {
				if (scripts[i]) {
					deleteInstance(scripts[i]);
					scripts[i] = NULL;
				}
			}

			DataChunkInfo info;
			info.label = label;
			info.parentLabel = AsciiString::TheEmptyString;
			info.version = ver;
			info.dataSize = file.getChunkDataSize();

			if (ScriptList::ParseScriptsDataChunk(file, &info, NULL)) {
				Int numLists = ScriptList::getReadScripts(scripts);
				chunkInfo.numScripts = numLists;
				for (Int i = 0; i < numLists; i++) {
					chunkInfo.scripts[i] = scripts[i];
					scripts[i] = NULL;
				}
			} else {
				chunkInfo.type = ChunkInfo::RAW;
				Int dataSize = file.getChunkDataSize();
				if (dataSize > 0) {
					std::vector<unsigned char> rawData(dataSize);
					file.readArrayOfBytes((char*)rawData.data(), dataSize);
					chunkInfo.rawChunk = RawChunk(label, ver, rawData.data(), rawData.size());
				}
			}
		} else {
			chunkInfo.type = ChunkInfo::RAW;
			Int dataSize = file.getChunkDataSize();
			if (dataSize > 0) {
				std::vector<unsigned char> rawData(dataSize);
				file.readArrayOfBytes((char*)rawData.data(), dataSize);
				chunkInfo.rawChunk = RawChunk(label, ver, rawData.data(), rawData.size());
			}
		}

		chunks.push_back(chunkInfo);
		file.closeDataChunk();
	}

	inputStream.close();

	std::map<UnsignedInt, AsciiString> idToName;
	for (size_t i = 0; i < tocEntries.size(); i++) {
		idToName[tocEntries[i].id] = tocEntries[i].name;
	}

	std::set<UnsignedInt> allChunkIDs;
	for (size_t i = 0; i < chunks.size(); i++) {
		if (chunks[i].type == ChunkInfo::RAW && chunks[i].rawChunk.data.size() > 0) {
			extractChunkIDsFromRawData(chunks[i].rawChunk.data.data(), chunks[i].rawChunk.data.size(), allChunkIDs);
		}
	}

	for (auto it = allChunkIDs.begin(); it != allChunkIDs.end(); ++it) {
		UnsignedInt id = *it;
		bool found = false;
		for (size_t i = 0; i < tocEntries.size(); i++) {
			if (tocEntries[i].id == id) {
				found = true;
				break;
			}
		}
		if (!found) {
			auto nameIt = idToName.find(id);
			if (nameIt != idToName.end()) {
				TocEntry entry;
				entry.name = nameIt->second;
				entry.id = id;
				tocEntries.push_back(entry);
			}
		}
	}

	nlohmann::json root;
	root["chunks"] = nlohmann::json::array();
	nlohmann::json mergedToc;

	for (size_t i = 0; i < chunks.size(); i++) {
		if (chunks[i].type == ChunkInfo::PARSED) {
			JSONChunkOutput jsonWriter;
			ScriptList::WriteScriptsDataChunkJSON(jsonWriter, chunks[i].scripts, chunks[i].numScripts);

			nlohmann::json parsedJson = nlohmann::json::parse(jsonWriter.getJSONString());
			if (parsedJson.contains("chunks") && parsedJson["chunks"].is_array() && parsedJson["chunks"].size() > 0) {
				root["chunks"].push_back(parsedJson["chunks"][0]);
			}
			if (parsedJson.contains("toc") && parsedJson["toc"].is_object()) {
				for (auto& [key, value] : parsedJson["toc"].items()) {
					mergedToc[key] = value;
				}
			}
		} else {
			nlohmann::json rawChunkJson;
			rawChunkJson["label"] = chunks[i].label.str();
			rawChunkJson["version"] = chunks[i].version;
			rawChunkJson["type"] = "raw";
			rawChunkJson["data"] = base64_encode(chunks[i].rawChunk.data.data(), chunks[i].rawChunk.data.size());
			root["chunks"].push_back(rawChunkJson);
		}
	}

	for (size_t i = 0; i < chunks.size(); i++) {
		if (chunks[i].type == ChunkInfo::PARSED) {
			for (Int j = 0; j < chunks[i].numScripts; j++) {
				if (chunks[i].scripts[j]) {
					deleteInstance(chunks[i].scripts[j]);
				}
			}
		}
	}

	if (!mergedToc.empty()) {
		root["toc"] = mergedToc;
	}

	// Save TOC mappings in file order (write output order).
	// When writing back, we'll iterate in reverse to restore correct list order.
	nlohmann::json tocArray = nlohmann::json::array();
	for (size_t i = 0; i < tocEntries.size(); i++) {
		nlohmann::json tocEntry;
		tocEntry["name"] = tocEntries[i].name.str();
		tocEntry["id"] = tocEntries[i].id;
		tocArray.push_back(tocEntry);
	}
	root["toc_entries"] = tocArray;

	std::string jsonStr = root.dump(2);

	FILE* outFile = fopen(outputPath, "w");
	if (!outFile) {
		printf("Error: Could not open output file: %s\n", outputPath);
		return false;
	}

	fwrite(jsonStr.c_str(), 1, jsonStr.length(), outFile);
	fclose(outFile);

	return true;
}

static Bool convertJSONToSCB(const char* inputPath, const char* outputPath)
{
	FILE* inFile = fopen(inputPath, "rb");
	if (!inFile) {
		printf("Error: Could not open input file: %s\n", inputPath);
		return false;
	}

	fseek(inFile, 0, SEEK_END);
	long fileSize = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);

	char* jsonData = new char[fileSize + 1];
	size_t bytesRead = fread(jsonData, 1, fileSize, inFile);
	fclose(inFile);
	jsonData[bytesRead] = '\0';

	nlohmann::json root = nlohmann::json::parse(jsonData);
	delete[] jsonData;

	if (!root.contains("chunks") || !root["chunks"].is_array()) {
		printf("Error: Invalid JSON file format - missing chunks array\n");
		return false;
	}

	FILE* outFile = fopen(outputPath, "wb");
	if (!outFile) {
		printf("Error: Could not open output file: %s\n", outputPath);
		return false;
	}

	Int parsedChunks = 0;
	Int rawChunks = 0;

	{
		SimpleFileOutputStream outputStream(outFile);
		DataChunkOutput chunkWriter(&outputStream);

		if (root.contains("toc_entries") && root["toc_entries"].is_array()) {
			const auto& tocArray = root["toc_entries"];
			for (int i = (int)tocArray.size() - 1; i >= 0; i--) {
				const auto& entry = tocArray[i];
				if (entry.contains("name") && entry.contains("id")) {
					std::string nameStr = entry["name"].get<std::string>();
					UnsignedInt id = entry["id"].get<UnsignedInt>();
					chunkWriter.setTOCEntry(AsciiString(nameStr.c_str()), id);
				}
			}
		}

		std::set<UnsignedInt> rawChunkIDs;
		for (size_t i = 0; i < root["chunks"].size(); i++) {
			const auto& chunkJson = root["chunks"][i];
			if (chunkJson.contains("type") && chunkJson["type"] == "raw" && chunkJson.contains("data")) {
				std::string base64Data = chunkJson["data"].get<std::string>();
				std::vector<unsigned char> rawData = base64_decode(base64Data);
				extractChunkIDsFromRawData(rawData.data(), rawData.size(), rawChunkIDs);
			}
		}

		std::map<UnsignedInt, std::string> idToName;
		if (root.contains("toc_entries") && root["toc_entries"].is_array()) {
			for (const auto& entry : root["toc_entries"]) {
				if (entry.contains("name") && entry.contains("id")) {
					idToName[entry["id"].get<UnsignedInt>()] = entry["name"].get<std::string>();
				}
			}
		}

		for (UnsignedInt id : rawChunkIDs) {
			auto it = idToName.find(id);
			if (it != idToName.end()) {
				chunkWriter.setTOCEntry(AsciiString(it->second.c_str()), id);
			}
		}

		for (size_t i = 0; i < root["chunks"].size(); i++) {
			nlohmann::json& chunkJson = root["chunks"][i];

			if (!chunkJson.contains("label") || !chunkJson.contains("version")) {
				printf("Warning: Skipping chunk %d - missing label or version\n", (int)i);
				continue;
			}

			std::string label = chunkJson["label"].get<std::string>();
			DataChunkVersionType version = chunkJson["version"].get<DataChunkVersionType>();

			if (chunkJson.contains("type") && chunkJson["type"] == "raw") {
				if (!chunkJson.contains("data") || !chunkJson["data"].is_string()) {
					printf("Warning: Skipping raw chunk %s - missing or invalid data\n", label.c_str());
					continue;
				}

				std::string base64Data = chunkJson["data"].get<std::string>();
				std::vector<unsigned char> rawData = base64_decode(base64Data);

				chunkWriter.openDataChunk(label.c_str(), version);
				if (rawData.size() > 0) {
					chunkWriter.writeArrayOfBytes((char*)rawData.data(), (Int)rawData.size());
				}
				chunkWriter.closeDataChunk();

				rawChunks++;
			} else {
				if (label == "PlayerScriptsList") {
					nlohmann::json singleChunkRoot;
					singleChunkRoot["chunks"] = nlohmann::json::array();
					singleChunkRoot["chunks"].push_back(chunkJson);
					if (root.contains("toc") && root["toc"].is_object()) {
						singleChunkRoot["toc"] = root["toc"];
					}

					std::string singleChunkJsonStr = singleChunkRoot.dump();
					JSONChunkInput singleChunkInput(singleChunkJsonStr.c_str(), singleChunkJsonStr.length());

					ScriptList *scripts[MAX_PLAYER_COUNT];
					for (Int j = 0; j < MAX_PLAYER_COUNT; j++) {
						scripts[j] = NULL;
					}

					singleChunkInput.registerParser("PlayerScriptsList", AsciiString::TheEmptyString, ScriptList::ParseScriptsDataChunkJSON);
					if (singleChunkInput.parse(NULL)) {
						Int numLists = ScriptList::getReadScripts(scripts);

						ScriptList::WriteScriptsDataChunk(chunkWriter, scripts, numLists);

						for (Int j = 0; j < numLists; j++) {
							if (scripts[j]) {
								deleteInstance(scripts[j]);
							}
						}

						parsedChunks++;
					} else {
						printf("Warning: Failed to parse %s chunk\n", label.c_str());
					}
				} else {
					printf("Warning: Unknown parsed chunk type: %s\n", label.c_str());
				}
			}
		}
	}

	fflush(outFile);
	fclose(outFile);

	return true;
}

int main(int argc, char* argv[])
{
	initMemoryManager();

	const char* gameDir = "C:\\Program Files\\EA Games\\Command and Conquer Generals Zero Hour\\Command and Conquer Generals";

	char currentDir[MAX_PATH];
	if (!GetCurrentDirectoryA(MAX_PATH, currentDir)) {
		printf("Error: Could not get current directory. Error code: %d\n", GetLastError());
		return 1;
	}

	DWORD dataAttrs = GetFileAttributesA("Data");
	bool inGameDir = (dataAttrs != INVALID_FILE_ATTRIBUTES && (dataAttrs & FILE_ATTRIBUTE_DIRECTORY));

	if (!inGameDir) {
		if (!SetCurrentDirectoryA(gameDir)) {
			printf("Error: Could not set working directory to game directory: %s\n", gameDir);
			return 1;
		}

		dataAttrs = GetFileAttributesA("Data");
		if (dataAttrs == INVALID_FILE_ATTRIBUTES || !(dataAttrs & FILE_ATTRIBUTE_DIRECTORY)) {
			printf("Error: Data directory not found in game directory.\n");
			printf("Expected: %s\\Data\n", gameDir);
			return 1;
		}
	}

	if (argc != 5) {
		printUsage(argv[0]);
		return 1;
	}

	const char* inputPath = NULL;
	const char* outputPath = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-in") == 0 && i + 1 < argc) {
			inputPath = argv[++i];
		} else if (strcmp(argv[i], "-out") == 0 && i + 1 < argc) {
			outputPath = argv[++i];
		}
	}

	if (!inputPath || !outputPath) {
		printUsage(argv[0]);
		return 1;
	}

	const char* inputExt = strrchr(inputPath, '.');
	const char* outputExt = strrchr(outputPath, '.');

	if (!inputExt || !outputExt) {
		printf("Error: Input and output files must have extensions\n");
		return 1;
	}

	Bool isInputSCB = (strcmp(inputExt, ".scb") == 0);
	Bool isInputJSON = (strcmp(inputExt, ".json") == 0);
	Bool isOutputSCB = (strcmp(outputExt, ".scb") == 0);
	Bool isOutputJSON = (strcmp(outputExt, ".json") == 0);

	if (!isInputSCB && !isInputJSON) {
		printf("Error: Input file must be .scb or .json\n");
		return 1;
	}

	if (!isOutputSCB && !isOutputJSON) {
		printf("Error: Output file must be .scb or .json\n");
		return 1;
	}

	if ((isInputSCB && isOutputSCB) || (isInputJSON && isOutputJSON)) {
		printf("Error: Input and output formats must be different (SCB <-> JSON)\n");
		return 1;
	}

	TheFileSystem = new FileSystem;

	TheNameKeyGenerator = new NameKeyGenerator();
	TheNameKeyGenerator->init();

	initSubsystem(TheLocalFileSystem, (LocalFileSystem*)new Win32LocalFileSystem);
	initSubsystem(TheArchiveFileSystem, (ArchiveFileSystem*)new Win32BIGFileSystem);

	initSubsystemSafe(TheWritableGlobalData, new GlobalData(), "Data\\INI\\Default\\GameData", "Data\\INI\\GameData");
	initSubsystem(TheGameText, CreateGameTextInterface());
	initSubsystemSafe(TheScienceStore, new ScienceStore(), "Data\\INI\\Default\\Science", "Data\\INI\\Science");
	initSubsystemSafe(TheMultiplayerSettings, new MultiplayerSettings(), "Data\\INI\\Default\\Multiplayer", "Data\\INI\\Multiplayer");
	initSubsystemSafe(TheTerrainTypes, new TerrainTypeCollection(), "Data\\INI\\Default\\TerrainTypes", "Data\\INI\\TerrainTypes");
	initSubsystemSafe(TheThingFactory, new ThingFactory(), "Data\\INI\\Default\\Thing", "Data\\INI\\Thing");
	initSubsystem(TheScriptEngine, new ScriptEngine());

	TheSubsystemListRecord.postProcessLoadAll();

	Bool success = false;
	if (isInputSCB && isOutputJSON) {
		success = convertSCBToJSON(inputPath, outputPath);
	} else if (isInputJSON && isOutputSCB) {
		success = convertJSONToSCB(inputPath, outputPath);
	}

	if (!success) {
		printf("Conversion failed\n");
		return 1;
	}

	printf("Conversion successful: %s -> %s\n", inputPath, outputPath);

	TheSubsystemListRecord.shutdownAll();

	delete TheFileSystem;
	TheFileSystem = NULL;

	delete TheNameKeyGenerator;
	TheNameKeyGenerator = NULL;

	shutdownMemoryManager();

	return 0;
}

Int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, Int nCmdShow)
{
	ApplicationHInstance = hInstance;
	int argc = __argc;
	char** argv = __argv;
	return main(argc, argv);
}
