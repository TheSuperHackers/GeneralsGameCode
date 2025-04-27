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

#ifndef __MANGLER_H__
#define __MANGLER_H__

// Packet should consist of a GlobalHeaderType followed by a GlobalPacketType with the fields set as
// indicated.


/*
********************************** Defines **********************************
*/

// This is the number of additional ports to which to reply.
#define BLITZ_SIZE 3

#define	MPLAYER_NAME_MAX	20
#define SERIAL_MAX			23
typedef unsigned char ForwardMaskType;
typedef enum NetCommandType {
	NET_MANGLER_REQUEST = 43,
	NET_MANGLER_RESPONSE = 44
} NetCommandType;

/*
** One byte alignment.
*/
#if !defined(__GNUC__)
#pragma pack(push, 1)
#define PACK
#else
#define PACK __attribute__ ((__packed__))
#endif

// size = 20 bytes
struct ManglerData {
	unsigned int		CRC PACK;
	unsigned short	magic PACK;
	unsigned short	packetID PACK;
	unsigned short	MyMangledPortNumber PACK;
	unsigned short	OriginalPortNumber PACK;
	unsigned char		MyMangledAddress[4] PACK;
	unsigned char		NetCommandType PACK;
	unsigned char		BlitzMe PACK;
	unsigned short	Padding PACK;
};

/*
   This is for older GCC versions that can't do byte-packing.
   Instead of declaring
	GlobalPacketType p;
	p.Command = NET_MANGLER_REQUEST;
   you would do something like this:
	GlobalPacketStruct p;
	Eval(&p, GPCommand, NetCommandType) = NET_MANGLER_REQUEST;
*/
#define Eval(x, y, z) ( *((z *)(&((x)->payload[(y)]))) )

#if !defined(__GNUC__)
#pragma pack(pop)
#endif

#endif // __MANGLER_H__
