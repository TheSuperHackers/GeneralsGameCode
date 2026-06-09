/*
**	Command & Conquer Generals(tm)
**	Copyright 2026 TheSuperHackers
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

// TheSuperHackers @feature bill-rich 10/03/2026 Game stats JSON exporter.

#pragma once

class AsciiString;

/// Export game statistics as a JSON file alongside the replay file.
/// @param replayDir Directory containing replays (e.g. "[UserDataPath]/Replays/")
/// @param replayFileName Replay filename with extension (e.g. "LastReplay.rep")
void ExportGameStatsJSON(const AsciiString& replayDir, const AsciiString& replayFileName);

/// Collect a time-series snapshot of all players' stats (called every game logic frame).
/// Snapshots are taken every 30 frames (~1 second) and stored in memory.
void StatsExporterCollectSnapshot();

/// Clear all stored time-series snapshots (called at game start/reset).
void StatsExporterClearSnapshots();
