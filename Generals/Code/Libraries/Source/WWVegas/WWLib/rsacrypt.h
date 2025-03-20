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

#ifndef RSACRYPT_H
#define RSACRYPT_H

#include <WWLib/INT.H>
#include <WWLib/WWFILE.H>

//#define SIMPLE_AND_SLOW_RSA

// Version identification string for OpenSSH identity files.
#define AUTHFILE_ID_STRING "SSH PRIVATE KEY FILE FORMAT 1.1\n"

//
// RSACrypt - Implementation of RSA using the Int class from wwlib
//
// For key generation I suggest you use OpenSSH (http://www.openssh.com/)
// There is a utility called "ssh-keygen", call:
//
// ssh-keygen [-b bits]
//
// If you generate the file on a UNIX system, be sure and FTP it in BINARY MODE!!!!
//
// You can then use 'Load_SSH_Keyset' on the identity file it produces.
//
// Note: Make sure you leave the keyphrase blank for the SSH keyset.  I don't support
//   decrypting that file!
//
// Notation:
// Public keys:
//	n = product of two primes, p & q (p & q must remain secret)
//	e = relatively prime to (p-1)(q-1)
// 
// Private keys:
//	d = e^-1 mod ((p-1)(q-1))		// e^-1 = inverse of e
//
// Encrypting:
// cyphertext = m^e mod n
//
// Decrypting:
// message = cyphertext^d mod n
//
// Note: I use a trick involving the chinese remainder theorem to get a 3x speedup in decryption.
// It's well documented on the web so I won't get into detail here.
//
template <int PRECISION>
class RSACrypt
{
 public:
		typedef Int<PRECISION>	Integer;

		RSACrypt()	{}
		~RSACrypt()	{}

		void Set_Public_Keys(const Integer &pub_n, const Integer &pub_e);
		void Set_Keys(const Integer &pub_n, const Integer &pub_e, 
			const Integer &priv_d, const Integer &keygen_p, const Integer &keygen_q);

		void Get_Public_Keys(Integer &pub_n, Integer &pub_e) const;
		void Get_Private_Key(Integer &priv_d) const;
		void Get_Keygen_Keys(Integer &keygen_p, Integer &keygen_q) const;

		bool Load_SSH_Keyset(FileClass *file);

		void Encrypt(const Integer &plaintext, Integer &cyphertext) const;
		void Decrypt(const Integer &cyphertext, Integer &plaintext) const;

 private:
		bool Load_Bignum(FileClass *file, Integer &num);

		void Decryption_Setup();		// Do precomputation to speedup decryption

		Integer		PublicN;
		Integer		PublicE;

		Integer		PrivateD;

		Integer		KeygenP;		// Primes P & Q generated as part of the keyset
		Integer		KeygenQ;

										// Precomputed values that speed up encryption
		Integer		DmodPm1;		// d mod p-1
		Integer		DmodQm1;		// d mod q-1

		Integer		RP;			// RP = q^(p-1) mod n
		Integer		RQ;			// RQ = p^(q-1) mod n;
};

//
// Set the two public keys: n & e
//
template <int PRECISION>
void RSACrypt<PRECISION>::Set_Public_Keys(const Integer &pub_n, const Integer &pub_e)
{
	PublicN=pub_n;
	PublicE=pub_e;
}

//
// Set the public & private keys: n, e & d
//
template <int PRECISION>
void RSACrypt<PRECISION>::Set_Keys(const Integer &pub_n, const Integer &pub_e, 
	const Integer &priv_d, const Integer &keygen_p, const Integer &keygen_q)
{
	PublicN=pub_n;
	PublicE=pub_e;
	PrivateD=priv_d;

	KeygenP=keygen_p;
	KeygenQ=keygen_q;

	Decrtyption_Setup();
}

//
// Get the public keys
//
template <int PRECISION>
void RSACrypt<PRECISION>::Get_Public_Keys(Integer &pub_n, Integer &pub_e) const
{
	pub_n=PublicN;
	pub_e=PublicE;
}

//
// Get the private key
//
template <int PRECISION>
void RSACrypt<PRECISION>::Get_Private_Key(Integer &priv_d) const 
{
	priv_d=PrivateD;
}


//
// Get the private numbers created during the keyset generation
// Private as in revealing these will reveal the private key!
//
template <int PRECISION>
void RSACrypt<PRECISION>::Get_Keygen_Keys(Integer &keygen_p, Integer &keygen_q) const
{
	keygen_p=KeygenP;
	keygen_q=KeygenQ;
}


//
// Load an RSA private keyset from an OpenSSH "identity" file.
//
template <int PRECISION>
bool RSACrypt<PRECISION>::Load_SSH_Keyset(FileClass *file)
{
	assert(file);
	if ( ! file)
		return(false);

	bool retval=true;
	unsigned char buffer[1024];

	if ( ! file->Open())
		return(false);

	file->Read(buffer, strlen(AUTHFILE_ID_STRING)+1);
	buffer[strlen(AUTHFILE_ID_STRING)]=0;	// null term

	if (strcmp((char *)buffer, AUTHFILE_ID_STRING))
		return(false);

	unsigned char cypher_type;		// keyfile encryption method
	file->Read(&cypher_type, 1);
	if (cypher_type != 0)
		return(false);

	file->Read(buffer, 4);		// reserved data

	file->Read(buffer, 4);		// ignored

	retval=retval && Load_Bignum(file, PublicN);
	retval=retval && Load_Bignum(file, PublicE);
	if (!retval)
		return(false);

	// comment string
	int comment_length;
	file->Read(&comment_length, 4);
	comment_length=ntohl(comment_length);

	file->Read(buffer, comment_length);

	// post-decrypt check chars
	file->Read(buffer, 4);
	if ((buffer[0] != buffer[2]) || (buffer[1] != buffer[3]))
		return(false);

	Integer q_inv_mod_p;	// invserse of q mod p (we don't need this)

	retval=retval && Load_Bignum(file, PrivateD);
	retval=retval && Load_Bignum(file, q_inv_mod_p);
	retval=retval && Load_Bignum(file, KeygenP);
	retval=retval && Load_Bignum(file, KeygenQ);
	if (!retval)
		return(false);

	// Any remaining bytes are padding

	Decryption_Setup();

	return(true);
}

//
// Precomputation for fast decryption
//
template <int PRECISION>
void RSACrypt<PRECISION>::Decryption_Setup(void)
{
	Integer temp;

	// If p < q, swap p & q
	if (KeygenP < KeygenQ)
	{
		temp=KeygenP;
		KeygenP=KeygenQ;
		KeygenQ=temp;
	}

	assert(KeygenP > KeygenQ);

	// pm1 = p - 1
	Integer pm1(KeygenP);
	--pm1;

	// pm1 = p - 1
	Integer qm1(KeygenQ);
	--qm1;

	// DmodPm1 = d mod (p-1)
	Integer::Unsigned_Divide(DmodPm1, temp, PrivateD, pm1);
	assert(DmodPm1 < pm1);

	// DmodQm1 = d mod (q-1)
	Integer::Unsigned_Divide(DmodQm1, temp, PrivateD, qm1);
	assert(DmodQm1 < qm1);

	RP = KeygenQ.exp_b_mod_c(pm1, PublicN);
	RQ = KeygenP.exp_b_mod_c(qm1, PublicN);
}


//
// RSA Encryption		c = m^e mod n
//
template <int PRECISION>
void RSACrypt<PRECISION>::Encrypt(const Integer &plaintext, Integer &cyphertext) const 
{
	Integer m(plaintext);
	cyphertext=m.exp_b_mod_c(PublicE, PublicN);
}


//
// RSA Decryption		m = c^d mod n
//
template <int PRECISION>
void RSACrypt<PRECISION>::Decrypt(const Integer &cyphertext, Integer &plaintext) const 
{
#ifdef SIMPLE_AND_SLOW_RSA
	Integer c(cyphertext);
	plaintext=c.exp_b_mod_c(PrivateD, PublicN);
#else
	Integer temp;

	// Get a version of the cyphertext mod q & p
	Integer cmp, cmq;
	Integer::Unsigned_Divide(cmp, temp, cyphertext, KeygenP);
	Integer::Unsigned_Divide(cmq, temp, cyphertext, KeygenQ);

	// mp = cmp ^ dmp mod p
	Integer mp;
	mp=cmp.exp_b_mod_c(DmodPm1, KeygenP);

	// mq = cmq ^ dmq mod q
	Integer mq;
	mq=cmq.exp_b_mod_c(DmodQm1, KeygenQ);


	Integer	sp, sq;

	//sp=mp * RP mod n;
	//sq=mq * RQ mod n;
	XMP_Prepare_Modulus(&PublicN.reg[0], PRECISION);
	XMP_Mod_Mult(&sp.reg[0], &mp.reg[0], &RP.reg[0], PRECISION);
	XMP_Mod_Mult(&sq.reg[0], &mq.reg[0], &RQ.reg[0], PRECISION);
	XMP_Mod_Mult_Clear(PRECISION);

	plaintext = sp+sq;
	if (plaintext >= PublicN)
		plaintext-=PublicN;

#endif
}


////////////////////////////////// Private Methods Below ///////////////////////////////////

//
// Load a large number from the SSH keyset file
//
template <int PRECISION>
bool RSACrypt<PRECISION>::Load_Bignum(FileClass *file, Integer &num)
{
	int readlen;
	unsigned char buffer[1024];
	unsigned short int n_bits, n_bytes;

	readlen=file->Read(&n_bits, 2);		// bits in network byte order
	if (readlen != 2)
		return(false);
	n_bits=ntohs(n_bits);
	n_bytes=(n_bits+7)/8;

	readlen=file->Read(buffer, n_bytes);
	if (readlen != n_bytes)
		return(false);

	num.Unsigned_Decode(buffer, n_bytes);

	return(true);
}


#endif