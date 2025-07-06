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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/ntree.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/18/00 7:02p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __NTREE_H
#define __NTREE_H


#include "refcount.h"
#include "wwstring.h"


//////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////
template<class T> class NTreeLeafClass;
template<class T> class SortedNTreeLeafClass;


//////////////////////////////////////////////////////////////////////////
//
//	NTreeClass
//
//////////////////////////////////////////////////////////////////////////
template<class T>
class NTreeClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	NTreeClass (void)
		:	m_Root (NULL)			{ }
	virtual ~NTreeClass (void)	{ Reset (); }
	
	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	virtual NTreeLeafClass<T> *	Add (const T &value);

	NTreeLeafClass<T> *				Peek_Root (void) { return m_Root; }
	virtual void						Reset (void);

protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	virtual NTreeLeafClass<T> *	Allocate_Leaf (void)	{ return W3DNEW NTreeLeafClass<T>; }

	//////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////
	NTreeLeafClass<T> *	m_Root;
};


/////////////////////////////////////////////////////////
//	Add
/////////////////////////////////////////////////////////
template<class T>
NTreeLeafClass<T> *NTreeClass<T>::Add (const T &value)
{
	NTreeLeafClass<T> *retval = NULL;

	if (m_Root == NULL) {
		
		//
		//	Allocate a new root node
		//
		m_Root = Allocate_Leaf ();
		m_Root->Set_Value (value);
		retval = m_Root;
		
	} else {

		//
		//	Simply add this value to list
		//
		retval = m_Root->Add (value);
	}

	return retval;
}


/////////////////////////////////////////////////////////
//	Reset
/////////////////////////////////////////////////////////
template<class T>
void NTreeClass<T>::Reset (void)
{
	if (m_Root != NULL) {

		//
		//	Find the last leaf in the root
		//
		NTreeLeafClass<T> *end_leaf = m_Root;
		while (end_leaf->Peek_Next () != NULL) {
			end_leaf = end_leaf->Peek_Next ();
		}
		
		//
		//	Remove all the top-level leaves from the tree.
		// Note: We work backwards from last root-leaf so each
		// leaf along the way is guarenteed to have at least 1
		// reference count on it.
		//
		for (NTreeLeafClass<T> *leaf = end_leaf; leaf != NULL; ) {
			NTreeLeafClass<T> *curr_leaf = leaf;
			leaf = leaf->Peek_Prev ();
			
			//
			//	Remove this leaf
			//
			curr_leaf->Remove ();
		}

		REF_PTR_RELEASE (m_Root);
	}
	return ;
}



//////////////////////////////////////////////////////////////////////////
//
//	SortedNTreeClass
//
//////////////////////////////////////////////////////////////////////////
template<class T>
class SortedNTreeClass : public NTreeClass<T>
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	SortedNTreeClass (void)				{ }
	virtual ~SortedNTreeClass (void)	{ }
	
	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	SortedNTreeLeafClass<T> *	Add_Sorted (const T &value, const char *name);

protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	NTreeLeafClass<T> *			Allocate_Leaf (void)	{ return W3DNEW SortedNTreeLeafClass<T>; }
};

/////////////////////////////////////////////////////////
//	Add_Sorted
/////////////////////////////////////////////////////////
template<class T>
SortedNTreeLeafClass<T> *SortedNTreeClass<T>::Add_Sorted (const T &value, const char *name)
{
	SortedNTreeLeafClass<T> *retval = NULL;

	if (m_Root == NULL) {
		
		//
		//	Allocate a new root node
		//
		m_Root = Allocate_Leaf ();
		
		retval = (SortedNTreeLeafClass<T> *)m_Root;
		retval->Set_Value (value);
		retval->Set_Name (name);		
		
	} else {

		//
		//	Simply add this value to list
		//
		retval = ((SortedNTreeLeafClass<T> *)m_Root)->Add_Sorted (value, name);

		//
		//	Make sure our 'root' pointer is the first one in the list
		//
		NTreeLeafClass<T> *prev = m_Root->Peek_Prev ();
		if (prev != NULL) {			
			REF_PTR_SET (m_Root, prev);
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////
//
//	NTreeLeafClass
//
//////////////////////////////////////////////////////////////////////////
template<class T>
class NTreeLeafClass : public RefCountClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	NTreeLeafClass (void)
		:	m_Parent (NULL),
			m_Child (NULL),
			m_PrevSibling (NULL),
			m_NextSibling (NULL)			{ }

	virtual ~NTreeLeafClass (void);
	
	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	
	//
	//	Tree management
	//
	virtual NTreeLeafClass<T> *	Add_Child (const T &value);
	virtual NTreeLeafClass<T> *	Add (const T &value);
	virtual void						Remove (void);

	//
	//	Value accessors
	//
	virtual const T &			Get_Value (void) const		{ return m_Data; }
	virtual void				Set_Value (const T &data)	{ m_Data = data; }
	
	//
	//	Tree traversal methods
	//
	NTreeLeafClass<T> *		Peek_Parent (void)	{ return m_Parent; }
	NTreeLeafClass<T> *		Peek_Child (void)		{ return m_Child; }
	NTreeLeafClass<T> *		Peek_Next (void)		{ return m_NextSibling; }
	NTreeLeafClass<T> *		Peek_Prev (void)		{ return m_PrevSibling; }

protected:

	//////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////
	void						Set_Parent (NTreeLeafClass<T> *parent)	{ REF_PTR_SET (m_Parent, parent); }
	void						Set_Child (NTreeLeafClass<T> *child)	{ REF_PTR_SET (m_Child, child); }
	void						Set_Prev (NTreeLeafClass<T> *prev)		{ REF_PTR_SET (m_PrevSibling, prev); }
	void						Set_Next (NTreeLeafClass<T> *next)		{ REF_PTR_SET (m_NextSibling, next); }	

	//////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////
	NTreeLeafClass<T> *	m_Parent;
	NTreeLeafClass<T> *	m_Child;
	NTreeLeafClass<T> *	m_NextSibling;
	NTreeLeafClass<T> *	m_PrevSibling;

	T							m_Data;
};


/////////////////////////////////////////////////////////
//	~NTreeLeafClass
/////////////////////////////////////////////////////////
template<class T>
NTreeLeafClass<T>::~NTreeLeafClass (void)
{
	REF_PTR_RELEASE (m_Parent);
	REF_PTR_RELEASE (m_Child);
	REF_PTR_RELEASE (m_NextSibling);
	REF_PTR_RELEASE (m_PrevSibling);
	return; 
}

/////////////////////////////////////////////////////////
//	Add_Child
/////////////////////////////////////////////////////////
template<class T>
NTreeLeafClass<T> *NTreeLeafClass<T>::Add_Child (const T &value)
{
	//
	//	Allocate a new leaf object
	//
	NTreeLeafClass<T> *new_child = W3DNEW NTreeLeafClass<T>;
	new_child->Set_Value (value);
	new_child->Set_Parent (this);
	
	//
	//	Link this new leaf into the hierarchy
	//
	if (m_Child != NULL) {
		m_Child->Set_Prev (new_child);
		new_child->Set_Next (m_Child);
	}
	m_Child = new_child;
	return m_Child;
}


/////////////////////////////////////////////////////////
//	Add
/////////////////////////////////////////////////////////
template<class T>
NTreeLeafClass<T> *NTreeLeafClass<T>::Add (const T &value)
{
	//
	//	Allocate a new leaf object
	//
	NTreeLeafClass<T> *new_leaf = W3DNEW NTreeLeafClass<T>;
	new_leaf->Set_Value (value);

	//
	//	Link this new leaf into the list
	//
	new_leaf->Set_Prev (this);
	new_leaf->Set_Next (m_NextSibling);
	Set_Next (new_leaf);

	//
	//	Release our hold on the new leaf
	//
	new_leaf->Release_Ref ();
	return new_leaf;
}


/////////////////////////////////////////////////////////
//	Remove
/////////////////////////////////////////////////////////
template<class T>
void NTreeLeafClass<T>::Remove (void)
{
	Add_Ref ();

	//
	//	Fixup the parent's child leaf object
	//
	if (m_Parent != NULL && m_Parent->Peek_Child () == this) {
		m_Parent->Set_Child (m_NextSibling);
	}

	//
	//	Remove all our children
	//
	while (m_Child != NULL) {
		m_Child->Remove ();
	}

	//
	//	Unlink ourselves from our siblings
	//
	if (m_NextSibling != NULL) {
		m_NextSibling->Set_Prev (NULL);
	}

	if (m_PrevSibling != NULL) {
		m_PrevSibling->Set_Next (NULL);
	}

	REF_PTR_RELEASE (m_Parent);
	REF_PTR_RELEASE (m_Child);
	REF_PTR_RELEASE (m_NextSibling);
	REF_PTR_RELEASE (m_PrevSibling);

	Release_Ref ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	SortedNTreeLeafClass
//
//////////////////////////////////////////////////////////////////////////
template<class T>
class SortedNTreeLeafClass : public NTreeLeafClass<T>
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	SortedNTreeLeafClass (void)		{ }
	~SortedNTreeLeafClass (void)		{ }
	
	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	SortedNTreeLeafClass<T> *	Add_Sorted (const T &value, const char *name);
	SortedNTreeLeafClass<T> *	Add_Child_Sorted (const T &value, const char *name);

	const StringClass &	Get_Name (void) const			{ return m_Name; }
	void						Set_Name (const char *name)	{ m_Name = name; }

protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	void						Insertion_Sort (SortedNTreeLeafClass<T> *start, SortedNTreeLeafClass<T> *new_sibling);

	//////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////
	StringClass				m_Name;
};


/////////////////////////////////////////////////////////
//	Add_Sorted
/////////////////////////////////////////////////////////
template<class T>
SortedNTreeLeafClass<T> *SortedNTreeLeafClass<T>::Add_Sorted (const T &value, const char *name)
{
	//
	//	Allocate a new leaf object
	//
	SortedNTreeLeafClass<T> *new_sibling = W3DNEW SortedNTreeLeafClass<T>;
	new_sibling->Set_Value (value);
	new_sibling->Set_Name (name);

	//
	//	Find the first-most sibling
	//
	SortedNTreeLeafClass<T> *start = this;
	while (start->Peek_Prev () != NULL)	{
		start = (SortedNTreeLeafClass<T> *)start->Peek_Prev ();
	}

	//
	//	Insert the new leaf in its sorted position
	//
	Insertion_Sort (start, new_sibling);

	//
	//	Release our hold on the object pointer
	//
	new_sibling->Release_Ref ();
	return new_sibling;
}

/////////////////////////////////////////////////////////
//	Add_Child_Sorted
/////////////////////////////////////////////////////////
template<class T>
SortedNTreeLeafClass<T> *SortedNTreeLeafClass<T>::Add_Child_Sorted (const T &value, const char *name)
{
	//
	//	Allocate a new leaf object
	//
	SortedNTreeLeafClass<T> *new_child = W3DNEW SortedNTreeLeafClass<T>;
	new_child->Set_Value (value);
	new_child->Set_Name (name);
	new_child->Set_Parent (this);

	if (m_Child == NULL) {
		m_Child = new_child;
	} else {

		//
		//	Insert the new leaf in its sorted position
		//
		Insertion_Sort ((SortedNTreeLeafClass<T> *)m_Child, new_child);

		//
		//	Make sure our 'child' pointer is the first one in the list
		//
		NTreeLeafClass<T> *prev = m_Child->Peek_Prev ();
		if (prev != NULL) {			
			REF_PTR_SET (m_Child, prev);
		}
		
		//
		//	Release our hold on the object pointer
		//
		new_child->Release_Ref ();
	}

	return new_child;
}

/////////////////////////////////////////////////////////
//	Insertion_Sort
/////////////////////////////////////////////////////////
template<class T>
void SortedNTreeLeafClass<T>::Insertion_Sort (SortedNTreeLeafClass<T> *start, SortedNTreeLeafClass<T> *new_sibling)
{
	const char *name = new_sibling->Get_Name ();

	//
	//	Determine where to insert the new sibling
	//
	bool inserted = false;
	for (	SortedNTreeLeafClass<T> *leaf = start;
			leaf != NULL && !inserted;
			leaf = (SortedNTreeLeafClass<T> *)leaf->Peek_Next ())
	{
		//
		//	Does the new sibling come before the current leaf?
		//
		if (::stricmp (name, leaf->Get_Name ()) < 0) {				

			//
			//	Insert this sibling before the leaf
			//
			SortedNTreeLeafClass<T> *prev = (SortedNTreeLeafClass<T> *)leaf->Peek_Prev ();
			new_sibling->Set_Prev (prev);
			new_sibling->Set_Next (leaf);
			leaf->Set_Prev (new_sibling);
			if (prev != NULL) {
				prev->Set_Next (new_sibling);
			}

			inserted = true;
			
		} else if (leaf->Peek_Next () == NULL) {

			//
			//	Put the new sibling on the end of the list
			//
			new_sibling->Set_Prev (leaf);
			leaf->Set_Next (new_sibling);
			inserted = true;
		}
	}

	return ;
}

#endif //__NTREE_H
