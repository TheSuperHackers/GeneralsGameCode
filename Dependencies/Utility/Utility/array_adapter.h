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

// Use the standard array on modern compilers and a C++98 backport on VC6.
// The backport provides the container interface, but not the C++11 tuple
// interface (get/tuple_size/tuple_element), constexpr, or move semantics.
// VC6 also rejects aggregate initialization when the element type is const.
#pragma once

#if !(defined(_MSC_VER) && _MSC_VER < 1300)

#include <array>

#else

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>

namespace utility_array_detail
{
// VC6 cannot partially specialize array<T, 0>. Specialize the size first,
// then select the element type through a member template instead.
template <size_t N>
struct storage
{
	template <class T>
	struct for_type
	{
		typedef T type[N];
		static T* data(T* elements) { return elements; }
		static const T* const_data(const T* elements) { return elements; }
	};
};

template <>
struct storage<0>
{
	template <class T>
	struct for_type
	{
		struct type {};
		static T* data(type&) { return 0; }
		static const T* const_data(const type&) { return 0; }
	};
};
}

namespace std
{
template <class T, size_t N>
struct array
{
	typedef T value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T* iterator;
	typedef const T* const_iterator;

#ifdef USING_STLPORT
	_STLP_DECLARE_REVERSE_ITERATORS(reverse_iterator);
#else
	// The original VC6 standard library needs explicit iterator value types.
	typedef std::reverse_iterator<iterator, T, reference, pointer, difference_type> reverse_iterator;
	typedef std::reverse_iterator<const_iterator, T, const_reference, const_pointer, difference_type> const_reverse_iterator;
#endif

	typedef typename utility_array_detail::storage<N>::template for_type<T> storage_traits;
	// Public storage and no constructors preserve aggregate initialization:
	// std::array<int, 3> values = {{1, 2, 3}};
	typename storage_traits::type elements;

	iterator begin() { return data(); }
	const_iterator begin() const { return data(); }
	const_iterator cbegin() const { return begin(); }
	iterator end() { return N ? data() + N : data(); }
	const_iterator end() const { return N ? data() + N : data(); }
	const_iterator cend() const { return end(); }
	reverse_iterator rbegin() { return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	const_reverse_iterator crbegin() const { return rbegin(); }
	reverse_iterator rend() { return reverse_iterator(begin()); }
	const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
	const_reverse_iterator crend() const { return rend(); }

	size_type size() const { return N; }
	size_type max_size() const { return N; }
	bool empty() const { return N == 0; }

	reference operator[](size_type index) { return data()[index]; }
	const_reference operator[](size_type index) const { return data()[index]; }
	reference at(size_type index)
	{
		if (index >= N)
		{
			throw std::out_of_range("array::at");
		}
		return (*this)[index];
	}
	const_reference at(size_type index) const
	{
		if (index >= N)
		{
			throw std::out_of_range("array::at");
		}
		return (*this)[index];
	}
	reference front() { return (*this)[0]; }
	const_reference front() const { return (*this)[0]; }
	reference back() { return (*this)[N - 1]; }
	const_reference back() const { return (*this)[N - 1]; }
	pointer data() { return storage_traits::data(elements); }
	const_pointer data() const { return storage_traits::const_data(elements); }

	void fill(const T& value)
	{
		std::fill(begin(), end(), value);
	}
	void swap(array& other)
	{
		std::swap_ranges(begin(), end(), other.begin());
	}
};

template <class T, size_t N>
inline bool operator==(const array<T, N>& lhs, const array<T, N>& rhs)
{
	return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class T, size_t N>
inline bool operator!=(const array<T, N>& lhs, const array<T, N>& rhs)
{
	return !(lhs == rhs);
}

template <class T, size_t N>
inline bool operator<(const array<T, N>& lhs, const array<T, N>& rhs)
{
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <class T, size_t N>
inline bool operator>(const array<T, N>& lhs, const array<T, N>& rhs)
{
	return rhs < lhs;
}

template <class T, size_t N>
inline bool operator<=(const array<T, N>& lhs, const array<T, N>& rhs)
{
	return !(rhs < lhs);
}

template <class T, size_t N>
inline bool operator>=(const array<T, N>& lhs, const array<T, N>& rhs)
{
	return !(lhs < rhs);
}

// Use the existing generic std::swap for non-member swaps. VC6 cannot
// order an array-specific overload against the generic STL swap template.
}

#endif
