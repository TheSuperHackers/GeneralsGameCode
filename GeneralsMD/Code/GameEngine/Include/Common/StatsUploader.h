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

#pragma once

#include "Common/AsciiString.h"

#include <vector>

/// Upload gzip-compressed stats data to a REST endpoint via HTTP POST.
/// Sends Content-Type: application/gzip with the X-Game-Seed header.
/// @param url Full URL including path (e.g. "http://server:8080/stats")
/// @param data Pointer to gzip-compressed data
/// @param dataLen Length of compressed data in bytes
/// @param seed Game seed for the X-Game-Seed header
void UploadStatsToServer(const AsciiString& url, const void *data, unsigned int dataLen, unsigned int seed);

/// Upload a replay file to a REST endpoint via HTTP POST as
/// multipart/form-data. Sends the binary replay as the "file" part and
/// up to four optional identifier fields the radarvan server stores on
/// the ReplayFile row: mac_id (MAC of the adapter whose IPv4 matches
/// the user's LAN-IP option, falling back to the first enumerated
/// adapter when none matches), board_id (SMBIOS system UUID), the
/// supplied player_name (UTF-8 encoded), and client_version (from
/// TheVersion). Fields whose values cannot be determined are simply
/// omitted from the form.
/// @param url Full URL including path (e.g. "https://www.radarvan.com/api/upload_replay")
/// @param data Pointer to the replay file's raw bytes
/// @param dataLen Length of replay data in bytes
/// @param filename Filename to advertise in the multipart part (e.g. "00000000.rep")
/// @param seed Game seed for the X-Game-Seed header
/// @param playerName In-game name the uploader played under (UTF-8). Empty to omit.
void UploadReplayToServer(const AsciiString& url, const void *data, unsigned int dataLen,
                          const AsciiString& filename, unsigned int seed,
                          const AsciiString& playerName);

/// Allocate a new buffer that is the on-disk replay bytes followed by a
/// fixed "ZUTG" trailer (magic + version + payload length) used to mark
/// uploads sourced from the Zulu client. The on-disk file itself is
/// never touched; this only exists to tag the upload-buffer copy so the
/// server can distinguish Zulu uploads from third-party (e.g. gentool)
/// uploads of the same file. Caller frees the returned buffer with free().
/// Trailer layout (little-endian, 8 bytes total):
///   [0..3] magic            "ZUTG"
///   [4..5] version          0x0001
///   [6..7] payload length   0x0000 (v1 has no payload)
/// @param fileData Pointer to the raw on-disk replay bytes
/// @param fileLen Length of fileData in bytes
/// @param outLen On success, receives the length of the returned buffer
/// @return Newly malloc'd buffer of length fileLen + 8, or nullptr on failure
void *AppendZuluUploadTag(const void *fileData, unsigned int fileLen,
                          unsigned int *outLen);

/// Ask the server whether it already has the map identified by mapCRC.
/// Issues an HTTP GET to "<checkUrl>?crc=<hex>" and inspects the response
/// body. Returns true only if the server explicitly returns "false"
/// (case-insensitive, leading/trailing whitespace ignored). Network errors,
/// non-2xx responses, "true" responses, and an empty checkUrl all return
/// false (i.e. "don't bother uploading").
/// @param checkUrl Full URL of the existence-check endpoint
/// @param mapCRC The map's stored CRC (from MapMetaData::m_CRC)
bool MapMissingFromServer(const AsciiString& checkUrl, unsigned int mapCRC);

/// Upload a single map asset (the .map file or its .tga preview) to a REST
/// endpoint via HTTP POST. Sends Content-Type: application/octet-stream plus
/// X-Map-CRC, X-Map-Name, X-Map-File, and X-Game-Seed headers. Both calls
/// for the same map share the same X-Map-CRC so the server can group them.
/// @param uploadUrl Full URL of the map-upload endpoint
/// @param data Pointer to the raw asset bytes
/// @param dataLen Length of the asset data in bytes
/// @param mapCRC The map's CRC for the X-Map-CRC header
/// @param mapName The map's path/name for the X-Map-Name header
/// @param fileKind Identifier for X-Map-File ("map" or "preview")
/// @param seed Game seed for the X-Game-Seed header
void UploadMapToServer(const AsciiString& uploadUrl, const void *data, unsigned int dataLen,
                       unsigned int mapCRC, const AsciiString& mapName,
                       const char *fileKind, unsigned int seed);

/// Conditionally upload an entire map (the .map plus every sidecar the host
/// has on disk) to the configured cncstats endpoint. Issues one
/// missing-from-server check; if the server already has the CRC, no uploads
/// happen. Otherwise the .map file is uploaded along with whatever subset
/// of preview/.ini/.str/solo.ini/assetusage.txt/readme.txt the contentsMask
/// indicates are present.
///
/// Best-effort and blocking; intended for lobby map-select (host side) and
/// end-of-match upload (recorder side). Silently no-ops if either URL is
/// empty, the map path is empty, or the missing-from-server check fails.
///
/// @param checkUrl Full URL of the map_exists endpoint (empty disables)
/// @param uploadUrl Full URL of the add_map endpoint (empty disables)
/// @param mapCRC The map's CRC for the missing-from-server check and the
///        X-Map-CRC header on each upload
/// @param mapPath The .map file path (typically MapMetaData::m_fileName)
/// @param contentsMask GameInfo::getMapContentsMask() bitfield indicating
///        which sidecars exist on disk: bit 2 = preview, 4 = map.ini,
///        8 = map.str, 16 = solo.ini, 32 = assetusage.txt, 64 = readme.txt.
///        Pass 0 to upload the .map only.
/// @param seed Game seed for X-Game-Seed correlation
void UploadAllMapAssetsIfMissing(const AsciiString& checkUrl,
                                 const AsciiString& uploadUrl,
                                 unsigned int mapCRC,
                                 const AsciiString& mapPath,
                                 unsigned int contentsMask,
                                 unsigned int seed);

/// Download a single map asset (the .map file, its preview, or one sidecar)
/// from a REST endpoint via HTTP GET. Issues "<downloadUrl>?crc=<dec>&kind=<X>"
/// and returns the response body on HTTP 200. The caller takes ownership of
/// *outData and must release it with free().
///
/// 404 responses are treated as "asset not present" and return false without
/// allocating; this lets callers walk every kind they want and silently skip
/// the ones the server doesn't have. Non-2xx other than 404, network failures,
/// and empty bodies all return false.
///
/// @param downloadUrl Full URL of the get_map_file endpoint (empty disables)
/// @param mapCRC The map's CRC for the ?crc= query param
/// @param fileKind Identifier matching the server's X-Map-File kinds
///        ("map", "preview", "ini", "str", "solo", "assets", "readme")
/// @param maxBytes Hard cap on the response body size in bytes. Bodies that
///        report a larger Content-Length, or that overrun the cap mid-read,
///        return false. Set to 0 to use an internal default (16 MiB).
/// @param outData On success, receives a newly malloc'd buffer of the
///        response body. Untouched on failure.
/// @param outLen On success, receives the length of the buffer in bytes.
///        Untouched on failure.
/// @return true if the asset was fetched, false otherwise.
bool DownloadMapAssetFromServer(const AsciiString& downloadUrl,
                                unsigned int mapCRC,
                                const char *fileKind,
                                unsigned int maxBytes,
                                void **outData,
                                unsigned int *outLen);

/// Download a full map (the .map plus the sidecars listed in contentsMask)
/// from the configured cncstats endpoint and install it on disk at
/// localMapPath (with sidecars derived via the FileTransfer.h helpers).
/// Verifies the downloaded .map's CRC against the expected mapCRC before
/// committing it; rejects the entire install on CRC mismatch. On success,
/// refreshes MapCache so the new map shows up immediately in the lobby UI.
///
/// This is the registered MapDownloadHook implementation; it matches the
/// MapDownloadHookFn signature so it can be installed at engine init.
///
/// Returns true only on a CRC-verified .map write (sidecars are best-effort
/// and their failures don't gate the return value). Returns false if the
/// download URL is empty, the .map download fails, the CRC doesn't match,
/// the file write fails, or any required parameter is invalid.
Bool DownloadAndInstallMap(const AsciiString& localMapPath,
                           UnsignedInt mapCRC,
                           UnsignedInt contentsMask);

/// Result of a balance_teams API call.
struct BalanceTeamsResult
{
	bool success;
	AsciiString errorMessage;            ///< populated when !success
	std::vector<AsciiString> team1;      ///< populated when success: names assigned to team 1
	std::vector<AsciiString> allKnown;   ///< populated when success: every canonical name appearing in the response (across all keys). Lets the caller distinguish "slot did not match team1" from "slot was not recognized by the server at all"
};

/// Query the team-balancing endpoint with a list of player names. The endpoint
/// returns a JSON object whose keys are comma-separated player lists ranked
/// by balance score; the first key is treated as the best-balanced team-1
/// split. Players not in the first key become team 2.
/// Best-effort, blocking; uses WinINet's default timeouts.
/// @param url Full URL of the balance_teams endpoint
/// @param playerNames Lobby player names to balance (URL-encoded internally)
BalanceTeamsResult BalanceTeamsFromServer(const AsciiString& url,
                                          const std::vector<AsciiString>& playerNames);

/// One name + general (faction template index, as found in a replay header)
/// + team (lobby team index from GameSlot::getTeamNumber(); -1 = no team).
struct MapSummaryPlayer
{
	AsciiString name;
	int general;
	int team;
};

/// Result of a map_summary API call. `success` is true only when the server
/// returned HTTP 2xx. The body, if any, is split on '\n' (empty lines
/// dropped) and surfaced in `lines`.
struct MapSummaryResult
{
	bool success;
	std::vector<AsciiString> lines;
};

/// Ask the map_summary endpoint for a human-readable blurb describing recent
/// history on the given map for the given roster. POSTs a JSON body of the
/// form { "map_name": "...", "players": [{"name": "...", "general": <int>, "team": <int>}, ...] }.
/// Best-effort, blocking; uses WinINet's default timeouts.
/// @param url Full URL of the map_summary endpoint
/// @param mapName Display name of the map
/// @param players Roster (name + general index) to summarize
MapSummaryResult MapSummaryFromServer(const AsciiString& url,
                                      const AsciiString& mapName,
                                      const std::vector<MapSummaryPlayer>& players);

/// Result of a map_vote/<N>/choose API call.
struct ChooseMapResult
{
	bool success;
	AsciiString errorMessage;   ///< populated when !success
	AsciiString chosenMap;      ///< populated when success: human-readable map name (e.g. "Hostile Dawn")
	unsigned int mapCRC;        ///< populated when the server returns one; 0 means "not provided"
	AsciiString mapFileName;    ///< populated when the server returns one (relative .map path); empty means "not provided"
	unsigned int contentsMask;  ///< populated when the server returns one; 0 means "not provided"
};

/// Ask the map_vote/<playerCount>/choose endpoint to pick a map for the lobby.
/// POSTs JSON {"players":[...]} and parses chosen_map from the response.
/// The CRC / filename / contents_mask fields in the response are optional;
/// when present they enable an automatic on-demand CDN download via
/// DownloadAndInstallMap before the caller applies the map.
/// Best-effort, blocking; uses WinINet's default timeouts.
/// @param baseUrl Base URL of the map_vote endpoint, e.g.
///        "https://www.radarvan.com/api/map_vote/". The handler appends
///        "<playerCount>/choose" before sending.
/// @param playerNames Lobby player names to send in the JSON body.
/// @param playerCount Total occupied slots in the lobby (URL path segment).
ChooseMapResult ChooseMapFromServer(const AsciiString& baseUrl,
                                    const std::vector<AsciiString>& playerNames,
                                    unsigned int playerCount);

/// Refresh the cached map_match_counts table from the radarvan endpoint if
/// the last successful fetch is older than maxAgeSec, or if no fetch has
/// happened yet this session. Best-effort and blocking; uses WinINet's
/// default timeouts. Silently no-ops on empty URL or HTTP failure (in which
/// case the previously cached table, if any, stays in place).
/// @param url Full URL of the map_match_counts endpoint
/// @param maxAgeSec How recent a cached snapshot must be to skip refetching
void FetchMapMatchCountsIfStale(const AsciiString& url, unsigned int maxAgeSec);

/// Look up how many recorded matches the radarvan server has for the map
/// identified by mapCacheKey (a MapCache iterator key, i.e. the full
/// lowercased filename with backslashes). The cache key is normalized to
/// the radarvan API's "maps/<folder>" or "userdata/maps/<folder>" form
/// before lookup. Returns 0 when no entry exists (no fetch yet, key
/// unrecognized, or simply zero plays).
int GetMapMatchCount(const AsciiString& mapCacheKey);
