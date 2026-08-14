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
#include "GameNetwork/NetCommandWrapperList.h"
#include "GameNetwork/NetPacketStructs.h"
#include "GameNetwork/NetCommandRef.h"
#include "GameNetwork/GameSpy/ThreadUtils.h"
#include "GameLogic/TriggerInfo.h"
#include "Common/FrameRateLimit.h"
#include "Common/GameMemory.h"

#include <stdio.h>
#include <windows.h>


class Win32Mouse;
HINSTANCE ApplicationHInstance = nullptr;
HWND ApplicationHWnd = nullptr;
Win32Mouse *TheWin32Mouse = nullptr;
DWORD TheMessageTime = 0;
const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";
const char *gAppPrefix = "";
ICoord2D TheMousePos = { 0, 0 };

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return 0;
}


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

	CHECK(CanTrackWrappedCommand(0, 0, 0, 128));
	CHECK(!CanTrackWrappedCommand(MAX_WRAPPED_COMMAND_NODES, 0, 0, 128));
	CHECK(!CanTrackWrappedCommand(0, MAX_WRAPPED_COMMAND_NODES_PER_PLAYER, 0, 128));
	CHECK(!CanTrackWrappedCommand(0, 0, MAX_WRAPPED_COMMAND_MEMORY, 128));
	CHECK(!CanTrackWrappedCommand(0, 0, MAX_WRAPPED_COMMAND_MEMORY + 1, 1));

	CHECK(!IsWrappedCommandExpired(1000, 1000));
	CHECK(IsWrappedCommandExpired(1000 + WRAPPED_COMMAND_IDLE_TIMEOUT, 1000));
	CHECK(IsWrappedCommandExpired(10, 10U - WRAPPED_COMMAND_IDLE_TIMEOUT));

	CHECK(IsValidExternalBuffer("x", 1, 1, 1024));
	CHECK(!IsValidExternalBuffer(nullptr, 1, 1, 1024));
	CHECK(!IsValidExternalBuffer("", 0, 1, 1024));
	CHECK(!IsValidExternalBuffer("x", 1025, 1, 1024));

	CHECK(IsValidRunAheadFrameRate(1));
	CHECK(!IsValidRunAheadFrameRate(0));

	UnsignedInt totalArguments = 0;
	size_t payloadBytes = 0;
	CHECK(TryAccumulateGameMessageArguments(ARGUMENTDATATYPE_INTEGER, 250, totalArguments, payloadBytes));
	CHECK(totalArguments == 250);
	CHECK(payloadBytes == 250 * sizeof(Int));
	CHECK(!TryAccumulateGameMessageArguments(ARGUMENTDATATYPE_INTEGER, 6, totalArguments, payloadBytes));
	CHECK(!TryAccumulateGameMessageArguments(ARGUMENTDATATYPE_UNKNOWN, 1, totalArguments, payloadBytes));
	CHECK(!TryAccumulateGameMessageArguments(ARGUMENTDATATYPE_INTEGER, 0, totalArguments, payloadBytes));
}

static void TestGameCommandParsing()
{
	UnsignedByte truncatedDescriptors[sizeof(Int) + sizeof(UnsignedByte)] = { 0 };
	size_t size = 0;
	size += network::writePrimitive(truncatedDescriptors + size, (Int)GameMessage::MSG_SELECTED_GROUP_COMMAND);
	size += network::writePrimitive(truncatedDescriptors + size, (UnsignedByte)255);
	NetworkGameMessageLayout layout;
	CHECK(!TryParseNetworkGameMessageLayout(truncatedDescriptors, size, layout));

	UnsignedByte invalidCount[sizeof(Int) + 3 * sizeof(UnsignedByte)] = { 0 };
	size = 0;
	size += network::writePrimitive(invalidCount + size, (Int)GameMessage::MSG_SELECTED_GROUP_COMMAND);
	size += network::writePrimitive(invalidCount + size, (UnsignedByte)1);
	size += network::writePrimitive(invalidCount + size, (UnsignedByte)ARGUMENTDATATYPE_INTEGER);
	size += network::writePrimitive(invalidCount + size, (UnsignedByte)0);
	CHECK(!TryParseNetworkGameMessageLayout(invalidCount, size, layout));

	UnsignedByte truncatedPayload[sizeof(Int) + 3 * sizeof(UnsignedByte)] = { 0 };
	size = 0;
	size += network::writePrimitive(truncatedPayload + size, (Int)GameMessage::MSG_SELECTED_GROUP_COMMAND);
	size += network::writePrimitive(truncatedPayload + size, (UnsignedByte)1);
	size += network::writePrimitive(truncatedPayload + size, (UnsignedByte)ARGUMENTDATATYPE_INTEGER);
	size += network::writePrimitive(truncatedPayload + size, (UnsignedByte)1);
	CHECK(!TryParseNetworkGameMessageLayout(truncatedPayload, size, layout));

	UnsignedByte invalidMessageType[sizeof(Int) + sizeof(UnsignedByte)] = { 0 };
	size = 0;
	size += network::writePrimitive(invalidMessageType + size, (Int)GameMessage::MSG_INVALID);
	size += network::writePrimitive(invalidMessageType + size, (UnsignedByte)0);
	CHECK(!TryParseNetworkGameMessageLayout(invalidMessageType, size, layout));

	UnsignedByte valid[sizeof(Int) + 3 * sizeof(UnsignedByte) + sizeof(Int)] = { 0 };
	size = 0;
	size += network::writePrimitive(valid + size, (Int)GameMessage::MSG_SELECTED_GROUP_COMMAND);
	size += network::writePrimitive(valid + size, (UnsignedByte)1);
	size += network::writePrimitive(valid + size, (UnsignedByte)ARGUMENTDATATYPE_INTEGER);
	size += network::writePrimitive(valid + size, (UnsignedByte)1);
	size += network::writePrimitive(valid + size, (Int)42);
	CHECK(TryParseNetworkGameMessageLayout(valid, size, layout));
	CHECK(layout.messageType == GameMessage::MSG_SELECTED_GROUP_COMMAND);
	CHECK(layout.argumentTypeCount == 1);
	CHECK(layout.argumentTypes[0] == ARGUMENTDATATYPE_INTEGER);
	CHECK(layout.argumentCounts[0] == 1);
	CHECK(layout.payloadBytes == sizeof(Int));
}

class StackNetGameCommandMsg : public NetGameCommandMsg
{
public:
	virtual ~StackNetGameCommandMsg() {}
};

class StackNetCommandRef : public NetCommandRef
{
public:
	StackNetCommandRef(NetCommandMsg *msg) : NetCommandRef(msg) {}
	virtual ~StackNetCommandRef() {}
};

static void TestMalformedGameCommandDeserialization()
{
	UnsignedByte truncatedDescriptors[sizeof(Int) + sizeof(UnsignedByte)] = { 0 };
	size_t size = 0;
	size += network::writePrimitive(truncatedDescriptors + size, (Int)GameMessage::MSG_SELECTED_GROUP_COMMAND);
	size += network::writePrimitive(truncatedDescriptors + size, (UnsignedByte)255);

	StackNetGameCommandMsg msg;
	msg.setPlayerID(1);
	StackNetCommandRef ref(&msg);
	CHECK(NetPacketGameCommandData::readMessage(ref, NetPacketBuf(truncatedDescriptors, size)) == size);
	CHECK(msg.getPlayerID() == MAX_SLOTS);
	CHECK(msg.getGameMessageType() == GameMessage::MSG_INVALID);
}

static NetWrapperCommandMsg *CreateWrapperMessage(UnsignedByte playerID, UnsignedShort commandID,
	UnsignedInt chunkNumber, UnsignedInt numChunks, UnsignedInt totalLength, UnsignedInt dataOffset)
{
	NetWrapperCommandMsg *msg = newInstance(NetWrapperCommandMsg)();
	msg->setPlayerID(playerID);
	msg->setWrappedCommandID(commandID);
	msg->setChunkNumber(chunkNumber);
	msg->setNumChunks(numChunks);
	msg->setTotalDataLength(totalLength);
	msg->setDataOffset(dataOffset);
	NetCommandDataChunk data(1);
	data.data()[0] = static_cast<UnsignedByte>(commandID);
	msg->setData(data);
	return msg;
}

static Bool ProcessWrapper(NetCommandWrapperList &list, NetWrapperCommandMsg *msg)
{
	NetCommandRef *ref = NEW_NETCOMMANDREF(msg);
	msg->detach();
	const Bool accepted = list.processWrapper(ref);
	deleteInstance(ref);
	return accepted;
}

static void TestWrapperLifecycle()
{
	NetCommandWrapperList *list = newInstance(NetCommandWrapperList)();
	CHECK(ProcessWrapper(*list, CreateWrapperMessage(1, 1, 0, 2, 3, 0)));
	CHECK(list->getNodeCount() == 1);
	const UnsignedInt firstAllocation = list->getAllocatedBytes();

	CHECK(ProcessWrapper(*list, CreateWrapperMessage(1, 1, 0, 2, 3, 0)));
	CHECK(list->getNodeCount() == 1);
	CHECK(list->getAllocatedBytes() == firstAllocation);

	CHECK(!ProcessWrapper(*list, CreateWrapperMessage(1, 1, 0, 2, 3, 1)));
	NetWrapperCommandMsg *conflictingData = CreateWrapperMessage(1, 1, 0, 2, 3, 0);
	NetCommandDataChunk replacementData(1);
	replacementData.data()[0] = 99;
	conflictingData->setData(replacementData);
	CHECK(!ProcessWrapper(*list, conflictingData));
	NetWrapperCommandMsg *conflictingLength = CreateWrapperMessage(1, 1, 0, 2, 3, 0);
	NetCommandDataChunk longerData(2);
	longerData.data()[0] = 1;
	longerData.data()[1] = 1;
	conflictingLength->setData(longerData);
	CHECK(!ProcessWrapper(*list, conflictingLength));
	CHECK(list->getNodeCount() == 1);
	CHECK(list->getAllocatedBytes() == firstAllocation);

	// Active transfer metadata is immutable, preventing allocation churn.
	CHECK(!ProcessWrapper(*list, CreateWrapperMessage(1, 1, 0, 3, 3, 0)));
	CHECK(list->getNodeCount() == 1);
	CHECK(list->getAllocatedBytes() == firstAllocation);

	// Once expired, the command ID can be reused with new metadata.
	list->purgeExpired(timeGetTime() + WRAPPED_COMMAND_IDLE_TIMEOUT);
	CHECK(list->getNodeCount() == 0);
	CHECK(ProcessWrapper(*list, CreateWrapperMessage(1, 1, 0, 3, 3, 0)));
	CHECK(list->getNodeCount() == 1);

	list->reset();
	for (UnsignedShort commandID = 0; commandID < MAX_WRAPPED_COMMAND_NODES_PER_PLAYER; ++commandID)
		CHECK(ProcessWrapper(*list, CreateWrapperMessage(2, commandID, 0, 2, 2, 0)));
	CHECK(!ProcessWrapper(*list,
		CreateWrapperMessage(2, MAX_WRAPPED_COMMAND_NODES_PER_PLAYER, 0, 2, 2, 0)));
	CHECK(list->getNodeCount() == MAX_WRAPPED_COMMAND_NODES_PER_PLAYER);
	list->removeForPlayer(2);
	CHECK(list->getNodeCount() == 0);
	CHECK(list->getAllocatedBytes() == 0);

	// A complete but malformed reconstructed command is discarded safely.
	CHECK(ProcessWrapper(*list, CreateWrapperMessage(3, 9, 0, 1, 1, 0)));
	NetCommandList *ready = list->getReadyCommands();
	CHECK(ready->length() == 0);
	CHECK(list->getNodeCount() == 0);
	deleteInstance(ready);
	deleteInstance(list);
}

static void TestStringConversionAndZeroLengthReads()
{
	CHECK(MultiByteToWideCharSingleLine("alpha\nbeta\r") == L"alpha beta ");
	CHECK(MultiByteToWideCharSingleLine(nullptr).empty());
	CHECK(WideCharStringToMultiByte(L"snowman \x2603") == "snowman \xE2\x98\x83");
	CHECK(WideCharStringToMultiByte(nullptr).empty());
	CHECK(network::readBytes(nullptr, 0, NetPacketBuf(nullptr, 0)) == 0);
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

static ULONGLONG FileTimeToTicks(const FILETIME &fileTime)
{
	ULARGE_INTEGER ticks;
	ticks.LowPart = fileTime.dwLowDateTime;
	ticks.HighPart = fileTime.dwHighDateTime;
	return ticks.QuadPart;
}

static void TestFrameRateLimitCpuUsage()
{
	FILETIME createTime;
	FILETIME exitTime;
	FILETIME kernelStart;
	FILETIME userStart;
	FILETIME kernelEnd;
	FILETIME userEnd;
	LARGE_INTEGER frequency;
	LARGE_INTEGER wallStart;
	LARGE_INTEGER wallEnd;

	if (!GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelStart, &userStart) ||
		!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&wallStart))
	{
		CHECK(FALSE);
		return;
	}

	FrameRateLimit limiter;
	for (Int i = 0; i < 240; ++i)
		limiter.wait(480);

	if (!QueryPerformanceCounter(&wallEnd) ||
		!GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelEnd, &userEnd))
	{
		CHECK(FALSE);
		return;
	}

	const double wallSeconds = static_cast<double>(wallEnd.QuadPart - wallStart.QuadPart) / frequency.QuadPart;
	const double cpuSeconds = static_cast<double>(
		FileTimeToTicks(kernelEnd) - FileTimeToTicks(kernelStart) +
		FileTimeToTicks(userEnd) - FileTimeToTicks(userStart)) / 10000000.0;
	const double cpuRatio = cpuSeconds / wallSeconds;
	printf("Frame limiter CPU ratio at 480 FPS: %.3f\n", cpuRatio);
	CHECK(wallSeconds >= 0.45);
	// Leave ample headroom for transient hosted-runner preemption while still
	// catching pathological stalls in the frame limiter.
	CHECK(wallSeconds < 1.50);
	CHECK(cpuRatio < 0.60);
}

static void TestFrameRateLimitWaitCalculation()
{
	CHECK(FrameRateLimit::calculateCoarseWaitTicks(1000, 200) == 800);
	CHECK(FrameRateLimit::calculateCoarseWaitTicks(200, 200) == 0);
	CHECK(FrameRateLimit::calculateCoarseWaitTicks(100, 200) == 0);
	CHECK(FrameRateLimit::calculateCoarseWaitTicks(-1, 200) == 0);
}

int main()
{
	initMemoryManager();

	TestNetworkValidation();
	TestGameCommandParsing();
	TestMalformedGameCommandDeserialization();
	TestWrapperLifecycle();
	TestNetworkReceiveBudget();
	TestStringConversionAndZeroLengthReads();
	TestTriggerInfoStorage();
	TestFrameRateLimitWaitCalculation();
	TestFrameRateLimitCpuUsage();

	if (s_failures != 0)
	{
		printf("%d runtime regression test(s) failed.\n", s_failures);
		shutdownMemoryManager();
		return 1;
	}

	printf("All runtime regression tests passed.\n");
	shutdownMemoryManager();
	return 0;
}
