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

// This file contains a VC6 compatible atomic template class for the integer types up to the size of long, enums and pointers.
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


	// The VC6 std::atomic compatible template uses long internally as storage. It supports the integer types up to
	// the size of long, enums and pointers. Its arithmetic and bitwise functions compile for the integer types only.
	template<typename T>
	class atomic
	{
	public:
		atomic(T value = (T)0)
			: m_stored(to_long(value))
		{
		}

		T load(memory_order = memory_order_seq_cst) const
		{
			return from_long(load_stored());
		}

		void store(T value, memory_order = memory_order_seq_cst)
		{
			InterlockedExchange(&m_stored, to_long(value));
		}

		T exchange(T value, memory_order = memory_order_seq_cst)
		{
			const long oldValue = InterlockedExchange(&m_stored, to_long(value));
			return from_long(oldValue);
		}

		T fetch_add(T value, memory_order = memory_order_seq_cst)
		{
			return from_long(fetch_modify(operation_add, to_operand(value)));
		}

		T fetch_sub(T value, memory_order = memory_order_seq_cst)
		{
			return from_long(fetch_modify(operation_sub, to_operand(value)));
		}

		T fetch_or(T value, memory_order = memory_order_seq_cst)
		{
			return from_long(fetch_modify(operation_or, to_operand(value)));
		}

		T fetch_and(T value, memory_order = memory_order_seq_cst)
		{
			return from_long(fetch_modify(operation_and, to_operand(value)));
		}

		T fetch_xor(T value, memory_order = memory_order_seq_cst)
		{
			return from_long(fetch_modify(operation_xor, to_operand(value)));
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

		// Fails to compile for a type that does not fit into the stored long.
		typedef char type_must_fit_in_long[sizeof(T) <= sizeof(long) ? 1 : -1];

		atomic(const atomic&) FUNCTION_DELETE;
		atomic& operator=(const atomic&) FUNCTION_DELETE;

		static long to_long(T value)
		{
			return (long)value;
		}

		static T from_long(long value)
		{
			return (T)value;
		}

		// Converts the argument of an arithmetic or bitwise function. Compiles for the integer types only,
		// because an int converts implicitly to neither an enum nor a pointer.
		static long to_operand(T value)
		{
			const T integerTypesOnly = 1;
			(void)integerTypesOnly;

			return to_long(value);
		}

		bool compare_exchange(T& expected, T desired)
		{
			const long storedExpected = to_long(expected);
			const long storedDesired = to_long(desired);

			const long oldValue = InterlockedCompareExchange(&m_stored, storedDesired, storedExpected);

			if (oldValue == storedExpected)
				return true;

			expected = from_long(oldValue);
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
			return to_long(from_long(result));
		}

		// Returns the stored value from before the operation.
		long fetch_modify(operation op, long operand)
		{
			// A long sized value needs no wrapping around to a smaller value range, so that one interlocked
			// function can do the whole addition without a retry loop.
			if (sizeof(T) == sizeof(long))
			{
				if (op == operation_add)
					return InterlockedExchangeAdd(&m_stored, operand);

				if (op == operation_sub)
					return InterlockedExchangeAdd(&m_stored, (long)(0ul - (unsigned long)operand));
			}

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
			const long operand = to_operand(value);
			const long oldValue = fetch_modify(op, operand);

			return from_long(compute(op, oldValue, operand));
		}

		mutable volatile long m_stored;
	};


	// A float fits into the stored long, but the conversion to long would drop its fraction.
	template<>
	class atomic<float>;


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
