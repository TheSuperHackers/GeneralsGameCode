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

// TheSuperHackers: XRC-generated base class for DebugWindow UI

#pragma once

#include <wx/button.h>
#include <wx/frame.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/xrc/xmlres.h>

void InitXmlResource();

class DbgWinBaseFrame : public wxFrame
{
public:
    DbgWinBaseFrame() {}
    DbgWinBaseFrame(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString &title = wxEmptyString,
        const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE, const wxString &name = wxFrameNameStr)
    {
        Create(parent, id, title, pos, size, style, name);
    }

    bool Create(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString &title = wxEmptyString,
        const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE, const wxString &name = wxFrameNameStr)
    {
        if (!wxXmlResource::Get()->LoadFrame(this, parent, "DbgWinBaseFrame")) {
            return false;
        }

        // Get references to all controls
        m_panel = XRCCTRL(*this, "m_panel", wxPanel);
        m_pauseBtn = XRCCTRL(*this, "m_pauseBtn", wxToggleButton);
        m_stepBtn = XRCCTRL(*this, "m_stepBtn", wxButton);
        m_stepTenBtn = XRCCTRL(*this, "m_stepTenBtn", wxButton);
        m_frameLabel = XRCCTRL(*this, "m_frameLabel", wxStaticText);
        m_frameTxt = XRCCTRL(*this, "m_frameTxt", wxStaticText);
        m_runFastXTenBtn = XRCCTRL(*this, "m_runFastXTenBtn", wxToggleButton);
        m_clearBtn = XRCCTRL(*this, "m_clearBtn", wxButton);
        m_variablesLabel = XRCCTRL(*this, "m_variablesLabel", wxStaticText);
        m_variableList = XRCCTRL(*this, "m_variableList", wxListBox);
        m_messagesLabel = XRCCTRL(*this, "m_messagesLabel", wxStaticText);
        m_msgTxt = XRCCTRL(*this, "m_msgTxt", wxTextCtrl);

        return true;
    }

protected:
    wxPanel *m_panel;
    wxToggleButton *m_pauseBtn;
    wxButton *m_stepBtn;
    wxButton *m_stepTenBtn;
    wxStaticText *m_frameLabel;
    wxStaticText *m_frameTxt;
    wxToggleButton *m_runFastXTenBtn;
    wxButton *m_clearBtn;
    wxStaticText *m_variablesLabel;
    wxListBox *m_variableList;
    wxStaticText *m_messagesLabel;
    wxTextCtrl *m_msgTxt;
};