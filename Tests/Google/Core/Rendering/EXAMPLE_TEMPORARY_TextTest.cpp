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

#include "RenderingTestFixture.h"
#include "WW3D2/render2dsentence.h"

// The UI draws its text with Render2DSentenceClass from glyphs that GDI renders, as W3DDisplayString does,
// including the drop shadow below the text.
TEST_F(RenderingTest, Text)
{
	const unsigned int background = 0x204060;
	clear(background);

	FontCharsClass *font = NEW_REF(FontCharsClass, ());
	ASSERT_TRUE(font->Initialize_GDI_Font("Arial", 16, true));
	Render2DSentenceClass sentence;
	sentence.Set_Font(font);
	REF_PTR_RELEASE(font);

	// The black shadow first, then the text over it.
	const WCHAR *text = L"Command & Conquer";
	sentence.Build_Sentence(text, nullptr, nullptr);
	sentence.Set_Location(Vector2(18.0f, 18.0f));
	sentence.Draw_Sentence(0xFF000000);
	sentence.Set_Location(Vector2(16.0f, 16.0f));
	sentence.Draw_Sentence(0xFFFFC000);
	sentence.Render();
	const Vector2 extents = sentence.Get_Text_Extents(text);

	capture();
	EXPECT_GT(extents.X, 100.0f);
	// GDI and its fonts differ between systems, so only some text color is expected in the middle row of the text.
	bool found = false;
	for (int x = 16; x < 16 + extents.X; ++x)
	{
		found |= pixel(x, 16 + static_cast<int>(extents.Y) / 2) == 0xFFC000;
	}
	EXPECT_TRUE(found);
}
