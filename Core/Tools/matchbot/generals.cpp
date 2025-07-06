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

#ifdef _WIN32
#include <process.h>
#endif

#include <wstring.h>
#include <tcp.h>
#include <wdebug.h>
#include "mydebug.h"

#include <ghttp/ghttp.h>

#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <strstream>

#ifdef _UNIX
using namespace std;
#endif

/*
#ifdef IN
#undef IN
#endif
#define IN const
*/
#include "generals.h"
/*
#ifdef IN
#undef IN
#endif
#define IN const
*/
#include "global.h"
/*
#ifdef IN
#undef IN
#endif
#define IN const
*/

std::string intToString(int val)
{
	std::string s = "";
	bool neg = (val < 0);
	if (val < 0)
	{
		val = -val;
	}
	if (val == 0)
		return "0";

	char buf[2];
	buf[1] = 0;
	while (val)
	{
		buf[0] = '0' + val%10;
		val /= 10;
		s.insert(0, buf);
	}
	if (neg)
		s.insert(0, "-");
	return s;
}

std::string uintToString(unsigned int val)
{
	std::string s = "";
	if (val == 0)
		return "0";

	char buf[2];
	buf[1] = 0;
	while (val)
	{
		buf[0] = '0' + val%10;
		val /= 10;
		s.insert(0, buf);
	}
	return s;
}

MapBitSet MapSetUnion(const MapBitSet& a, const MapBitSet& b)
{
	MapBitSet c;
	if (a.size() != b.size())
		return c;

	for (int i=0; i<(int)a.size(); ++i)
	{
		c.push_back(a[i] && b[i]);
	}

	return c;
}

int MapSetCount(const MapBitSet& a)
{
	int count=0;
	for (int i=0; i<(int)a.size(); ++i)
	{
		//DBGMSG(a[i]);
		if (a[i])
			++count;
	}
	return count;
}

// =====================================================================
// Users
// =====================================================================

GeneralsUser::GeneralsUser(void)
{
	status = STATUS_INCHANNEL;
	points = 1;
	minPoints = maxPoints = 100;
	country = color = -1;
	pseudoPing.clear();
	matchStart = time(NULL);
	timeToWiden = 0;
	widened = false;
	numPlayers = 2;
	discons = maxDiscons = 2;
	maps.clear();
	maxPing = 1000;
}

static const int MaxPingValue = 255*255*2;

int calcPingDelta(const GeneralsUser *a, const GeneralsUser *b)
{
	if (!a || !b || a->pseudoPing.size() != b->pseudoPing.size())
		return MaxPingValue; // Max ping

	int bestPing = MaxPingValue;
	for (int i=0; i<(int)a->pseudoPing.size(); ++i)
	{
		int p1, p2;
		p1 = a->pseudoPing[i];
		p2 = b->pseudoPing[i];

		if (p1 * p1 + p2 * p2 < bestPing)
			bestPing = p1 * p1 + p2 * p2;
	}

	return (int)sqrt(bestPing);
}


// =====================================================================
// Matcher thread
// =====================================================================

GeneralsMatcher::GeneralsMatcher()
{
	// Read some values from the config file
	int quietTMP = 0;
	Global.config.getInt("NOECHO", quietTMP);
	if (quietTMP)
		quiet = true;
	else
		quiet = false;

	// Grab the weights for different parameters
	Global.config.getInt("MATCH_WEIGHT_LOWPING", weightLowPing, "GENERALS");
	Global.config.getInt("MATCH_WEIGHT_AVGPOINTS", weightAvgPoints, "GENERALS");
	totalWeight = weightLowPing + weightAvgPoints;

	INFMSG("weightLowPing = " << weightLowPing);
	INFMSG("weightAvgPoints = " << weightAvgPoints);
	INFMSG("totalWeight = " << totalWeight);

	Global.config.getInt("SecondsBetweenPoolSizeAnnouncements", m_secondsBetweenPoolSizeAnnouncements, NULL);
	if (m_secondsBetweenPoolSizeAnnouncements < 10)
	{
		m_secondsBetweenPoolSizeAnnouncements = 10;
	}
	m_nextPoolSizeAnnouncement = time(NULL);
}

void GeneralsMatcher::init(void)
{}


#define W(x) setw(x) <<
void GeneralsMatcher::dumpUsers(void)
{}



void GeneralsMatcher::sendMatchInfo(std::string name1, std::string name2, std::string name3, std::string name4,
                                    std::string name5, std::string name6, std::string name7, std::string name8,
                                    GeneralsUser *user1, GeneralsUser *user2, GeneralsUser *user3, GeneralsUser *user4,
                                    GeneralsUser *user5, GeneralsUser *user6, GeneralsUser *user7, GeneralsUser *user8,
                                    int numPlayers, int ladderID)
{
	MapBitSet tmp = MapSetUnion(user1->maps, user2->maps);
	if (numPlayers > 2)
	{
		tmp = MapSetUnion(tmp, user3->maps);
		tmp = MapSetUnion(tmp, user4->maps);
	}
	if (numPlayers > 4)
	{
		tmp = MapSetUnion(tmp, user5->maps);
		tmp = MapSetUnion(tmp, user6->maps);
	}
	if (numPlayers > 6)
	{
		tmp = MapSetUnion(tmp, user7->maps);
		tmp = MapSetUnion(tmp, user8->maps);
	}

	int numMaps = MapSetCount(tmp);

	if (!numMaps)
	{
		DBGMSG("No maps!");
		user1->status = STATUS_WORKING;
		user2->status = STATUS_WORKING;
		if (numPlayers > 2)
		{
			user3->status = STATUS_WORKING;
			user4->status = STATUS_WORKING;
		}
		if (numPlayers > 4)
		{
			user5->status = STATUS_WORKING;
			user6->status = STATUS_WORKING;
		}
		if (numPlayers > 6)
		{
			user7->status = STATUS_WORKING;
			user8->status = STATUS_WORKING;
		}
		return;
	}
	user1->status = STATUS_MATCHED;
	user2->status = STATUS_MATCHED;
	if (numPlayers > 2)
	{
		user3->status = STATUS_MATCHED;
		user4->status = STATUS_MATCHED;
	}
	if (numPlayers > 4)
	{
		user5->status = STATUS_MATCHED;
		user6->status = STATUS_MATCHED;
	}
	if (numPlayers > 6)
	{
		user7->status = STATUS_MATCHED;
		user8->status = STATUS_MATCHED;
	}

	int whichMap = Global.rnd.Int(0, RAND_MAX-1);
	DBGMSG(whichMap);
	whichMap = whichMap%numMaps;
	DBGMSG(whichMap);
	++whichMap;
	DBGMSG(whichMap);
	DBGMSG("Random map #" << whichMap << "/" << numMaps);

	int i;
	for (i=0; i<(int)user1->maps.size(); ++i)
	{
		if (tmp[i])
			--whichMap;
		if (whichMap == 0)
			break;
	}
	DBGMSG("Playing on map in pos " << i);

	std::string s;
	s = "MBOT:MATCHED ";
	s.append(intToString(i));
	s.append(" ");
	s.append(intToString( Global.rnd.Int(0, RAND_MAX-1) ));
	s.append(" ");
	s.append(name1);
	s.append(" ");
	s.append(uintToString(user1->IP));
	s.append(" ");
	s.append(intToString(user1->country));
	s.append(" ");
	s.append(intToString(user1->color));
	s.append(" ");
	s.append(intToString(user1->NAT));
	s.append(" ");
	s.append(name2);
	s.append(" ");
	s.append(uintToString(user2->IP));
	s.append(" ");
	s.append(intToString(user2->country));
	s.append(" ");
	s.append(intToString(user2->color));
	s.append(" ");
	s.append(intToString(user2->NAT));
	if (user3)
	{
		s.append(" ");
		s.append(name3);
		s.append(" ");
		s.append(uintToString(user3->IP));
		s.append(" ");
		s.append(intToString(user3->country));
		s.append(" ");
		s.append(intToString(user3->color));
		s.append(" ");
		s.append(intToString(user3->NAT));
	}
	if (user4)
	{
		s.append(" ");
		s.append(name4);
		s.append(" ");
		s.append(uintToString(user4->IP));
		s.append(" ");
		s.append(intToString(user4->country));
		s.append(" ");
		s.append(intToString(user4->color));
		s.append(" ");
		s.append(intToString(user4->NAT));
	}
	if (user5)
	{
		s.append(" ");
		s.append(name5);
		s.append(" ");
		s.append(uintToString(user5->IP));
		s.append(" ");
		s.append(intToString(user5->country));
		s.append(" ");
		s.append(intToString(user5->color));
		s.append(" ");
		s.append(intToString(user5->NAT));
	}
	if (user6)
	{
		s.append(" ");
		s.append(name6);
		s.append(" ");
		s.append(uintToString(user6->IP));
		s.append(" ");
		s.append(intToString(user6->country));
		s.append(" ");
		s.append(intToString(user6->color));
		s.append(" ");
		s.append(intToString(user6->NAT));
	}
	if (user7)
	{
		s.append(" ");
		s.append(name7);
		s.append(" ");
		s.append(uintToString(user7->IP));
		s.append(" ");
		s.append(intToString(user7->country));
		s.append(" ");
		s.append(intToString(user7->color));
		s.append(" ");
		s.append(intToString(user7->NAT));
	}
	if (user8)
	{
		s.append(" ");
		s.append(name8);
		s.append(" ");
		s.append(uintToString(user8->IP));
		s.append(" ");
		s.append(intToString(user8->country));
		s.append(" ");
		s.append(intToString(user8->color));
		s.append(" ");
		s.append(intToString(user8->NAT));
	}

	std::string n;
	n = name1;
	n.append(",");
	n.append(name2);
	if (user3)
	{
		n.append(",");
		n.append(name3);
	}
	if (user4)
	{
		n.append(",");
		n.append(name4);
	}
	if (user5)
	{
		n.append(",");
		n.append(name5);
	}
	if (user6)
	{
		n.append(",");
		n.append(name6);
	}
	if (user7)
	{
		n.append(",");
		n.append(name7);
	}
	if (user8)
	{
		n.append(",");
		n.append(name8);
	}
	peerMessagePlayer(m_peer, n.c_str(), s.c_str(), NormalMessage);
}

void GeneralsMatcher::checkMatches(void)
{
	bool showPoolSize = false;
	time_t now = time(NULL);
	if (now > m_nextPoolSizeAnnouncement)
	{
		m_nextPoolSizeAnnouncement = now + m_secondsBetweenPoolSizeAnnouncements;
		showPoolSize = true;
	}
	checkMatchesInUserMap(m_nonLadderUsers1v1, 0, 2, showPoolSize);
	checkMatchesInUserMap(m_nonLadderUsers2v2, 0, 4, showPoolSize);
	checkMatchesInUserMap(m_nonLadderUsers3v3, 0, 6, showPoolSize);
	checkMatchesInUserMap(m_nonLadderUsers4v4, 0, 8, showPoolSize);

	for (LadderMap::iterator it = m_ladders.begin(); it != m_ladders.end(); ++it)
	{
		checkMatchesInUserMap(it->second, it->first, 2, showPoolSize);
	}
}

double GeneralsMatcher::computeMatchFitness(const std::string& i1, const GeneralsUser *u1, const std::string& i2, const GeneralsUser *u2)
{
	//DBGMSG("matching "<<i1<< " vs "<<i2);
	if (u1->status != STATUS_WORKING || u2->status != STATUS_WORKING)
		return 0.0;

	// see if they pinged the same # of servers (sanity).
	if (u1->pseudoPing.size() != u2->pseudoPing.size())
		return 0.0;

	// check point percentage ranges
	int p1 = max(1,u1->points), p2 = max(1,u2->points);
	double p1percent = (double)p2/(double)p1;
	double p2percent = (double)p1/(double)p2;
	//DBGMSG("points: " << p1 << "," << p2 << " - " << p1percent << "," << p2percent);
	if (!u1->widened && ( p1percent < u1->minPoints || p1percent > u1->maxPoints ))
		return 0.0;

	if (!u2->widened && ( p2percent < u2->minPoints || p2percent > u2->maxPoints ))
		return 0.0;


	int minP = min(p1, p2);
	int maxP = max(p1, p2);
	double pointPercent = (double)minP/(double)maxP;
	//DBGMSG("\tpointPercent = "<<pointPercent);

	// check pings
	int pingDelta = calcPingDelta(u1, u2);
	if (!u1->widened && pingDelta > u1->maxPing)
		return 0.0;
	if (!u2->widened && pingDelta > u2->maxPing)
		return 0.0;
	//DBGMSG("pingDelta="<<pingDelta);

	//DBGMSG(u1->discons << "," << u1->maxDiscons << "  " << u2->discons << "," << u2->maxDiscons);
	// check discons
	if (u1->maxDiscons && (!u1->widened && u2->discons > u1->maxDiscons))
		return 0.0;
	if (u2->maxDiscons && (!u2->widened && u1->discons > u2->maxDiscons))
		return 0.0;
	//DBGMSG("Made it through discons");

	{
		MapBitSet tmp = MapSetUnion(u1->maps, u2->maps);
		if (!MapSetCount(tmp))
			return 0.0;
	}

	// they have something in common.  calculate match fitness.
	double matchFitness = ( weightAvgPoints * (1-pointPercent) +
	                        weightLowPing * (MaxPingValue - pingDelta)/MaxPingValue ) / (double)totalWeight;
	//DBGMSG("Match fitness: "<<matchFitness);

	/*
	DBGMSG(i1->first << " vs " << i2->first << " has fitness " << matchFitness << "\n"
			   "\tpointPercent: " << pointPercent << "\n"
			   "\tpingDelta: " << pingDelta << "\n"
			   "\twidened: " << u1->widened << u2->widened << "\n"
			   "\tweightAvgPoints: " << weightAvgPoints << "\n"
			   "\tweightLowPing: " << weightLowPing << "\n"
			   "\ttotalWeight: " << totalWeight
			  );
				*/

	return matchFitness;
}

void GeneralsMatcher::checkMatchesInUserMap(UserMap& userMap, int ladderID, int numPlayers, bool showPoolSize)
{
	UserMap::iterator i1, i2, i3, i4, i5, i6, i7, i8;
	GeneralsUser *u1, *u2, *u3, *u4, *u5, *u6, *u7, *u8;
	static const double fitnessThreshold = 0.3;
	time_t now = time(NULL);

	std::string s;
	if (showPoolSize)
	{
		s = "MBOT:POOLSIZE ";
		s.append(intToString(userMap.size()));
	}

	// iterate through users, timing them out as neccessary
	for (i1 = userMap.begin(); i1 != userMap.end(); ++i1)
	{
		if (showPoolSize)
		{
			peerMessagePlayer(m_peer, i1->first.c_str(), s.c_str(), NormalMessage);
		}

		u1 = i1->second;
		if (u1->timeToWiden && u1->timeToWiden < now)
		{
			u1->timeToWiden = 0;
			u1->widened = true;
			for (int m=0; m<(int)u1->maps.size(); ++m)
				u1->maps[m] = 1;
			DBGMSG("Widening search for " << i1->first);
			peerMessagePlayer(m_peer, i1->first.c_str(), "MBOT:WIDENINGSEARCH", NormalMessage);
		}
	}

	// iterate through all users, looking for a match
	for (i1 = userMap.begin(); i1 != userMap.end(); ++i1)
	{
		u1 = i1->second;
		if (u1->status != STATUS_WORKING)
			continue;

		GeneralsUser *bestUser = NULL;
		double bestMatchFitness = 0.0;
		std::string bestName = "";

		// look at everybody left
		i2 = i1;
		for (++i2; i2 != userMap.end(); ++i2)
		{
			u2 = i2->second;
			if (u2->status != STATUS_WORKING)
				continue;

			double matchFitness = computeMatchFitness(i1->first, u1, i2->first, u2);
			if (matchFitness > fitnessThreshold)
			{
				if (numPlayers == 2)
				{
					if (matchFitness > bestMatchFitness)
					{
						// possibly match 2 players
						bestMatchFitness = matchFitness;
						bestUser = u2;
						bestName = i2->first;
					}
				}
				else
				{
					i3 = i2;
					for (++i3; i3 != userMap.end(); ++i3)
					{
						u3 = i3->second;
						if (u3->status != STATUS_WORKING)
							continue;

						double matchFitness1 = computeMatchFitness(i1->first, u1, i3->first, u3);
						double matchFitness2 = computeMatchFitness(i2->first, u2, i3->first, u3);
						MapBitSet tmp = MapSetUnion(u1->maps, u2->maps);
						tmp = MapSetUnion(tmp, u3->maps);
						if (MapSetCount(tmp) && matchFitness1 > fitnessThreshold && matchFitness2 > fitnessThreshold)
						{
							i4 = i3;
							for (++i4; i4 != userMap.end(); ++i4)
							{
								u4 = i4->second;
								if (u4->status != STATUS_WORKING)
									continue;

								double matchFitness1 = computeMatchFitness(i1->first, u1, i4->first, u4);
								double matchFitness2 = computeMatchFitness(i2->first, u2, i4->first, u4);
								double matchFitness3 = computeMatchFitness(i3->first, u3, i4->first, u4);
								MapBitSet tmp = MapSetUnion(u1->maps, u2->maps);
								tmp = MapSetUnion(tmp, u3->maps);
								tmp = MapSetUnion(tmp, u4->maps);
								if (MapSetCount(tmp) && matchFitness1 > fitnessThreshold && matchFitness2 > fitnessThreshold && matchFitness3 > fitnessThreshold)
								{
									if (numPlayers == 4)
									{
										// match 4 players
										sendMatchInfo(i1->first, i2->first, i3->first, i4->first, "", "", "", "",
										              u1, u2, u3, u4, NULL, NULL, NULL, NULL, 4, ladderID);
										break;
									}
									else
									{

										i5 = i4;
										for (++i5; i5 != userMap.end(); ++i5)
										{
											u5 = i5->second;
											if (u5->status != STATUS_WORKING)
												continue;

											double matchFitness1 = computeMatchFitness(i1->first, u1, i5->first, u5);
											double matchFitness2 = computeMatchFitness(i2->first, u2, i5->first, u5);
											double matchFitness3 = computeMatchFitness(i3->first, u3, i5->first, u5);
											double matchFitness4 = computeMatchFitness(i4->first, u4, i5->first, u5);
											MapBitSet tmp = MapSetUnion(u1->maps, u2->maps);
											tmp = MapSetUnion(tmp, u3->maps);
											tmp = MapSetUnion(tmp, u4->maps);
											tmp = MapSetUnion(tmp, u5->maps);
											if (MapSetCount(tmp) && matchFitness1 > fitnessThreshold && matchFitness2 > fitnessThreshold && matchFitness3 > fitnessThreshold && matchFitness4 > fitnessThreshold)
											{
												i6 = i5;
												for (++i6; i6 != userMap.end(); ++i6)
												{
													u6 = i6->second;
													if (u6->status != STATUS_WORKING)
														continue;

													double matchFitness1 = computeMatchFitness(i1->first, u1, i6->first, u6);
													double matchFitness2 = computeMatchFitness(i2->first, u2, i6->first, u6);
													double matchFitness3 = computeMatchFitness(i3->first, u3, i6->first, u6);
													double matchFitness4 = computeMatchFitness(i4->first, u4, i6->first, u6);
													double matchFitness5 = computeMatchFitness(i5->first, u5, i6->first, u6);
													MapBitSet tmp = MapSetUnion(u1->maps, u2->maps);
													tmp = MapSetUnion(tmp, u3->maps);
													tmp = MapSetUnion(tmp, u4->maps);
													tmp = MapSetUnion(tmp, u5->maps);
													tmp = MapSetUnion(tmp, u6->maps);
													if (MapSetCount(tmp) && matchFitness1 > fitnessThreshold && matchFitness2 > fitnessThreshold && matchFitness3 > fitnessThreshold && matchFitness4 > fitnessThreshold && matchFitness5 > fitnessThreshold)
													{
														if (numPlayers == 6)
														{
															// match 6 players
															sendMatchInfo(i1->first, i2->first, i3->first, i4->first, i5->first, i6->first, "", "",
															              u1, u2, u3, u4, u5, u6, NULL, NULL, 6, ladderID);
															break;
														}
														else
														{

															i7 = i6;
															for (++i7; i7 != userMap.end(); ++i7)
															{
																u7 = i7->second;
																if (u7->status != STATUS_WORKING)
																	continue;

																double matchFitness1 = computeMatchFitness(i1->first, u1, i7->first, u7);
																double matchFitness2 = computeMatchFitness(i2->first, u2, i7->first, u7);
																double matchFitness3 = computeMatchFitness(i3->first, u3, i7->first, u7);
																double matchFitness4 = computeMatchFitness(i4->first, u4, i7->first, u7);
																double matchFitness5 = computeMatchFitness(i5->first, u5, i7->first, u7);
																double matchFitness6 = computeMatchFitness(i6->first, u6, i7->first, u7);
																MapBitSet tmp = MapSetUnion(u1->maps, u2->maps);
																tmp = MapSetUnion(tmp, u3->maps);
																tmp = MapSetUnion(tmp, u4->maps);
																tmp = MapSetUnion(tmp, u5->maps);
																tmp = MapSetUnion(tmp, u6->maps);
																tmp = MapSetUnion(tmp, u7->maps);
																if (MapSetCount(tmp) && matchFitness1 > fitnessThreshold && matchFitness2 > fitnessThreshold && matchFitness3 > fitnessThreshold && matchFitness4 > fitnessThreshold && matchFitness5 > fitnessThreshold && matchFitness6 > fitnessThreshold)
																{
																	i8 = i7;
																	for (++i8; i8 != userMap.end(); ++i8)
																	{
																		u8 = i8->second;
																		if (u8->status != STATUS_WORKING)
																			continue;

																		double matchFitness1 = computeMatchFitness(i1->first, u1, i8->first, u8);
																		double matchFitness2 = computeMatchFitness(i2->first, u2, i8->first, u8);
																		double matchFitness3 = computeMatchFitness(i3->first, u3, i8->first, u8);
																		double matchFitness4 = computeMatchFitness(i4->first, u4, i8->first, u8);
																		double matchFitness5 = computeMatchFitness(i5->first, u5, i8->first, u8);
																		double matchFitness6 = computeMatchFitness(i6->first, u6, i8->first, u8);
																		double matchFitness7 = computeMatchFitness(i7->first, u7, i8->first, u8);
																		MapBitSet tmp = MapSetUnion(u1->maps, u2->maps);
																		tmp = MapSetUnion(tmp, u3->maps);
																		tmp = MapSetUnion(tmp, u4->maps);
																		tmp = MapSetUnion(tmp, u5->maps);
																		tmp = MapSetUnion(tmp, u6->maps);
																		tmp = MapSetUnion(tmp, u7->maps);
																		tmp = MapSetUnion(tmp, u8->maps);
																		if (MapSetCount(tmp) && matchFitness1 > fitnessThreshold && matchFitness2 > fitnessThreshold && matchFitness3 > fitnessThreshold && matchFitness4 > fitnessThreshold && matchFitness5 > fitnessThreshold && matchFitness6 > fitnessThreshold && matchFitness7 > fitnessThreshold)
																		{
																			// match 8 players
																			sendMatchInfo(i1->first, i2->first, i3->first, i4->first, i5->first, i6->first, i7->first, i8->first,
																			              u1, u2, u3, u4, u5, u6, u7, u8, 8, ladderID);
																			break;
																		}
																	}
																}
															}

														}
													}
												}
											}
										}

									}
								}
							}
						}
					}
				}
			}
		} // for i2

		if (bestUser && numPlayers == 2)
		{
			// we had a match.  send the info.
			DBGMSG("Matching " << i1->first << " with " << bestName << ":\n"
			       "\tmatch fitness: " << bestMatchFitness << "\n"
			       "\tpoint percentage: " << (1-bestUser->points/(double)u1->points)*100 << "\n"
			       "\tpoints: " << u1->points << ", " << u2->points << "\n"
			       "\tping in ms: " << sqrt(1000000 * calcPingDelta(u1, bestUser) / (255*255*2)) << "\n"
			       "\tprevious attempts: " << u1->widened << ", " << bestUser->widened);
			sendMatchInfo(i1->first, bestName, "", "", "", "", "", "",
			              u1, bestUser, NULL, NULL, NULL, NULL, NULL, NULL, 2, ladderID);
			break;
		}
	} // for i1

	dumpUsers();
}

// return false for possible hack attempt
bool GeneralsMatcher::handleUserWiden(const char *nick)
{
	GeneralsUser *userInfo = findUserInAnyLadder(nick);
	if (!userInfo)
	{
		userInfo = findNonLadderUser(nick);
	}

	if (!userInfo)
	{
		DBGMSG("Got Widen from nick not needing one!");
		peerMessagePlayer(m_peer, nick, "MBOT:CANTSENDWIDENNOW", NormalMessage);
		return false;
	}
	DBGMSG("Widening search for " << nick);
	peerMessagePlayer(m_peer, nick, "MBOT:WIDENINGSEARCH", NormalMessage);

	userInfo->widened = true;
	return true;
}

bool GeneralsMatcher::handleUserInfo(const char *nick, const std::string& msg)
{
	DBGMSG("Got user info [" << msg << "] from " << nick);
	GeneralsUser *userInfo = removeNonMatchingUser(nick);
	if (!userInfo)
	{
		DBGMSG("Got UserInfo from nick not needing one!");
		peerMessagePlayer(m_peer, nick, "MBOT:CANTSENDCINFONOW", NormalMessage);
		return false;
	}
	DBGMSG("Looking at " << nick << " with user info [" << msg << "]");

	int ladderID = 0;
	unsigned int ladderPassCRC = 0;

	int offset = 0;
	while (1)
	{
		int firstMarker = msg.find_first_of('\\', offset);
		if (firstMarker < 0)
			break;
		int secondMarker = msg.find_first_of('\\', firstMarker + 1);
		if (secondMarker < 0)
			break;
		int thirdMarker = msg.find_first_of('\\', secondMarker + 1);
		if (thirdMarker < 0)
			break;
		std::string k = msg.substr(firstMarker + 1, secondMarker - firstMarker - 1);
		std::string v = msg.substr(secondMarker + 1, thirdMarker - secondMarker - 1);
		offset = thirdMarker - 1;

		if (k == "Widen")
		{
			int val = atoi(v.c_str());
			if (val > 0)
				userInfo->timeToWiden = time(NULL) + val;
			else
				userInfo->timeToWiden = 0;
		}
		else if (k == "LadID")
		{
			ladderID = atoi(v.c_str());
		}
		else if (k == "LadPass")
		{
			ladderPassCRC = atoi(v.c_str());
		}
		else if (k == "PointsMin")
		{
			userInfo->minPoints = atoi(v.c_str());
		}
		else if (k == "PointsMax")
		{
			userInfo->maxPoints = atoi(v.c_str());
		}
		else if (k == "PingMax")
		{
			userInfo->maxPing = atoi(v.c_str());
		}
		else if (k == "DisconMax")
		{
			userInfo->maxDiscons = atoi(v.c_str());
		}
		else if (k == "Maps")
		{
#ifdef DEBUG
			//int curMaps = userInfo->maps.size();
#endif

			//DBGMSG("map cur size is " << curMaps);
			userInfo->maps.clear();
			if (!v.length())
			{
				INFMSG("Bad maps from " << nick << ": [" << v << "]");
				peerMessagePlayer(m_peer, nick, "MBOT:BADMAPS", NormalMessage);
				return false;
			}
			const char *buf = v.c_str();
			int pos = 0;
			while (*buf)
			{
				bool hasMap = (*buf != '0');
				//DBGMSG("Setting map " << pos << " to " << hasMap);
				userInfo->maps.push_back( hasMap );
				++pos;
				++buf;
			}
		}
		else if (k == "NumPlayers")
		{
			userInfo->numPlayers = atoi(v.c_str());
			if (userInfo->numPlayers != 2 && userInfo->numPlayers != 4 &&
			        userInfo->numPlayers != 6 && userInfo->numPlayers != 8)
			{
				INFMSG("Bad numPlayers from " << nick << ": [" << userInfo->numPlayers << "]");
				peerMessagePlayer(m_peer, nick, "MBOT:BADCINFO", NormalMessage);
				return false;
			}
		}
		else if (k == "IP")
		{
			userInfo->IP = atoi(v.c_str());
		}
		else if (k == "NAT")
		{
			userInfo->NAT = atoi(v.c_str());
		}
		else if (k == "Side")
		{
			userInfo->country = atoi(v.c_str());
		}
		else if (k == "Color")
		{
			userInfo->color = atoi(v.c_str());
		}
		else if (k == "Pings")
		{
			if (!v.length() || (v.length() % 2))
			{
				INFMSG("Bad pings from " << nick << ": [" << v << "]");
				peerMessagePlayer(m_peer, nick, "MBOT:BADPINGS", NormalMessage);
				return false;
			}
			int ping = 0;
			const char *buf = v.c_str();
			char buf2[3];
			buf2[2] = '\0';
			// We've already assured that pingStr has non-zero even length.
			while (*buf)
			{
				buf2[0] = *buf++;
				buf2[1] = *buf++;
				ping = (int)strtol(buf2, NULL, 16);
				userInfo->pseudoPing.push_back(ping);
			}
		}
		else if (k == "Points")
		{
			userInfo->points = max(1, atoi(v.c_str()));
		}
		else if (k == "Discons")
		{
			userInfo->discons = atoi(v.c_str());
		}
		else
		{
			INFMSG("Unknown key/value pair in user info [" << k << "]/[" << v << "]");
			peerMessagePlayer(m_peer, nick, "MBOT:BADCINFO", NormalMessage);
			return false;
		}
	}

	std::string s = "MBOT:WORKING ";

	if (ladderID)
	{
		addUserInLadder(nick, ladderID, userInfo);
		s.append(intToString(m_ladders[ladderID].size()));
	}
	else
	{
		addNonLadderUser(nick, userInfo);
		switch (userInfo->numPlayers)
		{
		case 2:
			s.append(intToString(m_nonLadderUsers1v1.size()));
			break;
		case 4:
			s.append(intToString(m_nonLadderUsers2v2.size()));
			break;
		case 6:
			s.append(intToString(m_nonLadderUsers3v3.size()));
			break;
		case 8:
			s.append(intToString(m_nonLadderUsers4v4.size()));
			break;
		}
	}

	userInfo->status = STATUS_WORKING;
	userInfo->matchStart = time(NULL);
	peerMessagePlayer(m_peer, nick, s.c_str(), NormalMessage);

	DBGMSG("Player " << nick << " is matching now, ack was [" << s << "]");
	return true;
}

GeneralsUser* GeneralsMatcher::findUser(const std::string& who)
{
	GeneralsUser *user;
	user = findNonLadderUser(who);
	if (user)
		return user;
	user = findNonMatchingUser(who);
	if (user)
		return user;
	user = findUserInAnyLadder(who);
	if (user)
		return user;

	return NULL;
}

GeneralsUser* GeneralsMatcher::findUserInAnyLadder(const std::string& who)
{
	for (LadderMap::iterator lIt = m_ladders.begin(); lIt != m_ladders.end(); ++lIt)
	{
		UserMap::iterator uIt = lIt->second.find(who);
		if (uIt != lIt->second.end())
			return uIt->second;
	}
	return NULL;
}

GeneralsUser* GeneralsMatcher::findUserInLadder(const std::string& who, int ladderID)
{
	LadderMap::iterator lIt = m_ladders.find(ladderID);
	if (lIt == m_ladders.end())
		return NULL;

	UserMap::iterator uIt = lIt->second.find(who);
	if (uIt == lIt->second.end())
		return NULL;

	return uIt->second;
}

GeneralsUser* GeneralsMatcher::findNonLadderUser(const std::string& who)
{
	UserMap::iterator it = m_nonLadderUsers1v1.find(who);
	if (it != m_nonLadderUsers1v1.end())
		return it->second;

	it = m_nonLadderUsers2v2.find(who);
	if (it != m_nonLadderUsers2v2.end())
		return it->second;

	it = m_nonLadderUsers3v3.find(who);
	if (it != m_nonLadderUsers3v3.end())
		return it->second;

	it = m_nonLadderUsers4v4.find(who);
	if (it != m_nonLadderUsers4v4.end())
		return it->second;

	return NULL;
}

GeneralsUser* GeneralsMatcher::findNonMatchingUser(const std::string& who)
{
	UserMap::iterator it = m_nonMatchingUsers.find(who);
	if (it == m_nonMatchingUsers.end())
		return NULL;

	return it->second;
}

void GeneralsMatcher::addUser(const std::string& who)
{
	if (findUser(who))
	{
		ERRMSG("Re-adding " << who);
		return;
	}

	addNonMatchingUser(who, new GeneralsUser);
}

void GeneralsMatcher::addUserInLadder(const std::string& who, int ladderID, GeneralsUser *user)
{
	m_ladders[ladderID][who] = user;
}

void GeneralsMatcher::addNonLadderUser(const std::string& who, GeneralsUser *user)
{
	switch (user->numPlayers)
	{
	case 2:
		m_nonLadderUsers1v1[who] = user;
		break;
	case 4:
		m_nonLadderUsers2v2[who] = user;
		break;
	case 6:
		m_nonLadderUsers3v3[who] = user;
		break;
	case 8:
		m_nonLadderUsers4v4[who] = user;
		break;
	}
}

void GeneralsMatcher::addNonMatchingUser(const std::string& who, GeneralsUser *user)
{
	m_nonMatchingUsers[who] = user;
}


bool GeneralsMatcher::removeUser(const std::string& who)
{
	GeneralsUser *user;
	user = removeUserInAnyLadder(who);
	if (user)
	{
		delete user;
		return true;
	}
	user = removeNonLadderUser(who);
	if (user)
	{
		delete user;
		return true;
	}
	user = removeNonMatchingUser(who);
	if (user)
	{
		delete user;
		return true;
	}

	return false;
}

GeneralsUser* GeneralsMatcher::removeUserInLadder(const std::string& who, int ladderID)
{
	LadderMap::iterator lIt = m_ladders.find(ladderID);
	if (lIt == m_ladders.end())
		return NULL;

	UserMap::iterator uIt = lIt->second.find(who);
	if (uIt == lIt->second.end())
		return NULL;

	GeneralsUser *user = uIt->second;
	lIt->second.erase(uIt);
	return user;
}

GeneralsUser* GeneralsMatcher::removeUserInAnyLadder(const std::string& who)
{
	for (LadderMap::iterator lIt = m_ladders.begin(); lIt != m_ladders.end(); ++lIt)
	{
		UserMap::iterator uIt = lIt->second.find(who);
		if (uIt != lIt->second.end())
		{
			GeneralsUser *user = uIt->second;
			lIt->second.erase(uIt);
			return user;
		}
	}
	return NULL;
}

GeneralsUser* GeneralsMatcher::removeNonLadderUser(const std::string& who)
{
	UserMap::iterator it = m_nonLadderUsers1v1.find(who);
	if (it != m_nonLadderUsers1v1.end())
	{
		GeneralsUser *user = it->second;
		m_nonLadderUsers1v1.erase(it);
		return user;
	}

	it = m_nonLadderUsers2v2.find(who);
	if (it != m_nonLadderUsers2v2.end())
	{
		GeneralsUser *user = it->second;
		m_nonLadderUsers2v2.erase(it);
		return user;
	}

	it = m_nonLadderUsers3v3.find(who);
	if (it != m_nonLadderUsers3v3.end())
	{
		GeneralsUser *user = it->second;
		m_nonLadderUsers3v3.erase(it);
		return user;
	}

	it = m_nonLadderUsers4v4.find(who);
	if (it != m_nonLadderUsers4v4.end())
	{
		GeneralsUser *user = it->second;
		m_nonLadderUsers4v4.erase(it);
		return user;
	}

	return NULL;
}

GeneralsUser* GeneralsMatcher::removeNonMatchingUser(const std::string& who)
{
	UserMap::iterator it = m_nonMatchingUsers.find(who);
	if (it == m_nonMatchingUsers.end())
		return NULL;

	GeneralsUser *user = it->second;
	m_nonMatchingUsers.erase(it);
	return user;
}


void GeneralsMatcher::handleDisconnect( const char *reason )
{
	ERRMSG("Disconnected");
	done = true;
	exit(0);
}

void GeneralsMatcher::handleRoomMessage( const char *nick, const char *message, MessageType messageType )
{
	if (messageType == ActionMessage)
	{
		PARANOIDMSG(nick << " " << message);
	}
	else
	{
		PARANOIDMSG("[" << nick << "] " << message);
	}
}

void GeneralsMatcher::handlePlayerMessage( const char *nick, const char *message, MessageType messageType )
{
	if (messageType == ActionMessage)
	{
		DBGMSG(nick << " " << message);
	}
	else
	{
		DBGMSG("[" << nick << "] " << message);
	}

	if (messageType != NormalMessage)
		return;

	std::string line = message;
	line.append("\\");

	int offset = 0;
	int firstMarker = line.find_first_of('\\', offset);
	int secondMarker = line.find_first_of('\\', firstMarker + 1);
	if (firstMarker >= 0 && secondMarker >= 0)
	{
		std::string marker = line.substr(firstMarker + 1, secondMarker - firstMarker - 1);
		if (marker == "CINFO")
		{
			handleUserInfo(nick, line.substr(secondMarker));//, std::string::npos));
		}
		else if (marker == "WIDEN")
		{
			handleUserWiden(nick);
		}
		else
		{
			INFMSG("Unknown marker [" << marker << "] in line [" << line << "] from " << nick);
		}
	}
	else
	{
		INFMSG("Failed to parse line [" << line << "] from " << nick);
	}
}

void GeneralsMatcher::handlePlayerJoined( const char *nick )
{
	DBGMSG("Player " << nick << " joined");
	addUser(nick);
}

void GeneralsMatcher::handlePlayerLeft( const char *nick )
{
	DBGMSG("Player " << nick << " left");
	if (m_nick != nick)
		removeUser(nick);
}

void GeneralsMatcher::handlePlayerChangedNick( const char *oldNick, const char *newNick )
{
	DBGMSG("Player " << oldNick << " changed nick to " << newNick << " - resetting to non-matching state");
	removeUser(oldNick);
	addUser(newNick);
}

void GeneralsMatcher::handlePlayerEnum( bool success, int gameSpyIndex, const char *nick, int flags )
{
	if (!nick)
		nick = "";
	DBGMSG("PlayerEnum: success=" << success << " index=" << gameSpyIndex << ", nick=" << nick << ", flags=" << flags);

	if (success && gameSpyIndex >= 0 && m_nick != nick)
	{
		addUser(nick);
	}
}


// =====================================================================
// TEST Client Matcher class
// =====================================================================

GeneralsClientMatcher::GeneralsClientMatcher()
{
	// Read some values from the config file
	int quietTMP = 0;
	Global.config.getInt("NOECHO", quietTMP);
	if (quietTMP)
		quiet = true;
	else
		quiet = false;
}

void GeneralsClientMatcher::init(void)
{
	m_baseNick.setFormatted("qmBot%d", time(NULL));
	m_profileID = 0;
}

void GeneralsClientMatcher::checkMatches(void)
{}

void GeneralsClientMatcher::handleDisconnect( const char *reason )
{}
void GeneralsClientMatcher::handleRoomMessage( const char *nick, const char *message, MessageType messageType )
{}
void GeneralsClientMatcher::handlePlayerMessage( const char *nick, const char *message, MessageType messageType )
{}
void GeneralsClientMatcher::handlePlayerJoined( const char *nick )
{}
void GeneralsClientMatcher::handlePlayerLeft( const char *nick )
{}
void GeneralsClientMatcher::handlePlayerChangedNick( const char *oldNick, const char *newNick )
{}
void GeneralsClientMatcher::handlePlayerEnum( bool success, int gameSpyIndex, const char *nick, int flags )
{}


// =====================================================================
// End of File
// =====================================================================


