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

#include "always.h"
#include "bittype.h"
#include "chunkio.h"
#include "wwdebug.h"
#include "saveload.h"
#include "persist.h"

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

	PersistFactoryClass(void);
	virtual ~PersistFactoryClass(void);

	virtual uint32				Chunk_ID(void) const												= 0;
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
template <class T,int CHUNKID> class SimplePersistFactoryClass : public PersistFactoryClass
{
public:

	virtual uint32				Chunk_ID(void) const										{ return CHUNKID; }
	virtual PersistClass *	Load(ChunkLoadClass & cload) const;
	virtual void				Save(ChunkSaveClass & csave,PersistClass * obj) const;

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
	T * old_obj = NULL;

	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == SIMPLEFACTORY_CHUNKID_OBJPOINTER);
	cload.Read(&old_obj,sizeof(T *));
	cload.Close_Chunk();

	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == SIMPLEFACTORY_CHUNKID_OBJDATA);
	new_obj->Load(cload);
	cload.Close_Chunk();

	SaveLoadSystemClass::Register_Pointer(old_obj,new_obj);
	return new_obj;
}


template<class T, int CHUNKID> void
SimplePersistFactoryClass<T,CHUNKID>::Save(ChunkSaveClass & csave,PersistClass * obj) const
{
	uint32 objptr = (uint32)obj;
	csave.Begin_Chunk(SIMPLEFACTORY_CHUNKID_OBJPOINTER);
	csave.Write(&objptr,sizeof(uint32));
	csave.End_Chunk();

	csave.Begin_Chunk(SIMPLEFACTORY_CHUNKID_OBJDATA);
	obj->Save(csave);
	csave.End_Chunk();
}
