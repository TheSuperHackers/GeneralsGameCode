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
	m_asciiGitCommitCount = buildAsciiGitCommitCount();
	m_asciiGitTagOrHash = buildAsciiGitTagOrHash();
	m_asciiGitCommitTime = buildAsciiGitCommitTime();
	m_unicodeGitCommitCount = buildUnicodeGitCommitCount();
	m_unicodeGitTagOrHash = buildUnicodeGitTagOrHash();
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

Int Version::getGitCommitCount()
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

AsciiString Version::getAsciiGitCommitCount() const
{
	return m_asciiGitCommitCount;
}

UnicodeString Version::getUnicodeGitCommitCount() const
{
	return m_unicodeGitCommitCount;
}

AsciiString Version::getAsciiGitTagOrHash() const
{
	return m_asciiGitTagOrHash;
}

UnicodeString Version::getUnicodeGitTagOrHash() const
{
	return m_unicodeGitTagOrHash;
}

AsciiString Version::getAsciiGitCommitTime() const
{
	return m_asciiGitCommitTime;
}

UnicodeString Version::getUnicodeGitCommitTime() const
{
	return m_unicodeGitCommitTime;
}

AsciiString Version::getAsciiGitVersion() const
{
	AsciiString str;
	if (m_showFullVersion)
	{
		str.format("%s %s",
			getAsciiGitCommitCount().str(),
			getAsciiGitTagOrHash().str());
	}
	else
	{
		str.format("%s",
			getAsciiGitCommitCount().str());
	}
	return str;
}

UnicodeString Version::getUnicodeGitVersion() const
{
	UnicodeString str;
	if (m_showFullVersion)
	{
		str.format(L"%s %s",
			getUnicodeGitCommitCount().str(),
			getUnicodeGitTagOrHash().str());
	}
	else
	{
		str.format(L"%s",
			getUnicodeGitCommitCount().str());
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

UnicodeString Version::getUnicodeProductTitle() const
{
	// @todo Make configurable
	return UnicodeString(L"Community Patch");
}

UnicodeString Version::getUnicodeProductVersion() const
{
	return getUnicodeGitVersion();
}

UnicodeString Version::getUnicodeProductAuthor() const
{
	return getUnicodeBuildUserOrGitCommitAuthorName();
}

UnicodeString Version::getUnicodeProductString() const
{
	UnicodeString str;
	UnicodeString productTitle = TheGameText->FETCH_OR_SUBSTITUTE("Version:ProductTitle", getUnicodeProductTitle().str());

	if (!productTitle.isEmpty())
	{
		UnicodeString productVersion = TheGameText->FETCH_OR_SUBSTITUTE("Version:ProductVersion", getUnicodeProductVersion().str());
		UnicodeString productAuthor = TheGameText->FETCH_OR_SUBSTITUTE("Version:ProductAuthor", getUnicodeProductAuthor().str());

		str.concat(productTitle);

		if (!productVersion.isEmpty())
		{
			str.concat(L" ");
			str.concat(productVersion);
		}

		if (!productAuthor.isEmpty())
		{
			str.concat(L" ");
			str.concat(productAuthor);
		}
	}

	return str;
}

UnicodeString Version::getUnicodeProductVersionHashString() const
{
	UnicodeString str;
	UnicodeString productString = getUnicodeProductString();
	UnicodeString gameVersion = getUnicodeVersion();
	UnicodeString gameHash;
	gameHash.format(L"exe:%08X ini:%08X", TheGlobalData->m_exeCRC, TheGlobalData->m_iniCRC);

	if (!productString.isEmpty())
	{
		str.concat(productString);
		str.concat(L" | ");
	}
	str.concat(gameHash);
	str.concat(L" ");
	str.concat(gameVersion);

	return str;
}

AsciiString Version::buildAsciiGitCommitCount()
{
	AsciiString str;
	str.format("%s%d",
		GitUncommittedChanges ? "~" : "",
		GitRevision);
	return str;
}

UnicodeString Version::buildUnicodeGitCommitCount()
{
	UnicodeString str;
	str.format(L"%s%d",
		GitUncommittedChanges ? L"~" : L"",
		GitRevision);
	return str;
}

AsciiString Version::buildAsciiGitTagOrHash()
{
	AsciiString str;
	str.format("%s%s",
		GitUncommittedChanges ? "~" : "",
		GitTag[0] ? GitTag : GitShortSHA1);
	return str;
}

UnicodeString Version::buildUnicodeGitTagOrHash()
{
	UnicodeString str;
	str.translate(buildAsciiGitTagOrHash());
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
