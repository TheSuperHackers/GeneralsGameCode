/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#pragma once

#include <Utility/CppMacros.h>
#include <utility>

namespace stl
{

// Finds first matching element in vector-like container and erases it.
template <typename Container>
bool find_and_erase(Container& container, const typename Container::value_type& value)
{
	typename Container::const_iterator it = container.begin();
	for (; it != container.end(); ++it)
	{
		if (*it == value)
		{
			container.erase(it);
			return true;
		}
	}
	return false;
}

// Finds first matching element in vector-like container and removes it by swapping it with the last element.
// This is generally faster than erasing from a vector, but will change the element sorting.
template <typename Container>
bool find_and_erase_unordered(Container& container, const typename Container::value_type& value)
{
	typename Container::iterator it = container.begin();
	for (; it != container.end(); ++it)
	{
		if (*it == value)
		{
			*it = CPP_11(std::move)(container.back());
			container.pop_back();
			return true;
		}
	}
	return false;
}

} // namespace stl
