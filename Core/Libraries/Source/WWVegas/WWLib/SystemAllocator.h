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

#include <cstddef> // std::size_t, std::ptrdiff_t
#include <new> // std::bad_alloc


namespace stl
{

// STL allocator that uses the Operating System allocator functions. Useful if allocations are meant to bypass new and delete, malloc and free.

template <typename T>
class system_allocator
{
public:

	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template <typename U>
	struct rebind
	{
		typedef system_allocator<U> other;
	};

	system_allocator() throw() {}

#if !(defined(_MSC_VER) && _MSC_VER < 1300)
	system_allocator(const system_allocator&) throw() {}
#endif

	template <typename U>
	system_allocator(const system_allocator<U>&) throw() {}

	~system_allocator() throw() {}

	pointer address(reference x) const { return &x; }
	const_pointer address(const_reference x) const { return &x; }

	pointer allocate(size_type n, const void* = 0)
	{
		if (n > max_size())
			throw std::bad_alloc();

		void* p = GlobalAlloc(GMEM_FIXED, n * sizeof(T));
		if (!p)
			throw std::bad_alloc();
		return static_cast<pointer>(p);
	}

	void deallocate(pointer p, size_type)
	{
		GlobalFree(p);
	}

	void construct(pointer p, const T& val)
	{
		new (static_cast<void*>(p)) T(val);
	}

	void destroy(pointer p)
	{
		p->~T();
	}

	size_type max_size() const throw()
	{
		return ~size_type(0) / sizeof(T);
	}
};

// Allocators of same type are always equal
template <typename T1, typename T2>
bool operator==(const system_allocator<T1>&, const system_allocator<T2>&) throw() {
	return true;
}

template <typename T1, typename T2>
bool operator!=(const system_allocator<T1>&, const system_allocator<T2>&) throw() {
	return false;
}

} // namespace stl


#if defined(USING_STLPORT)

// This tells STLport how to rebind system_allocator
namespace std
{
	template <class _Tp1, class _Tp2>
	struct __stl_alloc_rebind_helper;

	template <class Tp1, class Tp2>
	inline stl::system_allocator<Tp2>& __stl_alloc_rebind(stl::system_allocator<Tp1>& a, const Tp2*) {
		return *reinterpret_cast<stl::system_allocator<Tp2>*>(&a);
	}

	template <class Tp1, class Tp2>
	inline const stl::system_allocator<Tp2>& __stl_alloc_rebind(const stl::system_allocator<Tp1>& a, const Tp2*) {
		return *reinterpret_cast<const stl::system_allocator<Tp2>*>(&a);
	}
}

#endif
