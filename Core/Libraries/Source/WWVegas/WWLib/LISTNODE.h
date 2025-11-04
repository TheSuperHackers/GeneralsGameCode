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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/listnode.h                             $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 9/20/01 9:46p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

/*
** Includes
*/
#include	"assert.h"


#ifdef __BORLANDC__
#pragma warn -inl
#endif


/*
**	This is a doubly linked list node. Typical use of this node is to derive
**	objects from this node. The interface class for this node can be used for
**	added convenience.
*/
class GenericList;
class GenericNode {
	public:
		GenericNode(void) : NextNode(0), PrevNode(0) {}
		virtual ~GenericNode(void) {Unlink();}
		GenericNode(GenericNode & node) {node.Link(this);}
		GenericNode & operator = (GenericNode & node) {
			if (&node != this) {
				node.Link(this);
			}
			return(*this);
		}

		void Unlink(void) {
			// note that this means that the special generic node at the head
			// and tail of the list can not be unlinked.  This is done because
			// the user should never unlink them -- it will destroy the list in
			// an evil way.
			if (Is_Valid()) {
				PrevNode->NextNode = NextNode;
				NextNode->PrevNode = PrevNode;
				PrevNode = 0;
				NextNode = 0;
			}
		}

		GenericList * Main_List(void) const {
			GenericNode const * node = this;
			while (node->PrevNode) {
				node = PrevNode;
			}
			return((GenericList *)this);
		}
		void Link(GenericNode * node) {
			assert(node != (GenericNode *)0);
			node->Unlink();
			node->NextNode = NextNode;
			node->PrevNode = this;
			if (NextNode) NextNode->PrevNode = node;
			NextNode = node;
		}

		GenericNode * Next(void) const {return(NextNode);}
		GenericNode * Next_Valid(void) const {
			return ((NextNode && NextNode->NextNode) ? NextNode : (GenericNode *)0);
		}
		GenericNode * Prev(void) const {return(PrevNode);}
		GenericNode * Prev_Valid(void) const {
			return ((PrevNode && PrevNode->PrevNode) ? PrevNode : (GenericNode *)0);
		}
		bool Is_Valid(void) const {return(this != (GenericNode *)0 && NextNode != (GenericNode *)0 && PrevNode != (GenericNode *)0);}

	protected:
		GenericNode * NextNode;
		GenericNode * PrevNode;
};


/*
**	This is a generic list handler. It manages N generic nodes. Use the interface class
**	to the generic list for added convenience.
*/
class GenericList {
	public:
		GenericList(void) {
			FirstNode.Link(&LastNode);
		}


		virtual ~GenericList(void) {
			while (FirstNode.Next()->Is_Valid()) {
				FirstNode.Next()->Unlink();
			}
		}

		GenericNode * First(void) const {return(FirstNode.Next());}
		GenericNode * First_Valid(void) const
		{
			GenericNode *node = FirstNode.Next();
			return (node->Next() ? node : (GenericNode *)0);
		}

		GenericNode * Last(void) const {return(LastNode.Prev());}
		GenericNode * Last_Valid(void) const
		{
			GenericNode *node = LastNode.Prev();
			return (node->Prev() ? node : (GenericNode *)0);
		}

		bool Is_Empty(void) const {return(!FirstNode.Next()->Is_Valid());}
		void Add_Head(GenericNode * node) {FirstNode.Link(node);}
		void Add_Tail(GenericNode * node) {LastNode.Prev()->Link(node);}
//		void Delete(void) {while (FirstNode.Next()->Is_Valid()) delete FirstNode.Next();}

		int Get_Valid_Count(void) const
		{
			GenericNode * node = First_Valid();
			int counter = 0;
			while(node) {
				counter++;
				node = node->Next_Valid();
			}
			return counter;
		}

	protected:
		GenericNode FirstNode;
		GenericNode LastNode;

	private:
		GenericList(GenericList & list);
		GenericList & operator = (GenericList const &);
};



/*
**	This node class serves only as an "interface class" for the normal node
**	object. In order to use this interface class you absolutely must be sure
**	that the node is the root base object of the "class T". If it is true that the
**	address of the node is the same as the address of the "class T", then this
**	interface class will work. You can usually ensure this by deriving the
**	class T object from this node.
*/
template<class T> class List;
template<class T>
class Node : public GenericNode {
	public:
		List<T> * Main_List(void) const {return((List<T> *)GenericNode::Main_List());}
		T Next(void) const {return((T)GenericNode::Next());}
		T Next_Valid(void) const {return((T)GenericNode::Next_Valid());}
		T Prev(void) const {return((T)GenericNode::Prev());}
		T Prev_Valid(void) const {return((T)GenericNode::Prev_Valid());}
		bool Is_Valid(void) const {return(GenericNode::Is_Valid());}
};


/*
**	This is an "interface class" for a list of nodes. The rules for the class T object
**	are the same as the requirements required of the node class.
*/
template<class T>
class List : public GenericList {
	public:
		List(void) {};

		T First(void) const {return((T)GenericList::First());}
		T First_Valid(void) const {return((T)GenericList::First_Valid());}
		T Last(void) const {return((T)GenericList::Last());}
		T Last_Valid(void) const {return((T)GenericList::Last_Valid());}
		void Delete(void) {while (First()->Is_Valid()) delete First();}

	private:
		List(List<T> const & rvalue);
		List<T> operator = (List<T> const & rvalue);
};


/*

  The DataNode template class allows you to have a node with a peice of data associated with it.
  Commonly used to maintain a list of objects which are not derived from GenericNode.
		- EHC
*/

template<class T>
class DataNode : public GenericNode {
	T Value;
public:
	DataNode() {};
	DataNode(T value) { Set(value); };
	void Set(T value) { Value = value; };
	T Get() const { return Value; };

	DataNode<T> * Next(void) const { return (DataNode<T> *)GenericNode::Next(); }
	DataNode<T> * Next_Valid(void) const { return (DataNode<T> *)GenericNode::Next_Valid(); }
	DataNode<T> * Prev(void) const { return (DataNode<T> *)GenericNode::Prev(); }
	DataNode<T> * Prev_Valid(void) const { return (DataNode<T> *)GenericNode::Prev_Valid(); }
};



/*

	The ContextDataNode template class is an extension of the DataNode class and has an additional
	data value associated with it. Commonly used for tracking the owner of a DataNode.
		- EHC
*/
template<class C, class D>
class ContextDataNode : public DataNode<D> {
	C Context;
public:
	ContextDataNode() {};
	ContextDataNode(C context, D data) { Set_Context(context); Set(data); }
	void Set_Context(C context) { Context = context; };
	C Get_Context() { return Context; };
};


/*
**	A SafeContextDataNode requires the user to supply context and
** Data values to the constructor. This eliminates the problem of
** uninitialized ContextDataNodes. -DRM
*/
template<class C, class D>
class SafeContextDataNode : public ContextDataNode<C,D>
{
public:
	SafeContextDataNode(C context, D data) : ContextDataNode<C,D>(context, data) { }
private:
	// if the compiler complains that it can't access this constructor,
	// you are trying to construct a SafeContextDataNode without providing
	// context and data values. Perhaps you've included a SafeContextDataNode
	// as a member variable, but forgotten to add it to the initializer list
	// in your class constructor.
	// DO NOT change the void constructor to public. That would defeat
	// the purpose of the SafeContextDataNode. -DRM
	SafeContextDataNode();
};


/*
	The DoubleNode class has double everything a DataNode has!
	Each DataNode's data member points to the DoubleNode object
	that owns it. This type of object can therefore be in two distinct
	lists, and when it is destroyed it is removed from both of them
	automatically.

	usage example:

	typedef a symbol for the usage of the DoubleNode class as such:

		typedef DoubleNode<int, bool> DOUBLENODE;

	declare the head of the list as a DataNode with type DOUBLENODE

		List<DOUBLENODE *> PrimaryList; // member of object1
		List<DOUBLENODE *> SecondaryList; // member of object2

	Iterate through the list starting at PrimaryList or SecondaryList.
	These two lists are intended to be located in different
	objects, such as an Active list and an Association list as used in G
	Planet mode.
*/
template<class PRIMARY, class SECONDARY>
class DoubleNode {
	void Initialize() { Primary.Set(this); Secondary.Set(this); };
	PRIMARY PrimaryValue;
	SECONDARY SecondaryValue;

public:
	typedef DoubleNode<PRIMARY, SECONDARY> Type;

	DataNode<Type *> Primary;
	DataNode<Type *> Secondary;

	DoubleNode() { Initialize(); };
	DoubleNode(PRIMARY primary, SECONDARY secondary) { Initialize(); Set_Primary(primary); Set_Secondary(secondary); };
	void Set_Primary(PRIMARY value) { PrimaryValue = value; };
	void Set_Secondary(SECONDARY value) { SecondaryValue = value; };
	PRIMARY Get_Primary() { return PrimaryValue; };
	SECONDARY Get_Secondary() { return SecondaryValue; };
	void Unlink() { Primary.Unlink(); Secondary.Unlink(); };
};
