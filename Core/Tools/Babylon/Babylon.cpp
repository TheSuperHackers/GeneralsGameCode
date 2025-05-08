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

// Babylon.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include "Babylon.h"
#include "BabylonDlg.h"
#include "XLStuff.h"

#include <direct.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char AppTitle[200];
CBabylonDlg *MainDLG = NULL;

static const char *AppName = "Babylon:";
static int AlreadyRunning( void );
static HWND FoundWindow = NULL;
/////////////////////////////////////////////////////////////////////////////
// CBabylonApp

BEGIN_MESSAGE_MAP(CBabylonApp, CWinApp)
	//{{AFX_MSG_MAP(CBabylonApp)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBabylonApp construction

CBabylonApp::CBabylonApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBabylonApp object

CBabylonApp theApp;
TransDB				*BabylonstrDB = NULL;
TransDB				*MainDB = NULL;
char		BabylonstrFilename[_MAX_PATH];
char		MainXLSFilename[_MAX_PATH];
char		DialogPath[_MAX_PATH];
char		RootPath[_MAX_PATH];
LangID	CurrentLanguage = LANGID_US;


/////////////////////////////////////////////////////////////////////////////
// CBabylonApp initialization

BOOL CBabylonApp::InitInstance()
{
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Parse the command line to see if launched as OLE server
	if (RunEmbedded() || RunAutomated())
	{
		// Register all OLE server (factories) as running.  This enables the
		//  OLE libraries to create objects from other applications.
		COleTemplateServer::RegisterAll();
	}
	else
	{
		// When a server application is launched stand-alone, it is a good idea
		//  to update the system registry in case it has been damaged.
		COleObjectFactory::UpdateRegistryAll();
	}

	MainDB = new TransDB ( "Main" );
	BabylonstrDB = new TransDB ( "Generals.str" );
	MainDB->EnableIDs ();

	if ( !AfxInitRichEdit( ) )
	{
		AfxMessageBox ( "Error initializing Rich Edit" );
	}

	sprintf (AppTitle, "%s Built on %s - %s", AppName, __DATE__, __TIME__ );

  if ( !_getcwd( RootPath, _MAX_PATH ) )
	{
		AfxMessageBox ( "Failed to obtain current working directoy!\n\n Set directoy path to \"c:\\Babylon\"." );
		strcpy ( (char *) RootPath, "c:\\Babylon" );
	}
	strlwr ( RootPath );

	strcpy ( (char *) BabylonstrFilename, RootPath );
	strcat ( (char *) BabylonstrFilename, "\\Data\\Generals.str" );
	strcpy ( (char *) MainXLSFilename, RootPath );
	strcat ( (char *) MainXLSFilename, "\\Data\\main.db" );
	strcpy ( (char *) DialogPath, RootPath );
	strcat ( (char *) DialogPath, "\\dialog" );

	if ( AlreadyRunning ()  )
	{
		if ( FoundWindow )
		{
			SetForegroundWindow ( FoundWindow );
		}
	}
	else if ( OpenExcel ( ) )
	{
		CBabylonDlg dlg;
		m_pMainWnd = &dlg;
		MainDLG = &dlg;

		int nResponse = dlg.DoModal();

		CloseExcel ();
	}

	delete BabylonstrDB ;
	delete MainDB;
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

//DEL void CBabylonApp::OnDropFiles(HDROP hDropInfo) 
//DEL {
//DEL 	// TODO: Add your message handler code here and/or call default
//DEL 	
//DEL 	CWinApp::OnDropFiles(hDropInfo);
//DEL }

static BOOL CALLBACK EnumAllWindowsProc(HWND hWnd, LPARAM lParam);
static BOOL CALLBACK EnumAllWindowsProcExact(HWND hWnd, LPARAM lParam);
static const char *szSearchTitle;

static int AlreadyRunning( void )
{
	BOOL found = FALSE;
	
	szSearchTitle = AppName;
	
	EnumWindows((WNDENUMPROC) EnumAllWindowsProcExact, (LPARAM) &found);
	
	return found;
}

//--------------------------------------------------------------------------------

int ExcelRunning( void )
{
	BOOL found = FALSE;
	
	szSearchTitle = "Microsoft Excel";
	
	EnumWindows((WNDENUMPROC) EnumAllWindowsProc, (LPARAM) &found);
	
	return found;
}

//--------------------------------------------------------------------------------

BOOL CALLBACK EnumAllWindowsProc(HWND hWnd, LPARAM lParam)
{
	char szText[256];

	GetWindowText(hWnd, szText, sizeof(szText));
	
	if ( strstr(szText, szSearchTitle))
	{
		* (BOOL *) lParam = TRUE;
		 FoundWindow = hWnd;
		 return FALSE;
	}

	FoundWindow = NULL;
	return TRUE;
}

//--------------------------------------------------------------------------------

BOOL CALLBACK EnumAllWindowsProcExact(HWND hWnd, LPARAM lParam)
{
	char szText[256];

	GetWindowText(hWnd, szText, sizeof(szText));
	
	if ( !strncmp (szText, szSearchTitle, strlen ( szSearchTitle)))
	{
		* (BOOL *) lParam = TRUE;
		 FoundWindow = hWnd;
		 return FALSE;
	}

	FoundWindow = NULL;
	return TRUE;
}

