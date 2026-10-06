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
	// The VC6 atomic classes take the memory orders for source compatibility with the standard classes.
	// Their functions are always sequentially consistent, which satisfies every order.
	enum memory_order
	{
		memory_order_relaxed,
		memory_order_consume,
		memory_order_acquire,
		memory_order_release,
		memory_order_acq_rel,
		memory_order_seq_cst
	};


	// Capture unsupported types and error during compilation
	template<typename T>
	struct atomic_trait;

	template<>
	struct atomic_trait<long>
	{
		static long to_long(long value) { return value; }
		static long from_long(long value) { return value; }
	};

	template<>
	struct atomic_trait<unsigned long>
	{
		static long to_long(unsigned long value) { return (long)value; }
		static unsigned long from_long(long value) { return (unsigned long)value; }
	};

	template<>
	struct atomic_trait<int>
	{
		static long to_long(int value) { return (long)value; }
		static int from_long(long value) { return (int)value; }
	};

	template<>
	struct atomic_trait<unsigned int>
	{
		static long to_long(unsigned int value) { return (long)value; }
		static unsigned int from_long(long value) { return (unsigned int)value; }
	};

	template<>
	struct atomic_trait<short>
	{
		static long to_long(short value) { return (long)value; }
		static short from_long(long value) { return (short)value; }
	};

	template<>
	struct atomic_trait<unsigned short>
	{
		static long to_long(unsigned short value) { return (long)value; }
		static unsigned short from_long(long value) { return (unsigned short)value; }
	};

	template<>
	struct atomic_trait<char>
	{
		static long to_long(char value) { return (long)value; }
		static char from_long(long value) { return (char)value; }
	};

	template<>
	struct atomic_trait<unsigned char>
	{
		static long to_long(unsigned char value) { return (long)value; }
		static unsigned char from_long(long value) { return (unsigned char)value; }
	};

	template <>
	struct atomic_trait<signed char>
	{
		static long to_long(signed char value) { return (long)value; }
		static signed char from_long(long value) { return (signed char)value; }
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

		T load(memory_order = memory_order_seq_cst) const
		{
			return atomic_trait<T>::from_long(load_stored());
		}

		void store(T value, memory_order = memory_order_seq_cst)
		{
			InterlockedExchange(&m_stored, atomic_trait<T>::to_long(value));
		}

		T exchange(T value, memory_order = memory_order_seq_cst)
		{
			const long oldValue = InterlockedExchange(&m_stored, atomic_trait<T>::to_long(value));
			return atomic_trait<T>::from_long(oldValue);
		}

		T fetch_add(T value, memory_order = memory_order_seq_cst)
		{
			return atomic_trait<T>::from_long(fetch_modify(operation_add, atomic_trait<T>::to_long(value)));
		}

		T fetch_sub(T value, memory_order = memory_order_seq_cst)
		{
			return atomic_trait<T>::from_long(fetch_modify(operation_sub, atomic_trait<T>::to_long(value)));
		}

		T fetch_or(T value, memory_order = memory_order_seq_cst)
		{
			return atomic_trait<T>::from_long(fetch_modify(operation_or, atomic_trait<T>::to_long(value)));
		}

		T fetch_and(T value, memory_order = memory_order_seq_cst)
		{
			return atomic_trait<T>::from_long(fetch_modify(operation_and, atomic_trait<T>::to_long(value)));
		}

		T fetch_xor(T value, memory_order = memory_order_seq_cst)
		{
			return atomic_trait<T>::from_long(fetch_modify(operation_xor, atomic_trait<T>::to_long(value)));
		}

		bool compare_exchange_strong(T& expected, T desired, memory_order = memory_order_seq_cst)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_strong(T& expected, T desired, memory_order, memory_order)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_weak(T& expected, T desired, memory_order = memory_order_seq_cst)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_weak(T& expected, T desired, memory_order, memory_order)
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
			return modify(operation_add, (T)1);
		}

		T operator++(int)
		{
			return fetch_add((T)1);
		}

		T operator--()
		{
			return modify(operation_sub, (T)1);
		}

		T operator--(int)
		{
			return fetch_sub((T)1);
		}

		// Arithmetic operators.
		T operator+=(T value)
		{
			return modify(operation_add, value);
		}

		T operator-=(T value)
		{
			return modify(operation_sub, value);
		}

		// Optional bitwise operators.
		T operator|=(T value)
		{
			return modify(operation_or, value);
		}

		T operator&=(T value)
		{
			return modify(operation_and, value);
		}

		T operator^=(T value)
		{
			return modify(operation_xor, value);
		}

	private:
		enum operation
		{
			operation_add,
			operation_sub,
			operation_or,
			operation_and,
			operation_xor
		};

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

		long load_stored() const
		{
			return InterlockedCompareExchange(&m_stored, 0, 0);
		}

		static long compute(operation op, long stored, long operand)
		{
			long result;

			switch (op)
			{
			// Adds and subtracts unsigned, because that wraps around without undefined behavior.
			case operation_add: result = (long)((unsigned long)stored + (unsigned long)operand); break;
			case operation_sub: result = (long)((unsigned long)stored - (unsigned long)operand); break;
			case operation_or: result = stored | operand; break;
			case operation_and: result = stored & operand; break;
			default: result = stored ^ operand; break;
			}

			// Wraps the result around to the value range of the type.
			return atomic_trait<T>::to_long(atomic_trait<T>::from_long(result));
		}

		// Returns the stored value from before the operation.
		long fetch_modify(operation op, long operand)
		{
			long oldValue;
			long newValue;

			do
			{
				oldValue = load_stored();
				newValue = compute(op, oldValue, operand);
			}
			while (InterlockedCompareExchange(&m_stored, newValue, oldValue) != oldValue);

			return oldValue;
		}

		// Returns the value from after the operation.
		T modify(operation op, T value)
		{
			const long operand = atomic_trait<T>::to_long(value);
			const long oldValue = fetch_modify(op, operand);

			return atomic_trait<T>::from_long(compute(op, oldValue, operand));
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

		bool load(memory_order = memory_order_seq_cst) const
		{
			const long value = InterlockedCompareExchange(&m_stored, 0, 0);
			return from_long(value);
		}

		void store(bool value, memory_order = memory_order_seq_cst)
		{
			InterlockedExchange(&m_stored, to_long(value));
		}

		bool exchange(bool value, memory_order = memory_order_seq_cst)
		{
			const long oldValue = InterlockedExchange(&m_stored, to_long(value));
			return from_long(oldValue);
		}

		bool compare_exchange_strong(bool& expected, bool desired, memory_order = memory_order_seq_cst)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_strong(bool& expected, bool desired, memory_order, memory_order)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_weak(bool& expected, bool desired, memory_order = memory_order_seq_cst)
		{
			return compare_exchange(expected, desired);
		}

		bool compare_exchange_weak(bool& expected, bool desired, memory_order, memory_order)
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
