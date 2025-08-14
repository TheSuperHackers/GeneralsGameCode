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

// FILE: BitFlags.h /////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, March 2002
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __BitFlags_H_
#define __BitFlags_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/STLTypedefs.h"

class INI;
class Xfer;
class AsciiString;

//-------------------------------------------------------------------------------------------------
/*
	BitFlags is a wrapper class that exists primarily because of a flaw in std::bitset<>.
	Although quite useful, it has horribly non-useful constructor, which (1) don't let
	us initialize stuff in useful ways, and (2) provide a default constructor that implicitly
	converts ints into bitsets in a "wrong" way (ie, it treats the int as a mask, not an index).
	So we wrap to correct this, but leave the bitset "exposed" so that we can use all the non-ctor
	functions on it directly (since it doesn't overload operator= to do the "wrong" thing, strangley enough)
*/
template <size_t NUMBITS>
class BitFlags
{
private:
	std::bitset<NUMBITS>				m_bits;
	static const char*					s_bitNameList[];

public:

	/*
		just a little syntactic sugar so that there is no "foo = 0" compatible constructor
	*/
	enum BogusInitType
	{
		kInit = 0
	};

	inline BitFlags()
	{
	}

	inline BitFlags(BogusInitType k, Int idx1)
	{
		m_bits.set(idx1);
	}

	inline BitFlags(BogusInitType k, Int idx1, Int idx2)
	{
		m_bits.set(idx1);
		m_bits.set(idx2);
	}

	inline BitFlags(BogusInitType k, Int idx1, Int idx2, Int idx3)
	{
		m_bits.set(idx1);
		m_bits.set(idx2);
		m_bits.set(idx3);
	}

	inline BitFlags(BogusInitType k, Int idx1, Int idx2, Int idx3, Int idx4)
	{
		m_bits.set(idx1);
		m_bits.set(idx2);
		m_bits.set(idx3);
		m_bits.set(idx4);
	}

	inline BitFlags(BogusInitType k, Int idx1, Int idx2, Int idx3, Int idx4, Int idx5)
	{
		m_bits.set(idx1);
		m_bits.set(idx2);
		m_bits.set(idx3);
		m_bits.set(idx4);
		m_bits.set(idx5);
	}

	inline BitFlags(BogusInitType k,
										Int idx1,
										Int idx2,
										Int idx3,
										Int idx4,
										Int idx5,
										Int idx6,
										Int idx7,
										Int idx8,
										Int idx9,
										Int idx10,
										Int idx11,
										Int idx12
									)
	{
		m_bits.set(idx1);
		m_bits.set(idx2);
		m_bits.set(idx3);
		m_bits.set(idx4);
		m_bits.set(idx5);
		m_bits.set(idx6);
		m_bits.set(idx7);
		m_bits.set(idx8);
		m_bits.set(idx9);
		m_bits.set(idx10);
		m_bits.set(idx11);
		m_bits.set(idx12);
	}

	inline Bool operator==(const BitFlags& that) const
	{
		return this->m_bits == that.m_bits;
	}

	inline Bool operator!=(const BitFlags& that) const
	{
		return this->m_bits != that.m_bits;
	}

	inline void set(Int i, Int val = 1)
	{
		m_bits.set(i, val);
	}

	inline Bool test(Int i) const
	{
		return m_bits.test(i);
	}

	//Tests for any bits that are set in both.
	inline Bool testForAny( const BitFlags& that ) const
	{
		BitFlags tmp = *this;
		tmp.m_bits &= that.m_bits;
		return tmp.m_bits.any();
	}

	//All argument bits must be set in our bits too in order to return TRUE
	inline Bool testForAll( const BitFlags& that ) const
	{
		DEBUG_ASSERTCRASH( that.any(), ("BitFlags::testForAll is always true if you ask about zero flags.  Did you mean that?") );

		BitFlags tmp = *this;
		tmp.m_bits.flip();
		tmp.m_bits &= that.m_bits;
		return !tmp.m_bits.any();
	}

	//None of the argument bits must be set in our bits in order to return TRUE
	inline Bool testForNone( const BitFlags& that ) const
	{
		BitFlags tmp = *this;
		tmp.m_bits &= that.m_bits;
		return !tmp.m_bits.any();
	}

	inline Int size() const
	{
		return m_bits.size();
	}

	inline Int count() const
	{
		return m_bits.count();
	}

	inline Bool any() const
	{
		return m_bits.any();
	}

	inline void flip()
	{
		m_bits.flip();
	}

	inline void clear()
	{
		m_bits.reset();
	}

	inline Int countIntersection(const BitFlags& that) const
	{
		BitFlags tmp = *this;
		tmp.m_bits &= that.m_bits;
		return tmp.m_bits.count();
	}

	inline Int countInverseIntersection(const BitFlags& that) const
	{
		BitFlags tmp = *this;
		tmp.m_bits.flip();
		tmp.m_bits &= that.m_bits;
		return tmp.m_bits.count();
	}

	inline Bool anyIntersectionWith(const BitFlags& that) const
	{
		/// @todo srj -- improve me.
		BitFlags tmp = that;
		tmp.m_bits &= m_bits;
		return tmp.m_bits.any();
	}

	inline void clear(const BitFlags& clr)
	{
		m_bits &= ~clr.m_bits;
	}

	inline void set(const BitFlags& set)
	{
		m_bits |= set.m_bits;
	}

	inline void clearAndSet(const BitFlags& clr, const BitFlags& set)
	{
		m_bits &= ~clr.m_bits;
		m_bits |= set.m_bits;
	}

	inline Bool testSetAndClear(const BitFlags& mustBeSet, const BitFlags& mustBeClear) const
	{
		/// @todo srj -- improve me.
		BitFlags tmp = *this;
		tmp.m_bits &= mustBeClear.m_bits;
		if (tmp.m_bits.any())
			return false;

		tmp = *this;
		tmp.m_bits.flip();
		tmp.m_bits &= mustBeSet.m_bits;
		if (tmp.m_bits.any())
			return false;

		return true;
	}

  static const char** getBitNames()
  {
    return s_bitNameList;
  }

  static const char* getNameFromSingleBit(Int i)
  {
    return (i >= 0 && i < NUMBITS) ? s_bitNameList[i] : NULL;
  }

  static Int getSingleBitFromName(const char* token)
  {
    Int i = 0;
	  for(const char** name = s_bitNameList; *name; ++name, ++i )
	  {
		  if( stricmp( *name, token ) == 0 )
		  {
        return i;
		  }
	  }
		return -1;
  }

  const char* getBitNameIfSet(Int i) const
  {
    return test(i) ? s_bitNameList[i] : NULL;
  }

  Bool setBitByName(const char* token)
  {
    Int i = getSingleBitFromName(token);
		if (i >= 0)
		{
      set(i);
			return true;
		}
		else
		{
	    return false;
		}
  }

	void parse(INI* ini, AsciiString* str);
	void parseSingleBit(INI* ini, AsciiString* str);
	void xfer(Xfer* xfer);
	static void parseFromINI(INI* ini, void* /*instance*/, void *store, const void* /*userData*/); ///< Returns a BitFlag
	static void parseSingleBitFromINI(INI* ini, void* /*instance*/, void *store, const void* /*userData*/); ///< Returns an int, the Index of the one bit

	void buildDescription( AsciiString* str ) const
	{
		if ( str == NULL )
			return;//sanity

		for( Int i = 0; i < size(); ++i )
		{
			const char* bitName = getBitNameIfSet(i);

			if (bitName != NULL)
			{
				str->concat( bitName );
				str->concat( ",\n");
			}
		}
	}


};

#endif // __BitFlags_H_

