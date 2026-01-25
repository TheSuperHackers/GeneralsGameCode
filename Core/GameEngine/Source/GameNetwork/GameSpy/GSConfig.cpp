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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: GSConfig.cpp
// Author: Matthew D. Campbell, Sept 2002
// Description: GameSpy online config
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameState.h"
#include "GameClient/MapUtil.h"
#include "GameNetwork/GameSpy/GSConfig.h"
#include "GameNetwork/RankPointValue.h"


///////////////////////////////////////////////////////////////////////////////////////

GameSpyConfigInterface *TheGameSpyConfig = nullptr;

class GameSpyConfig : public GameSpyConfigInterface
{
public:
	GameSpyConfig( AsciiString config );
	~GameSpyConfig() {}

	// Pings
	std::list<AsciiString> getPingServers(void)	{ return m_pingServers; }
	Int getNumPingRepetitions(void)							{ return m_pingReps; }
	Int getPingTimeoutInMs(void)								{ return m_pingTimeout; }
	virtual Int getPingCutoffGood( void )				{	return m_pingCutoffGood; }
	virtual Int getPingCutoffBad( void )				{ return m_pingCutoffBad;	}

	// QM
	std::list<AsciiString> getQMMaps(void)			{ return m_qmMaps; }
	Int getQMBotID(void)												{ return m_qmBotID; }
	Int getQMChannel(void)											{ return m_qmChannel; }
	void setQMChannel(Int channel)							{ m_qmChannel = channel; }

	// Player Info
	Int getPointsForRank(Int rank);
	virtual Bool isPlayerVIP(Int id);

	virtual Bool getManglerLocation(Int index, AsciiString& host, UnsignedShort& port);

	// Ladder / Any other external parsing
	AsciiString getLeftoverConfig(void)					{ return m_leftoverConfig; }

	// NAT Timeouts
	virtual Int getTimeBetweenRetries() { return m_natRetryInterval; }
	virtual Int getMaxManglerRetries() { return m_natMaxManglerRetries; }
	virtual time_t getRetryInterval() { return m_natManglerRetryInterval; }
	virtual time_t getKeepaliveInterval() { return m_natKeepaliveInterval; }
	virtual time_t getPortTimeout() { return m_natPortTimeout; }
	virtual time_t getRoundTimeout() { return m_natRoundTimeout; }

	// Custom match
	virtual Bool restrictGamesToLobby() { return m_restrictGamesToLobby; }

protected:
	std::list<AsciiString> m_pingServers;
	Int m_pingReps;
	Int m_pingTimeout;
	Int m_pingCutoffGood;
	Int m_pingCutoffBad;

	Int m_natRetryInterval;
	Int m_natMaxManglerRetries;
	time_t m_natManglerRetryInterval;
	time_t m_natKeepaliveInterval;
	time_t m_natPortTimeout;
	time_t m_natRoundTimeout;

	std::vector<AsciiString> m_manglerHosts;
	std::vector<UnsignedShort> m_manglerPorts;

	std::list<AsciiString> m_qmMaps;
	Int m_qmBotID;
	Int m_qmChannel;

	Bool m_restrictGamesToLobby;

	std::set<Int> m_vip; // VIP people

	Int m_rankPoints[MAX_RANKS];

	AsciiString m_leftoverConfig;
};

///////////////////////////////////////////////////////////////////////////////////////

GameSpyConfigInterface* GameSpyConfigInterface::create(AsciiString config)
{
	return NEW GameSpyConfig(config);
}

///////////////////////////////////////////////////////////////////////////////////////

class SectionChecker
{
public:
	typedef std::list<const Bool *> SectionList;
	void addVar(const Bool *var) { m_bools.push_back(var); }
	Bool isInSection();
protected:
	 SectionList m_bools;
};
Bool SectionChecker::isInSection() {
	Bool ret = FALSE;
	for (SectionList::const_iterator it = m_bools.begin(); it != m_bools.end(); ++it)
	{
		ret = ret || **it;
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////

GameSpyConfig::GameSpyConfig( AsciiString config ) :
m_natRetryInterval(1000),
m_natMaxManglerRetries(25),
m_natManglerRetryInterval(300),
m_natKeepaliveInterval(15000),
m_natPortTimeout(10000),
m_natRoundTimeout(10000),
m_pingReps(1),
m_pingTimeout(1000),
m_pingCutoffGood(300),
m_pingCutoffBad(600),
m_restrictGamesToLobby(FALSE),
m_qmBotID(0),
m_qmChannel(0)
{
	m_rankPoints[0] = 0;
	m_rankPoints[1] = 5;
	m_rankPoints[2] = 10;
	m_rankPoints[3] = 20;
	m_rankPoints[4] = 50;
	m_rankPoints[5] = 100;
	m_rankPoints[6] = 200;
	m_rankPoints[7] = 500;
	m_rankPoints[8] = 1000;
	m_rankPoints[9] = 2000;

	AsciiString line;
	Bool inPingServers = FALSE;
	Bool inPingDuration = FALSE;
	Bool inQMMaps = FALSE;
	Bool inQMBot = FALSE;
	Bool inManglers = FALSE;
	Bool inVIP = FALSE;
	Bool inNAT = FALSE;
	Bool inCustom = FALSE;

	SectionChecker sections;
	sections.addVar(&inPingServers);
	sections.addVar(&inPingDuration);
	sections.addVar(&inQMMaps);
	sections.addVar(&inQMBot);
	sections.addVar(&inManglers);
	sections.addVar(&inVIP);
	sections.addVar(&inNAT);
	sections.addVar(&inCustom);

	while (config.nextToken(&line, "\n"))
	{
		if (line.getCharAt(line.getLength()-1) == '\r')
			line.removeLastChar();	// there is a trailing '\r'

		line.trim();

		if (line.isEmpty())
			continue;

		if (!sections.isInSection() && line.compare("<PingServers>") == 0)
		{
			inPingServers = TRUE;
		}
		else if (inPingServers && line.compare("</PingServers>") == 0)
		{
			inPingServers = FALSE;
		}
		else if (!sections.isInSection() && line.compare("<PingDuration>") == 0)
		{
			inPingDuration = TRUE;
		}
		else if (inPingDuration && line.compare("</PingDuration>") == 0)
		{
			inPingDuration = FALSE;
		}
		else if (!sections.isInSection() && line.compare("<QMMaps>") == 0)
		{
			inQMMaps = TRUE;
		}
		else if (inQMMaps && line.compare("</QMMaps>") == 0)
		{
			inQMMaps = FALSE;
		}
		else if (!sections.isInSection() && line.compare("<Manglers>") == 0)
		{
			inManglers = TRUE;
		}
		else if (inManglers && line.compare("</Manglers>") == 0)
		{
			inManglers = FALSE;
		}
		else if (!sections.isInSection() && line.compare("<QMBot>") == 0)
		{
			inQMBot = TRUE;
		}
		else if (inQMBot && line.compare("</QMBot>") == 0)
		{
			inQMBot = FALSE;
		}
		else if (!sections.isInSection() && line.compare("<VIP>") == 0)
		{
			inVIP = TRUE;
		}
		else if (inVIP && line.compare("</VIP>") == 0)
		{
			inVIP = FALSE;
		}
		else if (!sections.isInSection() && line.compare("<NAT>") == 0)
		{
			inNAT = TRUE;
		}
		else if (inNAT && line.compare("</NAT>") == 0)
		{
			inNAT = FALSE;
		}
		else if (!sections.isInSection() && line.compare("<Custom>") == 0)
		{
			inCustom = TRUE;
		}
		else if (inCustom && line.compare("</Custom>") == 0)
		{
			inCustom = FALSE;
		}
		else if (inVIP)
		{
			line.toLower();
			if (line.getLength())
			{
				Int val = atoi(line.str());
				if (val > 0)
					m_vip.insert(val);
			}
		}
		else if (inPingServers)
		{
			line.toLower();
			m_pingServers.push_back(line);
		}
		else if (inPingDuration)
		{
			line.toLower();
			AsciiString key, val;
			if (line.nextToken(&key, " ="))
			{
				if (key == "reps")
				{
					if (line.nextToken(&val, " ="))
					{
						m_pingReps = atoi(val.str());
					}
				}
				else if (key == "timeout")
				{
					if (line.nextToken(&val, " ="))
					{
						m_pingTimeout = atoi(val.str());
					}
				}
				else if (key == "low")
				{
					if (line.nextToken(&val, " ="))
					{
						m_pingCutoffGood = atoi(val.str());
					}
				}
				else if (key == "med")
				{
					if (line.nextToken(&val, " ="))
					{
						m_pingCutoffBad = atoi(val.str());
					}
				}
			}
		}
		else if (inManglers)
		{
			line.trim();
			line.toLower();
			AsciiString hostStr;
			AsciiString portStr;
			line.nextToken(&hostStr, ":");
			line.nextToken(&portStr, ":\n\r");
			if (hostStr.isNotEmpty() && portStr.isNotEmpty())
			{
				m_manglerHosts.push_back(hostStr);
				m_manglerPorts.push_back(atoi(portStr.str()));
			}
		}
		else if (inQMMaps)
		{
			line.toLower();
			AsciiString mapName;
			mapName.format("%s\\%s\\%s.map", TheMapCache->getMapDir().str(), line.str(), line.str());
			mapName = TheGameState->portableMapPathToRealMapPath(TheGameState->realMapPathToPortableMapPath(mapName));
			mapName.toLower();

			// [SKB: Jul 01 2003 @ 6:43pm] :
			// German2 is missing some maps because of content.  But, we need the m_qmMaps
			// to contain same number of strings as the Retail version so that the
			// QM Bot thinks that they have the same number of maps.
			#if RTS_GENERALS
			m_qmMaps.push_back(mapName);
			#else
			const MapMetaData *md = TheMapCache->findMap(mapName);
			if (md)
			{
				m_qmMaps.push_back(mapName);
			}
			#endif
		}
		else if (inQMBot)
		{
			line.toLower();
			AsciiString key, val;
			if (line.nextToken(&key, " ="))
			{
				if (key == "id")
				{
					if (line.nextToken(&val, " ="))
					{
						m_qmBotID = atoi(val.str());
					}
				}
			}
		}
		else if (inNAT)
		{
			line.toLower();
			AsciiString key, val;
			if (line.nextToken(&key, " ="))
			{
				if (key == "retryinterval")
				{
					if (line.nextToken(&val, " ="))
					{
						m_natRetryInterval = atoi(val.str());
					}
				}
				else if (key == "manglerretries")
				{
					if (line.nextToken(&val, " ="))
					{
						m_natMaxManglerRetries = atoi(val.str());
					}
				}
				else if (key == "manglerinterval")
				{
					if (line.nextToken(&val, " ="))
					{
						m_natManglerRetryInterval = atoi(val.str());
					}
				}
				else if (key == "keepaliveinterval")
				{
					if (line.nextToken(&val, " ="))
					{
						m_natKeepaliveInterval = atoi(val.str());
					}
				}
				else if (key == "porttimeout")
				{
					if (line.nextToken(&val, " ="))
					{
						m_natPortTimeout = atoi(val.str());
					}
				}
				else if (key == "roundtimeout")
				{
					if (line.nextToken(&val, " ="))
					{
						m_natRoundTimeout = atoi(val.str());
					}
				}
				else
				{
					DEBUG_LOG(("Unknown key '%s' = '%s' in NAT block of GameSpy Config", key.str(), val.str()));
				}
			}
			else
			{
				DEBUG_LOG(("Key '%s' missing val in NAT block of GameSpy Config", key.str()));
			}
		}
		else if (inCustom)
		{
			line.toLower();
			AsciiString key, val;
			if (line.nextToken(&key, " =") && line.nextToken(&val, " ="))
			{
				if (key == "restricted")
				{
					m_restrictGamesToLobby = atoi(val.str());
				}
				else
				{
					DEBUG_LOG(("Unknown key '%s' = '%s' in Custom block of GameSpy Config", key.str(), val.str()));
				}
			}
			else
			{
				DEBUG_LOG(("Key '%s' missing val in Custom block of GameSpy Config", key.str()));
			}
		}
		else
		{
			m_leftoverConfig.concat(line);
			m_leftoverConfig.concat('\n');
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////////

Int GameSpyConfig::getPointsForRank(Int rank)
{
	if (rank >= MAX_RANKS) rank = MAX_RANKS-1;
	if (rank < 0) rank = 0;
	return m_rankPoints[rank];
}

///////////////////////////////////////////////////////////////////////////////////////

Bool GameSpyConfig::getManglerLocation(Int index, AsciiString& host, UnsignedShort& port)
{
	if (index < 0 || index >= m_manglerHosts.size())
	{
		return FALSE;
	}

	host = m_manglerHosts[index];
	port = m_manglerPorts[index];
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////

Bool GameSpyConfig::isPlayerVIP(Int id)
{
	std::set<Int>::const_iterator it = std::find(m_vip.begin(), m_vip.end(), id);
	return it != m_vip.end();
}

///////////////////////////////////////////////////////////////////////////////////////
