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

// FILE: Dict.cpp
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  Dict.cpp
//
// Created:    Steven Johnson, November 2001
//
// Desc:       General-purpose dictionary class
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Dict.h"
#include "Common/GameMemory.h"

// -----------------------------------------------------
void Dict::DictPair::copyFrom(DictPair* that)
{
	Dict::DataType curType = this->getType();
	Dict::DataType newType = that->getType();
	if (curType != newType)
	{
		clear();
	}

	switch(newType)
	{
		case DICT_BOOL:
		case DICT_INT:
		case DICT_REAL:
			*this = *that;
			break;
		case DICT_ASCIISTRING:
			this->m_key = that->m_key;
			*this->asAsciiString() = *that->asAsciiString();
			break;
		case DICT_UNICODESTRING:
			this->m_key = that->m_key;
			*this->asUnicodeString() = *that->asUnicodeString();
			break;
	}
}

// -----------------------------------------------------
void Dict::DictPair::clear()
{
	switch (getType())
	{
		case DICT_BOOL:
		case DICT_INT:
		case DICT_REAL:
			m_value = nullptr;
			break;
		case DICT_ASCIISTRING:
			asAsciiString()->clear();
			break;
		case DICT_UNICODESTRING:
			asUnicodeString()->clear();
			break;
	}
}

// -----------------------------------------------------
void Dict::DictPair::setNameAndType(NameKeyType key, Dict::DataType type)
{
	Dict::DataType curType = getType();
	if (curType != type)
	{
		clear();
	}
	m_key = createKey(key, type);
}

// -----------------------------------------------------
#ifdef RTS_DEBUG
void Dict::validate() const
{
	if (!m_data) return;
	DEBUG_ASSERTCRASH(m_data->m_refCount > 0, ("m_refCount is zero"));
	DEBUG_ASSERTCRASH(m_data->m_refCount < 32000, ("m_refCount is suspiciously large"));
	DEBUG_ASSERTCRASH(m_data->m_numPairsAllocated > 0, ("m_numPairsAllocated is zero"));
	DEBUG_ASSERTCRASH(m_data->m_numPairsUsed >= 0, ("m_numPairsUsed is neg"));
	DEBUG_ASSERTCRASH(m_data->m_numPairsAllocated >= m_data->m_numPairsUsed, ("m_numPairsAllocated too small"));
	DEBUG_ASSERTCRASH(m_data->m_numPairsAllocated < 1024, ("m_numPairsAllocated suspiciously large"));
}
#endif

// -----------------------------------------------------
Dict::DictPair* Dict::findPairByKey(NameKeyType key) const
{
	DEBUG_ASSERTCRASH(key != NAMEKEY_INVALID, ("invalid namekey!"));
	DEBUG_ASSERTCRASH((UnsignedInt)key < (1L<<23), ("namekey too large!"));
	if (!m_data)
		return nullptr;
	DictPair* base = m_data->peek();
	Int minIdx = 0;
	Int maxIdx = m_data->m_numPairsUsed;
	while (minIdx < maxIdx)
	{
		Int midIdx = (((minIdx + maxIdx) - 1) >> 1);
		DictPair* mid = base + midIdx;
		NameKeyType midKey = mid->getName();
		if (key > midKey)
			minIdx = midIdx + 1;
		else if (key < midKey)
			maxIdx = midIdx;
		else
			return mid;
	}

	return nullptr;
}

// -----------------------------------------------------
Dict::DictPair *Dict::ensureUnique(int numPairsNeeded, Bool preserveData, DictPair *pairToTranslate)
{
	if (m_data &&
			m_data->m_refCount == 1 &&
			m_data->m_numPairsAllocated >= numPairsNeeded)
	{
		// no buffer manhandling is needed (it's already large enough, and unique to us)
		return pairToTranslate;
	}

	Dict::DictPairData* newData = nullptr;
	if (numPairsNeeded > 0)
	{
		DEBUG_ASSERTCRASH(TheDynamicMemoryAllocator != nullptr, ("Cannot use dynamic memory allocator before its initialization. Check static initialization order."));
		DEBUG_ASSERTCRASH(numPairsNeeded <= MAX_LEN, ("Dict::ensureUnique exceeds max pairs length %d with requested length %d", MAX_LEN, numPairsNeeded));
		int minBytes = sizeof(Dict::DictPairData) + numPairsNeeded*sizeof(Dict::DictPair);
		int actualBytes = TheDynamicMemoryAllocator->getActualAllocationSize(minBytes);
		// note: be certain to alloc with zero; we'll take advantage of the fact that all-zero
		// is a bit-pattern that happens to init all our pairs to legal values:
		// type BOOL, key INVALID, value FALSE.
		newData = (Dict::DictPairData*)TheDynamicMemoryAllocator->allocateBytes(actualBytes, "Dict::ensureUnique");
		newData->m_refCount = 1;
		newData->m_numPairsAllocated = (actualBytes - sizeof(Dict::DictPairData))/sizeof(Dict::DictPair);
		newData->m_numPairsUsed = 0;

		if (preserveData && m_data)
		{
			Dict::DictPair* src = m_data->peek();
			Dict::DictPair* dst = newData->peek();
			for (Int i = 0; i < m_data->m_numPairsUsed; ++i, ++src, ++dst)
				dst->copyFrom(src);
			newData->m_numPairsUsed = m_data->m_numPairsUsed;
		}
	}

	Int delta = 0;
	if (pairToTranslate && m_data)
		delta = pairToTranslate - m_data->peek();

	releaseData();
	m_data = newData;

	if (pairToTranslate && m_data)
		pairToTranslate = m_data->peek() + delta;

	return pairToTranslate;
}


// -----------------------------------------------------
void Dict::clear()
{
	releaseData();
	m_data = nullptr;
}

// -----------------------------------------------------
void Dict::releaseData()
{
	if (m_data)
	{
		if (--m_data->m_refCount == 0)
		{
			Dict::DictPair* src = m_data->peek();
			for (Int i = 0; i < m_data->m_numPairsUsed; ++i, ++src)
				src->clear();
			TheDynamicMemoryAllocator->freeBytes(m_data);
		}
		m_data = nullptr;
	}
}

// -----------------------------------------------------
Dict::Dict(Int numPairsToPreAllocate) : m_data(nullptr)
{

	/*
		This class plays some skanky games, in the name of memory and code
		efficiency; it assumes all the data types will fit into a pointer.
		This is currently true, but if that assumption ever changes, all hell
		will break loose. So we do a quick check to assure this...
	*/
	DEBUG_ASSERTCRASH(sizeof(Bool) <= sizeof(void*) &&
										sizeof(Int) <= sizeof(void*) &&
										sizeof(Real) <= sizeof(void*) &&
										sizeof(AsciiString) <= sizeof(void*) &&
										sizeof(UnicodeString) <= sizeof(void*), ("oops, this code needs attention"));

	if (numPairsToPreAllocate)
		ensureUnique(numPairsToPreAllocate, false, nullptr);	// will throw on error
}

// -----------------------------------------------------
Dict& Dict::operator=(const Dict& src)
{
	validate();
	if (&src != this)
	{
		releaseData();
		m_data = src.m_data;
		if (m_data)
			++m_data->m_refCount;
	}
	validate();
	return *this;
}

// -----------------------------------------------------
Dict::DataType Dict::getType(NameKeyType key) const
{
	validate();
	DictPair* pair = findPairByKey(key);
	if (pair)
		return pair->getType();
	return DICT_NONE;
}

// -----------------------------------------------------
Bool Dict::getBool(NameKeyType key, Bool *exists/*=nullptr*/) const
{
	validate();
	DictPair* pair = findPairByKey(key);
	if (pair && pair->getType() == DICT_BOOL)
	{
		if (exists) *exists = true;
		return *pair->asBool();
	}
	DEBUG_ASSERTCRASH(exists != nullptr, ("dict key missing, or of wrong type"));	// only assert if they didn't check result
	if (exists) *exists = false;
	return false;
}

// -----------------------------------------------------
Int Dict::getInt(NameKeyType key, Bool *exists/*=nullptr*/) const
{
	validate();
	DictPair* pair = findPairByKey(key);
	if (pair && pair->getType() == DICT_INT)
	{
		if (exists) *exists = true;
		return *pair->asInt();
	}
	DEBUG_ASSERTCRASH(exists != nullptr,("dict key missing, or of wrong type"));	// only assert if they didn't check result
	if (exists) *exists = false;
	return 0;
}

// -----------------------------------------------------
Real Dict::getReal(NameKeyType key, Bool *exists/*=nullptr*/) const
{
	validate();
	DictPair* pair = findPairByKey(key);
	if (pair && pair->getType() == DICT_REAL)
	{
		if (exists) *exists = true;
		return *pair->asReal();
	}
	DEBUG_ASSERTCRASH(exists != nullptr,("dict key missing, or of wrong type"));	// only assert if they didn't check result
	if (exists) *exists = false;
	return 0.0f;
}

// -----------------------------------------------------
AsciiString Dict::getAsciiString(NameKeyType key, Bool *exists/*=nullptr*/) const
{
	validate();
	DictPair* pair = findPairByKey(key);
	if (pair && pair->getType() == DICT_ASCIISTRING)
	{
		if (exists) *exists = true;
		return *pair->asAsciiString();
	}
	DEBUG_ASSERTCRASH(exists != nullptr,("dict key missing, or of wrong type"));	// only assert if they didn't check result
	if (exists) *exists = false;
	return AsciiString::TheEmptyString;
}

// -----------------------------------------------------
UnicodeString Dict::getUnicodeString(NameKeyType key, Bool *exists/*=nullptr*/) const
{
	validate();
	DictPair* pair = findPairByKey(key);
	if (pair && pair->getType() == DICT_UNICODESTRING)
	{
		if (exists) *exists = true;
		return *pair->asUnicodeString();
	}
	DEBUG_ASSERTCRASH(exists != nullptr,("dict key missing, or of wrong type"));	// only assert if they didn't check result
	if (exists) *exists = false;
	return UnicodeString::TheEmptyString;
}

// -----------------------------------------------------
Bool Dict::getNthBool(Int n) const
{
	validate();
	DEBUG_ASSERTCRASH(n >= 0 && n < getPairCount(), ("n out of range"));
	if (m_data)
	{
		DictPair* pair = &m_data->peek()[n];
		if (pair && pair->getType() == DICT_BOOL)
			return *pair->asBool();
	}
	DEBUG_CRASH(("dict key missing, or of wrong type"));
	return false;
}

// -----------------------------------------------------
Int Dict::getNthInt(Int n) const
{
	validate();
	DEBUG_ASSERTCRASH(n >= 0 && n < getPairCount(), ("n out of range"));
	if (m_data)
	{
		DictPair* pair = &m_data->peek()[n];
		if (pair && pair->getType() == DICT_INT)
			return *pair->asInt();
	}
	DEBUG_CRASH(("dict key missing, or of wrong type"));
	return 0;
}

// -----------------------------------------------------
Real Dict::getNthReal(Int n) const
{
	validate();
	DEBUG_ASSERTCRASH(n >= 0 && n < getPairCount(), ("n out of range"));
	if (m_data)
	{
		DictPair* pair = &m_data->peek()[n];
		if (pair && pair->getType() == DICT_REAL)
			return *pair->asReal();
	}
	DEBUG_CRASH(("dict key missing, or of wrong type"));
	return 0.0f;
}

// -----------------------------------------------------
AsciiString Dict::getNthAsciiString(Int n) const
{
	validate();
	DEBUG_ASSERTCRASH(n >= 0 && n < getPairCount(), ("n out of range"));
	if (m_data)
	{
		DictPair* pair = &m_data->peek()[n];
		if (pair && pair->getType() == DICT_ASCIISTRING)
			return *pair->asAsciiString();
	}
	DEBUG_CRASH(("dict key missing, or of wrong type"));
	return AsciiString::TheEmptyString;
}

// -----------------------------------------------------
UnicodeString Dict::getNthUnicodeString(Int n) const
{
	validate();
	DEBUG_ASSERTCRASH(n >= 0 && n < getPairCount(), ("n out of range"));
	if (m_data)
	{
		DictPair* pair = &m_data->peek()[n];
		if (pair && pair->getType() == DICT_UNICODESTRING)
			return *pair->asUnicodeString();
	}
	DEBUG_CRASH(("dict key missing, or of wrong type"));
	return UnicodeString::TheEmptyString;
}

// -----------------------------------------------------
Dict::DictPair *Dict::setPrep(NameKeyType key, Dict::DataType type)
{
	DictPair* pair = findPairByKey(key);
	Int pairsNeeded = getPairCount();
	if (!pair)
		++pairsNeeded;
	pair = ensureUnique(pairsNeeded, true, pair);
	if (!pair)
	{
		pair = &m_data->peek()[m_data->m_numPairsUsed++];
	}
	pair->setNameAndType(key, type);
	DEBUG_ASSERTCRASH(pair, ("pair must not be null here"));
	return pair;
}

// -----------------------------------------------------
void Dict::sortPairs()
{
	if (!m_data)
		return;

	// yer basic shellsort.
	for (Int gap = m_data->m_numPairsUsed >> 1; gap > 0; gap >>= 1)
	{
		for (Int i = gap; i < m_data->m_numPairsUsed; i++)
		{
			for (Int j = i - gap; j >= 0; j -= gap)
			{
				DictPair* a = m_data->peek() + j;
				DictPair* b = m_data->peek() + j + gap;
				if (a->getName() > b->getName())
				{
					DictPair tmp = *a;
					*a = *b;
					*b = tmp;
				}
				else
				{
					break;
				}
			}
		}
	}
}

// -----------------------------------------------------
void Dict::setBool(NameKeyType key, Bool value)
{
	validate();
	DictPair* pair = setPrep(key, DICT_BOOL);
	*pair->asBool() = value;
	sortPairs();
	validate();
}

// -----------------------------------------------------
void Dict::setInt(NameKeyType key, Int value)
{
	validate();
	DictPair* pair = setPrep(key, DICT_INT);
	*pair->asInt() = value;
	sortPairs();
	validate();
}

// -----------------------------------------------------
void Dict::setReal(NameKeyType key, Real value)
{
	validate();
	DictPair* pair = setPrep(key, DICT_REAL);
	*pair->asReal() = value;
	sortPairs();
	validate();
}

// -----------------------------------------------------
void Dict::setAsciiString(NameKeyType key, const AsciiString& value)
{
	validate();
	DictPair* pair = setPrep(key, DICT_ASCIISTRING);
	*pair->asAsciiString() = value;
	sortPairs();
	validate();
}

// -----------------------------------------------------
void Dict::setUnicodeString(NameKeyType key, const UnicodeString& value)
{
	validate();
	DictPair* pair = setPrep(key, DICT_UNICODESTRING);
	*pair->asUnicodeString() = value;
	sortPairs();
	validate();
}

// -----------------------------------------------------
Bool Dict::remove(NameKeyType key)
{
	validate();
	DictPair* pair = findPairByKey(key);
	if (pair)
	{
		pair = ensureUnique(m_data->m_numPairsUsed, true, pair);
		pair->setNameAndType((NameKeyType)0x7fffffff, DICT_BOOL);
		sortPairs();
		--m_data->m_numPairsUsed;
		validate();
		return true;
	}
	DEBUG_CRASH(("dict key missing in remove"));
	return false;
}

// -----------------------------------------------------
void Dict::copyPairFrom(const Dict& that, NameKeyType key)
{
	this->validate();
	DictPair* thatPair = that.findPairByKey(key);
	if (thatPair)
	{
		DictPair* thisPair = this->setPrep(key, thatPair->getType());
		thisPair->copyFrom(thatPair);
		this->sortPairs();
	}
	else
	{
		if (this->findPairByKey(key))
			this->remove(key);
	}
	this->validate();
}
