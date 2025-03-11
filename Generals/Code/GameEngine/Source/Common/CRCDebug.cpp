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


#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/CRCDebug.h"
#include "Common/Debug.h"
#include "Common/PerfTimer.h"
#include "GameClient/InGameUI.h"
#include "GameNetwork/IPEnumeration.h"
#include <cstdarg>

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

#ifdef DEBUG_CRC

static const Int MaxStrings = 64000;

static char DebugStrings[MaxStrings][1024];
static Int nextDebugString = 0;
static Int numDebugStrings = 0;
//static char DumpStrings[MaxStrings][1024];
//static Int nextDumpString = 0;
//static Int numDumpStrings = 0;

#define IS_FRAME_OK_TO_LOG TheGameLogic->isInGame() && !TheGameLogic->isInShellGame() && !TheDebugIgnoreSyncErrors && \
	TheCRCFirstFrameToLog >= 0 && TheCRCFirstFrameToLog <= (int)TheGameLogic->getFrame() \
	&& TheGameLogic->getFrame() <= TheCRCLastFrameToLog

CRCVerification::CRCVerification()
{
#ifdef DEBUG_LOGGING
/**/
	if (g_verifyClientCRC && (IS_FRAME_OK_TO_LOG))
	{
		m_startCRC = TheGameLogic->getCRC(CRC_RECALC, (g_clientDeepCRC)?"clientPre.crc":"");
	}
	else
	{
		m_startCRC = 0;
	}
/**/
#endif
}

CRCVerification::~CRCVerification()
{
#ifdef DEBUG_LOGGING
/**/
	UnsignedInt endCRC = 0;
	if (g_verifyClientCRC && (IS_FRAME_OK_TO_LOG))
	{
		endCRC = TheGameLogic->getCRC(CRC_RECALC, (g_clientDeepCRC)?"clientPost.crc":"");
	}
	DEBUG_ASSERTCRASH(!TheGameLogic->isInGame() || m_startCRC == endCRC, ("GameLogic changed outside of GameLogic::update() on frame %d!", TheGameLogic->getFrame()));
	if (TheGameLogic->isInMultiplayerGame() && m_startCRC != endCRC)
	{
		if (TheInGameUI)
		{
			TheInGameUI->message(UnicodeString(L"GameLogic changed outside of GameLogic::update() - call Matt (x36804)!"));
		}
		CRCDEBUG_LOG(("GameLogic changed outside of GameLogic::update()!!!\n"));
	}
/**/
#endif
}

static Bool dumped = FALSE;
void outputCRCDebugLines( void )
{
	if (dumped)
		return;
	dumped = TRUE;
	IPEnumeration ips;
	AsciiString fname;
	fname.format("crcDebug%s.txt", ips.getMachineName().str());
	FILE* fp;
	fopen_s(&fp, fname.str(), "wt");
	int start = 0;
	int end = nextDebugString;
	if (numDebugStrings >= MaxStrings)
		start = nextDebugString - MaxStrings;

	for (Int i=start; i<end; ++i)
	{
		const char *line = DebugStrings[ (i + MaxStrings) % MaxStrings ];
		DEBUG_LOG(("%s\n", line));
		if (fp) fprintf(fp, "%s\n", line);
	}

	if (fp) fclose(fp);
}

void outputCRCDumpLines( void )
{
	/*
	int start = 0;
	int end = nextDumpString;
	if (numDumpStrings >= MaxStrings)
		start = nextDumpString - MaxStrings;

	for (Int i=start; i<end; ++i)
	{
		const char *line = DumpStrings[ (i + MaxStrings) % MaxStrings ];
		DEBUG_LOG(("%s", line));
	}
	*/
}

static AsciiString getFname(AsciiString path)
{
	return path.reverseFind('\\') + 1;
}

Int lastCRCDebugFrame = 0;
Int lastCRCDebugIndex = 0;
extern Bool inCRCGen;
void addCRCDebugLine(const char *fmt, ...)
{
	if (dumped)// || inCRCGen /*|| !TheGameLogic->isInGameLogicUpdate()*/)
		return;

	if (IS_FRAME_OK_TO_LOG)
	{

		if (lastCRCDebugFrame != TheGameLogic->getFrame())
		{
			lastCRCDebugFrame = TheGameLogic->getFrame();
			lastCRCDebugIndex = 0;
		}

		sprintf_s(DebugStrings[nextDebugString], "%d:%d ",  TheGameLogic->getFrame(), lastCRCDebugIndex++);
		//DebugStrings[nextDebugString][0] = 0;
		Int len = (int)strlen(DebugStrings[nextDebugString]);

		va_list va;
		va_start( va, fmt );
		_vsnprintf(DebugStrings[nextDebugString]+len, 1024-len, fmt, va );
		DebugStrings[nextDebugString][1023] = 0;
		va_end( va );

		char *tmp = DebugStrings[nextDebugString];
		while (tmp && *tmp)
		{
			if (*tmp == '\r' || *tmp == '\n')
			{
				*tmp = ' ';
			}
			++tmp;
		}

		//DEBUG_LOG(("%s\n", DebugStrings[nextDebugString]));

		++nextDebugString;
		++numDebugStrings;
		if (nextDebugString == MaxStrings)
			nextDebugString = 0;

	}
}

void addCRCGenLine(const char *fmt, ...)
{
	if (dumped || !(IS_FRAME_OK_TO_LOG))
		return;

	static char buf[1024];
	va_list va;
	va_start( va, fmt );
	_vsnprintf(buf, 1024, fmt, va );
	va_end( va );
	buf[1023] = 0;
	addCRCDebugLine("%s", buf);

	//DEBUG_LOG(("%s", buf));
}

void addCRCDumpLine(const char *fmt, ...)
{
	/*
	va_list va;
	va_start( va, fmt );
	_vsnprintf(DumpStrings[nextDumpString], 1024, fmt, va );
	DumpStrings[nextDumpString][1023] = 0;
	va_end( va );

	++nextDumpString;
	++numDumpStrings;
	if (nextDumpString == MaxStrings)
		nextDumpString = 0;
		*/
}

void dumpVector3(const Vector3 *v, AsciiString name, AsciiString fname, Int line)
{
	if (dumped)
		return;

	if (!(IS_FRAME_OK_TO_LOG)) return;
	fname.toLower();
	fname = getFname(fname);
	addCRCDebugLine("dumpVector3() %s:%d %s %8.8X %8.8X %8.8X\n",
		fname.str(), line, name.str(),
		AS_INT(v->X), AS_INT(v->Y), AS_INT(v->Z));
}

void dumpCoord3D(const Coord3D *c, AsciiString name, AsciiString fname, Int line)
{
	if (dumped)
		return;

	if (!(IS_FRAME_OK_TO_LOG)) return;
	fname.toLower();
	fname = getFname(fname);
	addCRCDebugLine("dumpCoord3D() %s:%d %s %8.8X %8.8X %8.8X\n",
		fname.str(), line, name.str(),
		AS_INT(c->x), AS_INT(c->y), AS_INT(c->z));
}

void dumpMatrix3D(const Matrix3D *m, AsciiString name, AsciiString fname, Int line)
{
	if (dumped)
		return;

	if (!(IS_FRAME_OK_TO_LOG)) return;
	fname.toLower();
	fname = getFname(fname);
	const Real *matrix = (const Real *)m;
	addCRCDebugLine("dumpMatrix3D() %s:%d %s\n",
		fname.str(), line, name.str());
	for (Int i=0; i<3; ++i)
		addCRCDebugLine("      0x%08X 0x%08X 0x%08X 0x%08X\n",
			AS_INT(matrix[(i<<2)+0]), AS_INT(matrix[(i<<2)+1]), AS_INT(matrix[(i<<2)+2]), AS_INT(matrix[(i<<2)+3]));
}

void dumpReal(Real r, AsciiString name, AsciiString fname, Int line)
{
	if (dumped)
		return;

	if (!(IS_FRAME_OK_TO_LOG)) return;
	fname.toLower();
	fname = getFname(fname);
	addCRCDebugLine("dumpReal() %s:%d %s %8.8X (%f)\n",
		fname.str(), line, name.str(), AS_INT(r), r);
}

#endif // DEBUG_CRC
