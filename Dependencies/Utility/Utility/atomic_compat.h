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

// This file contains VC6 compatible atomic classes for the long, int, short and bool types
#pragma once

#if !(defined(_MSC_VER) && _MSC_VER < 1300) || __cplusplus >= 201103L
#include <atomic>
#else
#include <windows.h>
#include "interlocked_adapter.h"

namespace std
{
	// Capture unsupported types and error during compilation
	template<class T>
	class atomic;

	// Atomic long for VC6 internally uses windows interlocked functions
	template<>
	class atomic<long>
	{
	public:
		atomic(long value = 0)
			: m_stored(value)
		{
		}

		long load() const
		{
			return InterlockedCompareExchange(&m_stored, 0, 0);
		}

		void store(long value)
		{
			InterlockedExchange(&m_stored, value);
		}

		long exchange(long value)
		{
			return InterlockedExchange(&m_stored, value);
		}

		long fetch_add(long value)
		{
			return InterlockedExchangeAdd(&m_stored, value);
		}

		long fetch_sub(long value)
		{
			return InterlockedExchangeAdd(
				&m_stored,
				(long)(0UL - (unsigned long)value)
			);
		}

		long fetch_or(long value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = load();
				newValue = oldValue | value;
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return oldValue;
		}

		long fetch_and(long value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = load();
				newValue = oldValue & value;
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return oldValue;
		}

		long fetch_xor(long value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = load();
				newValue = oldValue ^ value;
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return oldValue;
		}

		bool compare_exchange_strong(long& expected, long desired)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_weak(long& expected, long desired)
		{
			return compare_exchange(expected, desired);
		}

		// Assignment from the underlying type.
		long operator=(long value)
		{
			store(value);
			return value;
		}

		// Implicit read, matching the usual std::atomic integral behavior.
		operator long() const
		{
			return load();
		}

		// Increment/decrement.
		long operator++()
		{
			return fetch_add(1) + 1;
		}

		long operator++(int)
		{
			return fetch_add(1);
		}

		long operator--()
		{
			return fetch_sub(1) - 1;
		}

		long operator--(int)
		{
			return fetch_sub(1);
		}

		// Arithmetic operators.
		long operator+=(long value)
		{
			return fetch_add(value) + value;
		}

		long operator-=(long value)
		{
			return fetch_sub(value) - value;
		}

		// Optional bitwise operators.
		long operator|=(long value)
		{
			return fetch_or(value) | value;
		}

		long operator&=(long value)
		{
			return fetch_and(value) & value;
		}

		long operator^=(long value)
		{
			return fetch_xor(value) ^ value;
		}

	private:
		atomic(const atomic&) FUNCTION_DELETE;
		atomic& operator=(const atomic&) FUNCTION_DELETE;

		bool compare_exchange(long& expected, long desired)
		{
			long oldValue = InterlockedCompareExchange(&m_stored, desired, expected);

			if (oldValue == expected)
				return true;

			expected = oldValue;
			return false;
		}

		mutable volatile long m_stored;
	};


	// Atomic short for VC6 internally stores data as a long and uses windows interlocked functions
	template<>
	class atomic<short>
	{
	public:
		atomic(short value = 0)
			: m_stored(encode(value))
		{
		}

		short load() const
		{
			long value = InterlockedCompareExchange(&m_stored, 0, 0);
			return decode(value);
		}

		void store(short value)
		{
			InterlockedExchange(&m_stored, encode(value));
		}

		short exchange(short value)
		{
			long oldValue = InterlockedExchange(&m_stored, encode(value));
			return decode(oldValue);
		}

		short fetch_add(short value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = InterlockedCompareExchange(&m_stored, 0, 0);

				newValue = encode( (short)((int)decode(oldValue) + (int)value) );
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return decode(oldValue);
		}

		short fetch_sub(short value)
		{
			return fetch_add((short)(-((int)value)));
		}

		short fetch_or(short value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = InterlockedCompareExchange(&m_stored, 0, 0);

				newValue = encode( (short)((unsigned short)decode(oldValue) | value) );
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return decode(oldValue);
		}

		short fetch_and(short value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = InterlockedCompareExchange(&m_stored, 0, 0);

				newValue = encode( (short)((unsigned short)decode(oldValue) & value) );
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return decode(oldValue);
		}

		short fetch_xor(short value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = InterlockedCompareExchange(&m_stored, 0, 0);

				newValue = encode( (short)((unsigned short)decode(oldValue) ^ value) );
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return decode(oldValue);
		}

		bool compare_exchange_strong(short& expected, short desired)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_weak(short& expected, short desired)
		{
			return compare_exchange(expected, desired);
		}

		short operator=(short value)
		{
			store(value);
			return value;
		}

		// Implicit read, matching the usual std::atomic integral behavior.
		operator short() const
		{
			return load();
		}

		short operator++()
		{
			return (short)(fetch_add(1) + 1);
		}

		short operator++(int)
		{
			return fetch_add(1);
		}

		short operator--()
		{
			return (short)(fetch_sub(1) - 1);
		}

		short operator--(int)
		{
			return fetch_sub(1);
		}

		short operator+=(short value)
		{
			return (short)(fetch_add(value) + value);
		}

		short operator-=(short value)
		{
			return (short)(fetch_sub(value) - value);
		}

		// Optional bitwise operators.
		short operator|=(short value)
		{
			return (short)(fetch_or(value) | value);
		}

		short operator&=(short value)
		{
			return (short)(fetch_and(value) & value);
		}

		short operator^=(short value)
		{
			return (short)(fetch_xor(value) ^ value);
		}

	private:

		static long encode(short value)
		{
			// Store the signed short as a sign-extended long.
			return (long)value;
		}

		static short decode(long value)
		{
			// Short is only m_stored in the lower 16 bits of the internal long
			return (short)value;
		}

		atomic(const atomic&) FUNCTION_DELETE;
		atomic& operator=(const atomic&) FUNCTION_DELETE;

		bool compare_exchange(short& expected, short desired)
		{
			long oldValue = InterlockedCompareExchange(&m_stored, encode(desired), encode(expected));

			if (oldValue == encode(expected))
				return true;

			expected = decode(oldValue);
			return false;
		}

		// For the short implementation, the value occupies the lower 16 bits and is sign-extended to a long
		mutable volatile long m_stored;
	};


	// Atomic int for VC6 internally stores data as a long and uses windows interlocked functions
	template<>
	class atomic<int>
	{
	public:
		atomic(int value = 0)
			: m_stored((long)value)
		{
		}

		int load() const
		{
			return (int)InterlockedCompareExchange(&m_stored, 0, 0);
		}

		void store(int value)
		{
			InterlockedExchange(&m_stored, (long)value);
		}

		int exchange(int value)
		{
			return (int)InterlockedExchange(&m_stored, (long)value);
		}

		int fetch_add(int value)
		{
			return (int)InterlockedExchangeAdd(&m_stored, (long)value);
		}

		int fetch_sub(int value)
		{
			return (int)InterlockedExchangeAdd(
				&m_stored,
				(long)(0UL - (unsigned long)value)
			);
		}

		int fetch_or(int value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = (long)load();
				newValue = oldValue | (long)value;
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return (int)oldValue;
		}

		int fetch_and(int value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = (long)load();
				newValue = oldValue & (long)value;
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return (int)oldValue;
		}

		int fetch_xor(int value)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = (long)load();
				newValue = oldValue ^ (long)value;
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return (int)oldValue;
		}

		bool compare_exchange_strong(int& expected, int desired)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_weak(int& expected, int desired)
		{
			return compare_exchange(expected, desired);
		}

		// Assignment from the underlying type.
		int operator=(int value)
		{
			store(value);
			return value;
		}

		// Implicit read, matching the usual std::atomic integral behavior.
		operator int() const
		{
			return load();
		}

		// Increment/decrement.
		int operator++()
		{
			return fetch_add(1) + 1;
		}

		int operator++(int)
		{
			return fetch_add(1);
		}

		int operator--()
		{
			return fetch_sub(1) - 1;
		}

		int operator--(int)
		{
			return fetch_sub(1);
		}

		// Arithmetic operators.
		int operator+=(int value)
		{
			return fetch_add(value) + value;
		}

		int operator-=(int value)
		{
			return fetch_sub(value) - value;
		}

		// Optional bitwise operators.
		int operator|=(int value)
		{
			return fetch_or(value) | value;
		}

		int operator&=(int value)
		{
			return fetch_and(value) & value;
		}

		int operator^=(int value)
		{
			return fetch_xor(value) ^ value;
		}

	private:
		atomic(const atomic&) FUNCTION_DELETE;
		atomic& operator=(const atomic&) FUNCTION_DELETE;

		bool compare_exchange(int& expected, int desired)
		{
			long oldValue = InterlockedCompareExchange(&m_stored, (long)desired, (long)expected);

			if (oldValue == expected)
				return true;

			expected = oldValue;
			return false;
		}

		mutable volatile long m_stored;
	};


	// Atomic bool for VC6 internally uses a long and windows interlocked functions
	template<>
	class atomic<bool>
	{
	public:
		atomic(bool value = 0)
			: m_stored(encode(value))
		{
		}

		bool load() const
		{
			long value = InterlockedCompareExchange(&m_stored, 0, 0);
			return decode(value);
		}

		void store(bool value)
		{
			InterlockedExchange(&m_stored, encode(value));
		}

		bool exchange(bool value)
		{
			long oldValue = InterlockedExchange(&m_stored, encode(value));
			return decode(oldValue);
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

		// basic operators that make sense for boolean types
		bool operator!() const
		{
			return !load();
		}

		bool operator==(bool value) const
		{
			return load() == value;
		}

		bool operator!=(bool value) const
		{
			return load() != value;
		}

	private:

		static long encode(bool value)
		{
			return value ? 1 : 0;
		}

		static bool decode(long value)
		{
			return value != 0;
		}

		atomic(const atomic&) FUNCTION_DELETE;
		atomic& operator=(const atomic&) FUNCTION_DELETE;

		bool compare_exchange(bool& expected, bool desired)
		{
			long oldValue = InterlockedCompareExchange(&m_stored, encode(desired), encode(expected));

			if (decode(oldValue) == expected)
				return true;

			expected = decode(oldValue);
			return false;
		}

		mutable volatile long m_stored;
	};

}

#endif //!(defined(_MSC_VER) && _MSC_VER < 1300)
