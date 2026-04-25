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
/// @param url Full URL including path (e.g. "http://server:8080/stats")
/// @param data Pointer to gzip-compressed data
/// @param dataLen Length of compressed data in bytes
/// @param seed Game seed for the X-Game-Seed header
void UploadStatsToServer(const AsciiString& url, const void *data, unsigned int dataLen, unsigned int seed);
