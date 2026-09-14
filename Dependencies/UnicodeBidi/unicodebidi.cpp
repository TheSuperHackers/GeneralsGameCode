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

#include "unicodebidi.h"

namespace
{

enum Direction
{
	Neutral,
	LeftToRight,
	RightToLeft
};

struct DirectionRange
{
	unsigned First;
	unsigned Last;
	Direction Value;
};

static const DirectionRange SupplementaryDirectionRanges[] = {
#include "supplementarybidi.inl"
};

static Direction Get_Supplementary_Direction(unsigned codepoint)
{
	unsigned first = 0;
	unsigned last = sizeof(SupplementaryDirectionRanges) / sizeof(SupplementaryDirectionRanges[0]);

	while (first < last) {
		const unsigned middle = first + (last - first) / 2;
		if (codepoint < SupplementaryDirectionRanges[middle].First) {
			last = middle;
		} else if (codepoint > SupplementaryDirectionRanges[middle].Last) {
			first = middle + 1;
		} else {
			return SupplementaryDirectionRanges[middle].Value;
		}
	}

	return LeftToRight;
}

static bool Is_Paragraph_Separator(WCHAR ch)
{
	return ch == L'\n' || ch == L'\r' || (ch >= 0x001C && ch <= 0x001E) ||
		ch == 0x0085 || ch == 0x2029;
}

}

namespace UnicodeBidi
{

WORD Get_Paragraph_Base_Level(const WCHAR *text, int length)
{
	int isolate_depth = 0;

	for (int index = 0; index < length; ++index) {
		const WCHAR ch = text[index];

		// First-strong detection applies to one bidi paragraph only.
		if (Is_Paragraph_Separator(ch)) {
			break;
		}

		if (ch >= 0x2066 && ch <= 0x2068) {
			++isolate_depth;
			continue;
		}
		if (ch == 0x2069) {
			if (isolate_depth > 0) {
				--isolate_depth;
			}
			continue;
		}

		// UAX #9 P2 excludes directional-isolate contents from the paragraph direction decision.
		if (isolate_depth > 0) {
			continue;
		}

		Direction direction = Neutral;
		if (ch >= 0xD800 && ch <= 0xDBFF) {
			if (index + 1 < length && text[index + 1] >= 0xDC00 && text[index + 1] <= 0xDFFF) {
				const unsigned codepoint = 0x10000 + ((ch - 0xD800) << 10) + (text[++index] - 0xDC00);
				direction = Get_Supplementary_Direction(codepoint);
			}
		} else if (ch < 0xDC00 || ch > 0xDFFF) {
			WORD type = C2_NOTAPPLICABLE;
			if (::GetStringTypeW(CT_CTYPE2, &ch, 1, &type)) {
				if (type == C2_LEFTTORIGHT) {
					direction = LeftToRight;
				} else if (type == C2_RIGHTTOLEFT) {
					direction = RightToLeft;
				}
			}
		}

		if (direction != Neutral) {
			return direction == RightToLeft ? 1 : 0;
		}
	}

	return 0;
}

}
