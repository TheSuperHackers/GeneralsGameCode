/*
**	Command & Conquer Generals(tm)
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

//
// list.cpp
//


#include "StdAfx.h"
#include <assert.h>
#include "list.h"

ListNode::ListNode	( void )
{
	prev = next = this;
	pri = NORMAL_PRIORITY;
	item = NULL;
}

void				ListNode::Append			( ListNode *new_node )
{
	assert ( ! new_node->InList());	/* node is already in a list or was not initialized*/

	new_node->prev = this;
	new_node->next = next;
	next = new_node;
	new_node->next->prev = new_node;

}

void				ListNode::Prepend			( ListNode *new_node )
{

	assert ( !new_node->InList() );	/* node is already in a list or was not initialized*/

	new_node->prev = prev;
	new_node->next = this;
	prev = new_node;
	new_node->prev->next = new_node;

}
void				ListNode::Link ( ListNode *node)
{
	next = node;
	node->prev = next;
}

void				ListNode::Remove			( void )
{
	prev->next = next;
	next->prev = prev;
	prev = next = this;		/* so we know that the node is not in a list */
}

ListNode*		ListNode::Next				( void )
{
	if ( next->IsHead ( ) )
	{
		return NULL;
	}

	return next;
}

ListNode*		ListNode::Prev				( void )
{
	if ( prev->IsHead () )
	{
		return NULL;
	}

	return prev;
}

ListNode*		ListNode::NextLoop		( void )
{
	ListNode *next_node = next;

	if ( next_node->IsHead ( ))
	{
		/* skip head node */
		next_node = next_node->next;
		if ( next_node->IsHead ( ))
		{
			return NULL;	/* it is an empty list */
		}
	}

	return next_node;

}

ListNode*		ListNode::PrevLoop		( void )
{
	ListNode *prev_node = prev;

	if ( prev_node->IsHead ( ))
	{
		/* skip head node */
		prev_node = prev_node->prev;
		if ( prev_node->IsHead ( ))
		{
			return NULL;	/* it is an empty list */
		}
	}

	return prev_node;
}

void*				ListNode::Item				( void )
{

	assert ( !IsHead () );

	return item;

}

void				ListNode::SetItem			( void *new_item )
{
	assert ( !IsHead () );
	item = new_item	;
}

int					ListNode::InList			( void )
{

	return prev != this;
}

int					ListNode::IsHead			( void )
{
	return item == &this->item;
}

int					ListNode::Priority		( void )
{
	return pri;

}
void				ListNode::SetPriority ( int new_pri )
{

	assert ( new_pri <= HIGHEST_PRIORITY );
	assert ( new_pri >= LOWEST_PRIORITY );
	pri = new_pri;

}

List::List ( void )
{

	SetItem ( &this->item );

}

void				List::AddToTail ( ListNode *node )
{
	assert ( IsHead ());
	Prepend ( node );
}

void				List::AddToHead ( ListNode *node )
{
	assert ( IsHead ());
	Append ( node );
}

void				List::Add				( ListNode *new_node )
{
	ListNode*	node;
	int		pri;

	assert ( IsHead ());
	assert ( !new_node->InList ());

	pri = new_node->Priority();
	node = FirstNode ( );

	while( node )
	{
		if (node->Priority() <= pri )
		{
			node->Prepend ( new_node );
			return;
		}

		node = node->Next ();
	}

	Prepend ( new_node );

}

void				List::Merge			( List *list )
{
	ListNode	*first,
						*last,
						*node;

	assert ( IsHead ());

	first = list->Next();
	last = list->Prev();

	if ( !first || !last )
	{
		return;
	}
	
	node = Prev();
	node->Link ( first );
	last->Link ( this );
	
	list->Empty ();

}

int					List::NumItems  ( void )
{
	int count = 0;
	ListNode *node;

	assert ( IsHead ());
	node = FirstNode();

	while ( node )
	{
		count++;
		node = node->Next ();
	}

	return count;
}

void*				List::Item			( int list_index )
{
	ListNode *node;

	assert ( IsHead ());
	node = FirstNode();

	while (list_index && node )
	{
		list_index--;
		node = node->Next ();
	}
	if ( node )
	{
		return node->Item();
	}

	return NULL;
}

ListNode*		List::FirstNode ( void )
{
	assert ( IsHead ());
	return Next ();

}

ListNode*		List::LastNode ( void )
{
	assert ( IsHead ());
	return Prev ();

}

int					List::IsEmpty		( void )
{
	assert ( IsHead ());

	return !InList();

}

void				List::Empty			( void )
{
	assert ( IsHead ());
	Remove ();
}


ListNode*		List::Find			( void *item )
{
	ListNode *node;

	node = FirstNode ();

	while ( node )
	{
		if ( node->Item () == item )
		{
			return node;
		}

		node = node->Next ();
	}
	return NULL;
}
