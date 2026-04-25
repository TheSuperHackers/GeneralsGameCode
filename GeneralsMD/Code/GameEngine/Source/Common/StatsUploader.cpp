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

void UploadStatsToServer(const AsciiString& url, const void *data, unsigned int dataLen, unsigned int seed)
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
		printf("Stats upload: failed to parse URL \"%s\"\n", url.str());
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
		printf("Stats upload: InternetOpen failed (%lu)\n", GetLastError());
		return;
	}

	HINTERNET hConnect = InternetConnectA(hInternet, hostBuf, port, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
	if (hConnect == nullptr)
	{
		printf("Stats upload: InternetConnect failed (%lu)\n", GetLastError());
		InternetCloseHandle(hInternet);
		return;
	}

	HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", pathBuf, nullptr, nullptr, nullptr, flags, 0);
	if (hRequest == nullptr)
	{
		printf("Stats upload: HttpOpenRequest failed (%lu)\n", GetLastError());
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return;
	}

	// Build headers
	char headers[512];
	sprintf(headers, "Content-Type: application/gzip\r\nX-Game-Seed: %u\r\n", seed);

	BOOL result = HttpSendRequestA(hRequest, headers, (DWORD)strlen(headers), const_cast<void*>(data), dataLen);

	if (result)
	{
		DWORD statusCode = 0;
		DWORD statusSize = sizeof(statusCode);
		HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, nullptr);
		printf("Stats upload: %s -> %lu\n", url.str(), statusCode);
	}
	else
	{
		printf("Stats upload: HttpSendRequest failed (%lu)\n", GetLastError());
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
}
