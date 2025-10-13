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

// TheSuperHackers: POSIX-specific implementation for wxWidgets DebugWindow plugin

#include "debugwindow_wx.h"
#include "dbgwinapp.h"
#include <wx/app.h>
#include <pthread.h>

namespace
{
pthread_t g_dllThread = 0;
bool g_threadRunning = false;

void* DllAppLauncher(void* arg)
{
    // Initialize wxWidgets
    wxInitializer wxinit;
    if (!wxinit.IsOk()) {
        return nullptr;
    }

    // Create and run the app
    wxApp::SetInstance(new DbgWinApp());
    
    if (wxApp::GetInstance()->OnInit()) {
        // Send a message to show the window
        wxThreadEvent *event = new wxThreadEvent(wxEVT_THREAD, CMD_SHOW_WINDOW);
        event->SetString("Debug Window");
        wxQueueEvent(wxApp::GetInstance(), event);
        
        // Run the main loop
        wxApp::GetInstance()->OnRun();
    }
    
    wxApp::GetInstance()->OnExit();
    wxApp::SetInstance(nullptr);
    
    g_threadRunning = false;
    return nullptr;
}

} // namespace

void CreateDebugDialog()
{
    if (!g_threadRunning) {
        g_threadRunning = true;
        
        if (pthread_create(&g_dllThread, nullptr, DllAppLauncher, nullptr) != 0) {
            g_threadRunning = false;
        }
    }
}

void DestroyDebugDialog()
{
    if (g_threadRunning) {
        // Send termination message to the thread
        if (wxApp::GetInstance()) {
            wxThreadEvent *event = new wxThreadEvent(wxEVT_THREAD, CMD_TERMINATE);
            wxQueueEvent(wxApp::GetInstance(), event);
        }
        
        // Wait for thread to finish
        pthread_join(g_dllThread, nullptr);
        g_threadRunning = false;
    }
}