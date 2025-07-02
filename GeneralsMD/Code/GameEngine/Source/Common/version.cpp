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

// FILE: version.cpp //////////////////////////////////////////////////////
// Generals version number class
// Author: Matthew D. Campbell, November 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/GameText.h"
#include "Common/version.h"

#include "gitinfo.h"

Version *TheVersion = NULL;	///< The Version singleton

Version::Version()
{
	m_major = 1;
	m_minor = 0;
	m_buildNum = 0;
	m_localBuildNum = 0;
	m_buildUser = AsciiString::TheEmptyString;
	m_buildLocation = AsciiString::TheEmptyString;
	m_asciiGitRevision = buildAsciiGitRevision();
	m_asciiGitVersion = buildAsciiGitVersion();
	m_asciiGitCommitTime = buildAsciiGitCommitTime();
	m_unicodeGitRevision = buildUnicodeGitRevision();
	m_unicodeGitVersion = buildUnicodeGitVersion();
	m_unicodeGitCommitTime = buildUnicodeGitCommitTime();
#if defined(RTS_DEBUG)
	m_showFullVersion = TRUE;
#else
	m_showFullVersion = FALSE;
#endif
}

void Version::setVersion(Int major, Int minor, Int buildNum,
												 Int localBuildNum, AsciiString user, AsciiString location,
												 AsciiString buildTime, AsciiString buildDate)
{
	m_major = major;
	m_minor = minor;
	m_buildNum = buildNum;
	m_localBuildNum = localBuildNum;
	m_buildUser = user;
	m_buildLocation = location;
	m_buildTime = buildTime;
	m_buildDate = buildDate;
}

UnsignedInt Version::getVersionNumber() const
{
	return m_major << 16 | m_minor;
}

AsciiString Version::getAsciiVersion() const
{
	AsciiString version;

	if (m_showFullVersion)
	{
		if (!m_localBuildNum)
		{
			version.format("%d.%d.%d", m_major, m_minor, m_buildNum);
		}
		else
		{
			AsciiString user = getAsciiBuildUserOrGitCommitAuthorName();
			// User name requires at least 2 characters
			if (user.getLength() < 2)
				user.concat("xx");

			version.format("%d.%d.%d.%d%c%c", m_major, m_minor, m_buildNum, m_localBuildNum,
				user.getCharAt(0), user.getCharAt(1));
		}
	}
	else
	{
		version.format("%d.%d", m_major, m_minor);
	}

#ifdef RTS_DEBUG
	version.concat(" Debug");
#endif

	return version;
}

UnicodeString Version::getUnicodeVersion() const
{
	UnicodeString version;

	if (m_showFullVersion)
	{
		if (!m_localBuildNum)
		{
			version.format(TheGameText->fetch("Version:Format3").str(), m_major, m_minor, m_buildNum);
		}
		else
		{
			UnicodeString user = getUnicodeBuildUserOrGitCommitAuthorName();
			// User name requires at least 2 characters
			if (user.getLength() < 2)
				user.concat(L"xx");

			version.format(TheGameText->fetch("Version:Format4").str(), m_major, m_minor, m_buildNum, m_localBuildNum,
				user.getCharAt(0), user.getCharAt(1));
		}
	}
	else
	{
		version.format(TheGameText->fetch("Version:Format2").str(), m_major, m_minor);
	}

#ifdef RTS_DEBUG
	version.concat(L" Debug");
#endif

	return version;
}

AsciiString Version::getAsciiBuildTime() const
{
	AsciiString timeStr;
	timeStr.format("%s %s", m_buildDate.str(), m_buildTime.str());

	return timeStr;
}

UnicodeString Version::getUnicodeBuildTime() const
{
	UnicodeString build;
	UnicodeString dateStr;
	UnicodeString timeStr;

	dateStr.translate(m_buildDate);
	timeStr.translate(m_buildTime);
	build.format(TheGameText->fetch("Version:BuildTime").str(), dateStr.str(), timeStr.str());

	return build;
}

AsciiString Version::getAsciiBuildLocation() const
{
	return m_buildLocation;
}

UnicodeString Version::getUnicodeBuildLocation() const
{
	UnicodeString build;

	if (!m_buildLocation.isEmpty())
	{
		UnicodeString machine;
		machine.translate(m_buildLocation);
		build.format(TheGameText->fetch("Version:BuildMachine").str(), machine.str());
	}

	return build;
}

AsciiString Version::getAsciiBuildUser() const
{
	return m_buildUser;
}

UnicodeString Version::getUnicodeBuildUser() const
{
	UnicodeString build;

	if (!m_buildUser.isEmpty())
	{
		UnicodeString user;
		user.translate(m_buildUser);
		build.format(TheGameText->fetch("Version:BuildUser").str(), user.str());
	}

	return build;
}

Int Version::getGitRevision()
{
	return GitRevision;
}

time_t Version::getGitCommitTime()
{
	return GitCommitTimeStamp;
}

const char* Version::getGitCommitAuthorName()
{
	return GitCommitAuthorName;
}

AsciiString Version::getAsciiGitRevision() const
{
	return m_asciiGitRevision;
}

UnicodeString Version::getUnicodeGitRevision() const
{
	return m_unicodeGitRevision;
}

AsciiString Version::getAsciiGitVersion() const
{
	return m_asciiGitVersion;
}

UnicodeString Version::getUnicodeGitVersion() const
{
	return m_unicodeGitVersion;
}

AsciiString Version::getAsciiGitCommitTime() const
{
	return m_asciiGitCommitTime;
}

UnicodeString Version::getUnicodeGitCommitTime() const
{
	return m_unicodeGitCommitTime;
}

AsciiString Version::getAsciiGameAndGitVersion() const
{
	AsciiString str;
	if (m_showFullVersion)
	{
		str.format("%s R %s %s",
			getAsciiVersion().str(),
			getAsciiGitRevision().str(),
			getAsciiGitVersion().str());
	}
	else
	{
		str.format("%s R %s",
			getAsciiVersion().str(),
			getAsciiGitRevision().str());
	}
	return str;
}

UnicodeString Version::getUnicodeGameAndGitVersion() const
{
	UnicodeString str;
	if (m_showFullVersion)
	{
		str.format(L"%s R %s %s",
			getUnicodeVersion().str(),
			getUnicodeGitRevision().str(),
			getUnicodeGitVersion().str());
	}
	else
	{
		str.format(L"%s R %s",
			getUnicodeVersion().str(),
			getUnicodeGitRevision().str());
	}
	return str;
}

AsciiString Version::getAsciiBuildUserOrGitCommitAuthorName() const
{
	AsciiString asciiUser = getAsciiBuildUser();

	if (asciiUser.isEmpty())
	{
		asciiUser = getGitCommitAuthorName();
	}

	return asciiUser;
}

UnicodeString Version::getUnicodeBuildUserOrGitCommitAuthorName() const
{
	UnicodeString str;
	AsciiString asciiUser = getAsciiBuildUserOrGitCommitAuthorName();

	if (!asciiUser.isEmpty())
	{
		UnicodeString unicodeUser;
		unicodeUser.translate(asciiUser);
		str.format(TheGameText->fetch("Version:BuildUser").str(), unicodeUser.str());
	}

	return str;
}

AsciiString Version::buildAsciiGitRevision()
{
	AsciiString str;
	str.format("%s%d",
		GitUncommittedChanges ? "~" : "",
		GitRevision);
	return str;
}

UnicodeString Version::buildUnicodeGitRevision()
{
	UnicodeString str;
	str.format(L"%s%d",
		GitUncommittedChanges ? L"~" : L"",
		GitRevision);
	return str;
}

AsciiString Version::buildAsciiGitVersion()
{
	AsciiString str;
	str.format("%s%s",
		GitUncommittedChanges ? "~" : "",
		GitTag[0] ? GitTag : GitShortSHA1);
	return str;
}

UnicodeString Version::buildUnicodeGitVersion()
{
	UnicodeString str;
	str.translate(buildAsciiGitVersion());
	return str;
}

AsciiString Version::buildAsciiGitCommitTime()
{
	const Int len = 19;
	AsciiString str;
	Char* buf = str.getBufferForRead(len);
	tm* time = gmtime(&GitCommitTimeStamp);
	strftime(buf, len+1, "%Y-%m-%d %H:%M:%S", time);
	return str;
}

UnicodeString Version::buildUnicodeGitCommitTime()
{
	const Int len = 19;
	UnicodeString str;
	WideChar* buf = str.getBufferForRead(len);
	tm* time = gmtime(&GitCommitTimeStamp);
	wcsftime(buf, len+1, L"%Y-%m-%d %H:%M:%S", time);
	return str;
}
