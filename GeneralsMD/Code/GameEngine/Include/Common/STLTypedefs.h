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

// FILE: STLTypedefs.h ////////////////////////////////////////////////////////////
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
// File name:  STLTypedefs.h
//
// Created:    John McDonald
//
// Desc:			 @todo
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef __STLTYPEDEFS_H__
#define __STLTYPEDEFS_H__

//-----------------------------------------------------------------------------
// srj sez: this must come first, first, first.
#define _STLP_USE_NEWALLOC					1
//#define _STLP_USE_CUSTOM_NEWALLOC		STLSpecialAlloc
class STLSpecialAlloc;

//-----------------------------------------------------------------------------
#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"
#include "Common/GameCommon.h"
#include "Common/GameMemory.h"

//-----------------------------------------------------------------------------


// FORWARD DECLARATIONS
class Object;
enum NameKeyType CPP_11(: Int);
enum ObjectID CPP_11(: Int);
enum DrawableID CPP_11(: Int);

#include <algorithm>
#include <bitset>
#include <Utility/hash_map_adapter.h>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

// List of AsciiStrings to allow list of ThingTemplate names from INI and such
typedef std::list< AsciiString >													AsciiStringList;
typedef std::list< AsciiString >::iterator								AsciiStringListIterator;
typedef std::list< AsciiString >::const_iterator					AsciiStringListConstIterator;

// One is used in GameLogic to keep track of objects to be destroyed
typedef std::list<Object *>																ObjectPointerList;
typedef std::list<Object *>::iterator											ObjectPointerListIterator;

typedef std::vector<ObjectID>															ObjectIDVector;
typedef std::vector<ObjectID>::iterator										ObjectIDVectorIterator;

// Terribly useful, especially with Bezier curves
typedef std::vector<Coord3D> VecCoord3D;
typedef VecCoord3D::iterator VecCoord3DIt;

// Used for cursor->3D position request caching in the heightmap
typedef std::pair<ICoord2D, Coord3D>											PosRequest;
typedef std::vector<PosRequest>														VecPosRequests;
typedef std::vector<PosRequest>::iterator									VecPosRequestsIt;

// Used to cache off names of objects for faster lookup
typedef std::pair<AsciiString, Object*>										NamedRequest;
typedef std::vector<NamedRequest>													VecNamedRequests;
typedef std::vector<NamedRequest>::iterator								VecNamedRequestsIt;

// Rumor has it that a Vector of Bools gets stored as a bitfield internally.
typedef std::vector<Bool>																	BoolVector;
typedef std::vector<Bool>::iterator												BoolVectorIterator;

typedef std::map< NameKeyType, Real, std::less<NameKeyType> > ProductionChangeMap;
typedef std::map< NameKeyType, VeterancyLevel, std::less<NameKeyType> > ProductionVeterancyMap;

// Some useful, common hash and equal_to functors for use with hash_map
namespace rts 
{
	
	// Generic hash functor. This should almost always be overridden for 
	// specific types.
	template<typename T> struct hash
	{
		size_t operator()(const T& __t) const
		{ 
			std::hash<T> tmp;
			return tmp(__t);
		}
	};

	// Generic equal_to functor. This should be overridden if there is no
	// operator==, or if that isn't the behavior desired. (For instance, in
	// the case of pointers.)
	template<typename T> struct equal_to
	{
		Bool operator()(const T& __t1, const T& __t2) const
		{
			return (__t1 == __t2);
		}
	};

	// Generic less_than_nocase functor. This should be overridden if there is no
	// operator<, or if that isn't the behavior desired. (For instance, in
	// the case of pointers, or strings.)
	template<typename T> struct less_than_nocase
	{
		bool operator()(const T& __t1, const T& __t2) const
		{
			return (__t1 < __t2);
		}
	};

	template<> struct hash<NameKeyType>
	{
		size_t operator()(NameKeyType nkt) const
		{ 
			std::hash<UnsignedInt> tmp;
			return tmp((UnsignedInt)nkt);
		}
	};

	template<> struct hash<DrawableID>
	{
		size_t operator()(DrawableID nkt) const
		{ 
			std::hash<UnsignedInt> tmp;
			return tmp((UnsignedInt)nkt);
		}
	};

	template<> struct hash<ObjectID>
	{
		size_t operator()(ObjectID nkt) const
		{ 
			std::hash<UnsignedInt> tmp;
			return tmp((UnsignedInt)nkt);
		}
	};

	// This is the equal_to overload for char* comparisons. We compare the
	// strings to determine whether they are equal or not.
	// Other overloads should go into specific header files, not here (unless
	// they are ot be used in lots of places.)
	template<> struct equal_to<const char*>
	{
		Bool operator()(const char* s1, const char* s2) const
		{
			return strcmp(s1, s2) == 0;
		}
	};

	template<> struct hash<AsciiString>
	{
		size_t operator()(const AsciiString& ast) const
		{
#ifdef USING_STLPORT
			std::hash<const char *> tmp;
			return tmp((const char *) ast.str());
#else
			// TheSuperHackers @bugfix xezon 16/03/2024 Re-implements hash function that works with non-STLPort.
			std::hash<std::string_view> hasher;
			return hasher(std::string_view(ast.str(), ast.getLength()));
#endif
		}
	};

	template<> struct equal_to<AsciiString>
	{
		Bool operator()(const AsciiString& __t1, const AsciiString& __t2) const
		{
			return (__t1 == __t2);
		}
	};

	template<> struct less_than_nocase<AsciiString>
	{
		bool operator()(const AsciiString& __t1, const AsciiString& __t2) const
		{
			return (__t1.compareNoCase(__t2) < 0);
		}
	};

	template<> struct less_than_nocase<UnicodeString>
	{
		bool operator()(const UnicodeString& __t1, const UnicodeString& __t2) const
		{
			return (__t1.compareNoCase(__t2) < 0);
		}
	};
}

#endif /* __STLTYPEDEFS_H__ */
