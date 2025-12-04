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

#pragma once

#include <wx/app.h>

extern const int CMD_SHOW_WINDOW;
extern const int CMD_TERMINATE;
class DbgWinFrame;

class DbgWinApp : public wxApp
{
public:
    DbgWinApp();

    virtual bool OnInit() override;

    DbgWinFrame *Frame() { return m_frame; }

private:
    void OnShowWindow(wxThreadEvent &event);
    void OnTerminate(wxThreadEvent &event);

private:
    DbgWinFrame *m_frame;
};

wxDECLARE_APP(DbgWinApp);