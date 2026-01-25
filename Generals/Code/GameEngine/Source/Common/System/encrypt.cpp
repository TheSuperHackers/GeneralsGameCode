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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Encrypt.cpp //////////////////////////////////////////////////////
// Ancient Westwood Online password encryption (obfuscation?) code
// Author: Anonymous

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/encrypt.h"

#define MAX_CHARS 65
static char Base_String[MAX_CHARS] =

    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

static char Return_Buffer[MAX_ENCRYPTED_STRING + 1];
static char Temp_Buffer [MAX_ENCRYPTED_STRING + 1];

/*******************************************************/
/* This function is a simple one-way encryption that   */
/* is portable across many platforms                   */
/*                                                     */
/* The 'String' to encrypt MUST be up to 8 chars long  */
/* and should be no shorter than 4 characters.         */
/* It can contain letters and numbers and '.' and '/'  */
/*******************************************************/

/* String is the original string to encrypt */
/* Seed is the string to encrypt            */
//char *encrypt(char  *String, char  *Seed)
const char *EncryptString(const char *String)
{
	/* We need a 56 bit key, so use two 32 bit values                 */
	/* and we'll strip off the high order 8 bits                      */
	//unsigned long Random_Seed_Value_high = 0; /* 32 bit seed value    */
	//unsigned long Random_Seed_Value_low  = 0; /* 32 bit seed value    */
	//unsigned long Temp_high = 0;              /* 32 bit storage value */
	//unsigned long Temp_low  = 0;              /* 32 bit storage value */
	unsigned int UpCnt = 0, DnCnt = 0, Cnt = 0;
	unsigned int Length = strlen(String);

	/* Ok, here is the algorithm:                    */
	/*                                               */

	if (Length > MAX_ENCRYPTED_STRING)
		Length = MAX_ENCRYPTED_STRING;

	for (UpCnt = 0, DnCnt = Length; UpCnt < Length; UpCnt++, DnCnt--)
		if (String[UpCnt] & 0x01)
			Temp_Buffer[UpCnt] = (String[UpCnt] << (String[UpCnt] & 0x01)) &
			                     String[DnCnt];
		else
			Temp_Buffer[UpCnt] = (String[UpCnt] << (String[UpCnt] & 0x01)) ^
			                     String[DnCnt];

	for (Cnt = 0; Cnt < MAX_ENCRYPTED_STRING; Cnt++)
		Return_Buffer[Cnt] = Base_String[Temp_Buffer[Cnt] & 0x3F];

	Return_Buffer[Cnt] = '\0';
	return (Return_Buffer);

}


#ifdef UNIT_TEST
void main(void)
{
	char Input_String[9];
	char *new_string;

	while (1)
	{
		printf ("Enter a string to encrypt:");
		gets(Input_String);
		printf("\nString enterred was: %s", Input_String);
		new_string = encrypt(Input_String, "ab");
		printf("\nEncrypted string is: %s", new_string);
	}
}

#endif

