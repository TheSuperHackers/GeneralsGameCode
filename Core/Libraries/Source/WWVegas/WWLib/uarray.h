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
 *                 Project Name : Library                                                      *
 *                                                                                             *
 *                     $Archive:: /G/wwlib/uarray.h                                           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/24/99 1:56p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   UniqueArrayClass<T>::UniqueArrayClass -- constructor                                      *
 *   UniqueArrayClass<T>::~UniqueArrayClass -- destructor                                      *
 *   UniqueArrayClass<T>::Add -- Add an item to the array                                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef UARRAY_H
#define UARRAY_H

#ifndef HASHCALC_H
#include "hashcalc.h"
#endif

#ifndef VECTOR_H
#include "Vector.H"
#endif


/*
** UniqueArrayClass
** This template class can be used to generate an array of unique objects
** amongst a huge list of objects which may or may not be unique.  However,
** in order to use the UniqueArrayClass, you will need to implement a 
** HashCalculatorClass for the type you are using.
**
** Note that the UniqueArrayClass does *copies* of the objects you are
** giving it.  It is meant to be used with relatively lightweight objects.
*/
template <class T> class UniqueArrayClass
{

public:

	UniqueArrayClass(int initialsize,int growthrate,HashCalculatorClass<T> * hasher);
	~UniqueArrayClass(void);

	int				Add(const T & new_item);

	int				Count(void) const								{ return Get_Unique_Count(); }
	int				Get_Unique_Count(void) const				{ return UniqueItems.Count(); }
	const T &		Get(int index) const							{ return UniqueItems[index].Item; }
	const T &		operator [] (int index) const				{ return Get(index); }

private:

	enum { NO_ITEM = 0xFFFFFFFF };

	class HashItem
	{
	public:
		T 		Item;
		int	NextHashIndex;

		bool operator == (const HashItem & that) { return ((Item == that.Item) && (NextHashIndex == that.NextHashIndex)); }
		bool operator != (const HashItem & that) { return !(*this == that); }
	};
		
	// Dynamic Vector of the unique items:
	DynamicVectorClass<HashItem>		UniqueItems;

	// Hash table:
	int										HashTableSize;
	int *										HashTable;

	// object which does the hashing for the type
	HashCalculatorClass<T> *			HashCalculator;

	friend class VectorClass<T>;
	friend class DynamicVectorClass<T>;
};



/***********************************************************************************************
 * UniqueArrayClass<T>::UniqueArrayClass -- constructor                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/29/98    GTH : Created.                                                                 *
 *=============================================================================================*/
template <class T>
UniqueArrayClass<T>::UniqueArrayClass(int initial_size,int growth_rate,HashCalculatorClass<T> * hasher) :
	UniqueItems(initial_size),
	HashCalculator(hasher)
{
	// set the growth rate.
	UniqueItems.Set_Growth_Step(growth_rate);
		
	// sizing and allocating the actual hash table
	int bits = HashCalculator->Num_Hash_Bits();
	assert(bits > 0);
	assert(bits < 24);
	HashTableSize = 1<<bits;
	HashTable = W3DNEWARRAY int[HashTableSize];

	for (int hidx=0; hidx < HashTableSize; hidx++) {
		HashTable[hidx] = NO_ITEM;
	}
}


/***********************************************************************************************
 * UniqueArrayClass<T>::~UniqueArrayClass -- destructor                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/29/98    GTH : Created.                                                                 *
 *=============================================================================================*/
template <class T>
UniqueArrayClass<T>::~UniqueArrayClass(void)
{
	if (HashTable != NULL) {
		delete[] HashTable;
		HashTable = NULL;
	}
}


/***********************************************************************************************
 * UniqueArrayClass<T>::Add -- Add an item to the array                                        *
 *                                                                                             *
 * Only adds the item to the end of the array if another duplicate item is not found.  Returns *
 * the array index of where the item is stored.                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/29/98    GTH : Created.                                                                 *
 *=============================================================================================*/
template <class T> 
inline int UniqueArrayClass<T>::Add(const T & new_item) 
{
	/*
	** Use the hash table to quickly (hopefully :-) detect
	** whether this item is already in the array
	*/
	int num_hash_vals;
	HashCalculator->Compute_Hash(new_item);
	num_hash_vals = HashCalculator->Num_Hash_Values();
	
	unsigned int lasthash = 0xFFFFFFFF;
	unsigned int hash;
	
	for (int hidx = 0; hidx < num_hash_vals; hidx++) {
		hash = HashCalculator->Get_Hash_Value(hidx);
		if (hash != lasthash) {
			
			int test_item_index = HashTable[hash];

			while (test_item_index != 0xFFFFFFFF) {
				if (HashCalculator->Items_Match(UniqueItems[test_item_index].Item,new_item)) {
					return test_item_index;
				}
				test_item_index = UniqueItems[test_item_index].NextHashIndex;
			}
		}
		lasthash = hash;
	}

	/*
	** Ok, this is a new item so add it (copy it!) into the array
	*/
	int index = UniqueItems.Count();
	int hash_index = HashCalculator->Get_Hash_Value(0);

	HashItem entry;
	entry.Item = new_item;
	entry.NextHashIndex = HashTable[hash_index];
	HashTable[hash_index] = index;
	
	UniqueItems.Add(entry);

	return index;
}


#endif // UARRAY_H

