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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/dx8vertexbuffer.h                      $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/09/01 8:15p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "wwdebug.h"
#include "refcount.h"
#include "dx8fvf.h"

const unsigned dynamic_fvf_type=D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2|D3DFVF_DIFFUSE;

class DX8Wrapper;
class SortingRendererClass;
class Vector2;
class Vector3;
class Vector4;
class StringClass;
class DX8VertexBufferClass;
class FVFInfoClass;
struct IDirect3DVertexBuffer8;
class VertexBufferClass;
struct VertexFormatXYZNDUV2;

class VertexBufferLockClass
{
protected:
	VertexBufferClass* VertexBuffer;
	void* Vertices;

	// This class can't be used directly, so constructor as to be protected
	VertexBufferLockClass(VertexBufferClass* vertex_buffer_) : VertexBuffer(vertex_buffer_) {}
public:
	void* Get_Vertex_Array() { return Vertices; }
};

/**
** DX8VertexBufferClass
** This class wraps a DX8 vertex buffer.  Use the lock objects to modify or append to the vertex buffer.
*/
class VertexBufferClass : public W3DMPO, public RefCountClass
{
	// nope, an ABC
	//W3DMPO_GLUE(VertexBufferClass)

protected:
	VertexBufferClass(unsigned type, unsigned FVF, unsigned short VertexCount);
	virtual ~VertexBufferClass();
public:

	inline const FVFInfoClass& FVF_Info() const { return *fvf_info; }
	inline unsigned short Get_Vertex_Count() const { return VertexCount; }
	inline unsigned Type() const { return type; }

	void Add_Engine_Ref() const;
	void Release_Engine_Ref() const;
	inline unsigned Engine_Refs() const { return engine_refs; }

	class WriteLockClass : public VertexBufferLockClass
	{
	public:
		WriteLockClass(VertexBufferClass* vertex_buffer, int flags=0);
		~WriteLockClass();
	};

	class AppendLockClass : public VertexBufferLockClass
	{
	public:
		AppendLockClass(VertexBufferClass* vertex_buffer,unsigned start_index, unsigned index_range);
		~AppendLockClass();
	};

	static unsigned Get_Total_Buffer_Count();
	static unsigned Get_Total_Allocated_Vertices();
	static unsigned Get_Total_Allocated_Memory();

protected:
	unsigned							type;
	unsigned short					VertexCount;
	mutable int						engine_refs;
	FVFInfoClass*					fvf_info;
};



/**
** Dynamic vertex buffer access is a wrapper to a single cycled dynamic vertex
** buffer.
** DynamicVBAccess gains an access to the dynamic vertex buffer and only
** only of these are allowed at any one time.
**
** The dynamic fvf buffers are always of the same type.
**
** NOTE: Dynamic vertex buffers accessors should only be used locally!
**
*/

class DynamicVBAccessClass
{
	friend DX8Wrapper;
	friend SortingRendererClass;

	const FVFInfoClass& FVFInfo;
	unsigned Type;
	unsigned short VertexCount;
	unsigned short VertexBufferOffset;
	VertexBufferClass* VertexBuffer;
//	static VertexFormatXYZNDUV2* _Get_Sorting_Vertex_Array();

	void Allocate_Sorting_Dynamic_Buffer();
	void Allocate_DX8_Dynamic_Buffer();
public:
	// Type parameter can be either BUFFER_TYPE_DYNAMIC_DX8 or BUFFER_TYPE_DYNAMIC_SORTING.

	// Note: Even though the constructor takes fvf as a parameter, currently the
	// only acceptable parameter is "dynamic_fvf_type". Any other type will
	// result to an assert.
	DynamicVBAccessClass(unsigned type,unsigned fvf,unsigned short vertex_count);
	~DynamicVBAccessClass();

	// Access fvf
	const FVFInfoClass& FVF_Info() const { return FVFInfo; }
	unsigned Get_Type() const { return Type; }
	unsigned short Get_Vertex_Count() const { return VertexCount; }

	// Call at the end of the execution, or at whatever time you wish to release
	// the recycled dynamic vertex buffer.
	static void _Deinit();
	static void _Reset(bool frame_changed);
	static unsigned short Get_Default_Vertex_Count(void);	///<current size of dynamic vertex buffer

	// To lock the vertex buffer, create instance of this write class locally.
	// The buffer is automatically unlocked when you exit the scope.
	class WriteLockClass// : public VertexBufferLockClass
	{
		DynamicVBAccessClass* DynamicVBAccess;
		VertexFormatXYZNDUV2 * Vertices;
	public:
		WriteLockClass(DynamicVBAccessClass* vb_access);
		~WriteLockClass();

		// Use this function to get a pointer to the first vertex you can write into.
		// If we ever change the format used by DynamicVBAccessClass, then the
		// return type of this function will change and we'll easily find all code
		// using it.
		VertexFormatXYZNDUV2 * Get_Formatted_Vertex_Array();
	};
	friend WriteLockClass;
};

// ----------------------------------------------------------------------------

inline VertexFormatXYZNDUV2 * DynamicVBAccessClass::WriteLockClass::Get_Formatted_Vertex_Array()
{
	// assert that the format of the dynamic vertex buffer is still what we think it is.
	WWASSERT(DynamicVBAccess->VertexBuffer->FVF_Info().Get_FVF() == (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2|D3DFVF_DIFFUSE));
	return Vertices;
}

// ----------------------------------------------------------------------------

/**
** DX8VertexBufferClass
** This class wraps a DX8 vertex buffer.  Use the lock objects to modify or append to the vertex buffer.
*/
class DX8VertexBufferClass : public VertexBufferClass
{
	W3DMPO_GLUE(DX8VertexBufferClass)
protected:
	~DX8VertexBufferClass();
public:
	enum UsageType {
		USAGE_DEFAULT=0,
		USAGE_DYNAMIC=1,
		USAGE_SOFTWAREPROCESSING=2,
		USAGE_NPATCHES=4
	};

	DX8VertexBufferClass(unsigned FVF, unsigned short VertexCount, UsageType usage=USAGE_DEFAULT);
	DX8VertexBufferClass(const Vector3* vertices, const Vector3* normals, const Vector2* tex_coords, unsigned short VertexCount,UsageType usage=USAGE_DEFAULT);
	DX8VertexBufferClass(const Vector3* vertices, const Vector3* normals, const Vector4* diffuse, const Vector2* tex_coords, unsigned short VertexCount,UsageType usage=USAGE_DEFAULT);
	DX8VertexBufferClass(const Vector3* vertices, const Vector4* diffuse, const Vector2* tex_coords, unsigned short VertexCount,UsageType usage=USAGE_DEFAULT);
	DX8VertexBufferClass(const Vector3* vertices, const Vector2* tex_coords, unsigned short VertexCount,UsageType usage=USAGE_DEFAULT);

	IDirect3DVertexBuffer8* Get_DX8_Vertex_Buffer() { return VertexBuffer; }

	void Copy(const Vector3* loc, unsigned first_vertex, unsigned count);
	void Copy(const Vector3* loc, const Vector2* uv, unsigned first_vertex, unsigned count);
	void Copy(const Vector3* loc, const Vector3* norm, unsigned first_vertex, unsigned count);
	void Copy(const Vector3* loc, const Vector3* norm, const Vector2* uv, unsigned first_vertex, unsigned count);
	void Copy(const Vector3* loc, const Vector3* norm, const Vector2* uv, const Vector4* diffuse, unsigned first_vertex, unsigned count);
	void Copy(const Vector3* loc, const Vector2* uv, const Vector4* diffuse, unsigned first_vertex, unsigned count);

protected:
	IDirect3DVertexBuffer8*		VertexBuffer;

	void Create_Vertex_Buffer(UsageType usage);
};


/**
** SortingVertexBufferClass
** This class acts as a vertex buffer for the vertices that need to be passed to alpha renderer.
*/
class SortingVertexBufferClass : public VertexBufferClass
{
	W3DMPO_GLUE(SortingVertexBufferClass)

	friend DX8Wrapper;
	friend SortingRendererClass;
	friend VertexBufferClass::WriteLockClass;
	friend VertexBufferClass::AppendLockClass;
	friend DynamicVBAccessClass::WriteLockClass;

	VertexFormatXYZNDUV2* VertexBuffer;

protected:
	~SortingVertexBufferClass();
public:
	SortingVertexBufferClass(unsigned short VertexCount);
};
