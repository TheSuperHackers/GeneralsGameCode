/*
**	Command & Conquer Generals(tm)
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
 *                     $Archive:: /G/wwlib/PK.H                                               $* 
 *                                                                                             * 
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 4/02/99 12:00p                                              $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef PK_H
#define PK_H

#ifndef STRAW_H
#include "STRAW.H"
#endif

#include	"INT.H"

/*
**	This class holds a public or private key used in Public Key Cryptography. It also serves
**	as the conduit for encrypting/decrypting data using that key. Cryptography, using this
**	method, has a couple of characteristics that affect how it is used. One, the process of
**	encrypting/decrypting is very slow. This limits the effective quantity of data that can
**	be processed. Two, the ciphertext is larger than the plaintext. This property generally
**	limits its use to streaming data as opposed to random access data. The data is processed
**	in blocks. The size of the ciphertext and plaintext blocks can be determined only from
**	the key itself.
**
**	A reasonable use of this technology would be to encrypt only critical data such as the
**	password for a fast general purpose cryptographic algorithm.
*/
class PKey
{
	public:
		PKey(void) : Modulus(0), Exponent(0), BitPrecision(0) {}
		PKey(void const * exponent, void const * modulus);		// DER initialization.

		int Encrypt(void const * source, int slen, void * dest) const;
		int Decrypt(void const * source, int slen, void * dest) const;

		static void Generate(Straw & random, int bits, PKey & fastkey, PKey & slowkey);

		int Plain_Block_Size(void) const {return((BitPrecision-1)/8);}
		int Crypt_Block_Size(void) const {return(Plain_Block_Size()+1);}
		int Block_Count(int plaintext_length) const {return((((plaintext_length-1)/Plain_Block_Size())+1));}

		int Encode_Modulus(void * buffer) const;
		int Encode_Exponent(void * buffer) const;

		void Decode_Modulus(void * buffer);
		void Decode_Exponent(void * buffer);

		static long Fast_Exponent(void) {return(65537L);}

//	private:

		// p*q
		BigInt Modulus;

		// 65537 or
		// inverse of (p-1)(q-1).
		BigInt Exponent;

		// Maximum bits allowed for block.
		int BitPrecision;
};


#endif
