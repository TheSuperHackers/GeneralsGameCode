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

#include "FastAllocator.h"
#include <new>

static FastAllocatorGeneral* generalAllocator; //This general allocator will do all allocations for us.

FastAllocatorGeneral* FastAllocatorGeneral::Get_Allocator()
{
	if (!generalAllocator) {
		generalAllocator=reinterpret_cast<FastAllocatorGeneral*>(::malloc(sizeof(FastAllocatorGeneral)));

		new (generalAllocator) FastAllocatorGeneral();
	}
	return generalAllocator;
}

FastAllocatorGeneral::FastAllocatorGeneral() : MemoryLeakLogEnabled(false), AllocatedWithMalloc(0), AllocatedWithMallocCount(0), ActualMemoryUsage(0)
{
	int alloc_size=ALLOC_STEP;
	for (int i=0;i<MAX_ALLOC_SIZE/ALLOC_STEP;++i) {
	   allocators[i].Init(alloc_size);
		alloc_size+=ALLOC_STEP;
	}
}

