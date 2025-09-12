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
// File name:  C:\projects\RTS\code\Tools\GUIEdit\Source\Dialog Procedures\TabControlProperties.cpp
//
// Created:    Graham Smallwood, November 2001
//
// Desc:       Tab Control properties
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/NameKeyGenerator.h"
#include "GameClient/GameWindowManager.h"
#include "GUIEdit.h"
#include "HierarchyView.h"
#include "Properties.h"
#include "resource.h"
#include "GameClient/GadgetTabControl.h"
#include "GameClient/Gadget.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///< Pane names are derived off the Tab Control's name.
static void GadgetTabControlUpdatePaneNames( GameWindow *tabControl )
{
	TabControlData *tabData = (TabControlData *)tabControl->winGetUserData();
	WinInstanceData *controlInstData = tabControl->winGetInstanceData();

	for( Int paneIndex = 0; paneIndex < NUM_TAB_PANES; paneIndex++ )
	{
		WinInstanceData *paneInstData = tabData->subPanes[paneIndex]->winGetInstanceData();

		char buffer[128];//legal limit is 64, which will be checked at save.
		sprintf( buffer, "%s Pane %d", controlInstData->m_decoratedNameString.str() ,paneIndex );
		paneInstData->m_decoratedNameString = buffer;
		if( TheHierarchyView )
			TheHierarchyView->updateWindowName( tabData->subPanes[paneIndex] );
	}
}

// radioButtonPropertiesCallback ==============================================
/** Dialog callback for properties */
//=============================================================================
static LRESULT CALLBACK tabControlPropertiesCallback( HWND hWndDialog,
																											 UINT message,
																											 WPARAM wParam,
																											 LPARAM lParam )
{
	Int returnCode;

	//
	// handle any common messages between all property dialogs cause they
	// are designed to have controls doing the same functionality
	// and names
	//
	if( HandleCommonDialogMessages( hWndDialog, message,
																	wParam, lParam, &returnCode ) == TRUE )
		return returnCode;

	switch( message )
	{

		// ------------------------------------------------------------------------
    case WM_COMMAND:
    {
//			Int notifyCode = HIWORD( wParam );  // notification code
			Int controlID = LOWORD( wParam );  // control ID
//			HWND hWndControl = (HWND)lParam;  // control window handle

      switch( controlID )
      {

				// --------------------------------------------------------------------
        case IDOK:
				{
					GameWindow *tabControl = TheEditor->getPropertyTarget();

					// sanity
					if( tabControl )
					{
						// save the common properties
						if( SaveCommonDialogProperties( hWndDialog, tabControl ) == FALSE )
							break;

						ImageAndColorInfo *info;

						info = GetStateInfo( TC_TAB_0_ENABLED );
						GadgetTabControlSetEnabledImageTabZero( tabControl, info->image );
						GadgetTabControlSetEnabledColorTabZero( tabControl, info->color );
						GadgetTabControlSetEnabledBorderColorTabZero( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_1_ENABLED );
						GadgetTabControlSetEnabledImageTabOne( tabControl, info->image );
						GadgetTabControlSetEnabledColorTabOne( tabControl, info->color );
						GadgetTabControlSetEnabledBorderColorTabOne( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_2_ENABLED );
						GadgetTabControlSetEnabledImageTabTwo( tabControl, info->image );
						GadgetTabControlSetEnabledColorTabTwo( tabControl, info->color );
						GadgetTabControlSetEnabledBorderColorTabTwo( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_3_ENABLED );
						GadgetTabControlSetEnabledImageTabThree( tabControl, info->image );
						GadgetTabControlSetEnabledColorTabThree( tabControl, info->color );
						GadgetTabControlSetEnabledBorderColorTabThree( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_4_ENABLED );
						GadgetTabControlSetEnabledImageTabFour( tabControl, info->image );
						GadgetTabControlSetEnabledColorTabFour( tabControl, info->color );
						GadgetTabControlSetEnabledBorderColorTabFour( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_5_ENABLED );
						GadgetTabControlSetEnabledImageTabFive( tabControl, info->image );
						GadgetTabControlSetEnabledColorTabFive( tabControl, info->color );
						GadgetTabControlSetEnabledBorderColorTabFive( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_6_ENABLED );
						GadgetTabControlSetEnabledImageTabSix( tabControl, info->image );
						GadgetTabControlSetEnabledColorTabSix( tabControl, info->color );
						GadgetTabControlSetEnabledBorderColorTabSix( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_7_ENABLED );
						GadgetTabControlSetEnabledImageTabSeven( tabControl, info->image );
						GadgetTabControlSetEnabledColorTabSeven( tabControl, info->color );
						GadgetTabControlSetEnabledBorderColorTabSeven( tabControl, info->borderColor );

						info = GetStateInfo( TAB_CONTROL_ENABLED );
						GadgetTabControlSetEnabledImageBackground( tabControl, info->image );
						GadgetTabControlSetEnabledColorBackground( tabControl, info->color );
						GadgetTabControlSetEnabledBorderColorBackground( tabControl, info->borderColor );



						info = GetStateInfo( TC_TAB_0_DISABLED );
						GadgetTabControlSetDisabledImageTabZero( tabControl, info->image );
						GadgetTabControlSetDisabledColorTabZero( tabControl, info->color );
						GadgetTabControlSetDisabledBorderColorTabZero( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_1_DISABLED );
						GadgetTabControlSetDisabledImageTabOne( tabControl, info->image );
						GadgetTabControlSetDisabledColorTabOne( tabControl, info->color );
						GadgetTabControlSetDisabledBorderColorTabOne( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_2_DISABLED );
						GadgetTabControlSetDisabledImageTabTwo( tabControl, info->image );
						GadgetTabControlSetDisabledColorTabTwo( tabControl, info->color );
						GadgetTabControlSetDisabledBorderColorTabTwo( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_3_DISABLED );
						GadgetTabControlSetDisabledImageTabThree( tabControl, info->image );
						GadgetTabControlSetDisabledColorTabThree( tabControl, info->color );
						GadgetTabControlSetDisabledBorderColorTabThree( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_4_DISABLED );
						GadgetTabControlSetDisabledImageTabFour( tabControl, info->image );
						GadgetTabControlSetDisabledColorTabFour( tabControl, info->color );
						GadgetTabControlSetDisabledBorderColorTabFour( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_5_DISABLED );
						GadgetTabControlSetDisabledImageTabFive( tabControl, info->image );
						GadgetTabControlSetDisabledColorTabFive( tabControl, info->color );
						GadgetTabControlSetDisabledBorderColorTabFive( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_6_DISABLED );
						GadgetTabControlSetDisabledImageTabSix( tabControl, info->image );
						GadgetTabControlSetDisabledColorTabSix( tabControl, info->color );
						GadgetTabControlSetDisabledBorderColorTabSix( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_7_DISABLED );
						GadgetTabControlSetDisabledImageTabSeven( tabControl, info->image );
						GadgetTabControlSetDisabledColorTabSeven( tabControl, info->color );
						GadgetTabControlSetDisabledBorderColorTabSeven( tabControl, info->borderColor );

						info = GetStateInfo( TAB_CONTROL_DISABLED );
						GadgetTabControlSetDisabledImageBackground( tabControl, info->image );
						GadgetTabControlSetDisabledColorBackground( tabControl, info->color );
						GadgetTabControlSetDisabledBorderColorBackground( tabControl, info->borderColor );




						info = GetStateInfo( TC_TAB_0_HILITE );
						GadgetTabControlSetHiliteImageTabZero( tabControl, info->image );
						GadgetTabControlSetHiliteColorTabZero( tabControl, info->color );
						GadgetTabControlSetHiliteBorderColorTabZero( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_1_HILITE );
						GadgetTabControlSetHiliteImageTabOne( tabControl, info->image );
						GadgetTabControlSetHiliteColorTabOne( tabControl, info->color );
						GadgetTabControlSetHiliteBorderColorTabOne( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_2_HILITE );
						GadgetTabControlSetHiliteImageTabTwo( tabControl, info->image );
						GadgetTabControlSetHiliteColorTabTwo( tabControl, info->color );
						GadgetTabControlSetHiliteBorderColorTabTwo( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_3_HILITE );
						GadgetTabControlSetHiliteImageTabThree( tabControl, info->image );
						GadgetTabControlSetHiliteColorTabThree( tabControl, info->color );
						GadgetTabControlSetHiliteBorderColorTabThree( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_4_HILITE );
						GadgetTabControlSetHiliteImageTabFour( tabControl, info->image );
						GadgetTabControlSetHiliteColorTabFour( tabControl, info->color );
						GadgetTabControlSetHiliteBorderColorTabFour( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_5_HILITE );
						GadgetTabControlSetHiliteImageTabFive( tabControl, info->image );
						GadgetTabControlSetHiliteColorTabFive( tabControl, info->color );
						GadgetTabControlSetHiliteBorderColorTabFive( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_6_HILITE );
						GadgetTabControlSetHiliteImageTabSix( tabControl, info->image );
						GadgetTabControlSetHiliteColorTabSix( tabControl, info->color );
						GadgetTabControlSetHiliteBorderColorTabSix( tabControl, info->borderColor );

						info = GetStateInfo( TC_TAB_7_HILITE );
						GadgetTabControlSetHiliteImageTabSeven( tabControl, info->image );
						GadgetTabControlSetHiliteColorTabSeven( tabControl, info->color );
						GadgetTabControlSetHiliteBorderColorTabSeven( tabControl, info->borderColor );

						info = GetStateInfo( TAB_CONTROL_HILITE );
						GadgetTabControlSetHiliteImageBackground( tabControl, info->image );
						GadgetTabControlSetHiliteColorBackground( tabControl, info->color );
						GadgetTabControlSetHiliteBorderColorBackground( tabControl, info->borderColor );


						TabControlData *tabData = (TabControlData *)tabControl->winGetUserData();

						tabData->tabWidth = GetDlgItemInt( hWndDialog, TAB_WIDTH, NULL, FALSE );
						tabData->tabHeight = GetDlgItemInt(hWndDialog, TAB_HEIGHT, NULL, FALSE );
						tabData->tabCount = GetDlgItemInt(hWndDialog, TAB_COUNT, NULL, FALSE );
						tabData->paneBorder = GetDlgItemInt(hWndDialog, BORDER_WIDTH, NULL, FALSE );
						tabData->activeTab = GetDlgItemInt(hWndDialog, ACTIVE_TAB, NULL, FALSE );

						if( IsDlgButtonChecked( hWndDialog, DISABLE_TAB_0 ) )
							tabData->subPaneDisabled[0] = TRUE;
						else
							tabData->subPaneDisabled[0] = FALSE;
						if( IsDlgButtonChecked( hWndDialog, DISABLE_TAB_1 ) )
							tabData->subPaneDisabled[1] = TRUE;
						else
							tabData->subPaneDisabled[1] = FALSE;
						if( IsDlgButtonChecked( hWndDialog, DISABLE_TAB_2 ) )
							tabData->subPaneDisabled[2] = TRUE;
						else
							tabData->subPaneDisabled[2] = FALSE;
						if( IsDlgButtonChecked( hWndDialog, DISABLE_TAB_3 ) )
							tabData->subPaneDisabled[3] = TRUE;
						else
							tabData->subPaneDisabled[3] = FALSE;
						if( IsDlgButtonChecked( hWndDialog, DISABLE_TAB_4 ) )
							tabData->subPaneDisabled[4] = TRUE;
						else
							tabData->subPaneDisabled[4] = FALSE;
						if( IsDlgButtonChecked( hWndDialog, DISABLE_TAB_5 ) )
							tabData->subPaneDisabled[5] = TRUE;
						else
							tabData->subPaneDisabled[5] = FALSE;
						if( IsDlgButtonChecked( hWndDialog, DISABLE_TAB_6 ) )
							tabData->subPaneDisabled[6] = TRUE;
						else
							tabData->subPaneDisabled[6] = FALSE;
						if( IsDlgButtonChecked( hWndDialog, DISABLE_TAB_7 ) )
							tabData->subPaneDisabled[7] = TRUE;
						else
							tabData->subPaneDisabled[7] = FALSE;

						if( IsDlgButtonChecked( hWndDialog, LEFT_JUSTIFY) )
							tabData->tabOrientation = TP_TOPLEFT;
						else if( IsDlgButtonChecked( hWndDialog, CENTER_JUSTIFY ) )
							tabData->tabOrientation = TP_CENTER;
						else if( IsDlgButtonChecked( hWndDialog, RIGHT_JUSTIFY ) )
							tabData->tabOrientation = TP_BOTTOMRIGHT;

						if( IsDlgButtonChecked( hWndDialog, TOP_SIDE ) )
							tabData->tabEdge = TP_TOP_SIDE;
						else if( IsDlgButtonChecked( hWndDialog, RIGHT_SIDE ) )
							tabData->tabEdge = TP_RIGHT_SIDE;
						else if( IsDlgButtonChecked( hWndDialog, LEFT_SIDE ) )
							tabData->tabEdge = TP_LEFT_SIDE;
						else if( IsDlgButtonChecked( hWndDialog, BOTTOM_SIDE ) )
							tabData->tabEdge = TP_BOTTOM_SIDE;

						//safeties
						tabData->tabCount = max( tabData->tabCount, 1 );
						tabData->tabCount = min( tabData->tabCount, NUM_TAB_PANES );

						GadgetTabControlComputeTabRegion( tabControl );
						GadgetTabControlResizeSubPanes( tabControl );
						GadgetTabControlShowSubPane( tabControl, tabData->activeTab );
						GadgetTabControlUpdatePaneNames( tabControl );

					}

          DestroyWindow( hWndDialog );
          break;

				}

				// --------------------------------------------------------------------
        case IDCANCEL:
				{

          DestroyWindow( hWndDialog );
          break;

				}

      }

      return 0;

    }

		// ------------------------------------------------------------------------
    case WM_CLOSE:
		{

      DestroyWindow( hWndDialog );
      return 0;

		}

		// ------------------------------------------------------------------------
		default:
			return 0;

  }

}


///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// InitTabControlPropertiesDialog ============================================
/** Bring up the tab control properties dialog */
//=============================================================================
HWND InitTabControlPropertiesDialog( GameWindow *tabControl )
{
	HWND dialog;

	// create the dialog box
	dialog = CreateDialog( TheEditor->getInstance(),
												 (LPCTSTR)TAB_CONTROL_PROPERTIES_DIALOG,
												 TheEditor->getWindowHandle(),
												 (DLGPROC)tabControlPropertiesCallback );
	if( dialog == NULL )
		return NULL;

	// do the common initialization
	CommonDialogInitialize( tabControl, dialog );

	//
	// store in the image and color table the values for this button
	//

	const Image *image;
	Color color, borderColor;

	image = GadgetTabControlGetEnabledImageTabZero( tabControl );
	color = GadgetTabControlGetEnabledColorTabZero( tabControl );
	borderColor = GadgetTabControlGetEnabledBorderColorTabZero( tabControl );
	StoreImageAndColor( TC_TAB_0_ENABLED, image, color, borderColor );

	image = GadgetTabControlGetEnabledImageTabOne( tabControl );
	color = GadgetTabControlGetEnabledColorTabOne( tabControl );
	borderColor = GadgetTabControlGetEnabledBorderColorTabOne( tabControl );
	StoreImageAndColor( TC_TAB_1_ENABLED, image, color, borderColor );

	image = GadgetTabControlGetEnabledImageTabTwo( tabControl );
	color = GadgetTabControlGetEnabledColorTabTwo( tabControl );
	borderColor = GadgetTabControlGetEnabledBorderColorTabTwo( tabControl );
	StoreImageAndColor( TC_TAB_2_ENABLED, image, color, borderColor );

	image = GadgetTabControlGetEnabledImageTabThree( tabControl );
	color = GadgetTabControlGetEnabledColorTabThree( tabControl );
	borderColor = GadgetTabControlGetEnabledBorderColorTabThree( tabControl );
	StoreImageAndColor( TC_TAB_3_ENABLED, image, color, borderColor );

	image = GadgetTabControlGetEnabledImageTabFour( tabControl );
	color = GadgetTabControlGetEnabledColorTabFour( tabControl );
	borderColor = GadgetTabControlGetEnabledBorderColorTabFour( tabControl );
	StoreImageAndColor( TC_TAB_4_ENABLED, image, color, borderColor );

	image = GadgetTabControlGetEnabledImageTabFive( tabControl );
	color = GadgetTabControlGetEnabledColorTabFive( tabControl );
	borderColor = GadgetTabControlGetEnabledBorderColorTabFive( tabControl );
	StoreImageAndColor( TC_TAB_5_ENABLED, image, color, borderColor );

	image = GadgetTabControlGetEnabledImageTabSix( tabControl );
	color = GadgetTabControlGetEnabledColorTabSix( tabControl );
	borderColor = GadgetTabControlGetEnabledBorderColorTabSix( tabControl );
	StoreImageAndColor( TC_TAB_6_ENABLED, image, color, borderColor );

	image = GadgetTabControlGetEnabledImageTabSeven( tabControl );
	color = GadgetTabControlGetEnabledColorTabSeven( tabControl );
	borderColor = GadgetTabControlGetEnabledBorderColorTabSeven( tabControl );
	StoreImageAndColor( TC_TAB_7_ENABLED, image, color, borderColor );

	image = GadgetTabControlGetEnabledImageBackground( tabControl );
	color = GadgetTabControlGetEnabledColorBackground( tabControl );
	borderColor = GadgetTabControlGetEnabledBorderColorBackground( tabControl );
	StoreImageAndColor( TAB_CONTROL_ENABLED, image, color, borderColor );



	image = GadgetTabControlGetDisabledImageTabZero( tabControl );
	color = GadgetTabControlGetDisabledColorTabZero( tabControl );
	borderColor = GadgetTabControlGetDisabledBorderColorTabZero( tabControl );
	StoreImageAndColor( TC_TAB_0_DISABLED, image, color, borderColor );

	image = GadgetTabControlGetDisabledImageTabOne( tabControl );
	color = GadgetTabControlGetDisabledColorTabOne( tabControl );
	borderColor = GadgetTabControlGetDisabledBorderColorTabOne( tabControl );
	StoreImageAndColor( TC_TAB_1_DISABLED, image, color, borderColor );

	image = GadgetTabControlGetDisabledImageTabTwo( tabControl );
	color = GadgetTabControlGetDisabledColorTabTwo( tabControl );
	borderColor = GadgetTabControlGetDisabledBorderColorTabTwo( tabControl );
	StoreImageAndColor( TC_TAB_2_DISABLED, image, color, borderColor );

	image = GadgetTabControlGetDisabledImageTabThree( tabControl );
	color = GadgetTabControlGetDisabledColorTabThree( tabControl );
	borderColor = GadgetTabControlGetDisabledBorderColorTabThree( tabControl );
	StoreImageAndColor( TC_TAB_3_DISABLED, image, color, borderColor );

	image = GadgetTabControlGetDisabledImageTabFour( tabControl );
	color = GadgetTabControlGetDisabledColorTabFour( tabControl );
	borderColor = GadgetTabControlGetDisabledBorderColorTabFour( tabControl );
	StoreImageAndColor( TC_TAB_4_DISABLED, image, color, borderColor );

	image = GadgetTabControlGetDisabledImageTabFive( tabControl );
	color = GadgetTabControlGetDisabledColorTabFive( tabControl );
	borderColor = GadgetTabControlGetDisabledBorderColorTabFive( tabControl );
	StoreImageAndColor( TC_TAB_5_DISABLED, image, color, borderColor );

	image = GadgetTabControlGetDisabledImageTabSix( tabControl );
	color = GadgetTabControlGetDisabledColorTabSix( tabControl );
	borderColor = GadgetTabControlGetDisabledBorderColorTabSix( tabControl );
	StoreImageAndColor( TC_TAB_6_DISABLED, image, color, borderColor );

	image = GadgetTabControlGetDisabledImageTabSeven( tabControl );
	color = GadgetTabControlGetDisabledColorTabSeven( tabControl );
	borderColor = GadgetTabControlGetDisabledBorderColorTabSeven( tabControl );
	StoreImageAndColor( TC_TAB_7_DISABLED, image, color, borderColor );

	image = GadgetTabControlGetDisabledImageBackground( tabControl );
	color = GadgetTabControlGetDisabledColorBackground( tabControl );
	borderColor = GadgetTabControlGetDisabledBorderColorBackground( tabControl );
	StoreImageAndColor( TAB_CONTROL_DISABLED, image, color, borderColor );



	image = GadgetTabControlGetHiliteImageTabZero( tabControl );
	color = GadgetTabControlGetHiliteColorTabZero( tabControl );
	borderColor = GadgetTabControlGetHiliteBorderColorTabZero( tabControl );
	StoreImageAndColor( TC_TAB_0_HILITE, image, color, borderColor );

	image = GadgetTabControlGetHiliteImageTabOne( tabControl );
	color = GadgetTabControlGetHiliteColorTabOne( tabControl );
	borderColor = GadgetTabControlGetHiliteBorderColorTabOne( tabControl );
	StoreImageAndColor( TC_TAB_1_HILITE, image, color, borderColor );

	image = GadgetTabControlGetHiliteImageTabTwo( tabControl );
	color = GadgetTabControlGetHiliteColorTabTwo( tabControl );
	borderColor = GadgetTabControlGetHiliteBorderColorTabTwo( tabControl );
	StoreImageAndColor( TC_TAB_2_HILITE, image, color, borderColor );

	image = GadgetTabControlGetHiliteImageTabThree( tabControl );
	color = GadgetTabControlGetHiliteColorTabThree( tabControl );
	borderColor = GadgetTabControlGetHiliteBorderColorTabThree( tabControl );
	StoreImageAndColor( TC_TAB_3_HILITE, image, color, borderColor );

	image = GadgetTabControlGetHiliteImageTabFour( tabControl );
	color = GadgetTabControlGetHiliteColorTabFour( tabControl );
	borderColor = GadgetTabControlGetHiliteBorderColorTabFour( tabControl );
	StoreImageAndColor( TC_TAB_4_HILITE, image, color, borderColor );

	image = GadgetTabControlGetHiliteImageTabFive( tabControl );
	color = GadgetTabControlGetHiliteColorTabFive( tabControl );
	borderColor = GadgetTabControlGetHiliteBorderColorTabFive( tabControl );
	StoreImageAndColor( TC_TAB_5_HILITE, image, color, borderColor );

	image = GadgetTabControlGetHiliteImageTabSix( tabControl );
	color = GadgetTabControlGetHiliteColorTabSix( tabControl );
	borderColor = GadgetTabControlGetHiliteBorderColorTabSix( tabControl );
	StoreImageAndColor( TC_TAB_6_HILITE, image, color, borderColor );

	image = GadgetTabControlGetHiliteImageTabSeven( tabControl );
	color = GadgetTabControlGetHiliteColorTabSeven( tabControl );
	borderColor = GadgetTabControlGetHiliteBorderColorTabSeven( tabControl );
	StoreImageAndColor( TC_TAB_7_HILITE, image, color, borderColor );

	image = GadgetTabControlGetHiliteImageBackground( tabControl );
	color = GadgetTabControlGetHiliteColorBackground( tabControl );
	borderColor = GadgetTabControlGetHiliteBorderColorBackground( tabControl );
	StoreImageAndColor( TAB_CONTROL_HILITE, image, color, borderColor );


	// tab data
	TabControlData *tabData = (TabControlData *)tabControl->winGetUserData();

	//
	// initialize the dialog with values from the window
	//

	SetDlgItemInt(dialog, TAB_WIDTH,		tabData->tabWidth, FALSE);
	SetDlgItemInt(dialog, TAB_HEIGHT,		tabData->tabHeight, FALSE);
	SetDlgItemInt(dialog, TAB_COUNT,		tabData->tabCount, FALSE);
	SetDlgItemInt(dialog, BORDER_WIDTH, tabData->paneBorder, FALSE);
	SetDlgItemInt(dialog, ACTIVE_TAB,		tabData->activeTab, FALSE);

	if( tabData->subPaneDisabled[0] )
		CheckDlgButton( dialog, DISABLE_TAB_0, BST_CHECKED );
	else
		CheckDlgButton( dialog, DISABLE_TAB_0, BST_UNCHECKED );
	if( tabData->subPaneDisabled[1] )
		CheckDlgButton( dialog, DISABLE_TAB_1, BST_CHECKED );
	else
		CheckDlgButton( dialog, DISABLE_TAB_1, BST_UNCHECKED );
	if( tabData->subPaneDisabled[2] )
		CheckDlgButton( dialog, DISABLE_TAB_2, BST_CHECKED );
	else
		CheckDlgButton( dialog, DISABLE_TAB_2, BST_UNCHECKED );
	if( tabData->subPaneDisabled[3] )
		CheckDlgButton( dialog, DISABLE_TAB_3, BST_CHECKED );
	else
		CheckDlgButton( dialog, DISABLE_TAB_3, BST_UNCHECKED );
	if( tabData->subPaneDisabled[4] )
		CheckDlgButton( dialog, DISABLE_TAB_4, BST_CHECKED );
	else
		CheckDlgButton( dialog, DISABLE_TAB_4, BST_UNCHECKED );
	if( tabData->subPaneDisabled[5] )
		CheckDlgButton( dialog, DISABLE_TAB_5, BST_CHECKED );
	else
		CheckDlgButton( dialog, DISABLE_TAB_5, BST_UNCHECKED );
	if( tabData->subPaneDisabled[6] )
		CheckDlgButton( dialog, DISABLE_TAB_6, BST_CHECKED );
	else
		CheckDlgButton( dialog, DISABLE_TAB_6, BST_UNCHECKED );
	if( tabData->subPaneDisabled[7] )
		CheckDlgButton( dialog, DISABLE_TAB_7, BST_CHECKED );
	else
		CheckDlgButton( dialog, DISABLE_TAB_7, BST_UNCHECKED );

	if( tabData->tabOrientation == TP_TOPLEFT )
		CheckDlgButton( dialog, LEFT_JUSTIFY, BST_CHECKED );
	else if( tabData->tabOrientation == TP_CENTER )
		CheckDlgButton( dialog, CENTER_JUSTIFY, BST_CHECKED );
	else if( tabData->tabOrientation == TP_BOTTOMRIGHT )
		CheckDlgButton( dialog, RIGHT_JUSTIFY, BST_CHECKED );

	if( tabData->tabEdge == TP_TOP_SIDE )
		CheckDlgButton( dialog, TOP_SIDE, BST_CHECKED );
	else if( tabData->tabEdge == TP_RIGHT_SIDE )
		CheckDlgButton( dialog, RIGHT_SIDE, BST_CHECKED );
	else if( tabData->tabEdge == TP_LEFT_SIDE )
		CheckDlgButton( dialog, LEFT_SIDE, BST_CHECKED );
	else if( tabData->tabEdge == TP_BOTTOM_SIDE )
		CheckDlgButton( dialog, BOTTOM_SIDE, BST_CHECKED );

	return dialog;

}



