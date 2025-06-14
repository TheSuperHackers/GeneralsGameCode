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

#include "ScopedFileRenamer.h"


ScopedFileRenamer::ScopedFileRenamer()
	: m_file(NULL)
{
}

ScopedFileRenamer::ScopedFileRenamer(const char* oldName, const char* newName)
	: m_file(NULL)
{
	this->rename(oldName, newName);
}

ScopedFileRenamer::~ScopedFileRenamer()
{
	revert();
}

bool ScopedFileRenamer::rename(const char* oldName, const char* newName)
{
	revert();

	m_oldName = oldName;
	m_newName = newName;

	if (0 == ::rename(m_oldName.c_str(), m_newName.c_str()))
	{
		// Creates an empty *.tmp dummy file to remember that this program has renamed the file.
		// If the program would crash before the file was renamed back to the previous name, then
		// the rename condition will be able to recover the next time this code runs successfully.
		std::string tmpFilename = createTmpName();
		m_file = ::fopen(tmpFilename.c_str(), "wb");

		return true;
	}

	return false;
}

bool ScopedFileRenamer::revert()
{
	if (m_oldName.empty())
		return false;

	bool success = false;
	std::string tmpName;

	if (m_file == NULL)
	{
		tmpName = createTmpName();
		m_file = ::fopen(tmpName.c_str(), "rb");
	}

	if (m_file != NULL)
	{
		::fclose(m_file);
		m_file = NULL;

		if (0 == ::rename(m_newName.c_str(), m_oldName.c_str()))
			success = true;

		if (tmpName.empty())
			tmpName = createTmpName();

		::remove(tmpName.c_str());
	}

	m_oldName.clear();
	m_newName.clear();

	return success;
}

std::string ScopedFileRenamer::createTmpName() const
{
	std::string tmpFilename = m_oldName;
	tmpFilename.append(".tmp");
	return tmpFilename;
}
