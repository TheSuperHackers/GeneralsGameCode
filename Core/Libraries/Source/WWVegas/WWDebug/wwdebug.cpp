/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWDebug                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwdebug/wwdebug.cpp                          $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/13/02 1:46p                                               $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "wwdebug.h"
//#include "win.h" can use this if allowed to see wwlib
#include <stdlib.h>
#include <stdarg.h>
#include <Utility/stdio_adapter.h>
#include <assert.h>
#include <signal.h>
#include "WWLib/Except.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#endif

static PrintFunc			_CurMessageHandler = nullptr;
static AssertPrintFunc	_CurAssertHandler = nullptr;
static TriggerFunc		_CurTriggerHandler = nullptr;
static ProfileFunc		_CurProfileStartHandler = nullptr;
static ProfileFunc		_CurProfileStopHandler = nullptr;

// Convert the latest system error into a string and return a pointer to
// a static buffer containing the error string.

void Convert_System_Error_To_String(int id, char* buffer, int buf_len)
{
#ifndef _UNIX
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr,
		id,
		0,
		buffer,
		buf_len,
		nullptr);
#endif
}

int Get_Last_System_Error()
{
#ifdef _WIN32
	return GetLastError();
#else
	return errno;
#endif
}

PrintFunc WWDebug_Install_Message_Handler(PrintFunc func)
{
	PrintFunc tmp = _CurMessageHandler;
	_CurMessageHandler = func;
	return tmp;
}


AssertPrintFunc WWDebug_Install_Assert_Handler(AssertPrintFunc func)
{
	AssertPrintFunc tmp = _CurAssertHandler;
	_CurAssertHandler = func;
	return tmp;
}


TriggerFunc	WWDebug_Install_Trigger_Handler(TriggerFunc func)
{
	TriggerFunc tmp = _CurTriggerHandler;
	_CurTriggerHandler = func;
	return tmp;
}


ProfileFunc	WWDebug_Install_Profile_Start_Handler(ProfileFunc func)
{
	ProfileFunc tmp = _CurProfileStartHandler;
	_CurProfileStartHandler = func;
	return tmp;
}


ProfileFunc	WWDebug_Install_Profile_Stop_Handler(ProfileFunc func)
{
	ProfileFunc tmp = _CurProfileStopHandler;
	_CurProfileStopHandler = func;
	return tmp;
}


// Internal function for passing messages to installed handler.

void WWDebug_Printf(const char * format,...)
{
	if (_CurMessageHandler != nullptr) {

		va_list	va;
		char buffer[4096];

		va_start(va, format);
		vsprintf(buffer, format, va);
		WWASSERT((strlen(buffer) < sizeof(buffer)));

		_CurMessageHandler(WWDEBUG_TYPE_INFORMATION, buffer);
		va_end(va);

	}
}

// Internal function for passing messages to installed handler.

void WWDebug_Printf_Warning(const char * format,...)
{
	if (_CurMessageHandler != nullptr) {

		va_list	va;
		char buffer[4096];

		va_start(va, format);
		vsprintf(buffer, format, va);
		WWASSERT((strlen(buffer) < sizeof(buffer)));

		_CurMessageHandler(WWDEBUG_TYPE_WARNING, buffer);
		va_end(va);

	}
}

// Internal function for passing messages to installed handler.

void WWDebug_Printf_Error(const char * format,...)
{
	if (_CurMessageHandler != nullptr) {

		va_list	va;
		char buffer[4096];

		va_start(va, format);
		vsprintf(buffer, format, va);
		WWASSERT((strlen(buffer) < sizeof(buffer)));

		_CurMessageHandler(WWDEBUG_TYPE_ERROR, buffer);
		va_end(va);

	}
}

// Internal function for passing assert messages to installed handler.
#ifdef WWDEBUG
void WWDebug_Assert_Fail(const char * expr,const char * file, int line)
{
	if (_CurAssertHandler != nullptr) {

		char buffer[4096];
		sprintf(buffer,"%s (%d) Assert: %s\n",file,line,expr);
		_CurAssertHandler(buffer);

	} else {

		/*
		// If the exception handler is try to quit the game then don't show an assert.
		*/
		if (Is_Trying_To_Exit()) {
			ExitProcess(0);
		}

      char assertbuf[4096];
		sprintf(assertbuf, "Assert failed\n\n. File %s Line %d", file, line);

      int code = MessageBoxA(nullptr, assertbuf, "WWDebug_Assert_Fail", MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

      if (code == IDABORT) {
      	raise(SIGABRT);
      	_exit(3);
      }

		if (code == IDRETRY) {
			WWDEBUG_BREAK
      	return;
		}
   }
}
#endif



// Internal function, passes assert message to handler.
#ifdef WWDEBUG
void WWDebug_Assert_Fail_Print(const char * expr,const char * file, int line,const char * string)
{
	if (_CurAssertHandler != nullptr) {

		char buffer[4096];
		sprintf(buffer,"%s (%d) Assert: %s %s\n",file,line,expr, string);
		_CurAssertHandler(buffer);

	} else {

		assert(0);

	}
}
#endif


// Calls the user-installed debug trigger handler.
bool WWDebug_Check_Trigger(int trigger_num)
{
	if (_CurTriggerHandler != nullptr) {
		return _CurTriggerHandler(trigger_num);
	} else {
		return false;
	}
}


// Calls the user-installed profile start handler.
void WWDebug_Profile_Start( const char * title)
{
	if (_CurProfileStartHandler != nullptr) {
		_CurProfileStartHandler( title );
	}
}


// Calls the user-installed profile stop handler.
void WWDebug_Profile_Stop( const char * title)
{
	if (_CurProfileStopHandler != nullptr) {
		_CurProfileStopHandler( title );
	}
}



#ifdef WWDEBUG
void WWDebug_DBWin32_Message_Handler( const char * str )
{

    HANDLE heventDBWIN;  /* DBWIN32 synchronization object */
    HANDLE heventData;   /* data passing synch object */
    HANDLE hSharedFile;  /* memory mapped file shared data */
    LPSTR lpszSharedMem;

    /* make sure DBWIN is open and waiting */
    heventDBWIN = OpenEvent(EVENT_MODIFY_STATE, FALSE, "DBWIN_BUFFER_READY");
    if ( !heventDBWIN )
    {
        //MessageBox(nullptr, "DBWIN_BUFFER_READY nonexistent", nullptr, MB_OK);
        return;
    }

    /* get a handle to the data synch object */
    heventData = OpenEvent(EVENT_MODIFY_STATE, FALSE, "DBWIN_DATA_READY");
    if ( !heventData )
    {
        // MessageBox(nullptr, "DBWIN_DATA_READY nonexistent", nullptr, MB_OK);
        CloseHandle(heventDBWIN);
        return;
    }

    hSharedFile = CreateFileMapping((HANDLE)-1, nullptr, PAGE_READWRITE, 0, 4096, "DBWIN_BUFFER");
    if (!hSharedFile)
    {
        //MessageBox(nullptr, "DebugTrace: Unable to create file mapping object DBWIN_BUFFER", "Error", MB_OK);
        CloseHandle(heventDBWIN);
        CloseHandle(heventData);
        return;
    }

    lpszSharedMem = (LPSTR)MapViewOfFile(hSharedFile, FILE_MAP_WRITE, 0, 0, 512);
    if (!lpszSharedMem)
    {
        //MessageBox(nullptr, "DebugTrace: Unable to map shared memory", "Error", MB_OK);
        CloseHandle(heventDBWIN);
        CloseHandle(heventData);
        return;
    }

    /* wait for buffer event */
    WaitForSingleObject(heventDBWIN, INFINITE);

    /* write it to the shared memory */
    *((LPDWORD)lpszSharedMem) = 0;
    wsprintf(lpszSharedMem + sizeof(DWORD), "%s", str);

    /* signal data ready event */
    SetEvent(heventData);

    /* clean up handles */
    CloseHandle(hSharedFile);
    CloseHandle(heventData);
    CloseHandle(heventDBWIN);
}
#endif // WWDEBUG
