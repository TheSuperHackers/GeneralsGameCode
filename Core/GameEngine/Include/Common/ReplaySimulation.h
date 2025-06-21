/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

class ReplaySimulation
{
public:

	// TheSuperHackers @feature helmutbuhler 13/04/2025
	// Simulate a list of replays without graphics.
	// Returns exit code 1 if mismatch or other error occurred
	// Returns exit code 0 if all replays were successfully simulated without mismatches
	static int simulateReplays(const std::vector<AsciiString> &filenames, int maxProcesses);

	static void stop() { s_isRunning = false; }

	static Bool isRunning() { return s_isRunning; }
	static UnsignedInt getCurrentReplayIndex() { return s_replayIndex; }
	static UnsignedInt getReplayCount() { return s_replayCount; }

private:

	static int simulateReplaysInThisProcess(const std::vector<AsciiString> &filenames);
	static int simulateReplaysInWorkerProcesses(const std::vector<AsciiString> &filenames, int maxProcesses);
	static std::vector<AsciiString> resolveFilenameWildcards(const std::vector<AsciiString> &filenames);

private:

	static Bool s_isRunning;
	static UnsignedInt s_replayIndex;
	static UnsignedInt s_replayCount;
};
