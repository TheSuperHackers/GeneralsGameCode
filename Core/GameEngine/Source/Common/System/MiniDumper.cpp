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

#ifdef RTS_ENABLE_CRASHDUMP
#include "Common/MiniDumper.h"
#include <wctype.h>
#include "gitinfo.h"

// Globals for storing the pointer to the exception
_EXCEPTION_POINTERS* g_dumpException = nullptr;
DWORD g_dumpExceptionThreadId = 0;

MiniDumper* TheMiniDumper = nullptr;

// Globals containing state about the current exception that's used for context in the mini dump.
// These are populated by MiniDumper::DumpingExceptionFilter to store a copy of the exception in case it goes out of scope
_EXCEPTION_POINTERS g_exceptionPointers = { 0 };
EXCEPTION_RECORD g_exceptionRecord = { 0 };
CONTEXT g_exceptionContext = { 0 };

constexpr const char* DumpFileNamePrefix = "Crash";

void MiniDumper::initMiniDumper(const AsciiString& userDirPath)
{
	DEBUG_ASSERTCRASH(TheMiniDumper == nullptr, ("MiniDumper::initMiniDumper called on already created instance"));

	// Use placement new on the process heap so TheMiniDumper is placed outside the MemoryPoolFactory managed area.
	// If the crash is due to corrupted MemoryPoolFactory structures, try to mitigate the chances of MiniDumper memory also being corrupted
	TheMiniDumper = new (::HeapAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(MiniDumper))) MiniDumper;
	TheMiniDumper->Initialize(userDirPath);
}

void MiniDumper::shutdownMiniDumper()
{
	if (TheMiniDumper)
	{
		TheMiniDumper->ShutDown();
		TheMiniDumper->~MiniDumper();
		::HeapFree(::GetProcessHeap(), 0, TheMiniDumper);
		TheMiniDumper = nullptr;
	}
}

MiniDumper::MiniDumper()
{
	m_miniDumpInitialized = false;
	m_loadedDbgHelp = false;
	m_requestedDumpType = DumpType_Minimal;
	m_dumpRequested = nullptr;
	m_dumpComplete = nullptr;
	m_quitting = nullptr;
	m_dumpThread = nullptr;
	m_dumpThreadId = 0;
	m_dumpDir[0] = 0;
	m_dumpFile[0] = 0;
	m_executablePath[0] = 0;
};

LONG WINAPI MiniDumper::DumpingExceptionFilter(_EXCEPTION_POINTERS* e_info)
{
	// Store the exception info in the global variables for later use by the dumping thread
	g_exceptionRecord = *(e_info->ExceptionRecord);
	g_exceptionContext = *(e_info->ContextRecord);
	g_exceptionPointers.ContextRecord = &g_exceptionContext;
	g_exceptionPointers.ExceptionRecord = &g_exceptionRecord;
	g_dumpException = &g_exceptionPointers;

	return EXCEPTION_EXECUTE_HANDLER;
}

void MiniDumper::TriggerMiniDump(DumpType dumpType)
{
	if (!m_miniDumpInitialized)
	{
		DEBUG_LOG(("MiniDumper::TriggerMiniDump: Attempted to use an uninitialized instance."));
		return;
	}

	__try
	{
		// Use DebugBreak to raise an exception that can be caught in the __except block
		::DebugBreak();
	}
	__except (DumpingExceptionFilter(GetExceptionInformation()))
	{
		TriggerMiniDumpForException(g_dumpException, dumpType);
	}
}

void MiniDumper::TriggerMiniDumpForException(_EXCEPTION_POINTERS* e_info, DumpType dumpType)
{
	if (!m_miniDumpInitialized)
	{
		DEBUG_LOG(("MiniDumper::TriggerMiniDumpForException: Attempted to use an uninitialized instance."));
		return;
	}

	g_dumpException = e_info;
	g_dumpExceptionThreadId = ::GetCurrentThreadId();
	m_requestedDumpType = dumpType;

	DEBUG_ASSERTCRASH(IsDumpThreadStillRunning(), ("MiniDumper::TriggerMiniDumpForException: Dumping thread has exited."));
	::SetEvent(m_dumpRequested);
	DWORD wait = ::WaitForSingleObject(m_dumpComplete, INFINITE);
	if (wait != WAIT_OBJECT_0)
	{
		if (wait == WAIT_FAILED)
		{
			DEBUG_LOG(("MiniDumper::TriggerMiniDumpForException: Waiting for minidump triggering failed: status=%u, error=%u", wait, ::GetLastError()));
		}
		else
		{
			DEBUG_LOG(("MiniDumper::TriggerMiniDumpForException: Waiting for minidump triggering failed: status=%u", wait));
		}
	}

	::ResetEvent(m_dumpComplete);
}

void MiniDumper::Initialize(const AsciiString& userDirPath)
{
	m_loadedDbgHelp = DbgHelpLoader::load();

	// We want to only use the dbghelp.dll from the OS installation, as the one bundled with the game does not support MiniDump functionality
	if (!(m_loadedDbgHelp && DbgHelpLoader::isLoadedFromSystem()))
	{
		DEBUG_LOG(("MiniDumper::Initialize: Unable to load system-provided dbghelp.dll, minidump functionality disabled."));
		return;
	}

	DWORD executableSize = ::GetModuleFileNameW(nullptr, m_executablePath, ARRAY_SIZE(m_executablePath));
	if (executableSize == 0 || executableSize == ARRAY_SIZE(m_executablePath))
	{
		DEBUG_LOG(("MiniDumper::Initialize: Could not get executable file name. Returned value=%u", executableSize));
		return;
	}

	// Create & store dump folder
	if (!InitializeDumpDirectory(userDirPath))
	{
		return;
	}

	m_dumpRequested = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	m_dumpComplete = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	m_quitting = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	if (m_dumpRequested == nullptr || m_dumpComplete == nullptr || m_quitting == nullptr)
	{
		// Something went wrong with the creation of the events..
		DEBUG_LOG(("MiniDumper::Initialize: Unable to create events: error=%u", ::GetLastError()));
		return;
	}

	m_dumpThread = ::CreateThread(nullptr, 0, MiniDumpThreadProc, this, CREATE_SUSPENDED, &m_dumpThreadId);
	if (!m_dumpThread)
	{
		DEBUG_LOG(("MiniDumper::Initialize: Unable to create thread: error=%u", ::GetLastError()));
		return;
	}

	if (::ResumeThread(m_dumpThread) != 1)
	{
		DEBUG_LOG(("MiniDumper::Initialize: Unable to resume thread: error=%u", ::GetLastError()));
		return;
	}

	DEBUG_LOG(("MiniDumper::Initialize: Configured to store crash dumps in '%s'", m_dumpDir));
	m_miniDumpInitialized = true;
}

Bool MiniDumper::IsInitialized() const
{
	return m_miniDumpInitialized;
}

Bool MiniDumper::IsDumpThreadStillRunning() const
{
	DWORD exitCode;
	if (m_dumpThread != nullptr && ::GetExitCodeThread(m_dumpThread, &exitCode) && exitCode == STILL_ACTIVE)
	{
		return true;
	}

	return false;
}

Bool MiniDumper::InitializeDumpDirectory(const AsciiString& userDirPath)
{
	constexpr const Int MaxFullFileCount = 2;
	constexpr const Int MaxMiniFileCount = 10;

	strlcpy(m_dumpDir, userDirPath.str(), ARRAY_SIZE(m_dumpDir));
	strlcat(m_dumpDir, "CrashDumps\\", ARRAY_SIZE(m_dumpDir));
	if (::_access(m_dumpDir, 0) != 0)
	{
		if (!::CreateDirectory(m_dumpDir, nullptr))
		{
			DEBUG_LOG(("MiniDumper::Initialize: Unable to create path for crash dumps at '%s': error=%u", m_dumpDir, ::GetLastError()));
			return false;
		}
	}

	// Clean up old files (we keep a maximum of 10 small, 2 full)
	KeepNewestFiles(m_dumpDir, DumpType_Full, MaxFullFileCount);
	KeepNewestFiles(m_dumpDir, DumpType_Minimal, MaxMiniFileCount);

	return true;
}

void MiniDumper::ShutdownDumpThread()
{
	if (IsDumpThreadStillRunning())
	{
		DEBUG_ASSERTCRASH(m_quitting != nullptr, ("MiniDumper::ShutdownDumpThread: Dump thread still running despite m_quitting being null"));
		::SetEvent(m_quitting);

		DWORD waitRet = ::WaitForSingleObject(m_dumpThread, 3000);
		switch (waitRet)
		{
		case WAIT_OBJECT_0:
			// Wait for thread exit was successful
			break;
		case WAIT_TIMEOUT:
			DEBUG_LOG(("MiniDumper::ShutdownDumpThread: Waiting for dumping thread to exit timed out, killing thread", waitRet));
			::TerminateThread(m_dumpThread, MiniDumperExitCode_ForcedTerminate);
			break;
		case WAIT_FAILED:
			DEBUG_LOG(("MiniDumper::ShutdownDumpThread: Waiting for minidump triggering failed: status=%u, error=%u", waitRet, ::GetLastError()));
			break;
		default:
			DEBUG_LOG(("MiniDumper::ShutdownDumpThread: Waiting for minidump triggering failed: status=%u", waitRet));
			break;
		}
	}
}

void MiniDumper::ShutDown()
{
	ShutdownDumpThread();

	if (m_dumpThread != nullptr)
	{
		DEBUG_ASSERTCRASH(!IsDumpThreadStillRunning(), ("MiniDumper::ShutDown: ShutdownDumpThread() was unable to stop Dump thread"));
		::CloseHandle(m_dumpThread);
		m_dumpThread = nullptr;
	}

	if (m_quitting != nullptr)
	{
		::CloseHandle(m_quitting);
		m_quitting = nullptr;
	}

	if (m_dumpComplete != nullptr)
	{
		::CloseHandle(m_dumpComplete);
		m_dumpComplete = nullptr;
	}

	if (m_dumpRequested != nullptr)
	{
		::CloseHandle(m_dumpRequested);
		m_dumpRequested = nullptr;
	}

	if (m_loadedDbgHelp)
	{
		DbgHelpLoader::unload();
		m_loadedDbgHelp = false;
	}

	m_miniDumpInitialized = false;
}

DWORD MiniDumper::ThreadProcInternal()
{
	while (true)
	{
		HANDLE waitEvents[2] = { m_dumpRequested, m_quitting };
		DWORD event = ::WaitForMultipleObjects(ARRAY_SIZE(waitEvents), waitEvents, FALSE, INFINITE);
		switch (event)
		{
		case WAIT_OBJECT_0 + 0:
			// A dump is requested (m_dumpRequested)
			::ResetEvent(m_dumpComplete);
			CreateMiniDump(m_requestedDumpType);
			::ResetEvent(m_dumpRequested);
			::SetEvent(m_dumpComplete);
			break;
		case WAIT_OBJECT_0 + 1:
			// Quit (m_quitting)
			return MiniDumperExitCode_Success;
		case WAIT_FAILED:
			DEBUG_LOG(("MiniDumper::ThreadProcInternal: Waiting for events failed: status=%u, error=%u", event, ::GetLastError()));
			return MiniDumperExitCode_FailureWait;
		default:
			DEBUG_LOG(("MiniDumper::ThreadProcInternal: Waiting for events failed: status=%u", event));
			return MiniDumperExitCode_FailureWait;
		}
	}
}

DWORD WINAPI MiniDumper::MiniDumpThreadProc(LPVOID lpParam)
{
	if (lpParam == nullptr)
	{
		DEBUG_LOG(("MiniDumper::MiniDumpThreadProc: The provided parameter was null, exiting thread."));
		return MiniDumperExitCode_FailureParam;
	}

	MiniDumper* dumper = static_cast<MiniDumper *>(lpParam);
	return dumper->ThreadProcInternal();
}


void MiniDumper::CreateMiniDump(DumpType dumpType)
{
	// Create a unique dump file name, using the path from m_dumpDir, in m_dumpFile
	SYSTEMTIME sysTime;
	::GetLocalTime(&sysTime);
#if RTS_GENERALS
	const Char product = 'G';
#elif RTS_ZEROHOUR
	const Char product = 'Z';
#endif
	Char dumpTypeSpecifier = static_cast<Char>(dumpType);
	DWORD currentProcessId = ::GetCurrentProcessId();

	// m_dumpDir is stored with trailing backslash in Initialize
	snprintf(m_dumpFile, ARRAY_SIZE(m_dumpFile), "%s%s%c%c-%04d%02d%02d-%02d%02d%02d-%s-pid%ld.dmp",
		m_dumpDir, DumpFileNamePrefix, dumpTypeSpecifier, product, sysTime.wYear, sysTime.wMonth,
		sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
		GitShortSHA1, currentProcessId);

	HANDLE dumpFile = ::CreateFile(m_dumpFile, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (dumpFile == nullptr || dumpFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(("MiniDumper::CreateMiniDump: Unable to create dump file '%s': error=%u", m_dumpFile, ::GetLastError()));
		return;
	}

	PMINIDUMP_EXCEPTION_INFORMATION exceptionInfoPtr = nullptr;
	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo = { 0 };
	if (g_dumpException != nullptr)
	{
		exceptionInfo.ExceptionPointers = g_dumpException;
		exceptionInfo.ThreadId = g_dumpExceptionThreadId;
		exceptionInfo.ClientPointers = FALSE;
		exceptionInfoPtr = &exceptionInfo;
	}

	int dumpTypeFlags = MiniDumpNormal;
	switch (dumpType)
	{
	case DumpType_Full:
		dumpTypeFlags |= MiniDumpWithFullMemory | MiniDumpWithDataSegs | MiniDumpWithHandleData |
			MiniDumpWithThreadInfo | MiniDumpWithFullMemoryInfo | MiniDumpWithPrivateReadWriteMemory;
		FALLTHROUGH;
	case DumpType_Minimal:
		dumpTypeFlags |= MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory;
		break;
	}

	MINIDUMP_TYPE miniDumpType = static_cast<MINIDUMP_TYPE>(dumpTypeFlags);
	BOOL success = DbgHelpLoader::miniDumpWriteDump(
		::GetCurrentProcess(),
		currentProcessId,
		dumpFile,
		miniDumpType,
		exceptionInfoPtr,
		nullptr,
		nullptr);

	if (!success)
	{
		DEBUG_LOG(("MiniDumper::CreateMiniDump: Unable to write minidump file '%s': error=%u", m_dumpFile, ::GetLastError()));
	}
	else
	{
		DEBUG_LOG(("MiniDumper::CreateMiniDump: Successfully wrote minidump file to '%s'", m_dumpFile));
	}

	::CloseHandle(dumpFile);
}

// Comparator for sorting files by last modified time (newest first)
bool MiniDumper::CompareByLastWriteTime(const FileInfo& a, const FileInfo& b)
{
	return ::CompareFileTime(&a.lastWriteTime, &b.lastWriteTime) > 0;
}

void MiniDumper::KeepNewestFiles(const std::string& directory, const DumpType dumpType, const Int keepCount)
{
	// directory already contains trailing backslash
	std::string searchPath = directory + DumpFileNamePrefix + static_cast<Char>(dumpType) + "*";
	WIN32_FIND_DATA findData;
	HANDLE hFind = ::FindFirstFile(searchPath.c_str(), &findData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		if (::GetLastError() != ERROR_FILE_NOT_FOUND)
		{
			DEBUG_LOG(("MiniDumper::KeepNewestFiles: Unable to find files in directory '%s': error=%u", searchPath.c_str(), ::GetLastError()));
		}

		return;
	}

	std::vector<FileInfo> files;
	do
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}

		// Store file info
		FileInfo fileInfo;
		fileInfo.name = directory + findData.cFileName;
		fileInfo.lastWriteTime = findData.ftLastWriteTime;
		files.push_back(fileInfo);

	} while (::FindNextFile(hFind, &findData));

	::FindClose(hFind);

	// Sort files by last modified time in descending order
	std::sort(files.begin(), files.end(), CompareByLastWriteTime);

	// Delete files beyond the newest keepCount
	for (size_t i = keepCount; i < files.size(); ++i)
	{
		if (::DeleteFile(files[i].name.c_str()))
		{
			DEBUG_LOG(("MiniDumper::KeepNewestFiles: Deleted old dump file '%s'.", files[i].name.c_str()));
		}
		else
		{
			DEBUG_LOG(("MiniDumper::KeepNewestFiles: Failed to delete file '%s': error=%u", files[i].name.c_str(), ::GetLastError()));
		}
	}
}
#endif
