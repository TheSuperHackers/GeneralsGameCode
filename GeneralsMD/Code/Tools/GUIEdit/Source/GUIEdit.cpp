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

// FILE: GUIEdit.cpp //////////////////////////////////////////////////////////
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
// File name:  GUIEdit.cpp
//
// Created:    Colin Day, July 2001
//
// Desc:       GUI Edit and window layout entry point
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "Common/NameKeyGenerator.h"
#include "Common/GameEngine.h"
#include "Common/GlobalData.h"
#include "GameClient/GlobalLanguage.h"
#include "Common/PlayerList.h"
#include "GameLogic/RankInfo.h"
#include "Common/FileSystem.h"
#include "Common/LocalFileSystem.h"
#include "Common/ArchiveFileSystem.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetProgressBar.h"
#include "GameClient/GadgetRadioButton.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetSlider.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GameText.h"
#include "GameClient/GadgetTabControl.h"
#include "GameClient/IMEManager.h"
#include "GameClient/InGameUI.h"
#include "GameClient/WindowXlat.h"
#include "GameClient/HeaderTemplate.h"

#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/GameClient/W3DGameWindowManager.h"
#include "W3DDevice/GameClient/W3DGameFont.h"
#include "W3DDevice/GameClient/W3DDisplayStringManager.h"
#include "GameClient/Keyboard.h"
#include "Win32Device/GameClient/Win32Mouse.h"
#include "Win32Device/GameClient/Win32DIKeyboard.h"
#include "Win32Device/Common/Win32LocalFileSystem.h"
#include "Win32Device/Common/Win32BIGFileSystem.h"

#include "resource.h"
#include "WinMain.h"
#include "GUIEdit.h"
#include "HierarchyView.h"
#include "EditWindow.h"
#include "GUIEditWindowManager.h"
#include "GUIEditDisplay.h"
#include "DialogProc.h"
#include "LayoutScheme.h"

///////////////////////////////////////////////////////////////////////////////
// DEFINES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define GUIEDIT_CONFIG_VERSION 4

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////
GUIEdit *TheEditor = NULL;

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GUIEdit::saveAsDialog ======================================================
/** Bring up the standard windows browser save as dialog and return
	* filename selected */
//=============================================================================
char *GUIEdit::saveAsDialog( void )
{
	static char filename[ _MAX_PATH ];
  OPENFILENAME ofn;
  Bool returnCode;
  char filter[] = "Window Files (*.wnd)\0*.wnd\0"  \
                  "All Files (*.*)\0*.*\0\0" ;

  ofn.lStructSize       = sizeof( OPENFILENAME );
  ofn.hwndOwner         = m_appHWnd;
  ofn.hInstance         = NULL;
  ofn.lpstrFilter       = filter;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter    = 0;
  ofn.nFilterIndex      = 0;
  ofn.lpstrFile         = filename;
  ofn.nMaxFile          = _MAX_PATH;
  ofn.lpstrFileTitle    = NULL;
  ofn.nMaxFileTitle     = 0;
  ofn.lpstrInitialDir   = NULL;
  ofn.lpstrTitle        = NULL;
  ofn.Flags             = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.nFileOffset       = 0;
  ofn.nFileExtension    = 0;
  ofn.lpstrDefExt       = "wnd";
  ofn.lCustData         = 0L ;
  ofn.lpfnHook          = NULL ;
  ofn.lpTemplateName    = NULL ;

  returnCode = GetSaveFileName( &ofn );

  if( returnCode )
		return filename;
	else
		return NULL;

}  // end saveAsDialog

// GUIEdit::openDialog ========================================================
/** Bring up the standard windows browser open dialog and return
	* filename selected */
//=============================================================================
char *GUIEdit::openDialog( void )
{
	static char filename[ _MAX_PATH ];
  OPENFILENAME ofn;
  Bool returnCode;
  char filter[] = "Window Files (*.wnd)\0*.wnd\0"  \
                  "All Files (*.*)\0*.*\0\0" ;

  ofn.lStructSize       = sizeof( OPENFILENAME );
  ofn.hwndOwner         = m_appHWnd;
  ofn.hInstance         = NULL;
  ofn.lpstrFilter       = filter;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter    = 0;
  ofn.nFilterIndex      = 0;
  ofn.lpstrFile         = filename;
  ofn.nMaxFile          = _MAX_PATH;
  ofn.lpstrFileTitle    = NULL;
  ofn.nMaxFileTitle     = 0;
  ofn.lpstrInitialDir   = NULL;
  ofn.lpstrTitle        = NULL;
  ofn.Flags             = OFN_NOREADONLYRETURN | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.nFileOffset       = 0;
  ofn.nFileExtension    = 0;
  ofn.lpstrDefExt       = "wnd";
  ofn.lCustData         = 0L ;
  ofn.lpfnHook          = NULL ;
  ofn.lpTemplateName    = NULL ;

  returnCode = GetOpenFileName( &ofn );

  if( returnCode )
		return filename;
	else
		return NULL;

}  // end openDialog

// GUIEdit::setUnsaved ========================================================
/** Set the current contents of the editor as either saved or unsaved,
	* when we're entering an unsaved state we will put a marker in the title
	* bar of the app, when leaving an unsaved state we will remove
	* that marker */
//=============================================================================
void GUIEdit::setUnsaved( Bool unsaved )
{
//	char *saveStatus = " *";
	const char *unsavedFilename = "New File";
	const char *filename;

	// which filename to use in title bar
	if( strlen( m_saveFilename ) == 0 )
		filename = unsavedFilename;
	else
		filename = m_saveFilename;

//	if( m_unsaved == FALSE && unsaved == TRUE )
	if( unsaved == TRUE )
	{
		char title[ 256 ];

		// entering unsaved state, place '*' in title bar
		sprintf( title, "GUIEdit:  %s  *", filename );
		SetWindowText( m_appHWnd, title );

	}  // end if
	//else if( m_unsaved == TRUE && unsaved == FALSE )
	else
	{
		char title[ 256 ];

		// leaving unsaved state, place '*' in title bar
		sprintf( title, "GUIEdit:  %s", filename );
		SetWindowText( m_appHWnd, title );

	}  // end else

	// save the new state we're in
	m_unsaved = unsaved;

}  // end setUnsaved

// GUIEdit::setSaveFile =======================================================
/** Set our member variables for the full path and filename passed in
	* to this method.  We will also extract the filename only from the
	* full path and save that separately */
//=============================================================================
void GUIEdit::setSaveFile( const char *fullPathAndFilename )
{
  const char *ptr;

	// copy over the full path and filename
	strcpy( m_savePathAndFilename, fullPathAndFilename );

	//
	// copy everything after the last '\' from the full path, this will
	// be just the filename with extension
	//
	ptr = strrchr( fullPathAndFilename, '\\' ) + 1;
	strcpy( m_saveFilename, ptr );

}  // end setSaveFile

// GUIEdit::validateParentForCreate ===========================================
/** This method is used when creating new windows and gadgets, if a
	* parent is present, that parent cannot be a GUI gadget because those
	* gadgets are atomic units, they can not contain user defined children */
//=============================================================================
Bool GUIEdit::validateParentForCreate( GameWindow *parent )
{

	//
	// if there is a parent, that parent cannot be a gadget control,
	// gadgets are units themselves and should have no user defined
	// children
	//
	if( parent && TheEditor->windowIsGadget( parent ) )
	{

		MessageBox( TheEditor->getWindowHandle(),
								"You cannot make a new window as a child to a GUI Gadget Control",
								"Illegal Parent", MB_OK );
		return FALSE;

	}  // end if

	return TRUE;  // ok

}  // end validateParentForCreate

// GUIEdit::findSelectionEntry ================================================
/** Find selection entry for this window */
//=============================================================================
WindowSelectionEntry *GUIEdit::findSelectionEntry( GameWindow *window )
{
	WindowSelectionEntry *entry;

	// sanity
	if( window == NULL )
		return NULL;

	// search the list
	entry = m_selectList;
	while( entry )
	{

		if( entry->window == window )
			return entry;

		// next entry
		entry = entry->next;

	}  // end while

	return NULL;  // not found

}  // end findSelectionEntry

// GUIEdit::normalizeRegion ===================================================
/** Normalize the region so that lo and hi are actually lo and hi */
//=============================================================================
void GUIEdit::normalizeRegion( IRegion2D *region )
{
	ICoord2D temp;

	if( region->hi.x < region->lo.x )
	{

		if( region->hi.y < region->lo.y )
		{

			temp = region->hi;
			region->hi = region->lo;
			region->lo = temp;

		}  // end if
		else
		{

			temp = region->hi;
			region->hi.x = region->lo.x;
			region->lo.x = temp.x;

		}  // end else

	}  // end if
	else if( region->hi.y < region->lo.y )
	{

		temp = region->hi;
		region->hi.y = region->lo.y;
		region->lo.y = temp.y;

	}  // end if

}  // end normalizeRegion

// GUIEdit::selectWindowsInRegion =============================================
/** Select all the windows that are fully in the region */
//=============================================================================
void GUIEdit::selectWindowsInRegion( IRegion2D *region )
{
	GameWindow *window;
	ICoord2D origin, size;

	// sanity
	if( region == NULL )
		return;

	// unselect everything selected
	clearSelections();

	// normalize the region if needed
	normalizeRegion( region );

	// get the window list
	window = TheWindowManager->winGetWindowList();
	while( window )
	{

		// get the screen position and size of window
		window->winGetScreenPosition( &origin.x, &origin.y );
		window->winGetSize( &size.x, &size.y );

		// compare window extents to selection region
		if( origin.x >= region->lo.x &&
				origin.y >= region->lo.y &&
				origin.x + size.x <= region->hi.x &&
				origin.y + size.y <= region->hi.y )
		{

			// add to selection list
			selectWindow( window );

		}  // end if

		// go to next window
		window = window->winGetNext();

	}  // end while

}  // end selectWindowsInRegion

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GUIEdit::GUIEdit ===========================================================
/** */
//=============================================================================
GUIEdit::GUIEdit( void )
{

	m_appInst = 0;
	m_appHWnd = NULL;
	m_statusBarHWnd = NULL;
	m_toolbarHWnd = NULL;

	m_unsaved = FALSE;
	m_mode = MODE_UNDEFINED;
	strcpy( m_savePathAndFilename, "" );
	strcpy( m_saveFilename, "" );

	m_selectList = NULL;
	m_propertyTarget = NULL;

	m_gridVisible = TRUE;
	m_snapToGrid = TRUE;
	m_gridResolution		= 8;
	m_gridColor.red			= 112;
	m_gridColor.green		= 112;
	m_gridColor.blue		= 112;
	m_gridColor.alpha		= 64;
	m_showHiddenOutlines = TRUE;
	m_showSeeThruOutlines = TRUE;

	m_layoutInitString = GUIEDIT_NONE_STRING;
	m_layoutUpdateString = GUIEDIT_NONE_STRING;
	m_layoutShutdownString = GUIEDIT_NONE_STRING;
}  // end GUIEdit

// GUIEdit::~GUIEdit ==========================================================
/** */
//=============================================================================
GUIEdit::~GUIEdit( void )
{
	if (TheHeaderTemplateManager)
	{
		delete TheHeaderTemplateManager;
		TheHeaderTemplateManager = NULL;
	}

	if (TheGameText)
	{
		delete TheGameText;
		TheGameText = NULL;
	}

	// delete the IME Manager
//	if ( TheIMEManager )
//	{
//		delete TheIMEManager;
//		TheIMEManager = NULL;
//	}


	// all the shutdown routine
	shutdown();

}  // end ~GUIEdit

// GUIEdit::init ==============================================================
/** Initialize all the GUI edit data */
//=============================================================================
void GUIEdit::init( void )
{

	//
	// save the couple of globals for our app main window as locals for
	// convenient access
	//
	m_appInst = ApplicationHInstance;
	m_appHWnd = ApplicationHWnd;

	// add status bar to the bottom
	createStatusBar();

	// create the name key generator
	TheNameKeyGenerator = new NameKeyGenerator;
	TheNameKeyGenerator->init();

	// create file system
	TheFileSystem = new FileSystem;
	TheLocalFileSystem = new Win32LocalFileSystem;
	TheArchiveFileSystem = new Win32BIGFileSystem;
	TheFileSystem->init();

	TheGlobalLanguageData = new GlobalLanguage;
	TheGlobalLanguageData->init();
	//---------------------------------------------------------------------------
	// GUI tool specific initializations ----------------------------------------
	//---------------------------------------------------------------------------

	//
	// create the window for the rendered GUI interaction, the GUIEdit
	// program MUST run in the same resolution that the window files are
	// created in, we have chosen 800x600 for all our GUI creation (CDay)
	//
	TheEditWindow = new EditWindow;
//	TheEditWindow->init( 640, 480 );
	TheEditWindow->init( 800, 600 );
//	TheEditWindow->init( 1024, 768 );

	// create hierarchy view
	TheHierarchyView = new HierarchyView;
	TheHierarchyView->init();

	//---------------------------------------------------------------------------
	// Game engine specific initializations -------------------------------------
	//---------------------------------------------------------------------------

	// create the name key generator
	TheWritableGlobalData = new GlobalData;
	TheWritableGlobalData->init();

	// create the message stream
	TheMessageStream = new MessageStream;
	TheMessageStream->init();
	TheMessageStream->attachTranslator( new WindowTranslator, 10 );

	// create the command list
	TheCommandList = new CommandList;

	// create the function lexicon
	TheFunctionLexicon = new W3DFunctionLexicon;
	TheFunctionLexicon->init();
	TheFunctionLexicon->validate();

	// create the font library
	TheFontLibrary = new W3DFontLibrary;
	TheFontLibrary->init();

	// load the set of GUIEdit default fonts for controls
	loadGUIEditFontLibrary( TheFontLibrary );

	// create GUI image collection
	TheMappedImageCollection = new ImageCollection;
	TheMappedImageCollection->init();
	TheMappedImageCollection->load( 512 );

	// create display string manager
	TheDisplayStringManager = new W3DDisplayStringManager;
	TheDisplayStringManager->init();

	// create the window manager
	TheWindowManager = new GUIEditWindowManager;
	TheWindowManager->init();

	TheRankInfoStore = new RankInfoStore;
	TheRankInfoStore->init();

	ThePlayerList = new PlayerList;
	ThePlayerList->init();

	TheKeyboard = new DirectInputKeyboard;
	//
	// in order to easily expose the new methods for our own internal editor
	// use we will keep a pointer to what we KNOW to be a gui edit window man.
	//
	TheGUIEditWindowManager = static_cast<GUIEditWindowManager *>(TheWindowManager);

	// create the display
	TheDisplay = new GUIEditDisplay;
	TheDisplay->init();
	ICoord2D size;
	TheEditWindow->getSize( &size );
	TheDisplay->setWidth( size.x );
	TheDisplay->setHeight( size.y );

	// set our initial mode
	setMode( MODE_EDIT );

	// create the default scheme, this must be done after image collections are loaded
	TheDefaultScheme = new LayoutScheme;
	TheDefaultScheme->init();

	// read our configuration file
	readConfigFile( GUIEDIT_CONFIG_FILENAME );

	// read the available font definitions
	readFontFile( GUIEDIT_FONT_FILENAME );

	// create the mouse
	TheWin32Mouse = new Win32Mouse;
	TheMouse = TheWin32Mouse;
	TheMouse->init();

	// lastly just for testing
	TheWindowManager->initTestGUI();

	// load the layout scheme now read in from the cofig file
	TheDefaultScheme->loadScheme( TheDefaultScheme->getSchemeFilename() );

	// create the localized game text interface
	TheGameText = CreateGameTextInterface();
	if ( TheGameText )
	{
		TheGameText->init();
	}

	TheHeaderTemplateManager = new HeaderTemplateManager;
	if(TheHeaderTemplateManager)
		TheHeaderTemplateManager->init();
	// create the IME manager
//	TheIMEManager = CreateIMEManagerInterface();
//	if ( TheIMEManager )
//	{
//		TheIMEManager->init();
//	}
}  // end init

// GUIEdit::shutdown ==========================================================
/** Shutdown our GUI edit application */
//=============================================================================
void GUIEdit::shutdown( void )
{

	// write our configuration file
	writeConfigFile( GUIEDIT_CONFIG_FILENAME );

	// write our loaded fonts into the font file if we can
	writeFontFile( GUIEDIT_FONT_FILENAME );

	// delete all the selection entries
	clearSelections();

	// delete the display
	delete TheDisplay;
	TheDisplay = NULL;

	// delete all windows properly in the editor
	deleteAllWindows();

	// delete the mouse
	delete TheMouse;
	TheMouse = NULL;
	TheWin32Mouse = NULL;

	delete ThePlayerList;
	ThePlayerList = NULL;

	delete TheRankInfoStore;
	TheRankInfoStore = NULL;

	// delete the window manager
	delete TheWindowManager;
	TheWindowManager = NULL;
	TheGUIEditWindowManager = NULL;

	// delete display string manager
	delete TheDisplayStringManager;
	TheDisplayStringManager = NULL;

	// delete image collection
	delete TheMappedImageCollection;
	TheMappedImageCollection = NULL;

	// delete the font library
	delete TheFontLibrary;
	TheFontLibrary = NULL;

	delete TheCommandList;
	TheCommandList = NULL;

	delete TheMessageStream;
	TheMessageStream = NULL;

	// delete the function lexicon
	delete TheFunctionLexicon;
	TheFunctionLexicon = NULL;

	// delete name key generator
	delete TheNameKeyGenerator;
	TheNameKeyGenerator = NULL;

	delete TheGlobalLanguageData;
	TheGlobalLanguageData = NULL;

	// delete file system
	delete TheFileSystem;
	TheFileSystem = NULL;

	delete TheLocalFileSystem;
	TheLocalFileSystem = NULL;

	delete TheArchiveFileSystem;
	TheArchiveFileSystem = NULL;


	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// delete the hierarchy view
	delete TheHierarchyView;
	TheHierarchyView = NULL;

	// destroy the edit window
	delete TheEditWindow;
	TheEditWindow = NULL;


	delete TheKeyboard;
	TheKeyboard = NULL;


}  // end shutdown

// GUIEdit::update ============================================================
/** Update method for our GUI edit application */
//=============================================================================
void GUIEdit::update( void )
{

	// update mouse info
	TheMouse->update();

	// process the mouse if we're in test mode
	if( TheEditor->getMode() == MODE_TEST_RUN )
	{

		// send input through the window system and clear all messages after
		TheMouse->createStreamMessages();
		TheMessageStream->propagateMessages();
		TheCommandList->reset();

	}  // end if

	// update the window manager itself
	TheWindowManager->update();

	// draw the edit window
	TheEditWindow->draw();

}  // end update

// GUIEdit::writeConfigFile ===================================================
/** Write the guiedit config file */
//=============================================================================
Bool GUIEdit::writeConfigFile( const char *filename )
{
	FILE *fp;

	// open the file
	fp = fopen( filename, "w" );
	if( fp == NULL )
	{

		DEBUG_LOG(( "writeConfigFile: Unable to open file '%s'", filename ));
		assert( 0 );
		return FALSE;

	}  // end if

	// version
	fprintf( fp, "GUIEdit Config file version '%d'\n", GUIEDIT_CONFIG_VERSION );

	// edit window background color
	RGBColorReal backColor = TheEditWindow->getBackgroundColor();
	fprintf( fp, "BACKGROUNDCOLOR = %f %f %f %f\n",
					 backColor.red, backColor.green, backColor.blue, backColor.alpha );

	// grid settings
	fprintf( fp, "GRIDRESOLUTION = %d\n", TheEditor->getGridResolution() );
	RGBColorInt *gridColor = TheEditor->getGridColor();
	fprintf( fp, "GRIDCOLOR = %d %d %d %d\n",
					 gridColor->red, gridColor->green, gridColor->blue, gridColor->alpha );
	fprintf( fp, "SNAPTOGRID = %d\n", TheEditor->isGridSnapOn() );
	fprintf( fp, "GRIDVISIBLE = %d\n", TheEditor->isGridVisible() );

	// write hierarchy position and size
	ICoord2D pos, size;
	TheHierarchyView->getDialogPos( &pos );
	TheHierarchyView->getDialogSize( &size );
	fprintf( fp, "HIERARCHYPOSITION = %d %d\n", pos.x, pos.y );
	fprintf( fp, "HIERARCHYSIZE = %d %d\n", size.x, size.y );

	// hidden and see thru options
	fprintf( fp, "SHOWSEETHRUOUTLINES = %d\n", getShowSeeThruOutlines() );
	fprintf( fp, "SHOWHIDDENOUTLINES = %d\n", getShowHiddenOutlines() );

	// scheme file
	fprintf( fp, "SCHEMEFILE = %s\n", TheDefaultScheme->getSchemeFilename() );

	// close the file
	fclose( fp );

	return TRUE;

}  // end writeConfigFile

// GUIEdit::readConfigFile ====================================================
/** Read the guiedit config file */
//=============================================================================
Bool GUIEdit::readConfigFile( const char *filename )
{
	FILE *fp;

	// open the file
	fp = fopen( filename, "r" );
	if( fp == NULL )
		return TRUE;

	// version
	Int version;
	fscanf( fp, "GUIEdit Config file version '%d'\n", &version );
	if( version != GUIEDIT_CONFIG_VERSION )
		return FALSE;  // version has changed, use defaults again

	// edit window background color
	RGBColorReal backColor;
	fscanf( fp, "BACKGROUNDCOLOR = %f %f %f %f\n",
				  &backColor.red, &backColor.green, &backColor.blue, &backColor.alpha );
	TheEditWindow->setBackgroundColor( backColor );

	// grid settings
	Int intData;
	fscanf( fp, "GRIDRESOLUTION = %d\n", &intData );
	TheEditor->setGridResolution( intData );
	RGBColorInt gridColor;
	fscanf( fp, "GRIDCOLOR = %d %d %d %d\n",
					&gridColor.red, &gridColor.green, &gridColor.blue, &gridColor.alpha );
	TheEditor->setGridColor( &gridColor );
	fscanf( fp, "SNAPTOGRID = %d\n", &intData );
	TheEditor->setGridSnap( intData );
	fscanf( fp, "GRIDVISIBLE = %d\n", &intData );
	TheEditor->setGridVisible( intData );

	// hierarchy view
	ICoord2D pos, size;
	fscanf( fp, "HIERARCHYPOSITION = %d %d\n", &pos.x, &pos.y );
	fscanf( fp, "HIERARCHYSIZE = %d %d\n", &size.x, &size.y );
	TheHierarchyView->setDialogPos( &pos );
	TheHierarchyView->setDialogSize( &size );

	// hidden and see thru options
	Int show;
	fscanf( fp, "SHOWSEETHRUOUTLINES = %d\n", &show );
	setShowSeeThruOutlines( show );
	fscanf( fp, "SHOWHIDDENOUTLINES = %d\n", &show );
	setShowHiddenOutlines( show );

	// scheme file
	char file[ _MAX_PATH ];
	if (fscanf( fp, "SCHEMEFILE = %s\n", file ) == 1)
	{
		TheDefaultScheme->setSchemeFilename( file );
	}

	// close the file
	fclose( fp );

	return TRUE;

}  // end readConfigFile

// GUIEdit::readFontFile ======================================================
/** Read the font file defintitions and load them */
//=============================================================================
void GUIEdit::readFontFile( const char *filename )
{
	FILE *fp;

	// sanity
	if( filename == NULL )
		return;

	// open the file
	fp = fopen( filename, "r" );
	if( fp == NULL )
		return;

	// read how many entries follow
	Int fontCount;
	fscanf( fp, "AVAILABLE_FONT_COUNT = %d\n", &fontCount );

	for( Int i = 0; i < fontCount; i++ )
	{

		// read all the font defitions
		char fontBuffer[ 512 ];
		Int size, bold;
		char c = fgetc( fp );

		// skip past the first quote
		while( c != '\"' )
			c = fgetc( fp );
		c = fgetc( fp );  // the quote itself

		// copy the name till the next quote is read
		Int index = 0;
		while( c != '\"' )
		{

			fontBuffer[ index++ ] = c;
			c = fgetc( fp );

		}  // end while
		fontBuffer[ index ] = '\0';
		c = fgetc( fp );  // the end quite itself

		// read the size and bold data elements
		fscanf( fp, " Size: %d Bold: %d\n", &size, &bold );

		// set the font
		GameFont *font = TheFontLibrary->getFont( AsciiString(fontBuffer), size, bold );
		if( font == NULL )
		{
			char buffer[ 1024 ];

			sprintf( buffer, "Warning: The font '%s' Size: '%d' Bold: '%d', specified in the config file could not be loaded.  Does that font exist?",
							 fontBuffer, size, bold );
			MessageBox( m_appHWnd, buffer, "Cannot Load Font", MB_OK );

		}  // end if

	}  // end for i

	// close the file
	fclose( fp );

}  // end readFontFile

// GUIEdit::writeFontFile =====================================================
/** If we can, write a file containing a definition of all the fonts
	* we have loaded */
//=============================================================================
void GUIEdit::writeFontFile( const char *filename )
{
	FILE *fp;

	// sanity
	if( filename == NULL )
		return;

	// open the file
	fp = fopen( filename, "w" );
	if( fp == NULL )
		return;  // dont bother making an error, it's likely to be read only a lot

	// available fonts
	Int count = TheFontLibrary->getCount();
	fprintf( fp, "AVAILABLE_FONT_COUNT = %d\n", count );

	GameFont *font;
	for( font = TheFontLibrary->firstFont();
			 font;
			 font = TheFontLibrary->nextFont( font ) )
	{
		fprintf( fp, "AVAILABLEFONT = \"%s\" Size: %d Bold: %d\n",
						 font->nameString.str(), font->pointSize, font->bold );
	}  // end for

	// close the file
	fclose( fp );

}  // end writeFontFile

// GUIEdit::setMode ===========================================================
/** Set a new mode for the editor */
//=============================================================================
void GUIEdit::setMode( EditMode mode )
{
	static TranslatorID transID = 0;

	// if same mode do nothing
	if( m_mode == mode )
		return;

	// if leaving test mode, remove the translator for test input
	if( m_mode == MODE_TEST_RUN )
	{

		TheMessageStream->removeTranslator( transID );
		transID = 0;

	}  // end if

	// assign new mode
	m_mode = mode;

	// uncheck menu item for test mode by default
	unCheckMenuItem( MENU_TEST_MODE );

	// update status message
	switch( m_mode )
	{

		// ------------------------------------------------------------------------
		case MODE_TEST_RUN:
			statusMessage( STATUS_MODE, "Mode: Test Run" );
			// check menu item for test mode
			checkMenuItem( MENU_TEST_MODE );

			// attach window translator to stream for testing
			transID = TheMessageStream->attachTranslator( new WindowTranslator, 10 );

			break;

		// ------------------------------------------------------------------------
		case MODE_EDIT:
			setCursor( CURSOR_NORMAL );
			statusMessage( STATUS_MODE, "Ready" );
			break;

		// ------------------------------------------------------------------------
		case MODE_DRAG_MOVE:
			setCursor( CURSOR_MOVE );
			statusMessage( STATUS_MODE, "Drag window with mouse" );
			break;

		// ------------------------------------------------------------------------
		case MODE_KEYBOARD_MOVE:
			//setCursor( CURSOR_MOVE );
			statusMessage( STATUS_MODE, "Hit enter to accept or Esc to cancel" );
			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_TOP_LEFT:
			setCursor( CURSOR_SIZE_NWSE );
			statusMessage( STATUS_MODE, "Drag top left corner to resize" );
			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_TOP_RIGHT:
			setCursor( CURSOR_SIZE_NESW );
			statusMessage( STATUS_MODE, "Drag top right corner to resize" );
			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_BOTTOM_RIGHT:
			setCursor( CURSOR_SIZE_NWSE );
			statusMessage( STATUS_MODE, "Drag bottom right corner to resize" );
			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_BOTTOM_LEFT:
			setCursor( CURSOR_SIZE_NESW );
			statusMessage( STATUS_MODE, "Drag bottom left corner to resize" );
			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_TOP:
			setCursor( CURSOR_SIZE_NS );
			statusMessage( STATUS_MODE, "Drag top to resize" );
			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_BOTTOM:
			setCursor( CURSOR_SIZE_NS );
			statusMessage( STATUS_MODE, "Drag bottom to resize" );
			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_LEFT:
			setCursor( CURSOR_SIZE_WE );
			statusMessage( STATUS_MODE, "Drag left to resize" );
			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_RIGHT:
			setCursor( CURSOR_SIZE_WE );
			statusMessage( STATUS_MODE, "Drag right to resize" );
			break;

	}  // end switch

}  // end setMode

// GUIEdit::setCursor =========================================================
/** Set the cursor to the specified type */
//=============================================================================
void GUIEdit::setCursor( CursorType type )
{
	char *identifier;

	switch( type )
	{

		// ------------------------------------------------------------------------
		case CURSOR_NORMAL:
			identifier = IDC_ARROW;
			break;

		// ------------------------------------------------------------------------
		case CURSOR_MOVE:
			identifier = IDC_SIZEALL;
			break;

		// ------------------------------------------------------------------------
		case CURSOR_SIZE_NESW:
			identifier = IDC_SIZENESW;
			break;

		// ------------------------------------------------------------------------
		case CURSOR_SIZE_NS:
			identifier = IDC_SIZENS;
			break;

		// ------------------------------------------------------------------------
		case CURSOR_SIZE_NWSE:
			identifier = IDC_SIZENWSE;
			break;

		// ------------------------------------------------------------------------
		case CURSOR_SIZE_WE:
			identifier = IDC_SIZEWE;
			break;

		// ------------------------------------------------------------------------
		case CURSOR_WAIT:
			identifier = IDC_WAIT;
			break;

	}  // end switchType

	// set the new cursor
	SetCursor( LoadCursor( NULL, identifier ) );

}  // end setCursor

// pointInChild ===============================================================
/** Given a mouse position, get the topmost window at that location. We needed
		to pull this out into it's own special case function since the standard
		winPointInAnyChild is used by the game and we needed a special case to
		select a disabled control in GUIEdit																		*/
//=============================================================================
static GameWindow *pointInChild( Int x, Int y , GameWindow *win)
{
	GameWindow *parent;
	GameWindow *child;
	ICoord2D origin;
	IRegion2D tempRegion;
	Int tempX, tempY;
	for( child = win->winGetChild(); child; child = child->winGetNext() )
	{

		child->winGetRegion(&tempRegion);
		origin = tempRegion.lo;
		parent = child->winGetParent();

		while( parent )
		{
			parent->winGetRegion(&tempRegion);
			origin.x += tempRegion.lo.x;
			origin.y += tempRegion.lo.y;
			parent = parent->winGetParent();

		}  // end while
		child->winGetSize(&tempX,&tempY);
		if( x >= origin.x && x <= origin.x + tempX &&
				y >= origin.y && y <= origin.y + tempY &&
				BitIsSet( child->winGetStatus(), WIN_STATUS_HIDDEN ) == FALSE)
			return child->winPointInChild( x, y );

	}  // end for child

	// not in any children, must be in parent
	return win;

}  // end pointInChild

// pointInAnyChild ============================================================
/** Given a mouse position, get the topmost window at that location. We needed
		to pull this out into it's own special case function since the standard
		winPointInAnyChild is used by the game and we needed a special case to
		select a disabled control in GUIEdit																		*/
//=============================================================================
static GameWindow *pointInAnyChild( Int x, Int y, Bool ignoreHidden, GameWindow *win )
{
	GameWindow *parent;
	GameWindow *child;
	ICoord2D origin;
	IRegion2D tempRegion;
	Int tempX, tempY;
	for( child = win->winGetChild(); child; child = child->winGetNext() )
	{
		child->winGetRegion(&tempRegion);
		origin = tempRegion.lo;
		parent = child->winGetParent();

		while( parent )
		{

			parent->winGetRegion(&tempRegion);
			origin.x += tempRegion.lo.x;
			origin.y += tempRegion.lo.y;
			parent = parent->winGetParent();

		}  // end while
		child->winGetSize(&tempX,&tempY);
		if( x >= origin.x && x <= origin.x + tempX &&
				y >= origin.y && y <= origin.y + tempY )
		{

			if( !(ignoreHidden == TRUE &&	BitIsSet( child->winGetStatus(), WIN_STATUS_HIDDEN )) )
				return pointInChild( x, y , child);

		}  // end if

	}  // end for child

	// not in any children, must be in parent
	return win;

}  // end pointInAnyChild

// GUIEdit::getWindowAtPos ====================================================
/** Given a mouse position, get the topmost window at that location */
//=============================================================================
GameWindow *GUIEdit::getWindowAtPos( Int x, Int y )
{
	GameWindow *window;
	GameWindow *pick = NULL;
	IRegion2D region;

	for( window = TheWindowManager->winGetWindowList();
			 window;
			 window = window->winGetNext() )
	{

		// get window region
		window->winGetRegion( &region );

		if( x >= region.lo.x && x <= region.hi.x &&
				y >= region.lo.y && y <= region.hi.y )
		{

			pick = pointInAnyChild( x, y, FALSE, window );
			break;  // exit for

		}  // end if

	}  // end for

	//
	// gadget controls are just composed of generic windows and buttons,
	// we will not allow you to select these componenets themselves in a gadget
	// because the gadget is the atomic "unit" as far as GUI editing goes.
	// If we've picked a gadget component we will instead just return the gadget
	// itself
	//
	if( pick )
	{
		GameWindow *parent = pick->winGetParent();

		//
		// all gadget components are children of their parent, therefore they
		// all have a parent
		//
		if( parent )
		{

			if( BitIsSet( parent->winGetStyle(), GWS_VERT_SLIDER |
																					GWS_HORZ_SLIDER |
																					GWS_SCROLL_LISTBOX |
																					GWS_COMBO_BOX |
																					GWS_TAB_CONTROL
									)
				)
			{

				// the parent is what we care about
				pick = parent;

				//
				// the parent is the thing we want to return here, unless we clicked
				// on the thumb of a scroll list box, that element is implemented as
				// a slider, therefore in that situation only we want to return the
				// parent of the slider
				//
				if( BitIsSet( parent->winGetStyle(), GWS_HORZ_SLIDER |
																						GWS_VERT_SLIDER ) )
				{
					GameWindow *grandParent = parent->winGetParent();

					if( grandParent && BitIsSet( grandParent->winGetStyle(),
																			GWS_SCROLL_LISTBOX ) )
						pick = grandParent;

				}  // end if

				//must check to see of the parent of a scroll box is a combo box
				if(BitIsSet(pick->winGetStyle(), GWS_SCROLL_LISTBOX))
				{
					GameWindow *grandParent = pick->winGetParent();
					if( grandParent && BitIsSet( grandParent->winGetStyle(), GWS_COMBO_BOX))
						pick = grandParent;
				}
			}  // end if

		}  // end if

	}  // end if

	return pick;

}  // end getWindowAtPos

// GUIEdit::clipCreationParamsToParent ========================================
/** when creating child windows we don't want them to exist outside the
	* parent so we use this to clip them to the the parent size and
	* locations */
//=============================================================================
void GUIEdit::clipCreationParamsToParent( GameWindow *parent,
																					Int *x, Int *y,
																					Int *width, Int *height )
{
	IRegion2D parentScreenRegion;
	ICoord2D parentSize;
	ICoord2D parentPos;
	Int newX, newY,
			newWidth, newHeight;

	// sanity
	if( parent == NULL ||
			x == NULL || y == NULL ||
			width == NULL || height == NULL )
		return;

	// get parent screen region and size
	parent->winGetScreenPosition( &parentPos.x, &parentPos.y );
	parent->winGetSize( &parentSize.x, &parentSize.y );

	//
	// compute the screen region, note this is NOT the same as the region
	// that comes back from winGetRegion, those regions are relative to their
	// parents
	//
	parentScreenRegion.lo.x = parentPos.x;
	parentScreenRegion.lo.y = parentPos.y;
	parentScreenRegion.hi.x = parentPos.x + parentSize.x;
	parentScreenRegion.hi.y = parentPos.y + parentSize.y;

	//
	// the new window position is really the difference between where
	// we clicked and the parent location
	//
	newX = *x - parentScreenRegion.lo.x;
	newY = *y - parentScreenRegion.lo.y;

	// new child windows must stay contained within their parent
	newWidth = *width;
	if( *x + *width > parentScreenRegion.hi.x )
		newWidth = parentSize.x - newX;

	newHeight = *height;
	if( *y + *height > parentScreenRegion.hi.y )
		newHeight = parentSize.y - newY;

	*x = newX;
	*y = newY;
	*width = newWidth;
	*height= newHeight;

}  // end clipcreationParamsToParent

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GUIEdit::deleteAllWindows ==================================================
/** Delete all windows in the editor */
//=============================================================================
void GUIEdit::deleteAllWindows( void )
{
	GameWindow *window, *next;

	window = TheWindowManager->winGetWindowList();
	while( window )
	{

		next = window->winGetNext();
		deleteWindow( window );
		window = next;

	}  // end while

}  // end deleteAllWindows

// GUIEdit::removeWindowCleanup ===============================================
/** This is called on each window before it is deleted or removed from
	* the active windows in the editor to allow the system to
	* cleanup before the delete */
//=============================================================================
void GUIEdit::removeWindowCleanup( GameWindow *window )
{

	// end of recursion
	if( window == NULL )
		return;

	//
	// unselect the window if selected, this should be done first as further
	// operations like deleting the tree entry change selections
	//
	unSelectWindow( window );

	// remove this window from the hierarchy
	TheHierarchyView->removeWindow( window );

	// take this out of the property target if present
	if( m_propertyTarget == window )
		m_propertyTarget = NULL;

	// notify the edit window this is going away
	TheEditWindow->notifyWindowDeleted( window );

	//
	// any children of this window will be destroyed as well, do cleanup
	// for them as well
	//
	GameWindow *child;
	for( child = window->winGetChild(); child; child = child->winGetNext() )
		removeWindowCleanup( child );

}  // end removeWindowCleanup

// GUIEdit::deleteWindow ======================================================
/** Delete the window from the editor */
//=============================================================================
void GUIEdit::deleteWindow( GameWindow *window )
{

	// have the editor cleanup all references to this window
	removeWindowCleanup( window );

	// destroy the game winodow
	TheWindowManager->winDestroy( window );

	// we've changed contents
	setUnsaved( TRUE );

}  // end deleteWindow

// GUIEdit::newWindow =========================================================
/** Create a new window of the specified type.  This is the single creation
	* point for all windows created in the editor */
//=============================================================================
GameWindow *GUIEdit::newWindow( UnsignedInt windowStyle,
																GameWindow *parent,
																Int x, Int y,
																Int width, Int height )
{
	GameWindow *window = NULL;

	// create the appropriate window based on style bit passed in
	switch( windowStyle )
	{

		// ------------------------------------------------------------------------
		case GWS_USER_WINDOW:
			window = newUserWindow( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_PUSH_BUTTON:
			window = newPushButton( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_CHECK_BOX:
			window = newCheckBox( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_RADIO_BUTTON:
			window = newRadioButton( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_TAB_CONTROL:
			window = newTabControl( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_HORZ_SLIDER:
			window = newHorizontalSlider( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_VERT_SLIDER:
			window = newVerticalSlider( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_SCROLL_LISTBOX:
			window = newListbox( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_COMBO_BOX:
			window = newComboBox( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_PROGRESS_BAR:
			window = newProgressBar( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_ENTRY_FIELD:
			window = newTextEntry( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		case GWS_STATIC_TEXT:
			window = newStaticText( parent, x, y, width, height );
			break;

		// ------------------------------------------------------------------------
		default:
			MessageBox( m_appHWnd, "Unknown window type to newWindow()", "ERROR",
									MB_OK | MB_ICONERROR );
			break;

	}  // end switch

	return window;

}  // end newWindow

// GUIEdit::newUserWindow =====================================================
/** Create a new window at the given location */
//=============================================================================
GameWindow *GUIEdit::newUserWindow( GameWindow *parent, Int x, Int y,
																		Int width, Int height )
{
	UnsignedInt status = WIN_STATUS_ENABLED;
	GameWindow *window;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	//
	// if there is a parent present we need to translate the screen x and y
	// location to coords that are local to the parent window
	//
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// create the new window
	window = TheWindowManager->winCreate( parent, status,
																				x, y,
																				width, height,
																				NULL, NULL );

	// a window created in the editor here is a user window
	WinInstanceData *instData = window->winGetInstanceData();
	BitSet( instData->m_style, GWS_USER_WINDOW );

	// set default colors based on the default scheme
	ImageAndColorInfo *info;

	info = TheDefaultScheme->getImageAndColor( GENERIC_ENABLED );
	window->winSetEnabledImage( 0, info->image );
	window->winSetEnabledColor( 0, info->color );
	window->winSetEnabledBorderColor( 0, info->borderColor );

	info = TheDefaultScheme->getImageAndColor( GENERIC_DISABLED );
	window->winSetDisabledImage( 0, info->image );
	window->winSetDisabledColor( 0, info->color );
	window->winSetDisabledBorderColor( 0, info->borderColor );

	info = TheDefaultScheme->getImageAndColor( GENERIC_HILITE );
	window->winSetHiliteImage( 0, info->image );
	window->winSetHiliteColor( 0, info->color );
	window->winSetHiliteBorderColor( 0, info->borderColor );

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}  // end newUserWindow

// GUIEdit::newPushButton =====================================================
/** Create a new push button */
//=============================================================================
GameWindow *GUIEdit::newPushButton( GameWindow *parent,
																		Int x, Int y, Int width, Int height )
{
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED | WIN_STATUS_IMAGE;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep the button inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_PUSH_BUTTON | GWS_MOUSE_TRACK;
	instData.m_textLabelString = "Button";
	window = TheWindowManager->gogoGadgetPushButton( parent,
																									 status,
																									 x, y,
																									 width, height,
																									 &instData,
																									 NULL,
																									 TRUE );


	// set default colors based on the default scheme
	if( window )
	{
		ImageAndColorInfo *info;

		info = TheDefaultScheme->getImageAndColor( BUTTON_ENABLED );
		GadgetButtonSetEnabledImage( window, info->image );
		GadgetButtonSetEnabledColor( window, info->color );
		GadgetButtonSetEnabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( BUTTON_ENABLED_PUSHED );
		GadgetButtonSetEnabledSelectedImage( window, info->image );
		GadgetButtonSetEnabledSelectedColor( window, info->color );
		GadgetButtonSetEnabledSelectedBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( BUTTON_DISABLED );
		GadgetButtonSetDisabledImage( window, info->image );
		GadgetButtonSetDisabledColor( window, info->color );
		GadgetButtonSetDisabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( BUTTON_DISABLED_PUSHED );
		GadgetButtonSetDisabledSelectedImage( window, info->image );
		GadgetButtonSetDisabledSelectedColor( window, info->color );
		GadgetButtonSetDisabledSelectedBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( BUTTON_HILITE );
		GadgetButtonSetHiliteImage( window, info->image );
		GadgetButtonSetHiliteColor( window, info->color );
		GadgetButtonSetHiliteBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( BUTTON_HILITE_PUSHED );
		GadgetButtonSetHiliteSelectedImage( window, info->image );
		GadgetButtonSetHiliteSelectedColor( window, info->color );
		GadgetButtonSetHiliteSelectedBorderColor( window, info->borderColor );

		Color color, border;

		color = TheDefaultScheme->getEnabledTextColor();
		border = TheDefaultScheme->getEnabledTextBorderColor();
		window->winSetEnabledTextColors( color, border );

		color = TheDefaultScheme->getDisabledTextColor();
		border = TheDefaultScheme->getDisabledTextBorderColor();
		window->winSetDisabledTextColors( color, border );

		color = TheDefaultScheme->getHiliteTextColor();
		border = TheDefaultScheme->getHiliteTextBorderColor();
		window->winSetHiliteTextColors( color, border );

		// set default font
		window->winSetFont( TheDefaultScheme->getFont() );

	}  // end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}  // end newPushButton

// GUIEdit::newCheckBox =======================================================
/** Create a new check box */
//=============================================================================
GameWindow *GUIEdit::newCheckBox( GameWindow *parent,
																	Int x, Int y, Int width, Int height )
{
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED | WIN_STATUS_IMAGE;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_CHECK_BOX | GWS_MOUSE_TRACK;
	instData.m_textLabelString = "Check";

	// create gadget
	window = TheWindowManager->gogoGadgetCheckbox( parent,
																							   status,
																								 x, y,
																								 width, height,
																								 &instData,
																								 NULL,
																								 TRUE );

	// set default colors based on the default scheme
	if( window )
	{
		ImageAndColorInfo *info;

		info = TheDefaultScheme->getImageAndColor( CHECK_BOX_ENABLED );
		GadgetCheckBoxSetEnabledImage( window, info->image );
		GadgetCheckBoxSetEnabledColor( window, info->color );
		GadgetCheckBoxSetEnabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( CHECK_BOX_ENABLED_UNCHECKED_BOX );
		GadgetCheckBoxSetEnabledUncheckedBoxImage( window, info->image );
		GadgetCheckBoxSetEnabledUncheckedBoxColor( window, info->color );
		GadgetCheckBoxSetEnabledUncheckedBoxBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( CHECK_BOX_ENABLED_CHECKED_BOX );
		GadgetCheckBoxSetEnabledCheckedBoxImage( window, info->image );
		GadgetCheckBoxSetEnabledCheckedBoxColor( window, info->color );
		GadgetCheckBoxSetEnabledCheckedBoxBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( CHECK_BOX_DISABLED );
		GadgetCheckBoxSetDisabledImage( window, info->image );
		GadgetCheckBoxSetDisabledColor( window, info->color );
		GadgetCheckBoxSetDisabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( CHECK_BOX_DISABLED_UNCHECKED_BOX );
		GadgetCheckBoxSetDisabledUncheckedBoxImage( window, info->image );
		GadgetCheckBoxSetDisabledUncheckedBoxColor( window, info->color );
		GadgetCheckBoxSetDisabledUncheckedBoxBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( CHECK_BOX_DISABLED_CHECKED_BOX );
		GadgetCheckBoxSetDisabledCheckedBoxImage( window, info->image );
		GadgetCheckBoxSetDisabledCheckedBoxColor( window, info->color );
		GadgetCheckBoxSetDisabledCheckedBoxBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( CHECK_BOX_HILITE );
		GadgetCheckBoxSetHiliteImage( window, info->image );
		GadgetCheckBoxSetHiliteColor( window, info->color );
		GadgetCheckBoxSetHiliteBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( CHECK_BOX_HILITE_UNCHECKED_BOX );
		GadgetCheckBoxSetHiliteUncheckedBoxImage( window, info->image );
		GadgetCheckBoxSetHiliteUncheckedBoxColor( window, info->color );
		GadgetCheckBoxSetHiliteUncheckedBoxBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( CHECK_BOX_HILITE_CHECKED_BOX );
		GadgetCheckBoxSetHiliteCheckedBoxImage( window, info->image );
		GadgetCheckBoxSetHiliteCheckedBoxColor( window, info->color );
		GadgetCheckBoxSetHiliteCheckedBoxBorderColor( window, info->borderColor );

		Color color, border;

		color = TheDefaultScheme->getEnabledTextColor();
		border = TheDefaultScheme->getEnabledTextBorderColor();
		window->winSetEnabledTextColors( color, border );

		color = TheDefaultScheme->getDisabledTextColor();
		border = TheDefaultScheme->getDisabledTextBorderColor();
		window->winSetDisabledTextColors( color, border );

		color = TheDefaultScheme->getHiliteTextColor();
		border = TheDefaultScheme->getHiliteTextBorderColor();
		window->winSetHiliteTextColors( color, border );

		// set default font
		window->winSetFont( TheDefaultScheme->getFont() );

	}  // end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}  // end newCheckBox

// GUIEdit::newRadioButton ====================================================
/** Create a new radio button */
//=============================================================================
GameWindow *GUIEdit::newRadioButton( GameWindow *parent,
																		 Int x, Int y, Int width, Int height )
{
	RadioButtonData radioData;
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED | WIN_STATUS_IMAGE;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_RADIO_BUTTON | GWS_MOUSE_TRACK;
	memset( &radioData, 0, sizeof( radioData ) );
	instData.m_textLabelString = "Radio";

	// create gadget
	window = TheWindowManager->gogoGadgetRadioButton( parent,
																									  status,
																									  x, y,
																									  width, height,
																									  &instData,
																										&radioData,
																									  NULL,
																									  TRUE );

	// set default colors based on the default scheme
	if( window )
	{
		ImageAndColorInfo *info;

		info = TheDefaultScheme->getImageAndColor( RADIO_ENABLED );
		GadgetRadioSetEnabledImage( window, info->image );
		GadgetRadioSetEnabledColor( window, info->color );
		GadgetRadioSetEnabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( RADIO_ENABLED_UNCHECKED_BOX );
		GadgetRadioSetEnabledUncheckedBoxImage( window, info->image );
		GadgetRadioSetEnabledUncheckedBoxColor( window, info->color );
		GadgetRadioSetEnabledUncheckedBoxBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( RADIO_ENABLED_CHECKED_BOX );
		GadgetRadioSetEnabledCheckedBoxImage( window, info->image );
		GadgetRadioSetEnabledCheckedBoxColor( window, info->color );
		GadgetRadioSetEnabledCheckedBoxBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( RADIO_DISABLED );
		GadgetRadioSetDisabledImage( window, info->image );
		GadgetRadioSetDisabledColor( window, info->color );
		GadgetRadioSetDisabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( RADIO_DISABLED_UNCHECKED_BOX );
		GadgetRadioSetDisabledUncheckedBoxImage( window, info->image );
		GadgetRadioSetDisabledUncheckedBoxColor( window, info->color );
		GadgetRadioSetDisabledUncheckedBoxBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( RADIO_DISABLED_CHECKED_BOX );
		GadgetRadioSetDisabledCheckedBoxImage( window, info->image );
		GadgetRadioSetDisabledCheckedBoxColor( window, info->color );
		GadgetRadioSetDisabledCheckedBoxBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( RADIO_HILITE );
		GadgetRadioSetHiliteImage( window, info->image );
		GadgetRadioSetHiliteColor( window, info->color );
		GadgetRadioSetHiliteBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( RADIO_HILITE_UNCHECKED_BOX );
		GadgetRadioSetHiliteUncheckedBoxImage( window, info->image );
		GadgetRadioSetHiliteUncheckedBoxColor( window, info->color );
		GadgetRadioSetHiliteUncheckedBoxBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( RADIO_HILITE_CHECKED_BOX );
		GadgetRadioSetHiliteCheckedBoxImage( window, info->image );
		GadgetRadioSetHiliteCheckedBoxColor( window, info->color );
		GadgetRadioSetHiliteCheckedBoxBorderColor( window, info->borderColor );

		Color color, border;

		color = TheDefaultScheme->getEnabledTextColor();
		border = TheDefaultScheme->getEnabledTextBorderColor();
		window->winSetEnabledTextColors( color, border );

		color = TheDefaultScheme->getDisabledTextColor();
		border = TheDefaultScheme->getDisabledTextBorderColor();
		window->winSetDisabledTextColors( color, border );

		color = TheDefaultScheme->getHiliteTextColor();
		border = TheDefaultScheme->getHiliteTextBorderColor();
		window->winSetHiliteTextColors( color, border );

		// set default font
		window->winSetFont( TheDefaultScheme->getFont() );

	}  // end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}  // end newRadioButton

// GUIEdit::newTabControl ====================================================
/** Create a tab control gadget */
//=============================================================================
GameWindow *GUIEdit::newTabControl( GameWindow *parent,
																		 Int x, Int y, Int width, Int height )
{
	TabControlData tabControlData;
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_TAB_CONTROL | GWS_MOUSE_TRACK;
	memset( &tabControlData, 0, sizeof( tabControlData ) );

	tabControlData.tabOrientation = TP_TOPLEFT;
	tabControlData.tabEdge = TP_TOP_SIDE;
	tabControlData.tabWidth = 50;
	tabControlData.tabHeight = 25;
	tabControlData.tabCount = 2;
	tabControlData.paneBorder = 5;

	// create gadget
	window = TheWindowManager->gogoGadgetTabControl( parent,
																									  status,
																									  x, y,
																									  width, height,
																									  &instData,
																										&tabControlData,
																									  NULL,
																									  TRUE );

	// set default colors based on the default scheme
	if( window )
	{
		ImageAndColorInfo *info;

		info = TheDefaultScheme->getImageAndColor( TC_TAB_0_ENABLED );
		GadgetTabControlSetEnabledImageTabZero( window, info->image );
		GadgetTabControlSetEnabledColorTabZero( window, info->color );
		GadgetTabControlSetEnabledBorderColorTabZero( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_1_ENABLED );
		GadgetTabControlSetEnabledImageTabOne( window, info->image );
		GadgetTabControlSetEnabledColorTabOne( window, info->color );
		GadgetTabControlSetEnabledBorderColorTabOne( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_2_ENABLED );
		GadgetTabControlSetEnabledImageTabTwo( window, info->image );
		GadgetTabControlSetEnabledColorTabTwo( window, info->color );
		GadgetTabControlSetEnabledBorderColorTabTwo( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_3_ENABLED );
		GadgetTabControlSetEnabledImageTabThree( window, info->image );
		GadgetTabControlSetEnabledColorTabThree( window, info->color );
		GadgetTabControlSetEnabledBorderColorTabThree( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_4_ENABLED );
		GadgetTabControlSetEnabledImageTabFour( window, info->image );
		GadgetTabControlSetEnabledColorTabFour( window, info->color );
		GadgetTabControlSetEnabledBorderColorTabFour( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_5_ENABLED );
		GadgetTabControlSetEnabledImageTabFive( window, info->image );
		GadgetTabControlSetEnabledColorTabFive( window, info->color );
		GadgetTabControlSetEnabledBorderColorTabFive( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_6_ENABLED );
		GadgetTabControlSetEnabledImageTabSix( window, info->image );
		GadgetTabControlSetEnabledColorTabSix( window, info->color );
		GadgetTabControlSetEnabledBorderColorTabSix( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_7_ENABLED );
		GadgetTabControlSetEnabledImageTabSeven( window, info->image );
		GadgetTabControlSetEnabledColorTabSeven( window, info->color );
		GadgetTabControlSetEnabledBorderColorTabSeven( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TAB_CONTROL_ENABLED );
		GadgetTabControlSetEnabledImageBackground( window, info->image );
		GadgetTabControlSetEnabledColorBackground( window, info->color );
		GadgetTabControlSetEnabledBorderColorBackground( window, info->borderColor );



		info = TheDefaultScheme->getImageAndColor( TC_TAB_0_DISABLED );
		GadgetTabControlSetDisabledImageTabZero( window, info->image );
		GadgetTabControlSetDisabledColorTabZero( window, info->color );
		GadgetTabControlSetDisabledBorderColorTabZero( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_1_DISABLED );
		GadgetTabControlSetDisabledImageTabOne( window, info->image );
		GadgetTabControlSetDisabledColorTabOne( window, info->color );
		GadgetTabControlSetDisabledBorderColorTabOne( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_2_DISABLED );
		GadgetTabControlSetDisabledImageTabTwo( window, info->image );
		GadgetTabControlSetDisabledColorTabTwo( window, info->color );
		GadgetTabControlSetDisabledBorderColorTabTwo( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_3_DISABLED );
		GadgetTabControlSetDisabledImageTabThree( window, info->image );
		GadgetTabControlSetDisabledColorTabThree( window, info->color );
		GadgetTabControlSetDisabledBorderColorTabThree( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_4_DISABLED );
		GadgetTabControlSetDisabledImageTabFour( window, info->image );
		GadgetTabControlSetDisabledColorTabFour( window, info->color );
		GadgetTabControlSetDisabledBorderColorTabFour( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_5_DISABLED );
		GadgetTabControlSetDisabledImageTabFive( window, info->image );
		GadgetTabControlSetDisabledColorTabFive( window, info->color );
		GadgetTabControlSetDisabledBorderColorTabFive( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_6_DISABLED );
		GadgetTabControlSetDisabledImageTabSix( window, info->image );
		GadgetTabControlSetDisabledColorTabSix( window, info->color );
		GadgetTabControlSetDisabledBorderColorTabSix( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_7_DISABLED );
		GadgetTabControlSetDisabledImageTabSeven( window, info->image );
		GadgetTabControlSetDisabledColorTabSeven( window, info->color );
		GadgetTabControlSetDisabledBorderColorTabSeven( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TAB_CONTROL_DISABLED );
		GadgetTabControlSetDisabledImageBackground( window, info->image );
		GadgetTabControlSetDisabledColorBackground( window, info->color );
		GadgetTabControlSetDisabledBorderColorBackground( window, info->borderColor );




		info = TheDefaultScheme->getImageAndColor( TC_TAB_0_HILITE );
		GadgetTabControlSetHiliteImageTabZero( window, info->image );
		GadgetTabControlSetHiliteColorTabZero( window, info->color );
		GadgetTabControlSetHiliteBorderColorTabZero( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_1_HILITE );
		GadgetTabControlSetHiliteImageTabOne( window, info->image );
		GadgetTabControlSetHiliteColorTabOne( window, info->color );
		GadgetTabControlSetHiliteBorderColorTabOne( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_2_HILITE );
		GadgetTabControlSetHiliteImageTabTwo( window, info->image );
		GadgetTabControlSetHiliteColorTabTwo( window, info->color );
		GadgetTabControlSetHiliteBorderColorTabTwo( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_3_HILITE );
		GadgetTabControlSetHiliteImageTabThree( window, info->image );
		GadgetTabControlSetHiliteColorTabThree( window, info->color );
		GadgetTabControlSetHiliteBorderColorTabThree( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_4_HILITE );
		GadgetTabControlSetHiliteImageTabFour( window, info->image );
		GadgetTabControlSetHiliteColorTabFour( window, info->color );
		GadgetTabControlSetHiliteBorderColorTabFour( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_5_HILITE );
		GadgetTabControlSetHiliteImageTabFive( window, info->image );
		GadgetTabControlSetHiliteColorTabFive( window, info->color );
		GadgetTabControlSetHiliteBorderColorTabFive( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_6_HILITE );
		GadgetTabControlSetHiliteImageTabSix( window, info->image );
		GadgetTabControlSetHiliteColorTabSix( window, info->color );
		GadgetTabControlSetHiliteBorderColorTabSix( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TC_TAB_7_HILITE );
		GadgetTabControlSetHiliteImageTabSeven( window, info->image );
		GadgetTabControlSetHiliteColorTabSeven( window, info->color );
		GadgetTabControlSetHiliteBorderColorTabSeven( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( TAB_CONTROL_HILITE );
		GadgetTabControlSetHiliteImageBackground( window, info->image );
		GadgetTabControlSetHiliteColorBackground( window, info->color );
		GadgetTabControlSetHiliteBorderColorBackground( window, info->borderColor );

	}  // end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}  // end newTabControl

// GUIEdit::newHorizontalSlider ===============================================
/** Create a new horizontal slider*/
//=============================================================================
GameWindow *GUIEdit::newHorizontalSlider( GameWindow *parent,
																					Int x, Int y,
																					Int width, Int height )
{
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED | WIN_STATUS_IMAGE;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_HORZ_SLIDER | GWS_MOUSE_TRACK;

	// initialize slider data
	SliderData sliderData;
	memset( &sliderData, 0, sizeof( sliderData ) );
	sliderData.maxVal = 100;
	sliderData.minVal = 0;
	sliderData.numTicks = 100;
	sliderData.position = 0;

	// make control
	window = TheWindowManager->gogoGadgetSlider( parent,
																							 status,
																							 x, y,
																							 width, height,
																							 &instData,
																							 &sliderData,
																							 NULL,
																							 TRUE );

	// set default colors based on the default scheme
	if( window )
	{
		ImageAndColorInfo *info;

		info = TheDefaultScheme->getImageAndColor( HSLIDER_ENABLED_LEFT );
		GadgetSliderSetEnabledImageLeft( window, info->image );
		GadgetSliderSetEnabledColor( window, info->color );
		GadgetSliderSetEnabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_ENABLED_RIGHT );
		GadgetSliderSetEnabledImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_ENABLED_CENTER );
		GadgetSliderSetEnabledImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_ENABLED_SMALL_CENTER );
		GadgetSliderSetEnabledImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( HSLIDER_DISABLED_LEFT );
		GadgetSliderSetDisabledImageLeft( window, info->image );
		GadgetSliderSetDisabledColor( window, info->color );
		GadgetSliderSetDisabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_DISABLED_RIGHT );
		GadgetSliderSetDisabledImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_DISABLED_CENTER );
		GadgetSliderSetDisabledImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_DISABLED_SMALL_CENTER );
		GadgetSliderSetDisabledImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( HSLIDER_HILITE_LEFT );
		GadgetSliderSetHiliteImageLeft( window, info->image );
		GadgetSliderSetHiliteColor( window, info->color );
		GadgetSliderSetHiliteBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_HILITE_RIGHT );
		GadgetSliderSetHiliteImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_HILITE_CENTER );
		GadgetSliderSetHiliteImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_HILITE_SMALL_CENTER );
		GadgetSliderSetHiliteImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( HSLIDER_THUMB_ENABLED );
		GadgetSliderSetEnabledThumbImage( window, info->image );
		GadgetSliderSetEnabledThumbColor( window, info->color );
		GadgetSliderSetEnabledThumbBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_THUMB_ENABLED_PUSHED );
		GadgetSliderSetEnabledSelectedThumbImage( window, info->image );
		GadgetSliderSetEnabledSelectedThumbColor( window, info->color );
		GadgetSliderSetEnabledSelectedThumbBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( HSLIDER_THUMB_DISABLED );
		GadgetSliderSetDisabledThumbImage( window, info->image );
		GadgetSliderSetDisabledThumbColor( window, info->color );
		GadgetSliderSetDisabledThumbBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_THUMB_DISABLED_PUSHED );
		GadgetSliderSetDisabledSelectedThumbImage( window, info->image );
		GadgetSliderSetDisabledSelectedThumbColor( window, info->color );
		GadgetSliderSetDisabledSelectedThumbBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( HSLIDER_THUMB_HILITE );
		GadgetSliderSetHiliteThumbImage( window, info->image );
		GadgetSliderSetHiliteThumbColor( window, info->color );
		GadgetSliderSetHiliteThumbBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( HSLIDER_THUMB_HILITE_PUSHED );
		GadgetSliderSetHiliteSelectedThumbImage( window, info->image );
		GadgetSliderSetHiliteSelectedThumbColor( window, info->color );
		GadgetSliderSetHiliteSelectedThumbBorderColor( window, info->borderColor );

		// set default font
		window->winSetFont( TheDefaultScheme->getFont() );

	}  // end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}  // end newHorizontalSlider

// GUIEdit::newVerticlaSlider =================================================
/** Create a new vertical slider */
//=============================================================================
GameWindow *GUIEdit::newVerticalSlider( GameWindow *parent,
																				Int x, Int y,
																				Int width, Int height )
{
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED | WIN_STATUS_IMAGE;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_VERT_SLIDER | GWS_MOUSE_TRACK;

	// initialize slider data
	SliderData sliderData;
	memset( &sliderData, 0, sizeof( sliderData ) );
	sliderData.maxVal = 100;
	sliderData.minVal = 0;
	sliderData.numTicks = 100;
	sliderData.position = 0;

	// make control
	window = TheWindowManager->gogoGadgetSlider( parent,
																							 status,
																							 x, y,
																							 width, height,
																							 &instData,
																							 &sliderData,
																							 NULL,
																							 TRUE );

	// set default colors based on the default scheme
	if( window )
	{
		ImageAndColorInfo *info;

		info = TheDefaultScheme->getImageAndColor( VSLIDER_ENABLED_TOP );
		GadgetSliderSetEnabledImageLeft( window, info->image );
		GadgetSliderSetEnabledColor( window, info->color );
		GadgetSliderSetEnabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_ENABLED_BOTTOM );
		GadgetSliderSetEnabledImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_ENABLED_CENTER );
		GadgetSliderSetEnabledImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_ENABLED_SMALL_CENTER );
		GadgetSliderSetEnabledImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( VSLIDER_DISABLED_TOP );
		GadgetSliderSetDisabledImageLeft( window, info->image );
		GadgetSliderSetDisabledColor( window, info->color );
		GadgetSliderSetDisabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_DISABLED_BOTTOM );
		GadgetSliderSetDisabledImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_DISABLED_CENTER );
		GadgetSliderSetDisabledImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_DISABLED_SMALL_CENTER );
		GadgetSliderSetDisabledImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( VSLIDER_HILITE_TOP );
		GadgetSliderSetHiliteImageLeft( window, info->image );
		GadgetSliderSetHiliteColor( window, info->color );
		GadgetSliderSetHiliteBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_HILITE_BOTTOM );
		GadgetSliderSetHiliteImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_HILITE_CENTER );
		GadgetSliderSetHiliteImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_HILITE_SMALL_CENTER );
		GadgetSliderSetHiliteImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( VSLIDER_THUMB_ENABLED );
		GadgetSliderSetEnabledThumbImage( window, info->image );
		GadgetSliderSetEnabledThumbColor( window, info->color );
		GadgetSliderSetEnabledThumbBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_THUMB_ENABLED_PUSHED );
		GadgetSliderSetEnabledSelectedThumbImage( window, info->image );
		GadgetSliderSetEnabledSelectedThumbColor( window, info->color );
		GadgetSliderSetEnabledSelectedThumbBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( VSLIDER_THUMB_DISABLED );
		GadgetSliderSetDisabledThumbImage( window, info->image );
		GadgetSliderSetDisabledThumbColor( window, info->color );
		GadgetSliderSetDisabledThumbBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_THUMB_DISABLED_PUSHED );
		GadgetSliderSetDisabledSelectedThumbImage( window, info->image );
		GadgetSliderSetDisabledSelectedThumbColor( window, info->color );
		GadgetSliderSetDisabledSelectedThumbBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( VSLIDER_THUMB_HILITE );
		GadgetSliderSetHiliteThumbImage( window, info->image );
		GadgetSliderSetHiliteThumbColor( window, info->color );
		GadgetSliderSetHiliteThumbBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( VSLIDER_THUMB_HILITE_PUSHED );
		GadgetSliderSetHiliteSelectedThumbImage( window, info->image );
		GadgetSliderSetHiliteSelectedThumbColor( window, info->color );
		GadgetSliderSetHiliteSelectedThumbBorderColor( window, info->borderColor );

		// set default font
		window->winSetFont( TheDefaultScheme->getFont() );

	}  // end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}  // end newVerticalSlider

// GUIEdit::newProgressBar ====================================================
/** Create a new progress bar */
//=============================================================================
GameWindow *GUIEdit::newProgressBar( GameWindow *parent,
																		 Int x, Int y,
																		 Int width, Int height )
{
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED | WIN_STATUS_IMAGE;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_PROGRESS_BAR | GWS_MOUSE_TRACK;

	// make control
	window = TheWindowManager->gogoGadgetProgressBar( parent,
																									  status,
																									  x, y,
																									  width, height,
																									  &instData,
																									  NULL,
																									  TRUE );

	// set default colors based on the default scheme
	if( window )
	{
		ImageAndColorInfo *info;

		//-------------------------------------------------------------------------
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_ENABLED_LEFT );
		GadgetProgressBarSetEnabledImageLeft( window, info->image );
		GadgetProgressBarSetEnabledColor( window, info->color );
		GadgetProgressBarSetEnabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_ENABLED_RIGHT );
		GadgetProgressBarSetEnabledImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_ENABLED_CENTER );
		GadgetProgressBarSetEnabledImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_ENABLED_SMALL_CENTER );
		GadgetProgressBarSetEnabledImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_ENABLED_BAR_LEFT );
		GadgetProgressBarSetEnabledBarImageLeft( window, info->image );
		GadgetProgressBarSetEnabledBarColor( window, info->color );
		GadgetProgressBarSetEnabledBarBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_ENABLED_BAR_RIGHT );
		GadgetProgressBarSetEnabledBarImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_ENABLED_BAR_CENTER );
		GadgetProgressBarSetEnabledBarImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_ENABLED_BAR_SMALL_CENTER );
		GadgetProgressBarSetEnabledBarImageSmallCenter( window, info->image );

		//-------------------------------------------------------------------------
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_DISABLED_LEFT );
		GadgetProgressBarSetDisabledImageLeft( window, info->image );
		GadgetProgressBarSetDisabledColor( window, info->color );
		GadgetProgressBarSetDisabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_DISABLED_RIGHT );
		GadgetProgressBarSetDisabledImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_DISABLED_CENTER );
		GadgetProgressBarSetDisabledImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_DISABLED_SMALL_CENTER );
		GadgetProgressBarSetDisabledImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_DISABLED_BAR_LEFT );
		GadgetProgressBarSetDisabledBarImageLeft( window, info->image );
		GadgetProgressBarSetDisabledBarColor( window, info->color );
		GadgetProgressBarSetDisabledBarBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_DISABLED_BAR_RIGHT );
		GadgetProgressBarSetDisabledBarImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_DISABLED_BAR_CENTER );
		GadgetProgressBarSetDisabledBarImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_DISABLED_BAR_SMALL_CENTER );
		GadgetProgressBarSetDisabledBarImageSmallCenter( window, info->image );

		//-------------------------------------------------------------------------
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_HILITE_LEFT );
		GadgetProgressBarSetHiliteImageLeft( window, info->image );
		GadgetProgressBarSetHiliteColor( window, info->color );
		GadgetProgressBarSetHiliteBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_HILITE_RIGHT );
		GadgetProgressBarSetHiliteImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_HILITE_CENTER );
		GadgetProgressBarSetHiliteImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_HILITE_SMALL_CENTER );
		GadgetProgressBarSetHiliteImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_HILITE_BAR_LEFT );
		GadgetProgressBarSetHiliteBarImageLeft( window, info->image );
		GadgetProgressBarSetHiliteBarColor( window, info->color );
		GadgetProgressBarSetHiliteBarBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_HILITE_BAR_RIGHT );
		GadgetProgressBarSetHiliteBarImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_HILITE_BAR_CENTER );
		GadgetProgressBarSetHiliteBarImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( PROGRESS_BAR_HILITE_BAR_SMALL_CENTER );
		GadgetProgressBarSetHiliteBarImageSmallCenter( window, info->image );

		Color color, border;

		color = TheDefaultScheme->getEnabledTextColor();
		border = TheDefaultScheme->getEnabledTextBorderColor();
		window->winSetEnabledTextColors( color, border );

		color = TheDefaultScheme->getDisabledTextColor();
		border = TheDefaultScheme->getDisabledTextBorderColor();
		window->winSetDisabledTextColors( color, border );

		color = TheDefaultScheme->getHiliteTextColor();
		border = TheDefaultScheme->getHiliteTextBorderColor();
		window->winSetHiliteTextColors( color, border );

		// set default font
		window->winSetFont( TheDefaultScheme->getFont() );

	}  // end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}  // end newProgressBar

// GUIEdit::newListbox ========================================================
/** Create a new list box */
//=============================================================================
GameWindow *GUIEdit::newComboBox( GameWindow *parent,
																 Int x, Int y,
																 Int width, Int height )
{
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED | WIN_STATUS_IMAGE;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_COMBO_BOX | GWS_MOUSE_TRACK;

	//initialize combo box data

	ComboBoxData *comboData = new ComboBoxData;
	memset ( comboData, 0, sizeof(comboData));

	comboData->entryData = new EntryData;
	memset ( comboData->entryData, 0, sizeof(EntryData));
	comboData->listboxData = new ListboxData;
	memset ( comboData->listboxData, 0, sizeof(ListboxData));


	//initialize combo box data
	comboData->isEditable = TRUE;
	comboData->maxChars = 16;
	comboData->maxDisplay = 5;
	comboData->entryCount = 0;
	comboData->dontHide = FALSE;
	comboData->asciiOnly = FALSE;
	comboData->lettersAndNumbersOnly = FALSE;

	//initialize entry data
	comboData->entryData->maxTextLen = 16;
	comboData->entryData->alphaNumericalOnly = FALSE;
	comboData->entryData->aSCIIOnly = FALSE;
	comboData->entryData->numericalOnly = FALSE;


	//initialize listbox data
	comboData->listboxData->listLength = 10;
	comboData->listboxData->autoScroll = 0;
	comboData->listboxData->scrollIfAtEnd = FALSE;
	comboData->listboxData->autoPurge = 0;
	comboData->listboxData->scrollBar = 1;
	comboData->listboxData->multiSelect = 0;
	comboData->listboxData->forceSelect = 1;
	comboData->listboxData->columns = 1;
	comboData->listboxData->columnWidth = NULL;
	comboData->listboxData->columnWidthPercentage = NULL;

	//create the control
	window = TheWindowManager->gogoGadgetComboBox( parent,
																							  status,
																							  x, y,
																							  width, height,
																							  &instData,
																							  comboData,
																								NULL,
																								TRUE );

	// set default colors based on the default scheme

	if( window )
	{
		ImageAndColorInfo *info;
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_ENABLED );
		GadgetComboBoxSetEnabledImage( window, info->image );
		GadgetComboBoxSetEnabledColor( window, info->color );
		GadgetComboBoxSetEnabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_LEFT );
		GadgetComboBoxSetEnabledSelectedItemImageLeft( window, info->image );
		GadgetComboBoxSetEnabledSelectedItemColor( window, info->color );
		GadgetComboBoxSetEnabledSelectedItemBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_RIGHT );
		GadgetComboBoxSetEnabledSelectedItemImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_CENTER );
		GadgetComboBoxSetEnabledSelectedItemImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER );
		GadgetComboBoxSetEnabledSelectedItemImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( COMBOBOX_DISABLED );
		GadgetComboBoxSetDisabledImage( window, info->image );
		GadgetComboBoxSetDisabledColor( window, info->color );
		GadgetComboBoxSetDisabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_LEFT );
		GadgetComboBoxSetDisabledSelectedItemImageLeft( window, info->image );
		GadgetComboBoxSetDisabledSelectedItemColor( window, info->color );
		GadgetComboBoxSetDisabledSelectedItemBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_RIGHT );
		GadgetComboBoxSetDisabledSelectedItemImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_CENTER );
		GadgetComboBoxSetDisabledSelectedItemImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER );
		GadgetComboBoxSetDisabledSelectedItemImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( COMBOBOX_HILITE );
		GadgetComboBoxSetHiliteImage( window, info->image );
		GadgetComboBoxSetHiliteColor( window, info->color );
		GadgetComboBoxSetHiliteBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_LEFT );
		GadgetComboBoxSetHiliteSelectedItemImageLeft( window, info->image );
		GadgetComboBoxSetHiliteSelectedItemColor( window, info->color );
		GadgetComboBoxSetHiliteSelectedItemBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_RIGHT );
		GadgetComboBoxSetHiliteSelectedItemImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_CENTER );
		GadgetComboBoxSetHiliteSelectedItemImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( COMBOBOX_HILITE_SELECTED_ITEM_SMALL_CENTER );
		GadgetComboBoxSetHiliteSelectedItemImageSmallCenter( window, info->image );



		GameWindow *editBox = GadgetComboBoxGetEditBox( window );
		if(editBox)
		{
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_LEFT );
			GadgetTextEntrySetEnabledImageLeft( editBox, info->image );
			GadgetTextEntrySetEnabledColor( editBox, info->color );
			GadgetTextEntrySetEnabledBorderColor( editBox, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_RIGHT );
			GadgetTextEntrySetEnabledImageRight( editBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_CENTER );
			GadgetTextEntrySetEnabledImageCenter( editBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_ENABLED_SMALL_CENTER );
			GadgetTextEntrySetEnabledImageSmallCenter( editBox, info->image );

			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_LEFT );
			GadgetTextEntrySetDisabledImageLeft( editBox, info->image );
			GadgetTextEntrySetDisabledColor( editBox, info->color );
			GadgetTextEntrySetDisabledBorderColor( editBox, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_RIGHT );
			GadgetTextEntrySetDisabledImageRight( editBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_CENTER );
			GadgetTextEntrySetDisabledImageCenter( editBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_DISABLED_SMALL_CENTER );
			GadgetTextEntrySetDisabledImageSmallCenter( editBox, info->image );

			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_HILITE_LEFT );
			GadgetTextEntrySetHiliteImageLeft( editBox, info->image );
			GadgetTextEntrySetHiliteColor( window, info->color );
			GadgetTextEntrySetHiliteBorderColor( editBox, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_HILITE_RIGHT );
			GadgetTextEntrySetHiliteImageRight( editBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_HILITE_CENTER );
			GadgetTextEntrySetHiliteImageCenter( editBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_EDIT_BOX_HILITE_SMALL_CENTER );
			GadgetTextEntrySetHiliteImageSmallCenter( editBox, info->image );
		}



		GameWindow *listBox = GadgetComboBoxGetListBox( window );
		if(listBox)
		{
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_ENABLED );
			GadgetListBoxSetEnabledImage( listBox, info->image );
			GadgetListBoxSetEnabledColor( listBox, info->color );
			GadgetListBoxSetEnabledBorderColor( listBox, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_LEFT );
			GadgetListBoxSetEnabledSelectedItemImageLeft( listBox, info->image );
			GadgetListBoxSetEnabledSelectedItemColor( listBox, info->color );
			GadgetListBoxSetEnabledSelectedItemBorderColor( listBox, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_RIGHT );
			GadgetListBoxSetEnabledSelectedItemImageRight( listBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_CENTER );
			GadgetListBoxSetEnabledSelectedItemImageCenter( listBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER );
			GadgetListBoxSetEnabledSelectedItemImageSmallCenter( listBox, info->image );

			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DISABLED );
			GadgetListBoxSetDisabledImage( listBox, info->image );
			GadgetListBoxSetDisabledColor( listBox, info->color );
			GadgetListBoxSetDisabledBorderColor( listBox, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_LEFT );
			GadgetListBoxSetDisabledSelectedItemImageLeft( listBox, info->image );
			GadgetListBoxSetDisabledSelectedItemColor( listBox, info->color );
			GadgetListBoxSetDisabledSelectedItemBorderColor( listBox, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_RIGHT );
			GadgetListBoxSetDisabledSelectedItemImageRight( listBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_CENTER );
			GadgetListBoxSetDisabledSelectedItemImageCenter( listBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER );
			GadgetListBoxSetDisabledSelectedItemImageSmallCenter( listBox, info->image );

			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_HILITE );
			GadgetListBoxSetHiliteImage( listBox, info->image );
			GadgetListBoxSetHiliteColor( listBox, info->color );
			GadgetListBoxSetHiliteBorderColor( listBox, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_LEFT );
			GadgetListBoxSetHiliteSelectedItemImageLeft( listBox, info->image );
			GadgetListBoxSetHiliteSelectedItemColor( listBox, info->color );
			GadgetListBoxSetHiliteSelectedItemBorderColor( listBox, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_RIGHT );
			GadgetListBoxSetHiliteSelectedItemImageRight( listBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_CENTER );
			GadgetListBoxSetHiliteSelectedItemImageCenter( listBox, info->image );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER );
			GadgetListBoxSetHiliteSelectedItemImageSmallCenter( listBox, info->image );

			GameWindow *upButton = GadgetListBoxGetUpButton( listBox );
			if( upButton )
			{

				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED );
				GadgetButtonSetEnabledImage( upButton, info->image );
				GadgetButtonSetEnabledColor( upButton, info->color );
				GadgetButtonSetEnabledBorderColor( upButton, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_ENABLED_PUSHED );
				GadgetButtonSetEnabledSelectedImage( upButton, info->image );
				GadgetButtonSetEnabledSelectedColor( upButton, info->color );
				GadgetButtonSetEnabledSelectedBorderColor( upButton, info->borderColor );

				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED );
				GadgetButtonSetDisabledImage( upButton, info->image );
				GadgetButtonSetDisabledColor( upButton, info->color );
				GadgetButtonSetDisabledBorderColor( upButton, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_DISABLED_PUSHED );
				GadgetButtonSetDisabledSelectedImage( upButton, info->image );
				GadgetButtonSetDisabledSelectedColor( upButton, info->color );
				GadgetButtonSetDisabledSelectedBorderColor( upButton, info->borderColor );

				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_HILITE );
				GadgetButtonSetHiliteImage( upButton, info->image );
				GadgetButtonSetHiliteColor( upButton, info->color );
				GadgetButtonSetHiliteBorderColor( upButton, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_UP_BUTTON_HILITE_PUSHED );
				GadgetButtonSetHiliteSelectedImage( upButton, info->image );
				GadgetButtonSetHiliteSelectedColor( upButton, info->color );
				GadgetButtonSetHiliteSelectedBorderColor( upButton, info->borderColor );

			}  // end if

			GameWindow *downButton = GadgetListBoxGetDownButton( listBox );
			if( downButton )
			{

				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED );
				GadgetButtonSetEnabledImage( downButton, info->image );
				GadgetButtonSetEnabledColor( downButton, info->color );
				GadgetButtonSetEnabledBorderColor( downButton, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_ENABLED_PUSHED );
				GadgetButtonSetEnabledSelectedImage( downButton, info->image );
				GadgetButtonSetEnabledSelectedColor( downButton, info->color );
				GadgetButtonSetEnabledSelectedBorderColor( downButton, info->borderColor );

				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED );
				GadgetButtonSetDisabledImage( downButton, info->image );
				GadgetButtonSetDisabledColor( downButton, info->color );
				GadgetButtonSetDisabledBorderColor( downButton, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_DISABLED_PUSHED );
				GadgetButtonSetDisabledSelectedImage( downButton, info->image );
				GadgetButtonSetDisabledSelectedColor( downButton, info->color );
				GadgetButtonSetDisabledSelectedBorderColor( downButton, info->borderColor );

				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE );
				GadgetButtonSetHiliteImage( downButton, info->image );
				GadgetButtonSetHiliteColor( downButton, info->color );
				GadgetButtonSetHiliteBorderColor( downButton, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_DOWN_BUTTON_HILITE_PUSHED );
				GadgetButtonSetHiliteSelectedImage( downButton, info->image );
				GadgetButtonSetHiliteSelectedColor( downButton, info->color );
				GadgetButtonSetHiliteSelectedBorderColor( downButton, info->borderColor );

			}  // end if

			GameWindow *slider = GadgetListBoxGetSlider( listBox );
			if( slider )
			{

				// ----------------------------------------------------------------------
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_TOP );
				GadgetSliderSetEnabledImageTop( slider, info->image );
				GadgetSliderSetEnabledColor( slider, info->color );
				GadgetSliderSetEnabledBorderColor( slider, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_BOTTOM );
				GadgetSliderSetEnabledImageBottom( slider, info->image );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_CENTER );
				GadgetSliderSetEnabledImageCenter( slider, info->image );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_ENABLED_SMALL_CENTER );
				GadgetSliderSetEnabledImageSmallCenter( slider, info->image );

				// ----------------------------------------------------------------------
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_TOP );
				GadgetSliderSetDisabledImageTop( slider, info->image );
				GadgetSliderSetDisabledColor( slider, info->color );
				GadgetSliderSetDisabledBorderColor( slider, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_BOTTOM );
				GadgetSliderSetDisabledImageBottom( slider, info->image );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_CENTER );
				GadgetSliderSetDisabledImageCenter( slider, info->image );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_DISABLED_SMALL_CENTER );
				GadgetSliderSetDisabledImageSmallCenter( slider, info->image );

				// ----------------------------------------------------------------------
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_TOP );
				GadgetSliderSetHiliteImageTop( slider, info->image );
				GadgetSliderSetHiliteColor( slider, info->color );
				GadgetSliderSetHiliteBorderColor( slider, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_BOTTOM );
				GadgetSliderSetHiliteImageBottom( slider, info->image );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_CENTER );
				GadgetSliderSetHiliteImageCenter( slider, info->image );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_HILITE_SMALL_CENTER );
				GadgetSliderSetHiliteImageSmallCenter( slider, info->image );

				//-----------------------------------------------------------------------

				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED );
				GadgetSliderSetEnabledThumbImage( slider, info->image );
				GadgetSliderSetEnabledThumbColor( slider, info->color );
				GadgetSliderSetEnabledThumbBorderColor( slider, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_ENABLED_PUSHED );
				GadgetSliderSetEnabledSelectedThumbImage( slider, info->image );
				GadgetSliderSetEnabledSelectedThumbColor( slider, info->color );
				GadgetSliderSetEnabledSelectedThumbBorderColor( slider, info->borderColor );

				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED );
				GadgetSliderSetDisabledThumbImage( slider, info->image );
				GadgetSliderSetDisabledThumbColor( slider, info->color );
				GadgetSliderSetDisabledThumbBorderColor( slider, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_DISABLED_PUSHED );
				GadgetSliderSetDisabledSelectedThumbImage( slider, info->image );
				GadgetSliderSetDisabledSelectedThumbColor( slider, info->color );
				GadgetSliderSetDisabledSelectedThumbBorderColor( slider, info->borderColor );

				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE );
				GadgetSliderSetHiliteThumbImage( slider, info->image );
				GadgetSliderSetHiliteThumbColor( slider, info->color );
				GadgetSliderSetHiliteThumbBorderColor( slider, info->borderColor );
				info = TheDefaultScheme->getImageAndColor( COMBOBOX_LISTBOX_SLIDER_THUMB_HILITE_PUSHED );
				GadgetSliderSetHiliteSelectedThumbImage( slider, info->image );
				GadgetSliderSetHiliteSelectedThumbColor( slider, info->color );
				GadgetSliderSetHiliteSelectedThumbBorderColor( slider, info->borderColor );

				}
		} //end listbox if
		GameWindow *dropDownButton = GadgetComboBoxGetDropDownButton( window );
		if( dropDownButton )
		{

			info = TheDefaultScheme->getImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_ENABLED );
			GadgetButtonSetEnabledImage( dropDownButton, info->image );
			GadgetButtonSetEnabledColor( dropDownButton, info->color );
			GadgetButtonSetEnabledBorderColor( dropDownButton, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_ENABLED_PUSHED );
			GadgetButtonSetEnabledSelectedImage( dropDownButton, info->image );
			GadgetButtonSetEnabledSelectedColor( dropDownButton, info->color );
			GadgetButtonSetEnabledSelectedBorderColor( dropDownButton, info->borderColor );

			info = TheDefaultScheme->getImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_DISABLED );
			GadgetButtonSetDisabledImage( dropDownButton, info->image );
			GadgetButtonSetDisabledColor( dropDownButton, info->color );
			GadgetButtonSetDisabledBorderColor( dropDownButton, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_DISABLED_PUSHED );
			GadgetButtonSetDisabledSelectedImage( dropDownButton, info->image );
			GadgetButtonSetDisabledSelectedColor( dropDownButton, info->color );
			GadgetButtonSetDisabledSelectedBorderColor( dropDownButton, info->borderColor );

			info = TheDefaultScheme->getImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_HILITE );
			GadgetButtonSetHiliteImage( dropDownButton, info->image );
			GadgetButtonSetHiliteColor( dropDownButton, info->color );
			GadgetButtonSetHiliteBorderColor( dropDownButton, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( COMBOBOX_DROP_DOWN_BUTTON_HILITE_PUSHED );
			GadgetButtonSetHiliteSelectedImage( dropDownButton, info->image );
			GadgetButtonSetHiliteSelectedColor( dropDownButton, info->color );
			GadgetButtonSetHiliteSelectedBorderColor( dropDownButton, info->borderColor );

		}  // end if

		Color color, border;

		color = TheDefaultScheme->getEnabledTextColor();
		border = TheDefaultScheme->getEnabledTextBorderColor();
		window->winSetEnabledTextColors( color, border );

		color = TheDefaultScheme->getDisabledTextColor();
		border = TheDefaultScheme->getDisabledTextBorderColor();
		window->winSetDisabledTextColors( color, border );


		color = TheDefaultScheme->getHiliteTextColor();
		border = TheDefaultScheme->getHiliteTextBorderColor();
		window->winSetHiliteTextColors( color, border );

		/// @TODO: We still need to set the IME Composite Text colors in TheDefaultScheme
		window->winSetIMECompositeTextColors( color, border );

		// set default font
		window->winSetFont( TheDefaultScheme->getFont() );

	}// end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;


} // end newComboBox

// GUIEdit::newListbox ========================================================
/** Create a new list box */
//=============================================================================
GameWindow *GUIEdit::newListbox( GameWindow *parent,
																 Int x, Int y,
																 Int width, Int height )
{
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED | WIN_STATUS_IMAGE;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_SCROLL_LISTBOX | GWS_MOUSE_TRACK;

	// initialize listbox data
	ListboxData listData;
	memset( &listData, 0, sizeof( listData ) );
	listData.listLength = 10;
	listData.autoScroll = 0;
	listData.scrollIfAtEnd = FALSE;
	listData.autoPurge = 0;
	listData.scrollBar = 1;
	listData.multiSelect = 0;
	listData.forceSelect = 0;
	listData.columns = 1;
	listData.columnWidth = NULL;
	listData.columnWidthPercentage = NULL;

	// make control
	window = TheWindowManager->gogoGadgetListBox( parent,
																							  status,
																							  x, y,
																							  width, height,
																							  &instData,
																							  &listData,
																								NULL,
																								TRUE );

	// set default colors based on the default scheme
	if( window )
	{
		ImageAndColorInfo *info;

		info = TheDefaultScheme->getImageAndColor( LISTBOX_ENABLED );
		GadgetListBoxSetEnabledImage( window, info->image );
		GadgetListBoxSetEnabledColor( window, info->color );
		GadgetListBoxSetEnabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_LEFT );
		GadgetListBoxSetEnabledSelectedItemImageLeft( window, info->image );
		GadgetListBoxSetEnabledSelectedItemColor( window, info->color );
		GadgetListBoxSetEnabledSelectedItemBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_RIGHT );
		GadgetListBoxSetEnabledSelectedItemImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_CENTER );
		GadgetListBoxSetEnabledSelectedItemImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_ENABLED_SELECTED_ITEM_SMALL_CENTER );
		GadgetListBoxSetEnabledSelectedItemImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( LISTBOX_DISABLED );
		GadgetListBoxSetDisabledImage( window, info->image );
		GadgetListBoxSetDisabledColor( window, info->color );
		GadgetListBoxSetDisabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_LEFT );
		GadgetListBoxSetDisabledSelectedItemImageLeft( window, info->image );
		GadgetListBoxSetDisabledSelectedItemColor( window, info->color );
		GadgetListBoxSetDisabledSelectedItemBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_RIGHT );
		GadgetListBoxSetDisabledSelectedItemImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_CENTER );
		GadgetListBoxSetDisabledSelectedItemImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_DISABLED_SELECTED_ITEM_SMALL_CENTER );
		GadgetListBoxSetDisabledSelectedItemImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( LISTBOX_HILITE );
		GadgetListBoxSetHiliteImage( window, info->image );
		GadgetListBoxSetHiliteColor( window, info->color );
		GadgetListBoxSetHiliteBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_LEFT );
		GadgetListBoxSetHiliteSelectedItemImageLeft( window, info->image );
		GadgetListBoxSetHiliteSelectedItemColor( window, info->color );
		GadgetListBoxSetHiliteSelectedItemBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_RIGHT );
		GadgetListBoxSetHiliteSelectedItemImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_CENTER );
		GadgetListBoxSetHiliteSelectedItemImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( LISTBOX_HILITE_SELECTED_ITEM_SMALL_CENTER );
		GadgetListBoxSetHiliteSelectedItemImageSmallCenter( window, info->image );

		GameWindow *upButton = GadgetListBoxGetUpButton( window );
		if( upButton )
		{

			info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_ENABLED );
			GadgetButtonSetEnabledImage( upButton, info->image );
			GadgetButtonSetEnabledColor( upButton, info->color );
			GadgetButtonSetEnabledBorderColor( upButton, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_ENABLED_PUSHED );
			GadgetButtonSetEnabledSelectedImage( upButton, info->image );
			GadgetButtonSetEnabledSelectedColor( upButton, info->color );
			GadgetButtonSetEnabledSelectedBorderColor( upButton, info->borderColor );

			info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_DISABLED );
			GadgetButtonSetDisabledImage( upButton, info->image );
			GadgetButtonSetDisabledColor( upButton, info->color );
			GadgetButtonSetDisabledBorderColor( upButton, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_DISABLED_PUSHED );
			GadgetButtonSetDisabledSelectedImage( upButton, info->image );
			GadgetButtonSetDisabledSelectedColor( upButton, info->color );
			GadgetButtonSetDisabledSelectedBorderColor( upButton, info->borderColor );

			info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_HILITE );
			GadgetButtonSetHiliteImage( upButton, info->image );
			GadgetButtonSetHiliteColor( upButton, info->color );
			GadgetButtonSetHiliteBorderColor( upButton, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_UP_BUTTON_HILITE_PUSHED );
			GadgetButtonSetHiliteSelectedImage( upButton, info->image );
			GadgetButtonSetHiliteSelectedColor( upButton, info->color );
			GadgetButtonSetHiliteSelectedBorderColor( upButton, info->borderColor );

		}  // end if

		GameWindow *downButton = GadgetListBoxGetDownButton( window );
		if( downButton )
		{

			info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_ENABLED );
			GadgetButtonSetEnabledImage( downButton, info->image );
			GadgetButtonSetEnabledColor( downButton, info->color );
			GadgetButtonSetEnabledBorderColor( downButton, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_ENABLED_PUSHED );
			GadgetButtonSetEnabledSelectedImage( downButton, info->image );
			GadgetButtonSetEnabledSelectedColor( downButton, info->color );
			GadgetButtonSetEnabledSelectedBorderColor( downButton, info->borderColor );

			info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_DISABLED );
			GadgetButtonSetDisabledImage( downButton, info->image );
			GadgetButtonSetDisabledColor( downButton, info->color );
			GadgetButtonSetDisabledBorderColor( downButton, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_DISABLED_PUSHED );
			GadgetButtonSetDisabledSelectedImage( downButton, info->image );
			GadgetButtonSetDisabledSelectedColor( downButton, info->color );
			GadgetButtonSetDisabledSelectedBorderColor( downButton, info->borderColor );

			info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_HILITE );
			GadgetButtonSetHiliteImage( downButton, info->image );
			GadgetButtonSetHiliteColor( downButton, info->color );
			GadgetButtonSetHiliteBorderColor( downButton, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_DOWN_BUTTON_HILITE_PUSHED );
			GadgetButtonSetHiliteSelectedImage( downButton, info->image );
			GadgetButtonSetHiliteSelectedColor( downButton, info->color );
			GadgetButtonSetHiliteSelectedBorderColor( downButton, info->borderColor );

		}  // end if

		GameWindow *slider = GadgetListBoxGetSlider( window );
		if( slider )
		{

			// ----------------------------------------------------------------------
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_ENABLED_TOP );
			GadgetSliderSetEnabledImageTop( slider, info->image );
			GadgetSliderSetEnabledColor( slider, info->color );
			GadgetSliderSetEnabledBorderColor( slider, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_ENABLED_BOTTOM );
			GadgetSliderSetEnabledImageBottom( slider, info->image );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_ENABLED_CENTER );
			GadgetSliderSetEnabledImageCenter( slider, info->image );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_ENABLED_SMALL_CENTER );
			GadgetSliderSetEnabledImageSmallCenter( slider, info->image );

			// ----------------------------------------------------------------------
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_DISABLED_TOP );
			GadgetSliderSetDisabledImageTop( slider, info->image );
			GadgetSliderSetDisabledColor( slider, info->color );
			GadgetSliderSetDisabledBorderColor( slider, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_DISABLED_BOTTOM );
			GadgetSliderSetDisabledImageBottom( slider, info->image );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_DISABLED_CENTER );
			GadgetSliderSetDisabledImageCenter( slider, info->image );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_DISABLED_SMALL_CENTER );
			GadgetSliderSetDisabledImageSmallCenter( slider, info->image );

			// ----------------------------------------------------------------------
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_HILITE_TOP );
			GadgetSliderSetHiliteImageTop( slider, info->image );
			GadgetSliderSetHiliteColor( slider, info->color );
			GadgetSliderSetHiliteBorderColor( slider, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_HILITE_BOTTOM );
			GadgetSliderSetHiliteImageBottom( slider, info->image );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_HILITE_CENTER );
			GadgetSliderSetHiliteImageCenter( slider, info->image );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_HILITE_SMALL_CENTER );
			GadgetSliderSetHiliteImageSmallCenter( slider, info->image );

			//-----------------------------------------------------------------------

			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_ENABLED );
			GadgetSliderSetEnabledThumbImage( slider, info->image );
			GadgetSliderSetEnabledThumbColor( slider, info->color );
			GadgetSliderSetEnabledThumbBorderColor( slider, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_ENABLED_PUSHED );
			GadgetSliderSetEnabledSelectedThumbImage( slider, info->image );
			GadgetSliderSetEnabledSelectedThumbColor( slider, info->color );
			GadgetSliderSetEnabledSelectedThumbBorderColor( slider, info->borderColor );

			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_DISABLED );
			GadgetSliderSetDisabledThumbImage( slider, info->image );
			GadgetSliderSetDisabledThumbColor( slider, info->color );
			GadgetSliderSetDisabledThumbBorderColor( slider, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_DISABLED_PUSHED );
			GadgetSliderSetDisabledSelectedThumbImage( slider, info->image );
			GadgetSliderSetDisabledSelectedThumbColor( slider, info->color );
			GadgetSliderSetDisabledSelectedThumbBorderColor( slider, info->borderColor );

			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_HILITE );
			GadgetSliderSetHiliteThumbImage( slider, info->image );
			GadgetSliderSetHiliteThumbColor( slider, info->color );
			GadgetSliderSetHiliteThumbBorderColor( slider, info->borderColor );
			info = TheDefaultScheme->getImageAndColor( LISTBOX_SLIDER_THUMB_HILITE_PUSHED );
			GadgetSliderSetHiliteSelectedThumbImage( slider, info->image );
			GadgetSliderSetHiliteSelectedThumbColor( slider, info->color );
			GadgetSliderSetHiliteSelectedThumbBorderColor( slider, info->borderColor );

		}  // end if, slider

		Color color, border;

		color = TheDefaultScheme->getEnabledTextColor();
		border = TheDefaultScheme->getEnabledTextBorderColor();
		window->winSetEnabledTextColors( color, border );

		color = TheDefaultScheme->getDisabledTextColor();
		border = TheDefaultScheme->getDisabledTextBorderColor();
		window->winSetDisabledTextColors( color, border );

		color = TheDefaultScheme->getHiliteTextColor();
		border = TheDefaultScheme->getHiliteTextBorderColor();
		window->winSetHiliteTextColors( color, border );

		// set default font
		window->winSetFont( TheDefaultScheme->getFont() );

	}  // end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}	 // end newListbox

// GUIEdit::newTextEntry ======================================================
/** Create a new text entry */
//=============================================================================
GameWindow *GUIEdit::newTextEntry( GameWindow *parent,
																	 Int x, Int y,
																	 Int width, Int height )
{
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED | WIN_STATUS_IMAGE;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_ENTRY_FIELD | GWS_MOUSE_TRACK;
	instData.m_textLabelString = "Entry";

	// initialize listbox data
	EntryData entryData;
	memset( &entryData, 0, sizeof( entryData ) );
	entryData.maxTextLen = 64;

	// make control
	window = TheWindowManager->gogoGadgetTextEntry( parent,
																									status,
																									x, y,
																									width, height,
																									&instData,
																									&entryData,
																									NULL,
																									TRUE );

	// set default colors based on the default scheme
	if( window )
	{
		ImageAndColorInfo *info;

		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_ENABLED_LEFT );
		GadgetTextEntrySetEnabledImageLeft( window, info->image );
		GadgetTextEntrySetEnabledColor( window, info->color );
		GadgetTextEntrySetEnabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_ENABLED_RIGHT );
		GadgetTextEntrySetEnabledImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_ENABLED_CENTER );
		GadgetTextEntrySetEnabledImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_ENABLED_SMALL_CENTER );
		GadgetTextEntrySetEnabledImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_DISABLED_LEFT );
		GadgetTextEntrySetDisabledImageLeft( window, info->image );
		GadgetTextEntrySetDisabledColor( window, info->color );
		GadgetTextEntrySetDisabledBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_DISABLED_RIGHT );
		GadgetTextEntrySetDisabledImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_DISABLED_CENTER );
		GadgetTextEntrySetDisabledImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_DISABLED_SMALL_CENTER );
		GadgetTextEntrySetDisabledImageSmallCenter( window, info->image );

		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_HILITE_LEFT );
		GadgetTextEntrySetHiliteImageLeft( window, info->image );
		GadgetTextEntrySetHiliteColor( window, info->color );
		GadgetTextEntrySetHiliteBorderColor( window, info->borderColor );
		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_HILITE_RIGHT );
		GadgetTextEntrySetHiliteImageRight( window, info->image );
		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_HILITE_CENTER );
		GadgetTextEntrySetHiliteImageCenter( window, info->image );
		info = TheDefaultScheme->getImageAndColor( TEXT_ENTRY_HILITE_SMALL_CENTER );
		GadgetTextEntrySetHiliteImageSmallCenter( window, info->image );

		Color color, border;

		color = TheDefaultScheme->getEnabledTextColor();
		border = TheDefaultScheme->getEnabledTextBorderColor();
		window->winSetEnabledTextColors( color, border );

		color = TheDefaultScheme->getDisabledTextColor();
		border = TheDefaultScheme->getDisabledTextBorderColor();
		window->winSetDisabledTextColors( color, border );

		color = TheDefaultScheme->getHiliteTextColor();
		border = TheDefaultScheme->getHiliteTextBorderColor();
		window->winSetHiliteTextColors( color, border );

		// set default font
		window->winSetFont( TheDefaultScheme->getFont() );

	}  // end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}  // end newTextEntry

// GUIEdit::newStaticText =====================================================
/** Create a new static text*/
//=============================================================================
GameWindow *GUIEdit::newStaticText( GameWindow *parent,
																		Int x, Int y,
																		Int width, Int height )
{
	GameWindow *window;
	WinInstanceData instData;
	UnsignedInt status = WIN_STATUS_ENABLED | WIN_STATUS_IMAGE;

	// validate the parent to disallow illegal relationships
	if( validateParentForCreate( parent ) == FALSE )
		return NULL;

	// keep inside a parent if present
	if( parent )
		clipCreationParamsToParent( parent, &x, &y, &width, &height );

	// initialize inst data
	instData.init();
	instData.m_style = GWS_STATIC_TEXT | GWS_MOUSE_TRACK;
	instData.m_textLabelString = "Static Text";

	// initialize listbox data
	TextData textData;
	memset( &textData, 0, sizeof( textData ) );
	textData.centered = TRUE;

	// make control
	window = TheWindowManager->gogoGadgetStaticText( parent,
																									 status,
																									 x, y,
																									 width, height,
																									 &instData,
																									 &textData,
																									 NULL,
																									 TRUE );

	// set default colors based on the default scheme
	if( window )
	{
		ImageAndColorInfo *info;

		info = TheDefaultScheme->getImageAndColor( STATIC_TEXT_ENABLED );
		GadgetStaticTextSetEnabledImage( window, info->image );
		GadgetStaticTextSetEnabledColor( window, info->color );
		GadgetStaticTextSetEnabledBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( STATIC_TEXT_DISABLED );
		GadgetStaticTextSetDisabledImage( window, info->image );
		GadgetStaticTextSetDisabledColor( window, info->color );
		GadgetStaticTextSetDisabledBorderColor( window, info->borderColor );

		info = TheDefaultScheme->getImageAndColor( STATIC_TEXT_DISABLED );
		GadgetStaticTextSetDisabledImage( window, info->image );
		GadgetStaticTextSetDisabledColor( window, info->color );
		GadgetStaticTextSetDisabledBorderColor( window, info->borderColor );

		Color color, border;

		color = TheDefaultScheme->getEnabledTextColor();
		border = TheDefaultScheme->getEnabledTextBorderColor();
		window->winSetEnabledTextColors( color, border );

		color = TheDefaultScheme->getDisabledTextColor();
		border = TheDefaultScheme->getDisabledTextBorderColor();
		window->winSetDisabledTextColors( color, border );

		color = TheDefaultScheme->getHiliteTextColor();
		border = TheDefaultScheme->getHiliteTextBorderColor();
		window->winSetHiliteTextColors( color, border );

		// set default font
		window->winSetFont( TheDefaultScheme->getFont() );

	}  // end if

	// contents have changed
	setUnsaved( TRUE );

	// notify the editor of the new window
	notifyNewWindow( window );

	return window;

}  // end newStaticText

// GUIEdit::createStatusBar ===================================================
/** Create a status bar at the bottom of the editor */
//=============================================================================
void GUIEdit::createStatusBar( void )
{
	RECT rect;
	Int width;
  Int sizes[ STATUS_NUM_PARTS ];

	// create the bar
	m_statusBarHWnd = CreateStatusWindow( WS_CHILD |
																				WS_VISIBLE |
																				WS_CLIPSIBLINGS |
																				CCS_BOTTOM |
																				SBARS_SIZEGRIP,
																				"Ready",
																				m_appHWnd,
																				STATUS_BAR_ID );

	// get size of status bar
	GetWindowRect( m_statusBarHWnd, &rect );
	width = rect.right - rect.left;

	//
	// split the bar up into segments so we can put different text in
	// each of them
	//
	for( Int i = 0; i < STATUS_NUM_PARTS - 1; i++ )
	{

		sizes[ i ] = (i + 1) * (width / STATUS_NUM_PARTS);

	}  // end for i
	sizes[ STATUS_NUM_PARTS - 1 ] = -1;  // right edge
  SendMessage( m_statusBarHWnd, SB_SETPARTS, STATUS_NUM_PARTS, (LPARAM)sizes );

}  // end createStatusBar

// GUIEdit::statusMessage =====================================================
/** Set a message in the status bar */
//=============================================================================
void GUIEdit::statusMessage( StatusPart part, const char *message )
{

	// check for out of bounds part
	if( part < 0 || part >= STATUS_NUM_PARTS )
	{

		DEBUG_LOG(( "Status message part out of range '%d', '%s'", part, message ));
		assert( 0 );
		return;

	}  // end if

	if( m_statusBarHWnd )
		SendMessage( m_statusBarHWnd, SB_SETTEXT, part, (LPARAM)message );

}  // end statusMessage

// GUIEdit::createToolbar =====================================================
/** Create the toolbar for the editor */
//=============================================================================
void GUIEdit::createToolbar( void )
{

}  // end createToolbar

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GUIEdit::newLayout =========================================================
/** Reset the editor for a new layout */
//=============================================================================
Bool GUIEdit::newLayout( void )
{

	// delete all windows
	deleteAllWindows();

	// we are now fresh and new, like when everything was green and good
	setUnsaved( FALSE );

	// remove layout entries
	m_layoutInitString = GUIEDIT_NONE_STRING;
	m_layoutUpdateString = GUIEDIT_NONE_STRING;
	m_layoutShutdownString = GUIEDIT_NONE_STRING;

	return TRUE;

}  // end newLayout

// GUIEdit::menuExit ==========================================================
/** The user clicked on exit in the menu and wishes to exit the editor */
//=============================================================================
Bool GUIEdit::menuExit( void )
{
	Int result;

	//
	// if the contents of the editor are unsaved ask the user if they want
	// to save before quitting
	//
	if( m_unsaved )
	{

		result = MessageBox( m_appHWnd, "Save file before quitting?",
												 "Save?", MB_YESNOCANCEL );
		if( result == IDCANCEL )
			return TRUE;  // no error
		else if( result == IDYES )
		{
			Bool success;

			// save all our data
			success = TheEditor->menuSave();

			//
			// if we were unable to save file ask them if it's still OK to
			// quit and lose the data
			//
			if( success == FALSE )
				MessageBox( m_appHWnd,
										"File not saved.  If you continue to exit all data will be LOST!",
										"File Not Saved", MB_OK );

		}  // end if

	}  // end if

	// ask them if they really want to quit
	result = MessageBox( m_appHWnd, "Exit GUIEdit?", "Really Quit?", MB_YESNO );
	if( result == IDYES )
		DestroyWindow( m_appHWnd );

	return TRUE;

}  // end menuExit

// GUIEdit::menuNew ===========================================================
/** file->new menu option */
//=============================================================================
Bool GUIEdit::menuNew( void )
{

	//
	// if the current data is unsaved ask the user if they want to save
	// before proceeding with a new layout
	//
	if( m_unsaved == TRUE )
	{
		Int result;

		result = MessageBox( m_appHWnd,
												 "Current data is not saved.  Save before proceeding?",
												 "Save?",
												 MB_YESNOCANCEL );
		if( result == IDCANCEL )
			return TRUE;  // no error
		else if( result == IDYES )
		{
			Bool success;

			// do the save
			success = menuSave();

			// save failed, ask to proceed anyway with new layout
			if( success == FALSE )
			{

				result = MessageBox( m_appHWnd,
														 "File not saved.  Proceed with new layout anyway?  Current layout will be LOST!",
														 "File Not Saved",
														 MB_YESNO );
				if( result == IDNO )
					return TRUE;  // they chose to proceed anyway, no error

			}  // end if

		}  // end if

	}  // end if

	// reset the layout in the editor
	strcpy( m_savePathAndFilename, "" );
	strcpy( m_saveFilename, "" );

	newLayout();

	return TRUE;

}  // end menuNew

// GUIEdit::stripNameDecorations ==============================================
/** Strip name decorations of the entire tree of windows */
//=============================================================================
void GUIEdit::stripNameDecorations( GameWindow *root )
{

	// end of recursion
	if( root == NULL )
		return;

	// strip off this name if present
	WinInstanceData *instData = root->winGetInstanceData();

	if( !instData->m_decoratedNameString.isEmpty() )
	{
		char nameOnly[ MAX_WINDOW_NAME_LEN ];
		const char *c;

		// skip past the "filename.wnd:" to the name only
		c = strchr( instData->m_decoratedNameString.str(), ':' );
		if( c )
		{

			// skip beyong the scope resolution colon
			c++;

			// copy the name
			strcpy( nameOnly, c );

			// put the name only in the decoration field
			instData->m_decoratedNameString = nameOnly;

		}  // end if

	}  // end if

	// strip from children
	GameWindow *child;
	for( child = root->winGetChild(); child; child = child->winGetChild() )
		stripNameDecorations( child );

	// onto the next
	stripNameDecorations( root->winGetNext() );

}  // end stripNameDecorations

//=============================================================================
/** After a load, we will restore default callbacks to all user windows
	* loaded from the file */
//=============================================================================
void GUIEdit::revertDefaultCallbacks( GameWindow *root )
{

	// end recursion
	if( root == NULL )
		return;

	// if this is a user window, set the default callbacks
	if( windowIsGadget( root ) == FALSE )
	{

		root->winSetSystemFunc( TheWindowManager->getDefaultSystem() );
		root->winSetInputFunc( TheWindowManager->getDefaultInput() );
		root->winSetTooltipFunc( TheWindowManager->getDefaultTooltip() );
		root->winSetDrawFunc( TheWindowManager->getDefaultDraw() );

	}  // end if

	// do the children
	revertDefaultCallbacks( root->winGetChild() );

	// do the next window
	revertDefaultCallbacks( root->winGetNext() );

}  // end revertDefaultCallbacks

// GUIEdit::menuOpen ==========================================================
/** User has clicked on file->open */
//=============================================================================
Bool GUIEdit::menuOpen( void )
{
	char *filePath;

	//
	// if the current data is unsaved ask the user if they want to save
	// before proceeding with a new layout
	//
	if( m_unsaved == TRUE )
	{
		Int result;

		result = MessageBox( m_appHWnd,
												 "Current data is not saved.  Save before proceeding?",
												 "Save?",
												 MB_YESNOCANCEL );
		if( result == IDCANCEL )
			return TRUE;  // no error
		else if( result == IDYES )
		{
			Bool success;

			// do the save
			success = menuSave();

			// save failed, ask to proceed anyway with new layout
			if( success == FALSE )
			{

				result = MessageBox( m_appHWnd,
														 "File not saved.  Proceed with open layout anyway?  Current layout will be LOST!",
														 "File Not Saved",
														 MB_YESNO );
				if( result == IDNO )
					return TRUE;  // they chose to proceed anyway, no error

			}  // end if

		}  // end if

	}  // end if

	// open the standard window file browser
	filePath = openDialog();

	// if no filename came back they cancelled this operation
	if( filePath == NULL )
		return FALSE;  // file not opened

	//
	// OK, given the full path to the file, save that full path and
	// just the filename in members of the editor
	//
	setSaveFile( filePath );

	// clear the contents of the editor
	newLayout();

	// load the new layout file
	WindowLayoutInfo info;
	TheWindowManager->winCreateFromScript( AsciiString(filePath), &info );

	// save the strings loaded for the layout callbacks
	setLayoutInit( info.initNameString );
	setLayoutUpdate( info.updateNameString );
	setLayoutShutdown( info.shutdownNameString );

	//
	// strip off all the decorations on each window name, they are
	// unnecessary complication in the editor
	//
	stripNameDecorations( TheWindowManager->winGetWindowList() );

	//
	// remove the user callbacks and set them to default draw, input, etc ...
	// we COULD use the real ones, but then we'd have to be assured that
	// every one we wrote would behave if in the editor or the game.
	// It's just easier to not worry about that, and the defaults are
	// good enough for general purpose in all situations
	//
	revertDefaultCallbacks( TheWindowManager->winGetWindowList() );

	// reset the tree view and add all the windows loaded
	TheHierarchyView->reset();
	GameWindow *window;
	for( window = TheWindowManager->winGetWindowList();
			 window;
			 window = window->winGetNext() )
		TheHierarchyView->addWindow( window, HIERARCHY_ADD_AT_BOTTOM );

	/** @todo should probably make the window manager interface for
	this a little nicer to tell us success and whatnot */

	return TRUE;

}  // end menuOpen

// GUIEdit::menuSave ==========================================================
/** file->save menu option */
//=============================================================================
Bool GUIEdit::menuSave( void )
{
	Bool success;

	// if no filename is yet specified then go through the save as logic
	if( strlen( m_savePathAndFilename ) == 0 )
		return menuSaveAs();

	// save all our data to the specified filename
	success = saveData( m_savePathAndFilename, m_saveFilename );
	if( success == TRUE )
	{

		// our contents are now considered "unchanged"
		setUnsaved( FALSE );

	}  // end if
	else
	{

		MessageBox( m_appHWnd, "Layout not saved!", "Error", MB_OK );

	}  // end else

	return success;

}  // end menuSave

// GUIEdit::menuSaveAs ========================================================
/** file->saveAs menu option */
//=============================================================================
Bool GUIEdit::menuSaveAs( void )
{
	char *filePath;
	Bool success;

	// open the standard window file browser
	filePath = saveAsDialog();

	// if no filename came back they cancelled this operation
	if( filePath == NULL )
		return FALSE;  // save not done

	// OK, save the filename we're going to use
	setSaveFile( filePath );

	// save all our data to the specified filename
	success = saveData( m_savePathAndFilename, m_saveFilename );
	if( success == TRUE )
	{

		// our contents are now considered "unchanged"
		setUnsaved( FALSE );

	}  // end if
	else
	{

		MessageBox( m_appHWnd, "Layout not saved!", "Error", MB_OK );

	}  // end else

	return success;

}  // end menuSaveAs

// GUIEdit::menuCopy ==========================================================
/** Copy selected windows into clipboard */
//=============================================================================
Bool GUIEdit::menuCopy( void )
{
	WindowSelectionEntry *select;

	// trivial case, nothing selected
	select = getSelectList();
	if( select == NULL )
	{

		MessageBox( m_appHWnd, "You must have windows selected before you can copy them.",
								"No Windows Selected", MB_OK );
		return TRUE;

	}  // end if

	//
	// cut the selected windows out of the current window system, and
	// into the clipboard
	//
	TheGUIEditWindowManager->copySelectedToClipboard();

	return TRUE;

}  // end menuCopy

// GUIEdit::menuPaste =========================================================
/** Paste contents of clipboard into current layout */
//=============================================================================
Bool GUIEdit::menuPaste( void )
{

	TheGUIEditWindowManager->pasteClipboard();
	return TRUE;

}  // end menuPaste

// GUIEdit::menuCut ===========================================================
/** Cut selected windows into the clipboard */
//=============================================================================
Bool GUIEdit::menuCut( void )
{
	WindowSelectionEntry *select;

	// trivial case, nothing selected
	select = getSelectList();
	if( select == NULL )
	{

		MessageBox( m_appHWnd, "You must have windows selected before you can cut.",
								"No Windows Selected", MB_OK );
		return TRUE;

	}  // end if

	//
	// cut the selected windows out of the current window system, and
	// into the clipboard
	//
	TheGUIEditWindowManager->cutSelectedToClipboard();

	return TRUE;

}  // end menuCut

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GUIEdit::isWindowSelected ==================================================
/** Is the window selected */
//=============================================================================
Bool GUIEdit::isWindowSelected( GameWindow *window )
{
	WindowSelectionEntry *entry;

	// sanity
	if( window == NULL )
		return FALSE;

	// find entry
	entry = findSelectionEntry( window );
	if( entry )
		return TRUE;
	else
		return FALSE;

}  // end isWindowSelected

// GUIEdit::selectWindow ======================================================
/** Add window to selection list */
//=============================================================================
void GUIEdit::selectWindow( GameWindow *window )
{
	WindowSelectionEntry *entry;

	// sanity
	if( window == NULL )
		return;

	// do not add to list if already on it
	if( isWindowSelected( window ) == TRUE )
		return;

	// allocate new entry and add to list
	entry = new WindowSelectionEntry;
	if( entry == NULL )
	{

		DEBUG_LOG(( "Unable to allocate selection entry for window" ));
		assert( 0 );
		return;

	}  // end if

	// fill out information and tie to head of list
	entry->window = window;
	entry->prev = NULL;
	entry->next = m_selectList;
	if( m_selectList )
		m_selectList->prev = entry;
	m_selectList = entry;

	// select this window in the hierarchy if there is only one selected
	if( selectionCount() == 1 )
		TheHierarchyView->selectWindow( window );

}  // end selectWindow

// GUIEdit::unSelectWindow ====================================================
/** Remove window from the selection list */
//=============================================================================
void GUIEdit::unSelectWindow( GameWindow *window )
{
	WindowSelectionEntry *entry;

	// sanity
	if( window == NULL )
		return;

	// find entry
	entry = findSelectionEntry( window );
	if( entry )
	{

		// remove from list
		if( entry->next )
			entry->next->prev = entry->prev;
		if( entry->prev )
			entry->prev->next = entry->next;
		else
			m_selectList = entry->next;

		// delete the entry
		delete entry;

	}  // end if

}  // end unSelectWindow

// GUIEdit::clearSelections ===================================================
/** Clear the entire selection list */
//=============================================================================
void GUIEdit::clearSelections( void )
{

	while( m_selectList )
		unSelectWindow( m_selectList->window );

}  // end clearSelections

// GUIEdit::selectionCount ====================================================
/** How many items are selected */
//=============================================================================
Int GUIEdit::selectionCount( void )
{
	WindowSelectionEntry *select;
	Int count = 0;

	select = m_selectList;
	while( select )
	{

		count++;
		select = select->next;

	}  // end while

	return count;

}  // end selectionCount

// GUIEdit::notifyNewWindow ===================================================
/** The passed in window has just been added into the GUI layout.  It
	* It may also contain children which we will recursively call upon
	* so that every window gets a chance to run through this method */
//=============================================================================
void GUIEdit::notifyNewWindow( GameWindow *window )
{

	// end of recursion
	if( window == NULL )
		return;

	//
	// add this window to the hierarchy view at the top, we're adding it
	// at the bottom because presumabely this method was called directly after
	// the window was placed in the world, and it now resides on the top
	// of the window chain, therefore at the bottom of the hierarchy, drawn last
	//
	TheHierarchyView->addWindow( window, HIERARCHY_ADD_AT_TOP );

	//
	// call this notification method for any children we might have, we
	// do NOT notify for the children of gadget controls since those are
	// an "atomic" unit, (except for the Tab Control, which has window children)
	//
	if( (windowIsGadget( window ) == FALSE)  ||  (window->winGetStyle() & GWS_TAB_CONTROL) )
	{
		GameWindow *child;

		for( child = window->winGetChild(); child; child = child->winGetNext() )
			notifyNewWindow( child );

	}  // end if

}  // end notifyNewWindow

// GUIEdit::deleteSelected ====================================================
/** Delete the windows in the selection list */
//=============================================================================
void GUIEdit::deleteSelected( void )
{
	Int count = TheEditor->selectionCount();
	Int i;
	GameWindow **deleteList;
	WindowSelectionEntry *select;

	//
	// the act of deleting things can actually change selections, like when
	// removing a hierarchy item, the selection is moved ... to avoid any
	// confusion we will build a list of windows to be deleted, independent
	// of the select list and delete those
	//
	deleteList = new GameWindow *[ count ];
	if( deleteList == NULL )
	{

		DEBUG_LOG(( "Cannot allocate delete list!" ));
		assert( 0 );
		return;

	}  // end if

	// fill out the delete list
	for( i = 0, select = m_selectList; i < count; i++, select = select->next )
		deleteList[ i ] = select->window;

	// delete them all
	for( i = 0; i < count; i++ )
		deleteWindow( deleteList[ i ] );

	// free memory for the delete list
	delete [] deleteList;

}  // end deleteSelected

// GUIEdit::bringSelectedToTop ================================================
/** Bring the selected windows to the top so they draw on top of
	* other windows.  For window with no parents this brings them to
	* the top of the window stack for all windows, for child windows it will
	* bring them to the top of the child list for their parent */
//=============================================================================
void GUIEdit::bringSelectedToTop( void )
{
	Int count = TheEditor->selectionCount();

	// no-op
	if( count == 0 )
		return;

	//
	// build a list of selections to operate on, we need to do this
	// because the process of changing the hierarchy view will change the
	// select list on us
	//
	GameWindow **snapshot;
	snapshot = new GameWindow *[ count ];
	if( snapshot == NULL )
	{

		DEBUG_LOG(( "bringSelectedToTop: Unabled to allocate selectList" ));
		assert( 0 );
		return;

	}  // end if

	// take the snapshot
	Int i;
	WindowSelectionEntry *select;
	for( i = 0, select = m_selectList; i < count; i++, select = select->next )
		snapshot[ i ] = select->window;

	// bring all the selected windows to the top
	for( i = 0; i < count; i++ )
	{

		// bring window to the top
		snapshot[ i ]->winBringToTop();

		// update the hierarchy to have the new window on the top
		TheHierarchyView->bringWindowToTop( snapshot[ i ] );

	}  // end for i

	// delete the snapshot list
	delete [] snapshot;

}  // end bringSelectedToTop

// GUIEdit::dragMoveSelectedWindows ===========================================
/** Move all the windows in the selection list from a drag move, note
	* that we are keeping the positions within the screen and parent
	* windows */
//=============================================================================
void GUIEdit::dragMoveSelectedWindows( ICoord2D *dragOrigin,
																			 ICoord2D *dragDest )
{
	WindowSelectionEntry *select;
	GameWindow *window;
	ICoord2D moveLoc, safeLoc;
	ICoord2D origin;

	// sanity
	if( dragOrigin == NULL || dragDest == NULL )
		return;

	// traverse selection list
	select = m_selectList;
	while( select )
	{

		// get window info
		window = select->window;
		window->winGetScreenPosition( &origin.x, &origin.y );

		// compute new location
		moveLoc.x = origin.x + (dragDest->x - dragOrigin->x);
		moveLoc.y = origin.y + (dragDest->y - dragOrigin->y);

		// snap move location to grid if on
		if( (TheEditor->getMode() == MODE_DRAG_MOVE) && TheEditor->isGridSnapOn() )
			TheEditor->gridSnapLocation( &moveLoc, &moveLoc );

		// kee the location legal
		computeSafeLocation( window, moveLoc.x, moveLoc.y, &safeLoc.x, &safeLoc.y );

		// move the window
		moveWindowTo( window, safeLoc.x, safeLoc.y );

		// goto next selected window
		select = select->next;

	}  // end while


}  // end dragMoveSelectedWindows

// GUIEdit::getSelectList =====================================================
/** Return the selection list */
//=============================================================================
WindowSelectionEntry *GUIEdit::getSelectList( void )
{

	return m_selectList;

}  // end getSelectList

// GUIEdit::getFirstSelected ==================================================
/** Get the first GameWindow * from the selection list */
//=============================================================================
GameWindow *GUIEdit::getFirstSelected( void )
{

	if( m_selectList )
		return m_selectList->window;

	return NULL;

}  // end getFirstSelected

// GUIEdit::computeSafeLocation ===============================================
/** If we attempt to move the window to the given (x,y) it may result
	* in that location being either outside of the screen or outside of
	* the parent.  This method will compute a location as close to the
	* desired (x,y) as possible but still remain within the parent
	* and on the screen */
//=============================================================================
void GUIEdit::computeSafeLocation( GameWindow *window,
																	 Int x, Int y, Int *safeX, Int *safeY )
{
	Int dx, dy;
	ICoord2D origin, size;
	GameWindow *parent;
	IRegion2D region;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get region of the window in question
	window->winGetRegion( &region );

	// how far from window current position to new one
	dx = x - origin.x;
	dy = y - origin.y;

	// clip window to parent if there is one
	parent = window->winGetParent();
	if( parent )
	{
		ICoord2D parentSize;

		// get parent size
		parent->winGetSize( &parentSize.x, &parentSize.y );

		if( region.lo.x + dx < 0 )
			dx = 0 - region.lo.x;
		else if( region.hi.x + dx > parentSize.x )
			dx = parentSize.x - region.hi.x;

		if( region.lo.y + dy < 0 )
			dy = 0 - region.lo.y;
		else if( region.hi.y + dy > parentSize.y )
			dy = parentSize.y - region.hi.y;

	}  // end else if, parent

	// Move the window, but keep it completely visible within screen boundaries
	IRegion2D newRegion;
	ICoord2D grabSize;

	window->winGetPosition( &newRegion.lo.x, &newRegion.lo.y );
	window->winGetSize( &grabSize.x, &grabSize.y );

	newRegion.lo.x += dx;
	newRegion.lo.y += dy;
	if( newRegion.lo.x < 0 )
		newRegion.lo.x = 0;
	if( newRegion.lo.y < 0 )
		newRegion.lo.y = 0;

	newRegion.hi.x = newRegion.lo.x + grabSize.x;
	newRegion.hi.y = newRegion.lo.y + grabSize.y;
	if( newRegion.hi.x > (Int)TheDisplay->getWidth() )
		newRegion.hi.x = (Int)TheDisplay->getWidth();
	if( newRegion.hi.y > (Int)TheDisplay->getHeight() )
		newRegion.hi.y = (Int)TheDisplay->getHeight();

	// compute new position
	newRegion.lo.x = newRegion.hi.x - grabSize.x;
	newRegion.lo.y = newRegion.hi.y - grabSize.y;

	// safe position is now all figured out
	*safeX = newRegion.lo.x;
	*safeY = newRegion.lo.y;

}  // end computeSafeLocation

// GUIEdit::computeSafeSizeLocation ===========================================
/** Like the method computeSafeLocation, this method also takes into
	* account the new position AND size, keeping them within the
	* screen and within any parents
	*
	* NOTE: this assumes it is for the resizing code which assumes that
	* the resize will not invert across the anchor point so we're
	* only checking origin points and sizes compared to parents
	* and the screen;
	*
	* Location returned is relative to parent window if present
	*/
//=============================================================================
void GUIEdit::computeSafeSizeLocation( GameWindow *window,
																			 Int newX, Int newY,
																			 Int newWidth, Int newHeight,
																			 Int *safeX, Int *safeY,
																			 Int *safeWidth, Int *safeHeight )
{
	GameWindow *parent;
	ICoord2D parentLoc;
	ICoord2D parentSize;
	ICoord2D topLeftLimit;
	ICoord2D bottomRightLimit;

	// get parent window and data is present
	parent = window->winGetParent();
	if( parent )
	{

		parent->winGetScreenPosition( &parentLoc.x, &parentLoc.y );
		parent->winGetSize( &parentSize.x, &parentSize.y );

	}  // end if

	// upper left corner must be in screen or in parent
	topLeftLimit.x = 0;  // screen top left
	topLeftLimit.y = 0;  // screen top left
	if( parent )
		topLeftLimit = parentLoc;

	if( newX < topLeftLimit.x )
	{
		newWidth = newWidth - (topLeftLimit.x - newX);
		newX = topLeftLimit.x;
	}
	if( newY < topLeftLimit.y )
	{
		newHeight = newHeight - (topLeftLimit.y - newY);
		newY = topLeftLimit.y;
	}

	// check size at new position compared to parent or screen
	bottomRightLimit.x = TheDisplay->getWidth();
	bottomRightLimit.y = TheDisplay->getHeight();
	if( parent )
	{

		bottomRightLimit.x = parentLoc.x + parentSize.x;
		bottomRightLimit.y = parentLoc.y + parentSize.y;

	}  // end if
	if( newX + newWidth > bottomRightLimit.x )
		newWidth = bottomRightLimit.x - newX;
	if( newY + newHeight > bottomRightLimit.y )
		newHeight = bottomRightLimit.y - newY;

	// all done, return location and width relative to parent
	*safeWidth = newWidth;
	*safeHeight = newHeight;
	*safeX = newX;
	*safeY = newY;
	if( parent )
	{
		*safeX = *safeX - parentLoc.x;
		*safeY = *safeY - parentLoc.y;
	}

}  // end computeSafeSizeLocation

// GUIEdit::computeResizeLocation =============================================
/** Given the current resize drag mode, the selected window to resize,
	* the start location of the drag, and the end location of the drag,
	* figure out what the upper left corner location and the size of the
	* window should be */
//=============================================================================
void GUIEdit::computeResizeLocation( EditMode resizeMode,
																		 GameWindow *window,
																		 ICoord2D *resizeOrigin,
																		 ICoord2D *resizeDest,
																		 ICoord2D *resultLoc,
																		 ICoord2D *resultSize )
{
	Int newX, newY, newSizeX, newSizeY;
	Int sizeLimit = 5;

	// sanity
	if( window == NULL || resizeOrigin == NULL || resizeDest == NULL ||
			resultLoc == NULL || resultSize == NULL )
		return;

	// get the current position and size of the window
	ICoord2D origin, size, bottomRight;

	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );
	bottomRight.x = origin.x + size.x;
	bottomRight.y = origin.y + size.y;

	//
	// compute new size and width based on which resize mode we're
	// using for the anchor point of the window
	//
	switch( resizeMode )
	{

		// ------------------------------------------------------------------------
		case MODE_RESIZE_TOP_LEFT:

			// bottom right is the anchor
			newX = origin.x + (resizeDest->x - resizeOrigin->x);
			newY = origin.y + (resizeDest->y - resizeOrigin->y);

			// don't let it become too small or inverted
			if( newX >= bottomRight.x - sizeLimit )
				newX = bottomRight.x - sizeLimit;
			if( newY >= bottomRight.y - sizeLimit )
				newY = bottomRight.y - sizeLimit;

			// compute new size
			newSizeX = bottomRight.x - newX;
			newSizeY = bottomRight.y - newY;

			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_TOP_RIGHT:

			// bottom left is the anchor
			newX = origin.x;
			newY = resizeDest->y;

			// don't let it become too small or inverted
			if( newY >= bottomRight.y - sizeLimit )
				newY = bottomRight.y - sizeLimit;

			// compute new size
			newSizeX = resizeDest->x - origin.x;
			newSizeY = (origin.y + size.y) - resizeDest->y;

			// don't let it invert
			if( newSizeX < sizeLimit )
				newSizeX = sizeLimit;
			if( newSizeY < sizeLimit )
				newSizeY = sizeLimit;

			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_BOTTOM_RIGHT:

			// top left is anchor
			newX = origin.x;
			newY = origin.y;

			newSizeX = resizeDest->x - origin.x;
			newSizeY = resizeDest->y - origin.y;

			// don't let it invert or get too small
			if( newSizeX < sizeLimit )
				newSizeX = sizeLimit;
			if( newSizeY < sizeLimit )
				newSizeY = sizeLimit;

			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_BOTTOM_LEFT:

			// top right is the anchor
			newX = resizeDest->x;
			newY = origin.y;

			// don't let it get too small or inverted
			if( newX >= bottomRight.x - sizeLimit )
				newX = bottomRight.x - sizeLimit;

			// compute new size
			newSizeX = bottomRight.x - newX;
			newSizeY = resizeDest->y - origin.y;

			// don't let it go inverted
			if( newSizeY < sizeLimit )
				newSizeY = sizeLimit;

			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_TOP:

			// bottom SIDE is the anchor
			newX = origin.x;
			newY = resizeDest->y;

			// don't let it invert
			if( newY >= bottomRight.y - sizeLimit )
				newY = bottomRight.y - sizeLimit;

			// compute new size
			newSizeX = size.x;
			newSizeY = bottomRight.y - newY;

			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_RIGHT:

			// left SIDE is the anchor
			newX = origin.x;
			newY = origin.y;

			// compute new size
			newSizeX = resizeDest->x - origin.x;
			newSizeY = size.y;

			// don't let it invert or get too small
			if( newSizeX < sizeLimit )
				newSizeX = sizeLimit;

			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_BOTTOM:

			// top SIDE is the anchor
			newX = origin.x;
			newY = origin.y;

			// compute new size
			newSizeX = size.x;
			newSizeY = resizeDest->y - origin.y;

			// dont' let it invert or get too small
			if( newSizeY < sizeLimit )
				newSizeY = sizeLimit;

			break;

		// ------------------------------------------------------------------------
		case MODE_RESIZE_LEFT:

			// right SIDE is the anchor
			newX = resizeDest->x;
			newY = origin.y;

			// don't let it invert or get to small
			if( newX >= bottomRight.x - sizeLimit )
				newX = bottomRight.x - sizeLimit;

			// compute new size
			newSizeX = bottomRight.x - newX;
			newSizeY = size.y;

			break;

	}  // end switch( resizeMode )

	// to finalize the size we must now clip to any parent or the screen
	computeSafeSizeLocation( window,
													 newX, newY,
													 newSizeX, newSizeY,
													 &resultLoc->x, &resultLoc->y,
													 &resultSize->x, &resultSize->y );

}  // end computeResizeLocation

// GUIEdit::moveWindowTo ======================================================
/** Move the window passed into the the absolute position (x,y),
	* note that there is NO SAFE checking done on that position */
//=============================================================================
void GUIEdit::moveWindowTo( GameWindow *window, Int x, Int y )
{

	// set the position
	window->winSetPosition( x, y );

	// we've now made a change
	TheEditor->setUnsaved( TRUE );

}  // end moveWindowTo

// GUIEdit::windowIsGadget ====================================================
/** Return TRUE if this window is one of our predefined gadtet types */
//=============================================================================
Bool GUIEdit::windowIsGadget( GameWindow *window )
{

	// sanity
	if( window == NULL )
		return FALSE;

	return BitIsSet( window->winGetStyle(), GWS_GADGET_WINDOW );

}  // end windowIsGadget

// GUIEdit::gridSnapLocation ==================================================
/** Given the source input point, return in 'snapped' the closest grid
	* point */
//=============================================================================
void GUIEdit::gridSnapLocation( ICoord2D *source, ICoord2D *snapped )
{

	// sanity
	if( source == NULL || snapped == NULL )
		return;

	snapped->x = (source->x / m_gridResolution) * m_gridResolution;
	snapped->y = (source->y / m_gridResolution) * m_gridResolution;

}  // end gridSnapLocation

// GUIEdit::checkMenuItem =====================================================
/** Check the menu item from the guiedit main menu */
//=============================================================================
void GUIEdit::checkMenuItem( Int item )
{
	HMENU menu = GetMenu( m_appHWnd );

	// sanity
	if( menu == NULL )
		return;

	// check it
	CheckMenuItem( menu, item, MF_CHECKED );

}  // end checkMenuItem

// GUIEdit::unCheckMenuItem ===================================================
/** Un-check the menu item from the guiedit main menu */
//=============================================================================
void GUIEdit::unCheckMenuItem( Int item )
{
	HMENU menu = GetMenu( m_appHWnd );

	// sanity
	if( menu == NULL )
		return;

	// check it
	CheckMenuItem( menu, item, MF_UNCHECKED );

}  // end unCheckMenuItem

// GUIEdit::isNameDuplicate ===================================================
/** Is the name passed in found as the name of any window in in the
	* tree starting at root ... but we will ignore the window 'ignore'
	* if present */
//=============================================================================
Bool GUIEdit::isNameDuplicate( GameWindow *root, GameWindow *ignore, AsciiString name )
{
	WinInstanceData *instData;

	// end of recursion, sanity for name, and empty name ("") is always OK
	if( root == NULL || name.isEmpty() )
		return FALSE;  // name is a-ok! :)

	// get instance data
	instData = root->winGetInstanceData();

	// compare name
	if( root != ignore )
		if( instData->m_decoratedNameString == name )
			return TRUE;  // no need to search anymore

	//You only call this on the first child since the call right after it will handle siblings (depth first)
	GameWindow *child = root->winGetChild();
	if( isNameDuplicate( child, ignore, name ) == TRUE )
		return TRUE; //duplicate was found

	// check the next window in the list
	return isNameDuplicate( root->winGetNext(), ignore, name );

}  // end isNameDuplicate

// GUIEdit::loadGUIEditFontLibrary ============================================
/** Load the set of fonts that we will make available to users in
	* in the editor */
//=============================================================================
void GUIEdit::loadGUIEditFontLibrary( FontLibrary *library )
{

	// sanity
	if( library == NULL )
		return;

	AsciiString fixedSys("FixedSys");
	AsciiString times("Times New Roman");
	library->getFont( fixedSys, 12, FALSE );
	library->getFont( times, 14, FALSE );
	library->getFont( times, 14, TRUE );
	library->getFont( times, 12, FALSE );
	library->getFont( times, 12, TRUE );
	library->getFont( times, 10, FALSE );
	library->getFont( times, 10, TRUE );

}  // end loadGUIEditFontLibrary

// GUIEdit::setShowHiddenOutlines =============================================
//=============================================================================
void GUIEdit::setShowHiddenOutlines( Bool show )
{

	m_showHiddenOutlines = show;

	if( m_showHiddenOutlines  )
		CheckMenuItem( GetMenu( m_appHWnd ), MENU_SHOW_HIDDEN_OUTLINES, MF_CHECKED );
	else
		CheckMenuItem( GetMenu( m_appHWnd ), MENU_SHOW_HIDDEN_OUTLINES, MF_UNCHECKED );

}  // end setShowHiddenOutlines

// GUIEdit::setShowSeeThruOutlines ============================================
//=============================================================================
void GUIEdit::setShowSeeThruOutlines( Bool show )
{

	m_showSeeThruOutlines = show;

	if( m_showSeeThruOutlines )
		CheckMenuItem( GetMenu( m_appHWnd ), MENU_SHOW_SEE_THRU_OUTLINES, MF_CHECKED );
	else
		CheckMenuItem( GetMenu( m_appHWnd ), MENU_SHOW_SEE_THRU_OUTLINES, MF_UNCHECKED );

}  // end setShowSeeThruOutlines

