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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/dllist.h                               $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/21/01 6:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DLLIST_H
#define DLLIST_H


template <class T> class DLNodeClass;

template <class T>
class DLListClass
{
	friend DLNodeClass<T>;
	DLNodeClass<T>* head;
	DLNodeClass<T>* tail;

public:
	DLListClass() : head(0), tail(0) {}
	virtual ~DLListClass() { }

	void Add_Head(DLNodeClass<T>* node);
	void Remove_Head();
	void Add_Tail(DLNodeClass<T>* node);
	void Remove_Tail();

	T* Head() { return static_cast<T*>(head); }
	T* Tail() { return static_cast<T*>(tail); }
	const T* Const_Head() const { return static_cast<const T*>(head); }
	const T* Const_Tail() const { return static_cast<const T*>(tail); }
};

// Destroy-list will call delete for all nodes when the list is destructed. Note that the class doesn't work
// with undeclared pointer types (destructor has to be known).
template <class T>
class DLDestroyListClass : public DLListClass<T>
{
	using DLListClass<T>::Head;

public:
	virtual ~DLDestroyListClass()
	{
		while (T* t=Head()) {		
			delete t;
		}
	}
};

template <class T>
class DLNodeClass : public W3DMPO
{
	// nope, this is an ABC
	//W3DMPO_GLUE(DLNodeClass)

	friend DLListClass<T>;
	DLNodeClass<T>* succ;
	DLNodeClass<T>* pred;
	DLListClass<T>* list;
public:
	DLNodeClass() : succ(0), pred(0), list(0) {}
	~DLNodeClass() { Remove(); }

	void Insert_Before(DLNodeClass<T>* n)
	{
		list=n->list;
		succ=n;
		pred=n->pred;
		if (n->pred) n->pred->succ=this;
		n->pred=this;

		if (list->head==n) {
			list->head=this;
		}
	}

	void Insert_After(DLNodeClass<T>* n)
	{
		list=n->list;
		pred=n;
		succ=n->succ;
		if (n->succ) n->succ->pred=this;
		n->succ=this;

		if (list->tail==n) {
			list->tail=this;
		}
	}

	void Remove()
	{
		if (!list) return;
		if (list->Head()==this) {
			DLListClass<T>* tmp_list=list;
			list=0;
			tmp_list->Remove_Head();
			return;
		}
		if (list->Tail()==this) {
			DLListClass<T>* tmp_list=list;
			list=0;
			tmp_list->Remove_Tail();
			return;
		}
		if (succ) succ->pred=pred;
		if (pred) pred->succ=succ;
		list=0;
	}


	T* Succ() { return static_cast<T*>(succ); }
	T* Pred() { return static_cast<T*>(pred); }
	const T* Const_Succ() const { return static_cast<const T*>(succ); }
	const T* Const_Pred() const { return static_cast<const T*>(pred); }

	DLListClass<T>* List() { return list; }
};


template <class T>
inline void DLListClass<T>::Add_Head(DLNodeClass<T>* n)
{
	n->list=this;
	if (head) {
		n->Insert_Before(head);
		head=n;
	}
	else {
		tail=n;
		head=n;
		n->succ=0;
		n->pred=0;
	}
}

template <class T>
inline void DLListClass<T>::Add_Tail(DLNodeClass<T>* n)
{
	n->list=this;
	if (tail) {
		n->Insert_After(tail);
		tail=n;
	}
	else {
		tail=n;
		head=n;
		n->succ=0;
		n->pred=0;
	}
}

template <class T>
inline void DLListClass<T>::Remove_Head()
{
	if (!head) return;
	DLNodeClass<T>* n=head;
	head=head->Succ();
	if (!head) tail=head;
	else head->pred=0;
	n->Remove();
}

template <class T>
inline void DLListClass<T>::Remove_Tail()
{
	if (!tail) return;
	DLNodeClass<T>* n=tail;
	tail=tail->Pred();
	if (!tail) head=tail;
	else tail->succ=0;
	n->Remove();
}

#endif //DLLIST_H
