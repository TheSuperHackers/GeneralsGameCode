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

// TheSuperHackers: wxWidgets window implementation for the DebugWindow plugin.

#include "dbgwinframe.h"
#include <wx/app.h>
#include <wx/utils.h>
#include <cstdio>

wxWindow *g_mainWindow;

DbgWinFrame::DbgWinFrame(wxWindow *parent, const wxString &label) :
    DbgWinBaseFrame(parent),
    m_variablesCache(),
    m_nextVarIndex(0),
    m_currentFrame(0),
    m_pauseFrame(-1),
    m_numberOfStepsAllowed(-1),
    m_blockVariableUpdate(false),
    m_runFast(false)
{
    g_mainWindow = parent;
    SetPosition({ 0, 0 });
    
    // Bind event handlers - equivalent to MFC message map
    Bind(wxEVT_BUTTON, &DbgWinFrame::On_Step, this, XRCID("m_stepBtn"));
    Bind(wxEVT_BUTTON, &DbgWinFrame::On_Step_Ten, this, XRCID("m_stepTenBtn"));
    Bind(wxEVT_BUTTON, &DbgWinFrame::On_Clear, this, XRCID("m_clearBtn"));
    Bind(wxEVT_SCROLLWIN_THUMBTRACK, &DbgWinFrame::On_Thumb, this, XRCID("m_variableList"));
    Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &DbgWinFrame::On_Thumb_Release, this, XRCID("m_variableList"));
    Bind(wxEVT_CLOSE_WINDOW, &DbgWinFrame::On_Exit, this);
}

void DbgWinFrame::On_Step(wxCommandEvent &event)
{
    // Equivalent to original OnStep() - allow one frame then pause
    m_numberOfStepsAllowed = 1;
    Set_Paused(false);
}

void DbgWinFrame::On_Step_Ten(wxCommandEvent &event)
{
    // Equivalent to original OnStepTen() - allow ten frames then pause
    m_numberOfStepsAllowed = 10;
    Set_Paused(false);
}

void DbgWinFrame::On_Clear(wxCommandEvent &event)
{
    // Equivalent to original OnClearWindows()
    m_variablesCache.clear();
    m_variableList->Clear();
    m_nextVarIndex = 0;
    m_msgTxt->Clear();
}

void DbgWinFrame::On_Exit(wxCloseEvent &event)
{
    // Equivalent to original OnClose() - minimize instead of closing
    Iconize();
}

void DbgWinFrame::On_Thumb(wxScrollWinEvent &event)
{
    if (!m_blockVariableUpdate) {
        m_blockVariableUpdate = true;
    }
}

void DbgWinFrame::On_Thumb_Release(wxScrollWinEvent &event)
{
    m_blockVariableUpdate = false;
}

void DbgWinFrame::Set_Frame_Number(int frame)
{
    // Equivalent to original SetFrameNumber()
    char buf[32];
    std::snprintf(buf, sizeof(buf), "Frame %9d", frame);
    m_frameTxt->SetLabel(buf);
    m_frameTxt->Refresh();
    m_currentFrame = frame;

    // Handle stepping logic
    if (m_numberOfStepsAllowed > 0) {
        m_numberOfStepsAllowed--;
        if (m_numberOfStepsAllowed == 0) {
            Set_Paused(true);
        }
    }
}

void DbgWinFrame::Append_Variable(const char *var, const char *val)
{
    // Equivalent to original AdjustVariable()
    wxString str = var;
    str += " = ";
    str += val;

    while (m_blockVariableUpdate) {
        wxMilliSleep(1);
    }

    auto it = m_variablesCache.find(var);
    if (it == m_variablesCache.end()) {
        unsigned index = m_nextVarIndex++;
        m_variablesCache[var] = index;
        m_variableList->Insert(str, index);
    } else {
        m_variableList->SetString(it->second, str);
    }
}

void DbgWinFrame::Append_Message(const char *msg)
{
    // Equivalent to original AppendMessage()
    wxString str = msg;
    str += "\n";
    m_msgTxt->AppendText(str);
}

bool DbgWinFrame::Paused()
{
    // Equivalent to original CanProceed() logic
    return m_pauseBtn->GetValue();
}

bool DbgWinFrame::Run_Fast()
{
    // Equivalent to original RunAppFast()
    return m_runFastXTenBtn->GetValue();
}

void DbgWinFrame::Set_Paused(bool paused)
{
    // Equivalent to original _UpdatePauseButton() and pause state management
    m_pauseBtn->SetValue(paused);
    if (!paused) {
        // Reset step counter when unpausing (unless it's already counting down)
        if (m_numberOfStepsAllowed == 0) {
            m_numberOfStepsAllowed = -1;  // Go forever
        }
    }
}

bool DbgWinFrame::Can_Proceed()
{
    // Equivalent to original CanProceed()
    if (m_numberOfStepsAllowed == 0) {
        return false;  // Explicitly stopped
    }
    return !Paused();
}

void DbgWinFrame::Force_Pause()
{
    // Equivalent to original ForcePause()
    Set_Paused(true);
    m_numberOfStepsAllowed = 0;
}

void DbgWinFrame::Force_Continue()
{
    // Equivalent to original ForceContinue()
    Set_Paused(false);
    m_numberOfStepsAllowed = -1;  // Go forever
}

void DbgWinFrame::_UpdatePauseButton()
{
    // This was a private method in the original - now handled in Set_Paused()
}