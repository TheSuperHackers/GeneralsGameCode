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

/************************************************************************************************
*   C O N F I D E N T I A L --- W E S T W O O D   A S S O C I A T E S							*
*************************************************************************************************
*
* FILE
*     $Archive: /Renegade Setup/Autorun/autorun.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Maria_l $
*
* VERSION INFO
*     $Modtime: 1/28/02 11:11a $
*     $Revision: 10 $
*
*************************************************************************************************/


#ifndef  AUTORUN_H
#define  AUTORUN_H

#include <tchar.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "TTFont.h"


//--------------------------------------------------------------------
// Externs
//--------------------------------------------------------------------
extern int	Language;

//--------------------------------------------------------------------
// Structs that everyone can use.
//--------------------------------------------------------------------
typedef enum {
	LANG_USA,	//0
	LANG_UK,	//1
	LANG_GER,	//2
	LANG_FRE,	//3
	LANG_DUT,	//4
	LANG_ITA,	//5
	LANG_JAP,	//6
	LANG_SPA,	//7
	LANG_SCA,	//8
	LANG_KOR,	//9
	LANG_CHI,	//10
	LANG_NUM,	
} LanguageType;

#define	IS_LANGUAGE_DBCS(l)	(((l)==LANG_CHI)||((l)==LANG_JAP)||((l)==LANG_KOR))		// [OYO]
#define	IS_CODEPAGE_DBCS(C)	((C==949)||(C==950)||(C==932))						// [OYO]

//----------------------------------------------------------------------------
// DEFINES
//----------------------------------------------------------------------------
#define MAX_COMMAND_LINE_ARGUMENTS	10
#define MAX_ARGUMENT_LENGTH			80

#define MIN(a,b)    				(((a) < (b)) ? (a) : (b))
#define MAX(a,b)    				(((a) > (b)) ? (a) : (b))

#define WM_GO					   	(WM_USER)+1
#define WM_USERSTAT 			   	(WM_USER + 100)

#define EXPLORER_NAME				"EXPLORER.EXE"
#define INSTALL_PATH_KEY			"InstallPath"
#define INTERNET_PATH_KEY			"InternetPath"
#define SETUP_NAME					"Setup.exe"
#define UNINSTALL_NAME	  			"Uninst.exe"

//#define SHELL_FOLDERS_KEY			"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
#define SHELL_UNINSTALL_KEY			"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstallShield_{F3E9C243-122E-4D6B-ACC1-E1FEC02F6CA1}"
#define SHELL_APP_PATHS_KEY			"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths" 
#define PROGRAMS_SUBKEY				"Programs"
#define COMMON_PROGRAMS_SUBKEY		"Common Programs"
#define SOFTWARE_SUBKEY				"Software"
/*
#define WESTWOOD_SUBKEY				"Westwood"
#define SOFTWARE_WESTWOOD_KEY		"Software\\Westwood\\"
#define WESTWOOD_WOLAPI_KEY			"Software\\Westwood\\WOLAPI"
#define WESTWOOD_REGISTER_KEY		"Software\\Westwood\\Register"
*/
#define ELECTRONICARTS_SUBKEY		"Electronic Arts"
#define EAGAMES_SUBKEY				"EA Games"
#define GENERALS_SUBKEY				"Command and Conquer Generals Zero Hour"
#define SOFTWARE_EAGAMES_KEY		"Software\\Electronic Arts\\EA Games\\"
#define EAGAMES_GENERALS_KEY		"Software\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour"
#define EAGAMES_ERGC_KEY			"Software\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour\\ergc"
#define LAUNCHER_FILENAME			"Generals.exe"
#define WORLDBUILDER_FILENAME		"WorldBuilder.exe"
#define PATCHGET_FILENAME			"patchget.dat"

#define UNINSTALL_STRING_SUBKEY		"UninstallString"
#define INSTALLPATH_SUBKEY			"InstallPath"
#define VERSION_SUBKEY				"Version"
#define LANGUAGE_SUBKEY				"Language"
#define MAPPACKVERSION_SUBKEY		"MapPackVersion"
											
#define DDRAW  							"DDRAW.DLL"
#define DSOUND 							"DSOUND.DLL"
#define DDHELP 							"DDHELP.EXE"

#define NORMAL							"Normal" 
#define FOCUSED							"Focused"
#define PRESSED							"Pressed"
/*
#define BUTTON1NORMAL 			"Button1Normal"
#define BUTTON2NORMAL				"Button2Normal"
#define BUTTON3NORMAL				"Button3Normal"
#define BUTTON4NORMAL				"Button4Normal"
#define BUTTON5NORMAL				"Button5Normal"
#define BUTTON6NORMAL				"Button6Normal"
#define BUTTON7NORMAL				"Button7Normal"
#define BUTTON8NORMAL				"Button8Normal"
#define BUTTON9NORMAL				"Button8Normal"
#define BUTTON10NORMAL			"Button8Normal"

#define BUTTON1FOCUSED			"Button1Focused"
#define BUTTON2FOCUSED			"Button2Focused"
#define BUTTON3FOCUSED			"Button3Focused"
#define BUTTON4FOCUSED			"Button4Focused"
#define BUTTON5FOCUSED			"Button5Focused"
#define BUTTON6FOCUSED			"Button6Focused"
#define BUTTON7FOCUSED			"Button7Focused"
#define BUTTON8FOCUSED			"Button8Focused"
#define BUTTON9FOCUSED			"Button8Focused"
#define BUTTON10FOCUSED			"Button8Focused"
*/

#define BUTTON_REG					"BUTTON_REG"
#define BUTTON_SEL					"BUTTON_SEL"

//-------------------------------------------------------------------------
// LaunchObject Class
//-------------------------------------------------------------------------
class LaunchObjectClass
{
	public:
		LaunchObjectClass ( char *path=NULL, char *args=NULL );

		void			SetPath				( char *path );
		void			SetArgs				( char *args );
		unsigned int	Launch				( void );
		bool			Launch_A_Program	( void )			{ return( LaunchSomething ); };
		void			Set_Launch			( bool value )		{ LaunchSomething = value; };

	public:
		char	szPath[ _MAX_PATH ];
		char	szArgs[ _MAX_PATH ];
		bool	LaunchSomething;
};

extern LaunchObjectClass LaunchObject;

//-------------------------------------------------------------------------
//  Main Class
//-------------------------------------------------------------------------
class Main
{
	public:
		static HINSTANCE hInstance;
		static HINSTANCE hPrevInstance;
		static HMODULE hModule;
		static int nCmdShow;
		static int MessageLoop( void );
};

//-------------------------------------------------------------------------
// (Base)Window Class
//
// Note that Window_Proc is a "pure virtual" function making this an
// abstract class.
//-------------------------------------------------------------------------
class Window
{
	protected:
		HWND hWnd;

	public:
		HWND GetHandle( void ) 		{ return hWnd; }
		BOOL Show( int nCmdShow ) 	{ return ShowWindow( hWnd, nCmdShow ); }
		void Update( void ) 		{ UpdateWindow( hWnd ); }
		virtual LRESULT Window_Proc( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam ) = 0;
};

//-------------------------------------------------------------------------
// MainWindow Class
//-------------------------------------------------------------------------
class MainWindow : public Window
{
	protected:
		
		static char szClassName[ 100 ];

	public:

		MainWindow( void );

		static void		Register		  		( void );
		static void	  	Reset_Class_Name		( char *string )
			{
				if ( string != NULL && string[0] != '\0' ) {
					strcpy( szClassName, string );
				}		
			};

		BOOL			Is_Product_Registered	( void );
		void	 		Create_Buttons	  		( HWND hWnd, RECT *dlg_rect );
		unsigned int	Run_Auto_Update			( HWND hWnd, RECT *rect );
		unsigned int	Run_Demo 		  		( HWND hWnd, RECT *rect, int cd_drive );
		BOOL			Run_Explorer	  		( char *, HWND hWnd, RECT *rect );
		unsigned int	Run_Game 		  		( HWND hWnd, RECT *rect );
		unsigned int	Run_WorldBuilder	( HWND hWnd, RECT *rect );
		unsigned int	Run_PatchGet			( HWND hWnd, RECT *rect );
		unsigned int	Run_New_Account	  		( HWND hWnd, RECT *rect );
		unsigned int	Run_Register	  		( HWND hWnd, RECT *rect );
		unsigned int	Run_Setup		  		( HWND hWnd, RECT *rect, int cd_drive );
		unsigned int	Run_Uninstall 	  		( HWND hWnd, RECT *rect );
		unsigned int	Run_OpenFile						(int cd_drive, const char *filename, bool wait = false);
		LRESULT			Window_Proc		  		( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam );
};


#endif

