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

// This file contains a VC6 compatible atomic template class for bool, the integer types up to the size of long, enums
// and pointers.
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


	// The VC6 std::atomic compatible template stores the value in its own size. It supports bool, the integer types
	// up to the size of long, enums and pointers. Its arithmetic and bitwise functions compile for the integer types
	// only.
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
			return load_stored();
		}

		void store(T value, memory_order = memory_order_seq_cst)
		{
			exchange_stored(value);
		}

		T exchange(T value, memory_order = memory_order_seq_cst)
		{
			return exchange_stored(value);
		}

		T fetch_add(T value, memory_order = memory_order_seq_cst)
		{
			return fetch_modify(operation_add, value);
		}

		T fetch_sub(T value, memory_order = memory_order_seq_cst)
		{
			return fetch_modify(operation_sub, value);
		}

		T fetch_or(T value, memory_order = memory_order_seq_cst)
		{
			return fetch_modify(operation_or, value);
		}

		T fetch_and(T value, memory_order = memory_order_seq_cst)
		{
			return fetch_modify(operation_and, value);
		}

		T fetch_xor(T value, memory_order = memory_order_seq_cst)
		{
			return fetch_modify(operation_xor, value);
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

		// The four functions below call the function for the size of the type and convert the value to and from the
		// type of that function. The size is constant, so that the compiler keeps the one case that applies.
		T load_stored() const
		{
			switch (sizeof(T))
			{
			case 1: return (T)::utility_atomic_detail::load8((const char volatile*)&m_stored);
			case 2: return (T)::utility_atomic_detail::load16((const short volatile*)&m_stored);
			default: return (T)::utility_atomic_detail::load32((const long volatile*)&m_stored);
			}
		}

		T exchange_stored(T value)
		{
			switch (sizeof(T))
			{
			case 1: return (T)InterlockedExchange8((char volatile*)&m_stored, (char)value);
			case 2: return (T)InterlockedExchange16((short volatile*)&m_stored, (short)value);
			default: return (T)InterlockedExchange((long volatile*)&m_stored, (long)value);
			}
		}

		T exchange_add_stored(T value)
		{
			switch (sizeof(T))
			{
			case 1: return (T)InterlockedExchangeAdd8((char volatile*)&m_stored, (char)value);
			case 2: return (T)InterlockedExchangeAdd16((short volatile*)&m_stored, (short)value);
			default: return (T)InterlockedExchangeAdd((long volatile*)&m_stored, (long)value);
			}
		}

		T compare_exchange_stored(T exchange, T comparand)
		{
			switch (sizeof(T))
			{
			case 1: return (T)InterlockedCompareExchange8((char volatile*)&m_stored, (char)exchange, (char)comparand);
			case 2: return (T)InterlockedCompareExchange16((short volatile*)&m_stored, (short)exchange, (short)comparand);
			default: return (T)InterlockedCompareExchange((long volatile*)&m_stored, (long)exchange, (long)comparand);
			}
		}

		bool compare_exchange(T& expected, T desired)
		{
			const T oldValue = compare_exchange_stored(desired, expected);

			if (oldValue == expected)
				return true;

			expected = oldValue;
			return false;
		}

		static T compute(operation op, T stored, T operand)
		{
			switch (op)
			{
			// Adds and subtracts unsigned, because that wraps around without undefined behavior.
			case operation_add: return (T)((unsigned long)stored + (unsigned long)operand);
			case operation_sub: return (T)((unsigned long)stored - (unsigned long)operand);
			case operation_or: return (T)(stored | operand);
			case operation_and: return (T)(stored & operand);
			default: return (T)(stored ^ operand);
			}
		}

		// Returns the value from before the operation.
		T fetch_modify(operation op, T operand)
		{
			// Compiles for the integer types only: an int converts implicitly to neither an enum nor a pointer, and a
			// bool cannot be decremented.
			T integerTypesOnly = 1;
			--integerTypesOnly;

			// The processor adds in the size of the type and wraps around like the type does, so that an addition
			// needs no retry loop.
			if (op == operation_add)
				return exchange_add_stored(operand);

			if (op == operation_sub)
				return exchange_add_stored((T)(0ul - (unsigned long)operand));

			T oldValue;
			T newValue;

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
			return compute(op, fetch_modify(op, value), value);
		}

		mutable volatile T m_stored;
	};


	// A float has a supported size, but the conversion to long would drop its fraction.
	template<>
	class atomic<float>;

}

#endif
