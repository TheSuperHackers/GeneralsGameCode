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

// W3DView.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include "W3DView.h"

#include "MainFrm.h"
#include "W3DViewDoc.h"
#include "W3DViewView.h"
#include "Utils.h"
#include "ColorUtils.h"
#include "verchk.h"
#include "wwmath.h"
#include "WWAudio.h"
#include "ViewerAssetMgr.h"
#include "Globals.h"
#include "AnimatedSoundOptionsDialog.h"
#include "animatedsoundmgr.h"


#undef STRICT
#include "ww3d.h"
#include "assetmgr.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


HINSTANCE ApplicationHInstance = NULL;  ///< our application instance

/// just to satisfy the game libraries we link to
HWND ApplicationHWnd = NULL;

const char *gAppPrefix = "w3_";

// Where are the default string files?
const char *g_strFile = "data\\Generals.str";
const char *g_csfFile = "data\\%s\\Generals.csf";

/////////////////////////////////////////////////////////////////////////////
//
//	Local prototypes
//
BOOL CALLBACK fnTopLevelWindowSearch (HWND hwnd, LPARAM lParam);


/////////////////////////////////////////////////////////////////////////////
//
// CW3DViewApp
//
BEGIN_MESSAGE_MAP(CW3DViewApp, CWinApp)
	//{{AFX_MSG_MAP(CW3DViewApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//
// CW3DViewApp construction
//
CW3DViewApp::CW3DViewApp (void)
	: m_bInitialized (false)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CW3DViewApp object

CW3DViewApp theApp;

extern int AFXAPI AfxWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);


int WINAPI
WinMain
(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine,
	int nCmdShow
)
{
	int retcode = 0;

#ifndef RTS_DEBUG
	try
	{
#endif //RTS_DEBUG

		//::AfxWinInit (hInstance, hPrevInstance, lpCmdLine, nCmdShow);
		//::AfxInitialize (FALSE, _MFC_VER);

		AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
		pModuleState->m_bDLL = (BYTE)FALSE;
	#ifdef _MBCS
		// set correct multi-byte code-page for Win32 apps
		_setmbcp(_MB_CP_ANSI);
	#endif //_MBCS

		retcode = ::AfxWinMain (hInstance, hPrevInstance, lpCmdLine, nCmdShow);

#ifndef RTS_DEBUG
	}
	catch (...)
	{

		::MessageBox (NULL, "Internal Application Error", "Unrecoverable Error", MB_ICONERROR | MB_OK);
	}
#endif //RTS_DEBUG

	return retcode;
}


///////////////////////////////////////////////////////////////
//
//  Do_Version_Check
//
////////////////////////////////////////////////////////////
void
Do_Version_Check (void)
{	
	char curr_filename[MAX_PATH];
	::GetModuleFileName (NULL, curr_filename, MAX_PATH);

	CString filename = "\\\\cabal\\mis\\r&d\\w3d\\w3dview\\";
	filename += ::Get_Filename_From_Path (curr_filename);

	//
	//	Check the version of the viewer that is out on the network
	// against the version we are running.
	//
	if (Compare_EXE_Version ((int)::AfxGetInstanceHandle (), filename) < 0) {
		::MessageBox (NULL, "There is a newer version of the W3DViewer, please run W3DUpdate to upgrade your local copy.", "Version Info", MB_ICONEXCLAMATION | MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// InitInstance
//
BOOL CW3DViewApp::InitInstance (void)
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

	Do_Version_Check ();

	RegisterColorPicker (::AfxGetInstanceHandle ());
	RegisterColorBar (::AfxGetInstanceHandle ());

	// Is there already an instance of the viewer running?
	HWND hprev_instance = NULL;
	::EnumWindows (fnTopLevelWindowSearch, (LPARAM)&hprev_instance);
	if (hprev_instance == NULL) {

		// Change the registry key under which our settings are stored.
		// You should modify this string to be something appropriate
		// such as the name of your company or organization.
		SetRegistryKey(_T("Westwood Studios"));
		
		//
		// Load standard INI file options (including MRU)
		//
		LoadStdProfileSettings (9);

		//
		//	Initialize the libraries
		//
		WWMath::Init ();
		AnimatedSoundOptionsDialogClass::Load_Animated_Sound_Settings ();

		//
		//	Disable the 3DFX logo
		//
		_putenv ("FX_GLIDE_NO_SPLASH=1");

		// Register the application's document templates.  Document templates
		//  serve as the connection between documents, frame windows and views.

		CSingleDocTemplate* pDocTemplate;
		pDocTemplate = new CSingleDocTemplate(
			IDR_MAINFRAME,
			RUNTIME_CLASS(CW3DViewDoc),
			RUNTIME_CLASS(CMainFrame),       // main SDI frame window
			RUNTIME_CLASS(CW3DViewView));
		AddDocTemplate(pDocTemplate);

		// Enable DDE Execute open
		EnableShellOpen();
		RegisterShellFileTypes(TRUE);

		 // Parse command line for standard shell commands, DDE, file open
		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);

		//
		//	Allocate an asset manager
		//
		_TheAssetMgr = new ViewerAssetMgrClass;

		// Dispatch commands specified on the command line
		if (!ProcessShellCommand(cmdInfo))
			return FALSE;

		// The one and only window has been initialized, so show and update it.
		m_pMainWnd->ShowWindow(SW_SHOW);
		m_pMainWnd->UpdateWindow();
		::SetProp (*m_pMainWnd, "WW3DVIEWER", (HANDLE)1);

		// Enable drag/drop open
		m_pMainWnd->DragAcceptFiles();
		m_bInitialized = true;
	} else {

		// Make the previous instance in the foreground
		::ShowWindow (hprev_instance, SW_NORMAL);
		::BringWindowToTop (hprev_instance);
		::SetForegroundWindow (hprev_instance);
	}

	return (hprev_instance == NULL);
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
	virtual BOOL OnInitDialog();
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CW3DViewApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/*
**
*/
void Debug_Refs(void)
{
#ifndef NDEBUG
	TRACE("Detecting Active Refs...\r\n");
   //ODS("At time %s", cMiscUtil::Get_Text_Time());
	RefCountNodeClass * first = RefCountClass::ActiveRefList.First();
	RefCountNodeClass * node = first;
	while (node->Is_Valid())
	{
		RefCountClass * obj = node->Get();
		ActiveRefStruct * ref = &(obj->ActiveRefInfo);

		bool display = true;
		int	count = 0;
		RefCountNodeClass * search = first;
		while (search->Is_Valid()) {

			if (search == node) {	// if this is not the first one
				if (count != 0) {
					display = false;
					break;
				}
			}

			RefCountClass * search_obj = search->Get();
			ActiveRefStruct * search_ref = &(search_obj->ActiveRefInfo);

			if ( ref->File && search_ref->File &&
				  !strcmp(search_ref->File, ref->File) &&
				  (search_ref->Line == ref->Line) ) {
				count++;
			} else if ( (ref->File == NULL) &&  (search_ref->File == NULL) ) {
				count++;
			}

			search = search->Next();
		}

		if ( display ) {
			TRACE ( "%d Active Ref: %s %d %p\n", count, ref->File,ref->Line,obj);

			static int num_printed = 0;
			if (++num_printed > 20) {
				TRACE( "And Many More......\n");
				break;
			}
		}

		node = node->Next();
	}
	TRACE("Done.\r\n");
   //ODS("At time %s", cMiscUtil::Get_Text_Time());
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
// CW3DViewApp
//
int
CW3DViewApp::ExitInstance() 
{
	//
	// Free any resources the WW3D engine allocated
	//
	if (m_bInitialized) {

	//
		//	Shutdown the audio system
		//
		WWAudioClass::Get_Instance ()->Shutdown ();

		//
		//	Shutdown W3D
		//
		WW3DAssetManager::Get_Instance()->Free_Assets ();
		WW3D::Shutdown ();

		//
		//	Shutdown the libraries
		//
		WWMath::Shutdown ();
		AnimatedSoundMgrClass::Shutdown ();

		//
		//	Free the asset manager
		//
		delete _TheAssetMgr;
		_TheAssetMgr = NULL;
	}

	Debug_Refs ();
	return CWinApp::ExitInstance ();
}


//////////////////////////////////////////////////////////////////////////////
//
//	fnTopLevelWindowSearch
//
BOOL CALLBACK
fnTopLevelWindowSearch
(
	HWND hwnd,
	LPARAM lParam
)
{
	BOOL bcontinue = TRUE;

	// Is this a viewer window?
	if (::GetProp (hwnd, "WW3DVIEWER") != 0) {
		bcontinue = false;
		(*((HWND *)lParam)) = hwnd;
	}

	// Return the TRUE/FALSE result code
	return bcontinue;
}


//////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
BOOL
CAboutDlg::OnInitDialog (void)
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();

	// Version 1.0 by default
	DWORD version_major = 1;
	DWORD version_minor = 0;

	// Get the name and path of the currently executing application
	TCHAR filename[MAX_PATH];
	::GetModuleFileName (NULL, filename, sizeof (filename));

	// Get the version information for this file
	DWORD dummy_var = 0;
	DWORD version_size = ::GetFileVersionInfoSize (filename, &dummy_var);
	if (version_size > 0) {

		// Get the file version block
		LPBYTE pblock = new BYTE[version_size];
		if (::GetFileVersionInfo (filename, 0L, version_size, pblock)) {

			// Query the block for the file version information
			UINT version_len = 0;
			VS_FIXEDFILEINFO *pversion_info = NULL;
			if (::VerQueryValue (pblock, "\\", (LPVOID *)&pversion_info, &version_len)) {
				version_major = pversion_info->dwFileVersionMS;
				version_minor = pversion_info->dwFileVersionLS;
			}
		}
		SAFE_DELETE_ARRAY(pblock);
	}
	
	// Put the version string into the dialog
	CString version_string;
	version_string.Format (IDS_VERSION, (version_major >> 16), (version_major & 0xFFFF));
	SetDlgItemText (IDC_VERSION, version_string);
	return TRUE;
}

