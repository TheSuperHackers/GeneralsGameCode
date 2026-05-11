/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

// LANObserverStream.h
// TCP-based streaming of an in-progress LAN game's replay file to spectators.
// The host opens a listen socket once the game starts and streams its
// growing .rep file to any number of connected observer clients. Observers
// receive the bytes into a local .rep file and feed them into a
// RECORDERMODETYPE_LIVE_OBSERVER playback that waits at EOF instead of
// terminating, so the watch session stays in sync with the live game.

#pragma once

#include "Lib/BaseType.h"
#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"

#include <stdio.h>
#include <vector>

// Release-mode diagnostic log for the observer feature. Writes to
// "ObserverLog.txt" next to the .exe, fflush after every line so it
// survives a crash. Always compiled in (DEBUG_LOG goes away in release).
void LANObsLog(const char* fmt, ...);

// Port relative to NETWORK_BASE_PORT_NUMBER (8088) used for the host's
// observer-stream listen socket. Chosen well above the per-slot ports.
#define LAN_OBSERVER_PORT_OFFSET 100

// 4-byte snapshot-size prefix sent before the host's initial file dump.
// Observer reads this first, then knows when the catch-up snapshot ends and
// live streaming bytes begin.
struct LANObserverStreamHeader
{
	UnsignedInt snapshotSize; // bytes of replay content immediately following
};


/**
 * Host-side TCP server that accepts observer connections and streams the
 * recording's .rep file to each. Polled from LANAPI::update; single-threaded
 * with non-blocking sockets.
 */
class LANObserverHost
{
public:
	LANObserverHost();
	~LANObserverHost();

	// Open listen socket on the given port. The host must call setReplayFile()
	// before any observer connects (typically at MSG_GAME_START time).
	Bool start(UnsignedShort port);

	// Tell the host which file to stream. Safe to call after start() and
	// before observers arrive. The host opens its own read handle per
	// observer when one connects.
	void setReplayFile(const AsciiString& path);

	// Close listen socket and drop all observer connections.
	void stop();

	// Non-blocking poll: accept new connections; for each observer, read
	// more bytes from the .rep file and send what fits in the kernel buffer.
	// Returns the number of newly-connected observers this tick (for
	// triggering chat notifications).
	Int update(UnicodeString* outNewObserverNames = nullptr, Int outNamesCap = 0);

	Bool isRunning() const { return m_listenFd != -1; }
	Int observerCount() const { return (Int)m_conns.size(); }

private:
	struct ObserverConn
	{
		Int        socketFd;
		FILE*      readHandle;     // separate read handle into the replay file
		UnsignedInt readOffset;    // where we are in the source file
		UnsignedInt snapshotSize;  // declared snapshot size (sent to observer)
		Bool       sentHeader;     // whether the 4-byte snapshot header is sent
		std::vector<unsigned char> sendBuf; // pending bytes to push to socket
		UnicodeString name;        // observer's display name (for chat)
		UnsignedInt connectedAt;   // ms timestamp for staleness tracking
	};

	Int           m_listenFd;
	UnsignedShort m_port;
	AsciiString   m_replayPath;
	std::vector<ObserverConn*> m_conns;

	void acceptNew(UnicodeString* outNewObserverNames, Int outNamesCap, Int& outNewCount);
	Bool pumpReadFromFile(ObserverConn* conn);
	Bool pumpSendToSocket(ObserverConn* conn);
	void closeConn(ObserverConn* conn);
};


/**
 * Observer-side TCP client. Connects to the host, receives the snapshot +
 * live stream into a local .rep file. The recorder is started in
 * RECORDERMODETYPE_LIVE_OBSERVER once the snapshot has been fully received.
 */
class LANObserverClient
{
public:
	enum State
	{
		STATE_IDLE,
		STATE_CONNECTING,
		STATE_BUFFERING,   // waiting for snapshot header / contents
		STATE_READY,       // snapshot complete; safe to call TheRecorder->playbackFile
		STATE_STREAMING,   // playback is running; new bytes are appended live
		STATE_CLOSED,      // host disconnected cleanly or with error
	};

	LANObserverClient();
	~LANObserverClient();

	// Open TCP connection to (ip,port) and prepare local file at localPath.
	// Non-blocking; check state() to know progress.
	Bool connect(UnsignedInt ipNetworkOrder, UnsignedShort port, const AsciiString& localPath);

	// Non-blocking poll: progress connect, recv bytes, write to file. Call
	// every tick.
	void update();

	// Transition out of STATE_READY into STATE_STREAMING. Call this once you
	// have kicked off the recorder playback against the local file.
	void markPlaybackStarted() { if (m_state == STATE_READY) m_state = STATE_STREAMING; }

	void close();

	State          state() const { return m_state; }
	AsciiString    localFilePath() const { return m_localPath; }
	UnsignedInt    bytesReceived() const { return m_bytesWritten; }
	UnsignedInt    snapshotSize() const { return m_snapshotSize; }
	Bool           isStreamClosed() const { return m_state == STATE_CLOSED; }

private:
	State        m_state;
	Int          m_socketFd;
	FILE*        m_writeHandle;
	AsciiString  m_localPath;
	UnsignedInt  m_bytesWritten;      // total bytes appended to local file

	// Snapshot header progress. Bytes accumulate into m_headerBuf until the
	// full LANObserverStreamHeader has arrived, then snapshotSize is parsed.
	UnsignedInt   m_headerBytesRead;
	unsigned char m_headerBuf[sizeof(LANObserverStreamHeader)];
	UnsignedInt   m_snapshotSize;
	UnsignedInt   m_snapshotBytesRead;
};
