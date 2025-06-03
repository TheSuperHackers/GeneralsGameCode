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

#include "StdAfx.h"
#include "DebugWindowDialog.h"

DebugWindowDialog::DebugWindowDialog(UINT nIDTemplate, CWnd* pParentWnd) : 
	CDialog(nIDTemplate, pParentWnd)
{
	mStepping = false;
	mRunFast = false;
	mNumberOfStepsAllowed = -1;
	mMainWndHWND = ::FindWindow(NULL, "Command & Conquer: Generals");
}

int DebugWindowDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return CDialog::OnCreate(lpCreateStruct);
}

HWND DebugWindowDialog::GetMainWndHWND(void)
{
	return mMainWndHWND;
}

void DebugWindowDialog::ForcePause(void)
{
	mNumberOfStepsAllowed = 0;
	_UpdatePauseButton();
}

void DebugWindowDialog::ForceContinue(void)
{
	mNumberOfStepsAllowed = -1;
	_UpdatePauseButton();
}

void DebugWindowDialog::OnPause()
{
	if (mNumberOfStepsAllowed < 0) {
		mNumberOfStepsAllowed = 0;
	} else {
		mNumberOfStepsAllowed = -1;
	}
	_UpdatePauseButton();
}

void DebugWindowDialog::OnStep()
{
	mStepping = true;
	mNumberOfStepsAllowed = 1;
	_UpdatePauseButton();
}

void DebugWindowDialog::OnRunFast()
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_RUN_FAST);
	mRunFast = pButton->GetCheck() == 1;
}

void DebugWindowDialog::OnStepTen()
{
	mStepping = true;
	mNumberOfStepsAllowed = 10;
	_UpdatePauseButton();
}

void DebugWindowDialog::OnClearWindows()
{
	mMessages.clear();
	mVariables.clear();
	_RebuildMesgString();
	_RebuildVarsString();
}

bool DebugWindowDialog::CanProceed(void)
{
	if (mNumberOfStepsAllowed < 0) {
		return true;
	} else if (mNumberOfStepsAllowed == 0) {
		if (mStepping) {
			mStepping = false;
			_UpdatePauseButton();
		}
		return false;
	}

	--mNumberOfStepsAllowed;
	return true;
}

bool DebugWindowDialog::RunAppFast(void)
{
	return mRunFast;
}

void DebugWindowDialog::AppendMessage(const std::string& messageToAppend)
{
	mMessages.push_back(messageToAppend);
	_RebuildMesgString();
}

void DebugWindowDialog::AdjustVariable(const std::string& varName, const std::string& varValue)
{
	for (VecPairStringIt it = mVariables.begin(); it != mVariables.end(); it++) {
		if (it->first == varName) { 
			it->second = varValue;
			_RebuildVarsString();
			return;
		}
	}
	
	PairString newPair;
	newPair.first = varName;
	newPair.second = varValue;
	mVariables.push_back(newPair);

	_RebuildVarsString();
}

void DebugWindowDialog::SetFrameNumber(int frameNumber)
{
	static int numDigits;
	numDigits = frameNumber / 10 + 2;	// 1 for 1 additional digit, 1 for \0

	mFrameNumber.resize(numDigits);
	sprintf(&mFrameNumber[0], "%d", frameNumber);

	CWnd *pWnd = GetDlgItem(IDC_FrameNumber);
	if (pWnd) {
		pWnd->SetWindowText(mFrameNumber.c_str());
	}
}

void DebugWindowDialog::_RebuildVarsString(void)
{
	int cursorPosBeg, cursorPosEnd;
	((CEdit*)GetDlgItem(IDC_Variables))->GetSel(cursorPosBeg, cursorPosEnd);
	mVariablesString = "";

	for (VecPairStringIt it = mVariables.begin(); it != mVariables.end(); it++) {
		mVariablesString += it->first;
		mVariablesString += " = ";
		mVariablesString += it->second;
		mVariablesString += "\r\n";
	}



	// Push the new string
	mVariablesDisplayString = mVariablesString.c_str();
	GetDlgItem(IDC_Variables)->SetWindowText(mVariablesDisplayString);
	((CEdit*)GetDlgItem(IDC_Variables))->GetSel(cursorPosBeg, cursorPosEnd);
}

void DebugWindowDialog::_RebuildMesgString(void)
{
	mMessagesString = "";

	for (VecStringIt it = mMessages.begin(); it != mMessages.end(); it++) {
		mMessagesString += (*it);
		mMessagesString += "\r\n";
	}

	// Push the new string
	mMessagesDisplayString = mMessagesString.c_str();
	GetDlgItem(IDC_Messages)->SetWindowText(mMessagesDisplayString);
	((CEdit*)GetDlgItem(IDC_Messages))->SetSel(mMessagesString.length(), mMessagesString.length(), false);
}

void DebugWindowDialog::_UpdatePauseButton(void)
{
	// huh huh huhuh he said pButt
	CButton* pButt = (CButton*) GetDlgItem(IDC_Pause);

	if (!pButt) {
		return;
	}

	// The state should of the button should reflect !mNumberOfStepsAllowed
	pButt->SetCheck((mNumberOfStepsAllowed ? FALSE : TRUE));
}


void DebugWindowDialog::OnClose()
{
	ShowWindow(SW_MINIMIZE);
}

void DebugWindowDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (nType == SIZE_MINIMIZED) {
		if (mMainWndHWND) {
			::SetFocus(mMainWndHWND);
		}
	}
}



BEGIN_MESSAGE_MAP(DebugWindowDialog, CDialog)
	ON_WM_CREATE( )
	ON_BN_CLICKED(IDC_Pause, OnPause)
	ON_BN_CLICKED(IDC_Step, OnStep)
	ON_BN_CLICKED(IDC_RUN_FAST, OnRunFast)
	ON_BN_CLICKED(IDC_StepTen, OnStepTen)
	ON_BN_CLICKED(IDC_ClearWindows, OnClearWindows)
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()