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
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WWDebug_Install_Message_Handler -- install function for handling the debug messages       *
 *   WWDebug_Install_Assert_Handler -- Install a function for handling the assert messages     *
 *   WWDebug_Install_Trigger_Handler -- install a trigger handler function                     *
 *   WWDebug_Printf -- Internal function for passing messages to installed handler             *
 *   WWDebug_Assert_Fail -- Internal function for passing assert messages to installed handler *
 *   WWDebug_Assert_Fail_Print -- Internal function, passes assert message to handler          *
 *   WWDebug_Check_Trigger -- calls the user-installed debug trigger handler                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "wwdebug.h"
#include <windows.h>
//#include "win.h" can use this if allowed to see wwlib
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include "Except.h"


static PrintFunc			_CurMessageHandler = NULL;
static AssertPrintFunc	_CurAssertHandler = NULL;
static TriggerFunc		_CurTriggerHandler = NULL;
static ProfileFunc		_CurProfileStartHandler = NULL;
static ProfileFunc		_CurProfileStopHandler = NULL;

// Convert the latest system error into a string and return a pointer to
// a static buffer containing the error string.

void Convert_System_Error_To_String(int id, char* buffer, int buf_len)
{
#ifndef _UNIX
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		id,
		0,
		buffer,
		buf_len,
		NULL);
#endif
}

int Get_Last_System_Error()
{
	return GetLastError();
}

/***********************************************************************************************
 * WWDebug_Install_Message_Handler -- install function for handling the debug messages         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
PrintFunc WWDebug_Install_Message_Handler(PrintFunc func)
{
	PrintFunc tmp = _CurMessageHandler;
	_CurMessageHandler = func;
	return tmp;
}


/***********************************************************************************************
 * WWDebug_Install_Assert_Handler -- Install a function for handling the assert messages       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
AssertPrintFunc WWDebug_Install_Assert_Handler(AssertPrintFunc func)
{
	AssertPrintFunc tmp = _CurAssertHandler;
	_CurAssertHandler = func;
	return tmp;
}


/***********************************************************************************************
 * WWDebug_Install_Trigger_Handler -- install a trigger handler function                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
TriggerFunc	WWDebug_Install_Trigger_Handler(TriggerFunc func)
{
	TriggerFunc tmp = _CurTriggerHandler;
	_CurTriggerHandler = func;
	return tmp;
}


/***********************************************************************************************
 * WWDebug_Install_Profile_Start_Handler -- install a profile handler function                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
ProfileFunc	WWDebug_Install_Profile_Start_Handler(ProfileFunc func)
{
	ProfileFunc tmp = _CurProfileStartHandler;
	_CurProfileStartHandler = func;
	return tmp;
}


/***********************************************************************************************
 * WWDebug_Install_Profile_Stop_Handler -- install a profile handler function                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
ProfileFunc	WWDebug_Install_Profile_Stop_Handler(ProfileFunc func)
{
	ProfileFunc tmp = _CurProfileStopHandler;
	_CurProfileStopHandler = func;
	return tmp;
}


/***********************************************************************************************
 * WWDebug_Printf -- Internal function for passing messages to installed handler               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/

void WWDebug_Printf(const char * format,...)
{
	if (_CurMessageHandler != NULL) {

		va_list	va;
		char buffer[4096];

		va_start(va, format);
		vsprintf(buffer, format, va);
		WWASSERT((strlen(buffer) < sizeof(buffer)));

		_CurMessageHandler(WWDEBUG_TYPE_INFORMATION, buffer);
		va_end(va);

	}
}

/***********************************************************************************************
 * WWDebug_Printf_Warning -- Internal function for passing messages to installed handler       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/

void WWDebug_Printf_Warning(const char * format,...)
{
	if (_CurMessageHandler != NULL) {

		va_list	va;
		char buffer[4096];

		va_start(va, format);
		vsprintf(buffer, format, va);
		WWASSERT((strlen(buffer) < sizeof(buffer)));

		_CurMessageHandler(WWDEBUG_TYPE_WARNING, buffer);
		va_end(va);

	}
}

/***********************************************************************************************
 * WWDebug_Printf_Error -- Internal function for passing messages to installed handler         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/

void WWDebug_Printf_Error(const char * format,...)
{
	if (_CurMessageHandler != NULL) {

		va_list	va;
		char buffer[4096];

		va_start(va, format);
		vsprintf(buffer, format, va);
		WWASSERT((strlen(buffer) < sizeof(buffer)));

		_CurMessageHandler(WWDEBUG_TYPE_ERROR, buffer);
		va_end(va);

	}
}

/***********************************************************************************************
 * WWDebug_Assert_Fail -- Internal function for passing assert messages to installed handler   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
#ifdef WWDEBUG
void WWDebug_Assert_Fail(const char * expr,const char * file, int line)
{
	if (_CurAssertHandler != NULL) {

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

      int code = MessageBoxA(NULL, assertbuf, "WWDebug_Assert_Fail", MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

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




/***********************************************************************************************
 * _assert -- Catch all asserts by overriding lib function                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Assert stuff                                                                      *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/11/2001 3:56PM ST : Created                                                            *
 *=============================================================================================*/
#if 0 //(gth) this is giving me link errors for some reason...

#ifndef W3D_MAX4
#ifdef WWDEBUG
void __cdecl _assert(void *expr, void *filename, unsigned lineno)
{
	WWDebug_Assert_Fail((const char*)expr, (const char*)filename, lineno);
}
#endif //WWDEBUG
#endif

#endif



/***********************************************************************************************
 * WWDebug_Assert_Fail_Print -- Internal function, passes assert message to handler            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
#ifdef WWDEBUG
void WWDebug_Assert_Fail_Print(const char * expr,const char * file, int line,const char * string)
{
	if (_CurAssertHandler != NULL) {

		char buffer[4096];
		sprintf(buffer,"%s (%d) Assert: %s %s\n",file,line,expr, string);
		_CurAssertHandler(buffer);

	} else {

		assert(0);

	}
}
#endif


/***********************************************************************************************
 * WWDebug_Check_Trigger -- calls the user-installed debug trigger handler                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool WWDebug_Check_Trigger(int trigger_num)
{
	if (_CurTriggerHandler != NULL) {
		return _CurTriggerHandler(trigger_num);
	} else {
		return false;
	}
}


/***********************************************************************************************
 * WWDebug_Profile_Start -- calls the user-installed profile start handler                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void WWDebug_Profile_Start( const char * title)
{
	if (_CurProfileStartHandler != NULL) {
		_CurProfileStartHandler( title );
	}
}


/***********************************************************************************************
 * WWDebug_Profile_Stop -- calls the user-installed profile start handler                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void WWDebug_Profile_Stop( const char * title)
{
	if (_CurProfileStopHandler != NULL) {
		_CurProfileStopHandler( title );
	}
}



#ifdef WWDEBUG
/***********************************************************************************************
 * WWDebug_DBWin32_Message_Handler --                                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/30/98    BMG : Created.                                                                *
 *=============================================================================================*/
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
        //MessageBox(NULL, "DBWIN_BUFFER_READY nonexistent", NULL, MB_OK);
        return;
    }

    /* get a handle to the data synch object */
    heventData = OpenEvent(EVENT_MODIFY_STATE, FALSE, "DBWIN_DATA_READY");
    if ( !heventData )
    {
        // MessageBox(NULL, "DBWIN_DATA_READY nonexistent", NULL, MB_OK);
        CloseHandle(heventDBWIN);
        return;
    }

    hSharedFile = CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0, 4096, "DBWIN_BUFFER");
    if (!hSharedFile)
    {
        //MessageBox(NULL, "DebugTrace: Unable to create file mapping object DBWIN_BUFFER", "Error", MB_OK);
        CloseHandle(heventDBWIN);
        CloseHandle(heventData);
        return;
    }

    lpszSharedMem = (LPSTR)MapViewOfFile(hSharedFile, FILE_MAP_WRITE, 0, 0, 512);
    if (!lpszSharedMem)
    {
        //MessageBox(NULL, "DebugTrace: Unable to map shared memory", "Error", MB_OK);
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

    return;
}
#endif // WWDEBUG
