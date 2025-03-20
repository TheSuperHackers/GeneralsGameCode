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
 *                     $Archive:: /Commando/Library/Obscure.cpp                               $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   Obfuscate -- Sufficiently transform parameter to thwart casual hackers.                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"CRC.H"
#include	"obscure.h"
#include	<ctype.h>
#include	<string.h>

/***********************************************************************************************
 * Obfuscate -- Sufficiently transform parameter to thwart casual hackers.                     *
 *                                                                                             *
 *    This routine borrows from CRC and PGP technology to sufficiently alter the parameter     *
 *    in order to make it difficult to reverse engineer the key phrase. This is designed to    *
 *    be used for hidden game options that will be released at a later time over Westwood's    *
 *    Web page or through magazine hint articles.                                              *
 *                                                                                             *
 *    This algorithm is cryptographically categorized as a "one way hash".                     *
 *                                                                                             *
 *    Since this is a one way transformation, it becomes much more difficult to reverse        *
 *    engineer the pass phrase even if the resultant pass code is known. This has an added     *
 *    benefit of making this algorithm immune to traditional cryptographic attacks.            *
 *                                                                                             *
 *    The largest strength of this transformation algorithm lies in the restriction on the     *
 *    source vector being legal ASCII uppercase characters. This restriction alone makes even  *
 *    a simple CRC transformation practically impossible to reverse engineer. This algorithm   *
 *    uses far more than a simple CRC transformation to achieve added strength from advanced   *
 *    attack methods.                                                                          *
 *                                                                                             *
 * INPUT:   string   -- Pointer to the key phrase that will be transformed into a code.        *
 *                                                                                             *
 * OUTPUT:  Returns with the code that the key phrase is translated into.                      *
 *                                                                                             *
 * WARNINGS:   A zero length pass phrase results in a 0x00000000 result code.                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
long Obfuscate(char const * string)
{
	char buffer[128];

	if (!string) return(0);
	memset(buffer, '\xA5', sizeof(buffer));

	/*
	**	Copy key phrase into a working buffer. This hides any transformation done
	**	to the string.
	*/
	strncpy(buffer, string, sizeof(buffer));
	buffer[sizeof(buffer)-1] = '\0';
	int length = strlen(buffer);

	/*
	**	Only upper case letters are significant.
	*/
	strupr(buffer);

	/*
	**	Ensure that only visible ASCII characters compose the key phrase. This
	**	discourages the direct forced illegal character input method of attack.
	*/
	for (int index = 0; index < length; index++) {
		if (!isgraph(buffer[index])) {
			buffer[index] = (char)('A' + (index%26));
		}
	}

	/*
	**	Increase the strength of even short pass phrases by extending the
	**	length to be at least a minimum number of characters. This helps prevent
	**	a weak pass phrase from compromising the obfuscation process. This
	**	process also forces the key phrase to be an even multiple of four.
	**	This is necessary to support the cypher process that occurs later.
	*/
	if (length < 16 || (length & 0x03)) {
		int maxlen = 16;
		if (((length+3) & 0x00FC) > maxlen) {
			maxlen = ((length+3) & 0x00FC);
		}
		int index;
		for (index = length; index < maxlen; index++) {
			buffer[index] = (char)('A' + ((('?' ^ buffer[index-length]) + index) % 26));
		}
		length = index;
		buffer[length] = '\0';
	}

	/*
	**	Transform the buffer into a number. This transformation is character
	**	order dependant.
	*/
	long code = CRCEngine()(buffer, length);

	/*
	**	Record a copy of this initial transformation to be used in a later
	**	self referential transformation.
	*/
	long copy = code;

	/*
	**	Reverse the character string and combine with the previous transformation.
	**	This doubles the workload of trying to reverse engineer the CRC calculation.
	*/
	strrev(buffer);
	code ^= CRCEngine()(buffer, length);

	/*
	**	Perform a self referential transformation. This makes a reverse engineering
	**	by using a cause and effect attack more difficult.
	*/
	code = code ^ copy;

	/*
	**	Unroll and combine the code value into the pass phrase and then perform
	**	another self referential transformation. Although this is a trivial cypher
	**	process, it gives the sophisticated hacker false hope since the strong
	**	cypher process occurs later.
	*/
	strrev(buffer);		// Restore original string order.
	for (int index2 = 0; index2 < length; index2++) {
		code ^= (unsigned char)buffer[index2];
		unsigned char temp = (unsigned char)code;
		buffer[index2] ^= temp;
		code >>= 8;
		code |= (((long)temp)<<24);
	}

	/*
	**	Introduce loss into the vector. This strengthens the key against traditional
	**	cryptographic attack engines. Since this also weakens the key against
	**	unconventional attacks, the loss is limited to less than 10%.
	*/
	for (int index3 = 0; index3 < length; index3++) {
		static unsigned char _lossbits[] = {0x00,0x08,0x00,0x20,0x00,0x04,0x10,0x00};
		static unsigned char _addbits[] = {0x10,0x00,0x00,0x80,0x40,0x00,0x00,0x04};

		buffer[index3] |= _addbits[index3 % (sizeof(_addbits)/sizeof(_addbits[0]))];
		buffer[index3] &= (char)(~_lossbits[index3 % (sizeof(_lossbits)/sizeof(_lossbits[0]))]);
	}

	/*
	**	Perform a general cypher transformation on the vector
	**	and use the vector itself as the cypher key. This is a variation on the
	**	cypher process used in PGP. It is a very strong cypher process with no known
	**	weaknesses. However, in this case, the cypher key is the vector itself and this
	**	opens up a weakness against attacks that have access to this transformation
	**	algorithm. The sheer workload of reversing this transformation should be enough
	**	to discourage even the most determined hackers.
	*/
	for (int index4 = 0; index4 < length; index4 += 4) {
		short key1 = buffer[index4];
		short key2 = buffer[index4+1];
		short key3 = buffer[index4+2];
		short key4 = buffer[index4+3];
		short val1 = key1;
		short val2 = key2;
		short val3 = key3;
		short val4 = key4;

		val1 *= key1;
		val2 += key2;
		val3 += key3;
		val4 *= key4;

		short s3 = val3;
		val3 ^= val1;
		val3 *= key1;
		short s2 = val2;
		val2 ^= val4;
		val2 += val3;
		val2 *= key3;
		val3 += val2;

		val1 ^= val2;
		val4 ^= val3;

		val2 ^= s3;
		val3 ^= s2;

		buffer[index4] = val1;
		buffer[index4+1] = val2;
		buffer[index4+2] = val3;
		buffer[index4+3] = val4;
	}

	/*
	**	Convert this final vector into a cypher key code to be
	**	returned by this routine.
	*/
	code = CRCEngine()(buffer, length);

	/*
	**	Return the final code value.
	*/
	return(code);
}


