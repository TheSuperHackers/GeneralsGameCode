/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#pragma once

#include "GameNetwork/NetworkDefs.h"


struct WrappedCommandMetadata
{
	UnsignedByte playerID;
	UnsignedInt chunkNumber;
	UnsignedInt numChunks;
	UnsignedInt totalDataLength;
	UnsignedInt dataLength;
	UnsignedInt dataOffset;
};

inline Bool IsValidNetworkPayloadLength(UnsignedInt dataLength, size_t availableLength, UnsignedInt maxDataLength)
{
	return dataLength <= maxDataLength && dataLength <= availableLength;
}

inline Bool IsValidWrappedCommandMetadata(const WrappedCommandMetadata &metadata)
{
	if (metadata.playerID >= MAX_SLOTS)
		return false;

	if (metadata.numChunks == 0 || metadata.numChunks > MAX_WRAPPED_COMMAND_CHUNKS)
		return false;

	if (metadata.totalDataLength == 0 || metadata.totalDataLength > MAX_WRAPPED_COMMAND_SIZE)
		return false;

	// Every chunk must contribute at least one byte.
	if (metadata.numChunks > metadata.totalDataLength)
		return false;

	if (metadata.chunkNumber >= metadata.numChunks)
		return false;

	if (metadata.dataLength == 0 || metadata.dataLength > MAX_PACKET_SIZE)
		return false;

	if (metadata.dataOffset >= metadata.totalDataLength)
		return false;

	// Subtraction after the offset check avoids overflow from offset + length.
	if (metadata.dataLength > metadata.totalDataLength - metadata.dataOffset)
		return false;

	return true;
}

inline UnsignedInt GetWrappedCommandAllocationSize(UnsignedInt totalDataLength, UnsignedInt numChunks)
{
	return totalDataLength + numChunks * (sizeof(Bool) + 2 * sizeof(UnsignedInt));
}

inline Bool IsCompleteWrappedCommandLayout(const UnsignedInt *chunkOffsets, const UnsignedInt *chunkLengths,
	UnsignedInt numChunks, UnsignedInt totalDataLength)
{
	if (chunkOffsets == nullptr || chunkLengths == nullptr || numChunks == 0 || totalDataLength == 0)
		return false;

	UnsignedInt expectedOffset = 0;
	for (UnsignedInt i = 0; i < numChunks; ++i)
	{
		if (chunkOffsets[i] != expectedOffset || chunkLengths[i] == 0)
			return false;

		if (chunkLengths[i] > totalDataLength - expectedOffset)
			return false;

		expectedOffset += chunkLengths[i];
	}

	return expectedOffset == totalDataLength;
}

inline Bool ShouldReceiveNetworkMessage(UnsignedInt processedMessages, Bool hasBufferCapacity)
{
	return hasBufferCapacity && processedMessages < MAX_MESSAGES;
}
