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
//
// The VC6 classes have the size of the type they hold, like the standard classes. They differ in one point: their
// constructors are not constexpr. An object with static storage duration is initialized when its constructor runs
// during the static initialization, where the standard classes initialize it at compile time. A value that another
// static initializer stored into the object before that is overwritten.
#pragma once

#if !(defined(_MSC_VER) && _MSC_VER < 1300)

#include <atomic>

#else

#include <windows.h>

#include "Utility/interlocked_adapter.h"

namespace utility_atomic_detail
{
	// Read a value without locking, which is sufficient because every store of the atomic classes is a locked
	// instruction. The functions are naked, so that VC6 never expands them inline. Its optimizer does not reliably treat
	// a data member as volatile and would otherwise move the read out of a loop that waits for another thread to store
	// a value. With __fastcall the argument is in ecx. The result is returned in al, ax or eax.
	__declspec(naked) inline char __fastcall load8(const char volatile *source)
	{
		__asm
		{
			mov al, byte ptr [ecx]
			ret
		}
	}

	__declspec(naked) inline short __fastcall load16(const short volatile *source)
	{
		__asm
		{
			mov ax, word ptr [ecx]
			ret
		}
	}

	__declspec(naked) inline long __fastcall load32(const long volatile *source)
	{
		__asm
		{
			mov eax, dword ptr [ecx]
			ret
		}
	}
}

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


	// The VC6 std::atomic compatible template stores the value in its own size. It supports the integer types up to
	// the size of long, enums and pointers. Its arithmetic and bitwise functions compile for the integer types only.
	template<typename T>
	class atomic
	{
	public:
		atomic(T value = (T)0)
			: m_stored(value)
		{
		}

		T load(memory_order = memory_order_seq_cst) const
		{
			return from_long(load_stored());
		}

		void store(T value, memory_order = memory_order_seq_cst)
		{
			exchange_stored(to_long(value));
		}

		T exchange(T value, memory_order = memory_order_seq_cst)
		{
			return from_long(exchange_stored(to_long(value)));
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

		static_assert_cpp98(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "The type must have a size that interlocked functions exist for");

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

		// The four functions below call the function for the size of the type. They pass the value as a long whose
		// low bytes hold it. The size is constant, so that the compiler keeps the one case that applies.
		long load_stored() const
		{
			switch (sizeof(T))
			{
			case 1: return ::utility_atomic_detail::load8((const char volatile*)&m_stored);
			case 2: return ::utility_atomic_detail::load16((const short volatile*)&m_stored);
			default: return ::utility_atomic_detail::load32((const long volatile*)&m_stored);
			}
		}

		long exchange_stored(long value)
		{
			switch (sizeof(T))
			{
			case 1: return InterlockedExchange8((char volatile*)&m_stored, (char)value);
			case 2: return InterlockedExchange16((short volatile*)&m_stored, (short)value);
			default: return InterlockedExchange((long volatile*)&m_stored, value);
			}
		}

		long exchange_add_stored(long value)
		{
			switch (sizeof(T))
			{
			case 1: return InterlockedExchangeAdd8((char volatile*)&m_stored, (char)value);
			case 2: return InterlockedExchangeAdd16((short volatile*)&m_stored, (short)value);
			default: return InterlockedExchangeAdd((long volatile*)&m_stored, value);
			}
		}

		long compare_exchange_stored(long exchange, long comparand)
		{
			switch (sizeof(T))
			{
			case 1: return InterlockedCompareExchange8((char volatile*)&m_stored, (char)exchange, (char)comparand);
			case 2: return InterlockedCompareExchange16((short volatile*)&m_stored, (short)exchange, (short)comparand);
			default: return InterlockedCompareExchange((long volatile*)&m_stored, exchange, comparand);
			}
		}

		bool compare_exchange(T& expected, T desired)
		{
			const T oldValue = from_long(compare_exchange_stored(to_long(desired), to_long(expected)));

			if (oldValue == expected)
				return true;

			expected = oldValue;
			return false;
		}

		static long compute(operation op, long stored, long operand)
		{
			switch (op)
			{
			// Adds and subtracts unsigned, because that wraps around without undefined behavior.
			case operation_add: return (long)((unsigned long)stored + (unsigned long)operand);
			case operation_sub: return (long)((unsigned long)stored - (unsigned long)operand);
			case operation_or: return stored | operand;
			case operation_and: return stored & operand;
			default: return stored ^ operand;
			}
		}

		// Returns the stored value from before the operation.
		long fetch_modify(operation op, long operand)
		{
			// The processor adds in the size of the type and wraps around like the type does, so that an addition
			// needs no retry loop.
			if (op == operation_add)
				return exchange_add_stored(operand);

			if (op == operation_sub)
				return exchange_add_stored((long)(0ul - (unsigned long)operand));

			long oldValue;
			long newValue;

			do
			{
				oldValue = load_stored();
				newValue = compute(op, oldValue, operand);
			}
			while (compare_exchange_stored(newValue, oldValue) != oldValue);

			return oldValue;
		}

		// Returns the value from after the operation.
		T modify(operation op, T value)
		{
			const long operand = to_operand(value);
			const long oldValue = fetch_modify(op, operand);

			return from_long(compute(op, oldValue, operand));
		}

		mutable volatile T m_stored;
	};


	// A float has a supported size, but the conversion to long would drop its fraction.
	template<>
	class atomic<float>;


	// Atomic bool is a specialized template and for VC6 it internally uses a char and the 8 bit interlocked functions
	template<>
	class atomic<bool>
	{
	public:
		atomic(bool value = false)
			: m_stored(to_char(value))
		{
		}

		bool load(memory_order = memory_order_seq_cst) const
		{
			const char value = ::utility_atomic_detail::load8(&m_stored);
			return from_char(value);
		}

		void store(bool value, memory_order = memory_order_seq_cst)
		{
			InterlockedExchange8(&m_stored, to_char(value));
		}

		bool exchange(bool value, memory_order = memory_order_seq_cst)
		{
			const char oldValue = InterlockedExchange8(&m_stored, to_char(value));
			return from_char(oldValue);
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

		static char to_char(bool value)
		{
			return value ? 1 : 0;
		}

		static bool from_char(char value)
		{
			return value != 0;
		}

		atomic(const atomic&) FUNCTION_DELETE;
		atomic& operator=(const atomic&) FUNCTION_DELETE;

		bool compare_exchange(bool& expected, bool desired)
		{
			const char storedExpected = to_char(expected);
			const char storedDesired = to_char(desired);

			const char oldValue = InterlockedCompareExchange8(&m_stored, storedDesired, storedExpected);

			if (oldValue == storedExpected)
				return true;

			expected = from_char(oldValue);
			return false;
		}

		mutable volatile char m_stored;
	};

}

#endif
