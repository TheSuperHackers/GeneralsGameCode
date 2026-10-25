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

#include "WW3D2/texture.h"
#include "WWLib/ref_ptr.h"
#include "WWLib/wwstring.h"
#include "WWMath/vector4.h"
#include <map>
#include <vector>

namespace TextureAtlas
{
constexpr const int PAGE_SIZE = 2048;
constexpr const int MIP_LEVEL_COUNT = 5;
constexpr const int PADDING = 1 << (MIP_LEVEL_COUNT - 1);
constexpr const int MAX_PAGES = 4;
constexpr const int MAX_PACKS_PER_FRAME = 2;    // Spreads a burst of new textures over several frames.
}    // namespace TextureAtlas

/// Packs textures into shared atlas pages.
/// Page: a 2048x2048 texture with 5 mip levels, holding many cells.
/// Cell: one texture plus its padding, a border of repeated edge texels that keeps filtering from reaching a neighbour.
/// Row: a strip across a page holding cells of the same height, left to right.
/// Premultiply: optionally multiplies a cell's color by its alpha.
class TextureAtlasClass
{
public:
	TextureAtlasClass()
	  : PackFrame(0)
	  , PacksThisFrame(0)
	{}

	struct TextureLocation
	{
		TextureClass* page;    // nullptr if the texture could not be packed.
		Vector4 uvRect;

		static TextureLocation Create_Invalid()
		{
			TextureLocation location;
			location.page = nullptr;
			location.uvRect.Set(0.0f, 0.0f, 1.0f, 1.0f);
			return location;
		}
	};

	TextureLocation Find_Or_Insert(TextureClass* source, bool premultiplyAlpha);    // Gets the page holding the texture and its uv rectangle, or a nullptr page if not found.
	void Reset();    // Release all pages.

	static int Wrap_Or_Clamp(int coordinate, int size, bool wrap);
	static void Fill_Padding(unsigned char* cell, int pitch, int width, int height, int padding, bool wrapU, bool wrapV);

private:
	struct Row
	{
		int y;
		int height;
		int usedWidth;
	};

	struct Page
	{
		RefCountPtr<TextureClass> texture;
		std::vector<Row> rows;
		int usedHeight;
	};

	typedef std::map<StringClass, TextureLocation> TextureLocationMap;

	bool Has_Pack_Budget();
	TextureLocation Pack_Texture(TextureClass* source, bool premultiplyAlpha);
	bool Allocate_Cell(int width, int height, int& pageIndex, int& x, int& y);
	bool Create_Page();
	static bool Copy_To_Cell(TextureClass* source, TextureClass* page, bool premultiplyAlpha, int left, int top);

	std::vector<Page> Pages;
	TextureLocationMap TextureLocations[2];    // Two keys for premultiply alpha off/on.
	unsigned PackFrame;
	int PacksThisFrame;
};
