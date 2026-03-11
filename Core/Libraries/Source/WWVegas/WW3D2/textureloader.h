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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : DX8 Texture Manager                                          *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/textureloader.h                            $*
 *                                                                                             *
 *              Original Author:: vss_sync                                                   *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/27/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * 06/27/02 KM Texture class abstraction																			*
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "texture.h"

class StringClass;
struct IDirect3DTexture8;
class TextureLoadTaskClass;
class TextureLoadTaskListClass;

class TextureLoader
{
public:
	static void Init();
	static void Deinit();

	// Modify given texture size to nearest valid size on current hardware.
	static void Validate_Texture_Size(unsigned& width, unsigned& height, unsigned& depth);

	static IDirect3DTexture8 * Load_Thumbnail(
		const StringClass& filename,const Vector3& hsv_shift);
//		WW3DFormat texture_format);	// Pass WW3D_FORMAT_UNKNOWN if you don't care

	static IDirect3DSurface8 *		Load_Surface_Immediate(
		const StringClass& filename,
		WW3DFormat surface_format,		// Pass WW3D_FORMAT_UNKNOWN if you don't care
		bool allow_compression);

	static void	Request_Thumbnail(TextureBaseClass* tc);

	// Adds a loading task to the system. The task if processed in a separate
	// thread as soon as possible. The task will appear in finished tasks list
	// when it's been completed. The texture will be refreshed on the next
	// update call after appearing to the finished tasks list.
	static void Request_Background_Loading(TextureBaseClass* tc);

	// Textures can only be created and locked by the main thread so this function sends a request to the texture
	// handling system to load the texture immediatelly next time it enters the main thread. If this function
	// is called from the main thread the texture is loaded immediatelly.
	static void Request_Foreground_Loading(TextureBaseClass* tc);

	static void	Flush_Pending_Load_Tasks();
	static void Update(void(*network_callback)() = nullptr);

	// returns true if current thread of execution is allowed to make DX8 calls.
	static bool Is_DX8_Thread();

	static void Suspend_Texture_Load();
	static void Continue_Texture_Load();

	static void Set_Texture_Inactive_Override_Time(int time_ms) {TextureInactiveOverrideTime = time_ms;}

private:
	static void Process_Foreground_Load			(TextureLoadTaskClass *task);
	static void Process_Foreground_Thumbnail	(TextureLoadTaskClass *task);

	static void Begin_Load_And_Queue				(TextureLoadTaskClass *task);
	static void Load_Thumbnail						(TextureBaseClass *tc);

	static bool TextureLoadSuspended;

	// The time in ms before a texture is thrown out.
	// The default is zero.  The scripted movies set this to reduce texture stalls in movies.
	static int	TextureInactiveOverrideTime;
};

class TextureLoadTaskListNodeClass
{
	friend class TextureLoadTaskListClass;

	public:
		TextureLoadTaskListNodeClass() : Next(0), Prev(0) { }

		TextureLoadTaskListClass *Get_List()		{ return List; }

		TextureLoadTaskListNodeClass *Next;
		TextureLoadTaskListNodeClass *Prev;
		TextureLoadTaskListClass *		List;
};


class TextureLoadTaskListClass
{
	// This class implements an unsynchronized, double-linked list of TextureLoadTaskClass
	// objects, using an embedded list node.

	public:
		TextureLoadTaskListClass();

		// Returns true if list is empty, false otherwise.
		bool									Is_Empty		() const		{ return (Root.Next == &Root); }

		// Add a task to beginning of list
		void									Push_Front	(TextureLoadTaskClass *task);

		// Add a task to end of list
		void									Push_Back	(TextureLoadTaskClass *task);

		// Remove and return a task from beginning of list, or null if list is empty.
		TextureLoadTaskClass *			Pop_Front	();

		// Remove and return a task from end of list, or null if list is empty
		TextureLoadTaskClass *			Pop_Back		();

		// Remove specified task from list, if present
		void									Remove		(TextureLoadTaskClass *task);

	private:
		// This list is implemented using a sentinel node.
		TextureLoadTaskListNodeClass	Root;
};


class SynchronizedTextureLoadTaskListClass : public TextureLoadTaskListClass
{
	// This class added thread-safety to the basic TextureLoadTaskListClass.

	public:
		SynchronizedTextureLoadTaskListClass();

		// See comments above for description of member functions.
		void									Push_Front	(TextureLoadTaskClass *task);
		void									Push_Back	(TextureLoadTaskClass *task);
		TextureLoadTaskClass *			Pop_Front	();
		TextureLoadTaskClass *			Pop_Back		();
		void									Remove		(TextureLoadTaskClass *task);

	private:
		FastCriticalSectionClass		CriticalSection;
};

/*
** (gth) The allocation system we're using for TextureLoadTaskClass has gotten a little
** complicated since Kenny added the new task types for Cube and Volume textures.  The
** ::Destroy member is used to return a task to the pool now and must be over-ridden in
** each derived class to put the task back into the correct free list.
*/


class TextureLoadTaskClass : public TextureLoadTaskListNodeClass
{
	public:
		enum TaskType {
			TASK_NONE,
			TASK_THUMBNAIL,
			TASK_LOAD,
		};

		enum PriorityType {
			PRIORITY_LOW,
			PRIORITY_HIGH,
		};

		enum StateType {
			STATE_NONE,

			STATE_LOAD_BEGUN,
			STATE_LOAD_MIPMAP,
			STATE_LOAD_COMPLETE,

			STATE_COMPLETE,
		};


		TextureLoadTaskClass();
		~TextureLoadTaskClass();

		static TextureLoadTaskClass *	Create			(TextureBaseClass *tc, TaskType type, PriorityType priority);
		static void				Delete_Free_Pool			();

		virtual void			Destroy						();
		virtual void			Init							(TextureBaseClass *tc, TaskType type, PriorityType priority);
		virtual void			Deinit						();

		TaskType					Get_Type						() const		{ return Type;				}
		PriorityType			Get_Priority				() const		{ return Priority;		}
		StateType				Get_State					() const		{ return State;			}

		WW3DFormat				Get_Format					() const		{ return Format;			}
		unsigned int			Get_Width					() const		{ return Width;			}
		unsigned int			Get_Height					() const		{ return Height;			}
		unsigned int			Get_Mip_Level_Count		() const		{ return MipLevelCount; }
		unsigned int			Get_Reduction				() const		{ return Reduction;		}

		unsigned char *		Get_Locked_Surface_Ptr	(unsigned int level);
		unsigned int			Get_Locked_Surface_Pitch(unsigned int level) const;

		TextureBaseClass *	Peek_Texture				()				{ return Texture;			}
		IDirect3DTexture8	*	Peek_D3D_Texture			()				{ return (IDirect3DTexture8*)D3DTexture;		}

		void						Set_Type						(TaskType t)		{ Type		= t;			}
		void						Set_Priority				(PriorityType p)	{ Priority	= p;			}
		void						Set_State					(StateType s)		{ State		= s;			}

		bool						Begin_Load					();
		bool						Load							();
		void						End_Load						();
		void						Finish_Load					();
		void						Apply_Missing_Texture	();

	protected:
		virtual bool			Begin_Compressed_Load	();
		virtual bool			Begin_Uncompressed_Load	();

		virtual bool			Load_Compressed_Mipmap	();
		virtual bool			Load_Uncompressed_Mipmap();

		virtual void			Lock_Surfaces				();
		virtual void			Unlock_Surfaces			();

		void						Apply							(bool initialize);

		TextureBaseClass*		Texture;
		IDirect3DBaseTexture8*	D3DTexture;
		WW3DFormat				Format;

		unsigned int			Width;
		unsigned	int			Height;
		unsigned	int			MipLevelCount;
		unsigned	int			Reduction;
		Vector3					HSVShift;

		unsigned char *		LockedSurfacePtr[MIP_LEVELS_MAX];
		unsigned	int			LockedSurfacePitch[MIP_LEVELS_MAX];

		TaskType					Type;
		PriorityType			Priority;
		StateType				State;
};

class CubeTextureLoadTaskClass : public TextureLoadTaskClass
{
public:
	CubeTextureLoadTaskClass();

	virtual void			Destroy						();
	virtual void			Init							(TextureBaseClass *tc, TaskType type, PriorityType priority);
	virtual void			Deinit						();

protected:
	virtual bool			Begin_Compressed_Load	();
	virtual bool			Begin_Uncompressed_Load	();

	virtual bool			Load_Compressed_Mipmap	();
//	virtual bool			Load_Uncompressed_Mipmap();

	virtual void			Lock_Surfaces				();
	virtual void			Unlock_Surfaces			();

private:
	unsigned char*			Get_Locked_CubeMap_Surface_Pointer(unsigned int face, unsigned int level);
	unsigned int			Get_Locked_CubeMap_Surface_Pitch(unsigned int face, unsigned int level) const;

	IDirect3DCubeTexture8*	Peek_D3D_Cube_Texture()				{ return (IDirect3DCubeTexture8*)D3DTexture;		}

	unsigned char*			LockedCubeSurfacePtr[6][MIP_LEVELS_MAX];
	unsigned int			LockedCubeSurfacePitch[6][MIP_LEVELS_MAX];
};

class VolumeTextureLoadTaskClass : public TextureLoadTaskClass
{
public:
	VolumeTextureLoadTaskClass();

	virtual void			Destroy						();
	virtual void			Init							(TextureBaseClass *tc, TaskType type, PriorityType priority);

protected:
	virtual bool			Begin_Compressed_Load	();
	virtual bool			Begin_Uncompressed_Load	();

	virtual bool			Load_Compressed_Mipmap	();
//	virtual bool			Load_Uncompressed_Mipmap();

	virtual void			Lock_Surfaces				();
	virtual void			Unlock_Surfaces			();

private:
	unsigned char*			Get_Locked_Volume_Pointer(unsigned int level);
	unsigned int			Get_Locked_Volume_Row_Pitch(unsigned int level);
	unsigned int			Get_Locked_Volume_Slice_Pitch(unsigned int level);

	IDirect3DVolumeTexture8*	Peek_D3D_Volume_Texture()				{ return (IDirect3DVolumeTexture8*)D3DTexture;		}

	unsigned	int			LockedSurfaceSlicePitch[MIP_LEVELS_MAX];

	unsigned int		Depth;
};
