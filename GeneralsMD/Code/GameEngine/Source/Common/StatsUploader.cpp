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

#pragma comment(lib, "wininet.lib")

// Shared HTTP POST helper. Posts arbitrary bytes to a URL with the given
// Content-Type and an X-Game-Seed header. Logs status to stdout; does not
// propagate errors to the caller (game-end uploads are best-effort).
static void httpPostBytes(const AsciiString& url,
                          const void *data,
                          unsigned int dataLen,
                          unsigned int seed,
                          const char *contentType,
                          const char *logTag)
{
	if (url.isEmpty() || data == nullptr || dataLen == 0)
		return;

	// Parse URL components
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
		printf("%s upload: failed to parse URL \"%s\"\n", logTag, url.str());
		return;
	}

	INTERNET_PORT port = uc.nPort;
	if (port == 0)
		port = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

	DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
	if (uc.nScheme == INTERNET_SCHEME_HTTPS)
		flags |= INTERNET_FLAG_SECURE;

	HINTERNET hInternet = InternetOpenA("GeneralsStatsExporter/1.0", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	if (hInternet == nullptr)
	{
		printf("%s upload: InternetOpen failed (%lu)\n", logTag, GetLastError());
		return;
	}

	HINTERNET hConnect = InternetConnectA(hInternet, hostBuf, port, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
	if (hConnect == nullptr)
	{
		printf("%s upload: InternetConnect failed (%lu)\n", logTag, GetLastError());
		InternetCloseHandle(hInternet);
		return;
	}

	HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", pathBuf, nullptr, nullptr, nullptr, flags, 0);
	if (hRequest == nullptr)
	{
		printf("%s upload: HttpOpenRequest failed (%lu)\n", logTag, GetLastError());
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return;
	}

	// Build headers
	char headers[512];
	sprintf(headers, "Content-Type: %s\r\nX-Game-Seed: %u\r\n", contentType, seed);

	BOOL result = HttpSendRequestA(hRequest, headers, (DWORD)strlen(headers), const_cast<void*>(data), dataLen);

	if (result)
	{
		DWORD statusCode = 0;
		DWORD statusSize = sizeof(statusCode);
		HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, nullptr);
		printf("%s upload: %s -> %lu\n", logTag, url.str(), statusCode);
	}
	else
	{
		printf("%s upload: HttpSendRequest failed (%lu)\n", logTag, GetLastError());
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
}

void UploadStatsToServer(const AsciiString& url, const void *data, unsigned int dataLen, unsigned int seed)
{
	httpPostBytes(url, data, dataLen, seed, "application/gzip", "Stats");
}

void UploadReplayToServer(const AsciiString& url, const void *data, unsigned int dataLen, unsigned int seed)
{
	httpPostBytes(url, data, dataLen, seed, "application/octet-stream", "Replay");
}
