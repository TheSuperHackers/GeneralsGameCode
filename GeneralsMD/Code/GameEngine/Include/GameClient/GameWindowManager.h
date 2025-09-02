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

// FILE: GameWindowManager.h //////////////////////////////////////////////////////////////////////
// Created:    Colin Day, June 2001
// Desc:       The game window manager is the interface for interacting with
//						 the windowing system for purposes of any menus, or GUI
//						 controls.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __GAMEWINDOWMANAGER_H_
#define __GAMEWINDOWMANAGER_H_


#include "Common/STLTypedefs.h"
#include "Common/SubsystemInterface.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/Gadget.h"

class GameWindow;

//-------------------------------------------------------------------------------------------------
/** Window layout info is a structure that can be passed to the load function of
	* a window script.  After the script is loaded, this parameter (if present)
	* will contain information about the script file at a global level such
	* as what file version was loaded, global layout callbacks, etc */
//-------------------------------------------------------------------------------------------------
enum { MAX_LAYOUT_FUNC_LEN = 256 };

typedef std::list <GameWindow *> GameWindowList;

class WindowLayoutInfo
{

public:
	WindowLayoutInfo();

	UnsignedInt version;												///< file version that was loaded
	WindowLayoutInitFunc init;									///< init method (if specified)
	WindowLayoutUpdateFunc update;							///< update method (if specified)
	WindowLayoutShutdownFunc shutdown;					///< shutdown method (if specified)
	AsciiString initNameString;									///< init method in flavor of string name
	AsciiString updateNameString;								///< update method in string flavor
	AsciiString  shutdownNameString;						///< shutdown method in string flavor, mmm!
	std::list<GameWindow *> windows;						///< list of top-level windows in the layout
};

//-------------------------------------------------------------------------------------------------
/** There exists a singleton GameWindowManager that defines how we can
	* interact with the game windowing system */
//-------------------------------------------------------------------------------------------------
class GameWindowManager : public SubsystemInterface
{

friend class GameWindow;

public:

	GameWindowManager( void );
	virtual ~GameWindowManager( void );

	//-----------------------------------------------------------------------------------------------
	virtual void init( void );		///< initialize function
	virtual void reset( void );		///< reset the system
	virtual void update( void );  ///< update method, called once per frame

	virtual GameWindow *allocateNewWindow( void ) = 0;  ///< new game window

	void linkWindow( GameWindow *window );  ///< link into master list
	void unlinkWindow( GameWindow *window );  ///< unlink from master list
	void unlinkChildWindow( GameWindow *window );  ///< remove child from parent list
	void insertWindowAheadOf( GameWindow *window, GameWindow *aheadOf );  ///< add window to list 'ahead of'

	virtual GameWinDrawFunc getPushButtonImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getPushButtonDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getCheckBoxImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getCheckBoxDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getRadioButtonImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getRadioButtonDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getTabControlImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getTabControlDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getListBoxImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getListBoxDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getComboBoxImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getComboBoxDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getHorizontalSliderImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getHorizontalSliderDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getVerticalSliderImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getVerticalSliderDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getProgressBarImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getProgressBarDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getStaticTextImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getStaticTextDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getTextEntryImageDrawFunc( void ) = 0;
	virtual GameWinDrawFunc getTextEntryDrawFunc( void ) = 0;

	//---------------------------------------------------------------------------

	virtual GameWinDrawFunc getDefaultDraw( void );  ///< return default draw func
	virtual GameWinSystemFunc getDefaultSystem( void );  ///< return default system func
	virtual GameWinInputFunc getDefaultInput( void );  ///< return default input func
	virtual GameWinTooltipFunc getDefaultTooltip( void );  ///< return default tooltip func

	//---------------------------------------------------------------------------
	// MessageBox creation
	virtual GameWindow *gogoMessageBox(Int x, Int y, Int width, Int height, UnsignedShort buttonFlags,
                        UnicodeString titleString, UnicodeString bodyString,
                        GameWinMsgBoxFunc yesCallback,
                        GameWinMsgBoxFunc noCallback,
                        GameWinMsgBoxFunc okCallback,
                        GameWinMsgBoxFunc cancelCallback );

	virtual GameWindow *gogoMessageBox(Int x, Int y, Int width, Int height, UnsignedShort buttonFlags,
                        UnicodeString titleString, UnicodeString bodyString,
                        GameWinMsgBoxFunc yesCallback,
                        GameWinMsgBoxFunc noCallback,
                        GameWinMsgBoxFunc okCallback,
                        GameWinMsgBoxFunc cancelCallback, Bool useLogo );


	//---------------------------------------------------------------------------
	// gadget creation
	virtual GameWindow *gogoGadgetPushButton( GameWindow *parent, UnsignedInt status,
																						Int x, Int y, Int width, Int height,
																						WinInstanceData *instData,
																						GameFont *defaultFont, Bool defaultVisual );
	virtual GameWindow *gogoGadgetCheckbox( GameWindow *parent, UnsignedInt status,
																					Int x, Int y, Int width, Int height,
																					WinInstanceData *instData,
																					GameFont *defaultFont, Bool defaultVisual );
	virtual GameWindow *gogoGadgetRadioButton( GameWindow *parent, UnsignedInt status,
																						 Int x, Int y, Int width, Int height,
																						 WinInstanceData *instData,
																						 RadioButtonData *rData,
																						 GameFont *defaultFont, Bool defaultVisual );
	virtual GameWindow *gogoGadgetTabControl( GameWindow *parent, UnsignedInt status,
																						 Int x, Int y, Int width, Int height,
																						 WinInstanceData *instData,
																						 TabControlData *rData,
																						 GameFont *defaultFont, Bool defaultVisual );
	virtual GameWindow *gogoGadgetListBox( GameWindow *parent, UnsignedInt status,
																				 Int x, Int y, Int width, Int height,
																				 WinInstanceData *instData,
																				 ListboxData *listboxData,
																				 GameFont *defaultFont, Bool defaultVisual );
	virtual GameWindow *gogoGadgetSlider( GameWindow *parent, UnsignedInt status,
																				Int x, Int y, Int width, Int height,
																				WinInstanceData *instData,
																				SliderData *sliderData,
																				GameFont *defaultFont, Bool defaultVisual );
	virtual GameWindow *gogoGadgetProgressBar( GameWindow *parent, UnsignedInt status,
																						 Int x, Int y, Int width, Int height,
																						 WinInstanceData *instData,
																						 GameFont *defaultFont, Bool defaultVisual );
	virtual GameWindow *gogoGadgetStaticText( GameWindow *parent, UnsignedInt status,
																						Int x, Int y, Int width, Int height,
																						WinInstanceData *instData,
																						TextData *textData,
																						GameFont *defaultFont, Bool defaultVisual );
	virtual GameWindow *gogoGadgetTextEntry( GameWindow *parent, UnsignedInt status,
																					 Int x, Int y, Int width, Int height,
																					 WinInstanceData *instData,
																					 EntryData *entryData,
																					 GameFont *defaultFont, Bool defaultVisual );
	virtual GameWindow *gogoGadgetComboBox( GameWindow *parent, UnsignedInt status,
										                              Int x, Int y, Int width, Int height,
																									WinInstanceData *instData,
																									ComboBoxData *comboBoxDataTemplate,
																								  GameFont *defaultFont, Bool defaultVisual );

	/** Use this method to assign the default images to gadgets as
		* they area created */
	virtual void assignDefaultGadgetLook( GameWindow *gadget,
																				GameFont *defaultFont,
																				Bool assignVisual );

	//---------------------------------------------------------------------------
	// Creating windows
	/// create new window(s) from .wnd file ... see definition for what is returned
	virtual GameWindow *winCreateFromScript( AsciiString filename, WindowLayoutInfo *info = NULL );

	/// create new window(s) from .wnd file and wrap in a WindowLayout
	virtual WindowLayout *winCreateLayout( AsciiString filename );

	/// free temporary strings to make the memory leak manager happy.
	virtual void freeStaticStrings(void);

	/// create a new window by setting up parameters and callbacks
	virtual GameWindow *winCreate( GameWindow *parent, UnsignedInt status,
																 Int x, Int y, Int width, Int height,
																 GameWinSystemFunc system,
																 WinInstanceData *instData = NULL );

	//---------------------------------------------------------------------------
	// Manipulating windows in the system
	virtual Int winDestroy( GameWindow *window );  ///< destroy this window
	virtual Int winDestroyAll( void );  ///< destroy all windows in the system
	virtual GameWindow *winGetWindowList( void );  ///< get head of master list

	/// hide all windows in a certain range of id's (inclusinve );
	virtual void hideWindowsInRange( GameWindow *baseWindow, Int first, Int last,
																	 Bool hideFlag );
	/// enable all windows in a range of id's (inclusive)
	virtual void enableWindowsInRange( GameWindow *baseWindow, Int first, Int last,
																		 Bool enableFlag );

	/// this gets called from winHide() when a window hides itself
	virtual void windowHiding( GameWindow *window );

	virtual void winRepaint( void );  ///< draw GUI in reverse order

	virtual void winNextTab( GameWindow *window );	///< give keyboard focus to the next window in the tab list

	virtual void winPrevTab( GameWindow *window );	///< give keyboard focus to the previous window in the tab list

	virtual void registerTabList( GameWindowList tabList );	///< we have to register a Tab List
	virtual void clearTabList( void );	///< we's gotz ta clear the tab list yo!

	// --------------------------------------------------------------------------
	/// process a single mouse event
	virtual WinInputReturnCode winProcessMouseEvent( GameWindowMessage msg,
														 ICoord2D *mousePos,
														 void *data );
	/// process a singke key event
	virtual WinInputReturnCode winProcessKey( UnsignedByte key,
																						UnsignedByte state );
	// --------------------------------------------------------------------------

	virtual GameWindow *winGetFocus( void );  ///< return window that has the focus
	virtual Int winSetFocus( GameWindow *window );  ///< set this window as has focus
	virtual void winSetGrabWindow( GameWindow *window );  ///< set the grab window
	virtual GameWindow *winGetGrabWindow( void );  ///< who is currently 'held' by mouse
	virtual void winSetLoneWindow( GameWindow *window );  ///< set the open window

	// Finds the top-level window at the mouse position that matches the required and forbidden status masks.
	virtual GameWindow* findWindowUnderMouse(GameWindow*& toolTipWindow, const ICoord2D* mousePos, unsigned int requiredStatusMask, unsigned int forbiddenStatusMask);
	static bool isMouseWithinWindow(GameWindow* window, const ICoord2D* mousePos, unsigned int requiredStatusMask, unsigned int forbiddenStatusMask);

	virtual Bool isEnabled( GameWindow *win );  ///< is window or parents enabled
	virtual Bool isHidden( GameWindow *win );  ///< is parent or parents hidden
	virtual void addWindowToParent( GameWindow *window, GameWindow *parent );
	virtual void addWindowToParentAtEnd( GameWindow *window, GameWindow *parent );

	/// sends a system message to specified window
	virtual WindowMsgHandledType winSendSystemMsg( GameWindow *window, UnsignedInt msg,
																 WindowMsgData mData1, WindowMsgData mData2 );

	/// sends an input message to the specified window
	virtual WindowMsgHandledType winSendInputMsg( GameWindow *window, UnsignedInt msg,
																WindowMsgData mData1, WindowMsgData mData2 );

	/** get the window pointer from id, starting at 'window' and searching
	down the heirarchy.  If 'window' is NULL then all windows will
	be searched */
	virtual GameWindow *winGetWindowFromId( GameWindow *window, Int id );
	virtual Int winCapture( GameWindow *window );  ///< captures the mouse
	virtual Int winRelease( GameWindow *window );  ///< release mouse capture
	virtual GameWindow *winGetCapture( void );  ///< current mouse capture settings

	virtual Int winSetModal( GameWindow *window );  ///< put at top of modal stack
	virtual Int winUnsetModal( GameWindow *window );  /**< take window off modal stack, if window is
																										not at top of stack and error will occur */

	//---------------------------------------------------------------------------
	/////////////////////////////////////////////////////////////////////////////
	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------
	// The following public functions you should implement change the guts
	// for to work with your application.  Rather than draw images, or do string
	// operations with native methods, the game window system will always call
	// these methods to get the work done it needs
	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------
	/////////////////////////////////////////////////////////////////////////////
	//---------------------------------------------------------------------------

	/// draw image, coord are in screen and should be kepth within that box specified
	virtual void winDrawImage( const Image *image, Int startX, Int startY, Int endX, Int endY, Color color = 0xFFFFFFFF );
	/// draw filled rect, coords are absolute screen coords
	virtual void winFillRect( Color color, Real width, Int startX, Int startY, Int endX, Int endY );
	/// draw rect outline, coords are absolute screen coords
	virtual void winOpenRect( Color color, Real width, Int startX, Int startY, Int endX, Int endY );
	/// draw line, coords are absolute screen coords
	virtual void winDrawLine( Color color, Real width, Int startX, Int startY, Int endX, Int endY );
	/// Make a color representation out of RGBA components
	virtual Color winMakeColor( UnsignedByte red, UnsignedByte green, UnsignedByte blue, UnsignedByte alpha );
	/** Find an image reference and return a pointer to its image, you may
	recreate all Image structs to suit your project */
	virtual const Image *winFindImage( const char *name );
	virtual Int winFontHeight( GameFont *font );  ///< get height of font in pixels
	virtual Int winIsDigit( Int c );  ///< is character a digit
	virtual Int winIsAscii( Int c );  ///< is character a digit
	virtual Int winIsAlNum( Int c );  ///< is character alpha-numeric
	virtual void winFormatText( GameFont *font, UnicodeString text, Color color,
															Int x, Int y, Int width, Int height );
	virtual void winGetTextSize( GameFont *font, UnicodeString text,
															 Int *width, Int *height, Int maxWidth );
	virtual UnicodeString winTextLabelToText( AsciiString label );  ///< convert localizable text label to real text
	virtual GameFont *winFindFont( AsciiString fontName, Int pointSize, Bool bold );  ///< get a font given a name

	/// @todo just for testing, remov this
	Bool initTestGUI( void );

	virtual GameWindow *getWindowUnderCursor( Int x, Int y, Bool ignoreEnabled = FALSE );	///< find the top window at the given coordinates

	//---------------------------------------------------------------------------
	/////////////////////////////////////////////////////////////////////////////
	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------
	/////////////////////////////////////////////////////////////////////////////
	//---------------------------------------------------------------------------

protected:

	void processDestroyList( void );  ///< process windows waiting to be killed

	Int drawWindow( GameWindow *window );  ///< draw this window

	void dumpWindow( GameWindow *window );  ///< for debugging

	GameWindow *m_windowList;			// list of all top level windows
	GameWindow *m_windowTail;			// last in windowList

	GameWindow *m_destroyList;			// list of windows to destroy

	GameWindow *m_currMouseRgn;		// window that mouse is over
	GameWindow *m_mouseCaptor;			// window that captured mouse
	GameWindow *m_keyboardFocus;		// window that has input focus
	ModalWindow *m_modalHead;			// top of windows in the modal stack
	GameWindow *m_grabWindow;			// window that grabbed the last down event
	GameWindow *m_loneWindow;			// Set if we just opened a Lone Window
	GameWindowList m_tabList;			// we have to register a tab list to make a tab list.
	const Image *m_cursorBitmap;
	UnsignedInt m_captureFlags;

};  // end GameWindowManager

// INLINE /////////////////////////////////////////////////////////////////////////////////////////
inline GameWinDrawFunc GameWindowManager::getDefaultDraw( void ) { return GameWinDefaultDraw; }
inline GameWinSystemFunc GameWindowManager::getDefaultSystem( void ) { return GameWinDefaultSystem; }
inline GameWinInputFunc GameWindowManager::getDefaultInput( void )  { return GameWinDefaultInput; }
inline GameWinTooltipFunc GameWindowManager::getDefaultTooltip( void ) { return GameWinDefaultTooltip; }

// EXTERN /////////////////////////////////////////////////////////////////////////////////////////
extern GameWindowManager *TheWindowManager;			///< singleton extern definition
extern UnsignedInt WindowLayoutCurrentVersion;  ///< current version of our window layouts

// this function lets us generically pass button selections to our parent, we may
// frequently want to do this because we want windows grouped on child windows for
// convenience, but only want one logical system procedure responding to them all
extern WindowMsgHandledType PassSelectedButtonsToParentSystem( GameWindow *window,
																															 UnsignedInt msg,
																															 WindowMsgData mData1,
																															 WindowMsgData mData2 );
extern WindowMsgHandledType PassMessagesToParentSystem( GameWindow *window,
																															 UnsignedInt msg,
																															 WindowMsgData mData1,
																															 WindowMsgData mData2 );

// TheSuperHackers @feature helmutbuhler 24/04/2025
// GameWindowManager that does nothing. Used for Headless Mode.
class GameWindowManagerDummy : public GameWindowManager
{
public:
	virtual GameWindow *winGetWindowFromId(GameWindow *window, Int id);
	virtual GameWindow *winCreateFromScript(AsciiString filenameString, WindowLayoutInfo *info);

	virtual GameWindow *allocateNewWindow() { return newInstance(GameWindowDummy); }

	virtual GameWinDrawFunc getPushButtonImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getPushButtonDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getCheckBoxImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getCheckBoxDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getRadioButtonImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getRadioButtonDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getTabControlImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getTabControlDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getListBoxImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getListBoxDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getComboBoxImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getComboBoxDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getHorizontalSliderImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getHorizontalSliderDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getVerticalSliderImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getVerticalSliderDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getProgressBarImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getProgressBarDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getStaticTextImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getStaticTextDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getTextEntryImageDrawFunc() { return NULL; }
	virtual GameWinDrawFunc getTextEntryDrawFunc() { return NULL; }
};

#endif // __GAMEWINDOWMANAGER_H_

