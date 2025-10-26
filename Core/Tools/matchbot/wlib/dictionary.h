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
Project Name: Carpenter  (The RedAlert ladder creator)
File Name   : dictionary.h
Author      : Neal Kettler
Start Date  : June 1, 1997
Last Update : June 17, 1997

This template file implements a hash dictionary.  A hash dictionary is
used to quickly match a value with a key.  This works well for very
large sets of data.  A table is constructed that has some power of two
number of pointers in it.  Any value to be put in the table has a hashing
function applied to the key.  That key/value pair is then put in the
linked list at the slot the hashing function specifies.  If everything
is working well, this is much faster than a linked list, but only if
your hashing function is good.
\****************************************************************************/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <crtdbg.h>

#include "wstypes.h"

// Every entry in the hash dictionary must be an instance of the DNode
// template.  'K' and 'V' denote Key and Value.
template <class K,class V>
class DNode
{
 public:
  K               key;
  V               value;
  DNode<K,V>     *hashNext;
};

template <class K,class V>
class Dictionary
{
 public:
                   ////////////////Dictionary(uint32 (* hashFn)(K &key));


// Note: I had to put this inside the class definition because VC5 sucks butt

//Create the empty hash dictionary
Dictionary(uint32 (*hashFn)(const K &key)) :
 SHRINK_THRESHOLD(0.20), // When table is only 20% full shrink it
 EXPAND_THRESHOLD(0.80), // When table is 80% full grow it
 MIN_TABLE_SIZE(128)      // must be a power of 2
{
  log2Size=MIN_TABLE_SIZE;
  size=MIN_TABLE_SIZE;
  assert(size>=4);
  tableBits=0;
  while(log2Size) { tableBits++; log2Size>>=1; }
  tableBits--;
  size=1<<tableBits;  //Just in case MIN_TABLE_SIZE wasn't a power of 2
  entries=0;
  keepSize=FALSE;

  //Table is a pointer to a list of pointers (the hash table)
  table=(DNode<K,V> **)new DNode<K,V>* [size];
  assert(table!=NULL);

  memset((void *)table,0,size*sizeof(void *));
  hashFunc=hashFn;
}


                  ~Dictionary();

  void             clear(void);
  bit8             add(IN K &key,IN V &value);
  bool             getValue(IN K &key, OUT V &value) RO;
  bool             getPointer(IN K &key, OUT V **value) RO;  // ptr to internal storage (Careful!)
  void             print(FILE *out) RO;
  uint32           getSize(void) RO;
  uint32           getEntries(void) RO;
  bit8             contains(IN K &key) RO;
  bit8             updateValue(IN K &key,IN V &value);
  bit8             remove(IN K &key,OUT V &value);
  bit8             remove(IN K &key);
  bit8             removeAny(OUT K &key,OUT V &value);
  bit8             iterate(INOUT int &index,INOUT int &offset, OUT V &value) RO;
  bit8             iterate(INOUT int &index,INOUT int &offset, OUT K &key, OUT V &value) RO;
  Dictionary<K,V>  &operator=(Dictionary<K,V> &other);

 private:
  void             shrink(void);  // halve the number of slots
  void             expand(void);  // double the number of slots


  DNode<K,V>     **table;      // This stores the lists at each slot

  uint32           entries;    // number of entries
  uint32           size;       // size of table
  uint32           tableBits;  // table is 2^tableBits big
  uint32           log2Size;   // Junk variable
  bit8             keepSize;   // If true don't shrink or expand

  uint32           (* hashFunc)(IN K &key);   // User provided hash function
  uint32           keyHash(IN K &key) RO;     // This will reduce to correct range


  // See initilizer list of constructor for values
  const double     SHRINK_THRESHOLD; // When table is this % full shrink it
  const double     EXPAND_THRESHOLD; // When table is this % full grow it
  const int        MIN_TABLE_SIZE;   // must be a power of 2
};



//Free all the memory...
template <class K,class V>
Dictionary<K,V>::~Dictionary()
{
  clear();          // Remove the entries
  delete[](table);  // And the table as well
}

// Remove all the entries and free the memory
template <class K,class V>
void Dictionary<K,V>::clear()
{
  DNode<K,V> *temp,*del;
  uint32 i;
  //free all the data
  for (i=0; i<size; i++)
  {
    temp=table[i];
    while(temp!=NULL)
    {
      del=temp;
      temp=temp->hashNext;
      delete(del);
    }
    table[i]=NULL;
  }
  entries=0;

  while ((getSize()>(uint32)MIN_TABLE_SIZE)&&(keepSize==FALSE))
    shrink();
}

template <class K,class V>
uint32 Dictionary<K,V>::keyHash(IN K &key) RO
{
  uint32 retval=hashFunc(key);
  retval &= ((1<<tableBits)-1);
  assert(retval<getSize());
  return(retval);
}


template <class K,class V>
void Dictionary<K,V>::print(FILE *out) RO
{
  DNode<K,V> *temp;
  uint32 i;

  fprintf(out,"--------------------\n");
  for (i=0; i<getSize(); i++)
  {
    temp=table[i];

    fprintf(out," |\n");
    fprintf(out,"[ ]");

    while (temp!=NULL)
    {
      fprintf(out,"--[ ]");
      temp=temp->hashNext;
    }
    fprintf(out,"\n");
  }
  fprintf(out,"--------------------\n");
}


template <class K, class V>
Dictionary<K,V> &Dictionary<K,V>::operator=(Dictionary<K,V> &other)
{
  _ASSERTE(0);
}



//
// Iterate through all the records. Index is for the table, offset specifies the
//   element in the linked list.  Set both to 0 and continue calling till false
//   is returned.
template <class K,class V>
bit8 Dictionary<K,V>::iterate(INOUT int &index,INOUT int &offset,
    OUT V &value) RO
{
  DNode<K,V> *temp;

  // index out of range
  if ((index<0)||(index >= (int)getSize()))
    return(FALSE);

  temp=table[index];
  while ((temp==NULL)&&((++index) < (int)getSize()))
  {
    temp=table[index];
    offset=0;
  }

  if (temp==NULL)   // no more slots with data
    return(FALSE);

  uint32 i=0;
  while ((temp!=NULL) && ((int)i < offset))
  {
    temp=temp->hashNext;
    i++;
  }

  if (temp==NULL)  // should never happen
    return(FALSE);

  value=temp->value;
  if (temp->hashNext==NULL)
  {
    index++;
    offset=0;
  }
  else
    offset++;

  return(TRUE);
}




//
// Iterate through all the records. Index is for the table, offset specifies the
//   element in the linked list.  Set both to 0 and continue calling till false
//   is returned.
template <class K,class V>
bit8 Dictionary<K,V>::iterate(INOUT int &index,INOUT int &offset,
    OUT K &key, OUT V &value) RO
{
  DNode<K,V> *temp;

  // index out of range
  if ((index<0)||(index >= (int)getSize()))
    return(FALSE);

  temp=table[index];
  while ((temp==NULL)&&((++index) < (int)getSize()))
  {
    temp=table[index];
    offset=0;
  }

  if (temp==NULL)   // no more slots with data
    return(FALSE);

  uint32 i=0;
  while ((temp!=NULL) && ((int)i < offset))
  {
    temp=temp->hashNext;
    i++;
  }

  if (temp==NULL)  // should never happen
    return(FALSE);

  value=temp->value;
  key=temp->key;
  if (temp->hashNext==NULL)
  {
    index++;
    offset=0;
  }
  else
    offset++;

  return(TRUE);
}




// Return the current size of the hash table
template <class K,class V>
uint32 Dictionary<K,V>::getSize(void) RO
{ return(size); }


// Return the current number of entries in the table
template <class K,class V>
uint32 Dictionary<K,V>::getEntries(void) RO
{ return(entries); }


// Does the Dictionary contain the key?
template <class K,class V>
bit8 Dictionary<K,V>::contains(IN K &key) RO
{
  int offset;
  DNode<K,V> *node;

  offset=keyHash(key);

  node=table[offset];

  if (node==NULL)
  { return(FALSE); }  // can't find it

  while(node!=NULL)
  {
    if ((node->key)==key)
    { return(TRUE); }
    node=node->hashNext;
  }
  return(FALSE);
}


// Try and update the value of an already existing object
template <class K,class V>
bit8 Dictionary<K,V>::updateValue(IN K &key,IN V &value)
{
  sint32 retval;

  retval=remove(key);
  if (retval==FALSE)
    return(FALSE);

  add(key,value);
  return(TRUE);
}


// Add to the dictionary (if key exists, value is updated with the new V)
template <class K, class V>
bit8 Dictionary<K,V>::add(IN K &key,IN V &value)
{
  int offset;
  DNode<K,V> *node,*item,*temp;
  float percent;

  item=(DNode<K,V> *)new DNode<K,V>;
  assert(item!=NULL);

  #ifdef KEY_MEM_OPS
    memcpy(&(item->key),&key,sizeof(K));
  #else
    item->key=key;
  #endif

  #ifdef VALUE_MEM_OPS
    memcpy(&(item->value),&value,sizeof(V));
  #else
    item->value=value;
  #endif

  item->hashNext=NULL;

  //If key already exists, it will be overwritten
  remove(key);   // Hopefully this will be false...

  offset=keyHash(key);

  node=table[offset];

  if (node==NULL)
  { table[offset]=item; }
  else
  {
    temp=table[offset];
    table[offset]=item;
    item->hashNext=temp;
  }

  entries++;
  percent=(float)entries;
  percent/=(float)getSize();
  if (percent>= EXPAND_THRESHOLD ) expand();

  return(TRUE);
}

// Remove an item from the dictionary
template <class K,class V>
bit8 Dictionary<K,V>::remove(IN K &key,OUT V &value)
{
  int offset;
  DNode<K,V> *node,*last,*temp;
  float percent;

  if (entries==0)
    return(FALSE);

  percent=(float)(entries-1);
  percent/=(float)getSize();

  offset=keyHash(key);
  node=table[offset];

  last=node;
  if (node==NULL)
    return(FALSE);

  //special case table points to thing to delete

  #ifdef KEY_MEM_OPS
  if (0==memcmp(&(node->key),&key,sizeof(K)))
  #else
  if ((node->key)==key)
  #endif
  {
    #ifdef VALUE_MEM_OPS
      memcpy(&value,&(node->value),sizeof(V));
    #else
      value=node->value;
    #endif
    temp=table[offset]->hashNext;
    delete(table[offset]);
    table[offset]=temp;
    entries--;
    if (percent <= SHRINK_THRESHOLD)
      shrink();
    return(TRUE);
  }
  node=node->hashNext;

  bit8 retval=FALSE;  // wow, didn't add this for years... (DOH!)

  //Now the case if the thing to delete is not the first
  while (node!=NULL)
  {
    #ifdef KEY_MEM_OPS
      if (0==memcmp(&(node->key),&key,sizeof(K)))
    #else
      if (node->key==key)
    #endif
    {
      #ifdef VALUE_MEM_OPS
        memcpy(&value,&(node->value),sizeof(V));
      #else
        value=node->value;
      #endif
      last->hashNext=node->hashNext;
      entries--;
      delete(node);
      retval=TRUE;  // yes, we deleted something
      break;
    }
    last=node;
    node=node->hashNext;
  }

  if (percent <= SHRINK_THRESHOLD)
    shrink();
  return(retval);
}


template <class K,class V>
bit8 Dictionary<K,V>::remove(IN K &key)
{
  V temp;
  return(remove(key,temp));
}


// Remove some random K/V pair that's in the Dictionary
template <class K,class V>
bit8 Dictionary<K,V>::removeAny(OUT K &key,OUT V &value)
{
  int offset;
  DNode<K,V> *node,*last,*temp;
  float percent;

  if (entries==0)
    return(FALSE);

  percent=(entries-1);
  percent/=(float)getSize();

  int i;
  offset=-1;
  for (i=0; i<(int)getSize(); i++)
    if (table[i]!=NULL)
    {
      offset=i;
      break;
    }

  if (offset==-1)    // Nothing there
    return(FALSE);

  node=table[offset];
  last=node;

  #ifdef KEY_MEM_OPS
    memcpy(&key,&(node->key),sizeof(K));
  #else
    key=node->key;
  #endif
  #ifdef VALUE_MEM_OPS
    memcpy(&value,&(node->value),sizeof(V));
  #else
    value=node->value;
  #endif

  temp=table[offset]->hashNext;
  delete(table[offset]);
  table[offset]=temp;
  entries--;
  if (percent <= SHRINK_THRESHOLD)
    shrink();
  return(TRUE);
}


template <class K,class V>
bool Dictionary<K,V>::getValue(IN K &key,OUT V &value) RO
{
  V *valptr=NULL;
  bool retval=getPointer(key,&valptr);
  if (retval && valptr)
  {
    #ifdef VALUE_MEM_OPS
      assert(0);
    #else
      value=*valptr;
    #endif
  }
  return(retval);
}

// Try and avoid this since you're getting a pointer to the internally
//  managed data!
template <class K,class V>
bool Dictionary<K,V>::getPointer(IN K &key,OUT V **valptr) RO
{
  int offset;
  DNode<K,V> *node;

  if (entries==0)
    return(FALSE);

  offset=keyHash(key);

  node=table[offset];

  if (node==NULL)
    return(FALSE);

  #ifdef KEY_MEM_OPS
    while ((node!=NULL)&&(memcmp(&(node->key),&key,sizeof(K))))
  #else
    while ((node!=NULL)&&( ! ((node->key)==key)) )  // odd syntax so you don't
  #endif                                            // have to do oper !=
  { node=node->hashNext; }

  if (node==NULL)
  { return(FALSE); }

  *valptr=&(node->value);

  return(TRUE);
}


//A note about Shrink and Expand: They are never necessary, they are
//only here to improve performance of the hash table by reducing
//the length of the linked list at each table entry.

// Shrink the hash table by a factor of 2 (and relocate entries)
template <class K,class V>
void Dictionary<K,V>::shrink(void)
{
  int    i;
  int    oldsize;
  uint32 offset;
  DNode<K,V> **oldtable,*temp,*first,*next;

  if ((size<=(uint32)MIN_TABLE_SIZE)||(keepSize==TRUE))
    return;

  //fprintf(stderr,"Shrinking....\n");

  oldtable=table;
  oldsize=size;
  size/=2;
  tableBits--;

  table=(DNode<K,V> **)new DNode<K,V>*[size];
  assert(table!=NULL);
  memset((void *)table,0,size*sizeof(void *));

  for (i=0; i<oldsize; i++)
  {
    temp=oldtable[i];
    while (temp!=NULL)
    {
      offset=keyHash(temp->key);
      first=table[offset];
      table[offset]=temp;
      next=temp->hashNext;
      temp->hashNext=first;
      temp=next;
    }
  }
  delete[](oldtable);
}


template <class K,class V>
void Dictionary<K,V>::expand(void)
{
  int    i;
  int    oldsize;
  uint32 offset;
  DNode<K,V> **oldtable,*temp,*first,*next;

  if (keepSize==TRUE)
    return;

  //fprintf(stderr,"Expanding...\n");

  oldtable=table;
  oldsize=size;
  size*=2;
  tableBits++;

  table=(DNode<K,V> **)new DNode<K,V>* [size];
  assert(table!=NULL);
  memset((void *)table,0,size*sizeof(void *));

  for (i=0; i<oldsize; i++)
  {
    temp=oldtable[i];
    while (temp!=NULL)
    {
      offset=keyHash(temp->key);
      first=table[offset];
      table[offset]=temp;
      next=temp->hashNext;
      temp->hashNext=first;
      temp=next;
    }
  }
  delete[](oldtable);
}
