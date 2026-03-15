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
 *                 Project Name : Blowfish Component                                           *
 *                                                                                             *
 *                     $Archive:: /Sun/Blowfish/bfish.h                                       $*
 *                                                                                             *
 *                      $Author:: Joe_b                                                       $*
 *                                                                                             *
 *                     $Modtime:: 12/02/97 9:35p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include	<limits.h>


/*
**	This engine will process data blocks by encryption and decryption.
**	The "Blowfish" algorithm is in the public domain. It uses
**	a Feistal network (similar to IDEA). It has no known
**	weaknesses, but is still relatively new. Blowfish is particularly strong
**	against brute force attacks. It is also quite strong against linear and
**	differential cryptanalysis. Unlike public key encription, it is very
**	fast at encryption, as far as cryptography goes. Its weakness is that
**	it takes a relatively long time to set up with a new key (1/100th of
**	a second on a P6-200). The time to set up a key is equivalent to
**	encrypting 4240 bytes.
*/
class BlowfishEngine {
	public:
		BlowfishEngine(void) : IsKeyed(false) {}
		~BlowfishEngine(void);

		void Submit_Key(void const * key, int length);

		int Encrypt(void const * plaintext, int length, void * cyphertext);
		int Decrypt(void const * cyphertext, int length, void * plaintext);

		/*
		**	This is the maximum key length supported.
		*/
		enum {MAX_KEY_LENGTH=56,
				BYTES_PER_BLOCK=8		// The number of bytes in each cypher block (don't change).
		};

	private:
		bool IsKeyed;

		void Sub_Key_Encrypt(unsigned long & left, unsigned long & right);

		void Process_Block(void const * plaintext, void * cyphertext, unsigned long const * ptable);
		void Initialize_Tables(void);

		enum {
			ROUNDS = 16		// Feistal round count (16 is standard).
		};

		/*
		**	Initialization data for sub keys. The initial values are constant and
		**	filled with a number generated from pi. Thus they are not random but
		**	they don't hold a weak pattern either.
		*/
		static unsigned long const P_Init[ROUNDS+2];
		static unsigned long const S_Init[4][UCHAR_MAX+1];

		/*
		**	Permutation tables for encryption and decryption.
		*/
 		unsigned long P_Encrypt[ROUNDS+2];
 		unsigned long P_Decrypt[ROUNDS+2];

		/*
		**	S-Box tables (four).
		*/
		unsigned long bf_S[4][UCHAR_MAX+1];
};
