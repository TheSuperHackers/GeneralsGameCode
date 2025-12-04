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

// TheSuperHackers: C Interface for wxWidgets DebugWindow plugin - maintains compatibility with original API

#include "debugwindow_wx.h"
#include "dbgwinapp.h"
#include "dbgwinframe.h"
#include <cstdio>

void AdjustVariable(const char *var, const char *val)
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Append_Variable(var, val);
    }
}

void AdjustVariableAndPause(const char *var, const char *val)
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Append_Variable(var, val);
        wxframe->Force_Pause();
    }
}

void AppendMessage(const char *msg)
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Append_Message(msg);
    }
}

void AppendMessageAndPause(const char *msg)
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Append_Message(msg);
        wxframe->Force_Pause();
    }
}

bool CanAppContinue()
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        return wxframe->Can_Proceed();
    }

    return true;
}

void ForceAppContinue()
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Force_Continue();
    }
}

bool RunAppFast()
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        return wxframe->Run_Fast();
    }

    return false;
}

void SetFrameNumber(int frame)
{
    DbgWinFrame *wxframe = wxGetApp().Frame();

    if (wxframe != nullptr) {
        wxframe->Set_Frame_Number(frame);
    }
}