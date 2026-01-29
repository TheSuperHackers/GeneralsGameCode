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
 *                 Project Name : WWDebug                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwdebug/wwmemlog.h                           $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/09/01 6:51p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <stddef.h>

#define LOG_MEMORY	// Comment this out to disable memlog compiling in

class MemLogClass;

/**
** Memory Log Categories
** You can cause memory allocations to be "counted" against any of the following categories.
** NOTE: if you add a new category here, be sure to add its name to the array in the .cpp file...
*/
enum
{
	MEM_UNKNOWN				= 0,
	MEM_GEOMETRY,			// memory used by geometry data
	MEM_ANIMATION,			// memory used by animation data
	MEM_TEXTURE,			// memory used by textures
	MEM_PATHFIND,			// memory used by the pathfind system
	MEM_VIS,					// memory used by the vis system
	MEM_SOUND,				// memory used by the sound system
	MEM_CULLINGDATA,		// culling systems
	MEM_STRINGS,			// string data
	MEM_GAMEDATA,			// game engine datastructures
	MEM_PHYSICSDATA,		// physics engine datastructures
	MEM_W3DDATA,			// w3d datastructures (not including ones more applicable to above categories)
	MEM_STATICALLOCATION,// all the allocations that happen before the memlog Init() function call are from statically allocated objects
	MEM_GAMEINIT,			// game init time allocations
	MEM_RENDERER,			// dx8 renderer
	MEM_NETWORK,
	MEM_BINK,

	MEM_COUNT
};



/**
** WWMemoryLogClass
** This interface can provide information on how much memory has been allocated to each
** memory category.  In order to enable this logging, you will need to implement global
** new and delete functions which call the Allocate_Memory and Release_Memory functions
** in this class.  For example:
**
** void * ::operator new (size_t size)
** {
** 	return WWMemoryLogClass::Allocate_Memory(size);
** }
**
** void ::operator delete (void *ptr)
** {
** 	WWMemoryLogClass::Release_Memory(ptr);
** }
*/
class WWMemoryLogClass
{
public:
	static void				Enable_Memory_Log(bool enable) { IsMemoryLogEnabled=enable; }
	static bool				Is_Memory_Log_Enabled() { return IsMemoryLogEnabled; }

	/*
	** Accessors to the current memory map
	*/
	static int				Get_Category_Count(void);
	static const char *	Get_Category_Name(int category);
	static int				Get_Current_Allocated_Memory(int category);
	static int				Get_Peak_Allocated_Memory(int category);

	/*
	** Interface for the debug version of new and delete
	*/
	static int				Register_Memory_Allocated(int size);
	static void				Register_Memory_Released(int category,int size);

	/*
	** New and Delete functions.  If you want to use this logging system,
	** implement global new and delete functions which call into these
	** functions.
	*/
	static void *			Allocate_Memory(size_t size);
	static void				Release_Memory(void * mem);

	static void				Reset_Counters();			// Reset allocate and free counters
	static int				Get_Allocate_Count();	// Return allocate count since last reset
	static int				Get_Free_Count();			// Return allocate count since last reset

	static void				Init();
protected:

	/*
	** Interface for WWMemorySampleClass to set the active category
	*/
	static void				Push_Active_Category(int category);
	static void				Pop_Active_Category(void);

	static MemLogClass * Get_Log(void);
	static void  Release_Log(void);

	static bool IsMemoryLogEnabled;

	friend class WWMemorySampleClass;
};



/**
** WWMemorySampleClass
** This class is meant to be created and destroyed on the stack to automatically push
** and pop the desired memory category.  NOTE: this class should not be used directly,
** instead, use the WWMEMLOG macros!
*/
class WWMemorySampleClass
{
	bool category_push;
public:
	WWMemorySampleClass(int category) : category_push(WWMemoryLogClass::Is_Memory_Log_Enabled())
	{
		if (category_push) {
			WWMemoryLogClass::Push_Active_Category(category);
		}
	}

	~WWMemorySampleClass(void)
	{
		if (category_push) {
			WWMemoryLogClass::Pop_Active_Category();
		}
	}
};



/*
** Use the WWMEMLOG macro to track all memory allocations within the current scope.
** If WWDEBUG is not enabled, memory usage logging will be disabled.
*/
#ifdef USE_MEMLOG
#define	WWMEMLOG( category )					WWMemorySampleClass _memsample( category )
#else
#define	WWMEMLOG( category )
#endif
