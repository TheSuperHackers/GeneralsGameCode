/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

// This file provides std::erase and std::erase_if for std::vector, std::list and std::deque. C++20 declares them in
// each container header, so this file includes those headers on modern compilers. VC6 has no C++20 and gets function
// templates that move the kept elements to the front with std::remove_if and then erase the rest in one call.
// Use them instead of writing a loop that erases the elements that equal a value or match a predicate.
//
// The VC6 functions differ from the standard functions in these points:
// - They accept any sequence container with erase(first, last). They do not work on std::map, std::set and the other
//   associative containers.
// - On a std::list they move elements by assignment where the standard function relinks the nodes.
// - std::erase converts the value to the element type before it compares them.
// - They return size_t instead of the size_type of the container.

#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1300

#include <algorithm>
#include <functional>
#include <stddef.h>

namespace std
{
template <class Container, class Predicate>
inline size_t erase_if(Container& container, Predicate pred)
{
	const size_t oldSize = container.size();
	container.erase(std::remove_if(container.begin(), container.end(), pred), container.end());
	return oldSize - container.size();
}

template <class Container, class T>
inline size_t erase(Container& container, const T& value)
{
	typedef typename Container::value_type value_type;
	return std::erase_if(container, std::bind2nd(std::equal_to<value_type>(), value));
}
}

#else

#include <deque>
#include <list>
#include <vector>

#endif
