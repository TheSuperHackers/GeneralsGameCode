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

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include "Common/UnicodeString.h"
#include "Common/XferLoad.h"
#include "Common/XferSave.h"

// The save file format stores a unicode string as a length byte followed by 2 byte UTF-16 code
// units, the width of wchar_t on Windows. The engine writes and reads that width on every target,
// including the ones where wchar_t is 4 bytes, so that save files stay interchangeable between
// platforms and with the retail game.
class XferUnicodeStringTest : public ::testing::Test
{
protected:
	virtual void SetUp() override
	{
		m_directory = std::filesystem::temp_directory_path()
			/ ("XferUnicodeStringTest_" + std::to_string(std::random_device()()));
		std::filesystem::remove_all(m_directory);
		std::filesystem::create_directories(m_directory);
		m_path = (m_directory / "unicode.sav").string();
	}

	virtual void TearDown() override
	{
		std::filesystem::remove_all(m_directory);
	}

	// Writes the string to the test file and returns the bytes that ended up in it.
	std::vector<UnsignedByte> writeString(UnicodeString text)
	{
		XferSave xfer;
		xfer.open(AsciiString(m_path.c_str()));
		xfer.xferUnicodeString(&text);
		xfer.close();

		std::ifstream file(m_path, std::ios::binary);
		return std::vector<UnsignedByte>(std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>());
	}

	// Reads a string back from the test file.
	UnicodeString readString()
	{
		XferLoad xfer;
		xfer.open(AsciiString(m_path.c_str()));
		UnicodeString text;
		xfer.xferUnicodeString(&text);
		xfer.close();
		return text;
	}

	std::filesystem::path m_directory;
	std::string m_path;
};

TEST_F(XferUnicodeStringTest, WritesTwoBytesPerCharacter)
{
	const std::vector<UnsignedByte> bytes = writeString(UnicodeString(L"Début"));

	// one length byte, then one 2 byte code unit per character
	ASSERT_EQ(bytes.size(), 1u + 2u * 5u);
	EXPECT_EQ(bytes[0], 5u);

	const UnsignedByte expected[] = { 'D', 0x00, 0xE9, 0x00, 'b', 0x00, 'u', 0x00, 't', 0x00 };
	EXPECT_TRUE(std::equal(std::begin(expected), std::end(expected), bytes.begin() + 1));
}

TEST_F(XferUnicodeStringTest, ReadsBackWhatItWrote)
{
	writeString(UnicodeString(L"Début de mission : USA 2"));

	EXPECT_STREQ(readString().str(), L"Début de mission : USA 2");
}

TEST_F(XferUnicodeStringTest, LeavesTheFilePositionOnTheNextValue)
{
	// A string that is read too wide swallows the data behind it, which is what garbled the save
	// game descriptions and broke the reads that follow them.
	{
		XferSave xfer;
		xfer.open(AsciiString(m_path.c_str()));
		UnicodeString text(L"Début");
		xfer.xferUnicodeString(&text);
		AsciiString mapLabel("MD_USA01.map");
		xfer.xferAsciiString(&mapLabel);
		xfer.close();
	}

	XferLoad xfer;
	xfer.open(AsciiString(m_path.c_str()));
	UnicodeString text;
	xfer.xferUnicodeString(&text);
	AsciiString mapLabel;
	xfer.xferAsciiString(&mapLabel);
	xfer.close();

	EXPECT_STREQ(text.str(), L"Début");
	EXPECT_STREQ(mapLabel.str(), "MD_USA01.map");
}

TEST_F(XferUnicodeStringTest, WritesAnEmptyStringAsItsLengthAlone)
{
	const std::vector<UnsignedByte> bytes = writeString(UnicodeString(L""));

	ASSERT_EQ(bytes.size(), 1u);
	EXPECT_EQ(bytes[0], 0u);
	EXPECT_STREQ(readString().str(), L"");
}
