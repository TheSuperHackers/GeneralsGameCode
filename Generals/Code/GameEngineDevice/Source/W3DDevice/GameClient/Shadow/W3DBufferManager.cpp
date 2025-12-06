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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////


#include "Common/Debug.h"
#include "W3DDevice/GameClient/W3DBufferManager.h"

W3DBufferManager *TheW3DBufferManager=NULL;	//singleton

static int FVFTypeIndexList[W3DBufferManager::MAX_FVF]=
{
	D3DFVF_XYZ,
	D3DFVF_XYZ|D3DFVF_DIFFUSE,
	D3DFVF_XYZ|D3DFVF_TEX1,
	D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1,
	D3DFVF_XYZ|D3DFVF_TEX2,
	D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX2,
	D3DFVF_XYZ|D3DFVF_NORMAL,
	D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE,
	D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1,
	D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1,
	D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2,
	D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2,
	D3DFVF_XYZRHW,
	D3DFVF_XYZRHW|D3DFVF_DIFFUSE,
	D3DFVF_XYZRHW|D3DFVF_TEX1,
	D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1,
	D3DFVF_XYZRHW|D3DFVF_TEX2,
	D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX2
};

Int W3DBufferManager::getDX8Format(VBM_FVF_TYPES format)
{
	return FVFTypeIndexList[format];
}

W3DBufferManager::W3DBufferManager(void)
{
	m_numEmptySlotsAllocated=0;
	m_numEmptyVertexBuffersAllocated=0;
	m_numEmptyIndexSlotsAllocated=0;
	m_numEmptyIndexBuffersAllocated=0;

	Int i=0;
	for (; i<MAX_FVF; i++)
		m_W3DVertexBuffers[i]=NULL;
	for (i=0; i<MAX_FVF; i++)
		for (Int j=0; j<MAX_VB_SIZES; j++)
			m_W3DVertexBufferSlots[i][j]=NULL;

	m_W3DIndexBuffers=NULL;
	for (Int j=0; j<MAX_IB_SIZES; j++)
		m_W3DIndexBufferSlots[j]=NULL;
}

W3DBufferManager::~W3DBufferManager(void)
{
	freeAllSlots();
	freeAllBuffers();
}

void W3DBufferManager::freeAllSlots(void)
{
	Int i,j;

	for (i=0; i<MAX_FVF; i++)
	{
		for (j=0; j<MAX_VB_SIZES; j++)
		{
			//Release all slots allocated for each size
			W3DVertexBufferSlot *vbSlot = m_W3DVertexBufferSlots[i][j];
			while (vbSlot)
			{
				if (vbSlot->m_prevSameVB)
					vbSlot->m_prevSameVB->m_nextSameVB=vbSlot->m_nextSameVB;
				else
					vbSlot->m_VB->m_usedSlots=NULL;

				if (vbSlot->m_nextSameVB)
					vbSlot->m_nextSameVB->m_prevSameVB=vbSlot->m_prevSameVB;
				vbSlot=vbSlot->m_nextSameSize;
				m_numEmptySlotsAllocated--;
			}
			m_W3DVertexBufferSlots[i][j]=NULL;
		}
	}

	for (j=0; j<MAX_IB_SIZES; j++)
	{
		//Release all slots allocated for each size
		W3DIndexBufferSlot *ibSlot = m_W3DIndexBufferSlots[j];
		while (ibSlot)
		{
			if (ibSlot->m_prevSameIB)
				ibSlot->m_prevSameIB->m_nextSameIB=ibSlot->m_nextSameIB;
			else
				ibSlot->m_IB->m_usedSlots=NULL;

			if (ibSlot->m_nextSameIB)
				ibSlot->m_nextSameIB->m_prevSameIB=ibSlot->m_prevSameIB;
			ibSlot=ibSlot->m_nextSameSize;
			m_numEmptyIndexSlotsAllocated--;
		}
		m_W3DIndexBufferSlots[j]=NULL;
	}

	DEBUG_ASSERTCRASH(m_numEmptySlotsAllocated==0, ("Failed to free all empty vertex buffer slots"));
	DEBUG_ASSERTCRASH(m_numEmptyIndexSlotsAllocated==0, ("Failed to free all empty index buffer slots"));
}

void W3DBufferManager::freeAllBuffers(void)
{
	Int i;

	//Make sure all slots are free
	freeAllSlots();	///<release all slots to pool.

	for (i=0; i<MAX_FVF; i++)
	{
		W3DVertexBuffer *vb = m_W3DVertexBuffers[i];
		while (vb)
		{	DEBUG_ASSERTCRASH(vb->m_usedSlots == NULL, ("Freeing Non-Empty Vertex Buffer"));
			REF_PTR_RELEASE(vb->m_DX8VertexBuffer);
			m_numEmptyVertexBuffersAllocated--;
			vb=vb->m_nextVB;	//get next vertex buffer of this type
		}
		m_W3DVertexBuffers[i]=NULL;
	}

	W3DIndexBuffer *ib = m_W3DIndexBuffers;
	while (ib)
	{	DEBUG_ASSERTCRASH(ib->m_usedSlots == NULL, ("Freeing Non-Empty Index Buffer"));
		REF_PTR_RELEASE(ib->m_DX8IndexBuffer);
		m_numEmptyIndexBuffersAllocated--;
		ib=ib->m_nextIB;	//get next vertex buffer of this type
	}
	m_W3DIndexBuffers=NULL;

	DEBUG_ASSERTCRASH(m_numEmptyVertexBuffersAllocated==0, ("Failed to free all empty vertex buffers"));
	DEBUG_ASSERTCRASH(m_numEmptyIndexBuffersAllocated==0, ("Failed to free all empty index buffers"));
}

void W3DBufferManager::ReleaseResources(void)
{
	for (Int i=0; i<MAX_FVF; i++)
	{
		W3DVertexBuffer *vb = m_W3DVertexBuffers[i];
		while (vb)
		{
			REF_PTR_RELEASE(vb->m_DX8VertexBuffer);
			vb=vb->m_nextVB;	//get next vertex buffer of this type
		}
	}

	W3DIndexBuffer *ib = m_W3DIndexBuffers;
	while (ib)
	{
		REF_PTR_RELEASE(ib->m_DX8IndexBuffer);
		ib=ib->m_nextIB;	//get next vertex buffer of this type
	}
}

Bool W3DBufferManager::ReAcquireResources(void)
{
	for (Int i=0; i<MAX_FVF; i++)
	{
		W3DVertexBuffer *vb = m_W3DVertexBuffers[i];
		while (vb)
		{	DEBUG_ASSERTCRASH( vb->m_DX8VertexBuffer == NULL, ("ReAcquire of existing vertex buffer"));
			vb->m_DX8VertexBuffer=NEW_REF(DX8VertexBufferClass,(FVFTypeIndexList[vb->m_format],vb->m_size,DX8VertexBufferClass::USAGE_DEFAULT));
			DEBUG_ASSERTCRASH( vb->m_DX8VertexBuffer, ("Failed ReAcquire of vertex buffer"));
			if (!vb->m_DX8VertexBuffer)
				return FALSE;
			vb=vb->m_nextVB;	//get next vertex buffer of this type
		}
	}

	W3DIndexBuffer *ib = m_W3DIndexBuffers;
	while (ib)
	{	DEBUG_ASSERTCRASH( ib->m_DX8IndexBuffer == NULL, ("ReAcquire of existing index buffer"));
		ib->m_DX8IndexBuffer=NEW_REF(DX8IndexBufferClass,(ib->m_size,DX8IndexBufferClass::USAGE_DEFAULT));
		DEBUG_ASSERTCRASH( ib->m_DX8IndexBuffer, ("Failed ReAcquire of index buffer"));
		if (!ib->m_DX8IndexBuffer)
			return FALSE;
		ib=ib->m_nextIB;	//get next vertex buffer of this type
	}

	return TRUE;
}

/**Searches through previously allocated vertex buffer slots and returns a matching type.  If none found,
   creates a new slot and adds it to the pool.  Returns a pointer to the VB slot.
   Returns NULL in case of failure.
*/
W3DBufferManager::W3DVertexBufferSlot *W3DBufferManager::getSlot(VBM_FVF_TYPES fvfType, Int size)
{
	W3DVertexBufferSlot *vbSlot=NULL;

	//round size to next multiple of minimum slot size.
	//should help avoid fragmentation.
	size = (size + (MIN_SLOT_SIZE-1)) & (~(MIN_SLOT_SIZE-1));
	Int sizeIndex = (size >> MIN_SLOT_SIZE_SHIFT)-1;

	DEBUG_ASSERTCRASH(sizeIndex < MAX_VB_SIZES && size > 0, ("Allocating too large vertex buffer slot"));
	// TheSuperHackers @bugfix xezon 18/05/2025 Protect against indexing slots beyond the max size.
	// This will happen when a mesh is too complex to draw shadows with.
	if (sizeIndex >= MAX_VB_SIZES || size <= 0) {
		return NULL;
	}

	if ((vbSlot=m_W3DVertexBufferSlots[fvfType][sizeIndex]) != NULL)
	{	//found a previously allocated slot matching required size
		m_W3DVertexBufferSlots[fvfType][sizeIndex]=vbSlot->m_nextSameSize;
		if (vbSlot->m_nextSameSize)
			vbSlot->m_nextSameSize->m_prevSameSize=NULL;
		return vbSlot;
	}
	else
	{	//need to allocate a new slot
		return allocateSlotStorage(fvfType, size);
	}

	return NULL;
}

/**Returns vertex buffer space back to pool so it can be reused later*/
void W3DBufferManager::releaseSlot(W3DVertexBufferSlot *vbSlot)
{
	Int sizeIndex = (vbSlot->m_size >> MIN_SLOT_SIZE_SHIFT)-1;

	vbSlot->m_nextSameSize=m_W3DVertexBufferSlots[vbSlot->m_VB->m_format][sizeIndex];
	if (m_W3DVertexBufferSlots[vbSlot->m_VB->m_format][sizeIndex])
		m_W3DVertexBufferSlots[vbSlot->m_VB->m_format][sizeIndex]->m_prevSameSize=vbSlot;

	m_W3DVertexBufferSlots[vbSlot->m_VB->m_format][sizeIndex]=vbSlot;
}

/**Reserves space inside existing vertex buffer or allocates a new one to fit the required size.
*/
W3DBufferManager::W3DVertexBufferSlot * W3DBufferManager::allocateSlotStorage(VBM_FVF_TYPES fvfType, Int size)
{

	W3DVertexBuffer *pVB;
	W3DVertexBufferSlot *vbSlot;
//	Int sizeIndex = (size >> MIN_SLOT_SIZE_SHIFT)-1;

	DEBUG_ASSERTCRASH(m_numEmptySlotsAllocated < MAX_NUMBER_SLOTS, ("No more VB Slots"));
	// TheSuperHackers @bugfix xezon 18/05/2025 Protect against allocating slot storage beyond the max size.
	// This will happen when there are too many meshes in the scene to draw shadows with.
	if (m_numEmptySlotsAllocated >= MAX_NUMBER_SLOTS) {
		return NULL;
	}

	pVB=m_W3DVertexBuffers[fvfType];
	while (pVB)
	{
		if ((pVB->m_size - pVB->m_startFreeIndex) >= size)
		{	//found enough free space in this vertex buffer
			vbSlot=&m_W3DVertexBufferEmptySlots[m_numEmptySlotsAllocated];
			vbSlot->m_size=size;
			vbSlot->m_start=pVB->m_startFreeIndex;
			vbSlot->m_VB=pVB;
			//Link to VB list of slots
			vbSlot->m_nextSameVB=pVB->m_usedSlots;
			vbSlot->m_prevSameVB=NULL;	//this will be the new head
			if (pVB->m_usedSlots)
				pVB->m_usedSlots->m_prevSameVB=vbSlot;
			vbSlot->m_prevSameSize=vbSlot->m_nextSameSize=NULL;
			pVB->m_usedSlots=vbSlot;
			pVB->m_startFreeIndex += size;
			m_numEmptySlotsAllocated++;
			return vbSlot;
		}
		pVB = pVB->m_nextVB;
	}

	pVB=m_W3DVertexBuffers[fvfType];	//save old list head

	//Didn't find any vertex buffers with room, create a new one
	DEBUG_ASSERTCRASH(m_numEmptyVertexBuffersAllocated < MAX_VERTEX_BUFFERS_CREATED, ("Reached Max Static VB Shadow Geometry"));

	if (m_numEmptyVertexBuffersAllocated < MAX_VERTEX_BUFFERS_CREATED)
	{
		m_W3DVertexBuffers[fvfType] = &m_W3DEmptyVertexBuffers[m_numEmptyVertexBuffersAllocated];
		m_W3DVertexBuffers[fvfType]->m_nextVB=pVB;	//link to list
		m_numEmptyVertexBuffersAllocated++;

		pVB=m_W3DVertexBuffers[fvfType];	//get new list head

		Int vbSize=__max(DEFAULT_VERTEX_BUFFER_SIZE,size);

		pVB->m_DX8VertexBuffer=NEW_REF(DX8VertexBufferClass,(FVFTypeIndexList[fvfType],vbSize,DX8VertexBufferClass::USAGE_DEFAULT));
		pVB->m_format=fvfType;
		pVB->m_startFreeIndex=size;
		pVB->m_size=vbSize;
		vbSlot=&m_W3DVertexBufferEmptySlots[m_numEmptySlotsAllocated];
		m_numEmptySlotsAllocated++;
		pVB->m_usedSlots=vbSlot;
		vbSlot->m_size=size;
		vbSlot->m_start=0;
		vbSlot->m_VB=pVB;
		vbSlot->m_prevSameVB=vbSlot->m_nextSameVB=NULL;
		vbSlot->m_prevSameSize=vbSlot->m_nextSameSize=NULL;
		return vbSlot;
	}

	return NULL;
}

//******************************** Index Buffer code ******************************************************
/**Searches through previously allocated index buffer slots and returns a matching type.  If none found,
   creates a new slot and adds it to the pool.  Returns a pointer to the IB slot.
   Returns NULL in case of failure.
*/
W3DBufferManager::W3DIndexBufferSlot *W3DBufferManager::getSlot(Int size)
{
	W3DIndexBufferSlot *ibSlot=NULL;

	//round size to next multiple of minimum slot size.
	//should help avoid fragmentation.
	size = (size + (MIN_SLOT_SIZE-1)) & (~(MIN_SLOT_SIZE-1));
	Int sizeIndex = (size >> MIN_SLOT_SIZE_SHIFT)-1;

	DEBUG_ASSERTCRASH(sizeIndex < MAX_IB_SIZES && size > 0, ("Allocating too large index buffer slot"));
	// TheSuperHackers @bugfix xezon 18/05/2025 Protect against indexing slots beyond the max size.
	// This will happen when a mesh is too complex to draw shadows with.
	if (sizeIndex >= MAX_IB_SIZES || size <= 0) {
		return NULL;
	}

	if ((ibSlot=m_W3DIndexBufferSlots[sizeIndex]) != NULL)
	{	//found a previously allocated slot matching required size
		m_W3DIndexBufferSlots[sizeIndex]=ibSlot->m_nextSameSize;
		if (ibSlot->m_nextSameSize)
			ibSlot->m_nextSameSize->m_prevSameSize=NULL;
		return ibSlot;
	}
	else
	{	//need to allocate a new slot
		return allocateSlotStorage(size);
	}

	return NULL;
}

/**Returns index buffer space back to pool so it can be reused later*/
void W3DBufferManager::releaseSlot(W3DIndexBufferSlot *ibSlot)
{
	Int sizeIndex = (ibSlot->m_size >> MIN_SLOT_SIZE_SHIFT)-1;

	ibSlot->m_nextSameSize=m_W3DIndexBufferSlots[sizeIndex];
	if (m_W3DIndexBufferSlots[sizeIndex])
		m_W3DIndexBufferSlots[sizeIndex]->m_prevSameSize=ibSlot;

	m_W3DIndexBufferSlots[sizeIndex]=ibSlot;
}

/**Reserves space inside existing index buffer or allocates a new one to fit the required size.
*/
W3DBufferManager::W3DIndexBufferSlot * W3DBufferManager::allocateSlotStorage(Int size)
{

	W3DIndexBuffer *pIB;
	W3DIndexBufferSlot *ibSlot;
//	Int sizeIndex = (size >> MIN_SLOT_SIZE_SHIFT)-1;

	DEBUG_ASSERTCRASH(m_numEmptyIndexSlotsAllocated < MAX_NUMBER_SLOTS, ("No more IB Slots"));
	// TheSuperHackers @bugfix xezon 18/05/2025 Protect against allocating slot storage beyond the max size.
	// This will happen when there are too many meshes in the scene to draw shadows with.
	if (m_numEmptyIndexSlotsAllocated >= MAX_NUMBER_SLOTS) {
		return NULL;
	}

	pIB=m_W3DIndexBuffers;
	while (pIB)
	{
		if ((pIB->m_size - pIB->m_startFreeIndex) >= size)
		{	//found enough free space in this index buffer
			ibSlot=&m_W3DIndexBufferEmptySlots[m_numEmptyIndexSlotsAllocated];
			ibSlot->m_size=size;
			ibSlot->m_start=pIB->m_startFreeIndex;
			ibSlot->m_IB=pIB;
			//Link to IB list of slots
			ibSlot->m_nextSameIB=pIB->m_usedSlots;
			ibSlot->m_prevSameIB=NULL;	//this will be the new head
			if (pIB->m_usedSlots)
				pIB->m_usedSlots->m_prevSameIB=ibSlot;
			ibSlot->m_prevSameSize=ibSlot->m_nextSameSize=NULL;
			pIB->m_usedSlots=ibSlot;
			pIB->m_startFreeIndex += size;
			m_numEmptyIndexSlotsAllocated++;
			return ibSlot;
		}
		pIB = pIB->m_nextIB;
	}

	pIB=m_W3DIndexBuffers;	//save old list head

	//Didn't find any index buffers with room, create a new one
	DEBUG_ASSERTCRASH(m_numEmptyIndexBuffersAllocated < MAX_INDEX_BUFFERS_CREATED, ("Reached Max Static IB Shadow Geometry"));

	if (m_numEmptyIndexBuffersAllocated < MAX_INDEX_BUFFERS_CREATED)
	{
		m_W3DIndexBuffers = &m_W3DEmptyIndexBuffers[m_numEmptyIndexBuffersAllocated];
		m_W3DIndexBuffers->m_nextIB=pIB;	//link to list
		m_numEmptyIndexBuffersAllocated++;

		pIB=m_W3DIndexBuffers;	//get new list head

		Int ibSize=__max(DEFAULT_INDEX_BUFFER_SIZE,size);

		pIB->m_DX8IndexBuffer=NEW_REF(DX8IndexBufferClass,(ibSize,DX8IndexBufferClass::USAGE_DEFAULT));
		pIB->m_startFreeIndex=size;
		pIB->m_size=ibSize;
		ibSlot=&m_W3DIndexBufferEmptySlots[m_numEmptyIndexSlotsAllocated];
		m_numEmptyIndexSlotsAllocated++;
		pIB->m_usedSlots=ibSlot;
		ibSlot->m_size=size;
		ibSlot->m_start=0;
		ibSlot->m_IB=pIB;
		ibSlot->m_prevSameIB=ibSlot->m_nextSameIB=NULL;
		ibSlot->m_prevSameSize=ibSlot->m_nextSameSize=NULL;
		return ibSlot;
	}

	return NULL;
}
