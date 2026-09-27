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

#pragma once

#include <utility>

#if __cplusplus >= 201103L

#define MOVE_TO(x) x

#else

// C++98-compatible emulation of:
//
//   MOVE_TO(dest) = std::move(src);
//
// In C++11:
//
//   std::move(src) produces T&&
//
// In C++98:
//
//	 std::move(src) produces a move_source<T> proxy.
//
// MOVE_TO(dest) produces a move_to_proxy<T>.
//
// The assignment is therefore:
//
//   move_to_proxy<T>::operator=(move_source<T>)
//
// which performs the swap-based move.
//
// Requires the destination type to have a member swap() and a default
// constructor.

namespace move_to_detail
{
	template <typename T>
	class move_source
	{
	public:
		explicit move_source(T& value)
			: m_value(value)
		{
		}

		T& get() const
		{
			return m_value;
		}

	private:
		T& m_value;
	};

	template <typename T>
	inline move_source<T> move(T& value)
	{
		return move_source<T>(value);
	}

	template <typename T>
	class move_to_proxy
	{
	public:
		explicit move_to_proxy(T& value)
			: m_value(value)
		{
		}

		move_to_proxy& operator=(move_source<T> source)
		{
			T empty;
			m_value.swap(source.get());
			source.get().swap(empty);
			return *this;
		}

	private:
		T& m_value;
	};

	template <typename T>
	inline move_to_proxy<T> make_move_to_proxy(T& value)
	{
		return move_to_proxy<T>(value);
	}

} // namespace move_to_detail

namespace std
{
	template <typename T>
	inline ::move_to_detail::move_source<T> move(T& value)
	{
		return ::move_to_detail::move(value);
	}
}

#define MOVE_TO(x) ::move_to_detail::make_move_to_proxy(x)

#endif
