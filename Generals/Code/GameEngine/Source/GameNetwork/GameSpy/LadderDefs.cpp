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

// FILE: LadderDefs.cpp //////////////////////////////////////////////////////
// Generals ladder code
// Author: Matthew D. Campbell, August 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameNetwork/GameSpy/ThreadUtils.h"
#include "GameNetwork/GameSpy/LadderDefs.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/GSConfig.h"
#include "Common/GameState.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/PlayerTemplate.h"
#include "GameClient/GameText.h"
#include "GameClient/MapUtil.h"


LadderList *TheLadderList = NULL;

LadderInfo::LadderInfo()
{
	playersPerTeam = 1;
	minWins = 0;
	maxWins = 0;
	randomMaps = TRUE;
	randomFactions = TRUE;
	validQM = TRUE;
	validCustom = FALSE;
	port = 0;
	submitReplay = FALSE;
	index = -1;
}

static LadderInfo *parseLadder(AsciiString raw)
{
	DEBUG_LOG(("Looking at ladder:\n%s", raw.str()));
	LadderInfo *lad = NULL;
	AsciiString line;
	while (raw.nextToken(&line, "\n"))
	{
		if (line.getCharAt(line.getLength()-1) == '\r')
			line.removeLastChar();	// there is a trailing '\r'

		line.trim();

		if (line.isEmpty())
			continue;

		// woohoo!  got a line!
		line.trim();
		if ( !lad && line.startsWith("<Ladder ") )
		{
			// start of a ladder def
			lad = NEW LadderInfo;

			// fill in some info
			AsciiString tokenName, tokenAddr, tokenPort, tokenHomepage;
			line.removeLastChar(); // the '>'
			line = line.str() + 7; // the "<Ladder "
			line.nextToken(&tokenAddr, "\" ");
			line.nextToken(&tokenPort, " ");
			line.nextToken(&tokenHomepage, " ");

			lad->name = MultiByteToWideCharSingleLine(tokenName.str()).c_str();
			lad->name.truncateTo(20); // Per Harvard's request, ladder names are limited to 20 chars
			lad->address = tokenAddr;
			lad->port = atoi(tokenPort.str());
			lad->homepageURL = tokenHomepage;
		}
		else if ( lad && line.startsWith("Name ") )
		{
			lad->name = MultiByteToWideCharSingleLine(line.str() + 5).c_str();
		}
		else if ( lad && line.startsWith("Desc ") )
		{
			lad->description = MultiByteToWideCharSingleLine(line.str() + 5).c_str();
		}
		else if ( lad && line.startsWith("Loc ") )
		{
			lad->location = MultiByteToWideCharSingleLine(line.str() + 4).c_str();
		}
		else if ( lad && line.startsWith("TeamSize ") )
		{
			lad->playersPerTeam = atoi(line.str() + 9);
		}
		else if ( lad && line.startsWith("RandomMaps ") )
		{
			lad->randomMaps = atoi(line.str() + 11);
		}
		else if ( lad && line.startsWith("RandomFactions ") )
		{
			lad->randomFactions = atoi(line.str() + 15);
		}
		else if ( lad && line.startsWith("Faction ") )
		{
			AsciiString faction = line.str() + 8;
			AsciiStringList outStringList;
			ThePlayerTemplateStore->getAllSideStrings(&outStringList);

			AsciiStringList::iterator aIt = std::find(outStringList.begin(), outStringList.end(), faction);
			if (aIt != outStringList.end())
			{
				// valid faction - now check for dupes
				aIt = std::find(lad->validFactions.begin(), lad->validFactions.end(), faction);
				if (aIt == lad->validFactions.end())
				{
					lad->validFactions.push_back(faction);
				}
			}
		}
		/*
		else if ( lad && line.startsWith("QM ") )
		{
			lad->validQM = atoi(line.str() + 3);
		}
		else if ( lad && line.startsWith("Custom ") )
		{
			lad->validCustom = atoi(line.str() + 7);
		}
		*/
		else if ( lad && line.startsWith("MinWins ") )
		{
			lad->minWins = atoi(line.str() + 8);
		}
		else if ( lad && line.startsWith("MaxWins ") )
		{
			lad->maxWins = atoi(line.str() + 8);
		}
		else if ( lad && line.startsWith("CryptedPass ") )
		{
			lad->cryptedPassword = line.str() + 12;
		}
		else if ( lad && line.compare("</Ladder>") == 0 )
		{
			DEBUG_LOG(("Saw a ladder: name=%ls, addr=%s:%d, players=%dv%d, pass=%s, replay=%d, homepage=%s",
				lad->name.str(), lad->address.str(), lad->port, lad->playersPerTeam, lad->playersPerTeam, lad->cryptedPassword.str(),
				lad->submitReplay, lad->homepageURL.str()));
			// end of a ladder
			if (lad->playersPerTeam >= 1 && lad->playersPerTeam <= MAX_SLOTS/2)
			{
				if (lad->validFactions.size() == 0)
				{
					DEBUG_LOG(("No factions specified.  Using all."));
					lad->validFactions.clear();
					Int numTemplates = ThePlayerTemplateStore->getPlayerTemplateCount();
					for ( Int i = 0; i < numTemplates; ++i )
					{
						const PlayerTemplate *pt = ThePlayerTemplateStore->getNthPlayerTemplate(i);
						if (!pt)
							continue;

						if (pt->isPlayableSide()  &&  pt->getSide().compare("Boss") != 0 )
							lad->validFactions.push_back(pt->getSide());
					}
				}
				else
				{
					AsciiStringList validFactions = lad->validFactions;
					for (AsciiStringListIterator it = validFactions.begin(); it != validFactions.end(); ++it)
					{
						AsciiString faction = *it;
						AsciiString marker;
						marker.format("INI:Faction%s", faction.str());
						DEBUG_LOG(("Faction %s has marker %s corresponding to str %ls", faction.str(), marker.str(), TheGameText->fetch(marker).str()));
					}
				}

				if (lad->validMaps.size() == 0)
				{
					DEBUG_LOG(("No maps specified.  Using all."));
					std::list<AsciiString> qmMaps = TheGameSpyConfig->getQMMaps();
					for (std::list<AsciiString>::const_iterator it = qmMaps.begin(); it != qmMaps.end(); ++it)
					{
						AsciiString mapName = *it;

						// check sizes on the maps before allowing them
						const MapMetaData *md = TheMapCache->findMap(mapName);
						if (md && md->m_numPlayers >= lad->playersPerTeam*2)
						{
							lad->validMaps.push_back(mapName);
						}
					}
				}
				return lad;
			}
			else
			{
				// no maps?  don't play on it!
				delete lad;
				lad = NULL;
				return NULL;
			}
		}
		else if ( lad && line.startsWith("Map ") )
		{
			// valid map
			AsciiString mapName = line.str() + 4;
			mapName.trim();
			if (mapName.isNotEmpty())
			{
				mapName.format("%s\\%s\\%s.map", TheMapCache->getMapDir().str(), mapName.str(), mapName.str());
				mapName = TheGameState->portableMapPathToRealMapPath(TheGameState->realMapPathToPortableMapPath(mapName));
				mapName.toLower();
				std::list<AsciiString> qmMaps = TheGameSpyConfig->getQMMaps();
				if (std::find(qmMaps.begin(), qmMaps.end(), mapName) != qmMaps.end())
				{
					// check sizes on the maps before allowing them
					const MapMetaData *md = TheMapCache->findMap(mapName);
					if (md && md->m_numPlayers >= lad->playersPerTeam*2)
						lad->validMaps.push_back(mapName);
				}
			}
		}
		else
		{
			// bad ladder - kill it
			delete lad;
			lad = NULL;
		}
	}

	delete lad;
	return NULL;
}

LadderList::LadderList()
{
	//Int profile = TheGameSpyInfo->getLocalProfileID();

	AsciiString rawMotd = TheGameSpyConfig->getLeftoverConfig();
	AsciiString line;
	Bool inLadders = FALSE;
	Bool inSpecialLadders = FALSE;
	Bool inLadder = FALSE;
	LadderInfo *lad = NULL;
	Int index = 1;
	AsciiString rawLadder;

	while (rawMotd.nextToken(&line, "\n"))
	{
		if (line.getCharAt(line.getLength()-1) == '\r')
			line.removeLastChar();	// there is a trailing '\r'

		line.trim();

		if (line.isEmpty())
			continue;

		if (!inLadders && line.compare("<Ladders>") == 0)
		{
			inLadders = TRUE;
			rawLadder.clear();
		}
		else if (inLadders && line.compare("</Ladders>") == 0)
		{
			inLadders = FALSE;
		}
		else if (!inSpecialLadders && line.compare("<SpecialLadders>") == 0)
		{
			inSpecialLadders = TRUE;
			rawLadder.clear();
		}
		else if (inSpecialLadders && line.compare("</SpecialLadders>") == 0)
		{
			inSpecialLadders = FALSE;
		}
		else if (inLadders || inSpecialLadders)
		{
			if (line.startsWith("<Ladder ") && !inLadder)
			{
				inLadder = TRUE;
				rawLadder.clear();
				rawLadder.concat(line);
				rawLadder.concat('\n');
			}
			else if (line.compare("</Ladder>") == 0 && inLadder)
			{
				inLadder = FALSE;
				rawLadder.concat(line);
				rawLadder.concat('\n');
				if ((lad = parseLadder(rawLadder)) != NULL)
				{
					lad->index = index++;
					if (inLadders)
					{
						DEBUG_LOG(("Adding to standard ladders"));
						m_standardLadders.push_back(lad);
					}
					else
					{
						DEBUG_LOG(("Adding to special ladders"));
						m_specialLadders.push_back(lad);
					}
				}
				rawLadder.clear();
			}
			else if (inLadder)
			{
				rawLadder.concat(line);
				rawLadder.concat('\n');
			}
		}
	}

	// look for local ladders
	loadLocalLadders();

	DEBUG_LOG(("After looking for ladders, we have %d local, %d special && %d normal", m_localLadders.size(), m_specialLadders.size(), m_standardLadders.size()));
}

LadderList::~LadderList()
{
	LadderInfoList::iterator it;
	for (it = m_specialLadders.begin(); it != m_specialLadders.end(); it = m_specialLadders.begin())
	{
		delete *it;
		m_specialLadders.pop_front();
	}
	for (it = m_standardLadders.begin(); it != m_standardLadders.end(); it = m_standardLadders.begin())
	{
		delete *it;
		m_standardLadders.pop_front();
	}
	for (it = m_localLadders.begin(); it != m_localLadders.end(); it = m_localLadders.begin())
	{
		delete *it;
		m_localLadders.pop_front();
	}
}

const LadderInfo* LadderList::findLadder( const AsciiString& addr, UnsignedShort port )
{
	LadderInfoList::const_iterator cit;

	for (cit = m_specialLadders.begin(); cit != m_specialLadders.end(); ++cit)
	{
		const LadderInfo *li = *cit;
		if (li->address == addr && li->port == port)
		{
			return li;
		}
	}

	for (cit = m_standardLadders.begin(); cit != m_standardLadders.end(); ++cit)
	{
		const LadderInfo *li = *cit;
		if (li->address == addr && li->port == port)
		{
			return li;
		}
	}

	for (cit = m_localLadders.begin(); cit != m_localLadders.end(); ++cit)
	{
		const LadderInfo *li = *cit;
		if (li->address == addr && li->port == port)
		{
			return li;
		}
	}

	return NULL;
}

const LadderInfo* LadderList::findLadderByIndex( Int index )
{
	if (index == 0)
		return NULL;

	LadderInfoList::const_iterator cit;

	for (cit = m_specialLadders.begin(); cit != m_specialLadders.end(); ++cit)
	{
		const LadderInfo *li = *cit;
		if (li->index == index)
		{
			return li;
		}
	}

	for (cit = m_standardLadders.begin(); cit != m_standardLadders.end(); ++cit)
	{
		const LadderInfo *li = *cit;
		if (li->index == index)
		{
			return li;
		}
	}

	for (cit = m_localLadders.begin(); cit != m_localLadders.end(); ++cit)
	{
		const LadderInfo *li = *cit;
		if (li->index == index)
		{
			return li;
		}
	}

	return NULL;
}

const LadderInfoList* LadderList::getSpecialLadders( void )
{
	return &m_specialLadders;
}

const LadderInfoList* LadderList::getStandardLadders( void )
{
	return &m_standardLadders;
}

const LadderInfoList* LadderList::getLocalLadders( void )
{
	return &m_localLadders;
}

void LadderList::loadLocalLadders( void )
{
	AsciiString dirname;
	dirname.format("%sGeneralsOnline\\Ladders\\", TheGlobalData->getPath_UserData().str());
	FilenameList filenameList;
	TheFileSystem->getFileListInDirectory(dirname, AsciiString("*.ini"), filenameList, TRUE);

	Int index = -1;

	FilenameList::iterator it = filenameList.begin();
	while (it != filenameList.end())
	{
		AsciiString filename = *it;
		DEBUG_LOG(("Looking at possible ladder info file '%s'", filename.str()));
		filename.toLower();
		checkLadder( filename, index-- );
		++it;
	}
}

void LadderList::checkLadder( AsciiString fname, Int index )
{
	File *fp = TheFileSystem->openFile(fname.str(), File::READ | File::TEXT);
	char buf[1024];
	AsciiString rawData;
	if (fp)
	{
		Int len;
		while (!fp->eof())
		{
			len = fp->read(buf, 1023);
			buf[len] = 0;
			buf[1023] = 0;
			rawData.concat(buf);
		}
		fp->close();
		fp = NULL;
	}

	DEBUG_LOG(("Read %d bytes from '%s'", rawData.getLength(), fname.str()));
	if (rawData.isEmpty())
		return;

	LadderInfo *li = parseLadder(rawData);
	if (!li)
	{
		return;
	}

	// sanity check
	if (li->address.isEmpty())
	{
		DEBUG_LOG(("Bailing because of li->address.isEmpty()"));
		delete li;
		return;
	}

	if (!li->port)
	{
		DEBUG_LOG(("Bailing because of !li->port"));
		delete li;
		return;
	}

	if (li->validMaps.size() == 0)
	{
		DEBUG_LOG(("Bailing because of li->validMaps.size() == 0"));
		delete li;
		return;
	}

	li->index = index;

	// ladders are QM-only at this point, which kinda invalidates the whole concept of local ladders.  Oh well.
	li->validQM = FALSE; // no local ladders in QM
	li->validCustom = FALSE;

	//for (Int i=0; i<4; ++i)
	//	fname.removeLastChar(); // remove .lad
	//li->name = UnicodeString(MultiByteToWideCharSingleLine(fname.reverseFind('\\')+1).c_str());

	DEBUG_LOG(("Adding local ladder %ls", li->name.str()));
	m_localLadders.push_back(li);
}
