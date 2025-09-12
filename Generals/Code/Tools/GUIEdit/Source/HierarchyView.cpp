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

// FILE: HierarchyView.cpp ////////////////////////////////////////////////////
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
// File name:  HierarchyView.cpp
//
// Created:    Colin Day, July 2001
//
// Desc:			 Manipulation the widows heirarchy through the tree
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <windows.h>
#include <commctrl.h>
#include <string.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "GameClient/Gadget.h"
#include "resource.h"
#include "HierarchyView.h"
#include "WinMain.h"
#include "GUIEdit.h"
#include "EditWindow.h"
#include "GUIEditWindowManager.h"
#include "Properties.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static ICoord2D dialogPos;
static ICoord2D dialogSize;


// PUBLIC DATA ////////////////////////////////////////////////////////////////
HierarchyView *TheHierarchyView = NULL;  ///< the view singleton

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// HierarchyView::dialogProc ==================================================
/** Windows dialog procedure for the control palette */
//=============================================================================
LRESULT CALLBACK HierarchyView::dialogProc( HWND hWndDialog, UINT message,
																						WPARAM wParam, LPARAM lParam )
{

	switch( message )
	{

   		// ------------------------------------------------------------------------
 		case WM_MOVE:
 		{
// 			Int x = LOWORD( lParam );
// 			Int y = HIWORD( lParam );

 			// record our position
 			RECT rect;
 			POINT p;
 			GetWindowRect( hWndDialog, &rect );
 			p.x = rect.left;
 			p.y = rect.top;
 			ScreenToClient( TheEditor->getWindowHandle(), &p );
 			dialogPos.x = p.x;
 			dialogPos.y = p.y;

 			return 0;

 		}

		// ------------------------------------------------------------------------
		case WM_SIZE:
		{
//			Int sizeType = wParam;  // resizing flag
			Int width = LOWORD( lParam );  // width of client
			Int height = HIWORD( lParam );  // height of client

			// record the new height
			RECT rect;
			GetWindowRect( hWndDialog, &rect );
			dialogSize.x = rect.right - rect.left;
			dialogSize.y = rect.bottom - rect.top;

			// resizing the dialog will cause us to resize the tree view control
			if( TheHierarchyView )
			{
				HWND tree = TheHierarchyView->getTreeHandle();

				if( tree )
				{
					Int border = 10;
					RECT treeRect;
					POINT p;

					GetWindowRect( tree, &treeRect );
					p.x = treeRect.left;
					p.y = treeRect.top;
					ScreenToClient( hWndDialog, &p );
					MoveWindow( tree, border, p.y,
											width - (border * 2), (height - border) - p.y,
											TRUE );

				}

			}

			return 0;

		}

		// ------------------------------------------------------------------------
		case WM_MOUSEMOVE:
		{
			Int x = LOWORD( lParam );
			Int y = HIWORD( lParam );
			GameWindow *dragWindow = TheHierarchyView->getDragWindow();

			// dragging a window then hilite any tree item we're over
			if( dragWindow )
			{
				POINT treePoint;

				// translate the mouse coords to the coords of the tree
				treePoint.x = x;
				treePoint.y = y;
				ClientToScreen( hWndDialog, &treePoint );
				ScreenToClient( TheHierarchyView->getTreeHandle(), &treePoint );

				// get the tree item if any that we're over and select it if present
				HTREEITEM overItem = TheHierarchyView->treePointToItem( treePoint.x, treePoint.y );
				if( overItem )
				{
					GameWindow *target = TheHierarchyView->getWindowFromItem( overItem );

					//
					// set this window as the drag target, we use it to draw visual
					// feeback in the edit window
					//
					TheHierarchyView->setDragTarget( target );

					// select the item in the tree
					TreeView_SelectItem( TheHierarchyView->getTreeHandle(), overItem );

					//
					// if this operation is legal set the cross cursor, otherwise
					// use the "no" cursor
					//
					if( TheHierarchyView->validateDragDropOperation( dragWindow, target ) )
						SetCursor( LoadCursor( NULL, IDC_CROSS ) );
					else
						SetCursor( LoadCursor( NULL, IDC_NO ) );

				}

			}

			return 0;

		}

		// ------------------------------------------------------------------------
		case WM_LBUTTONUP:
		{
			Int x = LOWORD( lParam );
			Int y = HIWORD( lParam );
			GameWindow *dragWindow = TheHierarchyView->getDragWindow();
			Bool clearDragWindow = TRUE;

			if( dragWindow )
			{
				POINT treePoint;

				// translate the mouse coords to the coords of the tree
				treePoint.x = x;
				treePoint.y = y;
				ClientToScreen( hWndDialog, &treePoint );
				ScreenToClient( TheHierarchyView->getTreeHandle(), &treePoint );

				// get the tree item if any that we're over
				HTREEITEM overItem = TheHierarchyView->treePointToItem( treePoint.x, treePoint.y );
				if( overItem )
				{
					GameWindow *overWindow;
					TVITEM overItemInfo;

					// get the node info from the tree item we're over
					overItemInfo.hItem = overItem;
					overItemInfo.lParam = NULL;
					overItemInfo.mask = TVIF_HANDLE | TVIF_PARAM;
					TreeView_GetItem( TheHierarchyView->getTreeHandle(), &overItemInfo );
					overWindow = (GameWindow *)overItemInfo.lParam;

					// this should always be true!
					assert( overWindow );

					// do the drag drop if allowed
					if( TheHierarchyView->validateDragDropOperation( dragWindow,
																													 overWindow ) )
					{

						//
						// if our target window is NOT a gadget we have the option
						// of moving the drag window to this position OR making the
						// drag window a child of the target overWindow.  If that is
						// the case we need a little popup menu to decide
						//
						if( TheEditor->windowIsGadget( overWindow ) == FALSE )
						{

							//
							// bring up the popup menu, note that we must translate the
							// local mouse pos to the screen
							//
							HMENU menu, subMenu;
							POINT screen;

							menu = LoadMenu( TheEditor->getInstance(), (LPCTSTR)HIERARCHY_DRAG_DROP_MENU );
							subMenu = GetSubMenu( menu, 0 );
							screen.x = x;
							screen.y = y;
							ClientToScreen( hWndDialog, &screen );
							TrackPopupMenuEx( subMenu, 0, screen.x, screen.y, hWndDialog, NULL );

							//
							// do not reset the drag window, and set the target window as
							// the popup target know which window to target after they
							// select from the popup menu
							//
							clearDragWindow = FALSE;
							TheHierarchyView->setPopupTarget( overWindow );

						}
						else
						{

							// our only option is to move the window here
							TheGUIEditWindowManager->moveAheadOf( dragWindow, overWindow );

						}

						// we've made a change now
						TheEditor->setUnsaved( TRUE );

					}

				}

				// window has been dragged and operation complete
				if( clearDragWindow )
				{

					TheHierarchyView->setDragWindow( NULL );
					TheHierarchyView->setDragTarget( NULL );

				}

				// release window capture
				ReleaseCapture();

				// set the cursor back to normal
				SetCursor( LoadCursor( NULL, IDC_ARROW ) );

			}

			return 0;

		}

		// ------------------------------------------------------------------------
		case WM_RBUTTONUP:
		{
			Int x = LOWORD( lParam );
			Int y = HIWORD( lParam );
			POINT treePoint;

			// translate the mouse coords to the coords of the tree
			treePoint.x = x;
			treePoint.y = y;
			ClientToScreen( hWndDialog, &treePoint );
			ScreenToClient( TheHierarchyView->getTreeHandle(), &treePoint );

			// get the tree item if any that we're over
			HTREEITEM overItem = TheHierarchyView->treePointToItem( treePoint.x, treePoint.y );
			if( overItem )
			{
				GameWindow *overWindow;

				// get the game window from the tree item
				overWindow = TheHierarchyView->getWindowFromItem( overItem );

				// unselect all windows in the editor
				TheEditor->clearSelections();

				// select this one window
				TheEditor->selectWindow( overWindow );

				// set this window as the popup window target
				TheHierarchyView->setPopupTarget( overWindow );

				//
				// bring up the popup menu, note that we must translate the
				// local mouse pos to the screen
				//
				HMENU menu, subMenu;
				POINT screen;

				menu = LoadMenu( TheEditor->getInstance(), (LPCTSTR)HIERARCHY_POPUP_MENU );
				subMenu = GetSubMenu( menu, 0 );
				screen.x = x;
				screen.y = y;
				ClientToScreen( hWndDialog, &screen );
				TrackPopupMenuEx( subMenu, 0, screen.x, screen.y, hWndDialog, NULL );

			}

			return 0;

		}

		// ------------------------------------------------------------------------
		case WM_NOTIFY:
		{
			UnsignedInt controlID = (Int)wParam;
			LPNMHDR notifyMessageHandler = (LPNMHDR)lParam;

			// switch on control
			switch( controlID )
			{

				// --------------------------------------------------------------------
				case TREE_HIERARCHY:
				{

					// switch on notify code for the tree
					switch( notifyMessageHandler->code )
					{

						// ----------------------------------------------------------------
						case TVN_SELCHANGED:
						{
							LPNMTREEVIEW treeNotify = (LPNMTREEVIEW)lParam;

							if( treeNotify->action != TVC_UNKNOWN )
							{
								TVITEM newItem;

								// get the new item selected
								newItem = treeNotify->itemNew;

								// get the window data pointers
								GameWindow *window = (GameWindow *)newItem.lParam;

								// unselect everything else and select the new window
								TheEditor->clearSelections();
								if( window )
									TheEditor->selectWindow( window );

							}

							break;

						}

						// ----------------------------------------------------------------
						case NM_DBLCLK:
						{

							// get the selected tree item
							HTREEITEM selected = TreeView_GetSelection( TheHierarchyView->getTreeHandle() );
							if( selected )
							{
								GameWindow *overWindow;

								// get the game window from the tree item
								overWindow = TheHierarchyView->getWindowFromItem( selected );

								// unselect all windows in the editor
								TheEditor->clearSelections();

								// select this one window
								TheEditor->selectWindow( overWindow );

								// set this window as the popup window target
								TheHierarchyView->setPopupTarget( overWindow );

								//
								// bring up the popup menu, note that we must translate the
								// local mouse pos to the screen
								//
								HMENU menu, subMenu;
								POINT screen;

								menu = LoadMenu( TheEditor->getInstance(), (LPCTSTR)HIERARCHY_POPUP_MENU );
								subMenu = GetSubMenu( menu, 0 );
								GetCursorPos( &screen );
								TrackPopupMenuEx( subMenu, 0, screen.x, screen.y, hWndDialog, NULL );

							}

							break;

						}

						// ----------------------------------------------------------------
						case TVN_BEGINDRAG:
						{
							LPNMTREEVIEW treeNotify = (LPNMTREEVIEW)lParam;
							TVITEM newItem;

							// get the item being dragged
							newItem = treeNotify->itemNew;

							// save the window being dragged
							TheHierarchyView->setDragWindow( (GameWindow *)newItem.lParam );
							TheHierarchyView->setDragTarget( NULL );

							// capture the mouse
							SetCapture( TheHierarchyView->getHierarchyHandle() );

							break;

						}

					}

				}

			}

			return 0;

		}

		// ------------------------------------------------------------------------
    case WM_COMMAND:
    {
//			Int notifyCode = HIWORD( wParam );
			Int controlID = LOWORD( wParam );
//			HWND hWndControl = (HWND)lParam;

      switch( controlID )
      {

				// --------------------------------------------------------------------
				case MENU_HIERARCHY_MOVE_HERE:
				{
					GameWindow *drag = TheHierarchyView->getDragWindow();
					GameWindow *target = TheHierarchyView->getPopupTarget();

					// do the logic if after a sanity check on the targets
					if( TheHierarchyView->validateDragDropOperation( drag, target ) )
						TheGUIEditWindowManager->moveAheadOf( drag, target );

					// we're done with the drag and popup ops now
					TheHierarchyView->setDragWindow( NULL );
					TheHierarchyView->setDragTarget( NULL );
					TheHierarchyView->setPopupTarget( NULL );

					break;

				}

				// --------------------------------------------------------------------
				case HIERARCHY_MAKE_CHILD_HERE:
				{
					GameWindow *drag = TheHierarchyView->getDragWindow();
					GameWindow *target = TheHierarchyView->getPopupTarget();

					// do the logic if after a sanity check on the targets
					if( TheHierarchyView->validateDragDropOperation( drag, target ) )
						TheGUIEditWindowManager->makeChildOf( drag, target );

					// we're done with the drag and popup ops now
					TheHierarchyView->setDragWindow( NULL );
					TheHierarchyView->setDragTarget( NULL );
					TheHierarchyView->setPopupTarget( NULL );

					break;

				}

				// --------------------------------------------------------------------
				case HIERARCHY_POPUP_MOVE:
				{
					GameWindow *target = TheHierarchyView->getPopupTarget();

					// sanity
					if( target == NULL )
						break;

					//
					// just to be safe, unselect all other windows, select this
					// one, and go into move mode
					//
					TheEditor->clearSelections();
					TheEditor->selectWindow( target );
					TheEditor->setMode( MODE_DRAG_MOVE );

					// set the locatoin of the move to the window position for now
					ICoord2D pos;
					target->winGetScreenPosition( &pos.x, &pos.y );
					TheEditWindow->setDragMoveDest( &pos );
					TheEditWindow->setDragMoveOrigin( &pos );

					break;

				}

				// --------------------------------------------------------------------
				case HIERARCHY_POPUP_DELETE:
				{
					GameWindow *target = TheHierarchyView->getPopupTarget();

					if( target )
						TheEditor->deleteWindow( target );

					break;

				}

				// --------------------------------------------------------------------
				case HIERARCHY_POPUP_PROPERTIES:
				{
					GameWindow *target = TheHierarchyView->getPopupTarget();

					if( target )
					{
						POINT p;

						GetCursorPos( &p );
						ScreenToClient( TheEditWindow->getWindowHandle(), &p );
						InitPropertiesDialog( target, p.x, p.y );

					}

					break;

				}

				// --------------------------------------------------------------------
        case IDOK:
          break;

				// --------------------------------------------------------------------
        case IDCANCEL:
          break;

      }

      return 0;

    }

		// ------------------------------------------------------------------------
		default:
			return 0;

  }

}

// HierarchyView::findItemEntry ===============================================
/** Workhorse to find the tree item anywhere in the tree with the
	* matching data pointer of this game window */
//=============================================================================
HTREEITEM HierarchyView::findItemEntry( HTREEITEM node, GameWindow *window )
{

	// end of recursion
	if( node == NULL || window == NULL )
		return NULL;

	// is it in this node
	TVITEM item;
	item.hItem = node;
	item.lParam = NULL;
	item.mask = TVIF_HANDLE | TVIF_PARAM;
	TreeView_GetItem( m_tree, &item );
	if( (GameWindow *)item.lParam == window )
		return node;

	// not there, check our children
	HTREEITEM child;
	HTREEITEM found = NULL;
	for( child = TreeView_GetNextItem( m_tree, node, TVGN_CHILD );
			 child;
			 child = TreeView_GetNextItem( m_tree, child, TVGN_NEXT ) )
	{

		found = findItemEntry( child, window );
		if( found )
			return found;

	}

	// not there, check the siblings
	return findItemEntry( TreeView_GetNextItem( m_tree, node, TVGN_NEXT ),
												window );

}

// HierarchyView::findTreeEntry ===============================================
/** Find the game window entry in the hierarchy tree, if found the
	* item in the tree will be returned containing the window */
//=============================================================================
HTREEITEM HierarchyView::findTreeEntry( GameWindow *window )
{

	// no-op
	if( window == NULL )
		return NULL;

	// get root and search from there
	return findItemEntry( TreeView_GetRoot( m_tree ), window );

}

// HierarchyView::addWindowToTree =============================================
/** Add a single window to the hierarchy tree */
//=============================================================================
void HierarchyView::addWindowToTree( GameWindow *window,
																		 HTREEITEM treeParent,
																		 HierarchyOption option,
																		 Bool addChildren,
																		 Bool addSiblings )
{
	HTREEITEM newItem = NULL;

	// end of recursion
	if( window == NULL )
		return;

	// add only if not in tree already
	newItem = findTreeEntry( window );
	if( newItem == NULL )
	{

		// setup insert struct
		TVINSERTSTRUCT insert;
		insert.hParent = treeParent;
		if( option == HIERARCHY_ADD_AT_TOP )
			insert.hInsertAfter = TVI_FIRST;
		else
			insert.hInsertAfter = TVI_LAST;
		insert.itemex.mask = TVIF_TEXT | TVIF_PARAM;
		insert.itemex.lParam = (LPARAM)window;  // attach window to this item in lParam

		//
		// if the window has a name use it in the tree view, otherwise use a
		// name based on the type of the window
		//
		insert.itemex.pszText = getWindowTreeName( window );

		// add the item
		newItem = TreeView_InsertItem( m_tree, &insert );

		// sanity
		if( newItem == NULL )
		{

			DEBUG_LOG(( "Error adding window to tree" ));
			assert( 0 );
			return;

		}

	}

	//
	// add children if requested, but not on gadgets no matter what becuase
	// they are "atomic units", except for tab controls.
	//
	if( addChildren && TheEditor->windowIsGadget( window ) == FALSE  ||  (window->winGetStyle() & GWS_TAB_CONTROL) )
	{
		GameWindow *child;

		for( child = window->winGetChild(); child; child = child->winGetNext() )
			addWindowToTree( child, newItem, HIERARCHY_ADD_AT_BOTTOM, TRUE, TRUE );

	}

	// add siblings if requested
	if( addSiblings )
		addWindowToTree( window->winGetNext(), treeParent, option,
										 addChildren, addSiblings );

}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// HierarchyView::HierarchyView ===============================================
/** */
//=============================================================================
HierarchyView::HierarchyView( void )
{

	m_dialog = NULL;
	m_tree = NULL;
	dialogPos.x = dialogPos.y = 0;
	dialogSize.x = dialogSize.y = 0;
	m_dragWindow = NULL;
	m_dragTarget = NULL;
	m_popupTarget = NULL;

}

// HierarchyView::~HierarchyView ==============================================
/** */
//=============================================================================
HierarchyView::~HierarchyView( void )
{

	// call the shutdown
	shutdown();

}

// HierarchyView::init =========================================================
/** Create the control palette */
//=============================================================================
void HierarchyView::init( void )
{
	RECT dialogRect;
	RECT appRect;

	// create the modless dialog box
	m_dialog = CreateDialog( TheEditor->getInstance(),
													 (LPCSTR)HIERARCHY_DIALOG,
													 TheEditor->getWindowHandle(),
													 (DLGPROC)dialogProc );

	// display the dialog
	ShowWindow( m_dialog, SW_SHOW );

	// get the size of the control palette
	GetWindowRect( m_dialog, &dialogRect );
	dialogSize.x = dialogRect.right - dialogRect.left;
	dialogSize.y = dialogRect.bottom - dialogRect.top;

	POINT p;
	p.x = dialogRect.left;
	p.y = dialogRect.top;
	ScreenToClient( TheEditor->getWindowHandle(), &p );
	dialogPos.x = p.x;
	dialogPos.y = p.y;

	// get the rect of the app window
	GetClientRect( TheEditor->getWindowHandle(), &appRect );

	// place control palette at the top right of client area in edit window
	MoveWindow( m_dialog,
							appRect.right - dialogSize.x, 0,
							dialogSize.x, dialogSize.y,
							TRUE );

	// keep a handle to the tree
	m_tree = GetDlgItem( m_dialog, TREE_HIERARCHY );

}

// HierarchyView::reset =======================================================
/** Reset everything about our hierarchy view */
//=============================================================================
void HierarchyView::reset( void )
{

	// reset the tree control
	HTREEITEM parentItem = TreeView_GetRoot( m_tree );
	SendMessage( m_tree, TVM_EXPAND, TVE_COLLAPSERESET, (LPARAM)parentItem );

}

// HierarchyView::shutdown ====================================================
/** Destroy the control palette and all data associated with it */
//=============================================================================
void HierarchyView::shutdown( void )
{

	// destroy the control palette window
	DestroyWindow( m_dialog );
	m_dialog = NULL;
	m_tree = NULL;

}

// HierarchyView::getWindowTreeName ===========================================
/** Given a window, return a string representation for that window in
	* the hierarchy view */
//=============================================================================
char *HierarchyView::getWindowTreeName( GameWindow *window )
{
	static char buffer[ 256 ];

	// init buffer to return to nothing
	strcpy( buffer, "" );

	// sanity
	if( window == NULL )
		return buffer;

	// no name available, construct one based on type
	Int style = window->winGetStyle();
	if( BitIsSet( style, GWS_PUSH_BUTTON ) )
		strcpy( buffer, "Button" );
	else if( BitIsSet( style, GWS_RADIO_BUTTON ) )
		strcpy( buffer, "Radio Button" );
	else if( BitIsSet( style, GWS_TAB_CONTROL ) )
		strcpy( buffer, "Tab Control" );
	else if( BitIsSet( style, GWS_CHECK_BOX ) )
		strcpy( buffer, "Check Box" );
	else if( BitIsSet( style, GWS_HORZ_SLIDER ) )
		strcpy( buffer, "Horizontal Slider" );
	else if( BitIsSet( style, GWS_VERT_SLIDER ) )
		strcpy( buffer, "Vertical Slider" );
	else if( BitIsSet( style, GWS_STATIC_TEXT ) )
		strcpy( buffer, "Static Text" );
	else if( BitIsSet( style, GWS_ENTRY_FIELD ) )
		strcpy( buffer, "Text Entry" );
	else if( BitIsSet( style, GWS_SCROLL_LISTBOX ) )
		strcpy( buffer, "Listbox" );
	else if( BitIsSet( style, GWS_PROGRESS_BAR ) )
		strcpy( buffer, "Progress Bar" );
	else if( BitIsSet( style, GWS_USER_WINDOW ) )
		strcpy( buffer, "User Window" );
	else if( BitIsSet( style, GWS_TAB_PANE ) )
		strcpy( buffer, "Tab Pane" );
	else if( BitIsSet( style, GWS_COMBO_BOX ) )
		strcpy( buffer, "Combo Box" );
	else
		strcpy( buffer, "Undefined" );

	// if window has a name, concatenate it on the end
	WinInstanceData *instData = window->winGetInstanceData();
	if( !instData->m_decoratedNameString.isEmpty() )
	{

		strcat( buffer, ": " );
		strcat( buffer, instData->m_decoratedNameString.str() );

	}

	return buffer;

}

// HierarchyView::addWindow ===================================================
/** Add a window to the hierarchy view AND any of it's children */
//=============================================================================
void HierarchyView::addWindow( GameWindow *window, HierarchyOption option )
{

	// sanity
	if( window == NULL || m_dialog == NULL )
		return;

	// do not add again if already in the tree
	if( findTreeEntry( window ) != NULL )
		return;

	// get the parent tree entry to this window, NULL if no parent
	GameWindow *parent = window->winGetParent();
	HTREEITEM parentItem = findTreeEntry( parent );

	// add this window
	addWindowToTree( window, parentItem, option, TRUE, FALSE );

	//
	// force the tree control to redraw, it seems to have problems updating
	// the plus signs, lame ass Microsoft
	//
	InvalidateRect( m_tree, NULL, TRUE );

}

// HierarchyView::removeWindow ================================================
/** Remove the window from the hierarchy tree view */
//=============================================================================
void HierarchyView::removeWindow( GameWindow *window )
{
	HTREEITEM item;

	// sanity
	if( window == NULL )
		return;

	// if this window is the drag window clean that mode up
	if( window == m_dragWindow )
		m_dragWindow = NULL;

	// clean up drag target
	if( window == m_dragTarget )
		m_dragTarget = NULL;

	// if this window is the popup target remove it
	if( window == m_popupTarget )
		m_popupTarget = NULL;

	// find this entry in the tree
	item = findTreeEntry( window );

	// if not in tree nothing to do
	if( item == NULL )
		return;

	// remove it from the tree
	TreeView_DeleteItem( m_tree, item );

}

// HierarchyView::bringWindowToTop ============================================
/** Bring the window to the top of its parent list in the hierarchy
	* view, if no parent is present to the top of the hierarchy then */
//=============================================================================
void HierarchyView::bringWindowToTop( GameWindow *window )
{

	// sanity
	if( window == NULL )
		return;

	// find this window entry
	HTREEITEM item = findTreeEntry( window );
	if( item == NULL )
	{

		DEBUG_LOG(( "Cannot bring window to top, no entry in tree!" ));
		assert( 0 );
		return;

	}

	// remove the entry from the tree
	removeWindow( window );

	// find the parent tree entry
	HTREEITEM itemParent = findTreeEntry( window->winGetParent() );

	// add the window as a child of the parent entry at the top of it's list
	addWindowToTree( window, itemParent, HIERARCHY_ADD_AT_TOP, TRUE, FALSE );

}

// HierarchyView::updateWindowName ============================================
/** A window name may have been updated, reconstruct its hierarchy
	* tree entry cause we like to show names when we have them */
//=============================================================================
void HierarchyView::updateWindowName( GameWindow *window )
{

	// sanity
	if( window == NULL )
		return;

	// get the tree entry
	HTREEITEM item = findTreeEntry( window );
	if( item == NULL )
	{

		DEBUG_LOG(( "updateWindowName: No hierarchy entry for window!" ));
		assert( 0 );
		return;

	}

	// setup the item to modify in the tree
	TVITEM modify;
	modify.mask = TVIF_HANDLE | TVIF_TEXT;
	modify.hItem = item;
	modify.pszText = getWindowTreeName( window );

	// modify the item
	TreeView_SetItem( m_tree, &modify );

}

// HierarchyView::getDialogPos ================================================
/** Get the dialog position as recorded from the static */
//=============================================================================
void HierarchyView::getDialogPos( ICoord2D *pos )
{

	// sanity
	if( pos == NULL )
		return;

	*pos = dialogPos;

}

// HierarchyView::getDialogSize ===============================================
/** Get the dialog size as recorded from the static */
//=============================================================================
void HierarchyView::getDialogSize( ICoord2D *size )
{

	// sanity
	if( size == NULL )
		return;

	*size = dialogSize;

}

// HierarchyView::setDialogPos ================================================
/** */
//=============================================================================
void HierarchyView::setDialogPos( ICoord2D *pos )
{

	// sanity
	if( pos )
		dialogPos = *pos;

	MoveWindow( m_dialog, dialogPos.x, dialogPos.y,
							dialogSize.x, dialogSize.y, TRUE );

}

// HierarchyView::setDialogSize ===============================================
/** */
//=============================================================================
void HierarchyView::setDialogSize( ICoord2D *size )
{

	// sanity
	if( size )
		dialogSize = *size;

	MoveWindow( m_dialog, dialogPos.x, dialogPos.y,
							dialogSize.x, dialogSize.y, TRUE );

}

// HierarchyView::moveWindowAheadOf ===========================================
/** Move the window hierarchy representation to be just ahead of the
	* hierarchy entry of 'aheadOf' */
//=============================================================================
void HierarchyView::moveWindowAheadOf( GameWindow *window,
																			 GameWindow *aheadOf )
{

	// sanity
	if( window == NULL )
		return;

	// get the window hierarchy entry
	removeWindow( window );

	// we'll say and aheadOf of NULL means put at the top
	if( aheadOf == NULL )
	{

		addWindow( window, HIERARCHY_ADD_AT_TOP );
		return;

	}

	// get the hierarchy item of the aheadOf window
	HTREEITEM aheadOfItem = findTreeEntry( aheadOf );
	if( aheadOfItem == NULL )
	{

		DEBUG_LOG(( "moveWindowAheadOf: aheadOf has no hierarchy entry!" ));
		assert( 0 );
		return;

	}

	//
	// get the parent item we will be inserting the new entry at, a parent
	// of NULL is OK and will put it at the root of the tree
	//
	HTREEITEM parentItem = TreeView_GetNextItem( m_tree, aheadOfItem, TVGN_PARENT );

	//
	// get the item that we will be inserting after (just previous to
	// 'aheadOfItem' ... this can also be NULL for putting at the head
	//
	HTREEITEM prevItem = TreeView_GetNextItem( m_tree, aheadOfItem, TVGN_PREVIOUS );

	// setup insert struct
	TVINSERTSTRUCT insert;
	insert.itemex.mask = TVIF_TEXT | TVIF_PARAM;
	insert.hParent = parentItem;
	if( prevItem == NULL )
		insert.hInsertAfter = TVI_FIRST;
	else
		insert.hInsertAfter = prevItem;
	insert.itemex.lParam = (LPARAM)window;  // attach window to this item in lParam
	insert.itemex.pszText = getWindowTreeName( window );

	// add the item
	HTREEITEM newItem = TreeView_InsertItem( m_tree, &insert );

	// sanity
	if( newItem == NULL )
	{

		DEBUG_LOG(( "moveWindowAheadOf: Error adding window to tree" ));
		assert( 0 );
		return;

	}

	//
	// add ALL the children of this window as well, do not worry about
	// gadget children
	//
	if( TheEditor->windowIsGadget( window ) == FALSE )
	{
		GameWindow *child = window->winGetChild();

		addWindowToTree( child, newItem, HIERARCHY_ADD_AT_BOTTOM, TRUE, TRUE );

	}

}

// HierarchyView::moveWindowChildOf ===========================================
/** Move the hierarchy entry for window so that it is now the first
	* child in the list under parent */
//=============================================================================
void HierarchyView::moveWindowChildOf( GameWindow *window, GameWindow *parent )
{

	// sanity
	if( window == NULL )
		return;

	// remvoe the window from the hierarchy
	removeWindow( window );

	// if parent is NULL we'll put at top of list
	if( parent == NULL )
	{

		addWindow( window, HIERARCHY_ADD_AT_TOP );
		return;

	}

	// find the entry of the parent
	HTREEITEM parentItem = findTreeEntry( parent );
	if( parentItem == NULL )
	{

		DEBUG_LOG(( "moveWindowChildOf: No parent entry" ));
		assert( 0 );
		return;

	}

	// add the window as child of the parent at the top, dont forget to
	// also add the children of the window too!
	addWindowToTree( window, parentItem, HIERARCHY_ADD_AT_TOP, TRUE, FALSE );

}

// HierarchyView::treePointToItem =============================================
/** Given the location (x,y) in TREE COORDINATES, correlate that to
	* a tree item, if any */
//=============================================================================
HTREEITEM HierarchyView::treePointToItem( Int x, Int y )
{

	// which tree item are we now over
	TVHITTESTINFO hitTest;
	hitTest.pt.x = x;
	hitTest.pt.y = y;
	hitTest.hItem = NULL;
	hitTest.flags = TVHT_ONITEM;
	return TreeView_HitTest( TheHierarchyView->getTreeHandle(), &hitTest );

}

// HierarchyView::getWindowFromItem ===========================================
/** Get the game window we stored as the user data lParam in the tree
	* item */
//=============================================================================
GameWindow *HierarchyView::getWindowFromItem( HTREEITEM treeItem )
{

	// sanity
	if( treeItem == NULL )
		return NULL;

	// get the node info from the tree item we're over
	TVITEM itemInfo;
	GameWindow *window;

	itemInfo.hItem = treeItem;
	itemInfo.lParam = NULL;
	itemInfo.mask = TVIF_HANDLE | TVIF_PARAM;
	TreeView_GetItem( m_tree, &itemInfo );
	window = (GameWindow *)itemInfo.lParam;
	assert( window );

	return window;

}

// HierarchyView::selectWindow ================================================
/** Select the tree item */
//=============================================================================
void HierarchyView::selectWindow( GameWindow *window )
{
	HTREEITEM item = NULL;

	// get the item associated with the window
	if( window )
		item = findTreeEntry( window );

	// select the item, or no item NULL will select nothing
	TreeView_SelectItem( m_tree, item );
	TreeView_Expand( m_tree, item, 0 );

}

// HierarchyView::validateDragDropOperation ===================================
/** Return TRUE if the drag drop operation of source onto target
	* is logically OK.  It's not OK if target is a child of source because
	* you cannot move a parent window into it's own child list. */
//=============================================================================
Bool HierarchyView::validateDragDropOperation( GameWindow *source,
																							 GameWindow *target )
{

	// sanity
	if( source == NULL || target == NULL )
		return FALSE;

	// if target is the source or is a child of source in any way this is illegal
	GameWindow *other = target;
	while( other )
	{

		if( source == other )
			return FALSE;
		other = other->winGetParent();

	}

	// everything is ok
	return TRUE;

}





