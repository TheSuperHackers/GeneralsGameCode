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

//
//	XLStuff.cpp
//
//
#include "StdAfx.h"
#include "Babylon.h"
#include "resource.h"
#include <stdio.h>
#include "XLStuff.h"
#include <assert.h>
#include <comdef.h>

static const int xlWorkbookNormal = -4143;
static const int xlNoChange = 1;
static const int xlLocalSessionChanges = 2;
static const int xlWBATWorksheet = -4167;
static _Workbook *workbook = NULL;
static _Application *xl = NULL;
static Workbooks *wbs = NULL;
static Range *range = NULL;
static _Worksheet *ws = NULL;
static OLECHAR buffer[100*1024];

static VARIANT no, yes, dummy, dummy0, nullstring, empty;
static VARIANT continuous, automatic, medium, thin, none;
static VARIANT yellow, solid;

static VARIANT GetCell ( int row, int column )
{
	VARIANT cell;
	VARIANT result;
	LPDISPATCH dispatch;
	OLECHAR cellname[20];

	V_VT ( &cell ) = VT_EMPTY;

	assert ( column > 0 );
	swprintf ( cellname, L"%c%d", 'A'+column -1 , row );
 	V_VT ( &cell ) = VT_BSTR;
 	V_BSTR ( &cell ) = SysAllocString (cellname);

	V_VT ( &result ) = VT_BOOL;
	V_BOOL ( &result ) = FALSE;

	if ( !ws )
	{
		goto error;
	}

  if ( ! (dispatch = ws->GetRange (cell, cell )))
	{
		goto error;
	}


	range->AttachDispatch ( dispatch );
	result = range->GetValue ();
	range->ReleaseDispatch ( );

error:

	VariantClear ( &cell );

	return result;
}


int PutCell ( int row, int column, const OLECHAR *string, int val )
{
	VARIANT cell;
	VARIANT newValue;
	int ok = FALSE;
	LPDISPATCH dispatch;
	OLECHAR cellname[20];


	V_VT ( &newValue ) = VT_EMPTY;
	V_VT ( &cell ) = VT_EMPTY;

	assert ( column > 0 );
	swprintf ( cellname, L"%c%d", 'A'+column-1, row );
 	V_VT ( &cell ) = VT_BSTR;
 	V_BSTR ( &cell ) = SysAllocString (cellname);

	if ( !ws )
	{
		goto error;
	}

  if ( ! (dispatch = ws->GetRange (cell, cell )))
	{
		goto error;
	}

	range->AttachDispatch ( dispatch );

	if ( string )
	{
		V_VT ( &newValue ) = VT_BSTR;

		if ( string[0] == '\'')
		{
			buffer[0] = '\\';
			wcscpy ( &buffer[1], string );
			V_BSTR ( &newValue ) = SysAllocString ( buffer );
		}
		else
		{
			V_BSTR ( &newValue ) = SysAllocString ( string );
		}

	}
	else
	{
		V_VT ( &newValue ) = VT_I4;
		V_I4 ( &newValue ) = val;
	}

	range->SetValue ( newValue );
	range->ReleaseDispatch ( );
	ok = TRUE;

error:


	VariantClear ( &cell );
	VariantClear ( &newValue );

	return ok;
}

int PutSeparator ( int row )
{
//    Rows(row).Select
//    Selection.Borders(xlDiagonalDown).LineStyle = xlNone
//    Selection.Borders(xlDiagonalUp).LineStyle = xlNone
//    Selection.Borders(xlEdgeLeft).LineStyle = xlNone
//    Selection.Borders(xlEdgeTop).LineStyle = xlNone
//    With Selection.Borders(xlEdgeBottom)
//        .LineStyle = xlContinuous
//        .Weight = xlMedium
//        .ColorIndex = xlAutomatic
//    End With
//    With Selection.Borders(xlEdgeRight)
//        .LineStyle = xlContinuous
//        .Weight = xlThin
//        .ColorIndex = xlAutomatic
//    End With
	int ok = FALSE;
	Border *border = NULL;
	Borders *borders = NULL;
	LPDISPATCH dispatch;
	OLECHAR cellname1[20];
	OLECHAR cellname2[20];
	VARIANT cell1,cell2;

  if ( !ws )
  {
 		goto error;
  }

	assert ( row > 0 );
	swprintf ( cellname1, L"A%d", row );
	swprintf ( cellname2, L"%c%d", 'A'+CELL_LAST-1-1, row );
 	V_VT ( &cell1 ) = VT_BSTR;
 	V_BSTR ( &cell1 ) = SysAllocString (cellname1);

 	V_VT ( &cell2 ) = VT_BSTR;
 	V_BSTR ( &cell2 ) = SysAllocString (cellname2);


  if ( ! (dispatch = ws->GetRange (cell1, cell2 )))
	{
		goto error;
	}

	range->AttachDispatch ( dispatch );

	dispatch = range->GetBorders ();


	borders = new Borders ( dispatch );

	if ( !borders )
	{
		goto error;
	}

	dispatch = borders->GetItem ( xlEdgeBottom );

	border = new Border ( dispatch );

	if ( !border )
	{
		goto error;
	}

	border->SetLineStyle ( continuous );
	border->SetColorIndex ( automatic );
	border->SetWeight ( thin );

	ok = TRUE;

error:

	range->ReleaseDispatch ( );

	delete borders;
	delete border;

	VariantClear ( &cell1 );
	VariantClear ( &cell2 );

	return ok;
}


int PutSection ( int row, OLECHAR *title )
{

	int ok = FALSE;
	Range *range = NULL;
	Border *border = NULL;
	Borders *borders = NULL;
	Interior *interior = NULL;
	LPDISPATCH dispatch;
	OLECHAR cellname1[20];
	OLECHAR cellname2[20];
	VARIANT cell1,cell2;
	_Worksheet *ws = NULL;

  if ( !ws )
  {
 		goto error;
  }

	assert ( row > 0 );
	swprintf ( cellname1, L"A%d", row );
	swprintf ( cellname2, L"%c%d", 'A'+CELL_LAST-1-1, row );
 	V_VT ( &cell1 ) = VT_BSTR;
 	V_BSTR ( &cell1 ) = SysAllocString (cellname1);

 	V_VT ( &cell2 ) = VT_BSTR;
 	V_BSTR ( &cell2 ) = SysAllocString (cellname2);


  if ( ! (dispatch = ws->GetRange (cell1, cell2 )))
	{
		goto error;
	}

	range->AttachDispatch ( dispatch );

	dispatch = range->GetBorders ();


	borders = new Borders ( dispatch );

	if ( !borders )
	{
		goto error;
	}

	dispatch = borders->GetItem ( xlEdgeBottom );

	border = new Border ( dispatch );

	if ( !border )
	{
		goto error;
	}

	border->SetLineStyle ( continuous );
	border->SetColorIndex ( automatic );
	border->SetWeight ( thin );

	delete border;
	border = NULL;

	dispatch = borders->GetItem ( xlEdgeTop );

	border = new Border ( dispatch );

	if ( !border )
	{
		goto error;
	}

	border->SetLineStyle ( continuous );
	border->SetColorIndex ( automatic );
	border->SetWeight ( medium );

	delete border;
	border = NULL;

	dispatch = borders->GetItem ( xlEdgeRight );

	border = new Border ( dispatch );

	if ( !border )
	{
		goto error;
	}

	border->SetLineStyle ( none );

	delete border;
	border = NULL;

	dispatch = borders->GetItem ( xlEdgeLeft );

	border = new Border ( dispatch );

	if ( !border )
	{
		goto error;
	}

	border->SetLineStyle ( none );

	dispatch = range->GetInterior ( );

	interior = new Interior ( dispatch );

	if ( !interior )
	{
		goto error;
	}

	interior->SetColorIndex ( yellow );
	interior->SetPattern ( solid );

	PutCell ( row, 1, L"Section", 0 );
	PutCell ( row, 2, title, 0 );

	ok = TRUE;

error:

	range->ReleaseDispatch ( );

	delete borders;
	delete border;

	VariantClear ( &cell1 );
	VariantClear ( &cell2 );

	return ok;

}

int OpenExcel ( void )
{
	LPDISPATCH dispatch;

	if ( xl )
	{
		return TRUE;
	}
#if 0
	while ( ExcelRunning ())
	{
		if ( AfxMessageBox ( "Excel is running!\n\nClose or kill all instances of Excel and retry\n\nNOTE: Check task tray (CTRL-ALT-DELETE) for instances of Excel", MB_OKCANCEL ) == IDCANCEL )
		{
			return FALSE;
		}
	}
#endif

	xl = new _Application();

	if ( !xl )
	{
		return FALSE;
	}

	if ( !xl->CreateDispatch ("Excel.Application"))
	{
		goto error_access;
	}

	dispatch = xl->GetWorkbooks ( );

	if ( dispatch )
	{
		wbs = new Workbooks( dispatch );
	}

	if ( !wbs )
	{
		return FALSE;
	}

	if ( ! (ws = new _Worksheet ()))
	{
		return FALSE;
	}

	if ( ! (range = new Range ()))
	{
		return FALSE;
	}


	V_VT ( &no ) = VT_BOOL;
	V_VT ( &yes ) = VT_BOOL;
	V_VT ( &dummy ) = VT_I4;
	V_VT ( &dummy0 ) = VT_I4;
	V_VT ( &nullstring ) = VT_BSTR ;
	V_VT ( &empty ) = VT_EMPTY;
	V_VT ( &continuous ) = VT_I4;
	V_VT ( &automatic ) = VT_I4;
	V_VT ( &medium ) = VT_I4;
	V_VT ( &thin ) = VT_I4;
	V_VT ( &none ) = VT_I4;
	V_VT ( &solid ) = VT_I4;
	V_VT ( &yellow ) = VT_I4;

	V_BOOL ( &no ) = FALSE;
	V_BOOL ( &yes ) = TRUE;
	V_I4 ( &dummy ) = 1;
	V_I4 ( &dummy0 ) = 0;
	V_BSTR ( &nullstring ) = SysAllocString ( OLESTR ("") );

	V_I4 ( &continuous ) = xlContinuous;
	V_I4 ( &automatic ) = xlAutomatic;
	V_I4 ( &medium ) = xlMedium;
	V_I4 ( &thin ) = xlThin;
	V_I4 ( &none ) = xlThin;
	V_I4 ( &solid ) = xlSolid;
	V_I4 ( &yellow ) = 6;


	return TRUE;

error_access:
		AfxMessageBox ("Could not access Excel!\n\nMake sure Excel is installed on this system.");
		return FALSE;
}


void CloseExcel ( void )
{
	CloseWorkBook ();

	delete range;
	range = NULL;

	delete ws;
	ws = NULL;

	if ( wbs )
	{
		wbs->Close();
		delete wbs;
		wbs = NULL;
	}

	if ( xl )
	{
		xl->Quit();
		xl->ReleaseDispatch ();
		delete xl;
		xl = NULL;
	}

	VariantClear ( &nullstring );

}

int OpenWorkBook ( const char *filename )
{
	LPDISPATCH dispatch;

	dispatch = wbs->Open ((LPCTSTR) filename, dummy0, yes, dummy, nullstring, nullstring, yes, dummy, dummy, no, no, dummy, no );

	if ( dispatch )
	{
		workbook = new _Workbook ( dispatch );
	}

	if ( !workbook )
	{
		return FALSE;
	}

	SelectActiveSheet ( );

	return TRUE;
}

int NewWorkBook ( const char *path )
{
	LPDISPATCH dispatch;
	VARIANT temp;
	char tfile[200];
	char *p;
	WIN32_FIND_DATA finfo;
	HANDLE handle;

	V_VT ( &temp ) = VT_I4;
	V_I4 ( &temp ) = xlWBATWorksheet;

	if ( path )
	{
		strcpy ( tfile, path );
		if ( (p = strchr ( tfile, '.' )))
		{
			*p = 0;
		}

		strcpy ( p, ".xlt" );

		if ( (handle = FindFirstFile ( tfile, &finfo)) != INVALID_HANDLE_VALUE )
		{
			if ( !(finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				swprintf ( buffer, L"%S", tfile );
				V_VT ( &temp ) = VT_BSTR;
				V_BSTR ( &temp ) = SysAllocString ( buffer );
			}

			FindClose ( handle );
		}
	}

	dispatch = wbs->Add ( temp );

	VariantClear ( &temp );

	if ( dispatch )
	{
		workbook = new _Workbook ( dispatch );
	}

	if ( !workbook )
	{
		return FALSE;
	}
	SelectActiveSheet ( );
	return TRUE;
}

int SaveWorkBook ( const char *filename, int protect )
{
	VARIANT name, fileformat, rc;

	V_VT ( &name ) = VT_BSTR;
	swprintf ( buffer, L"%S", filename );
	V_BSTR ( &name ) = SysAllocString ( buffer );

	V_VT ( &fileformat ) = VT_I4;
	V_I4 ( &fileformat ) = xlWorkbookNormal;


	V_VT ( &rc ) = VT_I4;
	V_I4 ( &rc ) = xlLocalSessionChanges;

	if ( protect )
	{
		VARIANT password;
		V_VT ( &password ) = VT_BSTR;
		V_BSTR ( &password ) = SysAllocString ( L"" );

		ws->Protect ( password, yes, yes, yes, no );
		VariantClear ( &password );
	}
	workbook->SaveAs ( name, fileformat, nullstring, nullstring, no, no,
					xlNoChange, rc, no, empty, empty );

	VariantClear ( &name );

	return TRUE;

}

void CloseWorkBook ( void )
{
	if ( workbook )
	{
		workbook->SetSaved ( TRUE );
		workbook->Close ( no, nullstring, no );
		delete workbook;
		workbook = NULL;
	}

}

void SelectActiveSheet ( void )
{
	LPDISPATCH dispatch;

  if ( ! (dispatch = xl->GetActiveSheet ()))
	{
		return;
	}

	ws->ReleaseDispatch ( );
	ws->AttachDispatch ( dispatch );
}

int GetInt ( int row, int cell )
{
	long value;
	VARIANT var;
	_variant_t v;

	var = GetCell ( row, cell );

	v.Attach ( var );

	value = v;

	return (int) value;

}

int GetString ( int row, int cell, OLECHAR *string )
{
	VARIANT var;

	string[0] =0;
	var = GetCell ( row, cell );

	if ( V_VT ( &var ) == VT_BSTR )
	{
		wcscpy ( string, V_BSTR ( &var ));

	}
	VariantClear ( &var );

	return 1;

}

