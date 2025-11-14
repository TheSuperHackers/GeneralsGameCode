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

/****************************************************************************
 **	C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S			  ***
 ****************************************************************************
 *																			*
 *                  Project Name : Autorun									*
 *																			*
 *                     File Name : AUTORUN.cpp								*
 *																			*
 *                    Programmers: Maria del Mar McCready Legg				*
 *																			*
 *                    Start Date : September 5, 1997						*
 *																			*
 *                   Last Update : October 15, 2001  [MML]					*
 *																			*
 *--------------------------------------------------------------------------*
 * Functions:																*
 *		WinMain																*
 *		Main::MessageLoop													*
 *		MainWindow::Register												*
 *		MainWindow::MainWindow												*
 *		MainWindow::Window_Proc												*
 *		MainWindow::Is_Product_Registered									*
 *		MainWindow::Run_Explorer											*
 *		MainWindow::Run_Game												*
 *		MainWindow::Run_Setup												*
 *		MainWindow::Run_New_Account											*
 *		MainWindow::Run_Register											*
 *		MainWindow::Run_Auto_Update											*
 *		MainWindow::Run_Uninstall											*
 *		MainWindow::Create_Buttons											*
 *		Wnd_Proc															*
 *		Dialog_Box_Proc														*
 *		Stop_Sound_Playing													*
 *		Options																*
 *		Valid_Environment													*
 *		LoadResourceBitmap													*
 *		CreateDIBPalette													*
 *		LoadResourceButton													*
 *		Cant_Find_MessageBox												*
 *		Error_Message														*
 *		Path_Add_Back_Slash													*
 *		Path_Remove_Back_Slash												*
 *		PlugInProductName													*
 *		Fix_Single_Ampersands												*
 *		Fix_Double_Ampersands												*
 *		LaunchObjectClass::LaunchObjectClass								*
 *		LaunchObjectClass::SetPath											*
 *		LaunchObjectClass::SetArgs											*
 *		LaunchObjectClass::Launch											*
 *--------------------------------------------------------------------------*/


#define  STRICT
#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <ctype.h>
#include <direct.h>
#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <Utility/fstream_adapter.h>
#include <io.h>
#include <locale.h>
#include <math.h>
#include <mbctype.h>
#include <mmsystem.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <Utility/stdio_adapter.h>
#include <sys/stat.h>
#include <time.h>
#include <winuser.h>
#include "ARGS.h"
#include "autorun.h"
#include "DrawButton.h"
#include "resource.h"
#include "Wnd_File.h"
//#include "visualc.h"
#include "WinFix.h"
#include "CDCNTRL.h"
#include "IGR.h"
#include "ViewHTML.h"

#include "Utils.h"
#include "Locale_API.h"
//#include "resources.h"
#include "GetCD.h"

#include "WSYS_FileSystem.h"
#include "WSYS_StdFileSystem.h"

#include <string>
#include "GameText.h"

#include "leanAndMeanAutorun.h"

#ifndef LEAN_AND_MEAN
#include "Common/SubsystemInterface.h"
#include "GameClient/GameText.h"
#include "Common/UnicodeString.h"
#include "Win32Device/Common/Win32LocalFileSystem.h"
#include "Win32Device/Common/Win32BIGFileSystem.h"
#endif

//-----------------------------------------------------------------------------
//  DEFINES
//-----------------------------------------------------------------------------
#define		PRETEND_ON_CD_TEST				FALSE		// should be FALSE
//#define	PRETEND_ON_CD_TEST				TRUE		// should be FALSE

#define		WINDOW_BRUSH					FALSE
#define		BACKGROUND_BITMAP				TRUE
#define		USE_MOUSE_MOVES					TRUE
#define		DISABLE_KEYBOARD				FALSE

#define		BUTTON_WIDTH 					150
#define		BUTTON_HEIGHT					45
#define		NUM_BUTTONS						10
#define		NUM_ARGUMENTS					10
#define		NUM_SONGS						2		//32 //16
#define		NUM_FLICKER_FRAMES				1
#define		NUM_FLICKER_POSITIONS			15

#define		MOUSE_WAV						"MouseMove"
#define		SOUND_FILE1						"SPEECH_FILE1"
#define		SOUND_FILE2						"SPEECH_FILE2"

#define		MOH_DEMO_PROGRAM				"MOHAADEMO\\SETUP.EXE"
#define		SHOW_MOH_DEMO					FALSE

#define		BFAVI_FILENAME					"Autorun\\BF1942RTR.avi"
#define		SC4AVI_FILENAME					"Autorun\\Preview.avi"
#define		HELP_FILENAME					"HELP:FILENAME"//"Support\\eahelp.hlp"

#define		SHOW_GAMESPY_BUTTON				FALSE
#define		GAMESPY_WEBSITE					"http://www.gamespyarcade.com/features/launch.asp?svcname=ccrenegade&distID=391"

#define		RESOURCE_FILE					"Autorun.loc"
#define		SETUP_INI_FILE1					"Setup\\Setup.ini"
#define		SETUP_INI_FILE2					"Setup.ini"

#define		UNINSTALL_EXECUTABLE			"IDriver.exe"			// JFS

//-----------------------------------------------------------------------------
// These defines need the Product name from Setup.ini to complete.
//-----------------------------------------------------------------------------
#define		SETUP_MAIN_WINDOW_NAME			"%s Setup"
#define		CLASS_NAME						"%s Autorun"
#define		GAME_MAIN_WINDOW_NAME			"%s Game Window"

//#define	GAME_WEBSITE					"http://www.westwood.com/"
#define		GAME_WEBSITE					"http://generals.ea.com"

#define		AUTORUN_MUTEX_OBJECT			"01AF9993-3492-11d3-8F6F-0060089C05B1"
//#define		GAME_MUTEX_OBJECT			"C6D925A3-7A9B-4ca3-866D-8B4D506C3665"
#define		GAME_MUTEX_OBJECT				"685EAFF2-3216-4265-B047-251C5F4B82F3"
#define		PRODUCT_VOLUME_CD1	 			"Generals1"
#define		PRODUCT_VOLUME_CD2	 			"Generals2"


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
LaunchObjectClass	LaunchObject;
MainWindow			*GlobalMainWindow	= NULL;
int					Language			= 0;
int					LanguageToUse		= 0;

DrawButton *ButtonList			[ NUM_BUTTONS ];
RECT 		ButtonSizes			[ NUM_BUTTONS ];
char		ButtonImages		[ NUM_BUTTONS ][ MAX_PATH ];
CHAR		szSongPath			[ MAX_PATH ];
char	 	FocusedButtonImages	[ NUM_BUTTONS ][ MAX_PATH ];
char 		Arguments			[ NUM_ARGUMENTS ][ 30 ];
char	 	szWavs				[ NUM_SONGS][ _MAX_PATH ];
char 		szBuffer 	  		[ MAX_PATH ];
char 		szBuffer1			[ MAX_PATH ];
char 		szBuffer2			[ MAX_PATH ];
char	 	szBuffer3			[ MAX_PATH * 2];
char 		szInternetPath		[_MAX_PATH];
char 		szGamePath			[_MAX_PATH];
char		szWorldbuilderPath	[_MAX_PATH];
char		szPatchgetPath	[_MAX_PATH];
char	 	szSetupPath			[_MAX_PATH];
char 		szUninstallPath		[_MAX_PATH];
char		szUninstallCommandLine[_MAX_PATH];		// JFS: Returned value contains parameters needed.
char	 	szRegisterPath		[_MAX_PATH];
char		szButtonWav	 		[_MAX_PATH ];
char 		szSpeechWav			[_MAX_PATH ];
char	 	szArgvPath			[_MAX_PATH ];
char	 	drive				[_MAX_DRIVE];
char 		dir	 				[_MAX_DIR  ];
char 		szSetupWindow		[_MAX_PATH];
char	 	szGameWindow		[_MAX_PATH];
char	 	szRegistryKey		[_MAX_PATH];
char 		szClassName			[_MAX_PATH];
char 		szVolumeName		[_MAX_PATH];

char		szProduct_Name		[ _MAX_PATH ];


#ifdef LEAN_AND_MEAN

wchar_t 	szWideBuffer   		[ _MAX_PATH ];
wchar_t 	szWideBuffer0  		[ _MAX_PATH ];
wchar_t 	szWideBuffer2  		[ _MAX_PATH ];
wchar_t 	szWideBuffer3  		[ _MAX_PATH ];
wchar_t		szProductName		  [ _MAX_PATH ];
wchar_t		szFullProductName	[ _MAX_PATH ];

/*
enum
{
	IDS_INSTALL,
	IDS_EXPLORE_CD,
	IDS_PREVIEWS,
	IDS_CANCEL,
	IDS_AUTORUN_TITLE,
	IDS_CANT_FIND_IEXPLORER,
	IDS_WINDOWS_VERSION_TEXT,
	IDS_WINDOWS_VERSION_TITLE,
	IDS_CANT_FIND,
	IDS_UNINSTALL,
	IDS_WEBSITE,
	IDS_CHECKFORUPDATES,
	IDS_WORLDBUILDER,
	IDS_PLAY,

	IDS_GAME_TITLE,
	IDS_FULL_GAME_TITLE,
	IDS_REGISTRY_KEY,
	IDS_MAIN_WINDOW,


	IDS_COUNT
};
*/

#else

UnicodeString wideBuffer;
UnicodeString wideBuffer0;
UnicodeString wideBuffer2;
UnicodeString wideBuffer3;
UnicodeString	productName;
UnicodeString fullProductName;

#endif



bool		IsEnglish				= FALSE;
bool		UseSounds				= FALSE;
bool		b640X480				= FALSE;
bool		b800X600				= FALSE;
BOOL		OnCDRom 				= FALSE;
BOOL		IAmWindows95			= FALSE;
BOOL		InstallProduct			= TRUE;
BOOL		UninstallAvailable		= FALSE;
BOOL		IsUserRegistered		= FALSE;
BOOL		DisplayRegisterButton	= FALSE;
BOOL		IsWolapiAvailable		= FALSE;
BOOL		CDLocked				= FALSE;
int			WindowsVersion 			= 0;
int			NumberArguments			= 0;
int			SongNumber 				= 0;
HANDLE		AppMutex				= NULL;
HANDLE		GameAppMutex			= NULL;
HANDLE		SetupAppMutex	 		= NULL;




#ifdef LEAN_AND_MEAN

extern FileSystem *TheFileSystem;

#else

extern GameTextInterface *TheGameText;
extern LocalFileSystem *TheLocalFileSystem;
extern ArchiveFileSystem *TheArchiveFileSystem;

#endif

// stuff needed to compile.
HWND		ApplicationHWnd = NULL;
HINSTANCE ApplicationHInstance;				///< main application instance

const char *g_strFile = "Autorun.str";
const char *g_csfFile = "Autorun.csf";

const char *gAppPrefix = "ar_"; // prefix to the debug log.

int			FlickerPositions[ NUM_FLICKER_POSITIONS ][2];

#if( RTS_DEBUG )
char		szCDDrive[ MAX_PATH ];
#endif


//-----------------------------------------------------------------------------
// Global Function Definitions
//-----------------------------------------------------------------------------
void 		Cant_Find_MessageBox		( HINSTANCE hInstance, const char *szPath );
HPALETTE	CreateDIBPalette 			( LPBITMAPINFO lpbmi, LPINT lpiNumColors );
void		Debug_Date_And_Time_Stamp	( void );

void 		Error_Message				( HINSTANCE hInstance, int title, int string, const char *path );
void 		Error_Message				( HINSTANCE hInstance, const char * title, const char * string, const char *path );

bool		Is_On_CD					( const char * );
HBITMAP 	LoadResourceBitmap			( HMODULE hInstance, LPCTSTR lpString, HPALETTE FAR *lphPalette, bool loading_a_button=FALSE );
HBITMAP 	LoadResourceButton			( HMODULE hInstance, LPCTSTR lpString, HPALETTE FAR lphPalette );
BOOL 		Options						( Command_Line_Arguments *Orgs );
void		Prog_End					( void );
bool		Prompt_For_CD				( HWND window_handle, char *volume_name, const char * message1, const char * message2, int *cd_drive );
void		Reformat_Volume_Name		( const char *volume_name, char *new_volume_name );
int			Show_Message				( HWND window_handle, const char * message_num1, const char * message_num2 );
int			Show_Message				( HWND window_handle, int message_num1 );
void		Stop_Sound_Playing			( void );
BOOL 		Valid_Environment 			( void );

BOOL 		CALLBACK	Dialog_Box_Proc	( HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param );
LRESULT 	CALLBACK	Wnd_Proc 		( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam );


//-----------------------------------------------------------------------------
// Main & MainWindow Public Variables
//-----------------------------------------------------------------------------
HINSTANCE	Main::hInstance 	= 0;
HINSTANCE	Main::hPrevInstance	= 0;
HMODULE		Main::hModule		= 0;
int 		Main::nCmdShow 		= 0;
char   		MainWindow::szClassName[] = CLASS_NAME;


//*****************************************************************************
// WinMain -- 	Main Program Loop.
//
// INPUT:		HINSTANCE hInstance
//					HINSTANCE hPrevInstance
//					LPTSTR lpszCmdLine
//					int nCmdShow
//
// OUTPUT:		int.
//
// WARNINGS:	none.
//
// HISTORY:
//   06/04/1996	MML : Created.
//=============================================================================

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow )
{

	int		i = 0;
	char	szPath[ _MAX_PATH ];
	char	szIniPath[ _MAX_PATH ];

	HANDLE				handle;
	WIN32_FIND_DATA		FindFileData;

	Main::hInstance		= hInstance;
	ApplicationHInstance = hInstance;
	Main::hPrevInstance	= hPrevInstance;
	Main::nCmdShow		= nCmdShow;
	Main::hModule 		= GetModuleHandle( NULL );

	memset( szSetupWindow,		'\0', MAX_PATH );
	memset( szGameWindow,		'\0', MAX_PATH );
	memset( szProductName,		'\0', MAX_PATH );
	memset( szFullProductName,	'\0', MAX_PATH );
	memset( szRegistryKey,		'\0', MAX_PATH );
	memset( szClassName,		'\0', MAX_PATH );
	memset( szVolumeName,		'\0', MAX_PATH );

	Msg( __LINE__, __FILE__, "Entering WinMain." );

	//-------------------------------------------------------------------------
	// Set Cleanup function.
	//-------------------------------------------------------------------------
	atexit( Prog_End );

	//-------------------------------------------------------------------------
	// Clear Argument Array.
	//-------------------------------------------------------------------------
	for ( i = 0; i < NUM_ARGUMENTS; i++ ) {
		memset( Arguments[i], '\0', sizeof( Arguments[i] ));
	}
	for ( i = 0; i < NUM_SONGS; i++ ) {
		memset( szWavs[i], '\0', sizeof( szWavs[i] ));
	}

	//-------------------------------------------------------------------------
	// Init Args class.
	//-------------------------------------------------------------------------
	Args = new Command_Line_Arguments( hInstance, GetCommandLine());
	if ( Args == NULL ) {
//		Error_Message( hInstance, IDS_ERROR, IDS_COMMAND_LINE_ERR, NULL );
		Error_Message( hInstance, "Autorun:Error", "Autorun:CommandLineError", NULL );
		return( 0 );
	}
	Msg( __LINE__, __FILE__, "Args Created." );

#if( PRETEND_ON_CD_TEST )
	strcpy( szCDDrive, "E:\\" );
	Options( Args );
	strcat( szCDDrive, "autorun.exe" );
	Msg( __LINE__, __FILE__, "szCDDrive = %s.", szCDDrive );
	Args->Set_argv( 0, szCDDrive );
#endif

	//-------------------------------------------------------------------------
	// Get the CD volume to check on which CD in the product we are on.
	//-------------------------------------------------------------------------
	strcpy( szBuffer, Args->Get_argv(0));
	CDList.Get_Volume_For_This_CD_Drive( szBuffer, szVolumeName );

	Msg( __LINE__, __FILE__, "szVolumeName  = %s.", szVolumeName );


#ifdef LEAN_AND_MEAN

	TheFileSystem = new StdFileSystem;

#else

	// create singletons
	TheFileSystem->init();
	TheLocalFileSystem = new Win32LocalFileSystem;
	TheArchiveFileSystem = new Win32BIGFileSystem;
	TheGameText = CreateGameTextInterface();
	TheGameText->init();

#endif

	//=========================================================================
	// Make paths to .INI and .LOC files.
	//=========================================================================
	Make_Current_Path_To( RESOURCE_FILE, szPath );
	Make_Current_Path_To( SETUP_INI_FILE1, szIniPath );

	handle = FindFirstFile( szIniPath, &FindFileData );
	if ( handle == INVALID_HANDLE_VALUE ) {

		//---------------------------------------------------------------------
		// This might be a secondary CD.
		//---------------------------------------------------------------------
		Make_Current_Path_To( SETUP_INI_FILE2, szIniPath );
	}
	FindClose( handle );

	Msg( __LINE__, __FILE__, "Resource file = %s.", szPath );
	Msg( __LINE__, __FILE__, "Setup.ini file = %s.", szIniPath );

	//--------------------------------------------------------------------------
	// Get the language we are using from the Setup.ini file.
	//--------------------------------------------------------------------------
	Language = GetPrivateProfileInt( "Setup", "Language", 0, szIniPath );

	if( Language == 0 ) {
		IsEnglish = true;
	}

	Msg( __LINE__, __FILE__, "Language = %d.", Language );
	Msg( __LINE__, __FILE__, "IsEnglish = %d.", IsEnglish );

	//--------------------------------------------------------------------------
	// Set language to use.
	//--------------------------------------------------------------------------
	if( Locale_Use_Multi_Language_Files()) {

		if( IS_LANGUAGE_DBCS( Language ) && !IS_CODEPAGE_DBCS( CodePage )) {
			Language = LANG_USA;
		}

		/*switch( Language ) {

			case LANG_GER:
				LanguageToUse = IDL_GERMAN;
				break;

			case LANG_FRE:
				LanguageToUse = IDL_FRENCH;
				break;

			case LANG_JAP:
				LanguageToUse = IDL_JAPANESE;
				break;

			case LANG_KOR:
				LanguageToUse = IDL_KOREAN;
				break;

			case LANG_CHI:
				LanguageToUse = IDL_CHINESE;
				break;

			case LANG_USA:
			default:
				LanguageToUse = IDL_ENGLISH;
				break;
		}
		*/
		LanguageToUse = Language;

	}
	Msg( __LINE__, __FILE__, "LanguageToUse = %d.", LanguageToUse );

	//-------------------------------------------------------------------------
	// Process the Command Line Options.  This may change the language id.
	//-------------------------------------------------------------------------
	if ( !Options( Args )) {
		return( 0 );
	}

	//-------------------------------------------------------------------------
	// Save off the Current path for use by other stuff.
	//-------------------------------------------------------------------------
	_tcscpy( szArgvPath, Args->Get_argv(0));
	_tsplitpath( szArgvPath, drive, dir, NULL, NULL );
	_tmakepath ( szArgvPath, drive, dir, NULL, NULL );
	Path_Add_Back_Slash( szArgvPath );
	Msg( __LINE__, TEXT(__FILE__), TEXT("szArgvPath = %s."), szArgvPath );

	Msg( __LINE__, __FILE__, "About to Init text strings." );

	//=========================================================================
	// Init the strings chosen.
	//=========================================================================


	Locale_Init( LanguageToUse, szPath );

	/*
	if( !Locale_Init( LanguageToUse, szPath )) {
		LoadString( Main::hInstance, IDS_CANT_FIND_FILE, szBuffer1, _MAX_PATH );
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szPath, _MAX_PATH, szWideBuffer0, _MAX_PATH );
		sprintf( szBuffer2, szBuffer1, szWideBuffer0 );
		MessageBox( NULL, szBuffer2, "Autorun", MB_APPLMODAL | MB_OK );
		return 0;
	}
	*/


	//-------------------------------------------------------------------------
	// Get some pertinent strings.
	//-------------------------------------------------------------------------
	Locale_GetString( "Autorun:Title",		szProductName );
	Locale_GetString( "Autorun:Command&ConquerGenerals",		szFullProductName );
	//Locale_GetString( IDS_GAME_TITLE,	szProductName );
	//Locale_GetString( IDS_FULL_GAME_TITLE,	szFullProductName );
//	Locale_GetString( IDS_REGISTRY_KEY,		szRegistryKey );			// jfs
//	Locale_GetString( IDS_MAIN_WINDOW,		szGameWindow );


	//-------------------------------------------------------------------------
	// Set other variables used through out.
	//-------------------------------------------------------------------------
#ifdef LEAN_AND_MEAN

	//Fix_Single_Ampersands( &szProductName[0], false );
	//Fix_Single_Ampersands( &szFullProductName[0], false );
	Msg( __LINE__, __FILE__, "szProductName		= %s.", szProductName		);
	WideCharToMultiByte( CodePage, 0, szProductName, _MAX_PATH, szProduct_Name, _MAX_PATH, NULL, NULL );

#else

	productName = TheGameText->fetch("Autorun:Generals");
	fullProductName = TheGameText->fetch("Autorun:Command&ConquerGenerals");
  Msg( __LINE__, __FILE__, "Product Name = %ls.", productName.str() );
	Msg( __LINE__, __FILE__, "Full Product Name = %ls.", fullProductName.str()	);
	Msg( __LINE__, __FILE__, "szRegistryKey		= %s.", szRegistryKey		);
	Msg( __LINE__, __FILE__, "szGameWindow		= %s.", szGameWindow		);
	WideCharToMultiByte( CodePage, 0, productName.str(), productName.getLength()+1, szProduct_Name, _MAX_PATH, NULL, NULL );

#endif

	sprintf( szClassName, CLASS_NAME, szProduct_Name );
	MainWindow::Reset_Class_Name( szClassName );
	sprintf( szSetupWindow, SETUP_MAIN_WINDOW_NAME, szProduct_Name );

	Msg( __LINE__, __FILE__, "szClassName		= %s.", szClassName		);
	Msg( __LINE__, __FILE__, "szSetupWindow		= %s.", szSetupWindow	);

	//=========================================================================
	//	Create a mutex with a unique name to Autorun in order to determine if
	//	our app is already running.
	//
	//	Return Values:
	//	If the function succeeds, the return value is a handle to the mutex object.
	//	If the named mutex object existed before the function call, the function returns
	//	a handle to the existing object and GetLastError returns ERROR_ALREADY_EXISTS.
	//	Otherwise, the caller created the mutex.
	//	If the function fails, the return value is NULL. To get extended error
	//	information, call GetLastError.
	//
	// WARNING: DO NOT use this number for any other application except Autorun
	//=========================================================================
	if( WinVersion.Is_Win_XP() || WinVersion.Version() > 500 ) {
		strcat( strcpy( szBuffer, "Global\\" ), AUTORUN_MUTEX_OBJECT );
	} else {
		strcpy( szBuffer, AUTORUN_MUTEX_OBJECT );
	}
	AppMutex = CreateMutex( NULL, FALSE, szBuffer );

	if ( AppMutex != NULL && ( GetLastError() == ERROR_ALREADY_EXISTS )) {

		Msg( __LINE__, __FILE__, "AppMutex of %s already exists. Exit here.", szBuffer );

		//---------------------------------------------------------------------
		// Handle is closed in the ProgEnd().
		//---------------------------------------------------------------------

		//---------------------------------------------------------------------
		// Check if Game/Setup is already running, and is looking for the CDRom.
		//---------------------------------------------------------------------
		HWND prev = FindWindow( szClassName, NULL );
		if( prev ){
			//if( IsIconic( prev )){
				//ShowWindow( prev, SW_RESTORE );
			//}
			SetForegroundWindow( prev );
		}
		return 0;
	}
	Msg( __LINE__, __FILE__, "AppMutex of %s created.", szBuffer );

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// if AppMutex was NULL, let through. Perhaps in future we want to trap it?
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if ( AppMutex == NULL ) {
	}

	//=========================================================================
	//	Create a mutex with a unique name to Game/Setup in order to
	//	determine if our app is already running.
	//
	//	Return Values
	//	If the function succeeds, the return value is a handle to the mutex object.
	//	If the function fails, the return value is NULL. To get extended error
	//	information, call GetLastError.
	//
	// WARNING: DO NOT use this number for any other application except Game/Setup.
	//=========================================================================
	if( WinVersion.Is_Win_XP() || WinVersion.Version() > 500 ) {
		strcat( strcpy( szBuffer, "Global\\" ), GAME_MUTEX_OBJECT );
	} else {
		strcpy( szBuffer, GAME_MUTEX_OBJECT );
	}
	GameAppMutex = OpenMutex( MUTEX_ALL_ACCESS, FALSE, szBuffer );

	if ( GameAppMutex != NULL ) {

		Msg( __LINE__, TEXT(__FILE__), TEXT("Mutex Object of game found."));
		Msg( __LINE__, TEXT(__FILE__), TEXT("Looking for Game Window."));

		HWND ccwindow = FindWindow( szGameWindow, NULL );
		if ( ccwindow ) {

			Msg( __LINE__, TEXT(__FILE__), TEXT("Found Game Window."));

			if( IsIconic( ccwindow )){
				ShowWindow( ccwindow, SW_RESTORE );
			}
			SetForegroundWindow( ccwindow );

		} else {

			Msg( __LINE__, TEXT(__FILE__), TEXT("Looking for Setup Window."));

			ccwindow = FindWindow( szSetupWindow, NULL );
			if ( ccwindow ) {

				Msg( __LINE__, TEXT(__FILE__), TEXT("Found Setup Window."));

				if( IsIconic( ccwindow )){
					ShowWindow( ccwindow, SW_RESTORE );
				}
				BOOL result = SetForegroundWindow( ccwindow );
				Msg( __LINE__, TEXT(__FILE__), TEXT("SetForegroundWindow = %d."), result );
			}
		}

		//---------------------------------------------------------------------
		// Handle(s) are closed in the ProgEnd().
		//---------------------------------------------------------------------

		return 0;
	}

	//---------------------------------------------------------------------
	// Check if Game/Setup is already running, and is looking for the CDRom.
	//---------------------------------------------------------------------
	HWND prev = FindWindow( szClassName, NULL );
	if ( prev == NULL ) {
		prev = FindWindow( szGameWindow, NULL );
		if ( prev == NULL ) {
			prev = FindWindow( szSetupWindow, NULL );
		}
	}
	if( prev ){
		if( IsIconic( prev )){
			ShowWindow( prev, SW_RESTORE );
		}
		SetForegroundWindow( prev );

		Msg( __LINE__, __FILE__, "Either same app, game or setup was found. Exit and set window to foreground." );
		return 0;
	}

	//---------------------------------------------------------------------
	// Check if another installshield session is already running.  This happens
	// because we ask the user to insert CD-1 again at the end of the install
	// to prevent a crash on Windows ME where it tries to access CD-1 again.
	//---------------------------------------------------------------------
	prev = FindWindow( NULL,"InstallShield Wizard");
	if( prev ){
		return 0;
	}

	//=========================================================================
	// Select Sounds.
	//=========================================================================
	memset( szButtonWav, '\0', _MAX_PATH );
	_tcscpy( szButtonWav, _TEXT( MOUSE_WAV ));

//	memset( szSpeechWav, '\0', _MAX_PATH );
//	_tcscpy( szSpeechWav, _TEXT( AUTORUN_WAV ));

	if( LanguageID == LANG_USA ) {

		strcpy( szWavs[0], SOUND_FILE1 );
		strcpy( szWavs[1], SOUND_FILE2 );

		// Pick a number between 0 and 1.  ( NUM_SONGS - 1 )
		// Seed the random-number generator with current time so that
	    // the numbers will be different every time we run.

		Msg( __LINE__, __FILE__, "szWav[0] = %s.", szWavs[0] );
		Msg( __LINE__, __FILE__, "szWav[1] = %s.", szWavs[1] );

		srand(( unsigned )time( NULL ));
		SongNumber	= rand() & 1;
//		UseSounds	= TRUE;

		Msg( __LINE__, __FILE__, "SongNumber  = %d.", SongNumber );
		Msg( __LINE__, __FILE__, "UseSounds   = %d.", UseSounds );
	}

	//-------------------------------------------------------------------------
	// Get the CD volume to check on which CD in the product we are on.
	//-------------------------------------------------------------------------
//	strcpy( szBuffer, Args->Get_argv(0));
//	CDList.Get_Volume_For_This_CD_Drive( szBuffer, szVolumeName );

//	Msg( __LINE__, __FILE__, "szVolumeName  = %s.", szVolumeName );

	//-------------------------------------------------------------------------
	// Check if we are on the CD-ROM and in Windows 95.
	//-------------------------------------------------------------------------
	if ( !Valid_Environment(  )) {
		return( -1 );
	}

	//=========================================================================
	// Lock the CD!
	//=========================================================================
//	memset( szPath, '\0', _MAX_PATH );
//	szPath[0] = (char)( toupper( szArgvPath[0] ));
//	strcat( szPath, ":\\" );

	//-------------------------------------------------------------------------
	// NOTE: Based on A=0, B=1, etc.
	//-------------------------------------------------------------------------
//#if(!PRETEND_ON_CD_TEST)
	char driveLetter = Args->Get_argv(0)[0];
	Msg( __LINE__, TEXT(__FILE__), TEXT("About to lock on CD: %c:\\ "), toupper( driveLetter ));
//	CDLocked = CDControl.Lock_CD_Tray((unsigned)( toupper( szPath[0] ) - 'A' ));
	CDLocked = CDControl.Lock_CD_Tray((unsigned)( toupper( driveLetter ) - 'A' ));
	Msg( __LINE__, TEXT(__FILE__), TEXT("CDLocked = %d. "), CDLocked );
//#endif

	//=========================================================================
	// Set the buttons images. Use when images are unique for each button.
	//=========================================================================
	strcpy( ButtonImages[0], BUTTON_REG );
	strcpy( ButtonImages[1], BUTTON_REG );
	strcpy( ButtonImages[2], BUTTON_REG );
	strcpy( ButtonImages[3], BUTTON_REG );
	strcpy( ButtonImages[4], BUTTON_REG );
	strcpy( ButtonImages[5], BUTTON_REG );
	strcpy( ButtonImages[6], BUTTON_REG );
	strcpy( ButtonImages[7], BUTTON_REG );
	strcpy( ButtonImages[8], BUTTON_REG );
	strcpy( ButtonImages[9], BUTTON_REG );

	strcpy( FocusedButtonImages[0], BUTTON_SEL );
	strcpy( FocusedButtonImages[1], BUTTON_SEL );
	strcpy( FocusedButtonImages[2], BUTTON_SEL );
	strcpy( FocusedButtonImages[3], BUTTON_SEL );
	strcpy( FocusedButtonImages[4], BUTTON_SEL );
	strcpy( FocusedButtonImages[5], BUTTON_SEL );
	strcpy( FocusedButtonImages[6], BUTTON_SEL );
	strcpy( FocusedButtonImages[7], BUTTON_SEL );
	strcpy( FocusedButtonImages[8], BUTTON_SEL );
	strcpy( FocusedButtonImages[9], BUTTON_SEL );

	//=========================================================================
	// A Windows class should be registered with Windows before any windows
	// of that type are created. Register here all Windows classes that
	// will be used in the program.
	//-------------------------------------------------------------------------
	// Register the class only AFTER WinMain assigns appropriate values to
	// static members of Main and only if no previous instances of the program
	// exist (a previous instance would have already performed the registration).
	//=========================================================================
	if ( !Main::hPrevInstance ) {
		MainWindow::Register();
	}

	//-------------------------------------------------------------------------
	// Create MainWnd, calls MainWindow Constructor.
	//-------------------------------------------------------------------------
	MainWindow MainWnd;

	//-------------------------------------------------------------------------
	// Begin processing Window Messages.
	//-------------------------------------------------------------------------
	return( Main::MessageLoop( ));
}

//*****************************************************************************
// Prog_End -- Close all objects that were created.
//
// INPUT:			none.
//
// OUTPUT:			int.
//
// WARNINGS:		none.
//
// HISTORY:
//   01/22/2001  MML : Created.
//=============================================================================

void Prog_End ( void )
{
	//==========================================================================
	// UnLock the CD!
	//==========================================================================
	if( CDLocked ) {
		CDControl.Unlock_CD_Tray((unsigned)( toupper( szArgvPath[0] ) - 'A' ));
		CDLocked = false;
	}

	if( Args != NULL ) {
		delete( Args );
		Args = NULL;
		Msg( __LINE__, __FILE__, "Args deleted." );
	}

	if ( AppMutex != NULL ) {
		CloseHandle( AppMutex );
		AppMutex = NULL;
		Msg( __LINE__, __FILE__, "AppMutex deleted." );
	}

	if ( GameAppMutex != NULL) {
		CloseHandle( GameAppMutex );
		GameAppMutex = NULL;
	}

	if ( FontManager != NULL ) {
   		delete( FontManager );
		FontManager = NULL;
		Msg( __LINE__, __FILE__, "FontManager deleted." );
	}

	if ( OnlineOptions != NULL ) {
		delete( OnlineOptions );
		OnlineOptions = NULL;
		Msg( __LINE__, __FILE__, "OnlineOptions deleted." );
	}

	//-------------------------------------------------------------------------
	// Delete language resource file.
	//-------------------------------------------------------------------------
	Locale_Restore();

	Debug_Date_And_Time_Stamp();
}


//*****************************************************************************
// Main::MessageLoop -- Dispatch Message Loop.
//
// INPUT:			none.
//
// OUTPUT:			int.
//
// WARNINGS:		none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

int Main::MessageLoop( void )
{
	MSG msg;

	while( GetMessage( &msg, NULL, 0, 0 )) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	//--------------------------------------------------------------------------
	// Stop the sound if still going.
	//--------------------------------------------------------------------------
	Stop_Sound_Playing();

	//==========================================================================
	// UnLock the CD!
	//==========================================================================
//#if(!PRETEND_ON_CD_TEST)
	if( CDLocked ) {
		CDControl.Unlock_CD_Tray((unsigned)( toupper( szArgvPath[0] ) - 'A' ));
		CDLocked = false;
	}
//#endif

	//==========================================================================
	// Something to launch?
	//==========================================================================
	if( LaunchObject.Launch_A_Program()) {
		LaunchObject.Launch();
	}

	return( msg.wParam );
}


//*****************************************************************************
// MainWindow::Register -- Register the Main Window.
//
// INPUT:			none.
//
// OUTPUT:			none.
//
// WARNINGS:		none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

void MainWindow::Register( void )
{
	//--------------------------------------------------------------------------
	// Structure used to register Windows class.
	//--------------------------------------------------------------------------
	WNDCLASSEX wndclass;

	//--------------------------------------------------------------------------
	// set up and register window class
	//--------------------------------------------------------------------------
	wndclass.cbSize			= sizeof(WNDCLASSEX);
	wndclass.style			= CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc	= Wnd_Proc;
	wndclass.cbClsExtra		= 0;

	//--------------------------------------------------------------------------
	// Reserve extra bytes for each instance of the window. We will use these
	// bytes to store a pointer to the C++ (MainWindow) object corresponding
	// to the window. The size of a 'this' pointer depends on the memory model.
	//--------------------------------------------------------------------------
	wndclass.cbWndExtra		= sizeof( MainWindow * );
	wndclass.hInstance		= Main::hInstance;
	wndclass.hIcon	  		= LoadIcon( Main::hInstance, MAKEINTRESOURCE(1));

//	strcpy( szBuffer, "C&C2.ICO" );
//	wndclass.hIcon	= (HICON)LoadImage( NULL, szBuffer, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE );

	wndclass.hCursor		= LoadCursor( Main::hInstance, MAKEINTRESOURCE(2) );
	wndclass.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.lpszMenuName	= szClassName;
	wndclass.lpszClassName	= szClassName;
	wndclass.hIconSm		= LoadIcon( Main::hInstance, MAKEINTRESOURCE(1));

	if ( !RegisterClassEx((const WNDCLASSEX *) &wndclass ) ) {

	#if(RTS_DEBUG)
		LPVOID szMessage;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			GetLastError(),
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			(LPTSTR)&szMessage,
			0,
			NULL );

		_stprintf( szBuffer, TEXT( "%s(%lx)" ), szMessage, GetLastError());
		Msg( __LINE__, TEXT(__FILE__), TEXT("GetLastError: %s"), szBuffer );
	#endif

		exit( FALSE );
	}
}

//*****************************************************************************
// MainWindow::MainWindow -- Main Window Constructor function.
//
// INPUT:         none.
//
// OUTPUT:        none.
//
// WARNINGS:      none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

MainWindow::MainWindow( void )
{
	char szTitle[ _MAX_PATH ];
	hWnd = 0;

#ifdef LEAN_AND_MEAN

	WideCharToMultiByte( CodePage, 0, szFullProductName, _MAX_PATH, szBuffer, _MAX_PATH, NULL, NULL );

#else

	WideCharToMultiByte( CodePage, 0, fullProductName.str(), fullProductName.getLength()+1, szBuffer, _MAX_PATH, NULL, NULL );

#endif

	memset( szTitle, '\0', _MAX_PATH );
	sprintf( szTitle, CLASS_NAME, szBuffer );

	//--------------------------------------------------------------------------
	// Create the MainWindow.
	// Pass 'this' pointer in lpParam of CreateWindow().
	//--------------------------------------------------------------------------
	hWnd = CreateWindowEx(
   				0,
		 		szClassName,
		 		szClassName,
		 		WS_POPUPWINDOW | WS_MINIMIZE | !WS_VISIBLE,
		 		0,
		 		0,
				640,
				480,
		 		NULL,
		 		NULL,
		 		Main::hInstance,
		 		(LPTSTR) this );

	if ( !hWnd ) {
		Stop_Sound_Playing();
		exit( FALSE );
	}

	ApplicationHWnd = hWnd;

	//-------------------------------------------------------------------------
	// Save the pointer so we can access it globally.
	//-------------------------------------------------------------------------
	GlobalMainWindow = this;

	//--------------------------------------------------------------------------
	// If you want to see the Background Window, enable these.
	//--------------------------------------------------------------------------
//	Show( Main::nCmdShow );
//	Update();
}

//*****************************************************************************
// MainWindow::Window_Proc -- Main Window Procedure fnc to process msgs.
//
// INPUT:         none.
//
// OUTPUT:        none.
//
// WARNINGS:      none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

LRESULT MainWindow::Window_Proc( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam )
{
	static	RECT	rect;
	static	int		cxChar, cyChar;
	static	HBRUSH	hBrush = 0;
	static	POINT	point;
	int				decision;
	static	int		bits_pixel = 0;

	switch( iMessage ) {

		//-----------------------------------------------------------------------
		// Create Message.
		//-----------------------------------------------------------------------
		case WM_CREATE:
			#if(0)
				hBrush = CreateSolidBrush( GetSysColor( COLOR_WINDOW ));
			#endif
			GetClientRect( GetDesktopWindow(), &rect );
			SendMessage( hWnd, WM_GO, wParam, lParam );
			break;

		//-----------------------------------------------------------------------
		// Go!
		//-----------------------------------------------------------------------
		case WM_GO:

			decision = DialogBox( Main::hInstance, _TEXT( "BitmapDialog" ), hWnd, Dialog_Box_Proc );

			delete( Args );
			Args = NULL;

			Stop_Sound_Playing();

			MoveWindow(	hWnd, 0, 0, 0, 0, TRUE );
			SendMessage( hWnd, WM_DESTROY, wParam, lParam );
			Msg( __LINE__, TEXT(__FILE__), TEXT("---------------------- end of WM_GO ---------------------" ));
			break;

		//-----------------------------------------------------------------------
		// WM_CTLCOLOR Message.
		// wParam			   	Handle to Child Window's device context
		// LOWORD( lParam )		Child Window handle
		// HIWORD( lParam )		Type of Window: 	CTLCOLOR_MSGBOX, _EDIT, _LISTBOX, _BTN, _DLG, _SCROLLBAR, _STATIC
		//-----------------------------------------------------------------------
	#if( WINDOW_BRUSH )
		case WM_CTLCOLOR:
			if ( HIWORD( lParam ) == CTLCOLOR_BTN ) {
				SetBkColor( (HDC)wParam, GetSysColor( COLOR_WINDOW ));
				SetTextColor( (HDC)wParam, GetSysColor( COLOR_WINDOWTEXT ));
				UnrealizeObject( hBrush );							// reset the origin of the brush next time used.
				point.x = point.y = 0;								// create a point.
				ClientToScreen( hWnd, &point );						// translate into screen coordinates.
				SetBrushOrg( (HDC)wParam, point.x, point.y );		// New Origin to use when next selected.
				return ((DWORD) hBrush);
			}
			break;

		//-----------------------------------------------------------------------
		// WM_SYSCOLORCHANGE Message.
		//-----------------------------------------------------------------------
		case WM_SYSCOLORCHANGE:
			DeleteObject( hBrush );
			hBrush = CreateSolidBrush( GetSysColor( COLOR_WINDOW ));
			break;

		//-----------------------------------------------------------------------
		// WM_SIZE Message.
		//-----------------------------------------------------------------------
		case WM_SIZE:
			rect.left   = 24 * cxChar;
			rect.top    = 2 * cyChar;
			rect.right  = LOWORD( lParam );
			rect.bottom = HIWORD( lParam );
			break;
	#endif

		//-----------------------------------------------------------------------
		// WM_COMMAND Message.
		// wParam				Child Window ID
		// LOWORD( lParam )		Child Window handle
		// HIWORD( lParam )		Notification Code: BN_CLICKED, BN_PAINT, etc...
		//-----------------------------------------------------------------------
		case WM_COMMAND:
			break;

		//-----------------------------------------------------------------------
		// WM_DESTROY Message.
		//-----------------------------------------------------------------------
		case WM_DESTROY:
			#if(WINDOW_BRUSH)
				DeleteObject( hBrush );
			#endif
			PostQuitMessage( 0 );
			break;

		default:
			return DefWindowProc( hWnd, iMessage, wParam, lParam );
	}
	return 0;
}


//*****************************************************************************
// MainWindow::Is_Product_Registered -- Check the Registration Table information.
//
// INPUT:  		none.
//
//	OUTPUT: 		BOOL - TRUE if product is already installed, FALSE if not.
//
// WARNINGS:	none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

BOOL MainWindow::Is_Product_Registered( void )
{
	HKEY	phKey;
	BOOL	result = FALSE;

	char 		key			[_MAX_PATH];
	wchar_t 	szPath		[_MAX_PATH];
	char		aName		[_MAX_PATH];			//jfs

	unsigned long Type;
	unsigned long Size = _MAX_PATH;

	HANDLE  handle;
	WIN32_FIND_DATA FindFileData;

	memset( szPath,			'\0', sizeof( szPath ));
	memset( szGamePath,		'\0', sizeof( szGamePath ));
	memset( szSetupPath,	'\0', sizeof( szSetupPath ));
	memset( szRegisterPath, '\0', sizeof( szRegisterPath ));
	memset( szInternetPath, '\0', sizeof( szInternetPath ));
	memset( szUninstallPath,		'\0', sizeof( szUninstallPath ));
	memset( szUninstallCommandLine, '\0', sizeof( szUninstallCommandLine ));
	memset( FindFileData.cFileName, '\0', sizeof( FindFileData.cFileName ));
	memset( FindFileData.cAlternateFileName, '\0', sizeof( FindFileData.cAlternateFileName ));

	InstallProduct			= TRUE;
	UninstallAvailable		= FALSE;
	DisplayRegisterButton	= FALSE;
	IsWolapiAvailable		= FALSE;
	IsUserRegistered		= FALSE;

	//==========================================================================
	// Look for keys under the Game's Registry key.
	//==========================================================================
//	_tcscat( _tcscpy( key, SOFTWARE_EAGAMES_KEY ), szRegistryKey );
	_tcscat( _tcscpy( key, EAGAMES_GENERALS_KEY ), szRegistryKey );

	//--------------------------------------------------------------------------
	// Try to open the key.
	//--------------------------------------------------------------------------
	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, key, 0, KEY_QUERY_VALUE, &phKey ) == ERROR_SUCCESS ) {

		//-----------------------------------------------------------------------
		// Get Full path\filename of product to execute ("Play").
		//-----------------------------------------------------------------------
		Size = _MAX_PATH;
 		if ( RegQueryValueEx( phKey, INSTALL_PATH_KEY, NULL, &Type, (unsigned char *)szGamePath, &Size ) == ERROR_SUCCESS ) {
			_tcscpy(szWorldbuilderPath, szGamePath);
			_tcscpy(szPatchgetPath, szGamePath);
			_tcscat(szGamePath, LAUNCHER_FILENAME);
			_tcscat(szWorldbuilderPath, WORLDBUILDER_FILENAME);
			_tcscat(szPatchgetPath, PATCHGET_FILENAME);
			handle = FindFirstFile( szGamePath, &FindFileData );
			if ( handle != INVALID_HANDLE_VALUE ) {
				InstallProduct = FALSE;
				FindClose( handle );
			}
		}
		Msg( __LINE__, TEXT(__FILE__), TEXT("GamePath =  %s."), szGamePath );

		RegCloseKey( phKey );
	}

	//==========================================================================
	// Find Keys under "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
	//==========================================================================

	_tcscpy( key, SHELL_UNINSTALL_KEY );
	Path_Add_Back_Slash( key );
	_tcscat( key, szRegistryKey );

	//--------------------------------------------------------------------------
	// Query the Uninstall key "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
	//--------------------------------------------------------------------------
	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, key, 0, KEY_ALL_ACCESS, &phKey ) == ERROR_SUCCESS ) {

		Size = _MAX_PATH;
		if ( RegQueryValueEx( phKey, UNINSTALL_STRING_SUBKEY, NULL, &Type, (unsigned char *)aName, &Size ) == ERROR_SUCCESS )
		{
			//------------------------------------------------------------------------------------------------------
			// Look for the uninstall program.  If found, set flag.
			// JFS... need to extract path and command line...  8/26/03
			// JFS... further verify that we use a very limited uninstall based on the presence of "IDriver.exe"
			//------------------------------------------------------------------------------------------------------
			if(strstr(aName,UNINSTALL_EXECUTABLE) != NULL)
			{
				char	*sp;

				strcpy( szUninstallPath, aName );
				sp = strchr(szUninstallPath,'/');
				if(*sp != NULL)
				{
					strcpy( szUninstallCommandLine, sp );
					strcpy( szUninstallPath, aName );
					*sp = '\0';
				}
			}

			handle = FindFirstFile( szUninstallPath, &FindFileData );
			if ( handle != INVALID_HANDLE_VALUE ) {
				UninstallAvailable = TRUE;
				FindClose( handle );
			}
		}
		RegCloseKey( phKey );
	}

	//==========================================================================
	// Look for keys under Register.  If available, we should show the "Register"
	// button.	WESTWOOD_REGISTER_KEY "Software\\Westwood\\Register"
	//==========================================================================
//	_tcscpy( key, WESTWOOD_REGISTER_KEY );

	//-------------------------------------------------------------------------
	// Create IGR Options Object.
	//-------------------------------------------------------------------------
	IGROptionsClass *OnlineOptions = new IGROptionsClass();
	if ( OnlineOptions ) {
		OnlineOptions->Init();
	}

	//--------------------------------------------------------------------------
	// If "Options" is set under WOLAPI, then this is a Game Room Edition and
	// registration should NOT be allowed.
	//--------------------------------------------------------------------------
	if ( OnlineOptions ) {
		DisplayRegisterButton = OnlineOptions->Is_Running_Reg_App_Allowed();
	}

	//--------------------------------------------------------------------------
	// If Registration is allowed...
	//--------------------------------------------------------------------------
/*
	if ( DisplayRegisterButton ) {

		DisplayRegisterButton = false;

		if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, key, 0, KEY_QUERY_VALUE, &phKey ) == ERROR_SUCCESS ) {

			//-----------------------------------------------------------------------
			// Get Full path\filename of product to execute ("Register.exe").
			//-----------------------------------------------------------------------
			Size = _MAX_PATH;
			if ( RegQueryValueEx( phKey, INSTALLPATH_SUBKEY, NULL, &Type, (unsigned char *)szRegisterPath, &Size ) == ERROR_SUCCESS ) {

				//--------------------------------------------------------------------
				// Check if this executable exists.
				//--------------------------------------------------------------------
				handle = FindFirstFile( szRegisterPath, &FindFileData );
				if ( handle != INVALID_HANDLE_VALUE ) {
					DisplayRegisterButton = TRUE;
					FindClose( handle );
				}
			}
			RegCloseKey( phKey );
		}
	}
*/
	//==========================================================================
	// Is WOLAPI DLL installed?
	//==========================================================================
/*
	_tcscpy( key, WESTWOOD_WOLAPI_KEY );

	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, key, 0, KEY_QUERY_VALUE, &phKey ) == ERROR_SUCCESS ) {

		//-----------------------------------------------------------------------
		// Get Full path\filename of product to execute ("Register.exe").
		//-----------------------------------------------------------------------
		Size = _MAX_PATH;
		if ( RegQueryValueEx( phKey, INSTALLPATH_SUBKEY, NULL, &Type, (unsigned char *)szBuffer, &Size ) == ERROR_SUCCESS ) {

			//--------------------------------------------------------------------
			// Check if this executable exists.
			//--------------------------------------------------------------------
			handle = FindFirstFile( szBuffer, &FindFileData );
			if ( handle != INVALID_HANDLE_VALUE ) {
				IsWolapiAvailable = TRUE;
				FindClose( handle );
			}
		}
		RegCloseKey( phKey );
	}
*/

	Msg( __LINE__, TEXT(__FILE__), TEXT("---------------- Is_Product_Registered---------------" ));
	Msg( __LINE__, TEXT(__FILE__), TEXT(" InstallProduct		= %d."), InstallProduct	);
	Msg( __LINE__, TEXT(__FILE__), TEXT(" UninstallAvailable	= %d."), UninstallAvailable	);
	Msg( __LINE__, TEXT(__FILE__), TEXT(" IsUserRegistered		= %d."), IsUserRegistered );
	Msg( __LINE__, TEXT(__FILE__), TEXT(" DisplayRegisterButton	= %d."), DisplayRegisterButton );
	Msg( __LINE__, TEXT(__FILE__), TEXT(" szGamePath			= %s."), szGamePath	);
	Msg( __LINE__, TEXT(__FILE__), TEXT(" szSetupPath			= %s."), szSetupPath );
	Msg( __LINE__, TEXT(__FILE__), TEXT(" szRegisterPath		= %s."), szRegisterPath	);
	Msg( __LINE__, TEXT(__FILE__), TEXT(" szInternetPath		= %s."), szInternetPath	);
	Msg( __LINE__, TEXT(__FILE__), TEXT(" szUninstallPath		= %s."), szUninstallPath );

	return( result );
}


//*****************************************************************************
// MainWindow::Run_Explorer
//
//		Get the Current Dir from argv[0].  Open the Explorer Window with
//		this dir.  The Explorer will open and display everything in this dir.
//
//	INPUT:  	none.
//
//	OUTPUT: 	none.
//
//	WARNINGS:	none.
//
//	HISTORY:
//		06/04/1996  MML : Created.
//=============================================================================

BOOL MainWindow::Run_Explorer( const char *szString, HWND hWnd, RECT *rect )
{
	char 	szWindowsPath	[ _MAX_PATH ];
	char 	szPath			[ _MAX_PATH ];
	char 	szCurDir		[ _MAX_PATH ];
	char 	drive  			[ _MAX_DRIVE ];
	char 	dir				[ _MAX_DIR ];
	char 	lpszComLine		[ 127 ];

	BOOL result = FALSE;
	PROCESS_INFORMATION processinfo;
	STARTUPINFO startupinfo;

	//--------------------------------------------------------------------------
	// Get current drive/directory from _argv[0].
	//--------------------------------------------------------------------------
	_tcscpy( szPath, szArgvPath );
	_tsplitpath( szPath, drive, dir, NULL, NULL );
	_tmakepath ( szPath, drive, dir, NULL, NULL );

	//--------------------------------------------------------------------------
	// Get Windows directory and build path to Explorer.  Pas in szPath as
	// the directory for Explorer to open.
	//--------------------------------------------------------------------------
	GetWindowsDirectory( szWindowsPath, _MAX_PATH );
	Path_Add_Back_Slash( szWindowsPath );

	_tcscat( szWindowsPath, EXPLORER_NAME );
	_tcscat( _tcscat( _tcscpy( lpszComLine, szWindowsPath ), _TEXT( " " )), szPath );
	_tcscpy( szCurDir, szPath );

	//==========================================================================
	// Setup the call
	//==========================================================================
	memset( &startupinfo, 0, sizeof( STARTUPINFO ));
	startupinfo.cb = sizeof( STARTUPINFO );

	//--------------------------------------------------------------------------
	// Next, start the process
	//--------------------------------------------------------------------------
	result = CreateProcess(
				szWindowsPath, 				// address of module name
				lpszComLine,				// address of command line
				NULL,						// address of process security attributes
				NULL,						// address of thread security attributes
				FALSE,						// new process inherits handles
				0,							// creation flags
				NULL,						// address of new environment block
				szCurDir,					// address of current directory name
				&startupinfo,				// address of STARTUPINFO
				&processinfo );				// address of PROCESS_INFORMATION

	//--------------------------------------------------------------------------
	// If WinExec returned 0, error occurred.
	//--------------------------------------------------------------------------
	if ( !result ) {

		Cant_Find_MessageBox ( Main::hInstance, szPath );

	#if(BACKGROUND_BITMAP)
		//-----------------------------------------------------------------------
		// Recreate Buttons based on Registry, then repaint window.
		//-----------------------------------------------------------------------
		Create_Buttons( hWnd, rect );
		InvalidateRect( hWnd, rect, FALSE );
	#endif
	}
	return ( result );
}

//*****************************************************************************
// MainWindow::Run_Game -- Launch the game based on registry information.
//
// INPUT:  		none.
//
//	OUTPUT: 		none.
//
// WARNINGS:	none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

unsigned int MainWindow::Run_Game ( HWND hWnd, RECT *rect )
{
	char dir 	[_MAX_DIR];
	char ext  	[_MAX_EXT];
	char drive	[_MAX_DRIVE];
	char file 	[_MAX_FNAME];

//	unsigned abc  = 0;
	HANDLE  handle;
	WIN32_FIND_DATA FindFileData;

	//--------------------------------------------------------------------------
	// Check if C&C is already running, and is looking for the CDRom.
	// The Autorun keeps asking to "Play" when this happens.
	//--------------------------------------------------------------------------
	HWND game_window = FindWindow ( szGameWindow, NULL );
	if ( game_window ){
		ShowWindow( game_window, SW_RESTORE );
		SetForegroundWindow ( game_window );
		return FALSE;
	}

	//--------------------------------------------------------------------------
	// Split into parts.
	//--------------------------------------------------------------------------
	_tsplitpath( szGamePath, drive, dir, file, ext );

	//--------------------------------------------------------------------------
	// Launch the game.
	//--------------------------------------------------------------------------
	handle = FindFirstFile( szGamePath, &FindFileData );
	if ( handle == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}

	FindClose( handle );

	//-----------------------------------------------------------------------
	// Stop Sound if sound was playing.
	//-----------------------------------------------------------------------
	Stop_Sound_Playing();

	LaunchObject.SetPath( szGamePath );
	LaunchObject.Set_Launch( true );
	return( true );
}

//*****************************************************************************
// MainWindow::Run_WorldBuilder -- Launch the map editor based on registry information.
//
// INPUT:  		none.
//
//	OUTPUT: 		none.
//
// WARNINGS:	none.
//
// HISTORY:
//   12/02/2002  BGC : Created.
//=============================================================================

unsigned int MainWindow::Run_WorldBuilder( HWND hWnd, RECT *rect)
{
	HANDLE  handle;
	WIN32_FIND_DATA FindFileData;

	//--------------------------------------------------------------------------
	// Launch the game.
	//--------------------------------------------------------------------------
	handle = FindFirstFile( szWorldbuilderPath, &FindFileData );
	if ( handle == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}

	FindClose( handle );

	//-----------------------------------------------------------------------
	// Stop Sound if sound was playing.
	//-----------------------------------------------------------------------
	Stop_Sound_Playing();

	LaunchObject.SetPath( szWorldbuilderPath );
	LaunchObject.Set_Launch( true );
	return( true );
}

//*****************************************************************************
// MainWindow::Run_PatchGet -- Launch the patch checker based on registry information.
//
// INPUT:  		none.
//
//	OUTPUT: 		none.
//
// WARNINGS:	none.
//
// HISTORY:
//   12/02/2002  BGC : Created.
//=============================================================================

unsigned int MainWindow::Run_PatchGet( HWND hWnd, RECT *rect)
{
	HANDLE  handle;
	WIN32_FIND_DATA FindFileData;

	//--------------------------------------------------------------------------
	// Launch the game.
	//--------------------------------------------------------------------------
	handle = FindFirstFile( szPatchgetPath, &FindFileData );
	if ( handle == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}

	FindClose( handle );

	//-----------------------------------------------------------------------
	// Stop Sound if sound was playing.
	//-----------------------------------------------------------------------
	Stop_Sound_Playing();

	LaunchObject.SetPath( szPatchgetPath );
	LaunchObject.Set_Launch( true );
	return( true );
}

//*****************************************************************************
//	MainWindow::Run_Demo -- Launch a demo program if desired.
//
//	INPUT:  	none.
//
//	OUTPUT: 	none.
//
//	WARNINGS:	none.
//
//	HISTORY:
//		01/08/2002  MML : Created.
//=============================================================================

unsigned int MainWindow::Run_Demo ( HWND hWnd, RECT *rect, int cd_drive )
{
//	unsigned	abc  = 0;
	HANDLE		handle;
	WIN32_FIND_DATA FindFileData;

	//--------------------------------------------------------------------------
	// Make path to demo program.
	//--------------------------------------------------------------------------
//	Make_Current_Path_To( MOH_DEMO_PROGRAM, szBuffer );
	wsprintf( szBuffer, "%c:\\", 'A' + cd_drive );
	Path_Add_Back_Slash( szBuffer );
	strcat( szBuffer, MOH_DEMO_PROGRAM );

	//--------------------------------------------------------------------------
	// Launch the game.
	//--------------------------------------------------------------------------
	handle = FindFirstFile( szBuffer, &FindFileData );
	if ( handle == INVALID_HANDLE_VALUE ) {
		Cant_Find_MessageBox( Main::hInstance, szBuffer );
		return FALSE;
	}

	FindClose( handle );

	//-----------------------------------------------------------------------
	// Stop Sound if sound was playing.
	//-----------------------------------------------------------------------
	Stop_Sound_Playing();

	LaunchObject.SetPath( szBuffer );
	LaunchObject.Set_Launch( true );
	return( true );
}

//*****************************************************************************
// MainWindow::Run_OpenFile -- Main Window function to perform Setup tasks.
//
// INPUT:         none.
//
// OUTPUT:        none.
//
// WARNINGS:      none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

unsigned int MainWindow::Run_OpenFile(int cd_drive, const char *filename, bool wait /* = false */)
{
	char filepath[MAX_PATH];
	MSG msg;
	BOOL returnValue;

	SHELLEXECUTEINFO executeInfo;
	memset(&executeInfo, 0, sizeof(executeInfo));

	executeInfo.cbSize = sizeof(executeInfo);
	executeInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	executeInfo.hwnd = ApplicationHWnd;
	executeInfo.lpVerb = "open";
	executeInfo.lpFile = filename;
	executeInfo.nShow = SW_SHOWNORMAL;

	HANDLE hProcess;


	BOOL ret = ShellExecuteEx(&executeInfo);

	if ((ret == 0) || ((int)(executeInfo.hInstApp) <= 32)) {
		Msg(__LINE__, TEXT(__FILE__), TEXT("Couldn't find executable for %s\n"), filepath);
		return 0;
	}

	hProcess = executeInfo.hProcess;

	if (wait == true) {
		WaitForInputIdle(hProcess, 5000);

		bool waiting = true;
		bool quit = false;

		while ((waiting == true) && (quit != true)) {
			Sleep(0);

			while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
			{

				// get the message
				returnValue = GetMessage( &msg, NULL, 0, 0 );

				// check for quitting
				if( returnValue == 0 )
					quit = TRUE;

				// translate and dispatch the message
				TranslateMessage( &msg );
				DispatchMessage( &msg );

			}

			DWORD exitCode;
			GetExitCodeProcess(hProcess, &exitCode);

			if (exitCode != STILL_ACTIVE) {
				waiting = false;
			}
		}
	}

	return 1;
}

//*****************************************************************************
// MainWindow::Run_Setup -- Main Window function to perform Setup tasks.
//
// INPUT:         none.
//
// OUTPUT:        none.
//
// WARNINGS:      none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

unsigned int MainWindow::Run_Setup( HWND hWnd, RECT *rect, int cd_drive )
{
//	UINT				result = 0;
	int					i = 0;
	char	   			params		[ 127 ];
	char   				filepath[ _MAX_PATH ];
	HANDLE				handle;
	WIN32_FIND_DATA		FindFileData;

	Msg( __LINE__, TEXT(__FILE__), TEXT("---------------------- Run_Setup --------------------." ));

	//--------------------------------------------------------------------------
	// Clear these buffers for later use.
	//--------------------------------------------------------------------------
	memset( params, '\0', 127 );

	//--------------------------------------------------------------------------
	// Get Drive & Dir from ARGV[0] and create path to SETUP.EXE.
	//--------------------------------------------------------------------------
//	strcpy( filepath, szArgvPath );
//	Path_Add_Back_Slash( filepath );

	wsprintf( filepath, "%c:\\", 'A' + cd_drive );
	Path_Add_Back_Slash( filepath );
	strcat( filepath, SETUP_NAME );

	//--------------------------------------------------------------------------
	// If we could not find SETUP.EXE, then display error msg and exit.
	//--------------------------------------------------------------------------
	handle = FindFirstFile( filepath, &FindFileData );
	if ( handle == INVALID_HANDLE_VALUE ) {
		Cant_Find_MessageBox( Main::hInstance, filepath );
		return FALSE;
	}
	FindClose( handle );

	//--------------------------------------------------------------------------
	//	Create parameters to pass in with the program we are calling.
	//--------------------------------------------------------------------------
	memset( params, '\0', sizeof( params ));
	if ( NumberArguments ) {

		_tcscpy( params, Arguments[0] );
		i = 1;
		while (( i < NUM_ARGUMENTS ) && ( i < NumberArguments )) {
			_tcscat( _tcscat( params, _TEXT( " " )), Arguments[i] );
			i++;
		}
	}

	//--------------------------------------------------------------------------
	// Stop Sound if sound was playing.
	//--------------------------------------------------------------------------
	Stop_Sound_Playing();

	LaunchObject.SetPath( filepath );
	LaunchObject.SetArgs( params );
	LaunchObject.Set_Launch( true );
	return( true );
}


//*****************************************************************************
// MainWindow::Run_New_Account -- Create a new online account.
//
// INPUT:         none.
//
// OUTPUT:        none.
//
// WARNINGS:      none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

unsigned int MainWindow::Run_New_Account ( HWND hWnd, RECT *rect )
{
//	UINT	result = 0;
	int		i = 0;
	char 	params	[ 127 ];
	char 	filepath[ _MAX_PATH ];
	HANDLE  handle;
	WIN32_FIND_DATA FindFileData;

	//--------------------------------------------------------------------------
	// Clear these buffers for later use.
	//--------------------------------------------------------------------------
	memset( params, '\0', 127 );

	//--------------------------------------------------------------------------
	// Get Drive & Dir from ARGV[0] and create path to SETUP.EXE.
	//--------------------------------------------------------------------------
	_tcscpy( filepath, szArgvPath );
	Path_Add_Back_Slash( filepath );
	_tcscat( filepath, SETUP_NAME );

	//--------------------------------------------------------------------------
	// If we could not find SETUP.EXE, then display error msg and exit.
	//--------------------------------------------------------------------------
	handle = FindFirstFile( filepath, &FindFileData );
	if ( handle == INVALID_HANDLE_VALUE ) {
		Cant_Find_MessageBox( Main::hInstance, SETUP_NAME );
		return FALSE;
	}
	FindClose( handle );

	//--------------------------------------------------------------------------
	//	Create parameters to pass in with the program we are calling.
	//--------------------------------------------------------------------------
	memset( params, '\0', sizeof( params ));
	if ( NumberArguments ) {

		_tcscpy( params, Arguments[0] );
		i = 1;
		while (( i < NUM_ARGUMENTS ) && ( i < NumberArguments )) {
			_tcscat( _tcscat( params, _TEXT( " " )), Arguments[i] );
			i++;
		}
		_tcscat( params, _TEXT( " " ));
	}
	_tcscat( params, "-o" );

	//--------------------------------------------------------------------------
	// Stop Sound if sound was playing.
	//--------------------------------------------------------------------------
	Stop_Sound_Playing();

	LaunchObject.SetPath( filepath );
	LaunchObject.SetArgs( params );
	LaunchObject.Set_Launch( true );
	return( true );
}


//*****************************************************************************
// MainWindow::Run_Register_Or_Auto_Update
//
//		Either run Register.exe or Game Update Program from the user's harddrive.
//
// INPUT:         none.
//
// OUTPUT:        none.
//
// WARNINGS:      none.
//
// HISTORY:
//   02/24/1999  MML : Created.
//=============================================================================

unsigned int MainWindow::Run_Register ( HWND hWnd, RECT *rect )
{
	char 				szArgs[ MAX_PATH ];
	HANDLE				handle;
	WIN32_FIND_DATA		FindFileData;
	BOOL				result = FALSE;

	//--------------------------------------------------------------------------
	// Register Program is available.  Continue...
	//--------------------------------------------------------------------------
	if ( DisplayRegisterButton ) {

		//-----------------------------------------------------------------------
		// Check again.  May have been changed ...
		//-----------------------------------------------------------------------
		handle = FindFirstFile( szRegisterPath, &FindFileData );
		if ( handle == INVALID_HANDLE_VALUE ) {
			return( 0 );
		}
		FindClose( handle );

		//-----------------------------------------------------------------------
		// User is already registered, so maybe we should Auto Update!
		//-----------------------------------------------------------------------
		strcpy( szArgs, _TEXT( " " ));

		LaunchObject.SetPath( szRegisterPath );
		LaunchObject.SetArgs( szArgs );
		LaunchObject.Set_Launch( true );
		result = true;
	}

	return( result );
}

//*****************************************************************************
// MainWindow::Run_Auto_Update
//
//		Either run Register.exe or Game Update Program from the user's harddrive.
//
// INPUT:         none.
//
// OUTPUT:        none.
//
// WARNINGS:      none.
//
// HISTORY:
//   02/24/1999  MML : Created.
//=============================================================================

unsigned int MainWindow::Run_Auto_Update ( HWND hWnd, RECT *rect )
{
	char 				szArgs		[ MAX_PATH ];
	HANDLE				handle;
	WIN32_FIND_DATA		FindFileData;
	BOOL				result = FALSE;

	//--------------------------------------------------------------------------
	// Register Program is available.  Continue...
	//--------------------------------------------------------------------------
	if ( !InstallProduct ) {

		//-----------------------------------------------------------------------
		// Check again.  May have been changed ...
		//-----------------------------------------------------------------------
		handle = FindFirstFile( szGamePath, &FindFileData );
		if ( handle == INVALID_HANDLE_VALUE ) {
			return( 0 );
		}
		FindClose( handle );

		_tcscpy( szArgs, _TEXT( " GrabPatches" ));

		LaunchObject.SetPath( szGamePath );
		LaunchObject.SetArgs( szArgs );
		LaunchObject.Set_Launch( true );
		result = true;
	}
	return( result );
}


//*****************************************************************************
// MainWindow::Run_Uninstall -- Main Window function to perform Setup tasks.
//
// INPUT:         none.
//
// OUTPUT:        none.
//
// WARNINGS:      none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

unsigned int MainWindow::Run_Uninstall( HWND hWnd, RECT *rect )
{
//	MSG		msg;
	UINT	result			= 0;
//	int		done 			= 0;
//	DWORD	dwTimeout		= 1500;
//	DWORD	dwRC			= WAIT_TIMEOUT;
	DWORD	lpExitCode;

	STARTUPINFO 			startupinfo;
	PROCESS_INFORMATION 	processinfo;
	HANDLE					handle;
	WIN32_FIND_DATA			FindFileData;

	char 	szCurDir	[_MAX_PATH];
	char 	file		[_MAX_FNAME];
	char 	ext			[_MAX_EXT];
	char 	szPath		[_MAX_PATH];

	//--------------------------------------------------------------------------
	// Launch the UNINSTALL.
	//--------------------------------------------------------------------------
	handle = FindFirstFile( szUninstallPath, &FindFileData );
	if ( handle == INVALID_HANDLE_VALUE ) {
		return( 0 );
	}

	FindClose( handle );

	_splitpath( szUninstallPath, drive, dir, NULL, NULL );
	_makepath ( szCurDir, drive, dir, NULL, NULL );

	//=======================================================================
	// Setup the call
	//=======================================================================
	memset( &startupinfo, 0, sizeof( STARTUPINFO ));
	startupinfo.cb = sizeof( STARTUPINFO );

	result = CreateProcess(
					szUninstallPath,			// address of module name
					szUninstallCommandLine,		// address of command line
					NULL,						// address of process security attributes
					NULL,						// address of thread security attributes
					0,							// new process inherits handles
					0,
					NULL,						// address of new environment block
					szCurDir,
					&startupinfo,				// address of STARTUPINFO
					&processinfo );				// address of PROCESS_INFORMATION

	//--------------------------------------------------------------------------
	// If WinExec returned 0, error occurred.
	//--------------------------------------------------------------------------
	if ( !result ) {

		_tsplitpath( szUninstallPath, NULL, NULL, file, ext );
		_tmakepath ( szPath, NULL, NULL, file, ext );
		Cant_Find_MessageBox ( Main::hInstance, szPath );

//	#if(BACKGROUND_BITMAP)
		//-----------------------------------------------------------------------
		// Recreate Buttons based on Registry, then repaint window.
		//-----------------------------------------------------------------------
//		Create_Buttons( hWnd, rect );
//		InvalidateRect( hWnd, rect, FALSE );
//	#endif

		return( result );
	}

	// JFS: 8-26-03... Can't have auto run going during an uninstall!
#if 0
	//--------------------------------------------------------------------------
	// Wait for App to shutdown
	//--------------------------------------------------------------------------
	while (dwRC == WAIT_TIMEOUT)
	{

		//-----------------------------------------------------------------------
		// Wait for object
		//-----------------------------------------------------------------------
		dwRC = WaitForSingleObject( processinfo.hProcess, dwTimeout );


		//-----------------------------------------------------------------------
		// Flush the Queue
		//-----------------------------------------------------------------------
		while (PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))  {
			TranslateMessage( &msg );
//			DispatchMessage( &msg );
		}
	}
#endif
	//--------------------------------------------------------------------------
	// If the specified process has not terminated, the termination status
	// returned is STILL_ACTIVE.
	//--------------------------------------------------------------------------
	GetExitCodeProcess( processinfo.hProcess, &lpExitCode );
	CloseHandle( processinfo.hProcess );
	CloseHandle( processinfo.hThread  );

#if(BACKGROUND_BITMAP)

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// MML 5/27/99:  This function is dropping through because we launch Uninstll.exe
	// which in turn launches Uninst.exe thus ::Run_Install ends before Uninst.exe is done.
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//--------------------------------------------------------------------------
	// Recreate Buttons based on Registry.
	// Repaint calling Window.
	//--------------------------------------------------------------------------
	Create_Buttons( hWnd, rect );
	InvalidateRect( hWnd, rect, FALSE );
#endif

	return( result );
}

//*****************************************************************************
// MainWindow::Create_Buttons -- Reset the Buttons.
//
// INPUT:         none.
//
// OUTPUT:        none.
//
// WARNINGS:      none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

void MainWindow::Create_Buttons( HWND hWnd, RECT *dlg_rect )
{
	int	j = 0;
	int	i = 0;
	int	x_pos = 0;
	int	y_pos = 0;
	int	width = 0;
	int	height = 0;
	int	button_index = 0;

	HBITMAP		hButtonBitmap  = 0;
	HPALETTE	hpal = 0;
	BITMAP		button_bm;
	char 		next_button_name[_MAX_PATH];
	char 		focused_button_name[_MAX_PATH];
	RECT *		button_size = ButtonSizes;

	//--------------------------------------------------------------------------
	// Reset all the flags.
	//--------------------------------------------------------------------------
	Is_Product_Registered( );

	Msg( __LINE__, TEXT(__FILE__), TEXT("------------------------ Create_Buttons ------------------------" ));

	//--------------------------------------------------------------------------
	// Get width and height of the button.
	//--------------------------------------------------------------------------
	hButtonBitmap = LoadResourceBitmap( Main::hModule, BUTTON_REG, &hpal, TRUE );
	if ( hButtonBitmap ) {
		GetObject( hButtonBitmap, sizeof( BITMAP ), (LPTSTR)&button_bm );
		width	= button_bm.bmWidth;
		height	= button_bm.bmHeight;
		DeleteObject( hButtonBitmap );
		hButtonBitmap = 0;
	} else {
		button_bm.bmWidth	= width		= BUTTON_WIDTH;
		button_bm.bmHeight	= height	= BUTTON_HEIGHT;
	}

	//--------------------------------------------------------------------------
	// Initialize the ButtonList.
	//--------------------------------------------------------------------------
	if( b640X480 || b800X600 ) {

		x_pos	= 410;
		y_pos	= 90;
	} else {
		x_pos	= 540;
		y_pos	= 117;
	}

	for ( i = 0; i < NUM_BUTTONS; i++ ) {

		delete( ButtonList[i] );
		ButtonList[i]			= NULL;
		ButtonSizes[i].left		= x_pos; 				// X position.
		ButtonSizes[i].top 		= y_pos;				// Y position.
		ButtonSizes[i].right	= width;				// Button's width.
		ButtonSizes[i].bottom	= height;				// Button's height.
		y_pos += height + 4;

		Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonSizes[%d] = ( %d, %d, %d, %d )"),
			i, ButtonSizes[i].left, ButtonSizes[i].top, ButtonSizes[i].right, ButtonSizes[i].bottom );
	}

	//==========================================================================
	// Create the "Buttons"
	//==========================================================================
	i = 0;
	j = 0;

	//--------------------------------------------------------------------------
	// Make any other necessary adjustments here.
	//--------------------------------------------------------------------------
	int count = 7;

	if ( !UninstallAvailable ) {
		count--;					// No uninstall button.
	}
	if ( InstallProduct ) {
		count--;					// No Website button.
	}
	button_index = 0;

	strcpy( next_button_name, ButtonImages[button_index] );
	strcpy( focused_button_name, FocusedButtonImages[button_index] );

	button_size = ButtonSizes;
	i = j = button_index;

	Msg( __LINE__, TEXT(__FILE__), TEXT("count = %d."), count );
	Msg( __LINE__, TEXT(__FILE__), TEXT("button_index = %d."), button_index );
	Msg( __LINE__, TEXT(__FILE__), TEXT("next_button_name = %s."), next_button_name );
	Msg( __LINE__, TEXT(__FILE__), TEXT("focused_button_name = %s."), focused_button_name );


	//-------------------------------------------------------------------------
	// INSTALL or PLAY?
	//-------------------------------------------------------------------------
	if ( InstallProduct ) {

		//---------------------------------------------------------------------
		// (8) INSTALL button.
		//---------------------------------------------------------------------
		Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=%d, String=%s."), i, IDD_OK, j, "Install" );
		ButtonList[i++] = new DrawButton(
			IDD_OK,
			button_size[j++],
			BUTTON_REG,
			BUTTON_SEL,
			BUTTON_SEL,
#ifdef LEAN_AND_MEAN
			Locale_GetString( "Autorun:Install" ),
#else
			AsciiString("Autorun:Install"),
#endif
			TTButtonFontPtr );

	} else {

		//---------------------------------------------------------------------
		// (8) PLAY button.
		//---------------------------------------------------------------------
		Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=%d, String=%s."), i, IDD_OK2, j, "Play" );
		ButtonList[i++] = new DrawButton(
			IDD_OK2,
			button_size[j++],
			BUTTON_REG,
			BUTTON_SEL,
			BUTTON_SEL,
#ifdef LEAN_AND_MEAN
			Locale_GetString( "Autorun:Play" ),
#else
			AsciiString("Autorun:Play"),
#endif
			TTButtonFontPtr );
/*
		//---------------------------------------------------------------------
		// (8) WorldBuilder button.
		//---------------------------------------------------------------------
		Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=%d, String=%s."), i, IDD_OK2, j, "WorldBuilder" );
//		ButtonList[i--] = new DrawButton(
		ButtonList[i++] = new DrawButton(
			IDD_OK3,
//			button_size[j--],
			button_size[j++],
			BUTTON_REG,
			BUTTON_SEL,
			BUTTON_SEL,
#ifdef LEAN_AND_MEAN
			Locale_GetString( "Autorun:Worldbuilder" ),
#else
			AsciiString("Autorun:Worldbuilder"),
#endif
			TTButtonFontPtr );
*/
		//---------------------------------------------------------------------
		// (8) Check for updates button.
		//---------------------------------------------------------------------
		Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=%d, String=%s."), i, IDD_OK2, j, "Check For Updates" );
//		ButtonList[i--] = new DrawButton(
		ButtonList[i++] = new DrawButton(
			IDD_OK4,
//			button_size[j--],
			button_size[j++],
			BUTTON_REG,
			BUTTON_SEL,
			BUTTON_SEL,
#ifdef LEAN_AND_MEAN
			Locale_GetString( "Autorun:CheckForUpdates" ),
#else
			AsciiString("Autorun:CheckForUpdates"),
#endif
			TTButtonFontPtr );
	}

	//-------------------------------------------------------------------------
	// (7) EXPLORE CD
	//-------------------------------------------------------------------------
	Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=%d, String=%s."), i, IDD_EXPLORE, j, "Explore" );
	ButtonList[i++] = new DrawButton (
		IDD_EXPLORE,
		button_size[j++],
		BUTTON_REG,
		BUTTON_SEL,
		BUTTON_SEL,
#ifdef LEAN_AND_MEAN
		Locale_GetString( "Autorun:ExploreCD" ),
#else
		AsciiString("Autorun:ExploreCD"),
#endif
		TTButtonFontPtr );
//	strcpy( next_button_name, ButtonImages[button_index] );
//	strcpy( focused_button_name, FocusedButtonImages[button_index--] );
//	strcpy( focused_button_name, FocusedButtonImages[button_index++] );

	if ( !InstallProduct ) {
		//-----------------------------------------------------------------------
		// (3) WebSite button.
		//-----------------------------------------------------------------------
		Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=%d, String=%s."), i, IDD_OK, j, "Install" );
		ButtonList[i++] = new DrawButton(
			IDD_INTERNET,
			button_size[j++],
			BUTTON_REG,
			BUTTON_SEL,
			BUTTON_SEL,
#ifdef LEAN_AND_MEAN
			Locale_GetString( "Autorun:Website" ),
#else
			AsciiString("Autorun:Website"),
#endif
			TTButtonFontPtr );

//		strcpy( next_button_name, ButtonImages[button_index] );
//		strcpy( focused_button_name, FocusedButtonImages[button_index--] );
//		strcpy( focused_button_name, FocusedButtonImages[button_index++] );
	}

	//--------------------------------------------------------------------------
	// (2) UNINSTALL?
	//--------------------------------------------------------------------------
	if ( UninstallAvailable && !InstallProduct ) {
		Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=%d, String=%s."), i, IDD_UNINSTALL, j, "Uninstall" );
		ButtonList[i++] = new DrawButton(
			IDD_UNINSTALL,
			button_size[j++],
			BUTTON_REG,
			BUTTON_SEL,
			BUTTON_SEL,
#ifdef LEAN_AND_MEAN
			Locale_GetString( "Autorun:Uninstall" ),
#else
			AsciiString("Autorun:Uninstall"),
#endif
			TTButtonFontPtr );

//		strcpy( next_button_name, ButtonImages[button_index] );
//		strcpy( focused_button_name, FocusedButtonImages[button_index--] );
//		strcpy( focused_button_name, FocusedButtonImages[button_index++] );
	}

	//--------------------------------------------------------------------------
	// (1) MOH movie
	//--------------------------------------------------------------------------
	Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=%d, String=%s."), i, IDD_PREVIEWS, j, "Preview Movies");
	ButtonList[i++] = new DrawButton(
		IDD_PREVIEWS,
		button_size[j++],
		BUTTON_REG,
		BUTTON_SEL,
		BUTTON_SEL,
#ifdef LEAN_AND_MEAN
    Locale_GetString( "Autorun:Previews" ),
#else
		AsciiString("Autorun:Previews"),
#endif
		TTButtonFontPtr );

	//--------------------------------------------------------------------------
	// (1) Help file
	//--------------------------------------------------------------------------
	Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=%d, String=%s."), i, IDD_HELP, j, "Help file");
	ButtonList[i++] = new DrawButton(
		IDD_HELP,
		button_size[j++],
		BUTTON_REG,
		BUTTON_SEL,
		BUTTON_SEL,
#ifdef LEAN_AND_MEAN
    Locale_GetString( "Autorun:Help" ),
#else
		AsciiString("Autorun:Help"),
#endif
		TTButtonFontPtr );

	//--------------------------------------------------------------------------
	// (1) CANCEL?
	//--------------------------------------------------------------------------
	Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=%d, String=%s."), i, IDD_CANCEL, j, "Cancel" );
	ButtonList[i++] = new DrawButton(
		IDD_CANCEL,
		button_size[j++],
		BUTTON_REG,
		BUTTON_SEL,
		BUTTON_SEL,
#ifdef LEAN_AND_MEAN
		Locale_GetString( "Autorun:Cancel" ),
#else
		AsciiString("Autorun:Cancel"),
#endif
		TTButtonFontPtr );

//	strcpy( next_button_name, ButtonImages[button_index] );
//	strcpy( focused_button_name, FocusedButtonImages[button_index--] );
//	strcpy( focused_button_name, FocusedButtonImages[button_index++] );

	//-------------------------------------------------------------------------
	// Set the top button to have the focus.
	//-------------------------------------------------------------------------
	if ( ButtonList[0]) {
		Msg( __LINE__, TEXT(__FILE__), TEXT("Button with starting Focus = %d"), i );
		ButtonList[0]->Set_State( DrawButton::FOCUS_STATE );
	}

#if( RTS_DEBUG )
	Msg( __LINE__, TEXT(__FILE__), TEXT("----------------------------------------------------------------------------------"));
	for( i=0; i<NUM_BUTTONS; i++ ) {
		if ( ButtonList[i]) {
			Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonList[%d]: Id=%d, ButtonSizes=[%d,%d,%d,%d]."),
				i,
				ButtonList[i]->Return_Id(),
				ButtonList[i]->Return_X_Pos(),
				ButtonList[i]->Return_Y_Pos(),
				ButtonList[i]->Return_Width(),
				ButtonList[i]->Return_Height());
		}
	}
#endif
}


//*****************************************************************************
// WndProc -- Get Main Window's Stored Word.
//
// INPUT:         HWND hWnd.
//						Window *pWindow
//
// OUTPUT:        none.
//
// WARNINGS:      none.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

LRESULT CALLBACK  Wnd_Proc ( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam )
{
	//--------------------------------------------------------------------------
	// If the GlobalMainWindow Ptr is not initialized, do it when the MW-CREATE
	// msg is called.  Then we use the GlobalMainWindow's WindowProc to
	// process all the individual msgs sent.
	//--------------------------------------------------------------------------
	if ( GlobalMainWindow == NULL ) {
		if ( iMessage == WM_CREATE ) {

				LPCREATESTRUCT lpcs;

				lpcs = (LPCREATESTRUCT) lParam;
				GlobalMainWindow = (MainWindow *) lpcs->lpCreateParams;

				//-----------------------------------------------------------------
				// Now let the object perform whatever initialization it needs
				// for WM_CREATE in its own WndProc.
				//-----------------------------------------------------------------
				return GlobalMainWindow->Window_Proc( hWnd, iMessage, wParam, lParam );

		} else {
			return DefWindowProc( hWnd, iMessage, wParam, lParam );
		}

	} else {
		return GlobalMainWindow->Window_Proc( hWnd, iMessage, wParam, lParam );
	}
}

//*****************************************************************************
// DIALOG_BOX_PROC -- Handles dlg messages
//
// INPUT: standard windows dialog command parameters
//
// OUTPUT: unused
//
// WARNINGS: none
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

BOOL CALLBACK  Dialog_Box_Proc( HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param )
{
	int		i = 0, j = 0;
	int		nResult = 0;
//	int		space_between = 5;
	int		prevDCStretchMode;
	int		prevMemDCStretchMode;
	int		prevButtonDCStretchMode;
	int		prevLicenseDCStretchMode;
	int	 	result 	= 0;
	char	buffer1[ 50 ];
//	char	buffer2[ 50 ];

	HDC 	   		hDC, memDC, buttonDC, licenseDC;
	BITMAP     		bm, fm, lm;
//	LOGPALETTE 	  *	plgpl = NULL;
	PAINTSTRUCT		ps;
	static int 		bits_pixel = 0;
	static int 		idCtl = 0;
	static int 		mouse_x, mouse_y;

	static char 	szBitmap[_MAX_PATH];
	static char 	szLicense[ _MAX_PATH ];
	static char 	szButtonBitmap[_MAX_PATH];

	static wchar_t	szString1[ 500 ];
	static wchar_t	szString2[ 500 ];
	static wchar_t	szWholeString[ 1000 ];
//	static wchar_t	szWSMsg1[ _MAX_PATH ];

#ifdef LEAN_AND_MEAN

#else
	static UnicodeString	wsMsg1;
#endif

	static wchar_t	szWSMsg2[ _MAX_PATH ];
	static wchar_t	szWholeWSMsg[ 1000 ];
//	static wchar_t	szInstallWarningMsg[ _MAX_PATH ];

	static HBITMAP		hBitmap   			= 0;
	static HBITMAP		oldBitmap 			= 0;
//	static HBITMAP		oldButtonBitmap		= 0;
	static HBITMAP		oldLicenseBitmap	= 0;
	static HBITMAP		hButtonBitmap		= 0;
	static HBITMAP		hFlicker[NUM_FLICKER_FRAMES];
	static HBITMAP		hLicenseBitmap;

	static POINT		point;
	static HBRUSH		hStaticBrush		= 0;
	static HPALETTE		hpal				= 0;
	static HPALETTE		hpalold				= 0;
	static BOOL			FirstTime			= TRUE;
//	static BOOL			Flicker  			= TRUE;
	static BOOL			Flicker  			= FALSE;
	static int			FlickerIndex 		= 0;
	static BOOL			PaintOnlyFlicker	= TRUE;
	static BOOL			PaintBackground		= TRUE;
	static UINT			timer_id 			= 0;
	static RECT			rect;										// Desktop Window ( used once ).
	static RECT			tray_rect;									// Desktop Window w/o Tray size.
	static RECT			paint_rect;									// Size that needs to be repainted.
	static RECT			dlg_rect;									// Dialog client window size.
	static RECT			bitmap_rect;								// Background bitmap size.
	static RECT			flicker_rect;								// Flicker bitmap size.
	static RECT			buttons_rect;								// Area where buttons are.
	static RECT			license_rect;								// Area where buttons are.
	static RECT			BackgroundRect[ ( NUM_BUTTONS * 3 ) + 3 ];	// Background areas outside button area.

#if(USE_MOUSE_MOVES)
	static int CurrentButton = 0;
	static int LastButton = 0;
	static int PrevButton = 0;
#endif

	#if(0)
	{
		//-------------------------------------------------------------------------------
		// Used for debugging -- lining up objects using the mouse coordinates.
		//-------------------------------------------------------------------------------
		int   i = 0;
		HDC   hdc = GetDC( window_handle );
		char  string[10];
		POINT pPoint;

		GetCursorPos( &pPoint );
		ScreenToClient( window_handle, &pPoint );
		sprintf( string, "%d, %d", pPoint.x, pPoint.y );
		TextOut( hdc, 10, 50, string, 8 );
		ReleaseDC( window_handle, hdc );
	}
	#endif


	//-----------------------------------------------------------------------------------
	// Process Dialogs messages.
	//-----------------------------------------------------------------------------------
	switch( message ) {

		case WM_INITDIALOG:
			{
				//-----------------------------------------------------------------------
				// Set dialog's caption.
				//-----------------------------------------------------------------------

#ifdef LEAN_AND_MEAN

				Locale_GetString( "Autorun:Title", szWideBuffer );
				memset( szWideBuffer2, '\0', _MAX_PATH );
				swprintf( szWideBuffer2, szWideBuffer, szProductName );
				swprintf( szWideBuffer2, szWideBuffer, szFullProductName );

#else

				wideBuffer = TheGameText->fetch("Autorun:Title");
				wideBuffer2.format(wideBuffer, fullProductName.str());
				WideCharToMultiByte( CodePage, 0, wideBuffer2.str(), wideBuffer2.getLength()+1, szBuffer, _MAX_PATH, NULL, NULL );

#endif


				SetWindowText( window_handle, szBuffer );

				//-----------------------------------------------------------------------
				// Set Icon.
				//-----------------------------------------------------------------------
				SendMessage( window_handle, WM_SETICON, ICON_SMALL, (long)LoadIcon( Main::hInstance, MAKEINTRESOURCE(1)));

			#if(BACKGROUND_BITMAP)

				//-----------------------------------------------------------------------
				// Get the DeskTop's size and this dialogs size (in pixels).
				//-----------------------------------------------------------------------
				GetClientRect( GetDesktopWindow(), &rect );
				SystemParametersInfo( SPI_GETWORKAREA, 0, &tray_rect, 0 );

///				if( rect.right <= 640 ) {
//					b640X480 = TRUE;
//				} else if( rect.right <= 800 ) {
					b800X600 = TRUE;
//				}

				//=======================================================================
				// Create Fonts.
				//=======================================================================
				HDC hdc = GetDC( window_handle );

				FontManager = new FontManagerClass( hdc );
				assert( FontManager != NULL );
				ReleaseDC( window_handle, hdc );

				//=======================================================================
				// Load messages for bottom of screen.
				//=======================================================================
//				memset( szString1,		'\0', MAX_PATH );
//				memset( szString2,		'\0', MAX_PATH );
//				memset( szWholeString,	'\0', 1000 );

//				Locale_GetString( EA_LICENSE_MSG1, szString1 );
//				Locale_GetString( EA_LICENSE_MSG2, szString2 );
//				swprintf( szWholeString, L"%s %s", szString1, szString2 );

//				memset( szWSMsg1,		'\0', _MAX_PATH );
				memset( szWSMsg2,		'\0', _MAX_PATH );
				memset( szWholeWSMsg,	'\0', 1000 );

//				Locale_GetString( WESTWOOD_COM_MSG, szWSMsg1 );
//				wsMsg1 = TheGameText->fetch("Autorun:WestwoodComMsg");

//				PlugInProductName( szWSMsg1, (wchar_t *)(fullProductName.str()) );
//				PlugInProductName( (wchar_t *)(wsMsg1.str()), (wchar_t *)(fullProductName.str()) );
//				wsMsg1 = Fix_Single_Ampersands ( wsMsg1, false );
//				swprintf( szWholeWSMsg, L"%s", wsMsg1.str() );

//				memset( szInstallWarningMsg, '\0', _MAX_PATH );

//				Locale_GetString( IDS_INSTALL_WARNING_MSG, szWideBuffer );
//				wideBuffer = TheGameText->fetch("Autorun:InstallWarningMsg");

//				swprintf( szInstallWarningMsg, wideBuffer.str(), fullProductName.str() );

				//=======================================================================
				// Load the correct background & animation bitmap.
				//=======================================================================
				memset( buffer1, '\0', sizeof( buffer1 ));

				if( b640X480 ) {

					strcpy( szBitmap, _TEXT( "Background2" ));

					if ( LANG_FRE == LanguageID ) {
						strcpy( szLicense, _TEXT( "License_FRENCH2" ));
					} else if ( LANG_GER == LanguageID ) {
						strcpy( szLicense, _TEXT( "License_GERMAN2" ));
					} else {
						strcpy( szLicense, _TEXT( "License_USA2" ));
					}
					license_rect.left   = 186;
					license_rect.top    = 414;

				} else if( b800X600 ) {

					strcpy( szBitmap, _TEXT( "Background" ));

					if ( LANG_FRE == LanguageID ) {
						strcpy( szLicense, _TEXT( "License_FRENCH2" ));
					} else if ( LANG_GER == LanguageID ) {
						strcpy( szLicense, _TEXT( "License_GERMAN2" ));
					} else {
						strcpy( szLicense, _TEXT( "License_USA2" ));
					}
					license_rect.left   = 186;
					license_rect.top    = 414;

				} else {

					strcpy( szBitmap, _TEXT( "Background" ));
					strcpy( buffer1, "FLICKER" );

					if ( LANG_FRE == LanguageID ) {
						strcpy( szLicense, _TEXT( "License_FRENCH" ));
					} else if ( LANG_GER == LanguageID ) {
						strcpy( szLicense, _TEXT( "License_GERMAN" ));
					} else {
						strcpy( szLicense, _TEXT( "License_USA" ));
					}
					license_rect.left   = 238;
					license_rect.top    = 580;
				}

				//=======================================================================
				// Load flicker bitmap.
				//=======================================================================
				for( i = 0; i < NUM_FLICKER_FRAMES; i++ ) {
					hFlicker[i] = 0;
//					sprintf( buffer2, "%s%02d", buffer1, i );
//					hFlicker[i] = LoadResourceBitmap( Main::hModule, buffer2, &hpal );
					hFlicker[i] = LoadResourceBitmap( Main::hModule, buffer1, &hpal );
				}

				//-----------------------------------------------------------------------
				// Get the flicker bitmap's dimensions.
				//-----------------------------------------------------------------------
				result = 712;

				for( i = 0; i < NUM_FLICKER_POSITIONS; i++ ) {
					if( i == 8 ) {
						FlickerPositions[i][0] = result - 33;
					} else if( i == 9 ) {
						FlickerPositions[i][0] = result - 35;
					} else if ( i > 8 ) {
						FlickerPositions[i][0] = result - 28;
					} else {
						FlickerPositions[i][0] = result - 31;
					}
					FlickerPositions[i][1] = 560;
					result = FlickerPositions[i][0];
				}

				if ( hFlicker[0] ) {
					GetObject( hFlicker[0], sizeof( BITMAP ), (LPTSTR)&fm );
					flicker_rect.left   = FlickerPositions[0][0];
					flicker_rect.top    = FlickerPositions[0][1];
					flicker_rect.right  = flicker_rect.left + fm.bmWidth;
					flicker_rect.bottom = flicker_rect.top  + fm.bmHeight;
				}

				//-----------------------------------------------------------------------
				// Get the bitmap's dimensions.
				//-----------------------------------------------------------------------
				hLicenseBitmap  = LoadResourceBitmap( Main::hModule, szLicense, &hpal );
				if ( hLicenseBitmap ) {
					GetObject( hLicenseBitmap, sizeof( BITMAP ), (LPTSTR)&lm );
					license_rect.right  = license_rect.left + lm.bmWidth;
					license_rect.bottom = license_rect.top  + lm.bmHeight;
				}

				//=======================================================================
				// Load background bitmap.
				//=======================================================================
				hBitmap  = LoadResourceBitmap( Main::hModule, szBitmap, &hpal );
				if ( hBitmap ) {
					GetObject( hBitmap, sizeof( BITMAP ), (LPTSTR)&bm );
				}

				//-----------------------------------------------------------------------
				// Set the x, y, width, height of the Dialog and bitmaps dimensions.
				//-----------------------------------------------------------------------
				bitmap_rect.left  	= dlg_rect.left		= 0;
				bitmap_rect.top		= dlg_rect.top		= 0;
				bitmap_rect.right 	= dlg_rect.right 	= bm.bmWidth;
				bitmap_rect.bottom	= dlg_rect.bottom 	= bm.bmHeight;

				//-----------------------------------------------------------------------
				// Set the x, y, width, height of the Dialog and bitmaps dimensions.
				//-----------------------------------------------------------------------
				dlg_rect.left	= 0;
				dlg_rect.top	= 0;

				if( b640X480 ) {
					dlg_rect.right 		= rect.right;
					dlg_rect.bottom 	= tray_rect.bottom;											// desktop smaller than image
				} else if( b800X600 ) {
					if(true){
						dlg_rect.right 		= bm.bmWidth + 6;
						dlg_rect.bottom 	= bm.bmHeight + GetSystemMetrics( SM_CYCAPTION ) + 6;		// desktop larger than image
					} else {
						dlg_rect.right 		= 640;//bm.bmWidth + 6;
						dlg_rect.bottom 	= 480; //bm.bmHeight + GetSystemMetrics( SM_CYCAPTION ) + 6;		// desktop larger than image
					}
				} else {
					dlg_rect.right 		= bm.bmWidth + 6;
					dlg_rect.bottom 	= bm.bmHeight + GetSystemMetrics( SM_CYCAPTION ) + 6;		// desktop larger than image
				}

				//=======================================================================
				// Recreate Buttons based on Registry.
				//=======================================================================
				GlobalMainWindow->Create_Buttons( window_handle, &dlg_rect );

				//=======================================================================
				// Set Main Rectangle Areas around all the Buttons.
				//=======================================================================

				//-----------------------------------------------------------------------
				// Who is the first button?
				//-----------------------------------------------------------------------
				i = 0;
				while ( i < NUM_BUTTONS ) {
					if ( ButtonList[i] == NULL ) {
						i++;
					} else {
						break;
					}
				}

				if( i >= NUM_BUTTONS || i < 0 ) {
					i = 0;
				}

				Msg( __LINE__, TEXT(__FILE__), TEXT("----------------------------- determining button area ---------------------------" ));

				buttons_rect.left	= ButtonSizes[i].left;
				buttons_rect.top  	= ButtonSizes[i].top;
				buttons_rect.right	= ButtonSizes[i].left + ButtonSizes[i].right;
				buttons_rect.bottom	= ButtonSizes[i].top  +  ButtonSizes[i].bottom;

//				Msg( __LINE__, TEXT(__FILE__), TEXT("buttons_rect = [%d,%d,%d,%d]"), buttons_rect.left, buttons_rect.top, buttons_rect.right, buttons_rect.bottom );

				for( j = 0; j < NUM_BUTTONS; j++ ) {
					if ( ButtonList[j] != NULL ) {
						buttons_rect.left	= __min( ButtonSizes[j].left							, buttons_rect.left	 );
						buttons_rect.top  	= __min( ButtonSizes[j].top								, buttons_rect.top 	 );
						buttons_rect.right	= __max( ButtonSizes[j].left + ButtonSizes[j].right		, buttons_rect.right  );
						buttons_rect.bottom	= __max( ButtonSizes[j].top  + ButtonSizes[j].bottom	, buttons_rect.bottom );
					}
				}

				Msg( __LINE__, TEXT(__FILE__), TEXT("buttons_rect = [%d,%d,%d,%d]"), buttons_rect.left, buttons_rect.top, buttons_rect.right, buttons_rect.bottom );
				Msg( __LINE__, TEXT(__FILE__), TEXT("----------------------------- determining button area ---------------------------" ));

				//=======================================================================
				// Center the dialog on the desktop.
				//=======================================================================
				if ( !b640X480 ) {
					MoveWindow(	window_handle,
						( rect.right  - dlg_rect.right  )/2,
						( rect.bottom - dlg_rect.bottom )/2,
						dlg_rect.right,
						dlg_rect.bottom, TRUE );
				} else {
					MoveWindow(	window_handle,
						0, 0,
						dlg_rect.right,
						dlg_rect.bottom, TRUE );
				}

				//-----------------------------------------------------------------------
				// Get the new Client area.
				//-----------------------------------------------------------------------
				GetClientRect( window_handle, &dlg_rect );


				//=======================================================================================
				// JFS: 8/26/03 -- This was not getting cleared so if the button cnt were reduced...
				//=======================================================================================
				memset(	BackgroundRect, 0, sizeof ( BackgroundRect ) );

				//=======================================================================
				// These are the areas of the Background to paint minus the Button Area.
				// This will prevent "flickering".
				//=======================================================================
				BackgroundRect[0].left		= dlg_rect.left;
				BackgroundRect[0].top		= dlg_rect.top;
				BackgroundRect[0].right		= dlg_rect.right;
				BackgroundRect[0].bottom	= buttons_rect.top;

				BackgroundRect[1].left		= dlg_rect.left;
				BackgroundRect[1].top		= buttons_rect.top;
				BackgroundRect[1].right		= buttons_rect.left;
				BackgroundRect[1].bottom	= dlg_rect.bottom;

				BackgroundRect[2].left		= buttons_rect.left;
				BackgroundRect[2].top		= buttons_rect.bottom;
				BackgroundRect[2].right		= buttons_rect.right;
				BackgroundRect[2].bottom	= dlg_rect.bottom;

				BackgroundRect[3].left		= buttons_rect.right;
				BackgroundRect[3].top		= buttons_rect.top;
				BackgroundRect[3].right		= dlg_rect.right;
				BackgroundRect[3].bottom	= dlg_rect.bottom;

				//=======================================================================
				// Find Areas that are in between, infront of, and behind each Buttons.
				//=======================================================================
				i = 0;
				j = 4;

				//-----------------------------------------------------------------------
				// Who is the first button?
				//-----------------------------------------------------------------------
				while ( i < NUM_BUTTONS ) {
					if ( ButtonList[i] == NULL ) {
						i++;
					} else {
						break;
					}
				}

				if( i >= NUM_BUTTONS || i < 0 ) {
					i = 0;
				}

				//-----------------------------------------------------------------------
				// For each button...
				//-----------------------------------------------------------------------
				int index = i;
				for( ; index < NUM_BUTTONS; index++ ) {

					//-------------------------------------------------------------------
					// Make areas between the buttons.
					//-------------------------------------------------------------------
					if ( ButtonList[index] != NULL && ButtonList[index+1] != NULL ) {

						// Area between buttons.
						BackgroundRect[j].top		= ButtonList[index]->Return_Y_Pos() + ButtonList[index]->Return_Height();
						BackgroundRect[j].bottom	= ButtonList[index+1]->Return_Y_Pos() - BackgroundRect[j].top;
						BackgroundRect[j].left		= BackgroundRect[1].right;
						BackgroundRect[j].right		= BackgroundRect[3].left - BackgroundRect[1].left - 1;
						j++;
					}

					//-------------------------------------------------------------------
					// Now look for areas in front of and behind each button.
					//-------------------------------------------------------------------
					if ( ButtonList[index] != NULL ) {

						// Area in front of buttons.
						BackgroundRect[j].top		= ButtonList[index]->Return_Y_Pos();
						BackgroundRect[j].bottom	= ButtonList[index]->Return_Height();
						BackgroundRect[j].left		= BackgroundRect[1].right;
						BackgroundRect[j].right		= ButtonList[index]->Return_X_Pos() - BackgroundRect[1].right;
						j++;

						// Area behind buttons.
						BackgroundRect[j].top		= ButtonList[index]->Return_Y_Pos();
						BackgroundRect[j].bottom	= ButtonList[index]->Return_Height();
						BackgroundRect[j].left		= ButtonList[index]->Return_X_Pos() + ButtonList[index]->Return_Width();
						BackgroundRect[j].right		= BackgroundRect[3].left - ( ButtonList[index]->Return_X_Pos() + ButtonList[index]->Return_Width());
						j++;
					}
				}

				#if(RTS_DEBUG)
					Msg( __LINE__, TEXT(__FILE__), TEXT("----------------------------- WM_INITDIALOG ---------------------------" ));
					Msg( __LINE__, TEXT(__FILE__), TEXT("DeskTopWindowRect(w/o tray) = [%d,%d,%d,%d]"), tray_rect.left, tray_rect.top, tray_rect.right, tray_rect.bottom );
					Msg( __LINE__, TEXT(__FILE__), TEXT("DeskTopWindowRect	= [%d,%d,%d,%d]"), rect.left, rect.top, rect.right, rect.bottom );
					Msg( __LINE__, TEXT(__FILE__), TEXT("b640X480		= [%d]"), b640X480 );
					Msg( __LINE__, TEXT(__FILE__), TEXT("b800X600		= [%d]"), b800X600 );
					Msg( __LINE__, TEXT(__FILE__), TEXT("ClientRect		= [%d,%d,%d,%d]"), dlg_rect.left, dlg_rect.top, dlg_rect.right, dlg_rect.bottom );

					for( index = 0; index < ( NUM_BUTTONS * 3 ) + 3; index++ ) {
						Msg( __LINE__, TEXT(__FILE__), TEXT("BackgroundRect[%d]	= [%d,%d,%d,%d]"), index, BackgroundRect[index].top, BackgroundRect[index].bottom, BackgroundRect[index].left, BackgroundRect[index].right );
					}

					Msg( __LINE__, TEXT(__FILE__), TEXT("BitmapRect	   	= [%d,%d,%d,%d]"), bitmap_rect.left, bitmap_rect.top, bitmap_rect.right, bitmap_rect.bottom );
					Msg( __LINE__, TEXT(__FILE__), TEXT("FlickerRect		= [%d,%d,%d,%d]"), flicker_rect.left, flicker_rect.top, flicker_rect.right, flicker_rect.bottom );
					Msg( __LINE__, TEXT(__FILE__), TEXT("ButtonsRect		= [%d,%d,%d,%d]"), buttons_rect.left, buttons_rect.top, buttons_rect.right, buttons_rect.bottom );
					Msg( __LINE__, TEXT(__FILE__), TEXT("-----------------------------------------------------------------------" ));
				#endif

			#else

				//-----------------------------------------------------------------------
				// Create Brush.
				//-----------------------------------------------------------------------
				hStaticBrush = CreateSolidBrush( RGB( 192, 192, 192 ));

			#endif

				//=======================================================================
				// Set dialog's timer!  1000 = 1 second.
				//=======================================================================
//				timer_id = SetTimer( window_handle, 1000, 250L, NULL );
				timer_id = SetTimer( window_handle, 1000, 500L, NULL );
			}
			return( TRUE );

		//-------------------------------------------------------------------------------
		// Try and set a custom cursor.
		//-------------------------------------------------------------------------------
//		case WM_SETCURSOR:
//			SetCursor(LoadCursor( Main::hInstance, MAKEINTRESOURCE(2)));
//			break;

		//-------------------------------------------------------------------------------
		// Tell Window to Select Palette if we are not the Focused Window.
		// This is to avoid getting stuck in a loop of receiving this msg
		// when we are focused.
		//-------------------------------------------------------------------------------
		case WM_PALETTECHANGED:
			if (( HWND )w_param != window_handle ) {
				SendMessage( window_handle, WM_QUERYNEWPALETTE, w_param, l_param );
			}
			break;

		//-------------------------------------------------------------------------------
		// Set and Realize our palette, then repaint if necessary.
		// Note that SelectPalette here is passed a FALSE.
		// This means reset palette as if we are in the foreground.
		//-------------------------------------------------------------------------------
		case WM_QUERYNEWPALETTE:
			hDC = GetDC( window_handle );
			hpalold = SelectPalette( hDC, hpal, FALSE );
			i = RealizePalette( hDC );
			if ( i != 0 ) {
				InvalidateRect( window_handle, &dlg_rect, TRUE );
			}
			SelectPalette( hDC, hpalold, TRUE );
			RealizePalette( hDC );
			ReleaseDC( window_handle, hDC );
			return i;

		//-------------------------------------------------------------------------------
		// Process Timer Messages.
		//-------------------------------------------------------------------------------
		case WM_TIMER:
			{
				if ( w_param == 1000 ) {

//					if ( Flicker && hFlicker[FlickerIndex] ) {
					if ( Flicker && hFlicker[0] ) {

						FlickerIndex++;
//						if ( FlickerIndex >= NUM_FLICKER_FRAMES ) {
						if ( FlickerIndex >= NUM_FLICKER_POSITIONS ) {
							FlickerIndex = 0;
						}

						InvalidateRect( window_handle, &flicker_rect, FALSE );
						flicker_rect.left  = FlickerPositions[ FlickerIndex ][0];
						InvalidateRect( window_handle, &flicker_rect, FALSE );
						UpdateWindow( window_handle );

						Msg( __LINE__, TEXT(__FILE__), TEXT("WM_TIMER: FlickerRect = [%d,%d,%d,%d]"), flicker_rect.left, flicker_rect.right, flicker_rect.top, flicker_rect.bottom );
					}
				}

				//-----------------------------------------------------------------------
				// If game is running, the mutex will return!  Time to exit...
				// Note:  This number is unique for Tiberian Sun ONLY!!!
				//-----------------------------------------------------------------------
				if( WinVersion.Is_Win_XP() || WinVersion.Version() > 500 ) {
					strcat( strcpy( szBuffer, "Global\\" ), GAME_MUTEX_OBJECT );
				} else {
					strcpy( szBuffer, GAME_MUTEX_OBJECT );
				}
				GameAppMutex = OpenMutex( MUTEX_ALL_ACCESS, FALSE, szBuffer );

				if ( GameAppMutex != NULL ) {

					//---------------------------------------------------------
					// Handle(s) are closed in the ProgEnd().
					//---------------------------------------------------------

					HWND ccwindow = FindWindow( szGameWindow, NULL );
					if ( ccwindow ) {
						if( IsIconic( ccwindow )){
							ShowWindow( ccwindow, SW_RESTORE );
						}
						SetForegroundWindow( ccwindow );

					} else {

						ccwindow = FindWindow( szSetupWindow, NULL );
						if ( ccwindow ) {
							if( IsIconic( ccwindow )){
								ShowWindow( ccwindow, SW_RESTORE );
							}
							SetForegroundWindow( ccwindow );
						}
					}

					PostMessage( window_handle, WM_COMMAND, (WPARAM)MAKELONG( IDD_CANCEL, BN_CLICKED ), (LPARAM)GetDlgItem( window_handle, IDD_CANCEL ));
				}
			}
			return ( 0 );

		//===============================================================================
		// Dialog's main paint routine.
		//===============================================================================
		case WM_PAINT:

			//---------------------------------------------------------------------------
			// If there is an area to update, repaint it.
			//---------------------------------------------------------------------------
			if ( GetUpdateRect( window_handle, &paint_rect, FALSE )) {

//				Msg( __LINE__, TEXT(__FILE__), TEXT("---------------------- WM_PAINT ---------------------"), i );
//				Msg( __LINE__, TEXT(__FILE__), TEXT("Rectangle to update  = [%d,%d,%d,%d]"), paint_rect.left, paint_rect.top, paint_rect.right, paint_rect.bottom );

				//-----------------------------------------------------------------------
				// Get dialog's hDC.
				//-----------------------------------------------------------------------
				hDC = BeginPaint( window_handle, &ps );

				#if( BACKGROUND_BITMAP )

					if ( hBitmap ) {

						//---------------------------------------------------------------
						// Create a compatible DC for the bitmap.
						//---------------------------------------------------------------
						memDC		= CreateCompatibleDC( hDC );
						buttonDC	= CreateCompatibleDC( hDC );
						licenseDC	= CreateCompatibleDC( hDC );

						//---------------------------------------------------------------
						// Set stretching mode for bitmaps.
						//---------------------------------------------------------------
						prevDCStretchMode	  		= SetStretchBltMode( hDC, STRETCH_DELETESCANS );
						prevMemDCStretchMode  		= SetStretchBltMode( memDC,	STRETCH_DELETESCANS );
						prevButtonDCStretchMode		= SetStretchBltMode( buttonDC,STRETCH_DELETESCANS );
						prevLicenseDCStretchMode	= SetStretchBltMode( licenseDC,STRETCH_DELETESCANS );

						//---------------------------------------------------------------
						// Set the palette in each DC.
						//---------------------------------------------------------------
						hpalold = SelectPalette( hDC, hpal, FALSE );
						RealizePalette( hDC );
						SelectPalette( memDC, hpal, FALSE );
						RealizePalette( memDC );
						SelectPalette( buttonDC, hpal, FALSE );
						RealizePalette( buttonDC );
						SelectPalette( licenseDC, hpal, FALSE );
						RealizePalette( licenseDC );

						//---------------------------------------------------------------
						// If area to update is a button area, this will be drawn farther
						// on. This is to prevent "flickering" by drawing the background
						// then the button image.
						//---------------------------------------------------------------
						for ( i = 0; i < NUM_BUTTONS; i++ ) {

							if ( ButtonList[i] ) {

								ButtonList[i]->Return_Area( &rect );

								if ( paint_rect.left	== rect.left	&&
									 paint_rect.top		== rect.top		&&
									 paint_rect.right	== rect.right	&&
									 paint_rect.bottom	== rect.bottom ) {

									PaintBackground = FALSE;
//									Msg( __LINE__, TEXT(__FILE__), TEXT("Rectangle matches a button to update = [%d,%d,%d,%d]"), rect.left, rect.top, rect.right, rect.bottom );
									break;
								}
							}
						}

						//===============================================================
						// Paint the background.
						//===============================================================
						if ( PaintBackground ) {

							//-----------------------------------------------------------
							// Select & Draw the background bitmap.
							//-----------------------------------------------------------
							oldBitmap = ( HBITMAP )SelectObject( memDC, hBitmap );

							//-----------------------------------------------------------
							// Updates area around the button area.
							//-----------------------------------------------------------
						#if(1)
							for( int i=0; i < ( NUM_BUTTONS * 3 ) + 3; i++ ) {

								if( BackgroundRect[i].left	!= 0 || BackgroundRect[i].top	 != 0 ||
									BackgroundRect[i].right != 0 || BackgroundRect[i].bottom != 0 ) {

									result = StretchBlt( hDC,
										BackgroundRect[i].left,
										BackgroundRect[i].top,
										BackgroundRect[i].right,
										BackgroundRect[i].bottom,
  										memDC,
										BackgroundRect[i].left,
										BackgroundRect[i].top,
										BackgroundRect[i].right,
										BackgroundRect[i].bottom,
										SRCCOPY );

									if( result != 0 ) {
//										Msg( __LINE__, TEXT(__FILE__), TEXT("Paint this rect = [%d,%d,%d,%d]"), BackgroundRect[i].left, BackgroundRect[i].top, BackgroundRect[i].right, BackgroundRect[i].bottom );
									}

								#if(0)
									HPEN 	pen		= CreatePen( /*PS_DOT*/ PS_SOLID, 1, TEXT_COLOR );
									HGDIOBJ	oldpen	= SelectObject( hDC, pen );
									SetBkMode( hDC, TRANSPARENT );

									MoveToEx(	hDC, BackgroundRect[i].left+1,  BackgroundRect[i].top+1,	NULL );
									LineTo(		hDC, BackgroundRect[i].right-1,	BackgroundRect[i].top+1 );
									LineTo(		hDC, BackgroundRect[i].right-1,	BackgroundRect[i].bottom-1 );
									LineTo(		hDC, BackgroundRect[i].left+1,	BackgroundRect[i].bottom-1 );
									LineTo(		hDC, BackgroundRect[i].left+1,	BackgroundRect[i].top+1 );

									SelectObject( hDC, oldpen );
									DeleteObject( pen );
								#endif
								}
							}
/*
							//-----------------------------------------------------------
							// Select & Draw the background bitmap.
							//-----------------------------------------------------------
							oldLicenseBitmap = ( HBITMAP )SelectObject( licenseDC, hLicenseBitmap );

							//-----------------------------------------------------------
							// Updates area around the button area.
							//-----------------------------------------------------------
							result = StretchBlt( hDC,
										license_rect.left,
										license_rect.top,
										license_rect.right,
										license_rect.bottom,
  										licenseDC,
										0,
										0,
										license_rect.right,
										license_rect.bottom,
										SRCCOPY );
*/
						#else

							//-----------------------------------------------------------
							// Blit whole background, in one shot.
							//-----------------------------------------------------------
							StretchBlt( hDC, dlg_rect.left, dlg_rect.top, dlg_rect.right, dlg_rect.bottom,
										memDC, bitmap_rect.left, bitmap_rect.top, bitmap_rect.right, bitmap_rect.bottom,
										SRCCOPY );
						#endif

							SelectObject( memDC, oldBitmap );
							SelectObject( licenseDC, oldLicenseBitmap );

						} else {
							PaintBackground = TRUE;
						}

						//---------------------------------------------------------------
						// Animation.
						//---------------------------------------------------------------
//						if ( Flicker && hFlicker[FlickerIndex] ) {
						if ( Flicker && hFlicker[0] ) {

//							oldBitmap = ( HBITMAP )SelectObject( memDC, hFlicker[FlickerIndex] );
							oldBitmap = ( HBITMAP )SelectObject( memDC, hFlicker[0] );

							StretchBlt(
								hDC,
								flicker_rect.left,
								flicker_rect.top,
								flicker_rect.right,
								flicker_rect.bottom,
								memDC,
								0,
								0,
								flicker_rect.right,
								flicker_rect.bottom,
								SRCCOPY );

							#if(0)
								HPEN 	pen		= CreatePen( /*PS_DOT*/ PS_SOLID, 1, TEXT_COLOR );
								HGDIOBJ	oldpen	= SelectObject( hDC, pen );
								SetBkMode( hDC, TRANSPARENT );

								MoveToEx(	hDC, flicker_rect.left+1,	flicker_rect.top+1,	NULL );
								LineTo(		hDC, flicker_rect.right-1,	flicker_rect.top+1 );
								LineTo(		hDC, flicker_rect.right-1,	flicker_rect.bottom-1 );
								LineTo(		hDC, flicker_rect.left+1,	flicker_rect.bottom-1 );
								LineTo(		hDC, flicker_rect.left+1,	flicker_rect.top+1 );

								SelectObject( hDC, oldpen );
								DeleteObject( pen );
							#endif

							SelectObject( memDC, oldBitmap );

//							Msg( __LINE__, TEXT(__FILE__), TEXT("		Drawing Flicker [%d,%d,%d,%d]."), flicker_rect.left, flicker_rect.top, flicker_rect.right, flicker_rect.bottom );
						}

						//---------------------------------------------------------------
						// Draw each button.
						//---------------------------------------------------------------
						for ( i = 0; i < NUM_BUTTONS; i++ ) {

							if ( ButtonList[i] ) {

								Rect rect;

								//-------------------------------------------------------
								// Uses Bitmaps or DrawText???
								//-------------------------------------------------------
								if ( ButtonList[i]->Draw_Bitmaps()) {

									RECT src_rect, dst_rect;

									strcpy( szButtonBitmap, ButtonList[i]->Return_Bitmap( ));
									hButtonBitmap = LoadResourceBitmap( Main::hInstance, szButtonBitmap, &hpal, TRUE );
									if ( hButtonBitmap ) {

										GetObject( hButtonBitmap, sizeof( BITMAP ), (LPTSTR)&bm );

										dst_rect.left		= ButtonList[i]->Return_X_Pos();
										dst_rect.top		= ButtonList[i]->Return_Y_Pos();
										dst_rect.right		= ButtonList[i]->Return_Stretched_Width();
										dst_rect.bottom		= ButtonList[i]->Return_Stretched_Height();
										src_rect.left		= 0;
										src_rect.top		= 0;
										src_rect.right		= bm.bmWidth;
										src_rect.bottom		= bm.bmHeight;

										//-----------------------------------------------
										// Draw the button's bitmap background.
										//-----------------------------------------------
										oldBitmap = ( HBITMAP ) SelectObject( buttonDC, hButtonBitmap );
										StretchBlt(
													hDC,
													dst_rect.left,
													dst_rect.top,
													dst_rect.right,
													dst_rect.bottom,
													buttonDC,
													src_rect.left,
													src_rect.top,
													src_rect.right,
													src_rect.bottom,
													SRCCOPY );

										SelectObject( buttonDC, oldBitmap );
										DeleteObject( hButtonBitmap );
										hButtonBitmap = 0;
									}

								}

#if(0)
								TTFontClass *fontptr = ButtonList[i]->Return_Font_Ptr();
								if ( fontptr ) {

									RECT outline_rect;

									ButtonList[i]->Return_Area( &outline_rect );
									ButtonList[i]->Return_Text_Area( &rect );

									/*
									** This function was combining the pixel color with the background,
									** so it never looked correct.
									*/
//									SetTextColor( hDC, RGB( 0, 240, 0 ));
//									DrawFocusRect(	hDC, &dst_rect );

									if ( ButtonList[i]->Get_State() == DrawButton::PRESSED_STATE ) {
										fontptr->Print(
											hDC,
											ButtonList[i]->Return_Text(),
											rect,
											TEXT_PRESSED_COLOR,
											TEXT_PRESSED_SHADOW_COLOR,
											TPF_BUTTON,
											TPF_SHADOW );

									} else if ( ButtonList[i]->Get_State() == DrawButton::FOCUS_STATE ) {
										fontptr->Print(
											hDC,
											ButtonList[i]->Return_Text(),
											rect,
											TEXT_FOCUSED_COLOR,
											TEXT_FOCUSED_SHADOW_COLOR,
											TPF_BUTTON,
											TPF_SHADOW );

									} else {
										fontptr->Print(
											hDC,
											ButtonList[i]->Return_Text(),
											rect,
											TEXT_NORMAL_COLOR,
											TEXT_NORMAL_SHADOW_COLOR,
											TPF_BUTTON,
											TPF_SHADOW );
									}

								#if(0)
									HPEN 	pen		= CreatePen( /*PS_DOT*/ PS_SOLID, 2, TEXT_COLOR );
									HGDIOBJ	oldpen	= SelectObject( hDC, pen );
									SetBkMode( hDC, TRANSPARENT );

									MoveToEx( hDC,					// handle to device context
										outline_rect.left,			// x-coordinate of new current position
										outline_rect.top,			// y-coordinate of new current position
										NULL );						// pointer to old current position

									LineTo( hDC,					// device context handle
										outline_rect.right,			// x-coordinate of line's ending point
										outline_rect.top );			// y-coordinate of line's ending point

									LineTo( hDC,					// device context handle
										outline_rect.right,			// x-coordinate of line's ending point
										outline_rect.bottom ); 		// y-coordinate of line's ending point

									LineTo( hDC,					// device context handle
										outline_rect.left, 			// x-coordinate of line's ending point
										outline_rect.bottom ); 		// y-coordinate of line's ending point

									LineTo( hDC,					// device context handle
										outline_rect.left, 			// x-coordinate of line's ending point
										outline_rect.top );			// y-coordinate of line's ending point

									SelectObject( hDC, oldpen );
									DeleteObject( pen );
								#endif
								}
#else
								ButtonList[i]->Draw_Text( hDC );
#endif



							}

						}

						//---------------------------------------------------------------
						// Used in debugging -- draw rect around where buttons are.
						//---------------------------------------------------------------
						#if(0)
							{
								HPEN hPen1  = CreatePen( PS_SOLID, 1, RGB( 255, 255, 255 ));
								if (hPen1) {

									for ( int i = 0; i < NUM_BUTTONS; i++ ) {
										if ( ButtonList[i] ) {

											HGDIOBJ	oldpen = SelectObject(  hDC, hPen1 );

											MoveToEx( hDC,
												ButtonList[i]->Return_X_Pos()-1,
												ButtonList[i]->Return_Y_Pos()-1, NULL );
											LineTo( hDC,
												ButtonList[i]->Return_X_Pos() + ButtonList[i]->Return_Width() + 1,
												ButtonList[i]->Return_Y_Pos()-1 );
											LineTo( hDC,
												ButtonList[i]->Return_X_Pos() + ButtonList[i]->Return_Width()  + 1,
												ButtonList[i]->Return_Y_Pos() + ButtonList[i]->Return_Height() + 1);
											LineTo( hDC,
												ButtonList[i]->Return_X_Pos()-1,
												ButtonList[i]->Return_Y_Pos() + ButtonList[i]->Return_Height() + 1);
											LineTo( hDC,
												ButtonList[i]->Return_X_Pos() - 1,
												ButtonList[i]->Return_Y_Pos() - 1);

											SelectObject( hDC, oldpen );
										}
									}
								}
								DeleteObject( hPen1 );
							}
						#endif

						//---------------------------------------------------------------
						// Restore all default objects to DCs and delete.
						//---------------------------------------------------------------
						SetStretchBltMode( hDC,	prevDCStretchMode );
						SetStretchBltMode( memDC, prevMemDCStretchMode );
						SetStretchBltMode( buttonDC, prevButtonDCStretchMode );
						SetStretchBltMode( licenseDC, prevLicenseDCStretchMode );

						SelectPalette( hDC,	hpalold, FALSE );
						SelectPalette( memDC, hpalold, FALSE );
						SelectPalette( buttonDC, hpalold, FALSE );
						SelectPalette( licenseDC, hpalold, FALSE );

						DeleteDC( memDC );
						DeleteDC( buttonDC );
						DeleteDC( licenseDC );

					}

					//===================================================================
					// Draw a solid colored background.
					//===================================================================
//					GetClientRect( window_handle, (LPRECT) &dlg_rect );
//					FillRect( hDC, &dlg_rect, (HBRUSH)( COLOR_WINDOW + 1 ));

					//===================================================================
					// Print text at bottom of screen.
					//===================================================================
					Rect		text_rect;
					TTFontClass *fontptr = NULL;

					if ( b640X480 ) {
						fontptr = TTTextFontPtr640;
					} else if ( b800X600 ) {
						fontptr = TTTextFontPtr800;
					} else {
						fontptr = TTTextFontPtr;
					}

					if ( fontptr ) {

						if ( b640X480 || b800X600 ) {
							text_rect.X			=  10;
							text_rect.Y			= 240;
							text_rect.Width		= 140;		//498;
							text_rect.Height	= 100;		//26;
						} else {
							text_rect.X			=  20;
							text_rect.Y			= 340;
							text_rect.Width		= 180;		//498;
							text_rect.Height	= 120;		//26;
						}

						#if(0)
							RECT one;

							one.left	= text_rect.X;
							one.top		= text_rect.Y;
							one.right	= text_rect.X + text_rect.Width;
							one.bottom	= text_rect.Y + text_rect.Height;

							FrameRect( hDC, &one, (HBRUSH)( COLOR_WINDOW + 1 ));
//							DrawFocusRect( hDC, &one );
						#endif

						//---------------------------------------------------------------
						// WESTWOOD_COM Message at the top.
						//---------------------------------------------------------------
/*
						fontptr->Print(
							hDC,
							szWholeWSMsg,
							text_rect,
							TEXT_COLOR,
							SHADOW_COLOR,
							TPF_CENTER_TEXT,
							TPF_SHADOW );
*/
					}

					//---------------------------------------------------------------
					// EA_COM Message at the bottom.
					//---------------------------------------------------------------
					#if(0)	// Moved this text to a bitmap.
					fontptr = TTLicenseFontPtr;
					if( fontptr ) {

						if ( b640X480 || b800X600 ) {
							text_rect.X			= 220;
							text_rect.Y			= 400;
							text_rect.Width		= 300;		//460;
							text_rect.Height	=  48;		//26;
						} else {
							text_rect.X			= 250;
							text_rect.Y			= 574;
							text_rect.Width		= 420;		//460;
							text_rect.Height	=  60;		//26;
						}

						#if(0)
							RECT one;

							one.left	= text_rect.X;
							one.top		= text_rect.Y;
							one.right	= text_rect.X + text_rect.Width;
							one.bottom	= text_rect.Y + text_rect.Height;

							FrameRect( hDC, &one, (HBRUSH)( COLOR_WINDOW + 1 ));
//							DrawFocusRect( hDC, &one );
						#endif

						fontptr->Print(
							hDC,
							szWholeString,
							text_rect,
							TEXT_COLOR,
							SHADOW_COLOR,
							TPF_CENTER_TEXT,
							TPF_SHADOW );
					}
					#endif

				#else
					//-------------------------------------------------------------------
					// Select the Brush if it was successfully created.
					//-------------------------------------------------------------------
					if ( hStaticBrush ) {
						HBRUSH  oldBrush   = (HBRUSH) SelectObject( hDC, hStaticBrush );
						GetClientRect( window_handle, (LPRECT) &dlg_rect );
						FillRect( hDC, &dlg_rect, hStaticBrush );
						SelectObject( hDC, oldBrush );
					}
				#endif

//				Msg( __LINE__, TEXT(__FILE__), TEXT("--------------------------------------------------------" ));

				EndPaint( window_handle, &ps );

				//-----------------------------------------------------------------------
				// Play DISK.WAV sound on CD.
				//-----------------------------------------------------------------------
				if ( FirstTime ) {
					if( UseSounds ) {
						PlaySound( szWavs[ SongNumber ], NULL, SND_ASYNC | SND_RESOURCE );
					}
					FirstTime = FALSE;
				}
			} /* end of if */
			break;

		//-------------------------------------------------------------------------------
		// Background needs to be erased.  Note we are returning 1 here to "fake"
		// Windows into thinking we have already repainted the background with
		// our window brush.  This prevents "flickering" because of background
		// being repainted ( ususally white ) before WM_PAINT is processed.
		//-------------------------------------------------------------------------------
		#if(BACKGROUND_BITMAP)
		case WM_ERASEBKGND:
			InvalidateRect( window_handle, &dlg_rect, FALSE );
			return ( 1 );
		#endif

		//-------------------------------------------------------------------------------
		// Check which button was pressed.  If Explorer button was pressed,
		// call it now so we don't have to exit dialog.
		//-------------------------------------------------------------------------------
		case WM_COMMAND:
			{
				idCtl = LOWORD( w_param );

				unsigned int	result		= TRUE;
				bool			end_dialog	= false;
				int				cd_drive;

				szBuffer[1] = '\0';
				szBuffer[0] = tolower( szArgvPath[0] );
//				cd_drive	= (int)( szBuffer[0] - 'a' + 1 );
				cd_drive	= (int)( szBuffer[0] - 'a' );

			#if(BACKGROUND_BITMAP)

				switch ( idCtl ) {

					//-------------------------------------------------------------------
					// IDD_MOHAVI
					//-------------------------------------------------------------------
					case IDD_PREVIEWS:
					{
						Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_PREVIEWS Selected." ));
						// show the previews in succession.  each will wait for the previous to finish
						// before playing.
						unsigned int success;

						char filepath[MAX_PATH];
						snprintf(filepath, MAX_PATH, "%s%s", szArgvPath, SC4AVI_FILENAME);

						success = GlobalMainWindow->Run_OpenFile(cd_drive, filepath, true);
//						if (success != 0) {
//							success = GlobalMainWindow->Run_OpenFile(cd_drive, BFAVI_FILENAME, true);
//						}
/*
						if (success == 0) {
							std::wstring wideBuffer = TheGameText->fetch("Autorun:CantRunAVIs");
							std::wstring wideBuffer2 = TheGameText->fetch("Autorun:Error");
							int length = wideBuffer.length();
							WideCharToMultiByte( CodePage, 0, wideBuffer.c_str(), length+1, szBuffer, _MAX_PATH, NULL, NULL );
							length = wideBuffer2.length();
							WideCharToMultiByte( CodePage, 0, wideBuffer2.c_str(), length+1, szBuffer2, _MAX_PATH, NULL, NULL );
							MessageBox( NULL, szBuffer, szBuffer2, MB_APPLMODAL | MB_OK );
						}
*/
					}
					break;

					case IDD_HELP:
					{
						std::wstring wFileName;
						wFileName = Locale_GetString(HELP_FILENAME);

						std::string fname;
						const wchar_t *tmp = wFileName.c_str();
						char hack[2] = "a";
						while (*tmp)
						{
							hack[0] = (char)( *tmp & 0xFF );
							fname.append( hack );
							tmp++;
						}

						char newdir[MAX_PATH];
						char olddir[MAX_PATH];
						char filepath[MAX_PATH];

						GetCurrentDirectory(MAX_PATH, olddir);

						snprintf(newdir, MAX_PATH, "%ssupport", szArgvPath);
						SetCurrentDirectory(newdir);

						snprintf(filepath, MAX_PATH, "%s%s", szArgvPath, fname.c_str());

						unsigned int success;
						success = GlobalMainWindow->Run_OpenFile(cd_drive, filepath, false);

						SetCurrentDirectory(olddir);

/*
						if (success == 0) {
							std::wstring wideBuffer = TheGameText->fetch("Autorun:CantRunHelp");
							std::wstring wideBuffer2 = TheGameText->fetch("Autorun:Error");
							int length = wideBuffer.length();
							WideCharToMultiByte( CodePage, 0, wideBuffer.c_str(), length+1, szBuffer, _MAX_PATH, NULL, NULL );
							length = wideBuffer2.length();
							WideCharToMultiByte( CodePage, 0, wideBuffer2.c_str(), length+1, szBuffer2, _MAX_PATH, NULL, NULL );
							MessageBox( NULL, szBuffer, szBuffer2, MB_APPLMODAL | MB_OK );
						}
*/
					}
					break;

					//-------------------------------------------------------------------
					// IDD_CANCEL
					//-------------------------------------------------------------------
					case IDD_CANCEL:
						Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_CANCEL Selected." ));
						end_dialog = true;
						break;

					//-------------------------------------------------------------------
					// IDD_OK	-- Install
					// IDD_OK2	-- Play
					//-------------------------------------------------------------------
					case IDD_OK:
					case IDD_OK2:
					case IDD_OK3:
					case IDD_OK4:

//						if( !Is_On_CD( PRODUCT_VOLUME_CD1 ) && IsEnglish ) {
						if( !Is_On_CD( PRODUCT_VOLUME_CD1 )) {

							//-----------------------------------------------------------
							// If false is returned, then CANCEL was pressed.
							//-----------------------------------------------------------
							char volume_to_match[ MAX_PATH ];

							Reformat_Volume_Name( PRODUCT_VOLUME_CD1, volume_to_match );
//							result = Prompt_For_CD( window_handle, volume_to_match, IDS_INSERT_CDROM_WITH_VOLUME1, IDS_EXIT_MESSAGE2, &cd_drive );
							result = Prompt_For_CD( window_handle, volume_to_match, "Autorun:InsertCDROMWithVolume1", "Autorun:ExitMessage2", &cd_drive );
						}

						if ( result ) {
							if ( idCtl == IDD_OK ) {
								Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_OK Selected." ));
								result = GlobalMainWindow->Run_Setup( window_handle, &dlg_rect, cd_drive );
							} else if ( idCtl == IDD_OK2 ) {
								Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_OK2 Selected." ));
								result = GlobalMainWindow->Run_Game( window_handle, &dlg_rect );
							} else if (idCtl == IDD_OK3 ) {
								Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_OK3 Selected, running WorldBuilder." ));
								result = GlobalMainWindow->Run_WorldBuilder( window_handle, &dlg_rect );
							} else if (idCtl == IDD_OK4 ) {
								Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_OK4 Selected, running PatchGet." ));
								result = GlobalMainWindow->Run_PatchGet( window_handle, &dlg_rect );
							}
						}

						if ( result ) {
							end_dialog = true;
						}
						break;

				#if(SHOW_MOH_DEMO)
					//-------------------------------------------------------------------
					// Launch demo from CD.
					//-------------------------------------------------------------------
					case IDD_VIEW_DEMO:

						if( !Is_On_CD( PRODUCT_VOLUME_CD2 )) {

							//-----------------------------------------------------------
							// If false is returned, then CANCEL was pressed.
							//-----------------------------------------------------------
							char volume_to_match[ MAX_PATH ];

							Reformat_Volume_Name( PRODUCT_VOLUME_CD2, volume_to_match );
//							result = Prompt_For_CD( window_handle, volume_to_match, IDS_INSERT_CDROM_WITH_VOLUME2, IDS_EXIT_MESSAGE2, &cd_drive );
							result = Prompt_For_CD( window_handle, volume_to_match, AsciiString("Autorun:InsertCDROMWithVolume2"), AsciiString("Autorun:ExitMessage2"), &cd_drive );
						}

						if ( result ) {
							Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_VIEW_DEMO Selected." ));
							result = GlobalMainWindow->Run_Demo( window_handle, &dlg_rect, cd_drive );
						}

						if ( result ) {
							end_dialog = true;
						}

						break;
				#endif

				#if( SHOW_GAMESPY_BUTTON )
					//-------------------------------------------------------------------
					// Launch GameSpy Website.
					//-------------------------------------------------------------------
					case IDD_GAMESPY:
						Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_GAMESPY Selected." ));
						if( ViewHTML( GAMESPY_WEBSITE ))
						{
							end_dialog = true;
						}
						else
						{
							Error_Message( Main::hInstance, AsciiString("Autorun:Generals"), AsciiString("Autorun:CantFindExplorer"), GAME_WEBSITE );
						}
						break;
				#endif

					//-------------------------------------------------------------------
					// Create a new online account.
					//-------------------------------------------------------------------
					case IDD_NEW_ACCOUNT:
						Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_NEW_ACCOUNT Selected." ));
						result = GlobalMainWindow->Run_New_Account( window_handle, &dlg_rect );
						if ( result ) {
							end_dialog = true;
						}
						break;

					//-------------------------------------------------------------------
					// IDD_REGISTER
					//-------------------------------------------------------------------
					case IDD_REGISTER:
						Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_REGISTER Selected." ));
						result = GlobalMainWindow->Run_Register( window_handle, &dlg_rect );
						if ( result ) {
							end_dialog = true;
						}
						break;

					//-------------------------------------------------------------------
					// IDD_INTERNET
					//-------------------------------------------------------------------
					case IDD_INTERNET:
						Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_INTERNET Selected." ));
						if( ViewHTML( GAME_WEBSITE )) {
							end_dialog = true;
						}
						else
						{
							Error_Message( Main::hInstance, "Autorun:Generals", "Autorun:CantFindExplorer", GAME_WEBSITE );
						}
						break;

					//-------------------------------------------------------------------
					// IDD_UPDATE
					//-------------------------------------------------------------------
					case IDD_UPDATE:
						Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_UPDATE Selected." ));
						result = GlobalMainWindow->Run_Auto_Update( window_handle, &dlg_rect );
						if ( result ) {
							end_dialog = true;
						}
						break;

					//-------------------------------------------------------------------
					// IDD_EXPLORE
					//-------------------------------------------------------------------
					case IDD_EXPLORE:
						Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_EXPLORE Selected." ));
						GlobalMainWindow->Run_Explorer( "", window_handle, &dlg_rect );
						end_dialog = true;
						break;

					//-------------------------------------------------------------------
					// IDD_UNINSTALL
					//-------------------------------------------------------------------
					case IDD_UNINSTALL:
						Msg( __LINE__, TEXT(__FILE__), TEXT("IDD_UNINSTALL Selected." ));
						result = GlobalMainWindow->Run_Uninstall( window_handle, &dlg_rect );

						//---------------------------------------------------------------
						// MML 5/27/99:  I am exiting here because the we launch
						// Uninstll.exe which in turn launches Uninst.exe thus
						// ::Run_Install ends before Uninst.exe is done.
						//---------------------------------------------------------------
						if ( result ) {
							end_dialog = true;
						}
						break;

					default:
						break;
				}

				//-----------------------------------------------------------------------
				// Exit Autorun.
				//-----------------------------------------------------------------------
				if( end_dialog ) {

					for ( i = 0; i < NUM_BUTTONS; i++ ) {
						delete( ButtonList[i] );
						ButtonList[i] = NULL;
					}
					if ( hpal ) {
						DeleteObject( hpal );
					}
					if ( hBitmap ) {
						DeleteObject( hBitmap );
					}
					for( i = 0; i < NUM_FLICKER_FRAMES; i++ ) {
						DeleteObject( hFlicker[i] );
						hFlicker[i] = 0;
					}
					Stop_Sound_Playing();
					KillTimer( window_handle, timer_id );
					EndDialog( window_handle, idCtl );
				}

			#else
				if ( hStaticBrush ) {
					DeleteObject( hStaticBrush );
					hStaticBrush = 0;
				}
				EndDialog( window_handle, idCtl );
				KillTimer( window_handle, timer_id );
				KillTimer( window_handle, gem_timer_id );
			#endif

			}
			break;

		//-------------------------------------------------------------------------------
		// This message is the response to the Close Button in upper right corner.
		//-------------------------------------------------------------------------------
		case WM_SYSCOMMAND:

			if ( w_param == SC_CLOSE ) {
				#if(BACKGROUND_BITMAP)

					for ( i = 0; i < NUM_BUTTONS; i++ ) {
						delete( ButtonList[i] );
						ButtonList[i] = NULL;
					}
					if ( hpal ) {
						DeleteObject( hpal );
					}
					if ( hBitmap ) {
						DeleteObject( hBitmap );
					}
					for( i = 0; i < NUM_FLICKER_FRAMES; i++ ) {
						DeleteObject( hFlicker[i] );
						hFlicker[i] = 0;
					}

				#else
					if ( hStaticBrush ) {
						DeleteObject( hStaticBrush );
						hStaticBrush = 0;
					}
				#endif

				//-----------------------------------------------------------------------
				// Stop the sound if still going.
				//-----------------------------------------------------------------------
				Stop_Sound_Playing();

				//-----------------------------------------------------------------------
				// Delete the arguments.
				//-----------------------------------------------------------------------
				delete( Args );
				Args = NULL;

				KillTimer( window_handle, timer_id );
				EndDialog( window_handle, w_param );
			}
			break;

		//-------------------------------------------------------------------------------
		// WM_SYSCOLORCHANGE Message.
		// If your applications uses controls in Windows 95/NT, forward the
		// WM_SYSCOLORCHANGE message to the controls.
		//-------------------------------------------------------------------------------
		#if( !BACKGROUND_BITMAP )
		case WM_SYSCOLORCHANGE:
			if ( hStaticBrush ) {
				DeleteObject( hStaticBrush );
				hStaticBrush = CreateSolidBrush( GetSysColor( COLOR_WINDOW ));
			}
			break;
		#endif

		//-------------------------------------------------------------------------------
		//	WM_CTLCOLOR Message.
		//	wParam					Handle to Child Window's device context
		//	LOWORD( lParam )		Child Window handle
		//	HIWORD( lParam )		Type of Window: 	CTLCOLOR_MSGBOX, _EDIT, _LISTBOX, _BTN, _DLG, _SCROLLBAR, _STATIC
		//
		//	WM_CTLCOLORMSGBOX
		//	WM_CTLCOLOREDIT
		//	WM_CTLCOLORLISTBOX
		//	WM_CTLCOLORBTN
		//	WM_CTLCOLORDLG
		//	WM_CTLCOLORSCROLLBAR
		//	WM_CTLCOLORSTATIC
		//	#define WM_CTLCOLOR								0x0019
		//	#define GET_WM_CTLCOLOR_HDC (wp, lp, msg)		(HDC)(wp)
		//	#define GET_WM_CTLCOLOR_HWND(wp, lp, msg)		(HWND)(lp)
		//	#define GET_WM_CTLCOLOR_TYPE(wp, lp, msg)		(WORD)(msg - WM_CTLCOLORMSGBOX)
		//	#define GET_WM_CTLCOLOR_MSG (type)				(WORD)(WM_CTLCOLORMSGBOX+(type))
		//-------------------------------------------------------------------------------
		#if( !BACKGROUND_BITMAP )
		case WM_CTLCOLOR:
			if ( HIWORD( l_param ) == CTLCOLOR_STATIC ) {

				SetTextColor(( HDC )w_param, GetSysColor( COLOR_WINDOWTEXT ));
				SetBkColor( (HDC)wParam, GetSysColor( COLOR_WINDOW ));
//				SetBkColor(( HDC )w_param, RGB( 192, 192, 192 ));

				UnrealizeObject( hStaticBrush );									// reset the origin of the brush next time used.
				point.x = point.y = 0;												// create a point.
				ClientToScreen( window_handle, &point );						// translate into screen coordinates.
				SetBrushOrgEx( (HDC)w_param, point.x, point.y, NULL );	// New Origin to use when next selected.
				return((LRESULT) hStaticBrush );
			}
		#endif

		//===============================================================================
		// Check where Left Mouse button was pressed.
		//===============================================================================
		#if(BACKGROUND_BITMAP)
		case WM_LBUTTONDOWN:
			{
				RECT rect;

				//----------------------------------------------------------------------
				// Get mouse coordinates.
				//----------------------------------------------------------------------
				mouse_x = LOWORD( l_param );
				mouse_y = HIWORD( l_param );

				//----------------------------------------------------------------------
				// For each button in the list...
				//----------------------------------------------------------------------
				for ( i = 0; i < NUM_BUTTONS; i++ ) {

					//-------------------------------------------------------------------
					// If mouse was clicked in one of the "buttons", then change
					// that button's state to "pressed".
					//-------------------------------------------------------------------
					if ( ButtonList[i] && ButtonList[i]->Is_Mouse_In_Region( mouse_x, mouse_y )) {

						if ( ButtonList[i]->Get_State() != DrawButton::PRESSED_STATE ) {

							ButtonList[i]->Return_Area ( &rect );
							ButtonList[i]->Set_State( DrawButton::PRESSED_STATE );
							InvalidateRect( window_handle, &rect, FALSE );

							Msg( __LINE__, TEXT(__FILE__), TEXT("WM_LBUTTONDOWN -- %s. rect = [%d,%d,%d,%d]."),
								ButtonList[i]->Return_Normal_Bitmap(), rect.left, rect.top, rect.right, rect.bottom );

							UpdateWindow( window_handle );
						}
						break;
					}
				}
			}
			break;
		#endif

		//===============================================================================
		// Check where Left Mouse button was released.
		//===============================================================================
		#if(BACKGROUND_BITMAP)
		case WM_LBUTTONUP:
			{
				RECT rect;
				int focus_index = 0;
				int found_focus = -1;

				//-----------------------------------------------------------------------
				// Get mouse coordinates.
				//-----------------------------------------------------------------------
				mouse_x = LOWORD( l_param );
				mouse_y = HIWORD( l_param );

				//=======================================================================
				// focus_index = previous PRESSED/FOCUSED button.
				// found_focus = new PRESSED/FOCUSED button ( if different ).
				//=======================================================================

				//-----------------------------------------------------------------------
				// First find the button that is either focused or pressed.
				//-----------------------------------------------------------------------
				for ( i = 0; i < NUM_BUTTONS; i++ ) {
					if ( ButtonList[i] ) {

						//---------------------------------------------------------------
						// Save index of button with focus.
						//---------------------------------------------------------------
						if(	ButtonList[i]->Get_State() == DrawButton::FOCUS_STATE ||
						  	ButtonList[i]->Get_State() == DrawButton::PRESSED_STATE ) {
							focus_index = i;
						}
					}
				}

				//-----------------------------------------------------------------------
				// Then find the button that is to be focused or pressed.
				//-----------------------------------------------------------------------
				for ( i = 0; i < NUM_BUTTONS; i++ ) {
					if ( ButtonList[i] && ButtonList[i]->Is_Mouse_In_Region( mouse_x, mouse_y )) {
						found_focus = i;
					}
				}

				//-----------------------------------------------------------------------
				// If new button is not found...
				//-----------------------------------------------------------------------
				if ( found_focus == -1 ) {

					//-------------------------------------------------------------------
					// Make sure previously focused/pressed button is now is a
					// focused state and no action is taken.  This occurs when
					// mouse is clicked outside of any button areas.
					//-------------------------------------------------------------------
					if ( ButtonList[focus_index] && ( ButtonList[focus_index]->Get_State() != DrawButton::FOCUS_STATE )) {

						ButtonList[focus_index]->Set_State( DrawButton::FOCUS_STATE );
						ButtonList[focus_index]->Return_Area ( &rect );
						InvalidateRect( window_handle, &rect, FALSE );

						Msg( __LINE__, TEXT(__FILE__), TEXT("WM_LBUTTONUP -- %s[FOCUS_STATE] = [x=%d, y=%d, w=%d, h=%d]."),
							ButtonList[ focus_index ]->Return_Normal_Bitmap(),	rect.left, rect.top, rect.right, rect.bottom );

						UpdateWindow( window_handle );
					}

				} else {

					//-------------------------------------------------------------------
					// Buttons are one and the same.
					//-------------------------------------------------------------------
					if( focus_index == found_focus ) {

						ButtonList[ found_focus ]->Set_State( DrawButton::FOCUS_STATE );
						ButtonList[ found_focus ]->Return_Area ( &rect );
						InvalidateRect( window_handle, &rect, FALSE );

						Msg( __LINE__, TEXT(__FILE__), TEXT("WM_LBUTTONUP -- %s[FOCUS_STATE] = [x=%d, y=%d, w=%d, h=%d]."),
							ButtonList[ found_focus ]->Return_Normal_Bitmap(), rect.left, rect.top, rect.right, rect.bottom );

						UpdateWindow( window_handle );

					} else {

						//---------------------------------------------------------------
						// Make previously focused button, Normal...
						//---------------------------------------------------------------
						if ( ButtonList[ focus_index ] ) {

							ButtonList[ focus_index ]->Set_State( DrawButton::NORMAL_STATE );
							ButtonList[ focus_index ]->Return_Area ( &rect );
							InvalidateRect( window_handle, &rect, FALSE );

							Msg( __LINE__, TEXT(__FILE__), TEXT("WM_LBUTTONUP -- %s[NORMAL_STATE] = [x=%d, y=%d, w=%d, h=%d]."),
								ButtonList[ focus_index ]->Return_Normal_Bitmap(),
								rect.left, rect.top, rect.right, rect.bottom );

							UpdateWindow( window_handle );
						}

						//---------------------------------------------------------------
						// ...and the new button now has focus.
						//---------------------------------------------------------------
						if ( ButtonList[ found_focus ] ) {

							ButtonList[ found_focus ]->Set_State( DrawButton::FOCUS_STATE );
							ButtonList[ found_focus ]->Return_Area ( &rect );
							InvalidateRect( window_handle, &rect, FALSE );

							Msg( __LINE__, TEXT(__FILE__), TEXT("WM_LBUTTONUP -- %s[FOCUS_STATE] = [x=%d, y=%d, w=%d, h=%d]."),
								ButtonList[ found_focus ]->Return_Normal_Bitmap(), rect.left, rect.top, rect.right, rect.bottom );

							UpdateWindow( window_handle );
						}
					}
				}

				//-----------------------------------------------------------------------
				// Repaint the Window now.
				//-----------------------------------------------------------------------
				nResult = UpdateWindow( window_handle );

				//-----------------------------------------------------------------------
				// Do the focus button's action.
				//-----------------------------------------------------------------------
				if ( found_focus >= 0 ) {
					if (( ButtonList[found_focus] ) &&
						( ButtonList[found_focus]->Get_State() == DrawButton::FOCUS_STATE ) &&
						( ButtonList[found_focus]->Is_Mouse_In_Region( mouse_x, mouse_y ))) {
							SendMessage( window_handle, WM_COMMAND, ButtonList[found_focus]->Return_Id(), 0L );
							break;
					}
				}
			}
			break;
		#endif

		//-------------------------------------------------------------------------------
		// Check Mouse moves over buttons.
		//-------------------------------------------------------------------------------
//#if(DISABLE_KEYBOARD)
		#if(BACKGROUND_BITMAP)
		case WM_MOUSEMOVE:
			{
				RECT rect;
				int j;
				int done = 0;

				//-----------------------------------------------------------------------
				// Get mouse coordinates.
				//-----------------------------------------------------------------------
				mouse_x = LOWORD( l_param );
				mouse_y = HIWORD( l_param );

			#if(USE_MOUSE_MOVES)
				//-----------------------------------------------------------------------
				// Reset most current button.
				//-----------------------------------------------------------------------
				CurrentButton = 0;
			#endif

				//-----------------------------------------------------------------------
				// For each button in the list...
				//-----------------------------------------------------------------------
				i = 0;
				while( i < NUM_BUTTONS && !done ) {

					//-------------------------------------------------------------------
					// For each button, check if mouse is in it's area.
					//-------------------------------------------------------------------
					if ( ButtonList[i] && ButtonList[i]->Is_Mouse_In_Region( mouse_x, mouse_y )) {

						//---------------------------------------------------------------
						// This is now the current button.
						//---------------------------------------------------------------
						CurrentButton = ButtonList[i]->Return_Id();

						if( CurrentButton != LastButton ) {

							//-----------------------------------------------------------
							// Make all other buttons, NORMAL.
							//-----------------------------------------------------------
							for ( j = 0; j < NUM_BUTTONS; j++ ) {
								if ( ButtonList[j] ) {
									ButtonList[j]->Set_State( DrawButton::NORMAL_STATE );
//									Msg( __LINE__, TEXT(__FILE__), TEXT("WM_MOUSEMOVE -- %s[NORMAL_STATE]]."), ButtonList[j]->Return_Normal_Bitmap());
								}
							}

							if ( w_param & MK_LBUTTON ) {

								//--------------------------------------------------------
								// Left Mouse button is pressed! Make it a pressed button!
								//--------------------------------------------------------
								if ( ButtonList[i] && ButtonList[i]->Get_State() != DrawButton::PRESSED_STATE ) {
									ButtonList[i]->Set_State( DrawButton::PRESSED_STATE );
//									Msg( __LINE__, TEXT(__FILE__), TEXT("WM_MOUSEMOVE -- %s[PRESSED_STATE]."), ButtonList[i]->Return_Normal_Bitmap());
								}

							} else {

								//--------------------------------------------------------
								// If this button is not already focused, give it the focus.
								//--------------------------------------------------------
								if ( ButtonList[i] && ButtonList[i]->Get_State() != DrawButton::FOCUS_STATE ) {
									ButtonList[i]->Set_State( DrawButton::FOCUS_STATE );
//									Msg( __LINE__, TEXT(__FILE__), TEXT("WM_MOUSEMOVE -- %s[FOCUS_STATE]."), ButtonList[i]->Return_Normal_Bitmap());
								}
							}

							//-----------------------------------------------------------
							// Get the area of the button, and post it for updating.
							//-----------------------------------------------------------
							for ( j = 0; j < NUM_BUTTONS; j++ ) {
								if ( ButtonList[j] ) {
									ButtonList[j]->Return_Area ( &rect );
									InvalidateRect( window_handle, &rect, FALSE );
								}
							}

							//-----------------------------------------------------------
							// Repaint now!
							//-----------------------------------------------------------
							UpdateWindow( window_handle );

							done = 1;
						}

					}
					i++;

				}


			#if( USE_MOUSE_MOVES )
	        	//-----------------------------------------------------------------------
				// If a MouseMove was found to be in one of the buttons, then
				// CurrentButton will have a value.
        		//-----------------------------------------------------------------------
				if ( CurrentButton != 0 ) {

					LastButton = CurrentButton;

		        	//-------------------------------------------------------------------
					// If we are still in the same button, don't make a sound!
				  	//-------------------------------------------------------------------
					if ( LastButton != PrevButton ) {
						PrevButton = LastButton;
						PlaySound( szButtonWav, Main::hModule, SND_ASYNC | SND_RESOURCE );
					}
				}
			#endif
			}
			break;

		#endif	// Background_Bitmap flag
//#endif

		//-------------------------------------------------------------------------------
		// Repaint when focus is restored (does partial repaint), and when
		// mouse is double clicked on dialog ( full repaint ).
		//-------------------------------------------------------------------------------
		case WM_LBUTTONDBLCLK:
		case WM_SETFOCUS:
			InvalidateRect( window_handle, &dlg_rect, TRUE );
//			nResult = UpdateWindow( window_handle );
//			Msg( __LINE__, TEXT(__FILE__), TEXT("WM_LBUTTONDBLCLK -- dlg_rect = [x=%d, y=%d, w=%d, h=%d]."),
//				dlg_rect.left, dlg_rect.top, dlg_rect.right, dlg_rect.bottom );
			break;

		#if(BACKGROUND_BITMAP)
		//-------------------------------------------------------------------------------
		// bit 30 of lParam - Specifies the previous key state.
		// The value is 1 if the key is down before the message is sent,
		// or it is 0 if the key is up.
		//-------------------------------------------------------------------------------
		case WM_KEYUP:
			{
//				int j = 0;

				switch( w_param ) {

					case VK_ESCAPE:
						SendMessage( window_handle, WM_SYSCOMMAND, SC_CLOSE, 0L );
						break;

//#if(DISABLE_KEYBOARD)
					case VK_RETURN:
						//---------------------------------------------------------------
						// If the Return/Enter key is pressed... find the focused
						//	button and call its action.
						//---------------------------------------------------------------
//						result = ( l_param & 0x40000000 );
						for ( i = 0; i < NUM_BUTTONS; i++ ) {
							if ( ButtonList[i] && ButtonList[i]->Get_State() == DrawButton::FOCUS_STATE ) {
								SendMessage( window_handle, WM_COMMAND, ButtonList[i]->Return_Id(), 0L );
								break;
							}
						}
						break;
//#endif

//#if(DISABLE_KEYBOARD)
					case VK_TAB:
					case VK_DOWN:
						{
							//-----------------------------------------------------------
							// Find the button with focus and "tab" to the next button by finding
							// the next valid index.  If past last button, circle back to the top.
							//-----------------------------------------------------------
							int focused_button = 0;
							int next_button = 0;

							for ( i = 0; i < NUM_BUTTONS; i++ ) {
								if ( ButtonList[i] && ButtonList[i]->Get_State() == DrawButton::FOCUS_STATE ) {

									focused_button = i;
									next_button = i+1;

									if ( next_button >= NUM_BUTTONS ) {
										next_button = 0;
									}
									while (( next_button < NUM_BUTTONS ) && !ButtonList[ next_button ] ) {
										next_button++;
									}

									if ( next_button >= NUM_BUTTONS ) {
										next_button = 0;
										while (( next_button < NUM_BUTTONS ) && !ButtonList[ next_button ] ) {
											next_button++;
										}
									}
									break;
								}
							}

							//-----------------------------------------------------------
							// Set the previous button to Normal status.
							//-----------------------------------------------------------
							if ( ButtonList[focused_button] && ( ButtonList[focused_button]->Get_State() != DrawButton::NORMAL_STATE )) {

								ButtonList[focused_button]->Set_State( DrawButton::NORMAL_STATE );
								ButtonList[focused_button]->Return_Area ( &rect );
								InvalidateRect( window_handle, &rect, FALSE );

								Msg( __LINE__, TEXT(__FILE__), TEXT("VK_DOWN/VK_TAB -- %s = [%s]."), ButtonList[focused_button]->Return_Normal_Bitmap(), "NORMAL_STATE" );
							}

							//-----------------------------------------------------------
							// Set the new button to focus status.
							//-----------------------------------------------------------
							if ( ButtonList[next_button] && ( ButtonList[next_button]->Get_State() != DrawButton::FOCUS_STATE )) {

								ButtonList[next_button]->Set_State( DrawButton::FOCUS_STATE );
								ButtonList[next_button]->Return_Area ( &rect );
								InvalidateRect( window_handle, &rect, FALSE );
								PlaySound( szButtonWav, Main::hModule, SND_ASYNC | SND_RESOURCE );

								Msg( __LINE__, TEXT(__FILE__), TEXT("VK_DOWN/VK_TAB -- %s = [%s]."), ButtonList[next_button]->Return_Normal_Bitmap(), "FOCUS_STATE" );
							}
						}
						break;

					case VK_UP:
						{
							//-----------------------------------------------------------
							// Find the button with focus and "tab" to the next button by finding
							// the next valid index.  If past last button, circle back to the top.
							//-----------------------------------------------------------
							int focused_button = 0;
							int next_button = 0;

							for ( i = 0; i < NUM_BUTTONS; i++ ) {
								if ( ButtonList[i] && ButtonList[i]->Get_State() == DrawButton::FOCUS_STATE ) {

									focused_button = i;
									next_button	= i-1;

									if ( next_button < 0 ) {
										next_button = NUM_BUTTONS - 1;
									}
									while (( next_button > 0 ) && !ButtonList[ next_button ] ) {
										next_button--;
									}

									if ( !ButtonList[ next_button ]) {
										next_button = NUM_BUTTONS - 1;
									}
									while (( next_button >= 0 ) && !ButtonList[ next_button ] ) {
										next_button--;
									}
									break;
								}
							}

							//-----------------------------------------------------------
							// Set the previous button to Normal status.
							//-----------------------------------------------------------
							if ( ButtonList[focused_button] && ( ButtonList[focused_button]->Get_State() != DrawButton::NORMAL_STATE )) {

								ButtonList[focused_button]->Set_State( DrawButton::NORMAL_STATE );
								ButtonList[focused_button]->Return_Area ( &rect );
								InvalidateRect( window_handle, &rect, FALSE );

								Msg( __LINE__, TEXT(__FILE__), TEXT("VK_DOWN/VK_TAB -- %s = [%s]."), ButtonList[focused_button]->Return_Normal_Bitmap(), "NORMAL_STATE" );
							}

							//-----------------------------------------------------------
							// Set the new button to focus status.
							//-----------------------------------------------------------
							if ( ButtonList[next_button] && ( ButtonList[next_button]->Get_State() != DrawButton::FOCUS_STATE )) {

								ButtonList[next_button]->Set_State( DrawButton::FOCUS_STATE );
								ButtonList[next_button]->Return_Area ( &rect );
								InvalidateRect( window_handle, &rect, FALSE );
								PlaySound( szButtonWav, Main::hModule, SND_ASYNC | SND_RESOURCE );

								Msg( __LINE__, TEXT(__FILE__), TEXT("VK_DOWN/VK_TAB -- %s = [%s]."), ButtonList[next_button]->Return_Normal_Bitmap(), "FOCUS_STATE" );
							}
						}
						break;
//#endif

				}	/* end of switch */
			}	/* end of case stmt */

			return ( 0 );

		#endif
	}
	return( FALSE );
}

//*****************************************************************************
// STOP_SOUND_PLAYING -- Stop the background sound.
//
// INPUT:		none.
//
// OUTPUT:		none.
//
// WARNINGS:	Will stop any sound started by PlaySound.
//
// HISTORY:
//   06/04/1999  MML : Created.
//=============================================================================

void Stop_Sound_Playing ( void )
{
	PlaySound( NULL, NULL, SND_ASYNC | SND_FILENAME );
}

//*****************************************************************************
// OPTIONS -- Find any user options and set the correct flags
//
// INPUT:   int argc      =  no. of arguments to check
//          BYTE *argv[]  =  ptr to actual command line parameters
//
// OUTPUT:
//
// WARNINGS:
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

BOOL Options( Command_Line_Arguments *Orgs )
{
	int		i;
	BOOL	result = TRUE;
	char 	buffer[ MAX_ARGUMENT_LENGTH ];

	//-------------------------------------------------------------------------
	// Scan arguments for any options ( / or - followed by a letter)
	//-------------------------------------------------------------------------
	for ( i = 0; i < Orgs->Get_argc(); i++ ) {

		//---------------------------------------------------------------------
		// Get the next item in the list.
		//---------------------------------------------------------------------
		memset( buffer, '\0', sizeof( buffer ));
		strcpy( buffer, Orgs->Get_argv(i));

		Msg( __LINE__, TEXT(__FILE__), TEXT("Options -- Argument[%d] = %s."), i, buffer );

		//---------------------------------------------------------------------
		// If starts with / or -
		//---------------------------------------------------------------------
		if (( buffer[0]  == '/' ) || ( buffer[0]  == '-' ))	{

			switch ( tolower( buffer[1] )) {

				//-------------------------------------------------------------
				// Retrieve the game's version info.
				//-------------------------------------------------------------
				case 'v':
					{
						char szPath   [ MAX_PATH ];
						char szVersion[ MAX_PATH ];

						Make_Current_Path_To( SETUP_INI_FILE1, szPath );
						GetPrivateProfileString( "Setup", "Version", "1.0", szVersion, sizeof( szVersion ), szPath );

						LoadString( Main::hInstance, IDS_VERSION_STRING, szBuffer,  MAX_PATH );

//						sprintf( szBuffer3, "V %s", szVersion );
						sprintf( szBuffer3, szBuffer, szVersion );
//						strcpy( szBuffer, szRegistryKey );

						MessageBox( NULL, szBuffer3, "Autorun", MB_TASKMODAL | MB_OK );
						result = FALSE;
					}
					break;

				//-------------------------------------------------------------
				// Bypass the volume check.
				//-------------------------------------------------------------
				case 'n':
					{
						HANDLE  handle;
						WIN32_FIND_DATA FindFileData;

						memset( szVolumeName, '\0', MAX_PATH );

						//-----------------------------------------------------
						// If we think we are on CD2, then use PRODUCT_VOLUME_CD2.
						//-----------------------------------------------------
						Make_Current_Path_To( MOH_DEMO_PROGRAM, szBuffer );
						handle = FindFirstFile( szBuffer, &FindFileData );
						if ( handle == INVALID_HANDLE_VALUE ) {
							strcpy( szVolumeName, PRODUCT_VOLUME_CD1 );
						} else {
							strcpy( szVolumeName, PRODUCT_VOLUME_CD2 );
							FindClose( handle );
						}

						strcpy( Arguments[ NumberArguments++ ], buffer );
					}
					break;

			#if( RTS_DEBUG )

				case 'c':
					if( buffer[2] == 'd' ) {
						szCDDrive[0] = buffer[3];
						szCDDrive[1] = ':';
						szCDDrive[2] = '\\';
					}
					break;

				//-------------------------------------------------------------
				// Change languages?
				//-------------------------------------------------------------
				case 'l':
					{
						//-----------------------------------------------------
						//	Identifier		Meaning
						//	932				Japan
						//	936				Chinese (PRC, Singapore)
						//	949				Korean
						//	1252			Windows 3.1 Latin 1 (US, Western Europe)
						//-----------------------------------------------------
						char *temp = buffer+2;
						int number = atoi( temp );

						switch( number ) {

								case LANG_GER:
									LanguageToUse = LANG_GER;
									CodePage = 1252;
									break;

								case LANG_FRE:
									LanguageToUse = LANG_FRE;
									CodePage = 1252;
									break;

								case LANG_JAP:
									LanguageToUse = LANG_JAP;
									CodePage = 932;
									break;

								case LANG_KOR:
									LanguageToUse = LANG_KOR;
									CodePage = 949;
									break;

								case LANG_CHI:
									LanguageToUse = LANG_CHI;
									CodePage = 950;
									break;

								case LANG_USA:
								default:
									LanguageToUse = LANG_USA;
									CodePage = 1252;
									break;
						}
					}
					break;


			#endif

				default:
					strcpy( Arguments[ NumberArguments++ ], buffer );
					break;
			}
		}
	}

	Msg( __LINE__, TEXT(__FILE__), TEXT("Options -- Language = %d"), Language );
	Msg( __LINE__, TEXT(__FILE__), TEXT("Options -- CodePage = %d"), CodePage );

#if(0)
	struct lconv *info = localeconv();
	char szDefaultLangID[ MAX_PATH ];

	GetLocaleInfo(
		  LOCALE_USER_DEFAULT,	// locale identifier
		  LOCALE_ILANGUAGE,		// type of information
		  szBuffer1,	 		// address of buffer for information
		  MAX_PATH );	 		// size of buffer

	Msg( __LINE__, TEXT(__FILE__), TEXT("Options -- GetLocalInfo = %s"), szBuffer1 );

	sprintf( szDefaultLangID, "%04X", GetUserDefaultLangID());
	Msg( __LINE__, __FILE__, "Options -- GetUserDefaultLangID() = %s", szDefaultLangID );

	sprintf( szDefaultLangID, "%04X", GetSystemDefaultLangID());
	Msg( __LINE__, __FILE__, "Options -- GetSystemDefaultLangID() = %s", szDefaultLangID );
	Msg( __LINE__, __FILE__, "-------------------------------------------------------------" );
#endif

	return( result );
}

//*****************************************************************************
// Valid_Environment -- Make sure this program is run from CD-ROM disc only
//					 	AND it is a Win95 system.
//
// INPUT:  		none.
//
//	OUTPUT: 	none.
//
// WARNINGS:	returns 0 if ok to continue.
//
// HISTORY:
//   06/04/1996  MML : Created.
//=============================================================================

BOOL Valid_Environment ( void )
{
	bool result = 0;

	//--------------------------------------------------------------------------
	// Check Windows Version.
	//--------------------------------------------------------------------------

	int length = 0;
	result = WinVersion.Meets_Minimum_Version_Requirements();
  if ( !result )
	{
		std::wstring wideBuffer = TheGameText->fetch("GUI:WindowsVersionText");
		std::wstring wideBuffer2 = TheGameText->fetch("GUI:WindowsVersionTitle");
		length = wideBuffer.length();
		WideCharToMultiByte( CodePage, 0, wideBuffer.c_str(), length+1, szBuffer, _MAX_PATH, NULL, NULL );
		length = wideBuffer2.length();
		WideCharToMultiByte( CodePage, 0, wideBuffer2.c_str(), length+1, szBuffer2, _MAX_PATH, NULL, NULL );
		MessageBox( NULL, szBuffer, szBuffer2, MB_APPLMODAL | MB_OK );
	}

	return( result );
}

//****************************************************************************
// LOADRESOURCEBITMAP -- Find & Load the bitmap from the resource.
//
// INPUT:   HINSTANCE hInstance -- Program's hInstance.
//          LPTSTR lpString -- name of bitmap to find.
//          HPALETTE FAR *lphPalette -- we will return palette in this.
//
// OUTPUT:  HBITMAP -- handle to the bitmap if found.
//
// WARNINGS:
//
// HISTORY: Found this routine on MS Developmemt CD, July 1996.
// 	09/26/1996  MML : Created.
//=============================================================================

HBITMAP LoadResourceBitmap( HINSTANCE hInstance, LPCTSTR lpString, HPALETTE FAR *lphPalette, bool loading_a_button )
{
//	HDC 		hdc;
	int 		iNumColors;
	HRSRC 		hRsrc;
	HGLOBAL 	hGlobal;
	HBITMAP 	hBitmapFinal = NULL;
	LPBITMAPINFOHEADER lpbi;

	hBitmapFinal = LoadBitmap( hInstance, lpString );

	//--------------------------------------------------------------------------
	// Find the Bitmap in this program's resources.
	//--------------------------------------------------------------------------
	hRsrc = FindResource( hInstance, lpString, RT_BITMAP );
	if ( hRsrc ) {

		//-----------------------------------------------------------------------
		// Load the resource, lock the memory, grab a DC.
		//-----------------------------------------------------------------------
		hGlobal	= LoadResource( hInstance, hRsrc );
		lpbi  	= (LPBITMAPINFOHEADER) LockResource( hGlobal );

		if ( loading_a_button ) {

			//--------------------------------------------------------------------------
			// Set number of colors ( 2 to the nth ).
			//--------------------------------------------------------------------------
			if ( lpbi->biBitCount <= 8 ) {
				iNumColors = ( 1 << lpbi->biBitCount );
			} else {
				iNumColors = 0;
			}

		} else {

			//--------------------------------------------------------------------
			// Get the palette from the resource.
			//--------------------------------------------------------------------
			*lphPalette = CreateDIBPalette((LPBITMAPINFO) lpbi, &iNumColors );
		}

		//-----------------------------------------------------------------------
		// Free DS and memory used.
		//-----------------------------------------------------------------------
		UnlockResource( hGlobal );
		FreeResource( hGlobal );
	}

	return( hBitmapFinal );
}

//*****************************************************************************
// CREATEDIBPALETTE -- Creates the palette from the Bitmap found above.
//
// INPUT:   LPBITMAPINFO lpbmi -- bitmap info from header.
//          LPINT lpiNumColors -- number of colors.
//
// OUTPUT:  HPALETTE -- handle to the bitmap if found.
//
// WARNINGS:
//
// HISTORY: Found this routine on MS Developmemt CD, July 1996.
// 	09/26/1996  MML : Created.
//=============================================================================
HPALETTE CreateDIBPalette ( LPBITMAPINFO lpbmi, LPINT lpiNumColors )
{
	LPBITMAPINFOHEADER lpbi;
	LPLOGPALETTE lpPal;
	HANDLE hLogPal;
	HPALETTE hPal = NULL;
	int i;

	lpbi = (LPBITMAPINFOHEADER) lpbmi;

	//--------------------------------------------------------------------------
	// Set number of colors ( 2 to the nth ).
	//--------------------------------------------------------------------------
	if ( lpbi->biBitCount <= 8 ) {
		*lpiNumColors = ( 1 << lpbi->biBitCount );
	} else {
		*lpiNumColors = 0;
	}

	//--------------------------------------------------------------------------
	// If bitmap has a palette ( bitcount ), lock some memory and create
	// a palette from the bitmapinfoheader passed in.
	//--------------------------------------------------------------------------
	if ( *lpiNumColors ) {

		hLogPal = GlobalAlloc( GHND, sizeof( LOGPALETTE ) + sizeof( PALETTEENTRY ) * ( *lpiNumColors ));
		lpPal	= (LPLOGPALETTE) GlobalLock( hLogPal );
		lpPal->palVersion	= 0x300;
		lpPal->palNumEntries = (WORD)*lpiNumColors;

		for ( i = 0; i < *lpiNumColors; i++ ) {
			lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}
		hPal = CreatePalette( lpPal );
		GlobalUnlock( hLogPal );
		GlobalFree( hLogPal );

#if(0)
		StandardFileClass fileout;
		char buff[2];

		fileout.Open( "test.pal", MODE_WRITE_TRUNCATE );
		for ( i = 0; i < *lpiNumColors; i++ ) {
			sprintf( buff, "%d", lpPal->palPalEntry[i].peRed >> 2 );
			fileout.Write(( void *)buff, 2 );
			sprintf( buff, "%d", lpPal->palPalEntry[i].peGreen >> 2 );
			fileout.Write(( void *)buff, 2 );
			sprintf( buff, "%d", lpPal->palPalEntry[i].peBlue >> 2 );
			fileout.Write(( void *)buff, 2 );
		}
		fileout.Close();
#endif

	}
	return( hPal );
}

//*****************************************************************************
// LOADRESOURCEBUTTON -- Find & Load the bitmap from the resource.
//
// INPUT:   HINSTANCE hInstance -- Program's hInstance.
//          LPTSTR lpString -- name of bitmap to find.
//          HPALETTE FAR *lphPalette -- we will return palette in this.
//
// OUTPUT:  HBITMAP -- handle to the bitmap if found.
//
// WARNINGS:
//
// HISTORY: Found this routine on MS Developmemt CD, July 1996.
// 	09/26/1996  MML : Created.
//=============================================================================
HBITMAP LoadResourceButton( HINSTANCE hInstance, LPCTSTR lpString, HPALETTE FAR lphPalette )
{
	HDC 		hdc;
	int 		iNumColors;
	HRSRC 	hRsrc;
	HGLOBAL 	hGlobal;
	HBITMAP 	hBitmapFinal = NULL;
	LPBITMAPINFOHEADER lpbi;

	//--------------------------------------------------------------------------
	// Find the Bitmap in this program's resources.
	//--------------------------------------------------------------------------
	hRsrc = FindResource( hInstance, lpString, RT_BITMAP );
	if ( hRsrc ) {

		//-----------------------------------------------------------------------
		// Load the resource, lock the memory, grab a DC.
		//-----------------------------------------------------------------------
		hGlobal	= LoadResource( hInstance, hRsrc );
		lpbi		= (LPBITMAPINFOHEADER) LockResource( hGlobal );
		hdc		= GetDC( NULL );

		//--------------------------------------------------------------------------
		// Set number of colors ( 2 to the nth ).
		//--------------------------------------------------------------------------
		if ( lpbi->biBitCount <= 8 ) {
			iNumColors = ( 1 << lpbi->biBitCount );
		} else {
			iNumColors = 0;
		}

		//-----------------------------------------------------------------------
		// Get the palette from the resource.
		// Select to the DC and realize it in the System palette.
		//-----------------------------------------------------------------------
//		*lphPalette = CreateDIBPalette((LPBITMAPINFO) lpbi, &iNumColors );
		if ( lphPalette != NULL ) {
			SelectPalette( hdc, lphPalette, FALSE );
			RealizePalette( hdc );
		}

		//-----------------------------------------------------------------------
		// Now create the bitmap itself.
		//-----------------------------------------------------------------------
		hBitmapFinal = CreateDIBitmap(
						hdc,
						(LPBITMAPINFOHEADER)lpbi,
						(LONG)CBM_INIT,
						(LPTSTR)lpbi + lpbi->biSize + iNumColors * sizeof( RGBQUAD ),
						(LPBITMAPINFO)lpbi,
						DIB_RGB_COLORS );

		//-----------------------------------------------------------------------
		// Free DS and memory used.
		//-----------------------------------------------------------------------
		ReleaseDC( NULL, hdc );
		UnlockResource( hGlobal );
		FreeResource( hGlobal );
	}
	return( hBitmapFinal );
}

//*****************************************************************************
// Cant_Find_MessageBox -- Find & Load the bitmap from the resource.
//
// INPUT:   HINSTANCE hInstance -- Program's hInstance.
//          LPTSTR lpString -- name of bitmap to find.
//          HPALETTE FAR *lphPalette -- we will return palette in this.
//
// OUTPUT:  HBITMAP -- handle to the bitmap if found.
//
// WARNINGS:
//
// HISTORY: Found this routine on MS Developmemt CD, July 1996.
// 	09/26/1996  MML : Created.
//  08/27/2003  JFS : Repaired!
//=============================================================================

void Cant_Find_MessageBox ( HINSTANCE hInstance, const char *szPath )
{

	//
	// JFS... Added functionality to make this work in wide characters.
	//
#ifdef LEAN_AND_MEAN
	{
		Locale_GetString( "Autorun:AutorunTitle", szWideBuffer );
		swprintf( szWideBuffer3, szWideBuffer, szProductName );

		Locale_GetString( "Autorun:CantFind", szWideBuffer );
		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szPath, _MAX_PATH, szWideBuffer0, _MAX_PATH );
		swprintf( szWideBuffer2, szWideBuffer, szWideBuffer0 );

		MessageBoxW( NULL,  szWideBuffer2, szWideBuffer3, MB_APPLMODAL | MB_OK );
	}

#else

	std::wstring wideBuffer = TheGameText->fetch("Autorun:AutorunTitle");
	std::wstring wideBuffer2.format( wideBuffer.str(), productName.str() );
	std::wstring wideBuffer3 = TheGameText->fetch("Autorun:CantFind");

	WideCharToMultiByte( CodePage, 0, wideBuffer3.str(), wideBuffer3.getLength()+1, szBuffer3, _MAX_PATH, NULL, NULL );
	WideCharToMultiByte( CodePage, 0, wideBuffer2.str(), wideBuffer2.getLength()+1, szBuffer2, _MAX_PATH, NULL, NULL );


	sprintf( szBuffer1, szBuffer3, szPath );


	if ( strlen( szPath ) < 3 )
	{
		MessageBox( NULL, "The path specified in Cant_Find_MessageBox was blank", "Autorun", MB_APPLMODAL | MB_OK );
		return;
	}
	if ( strlen( szBuffer1 ) < 3 && strlen( szBuffer3 ) < 3 )
	{
		MessageBox( NULL, "***MISSING MESSAGES***... IDS_AUTORUN_TITLE and IDS_CANT_FIND", "Autorun", MB_APPLMODAL | MB_OK );
		return;
	}
	if ( strlen( szBuffer1 ) < 3 )
	{
		MessageBox( NULL, "***MISSING MESSAGE***... IDS_AUTORUN_TITLE", "Autorun", MB_APPLMODAL | MB_OK );
		return;
	}
	if ( strlen( szBuffer3 ) < 3 )
	{
		MessageBox( NULL, "***MISSING MESSAGE***... IDS_CANT_FIND", "Autorun", MB_APPLMODAL | MB_OK );
		return;
	}




	MessageBox( NULL, szBuffer1, szBuffer2, MB_APPLMODAL | MB_OK );
#endif

}


/******************************************************************************
 * Error_Message --
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * WARNINGS:   none
 *
 * HISTORY:
 *   08/14/1998 MML : Created.
 *============================================================================*/

void Error_Message ( HINSTANCE hInstance, const char * title, const char * string, const char *path )
{

#ifndef LEAN_AND_MEAN

	wideBuffer2 = TheGameText->fetch(title);
	wideBuffer3 = TheGameText->fetch(string);

	if ( path && ( path[0] != '\0' ))
	{
		wideBuffer3.format( wideBuffer.str(), path );
	}
	else
	{
		wideBuffer3 = wideBuffer;					// insert not provided
	}

	WideCharToMultiByte( CodePage, 0, wideBuffer2.str(), wideBuffer2.getLength()+1, szBuffer2, _MAX_PATH, NULL, NULL );
	WideCharToMultiByte( CodePage, 0, wideBuffer3.str(), wideBuffer3.getLength()+1, szBuffer3, _MAX_PATH, NULL, NULL );

	MessageBox( NULL, szBuffer3, szBuffer2, MB_APPLMODAL | MB_OK );

#endif

	MessageBox( NULL, "ERROR_UNDEFINED", "ERROR_UNDEFINED", MB_APPLMODAL | MB_OK );


}


/******************************************************************************
/ Launch Class Object
/******************************************************************************/

LaunchObjectClass::LaunchObjectClass ( char *path, char *args )
{
	memset( szPath, '\0', _MAX_PATH );
	memset( szArgs, '\0', _MAX_PATH );

	if( path != NULL && path[0] != '\0' ) {
		strcpy( szPath, path );
	}
	if( args != NULL && args[0] != '\0' ) {
		strcpy( szArgs, args );
	}
}

void LaunchObjectClass::SetPath ( char *path )
{
	if( path != NULL && path[0] != '\0' ) {
		memset( szPath, '\0', _MAX_PATH );
		strcpy( szPath, path );
	}
}

void LaunchObjectClass::SetArgs ( char *args )
{
	if( args != NULL && args[0] != '\0' ) {
		memset( szArgs, '\0', _MAX_PATH );
		strcpy( szArgs, args );
	}
}

unsigned int LaunchObjectClass::Launch ( void )
{
	char 	filepath	[_MAX_PATH];
	char 	dir			[_MAX_DIR];
	char 	ext			[_MAX_EXT];
	char 	drive		[_MAX_DRIVE];
	char 	file		[_MAX_FNAME];
	char 	lpszComLine [ 127 ];

	PROCESS_INFORMATION processinfo;
	STARTUPINFO			startupinfo;
	unsigned int		abc = 0;
	unsigned int		result = 0;

	memset( lpszComLine, '\0', 127 );

	//--------------------------------------------------------------------------
	// Split into parts.
	//--------------------------------------------------------------------------
	_splitpath( szPath, drive, dir, file, ext );

	//--------------------------------------------------------------------------
	// Change current path to the correct dir.
	//
	// The _chdrive function changes the current working drive to the drive
	// specified by drive. The drive parameter uses an integer to specify the
	// new working drive (1=A, 2=B, and so forth). This function changes only
	// the working drive; _chdir changes the working directory.
	//--------------------------------------------------------------------------
	_makepath( filepath, drive, dir, NULL, NULL );
	Path_Remove_Back_Slash( filepath );

	abc = (unsigned)( toupper( filepath[0] ) - 'A' + 1 );
	if ( !_chdrive( abc )) {

		//----------------------------------------------------------------------
		// Returns a value of 0 if successful.
		//----------------------------------------------------------------------
		abc = _chdir( filepath );
	}

#if (RTS_DEBUG)

	int cde = _getdrive();
	_getcwd( szBuffer, _MAX_PATH );

	Msg( __LINE__, TEXT(__FILE__), TEXT("Current Working Dir = %d\\%s." ), cde, szBuffer );
#endif

	//--------------------------------------------------------------------------
	// Try to launch the EXE...
	//--------------------------------------------------------------------------
	_stprintf( lpszComLine, _TEXT( "%s %s" ), szPath, szArgs );

	//==========================================================================
	// Setup the call
	//==========================================================================
	memset( &startupinfo, 0, sizeof( STARTUPINFO ));
	startupinfo.cb = sizeof( STARTUPINFO );

	Msg( __LINE__, TEXT(__FILE__), TEXT("About to launch %s." ), lpszComLine );

	result = CreateProcess(
				szPath,												// address of module name
				lpszComLine, 										// address of command line
				NULL,												// address of process security attributes
				NULL,												// address of thread security attributes
				FALSE,												// new process inherits handles
				FALSE,
				NULL,												// address of new environment block
				NULL,												// address of current directory name
				&startupinfo,										// address of STARTUPINFO
				&processinfo );										// address of PROCESS_INFORMATION

	//--------------------------------------------------------------------------
	// If WinExec returned 0, error occurred.
	//--------------------------------------------------------------------------
	if ( !result ) {

		Msg( __LINE__, TEXT(__FILE__), TEXT("Launch of %s failed." ), lpszComLine );
		_makepath ( filepath, NULL, NULL, file, ext );
		Cant_Find_MessageBox ( Main::hInstance, filepath );
	}
	Msg( __LINE__, TEXT(__FILE__), TEXT("Launch of %s succeeded." ), lpszComLine );

	return( result );
}

void Debug_Date_And_Time_Stamp ( void )
{
	//-------------------------------------------------------------------------
	//	tm_sec	- Seconds after minute (0 - 59)
	//	tm_min	- Minutes after hour (0 - 59)
	//	tm_hour	- Hours after midnight (0 - 23)
	//	tm_mday	- Day of month (1 - 31)
	//	tm_mon	- Month (0 - 11; January = 0)
	//	tm_year	- Year (current year minus 1900)
	//	tm_wday	- Day of week (0 - 6; Sunday = 0)
	//	tm_yday	- Day of year (0 - 365; January 1 = 0)
	//-------------------------------------------------------------------------
	static const char *Month_Strings[ 12 ] = {
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};

	static const char *Week_Day_Strings[ 7 ] = {
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday",
	};

	char		ampm[] = "AM";
    time_t		ltime;
    struct tm *	today;

    /*-------------------------------------------------------------------------
	 *Convert to time structure and adjust for PM if necessary.
	 */
    time( &ltime );
    today = localtime( &ltime );
    if( today->tm_hour > 12 ) {
		strcpy( ampm, "PM" );
		today->tm_hour -= 12;
    }
	if( today->tm_hour == 0 ) {		/* Adjust if midnight hour. */
		today->tm_hour = 12;
	}

	Msg( __LINE__, __FILE__, "%s, %s %d, %d		%d:%d:%d %s",
		Week_Day_Strings[ today->tm_wday ],
		Month_Strings[ today->tm_mon ],
		today->tm_mday,
		today->tm_year + 1900,
		today->tm_hour,
		today->tm_min,
		today->tm_sec,
		ampm );

    /*-------------------------------------------------------------------------
	 * Note how pointer addition is used to skip the first 11
     * characters and printf is used to trim off terminating
     * characters.
     */
//	Msg( __LINE__, __FILE__, "%s %s\n", asctime( today ), ampm );
}


bool Is_On_CD ( const char *volume_name )
{
	char volume_to_match[ MAX_PATH ];

	Reformat_Volume_Name( volume_name, volume_to_match );

	if( _stricmp( szVolumeName, volume_to_match ) == 0 ) {
		return true;
	} else {
		return false;
	}
}

bool Prompt_For_CD ( HWND window_handle, char *volume_name, const char * message1, const char * message2, int *cd_drive )
{
	int drive;

	strcpy( szBuffer, Args->Get_argv( 0 ));
	drive = toupper( szBuffer[0] ) - 'A';
	memset( szBuffer, '\0', MAX_PATH );

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// This is the correct check for a CD Check.
	//
	// MML: Modified on 10/18/2000 so it would check for all available CD drives.
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int result = IDRETRY;

	while( result == IDRETRY ) {

			if ( CD_Volume_Verification( drive, szBuffer, volume_name )) {

				result = IDOK;
				*cd_drive = drive;

			} else {

				CDList.Reset_Index();

				while(( result == IDRETRY ) && ( CDList.Get_Index() < CDList.Get_Number_Of_Drives())) {

					drive = CDList.Get_Next_CD_Drive();

					if ( CD_Volume_Verification( drive, szBuffer, volume_name )) {
						result = IDOK;
						*cd_drive = drive;
					}
				}
			}

			if ( result == IDRETRY ) {
				result = ( Show_Message( window_handle, message1, message2 ));
			}
	}

	if ( result == IDCANCEL ) {
		return( false );
//		return true;
	}

	return( true );
}



int Show_Message ( HWND window_handle, const char * message1, const char * message2 )
{

#ifndef LEAN_AND_MEAN

	UnicodeString	string1;
	UnicodeString	string2;
	WCHAR	szString3[ MAX_PATH ];
	int		result = false;

	string1 = TheGameText->fetch(message1);
	string2 = TheGameText->fetch(message2);

	wcscpy( szString3, string1.str() );
	wcscat( szString3, L" " );
	wcscat( szString3, string2.str() );

	WideCharToMultiByte( CodePage, 0, szString3, _MAX_PATH, szBuffer, _MAX_PATH, NULL, NULL );
	result = MessageBox( window_handle, szBuffer, "Autorun", MB_RETRYCANCEL|MB_APPLMODAL|MB_SETFOREGROUND );

	return( result );

#else

	return ( 0 );

#endif

}


void Reformat_Volume_Name ( const char *volume_name, char *new_volume_name )
{
	char temp_volume_name[ MAX_PATH ];

	strcpy( temp_volume_name, volume_name );

	if( WinVersion.Is_Win95()) {
		temp_volume_name[11] = '\0';
	}

	if( new_volume_name != NULL ) {
		strcpy( new_volume_name, temp_volume_name );
	}
}


