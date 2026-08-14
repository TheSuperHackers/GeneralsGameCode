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

inline Bool CanTrackWrappedCommand(UnsignedInt nodeCount, UnsignedInt playerNodeCount,
	UnsignedInt allocatedBytes, UnsignedInt allocationSize)
{
	return allocatedBytes <= MAX_WRAPPED_COMMAND_MEMORY &&
		nodeCount < MAX_WRAPPED_COMMAND_NODES &&
		playerNodeCount < MAX_WRAPPED_COMMAND_NODES_PER_PLAYER &&
		allocationSize <= MAX_WRAPPED_COMMAND_MEMORY - allocatedBytes;
}

inline Bool IsWrappedCommandExpired(UnsignedInt now, UnsignedInt lastActivity)
{
	// Unsigned subtraction remains correct when timeGetTime wraps around.
	return now - lastActivity >= WRAPPED_COMMAND_IDLE_TIMEOUT;
}

inline Bool IsValidExternalBuffer(const void *buffer, Int64 length, size_t minimumLength, size_t maximumLength)
{
	return buffer != nullptr && length >= 0 &&
		static_cast<UnsignedInt64>(length) >= minimumLength &&
		static_cast<UnsignedInt64>(length) <= maximumLength;
}

inline Bool IsValidRunAheadFrameRate(UnsignedInt frameRate)
{
	return frameRate > 0;
}

inline size_t GetGameMessageArgumentSize(GameMessageArgumentDataType type)
{
	switch (type)
	{
	case ARGUMENTDATATYPE_INTEGER: return sizeof(Int);
	case ARGUMENTDATATYPE_REAL: return sizeof(Real);
	case ARGUMENTDATATYPE_BOOLEAN: return sizeof(Bool);
	case ARGUMENTDATATYPE_OBJECTID: return sizeof(ObjectID);
	case ARGUMENTDATATYPE_DRAWABLEID: return sizeof(DrawableID);
	case ARGUMENTDATATYPE_TEAMID: return sizeof(UnsignedInt);
	case ARGUMENTDATATYPE_LOCATION: return sizeof(Coord3D);
	case ARGUMENTDATATYPE_PIXEL: return sizeof(ICoord2D);
	case ARGUMENTDATATYPE_PIXELREGION: return sizeof(IRegion2D);
	case ARGUMENTDATATYPE_TIMESTAMP: return sizeof(UnsignedInt);
	case ARGUMENTDATATYPE_WIDECHAR: return sizeof(WideChar);
	default: return 0;
	}
}

inline Bool TryAccumulateGameMessageArguments(GameMessageArgumentDataType type, UnsignedInt count,
	UnsignedInt &totalArguments, size_t &payloadBytes)
{
	const size_t argumentSize = GetGameMessageArgumentSize(type);
	if (argumentSize == 0 || count == 0 || count > 255U - totalArguments)
		return false;

	totalArguments += count;
	payloadBytes += argumentSize * count;
	return true;
}

struct NetworkGameMessageLayout
{
	Int messageType;
	UnsignedByte argumentTypeCount;
	GameMessageArgumentDataType argumentTypes[255];
	UnsignedByte argumentCounts[255];
	size_t descriptorBytes;
	size_t payloadBytes;
};

inline Bool TryParseNetworkGameMessageLayout(const UnsignedByte *data, size_t dataLength,
	NetworkGameMessageLayout &layout)
{
	static const size_t headerSize = sizeof(Int) + sizeof(UnsignedByte);
	static const size_t descriptorSize = 2 * sizeof(UnsignedByte);
	if (data == nullptr || dataLength < headerSize)
		return false;

	memcpy(&layout.messageType, data, sizeof(layout.messageType));
	layout.argumentTypeCount = data[sizeof(layout.messageType)];
	if (layout.messageType <= GameMessage::MSG_BEGIN_NETWORK_MESSAGES ||
		layout.messageType >= GameMessage::MSG_END_NETWORK_MESSAGES)
		return false;

	layout.descriptorBytes = static_cast<size_t>(layout.argumentTypeCount) * descriptorSize;
	if (layout.descriptorBytes > dataLength - headerSize)
		return false;

	UnsignedInt totalArguments = 0;
	layout.payloadBytes = 0;
	for (UnsignedInt i = 0; i < layout.argumentTypeCount; ++i)
	{
		const size_t offset = headerSize + i * descriptorSize;
		layout.argumentTypes[i] = static_cast<GameMessageArgumentDataType>(data[offset]);
		layout.argumentCounts[i] = data[offset + 1];
		if (!TryAccumulateGameMessageArguments(layout.argumentTypes[i], layout.argumentCounts[i],
			totalArguments, layout.payloadBytes))
			return false;
	}

	return layout.payloadBytes <= dataLength - headerSize - layout.descriptorBytes;
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
