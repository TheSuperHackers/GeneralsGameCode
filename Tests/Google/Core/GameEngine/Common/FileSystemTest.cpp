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
#include <cstring>
#include <iterator>
#include <set>
#include <string>
#include <vector>

#include "Common/FileSystem.h"
#include "Common/STLTypedefs.h"

#include "FileSystemTestFixture.h"

class FileSystemTest : public FileSystemTestFixture
{
protected:
	// Writes a file with the file system of the game.
	static void writeFile(const char *filename, const char *content, Int access = File::WRITE | File::CREATE | File::BINARY)
	{
		File *file = TheFileSystem->openFile(filename, access);
		ASSERT_NE(file, nullptr) << filename;
		const Int size = static_cast<Int>(strlen(content));
		EXPECT_EQ(file->write(content, size), size);
		file->close();
	}
};

static std::string toPathKey(const char *path)
{
	Char key[_MAX_PATH];
	EXPECT_TRUE(FileSystem::makePathKey(key, sizeof(key), path)) << path;
	return key;
}

static int sign(int value)
{
	return (value > 0) - (value < 0);
}

template <typename FilenameContainer>
static std::vector<std::string> toVector(const FilenameContainer& filenames)
{
	std::vector<std::string> names;
	for (typename FilenameContainer::const_iterator it = filenames.begin(); it != filenames.end(); ++it)
	{
		names.push_back(it->str());
	}
	return names;
}

// The path key decides which paths refer to the same game file: the file exist cache and the archive files
// are searched with it.
TEST(FileSystemPathKey, IsLowercaseWithSingleBackslashes)
{
	EXPECT_EQ(toPathKey("Data\\INI\\Weapon.ini"), "data\\ini\\weapon.ini");
	EXPECT_EQ(toPathKey("Data/INI/Weapon.ini"), "data\\ini\\weapon.ini");
	EXPECT_EQ(toPathKey("DATA\\\\INI/\\/WEAPON.INI"), "data\\ini\\weapon.ini");
	EXPECT_EQ(toPathKey("/Data//INI/"), "\\data\\ini\\");
	EXPECT_EQ(toPathKey("Art\\Textures\\Tank_01 [Snow].TGA"), "art\\textures\\tank_01 [snow].tga");
	EXPECT_EQ(toPathKey(""), "");
}

#ifdef _WIN32
// Windows reads two separators at the start of a path as a UNC or device path, which is not the rooted
// path with one separator. Later runs of separators count as one, as they do for Windows.
TEST(FileSystemPathKey, KeepsSeparatorPairAtStart)
{
	EXPECT_EQ(toPathKey("\\\\Server\\Share\\Maps"), "\\\\server\\share\\maps");
	EXPECT_EQ(toPathKey("//Server//Share/Maps"), "\\\\server\\share\\maps");
	EXPECT_EQ(toPathKey("/\\Server"), "\\\\server");
	EXPECT_EQ(toPathKey("\\\\\\\\Server"), "\\\\\\server");
	EXPECT_EQ(toPathKey("\\\\?\\C:\\Data"), "\\\\?\\c:\\data");
	EXPECT_EQ(toPathKey("\\\\.\\Pipe\\Name"), "\\\\.\\pipe\\name");
	EXPECT_NE(toPathKey("\\\\Server\\Share"), toPathKey("\\Server\\Share"));
}
#endif

TEST(FileSystemPathKey, ReportsKeyThatDoesNotFit)
{
	Char key[9];

	EXPECT_TRUE(FileSystem::makePathKey(key, sizeof(key), "Data//Abc"));
	EXPECT_STREQ(key, "data\\abc");

	EXPECT_FALSE(FileSystem::makePathKey(key, sizeof(key), "Data\\Abcd"));
	EXPECT_STREQ(key, "");
}

TEST(FileSystemPathKey, ComparePathOrdersLikePathKeys)
{
	static const char *const paths[] =
	{
		"", "a", "A", "b", "aB", "Ab", "a.b", "a b", "a_b", "a[b", "a0",
		"a\\", "a/", "a\\\\", "a//", "\\a", "/a", "//a",
		"\\\\", "\\\\\\a", "\\\\\\\\a", "\\\\a\\b", "\\\\?\\c:\\a", "\\\\.\\a",
		"a\\b", "a/b", "a//b", "a\\/b", "A\\B\\", "a/b/",
		"a\\b\\c", "a/b\\c", "a\\b.c", "a\\b c", "a\\b_c",
		"Data\\INI", "Data\\INI\\", "Data\\INI.big", "Data\\INIZH.big", "Data\\INI_",
		"Data\\INI\\Weapon.ini", "data/ini/weapon.ini", "Data\\INI\\Weapon.ini.bak",
		"0", "Z", "z", "_", "[", "^", "~",
	};

	for (size_t a = 0; a < std::size(paths); ++a)
	{
		for (size_t b = 0; b < std::size(paths); ++b)
		{
			const int expected = sign(strcmp(toPathKey(paths[a]).c_str(), toPathKey(paths[b]).c_str()));
			EXPECT_EQ(sign(FileSystem::comparePath(paths[a], paths[b])), expected) << "'" << paths[a] << "' and '" << paths[b] << "'";
		}
	}
}

TEST(FilenameList, HoldsFileOnceInAnySpelling)
{
	FilenameList filenames;
	filenames.insert("Data\\INI\\Weapon.ini");
	filenames.insert("data/ini//WEAPON.INI");
	filenames.insert("Data/INI/Weapon.ini");
	filenames.insert("Data\\INI\\Armor.ini");

	const std::vector<std::string> expected = { "Data\\INI\\Armor.ini", "Data\\INI\\Weapon.ini" };
	EXPECT_EQ(toVector(filenames), expected);
}

#ifdef _WIN32
TEST(FilenameList, HoldsUncPathApartFromRootedPath)
{
	FilenameList filenames;
	filenames.insert("\\\\Server\\Share\\Maps\\A.map");
	filenames.insert("//server/share/maps/a.map");
	filenames.insert("\\Server\\Share\\Maps\\A.map");

	const std::vector<std::string> expected = { "\\\\Server\\Share\\Maps\\A.map", "\\Server\\Share\\Maps\\A.map" };
	EXPECT_EQ(toVector(filenames), expected);
}
#endif

// The game loads the INI files of a directory in the order of the file list, which was ordered by
// names without regard to letter case before it was ordered by path keys.
TEST(FilenameList, KeepsOrderOfNamesWithBackslashes)
{
	static const char *const names[] =
	{
		"Data\\INI\\Weapon.ini",
		"data\\ini\\Armor.ini",
		"Data\\INI\\Object.ini",
		"Data\\INI\\Object 2.ini",
		"Data\\INI\\Object-2.ini",
		"Data\\INI\\Object_2.ini",
		"Data\\INI\\Object2.ini",
		"Data\\INI\\ObjectCreationList.ini",
		"Data\\INI\\Object\\AmericaVehicle.ini",
		"Data\\INI\\Object\\americaInfantry.ini",
		"Data\\INI\\object\\ChinaTank.ini",
		"Data\\INI\\Object\\China_Tank.ini",
		"Data\\INI\\Object\\[Test].ini",
		"Data\\INI\\Object\\Sub\\A.ini",
		"Data\\INI\\Default\\Object.ini",
		"Data\\INIZH.big",
		"Data\\INI.big",
	};

	FilenameList filenames;
	std::set<AsciiString, rts::less_than_nocase<AsciiString> > filenamesNoCase;
	for (size_t i = 0; i < std::size(names); ++i)
	{
		filenames.insert(names[i]);
		filenamesNoCase.insert(names[i]);
	}

	EXPECT_EQ(filenames.size(), std::size(names));
	EXPECT_EQ(toVector(filenames), toVector(filenamesNoCase));
}

// The file exist cache holds a file under its path key. What is learned about a file in one spelling
// of its path is therefore known for all others.
TEST_F(FileSystemTest, KnowsCreatedFileInAnySpelling)
{
	EXPECT_FALSE(TheFileSystem->doesFileExist("Data\\INI\\Weapon.ini"));
	EXPECT_FALSE(TheFileSystem->doesFileExist("data/ini/weapon.ini"));

	writeFile("Data\\INI\\Weapon.ini", "Weapon");

	EXPECT_TRUE(TheFileSystem->doesFileExist("Data\\INI\\Weapon.ini"));
	EXPECT_TRUE(TheFileSystem->doesFileExist("data/ini/weapon.ini"));
	EXPECT_TRUE(TheFileSystem->doesFileExist("DATA\\\\INI//WEAPON.INI"));
}
