/*
**	Command & Conquer Generals(tm)
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
#include "ParticleEditor.h"
#include "ParticleEditorDialog.h"

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

void __declspec(dllexport) CreateParticleSystemDialog(void)
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

		DebugWindowDialog* tmpWnd;
		tmpWnd = new DebugWindowDialog;
		tmpWnd->Create(DebugWindowDialog::IDD, NULL);
		
		tmpWnd->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		tmpWnd->InitPanel();
		tmpWnd->ShowWindow(SW_SHOW);
		if (tmpWnd->GetMainWndHWND()) {
			SetFocus(tmpWnd->GetMainWndHWND());
		}
		
		theApp.SetDialogWindow(tmpWnd);
	} catch (...) { }
}

void __declspec(dllexport) DestroyParticleSystemDialog(void)
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->DestroyWindow();
			delete tmpWnd;
			theApp.SetDialogWindow(NULL);
		}
	} catch (...) { }
}

void __declspec(dllexport) RemoveAllParticleSystems(void)
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->clearAllParticleSystems();
		}
	} catch (...) { }
}

void __declspec(dllexport) AppendParticleSystem(const char* particleSystemName)
{
	try{
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->addParticleSystem(particleSystemName);
		}
	} catch (...) { }
}

void __declspec(dllexport) RemoveAllThingTemplates( void )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->clearAllThingTemplates();
		} 
	} catch (...) { }
}


void __declspec(dllexport) AppendThingTemplate( const char* thingTemplateName )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->addThingTemplate(thingTemplateName);
		}
	} catch (...) {	}
}


Bool __declspec(dllexport) HasUpdatedSelectedParticleSystem( void )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			return tmpWnd->hasSelectionChanged();
		}
	} catch (...) {	}

	return false;
}

void __declspec(dllexport) GetSelectedParticleSystemName( char *bufferToCopyInto )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->getSelectedSystemName(bufferToCopyInto);
		}
	} catch (...) {	}
}

void __declspec(dllexport) UpdateCurrentParticleCap( int currentParticleCap )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->updateCurrentParticleCap(currentParticleCap);
		}
	} catch (...) { }
}

void __declspec(dllexport) UpdateCurrentNumParticles( int currentParticleCount )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->updateCurrentNumParticles(currentParticleCount);
		}
	} catch (...) { }
}

int __declspec(dllexport) GetNewParticleCap( void )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			return tmpWnd->getNewParticleCap();
		}
	} catch (...) { }
	return -1;
}


void __declspec(dllexport) GetSelectedParticleAsciiStringParm( int parmNum, char *bufferToCopyInto, ParticleSystemTemplate **whichTemplate)
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->getSelectedParticleAsciiStringParm(parmNum, bufferToCopyInto);
			if (whichTemplate) {
				(*whichTemplate) = tmpWnd->getCurrentParticleSystem();
			}
		}
	} catch (...) { }
}

void __declspec(dllexport) UpdateParticleAsciiStringParm( int parmNum, const char *bufferToCopyFrom, ParticleSystemTemplate **whichTemplate)
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->updateParticleAsciiStringParm(parmNum, bufferToCopyFrom);
			if (whichTemplate) {
				(*whichTemplate) = tmpWnd->getCurrentParticleSystem();
			}
		}
	} catch (...) { }
}

void __declspec(dllexport) UpdateCurrentParticleSystem( ParticleSystemTemplate *particleTemplate)
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->updateCurrentParticleSystem(particleTemplate);
		}
	} catch (...) { }
}

void __declspec(dllexport) UpdateSystemUseParameters( ParticleSystemTemplate *particleTemplate)
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			tmpWnd->updateSystemUseParameters(particleTemplate);
		}
	} catch(...) { }
}

Bool __declspec(dllexport) ShouldWriteINI( void )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			return tmpWnd->shouldWriteINI();
		}
	} catch (...) { }

	return false;
}

Bool __declspec(dllexport) HasRequestedReload( void )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			return tmpWnd->hasRequestedReload();
		}
	} catch (...) { }

	return false;
}

Bool __declspec(dllexport) ShouldBusyWait( void )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			return tmpWnd->shouldBusyWait();
		}
	} catch (...) { }

	return false;
}

Bool __declspec(dllexport) ShouldUpdateParticleCap( void )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			return tmpWnd->shouldUpdateParticleCap();
		}
	} catch (...) { }

	return false;
}

Bool __declspec(dllexport) ShouldReloadTextures( void )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			return tmpWnd->shouldReloadTextures();
		}
	} catch (...) { }

	return false;
}

Bool __declspec(dllexport) HasRequestedKillAllSystems( void )
{
	try {
		AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		DebugWindowDialog* tmpWnd = theApp.GetDialogWindow(); 
		
		if (tmpWnd) {
			return tmpWnd->shouldKillAllParticleSystems();
		}
	} catch (...) { }

	return false;
}

int __declspec(dllexport) NextParticleEditorBehavior( void )
{
	try { 
		if (HasUpdatedSelectedParticleSystem()) {
			return PEB_UpdateCurrentSystem;
		}

		if (ShouldWriteINI()) {
			return PEB_SaveAllSystems;
		}

		if (HasRequestedReload()) {
			return PEB_ReloadCurrentSystem;
		}

		if (ShouldBusyWait()) {
			return PEB_BusyWait;
		}
		
		if (ShouldUpdateParticleCap()) {
			return PEB_SetParticleCap;
		}

		if (ShouldReloadTextures()) {
			return PEB_ReloadTextures;
		}

		if (HasRequestedKillAllSystems()) {
			return PEB_KillAllSystems;
		}

		return PEB_Continue;
	} catch (...) {	}
	return PEB_Error;
}


