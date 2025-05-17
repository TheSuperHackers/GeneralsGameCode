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

// DebugWindow.cpp : Defines the initialization routines for the DLL.
//

#include "StdAfx.h"
#include "DebugWindow.h"
#include "DebugWindowDialog.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CDebugWindowApp

BEGIN_MESSAGE_MAP(CDebugWindowApp, CWinApp)
	//{{AFX_MSG_MAP(CDebugWindowApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugWindowApp construction

CDebugWindowApp::CDebugWindowApp()
{
	AfxInitialize(true);
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	m_DialogWindow = NULL;
}

DebugWindowDialog* CDebugWindowApp::GetDialogWindow(void)
{
	return m_DialogWindow;
}

void CDebugWindowApp::SetDialogWindow(DebugWindowDialog* pWnd)
{
	m_DialogWindow = pWnd;
}

CDebugWindowApp::~CDebugWindowApp()
{
}


/////////////////////////////////////////////////////////////////////////////
// The one and only CDebugWindowApp object

CDebugWindowApp theApp;

void __declspec(dllexport) CreateDebugDialog(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	DebugWindowDialog* tmpWnd;
	tmpWnd = new DebugWindowDialog;
	tmpWnd->Create(DebugWindowDialog::IDD);
	tmpWnd->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	tmpWnd->ShowWindow(SW_SHOW);
	if (tmpWnd->GetMainWndHWND()) {
		SetFocus(tmpWnd->GetMainWndHWND());
	}
	
	theApp.SetDialogWindow(tmpWnd);
}

void __declspec(dllexport) DestroyDebugDialog(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
	
	if (tmpWnd) {
		tmpWnd->DestroyWindow();
		delete tmpWnd;
		theApp.SetDialogWindow(NULL);
	}
	
}

bool __declspec(dllexport) CanAppContinue(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	DebugWindowDialog* pDbg;
	pDbg = theApp.GetDialogWindow();
	if (!pDbg) {
		return true;
	}
	
	return pDbg->CanProceed();
}

void __declspec(dllexport) ForceAppContinue(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	DebugWindowDialog* pDbg;
	pDbg = theApp.GetDialogWindow();
	if (!pDbg) {
		return;
	}
	
	pDbg->ForceContinue();
}

bool __declspec(dllexport) RunAppFast(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	DebugWindowDialog* pDbg;
	pDbg = theApp.GetDialogWindow();
	if (!pDbg) {
		return true;
	}
	
	return pDbg->RunAppFast();
}

void __declspec(dllexport) AppendMessage(const char* messageToPass)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	DebugWindowDialog* pDbg;
	pDbg = theApp.GetDialogWindow();

	if (pDbg) {
		pDbg->AppendMessage(messageToPass);
	}
}

void __declspec(dllexport) SetFrameNumber(int frameNumber)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	DebugWindowDialog* pDbg;
	pDbg = theApp.GetDialogWindow();

	if (pDbg) {
		pDbg->SetFrameNumber(frameNumber);
	}
}


void __declspec(dllexport) AppendMessageAndPause(const char* messageToPass)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	DebugWindowDialog* pDbg;
	pDbg = theApp.GetDialogWindow();

	if (pDbg) {
		pDbg->AppendMessage(messageToPass);
		pDbg->ForcePause();
	}
}


void __declspec(dllexport) AdjustVariable(const char* variable, const char* value)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	DebugWindowDialog* pDbg;
	pDbg = theApp.GetDialogWindow();

	if (pDbg) {
		pDbg->AdjustVariable(variable, value);
	}
}

void __declspec(dllexport) AdjustVariableAndPause(const char* variable, const char* value)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	DebugWindowDialog* pDbg;
	pDbg = theApp.GetDialogWindow();

	if (pDbg) {
		pDbg->AdjustVariable(variable, value);
		pDbg->ForcePause();
	}
}