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

// WorldBuilder.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"
#include <eh.h>
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

#include "Common/WellKnownKeys.h"
#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

static SubsystemInterfaceList TheSubsystemListRecord;

template<class SUBSYSTEM>
void initSubsystem(SUBSYSTEM*& sysref, SUBSYSTEM* sys, const char* path1 = NULL, const char* path2 = NULL, const char* dirpath = NULL)
{
	sysref = sys;
	TheSubsystemListRecord.initSubsystem(sys, path1, path2, dirpath, NULL);
}


#define APP_SECTION "WorldbuilderApp"
#define OPEN_FILE_DIR "OpenDirectory"
#define GAME_DIR "GameDirectory"
#define EULA "AgreedToEula"
#define ABOUT_SECTION "AboutWindow"

#define NEWLINE "\r\n"


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
	m_tools[24] = &m_rulerTool;

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
	m_bLaunchOnStartUp = true;

	for (Int i=0; i<NUM_VIEW_TOOLS; i++) {
		if (m_tools[i]) {
			m_tools[i] = NULL;
		}
	}
	_exit(0);
}


/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderApp initialization

BOOL CWorldBuilderApp::InitInstance()
{
	
//#ifdef _RELEASE
	if (this->GetProfileInt(APP_SECTION, EULA, 0) == 0) {
		EulaDialog eulaDialog;
		if (eulaDialog.DoModal() == IDCANCEL) {
			return FALSE;
		}
		/**
		 * Adriane [Deathscythe] 
		 * Idk why this thing stores it on c:/windows
		 * instead of the actual worldbuilder.ini in documents but fuck it -- it works anyways.
		 */ 
		this->WriteProfileInt(APP_SECTION, EULA, 1);
	}
//#endif

	ApplicationHWnd = GetDesktopWindow();

	// initialization
  _set_se_translator( DumpExceptionInfo ); // Hook that allows stack trace.

	// start the log
	DEBUG_INIT(DEBUG_FLAGS_DEFAULT);

#ifdef DEBUG_LOGGING
	// Turn on console output jba [3/20/2003]
	DebugSetFlags(DebugGetFlags() | DEBUG_FLAG_LOG_TO_CONSOLE);
#endif

	DEBUG_LOG(("starting Worldbuilder.\n"));

#ifdef _INTERNAL
	DEBUG_LOG(("_INTERNAL defined.\n"));
#endif
#ifdef _DEBUG
	DEBUG_LOG(("_DEBUG defined.\n"));
#endif
	initMemoryManager();
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

	initSubsystem(TheWritableGlobalData, new GlobalData(), "Data\\INI\\Default\\GameData.ini", "Data\\INI\\GameData.ini");
	
#if defined(_DEBUG) || defined(_INTERNAL)
	ini.load( AsciiString( "Data\\INI\\GameDataDebug.ini" ), INI_LOAD_MULTIFILE, NULL );
#endif

#ifdef DEBUG_CRASHING
	TheWritableGlobalData->m_debugIgnoreAsserts = false;
#endif

#if defined(_INTERNAL)
	// leave on asserts for a while. jba. [4/15/2003] TheWritableGlobalData->m_debugIgnoreAsserts = true;
#endif
	DEBUG_LOG(("TheWritableGlobalData %x\n", TheWritableGlobalData));
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
	ini.load( AsciiString( "Data\\INI\\Default\\Water.ini" ), INI_LOAD_OVERWRITE, NULL );
	ini.load( AsciiString( "Data\\INI\\Water.ini" ), INI_LOAD_OVERWRITE, NULL );

	initSubsystem(TheGameText, CreateGameTextInterface());
	initSubsystem(TheScienceStore, new ScienceStore(), "Data\\INI\\Default\\Science.ini", "Data\\INI\\Science.ini");
	initSubsystem(TheMultiplayerSettings, new MultiplayerSettings(), "Data\\INI\\Default\\Multiplayer.ini", "Data\\INI\\Multiplayer.ini");
	initSubsystem(TheTerrainTypes, new TerrainTypeCollection(), "Data\\INI\\Default\\Terrain.ini", "Data\\INI\\Terrain.ini");
	initSubsystem(TheTerrainRoads, new TerrainRoadCollection(), "Data\\INI\\Default\\Roads.ini", "Data\\INI\\Roads.ini");

	WorldHeightMapEdit::init();

	initSubsystem(TheScriptEngine, (ScriptEngine*)(new ScriptEngine()));

	TheScriptEngine->turnBreezeOff(); // stop the tree sway.

	//  [2/11/2003]
	ini.load( AsciiString( "Data\\Scripts\\Scripts.ini" ), INI_LOAD_OVERWRITE, NULL );

	// need this before TheAudio in case we're running off of CD - TheAudio can try to open Music.big on the CD...
	initSubsystem(TheCDManager, CreateCDManager(), NULL);
	initSubsystem(TheAudio, (AudioManager*)new MilesAudioManager());
	if (!TheAudio->isMusicAlreadyLoaded())
		return FALSE;

	initSubsystem(TheVideoPlayer, (VideoPlayerInterface*)(new VideoPlayer()));
	initSubsystem(TheModuleFactory, (ModuleFactory*)(new W3DModuleFactory()));
	initSubsystem(TheSidesList, new SidesList());
	initSubsystem(TheCaveSystem, new CaveSystem());
	initSubsystem(TheRankInfoStore, new RankInfoStore(), NULL, "Data\\INI\\Rank.ini");
	initSubsystem(ThePlayerTemplateStore, new PlayerTemplateStore(), "Data\\INI\\Default\\PlayerTemplate.ini", "Data\\INI\\PlayerTemplate.ini");
	initSubsystem(TheSpecialPowerStore, new SpecialPowerStore(), "Data\\INI\\Default\\SpecialPower.ini", "Data\\INI\\SpecialPower.ini" );
	initSubsystem(TheParticleSystemManager, (ParticleSystemManager*)(new W3DParticleSystemManager()));
	initSubsystem(TheFXListStore, new FXListStore(), "Data\\INI\\Default\\FXList.ini", "Data\\INI\\FXList.ini");
	initSubsystem(TheWeaponStore, new WeaponStore(), NULL, "Data\\INI\\Weapon.ini");
	initSubsystem(TheObjectCreationListStore, new ObjectCreationListStore(), "Data\\INI\\Default\\ObjectCreationList.ini", "Data\\INI\\ObjectCreationList.ini");
	initSubsystem(TheLocomotorStore, new LocomotorStore(), NULL, "Data\\INI\\Locomotor.ini");
	initSubsystem(TheDamageFXStore, new DamageFXStore(), NULL, "Data\\INI\\DamageFX.ini");
	initSubsystem(TheArmorStore, new ArmorStore(), NULL, "Data\\INI\\Armor.ini");
	initSubsystem(TheThingFactory, new ThingFactory(), "Data\\INI\\Default\\Object.ini", NULL, "Data\\INI\\Object");
	initSubsystem(TheCrateSystem, new CrateSystem(), "Data\\INI\\Default\\Crate.ini", "Data\\INI\\Crate.ini");
	initSubsystem(TheUpgradeCenter, new UpgradeCenter, "Data\\INI\\Default\\Upgrade.ini", "Data\\INI\\Upgrade.ini");
	initSubsystem(TheAnim2DCollection, new Anim2DCollection ); //Init's itself.

	TheSubsystemListRecord.postProcessLoadAll();

	TheW3DFileSystem = new WB_W3DFileSystem;

	// Just to be sure - wb doesn't do well with half res terrain.
	DEBUG_ASSERTCRASH(!TheGlobalData->m_useHalfHeightMap, ("TheGlobalData->m_useHalfHeightMap : Don't use this setting in WB."));
	TheWritableGlobalData->m_useHalfHeightMap = false;

#if defined(_DEBUG) || defined(_INTERNAL)
	// WB never uses the shroud.
	TheWritableGlobalData->m_shroudOn = FALSE;
#endif

	TheWritableGlobalData->m_isWorldBuilder = TRUE;

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

	// Load GameDirectory
	CString gameDir = this->GetProfileString(APP_SECTION, GAME_DIR);
	m_gameDirectory = gameDir;

	if (m_gameDirectory != AsciiString("")){
		DEBUG_LOG(("gameDirectory=%s \n", m_gameDirectory.str()));
	} else {
		DEBUG_LOG(("Warning! Game directory not initialized.\n"));
	}

	CString openDir = this->GetProfileString(APP_SECTION, OPEN_FILE_DIR);
	m_currentDirectory = openDir;

	m_bLaunchOnStartUp=::AfxGetApp()->GetProfileInt(ABOUT_SECTION, "LaunchOnStartUp", 1);
	if(m_bLaunchOnStartUp){
		OnAppAbout();
	}

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
	BOOL m_bLaunchOnStartUpAbout;
	MapObject* m_lastSelectedObject;
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	afx_msg void OnCenterOnSelectedButtonWP();
	afx_msg void OnCenterOnSelectedButtonObject();
	afx_msg void OnFindButtonClicked();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnLaunchOnStartup();
	void OnCenterOnSelected(Bool findObject); 
	afx_msg void OnRefreshQueryObject();
	afx_msg void OnRefreshQueryWaypoint();
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_MOVE()
	ON_BN_CLICKED(IDC_FIND_HKEY_BUTTON, OnFindButtonClicked)

	ON_BN_CLICKED(IDC_FIND_OBJ_BUTTON, OnCenterOnSelectedButtonObject)
	ON_BN_CLICKED(IDC_REFRESH_OBJ_BUTTON, OnRefreshQueryObject)

	ON_BN_CLICKED(IDC_FIND_WP_BUTTON, OnCenterOnSelectedButtonWP)
	ON_BN_CLICKED(IDC_REFRESH_WP_BUTTON, OnRefreshQueryWaypoint)

	ON_BN_CLICKED(IDC_LAUNCH_ONSTARTUP, OnLaunchOnStartup)
	ON_WM_WINDOWPOSCHANGING() 
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	m_bLaunchOnStartUpAbout = true;
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::OnRefreshQueryObject()
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_FIND_QUERY_OBJ);
	if (!pCombo) return;

	pCombo->ResetContent();

	MapObject* pMapObj = MapObject::getFirstMapObject();
	while (pMapObj)
	{
		Bool exists;
		AsciiString objName = pMapObj->getProperties()->getAsciiString(TheKey_objectName, &exists);

		if (exists && !objName.isEmpty() && !pMapObj->getFlag(FLAG_ROAD_FLAGS))
		{
			pCombo->AddString(objName.str());
		}

		pMapObj = pMapObj->getNext();
	}
}

void CAboutDlg::OnRefreshQueryWaypoint()
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_FIND_QUERY_WP);
	if (!pCombo) return;

	pCombo->ResetContent();

	MapObject* pMapObj = MapObject::getFirstMapObject();
	while (pMapObj)
	{
		if (pMapObj->isWaypoint()) {
			AsciiString wayName = pMapObj->getWaypointName();
			if (wayName.isNotEmpty() && pCombo->FindStringExact(-1, wayName.str()) == CB_ERR) {
				pCombo->AddString(wayName.str());
			}
		}

		pMapObj = pMapObj->getNext();
	}
}


void CAboutDlg::OnCenterOnSelected(Bool findObject = true) 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(findObject ? IDC_FIND_QUERY_OBJ : IDC_FIND_QUERY_WP);
	if (!pCombo) return;

	CString selectedName;
	pCombo->GetWindowText(selectedName);

	if (selectedName.IsEmpty())
		return;

	MapObject* mapObject = MapObject::getFirstMapObject();
	while (mapObject) {
		if (findObject) {
			// Looking for a regular object by object name
			Bool exists;
			AsciiString objName = mapObject->getProperties()->getAsciiString(TheKey_objectName, &exists);

			if (exists && !objName.isEmpty() && selectedName == objName.str()) {
				const Coord3D* objectPosition = mapObject->getLocation();
				WbView3d* p3View = CWorldBuilderDoc::GetActive3DView();
				if (p3View) {
					p3View->setCenterInView(objectPosition->x / MAP_XY_FACTOR, objectPosition->y / MAP_XY_FACTOR);
				}
				return;
			}
		}
		else if (mapObject->isWaypoint()) {
			AsciiString wayName = mapObject->getWaypointName();
			if (!wayName.isEmpty() && selectedName == wayName.str()) {
				const Coord3D* objectPosition = mapObject->getLocation();
				WbView3d* p3View = CWorldBuilderDoc::GetActive3DView();
				if (p3View) {
					p3View->setCenterInView(objectPosition->x / MAP_XY_FACTOR, objectPosition->y / MAP_XY_FACTOR);
				}
				return;
			}
		}

		mapObject = mapObject->getNext();
	}
}


void CAboutDlg::OnCenterOnSelectedButtonObject() 
{
	OnCenterOnSelected();
}

void CAboutDlg::OnCenterOnSelectedButtonWP() 
{
	OnCenterOnSelected(false);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bLaunchOnStartUpAbout=::AfxGetApp()->GetProfileInt(ABOUT_SECTION, "LaunchOnStartUp", 1);
	CButton *pButton = (CButton*)GetDlgItem(IDC_LAUNCH_ONSTARTUP);
	pButton->SetCheck(m_bLaunchOnStartUpAbout ? 1:0);

	OnRefreshQueryObject();
	OnRefreshQueryWaypoint();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

void CAboutDlg::OnLaunchOnStartup()
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_LAUNCH_ONSTARTUP);
	m_bLaunchOnStartUpAbout = (pButton->GetCheck() == 1);
	::AfxGetApp()->WriteProfileInt(ABOUT_SECTION, "LaunchOnStartUp", m_bLaunchOnStartUpAbout ? 1 : 0);
}

void CAboutDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
    if (IsIconic())
    {
        // Stop the OS from repositioning the minimized window
        lpwndpos->flags |= SWP_NOMOVE;
    }

    CDialog::OnWindowPosChanging(lpwndpos);
}


void CAboutDlg::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	
	if (this->IsWindowVisible() && !this->IsIconic()) {
		CRect frameRect;
		GetWindowRect(&frameRect);
		::AfxGetApp()->WriteProfileInt(ABOUT_SECTION, "Top", frameRect.top);
		::AfxGetApp()->WriteProfileInt(ABOUT_SECTION, "Left", frameRect.left);
	}
	
}

void CAboutDlg::OnOK() 
{
    // Check if the IDC_OBJECT_SEARCH_EDIT control is focused
    if (GetFocus() == GetDlgItem(IDC_FIND_QUERY) || GetFocus() == GetDlgItem(IDC_HOTKEYLIST) )
    {
        OnFindButtonClicked();  // Trigger search on "Enter" key press
    }
    // else
    // {
    //     CDialog::OnOK();  // Call the default OK behavior if the search box isn't focused
    // }
}

void CAboutDlg::OnFindButtonClicked()
{
	CEdit* pFindBox = (CEdit*)GetDlgItem(IDC_FIND_QUERY);
	CEdit* pHotkeyList = (CEdit*)GetDlgItem(IDC_HOTKEYLIST);
	if (!pFindBox || !pHotkeyList) return;

	CString query, content;
	pFindBox->GetWindowText(query);
	pHotkeyList->GetWindowText(content);

	if (query.IsEmpty())
	{
		MessageBeep(MB_ICONEXCLAMATION);
		pHotkeyList->SetSel(-1, -1); // Clear selection
		return;
	}

	// Case-insensitive search
	CString contentLower = content;
	CString queryLower = query;
	contentLower.MakeLower();
	queryLower.MakeLower();

	int startChar = 0, endChar = 0;
	pHotkeyList->GetSel(startChar, endChar);

	// Start search just after current selection
	int pos = contentLower.Find(queryLower, endChar);

	// Wrap around if not found
	if (pos == -1 && endChar > 0)
		pos = contentLower.Find(queryLower, 0);

	if (pos != -1)
	{
		pHotkeyList->SetFocus();
		pHotkeyList->SetSel(pos, pos + query.GetLength());
		// GetDlgItem(IDC_FIND_QUERY)->SetFocus();
	}
	else
	{
		MessageBeep(MB_ICONEXCLAMATION);
		pHotkeyList->SetSel(-1, -1); // Clear selection if not found
		// GetDlgItem(IDC_FIND_QUERY)->SetFocus();
	}
}



// App command to run the dialog
void CWorldBuilderApp::OnAppAbout()
{

	CAboutDlg* pAboutDlg = new CAboutDlg;

	if (pAboutDlg->Create(IDD_ABOUTBOX, AfxGetMainWnd()))
	{
		int top  = ::AfxGetApp()->GetProfileInt(ABOUT_SECTION, "Top", -1);
		int left = ::AfxGetApp()->GetProfileInt(ABOUT_SECTION, "Left", -1);

		if (top != -1 && left != -1)
		{
			CRect rect;
			pAboutDlg->GetWindowRect(&rect);
			int width  = rect.Width();
			int height = rect.Height();

			pAboutDlg->SetWindowPos(NULL, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
		}

		pAboutDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		delete pAboutDlg;
	}

	CEdit* pEdit = (CEdit*)pAboutDlg->GetDlgItem(IDC_HOTKEYLIST);

	struct HotkeyEntry
	{
		const char* key;
		const char* description;
	};
	
	HotkeyEntry hotkeys[] =
	{
		{ "Tab", "Lock Horizontal" },
		{ "Q", "Brush Tool" },
		{ "W", "Add Brush" },
		{ "E", "Subtract Brush" },
		{ "R", "Feather Tool" },
		{ "T", "Mold Tool  " }, // artificially spaced it out -- theres a bug with tabs
		{ "Alt+1", "Show Objects" },
		{ "Alt+2", "Show Waypoints" },
		{ "Alt+3", "Show Polygon Triggers" },
		{ "Alt+4", "Show Labels" },
		{ "Alt+5", "Show Models" },
		{ "Alt+6", "Show Bounding Boxes" },
		{ "Alt+7", "Show Sight Ranges" },
		{ "Alt+8", "Show Weapon Ranges" },
		{ "Alt+9", "Show Map Boundaries" },
		{ "Alt+0", "Show Terrain" },
		{ "Y", "Water Tool" },
		{ "A", "Tile Tool    " }, // artificially spaced it out -- theres a bug with tabs
		{ "S", "Big Tile Tool" },
		{ "D", "Tile Flood Fill" },
		{ "F", "Auto Edge Out Tool" },
		{ "G", "Blend Edge Tool" },
		{ "Z", "Place Object Tool" },
		{ "X", "Road Tool" },
		{ "C", "Grove Tool" },
		{ "V", "Ramp Tool" },
		{ "B", "Scorch Tool" },
		{ "N", "Fence Tool" },
		{ "M", "Build List Tool" },
		{ "F1", "Waypoint Tool" },
		{ "F2", "Polygon Tool" },
		{ "F3", "Border Tool" },
		{ "F4", "Script Editor" },
		{ "F5", "Team Editor" },
	};
	
	CString text;
	int numHotkeys = sizeof(hotkeys) / sizeof(hotkeys[0]);
	const int maxFirstColumnLength = 17; // You can tweak this number (roughly key + description length)
	
	for (int i = 0; i < numHotkeys; i++)
	{
		CString firstEntry;
		firstEntry.Format("%s\t%s", hotkeys[i].key, hotkeys[i].description);
	
		if (strlen(hotkeys[i].description) >= maxFirstColumnLength || (i + 1) >= numHotkeys)
		{
			// Only one entry on this line
			text += firstEntry;
			text += "\t";
			text += NEWLINE;
		}
		else
		{
			// Two entries per line
			CString secondEntry;
			secondEntry.Format("%s\t%s", hotkeys[i + 1].key, hotkeys[i + 1].description);
	
			text += firstEntry;
			text += "\t"; // space between columns
			text += secondEntry;
			text += "\t";
			text += NEWLINE;
	
			i++; // skip the next one because we already used it
		}
	}
	
	pEdit->SetWindowText(text);	
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
	#ifdef MEMORYPOOL_DEBUG
		TheMemoryPoolFactory->debugMemoryReport(REPORT_POOLINFO | REPORT_POOL_OVERFLOW | REPORT_SIMPLE_LEAKS, 0, 0);
	#endif
	shutdownMemoryManager();
	DEBUG_SHUTDOWN();

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
