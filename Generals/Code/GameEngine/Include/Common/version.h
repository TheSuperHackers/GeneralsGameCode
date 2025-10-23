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

// FILE: version.h //////////////////////////////////////////////////////
// Generals version number class
// Author: Matthew D. Campbell, November 2001

#pragma once

#include <time.h>

/**
 * The Version class formats the version number into integer and string
 * values for different parts of the game.
 * @todo: increment build number on compile, and stamp exe with username
 */
// TheSuperHackers @tweak The Version class now also provides Git information
// alongside the original Version information.
class Version
{
public:
	Version();

	UnsignedInt getVersionNumber() const;           ///< Return a 4-byte integer suitable for WOLAPI

	AsciiString getAsciiVersion() const;            ///< Return a human-readable game version number
	UnicodeString getUnicodeVersion() const;        ///< Return a human-readable game version number. Is decorated with localized string

	AsciiString getAsciiBuildTime() const;          ///< Return a formated date/time string for build time
	UnicodeString getUnicodeBuildTime() const;      ///< Return a formated date/time string for build time. Is decorated with localized string

	AsciiString getAsciiBuildLocation() const;      ///< Return a string with the build location
	UnicodeString getUnicodeBuildLocation() const;  ///< Return a string with the build location. Is decorated with localized string

	AsciiString getAsciiBuildUser() const;          ///< Return a string with the build user
	UnicodeString getUnicodeBuildUser() const;      ///< Return a string with the build user. Is decorated with localized string

	static Int getGitCommitCount();                    ///< Returns the git commit count as a number
	static time_t getGitCommitTime();                  ///< Returns the git head commit time as a UTC timestamp
	static const char* getGitCommitAuthorName();       ///< Returns the git head commit author name

	AsciiString getAsciiGitCommitCount() const;        ///< Returns the git commit count. Is prefixed with ~ if there were uncommitted changes.
	UnicodeString getUnicodeGitCommitCount() const;    ///< Returns the git commit count. Is prefixed with ~ if there were uncommitted changes.

	AsciiString getAsciiGitTagOrHash() const;          ///< Returns the git head commit tag or hash. Is prefixed with ~ if there were uncommitted changes.
	UnicodeString getUnicodeGitTagOrHash() const;      ///< Returns the git head commit tag or hash. Is prefixed with ~ if there were uncommitted changes.

	AsciiString getAsciiGitCommitTime() const;         ///< Returns the git head commit time in YYYY-mm-dd HH:MM:SS format
	UnicodeString getUnicodeGitCommitTime() const;     ///< Returns the git head commit time in YYYY-mm-dd HH:MM:SS format

	AsciiString getAsciiGitVersion() const;            ///< Returns the git version
	UnicodeString getUnicodeGitVersion() const;        ///< Returns the git version

	AsciiString getAsciiBuildUserOrGitCommitAuthorName() const;
	UnicodeString getUnicodeBuildUserOrGitCommitAuthorName() const; ///< Is decorated with localized string

	UnicodeString getUnicodeProductTitle() const;
	UnicodeString getUnicodeProductVersion() const;
	UnicodeString getUnicodeProductAuthor() const; ///< Is decorated with localized string
	UnicodeString getUnicodeProductString() const; ///< Returns a string that contains product title, version and other, if specified. Is decorated with localized string
	UnicodeString getUnicodeProductVersionHashString() const; ///< Returns a string that contains the product string, game version and hashes. Is decorated with localized string

	Bool showFullVersion() const { return m_showFullVersion; }
	void setShowFullVersion( Bool val ) { m_showFullVersion = val; }

	void setVersion(Int major, Int minor, Int buildNum,
		Int localBuildNum, AsciiString user, AsciiString location,
		AsciiString buildTime, AsciiString buildDate);

private:
	static AsciiString buildAsciiGitCommitCount();
	static UnicodeString buildUnicodeGitCommitCount();

	static AsciiString buildAsciiGitTagOrHash();
	static UnicodeString buildUnicodeGitTagOrHash();

	static AsciiString buildAsciiGitCommitTime();
	static UnicodeString buildUnicodeGitCommitTime();

private:
	Int m_major;
	Int m_minor;
	Int m_buildNum;
	Int m_localBuildNum;
	AsciiString m_buildLocation;
	AsciiString m_buildUser;
	AsciiString m_buildTime;
	AsciiString m_buildDate;
	AsciiString m_asciiGitCommitCount;
	AsciiString m_asciiGitTagOrHash;
	AsciiString m_asciiGitCommitTime;
	UnicodeString m_unicodeGitCommitCount;
	UnicodeString m_unicodeGitTagOrHash;
	UnicodeString m_unicodeGitCommitTime;
	Bool m_showFullVersion;
};

extern Version *TheVersion;
