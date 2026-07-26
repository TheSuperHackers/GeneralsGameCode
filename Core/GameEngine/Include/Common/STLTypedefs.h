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
enum ParticleSystemID CPP_11(: Int);

#include <algorithm>
#include <bitset>
#include <Utility/hash_map_adapter.h>
#include <list>
#include <map>
#include <numeric>
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

// Used to cache off names of objects for faster lookup
typedef std::pair<AsciiString, Object*>										NamedRequest;
typedef std::vector<NamedRequest>													VecNamedRequests;
typedef std::vector<NamedRequest>::iterator								VecNamedRequestsIt;

// Rumor has it that a Vector of Bools gets stored as a bitfield internally.
typedef std::vector<Bool>																	BoolVector;
typedef std::vector<Bool>::iterator												BoolVectorIterator;

typedef std::map< NameKeyType, Real, std::less<NameKeyType>/**/> ProductionChangeMap;
typedef std::map< NameKeyType, VeterancyLevel, std::less<NameKeyType>/**/> ProductionVeterancyMap;

// Some useful, common hash and equal_to functors for use with hash_map
namespace rts
{

	// Generic hash functor. This should almost always be overridden for
	// specific types.
	template<typename T> struct hash
	{
		size_t operator()(const T& __t) const noexcept
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
		Bool operator()(const T& __t1, const T& __t2) const noexcept
		{
			return (__t1 == __t2);
		}
	};

	// Generic less_than_nocase functor. This should be overridden if there is no
	// operator<, or if that isn't the behavior desired. (For instance, in
	// the case of pointers, or strings.)
	template<typename T> struct less_than_nocase
	{
		bool operator()(const T& __t1, const T& __t2) const noexcept
		{
			return (__t1 < __t2);
		}
	};

#ifdef USING_STLPORT
	template<> struct hash<NameKeyType>
	{
		size_t operator()(NameKeyType nkt) const noexcept
		{
			std::hash<UnsignedInt> tmp;
			return tmp((UnsignedInt)nkt);
		}
	};

	template<> struct hash<DrawableID>
	{
		size_t operator()(DrawableID nkt) const noexcept
		{
			std::hash<UnsignedInt> tmp;
			return tmp((UnsignedInt)nkt);
		}
	};

	template<> struct hash<ObjectID>
	{
		size_t operator()(ObjectID nkt) const noexcept
		{
			std::hash<UnsignedInt> tmp;
			return tmp((UnsignedInt)nkt);
		}
	};

	template<> struct hash<ParticleSystemID>
	{
		size_t operator()(ParticleSystemID nkt) const noexcept
		{
			std::hash<UnsignedInt> tmp;
			return tmp((UnsignedInt)nkt);
		}
	};
#endif // USING_STLPORT

	template<> struct hash<const Char*>
	{
		size_t operator()(const Char* s) const noexcept
		{
#ifdef USING_STLPORT
			std::hash<const Char*> hasher;
			return hasher(s);
#else
			std::hash<std::string_view> hasher;
			return hasher(s);
#endif
		}
	};

	// This is the equal_to overload for char* comparisons. We compare the
	// strings to determine whether they are equal or not.
	// Other overloads should go into specific header files, not here (unless
	// they are to be used in lots of places.)
	template<> struct equal_to<const char*>
	{
		Bool operator()(const char* s1, const char* s2) const noexcept
		{
			return strcmp(s1, s2) == 0;
		}
	};

	template<> struct hash<AsciiString>
	{
		size_t operator()(const AsciiString& ast) const noexcept
		{
#ifdef USING_STLPORT
			std::hash<const char *> tmp;
			return tmp((const char *) ast.str());
#else
			std::hash<std::string_view> hasher;
			return hasher(std::string_view(ast.str(), ast.getLength()));
#endif
		}
	};

	template<> struct equal_to<AsciiString>
	{
		Bool operator()(const AsciiString& __t1, const AsciiString& __t2) const noexcept
		{
			return (__t1 == __t2);
		}
	};

	template<> struct less_than_nocase<AsciiString>
	{
		bool operator()(const AsciiString& __t1, const AsciiString& __t2) const noexcept
		{
			return (__t1.compareNoCase(__t2) < 0);
		}
	};

	template<> struct less_than_nocase<UnicodeString>
	{
		bool operator()(const UnicodeString& __t1, const UnicodeString& __t2) const noexcept
		{
			return (__t1.compareNoCase(__t2) < 0);
		}
	};

	// TheSuperHackers @info Functor to compare equal paths with. Ignores case, type of slash and trailing slash.
	struct equal_to_path
	{
		bool operator()(const char* __t1, const char* __t2) const noexcept
		{
			return comparePath(__t1, __t2) == 0;
		}

		bool operator()(const AsciiString& __t1, const AsciiString& __t2) const noexcept
		{
			return comparePath(__t1.str(), __t2.str()) == 0;
		}
	};

	// TheSuperHackers @info Functor to compare lesser paths with. Ignores case, type of slash and trailing slash.
	struct less_than_path
	{
		bool operator()(const char* __t1, const char* __t2) const noexcept
		{
			return comparePath(__t1, __t2) < 0;
		}

		bool operator()(const AsciiString& __t1, const AsciiString& __t2) const noexcept
		{
			return comparePath(__t1.str(), __t2.str()) < 0;
		}
	};

	// TheSuperHackers @info Functor to hash paths with. Ignores case, type of slash and trailing slash.
	struct hash_path
	{
		size_t operator()(const char* str) const noexcept
		{
			return hashPath(str);
		}

		size_t operator()(const AsciiString& str) const noexcept
		{
			return hashPath(str.str());
		}
	};

	// TheSuperHackers @info Structs to help create maps that can use C strings for
	// lookups without the need to allocate a string.
	template <typename String>
	struct string_key
	{
		typedef typename String::const_pointer const_pointer;

		static string_key temporary(const_pointer s)
		{
			string_key key;
			key.cstr = s;
			return key;
		}

		string_key(const_pointer s)
			: storage(s)
			, cstr(storage.str())
		{}

		string_key(const String& s)
			: storage(s)
			, cstr(storage.str())
		{}

		const_pointer c_str() const
		{
			return cstr;
		}

	private:
		string_key() {}

		String storage;
		const_pointer cstr;
	};

	struct string_key_hash
	{
		template <typename String>
		size_t operator()(const string_key<String>& key) const noexcept
		{
			typedef typename String::const_pointer const_pointer;
			return hash<const_pointer>()(key.c_str());
		}
	};

	struct string_key_equal
	{
		template <typename String>
		bool operator()(const string_key<String>& a, const string_key<String>& b) const noexcept
		{
			return strcmp(a.c_str(), b.c_str()) == 0;
		}
	};

	
	struct string_key_hash_path
	{
		template <typename String>
		size_t operator()(const string_key<String>& key) const noexcept
		{
			return hashPath(key.c_str());
		}
	};

	struct string_key_equal_to_path
	{
		template <typename String>
		bool operator()(const string_key<String>& a, const string_key<String>& b) const noexcept
		{
			return comparePath(a.c_str(), b.c_str()) == 0;
		}
	};

} // namespace rts
