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

// This file contains a VC6 compatible atomic template class for the numeric signed and unsigned; long, int, short and char types.
// It also contains a specialized template for the bool type.
#pragma once

#if !(defined(_MSC_VER) && _MSC_VER < 1300)

#include <atomic>

#else

#include <windows.h>

#include "Utility/interlocked_adapter.h"

namespace std
{
	// Capture unsupported types and error during compilation
	template<typename T>
	struct atomic_trait;

	template<>
	struct atomic_trait<long>
	{
		static long to_long(long value) { return value; }
		static long from_long(long value) { return value; }
		static unsigned long to_unsigned_long(long value) { return (unsigned long)value; }
		static long from_unsigned_long(unsigned long value) { return (long)value; }
	};

	template<>
	struct atomic_trait<unsigned long>
	{
		static long to_long(unsigned long value) { return (long)value; }
		static unsigned long from_long(long value) { return (unsigned long)value; }
		static unsigned long to_unsigned_long(unsigned long value) { return value; }
		static unsigned long from_unsigned_long(unsigned long value) { return value; }
	};

	template<>
	struct atomic_trait<int>
	{
		static long to_long(int value) { return (long)value; }
		static int from_long(long value) { return (int)value; }
		static unsigned long to_unsigned_long(int value) { return (unsigned long)value; }
		static int from_unsigned_long(unsigned long value) { return (int)value; }
	};

	template<>
	struct atomic_trait<unsigned int>
	{
		static long to_long(unsigned int value) { return (long)value; }
		static unsigned int from_long(long value) { return (unsigned int)value; }
		static unsigned long to_unsigned_long(unsigned int value) { return (unsigned long)value; }
		static unsigned int from_unsigned_long(unsigned long value) { return (unsigned int)value; }
	};

	template<>
	struct atomic_trait<short>
	{
		static long to_long(short value) { return (long)value; }
		static short from_long(long value) { return (short)value; }
		static unsigned long to_unsigned_long(short value) { return (unsigned short)value; }
		static short from_unsigned_long(unsigned long value) { return (short)(unsigned short)value; }
	};

	template<>
	struct atomic_trait<unsigned short>
	{
		static long to_long(unsigned short value) { return (long)value; }
		static unsigned short from_long(long value) { return (unsigned short)value; }
		static unsigned long to_unsigned_long(unsigned short value) { return (unsigned long)value; }
		static unsigned short from_unsigned_long(unsigned long value) { return (unsigned short)value; }
	};

	template<>
	struct atomic_trait<char>
	{
		static long to_long(char value) { return (long)value; }
		static char from_long(long value) { return (char)value; }
		static unsigned long to_unsigned_long(char value) { return (unsigned char)value; }
		static char from_unsigned_long(unsigned long value) { return (char)(unsigned char)value; }
	};

	template<>
	struct atomic_trait<unsigned char>
	{
		static long to_long(unsigned char value) { return (long)value; }
		static unsigned char from_long(long value) { return (unsigned char)value; }
		static unsigned long to_unsigned_long(unsigned char value) { return (unsigned char)value; }
		static unsigned char from_unsigned_long(unsigned long value) { return (unsigned char)value; }
	};

	template <>
	struct atomic_trait<signed char>
	{
		static long to_long(signed char value) { return (long)value; }
		static signed char from_long(long value) { return (signed char)value; }
		static unsigned long to_unsigned_long(signed char value) { return (unsigned char)value; }
		static signed char from_unsigned_long(unsigned long value) { return (signed char)(unsigned char)value; }
	};


	// The VC6 std::atomic compatible template only supports some base types and uses long internally as storage
	template<typename T>
	class atomic
	{
	public:
		atomic(T value = (T)0)
			: m_stored(atomic_trait<T>::to_long(value))
		{
		}

		T load() const
		{
			const long oldValue = InterlockedCompareExchange(&m_stored, 0, 0);
			return atomic_trait<T>::from_long(oldValue);
		}

		void store(T value)
		{
			InterlockedExchange(&m_stored, atomic_trait<T>::to_long(value));
		}

		T exchange(T value)
		{
			const long oldValue = InterlockedExchange(&m_stored, atomic_trait<T>::to_long(value));
			return atomic_trait<T>::from_long(oldValue);
		}

		T fetch_add(T value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = InterlockedCompareExchange(&m_stored, 0, 0);

				const T oldTyped = atomic_trait<T>::from_long(oldValue);

				const unsigned long oldUnsigned = atomic_trait<T>::to_unsigned_long(oldTyped);
				const unsigned long valueUnsigned = atomic_trait<T>::to_unsigned_long(value);
				const unsigned long newUnsigned = oldUnsigned + valueUnsigned;

				const T newTyped = atomic_trait<T>::from_unsigned_long(newUnsigned);

				newValue = atomic_trait<T>::to_long(newTyped);
			} while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return atomic_trait<T>::from_long(oldValue);
		}

		T fetch_sub(T value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = InterlockedCompareExchange(&m_stored, 0, 0);

				const T oldTyped = atomic_trait<T>::from_long(oldValue);

				const unsigned long oldUnsigned = atomic_trait<T>::to_unsigned_long(oldTyped);
				const unsigned long valueUnsigned = atomic_trait<T>::to_unsigned_long(value);
				const unsigned long newUnsigned = oldUnsigned - valueUnsigned;

				const T newTyped = atomic_trait<T>::from_unsigned_long(newUnsigned);

				newValue = atomic_trait<T>::to_long(newTyped);
			} while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return atomic_trait<T>::from_long(oldValue);
		}

		T fetch_or(T value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = InterlockedCompareExchange(&m_stored, 0, 0);

				const T oldTyped = atomic_trait<T>::from_long(oldValue);
				const T newTyped = (T)(oldTyped | value);

				newValue = atomic_trait<T>::to_long(newTyped);
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return atomic_trait<T>::from_long(oldValue);
		}

		T fetch_and(T value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = InterlockedCompareExchange(&m_stored, 0, 0);

				const T oldTyped = atomic_trait<T>::from_long(oldValue);
				const T newTyped = (T)(oldTyped & value);

				newValue = atomic_trait<T>::to_long(newTyped);
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return atomic_trait<T>::from_long(oldValue);
		}

		T fetch_xor(T value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = InterlockedCompareExchange(&m_stored, 0, 0);

				const T oldTyped = atomic_trait<T>::from_long(oldValue);
				const T newTyped = (T)(oldTyped ^ value);

				newValue = atomic_trait<T>::to_long(newTyped);
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return atomic_trait<T>::from_long(oldValue);
		}

		bool compare_exchange_strong(T& expected, T desired)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_weak(T& expected, T desired)
		{
			return compare_exchange(expected, desired);
		}

		// Assignment from the underlying type.
		T operator=(T value)
		{
			store(value);
			return value;
		}

		// Implicit read, matching the usual std::atomic integral behavior.
		operator T() const
		{
			return load();
		}

		// Increment/decrement.
		T operator++()
		{
			const T oldValue = fetch_add((T)1);
			return add_result(oldValue, (T)1);
		}

		T operator++(int)
		{
			return fetch_add((T)1);
		}

		T operator--()
		{
			const T oldValue = fetch_sub((T)1);
			return sub_result(oldValue, (T)1);
		}

		T operator--(int)
		{
			return fetch_sub((T)1);
		}

		// Arithmetic operators.
		T operator+=(T value)
		{
			const T oldValue = fetch_add(value);
			return add_result(oldValue, value);
		}

		T operator-=(T value)
		{
			const T oldValue = fetch_sub(value);
			return sub_result(oldValue, value);
		}

		// Optional bitwise operators.
		T operator|=(T value)
		{
			const T oldValue = fetch_or(value);

			const unsigned long newValue = atomic_trait<T>::to_unsigned_long(oldValue) | atomic_trait<T>::to_unsigned_long(value);
			return atomic_trait<T>::from_unsigned_long(newValue);
		}

		T operator&=(T value)
		{
			const T oldValue = fetch_and(value);

			const unsigned long newValue = atomic_trait<T>::to_unsigned_long(oldValue) & atomic_trait<T>::to_unsigned_long(value);
			return atomic_trait<T>::from_unsigned_long(newValue);
		}

		T operator^=(T value)
		{
			const T oldValue = fetch_xor(value);

			const unsigned long newValue = atomic_trait<T>::to_unsigned_long(oldValue) ^ atomic_trait<T>::to_unsigned_long(value);
			return atomic_trait<T>::from_unsigned_long(newValue);
		}

	private:
		atomic(const atomic&) FUNCTION_DELETE;
		atomic& operator=(const atomic&) FUNCTION_DELETE;

		bool compare_exchange(T& expected, T desired)
		{
			const long storedExpected = atomic_trait<T>::to_long(expected);
			const long storedDesired = atomic_trait<T>::to_long(desired);

			const long oldValue = InterlockedCompareExchange(&m_stored, storedDesired, storedExpected);

			if (oldValue == storedExpected)
				return true;

			expected = atomic_trait<T>::from_long(oldValue);
			return false;
		}

		T add_result(T oldValue, T value)
		{
			const unsigned long oldUnsigned =	atomic_trait<T>::to_unsigned_long(oldValue);
			const unsigned long valueUnsigned =	atomic_trait<T>::to_unsigned_long(value);

			return atomic_trait<T>::from_unsigned_long(oldUnsigned + valueUnsigned);
		}

		T sub_result(T oldValue, T value)
		{
			const unsigned long oldUnsigned =	atomic_trait<T>::to_unsigned_long(oldValue);
			const unsigned long valueUnsigned =	atomic_trait<T>::to_unsigned_long(value);

			return atomic_trait<T>::from_unsigned_long(oldUnsigned - valueUnsigned);
		}

		mutable volatile long m_stored;
	};


	// Atomic bool is a specialized template and for VC6 it internally uses a long and windows interlocked functions
	template<>
	class atomic<bool>
	{
	public:
		atomic(bool value = false)
			: m_stored(to_long(value))
		{
		}

		bool load() const
		{
			const long value = InterlockedCompareExchange(&m_stored, 0, 0);
			return from_long(value);
		}

		void store(bool value)
		{
			InterlockedExchange(&m_stored, to_long(value));
		}

		bool exchange(bool value)
		{
			const long oldValue = InterlockedExchange(&m_stored, to_long(value));
			return from_long(oldValue);
		}

		bool compare_exchange_strong(bool& expected, bool desired)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_weak(bool& expected, bool desired)
		{
			return compare_exchange(expected, desired);
		}

		// Assignment from the underlying type.
		bool operator=(bool value)
		{
			store(value);
			return value;
		}

		// Implicit read, matching the usual std::atomic integral behavior.
		operator bool() const
		{
			return load();
		}

	private:

		static long to_long(bool value)
		{
			return value ? 1 : 0;
		}

		static bool from_long(long value)
		{
			return value != 0;
		}

		atomic(const atomic&) FUNCTION_DELETE;
		atomic& operator=(const atomic&) FUNCTION_DELETE;

		bool compare_exchange(bool& expected, bool desired)
		{
			const long storedExpected = to_long(expected);
			const long storedDesired = to_long(desired);

			const long oldValue = InterlockedCompareExchange(&m_stored, storedDesired, storedExpected);

			if (oldValue == storedExpected)
				return true;

			expected = from_long(oldValue);
			return false;
		}

		mutable volatile long m_stored;
	};

}

#endif
