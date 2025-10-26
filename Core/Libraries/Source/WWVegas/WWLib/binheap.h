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
 *                 Project name : Buccaneer Bay                                                *
 *                                                                                             *
 *                    File name : Binary_Heap.h                                                *
 *                                                                                             *
 *                   Programmer : Mike Lytle                                                   *
 *                                                                                             *
 *                   Start date : 6/25/99                                                      *
 *                                                                                             *
 *                  Last update : 6/25/99                                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

/*=============================================================================================*/
// Includes.
/*=============================================================================================*/
#include <assert.h>
#include "bittype.h"

/*=============================================================================================*/
// Defines.
/*=============================================================================================*/

/*=============================================================================================*/
// Class declarations.
/*=============================================================================================*/
// WARNING!
// Any class used as an element of a heap for BinaryHeapClass must inherit HeapNodeClass.
template <class Key_Type>
class HeapNodeClass
{
	public:

		virtual uint32		Get_Heap_Location (void) const = 0;
		virtual void		Set_Heap_Location (uint32 location) = 0;

		// This is pure virtual so that any type of key can be used as long as it uses the comparison operators.
		virtual Key_Type	Heap_Key (void) const = 0;

};

// WARNING!
// To reduce the number of compares, element [0] is a sentinel.  It's key value must be the smallest or NULL.
// Keeps track of pointers to objects.
template <class Key_Type>
class BinaryHeapClass
{
	public:

		// This constructor uses elements that have already been allocated.
		BinaryHeapClass(HeapNodeClass<Key_Type> **allocated_list, unsigned int max_number_of_elements)
		{
			assert(allocated_list);
			assert(max_number_of_elements > 0);

			Elements = allocated_list;
			Max_Number_Of_Elements = max_number_of_elements;
			Number_Of_Elements = 0;
			Own_Array = false;
		}

		// This constructor allocates its own array of nodes
		BinaryHeapClass(unsigned int max_number_of_elements)
			:	Max_Number_Of_Elements (max_number_of_elements),
				Number_Of_Elements (0),
				Elements (NULL),
				Own_Array (false)
		{
			Resize_Array (max_number_of_elements);
		}

		// The destructor simply ensures the array is freed (if needs be)
		~BinaryHeapClass ()
		{
			Release_Array ();
		}

		// Reset all entries in the array to NULL
		void Flush_Array (void)
		{
			::memset (Elements, NULL, sizeof (HeapNodeClass<Key_Type> *) * Max_Number_Of_Elements);
			Number_Of_Elements = 0;
		}

		// Reallocate an array large enough to hold the elements
		void Resize_Array (unsigned int new_size)
		{
			// Start fresh
			Release_Array ();

			// Reallocate
			Elements						= W3DNEWARRAY HeapNodeClass<Key_Type> *[new_size];
			Max_Number_Of_Elements	= new_size;
			Number_Of_Elements		= 0;
			Own_Array					= true;

			// Initialize to NULL
			::memset (Elements, NULL, sizeof (HeapNodeClass<Key_Type> *) * new_size);
			return ;
		}

		void Release_Array (void)
		{
			if (Own_Array) {
				delete [] Elements;
				Elements = NULL;
				Number_Of_Elements = 0;
				Max_Number_Of_Elements = 0;
			}

			Own_Array = false;
			return ;
		}

		// Return the current number of elements.
		unsigned int Get_Number_Of_Elements()
		{
			return (Number_Of_Elements);
		}

		// Return the maximum number of elements.
		unsigned int Get_Max_Number_Of_Elements (void)
		{
			return (Max_Number_Of_Elements);
		}

		// Return a pointer to a node in the tree
		HeapNodeClass<Key_Type> *Peek_Node (unsigned int location)
		{
			return Elements[location];
		}

		// Insert an element into the tree.
		void Insert(HeapNodeClass<Key_Type> *node)
		{

			// Increment the number of elements in the heap.
			unsigned int i = ++Number_Of_Elements;

			// Doesn't handle the case of adding more elements than there is memory for.
			assert(Number_Of_Elements < Max_Number_Of_Elements);

			// Find the elements's place in the tree.  Remember: the smallest element is the root.
			while (Greater_Than(Elements[i / 2], node))
			{
				Elements[i] = Elements[i / 2];
				Elements[i]->Set_Heap_Location(i);
				i /= 2;
			}

			Elements[i] = node;
			Elements[i]->Set_Heap_Location(i);
		}

		// Move the element up in the tree if necessary.  Use this if the key value becomes smaller when it is
		// already in the heap.
		void Percolate_Up(unsigned int location)
		{
			assert(location < Max_Number_Of_Elements);

			unsigned int i = location;
			HeapNodeClass<Key_Type> *node = Elements[i];

			// Find the elements's place in the tree.  Remember: the smallest element is the root.
			while (Greater_Than(Elements[i / 2], node))
			{
				Elements[i] = Elements[i / 2];
				Elements[i]->Set_Heap_Location(i);
				i /= 2;
			}

			Elements[i] = node;
			Elements[i]->Set_Heap_Location(i);
		}

		// Take the smallest element out of the tree and reorder
		HeapNodeClass<Key_Type>* Remove_Min (void)
		{
			unsigned int	child;
			HeapNodeClass<Key_Type>*  	last_element;
			HeapNodeClass<Key_Type>* 	min_element;

			if (Number_Of_Elements == 0) {
				return NULL;
			}

			assert(Number_Of_Elements > 0);
			assert(Elements);

			// The smallest element is always at this position.
			min_element = Elements[1];
			if (min_element != NULL) {
				min_element->Set_Heap_Location (0);
			}

			last_element = Elements[Number_Of_Elements];

			// Decrement the number of elements in the tree.
			Number_Of_Elements--;

			for (unsigned int i = 1; (i * 2) <= Number_Of_Elements; i = child)
			{
				// Find a smaller child.
				child = i * 2;
				if ((child != Number_Of_Elements) && (Less_Than(Elements[child + 1], Elements[child])))
				{
					child++;
				}

				// Percolate down one level.
				if (Greater_Than(last_element, Elements[child]))
				{
					Elements[i] = Elements[child];
					Elements[i]->Set_Heap_Location(i);
				}
				else
				{
					break;
				}
			}

			Elements[i] = last_element;
			Elements[i]->Set_Heap_Location(i);

			return (min_element);
		}

	private:

		bool	Less_Than(HeapNodeClass<Key_Type> *op1, HeapNodeClass<Key_Type> *op2)
		{
			if (op1 == 0)
			{
				return (true);
			}

			if (op2 == 0)
			{
				return (false);
			}

			return (op1->Heap_Key() < op2->Heap_Key());
		}

		bool	Less_Than_Equal(HeapNodeClass<Key_Type> *op1, HeapNodeClass<Key_Type> *op2)
		{
			if (op1 == 0)
			{
				return (true);
			}

			if (op2 == 0)
			{
				return (false);
			}

			return (op1->Heap_Key() <= op2->Heap_Key());
		}


		bool	Greater_Than(HeapNodeClass<Key_Type> *op1, HeapNodeClass<Key_Type> *op2)
		{
			if (op1 == 0)
			{
				return (false);
			}

			if (op2 == 0)
			{
				return (true);
			}

			return (op1->Heap_Key() > op2->Heap_Key());
		}

	private:

		// The list of elements.
		HeapNodeClass<Key_Type>	**Elements;

		// The number of allocated elements.
		unsigned int					Max_Number_Of_Elements;

		// Current number of elements in the tree.
		unsigned int					Number_Of_Elements;

		// Flag to indicate who owns the memory for the
		// binary tree.
		bool								Own_Array;
};
