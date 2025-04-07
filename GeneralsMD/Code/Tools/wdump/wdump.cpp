/*
**	Command & Conquer Renegade(tm)
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

// wdump.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "wdump.h"

#include "MainFrm.h"
#include "wdumpDoc.h"
#include "wdView.h"

#include "FCNTL.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


HINSTANCE ApplicationHInstance = NULL;  ///< our application instance

/// just to satisfy the game libraries we link to
HWND ApplicationHWnd = NULL;

const char *gAppPrefix = "wd_";

// Where are the default string files?
const char *g_strFile = "data\\Generals.str";
const char *g_csfFile = "data\\%s\\Generals.csf";

/////////////////////////////////////////////////////////////////////////////
// CWdumpApp

BEGIN_MESSAGE_MAP(CWdumpApp, CWinApp)
	//{{AFX_MSG_MAP(CWdumpApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWdumpApp construction

CWdumpApp::CWdumpApp() 
: DumpTextures(false), NoWindow(false), TextureDumpFile(0)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWdumpApp object

CWdumpApp theApp;

// private class declaration of a CCommandLineInfo class that knows about our special options
class CWDumpCommandLineInfo : public CCommandLineInfo
{
	virtual void ParseParam(const TCHAR* pszParam,BOOL bFlag,BOOL bLast)
	{
		if (bFlag)
		{
			if(pszParam[0] == 't') {
				theApp.DumpTextures = true;
				ParseLast(bLast);
				return;
			}
			if(pszParam[0] == 'q') {
				theApp.NoWindow = true;
				ParseLast(bLast);
				return;
			}
		}

		CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
	}

};


/////////////////////////////////////////////////////////////////////////////
// CWdumpApp initialization

BOOL CWdumpApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Westwood Studios"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWdumpDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CWdumpView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CWDumpCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// note: if any other dump types are enabled, they should probably open different files.
	if(DumpTextures) {
		TextureDumpFile = fopen("textures.txt", "a");
	}

	if(NoWindow) {
		if(cmdInfo.m_nShellCommand == CWDumpCommandLineInfo::FileOpen) {
			const char *c = strrchr(cmdInfo.m_strFileName, '\\');
			if(c == 0) 
				c = (LPCTSTR) cmdInfo.m_strFileName;
			if(*c == '\\')
				c++;

			Filename = c;
			


/*			STARTUPINFO info;
			GetStartupInfo(&info);

			if(info.hStdOutput == NULL) {
				AllocConsole();                  // Allocate console window
				freopen("CONOUT$", "a", stdout);
				freopen("CONIN$", "r", stdin);
			} else {
				int CrtInput;
				int CrtOutput;
				if ( (CrtInput  =_open_osfhandle((long) info.hStdInput, _O_RDONLY)) != -1) {
					if ( (CrtOutput = _open_osfhandle((long) info.hStdOutput, _O_APPEND)) != -1) {
						_dup2( CrtInput, 0);
						_dup2( CrtOutput, 1);
					}
				}
					 
//				stdin = (struct _iobuf * ) info.hStdInput;
//				stdout = (struct _iobuf * ) info.hStdOutput;
			}
*/			

			CWdumpDoc *doc = (CWdumpDoc *) pDocTemplate->OpenDocumentFile(cmdInfo.m_strFileName, FALSE);

/*			if(info.hStdOutput == NULL) {
				printf("Press return to close this window..");
				getchar();
				FreeConsole();
			}
*/

			CloseAllDocuments(TRUE);
			PostQuitMessage(0);
			return TRUE;
		}
	}

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;


	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	POSITION p = pDocTemplate->GetFirstDocPosition();
	CWdumpDoc *doc = (CWdumpDoc *) pDocTemplate->GetNextDoc(p);
	doc->UpdateAllViews(0);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CWdumpApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CWdumpApp commands
