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


#pragma once

#ifndef __LANGUAGEFILTER_H
#define __LANGUAGEFILTER_H

#include "Common/STLTypedefs.h"
#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"

class File;

struct AsciiStringLessThan
{
	Bool operator()(AsciiString a, AsciiString b) const
	{
		return (a.compareNoCase(b) < 0);
	}
};

struct UnicodeStringLessThan
{
	Bool operator()(UnicodeString a, UnicodeString b) const
	{
		return (a.compareNoCase(b) < 0);
	}
};

struct UnicodeStringsEqual
{
	Bool operator()(UnicodeString a, UnicodeString b) const
	{
		Bool retval = (a.compareNoCase(b) == 0);
		DEBUG_LOG(("Comparing %ls with %ls, return value is ", a.str(), b.str()));
		if (retval) {
			DEBUG_LOG(("true."));
		} else {
			DEBUG_LOG(("false."));
		}
		return retval;
	}
};

typedef std::map<UnicodeString, Bool, UnicodeStringLessThan> LangMap;
typedef std::map<UnicodeString, Bool, UnicodeStringLessThan>::iterator LangMapIter;

static const int LANGUAGE_XOR_KEY = 0x5555;
static const char BadWordFileName[] = "langdata.dat";

class LanguageFilter : public SubsystemInterface {
public:
	LanguageFilter();
	~LanguageFilter();

	void init();
	void reset();
	void update();
	void filterLine(UnicodeString &line);

protected:
	Bool readWord(File *file1, WideChar *buf);
	void unHaxor(UnicodeString &word);
	LangMap m_wordList;
	LangMap m_subWordList;
};

extern LanguageFilter *TheLanguageFilter;
LanguageFilter * createLanguageFilter();

#endif //#define __LANGUAGEFILTER_H
