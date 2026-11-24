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
 *                     $Archive:: /Commando/Code/wwlib/vector.cpp                             $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/16/02 11:40a                                              $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include "Vector.h"

/*
**	The following template function can be located here ONLY if all the instantiations are
**	declared in a header file this module includes. By placing the template functions here,
**	it speeds up compiler operation and reduces object module size.
*/

//----------------------------------------------------------------------------------------------

// Takes the memory pointer provided and assigns that as the array data pointer.
// WARNING: You must make sure that the memory specified is large enough to contain the bits
// specified.
BooleanVectorClass::BooleanVectorClass(unsigned size, unsigned char * array) :
	BitCount(size),
	Copy(false),
	LastIndex(-1),
	BitArray(0, nullptr)
{
	BitArray.Resize(((size + (8-1)) / 8), array);
//	LastIndex = -1;
//	BitCount = size;
}


BooleanVectorClass::BooleanVectorClass(BooleanVectorClass const & vector)
{
	LastIndex = -1;
	*this = vector;
}


// The vector is copied into an already constructed existing vector. The values from the existing
// vector are destroyed by this copy.
BooleanVectorClass & BooleanVectorClass::operator =(BooleanVectorClass const & vector)
{
	Fixup();
	Copy = vector.Copy;
	LastIndex = vector.LastIndex;
	BitArray = vector.BitArray;
	BitCount = vector.BitCount;
	return(*this);
}


// Boolean vectors are equal if the bit count and bit values are identical.
bool BooleanVectorClass::operator == (const BooleanVectorClass & vector) const
{
	Fixup(LastIndex);
	return(BitCount == vector.BitCount && BitArray == vector.BitArray);
}


// An index value used with a boolean vector must be less than the value specified in as the new
// size.
// WARNING: The boolean array might be reallocated or even deleted by this routine.
int BooleanVectorClass::Resize(unsigned size)
{
	Fixup();

	if (size > 0) {

		/*
		**	Record the previous bit count of the boolean vector. This is used
		**	to determine if the array has grown in size and thus clearing is
		**	necessary.
		*/
		unsigned oldsize = BitCount;

		/*
		**	Actually resize the bit array. Since this is a bit packed array,
		**	there are 8 elements per byte (rounded up).
		*/
		int success = BitArray.Resize(((size + (8-1)) / 8));

		/*
		**	Since there is no default constructor for bit packed integers, a manual
		**	clearing of the bits is required.
		*/
		BitCount = size;
		if (success && oldsize < size) {
			for (unsigned index = oldsize; index < size; index++) {
				(*this)[index] = 0;
			}
		}

		return(success);
	}

	/*
	**	Resizing to zero is the same as clearing and deallocating the array.
	**	This is always successful.
	*/
	Clear();
	return(true);
}


// Frees any allocated memory.
// WARNING: The boolean vector cannot be used until it is resized to a non null condition.
void BooleanVectorClass::Clear()
{
	Fixup();
	BitCount = 0;
	BitArray.Clear();
}


// Clears all boolean values to false. This is the preferred (and quick) method.
void BooleanVectorClass::Reset()
{
	LastIndex = -1;
	if (BitArray.Length() > 0) {
		memset(&BitArray[0], '\0', BitArray.Length());
	}
}


// This is the preferred (and fast) way to set all boolean elements to true.
void BooleanVectorClass::Set()
{
	LastIndex = -1;
	if (BitArray.Length() > 0) {
		memset(&BitArray[0], '\xFF', BitArray.Length());
	}
}


// Updates the boolean vector to a known state. Use this routine to set the boolean value copy to
// match the appropriate bit in the boolean array. The boolean array will be updated with any
// changes from the last time a value was fetched from the boolean vector. By using this update
// method, the boolean array can be treated as a normal array even though the elements are composed
// of otherwise inaccessible bits. If the index is -1, then the previous value will be updated into
// the vector array, but no new value will be fetched from it.
// WARNING: Always call this routine with "-1" if any direct manipulation of the bit array is to
// occur. This ensures that the bit array is accurate.
void BooleanVectorClass::Fixup(int index) const
{
	/*
	**	If the requested index value is illegal, then force the index
	**	to be -1. This is the default non-index value.
	*/
	if ((unsigned)index >= (unsigned)BitCount) {
		index = -1;
	}

	/*
	**	If the new index is different than the previous index, there might
	**	be some fixing up required.
	*/
	if (index != LastIndex) {

		/*
		**	If the previously fetched boolean value was changed, then update
		**	the boolean array accordingly.
		*/
		if (LastIndex != -1) {
			assert(unsigned(LastIndex) < unsigned(BitCount));
			Set_Bit((void*)&BitArray[0], LastIndex, Copy);
		}

		/*
		**	If this new current index is valid, then fill in the reference boolean
		**	value with the appropriate data from the bit array.
		*/
		if (index != -1) {
			assert(unsigned(index) < unsigned(BitCount));
			((unsigned char &)Copy) = (unsigned char)Get_Bit((void*)&BitArray[0], index);
//			((unsigned char&)Copy) = Get_Bit((void*)&BitArray[0], index);
		}

		((BooleanVectorClass *)this)->LastIndex = index;
	}
}


void BooleanVectorClass::Init(unsigned size, unsigned char * array)
{
	Copy = false;
	LastIndex = -1;
	BitCount = size;
	BitArray.Resize(((size + (8-1)) / 8), array);
}


void BooleanVectorClass::Init(unsigned size)
{
	Copy = false;
	LastIndex = -1;
	BitCount = size;
	BitArray.Resize(((size + (8-1)) / 8));
}
