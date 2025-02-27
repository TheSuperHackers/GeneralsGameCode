/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
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

#include "always.h"
#include "wwstring.h"

// ----------------------------------------------------------------------------

class ThumbnailClass
{
	StringClass Name;
	unsigned char* Bitmap;
	unsigned Width;
	unsigned Height;
	bool Allocated;	// if true, destructor will free the memory
public:
	ThumbnailClass(const char* name, unsigned char* bitmap, unsigned w, unsigned h, bool allocated);
	ThumbnailClass(const StringClass& filename);
	~ThumbnailClass();

	unsigned char* Peek_Bitmap() { return Bitmap; }
	unsigned Get_Width() const { return Width; }
	unsigned Get_Height() const { return Height; }
	const StringClass& Get_Name() const { return Name; }

	static ThumbnailClass* Peek_Instance(const StringClass& name);
	static void Init();
	static void Deinit();
};

// ----------------------------------------------------------------------------
