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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: GameWindow.h /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  GameWindow.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Header for game windowing system for generic windows and GUI
//						 elements
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "Common/GameMemory.h"
#include "GameClient/Image.h"
#include "GameClient/DisplayString.h"
#include "GameClient/WinInstanceData.h"
#include "GameClient/Color.h"

///////////////////////////////////////////////////////////////////////////////
// FORWARD REFERENCES /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GameWindow;
class WindowLayout;
class GameFont;
class TransitionWindow;
struct GameWindowEditData;

///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum { WIN_COLOR_UNDEFINED = GAME_COLOR_UNDEFINED };

// WindowMsgData --------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef UnsignedInt WindowMsgData;

//-----------------------------------------------------------------------------
enum WindowMsgHandledType CPP_11(: Int) { MSG_IGNORED, MSG_HANDLED };

// callback types -------------------------------------------------------------
typedef void (*GameWinMsgBoxFunc)( void ); //used for the Message box callbacks.
typedef void (*GameWinDrawFunc)( GameWindow *,
																 WinInstanceData * );
typedef void (*GameWinTooltipFunc)( GameWindow *,
																		WinInstanceData *,
																		UnsignedInt );
typedef WindowMsgHandledType (*GameWinInputFunc)( GameWindow *,
																	UnsignedInt,
																	WindowMsgData,
																	WindowMsgData );
typedef WindowMsgHandledType (*GameWinSystemFunc)( GameWindow *,
																	 UnsignedInt,
																	 WindowMsgData,
																	 WindowMsgData );

enum
{

	WIN_MAX_WINDOWS			= 576,
	CURSOR_MOVE_TOL_SQ	= 4,
	TOOLTIP_DELAY				= 10,
	WIN_TOOLTIP_LEN			= 64,					// max length of tooltip text

};

// macros for easier conversion -----------------------------------------------
#define SHORTTOLONG(a, b) ((UnsignedShort)(a) | ((UnsignedShort)(b) << 16))
#define LOLONGTOSHORT(a)  ((a) & 0x0000FFFF)
#define HILONGTOSHORT(b)  (((b) & 0xFFFF0000) >> 16)

// Game window messages -------------------------------------------------------
//-----------------------------------------------------------------------------
enum GameWindowMessage CPP_11(: Int)
{

	GWM_NONE = 0,

	GWM_CREATE,									GWM_DESTROY,
	GWM_ACTIVATE,								GWM_ENABLE,
	GWM_LEFT_DOWN,							GWM_LEFT_UP,
	GWM_LEFT_DOUBLE_CLICK,			GWM_LEFT_DRAG,
	GWM_MIDDLE_DOWN,						GWM_MIDDLE_UP,
	GWM_MIDDLE_DOUBLE_CLICK,		GWM_MIDDLE_DRAG,
	GWM_RIGHT_DOWN,							GWM_RIGHT_UP,
	GWM_RIGHT_DOUBLE_CLICK,			GWM_RIGHT_DRAG,
	GWM_MOUSE_ENTERING,					GWM_MOUSE_LEAVING,
	GWM_WHEEL_UP,								GWM_WHEEL_DOWN,
	GWM_CHAR,										GWM_SCRIPT_CREATE,
	// note that GWM_MOUSE_POS is only actually propogated to windows if the static
	// sendMousePosMessages is set to true in the window manager file.  See the
	// comment on the static declaration for addtional info
	GWM_INPUT_FOCUS,						GWM_MOUSE_POS,
	GWM_IME_CHAR,								GWM_IME_STRING

};

// WinInputReturnCode ------------------------------------------------------
/** These return codes are returned when after processing events through
	* the window system */
//-----------------------------------------------------------------------------
enum WinInputReturnCode CPP_11(: Int)
{
	WIN_INPUT_NOT_USED = 0,
	WIN_INPUT_USED,
};


#define GWM_USER 32768

// Window status flags --------------------------------------------------------
//-----------------------------------------------------------------------------
enum
{

	// when you edit this, remember to edit WindowStatusNames[]
	WIN_STATUS_NONE								= 0x00000000,		// No status bits set at all
	WIN_STATUS_ACTIVE							= 0x00000001,		// At the top of the window list
	WIN_STATUS_TOGGLE							= 0x00000002,		// If set, click to toggle
	WIN_STATUS_DRAGABLE						= 0x00000004,		// Window can be dragged
	WIN_STATUS_ENABLED						= 0x00000008,		// Window can receive input
	WIN_STATUS_HIDDEN 						= 0x00000010,		// Window is hidden, no input
	WIN_STATUS_ABOVE    					= 0x00000020,		// Window is always above others
	WIN_STATUS_BELOW    					= 0x00000040,		// Window is always below others
	WIN_STATUS_IMAGE							= 0x00000080,	  // Window is drawn with images
	WIN_STATUS_TAB_STOP						= 0x00000100,	  // Window is a tab stop
	WIN_STATUS_NO_INPUT						= 0x00000200,	  // Window does not take input
	WIN_STATUS_NO_FOCUS						= 0x00000400,	  // Window does not take focus
	WIN_STATUS_DESTROYED					= 0x00000800,	  // Window has been destroyed
	WIN_STATUS_BORDER							= 0x00001000,	  // Window will be drawn with Borders & Corners
	WIN_STATUS_SMOOTH_TEXT				= 0x00002000,	  // Window text will be drawn with smoothing
	WIN_STATUS_ONE_LINE						= 0x00004000,	  // Window text will be drawn on only one line
	WIN_STATUS_NO_FLUSH						= 0x00008000,	  // Window images will not be unloaded when window is hidden
	WIN_STATUS_SEE_THRU						= 0x00010000,   // Will not draw, but it NOT hidden ... does not apply to children
	WIN_STATUS_RIGHT_CLICK				= 0x00020000,		// Window pays attention to right clicks
	WIN_STATUS_WRAP_CENTERED			= 0x00040000,		// Text will be centered on each word wrap or \n
	WIN_STATUS_CHECK_LIKE					= 0x00080000,		// Make push buttons "check-like" with dual state
	WIN_STATUS_HOTKEY_TEXT				= 0x00100000,		// Make push buttons "check-like" with dual state
	WIN_STATUS_USE_OVERLAY_STATES	= 0x00200000,		// Push buttons will use the global automatic rendering overlay for disabled, hilited, and pushed.
	WIN_STATUS_NOT_READY					= 0x00400000,		// A disabled button that is available -- but not yet (power charge, fire delay).
	WIN_STATUS_FLASHING						= 0x00800000,   // Used for buttons that do cameo flashes.
	WIN_STATUS_ALWAYS_COLOR				= 0x01000000,		// Never render these buttons using greyscale renderer when button disabled.
	WIN_STATUS_ON_MOUSE_DOWN			= 0x02000000,		// Pushbutton triggers on mouse down.
	WIN_STATUS_SHORTCUT_BUTTON		= 0x04000000,   // Oh god... this is a total hack for shortcut buttons to handle rendering text top left corner...
	// when you edit this, remember to edit WindowStatusNames[]

};


// Message Box Button flags --------------------------------------------------------
//-----------------------------------------------------------------------------
enum
{
	MSG_BOX_YES							= 0x01, //Display the yes button
	MSG_BOX_NO							= 0x02, //Display the No button
	MSG_BOX_OK							= 0x08, //Display the Ok button
	MSG_BOX_CANCEL					= 0x04, //Display the Cancel button
};


// WindowMessageBoxData ---------------------------------------------------------
/** Data attached to each Message box window */
//-----------------------------------------------------------------------------
struct WindowMessageBoxData
{
	GameWinMsgBoxFunc yesCallback; ///<Function pointer to the Yes Button Callback
  GameWinMsgBoxFunc noCallback;///<Function pointer to the No Button Callback
  GameWinMsgBoxFunc okCallback;///<Function pointer to the Ok Button Callback
  GameWinMsgBoxFunc cancelCallback;///<Function pointer to the Cancel Button Callback
};

// GameWindowEditData ---------------------------------------------------------
/** Data attached to each window specifically for the editor */
//-----------------------------------------------------------------------------
struct GameWindowEditData
{
	AsciiString systemCallbackString;		///< string for callback
	AsciiString inputCallbackString;			///< string for callback
	AsciiString tooltipCallbackString;		///< string for callback
	AsciiString drawCallbackString;			///< string for callback
};

// GameWindow -----------------------------------------------------------------
/** Class definition for a game window.  These are the basic elements of the
	* whole windowing sytem, all windows are GameWindows, as are all GUI controls
	* etc. */
//-----------------------------------------------------------------------------
class GameWindow : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_ABC( GameWindow )						///< this abstract class needs memory pool hooks

friend class GameWindowManager;

public:

	GameWindow( void );
	// already defined by MPO.
	// virtual ~GameWindow( void );

	/// draw border for this window only, NO child windows or anything
	virtual void winDrawBorder( void ) = 0;

	void linkTransitionWindow( TransitionWindow* transitionWindow );
	void unlinkTransitionWindow( TransitionWindow* transitionWindow );

	Int winSetWindowId( Int id );  ///< set id for this window
	Int winGetWindowId( void );  ///< return window id for this window
	Int winSetSize( Int width, Int height );  ///< set size
	Int winGetSize( Int *width, Int *height );  ///< return size
	Int winActivate( void );  ///< pop window to top of list and activate
	Int winBringToTop( void );  ///< bring this window to the top of the win list
	Int winEnable( Bool enable );  /**< enable/disable a window, a disbled
																 window can be seen but accepts no input */
  Bool winGetEnabled( void ); ///< Is window enabled?
	Int winHide( Bool hide );  ///< hide/unhide a window
	Bool winIsHidden( void );  ///< is this window hidden/
	UnsignedInt winSetStatus( UnsignedInt status );  ///< set status bits
	UnsignedInt winClearStatus( UnsignedInt status );  ///< clear status bits
	UnsignedInt winGetStatus( void );  ///< get status bits
	UnsignedInt winGetStyle( void );  ///< get style bits
	Int winNextTab( void );  ///< advance focus to next window
	Int winPrevTab( void );  ///< change focus to previous window
	Int winSetPosition( Int x, Int y );  ///< set window position
	Int winGetPosition( Int *x, Int *y );  ///< get window position
	Int winGetScreenPosition( Int *x, Int *y );  ///< get screen coordinates
	Int winGetRegion( IRegion2D *region );  ///< get window region
	Int winSetCursorPosition( Int x, Int y );  ///< set window cursor position
	Int winGetCursorPosition( Int *x, Int *y );  ///< get window cursor position

	// --------------------------------------------------------------------------
	// new methods for setting images
	Int winSetEnabledImage( Int index, const Image *image );
	Int winSetEnabledColor( Int index, Color color );
	Int winSetEnabledBorderColor( Int index, Color color );
	const Image *winGetEnabledImage( Int index ) { return m_instData.m_enabledDrawData[ index ].image; }
	Color winGetEnabledColor( Int index ) { return m_instData.m_enabledDrawData[ index ].color; }
	Color winGetEnabledBorderColor( Int index ) { return m_instData.m_enabledDrawData[ index ].borderColor; }

	Int winSetDisabledImage( Int index, const Image *image );
	Int winSetDisabledColor( Int index, Color color );
	Int winSetDisabledBorderColor( Int index, Color color );
	const Image *winGetDisabledImage( Int index ) { return m_instData.m_disabledDrawData[ index ].image; }
	Color winGetDisabledColor( Int index ) { return m_instData.m_disabledDrawData[ index ].color; }
	Color winGetDisabledBorderColor( Int index ) { return m_instData.m_disabledDrawData[ index ].borderColor; }

	Int winSetHiliteImage( Int index, const Image *image );
	Int winSetHiliteColor( Int index, Color color );
	Int winSetHiliteBorderColor( Int index, Color color );
	const Image *winGetHiliteImage( Int index ) { return m_instData.m_hiliteDrawData[ index ].image; }
	Color winGetHiliteColor( Int index ) { return m_instData.m_hiliteDrawData[ index ].color; }
	Color winGetHiliteBorderColor( Int index ) { return m_instData.m_hiliteDrawData[ index ].borderColor; }

	// --------------------------------------------------------------------------
	// draw methods and data
	Int winDrawWindow( void );  ///< draws the default background
	void winSetDrawOffset( Int x, Int y );  ///< set offset for drawing background image data
	void winGetDrawOffset( Int *x, Int *y );  ///< get draw offset
	void winSetHiliteState( Bool state );  ///< set hilite state
	void winSetTooltip( UnicodeString tip );  ///< set tooltip text
  Int  getTooltipDelay() { return m_instData.m_tooltipDelay; } ///< get tooltip delay
  void setTooltipDelay(Int delay) { m_instData.m_tooltipDelay = delay; } ///< set tooltip delay

	//-----------------------------------------------------------------------------
	// text methods
	virtual Int winSetText( UnicodeString newText );  ///< set text string
	UnicodeString winGetText( void );  ///< get text string
	Int winGetTextLength(); ///< get number of chars in text string
	GameFont *winGetFont( void );  ///< get the font being used by this window
	virtual void winSetFont( GameFont *font );  ///< set font for window
	void winSetEnabledTextColors( Color color, Color borderColor );
	void winSetDisabledTextColors( Color color, Color borderColor );
	void winSetIMECompositeTextColors( Color color, Color borderColor );
	void winSetHiliteTextColors( Color color, Color borderColor );
	Color winGetEnabledTextColor( void );
	Color winGetEnabledTextBorderColor( void );
	Color winGetDisabledTextColor( void );
	Color winGetDisabledTextBorderColor( void );
	Color winGetIMECompositeTextColor( void );
	Color winGetIMECompositeBorderColor( void );
	Color winGetHiliteTextColor( void );
	Color winGetHiliteTextBorderColor( void );

	// window instance data
	Int winSetInstanceData( WinInstanceData *data );  ///< copy over instance data
	WinInstanceData *winGetInstanceData( void );  ///< get instance data
	virtual void *winGetUserData( void );  ///< get the window user data
	void winSetUserData( void *userData );  ///< set the user data

	// heirarchy methods
	Int winSetParent( GameWindow *parent );  ///< set parent
	GameWindow *winGetParent( void );  ///< get parent
	Bool winIsChild( GameWindow *child );  ///< verifies parent
	GameWindow *winGetChild( void );  ///< get the child window
	Int winSetOwner( GameWindow *owner );  ///< set owner
	GameWindow *winGetOwner( void );  ///< get window's owner
	void winSetNext( GameWindow *next );  ///< set next pointer
	void winSetPrev( GameWindow *prev );  ///< set prev pointer
	GameWindow *winGetNext( void );  ///< get next window in window list
	GameWindow *winGetPrev( void );  ///< get previous window in window list

	// these are for interacting with a group of windows as a shell "screen"
	void winSetNextInLayout( GameWindow *next );  ///< set next in layout
	void winSetPrevInLayout( GameWindow *prev );  ///< set prev in layout
	void winSetLayout( WindowLayout *layout );  ///< set layout
	WindowLayout *winGetLayout( void );  ///< get layout layout
	GameWindow *winGetNextInLayout( void );  ///< get next window in layout
	GameWindow *winGetPrevInLayout( void );  ///< get prev window in layout

	// setting the callbacks ----------------------------------------------------
	Int winSetSystemFunc( GameWinSystemFunc system );  ///< set system
	Int winSetInputFunc( GameWinInputFunc input );  ///< set input
	Int winSetDrawFunc( GameWinDrawFunc draw );  ///< set draw
	Int winSetTooltipFunc( GameWinTooltipFunc tooltip );  ///< set tooltip
	Int winSetCallbacks( GameWinInputFunc input,
											 GameWinDrawFunc draw,
											 GameWinTooltipFunc tooltip );  ///< set draw, input, tooltip

	// pick correlation ---------------------------------------------------------
	Bool winPointInWindow( Int x, Int y );  /**is point inside this window?
																					also return TRUE if point is in
																					a child */
	/** given a piont, return the child window which contains the mouse pointer,
	if the point is not in a chilc, the function returns the 'window' paramater
	back to the caller */
	GameWindow *winPointInChild( Int x, Int y, Bool ignoreEnableCheck = FALSE, Bool playDisabledSound = FALSE );
	/** finds the child which contains the mouse pointer - reguardless of
	the enabled status of the child */
	GameWindow *winPointInAnyChild( Int x, Int y, Bool ignoreHidden, Bool ignoreEnableCheck = FALSE );

  // get the callbacks for a window -------------------------------------------
	GameWinInputFunc		winGetInputFunc( void );
	GameWinSystemFunc		winGetSystemFunc( void );
	GameWinDrawFunc			winGetDrawFunc( void );
	GameWinTooltipFunc	winGetTooltipFunc( void );

	// editor access only -------------------------------------------------------
	void winSetEditData( GameWindowEditData *editData );
	GameWindowEditData *winGetEditData( void );

protected:

	/// 'images' should be taken care of when we hide ourselves or are destroyed
	void freeImages( void ) { }
	Bool isEnabled( void );  ///< see if we and our parents are enabled

	void unlinkFromTransitionWindows( void );

	void normalizeWindowRegion( void );  ///< put UL corner in window region.lo

	GameWindow *findFirstLeaf( void );  ///< return first leaf of branch
	GameWindow *findLastLeaf( void );  ///< return last leaf of branch
	GameWindow *findPrevLeaf( void );  ///< return prev leav in tree
	GameWindow *findNextLeaf( void );  ///< return next leaf in tree

	// **************************************************************************

	Int m_status;      									// Status bits for this window
	ICoord2D  m_size;						     	  // Width and height of the window
	IRegion2D m_region;      					  // Current region occupied by window.
																			// Low x,y is the window's origin
	Int m_cursorX;											// window cursor X position if any
	Int m_cursorY;											// window cursor Y position if any

	void *m_userData;										// User defined data area
	WinInstanceData m_instData;					// Class data, varies by window type
	void *m_inputData;								  // Client data

  // user defined callbacks
	GameWinInputFunc			m_input;					///< callback for input
	GameWinSystemFunc			m_system;					///< callback for system messages
	GameWinDrawFunc				m_draw;						///< callback for drawing
	GameWinTooltipFunc		m_tooltip;				///< callback for tooltip execution

	GameWindow *m_next, *m_prev;	// List of sibling windows
	GameWindow *m_parent;				// Window which contains this window
	GameWindow *m_child;			  // List of windows within this window

	//
	// the following are for "layout screens" and ONLY apply to root/parent
	// windows in a layout, any children of a window that is part of a layout
	// does NOT have layout screen information
	//
	GameWindow *m_nextLayout;  ///< next in layout
	GameWindow *m_prevLayout;  ///< prev in layout
	WindowLayout *m_layout;    ///< layout this window is a part of

	// game window edit data for the GUIEditor only
	GameWindowEditData *m_editData;

	// vector of window transitions that have a relation to the current GameWindow
	std::vector<TransitionWindow*> m_transitionWindows;

};

// TheSuperHackers @feature helmutbuhler 24/04/2025
// GameWindow that does nothing. Used for Headless Mode.
class GameWindowDummy : public GameWindow
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(GameWindowDummy, "GameWindowDummy")
public:
	virtual void winDrawBorder() {}
	virtual void* winGetUserData(void) { return nullptr; }
};

// ModalWindow ----------------------------------------------------------------
//-----------------------------------------------------------------------------
class ModalWindow : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ModalWindow, "ModalWindow" )
public:
	GameWindow *window;						// Pointer to Modal Window
	ModalWindow *next;		// Next Window Pointer

};
EMPTY_DTOR(ModalWindow)

// Errors returned by window functions
enum
{

	WIN_ERR_OK								=  0,	// No Error
	WIN_ERR_GENERAL_FAILURE		= -1,	// General library failure
	WIN_ERR_INVALID_WINDOW		= -2,	// Window parameter was invalid
	WIN_ERR_INVALID_PARAMETER	= -3,	// Parameter was invalid
	WIN_ERR_MOUSE_CAPTURED		= -4,	// Mouse already captured
	WIN_ERR_KEYBOARD_CAPTURED	= -5,	// Keyboard already captured
	WIN_ERR_OUT_OF_WINDOWS		= -6		// Too many windows have been created

};

// Input capture/release flags
enum
{

	WIN_CAPTURE_MOUSE			= 0x00000001,		// capture mouse
	WIN_CAPTURE_KEYBOARD  = 0x00000002,		// capture keyboard
	WIN_CAPTURE_ALL       = 0xFFFFFFFF,		// capture keyboard and mouse

};

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////
extern void GameWinDefaultDraw( GameWindow *window,
																WinInstanceData *instData );
extern WindowMsgHandledType GameWinDefaultSystem( GameWindow *window,
																	UnsignedInt msg,
																  WindowMsgData mData1,
																	WindowMsgData mData2 );
extern WindowMsgHandledType GameWinDefaultInput( GameWindow *window,
																 UnsignedInt msg,
																 WindowMsgData mData1,
																 WindowMsgData mData2 );
extern WindowMsgHandledType GameWinBlockInput( GameWindow *window,
																 UnsignedInt msg,
																 WindowMsgData mData1,
																 WindowMsgData mData2 );
extern void GameWinDefaultTooltip( GameWindow *window,
																	 WinInstanceData *instData,
																	 UnsignedInt mouse );

extern const char *const WindowStatusNames[];
extern const char *const WindowStyleNames[];
