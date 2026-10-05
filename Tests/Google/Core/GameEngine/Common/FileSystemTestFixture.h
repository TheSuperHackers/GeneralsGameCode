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
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include "Common/BIGFileSystem.h"
#include "Common/FileSystem.h"
#include "Common/LocalFileSystem.h"

#include "StdDevice/Common/StdLocalFileSystem.h"

#ifdef _WIN32
#include "Win32Device/Common/Win32LocalFileSystem.h"
#endif

// A file that a test puts into an archive file.
struct ArchivedTestFile
{
	const char *path;
	const char *content;
};

class TestBIGFileSystem : public BIGFileSystem
{
public:
	// Removes the files of an archive file like closeArchiveFile does. That function cannot be used,
	// because it is made for the archive file of the music and stops the audio.
	void removeFilesOfArchiveFile(const char *name)
	{
		const ArchiveFileMap::iterator it = m_archiveFileMap.find(name);
		ASSERT_TRUE(it != m_archiveFileMap.end()) << name;
		removeFiles(it->second);
	}
};

// A local file system that the tests run against.
struct LocalFileSystemParam
{
	const char *name;
	LocalFileSystem *(*create)();
};

template <typename Type>
LocalFileSystem *createLocalFileSystem()
{
	return new Type;
}

// The game uses Win32LocalFileSystem on Windows and StdLocalFileSystem elsewhere. On Windows the tests run against both.
static const LocalFileSystemParam localFileSystems[] =
{
#ifdef _WIN32
	{ "Win32", createLocalFileSystem<Win32LocalFileSystem> },
#endif
	{ "Std", createLocalFileSystem<StdLocalFileSystem> },
};

inline std::string localFileSystemName(const ::testing::TestParamInfo<LocalFileSystemParam>& info)
{
	return info.param.name;
}

inline void PrintTo(const LocalFileSystemParam& param, std::ostream *os)
{
	*os << param.name;
}

// Each test runs in a working directory of its own, with the file systems of the game set up on it
// like the game does, once for each local file system. The test writes the files it needs into that directory.
class FileSystemTestFixture : public ::testing::TestWithParam<LocalFileSystemParam>
{
protected:
	virtual void SetUp() override
	{
		m_previousDirectory = std::filesystem::current_path();
		m_directory = std::filesystem::temp_directory_path() / ("FileSystemTest_" + std::to_string(std::random_device()()));
		std::filesystem::remove_all(m_directory);
		std::filesystem::create_directories(m_directory);
		std::filesystem::current_path(m_directory);

		TheLocalFileSystem = GetParam().create();
		m_archiveFileSystem = new TestBIGFileSystem;
		TheArchiveFileSystem = m_archiveFileSystem;
		TheFileSystem = new FileSystem;
	}

	virtual void TearDown() override
	{
		delete TheFileSystem;
		TheFileSystem = nullptr;
		delete TheArchiveFileSystem;
		TheArchiveFileSystem = nullptr;
		m_archiveFileSystem = nullptr;
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

	// Writes a BIG archive file. The header is followed by the table of the files and then by their contents.
	static void writeArchiveFile(const char *name, const std::vector<ArchivedTestFile>& files)
	{
		size_t contentOffset = 16;
		for (size_t i = 0; i < files.size(); ++i)
		{
			contentOffset += 8 + strlen(files[i].path) + 1;
		}

		std::string table;
		std::string contents;
		for (size_t i = 0; i < files.size(); ++i)
		{
			const size_t size = strlen(files[i].content);
			appendBigEndian(table, contentOffset + contents.size());
			appendBigEndian(table, size);
			table.append(files[i].path, strlen(files[i].path) + 1);
			contents.append(files[i].content, size);
		}

		std::string header("BIGF");
		appendLittleEndian(header, contentOffset + contents.size());
		appendBigEndian(header, files.size());
		appendBigEndian(header, contentOffset);

		writeLooseFile(name, header + table + contents);
	}

	// Loads the archive files of a directory, in the order of their names.
	static void loadArchiveFiles(const char *directory, Bool overwrite = FALSE)
	{
		EXPECT_TRUE(TheArchiveFileSystem->loadArchiveFilesFromDirectory(directory, "*.big", overwrite)) << directory;
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
	TestBIGFileSystem *m_archiveFileSystem;

private:
	static void appendBigEndian(std::string& data, size_t value)
	{
		data.push_back(static_cast<char>(value >> 24));
		data.push_back(static_cast<char>(value >> 16));
		data.push_back(static_cast<char>(value >> 8));
		data.push_back(static_cast<char>(value));
	}

	static void appendLittleEndian(std::string& data, size_t value)
	{
		data.push_back(static_cast<char>(value));
		data.push_back(static_cast<char>(value >> 8));
		data.push_back(static_cast<char>(value >> 16));
		data.push_back(static_cast<char>(value >> 24));
	}
};
