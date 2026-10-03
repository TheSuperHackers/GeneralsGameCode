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

static const DirectionRange DirectionRanges[] = {
#include "bididirection.inl"
};

static Direction Get_Direction(unsigned codepoint)
{
	unsigned first = 0;
	unsigned last = sizeof(DirectionRanges) / sizeof(DirectionRanges[0]);

	while (first < last) {
		const unsigned middle = first + (last - first) / 2;
		if (codepoint < DirectionRanges[middle].First) {
			last = middle;
		} else if (codepoint > DirectionRanges[middle].Last) {
			first = middle + 1;
		} else {
			return DirectionRanges[middle].Value;
		}
	}

	return LeftToRight;
}

static bool Is_Paragraph_Separator(unsigned ch)
{
	return ch == L'\n' || ch == L'\r' || (ch >= 0x001C && ch <= 0x001E) ||
		ch == 0x0085 || ch == 0x2029;
}

}

namespace UnicodeBidi
{

unsigned short Get_Paragraph_Base_Level(const wchar_t *text, int length)
{
	int isolate_depth = 0;

	for (int index = 0; index < length; ++index) {
		unsigned codepoint = static_cast<unsigned>(text[index]);

		// First-strong detection applies to one bidi paragraph only.
		if (Is_Paragraph_Separator(codepoint)) {
			break;
		}

		if (codepoint >= 0x2066 && codepoint <= 0x2068) {
			++isolate_depth;
			continue;
		}
		if (codepoint == 0x2069) {
			if (isolate_depth > 0) {
				--isolate_depth;
			}
			continue;
		}

		// UAX #9 P2 excludes directional-isolate contents from the paragraph direction decision.
		if (isolate_depth > 0) {
			continue;
		}

		// Decode a UTF-16 pair before classification; lone surrogates are ignored.
		if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
			if (index + 1 < length && text[index + 1] >= 0xDC00 && text[index + 1] <= 0xDFFF) {
				const unsigned high = codepoint - 0xD800;
				const unsigned low = static_cast<unsigned>(text[++index]) - 0xDC00;
				codepoint = 0x10000 + (high << 10) + low;
			} else {
				continue;
			}
		} else if ((codepoint >= 0xDC00 && codepoint <= 0xDFFF) || codepoint > 0x10FFFF) {
			continue;
		}

		const Direction direction = Get_Direction(codepoint);
		if (direction != Neutral) {
			return direction == RightToLeft ? 1 : 0;
		}
	}

	return 0;
}

}
