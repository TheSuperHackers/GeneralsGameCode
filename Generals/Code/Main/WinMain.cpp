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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: WinMain.cpp //////////////////////////////////////////////////////////
// 
// Entry point for game application
//
// Author: Colin Day, April 2001
//
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN  // only bare bones windows stuff wanted
#include <windows.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <eh.h>
#include <ole2.h>
#include <dbt.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "WinMain.h"
#include "Lib/BaseType.h"
#include "Common/CommandLine.h"
#include "Common/CriticalSection.h"
#include "Common/GlobalData.h"
#include "Common/GameEngine.h"
#include "Common/GameSounds.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#include "Common/StackDump.h"
#include "Common/MessageStream.h"
#include "Common/Team.h"
#include "GameClient/ClientInstance.h"
#include "GameClient/InGameUI.h"
#include "GameClient/GameClient.h"
#include "GameLogic/GameLogic.h"  ///< @todo for demo, remove
#include "GameClient/Mouse.h"
#include "GameClient/IMEManager.h"
#include "Win32Device/GameClient/Win32Mouse.h"
#include "Win32Device/Common/Win32GameEngine.h"
#include "Common/version.h"
#include "BuildVersion.h"
#include "GeneratedVersion.h"
#include "resource.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma message("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// GLOBALS ////////////////////////////////////////////////////////////////////
HINSTANCE ApplicationHInstance = NULL;  ///< our application instance
HWND ApplicationHWnd = NULL;  ///< our application window handle
Win32Mouse *TheWin32Mouse= NULL;  ///< for the WndProc() only
DWORD TheMessageTime = 0;	///< For getting the time that a message was posted from Windows.

const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";
const char *gAppPrefix = ""; /// So WB can have a different debug log file name.

#define DEFAULT_XRESOLUTION 800
#define DEFAULT_YRESOLUTION 600

static Bool gInitializing = false;
static Bool gDoPaint = true;
static Bool isWinMainActive = false; 

static HBITMAP gLoadScreenBitmap = NULL;

//#define DEBUG_WINDOWS_MESSAGES

#ifdef DEBUG_WINDOWS_MESSAGES
static const char *messageToString(unsigned int message)
{	
	static char name[32];

	switch (message)
	{
	case WM_NULL: return "WM_NULL";                     
	case WM_CREATE: return  "WM_CREATE";               
	case WM_DESTROY: return  "WM_DESTROY";            
	case WM_MOVE: return  "WM_MOVE";               
	case WM_SIZE: return  "WM_SIZE";                 
	case WM_ACTIVATE: return  "WM_ACTIVATE";             
	case WM_SETFOCUS: return  "WM_SETFOCUS";             
	case WM_KILLFOCUS: return  "WM_KILLFOCUS";            
	case WM_ENABLE: return  "WM_ENABLE";               
	case WM_SETREDRAW: return  "WM_SETREDRAW";            
	case WM_SETTEXT: return  "WM_SETTEXT";              
	case WM_GETTEXT: return  "WM_GETTEXT";              
	case WM_GETTEXTLENGTH: return  "WM_GETTEXTLENGTH";        
	case WM_PAINT: return  "WM_PAINT";                
	case WM_CLOSE: return  "WM_CLOSE";                
	case WM_QUERYENDSESSION: return  "WM_QUERYENDSESSION";      
	case WM_QUIT: return  "WM_QUIT";                 
	case WM_QUERYOPEN: return  "WM_QUERYOPEN";            
	case WM_ERASEBKGND: return  "WM_ERASEBKGND";           
	case WM_SYSCOLORCHANGE: return  "WM_SYSCOLORCHANGE";       
	case WM_ENDSESSION: return  "WM_ENDSESSION";           
	case WM_SHOWWINDOW: return  "WM_SHOWWINDOW";           
	case WM_WININICHANGE: return "WM_WININICHANGE";
	case WM_DEVMODECHANGE: return  "WM_DEVMODECHANGE";        
	case WM_ACTIVATEAPP: return  "WM_ACTIVATEAPP";          
	case WM_FONTCHANGE: return  "WM_FONTCHANGE";           
	case WM_TIMECHANGE: return  "WM_TIMECHANGE";           
	case WM_CANCELMODE: return  "WM_CANCELMODE";           
	case WM_SETCURSOR: return  "WM_SETCURSOR";            
	case WM_MOUSEACTIVATE: return  "WM_MOUSEACTIVATE";        
	case WM_CHILDACTIVATE: return  "WM_CHILDACTIVATE";        
	case WM_QUEUESYNC: return  "WM_QUEUESYNC";            
	case WM_GETMINMAXINFO: return  "WM_GETMINMAXINFO";        
	case WM_PAINTICON: return  "WM_PAINTICON";            
	case WM_ICONERASEBKGND: return  "WM_ICONERASEBKGND";       
	case WM_NEXTDLGCTL: return  "WM_NEXTDLGCTL";           
	case WM_SPOOLERSTATUS: return  "WM_SPOOLERSTATUS";        
	case WM_DRAWITEM: return  "WM_DRAWITEM";             
	case WM_MEASUREITEM: return  "WM_MEASUREITEM";          
	case WM_DELETEITEM: return  "WM_DELETEITEM";           
	case WM_VKEYTOITEM: return  "WM_VKEYTOITEM";           
	case WM_CHARTOITEM: return  "WM_CHARTOITEM";           
	case WM_SETFONT: return  "WM_SETFONT";              
	case WM_GETFONT: return  "WM_GETFONT";              
	case WM_SETHOTKEY: return  "WM_SETHOTKEY";            
	case WM_GETHOTKEY: return  "WM_GETHOTKEY";            
	case WM_QUERYDRAGICON: return  "WM_QUERYDRAGICON";        
	case WM_COMPAREITEM: return  "WM_COMPAREITEM";          
	case WM_COMPACTING: return  "WM_COMPACTING";           
	case WM_COMMNOTIFY: return  "WM_COMMNOTIFY";
	case WM_WINDOWPOSCHANGING: return  "WM_WINDOWPOSCHANGING";    
	case WM_WINDOWPOSCHANGED: return  "WM_WINDOWPOSCHANGED";     
	case WM_POWER: return  "WM_POWER";                
	case WM_COPYDATA: return  "WM_COPYDATA";             
	case WM_CANCELJOURNAL: return  "WM_CANCELJOURNAL";        
	case WM_NOTIFY: return  "WM_NOTIFY";               
	case WM_INPUTLANGCHANGEREQUEST: return  "WM_INPUTLANGCHANGEREQUES";
	case WM_INPUTLANGCHANGE: return  "WM_INPUTLANGCHANGE";      
	case WM_TCARD: return  "WM_TCARD";                
	case WM_HELP: return  "WM_HELP";                 
	case WM_USERCHANGED: return  "WM_USERCHANGED";          
	case WM_NOTIFYFORMAT: return  "WM_NOTIFYFORMAT";         
	case WM_CONTEXTMENU: return  "WM_CONTEXTMENU";          
	case WM_STYLECHANGING: return  "WM_STYLECHANGING";        
	case WM_STYLECHANGED: return  "WM_STYLECHANGED";         
	case WM_DISPLAYCHANGE: return  "WM_DISPLAYCHANGE";        
	case WM_GETICON: return  "WM_GETICON";              
	case WM_SETICON: return  "WM_SETICON";              
	case WM_NCCREATE: return  "WM_NCCREATE";             
	case WM_NCDESTROY: return  "WM_NCDESTROY";            
	case WM_NCCALCSIZE: return  "WM_NCCALCSIZE";           
	case WM_NCHITTEST: return  "WM_NCHITTEST";            
	case WM_NCPAINT: return  "WM_NCPAINT";              
	case WM_NCACTIVATE: return  "WM_NCACTIVATE";           
	case WM_GETDLGCODE: return  "WM_GETDLGCODE";           
	case WM_SYNCPAINT: return  "WM_SYNCPAINT";            
	case WM_NCMOUSEMOVE: return  "WM_NCMOUSEMOVE";          
	case WM_NCLBUTTONDOWN: return  "WM_NCLBUTTONDOWN";        
	case WM_NCLBUTTONUP: return  "WM_NCLBUTTONUP";          
	case WM_NCLBUTTONDBLCLK: return  "WM_NCLBUTTONDBLCLK";      
	case WM_NCRBUTTONDOWN: return  "WM_NCRBUTTONDOWN";        
	case WM_NCRBUTTONUP: return  "WM_NCRBUTTONUP";          
	case WM_NCRBUTTONDBLCLK: return  "WM_NCRBUTTONDBLCLK";      
	case WM_NCMBUTTONDOWN: return  "WM_NCMBUTTONDOWN";        
	case WM_NCMBUTTONUP: return  "WM_NCMBUTTONUP";          
	case WM_NCMBUTTONDBLCLK: return  "WM_NCMBUTTONDBLCLK";      
	case WM_KEYDOWN: return  "WM_KEYDOWN";              
	case WM_KEYUP: return  "WM_KEYUP";                
	case WM_CHAR: return  "WM_CHAR";                 
	case WM_DEADCHAR: return  "WM_DEADCHAR";             
	case WM_SYSKEYDOWN: return  "WM_SYSKEYDOWN";           
	case WM_SYSKEYUP: return  "WM_SYSKEYUP";             
	case WM_SYSCHAR: return  "WM_SYSCHAR";              
	case WM_SYSDEADCHAR: return  "WM_SYSDEADCHAR";          
	case WM_KEYLAST: return  "WM_KEYLAST";              
	case WM_IME_STARTCOMPOSITION: return  "WM_IME_STARTCOMPOSITION"; 
	case WM_IME_ENDCOMPOSITION: return  "WM_IME_ENDCOMPOSITION";   
	case WM_IME_COMPOSITION: return  "WM_IME_COMPOSITION";      
	case WM_INITDIALOG: return  "WM_INITDIALOG";           
	case WM_COMMAND: return  "WM_COMMAND";              
	case WM_SYSCOMMAND: return  "WM_SYSCOMMAND";           
	case WM_TIMER: return  "WM_TIMER";                
	case WM_HSCROLL: return  "WM_HSCROLL";              
	case WM_VSCROLL: return  "WM_VSCROLL";              
	case WM_INITMENU: return  "WM_INITMENU";             
	case WM_INITMENUPOPUP: return  "WM_INITMENUPOPUP";        
	case WM_MENUSELECT: return  "WM_MENUSELECT";           
	case WM_MENUCHAR: return  "WM_MENUCHAR";             
	case WM_ENTERIDLE: return  "WM_ENTERIDLE";            
	case WM_CTLCOLORMSGBOX: return  "WM_CTLCOLORMSGBOX";       
	case WM_CTLCOLOREDIT: return  "WM_CTLCOLOREDIT";         
	case WM_CTLCOLORLISTBOX: return  "WM_CTLCOLORLISTBOX";      
	case WM_CTLCOLORBTN: return  "WM_CTLCOLORBTN";          
	case WM_CTLCOLORDLG: return  "WM_CTLCOLORDLG";          
	case WM_CTLCOLORSCROLLBAR: return  "WM_CTLCOLORSCROLLBAR";    
	case WM_CTLCOLORSTATIC: return  "WM_CTLCOLORSTATIC";       
	case WM_MOUSEMOVE: return  "WM_MOUSEMOVE";            
	case WM_LBUTTONDOWN: return  "WM_LBUTTONDOWN";          
	case WM_LBUTTONUP: return  "WM_LBUTTONUP";            
	case WM_LBUTTONDBLCLK: return  "WM_LBUTTONDBLCLK";        
	case WM_RBUTTONDOWN: return  "WM_RBUTTONDOWN";          
	case WM_RBUTTONUP: return  "WM_RBUTTONUP";            
	case WM_RBUTTONDBLCLK: return  "WM_RBUTTONDBLCLK";        
	case WM_MBUTTONDOWN: return  "WM_MBUTTONDOWN";          
	case WM_MBUTTONUP: return  "WM_MBUTTONUP";            
	case WM_MBUTTONDBLCLK: return  "WM_MBUTTONDBLCLK";        
//	case WM_MOUSEWHEEL: return  "WM_MOUSEWHEEL";           
	case WM_PARENTNOTIFY: return  "WM_PARENTNOTIFY";         
	case WM_ENTERMENULOOP: return  "WM_ENTERMENULOOP";        
	case WM_EXITMENULOOP: return  "WM_EXITMENULOOP";         
	case WM_NEXTMENU: return  "WM_NEXTMENU";             
	case WM_SIZING: return  "WM_SIZING";               
	case WM_CAPTURECHANGED: return  "WM_CAPTURECHANGED";       
	case WM_MOVING: return  "WM_MOVING";               
	case WM_POWERBROADCAST: return  "WM_POWERBROADCAST";
	case WM_DEVICECHANGE: return  "WM_DEVICECHANGE";         
	case WM_MDICREATE: return  "WM_MDICREATE";            
	case WM_MDIDESTROY: return  "WM_MDIDESTROY";           
	case WM_MDIACTIVATE: return  "WM_MDIACTIVATE";          
	case WM_MDIRESTORE: return  "WM_MDIRESTORE";           
	case WM_MDINEXT: return  "WM_MDINEXT";              
	case WM_MDIMAXIMIZE: return  "WM_MDIMAXIMIZE";          
	case WM_MDITILE: return  "WM_MDITILE";              
	case WM_MDICASCADE: return  "WM_MDICASCADE";           
	case WM_MDIICONARRANGE: return  "WM_MDIICONARRANGE";       
	case WM_MDIGETACTIVE: return  "WM_MDIGETACTIVE";         
	case WM_MDISETMENU: return  "WM_MDISETMENU";           
	case WM_ENTERSIZEMOVE: return  "WM_ENTERSIZEMOVE";        
	case WM_EXITSIZEMOVE: return  "WM_EXITSIZEMOVE";         
	case WM_DROPFILES: return  "WM_DROPFILES";            
	case WM_MDIREFRESHMENU: return  "WM_MDIREFRESHMENU";       
	case WM_IME_SETCONTEXT: return  "WM_IME_SETCONTEXT";       
	case WM_IME_NOTIFY: return  "WM_IME_NOTIFY";           
	case WM_IME_CONTROL: return  "WM_IME_CONTROL";          
	case WM_IME_COMPOSITIONFULL: return  "WM_IME_COMPOSITIONFULL";  
	case WM_IME_SELECT: return  "WM_IME_SELECT";           
	case WM_IME_CHAR: return  "WM_IME_CHAR";             
	case WM_IME_KEYDOWN: return  "WM_IME_KEYDOWN";          
	case WM_IME_KEYUP: return  "WM_IME_KEYUP";            
//	case WM_MOUSEHOVER: return  "WM_MOUSEHOVER";           
//	case WM_MOUSELEAVE: return  "WM_MOUSELEAVE";           
	case WM_CUT: return  "WM_CUT";                  
	case WM_COPY: return  "WM_COPY";                 
	case WM_PASTE: return  "WM_PASTE";                
	case WM_CLEAR: return  "WM_CLEAR";                
	case WM_UNDO: return  "WM_UNDO";                 
	case WM_RENDERFORMAT: return  "WM_RENDERFORMAT";         
	case WM_RENDERALLFORMATS: return  "WM_RENDERALLFORMATS";     
	case WM_DESTROYCLIPBOARD: return  "WM_DESTROYCLIPBOARD";     
	case WM_DRAWCLIPBOARD: return  "WM_DRAWCLIPBOARD";        
	case WM_PAINTCLIPBOARD: return  "WM_PAINTCLIPBOARD";       
	case WM_VSCROLLCLIPBOARD: return  "WM_VSCROLLCLIPBOARD";     
	case WM_SIZECLIPBOARD: return  "WM_SIZECLIPBOARD";        
	case WM_ASKCBFORMATNAME: return  "WM_ASKCBFORMATNAME";      
	case WM_CHANGECBCHAIN: return  "WM_CHANGECBCHAIN";        
	case WM_HSCROLLCLIPBOARD: return  "WM_HSCROLLCLIPBOARD";     
	case WM_QUERYNEWPALETTE: return  "WM_QUERYNEWPALETTE";      
	case WM_PALETTEISCHANGING: return  "WM_PALETTEISCHANGING";    
	case WM_PALETTECHANGED: return  "WM_PALETTECHANGED";       
	case WM_HOTKEY: return  "WM_HOTKEY";               
	case WM_PRINT: return  "WM_PRINT";                
	case WM_PRINTCLIENT: return  "WM_PRINTCLIENT";          
	case WM_HANDHELDFIRST: return  "WM_HANDHELDFIRST";        
	case WM_HANDHELDLAST: return  "WM_HANDHELDLAST";         
	case WM_AFXFIRST: return  "WM_AFXFIRST";             
	case WM_AFXLAST: return  "WM_AFXLAST";              
	case WM_PENWINFIRST: return  "WM_PENWINFIRST";          
	case WM_PENWINLAST: return  "WM_PENWINLAST";
	default: return "WM_UNKNOWN";
	};
}
#endif

// WndProc ====================================================================
/** Window Procedure */
//=============================================================================
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, 
													WPARAM wParam, LPARAM lParam )
{

	try
	{
		// First let the IME manager do it's stuff. 
		if ( TheIMEManager )
		{
			if ( TheIMEManager->serviceIMEMessage( hWnd, message, wParam, lParam ) )
			{
				// The manager intercepted an IME message so return the result
				return TheIMEManager->result();
			}
		}

#ifdef	DEBUG_WINDOWS_MESSAGES
		static msgCount=0;
		char testString[256];
		sprintf(testString,"\n%d: %s (%X,%X)", msgCount++,messageToString(message), wParam, lParam); 
		OutputDebugString(testString);
#endif

		// handle all window messages
		switch( message ) 
		{
			//-------------------------------------------------------------------------
			case WM_NCHITTEST:
			// Prevent the user from selecting the menu in fullscreen mode
            if( !TheGlobalData->m_windowed )
                return HTCLIENT;
            break;

			//-------------------------------------------------------------------------
			case WM_POWERBROADCAST:
            switch( wParam )
            {
                #ifndef PBT_APMQUERYSUSPEND
                    #define PBT_APMQUERYSUSPEND 0x0000
                #endif
                case PBT_APMQUERYSUSPEND:
                    // At this point, the app should save any data for open
                    // network connections, files, etc., and prepare to go into
                    // a suspended mode.
                    return TRUE;

                #ifndef PBT_APMRESUMESUSPEND
                    #define PBT_APMRESUMESUSPEND 0x0007
                #endif
                case PBT_APMRESUMESUSPEND:
                    // At this point, the app should recover any data, network
                    // connections, files, etc., and resume running from when
                    // the app was suspended.
                    return TRUE;
            }
            break;
			//-------------------------------------------------------------------------
			case WM_SYSCOMMAND:
            // Prevent moving/sizing and power loss in fullscreen mode
            switch( wParam )
            {
                case SC_KEYMENU:
                    // TheSuperHackers @bugfix Mauller 10/05/2025 Always handle this command to prevent halting the game when left Alt is pressed.
                    return 1;
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_MONITORPOWER:
                    if( !TheGlobalData->m_windowed )
                        return 1;
                    break;
            }
            break;

			// ------------------------------------------------------------------------
			case WM_CLOSE:
			TheGameEngine->checkAbnormalQuitting();
			TheGameEngine->reset();
			TheGameEngine->setQuitting(TRUE);
			_exit(EXIT_SUCCESS);
			return 0;

			// ------------------------------------------------------------------------
			case WM_SETFOCUS:
			{

				//
				// reset the state of our keyboard cause we haven't been paying
				// attention to the keys while focus was away
				//
				if( TheKeyboard )
					TheKeyboard->resetKeys();

				if (TheWin32Mouse)
					TheWin32Mouse->lostFocus(FALSE);

				break;

			}  // end set focus

			//-------------------------------------------------------------------------
			case WM_SIZE:
				// When W3D initializes, it resizes the window.  So stop repainting.
				if (!gInitializing) 
					gDoPaint = false;
				break;

			//-------------------------------------------------------------------------
			case WM_KILLFOCUS:
			{
				if (TheKeyboard )
					TheKeyboard->resetKeys();
				if (TheWin32Mouse)
					TheWin32Mouse->lostFocus(TRUE);

				break;
			}

			//-------------------------------------------------------------------------
			case WM_ACTIVATEAPP:
			{
				if ((bool) wParam != isWinMainActive)
				{
					// TheSuperHackers @bugfix xezon 11/05/2025 This event originally called DX8Wrapper::Reset_Device,
					// intended to clear resources on a lost device in fullscreen, but effectively also in
					// windowed mode, if the DXMaximizedWindowedMode shim was applied in newer versions of Windows,
					// which lead to unfortunate application crashing. Resetting the device on WM_ACTIVATEAPP instead
					// of TestCooperativeLevel() == D3DERR_DEVICENOTRESET is not a requirement. There are other code
					// paths that take care of that.

					isWinMainActive = (BOOL) wParam;
					
					if (TheGameEngine)
						TheGameEngine->setIsActive(isWinMainActive);

					if (isWinMainActive)
					{	//restore mouse cursor to our custom version.
						if (TheWin32Mouse)
							TheWin32Mouse->setCursor(TheWin32Mouse->getMouseCursor());
					}
				}
				return 0;
			}
			//-------------------------------------------------------------------------
			case WM_ACTIVATE:
			{
				Int active = LOWORD( wParam );

				//
				// when window is becoming deactivated we must release mouse cursor
				// locks on our region, otherwise set the mouse limit region again
				// which will clip the cursor to our window
				//
				if( active == WA_INACTIVE )
				{

					ClipCursor( NULL );
					if (TheAudio)
						TheAudio->loseFocus();
				}  // end if
				else
				{
					if( TheMouse )
						TheMouse->setMouseLimits();

					if (TheAudio)
						TheAudio->regainFocus();

				}  // end else
				break;

			}  // end case activate

			//-------------------------------------------------------------------------
			case WM_KEYDOWN:
			{
				Int key = (Int)wParam;

				switch( key )
				{

					//---------------------------------------------------------------------
					case VK_ESCAPE:
					{

						PostQuitMessage( 0 );
						break;

					}  // end VK_ESCAPE


				}  // end switch

				return 0;

			}  // end WM_KEYDOWN

			//-------------------------------------------------------------------------
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:

			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MBUTTONDBLCLK:

			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDBLCLK:
			{

				if( TheWin32Mouse )
					TheWin32Mouse->addWin32Event( message, wParam, lParam, TheMessageTime );

				return 0;

			}  // end WM_LBUTTONDOWN

			//-------------------------------------------------------------------------
			case 0x020A: // WM_MOUSEWHEEL
			{
				long x = (long) LOWORD(lParam);
				long y = (long) HIWORD(lParam);
				RECT rect;

				// ignore when outside of client area
				GetWindowRect( ApplicationHWnd, &rect );
				if( x < rect.left || x > rect.right || y < rect.top || y > rect.bottom )
					return 0;

				if( TheWin32Mouse )
					TheWin32Mouse->addWin32Event( message, wParam, lParam, TheMessageTime );

				return 0;

			}  // end WM_MOUSEWHEEL


			//-------------------------------------------------------------------------
			case WM_MOUSEMOVE:
			{
				Int x = (Int)LOWORD( lParam );
				Int y = (Int)HIWORD( lParam );
				RECT rect;
//				Int keys = wParam;

				// ignore when outside of client area
				GetClientRect( ApplicationHWnd, &rect );
				if( x < rect.left || x > rect.right || y < rect.top || y > rect.bottom )
					return 0;

				if( TheWin32Mouse )
					TheWin32Mouse->addWin32Event( message, wParam, lParam, TheMessageTime );

				return 0;

			}  // end WM_MOUSEMOVE

			//-------------------------------------------------------------------------
			case WM_SETCURSOR:
			{
				if (TheWin32Mouse && (HWND)wParam == ApplicationHWnd)
					TheWin32Mouse->setCursor(TheWin32Mouse->getMouseCursor());
				return TRUE;	//tell Windows not to reset mouse cursor image to default.
			}

			case WM_PAINT:
			{
				if (gDoPaint) {
					PAINTSTRUCT paint;
					HDC dc = ::BeginPaint(hWnd, &paint);
#if 0  
					::SetTextColor(dc, RGB(255,255,255));
					::SetBkColor(dc, RGB(0,0,0));
					::TextOut(dc, 30, 30, "Loading Command & Conquer Generals...", 37);
#endif
					if (gLoadScreenBitmap!=NULL) {
						Int savContext = ::SaveDC(dc);
						HDC tmpDC = ::CreateCompatibleDC(dc);
						HBITMAP savBitmap = (HBITMAP)::SelectObject(tmpDC, gLoadScreenBitmap);
						::BitBlt(dc, 0, 0, DEFAULT_XRESOLUTION, DEFAULT_YRESOLUTION, tmpDC, 0, 0, SRCCOPY);
						::SelectObject(tmpDC, savBitmap);
						::DeleteDC(tmpDC);
						::RestoreDC(dc, savContext);
					}
					::EndPaint(hWnd, &paint);
					return TRUE;
				}
				break;
			}

			case WM_ERASEBKGND:
			{
				if (!gDoPaint) 
					return TRUE;	//we don't need to erase the background because we always draw entire window.
				break;
			}

// Well, it was a nice idea, but we don't get a message for an ejection. 
// (Really unforunate, actually.) I'm leaving this in in-case some one wants
// to trap a different device change (for instance, removal of a mouse) - jkmcd
#if 0
			case WM_DEVICECHANGE: 
			{
				if (((UINT) wParam) == DBT_DEVICEREMOVEPENDING) 
				{
					DEV_BROADCAST_HDR *hdr = (DEV_BROADCAST_HDR*) lParam;
					if (!hdr) {
						break;
					}

					if (hdr->dbch_devicetype != DBT_DEVTYP_VOLUME)  {
						break;
					}

					// Lets discuss how Windows is a flaming pile of poo. I'm now casting the header
					// directly into the structure, because its the one I want, and this is just how
					// its done. I hate Windows. - jkmcd
					DEV_BROADCAST_VOLUME *vol = (DEV_BROADCAST_VOLUME*) (hdr);

					// @todo - Yikes. This could cause us all kinds of pain. I don't really want 
					// to even think about the stink this could cause us.
					TheFileSystem->unloadMusicFilesFromCD(vol->dbcv_unitmask);
					return TRUE;
				}
				break;
			}
#endif
		}  // end switch

	}
	catch (...)
	{
		RELEASE_CRASH(("Uncaught exception in Main::WndProc... probably should not happen\n"));
		// no rethrow
	}

//In full-screen mode, only pass these messages onto the default windows handler.
//Appears to fix issues with dual monitor systems but doesn't seem safe?
///@todo: Look into proper support for dual monitor systems.
/*	if (!TheGlobalData->m_windowed)
	switch (message)
	{
		case WM_PAINT:
		case WM_NCCREATE:
		case WM_NCDESTROY:
		case WM_NCCALCSIZE:
		case WM_NCPAINT:
				return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return 0;*/

	return DefWindowProc( hWnd, message, wParam, lParam );

}  // end WndProc

// initializeAppWindows =======================================================
/** Register windows class and create application windows. */
//=============================================================================
static Bool initializeAppWindows( HINSTANCE hInstance, Int nCmdShow, Bool runWindowed )
{
	DWORD windowStyle;
	Int startWidth = DEFAULT_XRESOLUTION,
			startHeight = DEFAULT_YRESOLUTION;

	// register the window class

  WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, WndProc, 0, 0, hInstance,
                       LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ApplicationIcon)),
                       NULL/*LoadCursor(NULL, IDC_ARROW)*/, 
                       (HBRUSH)GetStockObject(BLACK_BRUSH), NULL,
	                     TEXT("Game Window") };
  RegisterClass( &wndClass );

   // Create our main window
	windowStyle =  WS_POPUP|WS_VISIBLE;
	if (runWindowed) 
		windowStyle |= WS_DLGFRAME | WS_CAPTION | WS_SYSMENU;
	else
		windowStyle |= WS_EX_TOPMOST | WS_SYSMENU;

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = startWidth;
	rect.bottom = startHeight;
	AdjustWindowRect (&rect, windowStyle, FALSE);
	if (runWindowed) {
		// Makes the normal debug 800x600 window center in the screen.
		startWidth = DEFAULT_XRESOLUTION;
		startHeight= DEFAULT_YRESOLUTION;
	}

	gInitializing = true;

  HWND hWnd = CreateWindow( TEXT("Game Window"),
                            TEXT("Command and Conquer Generals"),
                            windowStyle, 
														(GetSystemMetrics( SM_CXSCREEN ) / 2) - (startWidth / 2), // original position X
														(GetSystemMetrics( SM_CYSCREEN ) / 2) - (startHeight / 2),// original position Y
														// Lorenzen nudged the window higher
														// so the constantdebug report would 
														// not get obliterated by assert windows, thank you.
														//(GetSystemMetrics( SM_CXSCREEN ) / 2) - (startWidth / 2),   //this works with any screen res
														//(GetSystemMetrics( SM_CYSCREEN ) / 25) - (startHeight / 25),//this works with any screen res
														rect.right-rect.left,
														rect.bottom-rect.top,
														0L, 
														0L, 
														hInstance, 
														0L );


	if (!runWindowed)
	{	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0,SWP_NOSIZE |SWP_NOMOVE);
	}
	else 
		SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0,SWP_NOSIZE |SWP_NOMOVE);

	SetFocus(hWnd);

	SetForegroundWindow(hWnd);
	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );

	// save our application window handle for future use
	ApplicationHWnd = hWnd;
	gInitializing = false;
	if (!runWindowed) {
		gDoPaint = false;
	}

	return true;  // success

}  // end initializeAppWindows

// Necessary to allow memory managers and such to have useful critical sections
static CriticalSection critSec1, critSec2, critSec3, critSec4, critSec5;

// WinMain ====================================================================
/** Application entry point */
//=============================================================================
Int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine, Int nCmdShow )
{
	try {

		_set_se_translator( DumpExceptionInfo ); // Hook that allows stack trace.
		//
		// there is something about checkin in and out the .dsp and .dsw files 
		// that blows the working directory information away on each of the 
		// developers machines so we're going to hack it for a while and set our
		// working directory to the directory with the .exe since that's not the
		// default in a DevStudio project
		//

		TheAsciiStringCriticalSection = &critSec1;
		TheUnicodeStringCriticalSection = &critSec2;
		TheDmaCriticalSection = &critSec3;
		TheMemoryPoolCriticalSection = &critSec4;
		TheDebugLogCriticalSection = &critSec5;

		/// @todo remove this force set of working directory later
		Char buffer[ _MAX_PATH ];
		GetModuleFileName( NULL, buffer, sizeof( buffer ) );
		Char *pEnd = buffer + strlen( buffer );
		while( pEnd != buffer ) 
		{
			if( *pEnd == '\\' ) 
			{
				*pEnd = 0;
				break;
			}
			pEnd--;
		}
		::SetCurrentDirectory(buffer);

		CommandLine::parseCommandLineForStartup();

		#ifdef RTS_DEBUG
			// Turn on Memory heap tracking
			int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
			tmpFlag |= (_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
			tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;
			_CrtSetDbgFlag( tmpFlag );
		#endif



		// install debug callbacks
	//	WWDebug_Install_Message_Handler(WWDebug_Message_Callback);
	//	WWDebug_Install_Assert_Handler(WWAssert_Callback);

 		// [SKB: Jun 24 2003 @ 1:50pm] :
		// Force to be loaded from a file, not a resource so same exe can be used in germany and retail.
 		gLoadScreenBitmap = (HBITMAP)LoadImage(hInstance, "Install_Final.bmp",	IMAGE_BITMAP, 0, 0, LR_SHARED|LR_LOADFROMFILE);

		// register windows class and create application window
		if(!TheGlobalData->m_headless && initializeAppWindows(hInstance, nCmdShow, TheGlobalData->m_windowed) == false)
			return 0;
		
		// save our application instance for future use
		ApplicationHInstance = hInstance;

		if (gLoadScreenBitmap!=NULL) {
			::DeleteObject(gLoadScreenBitmap);
			gLoadScreenBitmap = NULL;
		}


		// BGC - initialize COM
	//	OleInitialize(NULL);

		// start the log
		DEBUG_INIT(DEBUG_FLAGS_DEFAULT);
		initMemoryManager();

 
		// Set up version info
		TheVersion = NEW Version;
		TheVersion->setVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNUM, VERSION_LOCALBUILDNUM,
			AsciiString(VERSION_BUILDUSER), AsciiString(VERSION_BUILDLOC),
			AsciiString(__TIME__), AsciiString(__DATE__));

		// TheSuperHackers @refactor The instance mutex now lives in its own class.

		if (!rts::ClientInstance::initialize())
		{
			HWND ccwindow = FindWindow(rts::ClientInstance::getFirstInstanceName(), NULL);
			if (ccwindow)
			{
				SetForegroundWindow(ccwindow);
				ShowWindow(ccwindow, SW_RESTORE);
			}

			DEBUG_LOG(("Generals is already running...Bail!\n"));
			delete TheVersion;
			TheVersion = NULL;
			shutdownMemoryManager();
			DEBUG_SHUTDOWN();
			return 0;
		}
		DEBUG_LOG(("Create Generals Mutex okay.\n"));
		DEBUG_LOG(("CRC message is %d\n", GameMessage::MSG_LOGIC_CRC));

		// run the game main loop
		GameMain(0, NULL);

		delete TheVersion;
		TheVersion = NULL;

	#ifdef MEMORYPOOL_DEBUG
		TheMemoryPoolFactory->debugMemoryReport(REPORT_POOLINFO | REPORT_POOL_OVERFLOW | REPORT_SIMPLE_LEAKS, 0, 0);
	#endif
	#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
		TheMemoryPoolFactory->memoryPoolUsageReport("AAAMemStats");
	#endif

		// close the log
		shutdownMemoryManager();
		DEBUG_SHUTDOWN();

		// BGC - shut down COM
	//	OleUninitialize();
	}	
	catch (...) 
	{ 
	
	}

	TheAsciiStringCriticalSection = NULL;
	TheUnicodeStringCriticalSection = NULL;
	TheDmaCriticalSection = NULL;
	TheMemoryPoolCriticalSection = NULL;

	return 0;

}  // end WinMain

// CreateGameEngine ===========================================================
/** Create the Win32 game engine we're going to use */
//=============================================================================
GameEngine *CreateGameEngine( void )
{
	Win32GameEngine *engine;

	engine = NEW Win32GameEngine;
	//game engine may not have existed when app got focus so make sure it
	//knows about current focus state.
	engine->setIsActive(isWinMainActive);

	return engine;

}  // end CreateGameEngine
