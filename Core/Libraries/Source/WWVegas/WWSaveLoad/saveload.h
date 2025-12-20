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
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwsaveload/saveload.h                        $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/19/01 4:13p                                               $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "pointerremap.h"
#include "bittype.h"
#include "SLIST.h"

class RefCountClass;
class SaveLoadSubSystemClass;
class PersistFactoryClass;
class PersistClass;
class PostLoadableClass;
class ChunkSaveClass;
class ChunkLoadClass;


//////////////////////////////////////////////////////////////////////////////////
//
//	WWSaveLoad
//
// The WWSaveLoad library is a framework for saving and loading.  The main
// goals that we attempted to achieve in designing this system are:
//
// - Save things in a form that could adapt as our code evolves.  We want
//   to be able to load files which were created with a previous version of the
//   application into the current version.
// - Use the same framework throughout all of our libraries with as small an
//   impact on them as possible
// - Automate as much of the implementation of save-load as possible.
// - Make this a generic hunk of code which contains no commando-specific parts
//   so that it can be used by other applications.
// - Make this system capable of generating the file formats for our level editor,
//   the game's level definition file, and player save files.
//
// To achive this, we developed several core concepts:
//
// - Persistant Objects: Most of the state of the game is contained in the objects
//   active at any given time.  PersistClass is an abstract interface which will allow
//   objects to be used with the save-load system.  It was also important to keep the
//   overhead caused by inheriting this class to an absolute minimum.
//
// - PersistFactories: We need an automatic "virtual-constructor" or "abstract-factory"
//   system for all objects that get saved.  This is the PersistFactoryClass and the
//   automated SimplePersistFactory template.  All objects that "persist" are derived from
//   PersistClass and all concrete derived PersistClasses have an associated static
//   instance of a PersistFactory which handles their saving and construction upon encountering
//   them while loading.  In certain cases these PersistFactories can also serve as
//   a "shortcut" where we cheat by not actually telling the object to save but simply save
//   a small piece of data that allows us to recreate an identical object when we load.  This
//   is used in WW3D sometimes; we just save a render object name and then ask the WW3D asset
//   manager to recreate that model for us.
//
// - SaveLoadSubSystems: The overall file structure will be governed by many sub-systems
//   (derived from SaveLoadSubSystemClass).  The application in-effect creates file formats
//   by simply having the sub-systems that it wants write into a file.  In this way you can
//   achieve things like saving only static data into one file and dynamic into another, etc.
//   All persistant objects that get saved will be told to save by some sub-system.  For
//   example: in Commando, I have a PhysicsDynamicDataSubSystem which saves all of the
//   dynamic physics objects.  In saving those objects I use the built-in PersistFactories
//   and am therefore completely safe from new object types being added to the system, it will just
//   automatically work
//
// - Pointer re-mapping: A pointer remaping system is built into the save-load system.  There
//   are several things that happen in this system.  Each object, as it is saved and loaded,
//   registers with the system its old address and its new address.  (the old address is saved
//   and the new address is available once the object is created).   This is automated by the
//   SimplePersistFactory for all but classes that use multiple inheritance.  During the load
//   process a table is built which contains all of these pointer pairs (old address, new address).
//   Whenever an object loads a pointer, it gives a "pointer to that pointer" to the save load system.
//   Then, after all of the objects have been loaded, the system goes through that list of pointers
//   and finds them in the pointer pair table.  NOTE: use the macros for re-mapping your
//   pointers to enable automatic debugging information when you build with WWDEBUG defined.
//
// - Chunks: The file format will be chunk based since that gives us the flexibility to
//   add new data and remove obsolete data without necessarily losing the ability
//   to read old files.  We will use a "high-granularity" of chunks.  In many cases, each
//   member variable will be in its own chunk for maximum flexibility.  To help soften
//   the memory usage for this approach, we developed the concept of "micro-chunks".
//   Micro-chunks are just like chunks in that they have an id and a size but
//   each of these fields are only a single byte and they are never hierarchical.
//
// - ChunkID's: The chunk ID's used by Subsystems and PersistFactories must be unique
//   but all others can be considered local to the object that is saving itself.  Unique
//   ids for the subsystems and factories are achieved by saveloadids.h defining ranges
//   of ids for various libraries and then those libraries maintaining a single header
//   file internally which gives unique id's within that range to all of their sub-systems
//   and persist factories.  Never re-use an id or you will break compatibility with older
//   versions of your files...
//
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
//
//	SaveLoadSystemClass
//
//////////////////////////////////////////////////////////////////////////////////
class SaveLoadSystemClass
{
public:

	/*
	** Save-Load interface.  To create a file, ask each sub-system to save itself.
	** To load a file just open it and pass it to the load method.
	*/
	static bool		Save (ChunkSaveClass &csave, SaveLoadSubSystemClass & subsystem);
	static bool		Load (ChunkLoadClass &cload,bool auto_post_load = true);
	static bool		Post_Load_Processing (void(*network_callback)(void));
	/*
	** Look up the persist factory for a given chunk id
	*/
	static PersistFactoryClass * Find_Persist_Factory(uint32 chunk_id);

	/*
	** Post-Load interface.  An object being loaded can ask for a callback after
	** all objects have been loaded and pointers re-mapped.
	*/
	static void		Register_Post_Load_Callback(PostLoadableClass * obj);

	/*
	** Pointer Remapping interface.  NOTE: use the macros defined below to
	** get debug info with your pointers when doing a debug build.
	*/
	static void		Register_Pointer (void *old_pointer, void *new_pointer);

#ifdef WWDEBUG
	static void		Request_Pointer_Remap (void **pointer_to_convert,const char * file = NULL,int line = 0);
	static void		Request_Ref_Counted_Pointer_Remap (RefCountClass **pointer_to_convert,const char * file = NULL,int line = 0);
#else
	static void		Request_Pointer_Remap (void **pointer_to_convert);
	static void		Request_Ref_Counted_Pointer_Remap (RefCountClass **pointer_to_convert);
#endif

protected:

	/*
	** Internal SaveLoadSystem functions
	*/
	static void		Register_Sub_System (SaveLoadSubSystemClass * subsys);
	static void		Unregister_Sub_System (SaveLoadSubSystemClass * subsys);
	static SaveLoadSubSystemClass * Find_Sub_System (uint32 chunk_id);

	static void		Register_Persist_Factory(PersistFactoryClass * factory);
	static void		Unregister_Persist_Factory(PersistFactoryClass * factory);

	static void		Link_Sub_System(SaveLoadSubSystemClass * subsys);
	static void		Unlink_Sub_System(SaveLoadSubSystemClass * subsys);
	static void		Link_Factory(PersistFactoryClass * factory);
	static void		Unlink_Factory(PersistFactoryClass * factory);

	static bool		Is_Post_Load_Callback_Registered(PostLoadableClass * obj);

	static SaveLoadSubSystemClass *		SubSystemListHead;
	static PersistFactoryClass *			FactoryListHead;
	static PointerRemapClass				PointerRemapper;
	static SList<PostLoadableClass>		PostLoadList;

	/*
	** these are friends so that they can register themselves at construction time.
	*/
	friend class SaveLoadSubSystemClass;
	friend class PersistFactoryClass;
};


/*
** Use the following macros to automatically enable pointer-remap DEBUG code.  Remember that
** in all cases you submit a pointer to the pointer you want re-mapped.
*/
#ifdef WWDEBUG
#define REQUEST_POINTER_REMAP(pp)					SaveLoadSystemClass::Request_Pointer_Remap(pp,__FILE__,__LINE__)
#define REQUEST_REF_COUNTED_POINTER_REMAP(pp)	SaveLoadSystemClass::Request_Ref_Counted_Pointer_Remap(pp,__FILE__,__LINE__)
#else
#define REQUEST_POINTER_REMAP(pp)					SaveLoadSystemClass::Request_Pointer_Remap(pp)
#define REQUEST_REF_COUNTED_POINTER_REMAP(pp)	SaveLoadSystemClass::Request_Ref_Counted_Pointer_Remap(pp)
#endif
