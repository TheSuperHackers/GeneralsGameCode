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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando					                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/hashtemplate.h                         $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 11/19/01 12:16p                                             $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef HASH_TEMPLATE_H
#define HASH_TEMPLATE_H

#include "always.h"
#include "wwstring.h"

// Class for providing hash values

template <class Key> class HashTemplateKeyClass
{
public:
	static inline unsigned int Get_Hash_Value (const Key& k);
};

// Default hash function for data types that can be cast into an unsigned int
template <class Key> inline unsigned int HashTemplateKeyClass<Key>::Get_Hash_Value (const Key& k)
{
	unsigned int hval = *((const unsigned int*)(&k));
	hval = hval + (hval>>5) + (hval>>10) + (hval >> 20);
	return hval;
}

// Specialization for floating point hash values (as the default dword-
// casting yields a very bad hash function)
template <> inline unsigned int HashTemplateKeyClass<float>::Get_Hash_Value (const float& s)
{
	unsigned int z = *((const unsigned int*)(&s));
	return ((z>>22)+(z>>12)+(z));
}

// Hash class
template <class KeyType, class ValueType>
class HashTemplateClass
{
	struct Entry;
public:

	enum
	{
		NIL = -1								// internal enumeration for representing a NULL link
	};

	HashTemplateClass(void);
	~HashTemplateClass(void);

	void Insert(const KeyType& s, const ValueType& d);
	void Set_Value(const KeyType& s, const ValueType& d);
	void Remove(const KeyType& s);
	void Remove(const KeyType& s, const ValueType& d);
	ValueType Get(const KeyType& s) const;
	bool Get(const KeyType& s, ValueType& d) const;
	bool Exists(const KeyType& s) const;
	bool Exists(const KeyType& s, const ValueType& d) const;
	void Remove_All(void);
	unsigned int Get_Size(void) const;

	int* Get_Hash() { return Hash; }
	Entry* Get_Table() { return Table; }

private:
	HashTemplateClass (const HashTemplateClass&);	// not allowed
	HashTemplateClass& operator= (const HashTemplateClass&);	// not allowed
	static unsigned int Get_Hash_Val(const KeyType& s, const unsigned int hash_array_size);
	void Re_Hash(void);

	int Alloc_Entry(void);

	struct Entry
	{
		int Next;						// next pointer in linked list
		KeyType Key;							// key
		ValueType Value;					// value
	};

	int* Hash;							// hash pointers
	Entry* Table;						// allocation table
	int First;							// handle of first free entry
	unsigned int Size;				// size of hash table
};

template <class KeyType, class ValueType>
class HashTemplateIterator
{
	int HashIndex;					// index to hash pointer table
	int Handle;
	HashTemplateClass<KeyType,ValueType>& HashTable;

public:
	HashTemplateIterator(HashTemplateClass<KeyType,ValueType>& hash_table)
		:
		HashTable(hash_table)
	{
		First();
	}

	void First()
	{
		Handle=HashTemplateClass<KeyType,ValueType>::NIL;
		int size=HashTable.Get_Size();
		for (HashIndex=0;HashIndex<size;++HashIndex) {
			Handle = HashTable.Get_Hash()[HashIndex];
			if (Handle!=HashTemplateClass<KeyType,ValueType>::NIL) break;
		}
	}

	void Next()
	{
		Handle=HashTable.Get_Table()[Handle].Next;
		if (Handle==HashTemplateClass<KeyType,ValueType>::NIL) {
			int size=HashTable.Get_Size();
			for (++HashIndex;HashIndex<size;++HashIndex) {
				Handle = HashTable.Get_Hash()[HashIndex];
				if (Handle!=HashTemplateClass<KeyType,ValueType>::NIL) break;
			}
		}
	}

	bool Is_Done() const
	{
		return HashIndex==int(HashTable.Get_Size());
	}

	ValueType& Peek_Value()
	{
		return HashTable.Get_Table()[Handle].Value;
	}

	const KeyType& Peek_Key()
	{
		return HashTable.Get_Table()[Handle].Key;
	}

};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

template <class KeyType, class ValueType> inline void HashTemplateClass<KeyType,ValueType>::Insert(const KeyType& s, const ValueType& d)
{
	int h;
	h				= Alloc_Entry();
	unsigned int hval	= Get_Hash_Val(s,Size);
	Table[h].Key	= s;
	Table[h].Value	= d;
	Table[h].Next	= Hash[hval];
	Hash[hval]		= h;
}

template <class KeyType, class ValueType> inline unsigned int HashTemplateClass<KeyType,ValueType>::Get_Size (void) const
{
	return Size;
}

template <class KeyType, class ValueType> inline void HashTemplateClass<KeyType,ValueType>::Remove_All (void)
{
	for (unsigned int i = 0; i < Size; i++)
	{
		int f = Hash[i];
		if (f!=NIL)
		{
			int h = f;
			while (Table[h].Next != NIL)
				h = Table[h].Next;
			Table[h].Next = First;		// link to first free
			First = f;					// link to beginning
			Hash[i] = NIL;				// kill entry in hash
		}
	}
}

template <class KeyType, class ValueType> inline void HashTemplateClass<KeyType,ValueType>::Remove	(const KeyType& s)
{
	if (!Hash) return;
	unsigned int hval = Get_Hash_Val(s,Size);
	int  prev = NIL;
	int	h	 = Hash[hval];

	while (h != NIL)
	{
		if (Table[h].Key == s)
		{
			if (prev!=NIL)
				Table[prev].Next = Table[h].Next;
			else
				Hash[hval] = Table[h].Next;
			Table[h].Next = First;
			First = h;
			return;
		}
		prev = h;
		h    = Table[h].Next;
	}
}

template <class KeyType, class ValueType> inline void HashTemplateClass<KeyType,ValueType>::Remove (const KeyType& s, const ValueType& d)
{
	if (!Hash) return;
	unsigned int hval = Get_Hash_Val(s,Size);
	int  prev = NIL;
	int	h	 = Hash[hval];

	while (h != NIL)
	{
		if (Table[h].Key == s && Table[h].Value == d)
		{
			if (prev!=NIL)
				Table[prev].Next = Table[h].Next;
			else
				Hash[hval] = Table[h].Next;
			Table[h].Next = First;
			First = h;
			return;
		}
		prev = h;
		h    = Table[h].Next;
	}
}

// Set the value at existing key, or if not found insert a new value
template <class KeyType, class ValueType> inline void HashTemplateClass<KeyType,ValueType>::Set_Value (const KeyType& s, const ValueType& v)
{
	if (Hash) {
		int  h = Hash[Get_Hash_Val(s,Size)];
		while (h!=NIL)	{
			if (Table[h].Key == s) {
				Table[h].Value=v;
				return;
			}
			h = Table[h].Next;
		}
	}
	Insert(s,v);
}

template <class KeyType, class ValueType> inline ValueType HashTemplateClass<KeyType,ValueType>::Get (const KeyType& s) const
{
	if (Hash) {
		int  h = Hash[Get_Hash_Val(s,Size)];
		while (h!=NIL)
		{
			if (Table[h].Key == s)
				return Table[h].Value;
			h = Table[h].Next;
		}
	}
	return ValueType(0);
}

template <class KeyType, class ValueType> inline bool HashTemplateClass<KeyType,ValueType>::Get(const KeyType& s, ValueType& d) const
{
	if (Hash) {
		int  h = Hash[Get_Hash_Val(s,Size)];
		while (h!=NIL)
		{
			if (Table[h].Key == s)
			{
				d = Table[h].Value;
				return true;
			}
			h = Table[h].Next;
		}
	}
	return false;
}

template <class KeyType, class ValueType> inline bool HashTemplateClass<KeyType,ValueType>::Exists(const KeyType& s) const
{
	if (Hash) {
		int  h = Hash[Get_Hash_Val(s,Size)];
		while (h!=NIL)
		{
			if (Table[h].Key == s)
				return true;
			h = Table[h].Next;
		}
	}
	return false;
}

template <class KeyType, class ValueType> inline bool HashTemplateClass<KeyType,ValueType>::Exists(const KeyType& s, const ValueType& d) const
{
	if (Hash) {
		int  h = Hash[Get_Hash_Val(s,Size)];
		while (h!=NIL)
		{
			if (Table[h].Key == s && Table[h].Value == d)
				return true;
			h = Table[h].Next;
		}
	}
	return false;
}

template <class KeyType, class ValueType> inline unsigned int HashTemplateClass<KeyType,ValueType>::Get_Hash_Val(const KeyType& s, const unsigned int hash_array_size)
{
	return HashTemplateKeyClass<KeyType>::Get_Hash_Value (s) & (hash_array_size-1);
}

template <class KeyType, class ValueType> inline void HashTemplateClass<KeyType,ValueType>::Re_Hash()
{
	unsigned int new_size = Size*2;
	if (new_size < 4)
		new_size = 4;

	Entry  *new_table = W3DNEWARRAY Entry[new_size];
	int *new_hash  = W3DNEWARRAY int[new_size];

	int cnt = 0;
	int	i;

	for (i = 0; i < (int)new_size; i++)
	{
		new_table[i].Next	= NIL;
		new_hash[i]			= NIL;
	}

	if (Size)											// if we have existing data, it needs to be rehashed
	{
		for (i = 0; i < (int)Size; i++)						// step through each old hash set
		{
			int	h = Hash[i];
			while (h != NIL)
			{
				unsigned int hVal		= Get_Hash_Val(Table[h].Key, new_size);
				new_table[cnt].Key	= Table[h].Key;
				new_table[cnt].Value = Table[h].Value;
				new_table[cnt].Next	= new_hash[hVal];
				new_hash[hVal]		= cnt;
				cnt++;
				h = Table[h].Next;
			}
		}
		delete[] Hash;
		delete[] Table;
	}

	for (i = cnt; i < (int)new_size; i++)
		new_table[i].Next = i+1;
	new_table[new_size-1].Next = NIL;

	First	= cnt;
	Hash	= new_hash;
	Table	= new_table;
	Size	= new_size;
}

template <class KeyType, class ValueType> inline int HashTemplateClass<KeyType,ValueType>::Alloc_Entry()
{
	if (First == NIL)					// need to re-alloc and re-hash tables
		Re_Hash();
	int h	= First;
	First	= Table[First].Next;
	return h;
}

template <class KeyType, class ValueType> inline HashTemplateClass<KeyType,ValueType>::HashTemplateClass() : Hash(0),Table(0),First(NIL),Size(0)
{
//	Re_Hash();
}

template <class KeyType, class ValueType> inline HashTemplateClass<KeyType,ValueType>::~HashTemplateClass()
{
	delete[] Hash;
	delete[] Table;
}

// Get_Hash_Value specialization for StringClass. This is intended to be used
// for filenames, so it takes into account the four characters that come before
// the file extension (.xxx - extension expected to be 4 characters).

template <> inline unsigned int HashTemplateKeyClass<StringClass>::Get_Hash_Value(const StringClass& s)
{
	unsigned int len=s.Get_Length();
	unsigned const char* buffer=(unsigned const char*)s.str();
	if (len<8) {
		unsigned int hval=0;
		for (unsigned int a=0;a<len;++a) {
			hval+=37*hval+buffer[a];
		}
		return hval;
	}
	unsigned int hval = *((const unsigned int*)(buffer+len-8));
	hval = hval + (hval>>5) + (hval>>10) + (hval >> 20);
	return hval;
}



#endif
