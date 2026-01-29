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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------=
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright(C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:    WSYS Library
//
// Module:
//
// File name:  wsys/List.h
//
// Created:    10/31/01
//
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include <Lib/BaseType.h>

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//================================
// LLNode
//================================
/**
  * Link list node abstraction for WSYS.
*/
//================================

class	LListNode
{
	friend class LList;

	protected:

		LListNode			*m_next;				///< Next node in list
		LListNode			*m_prev;				///< Previous node in list
		Int						m_pri;					///< Node's priority, used for sorting
		void					*m_item;				///< Item we are referencing if any
		Bool					m_autoDelete;		///< destroy() should call delete

	public:

									LListNode();											///< Constructor
		void					remove( void );										///< Removes node from list
		void					insert( LListNode *new_node );		///< Inserts new_node infront of itself
		void					append( LListNode *new_node );		///< Appends new node after itself
		LListNode*		next( void );											///< Returns next node in list
		LListNode*		prev( void );											///< Returns previous node in list
		LListNode*		loopNext( void );									///< Returns next node in list, wrapping round to start of list if necessary
		LListNode*		loopPrev( void );									///< Returns previous node in list, wrapping round to end of list if necessary
		Bool					inList( void );										///< Returns whether or not node in list
		Bool					isHead( void );										///< Returns whether or not this node is the head/tail node
		Int						priority( void );									///< Returns node's priority
		void					setPriority( Int new_pri );				///< Sets node's priority
		void*					item( void );											///< Returns the item this links to, if any
		void					setItem( void *item );						///< Make node point to an item
		void					destroy( void );									///< Delete node
		void					autoDelete( void );
}	;

//================================
// LList
//================================
/**
	* Linked list abstraction.
*/
//================================


class	LList
{
	public:

		/// Enumeration of list sorting methods
		enum SortMode
			{
				ASCENDING,  ///< Lower priority numbers to front of list
				DESCENDING  ///< Higher priority numbers to front of list
			};

	protected:

		LListNode			m_head;								///< List head node
		SortMode			m_sortMode;						///< What sorting method to use for this list's Add() operation
		Bool					m_addToEndOfGroup;		///< Add nodes to end of group of nodes with same priority


	public:

									LList( );
		void					addToHead( LListNode *new_node );				///< Adds new node to the front of the list
		void					addToTail( LListNode *new_node );				///< Adds new node to the end of the list
		void					add( LListNode *new_node );							///< Adds new node to list sorted by priority

		void					addItemToHead( void *item );						///< Adds new item to the front of the list
		void					addItemToTail( void *item );						///< Adds new item to the end of the list
		void					addItem( Int pri, void *item );					///< Adds new item to list sorted by priority

		Int						nodeCount( void );											///< Returns number of nodes currently in list
		LListNode*		firstNode( void );											///< Returns first node in list
		LListNode*		lastNode( void );												///< Returns last node in list
		LListNode*		getNode( Int index );										///< Returns node in list addressed by the zero-based index passed
		void					setSortMode( SortMode new_mode );				///< Sets the sorting mode for the Add() operation
		void					addToEndOfGroup( Bool yes = TRUE );			///< Add node to end or start of group with same priority
		Bool					isEmpty( void );												///< Returns whether or not the the list is empty
		void					clear( void );													///< Deletes all items in the list. Use with care!!
		void					merge( LList *list );										///< Move the contents of the specified list in to this list
		Bool					hasItem( void *item );									///< Tests if list has the specified item
		LListNode*		findItem( void *item );									///< Returns the LListNode that references item
		void					destroy( void );												///< Free up the list items

}	;

//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------

inline		Bool					LListNode::inList( void ) { return m_prev != this; };
inline		Bool					LListNode::isHead( void ) { return ( m_item == (void*)&this->m_item ); };
inline		Int						LListNode::priority( void ) { return m_pri; };
inline		void					LListNode::setPriority( Int new_pri ) { m_pri = new_pri; };
inline		void					LListNode::autoDelete( void ) { m_autoDelete = TRUE; };
inline		void*					LListNode::item( void ) { return m_item; };
inline		void					LListNode::setItem( void *item ) { m_item = item; };


inline		void					LList::addToHead( LListNode *new_node ) { m_head.append( new_node); };
inline		void					LList::addToTail( LListNode *new_node ) { m_head.insert( new_node); };
inline		LListNode*		LList::firstNode( void ) { return m_head.next();} ;
inline		LListNode*		LList::lastNode( void ) { return m_head.prev();} ;
inline		void					LList::setSortMode( SortMode new_mode ) { m_sortMode = new_mode; };
inline		Bool					LList::isEmpty( void ) { return !m_head.inList(); };
inline		void					LList::destroy( void ) { clear();};
