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

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/WorkerProcess.h"

// We need Job-related functions, but these aren't defined in the Windows-headers that VC6 uses.
// So we define them here and load them dynamically.
#if defined(_MSC_VER) && _MSC_VER < 1300
struct JOBOBJECT_BASIC_LIMIT_INFORMATION2
{
	LARGE_INTEGER PerProcessUserTimeLimit;
	LARGE_INTEGER PerJobUserTimeLimit;
	DWORD LimitFlags;
	SIZE_T MinimumWorkingSetSize;
	SIZE_T MaximumWorkingSetSize;
	DWORD ActiveProcessLimit;
	ULONG_PTR Affinity;
	DWORD PriorityClass;
	DWORD SchedulingClass;
};
struct IO_COUNTERS
{
	ULONGLONG ReadOperationCount;
	ULONGLONG WriteOperationCount;
	ULONGLONG OtherOperationCount;
	ULONGLONG ReadTransferCount;
	ULONGLONG WriteTransferCount;
	ULONGLONG OtherTransferCount;
};
struct JOBOBJECT_EXTENDED_LIMIT_INFORMATION
{
	JOBOBJECT_BASIC_LIMIT_INFORMATION2 BasicLimitInformation;
	IO_COUNTERS IoInfo;
	SIZE_T ProcessMemoryLimit;
	SIZE_T JobMemoryLimit;
	SIZE_T PeakProcessMemoryUsed;
	SIZE_T PeakJobMemoryUsed;
};

#define JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE 0x00002000
const int JobObjectExtendedLimitInformation = 9;

typedef HANDLE (WINAPI *PFN_CreateJobObjectW)(LPSECURITY_ATTRIBUTES, LPCWSTR);
typedef BOOL (WINAPI *PFN_SetInformationJobObject)(HANDLE, JOBOBJECTINFOCLASS, LPVOID, DWORD);
typedef BOOL (WINAPI *PFN_AssignProcessToJobObject)(HANDLE, HANDLE);

static PFN_CreateJobObjectW CreateJobObjectW = (PFN_CreateJobObjectW)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CreateJobObjectW");
static PFN_SetInformationJobObject SetInformationJobObject = (PFN_SetInformationJobObject)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetInformationJobObject");
static PFN_AssignProcessToJobObject AssignProcessToJobObject = (PFN_AssignProcessToJobObject)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "AssignProcessToJobObject");
#endif

WorkerProcess::WorkerProcess()
{
	m_processHandle = NULL;
	m_readHandle = NULL;
	m_jobHandle = NULL;
	m_exitcode = 0;
	m_isDone = false;
}

bool WorkerProcess::startProcess(UnicodeString command)
{
	m_stdOutput.clear();
	m_isDone = false;

	// Create pipe for reading console output
	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE;
	HANDLE writeHandle = NULL;
	if (!CreatePipe(&m_readHandle, &writeHandle, &saAttr, 0))
		return false;
	SetHandleInformation(m_readHandle, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOW si = { sizeof(STARTUPINFOW) };
	si.dwFlags = STARTF_FORCEOFFFEEDBACK; // Prevent cursor wait animation
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.hStdError = writeHandle;
	si.hStdOutput = writeHandle;

	PROCESS_INFORMATION pi = { 0 };

	if (!CreateProcessW(NULL, (LPWSTR)command.str(),
			NULL, NULL, /*bInheritHandles=*/TRUE, 0,
			NULL, 0, &si, &pi))
	{
		CloseHandle(writeHandle);
		CloseHandle(m_readHandle);
		m_readHandle = NULL;
		return false;
	}

	CloseHandle(pi.hThread);
	CloseHandle(writeHandle);
	m_processHandle = pi.hProcess;

	// We want to make sure that when our process is killed, our workers automatically terminate as well.
	// In Windows, the way to do this is to attach the worker to a job we own.
	m_jobHandle = CreateJobObjectW != NULL ? CreateJobObjectW(NULL, NULL) : NULL;
	if (m_jobHandle != NULL)
	{
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = { 0 };
		jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		SetInformationJobObject(m_jobHandle, (JOBOBJECTINFOCLASS)JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo));
		AssignProcessToJobObject(m_jobHandle, m_processHandle);
	}

	return true;
}

bool WorkerProcess::isRunning() const
{
	return m_processHandle != NULL;
}

bool WorkerProcess::isDone() const
{
	return m_isDone;
}

DWORD WorkerProcess::getExitCode() const
{
	return m_exitcode;
}

AsciiString WorkerProcess::getStdOutput() const
{
	return m_stdOutput;
}

bool WorkerProcess::fetchStdOutput()
{
	while (true)
	{
		// Call PeekNamedPipe to make sure ReadFile won't block
		DWORD bytesAvailable = 0;
		DEBUG_ASSERTCRASH(m_readHandle != NULL, ("Is not expected NULL"));
		BOOL success = PeekNamedPipe(m_readHandle, NULL, 0, NULL, &bytesAvailable, NULL);
		if (!success)
			return true;
		if (bytesAvailable == 0)
		{
			// Child process is still running and we have all output so far
			return false;
		}

		DWORD readBytes = 0;
		char buffer[1024];
		success = ReadFile(m_readHandle, buffer, ARRAY_SIZE(buffer)-1, &readBytes, NULL);
		if (!success)
			return true;
		DEBUG_ASSERTCRASH(readBytes != 0, ("expected readBytes to be non null"));

		// Remove \r, otherwise each new line is doubled when we output it again
		for (int i = 0; i < readBytes; i++)
			if (buffer[i] == '\r')
				buffer[i] = ' ';
		buffer[readBytes] = 0;
		m_stdOutput.concat(buffer);
	}
}

void WorkerProcess::update()
{
	if (!isRunning())
		return;

	if (!fetchStdOutput())
	{
		// There is still potential output pending
		return;
	}

	// Pipe broke, that means the process already exited. But we call this just to make sure
	WaitForSingleObject(m_processHandle, INFINITE);
	GetExitCodeProcess(m_processHandle, &m_exitcode);
	CloseHandle(m_processHandle);
	m_processHandle = NULL;

	CloseHandle(m_readHandle);
	m_readHandle = NULL;

	CloseHandle(m_jobHandle);
	m_jobHandle = NULL;

	m_isDone = true;
}

void WorkerProcess::kill()
{
	if (!isRunning())
		return;

	if (m_processHandle != NULL)
	{
		TerminateProcess(m_processHandle, 1);
		CloseHandle(m_processHandle);
		m_processHandle = NULL;
	}

	if (m_readHandle != NULL)
	{
		CloseHandle(m_readHandle);
		m_readHandle = NULL;
	}

	if (m_jobHandle != NULL)
	{
		CloseHandle(m_jobHandle);
		m_jobHandle = NULL;
	}

	m_stdOutput.clear();
	m_isDone = false;
}

