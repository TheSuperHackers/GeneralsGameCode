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

class AsciiString;

/// Upload gzip-compressed stats data to a REST endpoint via HTTP POST.
/// Sends Content-Type: application/gzip with the X-Game-Seed header.
/// @param url Full URL including path (e.g. "http://server:8080/stats")
/// @param data Pointer to gzip-compressed data
/// @param dataLen Length of compressed data in bytes
/// @param seed Game seed for the X-Game-Seed header
void UploadStatsToServer(const AsciiString& url, const void *data, unsigned int dataLen, unsigned int seed);

/// Upload a replay file to a REST endpoint via HTTP POST.
/// Sends Content-Type: application/octet-stream with the X-Game-Seed header.
/// @param url Full URL including path (e.g. "https://www.radarvan.com/api/upload_replay")
/// @param data Pointer to the replay file's raw bytes
/// @param dataLen Length of replay data in bytes
/// @param seed Game seed for the X-Game-Seed header
void UploadReplayToServer(const AsciiString& url, const void *data, unsigned int dataLen, unsigned int seed);

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
