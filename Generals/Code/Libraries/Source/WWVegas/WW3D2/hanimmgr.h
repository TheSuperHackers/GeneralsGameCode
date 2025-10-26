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

/* $Header: /Commando/Code/ww3d2/hanimmgr.h 1     1/22/01 3:36p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/hanimmgr.h                             $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "hash.h"
#include "wwstring.h"
#include "Vector.H"

class HAnimClass;
class ChunkLoadClass;
class W3DExclusionListClass;

/*
** An entry for a table of anims not found, so we can quickly determine their loss
*/
class MissingAnimClass : public HashableClass {

public:
	MissingAnimClass( const char * name ) : Name( name ) {}
	virtual	~MissingAnimClass( void ) {}

	virtual	const char * Get_Key( void )	{ return Name;	}

private:
	StringClass	Name;

};

/*
	HAnimManagerClass

	This class is used to keep track of all of the motion data.
*/

class HAnimManagerClass
{

public:
	HAnimManagerClass(void);
	~HAnimManagerClass(void);

	int			 		Load_Anim(ChunkLoadClass & cload);
	HAnimClass *		Get_Anim(const char * name);
	HAnimClass *		Peek_Anim(const char * name);
	bool					Add_Anim(HAnimClass *new_anim);
	void			 		Free_All_Anims(void);
	void			 		Free_All_Anims_With_Exclusion_List(const W3DExclusionListClass & exclusion_list);
	void					Create_Asset_List(DynamicVectorClass<StringClass> & exclusion_list);

	void					Register_Missing( const char * name );
	bool					Is_Missing( const char * name );
	void					Reset_Missing( void );

private:
	int					Load_Compressed_Anim(ChunkLoadClass & cload);
	int					Load_Raw_Anim(ChunkLoadClass & cload);
	int					Load_Morph_Anim(ChunkLoadClass & cload);

	HashTableClass	*	AnimPtrTable;
	HashTableClass	*	MissingAnimTable;

	friend	class		HAnimManagerIterator;
};


/*
** An Iterator to get to all loaded HAnims in a HAnimManager
*/
class HAnimManagerIterator : public HashTableIteratorClass {
public:
	HAnimManagerIterator( HAnimManagerClass & manager ) : HashTableIteratorClass( *manager.AnimPtrTable ) {}
	HAnimClass * Get_Current_Anim( void );
};
