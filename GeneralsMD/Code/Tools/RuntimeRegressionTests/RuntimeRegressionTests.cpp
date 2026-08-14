/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#include "GameNetwork/NetCommandValidation.h"
#include "GameLogic/TriggerInfo.h"

#include <stdio.h>


static Int s_failures = 0;

#define CHECK(expression) Check((expression), #expression, __LINE__)

static void Check(Bool result, const char *expression, Int line)
{
	if (!result)
	{
		printf("FAIL line %d: %s\n", line, expression);
		++s_failures;
	}
}

static void TestNetworkValidation()
{
	WrappedCommandMetadata metadata;
	metadata.playerID = 1;
	metadata.chunkNumber = 0;
	metadata.numChunks = 1;
	metadata.totalDataLength = 1024;
	metadata.dataLength = 128;
	metadata.dataOffset = 0;

	CHECK(IsValidWrappedCommandMetadata(metadata));

	metadata.numChunks = 0;
	CHECK(!IsValidWrappedCommandMetadata(metadata));

	metadata.numChunks = 1;
	metadata.totalDataLength = MAX_WRAPPED_COMMAND_SIZE + 1;
	CHECK(!IsValidWrappedCommandMetadata(metadata));

	CHECK(!IsValidNetworkPayloadLength(4096, 16, MAX_WRAPPED_COMMAND_SIZE));

	UnsignedInt chunkOffsets[2] = { 0, 4 };
	UnsignedInt chunkLengths[2] = { 4, 4 };
	CHECK(IsCompleteWrappedCommandLayout(chunkOffsets, chunkLengths, 2, 8));

	chunkOffsets[1] = 5;
	CHECK(!IsCompleteWrappedCommandLayout(chunkOffsets, chunkLengths, 2, 8));

	chunkOffsets[1] = 3;
	CHECK(!IsCompleteWrappedCommandLayout(chunkOffsets, chunkLengths, 2, 8));
}

static void TestNetworkReceiveBudget()
{
	CHECK(ShouldReceiveNetworkMessage(0, TRUE));
	CHECK(ShouldReceiveNetworkMessage(MAX_MESSAGES - 1, TRUE));
	CHECK(!ShouldReceiveNetworkMessage(MAX_MESSAGES, TRUE));
	CHECK(!ShouldReceiveNetworkMessage(0, FALSE));
}

static void TestTriggerInfoStorage()
{
	TriggerInfoStorage storage;
	CHECK(storage.resize(2));
	storage[0].isInside = false;
	storage[0].exited = true;
	storage[1].pTrigger = reinterpret_cast<const PolygonTrigger *>(1);
	storage[1].isInside = true;
	storage[1].entered = true;

	Int activeCount = storage.clearTransitionsAndRemoveExited(2);
	CHECK(activeCount == 1);
	CHECK(storage[0].pTrigger == reinterpret_cast<const PolygonTrigger *>(1));
	CHECK(storage[0].isInside);
	CHECK(!storage[0].entered);
	CHECK(!storage[0].exited);

	CHECK(storage.resize(6));
	for (Int i = 0; i < 6; ++i)
	{
		storage[i].pTrigger = reinterpret_cast<const PolygonTrigger *>(i + 1);
		storage[i].isInside = true;
	}
	activeCount = storage.clearTransitionsAndRemoveExited(6);
	CHECK(activeCount == 6);
	CHECK(storage[5].pTrigger == reinterpret_cast<const PolygonTrigger *>(6));
}

int main()
{
	TestNetworkValidation();
	TestNetworkReceiveBudget();
	TestTriggerInfoStorage();

	if (s_failures != 0)
	{
		printf("%d runtime regression test(s) failed.\n", s_failures);
		return 1;
	}

	printf("All runtime regression tests passed.\n");
	return 0;
}
