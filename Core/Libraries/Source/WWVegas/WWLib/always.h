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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/always.h                               $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/28/01 3:21p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "WWCommon.h"
#include "WWDefines.h"

#include <assert.h>
#include <new>

// TheSuperHackers @build feliwir 17/04/2025 include utility macros for cross-platform compatibility
#include <Utility/compat.h>
#include <Utility/stdint_adapter.h>

// Disable warning about exception handling not being enabled. It's used as part of STL - in a part of STL we don't use.
#pragma warning(disable : 4530)

/*
** Define for debug memory allocation to include __FILE__ and __LINE__ for every memory allocation.
** This helps find leaks.
*/
//#define STEVES_NEW_CATCHER
#ifdef RTS_DEBUG
#ifdef _MSC_VER
#ifdef STEVES_NEW_CATCHER

	#include	<crtdbg.h>
	#include <stdlib.h>
	#include <malloc.h>

	#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define   calloc(c, s)      _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define   _expand(p, s)     _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define   free(p)           _free_dbg(p, _NORMAL_BLOCK)
	#define   _msize(p)         _msize_dbg(p, _NORMAL_BLOCK)

	void* __cdecl operator new(unsigned int s);

#endif	//STEVES_NEW_CATCHER
#endif	//_MSC_VER
#endif	//RTS_DEBUG

#if !defined(DISABLE_GAMEMEMORY) // (gth) killing the Generals Memory Manager!

#ifndef _OPERATOR_NEW_DEFINED_

	#define _OPERATOR_NEW_DEFINED_

	extern void * __cdecl operator new		(size_t size);
	extern void __cdecl operator delete		(void *p);

	extern void * __cdecl operator new[]	(size_t size);
	extern void __cdecl operator delete[]	(void *p);

	// additional overloads to account for VC/MFC funky versions
	extern void* __cdecl operator new			(size_t nSize, const char *, int);
	extern void __cdecl operator delete		(void *, const char *, int);

	extern void* __cdecl operator new[]		(size_t nSize, const char *, int);
	extern void __cdecl operator delete[]	(void *, const char *, int);

#if defined(_MSC_VER) && _MSC_VER < 1300
	// additional overloads for 'placement new'
	//inline void* __cdecl operator new							(size_t s, void *p) { return p; }
	//inline void __cdecl operator delete						(void *, void *p)		{ }
	inline void* __cdecl operator new[]						(size_t s, void *p) { return p; }
	inline void __cdecl operator delete[]					(void *, void *p)		{ }
#endif

#endif

#if defined(RTS_DEBUG)
	#define MSGW3DNEW(MSG)					new( MSG, 0 )
	#define MSGW3DNEWARRAY(MSG)			new( MSG, 0 )
	#define W3DNEW									new("W3D_" __FILE__, 0)
	#define W3DNEWARRAY							new("W3A_" __FILE__, 0)
#else
	#define MSGW3DNEW(MSG)					new
	#define MSGW3DNEWARRAY(MSG)			new
	#define W3DNEW									new
	#define W3DNEWARRAY							new
#endif

// ----------------------------------------------------------------------------
extern void* createW3DMemPool(const char *poolName, int allocationSize);
extern void* allocateFromW3DMemPool(void* p, int allocationSize);
extern void* allocateFromW3DMemPool(void* p, int allocationSize, const char* msg, int unused);
extern void freeFromW3DMemPool(void* pool, void* p);

// ----------------------------------------------------------------------------
#define W3DMPO_GLUE(ARGCLASS) \
private: \
	static void* getClassMemoryPool() \
	{ \
		/* \
			Note that this static variable will be initialized exactly once: the first time \
			control flows over this section of code. This allows us to neatly resolve the \
			order-of-execution problem for static variables, ensuring this is not executed \
			prior to the initialization of TheMemoryPoolFactory. \
		*/ \
		static void* The##ARGCLASS##Pool = createW3DMemPool(#ARGCLASS, sizeof(ARGCLASS)); \
		return The##ARGCLASS##Pool; \
	} \
protected: \
	virtual int glueEnforcer() const { return sizeof(this); } \
public: \
	inline void* operator new(size_t s) { return allocateFromW3DMemPool(getClassMemoryPool(), s); } \
	inline void operator delete(void *p) { freeFromW3DMemPool(getClassMemoryPool(), p); } \
	inline void* operator new(size_t s, const char* msg, int unused) { return allocateFromW3DMemPool(getClassMemoryPool(), s, msg, unused); } \
	inline void operator delete(void *p, const char* msg, int unused) { freeFromW3DMemPool(getClassMemoryPool(), p); } \

// ----------------------------------------------------------------------------
class W3DMPO
{
private:
	static void* getClassMemoryPool()
	{
		assert(0);	// must replace this via W3DMPO_GLUE
		return 0;
	}
protected:
	// we never call this; it is present to cause compile errors in descendent classes
	virtual int glueEnforcer() const = 0;
public:
	virtual ~W3DMPO() { /* nothing */ }
};
// ----------------------------------------------------------------------------

#else

	#define MSGW3DNEW(MSG)					new
	#define MSGW3DNEWARRAY(MSG)			new
	#define W3DNEW									new
	#define W3DNEWARRAY							new

	#define W3DMPO_GLUE(ARGCLASS)

	class W3DMPO { };

#endif // (gth) removing the generals memory stuff from W3D


// Jani: MSVC doesn't necessarily inline code with inline keyword. Using __forceinline results better inlining
// and also prints out a warning if inlining wasn't possible. __forceinline is MSVC specific.
#if defined(_MSC_VER)
#define WWINLINE __forceinline
#else
#define WWINLINE inline
#endif

/*
** Define the MIN and MAX macros.
** NOTE: Joe used to #include <minmax.h> in the various compiler header files.  This
** header defines 'min' and 'max' macros which conflict with the surrender code so
** I'm relpacing all occurances of 'min' and 'max with 'MIN' and 'MAX'.  For code which
** is out of our domain (e.g. Max sdk) I'm declaring template functions for 'min' and 'max'
*/
#define NOMINMAX

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

template <class T> T min(T a,T b)
{
	if (a<b) {
		return a;
	} else {
		return b;
	}
}

template <class T> T max(T a,T b)
{
	if (a>b) {
		return a;
	} else {
		return b;
	}
}


/*
**	This includes the minimum set of compiler defines and pragmas in order to bring the
**	various compilers to a common behavior such that the C&C engine will compile without
**	error or warning.
*/
#if defined(__BORLANDC__)
#include	"borlandc.h"
#endif

#if defined(_MSC_VER)
#include	"visualc.h"
#endif

#if defined(__WATCOMC__)
#include	"WATCOM.H"
#endif


#ifndef	NULL
	#define	NULL		0
#endif


#ifndef size_of
#define size_of(typ,id) sizeof(((typ*)0)->id)
#endif
