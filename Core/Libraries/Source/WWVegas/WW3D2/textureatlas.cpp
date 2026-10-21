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

#include "WW3D2/textureatlas.h"
#include "WW3D2/surfaceclass.h"
#include "WW3D2/textureloader.h"
#include "WWMath/vector2i.h"
#include "WWMath/wwmath.h"
#include "WW3D2/ww3d.h"
#include "Utility/premultiply_alpha.h"
#include <d3d8.h>
#include <string.h>

enum
{
	PREMULTIPLY_OFF,
	PREMULTIPLY_ON
};

void TextureAtlasClass::Reset()
{
	TextureLocations[PREMULTIPLY_OFF].clear();
	TextureLocations[PREMULTIPLY_ON].clear();
	Pages.clear();
}

TextureClass* TextureAtlasClass::Find_Or_Insert(TextureClass* source, const bool premultiply, Vector4& rect)
{
	// Skip textures without name as it is required for indexing.
	if (source == nullptr || source->Get_Texture_Name().Is_Empty())
	{
		return nullptr;
	}

	TextureLocationMap& locations = TextureLocations[premultiply ? PREMULTIPLY_ON : PREMULTIPLY_OFF];
	TextureLocationMap::iterator it = locations.find(source->Get_Texture_Name());
	if (it == locations.end())
	{
		// To prevent lag spikes, limit the number of textures that can be inserted (packed) per frame.
		if (!Has_Pack_Budget())
		{
			return nullptr;
		}

		if (!source->Is_Initialized())
		{
			TextureLoader::Request_Foreground_Loading(source);
		}

		// Try to add a texture. If rejected, will be marked invalid, such that it will not get a retry later.
		TextureLocation packedTexture = Pack_Texture(source, premultiply);
		it = locations.insert(std::make_pair(source->Get_Texture_Name(), packedTexture)).first;
		++PacksThisFrame;
	}

	// Packing failed: unsupported size, no room left, or a page could not be created or written.
	if (it->second.pageIndex == TextureAtlas::INVALID_PAGE)
	{
		return nullptr;
	}

	rect = it->second.uvRect;
	return Pages[it->second.pageIndex].texture.Peek();
}

TextureAtlasClass::TextureLocation TextureAtlasClass::Pack_Texture(TextureClass* source, const bool premultiply)
{
	// A texture can be initialized without a D3D texture, e.g. with texturing disabled.
	if (source->Peek_D3D_Texture() == nullptr)
	{
		return TextureLocation::Invalid();
	}

	const int width = source->Get_Width();
	const int height = source->Get_Height();
	const int cellWidth = width + 2 * TextureAtlas::PADDING;
	const int cellHeight = height + 2 * TextureAtlas::PADDING;

	// Check size constraints.
	const bool powerOfTwo = WWMath::Is_Power_Of_2(width) && WWMath::Is_Power_Of_2(height);
	const bool largeEnough = width >= TextureAtlas::PADDING && height >= TextureAtlas::PADDING;
	const bool fitsOnPage = cellWidth <= TextureAtlas::PAGE_SIZE && cellHeight <= TextureAtlas::PAGE_SIZE;
	if (!powerOfTwo || !largeEnough || !fitsOnPage)
	{
		return TextureLocation::Invalid();
	}

	// Reserve space in the atlas, then write the new texture to it.
	int pageIndex = 0;
	int x = 0;
	int y = 0;
	if (!Allocate_Cell(cellWidth, cellHeight, pageIndex, x, y) ||
	    !Copy_To_Cell(source, Pages[pageIndex].texture.Peek(), premultiply, x + TextureAtlas::PADDING, y + TextureAtlas::PADDING))
	{
		return TextureLocation::Invalid();
	}

	TextureLocation location;
	constexpr const float scale = 1.0f / TextureAtlas::PAGE_SIZE;
	location.pageIndex = pageIndex;
	location.uvRect.Set((x + TextureAtlas::PADDING) * scale,
	                    (y + TextureAtlas::PADDING) * scale,
	                    (x + TextureAtlas::PADDING + width) * scale,
	                    (y + TextureAtlas::PADDING + height) * scale);
	return location;
}

bool TextureAtlasClass::Allocate_Cell(const int width, const int height, int& pageIndex, int& x, int& y)
{
	// Use the first row of exactly this height that still has room.
	for (int p = 0; p < Pages.size(); ++p)
	{
		for (int r = 0; r < Pages[p].rows.size(); ++r)
		{
			Row& row = Pages[p].rows[r];
			if (row.height == height && row.usedWidth + width <= TextureAtlas::PAGE_SIZE)
			{
				pageIndex = p;
				x = row.usedWidth;
				y = row.y;
				row.usedWidth += width;
				return true;
			}
		}
	}

	// Otherwise find the first page with enough height left.
	int newRowPage;
	for (newRowPage = 0; newRowPage < Pages.size(); ++newRowPage)
	{
		if (Pages[newRowPage].usedHeight + height <= TextureAtlas::PAGE_SIZE)
		{
			break;
		}
	}

	// The new cell does not fit, so try to create a new page.
	if (newRowPage == Pages.size() && !Create_Page())
	{
		return false;
	}

	// Start a new row with this cell at its left edge.
	Page& page = Pages[newRowPage];
	const Row added = { page.usedHeight, height, width };
	page.rows.push_back(added);
	page.usedHeight += height;

	pageIndex = newRowPage;
	x = 0;
	y = added.y;

	return true;
}

bool TextureAtlasClass::Create_Page()
{
	if (Pages.size() >= TextureAtlas::MAX_PAGES)
	{
		return false;
	}

	Page page;
	page.texture.Assign_No_Add_Ref(NEW_REF(TextureClass, (TextureAtlas::PAGE_SIZE, TextureAtlas::PAGE_SIZE, WW3D_FORMAT_A8R8G8B8, (MipCountType)TextureAtlas::MIP_LEVEL_COUNT)));
	page.usedHeight = 0;

	// The device may give a different format, size or mip count than asked for. In that case a page cannot be created.
	if (page.texture->Peek_D3D_Texture() == nullptr || page.texture->Peek_D3D_Texture()->GetLevelCount() != TextureAtlas::MIP_LEVEL_COUNT)
	{
		return false;
	}

	SurfaceClass::SurfaceDescription desc;
	page.texture->Get_Level_Description(desc, 0);
	if (desc.Format != WW3D_FORMAT_A8R8G8B8 || desc.Width != TextureAtlas::PAGE_SIZE || desc.Height != TextureAtlas::PAGE_SIZE)
	{
		return false;
	}

	// Bilinear or trilinear filtering only: anisotropic filtering samples further than the padding reaches.
	// Clamp addressing, so samples at the page edge never wrap around to the opposite side.
	TextureFilterClass& filter = page.texture->Get_Filter();
	filter.Set_Min_Filter(TextureFilterClass::FILTER_TYPE_FAST);
	filter.Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_FAST);
	filter.Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_DEFAULT);
	filter.Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	filter.Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);

	Pages.push_back(page);
	return true;
}

bool TextureAtlasClass::Copy_To_Cell(TextureClass* source, TextureClass* page, const bool premultiply, const int left, const int top)
{
	const int sourceLevels = (int)source->Peek_D3D_Texture()->GetLevelCount();

	// WW3D stores the address mode on the texture and applies it whenever the texture is drawn.
	// Padding will use that same filtering as in normal rendering.
	const bool wrapU = source->Get_Filter().Get_U_Addr_Mode() == TextureFilterClass::TEXTURE_ADDRESS_REPEAT;
	const bool wrapV = source->Get_Filter().Get_V_Addr_Mode() == TextureFilterClass::TEXTURE_ADDRESS_REPEAT;

	for (int level = 0; level < TextureAtlas::MIP_LEVEL_COUNT; ++level)
	{
		const int padding = TextureAtlas::PADDING >> level;
		const int width = source->Get_Width() >> level;
		const int height = source->Get_Height() >> level;
		const int x = left >> level;
		const int y = top >> level;

		// If the source has this mip level, copy it, converting to A8R8G8B8. Using the shipped mips keeps sprites looking
		// as they did. If the source has fewer levels than the page, scale down this cell's level above instead.
		RefCountPtr<SurfaceClass> destinationSurface;
		destinationSurface.Assign_No_Add_Ref(page->Get_Surface_Level(level));
		RefCountPtr<SurfaceClass> sourceSurface;
		if (level < sourceLevels)
		{
			sourceSurface.Assign_No_Add_Ref(source->Get_Surface_Level(level));
			destinationSurface->Copy(x, y, 0, 0, width, height, sourceSurface.Peek());
		}
		else
		{
			sourceSurface.Assign_No_Add_Ref(page->Get_Surface_Level(level - 1));
			destinationSurface->Stretch_Copy(x, y, width, height, x * 2, y * 2, width * 2, height * 2, sourceSurface.Peek());
		}

		// Lock the cell, padding included. Locking the top level also marks the cell for upload at every level.
		int pitch = 0;
		unsigned char* cell = (unsigned char*)destinationSurface->Lock(&pitch, Vector2i(x - padding, y - padding), Vector2i(x + width + padding, y + height + padding));
		if (cell == nullptr)
		{
			return false;
		}

		// Premultiply the copied texels. Filtered levels come from the level above, which is already premultiplied.
		if (level < sourceLevels && premultiply)
		{
			premultiply_alpha(cell + padding * pitch + padding * sizeof(unsigned), pitch, width, height);
		}

		// Fill the padding from the texture's edges.
		Fill_Padding(cell, pitch, width, height, padding, wrapU, wrapV);
		destinationSurface->Unlock();
	}
	return true;
}

bool TextureAtlasClass::Has_Pack_Budget()
{
	if (PackFrame != WW3D::Get_Frame_Count())
	{
		PackFrame = WW3D::Get_Frame_Count();
		PacksThisFrame = 0;
	}
	return PacksThisFrame < TextureAtlas::MAX_PACKS_PER_FRAME;
}

int TextureAtlasClass::Wrap_Or_Clamp(const int coordinate, const int size, const bool wrap)
{
	if (wrap)
	{
		return coordinate & (size - 1);
	}
	return WWMath::Clamp_Int(coordinate, 0, size - 1);
}

void TextureAtlasClass::Fill_Padding(unsigned char* cell, const int pitch, const int width, const int height, const int padding, const bool wrapU, const bool wrapV)
{
	for (int y = padding; y < padding + height; ++y)
	{
		unsigned* line = (unsigned*)(cell + y * pitch);
		for (int x = 0; x < padding; ++x)
		{
			line[x] = line[padding + Wrap_Or_Clamp(x - padding, width, wrapU)];
			line[padding + width + x] = line[padding + Wrap_Or_Clamp(width + x, width, wrapU)];
		}
	}

	const int lineBytes = (width + 2 * padding) * sizeof(unsigned);
	for (int row = 0; row < padding; ++row)
	{
		memcpy(cell + row * pitch, cell + (padding + Wrap_Or_Clamp(row - padding, height, wrapV)) * pitch, lineBytes);
		memcpy(cell + (padding + height + row) * pitch, cell + (padding + Wrap_Or_Clamp(height + row, height, wrapV)) * pitch, lineBytes);
	}
}
