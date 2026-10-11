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

// VC6-compatible implementation of std::atomic on top of the Interlocked functions.
// The file that includes this one provides interlocked_adapter.h, <stddef.h> and the namespace.

// The VC6 atomic class takes the memory orders for source compatibility with the standard class.
// Its functions are always sequentially consistent, which satisfies every order.
enum memory_order
{
	memory_order_relaxed,
	memory_order_consume,
	memory_order_acquire,
	memory_order_release,
	memory_order_acq_rel,
	memory_order_seq_cst
};


template<typename Float, typename Bits>
class atomic_floating;


// The VC6 std::atomic compatible template stores the value in its own size. It supports bool, the integer types,
// enums and pointers. All of them can be loaded, stored, exchanged and compare exchanged. The arithmetic and
// bitwise functions compile for the integer types only. The standard class does not have them for bool and enums
// either.
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

	static_assert_cpp98(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "The type must have a size that interlocked functions exist for");

	atomic(const atomic&) FUNCTION_DELETE;
	atomic& operator=(const atomic&) FUNCTION_DELETE;

	// The floating point classes start their retry loop with load_first_comparand, like fetch_modify does.
	friend class atomic_floating<float, T>;
	friend class atomic_floating<double, T>;

	// The four functions below call the function for the size of the type and convert the value to and from the
	// type of that function. The size is constant, so that the compiler keeps the one case that applies. A value is
	// cast to long first, because the cases that do not apply are compiled as well and a pointer does not cast to
	// a smaller type.
	T load_stored() const
	{
		switch (sizeof(T))
		{
		case 1: return (T)ReadAcquire8((const char volatile*)&m_stored);
		case 2: return (T)ReadAcquire16((const short volatile*)&m_stored);
		// Reading 8 bytes takes two instructions, between which another thread can store. A compare exchange of
		// 0 for 0 reads them in one step: it leaves every value as it is and returns it.
		case 8: return (T)InterlockedCompareExchange64((int64_t volatile*)&m_stored, 0, 0);
		default: return (T)ReadAcquire((const long volatile*)&m_stored);
		}
	}

	// Returns the value that a retry loop starts with. It saves the locked instruction that load_stored takes for
	// 8 bytes. Such a value is read in two steps here and can consist of the halves of two values. The compare
	// exchange of the loop does not match it then and hands back the stored value.
	T load_first_comparand() const
	{
		switch (sizeof(T))
		{
		case 8: return (T)ReadAcquire64((const int64_t volatile*)&m_stored);
		default: return load_stored();
		}
	}

	T exchange_stored(T value)
	{
		switch (sizeof(T))
		{
		case 1: return (T)InterlockedExchange8((char volatile*)&m_stored, (char)(long)value);
		case 2: return (T)InterlockedExchange16((short volatile*)&m_stored, (short)(long)value);
		case 8: return (T)InterlockedExchange64((int64_t volatile*)&m_stored, (int64_t)value);
		default: return (T)InterlockedExchange((long volatile*)&m_stored, (long)value);
		}
	}

	T exchange_add_stored(T value)
	{
		switch (sizeof(T))
		{
		case 1: return (T)InterlockedExchangeAdd8((char volatile*)&m_stored, (char)(long)value);
		case 2: return (T)InterlockedExchangeAdd16((short volatile*)&m_stored, (short)(long)value);
		case 8: return (T)InterlockedExchangeAdd64((int64_t volatile*)&m_stored, (int64_t)value);
		default: return (T)InterlockedExchangeAdd((long volatile*)&m_stored, (long)value);
		}
	}

	T compare_exchange_stored(T exchange, T comparand)
	{
		switch (sizeof(T))
		{
		case 1: return (T)InterlockedCompareExchange8((char volatile*)&m_stored, (char)(long)exchange, (char)(long)comparand);
		case 2: return (T)InterlockedCompareExchange16((short volatile*)&m_stored, (short)(long)exchange, (short)(long)comparand);
		case 8: return (T)InterlockedCompareExchange64((int64_t volatile*)&m_stored, (int64_t)exchange, (int64_t)comparand);
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
		// Adds and subtracts unsigned, because that wraps around without undefined behavior, and in 64 bits for the
		// types of that size. The compiler reduces that to the size of a smaller type.
		case operation_add: return (T)((uint64_t)stored + (uint64_t)operand);
		case operation_sub: return (T)((uint64_t)stored - (uint64_t)operand);
		case operation_or: return (T)(stored | operand);
		case operation_and: return (T)(stored & operand);
		default: return (T)(stored ^ operand);
		}
	}

	// Returns the value from before the operation.
	T fetch_modify(operation op, T operand)
	{
		// Makes the arithmetic and bitwise functions, which all come through here, fail to compile for bool, enums
		// and pointers: an int converts implicitly to neither an enum nor a pointer, and a bool cannot be decremented.
		T integerTypesOnly = 1;
		--integerTypesOnly;

		// The processor adds in the size of the type and wraps around like the type does, so that an addition
		// needs no retry loop.
		if (op == operation_add)
			return exchange_add_stored(operand);

		if (op == operation_sub)
			return exchange_add_stored((T)((uint64_t)0 - (uint64_t)operand));

		T oldValue = load_first_comparand();

		// A compare exchange that does not match returns the stored value, which is the comparand of the next
		// attempt.
		for (;;)
		{
			const T newValue = compute(op, oldValue, operand);
			const T storedValue = compare_exchange_stored(newValue, oldValue);

			if (storedValue == oldValue)
				return oldValue;

			oldValue = storedValue;
		}
	}

	// Returns the value from after the operation.
	T modify(operation op, T value)
	{
		return compute(op, fetch_modify(op, value), value);
	}

	mutable volatile T m_stored;
};


// A floating point value is kept as the integer with the same bytes, because the generic class would convert its
// value to an integer and would compare values with ==. Like the standard class this one compares bytes, which
// differs from == for a negative zero and for a NaN, and it adds and subtracts but has no other arithmetic.
// The specializations of atomic for the floating point types derive from this class.
template<typename Float, typename Bits>
class atomic_floating
{
public:
	atomic_floating(Float value)
		: m_bits(to_bits(value))
	{
	}

	Float load(memory_order = memory_order_seq_cst) const
	{
		return from_bits(m_bits.load());
	}

	void store(Float value, memory_order = memory_order_seq_cst)
	{
		m_bits.store(to_bits(value));
	}

	Float exchange(Float value, memory_order = memory_order_seq_cst)
	{
		return from_bits(m_bits.exchange(to_bits(value)));
	}

	Float fetch_add(Float value, memory_order = memory_order_seq_cst)
	{
		Bits newBits;
		return from_bits(modify_bits(value, false, newBits));
	}

	Float fetch_sub(Float value, memory_order = memory_order_seq_cst)
	{
		Bits newBits;
		return from_bits(modify_bits(value, true, newBits));
	}

	bool compare_exchange_strong(Float& expected, Float desired, memory_order = memory_order_seq_cst)
	{
		return compare_exchange(expected, desired);
	}

	bool compare_exchange_strong(Float& expected, Float desired, memory_order, memory_order)
	{
		return compare_exchange(expected, desired);
	}

	bool compare_exchange_weak(Float& expected, Float desired, memory_order = memory_order_seq_cst)
	{
		return compare_exchange(expected, desired);
	}

	bool compare_exchange_weak(Float& expected, Float desired, memory_order, memory_order)
	{
		return compare_exchange(expected, desired);
	}

	operator Float() const
	{
		return load();
	}

	Float operator+=(Float value)
	{
		Bits newBits;
		modify_bits(value, false, newBits);
		return from_bits(newBits);
	}

	Float operator-=(Float value)
	{
		Bits newBits;
		modify_bits(value, true, newBits);
		return from_bits(newBits);
	}

private:
	static_assert_cpp98(sizeof(Float) == sizeof(Bits), "The integer type must have the size of the floating point type");

	atomic_floating(const atomic_floating&) FUNCTION_DELETE;
	atomic_floating& operator=(const atomic_floating&) FUNCTION_DELETE;

	// The bytes are read through a pointer, so that the value does not pass through the floating point unit on
	// the way, which changes the bytes of a signaling NaN.
	static Bits to_bits(const Float& value)
	{
		return *(const Bits*)&value;
	}

	static Float from_bits(const Bits& bits)
	{
		return *(const Float*)&bits;
	}

	bool compare_exchange(Float& expected, Float desired)
	{
		Bits expectedBits = to_bits(expected);

		if (m_bits.compare_exchange_strong(expectedBits, to_bits(desired)))
			return true;

		*(Bits*)&expected = expectedBits;
		return false;
	}

	// Adds or subtracts with a retry loop. Returns the bytes from before the operation.
	Bits modify_bits(Float value, bool subtract, Bits& newBits)
	{
		Bits oldBits = m_bits.load_first_comparand();

		do
		{
			const Float oldValue = from_bits(oldBits);
			const Float newValue = subtract ? oldValue - value : oldValue + value;
			newBits = to_bits(newValue);
		}
		while (!m_bits.compare_exchange_weak(oldBits, newBits));

		return oldBits;
	}

	atomic<Bits> m_bits;
};

template<>
class atomic<float> : public atomic_floating<float, long>
{
public:
	atomic(float value = 0.0f)
		: atomic_floating<float, long>(value)
	{
	}

	float operator=(float value)
	{
		store(value);
		return value;
	}

private:
	atomic(const atomic&) FUNCTION_DELETE;
	atomic& operator=(const atomic&) FUNCTION_DELETE;
};

template<>
class atomic<double> : public atomic_floating<double, int64_t>
{
public:
	atomic(double value = 0.0)
		: atomic_floating<double, int64_t>(value)
	{
	}

	double operator=(double value)
	{
		store(value);
		return value;
	}

private:
	atomic(const atomic&) FUNCTION_DELETE;
	atomic& operator=(const atomic&) FUNCTION_DELETE;
};

// A long double has a supported size, but the conversion to a 64 bit integer would drop its fraction.
template<>
class atomic<long double>;

// The aliases of the standard for the types that the class supports.
typedef atomic<bool> atomic_bool;
typedef atomic<char> atomic_char;
typedef atomic<signed char> atomic_schar;
typedef atomic<unsigned char> atomic_uchar;
typedef atomic<short> atomic_short;
typedef atomic<unsigned short> atomic_ushort;
typedef atomic<int> atomic_int;
typedef atomic<unsigned int> atomic_uint;
typedef atomic<long> atomic_long;
typedef atomic<unsigned long> atomic_ulong;
// VC6 has no long long. With the compilers that have it, it is the type of int64_t.
typedef atomic<int64_t> atomic_llong;
typedef atomic<uint64_t> atomic_ullong;
typedef atomic<wchar_t> atomic_wchar_t;
typedef atomic<int8_t> atomic_int8_t;
typedef atomic<uint8_t> atomic_uint8_t;
typedef atomic<int16_t> atomic_int16_t;
typedef atomic<uint16_t> atomic_uint16_t;
typedef atomic<int32_t> atomic_int32_t;
typedef atomic<uint32_t> atomic_uint32_t;
typedef atomic<int64_t> atomic_int64_t;
typedef atomic<uint64_t> atomic_uint64_t;
typedef atomic<int_least8_t> atomic_int_least8_t;
typedef atomic<uint_least8_t> atomic_uint_least8_t;
typedef atomic<int_least16_t> atomic_int_least16_t;
typedef atomic<uint_least16_t> atomic_uint_least16_t;
typedef atomic<int_least32_t> atomic_int_least32_t;
typedef atomic<uint_least32_t> atomic_uint_least32_t;
typedef atomic<int_least64_t> atomic_int_least64_t;
typedef atomic<uint_least64_t> atomic_uint_least64_t;
typedef atomic<int_fast8_t> atomic_int_fast8_t;
typedef atomic<uint_fast8_t> atomic_uint_fast8_t;
typedef atomic<int_fast16_t> atomic_int_fast16_t;
typedef atomic<uint_fast16_t> atomic_uint_fast16_t;
typedef atomic<int_fast32_t> atomic_int_fast32_t;
typedef atomic<uint_fast32_t> atomic_uint_fast32_t;
typedef atomic<int_fast64_t> atomic_int_fast64_t;
typedef atomic<uint_fast64_t> atomic_uint_fast64_t;
typedef atomic<intptr_t> atomic_intptr_t;
typedef atomic<uintptr_t> atomic_uintptr_t;
typedef atomic<size_t> atomic_size_t;
typedef atomic<ptrdiff_t> atomic_ptrdiff_t;
typedef atomic<intmax_t> atomic_intmax_t;
typedef atomic<uintmax_t> atomic_uintmax_t;
