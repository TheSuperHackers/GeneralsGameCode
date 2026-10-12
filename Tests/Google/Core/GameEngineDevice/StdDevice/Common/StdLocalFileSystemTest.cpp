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

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

#include "StdDevice/Common/StdLocalFileSystem.h"

// The game refers to its files with relative paths in any letter case and with either path separator.
// Each test runs in a working directory of its own that holds a few files.
class StdLocalFileSystemTest : public ::testing::Test
{
protected:
	virtual void SetUp() override
	{
		m_previousDirectory = std::filesystem::current_path();
		m_directory = std::filesystem::temp_directory_path() / ("StdLocalFileSystemTest_" + std::to_string(std::random_device()()));
		std::filesystem::remove_all(m_directory);

		writeFile("Data/INI/Weapon.ini", "Weapon");
		writeFile("Generals.str", "Generals");

		std::filesystem::current_path(m_directory);
	}

	virtual void TearDown() override
	{
		std::filesystem::current_path(m_previousDirectory);
		std::filesystem::remove_all(m_directory);
	}

	void writeFile(const char *name, const char *content) const
	{
		const std::filesystem::path path = m_directory / name;
		std::filesystem::create_directories(path.parent_path());
		std::ofstream(path, std::ios::binary) << content;
	}

	static std::string readFile(File *file)
	{
		std::string content(file->size(), '\0');
		file->read(&content[0], static_cast<Int>(content.size()));
		file->close();
		return content;
	}

	Int countFilesAndFolders() const
	{
		Int count = 0;
		for (std::filesystem::recursive_directory_iterator it(m_directory); it != std::filesystem::recursive_directory_iterator(); ++it)
		{
			++count;
		}
		return count;
	}

	std::filesystem::path m_previousDirectory;
	std::filesystem::path m_directory;
	StdLocalFileSystem m_fileSystem;
};

TEST_F(StdLocalFileSystemTest, FindsFileInAnyLetterCase)
{
	EXPECT_TRUE(m_fileSystem.doesFileExist("Data\\INI\\Weapon.ini"));
	EXPECT_TRUE(m_fileSystem.doesFileExist("Data\\ini\\WEAPON.INI"));
	EXPECT_TRUE(m_fileSystem.doesFileExist("data\\ini\\weapon.ini"));
	EXPECT_TRUE(m_fileSystem.doesFileExist("generals.STR"));

	EXPECT_FALSE(m_fileSystem.doesFileExist("Data\\INI\\Armor.ini"));
	EXPECT_FALSE(m_fileSystem.doesFileExist("Art\\INI\\Weapon.ini"));
	EXPECT_FALSE(m_fileSystem.doesFileExist("Weapon.ini"));
}

TEST_F(StdLocalFileSystemTest, FindsFileWithEitherSeparator)
{
	EXPECT_TRUE(m_fileSystem.doesFileExist("Data/INI/Weapon.ini"));
	EXPECT_TRUE(m_fileSystem.doesFileExist("Data\\INI/Weapon.ini"));
	EXPECT_TRUE(m_fileSystem.doesFileExist("data/ini\\weapon.ini"));
}

TEST_F(StdLocalFileSystemTest, FindsFileByAbsolutePath)
{
	const std::string path = (m_directory / "Data" / "INI" / "Weapon.ini").string();
	EXPECT_TRUE(m_fileSystem.doesFileExist(path.c_str()));

	const std::string pathInOtherCase = (m_directory / "data" / "ini" / "weapon.ini").string();
	EXPECT_TRUE(m_fileSystem.doesFileExist(pathInOtherCase.c_str()));
}

TEST_F(StdLocalFileSystemTest, OpensFileInAnyLetterCase)
{
	File *file = m_fileSystem.openFile("Data\\INI\\Weapon.ini", File::READ | File::BINARY);
	ASSERT_NE(file, nullptr);
	EXPECT_EQ(readFile(file), "Weapon");

	file = m_fileSystem.openFile("data\\ini\\weapon.ini", File::READ | File::BINARY);
	ASSERT_NE(file, nullptr);
	EXPECT_EQ(readFile(file), "Weapon");

	file = m_fileSystem.openFile("generals.str", File::READ | File::BINARY);
	ASSERT_NE(file, nullptr);
	EXPECT_EQ(readFile(file), "Generals");

	EXPECT_EQ(m_fileSystem.openFile("data\\ini\\armor.ini", File::READ | File::BINARY), nullptr);
}

TEST_F(StdLocalFileSystemTest, GetsFileInfoInAnyLetterCase)
{
	FileInfo fileInfo;
	ASSERT_TRUE(m_fileSystem.getFileInfo("data\\ini\\weapon.ini", &fileInfo));
	EXPECT_EQ(fileInfo.size(), 6);

	EXPECT_FALSE(m_fileSystem.getFileInfo("data\\ini\\armor.ini", &fileInfo));
}

TEST_F(StdLocalFileSystemTest, CreatesFileInFolderOfOtherLetterCase)
{
	File *file = m_fileSystem.openFile("data\\Scripts\\Skirmish.scb", File::WRITE | File::CREATE | File::BINARY);
	ASSERT_NE(file, nullptr);
	EXPECT_EQ(file->write("Skirmish", 8), 8);
	file->close();

	// The file is in the Data folder that exists already. No second folder is created next to it.
	EXPECT_TRUE(std::filesystem::exists(m_directory / "Data" / "Scripts" / "Skirmish.scb"));
	EXPECT_EQ(countFilesAndFolders(), 6);

	file = m_fileSystem.openFile("Data\\Scripts\\Skirmish.scb", File::READ | File::BINARY);
	ASSERT_NE(file, nullptr);
	EXPECT_EQ(readFile(file), "Skirmish");
}
