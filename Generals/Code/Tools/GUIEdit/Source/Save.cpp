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

// FILE: Save.cpp /////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    GUIEdit
//
// File name:  Save.cpp
//
// Created:    Colin Day, July 2001
//
// Desc:       Save the window layout to a file
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdio.h>
#include <assert.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEdit.h"
#include "Common/Debug.h"
#include "Common/NameKeyGenerator.h"
#include "Common/FunctionLexicon.h"
#include "GameClient/Display.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetRadioButton.h"

///////////////////////////////////////////////////////////////////////////////
// DEFINES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define BUFFER_SIZE		(2048)
#define INDENT_SIZE		(2)

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static char buffer[ BUFFER_SIZE ];  ///< buffer for writing data to before file output
static char offendingNames[ 65536 ];

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// fileNewLine ================================================================
/** Write a blank line to the file */
//=============================================================================
static void fileNewLine( FILE *fp )
{

	fprintf( fp, "\n" );
	fflush( fp );

}  // end fileNewLine

// writeBufferToFile ==========================================================
/** Write the contents of the buffer to the file */
//=============================================================================
static void writeBufferToFile( FILE *fp, char *buffer )
{

	// do the write
	fprintf( fp, buffer );

	//
	// flush the file stream so that if errors occur and we don't properly
	// recover it will be useful debugging information to see exactly how
	// far we got
	//
	fflush( fp );

}  // end writeBufferToFile

// clearBufferToSpaces ========================================================
/** Clear the buffer to all spaces */
//=============================================================================
static void clearBufferToSpaces( void )
{
	Int i;

	for( i = 0; i < BUFFER_SIZE; i++ )
		buffer[ i ] = ' ';

}  // end clearBufferToSpaces

// saveType ===================================================================
/** Save type of window */
//=============================================================================
static Bool saveType( GameWindow *window, FILE *fp, Int dataIndent )
{
	const char *type;

	if( BitIsSet( window->winGetStyle(), GWS_PUSH_BUTTON ) )
		type = "PUSHBUTTON";
	else if( BitIsSet( window->winGetStyle(), GWS_RADIO_BUTTON ) )
		type = "RADIOBUTTON";
	else if( BitIsSet( window->winGetStyle(), GWS_TAB_CONTROL ) )
		type = "TABCONTROL";
	else if( BitIsSet( window->winGetStyle(), GWS_TAB_PANE ) )
		type = "TABPANE";
	else if( BitIsSet( window->winGetStyle(), GWS_CHECK_BOX ) )
		type = "CHECKBOX";
	else if( BitIsSet( window->winGetStyle(), GWS_VERT_SLIDER ) )
		type = "VERTSLIDER";
	else if( BitIsSet( window->winGetStyle(), GWS_HORZ_SLIDER ) )
		type = "HORZSLIDER";
	else if( BitIsSet( window->winGetStyle(), GWS_SCROLL_LISTBOX ) )
		type = "SCROLLLISTBOX";
	else if( BitIsSet( window->winGetStyle(), GWS_COMBO_BOX ) )
		type = "COMBOBOX";
	else if( BitIsSet( window->winGetStyle(), GWS_ENTRY_FIELD ) )
		type = "ENTRYFIELD";
	else if( BitIsSet( window->winGetStyle(), GWS_STATIC_TEXT ) )
		type = "STATICTEXT";
	else if( BitIsSet( window->winGetStyle(), GWS_PROGRESS_BAR ) )
		type = "PROGRESSBAR";
	else
		type = "USER";

	sprintf( &buffer[ dataIndent ], "WINDOWTYPE = %s;\n", type );
	writeBufferToFile( fp, buffer );

	return TRUE;

}  // end saveType

// savePosition ===============================================================
/** Save window position data */
//=============================================================================
static Bool savePosition( GameWindow *window, FILE *fp, Int dataIndent )
{
	IRegion2D screenRect;
	ICoord2D size;

	// get the 4 screen position points of the window
	window->winGetScreenPosition( &screenRect.lo.x, &screenRect.lo.y );
	window->winGetSize( &size.x, &size.y );
	screenRect.hi.x = screenRect.lo.x + size.x;
	screenRect.hi.y = screenRect.lo.y + size.y;

	//
	// write those 4 screen points out with the ratio divisor that we
	// created them in (the edit window size) so we can scale them by
	// that ratio if need be
	//
	sprintf( &buffer[ dataIndent ], "SCREENRECT = UPPERLEFT: %d %d,\n", 
					 screenRect.lo.x, screenRect.lo.y );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "             BOTTOMRIGHT: %d %d,\n", 
					 screenRect.hi.x, screenRect.hi.y );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "             CREATIONRESOLUTION: %d %d;\n", 
					 TheDisplay->getWidth(), TheDisplay->getHeight() );
	writeBufferToFile( fp, buffer );

	return TRUE;

}  // end savePosition

// saveName ===================================================================
// Save name */
//=============================================================================
static Bool saveName( GameWindow *window, FILE *fp, Int dataIndent )
{
	WinInstanceData *instData = window->winGetInstanceData();

	sprintf( &buffer[ dataIndent ], "NAME = \"%s:%s\";\n", 
					 TheEditor->getSaveFilename(), instData->m_decoratedNameString.str() );
	writeBufferToFile( fp, buffer );

	return TRUE;

}  // end saveName

// saveStatus =================================================================
/** Save status information */
//=============================================================================
static Bool saveStatus( GameWindow *window, FILE *fp, Int dataIndent )
{
	Int i;
	Bool bitWritten;
	UnsignedInt bit;

	sprintf( &buffer[ dataIndent ], "STATUS = " );
	i = 0;
	bitWritten = FALSE;
	while( WindowStatusNames[ i ] )
	{

		bit = 1 << i;
		if( BitIsSet( window->winGetStatus(), bit ) )
		{
			
			// if this is an additional bit add a +
			if( bitWritten == TRUE )
				strcat( buffer, "+");

			// add status name
			strcat( buffer, WindowStatusNames[ i ] );
			bitWritten = TRUE;

		}  // end
		i++;

	}  // end while
	
	// if no bits written write NONE in the file
	if( bitWritten == FALSE )
		strcat( buffer, "NONE" );

	// complete line and write
	strcat( buffer, ";\n" );
	writeBufferToFile( fp, buffer );

	return TRUE;

}  // end saveStatus

// saveStyle ==================================================================
/** Save style information */
//=============================================================================
static Bool saveStyle( GameWindow *window, FILE *fp, Int dataIndent )
{
	Int i;
	Bool bitWritten;
	UnsignedInt bit;

	sprintf( &buffer[ dataIndent ], "STYLE = " );
	i = 0;
	bitWritten = FALSE;
	while( WindowStyleNames[ i ] )
	{

		bit = 1 << i;
		if( BitIsSet( window->winGetStyle(), bit ) )
		{
			
			// if this is an additional bit add a +
			if( bitWritten == TRUE )
				strcat( buffer, "+");

			// add status name
			strcat( buffer, WindowStyleNames[ i ] );
			bitWritten = TRUE;

		}  // end
		i++;
	
	}  // end while

	// if no bits written write NONE in the file
	if( bitWritten == FALSE )
		strcat( buffer, "NONE" );

	// complete line and write
	strcat( buffer, ";\n" );
	writeBufferToFile( fp, buffer );

	return TRUE;

}  // end saveStyle

// saveCallbacks ==============================================================
/** Save string representations of the window function callbacks into
	* the save file */
//=============================================================================
static Bool saveCallbacks( GameWindow *window, FILE *fp, Int dataIndent )
{
	GameWindowEditData *editData = window->winGetEditData();

	// if no edit data don't write anything for callbacks
	if( editData == NULL )
		return FALSE;

	// system
	AsciiString name;

	name = editData->systemCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	sprintf( &buffer[ dataIndent ], "SYSTEMCALLBACK = \"%s\";\n", name.str() );
	writeBufferToFile( fp, buffer );

	// input
	name = editData->inputCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	sprintf( &buffer[ dataIndent ], "INPUTCALLBACK = \"%s\";\n", name.str() );
	writeBufferToFile( fp, buffer );

	// tooltip
	name = editData->tooltipCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	sprintf( &buffer[ dataIndent ], "TOOLTIPCALLBACK = \"%s\";\n", name.str() );
	writeBufferToFile( fp, buffer );

	// draw
	name = editData->drawCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	sprintf( &buffer[ dataIndent ], "DRAWCALLBACK = \"%s\";\n", name.str() );
	writeBufferToFile( fp, buffer );

	return TRUE;

}  // end saveCallbacks

// saveHeaderTemplate =========================================================
/** Save HeaderTemplate for a window */
//=============================================================================
static Bool  saveHeaderTemplate(GameWindow *window, FILE *fp, Int dataIndent  )
{
	AsciiString headerName = window->winGetInstanceData()->m_headerTemplateName;

	if(headerName.isEmpty())
		headerName = GUIEDIT_NONE_STRING;
	sprintf( &buffer[ dataIndent ], "HEADERTEMPLATE = \"%s\";\n", headerName.str() );
	writeBufferToFile( fp, buffer );
	
	return TRUE;
} 

// saveFont ===================================================================
/** Save font name for a window */
//=============================================================================
static Bool saveFont( GameWindow *window, FILE *fp, Int dataIndent )
{
	GameFont *font = window->winGetFont();

	// if no font data don't write anything
	if( font == NULL )	
		return TRUE;

	// write the font data
	sprintf( &buffer[ dataIndent ], "FONT = NAME: \"%s\", SIZE: %d, BOLD: %d;\n", 
					 font->nameString.str(), font->pointSize, font->bold );
	writeBufferToFile( fp, buffer );
	
	return TRUE;

}  // end saveFont

// saveTooltipText ===================================================================
/** Save the text for a window */
//=============================================================================
static Bool saveTooltipText( GameWindow *window, FILE *fp, Int dataIndent )
{
	WinInstanceData *instData = window->winGetInstanceData();

	// save the text label
	if (instData->getTooltipTextLength())
	{
		sprintf( &buffer[ dataIndent ], "TOOLTIPTEXT = \"%s\";\n", instData->m_tooltipString.str() );
		writeBufferToFile( fp, buffer );

	}  // end if

	return TRUE;

}  // end saveTooltipText

// saveTooltipDelay ===================================================================
/** Save the delay for a window */
//=============================================================================
static Bool saveTooltipDelay( GameWindow *window, FILE *fp, Int dataIndent )
{
	WinInstanceData *instData = window->winGetInstanceData();

	// save the text label
	//if (instData->getTooltipTextLength())
	//{//
		sprintf( &buffer[ dataIndent ], "TOOLTIPDELAY = %d;\n", instData->m_tooltipDelay );
		writeBufferToFile( fp, buffer );

	//}  // end if

	return TRUE;

}  // end saveTooltipText

// saveText ===================================================================
/** Save the text for a window */
//=============================================================================
static Bool saveText( GameWindow *window, FILE *fp, Int dataIndent )
{
	WinInstanceData *instData = window->winGetInstanceData();

	// save the text label
	if (!instData->m_textLabelString.isEmpty())
	{

		sprintf( &buffer[ dataIndent ], "TEXT = \"%s\";\n", instData->m_textLabelString.str() );
		writeBufferToFile( fp, buffer );

	}  // end if

	return TRUE;

}  // end saveText

// saveTextColor ==============================================================
/** Save the text colors for enabled, disable, and hilite with
	* drop border colors */
//=============================================================================
static Bool saveTextColor( GameWindow *window, FILE *fp, Int dataIndent )
{
	Int i, max = 3;  // 3 states, enabled, hilite and disabled
	UnsignedByte r, g, b, a, br, bg, bb, ba;
	Color color, border;

	for( i = 0; i < max; i++ )
	{

		switch( i )
		{
			case 0: color = window->winGetEnabledTextColor();
							border = window->winGetEnabledTextBorderColor();
							break;
			case 1: color = window->winGetDisabledTextColor();
							border = window->winGetDisabledTextBorderColor();
							break;
			case 2: color = window->winGetHiliteTextColor();
							border = window->winGetHiliteTextBorderColor();
							break;
		}

		// get color components
		GameGetColorComponents( color, &r, &g, &b, &a );
		GameGetColorComponents( border, &br, &bg, &bb, &ba );

		if( i == 0 )
			sprintf( &buffer[ dataIndent ], "TEXTCOLOR = ENABLED:  %d %d %d %d, ENABLEDBORDER:  %d %d %d %d,\n",
							 r, g, b, a, br, bg, bb, ba );
		else if( i == max - 1 )
			sprintf( &buffer[ dataIndent ], "            HILITE:   %d %d %d %d, HILITEBORDER:   %d %d %d %d;\n",			
							 r, g, b, a, br, bg, bb, ba );
		else
			sprintf( &buffer[ dataIndent ], "            DISABLED: %d %d %d %d, DISABLEDBORDER: %d %d %d %d,\n",			
							 r, g, b, a, br, bg, bb, ba );
		writeBufferToFile( fp, buffer );

	}  // end for i

	return TRUE;

}  // end saveTextColor

// tokenIsEnabledData =========================================================
/** Token refers to enabled draw data */
//=============================================================================
static Bool tokenIsEnabledData( const char *token )
{


	if( strcmp( token, "ENABLEDDRAWDATA" ) == 0 ||
			strcmp( token, "LISTBOXENABLEDUPBUTTONDRAWDATA" ) == 0 ||
			strcmp( token, "LISTBOXENABLEDDOWNBUTTONDRAWDATA" ) == 0 ||
			strcmp( token, "LISTBOXENABLEDSLIDERDRAWDATA" ) == 0 ||
			strcmp( token, "COMBOBOXDROPDOWNBUTTONENABLEDDRAWDATA" ) == 0 ||
			strcmp( token, "COMBOBOXEDITBOXENABLEDDRAWDATA" ) == 0 ||
			strcmp( token, "COMBOBOXLISTBOXENABLEDDRAWDATA" ) == 0 ||
			strcmp( token, "SLIDERTHUMBENABLEDDRAWDATA" ) == 0 )
		return TRUE;

	return FALSE;
	
}  // end tokenIsEnabledData

// tokenIsDisabledData ========================================================
/** Token refers to Disabled draw data */
//=============================================================================
static Bool tokenIsDisabledData( const char *token )
{

	if( strcmp( token, "DISABLEDDRAWDATA" ) == 0 ||
			strcmp( token, "LISTBOXDISABLEDUPBUTTONDRAWDATA" ) == 0 ||
			strcmp( token, "LISTBOXDISABLEDDOWNBUTTONDRAWDATA" ) == 0 ||
			strcmp( token, "LISTBOXDISABLEDSLIDERDRAWDATA" ) == 0 ||
			strcmp( token, "COMBOBOXDROPDOWNBUTTONDISABLEDDRAWDATA" ) == 0 ||
			strcmp( token, "COMBOBOXEDITBOXDISABLEDDRAWDATA" ) == 0 ||
			strcmp( token, "COMBOBOXLISTBOXDISABLEDDRAWDATA" ) == 0 ||
			strcmp( token, "SLIDERTHUMBDISABLEDDRAWDATA" ) == 0 )
		return TRUE;

	return FALSE;
	
}  // end tokenIsDisabledData

// tokenIsHiliteData ==========================================================
/** Token refers to Hilite draw data */
//=============================================================================
static Bool tokenIsHiliteData( const char *token )
{

	if( strcmp( token, "HILITEDRAWDATA" ) == 0 ||
			strcmp( token, "LISTBOXHILITEUPBUTTONDRAWDATA" ) == 0 ||
			strcmp( token, "LISTBOXHILITEDOWNBUTTONDRAWDATA" ) == 0 ||
			strcmp( token, "LISTBOXHILITESLIDERDRAWDATA" ) == 0 ||
			strcmp( token, "COMBOBOXLISTBOXHILITEDRAWDATA" ) == 0 ||
			strcmp( token, "COMBOBOXEDITBOXHILITEDRAWDATA" ) == 0 ||
			strcmp( token, "COMBOBOXDROPDOWNBUTTONHILITEDRAWDATA" ) == 0 ||
			strcmp( token, "SLIDERTHUMBHILITEDRAWDATA" ) == 0 )
		return TRUE;

	return FALSE;
	
}  // end tokenIsHiliteData

// saveDrawData ===============================================================
/** Save the draw data array */
//=============================================================================
static Bool saveDrawData( const char *token, GameWindow *window,
													FILE *fp, Int dataIndent )
{
	Int i;
	WinInstanceData *instData = window->winGetInstanceData();
	char spaces[ 128 ];

	// format spaces so it looks pretty and lines up in the text file
	Int len = strlen( token );
	assert( len < sizeof( spaces ) );
	for( i = 0; i < len; i++ )
		spaces[ i ] = ' ';
	spaces[ i ] = 0;  // terminate

	for( i = 0; i < MAX_DRAW_DATA; i++ )
	{
		WinDrawData *drawData;
		UnsignedByte r, g, b, a, br, bg, bb, ba;
		const Image *image;

		// get the right draw data
		if( tokenIsEnabledData( token ) )
			drawData = &instData->m_enabledDrawData[ i ];
		else if( tokenIsDisabledData( token ) )
			drawData = &instData->m_disabledDrawData[ i ];
		else if( tokenIsHiliteData( token ) )
			drawData = &instData->m_hiliteDrawData[ i ];
		else
		{

			DEBUG_LOG(( "Save draw data, unknown token '%s'\n", token ));
			assert( 0 );
			return FALSE;

		}  // end else

		image = drawData->image;
		GameGetColorComponents( drawData->color, &r, &g, &b, &a );
		GameGetColorComponents( drawData->borderColor, &br, &bg, &bb, &ba );

		if( i == 0 )
			sprintf( &buffer[ dataIndent ], "%s = IMAGE: %s, COLOR: %d %d %d %d, BORDERCOLOR: %d %d %d %d,\n",
							 token, image ? image->getName().str() : "NoImage", r, g, b, a, br, bg, bb, ba );
		else if( i == MAX_DRAW_DATA - 1 )
			sprintf( &buffer[ dataIndent ], "%s   IMAGE: %s, COLOR: %d %d %d %d, BORDERCOLOR: %d %d %d %d;\n",
							 spaces, image ? image->getName().str() : "NoImage", r, g, b, a, br, bg, bb, ba );
		else
			sprintf( &buffer[ dataIndent ], "%s   IMAGE: %s, COLOR: %d %d %d %d, BORDERCOLOR: %d %d %d %d,\n",
							 spaces, image ? image->getName().str() : "NoImage", r, g, b, a, br, bg, bb, ba );

		writeBufferToFile( fp, buffer );

	}  // end for i

	return TRUE;

}  // end saveDrawData

// saveListboxData ============================================================
/** Save listbox data to the file */
//=============================================================================
static Bool saveListboxData( GameWindow *window, FILE *fp, Int dataIndent )
{
	ListboxData *listData = (ListboxData *)window->winGetUserData();

	// sanity
	if( listData == NULL )
	{

		DEBUG_LOG(( "No listbox data to save for window '%d'\n", 
						 window->winGetWindowId() ));
		assert( 0 );
		return FALSE;

	}  // end if

	sprintf( &buffer[ dataIndent ], "LISTBOXDATA = LENGTH: %d,\n", listData->listLength );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "              AUTOSCROLL: %d,\n", listData->autoScroll );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "              SCROLLIFATEND: %d,\n", listData->scrollIfAtEnd );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "              AUTOPURGE: %d,\n", listData->autoPurge );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "              SCROLLBAR: %d,\n", listData->scrollBar );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "              MULTISELECT: %d,\n", listData->multiSelect );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "              COLUMNS: %d,\n", listData->columns );
	writeBufferToFile( fp, buffer );
	if(listData->columns > 1)
	{
		for(Int i = 0; i < listData->columns; i++ )
		{
			sprintf( &buffer[ dataIndent ], "              COLUMNSWIDTH%%: %d,\n", listData->columnWidthPercentage[i] );	
			writeBufferToFile( fp, buffer );	
		}
	}
	sprintf( &buffer[ dataIndent ], "              FORCESELECT: %d;\n", listData->forceSelect );	
	writeBufferToFile( fp, buffer );	
	
	// save the up button draw data for the listbox
	if( listData->upButton )
	{

		saveDrawData( "LISTBOXENABLEDUPBUTTONDRAWDATA", listData->upButton, fp, dataIndent );
		saveDrawData( "LISTBOXDISABLEDUPBUTTONDRAWDATA", listData->upButton, fp, dataIndent );
		saveDrawData( "LISTBOXHILITEUPBUTTONDRAWDATA", listData->upButton, fp, dataIndent );

	}  // end if

	// save down button draw data for listbox
	if( listData->downButton )
	{

		saveDrawData( "LISTBOXENABLEDDOWNBUTTONDRAWDATA", listData->downButton, fp, dataIndent );
		saveDrawData( "LISTBOXDISABLEDDOWNBUTTONDRAWDATA", listData->downButton, fp, dataIndent );
		saveDrawData( "LISTBOXHILITEDOWNBUTTONDRAWDATA", listData->downButton, fp, dataIndent );

	}  // end if

	// save the slider draw data on the listbox
	if( listData->slider )
	{
		GameWindow *thumb = listData->slider->winGetChild();

		saveDrawData( "LISTBOXENABLEDSLIDERDRAWDATA", listData->slider, fp, dataIndent );
		saveDrawData( "LISTBOXDISABLEDSLIDERDRAWDATA", listData->slider, fp, dataIndent );
		saveDrawData( "LISTBOXHILITESLIDERDRAWDATA", listData->slider, fp, dataIndent );
		if( thumb )
		{

			saveDrawData( "SLIDERTHUMBENABLEDDRAWDATA", thumb, fp, dataIndent );
			saveDrawData( "SLIDERTHUMBDISABLEDDRAWDATA", thumb, fp, dataIndent );
			saveDrawData( "SLIDERTHUMBHILITEDRAWDATA", thumb, fp, dataIndent );

		}  // end if

	}  // end if

	return TRUE;

}  // end saveListboxData

// saveComboBoxData ============================================================
/** Save Combo Box data to the file */
//=============================================================================
static Bool saveComboBoxData( GameWindow *window, FILE *fp, Int dataIndent )
{
	ComboBoxData *comboData = (ComboBoxData *)window->winGetUserData();

	// sanity
	if( comboData == NULL )
	{

		DEBUG_LOG(( "No comboData data to save for window '%d'\n", 
						 window->winGetWindowId() ));
		assert( 0 );
		return FALSE;

	}  // end if
	
	sprintf( &buffer[ dataIndent ], "COMBOBOXDATA = ISEDITABLE: %d,\n", comboData->isEditable );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "              MAXCHARS: %d,\n", comboData->maxChars );	
	writeBufferToFile( fp, buffer );	
	sprintf( &buffer[ dataIndent ], "              MAXDISPLAY: %d,\n", comboData->maxDisplay );	
	writeBufferToFile( fp, buffer );	
	sprintf( &buffer[ dataIndent ], "              ASCIIONLY: %d,\n", comboData->asciiOnly );	
	writeBufferToFile( fp, buffer );	
	sprintf( &buffer[ dataIndent ], "              LETTERSANDNUMBERS: %d;\n", comboData->lettersAndNumbersOnly );	
	writeBufferToFile( fp, buffer );	
	
	//Save teh dropDownButton draw data for the combo box
	if( comboData->dropDownButton )
	{
									 
		saveDrawData( "COMBOBOXDROPDOWNBUTTONENABLEDDRAWDATA", comboData->dropDownButton, fp, dataIndent );
		saveDrawData( "COMBOBOXDROPDOWNBUTTONDISABLEDDRAWDATA", comboData->dropDownButton, fp, dataIndent );
		saveDrawData( "COMBOBOXDROPDOWNBUTTONHILITEDRAWDATA", comboData->dropDownButton, fp, dataIndent );

	}  // end if

	if( comboData->editBox )
	{

		saveDrawData( "COMBOBOXEDITBOXENABLEDDRAWDATA", comboData->editBox, fp, dataIndent );
		saveDrawData( "COMBOBOXEDITBOXDISABLEDDRAWDATA", comboData->editBox, fp, dataIndent );
		saveDrawData( "COMBOBOXEDITBOXHILITEDRAWDATA", comboData->editBox, fp, dataIndent );

	}  // end if

	if(comboData->listBox)
	{
			ListboxData *listData = (ListboxData *)comboData->listBox->winGetUserData();

			saveDrawData( "COMBOBOXLISTBOXENABLEDDRAWDATA", comboData->listBox, fp, dataIndent );
			saveDrawData( "COMBOBOXLISTBOXDISABLEDDRAWDATA", comboData->listBox, fp, dataIndent );
			saveDrawData( "COMBOBOXLISTBOXHILITEDRAWDATA", comboData->listBox, fp, dataIndent );
		
		// save the up button draw data for the listbox
		if( listData->upButton )
		{

			saveDrawData( "LISTBOXENABLEDUPBUTTONDRAWDATA", listData->upButton, fp, dataIndent );
			saveDrawData( "LISTBOXDISABLEDUPBUTTONDRAWDATA", listData->upButton, fp, dataIndent );
			saveDrawData( "LISTBOXHILITEUPBUTTONDRAWDATA", listData->upButton, fp, dataIndent );

		}  // end if

		// save down button draw data for listbox
		if( listData->downButton )
		{

			saveDrawData( "LISTBOXENABLEDDOWNBUTTONDRAWDATA", listData->downButton, fp, dataIndent );
			saveDrawData( "LISTBOXDISABLEDDOWNBUTTONDRAWDATA", listData->downButton, fp, dataIndent );
			saveDrawData( "LISTBOXHILITEDOWNBUTTONDRAWDATA", listData->downButton, fp, dataIndent );

		}  // end if

		// save the slider draw data on the listbox
		if( listData->slider )
		{
			GameWindow *thumb = listData->slider->winGetChild();

			saveDrawData( "LISTBOXENABLEDSLIDERDRAWDATA", listData->slider, fp, dataIndent );
			saveDrawData( "LISTBOXDISABLEDSLIDERDRAWDATA", listData->slider, fp, dataIndent );
			saveDrawData( "LISTBOXHILITESLIDERDRAWDATA", listData->slider, fp, dataIndent );
			if( thumb )
			{

				saveDrawData( "SLIDERTHUMBENABLEDDRAWDATA", thumb, fp, dataIndent );
				saveDrawData( "SLIDERTHUMBDISABLEDDRAWDATA", thumb, fp, dataIndent );
				saveDrawData( "SLIDERTHUMBHILITEDRAWDATA", thumb, fp, dataIndent );

			}  // end if

		}  // end if
	}// end if
	return TRUE;

}  // end saveComboBoxData


// saveRadioButtonData ========================================================
/** Save radio button specific data */
//=============================================================================
static Bool saveRadioButtonData( GameWindow *window, FILE *fp, Int dataIndent )
{
	RadioButtonData *radioData = (RadioButtonData *)window->winGetUserData();

	if( radioData == NULL )
	{


		DEBUG_LOG(( "No radio button data to save for window '%d'\n", 
						 window->winGetWindowId() ));
		assert( 0 );
		return FALSE;

	}  // end if

	sprintf( &buffer[ dataIndent ], "RADIOBUTTONDATA = GROUP: %d;\n", radioData->group );
	writeBufferToFile( fp, buffer );

	return TRUE;

}  // end saveRadioButtonData

// saveSliderData =============================================================
/** Save slider specific data */
//=============================================================================
static Bool saveSliderData( GameWindow *window, FILE *fp, Int dataIndent )
{
	SliderData *sliderData = (SliderData *)window->winGetUserData();

	// sanity
	if( sliderData == NULL )
	{

		DEBUG_LOG(( "No slider data in window to save for window %d\n", 
							window->winGetWindowId() ));
		assert( 0 );
		return FALSE;

	}  // end if

	sprintf( &buffer[ dataIndent ], "SLIDERDATA = MINVALUE: %d,\n", sliderData->minVal );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "             MAXVALUE: %d;\n", sliderData->maxVal );
	writeBufferToFile( fp, buffer );

	// save data about the slider thumb
	GameWindow *thumb = window->winGetChild();
	if( thumb )
	{

		saveDrawData( "SLIDERTHUMBENABLEDDRAWDATA", thumb, fp, dataIndent );
		saveDrawData( "SLIDERTHUMBDISABLEDDRAWDATA", thumb, fp, dataIndent );
		saveDrawData( "SLIDERTHUMBHILITEDRAWDATA", thumb, fp, dataIndent );

	}  // end if

	return TRUE;

}  // end saveSliderData

// saveStaticTextData =========================================================
/** Save static text data entry */
//=============================================================================
static Bool saveStaticTextData( GameWindow *window, FILE *fp, Int dataIndent )
{
	TextData *textData = (TextData *)window->winGetUserData();

	// sanity
	if( textData == NULL )
	{

		DEBUG_LOG(( "No text data in window to save for window %d\n", 
							window->winGetWindowId() ));
		assert( 0 );
		return FALSE;

	}  // end if

	sprintf( &buffer[ dataIndent ], "STATICTEXTDATA = CENTERED: %d;\n", textData->centered );
	writeBufferToFile( fp, buffer );

	return TRUE;

}  // end saveStaticTextData

// saveTextEntryData ==========================================================
/** Save static text data entry */
//=============================================================================
static Bool saveTextEntryData( GameWindow *window, FILE *fp, Int dataIndent )
{
	EntryData *entryData = (EntryData *)window->winGetUserData();

	// sanity
	if( entryData == NULL )
	{

		DEBUG_LOG(( "No text entry data in window to save for window %d\n", 
							window->winGetWindowId() ));
		assert( 0 );
		return FALSE;

	}  // end if

	sprintf( &buffer[ dataIndent ], "TEXTENTRYDATA = MAXLEN: %d,\n", entryData->maxTextLen );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "                SECRETTEXT: %d,\n", entryData->secretText );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "                NUMERICALONLY: %d,\n", entryData->numericalOnly );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "                ALPHANUMERICALONLY: %d,\n", entryData->alphaNumericalOnly );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "                ASCIIONLY: %d;\n", entryData->aSCIIOnly );
	writeBufferToFile( fp, buffer );

	return TRUE;

}  // end saveTextEntryData

// saveTabControlData ==========================================================
/** Save tab control entry */
//=============================================================================
static Bool saveTabControlData( GameWindow *window, FILE *fp, Int dataIndent )
{
	TabControlData *tabControlData = (TabControlData *)window->winGetUserData();

	// sanity
	if( tabControlData == NULL )
	{

		DEBUG_LOG(( "No text entry data in window to save for window %d\n", 
							window->winGetWindowId() ));
		assert( 0 );
		return FALSE;

	}  // end if

	sprintf( &buffer[ dataIndent ], "TABCONTROLDATA = TABORIENTATION: %d,\n", tabControlData->tabOrientation );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "                 TABEDGE: %d,\n", tabControlData->tabEdge );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "                 TABWIDTH: %d,\n", tabControlData->tabWidth );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "                 TABHEIGHT: %d,\n", tabControlData->tabHeight );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "                 TABCOUNT: %d,\n", tabControlData->tabCount );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "                 PANEBORDER: %d,\n", tabControlData->paneBorder );
	writeBufferToFile( fp, buffer );
	sprintf( &buffer[ dataIndent ], "                 PANEDISABLED: %d,", NUM_TAB_PANES );//number of entries to follow
	writeBufferToFile( fp, buffer );
	for( Int paneIndex = 0; paneIndex < NUM_TAB_PANES; paneIndex ++ )
	{
		if( paneIndex == NUM_TAB_PANES - 1 )
			sprintf( &buffer[ dataIndent ], "%d;", tabControlData->subPaneDisabled[paneIndex] );
		else
			sprintf( &buffer[ dataIndent ], "%d,", tabControlData->subPaneDisabled[paneIndex] );

		writeBufferToFile( fp, buffer );
	}
	sprintf( &buffer[ dataIndent ], "\n" );
	writeBufferToFile( fp, buffer );

	return TRUE;
}
// saveGadgetData =============================================================
/** Save data for specific gadgets */
//=============================================================================
static Bool saveGadgetData( GameWindow *window, FILE *fp, Int dataIndent )
{

	if( BitIsSet( window->winGetStyle(), GWS_SCROLL_LISTBOX ) )
		return saveListboxData( window, fp, dataIndent );
	else if( BitIsSet( window->winGetStyle(), GWS_COMBO_BOX ) )
		return saveComboBoxData( window, fp, dataIndent );
	else if( BitIsSet( window->winGetStyle(), GWS_RADIO_BUTTON ) )
		return saveRadioButtonData( window, fp, dataIndent );
	else if( BitIsSet( window->winGetStyle(), GWS_VERT_SLIDER | 
																					 GWS_HORZ_SLIDER ) )
		return saveSliderData( window, fp, dataIndent );
	else if( BitIsSet( window->winGetStyle(), GWS_STATIC_TEXT ) )
		return saveStaticTextData( window, fp, dataIndent );
	else if( BitIsSet( window->winGetStyle(), GWS_ENTRY_FIELD ) )
		return saveTextEntryData( window, fp, dataIndent );
	else if( BitIsSet( window->winGetStyle(), GWS_TAB_CONTROL ) )
		return saveTabControlData( window, fp, dataIndent );

	return TRUE;

}  // end saveGadgetData

// saveWindow =================================================================
/** Save a single window and any of its child windows.  Note that child
	* widows are saved in reverse order just as the main window list is
	* so that the same widow will be restored upon loading */
//=============================================================================
static Bool saveWindow( FILE *fp, GameWindow *window, Int indent )
{
	GameWindow *child;
	Bool success = TRUE;
	Int dataIndent = indent + INDENT_SIZE;
//	WinInstanceData *instData = window->winGetInstanceData();

	// clear the buffer with all spaces
	clearBufferToSpaces();

	// start window definition
	sprintf( &buffer[ indent ], "WINDOW\n" );
	writeBufferToFile( fp, buffer );
	clearBufferToSpaces();

	// save the this window data
	saveType( window, fp, dataIndent );
	savePosition( window, fp, dataIndent );
	saveName( window, fp, dataIndent );
	saveStatus( window, fp, dataIndent );
	saveStyle( window, fp, dataIndent );
	saveCallbacks( window, fp, dataIndent );
	saveFont( window, fp, dataIndent );
	saveHeaderTemplate( window, fp, dataIndent );
	saveTooltipText( window, fp, dataIndent );
  saveTooltipDelay( window, fp, dataIndent );
	saveText( window, fp, dataIndent );
	saveTextColor( window, fp, dataIndent );
	saveDrawData( "ENABLEDDRAWDATA", window, fp, dataIndent );
	saveDrawData( "DISABLEDDRAWDATA", window, fp, dataIndent );
	saveDrawData( "HILITEDRAWDATA", window, fp, dataIndent );

	if( BitIsSet( window->winGetStyle(), GWS_TAB_CONTROL ) )
	{//Seems cleaner to put this before the children list since this Gadget needs both
		saveGadgetData( window, fp, dataIndent );
	}

	//
	// save child data if present, we do not save child data of GUI controls 
	// themselves, only generic stuff, except for the Tab Control again, which
	// is the gadget with children exception
	//
	if( (TheEditor->windowIsGadget( window ) == FALSE)  ||  BitIsSet(window->winGetStyle(), GWS_TAB_CONTROL) )
	{

		child = window->winGetChild();
		if( child )
		{

			// traverse to end of child list
			while( child->winGetNext() != NULL )
				child = child->winGetNext();
					
			// save children windows in reverse order
			while( child )
			{

				// write child marker
				sprintf( &buffer[ dataIndent ], "CHILD\n" );
				writeBufferToFile( fp, buffer );

				// save child data
				success = saveWindow( fp, child, indent + INDENT_SIZE );
				if( success == FALSE )
					break;

				// previous child
				child = child->winGetPrev();

			}  // end while

			// all children saved
			sprintf( &buffer[ dataIndent ], "ENDALLCHILDREN\n" );
			writeBufferToFile( fp, buffer );

		}  // end if, children present

	}  // end if
	else
	{

		// save specific gadget data
		saveGadgetData( window, fp, dataIndent );

	}  // end else
			
	// end of window definition
	sprintf( &buffer[ indent ], "END\n" );
	writeBufferToFile( fp, buffer );

	return success;

}  // end saveWindow

// GUIEdit::validateNames =====================================================
/** Stored in the m_decorated field for each window is a name given
	* by the user.  When we write this name into the window file it
	* will be prefixed by the filename and a : such as (MainMenu.wnd:OKButton).
	* When we re-load these window definitions the whole decorated name
	* will be read and loaded into the decorated name field, therefore we
	* must make sure the entire name once decorated will fit into the field
	*/
//=============================================================================
void GUIEdit::validateNames( GameWindow *root, char *filename, Bool *valid )
{
	
	// the end of recursion
	if( root == NULL )
		return;

	// trivial case
	if( strlen( filename ) >= MAX_WINDOW_NAME_LEN )
	{

		sprintf( offendingNames, "Filename '%s' is too long.  The max is '%d' for filename AND window name!\n",
						 filename, MAX_WINDOW_NAME_LEN );
		*valid = FALSE;
		return;

	}  // end if

	// check this name for too long
	WinInstanceData *instData = root->winGetInstanceData();
	if( strlen( filename ) + instData->m_decoratedNameString.getLength() >= MAX_WINDOW_NAME_LEN )
	{

		strcat( offendingNames, "[Too Long] " );
		strcat( offendingNames, filename );
		strcat( offendingNames, ":" );
		strcat( offendingNames, instData->m_decoratedNameString.str() );
		strcat( offendingNames, "\n");
		*valid = FALSE;

	}  // end if

	// check for a duplicate filename
	if( TheEditor->isNameDuplicate( TheWindowManager->winGetWindowList(),
																	root, instData->m_decoratedNameString ) )
	{

		strcat( offendingNames, "[Duplicate] " );
		strcat( offendingNames, filename );
		strcat( offendingNames, ":" );
		strcat( offendingNames, instData->m_decoratedNameString.str() );
		strcat( offendingNames, "\n" );
		*valid = FALSE;

	}  // end if

	//You only call this on the first child since the call right after it will handle siblings (depth first)
	GameWindow *child = root->winGetChild();
	validateNames( child, filename, valid );

	// onto the next window
	validateNames( root->winGetNext(), filename, valid );

}  // end validateNames

// GUIEdit::updateRadioScreenIdentifiers ======================================
/** update all radio button screen identifiers with the new identifier */
//=============================================================================
void GUIEdit::updateRadioScreenIdentifiers( GameWindow *window, Int screenID )
{

	// end recursion
	if( window == NULL )
		return;

	// is this a radio button
	if( BitIsSet( window->winGetStyle(), GWS_RADIO_BUTTON ) )
	{
		RadioButtonData *radioData = (RadioButtonData *)window->winGetUserData();

		GadgetRadioSetGroup( window, radioData->group, screenID );

	}  // end if

	// check our children
	GameWindow *child;
	for( child = window->winGetChild(); child; child = child->winGetNext() )
		updateRadioScreenIdentifiers( child, screenID );

	// check the next one
	updateRadioScreenIdentifiers( window->winGetNext(), screenID );

}  // end updateRadioScreenIdentifiers

//-------------------------------------------------------------------------------------------------
/** Write layout block for a window file */
//-------------------------------------------------------------------------------------------------
static void writeLayoutBlock( FILE *fp )
{

	// start marker
	fprintf( fp, "STARTLAYOUTBLOCK\n" );

	// callback methods
	fprintf( fp, "  LAYOUTINIT = %s;\n", TheEditor->getLayoutInit().str() );
	fprintf( fp, "  LAYOUTUPDATE = %s;\n", TheEditor->getLayoutUpdate().str() );
	fprintf( fp, "  LAYOUTSHUTDOWN = %s;\n", TheEditor->getLayoutShutdown().str() );

	// end marker
	fprintf( fp, "ENDLAYOUTBLOCK\n" );

}  // end writeLayoutBlock

// GUIEdit::saveData ==========================================================
/** Save all our data to the file specified in filePath, which is a full
	* absolute path to a filename */
//=============================================================================
Bool GUIEdit::saveData( char *filePathAndFilename, char *filename )
{
	Int version = WindowLayoutCurrentVersion;
	FILE *fp;
	GameWindow *window;
	Bool success = TRUE;

	// write the editor config file
	writeConfigFile( GUIEDIT_CONFIG_FILENAME );

	// write our loaded fonts into the font file if we can
	writeFontFile( GUIEDIT_FONT_FILENAME );

	// sanity
	if( filePathAndFilename == NULL )
		return FALSE;

	// check for empty layout and just get out of here
	window = TheWindowManager->winGetWindowList();
	if( window == NULL )
		return TRUE;

	// check all the names for sizes once decorated with filename
	Bool valid = TRUE;
	sprintf( offendingNames, "\nOne or more window names are illegal.  A window name PLUS the filename must be under '%d' characters and names cannot be duplicates.\n\nList of illegal window names:\n\n",
					 MAX_WINDOW_NAME_LEN );
	validateNames( window, filename, &valid );
	if( valid == FALSE )
	{

		MessageBox( TheEditor->getWindowHandle(), offendingNames, "Window Name Error", MB_OK );	
		return FALSE;

	}  // end if

	// update all radio button screen identifiers with the filename
	updateRadioScreenIdentifiers( TheWindowManager->winGetWindowList(),
																TheNameKeyGenerator->nameToKey( AsciiString(m_saveFilename) ) );

	// open the file
	fp = fopen( filePathAndFilename, "w" );
	if( fp == NULL )
		return FALSE;

	// write out a single line for our window file version
	fprintf( fp, "FILE_VERSION = %d;\n", version );

	// write the layout block for the file
	writeLayoutBlock( fp );

	//
	// save each of the windows in reverse order, when we load a layout
	// file in this reverse order, the original window order we presently
	// see in the editor will be recreated because windows loaded after
	// other windows are placed on the top of the widnow stack
	//

	// go to end of window list
	window = TheWindowManager->winGetWindowList();
	while( window->winGetNext() != NULL )
		window = window->winGetNext();

	// loop backwards saving all windows
	while( window )
	{

		success = saveWindow( fp, window, 0 );
		if( success == FALSE )
			break;
		window = window->winGetPrev();

	}  // end while

	// close the file
	fclose( fp );

	return success;

}  // end saveData

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
