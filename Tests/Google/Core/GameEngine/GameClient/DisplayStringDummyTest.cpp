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

#include "GameClient/DisplayStringManager.h"

class DisplayStringDummyTest : public testing::Test
{
protected:
	virtual void SetUp() override
	{
		m_previousFontLibrary = TheFontLibrary;
		TheFontLibrary = &m_fonts;
	}

	virtual void TearDown() override
	{
		TheFontLibrary = m_previousFontLibrary;
	}

	FontLibraryDummy m_fonts;
	FontLibrary *m_previousFontLibrary;
};

TEST_F(DisplayStringDummyTest, FontKeepsMetricsWithoutDeviceData)
{
	GameFont *font = m_fonts.getFont("Arial", 12, FALSE);
	ASSERT_NE(font, nullptr);
	EXPECT_EQ(font->fontData, nullptr);
	EXPECT_EQ(font->height, 12);
	EXPECT_EQ(m_fonts.getFont("Arial", 12, FALSE), font);
	EXPECT_NE(m_fonts.getFont("Arial", 12, TRUE), font);
}

TEST_F(DisplayStringDummyTest, StringKeepsTextAndFontButHasNoSize)
{
	DisplayStringManagerDummy strings;
	DisplayString *text = strings.newDisplayString();
	ASSERT_NE(text, nullptr);
	ASSERT_NE(text->getFont(), nullptr);

	text->setText(UnicodeString(L"Headless"));
	text->appendChar(L'!');
	EXPECT_EQ(text->getText(), UnicodeString(L"Headless!"));
	EXPECT_EQ(text->getTextLength(), 9);

	text->setWordWrap(20);
	text->setUseHotkey(TRUE, 0);
	text->draw(0, 0, 0, 0);
	Int width = -1;
	Int height = -1;
	text->getSize(&width, &height);
	EXPECT_EQ(width, 0);
	EXPECT_EQ(height, 0);
	EXPECT_EQ(text->getWidth(), 0);

	strings.freeDisplayString(text);
	strings.freeDisplayString(nullptr);
}

TEST_F(DisplayStringDummyTest, GroupMarkersExistAfterLoad)
{
	DisplayStringManagerDummy strings;
	strings.postProcessLoad();
	ASSERT_NE(strings.getGroupNumeralString(1), nullptr);
	EXPECT_EQ(strings.getFormationLetterString(), strings.getGroupNumeralString(1));
}
