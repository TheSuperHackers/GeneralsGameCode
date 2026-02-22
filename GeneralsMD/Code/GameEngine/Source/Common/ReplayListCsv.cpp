/*
**	Command & Conquer Generals Zero Hour(tm)
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

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ReplayListCsv.h"
#include "Common/Recorder.h"
#include "Common/FileSystem.h"
#include "GameClient/MapUtil.h"


Bool GetReplayMapInfo(const AsciiString& filename, RecorderClass::ReplayHeader *headerOut, ReplayGameInfo *infoOut, const MapMetaData **mdOut)
{
	// lets get some info about the replay
	RecorderClass::ReplayHeader header;
	header.forPlayback = FALSE;
	header.filename = filename;
	Bool success = TheRecorder && TheMapCache && TheRecorder->readReplayHeader( header );
	if (!success)
		return false;

	ReplayGameInfo info;
	if (!ParseAsciiStringToGameInfo( &info, header.gameOptions ))
		return false;

	header.replayName.translate(filename);
	for (Int tmp=0; tmp < TheRecorder->getReplayExtention().getLength(); ++tmp)
		header.replayName.removeLastChar();

	if (headerOut) *headerOut = header;
	if (infoOut) *infoOut = info;
	if (mdOut) *mdOut = TheMapCache->findMap(info.getMap());
	return true;
}

bool WriteOutReplayList(AsciiString relativeFolder)
{
	AsciiString dir;
	dir.format("%s%s", TheRecorder->getReplayDir().str(), relativeFolder.str());
	if (!dir.endsWith("\\") && !dir.endsWith("/"))
		dir.concat('/');
	AsciiString fname;
	fname.format("%s/replay_list.csv", dir.str());
	FILE *fp = fopen(fname.str(), "wt");
	if (!fp)
		return false;

	// Get list of replay filenames
	AsciiString asciisearch;
	asciisearch = "*";
	asciisearch.concat(TheRecorder->getReplayExtention());
	FilenameList replayFilenamesSet;
	TheFileSystem->getFileListInDirectory(dir, asciisearch, replayFilenamesSet, FALSE);
	std::vector<AsciiString> replayFilenames;
	for (FilenameListIter it = replayFilenamesSet.begin(); it != replayFilenamesSet.end(); ++it)
	{
		replayFilenames.push_back(*it);
	}

	TheMapCache->updateCache();

	std::set<int> foundSeeds;

	// Print out a line per filename. i = -1 is csv header.
	for (int i = -1; i < (int)replayFilenames.size(); i++)
	{
		AsciiString filename;
		RecorderClass::ReplayHeader header;
		ReplayGameInfo info;
		const MapMetaData *md = NULL;
		if (i != -1)
		{
			filename.set(replayFilenames[i].str() + TheRecorder->getReplayDir().getLength());
			Bool success = GetReplayMapInfo(filename, &header, &info, &md);
			if (!success)
				continue;
			filename.set(replayFilenames[i].str() + dir.getLength());
		}
		int numHumans = 0, numAIs = 0;
		for (int slot = 0; slot < MAX_SLOTS; slot++)
		{
			SlotState state = info.getSlot(slot)->getState();
			numHumans += state == SLOT_PLAYER ? 1 : 0;
			numAIs += state >= SLOT_EASY_AI && state <= SLOT_BRUTAL_AI ? 1 : 0;
		}

		bool compatibleVersion =
				header.versionString == UnicodeString(L"Version 1.4") ||
				header.versionString == UnicodeString(L"Version 1.04") ||
				header.versionString == UnicodeString(L"Version 1.05") ||
				header.versionString == UnicodeString(L"\x0412\x0435\x0440\x0441\x0438\x044f 1.04") ||
				header.versionString == UnicodeString(L"\x0412\x0435\x0440\x0441\x0438\x044f 1.05") ||
				header.versionString == UnicodeString(L"\x0412\x0435\x0440\x0441\x0438\x044f 1.4") ||
				header.versionString == UnicodeString(L"Versi\x00F3n 1.04") ||
				header.versionString == UnicodeString(L"Versi\x00F3n 1.05");

		// Some versions, e.g. "Zero Hour 1.04 The Ultimate Collection" have a different ini crc and are
		// actually incompatible. Mark them as incompatible
		compatibleVersion = compatibleVersion &&
				(header.iniCRC == 0xfeaae3f3 || header.iniCRC == 0xb859d2f9);

		// Check whether random seed appears multiple times. This can be used to check only one replay
		// per game in case multiple replays by different players of the same game are in the list.
		int seed = info.getSeed();
		bool uniqueSeed = foundSeeds.find(seed) == foundSeeds.end();
		if (uniqueSeed)
			foundSeeds.insert(seed);

		// When a csv file is loaded with -simReplayList, check indicates whether the replay should be simulated.
		// If you want to check replays with certain properties, you can change this expression
		// or change the csv file manually.
		bool check = md && !header.desyncGame && header.endTime != 0 &&
			compatibleVersion && uniqueSeed;// && numHumans > 1 && numAIs > 0;
		fprintf(fp, "%s", i == -1 ? "check" : check ? "1" : "0");

		if (i == -1)
			fprintf(fp, ",filename");
		else
			fprintf(fp, ",\"%s\"", filename.str());
		
		const char* mapName = info.getMap().reverseFind('\\');
		fprintf(fp, ",%s", i == -1 ? "map" : mapName ? mapName+1 : "");

		fprintf(fp, ",%s", i == -1 ? "mapExists" : md ? "1" : "0");
		fprintf(fp, ",%s", i == -1 ? "mismatch"   : header.desyncGame ? "1" : "0");
		fprintf(fp, ",%s", i == -1 ? "crash"      : header.endTime == 0 ? "1" : "0");
		fprintf(fp, i == -1 ? ",frames" : ",%d",    header.frameCount);

		UnsignedInt gameTime = header.frameCount / LOGICFRAMES_PER_SECOND;
		fprintf(fp, i == -1 ? ",time" : ",%02d:%02d", gameTime/60, gameTime%60);

		fprintf(fp, i == -1 ? ",numHumans" : ",%d", numHumans);
		fprintf(fp, i == -1 ? ",numAIs" : ",%d",    numAIs);

		AsciiString tmp;
		tmp.translate(header.versionString);
		if (i == -1)
			fprintf(fp, ",version");
		else
			fprintf(fp, ",\"%s\"", tmp.str());
		//fprintf(fp, i == -1 ? ",exeCRC" : ",0x%08x",    header.exeCRC);
		//fprintf(fp, i == -1 ? ",iniCRC" : ",0x%08x",    header.iniCRC);

		fprintf(fp, ",%s", i == -1 ? "compatibleVersion" : compatibleVersion ? "1" : "0");

		//fprintf(fp, i == -1 ? ",crcInterval" : ",%d", info.getCRCInterval());
		//fprintf(fp, i == -1 ? ",seed" : ",0x%08x", seed);

		fprintf(fp, "\n");

#if 0
		if (i != -1 && check)
		{
			AsciiString sourceFilename = replayFilenames[i];
			
			AsciiString targetFilename;
			targetFilename = TheRecorder->getReplayDir();
			targetFilename.concat("filter/");
			targetFilename.concat(filename);

			CopyFile(sourceFilename.str(), targetFilename.str(), FALSE);
		}
#endif
	}
	fclose(fp);
	return true;
}

static bool ReadLineFromFile(FILE *fp, AsciiString *str)
{
	const int bufferSize = 128;
	char buffer[bufferSize];
	str->clear();
	while (true)
	{
		if (fgets(buffer, bufferSize, fp) == NULL)
		{
			str->clear();
			return false;
		}
		buffer[bufferSize-1] = 0; // Should be already nul-terminated, just to be sure
		str->concat(buffer);
		if (strlen(buffer) != bufferSize-1 || buffer[bufferSize-2] == '\n')
			break;
	}
	return true;
}

static void NextToken(AsciiString *string, AsciiString *token, char separator)
{
	const char *tokenStart = string->str();

	const char *str = tokenStart;
	bool inQuotationMarks = false;
	while (*str)
	{
		if (*str == separator && !inQuotationMarks)
			break;
		if (*str == '\"')
			inQuotationMarks = !inQuotationMarks;
		str++;
	}
	const char *tokenEnd = str;

	Int len = tokenEnd - tokenStart;
	char *tmp = token->getBufferForRead(len + 1);
	memcpy(tmp, tokenStart, len);
	tmp[len] = 0;
	token->trim();

	string->set(*tokenEnd == 0 ? tokenEnd : tokenEnd+1);
}

bool ReadReplayListFromCsv(AsciiString filename, std::vector<AsciiString>* replayList)
{
	// Get path of csv file relative to replay folder.
	// Later we will search for replays in that path.
	AsciiString relativeFolder = filename;
	{
		int len = relativeFolder.getLength();
		while (len)
		{
			char c = relativeFolder.getCharAt(len-1);
			if (c == '/' || c == '\\')
				break;
			relativeFolder.removeLastChar();
			len--;
		}
	}

	AsciiString fname;
	fname.format("%s%s", TheRecorder->getReplayDir().str(), filename.str());
	FILE *fp = fopen(fname.str(), "rt");
	if (!fp)
		return false;

	// Parse header
	AsciiString line, token;
	ReadLineFromFile(fp, &line);
	char separator = line.find(';') == NULL ? ',' : ';';

	while (feof(fp) == 0)
	{
		ReadLineFromFile(fp, &line);

		// Parse check
		NextToken(&line, &token, separator);
		if (token != "1")
			continue;

		// Parse filename
		NextToken(&line, &token, separator);
		if (token.isEmpty())
			continue;
		if (token.getCharAt(0) == '\"' && token.getCharAt(token.getLength()-1) == '\"')
		{
			token.set(token.str()+1);
			token.removeLastChar();
		}
		if (!token.isEmpty())
		{
			AsciiString path;
			path.format("%s%s", relativeFolder.str(), token.str());
			replayList->push_back(path);
		}

		// Ignore remaining columns
	}
	fclose(fp);
	return true;
}
