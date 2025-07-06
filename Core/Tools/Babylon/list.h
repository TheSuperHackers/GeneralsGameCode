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


#ifndef	__LIST_H
#define	__LIST_H


const int LOWEST_PRIORITY = (int ) 0x80000000;
const	int HIGHEST_PRIORITY = (int) 0x7fffffff;
const int NORMAL_PRIORITY = 0;

class ListNode
{
	private:
	ListNode	*next;
	ListNode	*prev;
	int				pri;		// node's priority

	protected:
	void			*item;	// item

	public:

							ListNode		( void );
	void				Append			( ListNode *node );
	void				Prepend			( ListNode *node );
	void				Link				( ListNode *node);
	void				Remove			( void );
	ListNode*		Next				( void );
	ListNode*		Prev				( void );
	ListNode*		NextLoop		( void );
	ListNode*		PrevLoop		( void );
	void*				Item				( void );
	void				SetItem			( void *item );
	int					InList			( void );
	int					IsHead			( void );
	int					Priority		( void );
	void				SetPriority ( int new_pri );


};

class List: public ListNode
{

	public:
	List ( void );
	void				AddToTail ( ListNode *node );
	void				AddToHead ( ListNode *node );
	void				Add				( ListNode *node );
	void				Merge			( List *list );
	int					NumItems  ( void );
	void*				Item			( int list_index );
	ListNode*		FirstNode ( void );
	ListNode*		LastNode	( void );
	int					IsEmpty		( void );
	void				Empty			( void );
	ListNode*		Find			( void *item );

};

class ListSearch
{
	List				*head;
	ListNode		*node;

	public:

	ListNode*		Next ( void ) { if (node) { node = node->Next ();} return node;};
	ListNode*		Prev ( void ) { if (node) { node = node->Prev ();} return node;};
	ListNode*		FirstNode ( void ) { node = head; return Next (); };
	ListNode*		LastNode ( void ) { node = head; return Prev (); };
	ListNode*		FirstNode ( List *new_head ) { node = head = new_head; return Next (); };
	ListNode*		LastNode ( List *new_head) { node = head = new_head; return Prev (); };

};


#endif	//	__LIST_H 
