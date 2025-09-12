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

// FILE: GameWindowManagerScript.cpp //////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: GameWindowManagerScript.cpp
//
// Created:   Colin Day, June 2001
//						Dean Iverson, May 1998
//
// Desc:      Reading window definition files from disk for the window manager
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "Common/Debug.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GameMemory.h"
#include "Common/NameKeyGenerator.h"
#include "Common/FunctionLexicon.h"
#include "GameClient/Display.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetTabControl.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetRadioButton.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetSlider.h"
#include "GameClient/GameText.h"
#include "GameClient/HeaderTemplate.h"



// DEFINES ////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE TYPES //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
	WIN_BUFFER_LENGTH  = 2048,
	WIN_STACK_DEPTH    = 10,
};

//-------------------------------------------------------------------------------------------------
/** Layout parse structure ... these data items apply to the window file itself,
	* they are not associated with any window, but rather just a block of data in
	* every file */
//-------------------------------------------------------------------------------------------------
struct LayoutScriptParse
{

	const char *name;
	Bool (*parse)( const char *token, char *buffer, UnsignedInt version, WindowLayoutInfo *info );

};

// GameWindowParse ------------------------------------------------------------
/** used to match database fields to parsing functions */
//-----------------------------------------------------------------------------
struct GameWindowParse
{

	const char *name;
	Bool (*parse)( const char *token, WinInstanceData *, char *, void * );

};

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// window methods and their string representations
static GameWinSystemFunc		systemFunc = NULL;
static GameWinInputFunc		inputFunc = NULL;
static GameWinTooltipFunc	tooltipFunc = NULL;
static GameWinDrawFunc			drawFunc = NULL;
static AsciiString theSystemString;
static AsciiString theInputString;
static AsciiString theTooltipString;
static AsciiString theDrawString;

// default visual properties
static Color defEnabledColor		= 0;
static Color defDisabledColor		= 0;
static Color defBackgroundColor	= 0;
static Color defHiliteColor			= 0;
static Color defSelectedColor		= 0;
static Color defTextColor				= 0;
static GameFont  *defFont				= NULL;

//
// These strings must be in the same order as they are in their definitions
// (see WIN_STATUS_* enums and GWS_* enums).
//
const char *WindowStatusNames[] = { "ACTIVE", "TOGGLE", "DRAGABLE", "ENABLED", "HIDDEN",
														  "ABOVE", "BELOW", "IMAGE", "TABSTOP", "NOINPUT",
														  "NOFOCUS", "DESTROYED", "BORDER",
														  "SMOOTH_TEXT", "ONE_LINE", "NO_FLUSH", "SEE_THRU",
															"RIGHT_CLICK", "WRAP_CENTERED", "CHECK_LIKE","HOTKEY_TEXT",
															"USE_OVERLAY_STATES", "NOT_READY", "FLASHING", "ALWAYS_COLOR",
															"ON_MOUSE_DOWN",
															NULL };

const char *WindowStyleNames[] = { "PUSHBUTTON",	"RADIOBUTTON",	"CHECKBOX",
														 "VERTSLIDER",	"HORZSLIDER",		"SCROLLLISTBOX",
														 "ENTRYFIELD",	"STATICTEXT",		"PROGRESSBAR",
														 "USER",				"MOUSETRACK",		"ANIMATED",
														 "TABSTOP",			"TABCONTROL",		"TABPANE",
														 "COMBOBOX",
														 NULL };

// Implement a stack to keep track of parent/child nested window descriptions.
static GameWindow *windowStack[ WIN_STACK_DEPTH ];
static GameWindow **stackPtr;

// for parsing
static const char *seps = " =;\n\r\t";
WinDrawData enabledDropDownButtonDrawData[ MAX_DRAW_DATA ];  ///< for combo boxes
WinDrawData disabledDropDownButtonDrawData[ MAX_DRAW_DATA ];  ///< for combo boxes
WinDrawData hiliteDropDownButtonDrawData[ MAX_DRAW_DATA ];  ///< for combo boxes

WinDrawData enabledEditBoxDrawData[ MAX_DRAW_DATA ];  ///< for combo boxes
WinDrawData disabledEditBoxDrawData[ MAX_DRAW_DATA ];  ///< for combo boxes
WinDrawData hiliteEditBoxDrawData[ MAX_DRAW_DATA ];  ///< for combo boxes

WinDrawData enabledListBoxDrawData[ MAX_DRAW_DATA ];  ///< for combo boxes
WinDrawData disabledListBoxDrawData[ MAX_DRAW_DATA ];  ///< for combo boxes
WinDrawData hiliteListBoxDrawData[ MAX_DRAW_DATA ];  ///< for combo boxes

WinDrawData enabledUpButtonDrawData[ MAX_DRAW_DATA ];  ///< for list boxes and combo boxes
WinDrawData disabledUpButtonDrawData[ MAX_DRAW_DATA ];  ///< for list boxes and combo boxes
WinDrawData hiliteUpButtonDrawData[ MAX_DRAW_DATA ];  ///< for list boxes and combo boxes
WinDrawData enabledDownButtonDrawData[ MAX_DRAW_DATA ];  ///< for list boxes and combo boxes
WinDrawData disabledDownButtonDrawData[ MAX_DRAW_DATA ];  ///< for list boxes and combo boxes
WinDrawData hiliteDownButtonDrawData[ MAX_DRAW_DATA ];  ///< for list boxes and combo boxes
WinDrawData enabledSliderDrawData[ MAX_DRAW_DATA ];  ///< for list boxes and combo boxes
WinDrawData disabledSliderDrawData[ MAX_DRAW_DATA ];  ///< for list boxes and combo boxes
WinDrawData hiliteSliderDrawData[ MAX_DRAW_DATA ];  ///< for list boxes and combo boxes
WinDrawData enabledSliderThumbDrawData[ MAX_DRAW_DATA ];  ///< for sliders and list boxes and combo boxes
WinDrawData disabledSliderThumbDrawData[ MAX_DRAW_DATA ];  ///< for sliders and list boxes and combo boxes
WinDrawData hiliteSliderThumbDrawData[ MAX_DRAW_DATA ];  ///< for sliders and list boxes and combo boxes

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////
static GameWindow *parseWindow( File *inFile, char *buffer );

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// parseBitFlag ===============================================================
/** Parse one of the "flags" referred to below in the header comment
	* for ParseBitString().  Sets the appropriate bit in the 'bits' arg,
	* if successful.  Returns TRUE on success, else FALSE. */
//=============================================================================
static Bool parseBitFlag( const char *flagString, UnsignedInt *bits,
													const char **flagList )
{
	const char **c;
	int i;

	for( i = 0, c = flagList; *c; i++, c++ )
	{

		if( !stricmp( *c, flagString ) )
		{
			*bits |= (1 << i);
			return TRUE;
		}

	}

	return FALSE;

}

// parseBitString =============================================================
/** Given a character string of the form 'A+B+C+D', parse the
	* flags separated by the '+' symbols into a bitfield stored in the 'bits'
	* argument.
	* Note that this routine does not clear any bits, only sets them. */
//=============================================================================
static void parseBitString( const char *inBuffer, UnsignedInt *bits, const char **flagList )
{
	char buffer[256];
	char *tok;

	// do not modify the inBuffer argument
	strcpy( buffer, inBuffer );

	if( strncmp( buffer, "NULL", 4 ) )
	{
		for( tok = strtok( buffer, "+" ); tok; tok = strtok( NULL, "+" ) )
		{
			if ( !parseBitFlag( tok, bits, flagList ) )
			{
				DEBUG_LOG(( "ParseBitString: Invalid flag '%s'.", tok ));
			}
		}
	}

}

// readUntilSemicolon =========================================================
//=============================================================================
static void readUntilSemicolon( File *fp, char *buffer, int maxBufLen )
{
	int i = 0;
	Bool start = TRUE;

	while( i < maxBufLen )
	{

		// get next character
		fp->read(buffer + i, 1);

		// make all whitespace characters spaces
		if( isspace( buffer[ i ] ) )
		{

			if( start == FALSE )
				buffer[ i++ ] = ' ';

		}
		else
		{

			start = FALSE;

			if( buffer[ i ] == ';' )
			{

				// found end of data chunk
				buffer[ i ] = '\000';
				return;

			}

			i++;
		}

	}

	DEBUG_LOG(( "ReadUntilSemicolon: ERROR - Read buffer overflow - input truncated." ));

	buffer[ maxBufLen - 1 ] = '\000';

}

// scanBool ===================================================================
//=============================================================================
static Int scanBool( const char *source, Bool& val )
{
	Int temp = 0;
	Int ret = sscanf( source, "%d", &temp );
	val = (Bool)temp;

	return ret;
}

// scanShort ==================================================================
//=============================================================================
static Int scanShort( const char *source, Short& val )
{
	Int temp = 0;
	Int ret = sscanf( source, "%d", &temp );
	val = (Short)temp;

	return ret;
}

// scanInt ====================================================================
//=============================================================================
static Int scanInt( const char *source, Int& val )
{
	Int ret = sscanf( source, "%d", &val ); // not strictly necessary to wrap this, but it's more consistent

	return ret;
}

// scanUnsignedInt ============================================================
//=============================================================================
static Int scanUnsignedInt( const char *source, UnsignedInt& val )
{
	Int ret = sscanf( source, "%d", &val ); // not strictly necessary to wrap this, but it's more consistent

	return ret;
}

// resetWindowStack ===========================================================
//=============================================================================
static void resetWindowStack( void )
{

  memset( windowStack, 0, sizeof( windowStack ) );
  stackPtr = windowStack;

}

// resetWindowDefaults ========================================================
//=============================================================================
static void resetWindowDefaults( void )
{

	defEnabledColor = 0;
	defDisabledColor = 0;
	defBackgroundColor = 0;
	defHiliteColor = 0;
	defSelectedColor = 0;
	defTextColor = 0;
	defFont = 0;

}

// peekWindow =================================================================
//=============================================================================
static GameWindow *peekWindow( void )
{
  if (stackPtr == windowStack)
    return NULL;

  return *(stackPtr - 1);

}

// popWindow ==================================================================
//=============================================================================
static GameWindow *popWindow( void )
{

  if( stackPtr == windowStack )
    return NULL;
  stackPtr--;

  return *stackPtr;

}

// pushWindow =================================================================
//=============================================================================
static void pushWindow( GameWindow *window )
{

  if( stackPtr == &windowStack[ WIN_STACK_DEPTH - 1 ] )
	{

    DEBUG_LOG(( "pushWindow: Warning, stack overflow" ));
    return;

  }

  *stackPtr++ = window;

}

// parseColor =================================================================
/** Parse a color entry and store it in the value pointed to by the
	* 'color' parm. */
//=============================================================================
static Bool parseColor( Color *color, char *buffer )
{
  char *c;
  Byte red, green, blue;

	c = strtok( buffer, " \t\n\r" );
  red = atoi(c);

	c = strtok( NULL, " \t\n\r" );
  green = atoi(c);

	c = strtok( NULL, " \t\n\r" );
  blue = atoi(c);

	*color = TheWindowManager->winMakeColor( red, green, blue, 255 );

  return TRUE;

}

// parseDefaultColor ==========================================================
/** Parse a default color entry and store it in the value pointed to by
	* the 'color' parm. */
//=============================================================================
static Bool parseDefaultColor( Color *color, File *inFile, char *buffer )
{
	// eat '='
//	fscanf( inFile, "%*s" );
	AsciiString str;
	inFile->scanString(str);

  // Read the rest of the color definition
	readUntilSemicolon( inFile, buffer, WIN_BUFFER_LENGTH );

  if (!strcmp( buffer, "TRANSPARENT" ))
	{

		*color = WIN_COLOR_UNDEFINED;

	}
  else
    parseColor( color, buffer );

  return TRUE;

}

// parseDefaultFont ===========================================================
/** Parse the default font */
//=============================================================================
static Bool parseDefaultFont( GameFont *font, File *inFile, char *buffer )
{

	// eat '='
//	fscanf( inFile, "%*s" );
	AsciiString str;
	inFile->scanString(str);

  // Read the rest of the color definition
	readUntilSemicolon( inFile, buffer, WIN_BUFFER_LENGTH );

	/// @todo font parsing for window files work needed here
//	*font = GetFont( buffer );
//	if( *font == NULL )
//		return FALSE;

	return TRUE;

}

// parseTooltip ===============================================================
/** Parse the tooltip field */
//=============================================================================
static Bool parseTooltip( const char *token, WinInstanceData *instData,
													char *buffer, void *data )
{
	UnicodeString tooltip;
	tooltip.set(L"Need tooltip translation");
	/// @todo need to parse the tooltip in multibyte here

	instData->setTooltipText( tooltip );
	return TRUE;

}

// parseScreenRect ============================================================
/** Parse the screen rect entry which tells us the position and size
	* of window.  Note we scale for the current resolution if needed
	* and adjust to make the screen rect coords relative to any parent
	* if present */
//=============================================================================
static Bool parseScreenRect( const char *token, char *buffer,
														 Int *x, Int *y, Int *width, Int *height )
{
	GameWindow *parent = peekWindow();
	IRegion2D screenRegion;
	ICoord2D createRes;  // creation resolution
	const char *seps = " ,:=\n\r\t";
	char *c;

	c = strtok( NULL, seps );  // UPPERLEFT token
	c = strtok( NULL, seps );  // x position
	scanInt( c, screenRegion.lo.x );
	c = strtok( NULL, seps );  // y posotion
	scanInt( c, screenRegion.lo.y );

	c = strtok( NULL, seps );  // BOTTOMRIGHT token
	c = strtok( NULL, seps );  // x position
	scanInt( c, screenRegion.hi.x );
	c = strtok( NULL, seps );  // y posotion
	scanInt( c, screenRegion.hi.y );

	c = strtok( NULL, seps );  // CREATIONRESOLUTION token
	c = strtok( NULL, seps );  // x creation resolution
	scanInt( c, createRes.x );
	c = strtok( NULL, seps );  // y creation resolution
	scanInt( c, createRes.y );

	//
	// shrink or expand the screen region by the ratio of the current
	// resolution divided by the creation resolution
	//
	Real xScale = (Real)TheDisplay->getWidth() / (Real)createRes.x;
	Real yScale = (Real)TheDisplay->getHeight() / (Real)createRes.y;
	screenRegion.lo.x = (Int)((Real)screenRegion.lo.x * xScale);
	screenRegion.lo.y = (Int)((Real)screenRegion.lo.y * yScale);
	screenRegion.hi.x = (Int)((Real)screenRegion.hi.x * xScale);
	screenRegion.hi.y = (Int)((Real)screenRegion.hi.y * yScale);

	//
	// given the screen region upper left compute the upper left that we
	// will give this window, if we have a parent note that the position
	// is relative to the parent client area, if no parent is present
	// we're talking about the screen
	//
	if( parent )
	{
		ICoord2D parentScreenPos;

		// get parent position on screen
		parent->winGetScreenPosition( &parentScreenPos.x, &parentScreenPos.y );

		// save x and y with parent position as relative (0,0) location
		*x = screenRegion.lo.x - parentScreenPos.x;
		*y = screenRegion.lo.y - parentScreenPos.y;

	}
	else
	{

		*x = screenRegion.lo.x;
		*y = screenRegion.lo.y;

	}

	// save our width and height from the adjusted screen region locations
	*width = screenRegion.hi.x - screenRegion.lo.x;
	*height = screenRegion.hi.y - screenRegion.lo.y;

	return TRUE;

}

// parseImageOffset ===========================================================
/** Parse the image draw offset */
//=============================================================================
static Bool parseImageOffset( const char *token, WinInstanceData *instData,
															char *buffer, void *data )
{
  char *c;

	c = strtok( buffer, " \t\n\r" );
	instData->m_imageOffset.x = atoi( c );

	c = strtok( NULL, " \t\n\r" );
	instData->m_imageOffset.y = atoi( c );

  return TRUE;

}

// parseFont ==================================================================
/** Parse the font field */
//=============================================================================
static Bool parseFont( const char *token, WinInstanceData *instData,
											 char *buffer, void *data )
{
	char *c, *ptr;
	const char *seps = " ,\n\r\t";
	const char *stringSeps = ":,\n\r\t\"";
	char fontName[ 256 ];
	Int fontSize;
	Int fontBold;

	// "NAME"
	c = strtok( buffer, seps );  // label
	// scan to the first " mark
	ptr = buffer;
	while( *ptr != '"' )
		ptr++;
	ptr++;  // skip the "
	c = strtok( ptr, stringSeps );  // value
	strcpy( fontName, c );

	// "SIZE"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, fontSize );

	// "BOLD"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, fontBold );

	if( TheFontLibrary )
	{
		GameFont *font;

		font = TheFontLibrary->getFont( AsciiString(fontName), fontSize, fontBold );
		if( font )
			instData->m_font = font;

	}

	return TRUE;

}

// parseName =================================================================
/** Parse the NAME field */
//=============================================================================
static Bool parseName( const char *token, WinInstanceData *instData,
											 char *buffer, void *data )
{
	char *c, *ptr;
//	const char *seps = " ,\n\r\t";
	const char *stringSeps = "\"";

	// scan to the first " mark
	ptr = buffer;
	while( *ptr != '"' )
		ptr++;
	ptr++;  // skip the first "
	c = strtok( ptr, stringSeps );  // name value
	instData->m_decoratedNameString = c;

	// given the name assign a window ID from the
	assert( TheNameKeyGenerator );
	if( TheNameKeyGenerator )
		instData->m_id = (Int)TheNameKeyGenerator->nameToKey( instData->m_decoratedNameString );

	return TRUE;

}

// parseStatus ================================================================
/** Parse the STATUS field */
//=============================================================================
static Bool parseStatus( const char *token, WinInstanceData *instData,
												 char *buffer, void *data )
{

  instData->m_status = 0;
  parseBitString( buffer, &instData->m_status, WindowStatusNames );

  return TRUE;

}

// parseStyle =================================================================
/** Parse the STYLE field */
//=============================================================================
static Bool parseStyle( const char *token, WinInstanceData *instData,
												char *buffer, void *data )
{

  instData->m_style = 0;
  parseBitString( buffer, &instData->m_style, WindowStyleNames );

  return TRUE;

}

// parseSystemCallback ========================================================
/** Parse the system method callback for a window */
//=============================================================================
static Bool parseSystemCallback( const char *token, WinInstanceData *instData,
																 char *buffer, void *data )
{
	char *c, *ptr;
//	const char *seps = " ,\n\r\t";
	const char *stringSeps = "\"";

	// scan to the first " mark
	ptr = buffer;
	while( *ptr != '"' )
		ptr++;
	ptr++;  // skip the first "
	c = strtok( ptr, stringSeps );  // name value

	// save a pointer of the function address
	DEBUG_ASSERTCRASH( TheNameKeyGenerator && TheFunctionLexicon, ("Invalid singletons") );
	theSystemString = c;
	NameKeyType key = TheNameKeyGenerator->nameToKey( theSystemString );
	systemFunc = TheFunctionLexicon->gameWinSystemFunc( key );

	return TRUE;

}

// parseInputCallback =========================================================
/** Parse the Input method callback for a window */
//=============================================================================
static Bool parseInputCallback( const char *token, WinInstanceData *instData,
																char *buffer, void *data )
{
	char *c, *ptr;
//	const char *seps = " ,\n\r\t";
	const char *stringSeps = "\"";

	// scan to the first " mark
	ptr = buffer;
	while( *ptr != '"' )
		ptr++;
	ptr++;  // skip the first "
	c = strtok( ptr, stringSeps );  // name value

	// save a pointer of the function address
	DEBUG_ASSERTCRASH( TheNameKeyGenerator && TheFunctionLexicon, ("Invalid singletons") );
	theInputString = c;
	NameKeyType key = TheNameKeyGenerator->nameToKey( theInputString );
	inputFunc = TheFunctionLexicon->gameWinInputFunc( key );

	return TRUE;

}

// parseTooltipCallback =======================================================
/** Parse the Tooltip method callback for a window */
//=============================================================================
static Bool parseTooltipCallback( const char *token, WinInstanceData *instData,
																  char *buffer, void *data )
{
	char *c, *ptr;
//	const char *seps = " ,\n\r\t";
	const char *stringSeps = "\"";

	// scan to the first " mark
	ptr = buffer;
	while( *ptr != '"' )
		ptr++;
	ptr++;  // skip the first "
	c = strtok( ptr, stringSeps );  // name value

	// save a pointer of the function address
	DEBUG_ASSERTCRASH( TheNameKeyGenerator && TheFunctionLexicon, ("Invalid singletons") );
	theTooltipString = c;
	NameKeyType key = TheNameKeyGenerator->nameToKey( theTooltipString );
	tooltipFunc = TheFunctionLexicon->gameWinTooltipFunc( key );

	return TRUE;

}

// parseDrawCallback ==========================================================
/** Parse the Draw method callback for a window */
//=============================================================================
static Bool parseDrawCallback( const char *token, WinInstanceData *instData,
															 char *buffer, void *data )
{
	char *c, *ptr;
//	const char *seps = " ,\n\r\t";
	const char *stringSeps = "\"";

	// scan to the first " mark
	ptr = buffer;
	while( *ptr != '"' )
		ptr++;
	ptr++;  // skip the first "
	c = strtok( ptr, stringSeps );  // name value

	// save a pointer of the function address
	DEBUG_ASSERTCRASH( TheNameKeyGenerator && TheFunctionLexicon, ("Invalid singletons") );
	theDrawString = c;
	NameKeyType key = TheNameKeyGenerator->nameToKey( theDrawString );
	drawFunc = TheFunctionLexicon->gameWinDrawFunc( key );

	return TRUE;

}

// parseHeaderTemplate ==========================================================
/** Parse the Draw method callback for a window */
//=============================================================================
static Bool parseHeaderTemplate( const char *token, WinInstanceData *instData,
															 char *buffer, void *data )
{
	char *c, *ptr;
//	const char *seps = " ,\n\r\t";
	const char *stringSeps = "\"";

	// scan to the first " mark
	ptr = buffer;
	while( *ptr != '"' )
		ptr++;
	ptr++;  // skip the first "
	c = strtok( ptr, stringSeps );  // name value

	// save a pointer of the function address
	DEBUG_ASSERTCRASH( TheNameKeyGenerator && TheFunctionLexicon, ("Invalid singletons") );

	instData->m_headerTemplateName = c;

	return TRUE;

}

// parseListboxData ===========================================================
/** Parse listbox data entry */
//=============================================================================
static Bool parseListboxData( const char *token, WinInstanceData *instData,
															char *buffer, void *data )
{
	ListboxData *listData = (ListboxData *)data;
	char *c;
	const char *seps = " :,\n\r\t";

	// "LENGTH"
	c = strtok( buffer, seps );  // label
	c = strtok( NULL, seps );
	scanShort( c, listData->listLength );

	// "AUTOSCROLL"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanBool( c, listData->autoScroll );

	// "SCROLLIFATEND" (optional)
	c = strtok( NULL, seps );  // label
	if ( !stricmp(c, "ScrollIfAtEnd") )
	{
		c = strtok( NULL, seps );  // value
		scanBool( c, listData->scrollIfAtEnd );
		c = strtok( NULL, seps );  // label
	}
	else
	{
		listData->scrollIfAtEnd = FALSE;
	}

	// "AUTOPURGE"
	c = strtok( NULL, seps );  // value
	scanBool( c, listData->autoPurge );

	// "SCROLLBAR"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanBool( c, listData->scrollBar );

	// "MULTISELECT"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanBool( c, listData->multiSelect );

	// "COLUMNS"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanShort( c, listData->columns );
	if(listData->columns > 1)
	{
		listData->columnWidthPercentage = NEW Int[listData->columns];
		for(Int i = 0; i < listData->columns; i++ )
		{
			// "COLUMNS"
			c = strtok( NULL, seps );  // label
			c = strtok( NULL, seps );  // value
			scanInt( c, listData->columnWidthPercentage[i] );
		}
	}
	else
		listData->columnWidthPercentage = NULL;
	listData->columnWidth = NULL;
	// "FORCESELECT"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanBool( c, listData->forceSelect );


	// "
	return TRUE;

}

// parseComboBoxData ===========================================================
/** Parse Combo Box data entry */
//=============================================================================
static Bool parseComboBoxData( const char *token, WinInstanceData *instData,
															char *buffer, void *data )
{
	ComboBoxData *comboData = (ComboBoxData *)data;
	char *c;
	const char *seps = " :,\n\r\t";

	c = strtok( buffer, seps );  // label
	c = strtok( NULL, seps );	// value
  scanBool( c, comboData->isEditable );

	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );	// value
  scanInt( c, comboData->maxChars );

	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );	// value
  scanInt( c, comboData->maxDisplay );

	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );	// value
  scanBool( c, comboData->asciiOnly );

	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );	// value
  scanBool( c, comboData->lettersAndNumbersOnly );


  return TRUE;
}

// parseSliderData ============================================================
/** Parse slider data entry */
//=============================================================================
static Bool parseSliderData( const char *token, WinInstanceData *instData,
														 char *buffer, void *data )
{
	SliderData *sliderData = (SliderData *)data;
	char *c;
	const char *seps = " :,\n\r\t";

	// "MINVALUE"
	c = strtok( buffer, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, sliderData->minVal );

	// "MAXVALUE"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, sliderData->maxVal );

	return TRUE;

}

// parseRadioButtonData =======================================================
/** Parse radio button data entry */
//=============================================================================
static Bool parseRadioButtonData( const char *token, WinInstanceData *instData,
																	char *buffer, void *data )
{
	RadioButtonData *radioData = (RadioButtonData *)data;
	char *c;
	const char *seps = " :,\n\r\t";

	// "GROUP"
	c = strtok( buffer, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, radioData->group );

	return TRUE;

}


// parseTooltipText ===========================================================
/** Parse the TOOLTIPTEXT field */
//=============================================================================
static Bool parseTooltipText( const char *token, WinInstanceData *instData,
											 char *buffer, void *data )
{
	char *ptr = buffer;
	char *c;
	const char *stringSeps = "\n\r\t\"";

	// scan to the first " mark
	while( *ptr != '"' )
		ptr++;
	ptr++;  // skip the "
	if(strlen( ptr ) == 1 )
		return TRUE;
	c = strtok( ptr, stringSeps );  // value
	if( strlen( c ) >= MAX_TEXT_LABEL )
	{

		DEBUG_LOG(( "TextTooltip label '%s' is too long, max is '%d'", c, MAX_TEXT_LABEL ));
		assert( 0 );
		return FALSE;

	}
	instData->m_tooltipString.set(c);
	instData->setTooltipText(TheGameText->fetch(c));


  return TRUE;

}

// parseTooltipDelay =======================================================
/** Parse the tooltip delay */
//=============================================================================
static Bool parseTooltipDelay( const char *token, WinInstanceData *instData,
																	char *buffer, void *data )
{
	//RadioButtonData *radioData = (RadioButtonData *)data;
	char *c;
	const char *seps = " :,\n\r\t";

	// "getvalue"
	c = strtok( buffer, seps );  // value
	scanInt( c, instData->m_tooltipDelay );

	return TRUE;

}


// parseText ==================================================================
/** Parse the TEXT field */
//=============================================================================
static Bool parseText( const char *token, WinInstanceData *instData,
											 char *buffer, void *data )
{
	char *ptr = buffer;
	char *c;
	const char *stringSeps = "\n\r\t\"";

	// scan to the first " mark
	while( *ptr != '"' )
		ptr++;
	ptr++;  // skip the "
	c = strtok( ptr, stringSeps );  // value
	if( strlen( c ) >= MAX_TEXT_LABEL )
	{

		DEBUG_LOG(( "Text label '%s' is too long, max is '%d'", c, MAX_TEXT_LABEL ));
		assert( 0 );
		return FALSE;

	}
	instData->m_textLabelString = c;


  return TRUE;

}

// parseTextColor =============================================================
/** Parse text color entries for enabled, disabled, and hilite with
	* drop shadow colors */
//=============================================================================
static Bool parseTextColor( const char *token, WinInstanceData *instData,
														char *buffer, void *data )
{
	char *c;
	const char *seps = " :,\n\r\t";
	UnsignedInt r, g, b, a;
	Int i, states = 3;
	TextDrawData *textData;
	Bool first = TRUE;


	for( i = 0; i < states; i++ )
	{

		if( i == 0 )
			textData = &instData->m_enabledText;
		else if( i == 1 )
			textData = &instData->m_disabledText;
		else if( i == 2 )
			textData = &instData->m_hiliteText;
		else
		{

			DEBUG_LOG(( "Undefined state for text color" ));
			assert( 0 );
			return FALSE;

		}

		// color
		if( first == TRUE )
			c = strtok( buffer, seps );  // label
		else
			c = strtok( NULL, seps );  // label
		first = FALSE;
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, r );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, g );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, b );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, a );
		textData->color = GameMakeColor( r, g, b, a );

		// border color
		c = strtok( NULL, seps );  // label
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, r );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, g );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, b );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, a );
		textData->borderColor = GameMakeColor( r, g, b, a );

	}

	return TRUE;

}

// parseStaticTextData ========================================================
/** Parse static text data entry */
//=============================================================================
static Bool parseStaticTextData( const char *token, WinInstanceData *instData,
																 char *buffer, void *data )
{
	TextData *textData = (TextData *)data;
	char *c;
	const char *seps = " :,\n\r\t";

	// "CENTERED"
	c = strtok( buffer, seps );  // label
	c = strtok( NULL, seps );  // value
	scanBool( c, textData->centered );

	// @todo: add these  to GUIEdit options and output
	// These are initialized here because any TextData constructor would never get called.
	// The behavior with these defaults is the same as it was before these members were added.
	textData->centeredVertically = TRUE;
	textData->leftMargin = 7;
	textData->topMargin = 7;

	return TRUE;

}

// parseTextEntryData =========================================================
/** Parse text entry data entry */
//=============================================================================
static Bool parseTextEntryData( const char *token, WinInstanceData *instData,
																char *buffer, void *data )
{
	EntryData *entryData = (EntryData *)data;
	char *c;
	const char *seps = " :,\n\r\t";

	// "MAXLEN"
	c = strtok( buffer, seps );  // label
	c = strtok( NULL, seps );  // value
	scanShort( c, entryData->maxTextLen );

	// "SECRETTEXT"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanBool( c, entryData->secretText );

	// "NUMERICALONLY"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanBool( c, entryData->numericalOnly );

	// "ALPHANUMERICALONLY"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanBool( c, entryData->alphaNumericalOnly );

	// "ASCIIONLY"
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanBool( c, entryData->aSCIIOnly );

	return TRUE;

}

// parseTabControlData =========================================================
/** Parse tab control data entry */
//=============================================================================
static Bool parseTabControlData( const char *token, WinInstanceData *instData,
																char *buffer, void *data )
{
	TabControlData *tabControlData = (TabControlData *)data;
	char *c;
	const char *seps = " :,\n\r\t";

	//TABORIENTATION
	c = strtok( buffer, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, tabControlData->tabOrientation );

	//TABEDGE
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, tabControlData->tabEdge );

	//TABWIDTH
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, tabControlData->tabWidth );

	//TABHEIGHT
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, tabControlData->tabHeight );

	//TABCOUNT
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, tabControlData->tabCount );

	//PANEBORDER
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, tabControlData->paneBorder );

	//PANEDISABLED
	Int entryCount = 0;
	c = strtok( NULL, seps );  // label
	c = strtok( NULL, seps );  // value
	scanInt( c, entryCount );

	for( Int paneIndex = 0; paneIndex < entryCount; paneIndex ++ )
	{
		c = strtok( NULL, seps );  // value
		scanBool( c, tabControlData->subPaneDisabled[paneIndex] );
	}

	return TRUE;
}

// parseDrawData ==============================================================
/** Parse set of draw data elements */
//=============================================================================
static Bool parseDrawData( const char *token, WinInstanceData *instData,
													 char *buffer, void *data )
{
	Int i;
	UnsignedInt r, g, b, a;
	WinDrawData *drawData;
	Bool first = TRUE;
	char *c;
	const char *seps = " :,\n\r\t";

	for( i = 0; i < MAX_DRAW_DATA; i++ )
	{

		// get the right draw data
		if( strcmp( token, "ENABLEDDRAWDATA" ) == 0 )
			drawData = &instData->m_enabledDrawData[ i ];
		else if( strcmp( token, "DISABLEDDRAWDATA" ) == 0 )
			drawData = &instData->m_disabledDrawData[ i ];
		else if( strcmp( token, "HILITEDRAWDATA" ) == 0 )
			drawData = &instData->m_hiliteDrawData[ i ];
		else if( strcmp( token, "LISTBOXENABLEDUPBUTTONDRAWDATA" ) == 0 )
			drawData = &enabledUpButtonDrawData[ i ];
		else if( strcmp( token, "LISTBOXDISABLEDUPBUTTONDRAWDATA" ) == 0 )
			drawData = &disabledUpButtonDrawData[ i ];
		else if( strcmp( token, "LISTBOXHILITEUPBUTTONDRAWDATA" ) == 0 )
			drawData = &hiliteUpButtonDrawData[ i ];
		else if( strcmp( token, "LISTBOXENABLEDDOWNBUTTONDRAWDATA" ) == 0 )
			drawData = &enabledDownButtonDrawData[ i ];
		else if( strcmp( token, "LISTBOXDISABLEDDOWNBUTTONDRAWDATA" ) == 0 )
			drawData = &disabledDownButtonDrawData[ i ];
		else if( strcmp( token, "LISTBOXHILITEDOWNBUTTONDRAWDATA" ) == 0 )
			drawData = &hiliteDownButtonDrawData[ i ];
		else if( strcmp( token, "LISTBOXENABLEDSLIDERDRAWDATA" ) == 0 )
			drawData = &enabledSliderDrawData[ i ];
		else if( strcmp( token, "LISTBOXDISABLEDSLIDERDRAWDATA" ) == 0 )
			drawData = &disabledSliderDrawData[ i ];
		else if( strcmp( token, "LISTBOXHILITESLIDERDRAWDATA" ) == 0 )
			drawData = &hiliteSliderDrawData[ i ];
		else if( strcmp( token, "SLIDERTHUMBENABLEDDRAWDATA" ) == 0 )
			drawData = &enabledSliderThumbDrawData[ i ];
		else if( strcmp( token, "SLIDERTHUMBDISABLEDDRAWDATA" ) == 0 )
			drawData = &disabledSliderThumbDrawData[ i ];
		else if( strcmp( token, "SLIDERTHUMBHILITEDRAWDATA" ) == 0 )
			drawData = &hiliteSliderThumbDrawData[ i ];
		else if( strcmp( token, "COMBOBOXDROPDOWNBUTTONENABLEDDRAWDATA" ) == 0 )
			drawData = &enabledDropDownButtonDrawData[ i ];
		else if( strcmp( token, "COMBOBOXDROPDOWNBUTTONDISABLEDDRAWDATA" ) == 0 )
			drawData = &disabledDropDownButtonDrawData[ i ];
		else if( strcmp( token, "COMBOBOXDROPDOWNBUTTONHILITEDRAWDATA" ) == 0 )
			drawData = &hiliteDropDownButtonDrawData[ i ];
		else if( strcmp( token, "COMBOBOXEDITBOXENABLEDDRAWDATA" ) == 0 )
			drawData = &enabledEditBoxDrawData[ i ];
		else if( strcmp( token, "COMBOBOXEDITBOXDISABLEDDRAWDATA" ) == 0 )
			drawData = &disabledEditBoxDrawData[ i ];
		else if( strcmp( token, "COMBOBOXEDITBOXHILITEDRAWDATA" ) == 0 )
			drawData = &hiliteEditBoxDrawData[ i ];
		else if( strcmp( token, "COMBOBOXLISTBOXENABLEDDRAWDATA" ) == 0 )
			drawData = &enabledListBoxDrawData[ i ];
		else if( strcmp( token, "COMBOBOXLISTBOXDISABLEDDRAWDATA" ) == 0 )
			drawData = &disabledListBoxDrawData[ i ];
		else if( strcmp( token, "COMBOBOXLISTBOXHILITEDRAWDATA" ) == 0 )
			drawData = &hiliteListBoxDrawData[ i ];
		else
		{

			DEBUG_LOG(( "ParseDrawData, undefined token '%s'", token ));
			assert( 0 );
			return FALSE;

		}

		// IMAGE: X
		if( first == TRUE )
			c = strtok( buffer, seps );  // label
		else
			c = strtok( NULL, seps );  // label
		first = FALSE;

		c = strtok( NULL, seps );  // value
		if( strcmp( c, "NoImage" ) )
			drawData->image = TheMappedImageCollection->findImageByName( AsciiString( c ) );
		else
			drawData->image = NULL;
		// COLOR: R G B A
		c = strtok( NULL, seps );  // label
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, r );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, g );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, b );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, a );
		drawData->color = GameMakeColor( r, g, b, a );

		// BORDERCOLOR: R G B A
		c = strtok( NULL, seps );  // label
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, r );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, g );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, b );
		c = strtok( NULL, seps );  // value
		scanUnsignedInt( c, a );
		drawData->borderColor = GameMakeColor( r, g, b, a );

	}

	return TRUE;

}

// getDataTemplate ============================================================
/** Given a window type style string return the address of a static
	* gadget data type used for the generic data pointers in the
	* GUI gadget contorls */
//=============================================================================
void *getDataTemplate( char *type )
{
  static EntryData eData;
  static SliderData sData;
  static ListboxData lData;
  static TextData tData;
	static RadioButtonData rData;
	static TabControlData tcData;
	static ComboBoxData	cData;
	void *data;

  if( !strcmp( type, "VERTSLIDER" ) || !strcmp( type, "HORZSLIDER" ) )
	{

    memset( &sData, 0, sizeof( SliderData ) );
    data = &sData;

  }
	else if( !strcmp( type, "SCROLLLISTBOX" ) )
	{

    memset( &lData, 0, sizeof( ListboxData ) );
    data = &lData;

  }
	else if( !strcmp( type, "TABCONTROL" ) )
	{
    memset( &tcData, 0, sizeof( TabControlData ) );
    data = &tcData;
  }
	else if( !strcmp( type, "ENTRYFIELD" ) )
	{

    memset( &eData, 0, sizeof( EntryData ) );
    data = &eData;

  }
	else if( !strcmp( type, "STATICTEXT" ) )
	{

		memset( &tData, 0, sizeof( TextData ) );
		data = &tData;

  }
	else if( !strcmp( type, "RADIOBUTTON" ) )
	{

		memset( &rData, 0, sizeof( RadioButtonData ) );
		data = &rData;
	}
	else if( !strcmp( type, "COMBOBOX" ) )
	{

		memset( &cData, 0, sizeof( ComboBoxData ) );
		data = &cData;
	}
	else
    data = NULL;

  return data;

}

// parseData ==================================================================
//
// Parse the data for gadgets.  For sliders the data is in the format:
// <Minimum Value> <Maximum Value>
//
// For listboxes the data is formatted as:
// <Max number of entries> <Height of an entry> <AutoScroll> <AutoPurge> <ScrollBar> <MultiSelect> <ForceSelect>
//
// For Combo Boxes the data is formatted as:
// <is Editable> <Max Characters> <Max Entries before scroll appaears>
//
// For entry fields the data is as follows:
// <Max text length> <entry box width> <SecretText> <1 = NumericalOnly, 2 = AlphaNumericOnly>
//
// For text the data is as follows:
// <Centered> <Outlined> <StringManagerLabel>
//
// 1/2/2003: THIS FUNCTION IS NEVER REACHED; IT IS OBSOLETE -MDC
//
//=============================================================================
static Bool parseData( void **data, char *type, char *buffer )
{
  char *c;
  static EntryData eData;
  static SliderData sData;
  static ListboxData lData;
  static TextData tData;
	static RadioButtonData rData;
	static ComboBoxData cData;

  if( !strcmp( type, "VERTSLIDER" ) || !strcmp( type, "HORZSLIDER" ) )
	{

    memset( &sData, 0, sizeof( SliderData ) );

	  c = strtok( buffer, " \t\n\r" );
    sData.minVal = atoi(c);

	  c = strtok( NULL, " \t\n\r" );
    sData.maxVal = atoi(c);

    *data = &sData;

  }
	else if( !strcmp( type, "SCROLLLISTBOX" ) )
	{

    memset( &lData, 0, sizeof( ListboxData ) );

	  c = strtok( buffer, " \t\n\r" );
    lData.listLength = atoi(c);

//	  c = strtok( NULL, " \t\n\r" );
//    lData.entryHeight = atoi(c);

	  c = strtok( NULL, " \t\n\r" );
    lData.autoScroll = atoi(c);

	  c = strtok( NULL, " \t\n\r" );
    lData.autoPurge = atoi(c);

	  c = strtok( NULL, " \t\n\r" );
    lData.scrollBar = atoi(c);

	  c = strtok( NULL, " \t\n\r" );
    lData.multiSelect = atoi(c);

		c = strtok( NULL, " \t\n\r" );
		lData.forceSelect = atoi(c);

    *data = &lData;

  }
	else if( !strcmp( type, "ENTRYFIELD" ) )
	{

    memset( &eData, 0, sizeof( EntryData ) );

	  c = strtok( buffer, " \t\n\r" );
    eData.maxTextLen = atoi(c);

	  c = strtok( NULL, " \t\n\r" );
//    if (c)
//      eData.entryWidth = atoi(c);
//    else
//      eData.entryWidth = -1;

		c = strtok( NULL, " \t\n\r" );
		if (c)
		{
			eData.secretText = atoi(c);

			if( eData.secretText != FALSE )
				eData.secretText = TRUE;
		}
		else
			eData.secretText = FALSE;

		c = strtok( NULL, " \t\n\r" );
		if (c)
		{
			eData.numericalOnly = ( atoi(c) == 1 );
			eData.alphaNumericalOnly = ( atoi(c) == 2 );
			eData.aSCIIOnly = ( atoi(c) == 3 );
		}
		else
		{
			eData.numericalOnly = FALSE;
			eData.alphaNumericalOnly = FALSE;
			eData.aSCIIOnly = FALSE;
		}
    *data = &eData;

  }
	else if( !strcmp( type, "STATICTEXT" ) )
	{

	  c = strtok( buffer, " \t\n\r" );
    tData.centered = atoi(c);

		if( tData.centered != FALSE )
			tData.centered = TRUE;

	  c = strtok( NULL, " \t\n\r" );

		/** @todo need to get a label from the translation manager, uncomment
		the following line and remove the WideChar assignment when
		we have it */
//		text = StringManagerFetch( c );
//		text = L"Need StrManager, Remove me!";
//		TheWindowManager->winStrcpy( tData.text, text );

		*data = &tData;

  }
	else if( !strcmp( type, "RADIOBUTTON" ) )
	{

		c = strtok( buffer, " \t\n\r" );
		rData.group = atoi(c);
/// @todo Colin: Why was this here???
//		if( tData.centered != FALSE )
//		{
//			tData.centered = TRUE;
//		}
		*data = &rData;
	}
	else
    *data = NULL;

  return TRUE;

}

// setWindowText ==============================================================
/** Set the default text for a window or gadget control */
//=============================================================================
static void setWindowText( GameWindow *window, AsciiString textLabel )
{

	// sanity
	if (textLabel.isEmpty())
		return;

	UnicodeString theText, entryText;
	//Translate the text
	theText = TheGameText->fetch( (char *)textLabel.str());
	// set the text in the window based on what it is
	if( BitIsSet( window->winGetStyle(), GWS_PUSH_BUTTON ) )
		GadgetButtonSetText( window, theText );
	else if( BitIsSet( window->winGetStyle(), GWS_RADIO_BUTTON ) )
		GadgetRadioSetText( window, theText );
	else if( BitIsSet( window->winGetStyle(), GWS_CHECK_BOX ) )
		GadgetCheckBoxSetText( window, theText );
	else if( BitIsSet( window->winGetStyle(), GWS_STATIC_TEXT ) )
		GadgetStaticTextSetText( window, theText );
	else if( BitIsSet( window->winGetStyle(), GWS_ENTRY_FIELD ) )
	{
		entryText.translate(textLabel);
		GadgetTextEntrySetText( window, entryText );
	}
	else
		window->winSetText( theText );

}

// createGadget ===============================================================
/** Create a gadget based on the 'type' parm */
//=============================================================================
static GameWindow *createGadget( char *type,
																 GameWindow *parent,
																 Int status,
																 Int x, Int y,
																 Int width, Int height,
																 WinInstanceData *instData,
																 void *data )
{
  GameWindow *window = NULL;

  instData->m_owner = parent;

  if( !strcmp( type, "PUSHBUTTON" ) )
	{

    instData->m_style |= GWS_PUSH_BUTTON;
    window = TheWindowManager->gogoGadgetPushButton( parent, status, x, y,
																										 width, height,
																										 instData,
																										 instData->m_font, FALSE );

  }
	else if( !strcmp( type, "RADIOBUTTON" ) )
	{
		RadioButtonData *rData = (RadioButtonData *)data;
		char filename[ MAX_WINDOW_NAME_LEN ];
		char *c;

		//
		// assign a screen identifier to the radio button based on the
		// filename the radio button was saved in
		//

		strcpy( filename, instData->m_decoratedNameString.str() );
		c = strchr( filename, ':' );
		if( c )
			*c = 0;  // terminate after filename (format is filename:gadgetname)
		assert( TheNameKeyGenerator );
		if( TheNameKeyGenerator )
			rData->screen = (Int)(TheNameKeyGenerator->nameToKey( AsciiString(filename) ));

    instData->m_style |= GWS_RADIO_BUTTON;
    window = TheWindowManager->gogoGadgetRadioButton( parent, status, x, y,
																											width, height,
																											instData, rData,
																											instData->m_font, FALSE );

  }
	else if( !strcmp( type, "CHECKBOX" ) )
	{

    instData->m_style |= GWS_CHECK_BOX;
    window = TheWindowManager->gogoGadgetCheckbox( parent, status, x, y,
																									 width, height,
																									 instData,
																									 instData->m_font, FALSE );

  }
	else if( !strcmp( type, "TABCONTROL" ) )
	{
		TabControlData *tcData = (TabControlData *)data;
    instData->m_style |= GWS_TAB_CONTROL;
    window = TheWindowManager->gogoGadgetTabControl( parent, status, x, y,
																										width, height,
																										instData, tcData,
																										instData->m_font, FALSE );
	}
	else if( !strcmp( type, "VERTSLIDER" ) )
	{
    SliderData *sData = (SliderData *)data;

    instData->m_style |= GWS_VERT_SLIDER;
    window = TheWindowManager->gogoGadgetSlider( parent, status, x, y,
																								 width, height,
																								 instData, sData,
																								 instData->m_font, FALSE );

		//
		// we know we've read in the slider thumb data in the definition file
		// (it's guaranteed to be generated by the editor) so place that
		// draw data into the thumb of the slider
		//
		GameWindow *thumb = window->winGetChild();
		if( thumb )
		{
			WinInstanceData *instData = thumb->winGetInstanceData();

			for( Int i = 0; i < MAX_DRAW_DATA; i++ )
			{

				instData->m_enabledDrawData[ i ] = enabledSliderThumbDrawData[ i ];
				instData->m_disabledDrawData[ i ] = disabledSliderThumbDrawData[ i ];
				instData->m_hiliteDrawData[ i ] = hiliteSliderThumbDrawData[ i ];

			}

		}

  }
	else if( !strcmp( type, "HORZSLIDER" ) )
	{
    SliderData *sData = (SliderData *)data;

    instData->m_style |= GWS_HORZ_SLIDER;
    window = TheWindowManager->gogoGadgetSlider( parent, status, x, y,
																								 width, height,
																								 instData, sData,
																								 instData->m_font, FALSE );

		//
		// we know we've read in the slider thumb data in the definition file
		// (it's guaranteed to be generated by the editor) so place that
		// draw data into the thumb of the slider
		//
		GameWindow *thumb = window->winGetChild();
		if( thumb )
		{
			WinInstanceData *instData = thumb->winGetInstanceData();

			for( Int i = 0; i < MAX_DRAW_DATA; i++ )
			{

				instData->m_enabledDrawData[ i ] = enabledSliderThumbDrawData[ i ];
				instData->m_disabledDrawData[ i ] = disabledSliderThumbDrawData[ i ];
				instData->m_hiliteDrawData[ i ] = hiliteSliderThumbDrawData[ i ];

			}

		}

  }
	else if( !strcmp( type, "SCROLLLISTBOX" ) )
	{

    ListboxData *lData = (ListboxData *)data;

    instData->m_style |= GWS_SCROLL_LISTBOX;
    window = TheWindowManager->gogoGadgetListBox( parent, status, x, y,
																									width, height,
																									instData, lData,
																									instData->m_font, FALSE );

		//
		// we know that in the file we have read the draw data for the listbox
		// parts (up button, down button, and slider), now that we have those
		// parts actually created we must assign that data to them
		//
		GameWindow *upButton = GadgetListBoxGetUpButton( window );
		if( upButton )
		{
			WinInstanceData *instData = upButton->winGetInstanceData();

			for( Int i = 0; i < MAX_DRAW_DATA; i++ )
			{

				instData->m_enabledDrawData[ i ] = enabledUpButtonDrawData[ i ];
				instData->m_disabledDrawData[ i ] = disabledUpButtonDrawData[ i ];
				instData->m_hiliteDrawData[ i ] = hiliteUpButtonDrawData[ i ];

			}

		}

		GameWindow *downButton = GadgetListBoxGetDownButton( window );
		if( downButton )
		{
			WinInstanceData *instData = downButton->winGetInstanceData();

			for( Int i = 0; i < MAX_DRAW_DATA; i++ )
			{

				instData->m_enabledDrawData[ i ] = enabledDownButtonDrawData[ i ];
				instData->m_disabledDrawData[ i ] = disabledDownButtonDrawData[ i ];
				instData->m_hiliteDrawData[ i ] = hiliteDownButtonDrawData[ i ];

			}

		}

		GameWindow *slider = GadgetListBoxGetSlider( window );
		if( slider )
		{
			WinInstanceData *instData = slider->winGetInstanceData();

			for( Int i = 0; i < MAX_DRAW_DATA; i++ )
			{

				instData->m_enabledDrawData[ i ] = enabledSliderDrawData[ i ];
				instData->m_disabledDrawData[ i ] = disabledSliderDrawData[ i ];
				instData->m_hiliteDrawData[ i ] = hiliteSliderDrawData[ i ];

			}

			// do the slider thumb
			GameWindow *thumb = slider->winGetChild();
			if( thumb )
			{
				WinInstanceData *instData = thumb->winGetInstanceData();

				for( Int i = 0; i < MAX_DRAW_DATA; i++ )
				{

					instData->m_enabledDrawData[ i ] = enabledSliderThumbDrawData[ i ];
					instData->m_disabledDrawData[ i ] = disabledSliderThumbDrawData[ i ];
					instData->m_hiliteDrawData[ i ] = hiliteSliderThumbDrawData[ i ];

				}

			}

		}

  }
	else if( !strcmp( type, "COMBOBOX" ) )
	{
		ComboBoxData *cData = (ComboBoxData *)data;
		cData->entryData = NEW EntryData;
		memset ( cData->entryData, 0, sizeof(EntryData));
		cData->listboxData = NEW ListboxData;
		memset ( cData->listboxData, 0, sizeof(ListboxData));

		//initialize combo box data
		//cData->isEditable = TRUE;
		//cData->maxChars = 16;
		//cData->maxDisplay = 5;
		cData->entryCount = 0;
		//initialize entry data
		cData->entryData->aSCIIOnly = cData->asciiOnly;
		cData->entryData->alphaNumericalOnly = cData->lettersAndNumbersOnly;
		cData->entryData->maxTextLen = cData->maxChars;

		//initialize listbox data
		cData->listboxData->listLength = 10;
		cData->listboxData->autoScroll = 0;
		cData->listboxData->scrollIfAtEnd = FALSE;
		cData->listboxData->autoPurge = 0;
		cData->listboxData->scrollBar = 1;
		cData->listboxData->multiSelect = 0;
		cData->listboxData->forceSelect = 1;
		cData->listboxData->columns = 1;
		cData->listboxData->columnWidth = NULL;
		cData->listboxData->columnWidthPercentage = NULL;

    instData->m_style |= GWS_COMBO_BOX;
    window = TheWindowManager->gogoGadgetComboBox( parent, status, x, y,
																									width, height,
																									instData, cData,
																									instData->m_font, FALSE );

		GameWindow *dropDownButton = GadgetComboBoxGetDropDownButton( window );
		if( dropDownButton )
		{
			WinInstanceData *instData = dropDownButton->winGetInstanceData();

			for( Int i = 0; i < MAX_DRAW_DATA; i++ )
			{

				instData->m_enabledDrawData[ i ] = enabledDropDownButtonDrawData[ i ];
				instData->m_disabledDrawData[ i ] = disabledDropDownButtonDrawData[ i ];
				instData->m_hiliteDrawData[ i ] = hiliteDropDownButtonDrawData[ i ];

			}
		}

		GameWindow *editBox = GadgetComboBoxGetEditBox( window );
		if( editBox )
		{
			WinInstanceData *instData = editBox->winGetInstanceData();

			for( Int i = 0; i < MAX_DRAW_DATA; i++ )
			{

				instData->m_enabledDrawData[ i ] = enabledEditBoxDrawData[ i ];
				instData->m_disabledDrawData[ i ] = disabledEditBoxDrawData[ i ];
				instData->m_hiliteDrawData[ i ] = hiliteEditBoxDrawData[ i ];

			}
		}


		GameWindow *listBox = GadgetComboBoxGetListBox( window );
		if( listBox )
		{
			WinInstanceData *instData = listBox->winGetInstanceData();

			for( Int i = 0; i < MAX_DRAW_DATA; i++ )
			{

				instData->m_enabledDrawData[ i ] = enabledListBoxDrawData[ i ];
				instData->m_disabledDrawData[ i ] = disabledListBoxDrawData[ i ];
				instData->m_hiliteDrawData[ i ] = hiliteListBoxDrawData[ i ];

			}


			GameWindow *upButton = GadgetListBoxGetUpButton( listBox );
			if( upButton )
			{
				WinInstanceData *instData = upButton->winGetInstanceData();

				for( Int i = 0; i < MAX_DRAW_DATA; i++ )
				{

					instData->m_enabledDrawData[ i ] = enabledUpButtonDrawData[ i ];
					instData->m_disabledDrawData[ i ] = disabledUpButtonDrawData[ i ];
					instData->m_hiliteDrawData[ i ] = hiliteUpButtonDrawData[ i ];

				}

			}

			GameWindow *downButton = GadgetListBoxGetDownButton( listBox );
			if( downButton )
			{
				WinInstanceData *instData = downButton->winGetInstanceData();

				for( Int i = 0; i < MAX_DRAW_DATA; i++ )
				{

					instData->m_enabledDrawData[ i ] = enabledDownButtonDrawData[ i ];
					instData->m_disabledDrawData[ i ] = disabledDownButtonDrawData[ i ];
					instData->m_hiliteDrawData[ i ] = hiliteDownButtonDrawData[ i ];

				}

			}

			GameWindow *slider = GadgetListBoxGetSlider( listBox );
			if( slider )
			{
				WinInstanceData *instData = slider->winGetInstanceData();

				for( Int i = 0; i < MAX_DRAW_DATA; i++ )
				{

					instData->m_enabledDrawData[ i ] = enabledSliderDrawData[ i ];
					instData->m_disabledDrawData[ i ] = disabledSliderDrawData[ i ];
					instData->m_hiliteDrawData[ i ] = hiliteSliderDrawData[ i ];

				}

				// do the slider thumb
				GameWindow *thumb = slider->winGetChild();
				if( thumb )
				{
					WinInstanceData *instData = thumb->winGetInstanceData();

					for( Int i = 0; i < MAX_DRAW_DATA; i++ )
					{

						instData->m_enabledDrawData[ i ] = enabledSliderThumbDrawData[ i ];
						instData->m_disabledDrawData[ i ] = disabledSliderThumbDrawData[ i ];
						instData->m_hiliteDrawData[ i ] = hiliteSliderThumbDrawData[ i ];

					}

				}

			}
		}
  }
	else if( !strcmp( type, "ENTRYFIELD" ) )
	{
    EntryData *eData = (EntryData *)data;

    instData->m_style |= GWS_ENTRY_FIELD;
    window = TheWindowManager->gogoGadgetTextEntry( parent, status, x, y,
																										width, height,
																										instData, eData,
																										instData->m_font, FALSE );

  }
	else if( !strcmp( type, "STATICTEXT" ) )
	{
    TextData *tData = (TextData *)data;

    instData->m_style |= GWS_STATIC_TEXT;
    window = TheWindowManager->gogoGadgetStaticText( parent, status, x, y,
																										 width, height,
																										 instData, tData,
																										 instData->m_font, FALSE );


  }
	else if( !strcmp( type, "PROGRESSBAR" ) )
	{

    instData->m_style |= GWS_PROGRESS_BAR;
    window = TheWindowManager->gogoGadgetProgressBar( parent, status, x, y,
																											width, height,
																											instData,
																											instData->m_font, FALSE );

  }

  return window;

}

// createWindow ===============================================================
// Create a user window or a gadget depending on the 'type' parm
//=============================================================================
static GameWindow *createWindow( char *type,
																 Int id,
																 Int status,
																 Int x, Int y,
																 Int width, Int height,
																 WinInstanceData *instData,
																 void *data,
																 GameWinSystemFunc system,
																 GameWinInputFunc input,
																 GameWinTooltipFunc tooltip,
																 GameWinDrawFunc draw )
{
  GameWindow *window, *parent;

	// Check to see if this window has a parent
  parent = peekWindow();

  // If this is a regular window just create it
  if( !strcmp( type, "USER" ) )
	{

    window = TheWindowManager->winCreate( parent,
																					status, x, y,
																					width, height,
																					system );
		if( window )
		{

			instData->m_style |= GWS_USER_WINDOW;
			window->winSetInstanceData( instData );
			window->winSetWindowId( id );

		}

  }
	else if( !strcmp( type, "TABPANE" ) )
	{

    window = TheWindowManager->winCreate( parent,
																					status, x, y,
																					width, height,
																					system );
		if( window )
		{

			instData->m_style |= GWS_TAB_PANE;
			window->winSetInstanceData( instData );
			window->winSetWindowId( id );

		}

  }

	else
	{

    // Else parse the type and create the gadget
    window = createGadget( type,
													 parent,
													 status,
													 x, y,
													 width, height,
                           instData,
													 data );
		if( window )
		{

			// set id
			window->winSetWindowId( id );

		}

  }

	// assign the callbacks if they are not empty/NULL, that means they were read
	// in and parsed from the window definition file
	if( window )
	{

		if( system )
			window->winSetSystemFunc( system );
		if( input )
			window->winSetInputFunc( input );
		if( tooltip )
			window->winSetTooltipFunc( tooltip );
		if( draw )
			window->winSetDrawFunc( draw );

		// save strings for edit data if present
		GameWindowEditData *editData = window->winGetEditData();
		if( editData )
		{

			editData->systemCallbackString = theSystemString;
			editData->inputCallbackString = theInputString;
			editData->tooltipCallbackString = theTooltipString;
			editData->drawCallbackString = theDrawString;

		}

	}

	if( window )
	{

		// set any text read from the textLabel
		setWindowText( window, instData->m_textLabelString );

	}

  // If there is a parent window, send it the SCRIPT_CREATE message
  if( window && parent )
		TheWindowManager->winSendInputMsg( parent, GWM_SCRIPT_CREATE, id, 0 );

  return window;

}

// parseChildWindows ==========================================================
/** Parse window descriptions until an extra end is encountered indicating
	* the end of this block of child window descriptions. */
//=============================================================================
static Bool parseChildWindows( GameWindow *window,
															 File *inFile,
															 char *buffer )
{
  GameWindow *lastWindow;
	AsciiString asciibuf;

	//The gadget with children needs to delete its default created children in favor
	//of the ones from the script file.  So kill them before reading.
	if( BitIsSet( window->winGetStyle(), GWS_TAB_CONTROL ) )
	{
		GameWindow *nextWindow = NULL;
		for( GameWindow *myChild = window->winGetChild(); myChild; myChild = nextWindow )
		{
			nextWindow = myChild->winGetNext();
			TheWindowManager->winDestroy( myChild );
		}
	}

		// Push the current window onto the stack so we know it's the parent
  pushWindow( window );

	while( TRUE )
	{

		if (inFile->scanString(asciibuf) == FALSE) {
			break;
		}

		if (asciibuf.compare("ENDALLCHILDREN") == 0) {
			break;
		}

		if (asciibuf.compare("END") == 0) {
      break;
		}

		if (asciibuf.compare("ENABLEDCOLOR") == 0)
		{

			if( parseDefaultColor( &defEnabledColor, inFile, buffer ) == FALSE )
			{
				return FALSE;
			}

		}
		else if (asciibuf.compare("DISABLEDCOLOR") == 0)
		{

			if( parseDefaultColor( &defDisabledColor, inFile, buffer ) == FALSE )
			{
				return FALSE;
			}

		}
		else if (asciibuf.compare("HILITECOLOR") == 0)
		{

			if( parseDefaultColor( &defHiliteColor, inFile, buffer ) == FALSE )
			{
				return FALSE;
			}

		}
		else if (asciibuf.compare("SELECTEDCOLOR") == 0)
		{

			if( parseDefaultColor( &defSelectedColor, inFile, buffer ) == FALSE )
			{
				return FALSE;
			}

		}
		else if (asciibuf.compare("TEXTCOLOR") == 0)
		{

			if( parseDefaultColor( &defTextColor, inFile, buffer ) == FALSE )
			{
				return FALSE;
			}

		}
		else if (asciibuf.compare("WINDOW") == 0)
		{

      // Parse window descriptions until the last END is read
			if( parseWindow( inFile, buffer ) == NULL )
			{
				return FALSE;
			}

		}

	}

  // Pop the current window off the stack
  lastWindow = popWindow();

  if( lastWindow != window )
	{

    DEBUG_LOG(( "parseChildWindows: unmatched window on stack.  Corrupt stack or bad source" ));
    return FALSE;

  }

	if( BitIsSet( window->winGetStyle(), GWS_TAB_CONTROL ) )
		GadgetTabControlFixupSubPaneList( window );//all children created, so re-fill SubPane array with children

  return TRUE;

}

// lookup table for parsing functions
static GameWindowParse gameWindowFieldList[] =
{
	{ "NAME", parseName },
	{ "STATUS", parseStatus },
	{ "STYLE", parseStyle },
	{ "SYSTEMCALLBACK", parseSystemCallback },
	{ "INPUTCALLBACK", parseInputCallback },
	{ "TOOLTIPCALLBACK", parseTooltipCallback },
	{ "DRAWCALLBACK", parseDrawCallback },
	{ "FONT", parseFont },
	{ "HEADERTEMPLATE", parseHeaderTemplate },
	{ "LISTBOXDATA", parseListboxData },
	{ "COMBOBOXDATA", parseComboBoxData },
	{ "SLIDERDATA", parseSliderData },
	{ "RADIOBUTTONDATA", parseRadioButtonData },
	{	"TOOLTIPTEXT", parseTooltipText },
  { "TOOLTIPDELAY", parseTooltipDelay },
	{ "TEXT", parseText },
	{ "TEXTCOLOR", parseTextColor },
	{ "STATICTEXTDATA", parseStaticTextData },
	{ "TEXTENTRYDATA", parseTextEntryData },
	{ "TABCONTROLDATA", parseTabControlData },

	{ "ENABLEDDRAWDATA", parseDrawData },
	{ "DISABLEDDRAWDATA", parseDrawData },
	{ "HILITEDRAWDATA", parseDrawData },
	{ "LISTBOXENABLEDUPBUTTONDRAWDATA", parseDrawData },
	{ "LISTBOXENABLEDDOWNBUTTONDRAWDATA", parseDrawData },
	{ "LISTBOXENABLEDSLIDERDRAWDATA", parseDrawData },
	{ "LISTBOXDISABLEDUPBUTTONDRAWDATA", parseDrawData },
	{ "LISTBOXDISABLEDDOWNBUTTONDRAWDATA", parseDrawData },
	{ "LISTBOXDISABLEDSLIDERDRAWDATA", parseDrawData },
	{ "LISTBOXHILITEUPBUTTONDRAWDATA", parseDrawData },
	{ "LISTBOXHILITEDOWNBUTTONDRAWDATA", parseDrawData },
	{ "LISTBOXHILITESLIDERDRAWDATA", parseDrawData },
	{ "SLIDERTHUMBENABLEDDRAWDATA", parseDrawData },
	{ "SLIDERTHUMBDISABLEDDRAWDATA", parseDrawData },
	{ "SLIDERTHUMBHILITEDRAWDATA", parseDrawData },

	{ "COMBOBOXDROPDOWNBUTTONENABLEDDRAWDATA", parseDrawData },
	{ "COMBOBOXDROPDOWNBUTTONDISABLEDDRAWDATA", parseDrawData },
	{ "COMBOBOXDROPDOWNBUTTONHILITEDRAWDATA", parseDrawData },
	{ "COMBOBOXEDITBOXENABLEDDRAWDATA", parseDrawData },
	{ "COMBOBOXEDITBOXDISABLEDDRAWDATA", parseDrawData },
	{ "COMBOBOXEDITBOXHILITEDRAWDATA", parseDrawData },
	{ "COMBOBOXLISTBOXENABLEDDRAWDATA", parseDrawData },
	{ "COMBOBOXLISTBOXDISABLEDDRAWDATA", parseDrawData },
	{ "COMBOBOXLISTBOXHILITEDRAWDATA", parseDrawData },

	{ "IMAGEOFFSET", parseImageOffset },
	{ "TOOLTIP", parseTooltip },

	{ NULL, NULL }
};

// parseWindow ================================================================
/** Parse a WINDOW entry in the script. */
//=============================================================================
static GameWindow *parseWindow( File *inFile, char *buffer )
{
	GameWindowParse *parse;
	GameWindow *window = NULL;
	GameWindow *parent = peekWindow();
	WinInstanceData instData;
	char type[64];
	char token[ 256 ];
	char *c;
	Int x, y, width, height;
	void *data = NULL;
	ICoord2D parentSize;
	AsciiString asciibuf;

	//
	// reset our 'static globals' that house the current parsed window callback
	// definitions to empty
	//
	systemFunc = NULL;
	inputFunc = NULL;
	tooltipFunc = NULL;
	drawFunc = NULL;
	theSystemString.clear();
	theInputString.clear();
	theTooltipString.clear();
	theDrawString.clear();

	// get the size of the parent, or if no parent present the screen
	if( parent )
	{
		parent->winGetSize( &parentSize.x, &parentSize.y );
	}
	else
	{
		parentSize.x = TheDisplay->getWidth();
		parentSize.y = TheDisplay->getHeight();
	}

	// Initialize the instance data to the defaults
	/// @todo need to support enabled/disabled/hilite text colors here
	instData.init();
	instData.m_enabledText.color = defTextColor;
	instData.m_enabledText.borderColor = defTextColor;
	instData.m_disabledText.color = defTextColor;
	instData.m_disabledText.borderColor = defTextColor;
	instData.m_hiliteText.color = defTextColor;
	instData.m_hiliteText.borderColor = defTextColor;

	/// @todo need real font support here
	instData.m_font = defFont;

	//
	// read the first few lines that are required to be first in a
	// window definition file including position, size, type, and id
	//

	// window type
	readUntilSemicolon( inFile, buffer, WIN_BUFFER_LENGTH );
	c = strtok( buffer, seps );
	assert( strcmp( c, "WINDOWTYPE" ) == 0 );
	c = strtok( NULL, seps );  // get data to right of = sign
	strcpy( type, c );

	//
	// based on the window type get a pointer for any specific data
	// for the gadget controls needed
	//
	data = getDataTemplate( type );

	// position
	readUntilSemicolon( inFile, buffer, WIN_BUFFER_LENGTH );
	c = strtok( buffer, seps );
	assert( strcmp( c, "SCREENRECT" ) == 0 );
	if( parseScreenRect( c, buffer, &x, &y, &width, &height ) == FALSE )
		goto cleanupAndExit;

	// parse all the field definitions
	while( TRUE )
	{

		// get token
		inFile->scanString(asciibuf);

		// parse field
		for( parse = gameWindowFieldList; parse->parse; parse++ )
		{

			if (asciibuf.compare(parse->name) == 0)
			{

				strcpy( token, asciibuf.str() );

				// eat '='
				inFile->scanString(asciibuf);

				readUntilSemicolon( inFile, buffer, WIN_BUFFER_LENGTH );

				if (parse->parse( token, &instData, buffer, data ) == FALSE )
				{
					DEBUG_LOG(( "parseGameObject: Error parsing %s", parse->name ));
					goto cleanupAndExit;
				}

				break;
			}

		}

		if( parse->parse == NULL )
		{

			// If it's the END keyword
			if (asciibuf.compare("DATA") == 0)
			{

				// eat '='
				inFile->scanString(asciibuf);

				readUntilSemicolon( inFile, buffer, WIN_BUFFER_LENGTH );

				if( parseData( &data, type, buffer ) == FALSE )
				{
					DEBUG_LOG(( "parseGameWindow: Error parsing %s", parse->name ));
					goto cleanupAndExit;
				}

			}
			else if (asciibuf.compare("END") == 0)
			{
				// Check to see if we have a header template, if so, set the font equal to that.
				if(TheHeaderTemplateManager->getFontFromTemplate(instData.m_headerTemplateName))
					instData.m_font = TheHeaderTemplateManager->getFontFromTemplate(instData.m_headerTemplateName);

				// Create a window using the current description
				if( window == NULL )
					window = createWindow( type, instData.m_id, instData.getStatus(), x, y,
																 width, height, &instData, data,
																 systemFunc, inputFunc, tooltipFunc, drawFunc );


				goto cleanupAndExit;

			}
			else if (asciibuf.compare("CHILD") == 0)
			{

				// Create a window using the current description
				window = createWindow( type, instData.m_id, instData.getStatus(), x, y,
															 width, height, &instData, data,
															 systemFunc, inputFunc, tooltipFunc, drawFunc );

				if (window == NULL)
					goto cleanupAndExit;

				// Parses the CHILD's window info.
				if( parseChildWindows( window, inFile, buffer ) == FALSE )
				{

					TheWindowManager->winDestroy( window );
					window = NULL;
					goto cleanupAndExit;

				}

			}
			else
			{

				// Else it is unrecognized so eat associated data
				readUntilSemicolon( inFile, buffer, WIN_BUFFER_LENGTH );

			}

		}

	}

cleanupAndExit:

	//
	// this should be true since we should never set the text in
	// display strings in a instance data that is not inside a
	// window ... it's for sanity checking
	//
	// I am commenting this out to get tooltips working, If for
	// some reason we start having displayString problems... CLH
//	assert( instData.m_text == NULL && instData.m_tooltip == NULL );

	return window;

}

//=================================================================================================
//=================================================================================================
//=================================================================================================

//-------------------------------------------------------------------------------------------------
/** Parse init for layout file */
//-------------------------------------------------------------------------------------------------
Bool parseInit( const char *token, char *buffer, UnsignedInt version, WindowLayoutInfo *info )
{
	char *c;
	const char *seps = " \n\r\t";

	// get string
	c = strtok( buffer, seps );

	// translate string to function address
	info->initNameString = c;
	info->init = TheFunctionLexicon->winLayoutInitFunc( TheNameKeyGenerator->nameToKey( info->initNameString ) );

	return TRUE;  // success

}

//-------------------------------------------------------------------------------------------------
/** Parse update for layout file */
//-------------------------------------------------------------------------------------------------
Bool parseUpdate( const char *token, char *buffer, UnsignedInt version, WindowLayoutInfo *info )
{
	char *c;
	const char *seps = " \n\r\t";

	// get string
	c = strtok( buffer, seps );

	// translate string to function address
	info->updateNameString = c;
	info->update = TheFunctionLexicon->winLayoutUpdateFunc( TheNameKeyGenerator->nameToKey( info->updateNameString ) );

	return TRUE;  // success

}

//-------------------------------------------------------------------------------------------------
/** Parse shutdown for layout file */
//-------------------------------------------------------------------------------------------------
Bool parseShutdown( const char *token, char *buffer, UnsignedInt version, WindowLayoutInfo *info )
{
	char *c;
	const char *seps = " \n\r\t";

	// get string
	c = strtok( buffer, seps );

	// translate string to function address
	info->shutdownNameString = c;
	info->shutdown = TheFunctionLexicon->winLayoutShutdownFunc( TheNameKeyGenerator->nameToKey( info->shutdownNameString ) );

	return TRUE;  // success

}

static LayoutScriptParse layoutScriptTable[] =
{
	{ "LAYOUTINIT",										parseInit },
	{ "LAYOUTUPDATE",									parseUpdate },
	{ "LAYOUTSHUTDOWN",								parseShutdown },

	{ NULL,															NULL },

};

//-------------------------------------------------------------------------------------------------
/** Parse the layout block which MUST be present in every window file */
//-------------------------------------------------------------------------------------------------
Bool parseLayoutBlock( File *inFile, char *buffer, UnsignedInt version, WindowLayoutInfo *info )
{
	LayoutScriptParse *parse;
	char token[ 256 ];

	AsciiString asciitoken;
	if (inFile->scanString(asciitoken) == FALSE) {
		return FALSE;
	}

	// better be the layout block
	if (asciitoken.compare("STARTLAYOUTBLOCK") != 0) {
		return FALSE;
	}

	while( TRUE )
	{

		// get next token
		inFile->scanString(asciitoken);

		// check for end
		if (asciitoken.compare("ENDLAYOUTBLOCK") == 0) {
			break;
		}

		// search for token in the table
		for( parse = layoutScriptTable; parse && parse->name; parse++ )
		{

			if (asciitoken.compare(parse->name) == 0)
			{
				char *c;

				// read from file
				readUntilSemicolon( inFile, buffer, WIN_BUFFER_LENGTH );

				// eat equals separator " = "
				c = strtok( buffer, " =" );

				strcpy(token, asciitoken.str());

				// parse it
				if( parse->parse( token, c, version, info ) == FALSE )
					return FALSE;

				break;  // exit for

			}

		}

	}

	return TRUE;

}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GameWindowManager::winCreateLayout =========================================
/** Load window(s) from a .wnd definition file and wrap within a
	* new window layout */
//=============================================================================
WindowLayout *GameWindowManager::winCreateLayout( AsciiString filename )
{
	WindowLayout *layout;

	// allocate a new window layout
	layout = newInstance(WindowLayout);

	// load windows into layout
	if( layout->load( filename ) == FALSE )
	{

		deleteInstance(layout);
		return NULL;

	}

	// return loaded layout
	return layout;

}

/** Free up the memory used by static strings.  Normally this memory
is freed by the string destructor but we do it here to make the
memory leak detection code happy.*/
void GameWindowManager::freeStaticStrings(void)
{
	theSystemString.clear();
	theInputString.clear();
	theTooltipString.clear();
	theDrawString.clear();
}

WindowLayoutInfo::WindowLayoutInfo() :
	version(0),
	init(NULL),
	update(NULL),
	shutdown(NULL),
	initNameString(AsciiString::TheEmptyString),
	updateNameString(AsciiString::TheEmptyString),
	shutdownNameString(AsciiString::TheEmptyString)
{
		windows.clear();
}

// GameWindowManager::winCreateFromScript =====================================
/** Parse through a window .wnd file and create all the windows
	* within it.
	*
	* NOTE: The FIRST window created from the script is returned, this
	* way if you want to know ALL of the windows created from this
	* layout file you must iterate over info->windows, since the windows will
	* not be at the head of the window list if there is a modal window active.
	*/
//=============================================================================
GameWindow *GameWindowManager::winCreateFromScript( AsciiString filenameString,
																										WindowLayoutInfo *info )
{
	const char* filename = filenameString.str();
	static char buffer[ WIN_BUFFER_LENGTH ]; 		// input buffer for reading
	GameWindow *firstWindow = NULL;
  GameWindow *window;
  char filepath[ _MAX_PATH ] = "Window\\";
  File *inFile;
	WindowLayoutInfo scriptInfo;
	AsciiString asciibuf;

	// zero info struct
	//memset( &scriptInfo, 0, sizeof( WindowLayoutInfo ) ); // it's a class - use a constructor

  // Reset the window stack
  resetWindowStack();
	resetWindowDefaults();

	//
	// get the filename from the parameter, if it doesn't contain a '\' it is
	// a it is assumed to be a filename only, which we will prefix a "window\"
	// directory to, otherwise it is assumed to be an absolute path.  When using
	// a filename only make sure the current directory is set to the right
	// place for the window files subdirectory
	//
	if( strchr( filename, '\\' ) == NULL )
		sprintf( filepath, "Window\\%s", filename );
	else
		strcpy( filepath, filename );

  // Open the input file
	inFile = TheFileSystem->openFile(filepath, File::READ);
	if (inFile == NULL)
	{
		DEBUG_LOG(( "WinCreateFromScript: Cannot access file '%s'.", filename ));
		return NULL;
	}

  // read into memory
  inFile=inFile->convertToRAMFile();

	// read the file version
	Int version;
	inFile->read(NULL, strlen("FILE_VERSION = "));
	inFile->scanInt(version);
	inFile->nextLine();

	// version 2+ have a special block called the layout block
	if( version >= 2 )
	{

		if( parseLayoutBlock( inFile, buffer, version, &scriptInfo ) == FALSE )
		{

			DEBUG_LOG(( "WinCreateFromScript: Error parsing layout block" ));
			return FALSE;

		}

	}
	else
	{

		// default none names
		scriptInfo.initNameString = "[None]";
		scriptInfo.updateNameString = "[None]";
		scriptInfo.shutdownNameString = "[None]";

	}

	while( TRUE )
	{

		if (inFile->scanString(asciibuf) == FALSE) {
			break;
		}

		if (asciibuf.compare("END") == 0) {
      continue;
		}

		if (asciibuf.compare("ENABLEDCOLOR") == 0)
		{

			if( parseDefaultColor( &defEnabledColor, inFile, buffer ) == FALSE )
			{
				inFile->close();
				inFile = NULL;
				return NULL;
			}

		}
		else if (asciibuf.compare("DISABLEDCOLOR") == 0)
		{

			if( parseDefaultColor( &defDisabledColor, inFile, buffer ) == FALSE )
			{
				inFile->close();
				inFile = NULL;
				return NULL;
			}

		}
		else if (asciibuf.compare("HILITECOLOR") == 0)
		{

			if( parseDefaultColor( &defHiliteColor, inFile, buffer ) == FALSE )
			{
				inFile->close();
				inFile = NULL;
				return NULL;
			}

		}
		else if (asciibuf.compare("SELECTEDCOLOR") == 0)
		{

			if( parseDefaultColor( &defSelectedColor, inFile, buffer ) == FALSE )
			{
				inFile->close();
				inFile = NULL;
				return NULL;
			}

		}
		else if (asciibuf.compare("TEXTCOLOR") == 0)
		{

			if( parseDefaultColor( &defTextColor, inFile, buffer ) == FALSE )
			{
				inFile->close();
				inFile = NULL;
				return NULL;
			}

		}
		else if (asciibuf.compare("BACKGROUNDCOLOR") == 0)
		{

			if( parseDefaultColor( &defBackgroundColor, inFile, buffer ) == FALSE )
			{
				inFile->close();
				inFile = NULL;
				return NULL;
			}

		}
		else if (asciibuf.compare("FONT") == 0)
		{

			if( parseDefaultFont( defFont, inFile, buffer ) == FALSE )
			{
				inFile->close();
				inFile = NULL;
				return NULL;
			}

		}
		else if (asciibuf.compare("WINDOW") == 0)
		{

      // Parse window descriptions until the last END is read
      window = parseWindow( inFile, buffer );

			// save first window created
			if( firstWindow == NULL )
				firstWindow = window;

			scriptInfo.windows.push_back(window);

    }

	}

	// close the file
	inFile->close();
	inFile = NULL;

	// if info parameter is provided, copy info to the param
	if( info )
		*info = scriptInfo;

	// return the first window created
	return firstWindow;

}

