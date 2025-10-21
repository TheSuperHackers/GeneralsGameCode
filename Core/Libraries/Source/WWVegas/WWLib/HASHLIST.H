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

/* $Header: /Commando/Code/wwlib/hashlist.h 13    4/30/01 3:05p Joe_b $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/hashlist.h                             $*
 *                                                                                             *
 *                      Creator::Scott K. Bowen - 5/4/99                                      *
 *                                                                                             *
 *                      $Author:: Joe_b                                                       $*
 *                                                                                             *
 *                     $Modtime:: 4/30/01 2:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   HashListClass::Find -- Find record in hash table.                                         *
 *   HashListClass::Remove -- Remove record from hash table.                                   *
 *   HashListClass::Add -- Add record to hash list.                                            *
 *   HashListClass::Add -- Add record to list, list creates node for user.                     *
 *   HashListClass::Move_To -- Move nodes from one list to another.                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef HASHLIST_H
#define HASHLIST_H

#pragma warning (push)
#pragma warning (disable: 4786)

#include "LISTNODE.H"
#include <memory.h>

#ifndef NULL
#define NULL (0L)
#endif

#define A_LARGE_PRIME_NUMBER	257

// HashListClass<> and HashNodeClass<>:
// 	The purpose of these templates is to allow a user to insert records into
// a hash table that also contains a double linked list that includes every
// record in the hash table.  The functionality of the hash list mirrors
// very closely that of the DataNode<> and List<> templates.  The differences
// between them are:
// 	-User calls Add(?) to add to list instead of Add_Head() or others.
// 	-A key must be set to hash off of.
//
// Additional features of the HashList<> over List<> are:
//		-Quick searching as long as key's are well hashed..
//		-Node can inherit user class so additional data can be put in node.
//		 For example a timestamp to be set by user. Default is
//		 HashDefaluatUserClass that has no data.
//		-If class T is actually a class (not a pointer), user may get a ptr
//		 to class with Get_Ptr() instead of Get() that returns an INSTANCE
//		 of the class.
//		-Flags to tell user if record was just created or record was just put
//		 into a list (user must clear flags if desired.)
//		-HashListClass<> can create and destroy nodes so that the nodes do not
//		 need to be put as members in classes.  This allows an object to be
//		 put into an infinite (subject to memory constraints) lists.
//		-User may specify number of hash node slots (default is A_LARGE_PRIME_NUMBER).

// You can typedef these templates with the the following:
// 	typedef HashListClass<TestHashClass *> HashTableClass;
// 	typedef HashNodeClass<TestHashClass *> HashTableNodeClass;
// or if using a UserClass
// 	typedef HashListClass<TestHashClass *, 4, UserClass> HashTableClass;
// 	typedef HashNodeClass<TestHashClass *, UserClass> HashTableNodeClass;


template<class T, class U> class HashNodeClass;
template<class T, class U> class HashNodeFriendClass;
template<class T, class U, int NumHashValues> class HashListClass;


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// This is a default class that is stored in HashNodeClass.  It's purpose is
// to allow the user to override it with his/her own class so that additional
// data may be stored in the node about the object.
// Since the object might be in various lists, this data is specific to
// the list that it in.
class HashDefaultUserClass  {};

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, class U = class HashDefaultUserClass>
class HashNodeClass : public DataNode<HashNodeClass<T,U> *>, public U
{
	public:
		// Creation when key and record are known.
		HashNodeClass(T record, unsigned key) :
			Flags(0),
			Record(record),
			Key(key)
		{
			DataNode<HashNodeClass<T,U> *>::Set(this);
			Flag.NewRecord = true;
			Flag.KeySet = true;
		}

		// Creation when key and record are to be set later.
		// node may not be put into list until the key is set.
		HashNodeClass() :
			Flags(0),
			Key(-1)
		{
			DataNode<HashNodeClass<T,U> *>::Set(this);
			Flag.NewRecord = true;
		}

		virtual ~HashNodeClass() {
			// Assert is raised when user tries to delete HashNodeClass still in a list
			// NOTE: This might be raised because user expected node to clean
			// 		itself up when deleted.  Currently, this feature is not available
			//			because it would require an additional pointer  to the HashListClass
			//			that the node is a member of.
			assert(!Flag.InList);
		}

		// Get next and prev record given a node.
		// A node may be retrieved from: HashListClass::Find();
		HashNodeClass<T,U> *Next()  {
			return((HashNodeClass<T,U> *) DataNode<HashNodeClass<T,U> *>::Next());
		}
		HashNodeClass<T,U> *Prev()  {
			return((HashNodeClass<T,U> *) DataNode<HashNodeClass<T,U> *>::Prev());
		}

		// User may use these if he desires not to have to check for valid.
		HashNodeClass<T,U> *Next_Valid()  {
			HashNodeClass<T,U> *next = Next();
			if (next && next->Is_Valid()) {
				return(next);
			}
			return(NULL);
		}
		HashNodeClass<T,U> *Prev_Valid()  {
			HashNodeClass<T,U> *prev = Prev();
			if (prev && prev->Is_Valid()) {
				return(prev);
			}
			return(NULL);
		}

		// Get record that is in hash table.
		T Get_Record() {return(Record);}
		T Get_Record_Ptr() {return(&Record);}

		// Overload DataNode::Get() to get the record we are pointing to so
		// that it behaves how people expect List to operate.
		// To Get HashNodeClass - it is this.
		T Get() {return(Record);}

		// Get a pointer to Record to avoid a copy construction of Record if
		// it is an instance of a class that does not want to be copied all
		// over the place.
		T *Get_Ptr() {return(&Record);}

		// To mirror usage of DataNode().
		void Set(T record) {Record = record;}

		void Set_Key(unsigned key) {
			// Can't change key once in a list, we would never find it.
			assert(!Flag.InList);
			Flag.KeySet = true;
			Key = key;
		}

		// Get key of record for has table.
		unsigned Get_Key() {
			assert(Flag.KeySet);
			return (Key);
		}

		// Enable user to know if a record has just been added to the list.
		// It stays set until user clears it - not required.
		int Is_New_Record()		{return(Flag.NewRecord);}
		void Clear_New_Record() {Flag.NewRecord = false;}

		// Each time object gets put into a list NewInList is set.
		// Stays set until user clears - not required.
		int Is_New_In_List()		{return(Flag.NewInList);}
		void Clear_New_In_List() {Flag.NewInList = false;}

		bool Is_In_List()  		{return(Flag.InList);}
		bool Is_Key_Set()  		{return(Flag.KeySet);}


	protected:
		// Record that we are keeping in hash table.
		// This is commonly a pointer to the actual record.
		T							Record;

		// Key that user gives to identify record.
		// WARNING: Duplicate keys are undefined behavior.
		unsigned						Key;

		// Unioned flags so Flags can be initialized to 0.
		union {
			struct {
				// When record collides in hash list, these ?InTable
				// tell when to stop looking for object in hash entry.
				unsigned					FirstInTable:1;
				unsigned					LastInTable:1;

				// This is set when record is first created.  It is up to user
				// to clear if he uses flag.
				unsigned					NewRecord:1;

				// This is set when record is first put in list.  It is up to user
				// to clear if he uses flag.
				unsigned					NewInList:1;

				// Certain functions may not happen if node is InList (in hash table)
				// These include delete, Set_Key(), and possibly others.
				unsigned					InList:1;

				// This is set so HashListClass knows if it should delete the
				// node when removed from it's list.
				unsigned					ListCreated:1;

				// Make sure someone does not insert a node into a list
				// without setting it's key.
				unsigned					KeySet:1;
			} Flag;
			unsigned Flags;
		};

	private:
		// Not something the casual user can call.
		void Unlink(void) {
			DataNode<HashNodeClass<T,U> *>::Unlink();
		}

		friend class HashNodeFriendClass<T,U>;
};


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// This class is created so that HashListClass can access HashNodeClass.
// HashListClass cannot access directly because it is impossible
// (I could not figure out how) to tell HashNodeClass to be a friend
// to all creations of HashListClass<T,N> since HashNodeClass
// does not need the NumHashValues parameter.
#pragma warning(push)
#pragma warning(disable : 4786) // identifier was truncated to 255 chars in debug information.

template<class T, class U>
class HashNodeFriendClass
{
	protected:
		void Set_In_List(HashNodeClass<T,U> *ptr)  	{ptr->Flag.InList = true; ptr->Flag.NewInList = true;}
		void Clear_In_List(HashNodeClass<T,U> *ptr)  {ptr->Flag.InList = false; ptr->Flag.NewInList = false;}

		bool Is_First(HashNodeClass<T,U> *ptr)		{return(ptr->Flag.FirstInTable);}
		bool Is_Last(HashNodeClass<T,U> *ptr)		{return(ptr->Flag.LastInTable); }

		void Set_First(HashNodeClass<T,U> *ptr)	{ptr->Flag.FirstInTable = true; }
		void Set_Last(HashNodeClass<T,U> *ptr)		{ptr->Flag.LastInTable = true;  }

		void Clear_First(HashNodeClass<T,U> *ptr)	{ptr->Flag.FirstInTable = false;}
		void Clear_Last(HashNodeClass<T,U> *ptr)	{ptr->Flag.LastInTable = false; }

		void Set_List_Created(HashNodeClass<T,U> *ptr)	{ptr->Flag.ListCreated = true; }
		void Clear_List_Created(HashNodeClass<T,U> *ptr)	{ptr->Flag.ListCreated = false; }
		bool Is_List_Created(HashNodeClass<T,U> *ptr)	{return(ptr->Flag.ListCreated);}

		void Unlink(HashNodeClass<T,U> *ptr)  {ptr->Unlink();}
};

#pragma warning(pop)

// The sole purpose of using NumHashValues as a template is to make it easier to
// view in a debugger.  If it were an allocated array of pointers, the debugger does
// not easily show each element in the array.
template<class T, class U = class HashDefaultUserClass, int NumHashValues = A_LARGE_PRIME_NUMBER>
class HashListClass : protected HashNodeFriendClass<T,U>
{
	public:
		// Creation and destruction.
		HashListClass() :
			List(),NumRecords(0), UsedValues(0)
		{
			memset(HashTable, 0, sizeof(HashTable));
		}
		~HashListClass() {Delete();}

		// Get first node in list so user can itterate through it.
		// CAUTION: Do not destroy pointer returned or itterated though -
		//				you must call the Remove function on it.
		//				An assert() will come up if you try.
		HashNodeClass<T,U> *First()  {
			return((HashNodeClass<T,U> *)List.First());
		}
		HashNodeClass<T,U> *First_Valid()  {
			return((HashNodeClass<T,U> *)List.First_Valid());
		}
		HashNodeClass<T,U> *Last()  {
			return((HashNodeClass<T,U> *)List.Last());
		}
		HashNodeClass<T,U> *Last_Valid()  {
			return((HashNodeClass<T,U> *)List.Last_Valid());
		}


		// Add a record to hash creating new HashNodeClass.
		HashNodeClass<T,U> *Add(T record, unsigned key);

		// Add an existing node not in a list to this list.
		// This allows closer behavior to List class.
		HashNodeClass<T,U> *Add(HashNodeClass<T,U> *node);

		// Search for record based off key in hash table.
		HashNodeClass<T,U> *Find(unsigned key);

		// Use this remove if you know the node * already. It is quicker.
		void Remove(HashNodeClass<T,U> *node);

		// Use this remove if you need to do a find to get the node.
		// OK if key does not exist in table.
		bool Remove(unsigned key);

		// Removes all nodes in list.
		void Delete(void) {while (First()->Is_Valid()) Remove(First());}

		// Move contents of one list to another (this becomes empty).
		void Move_To(HashListClass<T,U> *newlist);

		// So we always do it the same way.
		unsigned Hash_Value(unsigned key)  {
			return(unsigned)(key % NumHashValues);
		}

		// Total number of records in hash table.
		int Num_Records()  {
			return(NumRecords);
		}
		// how many hash values are used.
		// The closer this number is to NumRecords, the
		// better hashing going on.
		int Num_Used_Values()  {
			return(UsedValues);
		}

	protected:
		// This list can be walked from start to end of all objects in the list.
		// User can use Get_First() to get first (valid) record in linked list.
		List<DataNode<T> *>		List;

		// Points to first record in List that has the right 'value'.
		// Must check LaastInTable to see if end of records in has list.
		HashNodeClass<T,U>		*HashTable[NumHashValues];

		// Number of records we have in class.
		int							NumRecords;
		int							UsedValues;

};

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

/***********************************************************************************************
 * HashListClass::Add -- Add record to hash list.                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
template <class T, class U, int NumHashValues>
HashNodeClass<T,U> *HashListClass<T, U, NumHashValues>::Add(HashNodeClass<T,U> *node)
{
	// Get our index into the hash table.
	int hashidx = Hash_Value(node->Get_Key());
	assert(hashidx < NumHashValues);

	// Tell system we are being put in the list (and make sure we weren't already in one).
	assert(!node->Is_In_List());
	Set_In_List(node);

	// Get first hit in table.
	HashNodeClass<T,U> *first = HashTable[hashidx];
	if (first) {
		// This will add the new node right after the first node in the list.
		first->Link(node);

		// If we are the first one to collide, then we will be at end of list.
		// otherwise, we are at the end.
		if (Is_Last(first)) {
			Clear_Last(first);
			Set_Last(node);
		}
	} else {
		// We are first hit at hash index.  Put ourselves at start of
		List.Add_Head(node);
		Set_First(node);
		Set_Last(node);
		HashTable[hashidx] = node;
		UsedValues++;
	}
	NumRecords++;
	return (node);
}


/***********************************************************************************************
 * HashListClass::Add -- Add record to list, list creates node for user.                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/03/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
template <class T, class U, int NumHashValues>
HashNodeClass<T,U> *HashListClass<T, U, NumHashValues>::Add(T record, unsigned key)
{
	// Create new node so we can add our record.
	HashNodeClass<T,U> *node = W3DNEW HashNodeClass<T,U>(record, key);
	Set_List_Created(node);

	// Now let the other add do all the work.
	Add(node);
	return (node);
}

/***********************************************************************************************
 * HashListClass::Find -- Find record in hash table.                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
template <class T, class U, int NumHashValues>
HashNodeClass<T,U> *HashListClass<T, U, NumHashValues>::Find(unsigned key)
{
	int hashidx = Hash_Value(key);
	assert(hashidx < NumHashValues);

	HashNodeClass<T,U> *cur = HashTable[hashidx];
	if (cur) for(;;) {
		// Have we found our match?
		if (cur->Get_Key() == key) {
			return(cur);
		}

		// We have to find record before the end of the list.
		if (Is_Last(cur))  {
			break;
		}

		// Continue on because we appear to have some collisions.
		cur = cur->Next();

		// The end of a list should always be marked with Is_Last().
		assert(cur);
		assert(cur->Is_Valid());
	}
	return(NULL);

}

/***********************************************************************************************
 * HashListClass::Remove_Node -- Remove node from hash table.                                  *
 *                               Protected function for usage by Remove() routines.            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *   HashNodeClass<T,U> *node - node to remove.                                                *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1999 SKB : Created.                                                                 *
 *   01/19/2000 SKB : Remove the node with Unlink.                                             *
 *=============================================================================================*/
template <class T, class U, int NumHashValues>
void HashListClass<T, U, NumHashValues>::Remove(HashNodeClass<T,U> *node)
{
	assert(node);
	assert(node->Is_In_List());

	// See if entry is first in list for hash value.
	if (Is_First(node)) {

		int hashidx = Hash_Value(node->Get_Key());
		assert(HashTable[hashidx]);

		// SKB: 2/20/01 - clear incase inserted in new list later.
		Clear_First(node);

		// is it only one in hash index?
		if (Is_Last(node)) {
			// SKB: 2/20/01 - clear incase inserted in new list later.
			Clear_Last(node);
			HashTable[hashidx] = NULL;
			UsedValues--;
		} else {
			HashTable[hashidx] = node->Next_Valid();
			Set_First(HashTable[hashidx]);
		}

	} else if (Is_Last(node)) {
		// SKB: 2/20/01 - clear incase inserted in new list later.
		Clear_Last(node);

		// There is one before us because it failed above.
		Set_Last(node->Prev());
	}


	// All done with you.  Set flag to avoid assert.
	Clear_In_List(node);

	// Delete node if it was created by the this class instead of by the user.
	if (Is_List_Created(node)) {
		delete node;
	} else {
		// Remove it from the link list at least so it can be put in another.
		Unlink(node);
	}
	NumRecords--;
}

/***********************************************************************************************
 * HashListClass::Remove -- Remove record from hash table.                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *   	unsigned key - key to find node by.                                                           *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
template <class T, class U, int NumHashValues>
bool HashListClass<T, U, NumHashValues>::Remove(unsigned key)
{
	int hashidx = Hash_Value(key);
	assert(hashidx < NumHashValues);

	HashNodeClass<T,U> *node = Find(key);
	if (node) {
		Remove(node);
		return(true);
	}
	return(false);
}

/***********************************************************************************************
 * HashListClass::Move_To -- Move nodes from one list to another.                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/04/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
template <class T, class U, int NumHashValues>
void HashListClass<T, U, NumHashValues>::Move_To(HashListClass<T,U> *newlist)
{
	HashNodeClass<T,U> *node = First_Valid();
	while (node) {
		HashNodeClass<T,U> *next = node->Next_Valid();

		// If list created node, clear that fact for a moment so it
		// will not get deleted in the remove.
		if (Is_List_Created(node)) {
			Clear_List_Created(node);
			Remove(node);
			newlist->Add(node);
			Set_List_Created(node);
		} else {
			Remove(node);
			newlist->Add(node);
		}
		node = next;
	}
}

#pragma warning (pop)

#endif // HASHLIST_H


















