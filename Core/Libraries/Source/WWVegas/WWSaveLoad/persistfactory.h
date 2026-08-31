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
 *                     $Archive:: /Commando/Code/wwsaveload/persistfactory.h                  $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/04/01 8:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "WWLib/always.h"
#include "WWLib/bittype.h"
#include "WWLib/chunkio.h"
#include "WWDebug/wwdebug.h"
#include "saveload.h"
#include "persist.h"
#include "Lib/BaseTypeCore.h"

/*
** PersistFactoryClass
** Create a PersistFactoryClass for each concrete derived PersistClass.  These
** factories automatically register with the SaveLoadSystem in their constructors
** and should be accessible through the virtual Get_Factory method of any
** derived PersistClass.
*/

class PersistFactoryClass
{
public:

	PersistFactoryClass();
	virtual ~PersistFactoryClass();

	virtual uint32				Chunk_ID() const												= 0;
	virtual PersistClass *	Load(ChunkLoadClass & cload) const	 						= 0;
	virtual void				Save(ChunkSaveClass & csave,PersistClass * obj)	const	= 0;

private:

	PersistFactoryClass * NextFactory;
	friend class SaveLoadSystemClass;
};




/*
** SimplePersistFactoryClass
** This template automates the creation of a PersistFactory for any type of Persist
** object.  Simply instantiate a single static instance of this template with the
** type and chunkid in the .cpp file of your class.
*/
// The on-disk width of the object-identity token, fixed by the retail save format.
// Changing it changes the format.
static_assert(sizeof(uint32) == 4, "savegame object token must stay 4 bytes on disk");

template <class T,int CHUNKID> class SimplePersistFactoryClass : public PersistFactoryClass
{
public:

	virtual uint32				Chunk_ID() const override { return CHUNKID; }
	virtual PersistClass *	Load(ChunkLoadClass & cload) const override;
	virtual void				Save(ChunkSaveClass & csave,PersistClass * obj) const override;

	/*
	** Internal chunk id's
	*/
	enum
	{
		SIMPLEFACTORY_CHUNKID_OBJPOINTER		=	 0x00100100,
		SIMPLEFACTORY_CHUNKID_OBJDATA
	};
};


template<class T, int CHUNKID> PersistClass *
SimplePersistFactoryClass<T,CHUNKID>::Load(ChunkLoadClass & cload) const
{
	T * new_obj = W3DNEW T;

	// Read exactly what Save wrote: a fixed-width 4-byte identity token, not
	// sizeof(T *). On x86-64 sizeof(T *) is 8, so reading sizeof(T *) here would
	// consume four bytes the writer never wrote and desynchronize the chunk
	// stream. The token is not a real pointer (see the TODO in Save, below); it
	// is carried through as an opaque value and only ever compared for equality
	// by Register_Pointer's pointer table.
	uint32 old_obj_token = 0;

	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == SIMPLEFACTORY_CHUNKID_OBJPOINTER);
	cload.Read(&old_obj_token,sizeof(uint32));
	cload.Close_Chunk();

	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == SIMPLEFACTORY_CHUNKID_OBJDATA);
	new_obj->Load(cload);
	cload.Close_Chunk();

	void * old_obj = (void *)(UnsignedIntPtr)old_obj_token;
	SaveLoadSystemClass::Register_Pointer(old_obj,new_obj);
	return new_obj;
}


template<class T, int CHUNKID> void
SimplePersistFactoryClass<T,CHUNKID>::Save(ChunkSaveClass & csave,PersistClass * obj) const
{
	// TODO(x64-savegame-format): on x86-64 this truncates a 64-bit pointer to a
	// 32-bit on-disk identity token, so two live objects can collide and pointer
	// fixup can bind the wrong object on load. The on-disk width is fixed by the
	// retail save format and cannot be widened here without breaking it.
	// See docs/x64/savegame-format-decision.md (written by the later task).
	uint32 objptr = (uint32)(UnsignedIntPtr)obj;
	csave.Begin_Chunk(SIMPLEFACTORY_CHUNKID_OBJPOINTER);
	csave.Write(&objptr,sizeof(uint32));
	csave.End_Chunk();

	csave.Begin_Chunk(SIMPLEFACTORY_CHUNKID_OBJDATA);
	obj->Save(csave);
	csave.End_Chunk();
}
