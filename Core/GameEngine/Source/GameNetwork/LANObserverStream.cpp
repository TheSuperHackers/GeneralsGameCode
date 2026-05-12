/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
*/

#include "PreRTS.h"
#include "GameNetwork/LANObserverStream.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>

// Release-mode log so we can diagnose observer flow without rebuilding
// with debug logging. fflush after every write so the file survives a
// crash. Static FILE* opened on first call.
static FILE* s_obsLogFile = NULL;
void LANObsLog(const char* fmt, ...)
{
	if (!s_obsLogFile)
	{
		s_obsLogFile = fopen("ObserverLog.txt", "w");
		if (!s_obsLogFile)
			return;
	}
	time_t now = time(NULL);
	struct tm* lt = localtime(&now);
	if (lt)
		fprintf(s_obsLogFile, "[%02d:%02d:%02d] ", lt->tm_hour, lt->tm_min, lt->tm_sec);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(s_obsLogFile, fmt, ap);
	va_end(ap);
	fputc('\n', s_obsLogFile);
	fflush(s_obsLogFile);
}

#ifdef _WIN32
#include <winsock.h>
#include <io.h>
#define CLOSE_SOCKET(fd) ::closesocket(fd)
#define SOCK_ERR_LAST   WSAGetLastError()
#define SOCK_ERR_WOULDBLOCK WSAEWOULDBLOCK
#define SOCK_ERR_INPROGRESS WSAEINPROGRESS
#define SOCK_ERR_ALREADY    WSAEALREADY
#define SOCK_ERR_ISCONN     WSAEISCONN
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#define CLOSE_SOCKET(fd) ::close(fd)
#define SOCK_ERR_LAST       errno
#define SOCK_ERR_WOULDBLOCK EWOULDBLOCK
#define SOCK_ERR_INPROGRESS EINPROGRESS
#define SOCK_ERR_ALREADY    EALREADY
#define SOCK_ERR_ISCONN     EISCONN
#define SOCKET_ERROR        (-1)
#endif

// Helper to put a socket into non-blocking mode (mirrors UDP::SetBlocking).
static Bool setNonBlocking(Int fd)
{
#ifdef _WIN32
	unsigned long flag = 1;
	return (ioctlsocket(fd, FIONBIO, &flag) != SOCKET_ERROR);
#else
	Int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return FALSE;
	return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) >= 0);
#endif
}

// Max chunk to read from the source replay file per observer per tick.
static const UnsignedInt OBS_READ_CHUNK_BYTES = 32 * 1024;
// Max bytes to push to a socket per tick (caps a single observer's bandwidth).
static const UnsignedInt OBS_SEND_CHUNK_BYTES = 32 * 1024;
// Soft cap on send-buffer growth before we throttle file reads.
static const UnsignedInt OBS_SEND_BUF_HIGH_WATER = 256 * 1024;


// =====================================================================
// LANObserverHost
// =====================================================================

LANObserverHost::LANObserverHost()
	: m_listenFd(-1)
	, m_port(0)
{
}

LANObserverHost::~LANObserverHost()
{
	stop();
}

Bool LANObserverHost::start(UnsignedShort port)
{
	if (m_listenFd != -1)
		return TRUE; // already listening

	Int fd = (Int)socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		DEBUG_LOG(("LANObserverHost::start - socket() failed (%d)", SOCK_ERR_LAST));
		return FALSE;
	}

	// SO_REUSEADDR so we can rebind quickly across consecutive games.
	Int yes = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(port);

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		DEBUG_LOG(("LANObserverHost::start - bind(%u) failed (%d)", port, SOCK_ERR_LAST));
		CLOSE_SOCKET(fd);
		return FALSE;
	}
	if (listen(fd, 8) == SOCKET_ERROR)
	{
		DEBUG_LOG(("LANObserverHost::start - listen failed (%d)", SOCK_ERR_LAST));
		CLOSE_SOCKET(fd);
		return FALSE;
	}
	if (!setNonBlocking(fd))
	{
		DEBUG_LOG(("LANObserverHost::start - setNonBlocking failed"));
		CLOSE_SOCKET(fd);
		return FALSE;
	}

	m_listenFd = fd;
	m_port     = port;
	DEBUG_LOG(("LANObserverHost::start - listening on port %u", port));
	return TRUE;
}

void LANObserverHost::setReplayFile(const AsciiString& path)
{
	m_replayPath = path;
}

void LANObserverHost::stop()
{
	for (size_t i = 0; i < m_conns.size(); ++i)
		closeConn(m_conns[i]);
	m_conns.clear();
	if (m_listenFd != -1)
	{
		CLOSE_SOCKET(m_listenFd);
		m_listenFd = -1;
	}
}

void LANObserverHost::closeConn(ObserverConn* conn)
{
	if (!conn) return;
	if (conn->socketFd != -1) CLOSE_SOCKET(conn->socketFd);
	if (conn->readHandle)     fclose(conn->readHandle);
	delete conn;
}

void LANObserverHost::acceptNew(UnicodeString* outNames, Int outNamesCap, Int& outCount)
{
	if (m_listenFd == -1)
		return;

	for (;;)
	{
		struct sockaddr_in peer;
		// socklen_t is missing in VC6 winsock; the int form works on both.
		int                peerLen = sizeof(peer);
		memset(&peer, 0, sizeof(peer));
		Int newFd = (Int)accept(m_listenFd, (struct sockaddr*)&peer, &peerLen);
		if (newFd < 0)
		{
			// Out of pending connections is the common case; bail quietly.
			break;
		}
		setNonBlocking(newFd);

		ObserverConn* conn = new ObserverConn();
		conn->socketFd     = newFd;
		conn->readHandle   = NULL;
		conn->readOffset   = 0;
		conn->snapshotSize = 0;
		conn->sentHeader   = FALSE;
		conn->connectedAt  = timeGetTime();

		// Snapshot the file size NOW; this becomes the "catch-up" boundary.
		// Everything else streams live.
		if (!m_replayPath.isEmpty())
		{
			conn->readHandle = fopen(m_replayPath.str(), "rb");
			if (conn->readHandle)
			{
				fseek(conn->readHandle, 0, SEEK_END);
				long sz = ftell(conn->readHandle);
				if (sz < 0) sz = 0;
				conn->snapshotSize = (UnsignedInt)sz;
				fseek(conn->readHandle, 0, SEEK_SET);
			}
			else
			{
				DEBUG_LOG(("LANObserverHost::acceptNew - fopen('%s') failed", m_replayPath.str()));
			}
		}

		// Queue the 4-byte snapshot-size header into the send buffer; the
		// observer reads it first to know when catch-up ends.
		LANObserverStreamHeader hdr;
		hdr.snapshotSize = conn->snapshotSize;
		const unsigned char* p = (const unsigned char*)&hdr;
		conn->sendBuf.insert(conn->sendBuf.end(), p, p + sizeof(hdr));

		m_conns.push_back(conn);

		// Report name slot for chat notification. We don't know the observer's
		// display name (TCP connection carries no identity), so we use the IP
		// string. The shell can replace this later via a separate hello msg.
		if (outNames && outCount < outNamesCap)
		{
			char ipbuf[64];
			UnsignedInt nbo = peer.sin_addr.s_addr; // network-order
			snprintf(ipbuf, sizeof(ipbuf), "Observer@%u.%u.%u.%u",
				(nbo) & 0xff, (nbo>>8) & 0xff, (nbo>>16) & 0xff, (nbo>>24) & 0xff);
			UnicodeString u;
			u.translate(AsciiString(ipbuf));
			outNames[outCount++] = u;
		}

		DEBUG_LOG(("LANObserverHost - new observer fd=%d snapshot=%u",
			newFd, conn->snapshotSize));
	}
}

// Read new file bytes into conn->sendBuf, up to one OBS_READ_CHUNK_BYTES chunk
// per call. Returns FALSE if the read encountered an unrecoverable error.
Bool LANObserverHost::pumpReadFromFile(ObserverConn* conn)
{
	if (!conn->readHandle)
		return TRUE; // nothing to read; harmless

	// Throttle to avoid blowing memory if the observer is slow to drain.
	if (conn->sendBuf.size() >= OBS_SEND_BUF_HIGH_WATER)
		return TRUE;

	// stdio buffer on the read handle won't see writes from the engine File
	// handle (separate FILE*) until we nudge it. clearerr + fseek-to-current
	// invalidates the read buffer reliably across platforms.
	clearerr(conn->readHandle);
	fseek(conn->readHandle, (long)conn->readOffset, SEEK_SET);

	unsigned char buf[OBS_READ_CHUNK_BYTES];
	size_t got = fread(buf, 1, sizeof(buf), conn->readHandle);
	if (got > 0)
	{
		conn->sendBuf.insert(conn->sendBuf.end(), buf, buf + got);
		conn->readOffset += (UnsignedInt)got;
	}
	return TRUE;
}

// Push as much of conn->sendBuf as the socket will accept this tick. Returns
// FALSE if the socket has died and the connection should be dropped.
Bool LANObserverHost::pumpSendToSocket(ObserverConn* conn)
{
	if (conn->sendBuf.empty())
		return TRUE;

	UnsignedInt remaining = (UnsignedInt)conn->sendBuf.size();
	UnsignedInt toSend = remaining > OBS_SEND_CHUNK_BYTES ? OBS_SEND_CHUNK_BYTES : remaining;

	Int sent = (Int)send(conn->socketFd, (const char*)&conn->sendBuf[0], toSend, 0);
	if (sent < 0)
	{
		Int err = SOCK_ERR_LAST;
		if (err == SOCK_ERR_WOULDBLOCK)
			return TRUE; // try again next tick
		DEBUG_LOG(("LANObserverHost - send error %d on fd=%d, dropping", err, conn->socketFd));
		return FALSE;
	}
	if (sent == 0)
	{
		// Peer closed cleanly.
		return FALSE;
	}
	conn->sendBuf.erase(conn->sendBuf.begin(), conn->sendBuf.begin() + sent);
	conn->sentHeader = TRUE;
	return TRUE;
}

Int LANObserverHost::update(UnicodeString* outNewObserverNames, Int outNamesCap)
{
	if (m_listenFd == -1)
		return 0;

	Int newCount = 0;
	acceptNew(outNewObserverNames, outNamesCap, newCount);

	for (size_t i = 0; i < m_conns.size(); )
	{
		ObserverConn* conn = m_conns[i];
		Bool ok = TRUE;
		if (ok) ok = pumpReadFromFile(conn);
		if (ok) ok = pumpSendToSocket(conn);
		if (!ok)
		{
			closeConn(conn);
			m_conns.erase(m_conns.begin() + i);
			continue;
		}
		++i;
	}
	return newCount;
}


// =====================================================================
// LANObserverClient
// =====================================================================

LANObserverClient::LANObserverClient()
	: m_state(STATE_IDLE)
	, m_socketFd(-1)
	, m_writeHandle(NULL)
	, m_bytesWritten(0)
	, m_headerBytesRead(0)
	, m_snapshotSize(0)
	, m_snapshotBytesRead(0)
{
}

LANObserverClient::~LANObserverClient()
{
	close();
}

Bool LANObserverClient::connect(UnsignedInt ipNetworkOrder, UnsignedShort port, const AsciiString& localPath)
{
	close();

	Int fd = (Int)socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		DEBUG_LOG(("LANObserverClient::connect - socket() failed (%d)", SOCK_ERR_LAST));
		return FALSE;
	}
	if (!setNonBlocking(fd))
	{
		CLOSE_SOCKET(fd);
		return FALSE;
	}

	m_writeHandle = fopen(localPath.str(), "wb");
	if (!m_writeHandle)
	{
		DEBUG_LOG(("LANObserverClient::connect - fopen('%s', wb) failed", localPath.str()));
		CLOSE_SOCKET(fd);
		return FALSE;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = ipNetworkOrder;
	addr.sin_port        = htons(port);

	Int rc = ::connect(fd, (struct sockaddr*)&addr, sizeof(addr));
	if (rc == SOCKET_ERROR)
	{
		Int err = SOCK_ERR_LAST;
		if (err != SOCK_ERR_WOULDBLOCK && err != SOCK_ERR_INPROGRESS && err != SOCK_ERR_ALREADY)
		{
			DEBUG_LOG(("LANObserverClient::connect - connect failed (%d)", err));
			CLOSE_SOCKET(fd);
			fclose(m_writeHandle);
			m_writeHandle = NULL;
			return FALSE;
		}
	}

	m_socketFd = fd;
	m_localPath = localPath;
	m_state = STATE_CONNECTING;
	DEBUG_LOG(("LANObserverClient::connect - connecting fd=%d to port %u", fd, port));
	return TRUE;
}

void LANObserverClient::close()
{
	if (m_socketFd != -1)
	{
		CLOSE_SOCKET(m_socketFd);
		m_socketFd = -1;
	}
	if (m_writeHandle)
	{
		fclose(m_writeHandle);
		m_writeHandle = NULL;
	}
	m_state = STATE_IDLE;
	m_bytesWritten      = 0;
	m_headerBytesRead   = 0;
	m_snapshotSize      = 0;
	m_snapshotBytesRead = 0;
}

void LANObserverClient::update()
{
	if (m_socketFd == -1 || m_state == STATE_IDLE || m_state == STATE_CLOSED)
		return;

	// CONNECTING: probe with a zero-byte recv to detect failure, or with a
	// non-blocking second connect() which returns EISCONN once established.
	if (m_state == STATE_CONNECTING)
	{
		struct sockaddr_in dummy;
		memset(&dummy, 0, sizeof(dummy));
		// Cheap probe: try recv(0). Either we get WOULDBLOCK (still in
		// flight) or we read 0 (connected, no data yet) or a real error.
		char probe;
		Int rc = (Int)recv(m_socketFd, &probe, 0, 0);
		Int err = SOCK_ERR_LAST;
		if (rc < 0 && err != SOCK_ERR_WOULDBLOCK)
		{
			DEBUG_LOG(("LANObserverClient - connect failed in poll (%d)", err));
			m_state = STATE_CLOSED;
			return;
		}
		// Optimistically transition to BUFFERING; the actual handshake we
		// detect via the snapshot-size header arriving below.
		m_state = STATE_BUFFERING;
		// fall through to recv below
	}

	// Drain whatever is available on the socket.
	unsigned char buf[16 * 1024];
	for (;;)
	{
		Int got = (Int)recv(m_socketFd, (char*)buf, sizeof(buf), 0);
		if (got > 0)
		{
			const unsigned char* p = buf;
			Int remaining = got;

			// First, consume up to sizeof(header) bytes into a header buffer.
			if (m_headerBytesRead < sizeof(LANObserverStreamHeader))
			{
				UnsignedInt want = (UnsignedInt)sizeof(LANObserverStreamHeader) - m_headerBytesRead;
				UnsignedInt take = (UnsignedInt)remaining < want ? (UnsignedInt)remaining : want;
				memcpy(m_headerBuf + m_headerBytesRead, p, take);
				m_headerBytesRead += take;
				p         += take;
				remaining -= take;
				if (m_headerBytesRead == sizeof(LANObserverStreamHeader))
				{
					LANObserverStreamHeader hdr;
					memcpy(&hdr, m_headerBuf, sizeof(hdr));
					m_snapshotSize = hdr.snapshotSize;
					DEBUG_LOG(("LANObserverClient - snapshot size = %u", m_snapshotSize));
				}
			}

			// Write the rest to the local file.
			if (remaining > 0 && m_writeHandle)
			{
				size_t wrote = fwrite(p, 1, (size_t)remaining, m_writeHandle);
				if (wrote != (size_t)remaining)
				{
					DEBUG_LOG(("LANObserverClient - fwrite short (%zu / %d)", wrote, remaining));
				}
				m_bytesWritten += (UnsignedInt)wrote;
				// Flush so the recorder's reader sees the new bytes.
				fflush(m_writeHandle);

				// Once we've received the full snapshot, mark ready so the
				// caller can kick off playbackFile.
				if (m_state == STATE_BUFFERING
				    && m_headerBytesRead == sizeof(LANObserverStreamHeader)
				    && m_bytesWritten >= m_snapshotSize)
				{
					m_state = STATE_READY;
					LANObsLog("client READY snapshot=%u written=%u", m_snapshotSize, m_bytesWritten);
					DEBUG_LOG(("LANObserverClient - snapshot received (%u bytes), READY", m_bytesWritten));
				}
			}
			continue;
		}
		if (got == 0)
		{
			// Peer closed.
			DEBUG_LOG(("LANObserverClient - host closed stream after %u bytes", m_bytesWritten));
			m_state = STATE_CLOSED;
			if (m_writeHandle) { fflush(m_writeHandle); fclose(m_writeHandle); m_writeHandle = NULL; }
			return;
		}
		Int err = SOCK_ERR_LAST;
		if (err == SOCK_ERR_WOULDBLOCK)
			return;
		DEBUG_LOG(("LANObserverClient - recv error %d", err));
		m_state = STATE_CLOSED;
		return;
	}
}
