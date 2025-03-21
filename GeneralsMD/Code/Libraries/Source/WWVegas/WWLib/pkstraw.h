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
 *                     $Archive:: /Commando/Library/PKSTRAW.H                                 $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef PKSTRAW_H
#define PKSTRAW_H

#include	"blwstraw.h"
#include	"PK.H"
#include	"pkstraw.h"
#include	"RNDSTRAW.H"

class PKStraw : public Straw
{
	public:
		typedef enum CryptControl {
			ENCRYPT,
			DECRYPT
		} CryptControl;

		PKStraw(CryptControl control, RandomStraw & rnd);

		virtual void Get_From(Straw * straw);
		virtual void Get_From(Straw & straw) {Get_From(&straw);}

		virtual int Get(void * source, int slen);

		// Submit key to be used for encryption/decryption.
		void Key(PKey const * key);

	private:
		enum {
			BLOWFISH_KEY_SIZE=BlowfishEngine::MAX_KEY_LENGTH,
			MAX_KEY_BLOCK_SIZE=256		// Maximum size of pk encrypted blowfish key.
		};

		/*
		**	This flag indicates whether the PK (fetch blowfish key) phase is
		**	in progress or not.
		*/
		bool IsGettingKey;

		/*
		**	This is the random straw that is needed to generate the
		**	blowfish key.
		*/
		RandomStraw & Rand;

		/*
		**	This is the attached blowfish pipe. After the blowfish key has been
		**	decrypted, then the PK processor goes dormant and the blowfish processor
		**	takes over the data flow.
		*/
		BlowStraw BF;

		/*
		**	This control member tells what method (encryption or decryption) that should
		**	be performed on the data stream.
		*/
		CryptControl Control;

		/*
		**	Pointer to the key to use for encryption or decryption. If this pointer is NULL, then
		**	the data passing through this segment will not be modified.
		*/
		PKey const * CipherKey;

		/*
		**	This is the staging buffer for the block of data. This block must be as large as
		**	the largest possible key size or the largest blowfish key size (whichever is
		**	greater).
		*/
		char Buffer[256];

		int Counter;

		/*
		**	This records the number of bytes remaining in the current block. This
		**	will be the number of bytes left to accumulate before the block can be
		**	processed either for encryption or decryption.
		*/
		int BytesLeft;

		int Encrypted_Key_Length(void) const;
		int Plain_Key_Length(void) const;

		PKStraw(PKStraw & rvalue);
		PKStraw & operator = (PKStraw const & straw);
};

#endif
