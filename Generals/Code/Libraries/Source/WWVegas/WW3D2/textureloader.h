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

#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "always.h"
#include "texture.h"

class StringClass;
struct IDirect3DTexture8;
class TextureLoadTaskClass;

class TextureLoader
{
	static void Init_Load_Task(TextureClass* tc);

	static bool Load_Uncompressed_Mipmap_Levels_From_TGA(TextureLoadTaskClass* texture);

public:
	static void Init();
	static void Deinit();

	// Modify given texture size to nearest valid size on current hardware.
	static void Validate_Texture_Size(unsigned& width, unsigned& height);

	// Adds a loading task to the system. The task if processed in a separate
	// thread as soon as possible. The task will appear in finished tasks list
	// when it's been completed. The texture will be refreshed on the next
	// update call after appearing to the finished tasks list.
	static void Add_Load_Task(TextureClass* tc);
	static IDirect3DTexture8* Load_Thumbnail(
		const StringClass& filename,
		WW3DFormat texture_format);	// Pass WW3D_FORMAT_UNKNOWN if you don't care
	static void Load_Mipmap_Levels(TextureLoadTaskClass* texture);

	static IDirect3DSurface8* Load_Surface_Immediate(
		const StringClass& filename,
		WW3DFormat surface_format,		// Pass WW3D_FORMAT_UNKNOWN if you don't care
		bool allow_compression);

	// Textures can only be created and locked by the main thread so this function sends a request to the texture
	// handling system to load the texture immediatelly next time it enters the main thread. If this function
	// is called from the main thread the texture is loaded immediatelly.
	static void Request_High_Priority_Loading(
		TextureClass* texture,
		MipCountType mip_level_count);
	static void	Request_Thumbnail(TextureClass* tc);

	static void Update();
	static void Flush_Pending_Load_Tasks();

	static IDirect3DTexture8* Generate_Bumpmap(TextureClass* texture);

};

// ----------------------------------------------------------------------------
//
// Texture loader task handler
//
// ----------------------------------------------------------------------------

class TextureLoadTaskClass : public W3DMPO
{
	W3DMPO_GLUE(TextureLoadTaskClass)

	static TextureLoadTaskClass* FreeTaskListHead;

	TextureClass* Texture;
	IDirect3DTexture8 *D3DTexture;
	unsigned Width;
	unsigned Height;
	WW3DFormat Format;
	unsigned char* LockedSurfacePtr[MIP_LEVELS_MAX];
	unsigned LockedSurfacePitch[MIP_LEVELS_MAX];
	unsigned MipLevelCount;
	unsigned Reduction;
	TextureLoadTaskClass* Succ;
	bool IsLoading;
	bool HasFailed;
	bool HighPriorityRequested;

	~TextureLoadTaskClass();
	TextureLoadTaskClass();
public:
	static TextureLoadTaskClass* Get_Instance(TextureClass* tc, bool high_priority);
	static void Release_Instance(TextureLoadTaskClass* task);
	static void shutdown(void) {TextureLoadTaskClass *pT; while (FreeTaskListHead) {pT = FreeTaskListHead; FreeTaskListHead = pT->Peek_Succ(); pT->Set_Succ(NULL); delete pT;} };

	void Init(TextureClass* tc,bool high_priority);
	void Deinit();

	unsigned Get_Mip_Level_Count() const { return MipLevelCount; }
	unsigned Get_Width() const { return Width; }
	unsigned Get_Height() const { return Height; }
	WW3DFormat Get_Format() const { return Format; }
	unsigned Get_Reduction() const { return Reduction; }

	unsigned char* Get_Locked_Surface_Ptr(unsigned level);
	unsigned Get_Locked_Surface_Pitch(unsigned level) const;

	bool Has_Failed() const { return HasFailed; }
	void Set_Fail(bool b) { HasFailed=b; }

	// Init the task or put it to a deferred init list if called from outside the main thread
	void Begin_Texture_Load();
	void Begin_Thumbnail_Load();
	void End_Load();		// Deinit must be called before Applying()

	void Apply(bool initialize);

	TextureLoadTaskClass* Peek_Succ() { return Succ; }
	void Set_Succ(TextureLoadTaskClass* succ);

	TextureClass* Peek_Texture() { return Texture; }
	IDirect3DTexture8* Peek_D3D_Texture() { return D3DTexture; }

	void Set_D3D_Texture(IDirect3DTexture8* texture);
};

#endif


