#pragma once

#ifdef RTS_ENABLE_CRASHDUMP

// Backported defines from minidumpapiset.h for VC6.
// minidumpapiset.h is Copyright (C) Microsoft Corporation. All rights reserved.

#if defined(_MSC_VER) && _MSC_VER < 1300
#include <pshpack4.h>

typedef enum _MINIDUMP_CALLBACK_TYPE {
	ModuleCallback,
	ThreadCallback,
	ThreadExCallback,
	IncludeThreadCallback,
	IncludeModuleCallback,
	MemoryCallback,
	CancelCallback,
	WriteKernelMinidumpCallback,
	KernelMinidumpStatusCallback,
	RemoveMemoryCallback,
	IncludeVmRegionCallback,
	IoStartCallback,
	IoWriteAllCallback,
	IoFinishCallback,
	ReadMemoryFailureCallback,
	SecondaryFlagsCallback,
	IsProcessSnapshotCallback,
	VmStartCallback,
	VmQueryCallback,
	VmPreReadCallback,
	VmPostReadCallback
} MINIDUMP_CALLBACK_TYPE;

typedef struct _MINIDUMP_THREAD_CALLBACK {
	ULONG ThreadId;
	HANDLE ThreadHandle;
#if defined(_ARM64_)
	ULONG Pad;
#endif
	CONTEXT Context;
	ULONG SizeOfContext;
	ULONG64 StackBase;
	ULONG64 StackEnd;
} MINIDUMP_THREAD_CALLBACK, * PMINIDUMP_THREAD_CALLBACK;

typedef struct _MINIDUMP_THREAD_EX_CALLBACK {
	ULONG ThreadId;
	HANDLE ThreadHandle;
#if defined(_ARM64_)
	ULONG Pad;
#endif
	CONTEXT Context;
	ULONG SizeOfContext;
	ULONG64 StackBase;
	ULONG64 StackEnd;
	ULONG64 BackingStoreBase;
	ULONG64 BackingStoreEnd;
} MINIDUMP_THREAD_EX_CALLBACK, * PMINIDUMP_THREAD_EX_CALLBACK;

typedef struct _MINIDUMP_MODULE_CALLBACK {
	PWCHAR FullPath;
	ULONG64 BaseOfImage;
	ULONG SizeOfImage;
	ULONG CheckSum;
	ULONG TimeDateStamp;
	VS_FIXEDFILEINFO VersionInfo;
	PVOID CvRecord;
	ULONG SizeOfCvRecord;
	PVOID MiscRecord;
	ULONG SizeOfMiscRecord;
} MINIDUMP_MODULE_CALLBACK, * PMINIDUMP_MODULE_CALLBACK;

typedef struct _MINIDUMP_INCLUDE_THREAD_CALLBACK {
	ULONG ThreadId;
} MINIDUMP_INCLUDE_THREAD_CALLBACK, * PMINIDUMP_INCLUDE_THREAD_CALLBACK;

typedef enum _THREAD_WRITE_FLAGS {
	ThreadWriteThread = 0x0001,
	ThreadWriteStack = 0x0002,
	ThreadWriteContext = 0x0004,
	ThreadWriteBackingStore = 0x0008,
	ThreadWriteInstructionWindow = 0x0010,
	ThreadWriteThreadData = 0x0020,
	ThreadWriteThreadInfo = 0x0040,
} THREAD_WRITE_FLAGS;

typedef struct _MINIDUMP_INCLUDE_MODULE_CALLBACK {
	ULONG64 BaseOfImage;
} MINIDUMP_INCLUDE_MODULE_CALLBACK, * PMINIDUMP_INCLUDE_MODULE_CALLBACK;

typedef struct _MINIDUMP_IO_CALLBACK {
	HANDLE Handle;
	ULONG64 Offset;
	PVOID Buffer;
	ULONG BufferBytes;
} MINIDUMP_IO_CALLBACK, * PMINIDUMP_IO_CALLBACK;

typedef struct _MINIDUMP_READ_MEMORY_FAILURE_CALLBACK
{
	ULONG64 Offset;
	ULONG Bytes;
	HRESULT FailureStatus;
} MINIDUMP_READ_MEMORY_FAILURE_CALLBACK,
* PMINIDUMP_READ_MEMORY_FAILURE_CALLBACK;

typedef struct _MINIDUMP_VM_QUERY_CALLBACK
{
	ULONG64 Offset;
} MINIDUMP_VM_QUERY_CALLBACK, * PMINIDUMP_VM_QUERY_CALLBACK;

typedef struct _MINIDUMP_VM_PRE_READ_CALLBACK
{
	ULONG64 Offset;
	PVOID Buffer;
	ULONG Size;
} MINIDUMP_VM_PRE_READ_CALLBACK, * PMINIDUMP_VM_PRE_READ_CALLBACK;

typedef struct _MINIDUMP_VM_POST_READ_CALLBACK
{
	ULONG64 Offset;
	PVOID Buffer;
	ULONG Size;
	ULONG Completed;
	HRESULT Status;
} MINIDUMP_VM_POST_READ_CALLBACK, * PMINIDUMP_VM_POST_READ_CALLBACK;

typedef struct _MINIDUMP_MEMORY_INFO {
	ULONG64 BaseAddress;
	ULONG64 AllocationBase;
	ULONG32 AllocationProtect;
	ULONG32 __alignment1;
	ULONG64 RegionSize;
	ULONG32 State;
	ULONG32 Protect;
	ULONG32 Type;
	ULONG32 __alignment2;
} MINIDUMP_MEMORY_INFO, * PMINIDUMP_MEMORY_INFO;

typedef struct _MINIDUMP_CALLBACK_INPUT {
	ULONG ProcessId;
	HANDLE ProcessHandle;
	ULONG CallbackType;
	union {
		HRESULT Status;
		MINIDUMP_THREAD_CALLBACK Thread;
		MINIDUMP_THREAD_EX_CALLBACK ThreadEx;
		MINIDUMP_MODULE_CALLBACK Module;
		MINIDUMP_INCLUDE_THREAD_CALLBACK IncludeThread;
		MINIDUMP_INCLUDE_MODULE_CALLBACK IncludeModule;
		MINIDUMP_IO_CALLBACK Io;
		MINIDUMP_READ_MEMORY_FAILURE_CALLBACK ReadMemoryFailure;
		ULONG SecondaryFlags;
		MINIDUMP_VM_QUERY_CALLBACK VmQuery;
		MINIDUMP_VM_PRE_READ_CALLBACK VmPreRead;
		MINIDUMP_VM_POST_READ_CALLBACK VmPostRead;
	};
} MINIDUMP_CALLBACK_INPUT, * PMINIDUMP_CALLBACK_INPUT;

typedef struct _MINIDUMP_CALLBACK_OUTPUT {
	union {
		ULONG ModuleWriteFlags;
		ULONG ThreadWriteFlags;
		ULONG SecondaryFlags;
		struct {
			ULONG64 MemoryBase;
			ULONG MemorySize;
		};
		struct {
			BOOL CheckCancel;
			BOOL Cancel;
		};
		HANDLE Handle;
		struct {
			MINIDUMP_MEMORY_INFO VmRegion;
			BOOL Continue;
		};
		struct {
			HRESULT VmQueryStatus;
			MINIDUMP_MEMORY_INFO VmQueryResult;
		};
		struct {
			HRESULT VmReadStatus;
			ULONG VmReadBytesCompleted;
		};
		HRESULT Status;
	};
} MINIDUMP_CALLBACK_OUTPUT, * PMINIDUMP_CALLBACK_OUTPUT;

typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
	DWORD ThreadId;
	PEXCEPTION_POINTERS ExceptionPointers;
	BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, * PMINIDUMP_EXCEPTION_INFORMATION;

typedef struct _MINIDUMP_USER_STREAM {
	ULONG32 Type;
	ULONG BufferSize;
	PVOID Buffer;

} MINIDUMP_USER_STREAM, * PMINIDUMP_USER_STREAM;

typedef struct _MINIDUMP_USER_STREAM_INFORMATION {
	ULONG UserStreamCount;
	PMINIDUMP_USER_STREAM UserStreamArray;
} MINIDUMP_USER_STREAM_INFORMATION, * PMINIDUMP_USER_STREAM_INFORMATION;

typedef
BOOL
(WINAPI* MINIDUMP_CALLBACK_ROUTINE) (
	PVOID CallbackParam,
	PMINIDUMP_CALLBACK_INPUT CallbackInput,
	PMINIDUMP_CALLBACK_OUTPUT CallbackOutput
	);

typedef struct _MINIDUMP_CALLBACK_INFORMATION {
	MINIDUMP_CALLBACK_ROUTINE CallbackRoutine;
	PVOID CallbackParam;
} MINIDUMP_CALLBACK_INFORMATION, * PMINIDUMP_CALLBACK_INFORMATION;

typedef enum _MINIDUMP_TYPE {
	MiniDumpNormal = 0x00000000,
	MiniDumpWithDataSegs = 0x00000001,
	MiniDumpWithFullMemory = 0x00000002,
	MiniDumpWithHandleData = 0x00000004,
	MiniDumpFilterMemory = 0x00000008,
	MiniDumpScanMemory = 0x00000010,
	MiniDumpWithUnloadedModules = 0x00000020,
	MiniDumpWithIndirectlyReferencedMemory = 0x00000040,
	MiniDumpFilterModulePaths = 0x00000080,
	MiniDumpWithProcessThreadData = 0x00000100,
	MiniDumpWithPrivateReadWriteMemory = 0x00000200,
	MiniDumpWithoutOptionalData = 0x00000400,
	MiniDumpWithFullMemoryInfo = 0x00000800,
	MiniDumpWithThreadInfo = 0x00001000,
	MiniDumpWithCodeSegs = 0x00002000,
	MiniDumpWithoutAuxiliaryState = 0x00004000,
	MiniDumpWithFullAuxiliaryState = 0x00008000,
	MiniDumpWithPrivateWriteCopyMemory = 0x00010000,
	MiniDumpIgnoreInaccessibleMemory = 0x00020000,
	MiniDumpWithTokenInformation = 0x00040000,
	MiniDumpWithModuleHeaders = 0x00080000,
	MiniDumpFilterTriage = 0x00100000,
	MiniDumpWithAvxXStateContext = 0x00200000,
	MiniDumpWithIptTrace = 0x00400000,
	MiniDumpScanInaccessiblePartialPages = 0x00800000,
	MiniDumpFilterWriteCombinedMemory = 0x01000000,
	MiniDumpValidTypeFlags = 0x01ffffff,
	MiniDumpNoIgnoreInaccessibleMemory = 0x02000000,
	MiniDumpValidTypeFlagsEx = 0x03ffffff,
} MINIDUMP_TYPE;

typedef enum _MODULE_WRITE_FLAGS {
	ModuleWriteModule = 0x0001,
	ModuleWriteDataSeg = 0x0002,
	ModuleWriteMiscRecord = 0x0004,
	ModuleWriteCvRecord = 0x0008,
	ModuleReferencedByMemory = 0x0010,
	ModuleWriteTlsData = 0x0020,
	ModuleWriteCodeSegs = 0x0040,
} MODULE_WRITE_FLAGS;

#include <poppack.h>
#endif
#endif
