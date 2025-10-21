/*
**	Command & Conquer Generals Zero Hour(tm)
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

// 08/07/02 KM Texture class redesign (revisited)
#ifndef TEXTURETHUMBNAIL_H
#define TEXTURETHUMBNAIL_H

#pragma once

#include "always.h"
#include "wwstring.h"
#include "hashtemplate.h"
#include "dllist.h"
#include "ww3dformat.h"

#define GLOBAL_THUMBNAIL_MANAGER_FILENAME "global.th6"

class ThumbnailManagerClass;

// ----------------------------------------------------------------------------

class ThumbnailClass
{
	friend ThumbnailManagerClass;

	StringClass Name;
	unsigned char* Bitmap;
	unsigned Width;
	unsigned Height;
	unsigned OriginalTextureWidth;
	unsigned OriginalTextureHeight;
	unsigned OriginalTextureMipLevelCount;
	WW3DFormat OriginalTextureFormat;
	unsigned long DateTime;
	bool Allocated;	// if true, destructor will free the memory
	ThumbnailManagerClass* Manager;

	ThumbnailClass(
		ThumbnailManagerClass* manager,
		const char* name,
		unsigned char* bitmap,
		unsigned w,
		unsigned h,
		unsigned original_w,
		unsigned original_h,
		unsigned original_mip_level_count,
		WW3DFormat original_format,
		bool allocated,
		unsigned long date_time);
	ThumbnailClass(
		ThumbnailManagerClass* manager,
		const StringClass& filename);
	~ThumbnailClass();
public:

	unsigned char* Peek_Bitmap() { return Bitmap; }
	WW3DFormat Get_Format() { return WW3D_FORMAT_A4R4G4B4; }
	unsigned Get_Width() const { return Width; }
	unsigned Get_Height() const { return Height; }
	unsigned Get_Original_Texture_Width() const { return OriginalTextureWidth; }
	unsigned Get_Original_Texture_Height() const { return OriginalTextureHeight; }
	unsigned Get_Original_Texture_Mip_Level_Count() const { return OriginalTextureMipLevelCount; }
	WW3DFormat Get_Original_Texture_Format() const { return OriginalTextureFormat; }
	unsigned long Get_Date_Time() const { return DateTime; }
	const StringClass& Get_Name() const { return Name; }

};

// ----------------------------------------------------------------------------

class ThumbnailManagerClass : public DLNodeClass<ThumbnailManagerClass>
{
	W3DMPO_GLUE(ThumbnailManagerClass);

	friend ThumbnailClass;

	static bool CreateThumbnailIfNotFound;
	bool PerTextureTimeStampUsed;
	StringClass ThumbnailFileName;
	HashTemplateClass<StringClass,ThumbnailClass*> ThumbnailHash;
	unsigned char* ThumbnailMemory;
	bool Changed;
	unsigned long DateTime;

	ThumbnailManagerClass(const char* thumbnail_filename);
	~ThumbnailManagerClass();

	void Remove_From_Hash(ThumbnailClass* thumb);
	void Insert_To_Hash(ThumbnailClass* thumb);
	ThumbnailClass* Get_From_Hash(const StringClass& name);

public:
	ThumbnailClass* Peek_Thumbnail_Instance(const StringClass& name);

	static void Add_Thumbnail_Manager(const char* thumbnail_filename);
	static void Remove_Thumbnail_Manager(const char* thumbnail_filename);
	static ThumbnailManagerClass* Peek_Thumbnail_Manager(const char* thumbnail_filename);

	static ThumbnailClass* Peek_Thumbnail_Instance_From_Any_Manager(const StringClass& name);

	static bool Is_Thumbnail_Created_If_Not_Found() { return CreateThumbnailIfNotFound; }
	static void Create_Thumbnail_If_Not_Found(bool create) { CreateThumbnailIfNotFound=create; }

	bool Is_Per_Texture_Time_Stamp_Used() const { return PerTextureTimeStampUsed; }
	void Enable_Per_Texture_Time_Stamp(bool enable) { PerTextureTimeStampUsed=enable; }

	static void Init();
	static void Deinit();
};

// ----------------------------------------------------------------------------

#endif
