/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/FileSystem.h"
#include "Common/INI.h"

struct FileSystemIgnore
{
	enum Type
	{
		Auto,
		File,
		Dir,
	};

	FileSystemIgnore() : m_type(Auto) {}

	AsciiString m_name;
	Type m_type;

	static const FieldParse s_fieldParseTable[];
	static const char *const s_typeNames[];
};

const char *const FileSystemIgnore::s_typeNames[] =
{
	"AUTO",
	"FILE",
	"DIR",
	nullptr
};

const FieldParse FileSystemIgnore::s_fieldParseTable[] =
{
	{ "Name", INI::parseQuotedAsciiString, nullptr, offsetof(FileSystemIgnore, m_name) },
	{ "Type", INI::parseIndexList, s_typeNames,		offsetof(FileSystemIgnore, m_type) },
	{ nullptr, nullptr, nullptr, 0 }
};

void INI::parseFileSystemIgnoreDefinition(INI* ini)
{
	DEBUG_ASSERTCRASH(TheFileSystem != nullptr, ("TheFileSystem is null"));

	FileSystemIgnore fsIgnore;
	ini->initFromINI(&fsIgnore, fsIgnore.s_fieldParseTable);

	if (fsIgnore.m_type == FileSystemIgnore::Auto)
	{
		if (isFilePath(fsIgnore.m_name.str()))
			fsIgnore.m_type = FileSystemIgnore::File;
		else
			fsIgnore.m_type = FileSystemIgnore::Dir;
	}

	switch (fsIgnore.m_type)
	{
	case FileSystemIgnore::File:
		TheFileSystem->ignoreFile(fsIgnore.m_name);
		break;

	case FileSystemIgnore::Dir:
		TheFileSystem->ignoreDirectory(fsIgnore.m_name);
		break;

	default:
		DEBUG_CRASH(("Unhandled case in parseFileSystemIgnoreDefinition"));
		break;
	}
}
