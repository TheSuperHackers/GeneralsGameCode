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

// TheSuperHackers: Windows-specific implementation for wxWidgets DebugWindow plugin

#include "dbgwinapp.h"
#include "debugwindow_wx.h"
#include <wx/dynlib.h>

#include <process.h> // for _beginthreadex()

namespace
{
// Critical section that guards everything related to wxWidgets "main" thread
// startup or shutdown.
wxCriticalSection g_startupCS;
// Handle of wx "main" thread if running, nullptr otherwise
HANDLE g_dllThread = nullptr;

//  wx application startup code -- runs from its own thread
unsigned wxSTDCALL DllAppLauncher(void *event)
{
    // Note: The thread that called DllAppLauncher() holds g_startupCS
    //       at this point and won't release it until we signal it.

    // We need to pass correct HINSTANCE to wxEntry() and the right value is
    // HINSTANCE of this DLL, not of the main .exe, use this MSW-specific wx
    // function to get it. Notice that under Windows XP and later the name is
    // not needed/used as we retrieve the DLL handle from an address inside it
    // but you do need to use the correct name for this code to work with older
    // systems as well.
    const HINSTANCE hInstance = wxDynamicLibrary::MSWGetModuleHandle("DebugWindow", &g_dllThread);
    if (!hInstance) {
        return 0; // failed to get DLL's handle
    }

    // IMPLEMENT_WXWIN_MAIN does this as the first thing
    wxDISABLE_DEBUG_SUPPORT();

    // We do this before wxEntry() explicitly, even though wxEntry() would
    // do it too, so that we know when wx is initialized and can signal
    // DllAppLauncher() about it *before* starting the event loop.
    wxInitializer wxinit;
    if (!wxinit.IsOk()) {
        return 0; // failed to init wx
    }

    // Signal DllAppLauncher() that it can continue
    HANDLE hEvent = *(static_cast<HANDLE *>(event));
    if (!SetEvent(hEvent)) {
        return 0; // failed setting up the mutex
    }

    // Run the app:
    wxEntry(hInstance);

    return 1;
}

} // namespace

void CreateDebugDialog()
{
    // In order to prevent conflicts with hosting app's event loop, we
    // launch wx app from the DLL in its own thread.
    //
    // We can't even use wxInitializer: it initializes wxModules and one of
    // the modules it handles is wxThread's private module that remembers
    // ID of the main thread. But we need to fool wxWidgets into thinking that
    // the thread we are about to create now is the main thread, not the one
    // from which this function is called.
    //
    // Note that we cannot use wxThread here, because the wx library wasn't
    // initialized yet. wxCriticalSection is safe to use, though.

    wxCriticalSectionLocker lock(g_startupCS);

    if (!g_dllThread) {
        HANDLE hEvent = CreateEventA(nullptr, // default security attributes
            FALSE, // auto-reset
            FALSE, // initially non-signaled
            nullptr // anonymous
        );
        if (!hEvent) {
            return; // error
        }

        g_dllThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, // default security
            0, // default stack size
            &DllAppLauncher,
            &hEvent, // arguments
            0, // don't start suspended
            nullptr // we don't need thread ID
        ));

        if (!g_dllThread) {
            CloseHandle(hEvent);
            return; // error
        }

        // Wait until DLL thread initializes before returning
        WaitForSingleObject(hEvent, INFINITE);
        CloseHandle(hEvent);

        // Send a message to show the window
        wxThreadEvent *event = new wxThreadEvent(wxEVT_THREAD, CMD_SHOW_WINDOW);
        event->SetString("Debug Window");
        wxQueueEvent(wxApp::GetInstance(), event);
    }
}

void DestroyDebugDialog()
{
    wxCriticalSectionLocker lock(g_startupCS);

    if (!g_dllThread)
        return;

    // Send termination message to the thread
    wxThreadEvent *event = new wxThreadEvent(wxEVT_THREAD, CMD_TERMINATE);
    wxQueueEvent(wxApp::GetInstance(), event);

    // Wait for the DLL thread to terminate (should be fast)
    WaitForSingleObject(g_dllThread, INFINITE);
    CloseHandle(g_dllThread);
    g_dllThread = nullptr;
}