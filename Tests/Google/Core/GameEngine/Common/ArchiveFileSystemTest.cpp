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
#include <string>
#include <vector>

#include "Common/ArchiveFileSystem.h"

#include "FileSystemTestFixture.h"

class ArchiveFileSystemTest : public FileSystemTestFixture
{
protected:
	static std::vector<std::string> listFiles(const char *directory, const char *searchName)
	{
		FilenameList filenames;
		TheArchiveFileSystem->getFileListInDirectory("", directory, searchName, filenames, TRUE);

		std::vector<std::string> names;
		for (FilenameList::const_iterator it = filenames.begin(); it != filenames.end(); ++it)
		{
			names.push_back(it->str());
		}
		return names;
	}

	static std::string readFile(const char *filename, FileInstance instance = 0)
	{
		return FileSystemTestFixture::readFile(TheArchiveFileSystem->openFile(filename, 0, instance));
	}

	static Int64 getFileSize(const char *filename, FileInstance instance = 0)
	{
		FileInfo fileInfo;
		return TheArchiveFileSystem->getFileInfo(filename, &fileInfo, instance) ? fileInfo.size() : -1;
	}
};

INSTANTIATE_TEST_SUITE_P(FS, ArchiveFileSystemTest, ::testing::ValuesIn(localFileSystems), localFileSystemName);

TEST_P(ArchiveFileSystemTest, FindsFileInAnySpelling)
{
	writeArchiveFile("Base/Base.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon" },
		{ "Art/Textures/Tank.TGA", "Tank" },
	});
	loadArchiveFiles("Base\\");

	EXPECT_TRUE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Weapon.ini"));
	EXPECT_TRUE(TheArchiveFileSystem->doesFileExist("data\\ini\\weapon.ini"));
	EXPECT_TRUE(TheArchiveFileSystem->doesFileExist("Data/INI/Weapon.ini"));
	EXPECT_TRUE(TheArchiveFileSystem->doesFileExist("DATA\\\\INI//WEAPON.INI"));
	EXPECT_TRUE(TheArchiveFileSystem->doesFileExist("\\Data\\INI\\Weapon.ini"));
	EXPECT_TRUE(TheArchiveFileSystem->doesFileExist("Art\\Textures\\Tank.tga"));
	EXPECT_TRUE(TheArchiveFileSystem->doesFileExist("art/textures/tank.tga"));

	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Armor.ini"));
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI"));
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI\\"));
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("INI\\Weapon.ini"));
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Weapon.ini"));
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist(""));
}

TEST_P(ArchiveFileSystemTest, OpensFile)
{
	writeArchiveFile("Base/Base.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon" },
		{ "Data\\INI\\Armor.ini", "Armor" },
	});
	loadArchiveFiles("Base\\");

	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini"), "Weapon");
	EXPECT_EQ(readFile("data/ini/armor.ini"), "Armor");
	EXPECT_EQ(getFileSize("data/ini/weapon.ini"), 6);
	EXPECT_EQ(getFileSize("Data\\INI\\Armor.ini"), 5);

	EXPECT_EQ(TheArchiveFileSystem->openFile("Data\\INI\\Locomotor.ini"), nullptr);
	EXPECT_EQ(getFileSize("Data\\INI\\Locomotor.ini"), -1);
}

// A file that several archive files hold has one file instance for each of them.
TEST_P(ArchiveFileSystemTest, OrdersFileInstancesByLoadOrder)
{
	writeArchiveFile("Base/Base.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon_base" },
		{ "Data\\INI\\Armor.ini", "Armor" },
	});
	writeArchiveFile("Patch/Patch.big", {
		{ "data/ini/weapon.ini", "Weapon_patch" },
	});
	loadArchiveFiles("Base\\");
	loadArchiveFiles("Patch\\");

	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 0), "Weapon_base");
	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 1), "Weapon_patch");
	EXPECT_EQ(getFileSize("Data\\INI\\Weapon.ini", 0), 11);
	EXPECT_EQ(getFileSize("Data\\INI\\Weapon.ini", 1), 12);
	EXPECT_TRUE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Weapon.ini", 1));

	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Weapon.ini", 2));
	EXPECT_EQ(TheArchiveFileSystem->openFile("Data\\INI\\Weapon.ini", 0, 2), nullptr);
	EXPECT_EQ(getFileSize("Data\\INI\\Weapon.ini", 2), -1);
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Armor.ini", 1));
}

// The archive files of one directory are loaded in the order of their names.
TEST_P(ArchiveFileSystemTest, OrdersFileInstancesByArchiveFileName)
{
	writeArchiveFile("Base/INI.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon_ini" },
	});
	writeArchiveFile("Base/!Patch.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon_!patch" },
	});
	loadArchiveFiles("Base\\");

	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 0), "Weapon_!patch");
	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 1), "Weapon_ini");
}

// The archive files of a mod are loaded last and their files come first.
TEST_P(ArchiveFileSystemTest, OverwritePutsFileInstancesInFront)
{
	writeArchiveFile("Base/Base.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon_base" },
		{ "Data\\INI\\Armor.ini", "Armor" },
	});
	writeArchiveFile("Mod/Mod.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon_mod" },
	});
	writeArchiveFile("Mod2/Mod2.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon_mod2" },
	});
	loadArchiveFiles("Base\\");
	loadArchiveFiles("Mod\\", TRUE);

	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 0), "Weapon_mod");
	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 1), "Weapon_base");
	EXPECT_EQ(readFile("Data\\INI\\Armor.ini", 0), "Armor");

	loadArchiveFiles("Mod2\\", TRUE);

	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 0), "Weapon_mod2");
	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 1), "Weapon_mod");
	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 2), "Weapon_base");
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Weapon.ini", 3));
}

TEST_P(ArchiveFileSystemTest, LastFileOfSamePathInArchiveFileCounts)
{
	writeArchiveFile("Base/Base.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon_first" },
		{ "Data\\INI\\Armor.ini", "Armor" },
		{ "data/ini/WEAPON.ini", "Weapon_second" },
	});
	loadArchiveFiles("Base\\");

	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 0), "Weapon_second");
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Weapon.ini", 1));
}

TEST_P(ArchiveFileSystemTest, ListsFilesOfDirectory)
{
	writeArchiveFile("Base/Base.big", {
		{ "Data\\INI\\Weapon.ini", "1" },
		{ "Data\\INI\\Armor.INI", "2" },
		{ "Data\\INI\\Object\\Tank.ini", "3" },
		{ "Data\\INI\\Object\\Sub\\Jeep.ini", "4" },
		{ "Data\\INI\\Readme.txt", "5" },
		{ "Data\\INIZH\\Other.ini", "6" },
		{ "Data\\Scripts\\Skirmish.scb", "7" },
		{ "Art\\Weapon.ini", "8" },
	});
	writeArchiveFile("Patch/Patch.big", {
		{ "Data\\INI\\Weapon.ini", "9" },
		{ "Data\\INI\\Science.ini", "10" },
	});
	loadArchiveFiles("Base\\");
	loadArchiveFiles("Patch\\");

	// The names begin with the directory as it is passed in. The rest is in lowercase.
	const std::vector<std::string> iniFiles =
	{
		"Data\\INI\\armor.ini",
		"Data\\INI\\object\\sub\\jeep.ini",
		"Data\\INI\\object\\tank.ini",
		"Data\\INI\\science.ini",
		"Data\\INI\\weapon.ini",
	};
	EXPECT_EQ(listFiles("Data\\INI\\", "*.ini"), iniFiles);

	const std::vector<std::string> objectFiles =
	{
		"data/ini/object\\sub\\jeep.ini",
		"data/ini/object\\tank.ini",
	};
	EXPECT_EQ(listFiles("data/ini/object", "*.ini"), objectFiles);

	const std::vector<std::string> weaponFiles =
	{
		"Art\\weapon.ini",
	};
	EXPECT_EQ(listFiles("Art\\", "w*.ini"), weaponFiles);

	// An empty directory is the root, which holds all archived files.
	const std::vector<std::string> allIniFiles =
	{
		"art\\weapon.ini",
		"data\\ini\\armor.ini",
		"data\\ini\\object\\sub\\jeep.ini",
		"data\\ini\\object\\tank.ini",
		"data\\ini\\science.ini",
		"data\\ini\\weapon.ini",
		"data\\inizh\\other.ini",
	};
	EXPECT_EQ(listFiles("", "*.ini"), allIniFiles);

	EXPECT_TRUE(listFiles("Data\\INI\\", "*.tga").empty());
	EXPECT_TRUE(listFiles("Data\\IN\\", "*.ini").empty());
	EXPECT_TRUE(listFiles("Maps\\", "*.map").empty());
}

// The game looks at all textures of a directory to pick the larger one of each.
TEST_P(ArchiveFileSystemTest, GivesArchivedFilesOfDirectory)
{
	writeArchiveFile("Base/Base.big", {
		{ "Art\\Textures\\Tank.tga", "Tank_base" },
		{ "Art\\Textures\\Jeep.tga", "Jeep" },
		{ "Art\\TexturesOld\\Tank.tga", "Tank_old" },
		{ "Art\\W3D\\Tank.w3d", "Mesh" },
	});
	writeArchiveFile("Patch/Patch.big", {
		{ "Art\\Textures\\Tank.tga", "Tank_patch" },
	});
	loadArchiveFiles("Base\\");
	loadArchiveFiles("Patch\\");

	const stl::range<ArchivedFileMap> files = TheArchiveFileSystem->friend_getArchivedFiles("Art/Textures/");
	std::vector<std::string> keys;
	std::vector<UnsignedInt> sizes;
	for (ArchivedFileMap::iterator it = files.begin; it != files.end; ++it)
	{
		keys.push_back(it->first.c_str());
		sizes.push_back(it->second.m_size);
	}

	const std::vector<std::string> expectedKeys =
	{
		"art\\textures\\jeep.tga",
		"art\\textures\\tank.tga",
		"art\\textures\\tank.tga",
	};
	const std::vector<UnsignedInt> expectedSizes = { 4, 9, 10 };
	EXPECT_EQ(keys, expectedKeys);
	EXPECT_EQ(sizes, expectedSizes);
}

TEST_P(ArchiveFileSystemTest, RemovesFilesOfArchiveFile)
{
	writeArchiveFile("Base/Base.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon_base" },
		{ "Data\\INI\\Armor.ini", "Armor" },
	});
	writeArchiveFile("Mod/Mod.big", {
		{ "Data\\INI\\Weapon.ini", "Weapon_mod" },
		{ "Data\\INI\\Science.ini", "Science" },
	});
	loadArchiveFiles("Base\\");
	loadArchiveFiles("Mod\\", TRUE);

	m_archiveFileSystem->removeFilesOfArchiveFile("Mod\\Mod.big");

	EXPECT_EQ(readFile("Data\\INI\\Weapon.ini", 0), "Weapon_base");
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Weapon.ini", 1));
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Science.ini"));
	EXPECT_EQ(readFile("Data\\INI\\Armor.ini", 0), "Armor");

	m_archiveFileSystem->removeFilesOfArchiveFile("Base\\Base.big");

	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Weapon.ini"));
	EXPECT_FALSE(TheArchiveFileSystem->doesFileExist("Data\\INI\\Armor.ini"));
	EXPECT_TRUE(listFiles("Data\\INI\\", "*.ini").empty());
}
