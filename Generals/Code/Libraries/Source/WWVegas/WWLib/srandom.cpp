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

//
// SecureRandomClass - Generate random values
//

#pragma warning(disable : 4514)	// unreferenced inline function removed....

#include "srandom.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef _UNIX
#include "osdep.h"
#else
#include "win.h"
#include <process.h>
#endif
#include <time.h>
#include <assert.h>
#include "sha.h"

// Static class variables
unsigned char	SecureRandomClass::Seeds[SecureRandomClass::SeedLength];
bool				SecureRandomClass::Initialized=false;
unsigned int	SecureRandomClass::RandomCache[SecureRandomClass::SHADigestBytes / sizeof(unsigned int)];
int				SecureRandomClass::RandomCacheEntries=0;
unsigned int	SecureRandomClass::Counter=0;
Random3Class	SecureRandomClass::RandomHelper;

SecureRandomClass::SecureRandomClass()
{
	if (Initialized == false)
	{
		Generate_Seed();
		Initialized=true;
	}
}

SecureRandomClass::~SecureRandomClass()
{
}


//
// Add seed values to our pool of randomness
//
void SecureRandomClass::Add_Seeds(unsigned char *values, int length)
{
	for (int i=0; i<length; i++)
	{
		Seeds[0]^=values[i];

		// Rotate the seeds to the left
		unsigned char uctemp=Seeds[SeedLength-1];
		for (int j=SeedLength-1; j>=1; j--)
			Seeds[j]=Seeds[j-1];
		Seeds[0]=uctemp;
	}

	// We have a better seed pool now so trigger new random values
	RandomCacheEntries=0;
}

//
// Get a 32bit random value
//
unsigned long SecureRandomClass::Randval(void)
{
	if (RandomCacheEntries == 0)
	{
		SHAEngine sha;
		char digest[SHADigestBytes];	// SHA produces a 20 byte hash

		sha.Hash(Seeds, SeedLength);
		sha.Result(digest);

		memcpy(RandomCache, digest, SHADigestBytes);
		RandomCacheEntries=(SHADigestBytes / sizeof(unsigned int));

		unsigned int *int_seeds=(unsigned int *)Seeds;
		int_seeds[0]^=Counter;			// remove the last counter (double xor)
		int_seeds[0]^=(Counter+1);		// put the new counter in place

		int_seeds[(SeedLength/sizeof(int))-1]^=Counter;		// remove the last counter (double xor)
		int_seeds[(SeedLength/sizeof(int))-1]^=(Counter+1);	// put the new counter in place

		Counter++;				// increment counter
	}

	unsigned long retval=RandomCache[--RandomCacheEntries];

	// SHA doesn't have the best distribution properties in the world
	//   We'll XOR the result with the output of another random number
	unsigned long helperval=RandomHelper();
	retval^=helperval;

	return(retval);
}



/////////////////////////////// Private Methods ///////////////////////////////////////


//
// Seed the random number generator.
// The seed is what makes each run of random numbers unique.  If an observer
//   can guess your seed they can predict your random numbers.
//
//	Note the use of XORs everywhere.  The XOR of a good random number and a bad random
//		number is still a good random number.
//
// Caution: Under windows this isn't nearly as safe as under UNIX!
//
void SecureRandomClass::Generate_Seed(void)
{
	int i;

	// Start with some garbage values
	memset(Seeds, 0xAA, SeedLength);

	unsigned int *int_seeds=(unsigned int *)Seeds;
	int int_seed_length=SeedLength/sizeof(unsigned int);

#ifdef _UNIX
	//
	// On UNIX we've already got a great random number souce.
	// This should be used only for a seed since it's slow.
	//
	FILE *in=fopen("/dev/random","r");
	if (in)
	{
		for (i=0; i<SeedLength; i++)
			Seeds[i]^=fgetc(in); 
		fclose(in);
	}
	else
		assert(0);
#else

	//
	// Get free drive space
	//
	DWORD spc, bps, nfc, tnc;	// various drive attributes (we don't care what they mean)
	GetDiskFreeSpace(NULL, &spc, &bps, &nfc, &tnc);
	int_seeds[0]^=spc;
	int_seeds[1 % int_seed_length]^=bps;
	int_seeds[2 % int_seed_length]^=nfc;
	int_seeds[3 % int_seed_length]^=tnc;

	//
	// Get computer & user name
	//
	char	comp_name[128];
	char	user_name[128];
	DWORD	comp_len=128;
	DWORD	name_len=128;

	GetComputerName(comp_name, &comp_len);
	GetUserName(user_name, &name_len);
	for (i=0; i<128; i++)
	{
		// Offset in case user_name == comp_name
		Seeds[(i+0) % SeedLength]^=comp_name[i];
		Seeds[(i+2) % SeedLength]^=user_name[i];
	}

#endif

	for (i=0; i<int_seed_length; i++)
	{
		if ((i % 4) == 0)
			int_seeds[i]^=time(NULL);
		else if ((i % 4) == 1)
			int_seeds[i]^=_getpid();
		else if ((i % 4) == 2)
			int_seeds[i]^=GetTickCount();
		else if ((i % 4) == 3)
			int_seeds[i]^=i;
	}
}