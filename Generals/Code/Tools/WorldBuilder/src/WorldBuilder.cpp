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

// WorldBuilder.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "euladialog.h"
#include "MainFrm.h"
#include "OpenMap.h"
#include "SplashScreen.h"
#include "textureloader.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "WBFrameWnd.h"
#include "wbview3d.h"

//#include <wsys/StdFileSystem.h>
#include "W3DDevice/GameClient/W3DFileSystem.h"
#include "Common/GlobalData.h"
#include "WHeightMapEdit.h"
//#include "Common/GameFileSystem.h"
#include "Common/FileSystem.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/LocalFileSystem.h"
#include "Common/CDManager.h"
#include "Common/Debug.h"
#include "Common/StackDump.h"
#include "Common/GameMemory.h"
#include "Common/Science.h"
#include "Common/ThingFactory.h"
#include "Common/INI.h"
#include "Common/GameAudio.h"
#include "Common/SpecialPower.h"
#include "Common/TerrainTypes.h"
#include "Common/DamageFX.h"
#include "Common/Upgrade.h"
#include "Common/ModuleFactory.h"
#include "Common/PlayerTemplate.h"
#include "Common/MultiplayerSettings.h"

#include "GameLogic/Armor.h"
#include "GameLogic/CaveSystem.h"
#include "GameLogic/CrateSystem.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/RankInfo.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/ScriptActions.h"
#include "GameClient/Anim2D.h"
#include "GameClient/GameText.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Water.h"
#include "GameClient/TerrainRoads.h"
#include "GameClient/FXList.h"
#include "GameClient/VideoPlayer.h"
#include "GameLogic/Locomotor.h"

#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "MilesAudioDevice/MilesAudioManager.h"

#include <io.h>
#include "Win32Device/GameClient/Win32Mouse.h"
#include "Win32Device/Common/Win32LocalFileSystem.h"
#include "Win32Device/Common/Win32BIGFileSystem.h"


static SubsystemInterfaceList TheSubsystemListRecord;

template<class SUBSYSTEM>
void initSubsystem(SUBSYSTEM*& sysref, SUBSYSTEM* sys, const char* path1 = NULL, const char* path2 = NULL)
{
	sysref = sys;
	TheSubsystemListRecord.initSubsystem(sys, path1, path2, NULL);
}


#define APP_SECTION "WorldbuilderApp"
#define OPEN_FILE_DIR "OpenDirectory"

Win32Mouse *TheWin32Mouse = NULL;
const char *gAppPrefix = "wb_"; /// So WB can have a different debug log file name.
const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";

/////////////////////////////////////////////////////////////////////////////
// WBGameFileClass - extends the file system a bit so we can get at some
// wb only data.  jba.

class WBGameFileClass : public GameFileClass
{

public:
	WBGameFileClass(char const *filename):GameFileClass(filename){};
	virtual char const * Set_Name(char const *filename);
};

//-------------------------------------------------------------------------------------------------
/** Sets the file name, and finds the GDI asset if present. */
//-------------------------------------------------------------------------------------------------
char const * WBGameFileClass::Set_Name( char const *filename )
{
	char const *pChar = GameFileClass::Set_Name(filename);
	if (this->Is_Available()) {
		return pChar; // it was found by the parent class.
	}

	if (TheFileSystem->doesFileExist(filename)) {
		strcpy( m_filePath, filename );
		m_fileExists = true;
	}
	return m_filename;
}



/////////////////////////////////////////////////////////////////////////////
// WB_W3DFileSystem - extends the file system a bit so we can get at some
// wb only data.  jba.

class	WB_W3DFileSystem : public W3DFileSystem {
	virtual FileClass * Get_File( char const *filename );
};

//-------------------------------------------------------------------------------------------------
/** Gets a file with the specified filename. */
//-------------------------------------------------------------------------------------------------
FileClass * WB_W3DFileSystem::Get_File( char const *filename )
{
	WBGameFileClass *pFile = new WBGameFileClass( filename );
	if (!pFile->Is_Available()) {
		pFile->Set_Name(filename);
	}
	return pFile;
}




/////////////////////////////////////////////////////////////////////////////
// The one and only CWorldBuilderApp object

static CWorldBuilderApp theApp;
HWND ApplicationHWnd = NULL;

/**
	* The ApplicationHInstance is needed for the WOL code,
	* which needs it for the COM initialization of WOLAPI.DLL.
	* Of course, the WOL code is in gameengine, while the
	* HINSTANCE is only in the various projects' main files.
	* So, we need to create the HINSTANCE, even if it always
	* stays NULL.  Just to make COM happy.  Whee.
	*/
HINSTANCE ApplicationHInstance = NULL;

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderApp

BEGIN_MESSAGE_MAP(CWorldBuilderApp, CWinApp)
	//{{AFX_MSG_MAP(CWorldBuilderApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(IDM_RESET_WINDOWS, OnResetWindows)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_TEXTURESIZING_MAPCLIFFTEXTURES, OnTexturesizingMapclifftextures)
	ON_UPDATE_COMMAND_UI(ID_TEXTURESIZING_MAPCLIFFTEXTURES, OnUpdateTexturesizingMapclifftextures)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
static Int gFirstCP = 0;

// CWorldBuilderApp construction

CWorldBuilderApp::CWorldBuilderApp() :
	m_curTool(NULL),
	m_selTool(NULL),
	m_lockCurTool(0),
	m_3dtemplate(NULL),
	m_pasteMapObjList(NULL)
{

	for (Int i=0; i<NUM_VIEW_TOOLS; i++) {
		m_tools[i] = NULL;

	}
	m_tools[0] = &m_brushTool;
	m_tools[1] = &m_tileTool;
	m_tools[2] = &m_featherTool;
	m_tools[3] = &m_autoEdgeOutTool;
	m_tools[4] = &m_bigTileTool;
	m_tools[5] = &m_floodFillTool;
	m_tools[6] = &m_moundTool;
	m_tools[7] = &m_digTool;
	m_tools[8] = &m_eyedropperTool;
	m_tools[9] = &m_objectTool;
	m_tools[10] = &m_pointerTool;
	m_tools[11] = &m_blendEdgeTool;
	m_tools[12] = &m_groveTool;
	m_tools[13] = &m_meshMoldTool;
	m_tools[14] = &m_roadTool;
	m_tools[15] = &m_handScrollTool;
	m_tools[16] = &m_waypointTool;
	m_tools[17] = &m_polygonTool;
	m_tools[18] = &m_buildListTool;
	m_tools[19] = &m_fenceTool;
	m_tools[20] = &m_waterTool;
	m_tools[21] = &m_rampTool;
	m_tools[22] = &m_scorchTool;
	m_tools[23] = &m_borderTool;

	// set up initial values.
	m_brushTool.setHeight(16);
	m_brushTool.setWidth(3);
	m_brushTool.setFeather(3);
	m_moundTool.setMoundHeight(3);
	m_moundTool.setWidth(3);
	m_moundTool.setFeather(3);
	m_featherTool.setFeather(3);
	m_featherTool.setRadius(1);
	m_featherTool.setRate(2);
}

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderApp destructor

CWorldBuilderApp::~CWorldBuilderApp()
{
	m_curTool = NULL;
	m_selTool = NULL;

	for (Int i=0; i<NUM_VIEW_TOOLS; i++) {
		if (m_tools[i]) {
			m_tools[i] = NULL;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Handler for unhandled win32 exceptions.

static LONG WINAPI UnHandledExceptionFilter(struct _EXCEPTION_POINTERS* e_info)
{
	DumpExceptionInfo(e_info->ExceptionRecord->ExceptionCode, e_info);
	return EXCEPTION_EXECUTE_HANDLER;
}

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderApp initialization

BOOL CWorldBuilderApp::InitInstance()
{
	EulaDialog eulaDialog;
	if( eulaDialog.DoModal() == IDCANCEL )
	{
		return FALSE;
	}

	// initialization
	SetUnhandledExceptionFilter(UnHandledExceptionFilter);

	// initialize the memory manager early
	initMemoryManager();

	DEBUG_LOG(("starting Worldbuilder."));
#ifdef RTS_DEBUG
	DEBUG_LOG(("RTS_DEBUG defined."));
#endif
#ifdef MEMORYPOOL_CHECKPOINTING
	gFirstCP = TheMemoryPoolFactory->debugSetCheckpoint();
#endif

	SplashScreen loadWindow;
	loadWindow.Create(IDD_LOADING, loadWindow.GetDesktopWindow());
	loadWindow.SetWindowText("Loading Worldbuilder");
	loadWindow.ShowWindow(SW_SHOW);
	loadWindow.UpdateWindow();

	CRect rect(15, 315, 230, 333);
	loadWindow.setTextOutputLocation(rect);
	loadWindow.outputText(IDS_SPLASH_LOADING);

	// not part of the subsystem list, because it should normally never be reset!
	TheNameKeyGenerator = new NameKeyGenerator;
	TheNameKeyGenerator->init();

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Set the current directory to the app directory.
	char buf[_MAX_PATH];
	GetModuleFileName(NULL, buf, sizeof(buf));
	char *pEnd = buf + strlen(buf);
	while (pEnd != buf) {
		if (*pEnd == '\\') {
			*pEnd = 0;
			break;
		}
		pEnd--;
	}
	::SetCurrentDirectory(buf);

	TheFileSystem = new FileSystem;

	initSubsystem(TheLocalFileSystem, (LocalFileSystem*)new Win32LocalFileSystem);
	initSubsystem(TheArchiveFileSystem, (ArchiveFileSystem*)new Win32BIGFileSystem);

	// Just for kicks, get the HINSTANCE that WOL would need
	// if we were going to use it, which we aren't.
	ApplicationHInstance = AfxGetInstanceHandle();

	INI ini;

	initSubsystem(TheWritableGlobalData, new GlobalData(), "Data\\INI\\Default\\GameData", "Data\\INI\\GameData");

#if defined(RTS_DEBUG)
	ini.loadFileDirectory( AsciiString( "Data\\INI\\GameDataDebug" ), INI_LOAD_MULTIFILE, NULL );
#endif

#ifdef DEBUG_CRASHING
	TheWritableGlobalData->m_debugIgnoreAsserts = true;
#endif

#if 1
	// srj sez: put INI into our user data folder, not the ap dir
	free((void*)m_pszProfileName);
	strcpy(buf, TheGlobalData->getPath_UserData().str());
	strcat(buf, "WorldBuilder.ini");
#else
	strcat(buf, "//");
	strcat(buf, m_pszProfileName);
	free((void*)m_pszProfileName);
#endif
	m_pszProfileName = (const char *)malloc(strlen(buf)+2);
	strcpy((char*)m_pszProfileName, buf);

	// ensure the user maps dir exists
	sprintf(buf, "%sMaps\\", TheGlobalData->getPath_UserData().str());
	CreateDirectory(buf, NULL);

	// read the water settings from INI (must do prior to initing GameClient, apparently)
	ini.loadFileDirectory( AsciiString( "Data\\INI\\Default\\Water" ), INI_LOAD_OVERWRITE, NULL );
	ini.loadFileDirectory( AsciiString( "Data\\INI\\Water" ), INI_LOAD_OVERWRITE, NULL );

	initSubsystem(TheGameText, CreateGameTextInterface());
	initSubsystem(TheScienceStore, new ScienceStore(), "Data\\INI\\Default\\Science", "Data\\INI\\Science");
	initSubsystem(TheMultiplayerSettings, new MultiplayerSettings(), "Data\\INI\\Default\\Multiplayer", "Data\\INI\\Multiplayer");
	initSubsystem(TheTerrainTypes, new TerrainTypeCollection(), "Data\\INI\\Default\\Terrain", "Data\\INI\\Terrain");
	initSubsystem(TheTerrainRoads, new TerrainRoadCollection(), "Data\\INI\\Default\\Roads", "Data\\INI\\Roads");

	WorldHeightMapEdit::init();

	initSubsystem(TheScriptEngine, (ScriptEngine*)(new ScriptEngine()));

	// need this before TheAudio in case we're running off of CD - TheAudio can try to open Music.big on the CD...
	initSubsystem(TheCDManager, CreateCDManager(), NULL);
	initSubsystem(TheAudio, (AudioManager*)new MilesAudioManager());
	if (!TheAudio->isMusicAlreadyLoaded())
		return FALSE;

	initSubsystem(TheVideoPlayer, (VideoPlayerInterface*)(new VideoPlayer()));
	initSubsystem(TheModuleFactory, (ModuleFactory*)(new W3DModuleFactory()));
	initSubsystem(TheSidesList, new SidesList());
	initSubsystem(TheCaveSystem, new CaveSystem());
	initSubsystem(TheRankInfoStore, new RankInfoStore(), NULL, "Data\\INI\\Rank");
	initSubsystem(ThePlayerTemplateStore, new PlayerTemplateStore(), "Data\\INI\\Default\\PlayerTemplate", "Data\\INI\\PlayerTemplate");
	initSubsystem(TheSpecialPowerStore, new SpecialPowerStore(), "Data\\INI\\Default\\SpecialPower", "Data\\INI\\SpecialPower" );
	initSubsystem(TheParticleSystemManager, (ParticleSystemManager*)(new W3DParticleSystemManager()));
	initSubsystem(TheFXListStore, new FXListStore(), "Data\\INI\\Default\\FXList", "Data\\INI\\FXList");
	initSubsystem(TheWeaponStore, new WeaponStore(), NULL, "Data\\INI\\Weapon");
	initSubsystem(TheObjectCreationListStore, new ObjectCreationListStore(), "Data\\INI\\Default\\ObjectCreationList", "Data\\INI\\ObjectCreationList");
	initSubsystem(TheLocomotorStore, new LocomotorStore(), NULL, "Data\\INI\\Locomotor");
	initSubsystem(TheDamageFXStore, new DamageFXStore(), NULL, "Data\\INI\\DamageFX");
	initSubsystem(TheArmorStore, new ArmorStore(), NULL, "Data\\INI\\Armor");
	initSubsystem(TheThingFactory, new ThingFactory(), "Data\\INI\\Default\\Object", "Data\\INI\\Object");
	initSubsystem(TheCrateSystem, new CrateSystem(), "Data\\INI\\Default\\Crate", "Data\\INI\\Crate");
	initSubsystem(TheUpgradeCenter, new UpgradeCenter, "Data\\INI\\Default\\Upgrade", "Data\\INI\\Upgrade");
	initSubsystem(TheAnim2DCollection, new Anim2DCollection ); //Init's itself.

	TheSubsystemListRecord.postProcessLoadAll();

	TheW3DFileSystem = new WB_W3DFileSystem;

	// Just to be sure - wb doesn't do well with half res terrain.
	DEBUG_ASSERTCRASH(!TheGlobalData->m_useHalfHeightMap, ("TheGlobalData->m_useHalfHeightMap : Don't use this setting in WB."));
	TheWritableGlobalData->m_useHalfHeightMap = false;

#if ENABLE_CONFIGURABLE_SHROUD
	// WB never uses the shroud. With shroud, terrain is black.
	TheWritableGlobalData->m_shroudOn = FALSE;
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	m_3dtemplate = new CSingleDocTemplate(
		IDR_MAPDOC,
		RUNTIME_CLASS(CWorldBuilderDoc),
		RUNTIME_CLASS(CWB3dFrameWnd),
		RUNTIME_CLASS(WbView3d));

	AddDocTemplate(m_3dtemplate);

#ifdef MDI
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAPDOC))
		return FALSE;
	m_pMainWnd = pMainFrame;
#endif

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// Parse command line for standard shell commands, DDE, file open
//	CCommandLineInfo cmdInfo;
//	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
//	if (!ProcessShellCommand(cmdInfo))
//		return FALSE;

	selectPointerTool();

	CString openDir = this->GetProfileString(APP_SECTION, OPEN_FILE_DIR);
	m_currentDirectory = openDir;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderApp message handlers

BOOL CWorldBuilderApp::OnCmdMsg(UINT nID, int nCode, void* pExtra,
							AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// If pHandlerInfo is NULL, then handle the message
	if (pHandlerInfo == NULL)
	{
		for (Int i=0; i<NUM_VIEW_TOOLS; i++) {
			Tool *pTool = m_tools[i];
			if (pTool==NULL) continue;
			if ((Int)nID == pTool->getToolID()) {
				if (nCode == CN_COMMAND)
				{
					// Handle WM_COMMAND message
					setActiveTool(pTool);
				}
				else if (nCode == CN_UPDATE_COMMAND_UI)
				{
					// Update UI element state
					CCmdUI *pUI = (CCmdUI*)pExtra;
					pUI->SetCheck(m_curTool == pTool?1:0);
					pUI->Enable(true);
				}
				return TRUE;
			}
		}
	}

	// If we didn't process the command, call the base class
	// version of OnCmdMsg so the message-map can handle the message
	return CWinApp::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

//=============================================================================
// CWorldBuilderApp::selectPointerTool
//=============================================================================
/** Sets the active tool to the pointer, and clears the selection. */
//=============================================================================
void CWorldBuilderApp::selectPointerTool(void)
{
	setActiveTool(&m_pointerTool);
	// Clear selection.
	m_pointerTool.clearSelection();
}

//=============================================================================
// CWorldBuilderApp::setActiveTool
//=============================================================================
/** Sets the active tool, and activates it after deactivating the current tool. */
//=============================================================================
void CWorldBuilderApp::setActiveTool(Tool *pNewTool)
{
	if (m_curTool == pNewTool) {
		// same tool
		return;
	}
	if (m_selTool && m_selTool != pNewTool) {
		m_selTool->deactivate();
	}
	if (pNewTool) {
		pNewTool->activate();
	}
	m_curTool = pNewTool;
	m_selTool = pNewTool;
}

//=============================================================================
// CWorldBuilderApp::updateCurTool
//=============================================================================
/** Checks to see if any key modifiers (ctrl or alt) are pressed.  If so,
selectes the appropriate tool, else uses the normal tool. */
//=============================================================================
void CWorldBuilderApp::updateCurTool(Bool forceHand)
{
	Tool *curTool = m_curTool;
	DEBUG_ASSERTCRASH((m_lockCurTool>=0),("oops"));
	if (!m_lockCurTool) {	 // don't change tools that are doing something.
		if (forceHand || (0x8000 & ::GetAsyncKeyState(VK_SPACE))) {
			// Space bar gives scroll hand.
			m_curTool = &m_handScrollTool;
		} else if (0x8000 & ::GetAsyncKeyState(VK_MENU)) {
			// Alt key gives eyedropper.
			m_curTool = &m_eyedropperTool;
		} else if (0x8000 & ::GetAsyncKeyState(VK_CONTROL)) {
			// Control key gives pointer.
			m_curTool = &m_pointerTool;
		} else {
			// Else the tool selected in the tool palette.
			m_curTool = m_selTool;
		}
	}
	if (curTool != m_curTool) {
		m_curTool->activate();
	}
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
void CWorldBuilderApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderApp message handlers

int CWorldBuilderApp::ExitInstance()
{

	WriteProfileString(APP_SECTION, OPEN_FILE_DIR, m_currentDirectory.str());
	m_currentDirectory.clear();

	ScriptList::reset();

	TheSubsystemListRecord.shutdownAll();

	WorldHeightMapEdit::shutdown();

	delete TheFileSystem;
	TheFileSystem = NULL;
	TextureLoadTaskClass::shutdown();

	delete TheW3DFileSystem;
	TheW3DFileSystem = NULL;

	delete TheNameKeyGenerator;
	TheNameKeyGenerator = NULL;

#ifdef MEMORYPOOL_CHECKPOINTING
	Int lastCP = TheMemoryPoolFactory->debugSetCheckpoint();
#endif
#ifdef MEMORYPOOL_CHECKPOINTING
	TheMemoryPoolFactory->debugMemoryReport(REPORT_FACTORYINFO | REPORT_CP_LEAKS | REPORT_CP_STACKTRACE, gFirstCP, lastCP);
#endif
	shutdownMemoryManager();

	return CWinApp::ExitInstance();
}

void CWorldBuilderApp::OnResetWindows()
{
	if (CMainFrame::GetMainFrame()) {
		CMainFrame::GetMainFrame()->ResetWindowPositions();
	}

}

void CWorldBuilderApp::OnFileOpen()
{
#ifdef DO_MAPS_IN_DIRECTORIES
	TOpenMapInfo info;
	OpenMap mapDlg(&info);
	if (mapDlg.DoModal() == IDOK) {
		if (!info.browse) {
			OpenDocumentFile(info.filename);
			return;
		}
	}	else {
		// cancelled so return.
		return;
	}
#endif

	CFileStatus status;
	if (m_currentDirectory != AsciiString("")) try {
		if (CFile::GetStatus(m_currentDirectory.str(), status)) {
			if (status.m_attribute & CFile::directory) {
				::SetCurrentDirectory(m_currentDirectory.str());
			}
		}
	} catch(...) {}

	CWinApp::OnFileOpen();
}

void CWorldBuilderApp::OnTexturesizingMapclifftextures()
{
	setActiveTool(&m_floodFillTool);
	m_floodFillTool.setAdjustCliffs(true);

}

void CWorldBuilderApp::OnUpdateTexturesizingMapclifftextures(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here

}
