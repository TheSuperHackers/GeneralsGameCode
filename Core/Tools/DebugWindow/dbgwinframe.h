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

#pragma once

#include <wx/button.h>
#include <wx/frame.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/xrc/xmlres.h>

#include "wxdbgwinui.h"

#include <atomic>
#include <map>
#include <string>
#include <vector>

extern wxWindow *g_mainWindow;

/**
 * The base class here is defined in an XRC resource file which is compiled into the binary by the build system.
 */
class DbgWinFrame : public DbgWinBaseFrame
{
public:
    DbgWinFrame(wxWindow *parent, const wxString &label);

    // wx Event handlers
    void On_Step(wxCommandEvent &event);
    void On_Step_Ten(wxCommandEvent &event);
    void On_Clear(wxCommandEvent &event);
    void On_Exit(wxCloseEvent &event);
    void On_Thumb(wxScrollWinEvent &event);
    void On_Thumb_Release(wxScrollWinEvent &event);

    // C API Handlers - equivalent to original MFC DebugWindowDialog methods
    void Set_Frame_Number(int frame);
    void Append_Variable(const char *var, const char *val);
    void Append_Message(const char *msg);
    bool Paused();
    bool Run_Fast();
    void Set_Paused(bool paused);
    bool Can_Proceed();
    void Force_Pause();
    void Force_Continue();

private:
    // Variable management - equivalent to original VecPairString mVariables
    std::map<wxString, int> m_variablesCache;
    std::atomic<unsigned> m_nextVarIndex;
    
    // Frame and stepping control - equivalent to original mNumberOfStepsAllowed, mStepping
    std::atomic<int> m_currentFrame;
    std::atomic<int> m_pauseFrame;
    std::atomic<int> m_numberOfStepsAllowed;  // -1 = go forever, 0 = stop now, positive = countdown
    
    // Thread synchronization
    std::atomic<bool> m_blockVariableUpdate;
    
    // State flags - equivalent to original mRunFast
    std::atomic<bool> m_runFast;
    
    void _UpdatePauseButton();
};