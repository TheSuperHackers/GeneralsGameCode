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

// FILE: GadgetListBox.cpp ////////////////////////////////////////////////////
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
// File name: ListBox.cpp
//
// Created:   Dean Iverson, March 1998
//						Colin Day, June 2001
//
// Desc:      ListBox GUI control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/AudioEventRTS.h"
#include "Common/Language.h"
#include "Common/Debug.h"
#include "Common/GameAudio.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetSlider.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/Keyboard.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// DEFINES ////////////////////////////////////////////////////////////////////
// Sets up the user's OS set doubleclick time so if they don't like it... they can
// change it in their OS.
static UnsignedInt doubleClickTime = GetDoubleClickTime();

// PRIVATE TYPES //////////////////////////////////////////////////////////////
typedef struct _AddMessageStruct
{
	Int row;			// The row to add the data to
	Int column;		// The column to add the data to
	const void *data;		// void pointer, can be either an DisplayString or an Image
	Int type;			// Can either be set to LISTBOX_TEXT or LISTBOX_IMAGE
	Bool overwrite;	// Do we overwrite existing data?
	Int width;			// set to -1 if we want the defaults
	Int height;			// set to -1 if we want the defaults
} AddMessageStruct;


typedef struct _TextAndColor
{
	UnicodeString string;			// Holds a unicode String
	Color color;							// holds a text's color
} TextAndColor;

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void doAudioFeedback(GameWindow *window)
{
	if (!window)
		return;

	ListboxData *lData = (ListboxData *)window->winGetUserData();
	if (!lData)
		return;

	if (lData->audioFeedback)
	{
		AudioEventRTS buttonClick("GUIComboBoxClick");

		if( TheAudio )
		{
			TheAudio->addAudioEvent( &buttonClick );
		}  // end if
	}
}

static Int getListboxEntryBasedOnCoord(GameWindow *window, Int x, Int y, Int &row, Int &column)
{
	Int pos;
	Int winx, winy, i;
	WinInstanceData *instData = window->winGetInstanceData();
	ListboxData *list = (ListboxData *)window->winGetUserData();

	window->winGetScreenPosition( &winx, &winy );

	// Adjust for title if present
	if( instData->getTextLength() )
		winy += TheWindowManager->winFontHeight( instData->getFont() ) + 1;

	pos = -2;

	for( i=0; ; i++ )
	{
		if( i > 0 )
			if( list->listData[ i - 1 ].listHeight > 
					(list->displayPos + list->displayHeight ) )
			{
				pos = -1;
				break;
			}

		if( i == list->endPos )						
		{
			pos = -1;
			break;
		}

		if( list->listData[i].listHeight > (y - winy + list->displayPos) )
			break;
	}

	column = -1;
	if( pos == -2 )
	{ 
		pos = i;
		Int total = 0;
		for( i = 0; i < list->columns ;i++)
		{
			total += list->columnWidth[i];
			if(x - winx < total)
			{
				column = i;
				break;
			}
		}
	}
	row = pos;
	return pos;
}

Int GadgetListBoxGetEntryBasedOnXY( GameWindow *listbox, Int x, Int y, Int &row, Int &column)
{
	return getListboxEntryBasedOnCoord( listbox, x, y, row, column  );

}

// getListboxTopEntry =========================================================
//=============================================================================
static Int getListboxTopEntry( ListboxData *list )
{
	Int entry;

	// determin which entry is at the top of the display area
	for( entry=0; ; entry++ )
	{
		if( list->listData[entry].listHeight > list->displayPos )
			return entry;

		if( entry >= list->endPos )
			return 0;
	}

	return 0;
}

// getListboxTopEntry =========================================================
//=============================================================================
static Int getListboxBottomEntry( ListboxData *list )
{
	Int entry;

	// determine which entry is at the top of the display area
	for( entry=list->endPos - 1; ; entry-- )
	{
		if( entry < 0 )
			return 0;
		if( list->listData[entry].listHeight == list->displayPos + list->displayHeight )
			return entry;
		if( list->listData[entry].listHeight < list->displayPos + list->displayHeight && entry != list->endPos - 1)
			return entry + 1;
		if( list->listData[entry].listHeight < list->displayPos + list->displayHeight)
			return entry;
	}

	return 0;
}


// removeSelection ============================================================
/** Remove Selection from a multiple selection list */
//=============================================================================
static void removeSelection( ListboxData *list, Int i )
{
	memcpy( &list->selections[i], &list->selections[(i+1)],
						((list->listLength - i) * sizeof(Int)) );

	// put -1 at end of list just for safety
	list->selections[(list->listLength - 1)] = -1;
}

// adjustDisplay ==============================================================
/** Update Display List information inlcuding scrollbar */
//=============================================================================
static void adjustDisplay( GameWindow *window, Int adjustment, 
													 Bool updateSlider )
{
	Int entry;
	SliderData *sData;
	ListboxData *list = (ListboxData *)window->winGetUserData();

	// determin which entry is at the top of the display area
	entry = getListboxTopEntry( list ) + adjustment;

	if( entry < 0 )
		entry = 0;
	else if( entry >= list->endPos )
		entry = list->endPos - 1;

	if( updateSlider )
	{
		if( entry > 0 )
			list->displayPos = list->listData[(entry - 1)].listHeight + 1;
		else
			list->displayPos = 0;
	}

	if( list->slider != NULL )
	{
		ICoord2D sliderSize, sliderChildSize;
		GameWindow *child;

		sData = (SliderData *)list->slider->winGetUserData();
		list->slider->winGetSize( &sliderSize.x, &sliderSize.y );
		// Take into account that there is a line-drawn outline surrounding listbox
		sData->maxVal = list->totalHeight - ( list->displayHeight - TOTAL_OUTLINE_HEIGHT ) + 1;
		
		if( sData->maxVal < 0 )
		{
			sData->maxVal = 0;
		}
		
		child = list->slider->winGetChild();
		child->winGetSize( &sliderChildSize.x, &sliderChildSize.y );
		sData->numTicks = (float)((sliderSize.y - sliderChildSize.y) / (float)sData->maxVal);

		if( updateSlider )
			TheWindowManager->winSendSystemMsg( list->slider, 
																					GSM_SET_SLIDER, 
																					(sData->maxVal - list->displayPos), 
																					0 );
	}

}  // end adjustDisplay

// computeTotalHeight =========================================================
/** Compute Total Height and fill in listHeight values */
//=============================================================================
static void computeTotalHeight( GameWindow *window )
{
	Int i, height = 0;
	Int tempHeight;
	ListboxData *list = (ListboxData *)window->winGetUserData();
	WinInstanceData *instData = window->winGetInstanceData();

	for( i=0; i<list->endPos; i++ )
	{
		
		if(!list->listData[i].cell)
			continue;
		tempHeight = 0;
		
		for (Int j = 0; j < list->columns; j++)
		{
			Int cellHeight = 0;
			if(list->listData[i].cell[j].cellType == LISTBOX_TEXT)
			{
				if( BitIsSet( window->winGetStatus(), WIN_STATUS_ONE_LINE ) == TRUE )
				{
					cellHeight = TheWindowManager->winFontHeight( instData->getFont() );
				}
				else
				{
					DisplayString *displayString = (DisplayString *)list->listData[i].cell[j].data;
					if(displayString)
						displayString->getSize( NULL, &cellHeight );		
				}//else
			}//if
			else if(list->listData[i].cell[j].cellType == LISTBOX_IMAGE)
			{
				if(list->listData[i].cell[j].height > 0)
					cellHeight = list->listData[i].cell[j].height + 1;
				else
					cellHeight = TheWindowManager->winFontHeight( instData->getFont() );
			}
			if(cellHeight > tempHeight)
				tempHeight = cellHeight;
		}//for
		list->listData[i].height = tempHeight;
		height += (list->listData[i].height + 1);
		list->listData[i].listHeight = height;
	}

	list->totalHeight = height;

	adjustDisplay( window, 0, TRUE );
}

// addImageEntry ==============================================================
/** Add Images to position and column. Row and Column are both based from starting
		Position 0 */
//=============================================================================
static Int addImageEntry( const Image *image, Color color, Int row, Int column, GameWindow *window, Bool overwrite, Int width, Int height )
{
//	WinInstanceData *instData = window->winGetInstanceData();
	ListboxData *list = (ListboxData *)window->winGetUserData();

	if( column >= list->columns  || row >= list->listLength )
	{
		DEBUG_ASSERTCRASH(false, ("Tried to add Image to Listbox at invalid position"));
		return -1;
	}
	
	// If we want to just add an entry to the bottom, set the defaults
	if (row == -1)
	{
		row = list->insertPos;
		list->insertPos++;
		list->endPos++;
	}
	if( column == -1 )
		column = 0;

	ListEntryRow *listRow = &list->listData[row];

	// Check and see if we have allocated cells for that row yet, if not, allocate them
	if(!listRow->cell)
	{
		listRow->cell = NEW ListEntryCell[list->columns];
		memset(listRow->cell,0,list->columns * sizeof(ListEntryCell));
	}
	// if we're copying over strings, then lets first deallocate them.
	if(listRow->cell[column].cellType == LISTBOX_TEXT)
	{
		TheDisplayStringManager->freeDisplayString((DisplayString *)listRow->cell[column].data);

	}
	//add Image to selected row/cell
	listRow->cell[column].cellType = LISTBOX_IMAGE;
	listRow->cell[column].data = (void *)image;
	listRow->cell[column].color = color;
	listRow->cell[column].height = height;
	listRow->cell[column].width = width;
	
	computeTotalHeight( window );

	return (row);	

}// static Int addImageEntry( Image image, Int column, GameWindow *window)

// startingRow will get moved to startingRow+1, etc.  This assumes there is space!!!!!
static Int moveRowsDown(ListboxData *list, Int startingRow)
{
	//
	// copy the cells down
	//
	Int copyLen = (list->endPos - startingRow) * sizeof(ListEntryRow);
	char *buf = NEW char[copyLen];
	memcpy(buf, list->listData + startingRow, copyLen);
	memcpy(list->listData + startingRow + 1, buf, copyLen );
	delete[] buf;

	list->endPos ++;
	list->insertPos = list->endPos;

	//
	// remove the display or links to images after the shift
	//
	list->listData[startingRow].cell = NULL;
	list->listData[startingRow].height = 0;
	list->listData[startingRow].listHeight = 0;

	if( list->multiSelect )
	{
		Int i = 0;

		while( list->selections[i] >= 0 )
		{
			if( startingRow <= list->selections[i] )
				list->selections[i]++;
			i++;
		}
	}
	else
	{
		if( list->selectPos >= startingRow )
			list->selectPos++;
	}

	/*
	if( list->displayPos > 0 )
		adjustDisplay( window, (-1 * mData1), TRUE );

	computeTotalHeight( window );
	*/

	return 1;
}

// addEntry ===================================================================
/** Add and process one string at insertPos */
//=============================================================================
static Int addEntry( UnicodeString *string, Int color, Int row, Int column, GameWindow *window, Bool overwrite )
{
//	WinInstanceData *instData = window->winGetInstanceData();
	ListboxData *list = (ListboxData *)window->winGetUserData();
	Int width;
	DisplayString *displayString;
	
	// make sure our params are good
	if( column >= list->columns  || row >= list->listLength )
	{
		DEBUG_ASSERTCRASH(false, ("Tried to add text to Listbox at invalid position"));
		return -1;
	}
	
	// If we want to just add an entry to the bottom, set the defaults
	if (row == -1)
	{
		row = list->insertPos;
		list->insertPos++;
		list->endPos++;
	}
	if( column == -1 )
		column = 0;

	width = list->columnWidth[column] - TEXT_WIDTH_OFFSET;

	Int rowsAdded = 0;

	ListEntryRow *listRow = &list->listData[row];
	// Here I've decided to just overright what's in the row, if that's not what we want, change it here
	// Check and see if we have allocated cells for that row yet, if not, allocate them
	if(!listRow->cell)
	{
		listRow->cell = NEW ListEntryCell[list->columns];
		memset(listRow->cell,0,list->columns * sizeof(ListEntryCell));
		rowsAdded = 1;
	}
	else if (!overwrite)
	{
		// Shove things down
		moveRowsDown(list, row);
		listRow->cell = NEW ListEntryCell[list->columns];
		memset(listRow->cell,0,list->columns * sizeof(ListEntryCell));
		rowsAdded = 1;
	}
	
	//add Image to selected row/cell
	listRow->cell[column].cellType = LISTBOX_TEXT;

	// assign the color to the list data element
	listRow->cell[column].color = color;

	// copy text
	if( !listRow->cell[column].data )
		listRow->cell[column].data = (void *) TheDisplayStringManager->newDisplayString();
	displayString = (DisplayString *) listRow->cell[column].data;
	if ( BitIsSet( window->winGetStatus(), WIN_STATUS_ONE_LINE ) == FALSE )
		displayString->setWordWrap( width );
	displayString->setText( *string );

	/** @todo we need for formalize this, but for now just set the font
	of this listbox entry to the font of the window */
	displayString->setFont( window->winGetFont() );

	if (overwrite)
	{
		Int oldRowHeight = listRow->height;
		Int oldTotalHeight = listRow->listHeight;
		Int rowHeight;
		Int totalHeight;

		if (!oldTotalHeight && row)
		{
			oldTotalHeight = list->listData[row-1].listHeight;
		}

		displayString->getSize( NULL, &rowHeight );
		if (rowHeight > oldRowHeight)
		{
			totalHeight = oldTotalHeight + (rowHeight - oldRowHeight);
			listRow->height = rowHeight;
			listRow->listHeight = totalHeight + rowsAdded;
			list->totalHeight += (rowHeight - oldRowHeight) + rowsAdded;
			adjustDisplay( window, 0, TRUE );
		}
	}
	else
	{
		computeTotalHeight( window );
	}

	return (row);
}  // end addEntry

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GadgetListBoxInput =========================================================
/** Handle input for list box */
//=============================================================================
WindowMsgHandledType GadgetListBoxInput( GameWindow *window, UnsignedInt msg,
												 WindowMsgData mData1, WindowMsgData mData2 )
{
	ListboxData *list = (ListboxData *)window->winGetUserData();
	WinInstanceData *instData = window->winGetInstanceData();

	switch (msg)
	{

		// ------------------------------------------------------------------------
		case GWM_CHAR:
		{

			switch (mData1)
			{
				
				// --------------------------------------------------------------------
				case KEY_ENTER:
				case KEY_SPACE:
				{

					if( BitIsSet( mData2, KEY_STATE_UP ) )
					{
						doAudioFeedback(window);

						TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																								GLM_DOUBLE_CLICKED,
																								(WindowMsgData)window, 
																								list->selectPos );
					}  // end if

					break;

				}  // end enter or space

				// --------------------------------------------------------------------
				case KEY_DOWN:
				{

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{

						if( list->selectPos == -1 )
						{
							list->selectPos = 0;
							adjustDisplay( window, 0, TRUE );
						}
						else if( list->selectPos < list->endPos - 1 )
						{

							list->selectPos++;

							while (1)
							{
								Int cellBottom = list->listData[list->selectPos].listHeight;
								Int cellTop = cellBottom - list->listData[list->selectPos].height;
								Int displayTop = list->displayPos;
								Int displayBottom = list->displayPos + list->displayHeight - 1; // account for the border

								if ( cellTop < displayTop )
								{
									adjustDisplay(window, -1, TRUE );
								}
								else if ( cellBottom < displayBottom )
								{
									adjustDisplay(window, 0, TRUE );
									break;
								}
								else
								{
									adjustDisplay(window, 1, TRUE );
								}
							}

						}

						TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																								GLM_SELECTED,
																								(WindowMsgData)window, 
																								list->selectPos );
					}  // end if

					break;

				}  // end key down

				// --------------------------------------------------------------------
				case KEY_UP:
				{

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{
	
						if( list->selectPos == -1 )
						{
							list->selectPos = 0;
							adjustDisplay( window, 0, TRUE );
						}
						else if( list->selectPos > 0 )
						{

							list->selectPos--;

							while (1)
							{
								if( list->listData[list->selectPos].listHeight - list->listData[list->selectPos].height < list->displayPos )
								{
									list->displayPos = list->listData[list->selectPos].listHeight +1;
									adjustDisplay( window, -1, TRUE );
								}
								else if( list->listData[list->selectPos].listHeight > list->displayPos  + list->displayHeight)
								{
									list->displayPos = list->listData[list->selectPos].listHeight - list->displayHeight;
									adjustDisplay(window, 1, TRUE);
								}
								else
								{
									adjustDisplay(window, 0, TRUE);
									break;
								}
							}
						}

						TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																								GLM_SELECTED,
																								(WindowMsgData)window, 
																								list->selectPos );
					}

					break;

				}  // end key up

				// --------------------------------------------------------------------
				case KEY_RIGHT:
				case KEY_TAB:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						TheWindowManager->winNextTab(window);
					break;

				// --------------------------------------------------------------------
				case KEY_LEFT:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						TheWindowManager->winPrevTab(window);
					break;

				// --------------------------------------------------------------------
				default:
					{
						Bool foundIt = false;
						if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						{
							// set the position to start looking for the line of text with this character
							Int position = list->selectPos;
							// only search the max number of times so that we're not looping over and over
							for(Int i = 0; i < list->endPos; ++i)
							{
								// start at the next position
								++position;
								// if we've reached the end of the list, start at the beginning
								if( position >= list->endPos)
									position = 0;
								
								ListEntryCell *cell = NULL;
								// go through the columns until we find a column with text
								Int j = 0;
								for(; j < list->columns; ++j)
								{
									cell = &list->listData[position].cell[j];
									if(cell && cell->cellType == LISTBOX_TEXT && cell->data)
									{
										break;
									}
								}
								if(!cell || cell->cellType != LISTBOX_TEXT)
									continue;
								DisplayString *dString = (DisplayString *)cell->data;
								if(!dString)
									continue;
								for(j = 0; j < TheKeyboard->MAX_KEY_STATES; ++j)
								{								
									if(dString->getText().getCharAt(0) == TheKeyboard->getPrintableKey(mData1, j))
									{
										list->selectPos = position;
										Int prevPos = getListboxTopEntry(list);
										//list->displayPos = list->listData[position].listHeight - list->displayHeight;
										adjustDisplay(window, position - prevPos, TRUE);
										foundIt = TRUE;
										break;
									}
								}
								if(foundIt)
								{
									doAudioFeedback(window);
									break;
								}

							}
							
							
						}

						if (!foundIt)
							return MSG_IGNORED;
					}

			}  // end switch( mData1 )

			break;

		}  // end case char

		// ------------------------------------------------------------------------
		case GWM_WHEEL_DOWN:
		{
			if( list->endPos <= 0)
				break;

			if (list->listData[list->endPos - 1].listHeight > list->displayHeight + list->displayPos)
				adjustDisplay( window, 1, TRUE );

			break;

		}  // end wheel down

		// ------------------------------------------------------------------------
		case GWM_WHEEL_UP:
		{
			if( list->endPos <= 0)
			 break;

			adjustDisplay( window, -1, TRUE );
			break;

		}  // end wheel up

		// ------------------------------------------------------------------------
		case GWM_LEFT_UP:
		{
			TheWindowManager->winSetFocus( window );
//			Int mousex = mData1 & 0xFFFF;
			Int mousey = mData1 >> 16;
			Int x, y, i;
			Int oldPos = list->selectPos;

			window->winGetScreenPosition( &x, &y );

			// Adjust for title if present
			if( instData->getTextLength() )
				y += TheWindowManager->winFontHeight( instData->getFont() ) + 1;

			list->selectPos = -2;

			for( i=0; ; i++ )
			{

				if( i > 0 )
					if( list->listData[ i - 1 ].listHeight > 
							(list->displayPos + list->displayHeight) )
					{
						list->selectPos = -1;
						break;
					}

				if( i == list->endPos )						
				{
					list->selectPos = -1;
					break;
				}

				if( list->listData[i].listHeight > (mousey - y + list->displayPos) )
					break;
			}
			
			//Bool dblClicked = FALSE;
			if( list->doubleClickTime + doubleClickTime > timeGetTime() && 
					(i == oldPos || (oldPos == -1 && ( i>=0 && i<list->endPos ) )) )
			{
				int temp;
				list->doubleClickTime = 0;
				if( oldPos == -1 )
					temp = i;
				else
					temp = oldPos;
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GLM_DOUBLE_CLICKED,
																					(WindowMsgData)window, 
																					temp );
				//break;
			}
			if( (i == oldPos) && (list->forceSelect == FALSE) )
			{
				list->selectPos = -1;
			}

			if( (list->selectPos == -2) && (i < list->endPos) )
			{
				list->selectPos = i;
			}

			if( (list->selectPos < 0) && (list->forceSelect) )
			{
				list->selectPos = oldPos;
			}
			list->doubleClickTime = timeGetTime();
			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GLM_SELECTED,
																					(WindowMsgData)window, 
																					list->selectPos );
			
			break;

		}  // end left click, left up

		// ------------------------------------------------------------------------
		case GWM_RIGHT_DOWN:
			doAudioFeedback(window);
			break;  // if we're in game, we want to eat this message because we're right clicking on a listbox
		case GWM_RIGHT_UP:
		{
			TheWindowManager->winSetFocus( window );
			Int pos;
			Int mousex = mData1 & 0xFFFF;
			Int mousey = mData1 >> 16;
			Int x, y, i;
			RightClickStruct rc;

			window->winGetScreenPosition( &x, &y );

			// Adjust for title if present
			if( instData->getTextLength() )
				y += TheWindowManager->winFontHeight( instData->getFont() ) + 1;

			pos = -2;

			for( i=0; ; i++ )
			{
				if( i > 0 )
					if( list->listData[ i - 1 ].listHeight > 
							(list->displayPos + list->displayHeight ) )
					{
						pos = -1;
						break;
					}

				if( i == list->endPos )						
				{
					pos = -1;
					break;
				}

				if( list->listData[i].listHeight > (mousey - y + list->displayPos) )
					break;
			}

			if( pos == -2 )
				pos = i;
					
			rc.pos = pos;
			rc.mouseX = mousex;
			rc.mouseY = mousey;
			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GLM_RIGHT_CLICKED,
																					(WindowMsgData)window, 
																					(WindowMsgData)&rc );
			break;

		}  // end right up, right click

		// ------------------------------------------------------------------------
		case GWM_MOUSE_ENTERING:
		{

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) ) 
			{

				BitSet( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GBM_MOUSE_ENTERING,
																						(WindowMsgData)window, 
																						0 );
				//TheWindowManager->winSetFocus( window );

			}  // end if

			break;

		}  //  end mouse entering

		// ------------------------------------------------------------------------
		case GWM_MOUSE_LEAVING:
		{

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK )) 
			{

				BitClear( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GBM_MOUSE_LEAVING,
																						(WindowMsgData)window, 
																						0 );
			}  // end if

			break;

		}  // end mouse leaving

		// ------------------------------------------------------------------------
		case GWM_LEFT_DRAG:

			if (BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) )
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GGM_LEFT_DRAG,
																						(WindowMsgData)window, 
																						0 );
			break;

		// ------------------------------------------------------------------------
		case GWM_LEFT_DOWN:
			doAudioFeedback(window);
			// we want to eat the down... so we may receive the up.
			return MSG_HANDLED;

		//-------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch msg

	return MSG_HANDLED;

}  // end GadgetListBoxInput

// GadgetListBoxMultiInput ====================================================
/** Handle input for multiple selection list box */
//=============================================================================
WindowMsgHandledType GadgetListBoxMultiInput( GameWindow *window, UnsignedInt msg,
															WindowMsgData mData1, WindowMsgData mData2 )
{
	ListboxData *list = (ListboxData *)window->winGetUserData();
	WinInstanceData *instData = window->winGetInstanceData();

	switch( msg ) 
	{
		// ------------------------------------------------------------------------
		case GWM_CHAR:
		{

			switch( mData1 )
			{

				// --------------------------------------------------------------------
				case KEY_TAB:
					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						window->winNextTab();
					break;

				// --------------------------------------------------------------------
				default:
					return MSG_IGNORED;
			}
			break;

		}  // end char

		// ------------------------------------------------------------------------
		case GWM_LEFT_UP:
		//case GWM_LEFT_CLICK:
		{
			TheWindowManager->winSetFocus( window );
//			Int *selections = list->selections;
			Int selectPos = -2;
//			Int mousex = mData1 & 0xFFFF;
			Int mousey = mData1 >> 16;
			Int x, y, i;
			Bool removed = FALSE;

			window->winGetScreenPosition( &x, &y );

			// Adjust for title if present
			if( instData->getTextLength() )
				y += TheWindowManager->winFontHeight( instData->getFont() ) + 1;

			for( i = 0; ; i++ )
			{

				if( i > 0 )
					if( list->listData[ i - 1 ].listHeight > 
							(list->displayPos + list->displayHeight ) )
					{
						selectPos = -1;
						break;
					}

				if( i == list->endPos )						
				{
					selectPos = -1;
					break;
				}

				if( list->listData[i].listHeight > (mousey - y + list->displayPos) )
					break;
			}

			if( selectPos == -2 )
				selectPos = i;

			i = 0;
			while( list->selections[i] >= 0 )
			{
				if( list->selections[i] == selectPos )
				{
					removeSelection( list, i );
					removed = TRUE;
					break;
				}
				
				i++;
			}

			if( removed == FALSE )
			{
				list->selections[ i] = selectPos;
				list->selections[ i + 1 ] = -1;
			}
		
			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GLM_SELECTED,
																					(WindowMsgData)window, 
																					selectPos );
			break;

		}  // end left up, left click

		// ------------------------------------------------------------------------
		case GWM_RIGHT_UP:
		{
/*
			Int selectPos = -2;
//			Int mousex = mData1 & 0xFFFF;
			Int mousey = mData1 >> 16;
			Int x, y, i;

			window->winGetScreenPosition( &x, &y );

			// Adjust for title if present
			if( instData->getTextLength() )
				y += TheWindowManager->winFontHeight( instData->getFont() ) + 1;

			for( i = 0; ; i++ )
			{
				if( i > 0 )
					if( list->listData[ i - 1 ].listHeight > 
							(list->displayPos + list->displayHeight ) )
					{
						selectPos = -1;
						break;
					}

				if( i == list->endPos )						
				{
					selectPos = -1;
					break;
				}

				if( list->listData[i].listHeight > (mousey - y + list->displayPos) )
					break;
			}

			if( selectPos == -2 )
				selectPos = i;

			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GLM_RIGHT_CLICKED,
																					(WindowMsgData)window, 
																					selectPos );*/
			TheWindowManager->winSetFocus( window );
			Int pos;
			Int mousex = mData1 & 0xFFFF;
			Int mousey = mData1 >> 16;
			Int x, y, i;
			RightClickStruct rc;

			window->winGetScreenPosition( &x, &y );

			// Adjust for title if present
			if( instData->getTextLength() )
				y += TheWindowManager->winFontHeight( instData->getFont() ) + 1;

			pos = -2;

			for( i=0; ; i++ )
			{
				if( i > 0 )
					if( list->listData[ i - 1 ].listHeight > 
							(list->displayPos + list->displayHeight ) )
					{
						pos = -1;
						break;
					}

				if( i == list->endPos )						
				{
					pos = -1;
					break;
				}

				if( list->listData[i].listHeight > (mousey - y + list->displayPos) )
					break;
			}

			if( pos == -2 )
				pos = i;
					
			rc.pos = pos;
			rc.mouseX = mousex;
			rc.mouseY = mousey;
			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GLM_RIGHT_CLICKED,
																					(WindowMsgData)window, 
																					(WindowMsgData)&rc );
			break;

		}  // end right up, right click

		// ------------------------------------------------------------------------
		case GWM_WHEEL_DOWN:

			// Simulate the down button if it exists
			if( list->downButton )
			{
				if( list->displayPos + list->displayHeight <= list->totalHeight )
					adjustDisplay( window, 1, TRUE );
			}
			break;

		// ------------------------------------------------------------------------
		case GWM_WHEEL_UP:

			// Simulate the up button if it exists
			if( list->upButton )
			{
				if( list->displayPos > 0 )
					adjustDisplay( window, -1, TRUE );
			}
			break;

		// ------------------------------------------------------------------------
		case GWM_MOUSE_ENTERING:
		{

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) ) 
			{

				BitSet( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GBM_MOUSE_ENTERING,
																						(WindowMsgData)window, 
																						0 );
				//TheWindowManager->winSetFocus( window );

			}  // end if

			break;

		}  //  end mouse entering

		// ------------------------------------------------------------------------
		case GWM_MOUSE_LEAVING:
		{

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK )) 
			{

				BitClear( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GBM_MOUSE_LEAVING,
																						(WindowMsgData)window, 
																						0 );
			}  // end if

			break;

		}  // end mouse leaving

		// ------------------------------------------------------------------------
		case GWM_LEFT_DRAG:

			if (BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) )
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GGM_LEFT_DRAG,
																						(WindowMsgData)window, 
																						0 );
			break;

		// ------------------------------------------------------------------------
		case GWM_LEFT_DOWN:
			doAudioFeedback(window);
			// we want to eat the down... so we may receive the up.
			return MSG_HANDLED;

		// ------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end GadgetListBoxMultiInput

// GadgetListBoxSystem ========================================================
/** Handle system messages for list box */
//=============================================================================
WindowMsgHandledType GadgetListBoxSystem( GameWindow *window, UnsignedInt msg,
													WindowMsgData mData1, WindowMsgData mData2 )
{
	ListboxData *list = (ListboxData *)window->winGetUserData();
	WinInstanceData *instData = window->winGetInstanceData();
	ICoord2D *pos;
		
	switch( msg )
	{
		// ------------------------------------------------------------------------
		case GGM_SET_LABEL:
		{
			instData->setText(*(UnicodeString*)mData1);
			break;

		}  // end set lavel

		// ------------------------------------------------------------------------
		case GLM_GET_TEXT:
		{
			pos = (ICoord2D *)mData1;
			TextAndColor *tAndC = (TextAndColor *)mData2;

			if(pos->x >= list->columns || pos->y >= list->listLength || 
					list->listData[pos->y].cell[pos->x].cellType != LISTBOX_TEXT)
			{
				tAndC->string = UnicodeString::TheEmptyString;
				tAndC->color = 0;				
			}
			else
			{
				tAndC->string = ((DisplayString *)list->listData[ pos->y ].cell[pos->x].data)->getText();
				tAndC->color = list->listData[ pos->y ].cell[pos->x].color;				
			}
		break;
		}
		
		// ------------------------------------------------------------------------
		case GBM_SELECTED:
		{

			// See if the up button was selected
			if( (GameWindow *)mData1 == list->upButton )
			{
				if( list->displayPos > 0 )
					adjustDisplay( window, -1, TRUE );
			}
			else if( (GameWindow *)mData1 == list->downButton )
			{
				if( list->displayPos + list->displayHeight <= list->totalHeight )
					adjustDisplay( window, 1, TRUE );
			}

			break;

		}  // end selected

		// ------------------------------------------------------------------------
		case GGM_LEFT_DRAG:
		{

			if( (GameWindow *)mData1 == list->upButton )
			{
				if( list->displayPos > 0 )
					adjustDisplay( window, -1, TRUE );
			}
			else if( (GameWindow *)mData1 == list->downButton )
			{
				if( list->displayPos + list->displayHeight <= list->totalHeight )
					adjustDisplay( window, 1, TRUE );
			}

			break;

		}  // end left drag

		// ------------------------------------------------------------------------
		case GLM_DEL_ALL:
		{

			//
			// Reset the listbox by freeing all the display string stuff and setting
			// everything else to zero
			//
			// Loop through and destroy any display strings we've allocated that aren't used
			for( Int i = 0; i < list->listLength; i++ )
			{
				// Loop though 
				ListEntryCell *cells = list->listData[i].cell;
				for (int j = list->columns - 1; j >=0; j-- )
				{			
					if(!cells)
						break;
					if( cells[j].cellType == LISTBOX_TEXT )
					{
						if ( cells[j].data )
						{
							TheDisplayStringManager->freeDisplayString((DisplayString *) cells[j].data );	
						}
					}
					
					cells[j].userData = NULL;
					cells[j].data = NULL;
				}
				delete[](list->listData[i].cell);
				list->listData[i].cell = NULL;
			}
			//zero out the header structure
			memset(list->listData,0,list->listLength * sizeof(ListEntryRow));

			if( mData1 != GP_DONT_UPDATE )
			{
				list->displayPos = 0;
			}
			
			if( list->multiSelect )
				memset( list->selections, -1, list->listLength * sizeof( Int ) );
			else
				list->selectPos = -1;

			list->insertPos = 0;
			list->endPos = 0;

			list->totalHeight = 0;

			adjustDisplay( window, 0, TRUE );
			break;

		}  // end delete all

		// ------------------------------------------------------------------------
		case GLM_DEL_ENTRY:
		{
			Int i;

			if( list->endPos <= (Int)mData1 )
				break;

			ListEntryCell *cells = list->listData[mData1].cell;
			if(cells)
				for( i = 0; i <= list->columns; i ++ )
				{
					if( cells[i].cellType == LISTBOX_TEXT && cells[i].data )
						TheDisplayStringManager->freeDisplayString((DisplayString *) cells[i].data );	
					cells[i].data = NULL;		
					cells[i].userData = NULL;
				}
			
			delete[](list->listData[mData1].cell);
			list->listData[mData1].cell = NULL;

			memcpy( &list->listData[mData1], &list->listData[(mData1+1)],
							(list->endPos - mData1 - 1) * sizeof(ListEntryRow) );

			list->endPos--;
			list->insertPos = list->endPos;

			if( list->multiSelect )
			{
				i = 0;

				while( list->selections[i] >= 0 )
				{
					if( (Int)mData1 < list->selections[i] )
						list->selections[i]--;
					else if ( (Int)mData1 == list->selections[i] )
					{
						removeSelection( list, i );
						i--;									// compensate for lost entry
					}

					i++;
				}
			}
			else
			{
				if( (Int)mData1 < list->selectPos )
					list->selectPos--;
				else if ( (Int)mData1 == list->selectPos )
					list->selectPos = -1;
			}

			computeTotalHeight( window );
			break;

		}  // end delete entry

		// ------------------------------------------------------------------------
		case GLM_ADD_ENTRY:
		{
			Bool success = TRUE;
			Int addedIndex = -1;			
			AddMessageStruct *addInfo = (AddMessageStruct*)mData1;
			if (addInfo->row >= list->insertPos)
				addInfo->row = -1;

			Int row = addInfo->row;
			// Special case, we're just appending and we've reached the end. 
			if( addInfo->row == -1 && list->insertPos == list->listLength ) 
			{
				row = list->insertPos;
				// Check to see if we've filled our buffer and need to scroll the window
				if( list->insertPos == list->listLength )
				{
					if( list->autoPurge )
						TheWindowManager->winSendSystemMsg( window, GLM_SCROLL_BUFFER, 1, 0 );
					else
						success = FALSE;
				}
			}
			else if (addInfo->row != -1 && !addInfo->overwrite && list->insertPos == list->listLength)
			{
				// We're inserting into the middle with no space - see if we can scroll the window
				if( list->autoPurge )
					TheWindowManager->winSendSystemMsg( window, GLM_SCROLL_BUFFER, 1, 0 );
				else
					success = FALSE;
			}

			if(success)
			{
				if( addInfo->type == LISTBOX_TEXT )
				{
					addedIndex = addEntry( (UnicodeString *)addInfo->data, mData2, addInfo->row, addInfo->column, window, addInfo->overwrite );
				}
				else if ( addInfo->type == LISTBOX_IMAGE )
				{
					addedIndex = addImageEntry( (const Image *)addInfo->data, mData2, addInfo->row, addInfo->column, window, addInfo->overwrite,addInfo->width, addInfo->height );
				}
				else
					success = FALSE;
			}

			if( success ) 
			{

				if( list->autoScroll )
				{

					while( TRUE )
					{
						// If off bottom of screen, scroll and try again.
						// we use -1 because insertPos was increased in addEntry
						if( row == -1 )
						{
							if( list->listData[(list->insertPos - 1)].listHeight >= 
									(list->displayPos + list->displayHeight) )
								adjustDisplay( window, 1, TRUE );
							else
								break;
						}
						else 
						{
							if( list->listData[( row )].listHeight >= 
									(list->displayPos + list->displayHeight) )
								adjustDisplay( window, 1, TRUE );
							else
								break;
						}
					}

				}

				if( list->multiSelect )
				{
					Int i = 0;

					while( list->selections[i] >= 0 )
					{

						if( (row = list->selections[i]) != 0 )
							list->selections[i] = -1;

						i++;

					}  // end while

				}  // end if
				else
				{
					if( row == list->selectPos )
						list->selectPos = -1;
				}

			}  // end success

			return((WindowMsgHandledType) addedIndex );
			
		}  // end add entry

		// ------------------------------------------------------------------------
		case GLM_TOGGLE_MULTI_SELECTION:
		{

			if( (Int)mData1 < 0 )
			{
				// a negative number will purge the entire list.
				if( list->multiSelect )
					memset( list->selections, -1, list->listLength * sizeof(Int) );
				else
				{
					// this message has no effect in a non-multi listbox
				}

				break;
			}

			// if there is no cells we shouldn't be selecting this entry
			if( !list->listData[ mData1 ].cell )
				break;

			if( list->multiSelect )
			{
				Int i = 0;
				Bool removed = FALSE;

				while( list->selections[i] >= 0 )
				{
					if( list->selections[i] == (Int)mData1 )
					{
						removeSelection( list, i );
						removed = TRUE;
						break;
					}
					
					i++;
				}

				if( removed == FALSE )
				{
					list->selections[i] = (Int)mData1;
					list->selections[i+1] = -1;
				}
			}
			else
			{
				// this message has no effect in a non-multi listbox
			}
			
			break;

		}  // end toggle multi-select

		// ------------------------------------------------------------------------
		case GLM_SET_SELECTION:
		{
			const Int *selectList = (const Int *)mData1;
			Int selectCount = (Int)mData2;
			DEBUG_ASSERTCRASH( list->multiSelect || selectCount == 1, ("Bad selection size"));

			if( selectList[0] < 0 || list->listLength <= selectList[0] )
			{

				if( list->multiSelect )
					memset( list->selections, -1, list->listLength * sizeof(Int) );
				else
					list->selectPos = -1;

				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GLM_SELECTED,
																						(WindowMsgData)window, 
																						list->selectPos );

				break;

			}

			if( list->multiSelect )
			{
				// forced selections override the entire selection list.
				Int i=0;
				for (; i<selectCount && i<list->endPos; ++i)
				{
					// don't select off the end
					if (list->listLength <= selectList[i])
					{
						break;
					}

					// if there is no cells we shouldn't be selecting this entry
					if( !list->listData[ selectList[i] ].cell )
					{
						break;
					}

					list->selections[i] = selectList[i];
				}
				list->selections[i] = -1;
			}
			else
			{
				// if there is no cells we shouldn't be selecting this entry
				if( !list->listData[ selectList[0] ].cell )
				{
					break;
				}

				list->selectPos = selectList[0];
				GameWindow *parent = window->winGetParent();
				if( parent && BitIsSet( parent->winGetStyle(), GWS_COMBO_BOX ) )
				{
					TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GLM_SELECTED,
																						(WindowMsgData)window, 
																						list->selectPos );				
					break;
				}
				if( list->listData[list->selectPos].listHeight < list->displayPos )
				{
				
					TheWindowManager->winSendSystemMsg( window, GLM_UPDATE_DISPLAY, 
																							list->selectPos, 0 );
				}
				else if( list->listData[list->selectPos].listHeight > 
								 (list->displayPos + list->displayHeight) )
				{

					if( list->selectPos > 0 )
						list->displayPos = 
							list->listData[list->selectPos].listHeight - list->displayHeight;
					else
						list->displayPos = 0;

					adjustDisplay( window, 0, TRUE );

				}  // end else if

			}  // end else

			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GLM_SELECTED,
																					(WindowMsgData)window, 
																					list->selectPos );
			break;

		}  // end set selection

		// ------------------------------------------------------------------------
		case GLM_SCROLL_BUFFER:
		{

			if( list->endPos < (Int)mData1 )
				break;

			//
			// scroll buffer literally scrolls the entire list data buffer
			// up one entry, effectively removing the top entry.  
			//
			
			//
			// Loop through and remove all the entries from the top up until we reach
			// the position mData1 contains
			//
			ListEntryCell *cells = NULL;
			Int i = 0;
			for (; i < (Int)mData1; i++)
			{
				cells = list->listData[i].cell;
				
				if(cells)
					for( Int j = 0; j < list->columns; j++ )
					{
						if( cells[j].cellType == LISTBOX_TEXT && cells[j].data )
							TheDisplayStringManager->freeDisplayString((DisplayString *) cells[j].data );	
//						if (cells[i].userData)
//							free(cells[i].userData);
						cells[j].data = NULL;		
						cells[j].userData = NULL;
						cells[j].color = 0;
						cells[j].cellType = 0;
					}
				
				delete[](list->listData[i].cell);
				list->listData[i].cell = NULL;
			}

			
			//
			// copy the cells up 
			//
			memcpy(list->listData, &list->listData[mData1],
						(list->endPos - mData1) * sizeof(ListEntryRow) );

			list->endPos -= mData1;
			list->insertPos = list->endPos;

			//
			// remove the display or links to images after the shift
			//
			for(i = 0; i < (Int)mData1; i ++)
			{
				list->listData[list->endPos + i].cell = NULL;
			}


			if( list->multiSelect )
			{
				Int i = 0;

				while( list->selections[i] >= 0 )
				{
					if( (Int)mData1 >= list->selections[i] )
						list->selections[i] -= (Int)mData1;
					else
					{
						removeSelection( list, i );
						i--;									// compensate for lost entry
					}

					i++;
				}
			}
			else
			{
				if( list->selectPos > 0 )
					list->selectPos -= mData1;
			}

			if( list->displayPos > 0 )
				adjustDisplay( window, (-1 * mData1), TRUE );

			computeTotalHeight( window );

			break;

		}  // end scroll buffer

		// ------------------------------------------------------------------------
		case GLM_GET_SELECTION:
		{
			
			if( list->multiSelect )
				*(Int*)mData2 = (Int)list->selections;
			else
				*(Int*)mData2 = list->selectPos;

			break;

		}  // end get selection

		// ------------------------------------------------------------------------
		case GLM_SET_UP_BUTTON:
			list->upButton = (GameWindow *)mData1;
			break;

		// ------------------------------------------------------------------------
		case GLM_SET_DOWN_BUTTON:
			list->downButton = (GameWindow *)mData1;
			break;

		// ------------------------------------------------------------------------
		case GLM_SET_SLIDER:
			list->slider = (GameWindow *)mData1;
			break;

		// ------------------------------------------------------------------------
		case GWM_CREATE:
			break;

		// ------------------------------------------------------------------------
		case GGM_RESIZED:
		{
			Int width = (Int)mData1;
			Int height = (Int)mData2;
			ICoord2D downSize = {0, 0};
			ICoord2D upSize = {0, 0};
			ICoord2D sliderSize = {0, 0};
			GameWindow *child = NULL;
			ICoord2D sliderChildSize = {0, 0};

			// get needed window sizes
			if (list->downButton)
				list->downButton->winGetSize( &downSize.x, &downSize.y );
			if (list->upButton)
				list->upButton->winGetSize( &upSize.x, &upSize.y );
			if (list->slider)
			{
				list->slider->winGetSize( &sliderSize.x, &sliderSize.y );
				child = list->slider->winGetChild();
				if (child)
					child->winGetSize( &sliderChildSize.x, &sliderChildSize.y );
			}

			if( list->upButton )
			{

				list->upButton->winSetPosition( width - upSize.x - 2, 2 );

			}

			if( list->downButton )
			{
	
				list->downButton->winSetPosition( width - downSize.x - 2,
																					height - downSize.y - 2 );

			}

			if( list->slider )
			{
				list->slider->winSetSize( sliderSize.x, 
																	height - (2 * upSize.y) -6 );
				list->slider->winSetPosition( width - sliderSize.x -2, upSize.y + 3 );
				
			}

			list->displayHeight = height;
			// store display height
			if( instData->getTextLength() )
			{
				list->displayHeight -= TheWindowManager->winFontHeight( instData->getFont() );
			}

			//
			// Setup listbox Columns
			//
			if( list->columns == 1 )
			{
				list->columnWidth[0] = width;
				if( list->slider )
				{
					ICoord2D sliderSize;

					list->slider->winGetSize( &sliderSize.x, &sliderSize.y );
					list->columnWidth[0] -= sliderSize.x;

				}  // end if
			}// if
			else
			{
				if( !list->columnWidthPercentage )
					break;
				if(!list->columnWidth)
					break;

				Int totalWidth = width;
				if( list->slider )
				{
					ICoord2D sliderSize;

					list->slider->winGetSize( &sliderSize.x, &sliderSize.y );
					totalWidth -= sliderSize.x;

				}  // end if
				for(Int i = 0; i < list->columns; i++ )
				{
					list->columnWidth[i] = list->columnWidthPercentage[i] * totalWidth / 100;
				}// for
			}// else
				//reset the total height
				computeTotalHeight(window);
			break;

		}  // end resized

		// ------------------------------------------------------------------------
		case GLM_UPDATE_DISPLAY:
		{

			if( mData1 > 0 )
				// set the display to the top of a specific entry
				// which is the previous listHeight + 1
				list->displayPos = list->listData[(mData1 - 1)].listHeight + 1;
			else
				list->displayPos = 0;

			if( list->displayPos + list->displayHeight >= list->totalHeight )
			{
				list->displayPos = list->totalHeight - list->displayHeight;
			}
			adjustDisplay( window, 0, TRUE );
			break;

		}  // end update display

		// ------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			Int i;

			// Loop through and destroy any display strings we've allocated 
			for( i = 0; i < list->listLength; i++ )
			{
				//We're now onto a row of cells we are not using anymore Pull off the cells and loop through them
				ListEntryCell *cells = list->listData[i].cell;
				for (int j = list->columns - 1; j >=0; j-- )
				{			
					if(!cells)
						break;
					if( cells[j].cellType == LISTBOX_TEXT )
					{
						// If we can delete the stuff that won't be showing up in the new listData struture
						if ( cells[j].data )
						{
							TheDisplayStringManager->freeDisplayString((DisplayString *) cells[j].data );	
						}
					}
//					if ( cells[j].userData ) 
//						free(cells[j].userData);
					
					// Null out the data pointers so they're not destroyed when we free up this listdata
					cells[j].userData = NULL;
					cells[j].data = NULL;
				}
				delete[](list->listData[i].cell);
				list->listData[i].cell = NULL;
			}
	
			delete[]( list->listData );
			if( list->columnWidth	)
				delete[]( list->columnWidth );
			if( list->columnWidthPercentage	)
				delete[]( list->columnWidthPercentage );
			if( list->multiSelect )
				delete[]( list->selections );
			delete( list );
			break;

		}  // end destroy

		// ------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// If we're losing focus
			if( mData1 == FALSE )
			{
				BitClear( instData->m_state, WIN_STATE_HILITED );
			}
			else
			{
				BitSet( instData->m_state, WIN_STATE_HILITED );
			}

			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GGM_FOCUS_CHANGE,
																					mData1, 
																					window->winGetWindowId() );

			*(Bool*)mData2 = TRUE;
			break;

		}  // end input focus
		// ------------------------------------------------------------------------
		case GSM_SLIDER_TRACK:
		{
			SliderData *sData = (SliderData *)list->slider->winGetUserData();
			list->displayPos = sData->maxVal - mData2; 

			if( list->displayPos > (list->totalHeight - list->displayHeight + 1) )
				list->displayPos = (list->totalHeight - list->displayHeight + 1);
			
			if( list->displayPos < 0)
				list->displayPos = 0;

			adjustDisplay( window, 0, FALSE );
			break;

		}  // end slider track
		// ------------------------------------------------------------------------
		case GLM_SET_ITEM_DATA:
		{
			void *data = (void *)mData2;
			pos = (ICoord2D *)mData1;
			
			if (pos->y >= 0 && pos->y < list->endPos && list->listData[pos->y].cell)
				list->listData[pos->y].cell[pos->x].userData = data;

			break;
		}//case GLM_SET_ITEM_DATA:
		// ------------------------------------------------------------------------
		case GLM_GET_ITEM_DATA:
		{
			pos = (ICoord2D *)mData1;
			void **data = (void **)mData2;

			*data = NULL;  // initialize to NULL			
			if (pos->y >= 0 && pos->y < list->endPos && list->listData[pos->y].cell)
				*data = list->listData[pos->y].cell[pos->x].userData;

			break;
		}//case GLM_GET_ITEM_DATA:


		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end GadgetListBoxSystem

// GadgetListBoxSetColors =====================================================
/** Set the colors for a list box, note that this will also automatically
	* change the colors of any attached slider, slider thumb, and slider 
	* buttons */
//=============================================================================
void GadgetListBoxSetColors( GameWindow *listbox,
														 Color enabledColor, 
														 Color enabledBorderColor,
														 Color enabledSelectedItemColor, 
														 Color enabledSelectedItemBorderColor,
														 Color disabledColor, 
														 Color disabledBorderColor,
														 Color disabledSelectedItemColor, 
														 Color disabledSelectedItemBorderColor,
														 Color hiliteColor, 
														 Color hiliteBorderColor,
														 Color hiliteSelectedItemColor, 
														 Color hiliteSelectedItemBorderColor )
{
	ListboxData *listboxData = (ListboxData *)listbox->winGetUserData();

	// enabled
	GadgetListBoxSetEnabledColor( listbox, enabledColor );
	GadgetListBoxSetEnabledBorderColor( listbox, enabledBorderColor );
	GadgetListBoxSetEnabledSelectedItemColor( listbox, enabledSelectedItemColor );
	GadgetListBoxSetEnabledSelectedItemBorderColor( listbox, enabledSelectedItemBorderColor );

	// disabled
	GadgetListBoxSetDisabledColor( listbox, disabledColor );
	GadgetListBoxSetDisabledBorderColor( listbox, disabledBorderColor );
	GadgetListBoxSetDisabledSelectedItemColor( listbox, disabledSelectedItemColor );
	GadgetListBoxSetDisabledSelectedItemBorderColor( listbox, disabledSelectedItemBorderColor );

	// hilited
	GadgetListBoxSetHiliteColor( listbox, hiliteColor );
	GadgetListBoxSetHiliteBorderColor( listbox, hiliteBorderColor );
	GadgetListBoxSetHiliteSelectedItemColor( listbox, hiliteSelectedItemColor );
	GadgetListBoxSetHiliteSelectedItemBorderColor( listbox, hiliteSelectedItemBorderColor );

	// assign default slider colors and images as part of the list box
	GameWindow *slider = listboxData->slider;
	if( slider )
	{
		GameWindow *upButton = listboxData->upButton;
		GameWindow *downButton = listboxData->downButton;

		// slider and slider thumb ----------------------------------------------

		// enabled
		GadgetSliderSetEnabledColor( slider, GadgetListBoxGetEnabledColor( listbox ) );
		GadgetSliderSetEnabledBorderColor( slider, GadgetListBoxGetEnabledBorderColor( listbox ) );

		// Disabled
		GadgetSliderSetDisabledColor( slider, GadgetListBoxGetDisabledColor( listbox ) );
		GadgetSliderSetDisabledBorderColor( slider, GadgetListBoxGetDisabledBorderColor( listbox ) );

		// Hilite
		GadgetSliderSetHiliteColor( slider, GadgetListBoxGetHiliteColor( listbox ) );
		GadgetSliderSetHiliteBorderColor( slider, GadgetListBoxGetHiliteBorderColor( listbox ) );
	
		// up button ------------------------------------------------------------

		// enabled
		GadgetButtonSetEnabledColor( upButton, GadgetSliderGetEnabledColor( slider ) );
		GadgetButtonSetEnabledBorderColor( upButton, GadgetSliderGetEnabledBorderColor( slider ) );
		GadgetButtonSetEnabledSelectedColor( upButton, GadgetSliderGetEnabledSelectedThumbColor( slider ) );
		GadgetButtonSetEnabledSelectedBorderColor( upButton, GadgetSliderGetEnabledSelectedThumbBorderColor( slider ) );

		// disabled
		GadgetButtonSetDisabledColor( upButton, GadgetSliderGetDisabledColor( slider ) );
		GadgetButtonSetDisabledBorderColor( upButton, GadgetSliderGetDisabledBorderColor( slider ) );
		GadgetButtonSetDisabledSelectedColor( upButton, GadgetSliderGetDisabledSelectedThumbColor( slider ) );
		GadgetButtonSetDisabledSelectedBorderColor( upButton, GadgetSliderGetDisabledSelectedThumbBorderColor( slider ) );

		// hilite
		GadgetButtonSetHiliteColor( upButton, GadgetSliderGetHiliteColor( slider ) );
		GadgetButtonSetHiliteBorderColor( upButton, GadgetSliderGetHiliteBorderColor( slider ) );
		GadgetButtonSetHiliteSelectedColor( upButton, GadgetSliderGetHiliteSelectedThumbColor( slider ) );
		GadgetButtonSetHiliteSelectedBorderColor( upButton, GadgetSliderGetHiliteSelectedThumbBorderColor( slider ) );

		// down button ----------------------------------------------------------

		// enabled
		GadgetButtonSetEnabledColor( downButton, GadgetSliderGetEnabledColor( slider ) );
		GadgetButtonSetEnabledBorderColor( downButton, GadgetSliderGetEnabledBorderColor( slider ) );
		GadgetButtonSetEnabledSelectedColor( downButton, GadgetSliderGetEnabledSelectedThumbColor( slider ) );
		GadgetButtonSetEnabledSelectedBorderColor( downButton, GadgetSliderGetEnabledSelectedThumbBorderColor( slider ) );

		// disabled
		GadgetButtonSetDisabledColor( downButton, GadgetSliderGetDisabledColor( slider ) );
		GadgetButtonSetDisabledBorderColor( downButton, GadgetSliderGetDisabledBorderColor( slider ) );
		GadgetButtonSetDisabledSelectedColor( downButton, GadgetSliderGetDisabledSelectedThumbColor( slider ) );
		GadgetButtonSetDisabledSelectedBorderColor( downButton, GadgetSliderGetDisabledSelectedThumbBorderColor( slider ) );

		// hilite
		GadgetButtonSetHiliteColor( downButton, GadgetSliderGetHiliteColor( slider ) );
		GadgetButtonSetHiliteBorderColor( downButton, GadgetSliderGetHiliteBorderColor( slider ) );
		GadgetButtonSetHiliteSelectedColor( downButton, GadgetSliderGetHiliteSelectedThumbColor( slider ) );
		GadgetButtonSetHiliteSelectedBorderColor( downButton, GadgetSliderGetHiliteSelectedThumbBorderColor( slider ) );

	}  // end if

}  // end GadgetListBoxSetColors

// GadgetListBoxGetText =======================================================
/** Get the text for a list box entry */
//=============================================================================
UnicodeString GadgetListBoxGetText( GameWindow *listbox, Int row, Int column)
{
	Color color;
	return GadgetListBoxGetTextAndColor( listbox,&color,row,column );
}  // end GadgetListBoxGetText

// GadgetListBoxGetText =======================================================
/** Get the text for a list box entry */
//=============================================================================
UnicodeString GadgetListBoxGetTextAndColor( GameWindow *listbox, Color *color, Int row, Int column)
{
	*color = 0;
	// sanity
	if( listbox == NULL  || row == -1 || column == -1)
		return UnicodeString::TheEmptyString;

	// verify that this is a list box
	if( BitIsSet( listbox->winGetStyle(), GWS_SCROLL_LISTBOX ) == FALSE )
		return UnicodeString::TheEmptyString;
	TextAndColor tAndC;
	//UnicodeString result;
	ICoord2D pos;
	pos.x = column;
	pos.y = row;
	TheWindowManager->winSendSystemMsg( listbox, GLM_GET_TEXT, (WindowMsgData)&pos, (WindowMsgData)&tAndC );
	

		*color = tAndC.color;
		return tAndC.string;
	
	//return UnicodeString::TheEmptyString;

}  // end GadgetListBoxGetText

// GadgetListBoxAddEntryText ==================================================
/** Add a new string entry into the listbox at the insert position */
//=============================================================================
Int GadgetListBoxAddEntryText( GameWindow *listbox,
														UnicodeString text,
														Color color, Int row, Int column, Bool overwrite )
{
	if (!listbox)
		return -1;
	if (text.isEmpty())
		text = UnicodeString(L" ");
	Int index;
	AddMessageStruct addInfo;
	addInfo.row = row;
	addInfo.column = column;
	addInfo.type = LISTBOX_TEXT;
	addInfo.data = &text;
	addInfo.overwrite = overwrite;
	addInfo.height = -1;
	addInfo.width = -1;

	ListboxData *listData = (ListboxData *)listbox->winGetUserData();
	Bool wasFull = (listData->listLength <= listData->endPos);
	Int newEntryOffset = (wasFull)?0:1;
	Int oldBottomIndex = GadgetListBoxGetBottomVisibleEntry(listbox);

	/// @TODO: Don't do this type cast!
	index = (Int) TheWindowManager->winSendSystemMsg( listbox, GLM_ADD_ENTRY, (WindowMsgData)&addInfo, color );

	//DEBUG_ASSERTLOG(!listData->scrollIfAtEnd, ("Adding line %d (orig end was %d, newEntryOffset is %d, (%d-%d)?=%d, isFull=%d/%d ll=%d, end=%d\n",
		//index, oldBottomIndex, newEntryOffset, index, oldBottomIndex, newEntryOffset, wasFull, GadgetListBoxIsFull(listbox), listData->listLength, listData->endPos));
	if(listData->scrollIfAtEnd && index - oldBottomIndex == newEntryOffset && GadgetListBoxIsFull(listbox))
	{
	  GadgetListBoxSetBottomVisibleEntry( listbox, index );
	}

	return (index);
}  // end GadgetListBoxAddEntry

// GadgetListBoxAddEntryImage =================================================
/** Add a new string entry into the listbox at the insert position */
//=============================================================================
Int GadgetListBoxAddEntryImage( GameWindow *listbox, const Image *image,
															 Int row, Int column,
															 Int hight, Int width,
															 Bool overwrite, Color color )
{
	Int index;
	AddMessageStruct addInfo;
	addInfo.row = row;
	addInfo.column = column;
	addInfo.type = LISTBOX_IMAGE;
	addInfo.data = image;
	addInfo.overwrite = overwrite;
	addInfo.height = hight;
	addInfo.width = width;
	/// @TODO: Don't do this type cast!
	index = (Int) TheWindowManager->winSendSystemMsg( listbox, GLM_ADD_ENTRY, (WindowMsgData)&addInfo, color );
	return (index);
}  // end GadgetListBoxAddEntryImage

Int GadgetListBoxAddEntryImage( GameWindow *listbox, const Image *image,
															 Int row, Int column,
															 Bool overwrite, Color color )
{
	return GadgetListBoxAddEntryImage(listbox, image, row, column,  -1, -1, overwrite, color);
}

// GadgetListBoxSetFont =======================================================
/** Set the font for a listbox control, we need to set the window
	* text font, the tooltip font, and the edit text display strings for
	* the text data itself and the secret text */
//=============================================================================
void GadgetListBoxSetFont( GameWindow *g, GameFont *font )
{
	ListboxData *listData = (ListboxData *)g->winGetUserData();
	DisplayString *dString;

	// set the font for the display strings all windows have
	dString = g->winGetInstanceData()->getTextDisplayString();
	if( dString )
		dString->setFont( font );
	dString = g->winGetInstanceData()->getTooltipDisplayString();
	if( dString )
		dString->setFont( font );

	// listbox specific	
	if( listData )
		for( Int i = 0; i < listData->listLength; i++ )
		{
			if(listData->listData[i].cell)
				for( Int j = 0; j < listData->columns; j++ )
				{
					if( listData->listData[i].cell[j].cellType == LISTBOX_TEXT && listData->listData[i].cell[j].data )
					{
						dString = (DisplayString *)listData->listData[i].cell[j].data;
						dString->setFont( font );
					}
				}
		}  // end for i

}  // end GadgetListBoxSetFont

// GadgetListboxCreateScrollbar ===============================================
/** Create the scroll bar using a slider and two buttons for a listbox */
//=============================================================================
void GadgetListboxCreateScrollbar( GameWindow *listbox )
{
	ListboxData *listData = (ListboxData *)listbox->winGetUserData();
	WinInstanceData winInstData;
	SliderData sData = { 0 };
	Int buttonWidth, buttonHeight;
	Int sliderButtonWidth, sliderButtonHeight;
	Int fontHeight;
	Int top;
	Int bottom;
	UnsignedInt status = listbox->winGetStatus();
	Bool title = FALSE;
	Int width, height;

	// get width and height of listbox
	listbox->winGetSize( &width, &height );

	// do we have a title
	if( listbox->winGetTextLength() )
		title = TRUE;

	// remove unwanted status bits.
	status &= ~(WIN_STATUS_BORDER | WIN_STATUS_HIDDEN | WIN_STATUS_NO_INPUT);

	fontHeight = TheWindowManager->winFontHeight( listbox->winGetFont() );
	top = title ? (fontHeight + 1):0;
	bottom = title ? (height - (fontHeight + 1)):height;

	// intialize instData
	winInstData.init();

	// size of button
	buttonWidth = 21;//GADGET_SIZE;
	buttonHeight = 22;//GADGET_SIZE;

	// ----------------------------------------------------------------------
	// Create Top Button
	// ----------------------------------------------------------------------
	status |= WIN_STATUS_IMAGE;

	winInstData.m_owner = listbox;
	winInstData.m_style = GWS_PUSH_BUTTON;

	// if listbox tracks, so will this sub control
	if( BitIsSet( listbox->winGetStyle(), GWS_MOUSE_TRACK ) )
		BitSet( winInstData.m_style, GWS_MOUSE_TRACK );

	listData->upButton = 
		 TheWindowManager->gogoGadgetPushButton( listbox,
																						 status | WIN_STATUS_ACTIVE | WIN_STATUS_ENABLED,
																						 width - buttonWidth -2, top+2,
																						 buttonWidth, buttonHeight,
																						 &winInstData, NULL, TRUE );

	// ----------------------------------------------------------------------
	// Create Bottom Button
	// ----------------------------------------------------------------------

	winInstData.init();
	winInstData.m_style = GWS_PUSH_BUTTON;
	winInstData.m_owner = listbox;

	// if listbox tracks, so will this sub control
	if( BitIsSet( listbox->winGetStyle(), GWS_MOUSE_TRACK ) )
		BitSet( winInstData.m_style, GWS_MOUSE_TRACK );

	listData->downButton = 
			 TheWindowManager->gogoGadgetPushButton( listbox,
																							 status | WIN_STATUS_ACTIVE | WIN_STATUS_ENABLED,
																							 width - buttonWidth -2,
																							 (top + bottom - buttonHeight -2),
																							 buttonWidth, buttonHeight,
																							 &winInstData, NULL, TRUE );

	// ----------------------------------------------------------------------
	// create the slider
	// ----------------------------------------------------------------------

	// size of button
	sliderButtonWidth = buttonWidth;//GADGET_SIZE;
	sliderButtonHeight = GADGET_SIZE;

	// intialize instData
	winInstData.init();
	winInstData.m_style = GWS_VERT_SLIDER;
	winInstData.m_owner = listbox;

	// if listbox tracks, so will this sub control
	if( BitIsSet( listbox->winGetStyle(), GWS_MOUSE_TRACK ) )
		BitSet( winInstData.m_style, GWS_MOUSE_TRACK );

	// intialize sData
	memset( &sData, 0, sizeof(SliderData) );

	// Create Slider
	listData->slider = 
			TheWindowManager->gogoGadgetSlider( listbox,
																					status | WIN_STATUS_ACTIVE | WIN_STATUS_ENABLED,
																					width - sliderButtonWidth - 2,
																					(top + buttonHeight + 3),
																					sliderButtonWidth, bottom - (2 * buttonHeight) - 6,
																					&winInstData, &sData, NULL, TRUE );

	// we now have all the scrollbar parts, this better be set :)
	listData->scrollBar = TRUE;

}  // end GadgetListBoxCreateScrollbar

// GadgetListBoxAddMultiSelect ================================================
/** Enable multi selections for a listbox
	*
	* ASSUMPTION: The listLength must already be set at this time so
	* that we can make enough space to hold all the selection data, if you
	* change the list length you must also change the selection array
	* for multi select listboxes 
	*/
//=============================================================================
void GadgetListBoxAddMultiSelect( GameWindow *listbox )
{
	ListboxData *listboxData = (ListboxData *)listbox->winGetUserData();

	DEBUG_ASSERTCRASH(listboxData && listboxData->selections == NULL, ("selections is not NULL"));
	listboxData->selections = NEW Int [listboxData->listLength];
	DEBUG_LOG(( "Enable list box multi select: listLength (select) = %d * %d = %d bytes;\n",
					 listboxData->listLength, sizeof(Int), 
					 listboxData->listLength *sizeof(Int) ));

	if( listboxData->selections == NULL )
	{

		delete[]( listboxData->listData );
		listboxData->listData = NULL;
		return;

	}  // end if

	memset( listboxData->selections, -1,
		      listboxData->listLength * sizeof(Int) );

	// set mutliselect flag
	listboxData->multiSelect = TRUE;

	// adjust the input procedure for the listbox
	listbox->winSetInputFunc( GadgetListBoxMultiInput );

}  // end GadgetListBoxEnableMultiSelect

// GadgetListBoxRemoveMultiSelect =============================================
/** Remove multi select capability from a listbox */
//=============================================================================
void GadgetListBoxRemoveMultiSelect( GameWindow *listbox )
{
	ListboxData *listData = (ListboxData *)listbox->winGetUserData();

	if( listData->selections )
	{

		delete[]( listData->selections );
		listData->selections = NULL;

	}  // end if

	listData->multiSelect = FALSE;

	// adjust the input procedure for the listbox
	listbox->winSetInputFunc( GadgetListBoxInput );

}  // end GadgetListBoxRemoveMultiSelect

// GadgetListBoxSetListLength =================================================
/** Set OR reset the list length data contained in the listboxData
	* parameter.  When adjusting the size of lists we also have to
	* adjust multiselection lists if present and any display 
	* strings present */
//=============================================================================
void GadgetListBoxSetListLength( GameWindow *listbox, Int newLength )
{
	ListboxData *listboxData = (ListboxData *)listbox->winGetUserData();


//	ListboxData *listboxData = (ListboxData *)listbox->winGetUserData();
//	ListEntry *newData = (ListEntry *)malloc(newLength * sizeof(ListEntry));
	DEBUG_ASSERTCRASH(listboxData, ("We don't have our needed listboxData!"));
	if( !listboxData )
		return;
	DEBUG_ASSERTCRASH(listboxData->columns > 0,("We need at least one Column in the listbox"));
	if( listboxData->columns < 1 )
		return;
	
  Int columns = listboxData->columns;
	ListEntryRow *newData = NEW ListEntryRow[ newLength ];	
	DEBUG_ASSERTCRASH(newData, ("Unable to allocate new data structures for the Listbox"));
	if( !newData )
		return;
	Int i;
  // zero out the new Data structure
	memset( newData, 0, newLength  * sizeof( ListEntryRow ) );
	 
	// we want to copy over different amounts of data depending on if we're adding
	// to the list box or removing from the listbox
	if(newLength >= listboxData->listLength)
	{
		memcpy(newData,listboxData->listData,listboxData->listLength * sizeof( ListEntryRow ) );
	}
	else
	{
		// If we're removing entries from the listbox, we need to reset the length, 
		// position, and selection to their new places
		if( listboxData->displayPos >newLength)
			listboxData->displayPos = newLength;
		//if we're multiselect, just select no position
		if(listboxData->selectPos > newLength || listboxData->multiSelect) 
			listboxData->selectPos = -1;
    if(listboxData->insertPos > newLength)
			listboxData->insertPos = newLength;

    listboxData->endPos = newLength;
		//copy only the data that we'll be needing.		
		memcpy(newData,listboxData->listData,newLength * sizeof( ListEntryRow ) );
	}

	// Loop through and destroy any display strings we've allocated that aren't used
	for( i = 0; i < listboxData->listLength; i++ )
	{
		//We're now onto a row of cells we are not using anymore Pull off the cells and loop through them
		ListEntryCell *cells = listboxData->listData[i].cell;
		for (int j = columns - 1; j >=0; j-- )
		{			
			if(!cells)
				break;
			if ( i >= newLength )
			{
				if( cells[j].cellType == LISTBOX_TEXT  && i >= newLength)
				{
					// If we can delete the stuff that won't be showing up in the new listData struture
					if ( cells[j].data )
					{
						TheDisplayStringManager->freeDisplayString((DisplayString *) cells[j].data );	
					}
				}
//			if ( cells[j].userData ) 
//					free(cells[j].userData);
			}
		}
		if (i >= newLength) {
			delete[](listboxData->listData[i].cell);
			listboxData->listData[i].cell = NULL;
		}
	}

	listboxData->listLength = newLength;

	if( listboxData->listData )
		delete[]( listboxData->listData );
	listboxData->listData = newData;
	
	//reset the total height
	computeTotalHeight(listbox);

  // Sanity check that everything was created properly
	if( listboxData->listData == NULL )
	{

		DEBUG_LOG(( "Unable to allocate listbox data pointer\n" ));
		assert( 0 );
		return;

	}  // end if
	
	// adjust the selection array for multi select listboxes
	if( listboxData->multiSelect )
	{
		
		GadgetListBoxRemoveMultiSelect( listbox );
		GadgetListBoxAddMultiSelect( listbox );

	}  // end if

}  // end GadgetListBoxSetListLength

// GadgetListBoxGetListLength =================================================
/** Get the list length data contained in the listboxData
	* parameter. */
//=============================================================================
Int GadgetListBoxGetListLength( GameWindow *listbox )
{
	ListboxData *listboxData = (ListboxData *)listbox->winGetUserData();
	if (listboxData->multiSelect)
	{
		return listboxData->listLength;
	}
	else
	{
		return 1;
	}
}  // end GadgetListBoxGetListLength

// GadgetListBoxGetNumEntries =================================================
/** Get the list length data contained in the listboxData
	* parameter. */
//=============================================================================
Int GadgetListBoxGetNumEntries( GameWindow *listbox )
{
	if (!listbox)
		return 0;

	ListboxData *listboxData = (ListboxData *)listbox->winGetUserData();
	if (listboxData)
		return listboxData->endPos;

	return 0;
}  // end GadgetListBoxGetNumEntries

//-------------------------------------------------------------------------------------------------
/** Get the selected item(s) of a listbox.  For a single select listbox the parameter
	* should be a single integer pointer.  For a multi select listbox the parameter
	* should be an (Int *), this array returned will be an array of indices of the selected items
	* of the list box.  An entry of -1 in this array is the "end" of the selected list,
	* and this list will never be larger than the max items in the list box */
//-------------------------------------------------------------------------------------------------
void GadgetListBoxGetSelected( GameWindow *listbox, Int *selectList )
{

	// sanity
	if( listbox == NULL )
		return;

	// get selected indeces via system message
	TheWindowManager->winSendSystemMsg( listbox, GLM_GET_SELECTION, 0, (WindowMsgData)selectList );

}  // end GadgetListBoxGetSelected

//-------------------------------------------------------------------------------------------------
/** Set the selected item of a listbox.  The parameter is a single integer.  If
  * the selected index is less than 0, nothing is selected. */
//-------------------------------------------------------------------------------------------------
void GadgetListBoxSetSelected( GameWindow *listbox, Int selectIndex )
{

	// sanity
	if( listbox == NULL )
		return;

	// set selected index via system message
	TheWindowManager->winSendSystemMsg( listbox, GLM_SET_SELECTION, (WindowMsgData)(&selectIndex), 1 );

}  // end GadgetListBoxSetSelected

//-------------------------------------------------------------------------------------------------
/** Set the selected item of a listbox. */
//-------------------------------------------------------------------------------------------------
void GadgetListBoxSetSelected( GameWindow *listbox, const Int *selectList, Int selectCount )
{
	// sanity
	if( listbox == NULL )
		return;
	// set selected index via system message
	TheWindowManager->winSendSystemMsg( listbox, GLM_SET_SELECTION, (WindowMsgData)selectList, selectCount );
}

//-------------------------------------------------------------------------------------------------
/** Reset the content of the listbox */
//-------------------------------------------------------------------------------------------------
void GadgetListBoxReset( GameWindow *listbox )
{

	// sanity
	if( listbox == NULL )
		return;

	// reset via system message
	TheWindowManager->winSendSystemMsg( listbox, GLM_DEL_ALL, 0, 0 );

}  // end GadgetListBoxReset

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void GadgetListBoxSetItemData( GameWindow *listbox, void *data, Int row, Int column )
{
	ICoord2D pos;
	pos.x = column;
	pos.y = row;

	if (listbox)
		TheWindowManager->winSendSystemMsg( listbox, GLM_SET_ITEM_DATA, (WindowMsgData)&pos, (WindowMsgData)data);
		
}// void GadgetListBoxSetItemData( Int index, void *data )

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void *GadgetListBoxGetItemData( GameWindow *listbox, Int row, Int column)
{
	void *data = NULL;
	ICoord2D pos;
	pos.x = column;
	pos.y = row;

	if (listbox)
	{
		TheWindowManager->winSendSystemMsg( listbox, GLM_GET_ITEM_DATA, (WindowMsgData)&pos, (WindowMsgData)&data);
	}
	return (data);
	
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Int GadgetListBoxGetBottomVisibleEntry( GameWindow *window )
{
	if (!window)
		return 0;

	ListboxData *listData = (ListboxData *)window->winGetUserData();
	if (!listData)
		return 0;

	return getListboxBottomEntry(listData);

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool GadgetListBoxIsFull(GameWindow *window)
{
	if (!window)
		return FALSE;
	ListboxData *listData = (ListboxData *)window->winGetUserData();
	if (!listData)
		return FALSE;

	Int entry = getListboxBottomEntry(listData);
	if(listData->listData[entry].listHeight >= listData->displayPos + listData->displayHeight - 5)
		return TRUE;
	else
		return FALSE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void GadgetListBoxSetBottomVisibleEntry( GameWindow *window, Int newPos )
{
	if (!window)
		return;

	ListboxData *listData = (ListboxData *)window->winGetUserData();
	if (!listData)
		return;

	int prevPos = GadgetListBoxGetBottomVisibleEntry( window );

	adjustDisplay(window, newPos - prevPos + 1, true);
} // void GadgetListBoxSetTopVisibleEntry( GameWindow *window, Int newPos )

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Int GadgetListBoxGetTopVisibleEntry( GameWindow *window )
{
	if (!window)
		return 0;

	ListboxData *listData = (ListboxData *)window->winGetUserData();
	if (!listData)
		return 0;

	return getListboxTopEntry(listData);
} // Int GadgetListBoxGetTopVisibleEntry( GameWindow *window )

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void GadgetListBoxSetTopVisibleEntry( GameWindow *window, Int newPos )
{
	if (!window)
		return;

	ListboxData *listData = (ListboxData *)window->winGetUserData();
	if (!listData)
		return;

	int prevPos = GadgetListBoxGetTopVisibleEntry( window );

	adjustDisplay(window, newPos - prevPos, true);
} // void GadgetListBoxSetTopVisibleEntry( GameWindow *window, Int newPos )

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void GadgetListBoxSetAudioFeedback( GameWindow *listbox, Bool enable )
{
	if (!listbox)
		return;
	ListboxData *listboxData = (ListboxData *)listbox->winGetUserData();
	if (!listboxData)
		return;

	listboxData->audioFeedback = enable;
}  // end GadgetListBoxSetAudioFeedback

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Int GadgetListBoxGetNumColumns( GameWindow *listbox )
{
	if (!listbox)
		return 0;
	ListboxData *listboxData = (ListboxData *)listbox->winGetUserData();
	if (!listboxData)
		return 0;

	return listboxData->columns;
}  // end GadgetListBoxGetNumColumns

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Int GadgetListBoxGetColumnWidth( GameWindow *listbox, Int column )
{
	if (!listbox)
		return 0;
	ListboxData *listboxData = (ListboxData *)listbox->winGetUserData();
	if (!listboxData)
		return 0;
	if (listboxData->columns <= column || column < 0)
		return 0;

	return listboxData->columnWidth[column];
}  // end GadgetListBoxGetNumColumns

