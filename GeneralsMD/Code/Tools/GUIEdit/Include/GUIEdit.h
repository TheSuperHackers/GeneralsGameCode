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

// FILE: GUIEdit.h ////////////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, July 2001
// Desc:       GUI Edit and window layout entry point
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __GUIEDIT_H_
#define __GUIEDIT_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>

#include "GameClient/GameWindow.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/GameWindowManager.h"
#include "GUIEditColor.h"
#include "Common/AsciiString.h"

// TYPE DEFINES ///////////////////////////////////////////////////////////////////////////////////
#define GUIEDIT_CONFIG_FILENAME "GUIEdit.cfg"
#define GUIEDIT_FONT_FILENAME		"GUIEFont.txt"
#define GUIEDIT_NONE_STRING "[None]"

#define STATUS_BAR_ID 1
#define TOOLBAR_ID		2

//-------------------------------------------------------------------------------------------------
/** Edit mode states for editor */
//-------------------------------------------------------------------------------------------------
typedef enum
{

	MODE_UNDEFINED = 0,							///< undefined mode
	MODE_EDIT,											///< normal edit mode
	MODE_TEST_RUN,									///< test input like ingame
	MODE_DRAG_MOVE,									///< moving windows with the mouse
	MODE_RESIZE_TOP_LEFT,						///< resize dragging top left corner
	MODE_RESIZE_TOP_RIGHT,					///< resize dragging top right cornder
	MODE_RESIZE_BOTTOM_RIGHT,				///< resize dragging bottom right corner
	MODE_RESIZE_BOTTOM_LEFT,				///< resize dragging bottom left corner
	MODE_RESIZE_TOP,								///< resize dragging top horizontal
	MODE_RESIZE_RIGHT,							///< resize dragging right vertical
	MODE_RESIZE_BOTTOM,							///< resize dragging bottom horizontal
	MODE_RESIZE_LEFT,								///< resize dragging left vertical
	MODE_KEYBOARD_MOVE,							///< moving windows with the Keyboard

} EditMode;

//-------------------------------------------------------------------------------------------------
/** The status bar is this many pieces and we can access them this way */
//-------------------------------------------------------------------------------------------------
typedef enum
{

	STATUS_MODE = 0,
	STATUS_PART2,			/// change to meaningful name when decided what goes here
	STATUS_PART3,			/// change to meaningful name when decided what goes here
	STATUS_PART4,			/// change to meaningful name when decided what goes here
	STATUS_MOUSE_COORDS,

	STATUS_NUM_PARTS

} StatusPart;

//-------------------------------------------------------------------------------------------------
/** Types of pointing cursors */
//-------------------------------------------------------------------------------------------------
typedef enum
{
	CURSOR_NORMAL,
	CURSOR_MOVE,
	CURSOR_SIZE_NESW,
	CURSOR_SIZE_NS,
	CURSOR_SIZE_NWSE,
	CURSOR_SIZE_WE,
	CURSOR_WAIT,

} CursorType;

//-------------------------------------------------------------------------------------------------
/** These entrys make up the selection list of windows */
//-------------------------------------------------------------------------------------------------
struct WindowSelectionEntry
{

	GameWindow *window;  ///< the window
	WindowSelectionEntry *next;
	WindowSelectionEntry *prev;

};

//-------------------------------------------------------------------------------------------------
/** Framework for GUI editor data */
//-------------------------------------------------------------------------------------------------
class GUIEdit
{

public:

	GUIEdit( void );
	~GUIEdit( void );

	void init( void );  ///< initialize data
	void shutdown( void );  ///< shutdown all our data
	void update( void );  ///< process the universe

	Bool readConfigFile( const char *filename );  ///< read the configuration file
	Bool writeConfigFile( const char *filename );  ///< write the configuration file

	void readFontFile( const char *filename );  ///< read file with available font definitions
	void writeFontFile( const char *filename );  ///< write all loaded fonts to a file

	char *getSaveFilename( void );
	char *getSavePathAndFilename( void );
	void setSaveFile( const char *fullPathAndFilename );  ///< set filename to use for saving

	HWND getWindowHandle( void );  ///< get window handle
	HINSTANCE getInstance( void );  ///< get application instance
	HWND getStatusBarWindowHandle( void );  ///< get status bar HWND
	void createStatusBar( void );  ///< create status bar
	void statusMessage( StatusPart part, const char *message );  ///< set status bar textl
	void createToolbar( void );  ///< create the toolbar
	void setCursor( CursorType type );  ///< set cursor

	void setPropertyTarget( GameWindow *window );  ///< set window for property editing
	GameWindow *getPropertyTarget( void );  ///< get window editing properties

	void loadGUIEditFontLibrary( FontLibrary *library );  ///< fonts available in the editor

	Bool isNameDuplicate( GameWindow *root, GameWindow *ignore, AsciiString name );

	EditMode getMode( void );  ///< return the current mode
	void setMode( EditMode mode );  ///< set the new mode

	void setUnsaved( Bool unsaved );  ///< set unsaved flag to FALSE or TRUE
	Bool newLayout( void );  ///< reset editor for new layout

	// grid settings
	void setGridResolution( Int res );
	Int getGridResolution( void );
	void setGridVisible( Bool visible );
	Bool isGridVisible( void );
	void setGridSnap( Bool on );
	Bool isGridSnapOn( void );
	void setGridColor( RGBColorInt *color );
	RGBColorInt *getGridColor( void );
	void gridSnapLocation( ICoord2D *source, ICoord2D *snapped );

	// display options for hidden/see thru windows
	void setShowHiddenOutlines( Bool show );
	Bool getShowHiddenOutlines( void );
	void setShowSeeThruOutlines( Bool show );
	Bool getShowSeeThruOutlines( void );

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// manipulating windows ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	Bool windowIsGadget( GameWindow *window );  ///< is the window a gadget

	// selection help ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	WindowSelectionEntry *getSelectList( void );  ///< get the selected list head
	GameWindow *getFirstSelected( void );  ///< get first selected window
	void moveWindowTo( GameWindow *window, Int x, Int y );  ///< move window
	Bool isWindowSelected( GameWindow *window );  ///< is window selected
	void selectWindow( GameWindow *window );  ///< add to selection list
	void unSelectWindow( GameWindow *window );  ///< remove from selection list
	void clearSelections( void );  ///< clear selection list
	Int  selectionCount( void );  ///< return # of selected windows
	void deleteSelected( void );  ///< delete selected windows
	void bringSelectedToTop( void );  ///< bring selected windows to top
	/// select the windows that are FULLY in the region specified
	void selectWindowsInRegion( IRegion2D *region );

	// resizing/moving help ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void dragMoveSelectedWindows( ICoord2D *dragOrigin, ICoord2D *dragDest );  ///< move windows via drag move
	/// given a position to move a window to, keep it onscreen and inside parent
	void computeSafeLocation( GameWindow *window, Int x, Int y,
														Int *safeX, Int *safeY );
	/// given position and size to move a window to, keep it inside parent and on screen
	void computeSafeSizeLocation( GameWindow *window,
																Int newX, Int newY,
																Int newWidth, Int newHeight,
																Int *safeX, Int *safeY,
																Int *safeWidth, Int *safeHeight );
	/// compute new size of window using drag-resize logic
	void computeResizeLocation( EditMode resizeMode,
														  GameWindow *window,
														  ICoord2D *resizeOrigin,
														  ICoord2D *resizeDest,
														  ICoord2D *resultLoc,
														  ICoord2D *resultSize );

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// creating new windows and controls ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	/** this is called after a window is added to the editor, the window may
	contain children so this might be recursively called from inside */
	void notifyNewWindow( GameWindow *window );

	GameWindow *getWindowAtPos( Int x, Int y );  ///< get topmost window at pos

	void deleteAllWindows( void );  ///< delete all windows in the editor
	void removeWindowCleanup( GameWindow *window );  ///< to cleanup before delete
	void deleteWindow( GameWindow *window );  ///< delete a game window

	/** when creating child windows we don't want them to exist outside the
	parent so we use this to clip them to the the parent size and locations */
	void clipCreationParamsToParent( GameWindow *parent,
																	 Int *x, Int *y, Int *width, Int *height );

	GameWindow *newWindow( UnsignedInt windowStyle,
												 GameWindow *parent,
												 Int x, Int y,
												 Int width, Int height );
	GameWindow *newUserWindow( GameWindow *parent,
														 Int x, Int y,
														 Int width, Int height );
	GameWindow *newPushButton( GameWindow *parent,
														 Int x, Int y,
														 Int width, Int height );
	GameWindow *newCheckBox( GameWindow *parent,
													 Int x, Int y,
													 Int width, Int height );
	GameWindow *newRadioButton( GameWindow *parent,
														  Int x, Int y,
															Int width, Int height );
	GameWindow *newTabControl( GameWindow *parent,
														  Int x, Int y,
															Int width, Int height );
	GameWindow *newHorizontalSlider( GameWindow *parent,
																	 Int x, Int y,
																	 Int width, Int height );
	GameWindow *newVerticalSlider( GameWindow *parent,
																 Int x, Int y,
																 Int width, Int height );
	GameWindow *newProgressBar( GameWindow *parent,
														  Int x, Int y,
															Int width, Int height );
	GameWindow *newListbox( GameWindow *parent,
													Int x, Int y,
													Int width, Int height );
	GameWindow *newTextEntry( GameWindow *parent,
													  Int x, Int y,
														Int width, Int height );
	GameWindow *newStaticText( GameWindow *parent,
														 Int x, Int y,
														 Int width, Int height );
	GameWindow *newComboBox( GameWindow *parent,
														 Int x, Int y,
														 Int width, Int height );

	// menu options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Bool menuNew( void );  ///< start a new layout process
	Bool menuOpen( void );  ///< hit open on menu
	void stripNameDecorations( GameWindow *root );		///< after a load
	void revertDefaultCallbacks( GameWindow *root );  ///< after a load
	Bool menuSave( void );  ///< hit save on menu
	Bool menuSaveAs( void );  ///< hit save as on menu
	Bool menuExit( void );  ///< exit application
	Bool menuCopy( void );  ///< copy selected windows into copy buffer
	Bool menuPaste( void );  ///< paste contents of copy buffer
	Bool menuCut( void );  ///< cut selected windows into copy buffer
	void checkMenuItem( Int item );
	void unCheckMenuItem( Int item );

	// global layout functions callbacks ------------------------------------------------------------
	void setLayoutInit( AsciiString init );								///< set layout init function name
	void setLayoutUpdate( AsciiString update );						///< set layout update function name
	void setLayoutShutdown( AsciiString shutdown );				///< set layout shutdown function name
	AsciiString getLayoutInit( void );										///< get layout init function name
	AsciiString getLayoutUpdate( void );									///< get layout update function name
	AsciiString getLayoutShutdown( void );								///< get layout shutdown function name

protected:

	char *saveAsDialog( void );  ///< save as standard browser
	char *openDialog( void );  ///< open standard browser

	void validateNames( GameWindow *root, char *filename, Bool *valid );
	void updateRadioScreenIdentifiers( GameWindow *window, Int screenID );
	Bool saveData( char *filePathAndFilename, char *filename );  ///< save data to file in filePath

	Bool validateParentForCreate( GameWindow *parent );  ///< validate parent OK

	/// normalize a region so that lo and hi are really lo and hi
	void normalizeRegion( IRegion2D *region );

	/// find selection entry associated with this window
	WindowSelectionEntry *findSelectionEntry( GameWindow *window );

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	HINSTANCE m_appInst;		///< main application hInstance
	HWND m_appHWnd;					///< main application hWnd
	HWND m_statusBarHWnd;		///< status bar window handle
	HWND m_toolbarHWnd;			///< toolbar window handle

	EditMode m_mode;				///< the current editor "mode"
	Bool m_unsaved;					///< TRUE when contents are unsaved

	char m_savePathAndFilename[ _MAX_PATH ];  ///< full path and filename to file
	char m_saveFilename[ _MAX_PATH ];					///< filename only with extension

	WindowSelectionEntry *m_selectList;  ///< list of "selected" windows
	GameWindow *m_propertyTarget;				 ///< the window to edit properties on

	Bool m_gridVisible;								///< TRUE when grid is visible
	Int m_gridResolution;							///< pixels between the grid marks
	Bool m_snapToGrid;								///< TRUE when it's on
	RGBColorInt m_gridColor;					///< the grid draw color

	Bool m_showHiddenOutlines;				///< show outlines around hidden windows
	Bool m_showSeeThruOutlines;				///< show outliens around see-thru windows

	AsciiString m_layoutInitString;		 ///< layout initi function name
	AsciiString m_layoutUpdateString;	 ///< layout update function name
	AsciiString m_layoutShutdownString;///< layout shutdown function name

};

// INLINING ///////////////////////////////////////////////////////////////////////////////////////
inline HWND GUIEdit::getWindowHandle( void ) { return m_appHWnd; }
inline HINSTANCE GUIEdit::getInstance( void ) { return m_appInst; }
inline HWND GUIEdit::getStatusBarWindowHandle( void ) { return m_statusBarHWnd; }
inline EditMode GUIEdit::getMode( void ) { return m_mode; }
inline void GUIEdit::setPropertyTarget( GameWindow *window ) { m_propertyTarget = window; }
inline GameWindow *GUIEdit::getPropertyTarget( void ) { return m_propertyTarget; }
inline char *GUIEdit::getSaveFilename( void ) { return m_saveFilename; }
inline char *GUIEdit::getSavePathAndFilename( void ) { return m_savePathAndFilename; }
inline void GUIEdit::setGridResolution( Int res ) { m_gridResolution = res; }
inline Int GUIEdit::getGridResolution( void ) { return m_gridResolution; }
inline void GUIEdit::setGridVisible( Bool visible ) { m_gridVisible = visible; }
inline Bool GUIEdit::isGridVisible( void ) { return m_gridVisible; }
inline void GUIEdit::setGridSnap( Bool on ) { m_snapToGrid = on; }
inline Bool GUIEdit::isGridSnapOn( void ) { return m_snapToGrid; }
inline void GUIEdit::setGridColor( RGBColorInt *color ) { m_gridColor = *color; }
inline RGBColorInt *GUIEdit::getGridColor( void ) { return &m_gridColor; }
inline Bool GUIEdit::getShowHiddenOutlines( void ) { return m_showHiddenOutlines; }
inline Bool GUIEdit::getShowSeeThruOutlines( void ) { return m_showSeeThruOutlines; }
inline void GUIEdit::setLayoutInit( AsciiString init ) { m_layoutInitString = init; }
inline void GUIEdit::setLayoutUpdate( AsciiString update ) { m_layoutUpdateString = update; }
inline void GUIEdit::setLayoutShutdown( AsciiString shutdown ) { m_layoutShutdownString = shutdown; }
inline AsciiString GUIEdit::getLayoutInit( void ) { return m_layoutInitString; }
inline AsciiString GUIEdit::getLayoutUpdate( void ) { return m_layoutUpdateString; }
inline AsciiString GUIEdit::getLayoutShutdown( void ) { return m_layoutShutdownString; }

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern GUIEdit *TheEditor;  ///< editor application singleton

#endif // __GUIEDIT_H_

