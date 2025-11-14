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

/****************************************************************************\
*        C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S         *
******************************************************************************
Project Name:
File Name   : linkedlist.h
Author      : Neal Kettler
Start Date  : June 19, 1997
Last Update : June 19, 1997

Linked list template.  This is a fairly standard doubly linked list that
allows insertion and removal at any point in the list.  A current pointer
is used to quickly access items when they are examined sequentially.
Copies of the data are stored instead of a pointer to the original.

If you want to store pointers then the template should be of a pointer type.
\****************************************************************************/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "wstypes.h"

template <class T>
class LNode
{
 public:
  T               Node;
  LNode<T>       *Next;
  LNode<T>       *Prev;
};

template <class T>
class LinkedList
{
 public:
                   LinkedList();
                   LinkedList(LinkedList<T> &other);
                  ~LinkedList();

  // Remove all entries from the lsit
  void             clear(void);

  // Add a node after the zero based 'pos'
  bit8             add(IN T &node,sint32 pos, OUT T **newnodeptr=NULL);
  bit8             addTail(IN T &node, OUT T **newnodeptr=NULL);
  bit8             addHead(IN T &node, OUT T **newnodeptr=NULL);

  // Remove a node
  bit8             remove(OUT T &node,sint32 pos);
  bit8             remove(sint32 pos);
  bit8             removeHead(OUT T &node);
  bit8             removeTail(OUT T &node);


  // Get a node without removing from the list
  bit8             get(OUT T &node,sint32 pos);
  bit8             getHead(OUT T &node);
  bit8             getTail(OUT T &node);

  // Get a pointer to the internally managed data (careful!)
  bit8             getPointer(OUT T **node, sint32 pos);

  // Get the number of entries in the list
  sint32           length(void);

  // Print information on the list
  void             print(IN FILE *out);

  // assignment operator
  LinkedList<T>   &operator=(LinkedList<T> &other);

 private:
  sint32           Entries;    // Number of entries
  LNode<T>        *Head;       // Head of the list
  LNode<T>        *Tail;       // Tail of the list

  LNode<T>        *Current;    // Current pointer & index for speed only
  sint32           CurIndex;
};


//Create the empty list
template <class T>
LinkedList<T>::LinkedList()
{
  Entries=0;
  Head=Tail=Current=NULL;
  CurIndex=-1;  // Not valid when 0 entries
}

// copy constructor
template <class T>
LinkedList<T>::LinkedList(LinkedList<T> &other)
{
  Entries=0;
  Head=Tail=Current=NULL;
  CurIndex=-1;  // Not valid when 0 entries
  (*this)=other;
}

//Free all the memory...
template <class T>
LinkedList<T>::~LinkedList()
{
  clear();          // Remove the entries
}

// assignment operator
template <class T>
LinkedList<T> &LinkedList<T>::operator=(LinkedList<T> &other)
{
  T node;
  clear();
  for (int i=0; i<other.length(); i++)
  {
    other.get(node,i);
    addTail(node);
  }
  return(*this);
}


// Remove all the entries and free the memory
template <class T>
void LinkedList<T>::clear()
{
  LNode<T> *temp,*del;

  temp=Head;
  while (temp) {
    del=temp;
    temp=temp->Next;
    delete(del);
  }
  Entries=0;
  CurIndex=-1;
  Head=Tail=Current=NULL;
}

// When adding into a position, the new node goes at the zero based slot
// specified by pos. All other nodes get moved one slot down.
template <class T>
bit8 LinkedList<T>::add(IN T &node,sint32 pos, OUT T **newnodeptr)
{
  LNode<T> *temp;
  LNode<T> *item;

  if (pos<0)
    pos=0;
  if (pos>Entries)
    pos=Entries;

  item=(LNode<T> *)new LNode<T>;
  assert(item!=NULL);
  item->Node=node;  // copy the passed in object
  item->Prev=NULL;
  item->Next=NULL;

  if (newnodeptr)
    *newnodeptr=&(item->Node);

  if ((pos==0)||(pos==Entries)) {  // Both cases can be true for a new list!
    if (pos==0) {
      item->Next=Head;
      if (Head)
        Head->Prev=item;
      Head=item;
    }
    if (pos==Entries) {
      item->Prev=Tail;
      if (Tail)
        Tail->Next=item;
      Tail=item;
    }
    Entries++;
    Current=item;
    CurIndex=pos;
    return(TRUE);
  }

  // If control is here, we know the new node is not an endpoint

  // Check for possible speedup, so we don't have to scan the list
  if (pos==CurIndex) {
    item->Next=Current;
    item->Prev=Current->Prev;
    Current->Prev=item;
    item->Prev->Next=item;
    Current=item;
    Entries++;
    return(TRUE);
  }
  // Check the other possible speedup (adding after CurIndex)
  if (pos==CurIndex+1) {
    item->Next=Current->Next;
    item->Prev=Current;
    Current->Next=item;
    item->Next->Prev=item;
    Current=item;
    CurIndex++;
    Entries++;
    return(TRUE);
  }

  // If control reaches here we have to scan the whole thing
  temp=Head->Next;  // Can start at node '1' because head was special cased
  for (int i=1; i<pos; i++) {
    temp=temp->Next;
    assert(temp!=NULL);
  }
  item->Next=temp;
  item->Prev=temp->Prev;
  temp->Prev=item;
  item->Prev->Next=item;
  Current=item;
  CurIndex=pos;
  Entries++;

  return(TRUE);
}


// Add to the first node, all others get shifted down one slot
template <class T>
bit8 LinkedList<T>::addHead(IN T &node, OUT T **newnodeptr)
{
  return(add(node,0,newnodeptr));
}


// Append to the end of the list
template <class T>
bit8 LinkedList<T>::addTail(IN T &node, OUT T **newnodeptr)
{
  return(add(node,length(),newnodeptr));
}


// Remove at the zero based index specified by 'pos'.  When removing from
// a slot, all others get shifted up by one.
template <class T>
bit8 LinkedList<T>::remove(OUT T &node, sint32 pos)
{
  ////////LNode<T> *temp;
  LNode<T> *item;

  if (Entries==0)
    return(FALSE);

  if (pos<0)
    pos=0;
  if (pos>=Entries)
    pos=Entries-1;

  if ((pos==0)||(pos==Entries-1)) {  // Both can be true for a 1 item list
    if (pos==0) {
      item=Head;
      if (item->Next)
        item->Next->Prev=NULL;
      Head=item->Next;
      node=item->Node;
      Current=Head;
      CurIndex=0;
    }
    if (pos==Entries-1) {
      item=Tail;
      if (item->Prev)
        item->Prev->Next=NULL;
      Tail=item->Prev;
      node=item->Node;
      Current=Tail;
      CurIndex=Entries-2;
    }
    delete(item);
    Entries--;

    if (Entries==0) { // Super paranoia check
      assert(Current==NULL);
      assert(CurIndex==-1);
      assert(Head==NULL);
      assert(Tail==NULL);
    }
    return(TRUE);
  }
  // If control is here, we know the target node is not an endpoint

  // Check for possible speedup, so we don't have to scan the list
  if (pos==CurIndex) {
    item=Current;
    item->Prev->Next=item->Next;
    item->Next->Prev=item->Prev;
    Current=item->Next;
    // CurIndex stays the same
    node=item->Node;
    delete(item);
    Entries--;
    return(TRUE);
  }

  // Check the other possible speedup (removing after CurIndex)
  if (pos==CurIndex+1) {
    item=Current->Next;
    item->Prev->Next=item->Next;
    item->Next->Prev=item->Prev;
    Current=item->Next;
    CurIndex++;
    node=item->Node;
    delete(item);
    Entries--;
    return(TRUE);
  }


  // If control reaches here we have to scan the whole thing
  item=Head->Next;  // Can start at node '1' because head was special cased
  for (int i=1; i<pos; i++) {
    item=item->Next;
    assert(item!=NULL);
  }

  item->Prev->Next=item->Next;
  item->Next->Prev=item->Prev;
  Current=item->Next;
  CurIndex=pos;
  node=item->Node;
  delete(item);
  Entries--;

  return(TRUE);
}




// Remove at the zero based index specified by 'pos'.  When removing from
// a slot, all others get shifted up by one.
template <class T>
bit8 LinkedList<T>::remove(sint32 pos)
{
  T temp_node;
  return(remove(temp_node,pos));
}


// Remove the first node of the list
template <class T>
bit8 LinkedList<T>::removeHead(OUT T &node)
{
  return(remove(node,0));
}


// Remove the last node of the list
template <class T>
bit8 LinkedList<T>::removeTail(OUT T &node)
{
  return(remove(node,Entries-1));
}



template <class T>
bit8 LinkedList<T>::get(OUT T &node, sint32 pos)
{
  T *objptr;
  bool retval=getPointer(&objptr,pos);
  if (retval && objptr)
    node=*objptr;

  return(retval);
}


template <class T>
bit8 LinkedList<T>::getPointer(OUT T **node,sint32 pos)
{
  if ((node==0)||(Entries==0))
    return(FALSE);

  LNode<T> *item;

  if (pos<0)
  {
    //pos=0;
    return(FALSE);
  }
  if (pos>=Entries)
  {
    //pos=Entries-1;
    return(FALSE);
  }

  if (pos==0) {
    *node=&(Head->Node);
    return(TRUE);
  } else if (pos==Entries-1) {
    *node=&(Tail->Node);
    return(TRUE);
  }
  // If control reaches here, we know target is not an endpoint

  // Check for possible speedup, so we don't have to scan the list
  if (pos==CurIndex) {
    *node=&(Current->Node);
    return(TRUE);
  } else if (pos==CurIndex+1) {
    *node=&(Current->Next->Node);
    CurIndex++;
    Current=Current->Next;
    return(TRUE);
  } else if (pos==CurIndex-1) {
    *node=&(Current->Prev->Node);
    CurIndex--;
    Current=Current->Prev;
    return(TRUE);
  }

  // If control reaches here we have to scan the whole thing
  item=Head->Next;  // Can start at node '1' because head was special cased
  for (int i=1; i<pos; i++) {
    item=item->Next;
    assert(item!=NULL);
  }
  *node=&(item->Node);
  CurIndex=pos;
  Current=item;

  return(TRUE);
}


// Remove the first node of the list
template <class T>
bit8 LinkedList<T>::getHead(OUT T &node)
{
  return(get(node,0));
}


// Remove the last node of the list
template <class T>
bit8 LinkedList<T>::getTail(OUT T &node)
{
  return(get(node,Entries-1));
}


template <class T>
void LinkedList<T>::print(IN FILE *out)
{
  LNode<T> *temp;

  fprintf(out,"--------------------\n");
  fprintf(out,"Entries = %d\n",length());
  fprintf(out,"H = %8p   C = %8p (%d)   T = %8p\n",Head,Current,CurIndex,Tail);

  temp=Head;
  while (temp) {
    fprintf(out,"  %8p<-((%8p))->%8p  \n",temp->Prev,temp,temp->Next);
    temp=temp->Next;
  }

  fprintf(out,"--------------------\n");
}

// Return the current length of the list
template <class T>
sint32 LinkedList<T>::length(void) {
  return(Entries);
}
