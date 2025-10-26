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
File Name   : arraylist.h
Author      : Neal Kettler
Start Date  : Jan 19, 1997
Last Update : Jan 19, 1997
------------------------------------------------------------------------------

Array implementation of a list.  Note: There are some freaky C++ memory tricks
going on here.  If you think there's a leak, see me before changing it.
The way this works is that it allocates an array to hold 'N' items on the
first list add.  It doesn't call the constructors for those 'N' items until
necessary (for efficiency).  When an item is added to a slot then a new
class is constructed inside the array element using the placement new operator
and the class's copy constructor.  When elements are removed the destructor
is then manually called on this memory location.

All data added to the list is copied so feel free to delete/destroy/modify
the original after an add.

You _must_ have a good copy constructor for any classes that you use this template
for!  A good copy constructor is one that won't blindly duplicate pointers
that don't belong to them, etc...

\****************************************************************************/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <new.h>
#include <math.h>

#include "wstypes.h"

//
// Usage: ArrayList<int> TheList;
//
template <class T>
class ArrayList
{
 public:
                   ArrayList();
                   ArrayList(ArrayList<T> &other);
                  ~ArrayList();

  // Remove all entries from the lsit
  void             clear(void);

  // Add a node after the zero based 'pos'
  bit8             add(IN T &node,sint32 pos);
  bit8             addTail(IN T &node);
  bit8             addHead(IN T &node);
  bit8		   addSortedAsc(IN T &node);		// Ascending
  bit8		   addSortedDes(IN T &node);		// Descending
  /*bit8		   addNumSortedAsc(IN T &node);		// Ascending
  bit8		   addNumSortedDes(IN T &node);		// Descending*/

  // Remove a node
  bit8             remove(OUT T &node,sint32 pos);
  bit8             remove(sint32 pos);
  bit8             removeHead(OUT T &node);
  bit8             removeTail(OUT T &node);

  // Replace one obj with another
  bit8             replace(IN T &node, sint32 pos);


  // Get a node without removing from the list
  bit8             get(OUT T &node,sint32 pos) RO;
  bit8             getHead(OUT T &node) RO;
  bit8             getTail(OUT T &node) RO;

  // Get a pointer to the interally managed copy (careful!)
  bit8             getPointer(OUT T **node,sint32 pos) RO;

  // Get the number of entries in the list
  sint32           length(void) RO;

  // UNSAFE! for classes, see note below!
  bit8             setSize(sint32 newsize, IN T &filler);

  // Print information on the list
  void             print(FILE *out);

  // assignment operator
  ArrayList<T>   &operator=(IN ArrayList<T> &other);

 private:
  sint32 	   _sortedLookup(IN T &target, int ascending);
  sint32           Entries_;   // Number of entries
  sint32           Slots_;     // Number of available slots

  T               *Vector_;    // The actual memory where the list is held

  enum
  {
    INITIAL_SIZE = 10
  };

  bit8             growVector(void);   // Expand the number of slots
  bit8             shrinkVector(void); // Reduce the number of slots
};


//Create the empty list
template <class T>
ArrayList<T>::ArrayList()
{
  Entries_=0;
  Slots_=0;
  Vector_=NULL;
}

// copy constructor
template <class T>
ArrayList<T>::ArrayList(ArrayList<T> &other)
{
  Entries_=0;
  Slots_=0;
  Vector_=NULL;
  (*this)=other;
}

//Free all the memory...
template <class T>
ArrayList<T>::~ArrayList()
{
  clear();                   // Remove the entries & call destructors on them

  delete[]((uint8*)Vector_); // this will prevent the destructors from
                             //  gettting called on elements not
                             //  containing valid objects.

  //fprintf(stderr,"Arraylist destructor\n");
}

// assignment operator
template <class T>
ArrayList<T> &ArrayList<T>::operator=(IN ArrayList<T> &other)
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
void ArrayList<T>::clear()
{
  for (int i=0; i<Entries_; i++)
  {
    (Vector_+i)->~T();    // Call the destructor manually. Don't try this
                          //  at home kiddies!
  }
  Entries_=0;
}

// ************************* UNSAFE UNSAFE UNSAFE *************************
// Note - Don't call this on any type with a constructor/destructor since this
//  is really dumb and just puts a new one of filler in. Well, it's kindof safe
//  just be careful.
// It's fine for simple classes like ints though..
//
// Add/remove entries in a stupid manner...
//
// **************************************************************************
template <class T>
bit8 ArrayList<T>::setSize(sint32 newsize, IN T &filler)
{
  int oldEntries=Entries_;
  Entries_ = newsize;

  if (newsize<0)
    return(false);

  // Grow the vector as much as we need to
  while (newsize > Slots_)
    growVector();

  // Create new objects in the blank holes
  for (int i=oldEntries; i<Entries_; i++)
  {
    // Now put the replacement object in there...
    new((void *)(Vector_+i)) T(filler); // Trust me, this isn't a memory leak
  }

  // If we're at 33% usage or less, shrink the vector
  if ((Entries_*3) <= Slots_)  // don't do while, because I think shrink will never goto 0
    shrinkVector();

  return(true);
}


// When adding into a position, the new node goes at the zero based slot
// specified by pos. All other nodes get moved one slot down.
template <class T>
bit8 ArrayList<T>::add(IN T &node,sint32 pos)
{
  if (pos > Entries_)      // You can only access one of the end of the vector
    pos=Entries_;
  if (pos >= Slots_)        // If we're at the end, grow the list
    growVector();
  if (Entries_ >= Slots_)   // enuff space?
    growVector();

  // If we are insering into the middle or front of the list we have to
  //  slide the old objects forward.
  if (pos < Entries_)   // If there are elements after the add point
    memmove(Vector_+pos+1,Vector_+pos,sizeof(T)*(Entries_-pos)); // move them forward

  //fprintf(stderr,"Placement new to %p\n",(Vector_+pos));

  // This uses the placement new operator. placement new allows us to
  //  specify the memory address for the new object.  In this case we
  //  want it at the 'pos' index into our array.
  new((void *)(Vector_+pos)) T((T &)node); // Trust me, this isn't a memory leak
  Entries_++;                         // one new entry
  return(TRUE);
}


// Add to the first node, all others get shifted down one slot
template <class T>
bit8 ArrayList<T>::addHead(IN T &node)
{
  return(add(node,0));
}


// Append to the end of the list
template <class T>
bit8 ArrayList<T>::addTail(IN T &node)
{
  return(add(node,length()));
}


// addSortedX only works (properly) if evrerything else in the list is added
// using addSorted.
template <class T>
bit8 ArrayList<T>::addSortedAsc(IN T &node)
{
	sint32 pos = _sortedLookup(node, 1);
	return(add(node, pos));
}


// addSortedX only works (properly) if evrerything else in the list is added
// using addSorted.
template <class T>
bit8 ArrayList<T>::addSortedDes(IN T &node)
{
	sint32 pos = _sortedLookup(node, 0);
	return(add(node, pos));
}


// This is the binary search used by addSorted
template <class T>
sint32 ArrayList<T>::_sortedLookup(IN T &target, int ascending)
{
	int	low, mid, high;
	T* 	lowtarget;
	T* 	hightarget;
	T* 	midtarget;


	// Trivial cases
	if( Entries_ == 0 )
		return 0;

	low = 0;
	high = Entries_ - 1;
	while( 1 )
	{
		assert( low <= high );
		mid = low + (int)(floor(((double)high - (double)low) / (double)2));

		getPointer(&lowtarget, low);
		getPointer(&hightarget, high);
		getPointer(&midtarget, mid);

		// Exact match
		if( *midtarget == target )  return mid;

		// Single element
		if( high == low )
		{
			if( ascending )
			{
				if( target <= *lowtarget )
					return low;
				else
					return low + 1;
			}
			else
			{
				if( target <= *lowtarget )
					return low + 1;
				else
					return low;
			}
		}

		// Two elemsnts
		if( (high - low) == 1 )
		{
			if( ascending )
			{
				if( target <= *lowtarget )
					return low;
				else if( target <= *hightarget )
					return high;
				else
					return high + 1;
			}
			else
			{
				if( target <= *hightarget )
					return high + 1;
				else if( target <= *lowtarget )
					return high;
				else
					return low;
			}
		}

		// Sorry, try again...
		if( ascending )
		{
			if( target < *midtarget )
				high = mid;
			else
				low = mid;
		}
		else
		{
			if( target < *midtarget )
				low = mid;
			else
				high = mid;
		}
	}
}


/*// addNumSortedX works in much the same way as addSortedX, except that I needed
// it for a very specific thing.  I needed a list of strings numerically sorted,
// not alphabetically sorted.  Furthermore these strings were composed of numbers
// delimited by underscores.  In the interest of keeping it generic, these
// functions take as args a node, a delimiting character, and a count of the
// number of fields to include in a sort.  If this is the list of strings:
//
//	55_100, 2_5, 23_32, 98_445, 2_48, 8_88, 2_3, 2_4
//
// An alphabetical sort is:
//
//	2_3, 2_4, 2_48, 2_5, 55_100, 8_88, 98_445
//
// But a numerical sort by calling addNumSortedAsc(<whatever>, "_", 2) will result in:
//
//	2_3, 2_4, 2_5, 2_48, 8_88, 55_100, 98_445
//
// Yes...now that you mention it I am on crack...
//
template <class T>
bit8 ArrayList<T>::addNumSortedAsc(IN T &node, char delim, int fields)
{
        sint32 pos = _numSortedLookup(node, delim, fields, 1);
        return(add(node, pos));
}


// See addNumSortedAsc comment above.
template <class T>
bit8 ArrayList<T>::addSortedDes(IN T &node, char delim, int fields)
{
        sint32 pos = _sortedLookup(node, delim, fields, 0);
        return(add(node, pos));
}


// This is the binary search used by addSorted
template <class T>
sint32 ArrayList<T>::_numSortedLookup(IN T &target, char delim, int fields, int ascending)
{
	int	low, mid, high;
	T* 	lowtarget;
	T* 	hightarget;
	T* 	midtarget;


	// Trivial case
	if( Entries_ == 0 )
		return 0;

	low = 0;
	high = Entries_;
	while( 1 )
	{
		assert( low <= high );
		mid = low + (int)(floor(((double)high - (double)low) / (double)2));

		getPointer(&lowtarget, low);
		getPointer(&hightarget, high);
		getPointer(&midtarget, mid);

		// Exact match
		if( *midtarget == target )  return mid;

		// Single element
		if( high == low )
		{
			if( ascending )
			{
				if( target <= *lowtarget )
					return low;
				else
					return low + 1;
			}
			else
			{
				if( target <= *lowtarget )
					return low + 1;
				else
					return low;
			}
		}

		// Two elemsnts
		if( (high - low) == 1 )
		{
			if( ascending )
			{
				if( target <= *lowtarget )
					return low;
				else
					return high;
			}
			else
			{
				if( target <= *lowtarget )
					return high;
				else
					return low;
			}
		}

		// Sorry, try again...
		if( ascending )
		{
			if( target < *midtarget )
				high = mid;
			else
				low = mid;
		}
		else
		{
			if( target < *midtarget )
				low = mid;
			else
				high = mid;
		}
	}
}*/


//
// Delete an item at this index and construct a new one in it's place
//
template <class T>
bit8 ArrayList<T>::replace(IN T &node, sint32 pos)
{
  if (Entries_==0)
    return(FALSE);
  if (pos<0)
    pos=0;
  if (pos >= Entries_)
    pos=Entries_-1;

  (Vector_+pos)->~T();    // Call the destructor manually. Don't try this
                          //  at home kiddies!

  // Now put the replacement object in there...
  new((void *)(Vector_+pos)) T(node); // Trust me, this isn't a memory leak

  return(TRUE);
}



// Remove at the zero based index specified by 'pos'.  When removing from
// a slot, all others get shifted up by one.
template <class T>
bit8 ArrayList<T>::remove(sint32 pos)
{
  if (Entries_==0)
    return(FALSE);
  if (pos<0)
    pos=0;
  if (pos >= Entries_)
    pos=Entries_-1;

  (Vector_+pos)->~T();    // Call the destructor manually. Don't try this
                          //  at home kiddies!

  memmove(Vector_+pos,Vector_+pos+1,sizeof(T)*(Entries_-pos-1));

  Entries_--;

  // If we're at 33% usage or less, shrink the vector
  if ( (Entries_*3) <= Slots_)
    shrinkVector();

  return(TRUE);
}


// Remove at the zero based index specified by 'pos'.  When removing from
// a slot, all others get shifted up by one.
template <class T>
bit8 ArrayList<T>::remove(OUT T &node, sint32 pos)
{
  bit8 retval;
  retval=get(node,pos);
  if (retval==FALSE)
    return(FALSE);
  return(remove(pos));
}


// Remove the first node of the list
template <class T>
bit8 ArrayList<T>::removeHead(OUT T &node)
{
  return(remove(node,0));
}


// Remove the last node of the list
template <class T>
bit8 ArrayList<T>::removeTail(OUT T &node)
{
  return(remove(node,Entries_-1));
}

// get a pointer to the internally managed object.  Try and avoid this, but
//  sometimes efficiency requires it...
// get a copy of an item
template <class T>
bit8 ArrayList<T>::getPointer(OUT T **node,sint32 pos) RO
{
  if ((pos < 0)||(pos >= Entries_))
    return(FALSE);
  *node=&(Vector_[pos]);
  return(TRUE);
}


// get a copy of an item
template <class T>
bit8 ArrayList<T>::get(OUT T &node,sint32 pos) RO
{
  if ((pos < 0)||(pos >= Entries_))
    return(FALSE);
  node=Vector_[pos];
  return(TRUE);
}


// get a copy of the first node of the list
template <class T>
bit8 ArrayList<T>::getHead(OUT T &node) RO
{
  return(get(node,0));
}


// get a copy of the last node
template <class T>
bit8 ArrayList<T>::getTail(OUT T &node) RO
{
  return(get(node,Entries_-1));
}

// just for debugging
template <class T>
void ArrayList<T>::print(FILE *out)
{
  fprintf(out,"--------------------\n");
  //for (int i=0; i<Entries_; i++)
  //  Vector_[i].print();
  fprintf(out,"Entries: %d  Slots:  %d  sizeof(T): %d\n",Entries_,Slots_,
    sizeof(T));
  fprintf(out,"--------------------\n");
}

// Return the current length of the list
template <class T>
sint32 ArrayList<T>::length(void) RO
{
  return(Entries_);
}

// Grow the vector by a factor of 2X
template <class T>
bit8 ArrayList<T>::growVector(void)
{
  if (Entries_ < Slots_)   // Don't grow until we're at 100% usage
    return(FALSE);

  int   newSlots=Entries_*2;
  if(newSlots < INITIAL_SIZE)
    newSlots=INITIAL_SIZE;

  //fprintf(stderr,"Growing vector to: %d\n",newSlots);

  // The goofy looking new below prevents operator new from getting called
  //  unnecessarily.  This is severall times faster than allocating all of
  //  the slots as objects and then calling the assignment operator on them
  //  when they actually get used.
  //
  T *newVector=(T *)(new uint8[newSlots * sizeof(T)]);
  memset(newVector,0,newSlots * sizeof(T)); // zero just to be safe

  if (Vector_ != NULL)
    memcpy(newVector,Vector_,Entries_*sizeof(T));

  delete[]((uint8 *)Vector_);  // Get rid of the old vector without calling
                               //  destructors

  Vector_=newVector;
  Slots_=newSlots;

  return(TRUE);
}


// Shrink the vector by a factor of 2X
template <class T>
bit8 ArrayList<T>::shrinkVector(void)
{
  //fprintf(stderr,"Shrink called\n");

  // Don't need to shrink until usage goes below 33%
  if ( (Entries_*3) > Slots_)
    return(FALSE);

  int   newSlots=Slots_/2;
  if(newSlots < INITIAL_SIZE) // never shrink past initial size
    newSlots=INITIAL_SIZE;

  if (newSlots >= Slots_)   // don't need to shrink
    return(FALSE);

  //fprintf(stderr,"Shrinking vector to: %d\n",newSlots);

  // The goofy looking new below prevents operator new from getting called
  //  unnecessarily.  This is severall times faster than allocating all of
  //  the slots as objects and then calling the assignment operator on them
  //  when they actually get used.
  //
  T *newVector=(T *)(new uint8[newSlots * sizeof(T)]);

  if (Vector_ != NULL)    // Vector_ better not be NULL!
    memcpy(newVector,Vector_,Entries_*sizeof(T));

  delete[]((uint8 *)Vector_);  // Get rid of the old vector without calling
                               //  destructors

  Vector_=newVector;
  Slots_=newSlots;

  return(TRUE);
}
