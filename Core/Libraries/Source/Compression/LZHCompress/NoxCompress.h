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

// compress.h
// Compress/Decompression header.
// Author: Jeff Brown, January 1999

#pragma once

#define MAP_EXTENSION ".map"
#define LZH_EXTENSION ".nxz"
#define RUL_EXTENSION ".rul"

Bool DecompressFile		(char *infile, char *outfile);
Bool CompressFile			(char *infile, char *outfile);
Bool CompressPacket		(char *inPacket, char *outPacket);
Bool DecompressPacket	(char *inPacket, char *outPacket);
UnsignedInt CalcNewSize		(UnsignedInt rawSize);

Bool DecompressMemory		(void *inBufferVoid, Int inSize, void *outBufferVoid, Int& outSize);
Bool CompressMemory			(void *inBufferVoid, Int inSize, void *outBufferVoid, Int& outSize);
