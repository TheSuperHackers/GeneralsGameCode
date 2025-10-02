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

#ifdef RTS_ENABLE_CRASHDUMP
#include <imagehlp.h>
#include "Common/MiniDumper_compat.h"

class MiniDumper
{
public:
	MiniDumper()
	{
		m_miniDumpInitialized = false;
		m_extendedInfoRequested = false;
		m_dbgHlp = NULL;
		m_pMiniDumpWriteDump = NULL;
		m_dumpRequested = NULL;
		m_dumpComplete = NULL;
		m_quitting = NULL;
		m_dumpThread = NULL;
		m_dumpThreadId = 0;
		m_dumpObjectsState = 0;
		m_dumpObjectsSubState = 0;
		m_dmaRawBlockIndex = 0;
		memset(m_dumpDir, 0, ARRAY_SIZE(m_dumpDir));
		memset(m_dumpFile, 0, ARRAY_SIZE(m_dumpFile));
		memset(m_sysDbgHelpPath, 0, ARRAY_SIZE(m_sysDbgHelpPath));
	};

	void Initialize(const AsciiString& userDirPath);
	Bool IsInitialized() const;
	void TriggerMiniDump(Bool extendedInfo = false);
	void TriggerMiniDumpForException(struct _EXCEPTION_POINTERS* e_info, Bool extendedInfo = false);
	void ShutDown();
	static LONG WINAPI DumpingExceptionFilter(struct _EXCEPTION_POINTERS* e_info);
private:
	void CreateMiniDump(Bool extendedInfo);
	BOOL DumpMemoryObjects(ULONG64& memoryBase, ULONG& memorySize);
	void CleanupResources();

	// Callbacks from dbghelp
	static BOOL CALLBACK MiniDumpCallback(PVOID CallbackParam, PMINIDUMP_CALLBACK_INPUT CallbackInput, PMINIDUMP_CALLBACK_OUTPUT CallbackOutput);
	BOOL CallbackInternal(const MINIDUMP_CALLBACK_INPUT& input, MINIDUMP_CALLBACK_OUTPUT& output);

	// Thread procs
	static DWORD WINAPI MiniDumpThreadProc(LPVOID lpParam);
	DWORD ThreadProcInternal();

	// Dump file directory bookkeeping
	Bool InitializeDumpDirectory(const AsciiString& userDirPath);
	static void KeepNewestFiles(const std::string& directory, const std::string& fileWildcard, const Int keepCount);

	// Struct to hold file information
	struct FileInfo {
		std::string name;
		FILETIME lastWriteTime;
	};

	static bool CompareByLastWriteTime(const FileInfo& a, const FileInfo& b);

private:
	Bool m_miniDumpInitialized;
	Bool m_extendedInfoRequested;

	// Path buffers
	Char m_dumpDir[MAX_PATH];
	Char m_dumpFile[MAX_PATH];
	Char m_sysDbgHelpPath[MAX_PATH];

	// Module handles
	HMODULE m_dbgHlp;

	// Event handles
	HANDLE m_dumpRequested;
	HANDLE m_dumpComplete;
	HANDLE m_quitting;

	// Thread handles
	HANDLE m_dumpThread;
	DWORD m_dumpThreadId;

	// Internal memory dumping progress state
	int m_dumpObjectsState;
	int m_dumpObjectsSubState;
	int m_dmaRawBlockIndex;

	AllocationRangeIterator m_RangeIter;
	AllocationRangeIterator m_endRangeIter;

	// Function pointer to MiniDumpWriteDump in dbghelp.dll
	typedef BOOL(WINAPI* MiniDumpWriteDump_t)(
		HANDLE hProcess,
		DWORD ProcessId,
		HANDLE hFile,
		MINIDUMP_TYPE DumpType,
		PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		PMINIDUMP_CALLBACK_INFORMATION CallbackParam
		);

	MiniDumpWriteDump_t m_pMiniDumpWriteDump;
};
#endif
