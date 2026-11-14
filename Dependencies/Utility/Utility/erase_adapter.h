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
// templates that erase the matching elements one by one with the iterator that erase returns.
// Use them instead of writing a loop that erases the elements that equal a value or match a predicate.
//
// The VC6 functions differ from the standard functions in these points:
// - They accept any container whose erase(iterator) returns the next iterator. They do not work on std::map, std::set
//   and the other associative containers, because their erase(iterator) returns void in STLport.
// - They erase each element on its own, so on a vector or deque they take quadratic time where the standard
//   functions take linear time. The remaining elements keep their order, like with the standard functions.
// - std::erase converts the value to the element type before it compares them.
// - They return size_t instead of the size_type of the container.

#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1300

#include <functional>
#include <stddef.h>

namespace std
{
template <class Container, class Predicate>
inline size_t erase_if(Container& container, Predicate pred)
{
	size_t count = 0;
	typename Container::iterator it = container.begin();
	while (it != container.end())
	{
		if (pred(*it))
		{
			it = container.erase(it);
			++count;
		}
		else
		{
			++it;
		}
	}
	return count;
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
