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

// TheSuperHackers: wxWidgets application implementation for the DebugWindow plugin.

#include "dbgwinapp.h"
#include "dbgwinframe.h"

#include <wx/xrc/xh_all.h>
#include <wx/xrc/xmlres.h>

const int CMD_SHOW_WINDOW = wxNewId();
const int CMD_TERMINATE = wxNewId();

DbgWinApp::DbgWinApp() : m_frame(nullptr)
{
    // Keep the wx "main" thread running even without windows. This greatly
    // simplifies threads handling, because we don't have to correctly
    // implement wx-thread restarting.
    //
    // Note that this only works if you don't explicitly call ExitMainLoop(),
    // except in reaction to wx_dll_cleanup()'s message. wx_dll_cleanup()
    // relies on the availability of wxApp instance and if the event loop
    // terminated, wxEntry() would return and wxApp instance would be
    // destroyed.
    //
    // Also note that this is efficient, because if there are no windows, the
    // thread will sleep waiting for a new event. We could safe some memory
    // by shutting the thread down when it's no longer needed, though.
    SetExitOnFrameDelete(false);

    Connect(CMD_SHOW_WINDOW, wxEVT_THREAD, wxThreadEventHandler(DbgWinApp::OnShowWindow));
    Connect(CMD_TERMINATE, wxEVT_THREAD, wxThreadEventHandler(DbgWinApp::OnTerminate));
}

bool DbgWinApp::OnInit()
{
    wxXmlResource *res = wxXmlResource::Get();
    res->AddHandler(new wxUnknownWidgetXmlHandler);
    res->AddHandler(new wxPanelXmlHandler);
    res->AddHandler(new wxSizerXmlHandler);
    res->AddHandler(new wxFrameXmlHandler);
    res->AddHandler(new wxScrolledWindowXmlHandler);
    res->AddHandler(new wxStaticTextXmlHandler);
    res->AddHandler(new wxTextCtrlXmlHandler);
    res->AddHandler(new wxButtonXmlHandler);
    res->AddHandler(new wxToggleButtonXmlHandler);
    res->AddHandler(new wxListBoxXmlHandler);
    InitXmlResource();

    return true;
}

void DbgWinApp::OnShowWindow(wxThreadEvent &event)
{
    m_frame = new DbgWinFrame(nullptr, event.GetString());
    m_frame->Show(true);
}

void DbgWinApp::OnTerminate(wxThreadEvent &WXUNUSED(event))
{
    ExitMainLoop();
}

// we can't have WinMain() in a DLL and want to start the app ourselves
wxIMPLEMENT_APP_NO_MAIN(DbgWinApp);