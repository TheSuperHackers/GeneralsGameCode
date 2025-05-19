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

// Helper class that allows you to start a worker process and retrieve its exit code
// and console output as a string.
// It also makes sure that the started process is killed in case our process exits in any way.
class WorkerProcess
{
public:
	WorkerProcess();

	bool StartProcess(UnicodeString command);

	bool IsRunning() const;

	// returns true iff the process exited.
	// Sets the parameters if return value is true.
	bool IsDone(DWORD *exitcode, AsciiString *stdOutput) const;
	void Update();

	// Terminate Process if it's running
	void Kill();

private:
	HANDLE m_processHandle;
	HANDLE m_readHandle;
	HANDLE m_jobHandle;
	AsciiString m_stdOutput;
	DWORD m_exitcode;
	bool m_isDone;
};
