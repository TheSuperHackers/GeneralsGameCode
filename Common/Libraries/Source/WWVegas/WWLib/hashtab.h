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
 *                 Project Name : Westwood Library                                             *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Library/hashtab.h                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 7/30/98 10:07a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef HASHTAB_H
#define HASHTAB_H

#if 0

template <class Object,class Key> class NamedObjectHashTableClass
{

public:

	HashTableClass(int initialsize,int growthrate,HashCalculatorClass<Key> * hasher);
	~HashTableClass(void);

	void				Add(Object * new_item,Key * key);
	void				Remove(Object * item,Key * key);

	int				Count(void) const								{ return Items.Count(); }
	Object *			Find(const Key & key) const						

private:

	enum { NO_ITEM = 0xFFFFFFFF };

	class HashItem
	{
	public:
		T * 	Item;
		int	NextHashIndex;

		bool operator == (const HashItem & that) { return ((Item == that.Item) && (NextHashIndex == that.NextHashIndex)); }
		bool operator != (const HashItem & that) { return !(*this == that); }
	};
		
	// Dynamic Vector of the unique items:
	DynamicVectorClass<HashItem>		Items;

	// Hash table:
	int										HashTableSize;
	int *										HashTable;

	// object which does the hashing for the type
	HashCalculatorClass<T> *			HashCalculator;

	friend class VectorClass;
	friend class DynamicVectorClass;
};



template <class Object,class Key>
HashTableClass<Object,Key>::HashTableClass
(
	int initialsize,
	int growthrate,
	HashCalculatorClass<Object> * hasher
)
{

}

template <class Object,class Key>
HashTableClass<Object,Key>::~HashTableClass(void)
{

}

template <class Object,class Key>
Object *	HashTableClass<Object,Key>::Find(Key * key) const
{
	// compute where in the hash table this key would go.
	HashCalculator->Compute_Hash(key);
	int hashval = HashCalculator->Get_Hash_Value(0);

	// now try to find an object which has the same key
	int test_item_index = HashTable[hash];
	while (test_item_index != 0xFFFFFFFF) {
		if (HashCalculator->Items_Match(Items[test_item_index].Item,new_item)) {
			return Items[test_item_index].Object;
		}
		test_item_index = Items[test_item_index].NextHashIndex;
	}

	// couldn't find it
	return NULL;
}

#endif


#endif

