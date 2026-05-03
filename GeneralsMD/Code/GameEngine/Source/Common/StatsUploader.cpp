/*
**	Command & Conquer Generals Zero Hour(tm)
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

#include "PreRTS.h"

#include "Common/StatsUploader.h"
#include "Common/AsciiString.h"

#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "wininet.lib")

// Internal: open a WinINet request handle for either a GET or POST.
// Returns nullptr on any failure (logs to stdout). On success the caller
// owns hInternet/hConnect/hRequest and must close them in reverse order.
struct WinInetSession
{
	HINTERNET hInternet;
	HINTERNET hConnect;
	HINTERNET hRequest;
};

static bool openHttpRequest(const AsciiString& url,
                            const char *method,
                            const char *pathOverride,
                            const char *logTag,
                            WinInetSession *out)
{
	out->hInternet = nullptr;
	out->hConnect = nullptr;
	out->hRequest = nullptr;

	if (url.isEmpty())
		return false;

	char hostBuf[256];
	char pathBuf[1024];
	URL_COMPONENTSA uc;
	memset(&uc, 0, sizeof(uc));
	uc.dwStructSize = sizeof(uc);
	uc.lpszHostName = hostBuf;
	uc.dwHostNameLength = sizeof(hostBuf);
	uc.lpszUrlPath = pathBuf;
	uc.dwUrlPathLength = sizeof(pathBuf);

	if (!InternetCrackUrlA(url.str(), 0, 0, &uc))
	{
		printf("%s: failed to parse URL \"%s\"\n", logTag, url.str());
		return false;
	}

	INTERNET_PORT port = uc.nPort;
	if (port == 0)
		port = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

	DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
	if (uc.nScheme == INTERNET_SCHEME_HTTPS)
		flags |= INTERNET_FLAG_SECURE;

	out->hInternet = InternetOpenA("GeneralsStatsExporter/1.0", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	if (out->hInternet == nullptr)
	{
		printf("%s: InternetOpen failed (%lu)\n", logTag, GetLastError());
		return false;
	}

	out->hConnect = InternetConnectA(out->hInternet, hostBuf, port, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
	if (out->hConnect == nullptr)
	{
		printf("%s: InternetConnect failed (%lu)\n", logTag, GetLastError());
		InternetCloseHandle(out->hInternet);
		out->hInternet = nullptr;
		return false;
	}

	const char *requestPath = (pathOverride != nullptr) ? pathOverride : pathBuf;
	out->hRequest = HttpOpenRequestA(out->hConnect, method, requestPath, nullptr, nullptr, nullptr, flags, 0);
	if (out->hRequest == nullptr)
	{
		printf("%s: HttpOpenRequest failed (%lu)\n", logTag, GetLastError());
		InternetCloseHandle(out->hConnect);
		InternetCloseHandle(out->hInternet);
		out->hConnect = nullptr;
		out->hInternet = nullptr;
		return false;
	}

	return true;
}

static void closeHttpRequest(WinInetSession *s)
{
	if (s->hRequest)  InternetCloseHandle(s->hRequest);
	if (s->hConnect)  InternetCloseHandle(s->hConnect);
	if (s->hInternet) InternetCloseHandle(s->hInternet);
}

// Shared HTTP POST. Posts arbitrary bytes with the given Content-Type and
// the X-Game-Seed header. Best-effort; logs status to stdout.
static void httpPostBytes(const AsciiString& url,
                          const void *data,
                          unsigned int dataLen,
                          const char *contentType,
                          const char *extraHeaders,
                          unsigned int seed,
                          const char *logTag)
{
	if (data == nullptr || dataLen == 0)
		return;

	WinInetSession s;
	if (!openHttpRequest(url, "POST", nullptr, logTag, &s))
		return;

	char headers[1024];
	int n = sprintf(headers, "Content-Type: %s\r\nX-Game-Seed: %u\r\n", contentType, seed);
	if (extraHeaders != nullptr && extraHeaders[0] != '\0' && n < (int)sizeof(headers))
	{
		// Caller-provided extra headers (already terminated with \r\n).
		strncat(headers, extraHeaders, sizeof(headers) - 1 - (size_t)n);
	}

	BOOL result = HttpSendRequestA(s.hRequest, headers, (DWORD)strlen(headers), const_cast<void*>(data), dataLen);

	if (result)
	{
		DWORD statusCode = 0;
		DWORD statusSize = sizeof(statusCode);
		HttpQueryInfoA(s.hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, nullptr);
		printf("%s: %s -> %lu\n", logTag, url.str(), statusCode);
	}
	else
	{
		printf("%s: HttpSendRequest failed (%lu)\n", logTag, GetLastError());
	}

	closeHttpRequest(&s);
}

void UploadStatsToServer(const AsciiString& url, const void *data, unsigned int dataLen, unsigned int seed)
{
	httpPostBytes(url, data, dataLen, "application/gzip", nullptr, seed, "Stats upload");
}

// Sanitize a filename for the multipart Content-Disposition header. Strips
// any path components (forward or back slashes), and any double-quote /
// CR / LF that would break the header. Output is bounded by outCap.
static void sanitizeMultipartFilename(const char *src, char *out, unsigned int outCap)
{
	if (outCap == 0)
		return;
	if (src == nullptr) src = "";
	// VC6 leaks for-loop variables into the enclosing scope, so declare
	// the iterator once and reuse it across the two scans.
	const char *p;
	// Skip to last path separator
	const char *base = src;
	for (p = src; *p != '\0'; ++p)
	{
		if (*p == '/' || *p == '\\')
			base = p + 1;
	}
	if (*base == '\0')
		base = "replay.rep";
	unsigned int n = 0;
	for (p = base; *p != '\0' && n + 1 < outCap; ++p)
	{
		char c = *p;
		if (c == '"' || c == '\r' || c == '\n')
			c = '_';
		out[n++] = c;
	}
	out[n] = '\0';
}

// Issue an HTTP POST as multipart/form-data with a single binary part.
// The request body is built in a single malloc'd buffer (header + bytes +
// trailer) and posted via the shared openHttpRequest plumbing. Best-effort;
// logs status to stdout.
static void httpPostMultipartFile(const AsciiString& url,
                                  const char *fieldName,
                                  const char *filename,
                                  const void *data,
                                  unsigned int dataLen,
                                  unsigned int seed,
                                  const char *logTag)
{
	if (data == nullptr || dataLen == 0)
		return;

	// Boundary: long enough that random collision with binary file bytes
	// is negligible. Must not appear inside the file payload preceded by
	// "\r\n--".
	static const char boundary[] = "----GeneralsReplayBoundaryK8nQv2pXr9TfH3";

	char prefix[512];
	int prefixLen = sprintf(prefix,
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"%.63s\"; filename=\"%.255s\"\r\n"
		"Content-Type: application/octet-stream\r\n"
		"\r\n",
		boundary, fieldName, filename);

	char trailer[64];
	int trailerLen = sprintf(trailer, "\r\n--%s--\r\n", boundary);

	if (prefixLen <= 0 || trailerLen <= 0)
		return;

	unsigned int bodyLen = (unsigned int)prefixLen + dataLen + (unsigned int)trailerLen;
	char *body = (char *)malloc(bodyLen);
	if (body == nullptr)
		return;
	memcpy(body, prefix, (size_t)prefixLen);
	memcpy(body + prefixLen, data, dataLen);
	memcpy(body + prefixLen + dataLen, trailer, (size_t)trailerLen);

	char contentType[128];
	sprintf(contentType, "multipart/form-data; boundary=%s", boundary);

	httpPostBytes(url, body, bodyLen, contentType, nullptr, seed, logTag);

	free(body);
}

void UploadReplayToServer(const AsciiString& url, const void *data, unsigned int dataLen,
                          const AsciiString& filename, unsigned int seed)
{
	char nameBuf[256];
	sanitizeMultipartFilename(filename.isEmpty() ? nullptr : filename.str(), nameBuf, sizeof(nameBuf));
	httpPostMultipartFile(url, "file", nameBuf, data, dataLen, seed, "Replay upload");
}

void UploadMapToServer(const AsciiString& uploadUrl, const void *data, unsigned int dataLen,
                       unsigned int mapCRC, const AsciiString& mapName,
                       const char *fileKind, unsigned int seed)
{
	if (uploadUrl.isEmpty() || data == nullptr || dataLen == 0)
		return;

	// Build extra headers: X-Map-CRC, X-Map-Name, X-Map-File. Truncate the
	// map name to keep the header bounded in size.
	char extra[512];
	const char *name = mapName.isEmpty() ? "" : mapName.str();
	const char *kind = (fileKind != nullptr && fileKind[0] != '\0') ? fileKind : "map";
	sprintf(extra, "X-Map-CRC: %u\r\nX-Map-Name: %.255s\r\nX-Map-File: %.31s\r\n",
		mapCRC, name, kind);

	httpPostBytes(uploadUrl, data, dataLen, "application/octet-stream", extra, seed, "Map upload");
}

// ---------------------------------------------------------------------------
// Map existence check via HTTP GET.
// ---------------------------------------------------------------------------

// Lowercase an ASCII string in place (for case-insensitive body comparison).
static void lowerAscii(char *s)
{
	for (; *s != '\0'; ++s)
	{
		if (*s >= 'A' && *s <= 'Z')
			*s = static_cast<char>(*s + ('a' - 'A'));
	}
}

// ---------------------------------------------------------------------------
// Team balancing via HTTP GET.
// ---------------------------------------------------------------------------

// Percent-encode an ASCII byte sequence into a query-string fragment. Writes
// at most outCap bytes (always null-terminated). Reserved characters become
// %XX; the unreserved set per RFC 3986 passes through.
static void urlEncode(const char *src, char *out, unsigned int outCap)
{
	if (outCap == 0) return;
	static const char hex[] = "0123456789ABCDEF";
	unsigned int o = 0;
	for (const char *p = src; *p != '\0' && o + 4 < outCap; ++p)
	{
		unsigned char c = (unsigned char)*p;
		bool unreserved = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
		                  (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~';
		if (unreserved)
		{
			out[o++] = (char)c;
		}
		else
		{
			out[o++] = '%';
			out[o++] = hex[(c >> 4) & 0xF];
			out[o++] = hex[c & 0xF];
		}
	}
	out[o] = '\0';
}

// Extract the first JSON object key from a response body. Assumes keys do
// not contain backslash-escaped quotes (player names won't).
static bool extractFirstJsonKey(const char *body, AsciiString &outKey)
{
	const char *p = strchr(body, '"');
	if (p == nullptr) return false;
	++p;
	const char *end = strchr(p, '"');
	if (end == nullptr) return false;
	outKey.set(p, (int)(end - p));
	return !outKey.isEmpty();
}

BalanceTeamsResult BalanceTeamsFromServer(const AsciiString& url,
                                          const std::vector<AsciiString>& playerNames)
{
	BalanceTeamsResult result;
	result.success = false;

	if (url.isEmpty())
	{
		result.errorMessage = "Team balance URL is not set.";
		return result;
	}
	if (playerNames.empty())
	{
		result.errorMessage = "No players to balance.";
		return result;
	}

	// Parse URL once so we can rebuild the path with our query string.
	char hostBuf[256];
	char pathBuf[1024];
	URL_COMPONENTSA uc;
	memset(&uc, 0, sizeof(uc));
	uc.dwStructSize = sizeof(uc);
	uc.lpszHostName = hostBuf;
	uc.dwHostNameLength = sizeof(hostBuf);
	uc.lpszUrlPath = pathBuf;
	uc.dwUrlPathLength = sizeof(pathBuf);

	if (!InternetCrackUrlA(url.str(), 0, 0, &uc))
	{
		result.errorMessage = "Failed to parse balance-teams URL.";
		return result;
	}

	// Build full path with ?players=A&players=B&... appended (preserving any
	// existing query string in the original URL path).
	char fullPath[4096];
	int fullLen = (int)strlen(pathBuf);
	if (fullLen >= (int)sizeof(fullPath) - 1)
	{
		result.errorMessage = "Balance-teams URL path too long.";
		return result;
	}
	memcpy(fullPath, pathBuf, (size_t)fullLen);
	fullPath[fullLen] = '\0';
	bool hasQuery = (strchr(fullPath, '?') != nullptr);
	for (size_t i = 0; i < playerNames.size(); ++i)
	{
		char encoded[256];
		urlEncode(playerNames[i].str(), encoded, sizeof(encoded));
		const char *sep = hasQuery ? "&" : "?";
		int remaining = (int)sizeof(fullPath) - fullLen;
		int written = _snprintf(fullPath + fullLen, (size_t)remaining,
		                        "%splayers=%s", sep, encoded);
		if (written < 0 || written >= remaining)
		{
			result.errorMessage = "Too many players for balance-teams URL.";
			return result;
		}
		fullLen += written;
		hasQuery = true;
	}

	WinInetSession s;
	if (!openHttpRequest(url, "GET", fullPath, "Balance teams", &s))
	{
		result.errorMessage = "Could not connect to balance-teams server.";
		return result;
	}

	BOOL sent = HttpSendRequestA(s.hRequest, "Accept: application/json\r\n",
	                             (DWORD)-1L, nullptr, 0);
	if (!sent)
	{
		result.errorMessage.format("Balance-teams request failed (WinINet %lu).",
		                           GetLastError());
		closeHttpRequest(&s);
		return result;
	}

	DWORD statusCode = 0;
	DWORD statusSize = sizeof(statusCode);
	HttpQueryInfoA(s.hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
	               &statusCode, &statusSize, nullptr);
	if (statusCode < 200 || statusCode >= 300)
	{
		result.errorMessage.format("Balance-teams server returned HTTP %lu.",
		                           statusCode);
		closeHttpRequest(&s);
		return result;
	}

	// Read the response body up to a sensible cap (small JSON dict).
	static const DWORD bodyCap = 16 * 1024;
	char *body = (char *)malloc(bodyCap);
	if (body == nullptr)
	{
		result.errorMessage = "Out of memory reading balance-teams response.";
		closeHttpRequest(&s);
		return result;
	}
	DWORD totalRead = 0;
	for (;;)
	{
		DWORD bytesRead = 0;
		if (!InternetReadFile(s.hRequest, body + totalRead,
		                      bodyCap - 1 - totalRead, &bytesRead))
			break;
		if (bytesRead == 0) break;
		totalRead += bytesRead;
		if (totalRead >= bodyCap - 1) break;
	}
	body[totalRead] = '\0';
	closeHttpRequest(&s);

	AsciiString firstKey;
	bool gotKey = extractFirstJsonKey(body, firstKey);
	free(body);
	if (!gotKey)
	{
		result.errorMessage = "Balance-teams response was empty.";
		return result;
	}

	// Split the first key by commas. The API documents these as a plain
	// comma-separated list of names; trim incidental whitespace just in case.
	AsciiString current;
	for (const char *p = firstKey.str(); *p != '\0'; ++p)
	{
		if (*p == ',')
		{
			if (!current.isEmpty())
			{
				result.team1.push_back(current);
				current.clear();
			}
		}
		else if (*p != ' ' && *p != '\t')
		{
			current.concat(*p);
		}
	}
	if (!current.isEmpty())
		result.team1.push_back(current);

	if (result.team1.empty())
	{
		result.errorMessage = "Balance-teams response had no team-1 players.";
		return result;
	}

	result.success = true;
	return result;
}

bool MapMissingFromServer(const AsciiString& checkUrl, unsigned int mapCRC)
{
	if (checkUrl.isEmpty())
		return false;

	// Append ?crc=<hex> (or &crc=...) to the URL. We rebuild the path so
	// existing query strings are preserved.
	char hostBuf[256];
	char pathBuf[1024];
	URL_COMPONENTSA uc;
	memset(&uc, 0, sizeof(uc));
	uc.dwStructSize = sizeof(uc);
	uc.lpszHostName = hostBuf;
	uc.dwHostNameLength = sizeof(hostBuf);
	uc.lpszUrlPath = pathBuf;
	uc.dwUrlPathLength = sizeof(pathBuf);

	if (!InternetCrackUrlA(checkUrl.str(), 0, 0, &uc))
	{
		printf("Map check: failed to parse URL \"%s\"\n", checkUrl.str());
		return false;
	}

	const char *separator = (strchr(pathBuf, '?') != nullptr) ? "&" : "?";
	char fullPath[1280];
	sprintf(fullPath, "%s%scrc=%u", pathBuf, separator, mapCRC);

	WinInetSession s;
	if (!openHttpRequest(checkUrl, "GET", fullPath, "Map check", &s))
		return false;

	BOOL result = HttpSendRequestA(s.hRequest, nullptr, 0, nullptr, 0);
	if (!result)
	{
		printf("Map check: HttpSendRequest failed (%lu)\n", GetLastError());
		closeHttpRequest(&s);
		return false;
	}

	DWORD statusCode = 0;
	DWORD statusSize = sizeof(statusCode);
	HttpQueryInfoA(s.hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, nullptr);

	if (statusCode < 200 || statusCode >= 300)
	{
		printf("Map check: %s?crc=%u -> %lu (treating as already-have)\n", checkUrl.str(), mapCRC, statusCode);
		closeHttpRequest(&s);
		return false;
	}

	// Read up to 31 bytes of body — only need enough to hold "true" / "false"
	// with some slack for whitespace.
	char body[32];
	memset(body, 0, sizeof(body));
	DWORD totalRead = 0;
	for (;;)
	{
		DWORD bytesRead = 0;
		if (!InternetReadFile(s.hRequest, body + totalRead,
				static_cast<DWORD>(sizeof(body) - 1 - totalRead), &bytesRead))
			break;
		if (bytesRead == 0)
			break;
		totalRead += bytesRead;
		if (totalRead >= sizeof(body) - 1)
			break;
	}
	body[totalRead] = '\0';

	closeHttpRequest(&s);

	// Trim leading/trailing whitespace.
	char *start = body;
	while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')
		++start;
	char *end = start + strlen(start);
	while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n'))
		--end;
	*end = '\0';

	lowerAscii(start);
	const bool missing = (strcmp(start, "false") == 0);
	printf("Map check: crc=%u -> %lu, body=\"%s\", missing=%s\n",
		mapCRC, statusCode, start, missing ? "true" : "false");
	return missing;
}
