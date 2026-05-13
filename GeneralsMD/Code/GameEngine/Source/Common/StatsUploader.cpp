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
#include "Common/OptionPreferences.h"
#include "Common/version.h"
#include "ZuluClientKey.h"

#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "wininet.lib")

// VC6's bundled Platform SDK predates iphlpapi.h and the
// GetSystemFirmwareTable declaration in winbase.h, so we resolve both
// APIs at runtime via LoadLibrary/GetProcAddress and forward-declare just
// the structs we touch. These layouts have been stable across every
// Windows SDK that ships them (Win2000+ for iphlpapi, XP SP2+ for
// GetSystemFirmwareTable).

#define LOCAL_MAX_ADAPTER_NAME_LENGTH        256
#define LOCAL_MAX_ADAPTER_DESCRIPTION_LENGTH 128
#define LOCAL_MAX_ADAPTER_ADDRESS_LENGTH     8

struct LocalIpAddressString
{
	char String[16];
};

struct LocalIpAddrString
{
	LocalIpAddrString *Next;
	LocalIpAddressString IpAddress;
	LocalIpAddressString IpMask;
	DWORD Context;
};

struct LocalIpAdapterInfo
{
	LocalIpAdapterInfo *Next;
	DWORD ComboIndex;
	char AdapterName[LOCAL_MAX_ADAPTER_NAME_LENGTH + 4];
	char Description[LOCAL_MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
	UINT AddressLength;
	BYTE Address[LOCAL_MAX_ADAPTER_ADDRESS_LENGTH];
	DWORD Index;
	UINT Type;
	UINT DhcpEnabled;
	LocalIpAddrString *CurrentIpAddress;
	LocalIpAddrString IpAddressList;
	LocalIpAddrString GatewayList;
	LocalIpAddrString DhcpServer;
	BOOL HaveWins;
	LocalIpAddrString PrimaryWinsServer;
	LocalIpAddrString SecondaryWinsServer;
	DWORD LeaseObtained;
	DWORD LeaseExpires;
};

typedef DWORD (WINAPI *FnGetAdaptersInfo)(LocalIpAdapterInfo *, ULONG *);
typedef UINT (WINAPI *FnGetSystemFirmwareTable)(DWORD, DWORD, PVOID, DWORD);

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

	// Inject the build-time radarvan auth key as an X-API-Key header so
	// every caller of openHttpRequest picks it up. ZULU_CLIENT_KEY is
	// baked in from GCP Secret Manager (secret zuluclientkey) by
	// cmake/zuluclientkey.cmake; configure fails if the fetch fails, so
	// the macro is guaranteed non-empty here.
	{
		char authHeader[512];
		int authLen = sprintf(authHeader, "X-API-Key: %s\r\n", ZULU_CLIENT_KEY);
		if (authLen > 0)
			HttpAddRequestHeadersA(out->hRequest, authHeader, (DWORD)authLen,
				HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
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

// One text part (form field) in a multipart/form-data body. Empty `value`
// signals "omit this field"; the caller is expected to skip it. The `value`
// bytes are emitted verbatim, so non-ASCII (e.g. UTF-8 player names) is the
// caller's responsibility.
struct MultipartTextField
{
	const char *name;
	AsciiString value;
};

// Issue an HTTP POST as multipart/form-data with one binary part plus any
// number of text parts. Skips any text field whose value is empty so we
// don't bake a sentinel into the form.
static void httpPostMultipartFile(const AsciiString& url,
                                  const char *fieldName,
                                  const char *filename,
                                  const void *data,
                                  unsigned int dataLen,
                                  const MultipartTextField *textFields,
                                  unsigned int textFieldCount,
                                  unsigned int seed,
                                  const char *logTag)
{
	if (data == nullptr || dataLen == 0)
		return;

	// Boundary: long enough that random collision with binary file bytes
	// is negligible. Must not appear inside the file payload preceded by
	// "\r\n--".
	static const char boundary[] = "----GeneralsReplayBoundaryK8nQv2pXr9TfH3";

	// Build the text-field block first so we know its size up front. Each
	// part is "--<boundary>\r\nContent-Disposition: form-data; name=\"X\"\r\n\r\n<value>\r\n".
	AsciiString textBlock;
	unsigned int ti;
	for (ti = 0; ti < textFieldCount; ++ti)
	{
		const MultipartTextField &tf = textFields[ti];
		if (tf.name == nullptr || tf.name[0] == '\0' || tf.value.isEmpty())
			continue;
		char header[256];
		sprintf(header,
			"--%s\r\n"
			"Content-Disposition: form-data; name=\"%.63s\"\r\n"
			"\r\n",
			boundary, tf.name);
		textBlock.concat(header);
		textBlock.concat(tf.value);
		textBlock.concat("\r\n");
	}

	char filePrefix[512];
	int filePrefixLen = sprintf(filePrefix,
		"--%s\r\n"
		"Content-Disposition: form-data; name=\"%.63s\"; filename=\"%.255s\"\r\n"
		"Content-Type: application/octet-stream\r\n"
		"\r\n",
		boundary, fieldName, filename);

	char trailer[64];
	int trailerLen = sprintf(trailer, "\r\n--%s--\r\n", boundary);

	if (filePrefixLen <= 0 || trailerLen <= 0)
		return;

	unsigned int textLen = (unsigned int)textBlock.getLength();
	unsigned int bodyLen = textLen + (unsigned int)filePrefixLen + dataLen + (unsigned int)trailerLen;
	char *body = (char *)malloc(bodyLen);
	if (body == nullptr)
		return;
	unsigned int pos = 0;
	if (textLen > 0)
	{
		memcpy(body, textBlock.str(), (size_t)textLen);
		pos += textLen;
	}
	memcpy(body + pos, filePrefix, (size_t)filePrefixLen);
	pos += (unsigned int)filePrefixLen;
	memcpy(body + pos, data, dataLen);
	pos += dataLen;
	memcpy(body + pos, trailer, (size_t)trailerLen);

	char contentType[128];
	sprintf(contentType, "multipart/form-data; boundary=%s", boundary);

	httpPostBytes(url, body, bodyLen, contentType, nullptr, seed, logTag);

	free(body);
}

// ---------------------------------------------------------------------------
// Per-machine identifiers: MAC of the LAN-IP-tied adapter, and SMBIOS UUID.
// Both are best-effort and quietly produce an empty string on failure so the
// caller can just omit the form field.
// ---------------------------------------------------------------------------

// Format the first six bytes of `mac` as 12 uppercase hex chars with no
// separators. Matches gentool's `5211058E5C33`-style display.
static void macBytesToHex(const unsigned char *mac, AsciiString &out)
{
	static const char hex[] = "0123456789ABCDEF";
	char buf[13];
	int b;
	for (b = 0; b < 6; ++b)
	{
		buf[b * 2 + 0] = hex[(mac[b] >> 4) & 0xF];
		buf[b * 2 + 1] = hex[mac[b] & 0xF];
	}
	buf[12] = '\0';
	out = buf;
}

// Format a host-byte-order IPv4 address as dotted decimal "A.B.C.D" so it
// can be compared against the dotted strings GetAdaptersInfo returns.
static void formatIpv4Dotted(UnsignedInt ipHostOrder, char *out, unsigned int outCap)
{
	if (outCap == 0)
		return;
	unsigned int a = (ipHostOrder >> 24) & 0xFF;
	unsigned int b = (ipHostOrder >> 16) & 0xFF;
	unsigned int c = (ipHostOrder >>  8) & 0xFF;
	unsigned int d = ipHostOrder & 0xFF;
	_snprintf(out, outCap, "%u.%u.%u.%u", a, b, c, d);
	out[outCap - 1] = '\0';
}

// Enumerate Windows IPv4 adapters and return the MAC of the adapter whose
// IPv4 list contains `wantIpHostOrder`. If no adapter matches (or
// `wantIpHostOrder` is 0), returns the MAC of the first enumerated adapter
// that has a 6-byte physical address. This mirrors gentool's fallback so a
// Zulu client without an explicit LAN-IP selection still emits the same
// identifier gentool would have.
static AsciiString getLocalMacIdHex(UnsignedInt wantIpHostOrder)
{
	AsciiString result;

	HMODULE hMod = LoadLibraryA("iphlpapi.dll");
	if (hMod == nullptr)
		return result;
	FnGetAdaptersInfo getAdaptersInfo = (FnGetAdaptersInfo)GetProcAddress(hMod, "GetAdaptersInfo");
	if (getAdaptersInfo == nullptr)
	{
		FreeLibrary(hMod);
		return result;
	}

	ULONG bufLen = 16 * 1024;
	LocalIpAdapterInfo *info = (LocalIpAdapterInfo *)malloc(bufLen);
	if (info == nullptr)
	{
		FreeLibrary(hMod);
		return result;
	}

	DWORD status = getAdaptersInfo(info, &bufLen);
	if (status == ERROR_BUFFER_OVERFLOW)
	{
		free(info);
		info = (LocalIpAdapterInfo *)malloc(bufLen);
		if (info == nullptr)
		{
			FreeLibrary(hMod);
			return result;
		}
		status = getAdaptersInfo(info, &bufLen);
	}
	if (status != ERROR_SUCCESS)
	{
		free(info);
		FreeLibrary(hMod);
		return result;
	}

	char wantDotted[16];
	wantDotted[0] = '\0';
	if (wantIpHostOrder != 0)
		formatIpv4Dotted(wantIpHostOrder, wantDotted, sizeof(wantDotted));

	AsciiString firstMac;
	LocalIpAdapterInfo *p;
	for (p = info; p != nullptr; p = p->Next)
	{
		if (p->AddressLength < 6)
			continue;

		if (firstMac.isEmpty())
			macBytesToHex(p->Address, firstMac);

		if (wantDotted[0] != '\0')
		{
			LocalIpAddrString *ip;
			for (ip = &p->IpAddressList; ip != nullptr; ip = ip->Next)
			{
				if (strcmp(ip->IpAddress.String, wantDotted) == 0)
				{
					macBytesToHex(p->Address, result);
					free(info);
					FreeLibrary(hMod);
					return result;
				}
			}
		}
	}

	free(info);
	FreeLibrary(hMod);
	if (result.isEmpty())
		result = firstMac;
	return result;
}

// Read the system SMBIOS table via GetSystemFirmwareTable('RSMB', ...) and
// return the type 1 (System Information) UUID as a canonical dashed string
// "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX". On SMBIOS 2.6+ the first three
// fields are stored little-endian and are byte-swapped here to match what
// `wmic csproduct get UUID` shows. Returns empty on failure or all-zero/all-
// FF UUID (the latter being SMBIOS's "not present" sentinel).
static AsciiString getBoardIdDashed()
{
	AsciiString result;

	HMODULE hMod = GetModuleHandleA("kernel32.dll");
	if (hMod == nullptr)
		return result;
	FnGetSystemFirmwareTable getSysFwTable =
		(FnGetSystemFirmwareTable)GetProcAddress(hMod, "GetSystemFirmwareTable");
	if (getSysFwTable == nullptr)
		return result; // pre-XP-SP2 host; quietly skip.

	const DWORD provider = 'RSMB'; // big-endian 'R','S','M','B'
	DWORD bufSize = getSysFwTable(provider, 0, nullptr, 0);
	if (bufSize == 0)
		return result;

	BYTE *buf = (BYTE *)malloc(bufSize);
	if (buf == nullptr)
		return result;

	DWORD got = getSysFwTable(provider, 0, buf, bufSize);
	if (got == 0 || got > bufSize)
	{
		free(buf);
		return result;
	}

	// RawSMBIOSData header: Used20CallingMethod, MajorVer, MinorVer,
	// DmiRevision, Length (DWORD), then SMBIOSTableData[Length].
	if (got < 8)
	{
		free(buf);
		return result;
	}
	BYTE majorVer = buf[1];
	BYTE minorVer = buf[2];
	DWORD tableLen = *(DWORD *)(buf + 4);
	if (tableLen + 8 > got)
		tableLen = got - 8;
	BYTE *table = buf + 8;

	const bool littleEndianFields = (majorVer > 2) || (majorVer == 2 && minorVer >= 6);

	BYTE *cur = table;
	BYTE *end = table + tableLen;
	while (cur + 4 <= end)
	{
		BYTE type = cur[0];
		BYTE structLen = cur[1];
		if (structLen < 4 || cur + structLen > end)
			break;

		if (type == 1 && structLen >= 0x18)
		{
			// UUID is 16 bytes at offset 8 of the formatted area.
			const BYTE *u = cur + 8;

			// Detect "not present" sentinels per the SMBIOS spec.
			bool allZero = true;
			bool allFF = true;
			int i;
			for (i = 0; i < 16; ++i)
			{
				if (u[i] != 0x00) allZero = false;
				if (u[i] != 0xFF) allFF = false;
			}
			if (!allZero && !allFF)
			{
				BYTE swapped[16];
				if (littleEndianFields)
				{
					swapped[0] = u[3]; swapped[1] = u[2]; swapped[2] = u[1]; swapped[3] = u[0];
					swapped[4] = u[5]; swapped[5] = u[4];
					swapped[6] = u[7]; swapped[7] = u[6];
					memcpy(swapped + 8, u + 8, 8);
				}
				else
				{
					memcpy(swapped, u, 16);
				}

				char outBuf[40];
				sprintf(outBuf,
					"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
					swapped[0], swapped[1], swapped[2], swapped[3],
					swapped[4], swapped[5],
					swapped[6], swapped[7],
					swapped[8], swapped[9],
					swapped[10], swapped[11], swapped[12], swapped[13], swapped[14], swapped[15]);
				result = outBuf;
			}
			break;
		}

		// Skip the formatted area, then walk the trailing string set:
		// strings are null-terminated; the set ends with a double null.
		BYTE *next = cur + structLen;
		if (next >= end) break;
		while (next < end - 1 && !(next[0] == 0 && next[1] == 0))
			++next;
		next += 2;
		if (next <= cur) break;
		cur = next;
	}

	free(buf);
	return result;
}

void UploadReplayToServer(const AsciiString& url, const void *data, unsigned int dataLen,
                          const AsciiString& filename, unsigned int seed,
                          const AsciiString& playerName)
{
	char nameBuf[256];
	sanitizeMultipartFilename(filename.isEmpty() ? nullptr : filename.str(), nameBuf, sizeof(nameBuf));

	// Populate the optional identifier fields. Each helper returns an empty
	// AsciiString on failure; httpPostMultipartFile drops empty fields.
	OptionPreferences prefs;
	UnsignedInt lanIp = prefs.getLANIPAddress();

	MultipartTextField fields[4];
	fields[0].name = "mac_id";
	fields[0].value = getLocalMacIdHex(lanIp);
	fields[1].name = "board_id";
	fields[1].value = getBoardIdDashed();
	fields[2].name = "player_name";
	fields[2].value = playerName;
	fields[3].name = "client_version";
	fields[3].value = (TheVersion != nullptr) ? TheVersion->getAsciiVersion() : AsciiString();

	httpPostMultipartFile(url, "file", nameBuf, data, dataLen,
		fields, 4, seed, "Replay upload");
}

void *AppendZuluUploadTag(const void *fileData, unsigned int fileLen,
                          unsigned int *outLen)
{
	static const unsigned char tag[8] = {
		'Z', 'U', 'T', 'G',
		0x01, 0x00,  // version 1, little-endian
		0x00, 0x00,  // payload length 0
	};
	if (outLen != nullptr)
		*outLen = 0;
	if (fileData == nullptr || fileLen == 0)
		return nullptr;
	unsigned int newLen = fileLen + (unsigned int)sizeof(tag);
	void *buf = malloc(newLen);
	if (buf == nullptr)
		return nullptr;
	memcpy(buf, fileData, fileLen);
	memcpy((char *)buf + fileLen, tag, sizeof(tag));
	if (outLen != nullptr)
		*outLen = newLen;
	return buf;
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

// Extract every top-level key from a flat JSON object. The balance-teams
// response is a dict whose values are floats, so every quoted token at the
// object's top level is a key.
static void extractAllJsonKeys(const char *body, std::vector<AsciiString> &outKeys)
{
	const char *p = body;
	while (*p && *p != '{') ++p;
	if (!*p) return;
	++p;

	while (*p)
	{
		while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',')
			++p;
		if (*p == '}' || !*p) break;
		if (*p != '"') break; // malformed; bail
		++p;
		const char *end = strchr(p, '"');
		if (!end) break;
		AsciiString k;
		k.set(p, (int)(end - p));
		if (!k.isEmpty())
			outKeys.push_back(k);
		p = end + 1;
		// Skip the value (which is a number or null in the balance-teams API).
		while (*p && *p != ',' && *p != '}') ++p;
	}
}

// Split a comma-separated key like "Pancake,OneThree111" into individual
// canonical names, trimming whitespace.
static void splitCommaList(const AsciiString &key, std::vector<AsciiString> &out)
{
	AsciiString current;
	for (const char *p = key.str(); *p != '\0'; ++p)
	{
		if (*p == ',')
		{
			if (!current.isEmpty())
			{
				out.push_back(current);
				current.clear();
			}
		}
		else if (*p != ' ' && *p != '\t')
		{
			current.concat(*p);
		}
	}
	if (!current.isEmpty())
		out.push_back(current);
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

	std::vector<AsciiString> keys;
	extractAllJsonKeys(body, keys);
	free(body);
	if (keys.empty())
	{
		result.errorMessage = "Balance-teams response was empty.";
		return result;
	}

	// First key is the best-balanced split; the names there are team 1.
	splitCommaList(keys[0], result.team1);
	if (result.team1.empty())
	{
		result.errorMessage = "Balance-teams response had no team-1 players.";
		return result;
	}

	// Union every name appearing in any key. The server fuzzy-resolves the
	// names we sent (so "Pan" comes back as "Pancake") and silently drops names
	// it doesn't recognize. allKnown lets the caller tell "slot belongs to
	// team 2" apart from "the server didn't recognize this slot at all".
	size_t k;
	for (k = 0; k < keys.size(); ++k)
	{
		std::vector<AsciiString> names;
		splitCommaList(keys[k], names);
		size_t n;
		for (n = 0; n < names.size(); ++n)
		{
			Bool dup = FALSE;
			size_t a;
			for (a = 0; a < result.allKnown.size(); ++a)
			{
				if (result.allKnown[a].compareNoCase(names[n]) == 0)
				{
					dup = TRUE;
					break;
				}
			}
			if (!dup)
				result.allKnown.push_back(names[n]);
		}
	}

	result.success = true;
	return result;
}

// ---------------------------------------------------------------------------
// Map summary blurb via HTTP POST.
// ---------------------------------------------------------------------------

// Append a JSON-escaped form of `src` to `out`. Escapes the characters that
// would otherwise break out of a JSON string: backslash, double-quote, and
// control characters. UTF-8 byte sequences pass through untouched.
static void appendJsonEscaped(AsciiString &out, const char *src)
{
	for (const char *p = src; *p != '\0'; ++p)
	{
		unsigned char c = (unsigned char)*p;
		switch (c)
		{
			case '"':  out.concat("\\\""); break;
			case '\\': out.concat("\\\\"); break;
			case '\b': out.concat("\\b");  break;
			case '\f': out.concat("\\f");  break;
			case '\n': out.concat("\\n");  break;
			case '\r': out.concat("\\r");  break;
			case '\t': out.concat("\\t");  break;
			default:
				if (c < 0x20)
				{
					char esc[8];
					sprintf(esc, "\\u%04X", c);
					out.concat(esc);
				}
				else
				{
					out.concat((char)c);
				}
				break;
		}
	}
}

MapSummaryResult MapSummaryFromServer(const AsciiString& url,
                                      const AsciiString& mapName,
                                      const std::vector<MapSummaryPlayer>& players)
{
	MapSummaryResult result;
	result.success = false;

	if (url.isEmpty())
		return result;

	// Build the JSON body: { "map_name": "...", "players": [{"name":"...","general":N,"team":T}, ...] }
	AsciiString body;
	body.concat("{\"map_name\":\"");
	appendJsonEscaped(body, mapName.isEmpty() ? "" : mapName.str());
	body.concat("\",\"players\":[");
	size_t i;
	for (i = 0; i < players.size(); ++i)
	{
		if (i > 0)
			body.concat(',');
		body.concat("{\"name\":\"");
		appendJsonEscaped(body, players[i].name.isEmpty() ? "" : players[i].name.str());
		char gbuf[64];
		sprintf(gbuf, "\",\"general\":%d,\"team\":%d}", players[i].general, players[i].team);
		body.concat(gbuf);
	}
	body.concat("]}");

	WinInetSession s;
	if (!openHttpRequest(url, "POST", nullptr, "Map summary", &s))
		return result;

	const char *headers = "Content-Type: application/json\r\nAccept: application/json\r\n";
	BOOL sent = HttpSendRequestA(s.hRequest, headers, (DWORD)strlen(headers),
	                             const_cast<char*>(body.str()), (DWORD)body.getLength());
	if (!sent)
	{
		printf("Map summary: HttpSendRequest failed (%lu)\n", GetLastError());
		closeHttpRequest(&s);
		return result;
	}

	DWORD statusCode = 0;
	DWORD statusSize = sizeof(statusCode);
	HttpQueryInfoA(s.hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
	               &statusCode, &statusSize, nullptr);
	if (statusCode < 200 || statusCode >= 300)
	{
		printf("Map summary: %s -> %lu\n", url.str(), statusCode);
		closeHttpRequest(&s);
		return result;
	}

	// Read the response body. The server returns a JSON string (a single
	// quoted, possibly newline-bearing token). Cap at 16 KiB.
	static const DWORD bodyCap = 16 * 1024;
	char *resp = (char *)malloc(bodyCap);
	if (resp == nullptr)
	{
		closeHttpRequest(&s);
		return result;
	}
	DWORD totalRead = 0;
	for (;;)
	{
		DWORD bytesRead = 0;
		if (!InternetReadFile(s.hRequest, resp + totalRead,
		                      bodyCap - 1 - totalRead, &bytesRead))
			break;
		if (bytesRead == 0) break;
		totalRead += bytesRead;
		if (totalRead >= bodyCap - 1) break;
	}
	resp[totalRead] = '\0';
	closeHttpRequest(&s);

	printf("Map summary: %s -> %lu\n", url.str(), statusCode);

	// Strip a single layer of surrounding double-quotes if present (the
	// endpoint returns a bare JSON string), and unescape \n / \" / \\ so
	// the caller can split the text on real newlines.
	const char *p = resp;
	while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
		++p;
	const char *end = resp + totalRead;
	while (end > p && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n'))
		--end;
	if (end > p && *p == '"' && end[-1] == '"')
	{
		++p;
		--end;
	}

	AsciiString current;
	while (p < end)
	{
		char c = *p++;
		if (c == '\\' && p < end)
		{
			char n = *p++;
			switch (n)
			{
				case 'n':
					if (!current.isEmpty())
					{
						result.lines.push_back(current);
						current.clear();
					}
					break;
				case 'r':                          break;
				case 't':  current.concat('\t');   break;
				case '"':  current.concat('"');    break;
				case '\\': current.concat('\\');   break;
				case '/':  current.concat('/');    break;
				default:   current.concat(n);      break;
			}
		}
		else if (c == '\n')
		{
			if (!current.isEmpty())
			{
				result.lines.push_back(current);
				current.clear();
			}
		}
		else if (c != '\r')
		{
			current.concat(c);
		}
	}
	if (!current.isEmpty())
		result.lines.push_back(current);

	free(resp);
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
