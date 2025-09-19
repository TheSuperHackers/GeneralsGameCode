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
#include "Common/GameMemory.h"
#include "gitinfo.h"

// Globals for storing the pointer to the exception
_EXCEPTION_POINTERS* g_dumpException = NULL;
DWORD g_dumpExceptionThreadId = 0;

// Globals containing state about the current exception that's used for context in the mini dump.
// These are populated by MiniDumper::DumpingExceptionFilter to store a copy of the exception in case it goes out of scope
_EXCEPTION_POINTERS g_exceptionPointers = { 0 };
EXCEPTION_RECORD g_exceptionRecord = { 0 };
CONTEXT g_exceptionContext = { 0 };


LONG WINAPI MiniDumper::DumpingExceptionFilter(struct _EXCEPTION_POINTERS* e_info)
{
	// Store the exception info in the global variables for later use by the dumping thread
	g_exceptionRecord = *(e_info->ExceptionRecord);
	g_exceptionContext = *(e_info->ContextRecord);
	g_exceptionPointers.ContextRecord = &g_exceptionContext;
	g_exceptionPointers.ExceptionRecord = &g_exceptionRecord;
	g_dumpException = &g_exceptionPointers;

	return EXCEPTION_EXECUTE_HANDLER;
}

void MiniDumper::TriggerMiniDump(Bool extendedInfo)
{
	if (!m_miniDumpInitialized)
	{
		DEBUG_LOG(("MiniDumper::TriggerMiniDump: Attempted to use an uninitialized instance."));
		return;
	}

	__try
	{
		// Use DebugBreak to raise an exception that can be caught in the __except block
		DebugBreak();
	}
	__except (DumpingExceptionFilter(GetExceptionInformation()))
	{
		TriggerMiniDumpForException(g_dumpException, extendedInfo);
	}
}

void MiniDumper::TriggerMiniDumpForException(struct _EXCEPTION_POINTERS* e_info, Bool extendedInfo)
{
	if (!m_miniDumpInitialized)
	{
		DEBUG_LOG(("MiniDumper::TriggerMiniDumpForException: Attempted to use an uninitialized instance."));
		return;
	}

	g_dumpException = e_info;
	g_dumpExceptionThreadId = GetCurrentThreadId();
	m_extendedInfoRequested = extendedInfo;

	SetEvent(m_dumpRequested);
	DWORD wait = WaitForSingleObject(m_dumpComplete, INFINITE);
	if (wait != WAIT_OBJECT_0)
	{
		if (wait == WAIT_FAILED)
		{
			DEBUG_LOG(("MiniDumper::TriggerMiniDumpForException: Waiting for minidump triggering failed, status=%u, error=%u", wait, GetLastError()));
		}
		else
		{
			DEBUG_LOG(("MiniDumper::TriggerMiniDumpForException: Waiting for minidump triggering failed, status=%u", wait));
		}
	}

	ResetEvent(m_dumpComplete);
}

void MiniDumper::Initialize(const AsciiString& userDirPath)
{
	// Find the full path to the dbghelp.dll file in the system32 dir
	GetSystemDirectory(m_sysDbgHelpPath, MAX_PATH);
	strlcat(m_sysDbgHelpPath, "\\dbghelp.dll", MAX_PATH);

	// We want to only use the dbghelp.dll from the OS installation, as the one bundled with the game does not support MiniDump functionality
	Bool loadedDbgHelp = false;
	HMODULE m_dbgHlp = GetModuleHandle(m_sysDbgHelpPath);
	if (m_dbgHlp == NULL)
	{
		// Load the dbghelp library from the system folder
		m_dbgHlp = LoadLibrary(m_sysDbgHelpPath);
		if (m_dbgHlp == NULL)
		{
			DEBUG_LOG(("MiniDumper::Initialize: Unable to load system-provided dbghelp.dll from '%s', error code=%u", m_sysDbgHelpPath, GetLastError()));
			return;
		}

		loadedDbgHelp = true;
	}

	m_pMiniDumpWriteDump = (MiniDumpWriteDump_t)GetProcAddress(m_dbgHlp, "MiniDumpWriteDump");
	if (m_pMiniDumpWriteDump == NULL)
	{
		if (loadedDbgHelp)
		{
			FreeLibrary(m_dbgHlp);
			m_dbgHlp = NULL;
		}

		DEBUG_LOG(("MiniDumper::Initialize: Could not get address of proc MiniDumpWriteDump from '%s'!", m_sysDbgHelpPath));
		return;
	}

	// Create & store dump folder
	if (!InitializeDumpDirectory(userDirPath))
	{
		return;
	}

	m_dumpRequested = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_dumpComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_quitting = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_dumpRequested == NULL || m_dumpComplete == NULL || m_quitting == NULL)
	{
		// Something went wrong with the creation of the events..
		DEBUG_LOG(("MiniDumper::Initialize: Unable to create events: error=%u", GetLastError()));
		CleanupResources();
		return;
	}

	m_dumpThread = CreateThread(NULL, 0, MiniDumpThreadProc, this, CREATE_SUSPENDED, &m_dumpThreadId);
	if (!m_dumpThread)
	{
		DEBUG_LOG(("MiniDumper::Initialize: Unable to create thread: error=%u", GetLastError()));
		CleanupResources();
		return;
	}

	if (!ResumeThread(m_dumpThread))
	{
		DEBUG_LOG(("MiniDumper::Initialize: Unable to resume thread: error=%u", GetLastError()));
		CleanupResources();
		return;
	}

	DEBUG_LOG(("MiniDumper::Initialize: Configured to store crash dumps in '%s'", m_dumpDir));
	m_miniDumpInitialized = true;
}

Bool MiniDumper::IsInitialized() const
{
	return m_miniDumpInitialized;
}

Bool MiniDumper::InitializeDumpDirectory(const AsciiString& userDirPath)
{
	constexpr Int MaxExtendedFileCount = 2;
	constexpr Int MaxMiniFileCount = 10;

	strlcpy(m_dumpDir, userDirPath.str(), ARRAY_SIZE(m_dumpDir));
	strlcat(m_dumpDir, "CrashDumps\\", ARRAY_SIZE(m_dumpDir));
	if (_access(m_dumpDir, 0) != 0)
	{
		if (!CreateDirectory(m_dumpDir, NULL))
		{
			DEBUG_LOG(("MiniDumper::Initialize: Unable to create path for crash dumps at '%s': %u", m_dumpDir, GetLastError()));
			return false;
		}
	}

	// Clean up old files (we keep a maximum of 10 small and 2 extended)
	KeepNewestFiles(m_dumpDir, "CrashX*", MaxExtendedFileCount);
	KeepNewestFiles(m_dumpDir, "CrashM*", MaxMiniFileCount);

	return true;
}

void MiniDumper::CleanupResources()
{
	// NOTE: This method should not be called unless the dump thread is confirmed to not be running anymore.
	if (m_dumpThread != NULL)
	{
		CloseHandle(m_dumpThread);
		m_dumpThread = NULL;
	}

	if (m_dumpComplete != NULL)
	{
		CloseHandle(m_dumpComplete);
		m_dumpComplete = NULL;
	}

	if (m_dumpRequested != NULL)
	{
		CloseHandle(m_dumpRequested);
		m_dumpRequested = NULL;
	}

	if (m_quitting != NULL)
	{
		CloseHandle(m_quitting);
		m_quitting = NULL;
	}

	if (m_dbgHlp != NULL)
	{
		FreeModule(m_dbgHlp);
		m_dbgHlp = NULL;
	}
}

void MiniDumper::ShutDown()
{
	if (!m_miniDumpInitialized)
	{
		return;
	}

	SetEvent(m_quitting);
	DWORD waitRet = WaitForSingleObject(m_dumpThread, 3000);
	if (waitRet != WAIT_OBJECT_0)
	{
		if (waitRet == WAIT_TIMEOUT)
		{
			DEBUG_LOG(("MiniDumper::ShutDown: Waiting for dumping thread to exit timed out, killing thread", waitRet));
			TerminateThread(m_dumpThread, 2);
		}
		else if (waitRet == WAIT_FAILED)
		{
			DEBUG_LOG(("MiniDumper::ShutDown: Waiting for minidump triggering failed, status=%u, error=%u", waitRet, GetLastError()));
		}
		else
		{
			DEBUG_LOG(("MiniDumper::ShutDown: Waiting for minidump triggering failed, status=%u", waitRet));
		}
		return;
	}

	CleanupResources();
	m_miniDumpInitialized = false;
}

DWORD MiniDumper::ThreadProcInternal()
{
	while (true)
	{
		HANDLE waitEvents[2] = { m_dumpRequested, m_quitting };
		DWORD event = WaitForMultipleObjects(ARRAY_SIZE(waitEvents), waitEvents, FALSE, INFINITE);
		if (event == WAIT_OBJECT_0 + 0)
		{
			// A dump is requested (m_dumpRequested)
			ResetEvent(m_dumpComplete);
			CreateMiniDump(m_extendedInfoRequested);
			ResetEvent(m_dumpRequested);
			SetEvent(m_dumpComplete);
		}
		else if (event == WAIT_OBJECT_0 + 1)
		{
			// Quit (m_quitting)
			return 0;
		}
		else
		{
			if (event == WAIT_FAILED)
			{
				DEBUG_LOG(("MiniDumper::ThreadProcInternal: Waiting for events failed, status=%u, error=%u", event, GetLastError()));
			}
			else
			{
				DEBUG_LOG(("MiniDumper::ThreadProcInternal: Waiting for events failed, status=%u", event));
			}
			return 1;
		}
	}
}

DWORD WINAPI MiniDumper::MiniDumpThreadProc(LPVOID lpParam)
{
	if (lpParam == NULL)
	{
		DEBUG_LOG(("MiniDumper::MiniDumpThreadProc: The provided parameter was NULL, exiting thread."));
		return -1;
	}

	MiniDumper* dumper = static_cast<MiniDumper *>(lpParam);
	return dumper->ThreadProcInternal();
}


void MiniDumper::CreateMiniDump(Bool extendedInfo)
{
	// Create a unique dump file name, using the path from m_dumpDir, in m_dumpFile
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
#if RTS_ZEROHOUR
	Char product = 'Z';
#else
	Char product = 'G';
#endif
	Char dumpTypeSpecifier = extendedInfo ? 'X' : 'M';
	DWORD currentProcessId = GetCurrentProcessId();
	DWORD currentThreadId = GetCurrentThreadId();

	// m_dumpDir is stored with trailing backslash in Initialize
	snprintf(m_dumpFile, ARRAY_SIZE(m_dumpFile), "%sCrash%c%c-%04d%02d%02d-%02d%02d%02d-%s-%ld-%ld.dmp",
		m_dumpDir, dumpTypeSpecifier, product, sysTime.wYear, sysTime.wMonth,
		sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
		GitShortSHA1, currentProcessId, currentThreadId);

	HANDLE dumpFile = CreateFile(m_dumpFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dumpFile == NULL || dumpFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(("MiniDumper::CreateMiniDump: Unable to create dump file '%s', error=%u", m_dumpFile, GetLastError()));
		return;
	}

	PMINIDUMP_EXCEPTION_INFORMATION exceptionInfoPtr = NULL;
	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo = { 0 };
	if (g_dumpException != NULL)
	{
		exceptionInfo.ExceptionPointers = g_dumpException;
		exceptionInfo.ThreadId = g_dumpExceptionThreadId;
		exceptionInfo.ClientPointers = FALSE;
		exceptionInfoPtr = &exceptionInfo;
	}

	PMINIDUMP_CALLBACK_INFORMATION callbackInfoPtr = NULL;
	MINIDUMP_CALLBACK_INFORMATION callBackInfo = { 0 };
	if (extendedInfo)
	{
		callBackInfo.CallbackRoutine = MiniDumpCallback;
		callBackInfo.CallbackParam = this;
		callbackInfoPtr = &callBackInfo;
	}

	MINIDUMP_TYPE dumpType = static_cast<MINIDUMP_TYPE>(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);
	if (extendedInfo)
	{
		dumpType = static_cast<MINIDUMP_TYPE>(MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithThreadInfo | MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithFullMemoryInfo);
	}

	BOOL success = m_pMiniDumpWriteDump(
		GetCurrentProcess(),
		currentProcessId,
		dumpFile,
		dumpType,
		exceptionInfoPtr,
		NULL,
		callbackInfoPtr);

	if (!success)
	{
		DEBUG_LOG(("MiniDumper::CreateMiniDump: Unable to write minidump file '%s', error=%u", m_dumpFile, GetLastError()));
	}
	else
	{
		DEBUG_LOG(("MiniDumper::CreateMiniDump: Successfully wrote minidump file to '%s'", m_dumpFile));
	}

	CloseHandle(dumpFile);
}

BOOL CALLBACK MiniDumper::MiniDumpCallback(PVOID CallbackParam, PMINIDUMP_CALLBACK_INPUT CallbackInput, PMINIDUMP_CALLBACK_OUTPUT CallbackOutput)
{
	if (CallbackParam == NULL || CallbackInput == NULL || CallbackOutput == NULL)
	{
		DEBUG_LOG(("MiniDumper::MiniDumpCallback: Required parameters were null; CallbackParam=%p, CallbackInput=%p, CallbackOutput=%p.", CallbackParam, CallbackInput, CallbackOutput));
		return false;
	}

	MiniDumper* dumper = static_cast<MiniDumper*>(CallbackParam);
	return dumper->CallbackInternal(*CallbackInput, *CallbackOutput);
}

// This is where the memory regions and things are being filtered
BOOL MiniDumper::CallbackInternal(const MINIDUMP_CALLBACK_INPUT& input, MINIDUMP_CALLBACK_OUTPUT& output)
{
	BOOL retVal = TRUE;
	switch (input.CallbackType)
	{
	case IncludeModuleCallback:
		retVal = TRUE;
		break;
	case ModuleCallback:
	{
		// Only include data segments for the game and ntdll modules to keep dump size low
		if (output.ModuleWriteFlags & ModuleWriteDataSeg)
		{
			if (!StrStrIW(input.Module.FullPath, L"generalszh.exe") && !StrStrIW(input.Module.FullPath, L"generalsv.exe") && !StrStrIW(input.Module.FullPath, L"ntdll.dll"))
			{
				// Exclude data segments for the module
				output.ModuleWriteFlags &= (~ModuleWriteDataSeg);
			}
		}

		retVal = TRUE;
		break;
	}
	case IncludeThreadCallback:
		// We want all threads except the dumping thread
		if (input.IncludeThread.ThreadId == m_dumpThreadId)
		{
			retVal = FALSE;
		}
		break;
	case ThreadCallback:
		retVal = TRUE;
		break;
	case ThreadExCallback:
		retVal = TRUE;
		break;
	case MemoryCallback:
	{
		do
		{
			// DumpMemoryObjects will return false once it's completed, signalling the end of memory callbacks
			retVal = DumpMemoryObjects(output.MemoryBase, output.MemorySize);
		} while ((output.MemoryBase == NULL || output.MemorySize == NULL) && retVal == TRUE);
		break;
	}
	case ReadMemoryFailureCallback:
	{
		DEBUG_LOG(("MiniDumper::CallbackInternal: ReadMemoryFailure with MemoryBase=%llu, MemorySize=%lu, error=%u", input.ReadMemoryFailure.Offset, input.ReadMemoryFailure.Bytes, input.ReadMemoryFailure.FailureStatus));
		retVal = TRUE;
		break;
	}
	case CancelCallback:
		output.Cancel = FALSE;
		output.CheckCancel = FALSE;
		retVal = TRUE;
		break;
	}

	return retVal;
}

BOOL MiniDumper::DumpMemoryObjects(ULONG64& memoryBase, ULONG& memorySize)
{
	BOOL moreToDo = TRUE;
	// m_dumpObjectsState is used to keep track of the current "phase" of the memory dumping process
	// m_dumpObjectsSubState is used to keep track of the progress within each phase, and is reset when advancing on to the next phase
	switch (m_dumpObjectsState)
	{
	case 0:
	{
		// Dump all the MemoryPool instances in TheMemoryPoolFactory
		// This only dumps the metadata, not the actual MemoryPool contents (done in the next phase).
		if (TheMemoryPoolFactory == NULL)
		{
			++m_dumpObjectsState;
			break;
		}

		Int poolCount = TheMemoryPoolFactory->getMemoryPoolCount();
		//m_dumpObjectsSubState contains the index in TheMemoryPoolFactory of the MemoryPool that is being processed
		if (m_dumpObjectsSubState < poolCount)
		{
			MemoryPool* pool = TheMemoryPoolFactory->getMemoryPoolN(m_dumpObjectsSubState);
			if (pool != NULL)
			{
				memoryBase = reinterpret_cast<ULONG64>(pool);
				memorySize = sizeof(MemoryPool);
				++m_dumpObjectsSubState;
			}
			else
			{
				m_dumpObjectsSubState = poolCount;
			}
		}

		if (m_dumpObjectsSubState == poolCount)
		{
			m_dumpObjectsSubState = 0;
			++m_dumpObjectsState;
		}
		break;
	}
	case 1:
	{
		// Iterate through all the allocations of memory pools and containing blobs that has been done via the memory pool factory
		// and include all of the storage space allocated for objects
		if (TheMemoryPoolFactory == NULL)
		{
			++m_dumpObjectsState;
			break;
		}

		//m_dumpObjectsSubState is used to track if the iterator needs to be initialized, otherwise just a counter of the number of items dumped
		if (m_dumpObjectsSubState == 0)
		{
			m_RangeIter = TheMemoryPoolFactory->cbegin();
			m_endRangeIter = TheMemoryPoolFactory->cend();
			++m_dumpObjectsSubState;
		}

		// m_RangeIter should != m_endRangeIter, unless the memory pool factory is corrupted (or has 0 entries)
		memoryBase = reinterpret_cast<ULONG64>(m_RangeIter->allocationAddr);
		memorySize = m_RangeIter->allocationSize;
		++m_dumpObjectsSubState;
		++m_RangeIter;

		if (m_RangeIter == m_endRangeIter)
		{
			++m_dumpObjectsState;
			m_dumpObjectsSubState = 0;
		}
		break;
	}
	case 2:
	{
		// Iterate through all the direct allocations ("raw blocks") done by DMAs, as these are done outside of the
		// memory pool factory allocations dumped in the previous phase.
		if (TheDynamicMemoryAllocator == NULL)
		{
			++m_dumpObjectsState;
			break;
		}

		DynamicMemoryAllocator* allocator = TheDynamicMemoryAllocator;

		//m_dumpObjectsSubState is used to track the index of the allocator we are currently traversing
		for (int i = 0; i < m_dumpObjectsSubState; ++i)
		{
			allocator = allocator->getNextDmaInList();
		}

		MemoryPoolAllocatedRange rawBlockRange = {0};
		int rawBlocksInDma = allocator->getRawBlockCount();
		if (m_dmaRawBlockIndex < rawBlocksInDma)
		{
			// Dump this block
			allocator->fillAllocationRangeForRawBlockN(m_dmaRawBlockIndex, rawBlockRange);
			memoryBase = reinterpret_cast<ULONG64>(rawBlockRange.allocationAddr);
			memorySize = rawBlockRange.allocationSize;
			++m_dmaRawBlockIndex;
		}

		if (rawBlocksInDma == m_dmaRawBlockIndex)
		{
			// Advance to the next DMA
			++m_dumpObjectsSubState;
			m_dmaRawBlockIndex = 0;
			if (allocator->getNextDmaInList() == NULL)
			{
				// Done iterating through all the DMAs
				m_dumpObjectsSubState = 0;
				++m_dumpObjectsState;
			}
		}
		break;
	}
	default:
		// Done, set "no more stuff" values
		m_dumpObjectsState = 0;
		m_dumpObjectsSubState = 0;
		m_dmaRawBlockIndex = 0;
		memoryBase = 0;
		memorySize = 0;
		moreToDo = FALSE;
		break;
	}

	return moreToDo;
}

// Comparator for sorting files by last modified time (newest first)
bool MiniDumper::CompareByLastWriteTime(const FileInfo& a, const FileInfo& b) {
	return CompareFileTime(&a.lastWriteTime, &b.lastWriteTime) > 0;
}

void MiniDumper::KeepNewestFiles(const std::string& directory, const std::string& fileWildcard, const Int keepCount)
{
	// directory already contains trailing backslash
	std::string searchPath = directory + fileWildcard;
	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);

	if (hFind == INVALID_HANDLE_VALUE) {
		if (GetLastError() != ERROR_FILE_NOT_FOUND)
		{
			DEBUG_LOG(("MiniDumper::KeepNewestFiles: Unable to find files in directory '%s': %u", searchPath.c_str(), GetLastError()));
		}

		return;
	}

	std::vector<FileInfo> files;
	do {
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		// Store file info
		FileInfo fileInfo;
		fileInfo.name = directory + findData.cFileName;
		fileInfo.lastWriteTime = findData.ftLastWriteTime;
		files.push_back(fileInfo);

	} while (FindNextFile(hFind, &findData));

	FindClose(hFind);

	// Sort files by last modified time in descending order
	std::sort(files.begin(), files.end(), CompareByLastWriteTime);

	// Delete files beyond the newest keepCount
	for (size_t i = keepCount; i < files.size(); ++i) {
		if (DeleteFile(files[i].name.c_str())) {
			DEBUG_LOG(("MiniDumper::KeepNewestFiles: Deleted old dump file '%s'.", files[i].name.c_str()));
		}
		else {
			DEBUG_LOG(("MiniDumper::KeepNewestFiles: Failed to delete file '%s', error=%u", files[i].name.c_str(), GetLastError()));
		}
	}
}
#endif
