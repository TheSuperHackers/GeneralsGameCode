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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright(C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:   WSYS Library
//
// Module:    List
//
// File name: WSYS_List.cpp
//
// Created:   10/31/01 TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "PreRTS.h"

#include "Common/List.h"

// 'assignment within condition expression'.
#pragma warning(disable : 4706)

//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------

//============================================================================
// LList::LList
//============================================================================

LList::LList( )
: m_sortMode(DESCENDING)
{
	m_head.setItem( &m_head.m_item);
};

//=================================================================
// LList::add
//=================================================================

void	LList::add( LListNode* new_node )
{
	LListNode*	node;
	Int	pri;

	if ( m_addToEndOfGroup )
	{
		pri = new_node->priority();
		node = &m_head;
		while( (node = node->prev() ))
		{
			if( (m_sortMode == ASCENDING && node->priority() >= pri)
					|| (m_sortMode == DESCENDING && node->priority() <= pri) )
			{
				node->append( new_node );
				return;
			}
		}
		m_head.append( new_node );
	}
	else
	{
		pri = new_node->priority();
		node = &m_head;
		while( (node = node->next() ))
		{
			if( (m_sortMode == ASCENDING && node->priority() <= pri)
					|| (m_sortMode == DESCENDING && node->priority() >= pri) )
			{
				node->insert( new_node );
				return;
			}
		}
		m_head.insert( new_node );
	}
}

//============================================================================
// LList::addGDFNode
//============================================================================

void	LList::addItem( Int pri, void* item )
{
	LListNode *node = NEW LListNode();	// poolify

	if ( node )
	{
		node->setPriority( pri );
		node->setItem( item );
		node->autoDelete();
		add( node );
	}
}

//============================================================================
// LList::addGDFNodeToHead
//============================================================================

void	LList::addItemToHead( void *item )
{
	LListNode *node = NEW LListNode();

	if ( node )
	{
		node->setItem( item );
		node->autoDelete();
		addToHead( node );
	}
}

//============================================================================
// LList::addGDFNodeToTail
//============================================================================

void	LList::addItemToTail( void *item )
{
	LListNode *node = NEW LListNode();

	if ( node )
	{
		node->setItem( item );
		node->autoDelete();
		addToTail( node );
	}
}

//============================================================================
// LList::Clear
//============================================================================

void LList::clear( void )
{
	LListNode *node;

	while ( (node = firstNode()) != nullptr )
	{
		node->remove();
		node->destroy();
	}
}

//=================================================================
// LList::nodeCount
//=================================================================

Int LList::nodeCount( void )
{
	LListNode* node;
	Int	count = 0;

	node = firstNode();

	while(node)
	{
		count++;
		node = node->next();
	}

	return count;
}

//=================================================================
// LList::getNode
//=================================================================

LListNode*	LList::getNode( Int index )
{
	LListNode* node;

	node = firstNode();

	while( node && index >= 0 )
	{
		if( index-- == 0 )
		{
			return node;
		}
		node = node->next();
	}

	return nullptr;
}

//============================================================================
// LList::merge
//============================================================================

void LList::merge( LList *list )
{

	if ( list == nullptr || list->isEmpty() )
	{
			return;
	}

	m_head.m_prev->m_next = list->m_head.m_next;
	list->m_head.m_next->m_prev = m_head.m_prev;
	list->m_head.m_prev->m_next = &m_head;
	m_head.m_prev = list->m_head.m_prev;
	list->m_head.m_next = &list->m_head;
	list->m_head.m_prev = &list->m_head;

}

//============================================================================
// LList::hasReference
//============================================================================

Bool LList::hasItem( void *item )
{
	return findItem( item ) != nullptr;
}

//============================================================================
// LList::findReference
//============================================================================

LListNode* LList::findItem( void *item )
{
	LListNode* node;

	node = firstNode();

	while( node )
	{
		if( node->item() == item )
		{
			return node;
		}
		node = node->next();
	}

	return nullptr;
}

//============================================================================
// LListNode::LListNode
//============================================================================

LListNode::LListNode()
: m_pri(0),
	m_item(nullptr),
	m_autoDelete(FALSE)
{
	m_next = m_prev = this;
};

//=================================================================
// LListNode::insert
//=================================================================

void		 	LListNode::insert( LListNode* new_node )
{
	new_node->m_prev = m_prev;
	new_node->m_next = this;
	m_prev = new_node;
	new_node->m_prev->m_next = new_node;
}

//=================================================================
// LListNode::append
//=================================================================

void 		 	LListNode::append( LListNode* new_node )
{
	new_node->m_prev = this;
	new_node->m_next = m_next;
	this->m_next = new_node;
	new_node->m_next->m_prev = new_node;
}

//=================================================================
// LListNode::remove
//=================================================================

void 		 	LListNode::remove( void )
{
	m_prev->m_next = m_next;
	m_next->m_prev = m_prev;
	m_prev = m_next = this;		// so we know that the node is not in a list
}

//=================================================================
// LListNode::next
//=================================================================

LListNode*		LListNode::next( void )
{

	if( m_next->isHead( ))
	{
		return nullptr;
	}

	return m_next;
}

//=================================================================
// LListNode::prev
//=================================================================

LListNode*		LListNode::prev( void )
{
	if( m_prev->isHead())
	{
		return nullptr;
	}

	return m_prev;
}

//=================================================================
// LListNode::loopNext
//=================================================================

LListNode*		LListNode::loopNext( void )
{
	LListNode*	next;

	next = m_next;

	if( next->isHead( ))
	{
		// skip head node
		next = next->m_next;
		if( next->isHead( ))
		{
			return nullptr;	// it is an empty list
		}
	}

	return next;
}

//=================================================================
// LListNode::loopPrev
//=================================================================

LListNode*		LListNode::loopPrev( void )
{
	LListNode*	prev;

	prev = m_prev;

	if( prev->isHead())
	{
		// skip head node
		prev = prev->m_prev;
		if( prev->isHead())
		{
			return nullptr;	// it is an empty list
		}
	}

	return prev;
}

//============================================================================
// LListNode::destroy
//============================================================================

void	LListNode::destroy( void )
{
	if ( m_autoDelete )
	{
		delete this;
	}
}

//============================================================================
// LList::addToEndOfGroup
//============================================================================

void	LList::addToEndOfGroup( Bool yes )
{
	m_addToEndOfGroup = yes;
}
