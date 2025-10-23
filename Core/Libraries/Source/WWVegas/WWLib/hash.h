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

/* $Header: /Commando/Code/wwlib/hash.h 2     6/15/00 10:53a Byon_g $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/hash.h                                 $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 6/15/00 9:43a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"

class HashTableIteratorClass;


/*
** HashableClass
*/
class	HashableClass {

public:
	HashableClass( void ) : NextHash( NULL ) {}
	virtual	~HashableClass( void ) {}

	virtual	const char * Get_Key( void )				= 0;

private:
	HashableClass * NextHash;
	friend	class		HashTableClass;
	friend	class		HashTableIteratorClass;
};


/*
** HashTableClass
*/
class HashTableClass {

public:
	HashTableClass( int size );
	~HashTableClass( void );

	void					Reset( void );
	void					Add( HashableClass * entry );
	bool					Remove( HashableClass * entry );

	HashableClass *	Find( const char * key );

private:
	// HashTableSize MUST be a power of two
	int					HashTableSize;
	HashableClass * *	HashTable;

	// Convert key to a table index
	int					Hash( const char * key );

	friend	class		HashTableIteratorClass;
};


/*
**
*/
class HashTableIteratorClass
{
public:
	HashTableIteratorClass( HashTableClass & table ) : Table( table ) {}
	virtual ~HashTableIteratorClass( void ) {}

	void					First( void );
	void					Next( void );
	bool					Is_Done( void )		{ return CurrentEntry == NULL; }
	HashableClass *	Get_Current( void )	{ return CurrentEntry; }

private:
	const HashTableClass	&	Table;
	int							Index;
	HashableClass *			CurrentEntry;
	HashableClass *			NextEntry;

	void					Advance_Next( void );
};
