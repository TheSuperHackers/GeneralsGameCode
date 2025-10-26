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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DBufferManager.h ///////////////////////////////////////////////////////////////////////////
// Author: Mark Wilczynski
// Desc:   A system for managing partial vertex buffer allocations.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lib/BaseType.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"

// TheSuperHackers @info The max sizes correspond to a number of vertices and indices of a mesh casting shadows
// in the scene. The more vertices the meshes are supposed to have, the higher these limits need to be.
// Multiplying these numbers by MIN_SLOT_SIZE gives the max supported amount of vertices and indices.
#define MAX_VB_SIZES 128 //number of different sized VB slots allowed.
#define MAX_IB_SIZES 128 //number of different sized IB slots allowed (goes all the way up to 65536)

#define MIN_SLOT_SIZE_SHIFT 5 //used for division by MIN_SLOT_SIZE
#define MIN_SLOT_SIZE (1 << MIN_SLOT_SIZE_SHIFT) //minimum number of vertices allocated per slot (power of 2).

// TheSuperHackers @info The number of slots corresponds to number of meshes casting shadows in the scene.
// The more meshes there are, the larger the slots needs to be.
// TheSuperHackers @tweak Double the original sizes to allow for twice as many shadows to be created in the scene.
#define SLOTS_MULTIPLIER 2
#define MAX_NUMBER_SLOTS (4096 * SLOTS_MULTIPLIER) //maximum number of slots that can be allocated.
#define MAX_VERTEX_BUFFERS_CREATED (32 * SLOTS_MULTIPLIER) //maximum number of D3D vertex buffers allowed to create per vertex type.
#define MAX_INDEX_BUFFERS_CREATED (32 * SLOTS_MULTIPLIER)

#define DEFAULT_VERTEX_BUFFER_SIZE 8192 //this size ends up generating VB's of about 256Kbytes
#define DEFAULT_INDEX_BUFFER_SIZE 32768

class W3DBufferManager
{
public:
	//List of all possible vertex formats available to the game.
	enum VBM_FVF_TYPES
	{
		VBM_FVF_XYZ,		//position
		VBM_FVF_XYZD,		//position, diffuse
		VBM_FVF_XYZUV,		//position, uv
		VBM_FVF_XYZDUV,		//position, diffuse, uv
		VBM_FVF_XYZUV2,		//position, uv1, uv2
		VBM_FVF_XYZDUV2,	//position, diffuse, uv1, uv2
		VBM_FVF_XYZN,		//position, normal
		VBM_FVF_XYZND,		//position, normal, diffuse
		VBM_FVF_XYZNUV,		//position, normal, uv
		VBM_FVF_XYZNDUV,	//position, normal, diffuse, uv
		VBM_FVF_XYZNUV2,	//position, normal, uv1, uv2
		VBM_FVF_XYZNDUV2,	//position, normal, diffuse, uv1, uv2
		VBM_FVF_XYZRHW,		//transformed position
		VBM_FVF_XYZRHWD,	//transformed position, diffuse
		VBM_FVF_XYZRHWUV,	//transformed position, uv
		VBM_FVF_XYZRHWDUV,	//transformed position, diffuse, uv
		VBM_FVF_XYZRHWUV2,	//transformed position, uv1, uv2
		VBM_FVF_XYZRHWDUV2,	//transformed position, diffuse, uv1, uv2
		MAX_FVF
	};

	struct W3DRenderTask
	{
		W3DRenderTask	*m_nextTask;	///<next rendering task
	};

	struct W3DVertexBuffer;	//forward reference
	struct W3DIndexBuffer; //forward reference

	struct W3DVertexBufferSlot
	{
		Int m_size;								///<number of vertices in slot
		Int m_start;							///<index of first vertex within VB
		W3DVertexBuffer *m_VB;		///<vertex buffer holding this slot.
		W3DVertexBufferSlot *m_prevSameSize;		//previous slot of equal size.
		W3DVertexBufferSlot *m_nextSameSize;		//next slot of equal size.
		W3DVertexBufferSlot *m_prevSameVB;			//previous slot in same VB.
		W3DVertexBufferSlot *m_nextSameVB;			//next slot in same VB.
	};

	struct W3DVertexBuffer
	{
		VBM_FVF_TYPES	m_format;			///<format of vertices in this VB.
		W3DVertexBufferSlot *m_usedSlots;	///<slots inside this vertex buffer being used.
		Int	m_startFreeIndex;				///<index of vertex at start of unallocated memory.
		Int m_size;							///<number of vertices allowed in VB.
		W3DVertexBuffer *m_nextVB;			///<next vertex buffer of same type.
		DX8VertexBufferClass *m_DX8VertexBuffer;	///<actual DX8 vertex buffer interface
		W3DRenderTask	*m_renderTaskList;	///<used to help app sort its D3D access by VB.
	};

	struct W3DIndexBufferSlot
	{
		Int m_size;								///<number of vertices in slot
		Int m_start;							///<index of first index within VB
		W3DIndexBuffer *m_IB;		///<index buffer holding this slot.
		W3DIndexBufferSlot *m_prevSameSize;		//previous slot of equal size.
		W3DIndexBufferSlot *m_nextSameSize;		//next slot of equal size.
		W3DIndexBufferSlot *m_prevSameIB;			//previous slot in same VB.
		W3DIndexBufferSlot *m_nextSameIB;			//next slot in same VB.
	};

	struct W3DIndexBuffer
	{
		W3DIndexBufferSlot *m_usedSlots;	///<slots inside this index buffer being used.
		Int	m_startFreeIndex;				///<index of index at start of unallocated memory.
		Int m_size;							///<number of vertices allowed in VB.
		W3DIndexBuffer *m_nextIB;			///<next index buffer of same type.
		DX8IndexBufferClass *m_DX8IndexBuffer;	///<actual DX8 index buffer interface
	};

	W3DBufferManager(void);
	~W3DBufferManager(void);

	///return free vertex buffer memory slot.
	W3DVertexBufferSlot *getSlot(VBM_FVF_TYPES fvfType, Int size);
	///return free index buffer memory slot.
	W3DIndexBufferSlot *getSlot(Int size);
	void releaseSlot(W3DVertexBufferSlot *vbSlot);	///<return slot to pool
	void releaseSlot(W3DIndexBufferSlot *vbSlot);	///<return slot to pool
	void freeAllSlots(void);	///<release all slots to pool.
	void freeAllBuffers(void);	///<release all vertex buffers to pool.
	void ReleaseResources(void);	///<release D3D/W3D resources.
	Bool ReAcquireResources(void);	///<reaquire D3D/W3D resources.
	///allows iterating over vertex buffers used by manager.  Input of NULL to get first.
	W3DVertexBuffer *getNextVertexBuffer(W3DVertexBuffer *pVb, VBM_FVF_TYPES type)
	{	if (pVb == NULL)
			return m_W3DVertexBuffers[type];
		return pVb->m_nextVB;
	};

	static Int getDX8Format(VBM_FVF_TYPES format);	///<translates our vertex format into D3D equivalent

protected:

	///holds previously allocated slots that are free to fill again.
	W3DVertexBufferSlot *m_W3DVertexBufferSlots[MAX_FVF][MAX_VB_SIZES];
	///holds allocated vertex buffers of each type.
	W3DVertexBuffer		*m_W3DVertexBuffers[MAX_FVF];
	///holds unallocated slot wrappers which were never initialized
	W3DVertexBufferSlot	m_W3DVertexBufferEmptySlots[MAX_NUMBER_SLOTS];
	Int m_numEmptySlotsAllocated;
	///holds unallocated vertex buffer wrappers which were never initialized
	W3DVertexBuffer		m_W3DEmptyVertexBuffers[MAX_VERTEX_BUFFERS_CREATED];
	Int m_numEmptyVertexBuffersAllocated;

	///holds previously allocated slots that are free to fill again.
	W3DIndexBufferSlot *m_W3DIndexBufferSlots[MAX_IB_SIZES];
	///holds allocated index buffers of each type.
	W3DIndexBuffer		*m_W3DIndexBuffers;
	///holds unallocated slot wrappers which were never initialized
	W3DIndexBufferSlot	m_W3DIndexBufferEmptySlots[MAX_NUMBER_SLOTS];
	Int m_numEmptyIndexSlotsAllocated;
	///holds unallocated index buffer wrappers which were never initialized
	W3DIndexBuffer		m_W3DEmptyIndexBuffers[MAX_INDEX_BUFFERS_CREATED];
	Int m_numEmptyIndexBuffersAllocated;

	///allocate new memory inside a vertex buffer.
	W3DVertexBufferSlot *allocateSlotStorage(VBM_FVF_TYPES fvfType, Int size);
	W3DIndexBufferSlot *allocateSlotStorage(Int size);
};

extern W3DBufferManager *TheW3DBufferManager;	//singleton
