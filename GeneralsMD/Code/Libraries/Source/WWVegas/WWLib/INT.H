/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /VSS_Sync/wwlib/int.h                                       $* 
 *                                                                                             * 
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             * 
 *                     $Modtime:: 3/21/01 12:01p                                              $*
 *                                                                                             * 
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef INT_H
#define INT_H

#include	"MPMATH.H"
#include	"STRAW.H"
#include	<assert.h>
#include	<limits.h>
#include	<memory.h>

#ifdef __BORLANDC__
#pragma warn -inl
#endif

template<class T> struct RemainderTable;
template<class T> T Gcd(const T & a, const T & n);
#ifdef _UNIX
 #define FN_TEMPLATE <>
#else
 #define FN_TEMPLATE
#endif


template<int PRECISION>
class Int {
	public:

		/*
		**	Constructors and initializers.
		*/
		Int(void) {XMP_Init(&reg[0], 0, PRECISION);}
		Int(unsigned long value) {XMP_Init(&reg[0], value, PRECISION);}

		void Randomize(Straw & rng, int bitcount) {XMP_Randomize(&reg[0], rng, bitcount, PRECISION);}
		void Randomize(Straw & rng, const Int & minval, const Int & maxval) {XMP_Randomize_Bounded(&reg[0], rng, minval, maxval, PRECISION); reg[0] |= 1;}

		/*
		**	Convenient conversion operators to get at the underlying array of
		**	integers. Big number math is basically manipulation of arbitrary
		**	length arrays.
		*/
		operator digit * () {return & reg[0];}
		operator const digit * () const {return & reg[0];}

		/*
		**	Array access operator (references bit position). Bit 0 is the first bit.
		*/
		bool operator[](unsigned bit) const {return(XMP_Test_Bit(&reg[0], bit));}

		/*
		**	Unary operators.
		*/
		Int & operator ++ (void) {XMP_Inc(&reg[0], PRECISION);return(*this);}
		Int & operator -- (void) {XMP_Dec(&reg[0], PRECISION);return(*this);}
		int operator ! (void) const {return(XMP_Test_Eq_Int(&reg[0], 0, PRECISION));}
		Int operator ~ (void) {XMP_Not(&reg[0], PRECISION);return(*this);}
		Int operator - (void) const {Int a = *this;a.Negate();return (a);}

		/*
		**	Attribute query functions.
		*/
		int ByteCount(void) const {return(XMP_Count_Bytes(&reg[0], PRECISION));}
		int BitCount(void) const {return(XMP_Count_Bits(&reg[0], PRECISION));}
		bool Is_Negative(void) const {return(XMP_Is_Negative(&reg[0], PRECISION));}
		unsigned MaxBitPrecision() const {return PRECISION*(sizeof(unsigned long)*CHAR_BIT);}
		bool IsSmallPrime(void) const {return(XMP_Is_Small_Prime(&reg[0], PRECISION));}
		bool SmallDivisorsTest(void) const {return(XMP_Small_Divisors_Test(&reg[0], PRECISION));}
		bool FermatTest(unsigned rounds) const {return(XMP_Fermat_Test(&reg[0], rounds, PRECISION));}
		bool IsPrime(void) const {return(XMP_Is_Prime(&reg[0], PRECISION));}
		bool RabinMillerTest(Straw & rng, unsigned int rounds) const {return(XMP_Rabin_Miller_Test(rng, &reg[0], rounds, PRECISION));}

		/*
		**	'in-place' binary operators.
		*/
		Int & operator += (const Int & number) {Carry = XMP_Add(&reg[0], &reg[0], number, 0, PRECISION);return(*this);}
		Int & operator -= (const Int & number) {Borrow = XMP_Sub(&reg[0], &reg[0], number, 0, PRECISION);return(*this);}
		Int & operator *= (const Int & multiplier) {Remainder = *this;Error=XMP_Signed_Mult(&reg[0], Remainder, multiplier, PRECISION);return(*this);}
		Int & operator /= (const Int & t) {*this = (*this) / t;return *this;}
		Int & operator %= (const Int & t) {*this = (*this) % t;return *this;}
		Int & operator <<= (int bits) {XMP_Shift_Left_Bits(&reg[0], bits, PRECISION);return *this;}
		Int & operator >>= (int bits) {XMP_Shift_Right_Bits(&reg[0], bits, PRECISION);return *this;}

		/*
		**	Mathematical binary operators.
		*/
		Int operator + (const Int & number) const {Int term;Carry = XMP_Add(term, &reg[0], number, 0, PRECISION);return(term);}
		Int operator + (unsigned short b) const {Int result;Carry=XMP_Add_Int(result, &reg[0], b, 0, PRECISION);return(result);}
		Int operator - (const Int & number) const {Int term;Borrow = XMP_Sub(term, &reg[0], number, 0, PRECISION);return(term);}
		Int operator - (unsigned short b) const {Int result;Borrow = XMP_Sub_Int(result, &reg[0], b, 0, PRECISION);return(result);}
		Int operator * (const Int & multiplier) const {Int result;Error=XMP_Signed_Mult(result, &reg[0], multiplier, PRECISION);return result;}
		Int operator * (unsigned short b) const {Int result;Error=XMP_Unsigned_Mult_Int(result, &reg[0], b, PRECISION);return(result);}
		Int operator / (const Int & divisor) const {Int quotient = *this;XMP_Signed_Div(Remainder, quotient, &reg[0], divisor, PRECISION);return (quotient);}
		Int operator / (unsigned long b) const {return(*this / Int<PRECISION>(b));}
		Int operator / (unsigned short divisor) const {Int quotient;Error=XMP_Unsigned_Div_Int(quotient, &reg[0], divisor, PRECISION);return(quotient);}
		Int operator % (const Int & divisor) const {Int remainder;XMP_Signed_Div(remainder, Remainder, &reg[0], divisor, PRECISION);return(remainder);}
		Int operator % (unsigned long b) const {return(*this % Int<PRECISION>(b));}
		unsigned short operator % (unsigned short divisor) const {return(XMP_Unsigned_Div_Int(Remainder, &reg[0], divisor, PRECISION));}

		/*
		**	Bitwise binary operators.
		*/
		Int operator >> (int bits) const {Int result = *this; XMP_Shift_Right_Bits(result, bits, PRECISION);return result;}
		Int operator << (int bits) const {Int result = *this; XMP_Shift_Left_Bits(result, bits, PRECISION);return result;}

		/*
		**	Comparison binary operators.
		*/
		int operator == (const Int &b) const {return (memcmp(&reg[0], &b.reg[0], (MAX_BIT_PRECISION/CHAR_BIT))==0);}
		int operator != (const Int& b) const {return !(*this == b);}
		int operator > (const Int & number) const {return(XMP_Compare(&reg[0], number, PRECISION) > 0);}
		int operator >= (const Int & number) const {return(XMP_Compare(&reg[0], number, PRECISION) >= 0);}
		int operator < (const Int & number) const {return(XMP_Compare(&reg[0], number, PRECISION) < 0);}
		int operator <= (const Int & number) const {return(XMP_Compare(&reg[0], number, PRECISION) <= 0);}

		/*
		**	Misc. mathematical and logical functions.
		*/
		void Negate(void) {XMP_Neg(&reg[0], PRECISION);}
		Int Abs(void) {XMP_Abs(&reg[0], PRECISION);return(*this);}

		Int exp_b_mod_c(const Int & e, const Int & m) const {
			Int result;
			Error=XMP_Exponent_Mod(result, &reg[0], e, m, PRECISION);
			return result;
		}

		void Set_Bit(int index) { XMP_Set_Bit(&reg[0], index); }
		static Int Unsigned_Mult(Int const & multiplicand, Int const & multiplier) {Int product;Error=XMP_Unsigned_Mult(&product.reg[0], &multiplicand.reg[0], &multiplier.reg[0], PRECISION);return(product);}
		static void Unsigned_Divide(Int & remainder, Int & quotient, const Int & dividend, const Int & divisor) {Error=XMP_Unsigned_Div(remainder, quotient, dividend, divisor, PRECISION);}
		static void Signed_Divide(Int & remainder, Int & quotient, const Int & dividend, const Int & divisor) {XMP_Signed_Div(remainder, quotient, dividend, divisor, PRECISION);}
		Int Inverse(const Int & modulus) const {Int result;XMP_Inverse_A_Mod_B(result, &reg[0], modulus, PRECISION);return(result);}

		static Int Decode_ASCII(char const * string) {Int result;XMP_Decode_ASCII(string, result, PRECISION);return(result);}

		// Number (sign independand) inserted into buffer.
		int Encode(unsigned char *output) const {return(XMP_Encode(output, &reg[0], PRECISION));}
		int Encode(unsigned char * output, unsigned length) const {return(XMP_Encode_Bounded(output, length, &reg[0], PRECISION));}
		void Signed_Decode(const unsigned char * from, int frombytes) {XMP_Signed_Decode(&reg[0], from, frombytes, PRECISION);}
		void Unsigned_Decode(const unsigned char * from, int frombytes) {XMP_Unsigned_Decode(&reg[0], from, frombytes, PRECISION);}

		// encode Int using Distinguished Encoding Rules, returns size of output
		int DEREncode(unsigned char * output) const {return(XMP_DER_Encode(&reg[0], output, PRECISION));}
		void DERDecode(const unsigned char *input) {XMP_DER_Decode(&reg[0], input, PRECISION);}

		// Friend helper functions.
		friend Int<PRECISION> Gcd FN_TEMPLATE (const Int<PRECISION> &, const Int<PRECISION> &);


		static int Error;

		// Carry result from last addition.
		static bool Carry;

		// Borrow result from last subtraction.
		static bool Borrow;

		// Remainder value from the various division routines.
		static Int Remainder;

		digit reg[PRECISION];

		friend struct RemainderTable< Int<PRECISION> >;
};

template<class T>
struct RemainderTable
{
	RemainderTable(const T & p) : HasZeroEntry(false)
	{
		int primesize = XMP_Fetch_Prime_Size();
		unsigned short const * primetable = XMP_Fetch_Prime_Table();
		for (int i = 0; i < primesize; i++) {
			table[i] = p % primetable[i];
		}
	}
	bool HasZero() const {return(HasZeroEntry);}
	void Increment(unsigned short increment = 1)
	{
		int primesize = XMP_Fetch_Prime_Size();
		unsigned short const * primetable = XMP_Fetch_Prime_Table();
		HasZeroEntry = false;
		for (int i = 0; i < primesize; i++) {
			table[i] += increment;
			while (table[i] >= primetable[i]) {
				table[i] -= primetable[i];
			}
			HasZeroEntry = (HasZeroEntry || !table[i]);
		}
	}
	void Increment(const RemainderTable & rtQ)
	{
		HasZeroEntry = false;
		int primesize = XMP_Fetch_Prime_Size();
		unsigned short const * primetable = XMP_Fetch_Prime_Table();
		for (int i = 0; i < primesize; i++) {
			table[i] += rtQ.table[i];
			if (table[i] >= primetable[i]) {
				table[i] -= primetable[i];
			}
			HasZeroEntry = (HasZeroEntry || !table[i]);
		}
	}
	bool HasZeroEntry;
	unsigned short table[3511];
};



template<class T>
T Gcd(const T & a, const T & n)
{
	T g[3]={n, a, 0UL};

	unsigned int i = 1;
	while (!!g[i%3]) {
		g[(i+1)%3] = g[(i-1)%3] % g[i%3];
		i++;
	}
	return g[(i-1)%3];
}



#if defined(__WATCOMC__)
#pragma warning 604 9
#pragma warning 595 9
#endif


template<class T>
T Generate_Prime(Straw & rng, int pbits, T const *)
{
	T minQ = (T(1UL) << (unsigned short)(pbits-(unsigned short)2));
	T maxQ = ((T(1UL) << (unsigned short)(pbits-(unsigned short)1)) - (unsigned short)1);

	T q;
	T p;

	do {
		q.Randomize(rng, minQ, maxQ);
		p = (q*2) + (unsigned short)1;

		RemainderTable<T> rtQ(q);
		RemainderTable<T> rtP(p);

		while (rtQ.HasZero() || rtP.HasZero() || !q.IsPrime() || !p.IsPrime()) {
			q += 2;
			p += 4;
			if (q > maxQ) break;

			rtQ.Increment(2);
			rtP.Increment(4);
		}
	} while (q > maxQ);

	return(p);
}




#define	UNITSIZE					32
#define	MAX_BIT_PRECISION		2048
#define	MAX_UNIT_PRECISION	(MAX_BIT_PRECISION/UNITSIZE)


typedef Int<MAX_UNIT_PRECISION>	bignum;
typedef Int<MAX_UNIT_PRECISION>	BigInt;


#endif

