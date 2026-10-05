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

#pragma once

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

#include "Common/BIGFileSystem.h"
#include "Common/FileSystem.h"
#include "Common/LocalFileSystem.h"

#ifdef _WIN32
#include "Win32Device/Common/Win32LocalFileSystem.h"
#else
#include "StdDevice/Common/StdLocalFileSystem.h"
#endif

// Each test runs in a working directory of its own, with the file systems of the game set up on it
// like the game does. The test writes the files it needs into that directory.
class FileSystemTestFixture : public ::testing::Test
{
protected:
	virtual void SetUp() override
	{
		m_previousDirectory = std::filesystem::current_path();
		m_directory = std::filesystem::temp_directory_path() / ("FileSystemTest_" + std::to_string(std::random_device()()));
		std::filesystem::remove_all(m_directory);
		std::filesystem::create_directories(m_directory);
		std::filesystem::current_path(m_directory);

#ifdef _WIN32
		TheLocalFileSystem = new Win32LocalFileSystem;
#else
		TheLocalFileSystem = new StdLocalFileSystem;
#endif
		TheArchiveFileSystem = new BIGFileSystem;
		TheFileSystem = new FileSystem;
	}

	virtual void TearDown() override
	{
		delete TheFileSystem;
		TheFileSystem = nullptr;
		delete TheArchiveFileSystem;
		TheArchiveFileSystem = nullptr;
		delete TheLocalFileSystem;
		TheLocalFileSystem = nullptr;

		std::filesystem::current_path(m_previousDirectory);
		std::filesystem::remove_all(m_directory);
	}

	// Writes a file without the file systems of the game. The name is relative to the working directory.
	static void writeLooseFile(const char *name, const std::string& content)
	{
		const std::filesystem::path path(name);
		if (path.has_parent_path())
		{
			std::filesystem::create_directories(path.parent_path());
		}
		std::ofstream(path, std::ios::binary) << content;
	}

	// Returns the content of the file and closes it. A file that was not opened fails the test.
	static std::string readFile(File *file)
	{
		if (file == nullptr)
		{
			ADD_FAILURE() << "The file was not opened";
			return std::string();
		}

		std::string content(file->size(), '\0');
		file->read(&content[0], static_cast<Int>(content.size()));
		file->close();
		return content;
	}

	std::filesystem::path m_previousDirectory;
	std::filesystem::path m_directory;
};
