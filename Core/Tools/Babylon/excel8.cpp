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

// Machine generated IDispatch wrapper class(es) created with ClassWizard

#include "StdAfx.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// Workbooks properties

/////////////////////////////////////////////////////////////////////////////
// Workbooks operations

LPDISPATCH Workbooks::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long Workbooks::GetCreator()
{
	long result;
	InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Workbooks::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Workbooks::Add(const VARIANT& Template)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xb5, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&Template);
	return result;
}

void Workbooks::Close()
{
	InvokeHelper(0x115, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

long Workbooks::GetCount()
{
	long result;
	InvokeHelper(0x76, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Workbooks::GetItem(const VARIANT& Index)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xaa, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		&Index);
	return result;
}

LPUNKNOWN Workbooks::Get_NewEnum()
{
	LPUNKNOWN result;
	InvokeHelper(0xfffffffc, DISPATCH_PROPERTYGET, VT_UNKNOWN, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Workbooks::Open(LPCTSTR Filename, const VARIANT& UpdateLinks, const VARIANT& ReadOnly, const VARIANT& Format, const VARIANT& Password, const VARIANT& WriteResPassword, const VARIANT& IgnoreReadOnlyRecommended, const VARIANT& Origin,
		const VARIANT& Delimiter, const VARIANT& Editable, const VARIANT& Notify, const VARIANT& Converter, const VARIANT& AddToMru)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x2aa, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		Filename, &UpdateLinks, &ReadOnly, &Format, &Password, &WriteResPassword, &IgnoreReadOnlyRecommended, &Origin, &Delimiter, &Editable, &Notify, &Converter, &AddToMru);
	return result;
}

void Workbooks::OpenText(LPCTSTR Filename, const VARIANT& Origin, const VARIANT& StartRow, const VARIANT& DataType, long TextQualifier, const VARIANT& ConsecutiveDelimiter, const VARIANT& Tab, const VARIANT& Semicolon, const VARIANT& Comma,
		const VARIANT& Space, const VARIANT& Other, const VARIANT& OtherChar, const VARIANT& FieldInfo, const VARIANT& TextVisualLayout)
{
	static BYTE parms[] =
		VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x2ab, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Filename, &Origin, &StartRow, &DataType, TextQualifier, &ConsecutiveDelimiter, &Tab, &Semicolon, &Comma, &Space, &Other, &OtherChar, &FieldInfo, &TextVisualLayout);
}

LPDISPATCH Workbooks::Get_Default(const VARIANT& Index)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		&Index);
	return result;
}


/////////////////////////////////////////////////////////////////////////////
// _Application properties

/////////////////////////////////////////////////////////////////////////////
// _Application operations

LPDISPATCH _Application::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long _Application::GetCreator()
{
	long result;
	InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetActiveCell()
{
	LPDISPATCH result;
	InvokeHelper(0x131, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetActiveChart()
{
	LPDISPATCH result;
	InvokeHelper(0xb7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

CString _Application::GetActivePrinter()
{
	CString result;
	InvokeHelper(0x132, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Application::SetActivePrinter(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x132, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

LPDISPATCH _Application::GetActiveSheet()
{
	LPDISPATCH result;
	InvokeHelper(0x133, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetActiveWindow()
{
	LPDISPATCH result;
	InvokeHelper(0x2f7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetActiveWorkbook()
{
	LPDISPATCH result;
	InvokeHelper(0x134, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetAddIns()
{
	LPDISPATCH result;
	InvokeHelper(0x225, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetAssistant()
{
	LPDISPATCH result;
	InvokeHelper(0x59e, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Application::Calculate()
{
	InvokeHelper(0x117, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH _Application::GetCells()
{
	LPDISPATCH result;
	InvokeHelper(0xee, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetCharts()
{
	LPDISPATCH result;
	InvokeHelper(0x79, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetColumns()
{
	LPDISPATCH result;
	InvokeHelper(0xf1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetCommandBars()
{
	LPDISPATCH result;
	InvokeHelper(0x59f, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long _Application::GetDDEAppReturnCode()
{
	long result;
	InvokeHelper(0x14c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::DDEExecute(long Channel, LPCTSTR String)
{
	static BYTE parms[] =
		VTS_I4 VTS_BSTR;
	InvokeHelper(0x14d, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Channel, String);
}

long _Application::DDEInitiate(LPCTSTR App, LPCTSTR Topic)
{
	long result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x14e, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		App, Topic);
	return result;
}

void _Application::DDEPoke(long Channel, const VARIANT& Item, const VARIANT& Data)
{
	static BYTE parms[] =
		VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x14f, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Channel, &Item, &Data);
}

VARIANT _Application::DDERequest(long Channel, LPCTSTR Item)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_I4 VTS_BSTR;
	InvokeHelper(0x150, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		Channel, Item);
	return result;
}

void _Application::DDETerminate(long Channel)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x151, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Channel);
}

VARIANT _Application::Evaluate(const VARIANT& Name)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x1, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&Name);
	return result;
}

VARIANT _Application::_Evaluate(const VARIANT& Name)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xfffffffb, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&Name);
	return result;
}

VARIANT _Application::ExecuteExcel4Macro(LPCTSTR String)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x15e, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		String);
	return result;
}

LPDISPATCH _Application::Intersect(LPDISPATCH Arg1, LPDISPATCH Arg2, const VARIANT& Arg3, const VARIANT& Arg4, const VARIANT& Arg5, const VARIANT& Arg6, const VARIANT& Arg7, const VARIANT& Arg8, const VARIANT& Arg9, const VARIANT& Arg10,
		const VARIANT& Arg11, const VARIANT& Arg12, const VARIANT& Arg13, const VARIANT& Arg14, const VARIANT& Arg15, const VARIANT& Arg16, const VARIANT& Arg17, const VARIANT& Arg18, const VARIANT& Arg19, const VARIANT& Arg20,
		const VARIANT& Arg21, const VARIANT& Arg22, const VARIANT& Arg23, const VARIANT& Arg24, const VARIANT& Arg25, const VARIANT& Arg26, const VARIANT& Arg27, const VARIANT& Arg28, const VARIANT& Arg29, const VARIANT& Arg30)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_DISPATCH VTS_DISPATCH VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x2fe, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		Arg1, Arg2, &Arg3, &Arg4, &Arg5, &Arg6, &Arg7, &Arg8, &Arg9, &Arg10, &Arg11, &Arg12, &Arg13, &Arg14, &Arg15, &Arg16, &Arg17, &Arg18, &Arg19, &Arg20, &Arg21, &Arg22, &Arg23, &Arg24, &Arg25, &Arg26, &Arg27, &Arg28, &Arg29, &Arg30);
	return result;
}

LPDISPATCH _Application::GetNames()
{
	LPDISPATCH result;
	InvokeHelper(0x1ba, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetRange(const VARIANT& Cell1, const VARIANT& Cell2)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xc5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		&Cell1, &Cell2);
	return result;
}

LPDISPATCH _Application::GetRows()
{
	LPDISPATCH result;
	InvokeHelper(0x102, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT _Application::Run(const VARIANT& Macro, const VARIANT& Arg1, const VARIANT& Arg2, const VARIANT& Arg3, const VARIANT& Arg4, const VARIANT& Arg5, const VARIANT& Arg6, const VARIANT& Arg7, const VARIANT& Arg8, const VARIANT& Arg9,
		const VARIANT& Arg10, const VARIANT& Arg11, const VARIANT& Arg12, const VARIANT& Arg13, const VARIANT& Arg14, const VARIANT& Arg15, const VARIANT& Arg16, const VARIANT& Arg17, const VARIANT& Arg18, const VARIANT& Arg19,
		const VARIANT& Arg20, const VARIANT& Arg21, const VARIANT& Arg22, const VARIANT& Arg23, const VARIANT& Arg24, const VARIANT& Arg25, const VARIANT& Arg26, const VARIANT& Arg27, const VARIANT& Arg28, const VARIANT& Arg29,
		const VARIANT& Arg30)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x103, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&Macro, &Arg1, &Arg2, &Arg3, &Arg4, &Arg5, &Arg6, &Arg7, &Arg8, &Arg9, &Arg10, &Arg11, &Arg12, &Arg13, &Arg14, &Arg15, &Arg16, &Arg17, &Arg18, &Arg19, &Arg20, &Arg21, &Arg22, &Arg23, &Arg24, &Arg25, &Arg26, &Arg27, &Arg28, &Arg29, &Arg30);
	return result;
}

VARIANT _Application::_Run2(const VARIANT& Macro, const VARIANT& Arg1, const VARIANT& Arg2, const VARIANT& Arg3, const VARIANT& Arg4, const VARIANT& Arg5, const VARIANT& Arg6, const VARIANT& Arg7, const VARIANT& Arg8, const VARIANT& Arg9,
		const VARIANT& Arg10, const VARIANT& Arg11, const VARIANT& Arg12, const VARIANT& Arg13, const VARIANT& Arg14, const VARIANT& Arg15, const VARIANT& Arg16, const VARIANT& Arg17, const VARIANT& Arg18, const VARIANT& Arg19,
		const VARIANT& Arg20, const VARIANT& Arg21, const VARIANT& Arg22, const VARIANT& Arg23, const VARIANT& Arg24, const VARIANT& Arg25, const VARIANT& Arg26, const VARIANT& Arg27, const VARIANT& Arg28, const VARIANT& Arg29,
		const VARIANT& Arg30)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x326, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&Macro, &Arg1, &Arg2, &Arg3, &Arg4, &Arg5, &Arg6, &Arg7, &Arg8, &Arg9, &Arg10, &Arg11, &Arg12, &Arg13, &Arg14, &Arg15, &Arg16, &Arg17, &Arg18, &Arg19, &Arg20, &Arg21, &Arg22, &Arg23, &Arg24, &Arg25, &Arg26, &Arg27, &Arg28, &Arg29, &Arg30);
	return result;
}

LPDISPATCH _Application::GetSelection()
{
	LPDISPATCH result;
	InvokeHelper(0x93, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Application::SendKeys(const VARIANT& Keys, const VARIANT& Wait)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x17f, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Keys, &Wait);
}

LPDISPATCH _Application::GetSheets()
{
	LPDISPATCH result;
	InvokeHelper(0x1e5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetThisWorkbook()
{
	LPDISPATCH result;
	InvokeHelper(0x30a, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::Union(LPDISPATCH Arg1, LPDISPATCH Arg2, const VARIANT& Arg3, const VARIANT& Arg4, const VARIANT& Arg5, const VARIANT& Arg6, const VARIANT& Arg7, const VARIANT& Arg8, const VARIANT& Arg9, const VARIANT& Arg10, const VARIANT& Arg11,
		const VARIANT& Arg12, const VARIANT& Arg13, const VARIANT& Arg14, const VARIANT& Arg15, const VARIANT& Arg16, const VARIANT& Arg17, const VARIANT& Arg18, const VARIANT& Arg19, const VARIANT& Arg20, const VARIANT& Arg21,
		const VARIANT& Arg22, const VARIANT& Arg23, const VARIANT& Arg24, const VARIANT& Arg25, const VARIANT& Arg26, const VARIANT& Arg27, const VARIANT& Arg28, const VARIANT& Arg29, const VARIANT& Arg30)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_DISPATCH VTS_DISPATCH VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x30b, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		Arg1, Arg2, &Arg3, &Arg4, &Arg5, &Arg6, &Arg7, &Arg8, &Arg9, &Arg10, &Arg11, &Arg12, &Arg13, &Arg14, &Arg15, &Arg16, &Arg17, &Arg18, &Arg19, &Arg20, &Arg21, &Arg22, &Arg23, &Arg24, &Arg25, &Arg26, &Arg27, &Arg28, &Arg29, &Arg30);
	return result;
}

LPDISPATCH _Application::GetWindows()
{
	LPDISPATCH result;
	InvokeHelper(0x1ae, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetWorkbooks()
{
	LPDISPATCH result;
	InvokeHelper(0x23c, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetWorksheetFunction()
{
	LPDISPATCH result;
	InvokeHelper(0x5a0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetWorksheets()
{
	LPDISPATCH result;
	InvokeHelper(0x1ee, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetExcel4IntlMacroSheets()
{
	LPDISPATCH result;
	InvokeHelper(0x245, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetExcel4MacroSheets()
{
	LPDISPATCH result;
	InvokeHelper(0x243, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Application::ActivateMicrosoftApp(long Index)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x447, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Index);
}

void _Application::AddChartAutoFormat(const VARIANT& Chart, LPCTSTR Name, const VARIANT& Description)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_BSTR VTS_VARIANT;
	InvokeHelper(0xd8, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Chart, Name, &Description);
}

void _Application::AddCustomList(const VARIANT& ListArray, const VARIANT& ByRow)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x30c, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &ListArray, &ByRow);
}

BOOL _Application::GetAlertBeforeOverwriting()
{
	BOOL result;
	InvokeHelper(0x3a2, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetAlertBeforeOverwriting(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x3a2, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

CString _Application::GetAltStartupPath()
{
	CString result;
	InvokeHelper(0x139, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Application::SetAltStartupPath(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x139, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

BOOL _Application::GetAskToUpdateLinks()
{
	BOOL result;
	InvokeHelper(0x3e0, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetAskToUpdateLinks(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x3e0, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Application::GetEnableAnimations()
{
	BOOL result;
	InvokeHelper(0x49c, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetEnableAnimations(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x49c, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

LPDISPATCH _Application::GetAutoCorrect()
{
	LPDISPATCH result;
	InvokeHelper(0x479, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long _Application::GetBuild()
{
	long result;
	InvokeHelper(0x13a, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

BOOL _Application::GetCalculateBeforeSave()
{
	BOOL result;
	InvokeHelper(0x13b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetCalculateBeforeSave(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x13b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

long _Application::GetCalculation()
{
	long result;
	InvokeHelper(0x13c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetCalculation(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x13c, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

VARIANT _Application::GetCaller(const VARIANT& Index)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x13d, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, parms,
		&Index);
	return result;
}

BOOL _Application::GetCanPlaySounds()
{
	BOOL result;
	InvokeHelper(0x13e, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

BOOL _Application::GetCanRecordSounds()
{
	BOOL result;
	InvokeHelper(0x13f, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

CString _Application::GetCaption()
{
	CString result;
	InvokeHelper(0x8b, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Application::SetCaption(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x8b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

BOOL _Application::GetCellDragAndDrop()
{
	BOOL result;
	InvokeHelper(0x140, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetCellDragAndDrop(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x140, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

double _Application::CentimetersToPoints(double Centimeters)
{
	double result;
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x43e, DISPATCH_METHOD, VT_R8, (void*)&result, parms,
		Centimeters);
	return result;
}

BOOL _Application::CheckSpelling(LPCTSTR Word, const VARIANT& CustomDictionary, const VARIANT& IgnoreUppercase)
{
	BOOL result;
	static BYTE parms[] =
		VTS_BSTR VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1f9, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		Word, &CustomDictionary, &IgnoreUppercase);
	return result;
}

VARIANT _Application::GetClipboardFormats(const VARIANT& Index)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x141, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, parms,
		&Index);
	return result;
}

BOOL _Application::GetDisplayClipboardWindow()
{
	BOOL result;
	InvokeHelper(0x142, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetDisplayClipboardWindow(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x142, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

long _Application::GetCommandUnderlines()
{
	long result;
	InvokeHelper(0x143, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetCommandUnderlines(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x143, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

BOOL _Application::GetConstrainNumeric()
{
	BOOL result;
	InvokeHelper(0x144, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetConstrainNumeric(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x144, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

VARIANT _Application::ConvertFormula(const VARIANT& Formula, long FromReferenceStyle, const VARIANT& ToReferenceStyle, const VARIANT& ToAbsolute, const VARIANT& RelativeTo)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x145, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&Formula, FromReferenceStyle, &ToReferenceStyle, &ToAbsolute, &RelativeTo);
	return result;
}

BOOL _Application::GetCopyObjectsWithCells()
{
	BOOL result;
	InvokeHelper(0x3df, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetCopyObjectsWithCells(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x3df, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

long _Application::GetCursor()
{
	long result;
	InvokeHelper(0x489, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetCursor(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x489, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

long _Application::GetCustomListCount()
{
	long result;
	InvokeHelper(0x313, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

long _Application::GetCutCopyMode()
{
	long result;
	InvokeHelper(0x14a, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetCutCopyMode(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x14a, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

long _Application::GetDataEntryMode()
{
	long result;
	InvokeHelper(0x14b, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetDataEntryMode(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x14b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

CString _Application::Get_Default()
{
	CString result;
	InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

CString _Application::GetDefaultFilePath()
{
	CString result;
	InvokeHelper(0x40e, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Application::SetDefaultFilePath(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x40e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

void _Application::DeleteChartAutoFormat(LPCTSTR Name)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0xd9, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Name);
}

void _Application::DeleteCustomList(long ListNum)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x30f, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 ListNum);
}

LPDISPATCH _Application::GetDialogs()
{
	LPDISPATCH result;
	InvokeHelper(0x2f9, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

BOOL _Application::GetDisplayAlerts()
{
	BOOL result;
	InvokeHelper(0x157, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetDisplayAlerts(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x157, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Application::GetDisplayFormulaBar()
{
	BOOL result;
	InvokeHelper(0x158, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetDisplayFormulaBar(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x158, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Application::GetDisplayFullScreen()
{
	BOOL result;
	InvokeHelper(0x425, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetDisplayFullScreen(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x425, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Application::GetDisplayNoteIndicator()
{
	BOOL result;
	InvokeHelper(0x159, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetDisplayNoteIndicator(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x159, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

long _Application::GetDisplayCommentIndicator()
{
	long result;
	InvokeHelper(0x4ac, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetDisplayCommentIndicator(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x4ac, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

BOOL _Application::GetDisplayExcel4Menus()
{
	BOOL result;
	InvokeHelper(0x39f, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetDisplayExcel4Menus(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x39f, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Application::GetDisplayRecentFiles()
{
	BOOL result;
	InvokeHelper(0x39e, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetDisplayRecentFiles(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x39e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Application::GetDisplayScrollBars()
{
	BOOL result;
	InvokeHelper(0x15a, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetDisplayScrollBars(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x15a, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Application::GetDisplayStatusBar()
{
	BOOL result;
	InvokeHelper(0x15b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetDisplayStatusBar(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x15b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Application::DoubleClick()
{
	InvokeHelper(0x15d, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

BOOL _Application::GetEditDirectlyInCell()
{
	BOOL result;
	InvokeHelper(0x3a1, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetEditDirectlyInCell(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x3a1, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Application::GetEnableAutoComplete()
{
	BOOL result;
	InvokeHelper(0x49b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetEnableAutoComplete(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x49b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

long _Application::GetEnableCancelKey()
{
	long result;
	InvokeHelper(0x448, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetEnableCancelKey(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x448, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

BOOL _Application::GetEnableSound()
{
	BOOL result;
	InvokeHelper(0x4ad, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetEnableSound(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x4ad, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

VARIANT _Application::GetFileConverters(const VARIANT& Index1, const VARIANT& Index2)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x3a3, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, parms,
		&Index1, &Index2);
	return result;
}

LPDISPATCH _Application::GetFileSearch()
{
	LPDISPATCH result;
	InvokeHelper(0x4b0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetFileFind()
{
	LPDISPATCH result;
	InvokeHelper(0x4b1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Application::FindFile()
{
	InvokeHelper(0x42c, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

BOOL _Application::GetFixedDecimal()
{
	BOOL result;
	InvokeHelper(0x15f, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetFixedDecimal(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x15f, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

long _Application::GetFixedDecimalPlaces()
{
	long result;
	InvokeHelper(0x160, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetFixedDecimalPlaces(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x160, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

VARIANT _Application::GetCustomListContents(long ListNum)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x312, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		ListNum);
	return result;
}

long _Application::GetCustomListNum(const VARIANT& ListArray)
{
	long result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x311, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		&ListArray);
	return result;
}

VARIANT _Application::GetOpenFilename(const VARIANT& FileFilter, const VARIANT& FilterIndex, const VARIANT& Title, const VARIANT& ButtonText, const VARIANT& MultiSelect)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x433, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&FileFilter, &FilterIndex, &Title, &ButtonText, &MultiSelect);
	return result;
}

VARIANT _Application::GetSaveAsFilename(const VARIANT& InitialFilename, const VARIANT& FileFilter, const VARIANT& FilterIndex, const VARIANT& Title, const VARIANT& ButtonText)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x434, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&InitialFilename, &FileFilter, &FilterIndex, &Title, &ButtonText);
	return result;
}

void _Application::Goto(const VARIANT& Reference, const VARIANT& Scroll)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1db, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Reference, &Scroll);
}

double _Application::GetHeight()
{
	double result;
	InvokeHelper(0x7b, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, nullptr);
	return result;
}

void _Application::SetHeight(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x7b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 newValue);
}

void _Application::Help(const VARIANT& HelpFile, const VARIANT& HelpContextID)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x162, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &HelpFile, &HelpContextID);
}

BOOL _Application::GetIgnoreRemoteRequests()
{
	BOOL result;
	InvokeHelper(0x164, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetIgnoreRemoteRequests(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x164, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

double _Application::InchesToPoints(double Inches)
{
	double result;
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x43f, DISPATCH_METHOD, VT_R8, (void*)&result, parms,
		Inches);
	return result;
}

VARIANT _Application::InputBox(LPCTSTR Prompt, const VARIANT& Title, const VARIANT& Default, const VARIANT& Left, const VARIANT& Top, const VARIANT& HelpFile, const VARIANT& HelpContextID, const VARIANT& Type)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x165, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		Prompt, &Title, &Default, &Left, &Top, &HelpFile, &HelpContextID, &Type);
	return result;
}

BOOL _Application::GetInteractive()
{
	BOOL result;
	InvokeHelper(0x169, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetInteractive(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x169, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

VARIANT _Application::GetInternational(const VARIANT& Index)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x16a, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, parms,
		&Index);
	return result;
}

BOOL _Application::GetIteration()
{
	BOOL result;
	InvokeHelper(0x16b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetIteration(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x16b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

double _Application::GetLeft()
{
	double result;
	InvokeHelper(0x7f, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, nullptr);
	return result;
}

void _Application::SetLeft(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x7f, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 newValue);
}

CString _Application::GetLibraryPath()
{
	CString result;
	InvokeHelper(0x16e, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Application::MacroOptions(const VARIANT& Macro, const VARIANT& Description, const VARIANT& HasMenu, const VARIANT& MenuText, const VARIANT& HasShortcutKey, const VARIANT& ShortcutKey, const VARIANT& Category, const VARIANT& StatusBar,
		const VARIANT& HelpContextID, const VARIANT& HelpFile)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x46f, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Macro, &Description, &HasMenu, &MenuText, &HasShortcutKey, &ShortcutKey, &Category, &StatusBar, &HelpContextID, &HelpFile);
}

void _Application::MailLogoff()
{
	InvokeHelper(0x3b1, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void _Application::MailLogon(const VARIANT& Name, const VARIANT& Password, const VARIANT& DownloadNewMail)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x3af, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Name, &Password, &DownloadNewMail);
}

VARIANT _Application::GetMailSession()
{
	VARIANT result;
	InvokeHelper(0x3ae, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

long _Application::GetMailSystem()
{
	long result;
	InvokeHelper(0x3cb, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

BOOL _Application::GetMathCoprocessorAvailable()
{
	BOOL result;
	InvokeHelper(0x16f, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

double _Application::GetMaxChange()
{
	double result;
	InvokeHelper(0x170, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, nullptr);
	return result;
}

void _Application::SetMaxChange(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x170, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 newValue);
}

long _Application::GetMaxIterations()
{
	long result;
	InvokeHelper(0x171, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetMaxIterations(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x171, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

long _Application::GetMemoryFree()
{
	long result;
	InvokeHelper(0x172, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

long _Application::GetMemoryTotal()
{
	long result;
	InvokeHelper(0x173, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

long _Application::GetMemoryUsed()
{
	long result;
	InvokeHelper(0x174, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

BOOL _Application::GetMouseAvailable()
{
	BOOL result;
	InvokeHelper(0x175, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

BOOL _Application::GetMoveAfterReturn()
{
	BOOL result;
	InvokeHelper(0x176, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetMoveAfterReturn(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x176, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

long _Application::GetMoveAfterReturnDirection()
{
	long result;
	InvokeHelper(0x478, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetMoveAfterReturnDirection(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x478, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

LPDISPATCH _Application::GetRecentFiles()
{
	LPDISPATCH result;
	InvokeHelper(0x4b2, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

CString _Application::GetName()
{
	CString result;
	InvokeHelper(0x6e, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::NextLetter()
{
	LPDISPATCH result;
	InvokeHelper(0x3cc, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

CString _Application::GetNetworkTemplatesPath()
{
	CString result;
	InvokeHelper(0x184, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetODBCErrors()
{
	LPDISPATCH result;
	InvokeHelper(0x4b3, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long _Application::GetODBCTimeout()
{
	long result;
	InvokeHelper(0x4b4, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetODBCTimeout(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x4b4, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

void _Application::OnKey(LPCTSTR Key, const VARIANT& Procedure)
{
	static BYTE parms[] =
		VTS_BSTR VTS_VARIANT;
	InvokeHelper(0x272, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Key, &Procedure);
}

void _Application::OnRepeat(LPCTSTR Text, LPCTSTR Procedure)
{
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x301, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Text, Procedure);
}

void _Application::OnTime(const VARIANT& EarliestTime, LPCTSTR Procedure, const VARIANT& LatestTime, const VARIANT& Schedule)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_BSTR VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x270, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &EarliestTime, Procedure, &LatestTime, &Schedule);
}

void _Application::OnUndo(LPCTSTR Text, LPCTSTR Procedure)
{
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x302, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Text, Procedure);
}

CString _Application::GetOnWindow()
{
	CString result;
	InvokeHelper(0x26f, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Application::SetOnWindow(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x26f, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

CString _Application::GetOperatingSystem()
{
	CString result;
	InvokeHelper(0x177, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

CString _Application::GetOrganizationName()
{
	CString result;
	InvokeHelper(0x178, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

CString _Application::GetPath()
{
	CString result;
	InvokeHelper(0x123, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

CString _Application::GetPathSeparator()
{
	CString result;
	InvokeHelper(0x179, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

VARIANT _Application::GetPreviousSelections(const VARIANT& Index)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x17a, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, parms,
		&Index);
	return result;
}

BOOL _Application::GetPivotTableSelection()
{
	BOOL result;
	InvokeHelper(0x4b5, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetPivotTableSelection(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x4b5, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Application::GetPromptForSummaryInfo()
{
	BOOL result;
	InvokeHelper(0x426, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetPromptForSummaryInfo(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x426, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Application::Quit()
{
	InvokeHelper(0x12e, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void _Application::RecordMacro(const VARIANT& BasicCode, const VARIANT& XlmCode)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x305, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &BasicCode, &XlmCode);
}

BOOL _Application::GetRecordRelative()
{
	BOOL result;
	InvokeHelper(0x17b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

long _Application::GetReferenceStyle()
{
	long result;
	InvokeHelper(0x17c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetReferenceStyle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x17c, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

VARIANT _Application::GetRegisteredFunctions(const VARIANT& Index1, const VARIANT& Index2)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x307, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, parms,
		&Index1, &Index2);
	return result;
}

BOOL _Application::RegisterXLL(LPCTSTR Filename)
{
	BOOL result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x1e, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		Filename);
	return result;
}

void _Application::Repeat()
{
	InvokeHelper(0x12d, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

BOOL _Application::GetRollZoom()
{
	BOOL result;
	InvokeHelper(0x4b6, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetRollZoom(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x4b6, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Application::SaveWorkspace(const VARIANT& Filename)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xd4, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Filename);
}

BOOL _Application::GetScreenUpdating()
{
	BOOL result;
	InvokeHelper(0x17e, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetScreenUpdating(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x17e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Application::SetDefaultChart(const VARIANT& FormatName, const VARIANT& Gallery)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xdb, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &FormatName, &Gallery);
}

long _Application::GetSheetsInNewWorkbook()
{
	long result;
	InvokeHelper(0x3e1, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetSheetsInNewWorkbook(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x3e1, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

BOOL _Application::GetShowChartTipNames()
{
	BOOL result;
	InvokeHelper(0x4b7, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetShowChartTipNames(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x4b7, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Application::GetShowChartTipValues()
{
	BOOL result;
	InvokeHelper(0x4b8, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetShowChartTipValues(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x4b8, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

CString _Application::GetStandardFont()
{
	CString result;
	InvokeHelper(0x39c, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Application::SetStandardFont(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x39c, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

double _Application::GetStandardFontSize()
{
	double result;
	InvokeHelper(0x39d, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, nullptr);
	return result;
}

void _Application::SetStandardFontSize(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x39d, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 newValue);
}

CString _Application::GetStartupPath()
{
	CString result;
	InvokeHelper(0x181, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

VARIANT _Application::GetStatusBar()
{
	VARIANT result;
	InvokeHelper(0x182, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void _Application::SetStatusBar(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x182, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

CString _Application::GetTemplatesPath()
{
	CString result;
	InvokeHelper(0x17d, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

BOOL _Application::GetShowToolTips()
{
	BOOL result;
	InvokeHelper(0x183, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetShowToolTips(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x183, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

double _Application::GetTop()
{
	double result;
	InvokeHelper(0x7e, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, nullptr);
	return result;
}

void _Application::SetTop(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x7e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 newValue);
}

long _Application::GetDefaultSaveFormat()
{
	long result;
	InvokeHelper(0x4b9, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetDefaultSaveFormat(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x4b9, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

CString _Application::GetTransitionMenuKey()
{
	CString result;
	InvokeHelper(0x136, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Application::SetTransitionMenuKey(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x136, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

long _Application::GetTransitionMenuKeyAction()
{
	long result;
	InvokeHelper(0x137, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetTransitionMenuKeyAction(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x137, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

BOOL _Application::GetTransitionNavigKeys()
{
	BOOL result;
	InvokeHelper(0x138, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetTransitionNavigKeys(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x138, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Application::Undo()
{
	InvokeHelper(0x12f, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

double _Application::GetUsableHeight()
{
	double result;
	InvokeHelper(0x185, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, nullptr);
	return result;
}

double _Application::GetUsableWidth()
{
	double result;
	InvokeHelper(0x186, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, nullptr);
	return result;
}

BOOL _Application::GetUserControl()
{
	BOOL result;
	InvokeHelper(0x4ba, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetUserControl(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x4ba, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

CString _Application::GetUserName_()
{
	CString result;
	InvokeHelper(0x187, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Application::SetUserName(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x187, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

CString _Application::GetValue()
{
	CString result;
	InvokeHelper(0x6, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Application::GetVbe()
{
	LPDISPATCH result;
	InvokeHelper(0x4bb, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

CString _Application::GetVersion()
{
	CString result;
	InvokeHelper(0x188, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

BOOL _Application::GetVisible()
{
	BOOL result;
	InvokeHelper(0x22e, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetVisible(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x22e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Application::Volatile(const VARIANT& Volatile)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x314, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Volatile);
}

void _Application::Wait(const VARIANT& Time)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x189, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Time);
}

double _Application::GetWidth()
{
	double result;
	InvokeHelper(0x7a, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, nullptr);
	return result;
}

void _Application::SetWidth(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x7a, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 newValue);
}

BOOL _Application::GetWindowsForPens()
{
	BOOL result;
	InvokeHelper(0x18b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

long _Application::GetWindowState()
{
	long result;
	InvokeHelper(0x18c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetWindowState(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x18c, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

long _Application::GetUILanguage()
{
	long result;
	InvokeHelper(0x2, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetUILanguage(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x2, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

long _Application::GetDefaultSheetDirection()
{
	long result;
	InvokeHelper(0xe5, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetDefaultSheetDirection(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xe5, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

long _Application::GetCursorMovement()
{
	long result;
	InvokeHelper(0xe8, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetCursorMovement(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xe8, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

long _Application::GetControlCharacters()
{
	long result;
	InvokeHelper(0xe9, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Application::SetControlCharacters(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xe9, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

BOOL _Application::GetEnableEvents()
{
	BOOL result;
	InvokeHelper(0x4bc, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Application::SetEnableEvents(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x4bc, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}


/////////////////////////////////////////////////////////////////////////////
// _Workbook properties

/////////////////////////////////////////////////////////////////////////////
// _Workbook operations

LPDISPATCH _Workbook::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long _Workbook::GetCreator()
{
	long result;
	InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetAcceptLabelsInFormulas()
{
	BOOL result;
	InvokeHelper(0x5a1, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetAcceptLabelsInFormulas(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x5a1, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Workbook::Activate()
{
	InvokeHelper(0x130, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH _Workbook::GetActiveChart()
{
	LPDISPATCH result;
	InvokeHelper(0xb7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::GetActiveSheet()
{
	LPDISPATCH result;
	InvokeHelper(0x133, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long _Workbook::GetAutoUpdateFrequency()
{
	long result;
	InvokeHelper(0x5a2, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetAutoUpdateFrequency(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x5a2, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

BOOL _Workbook::GetAutoUpdateSaveChanges()
{
	BOOL result;
	InvokeHelper(0x5a3, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetAutoUpdateSaveChanges(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x5a3, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

long _Workbook::GetChangeHistoryDuration()
{
	long result;
	InvokeHelper(0x5a4, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetChangeHistoryDuration(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x5a4, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

LPDISPATCH _Workbook::GetBuiltinDocumentProperties()
{
	LPDISPATCH result;
	InvokeHelper(0x498, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Workbook::ChangeFileAccess(long Mode, const VARIANT& WritePassword, const VARIANT& Notify)
{
	static BYTE parms[] =
		VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x3dd, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Mode, &WritePassword, &Notify);
}

void _Workbook::ChangeLink(LPCTSTR Name, LPCTSTR NewName, long Type)
{
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR VTS_I4;
	InvokeHelper(0x322, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Name, NewName, Type);
}

LPDISPATCH _Workbook::GetCharts()
{
	LPDISPATCH result;
	InvokeHelper(0x79, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Workbook::Close(const VARIANT& SaveChanges, const VARIANT& Filename, const VARIANT& RouteWorkbook)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x115, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &SaveChanges, &Filename, &RouteWorkbook);
}

CString _Workbook::GetCodeName()
{
	CString result;
	InvokeHelper(0x55d, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

CString _Workbook::Get_CodeName()
{
	CString result;
	InvokeHelper(0x80010000, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Workbook::Set_CodeName(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x80010000, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

VARIANT _Workbook::GetColors(const VARIANT& Index)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x11e, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, parms,
		&Index);
	return result;
}

void _Workbook::SetColors(const VARIANT& Index, const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x11e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &Index, &newValue);
}

LPDISPATCH _Workbook::GetCommandBars()
{
	LPDISPATCH result;
	InvokeHelper(0x59f, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long _Workbook::GetConflictResolution()
{
	long result;
	InvokeHelper(0x497, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetConflictResolution(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x497, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

LPDISPATCH _Workbook::GetContainer()
{
	LPDISPATCH result;
	InvokeHelper(0x4a6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetCreateBackup()
{
	BOOL result;
	InvokeHelper(0x11f, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::GetCustomDocumentProperties()
{
	LPDISPATCH result;
	InvokeHelper(0x499, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetDate1904()
{
	BOOL result;
	InvokeHelper(0x193, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetDate1904(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x193, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Workbook::DeleteNumberFormat(LPCTSTR NumberFormat)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x18d, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 NumberFormat);
}

long _Workbook::GetDisplayDrawingObjects()
{
	long result;
	InvokeHelper(0x194, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetDisplayDrawingObjects(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x194, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

BOOL _Workbook::ExclusiveAccess()
{
	BOOL result;
	InvokeHelper(0x490, DISPATCH_METHOD, VT_BOOL, (void*)&result, nullptr);
	return result;
}

long _Workbook::GetFileFormat()
{
	long result;
	InvokeHelper(0x120, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Workbook::ForwardMailer()
{
	InvokeHelper(0x3cd, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

CString _Workbook::GetFullName()
{
	CString result;
	InvokeHelper(0x121, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetHasPassword()
{
	BOOL result;
	InvokeHelper(0x122, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetHasRoutingSlip()
{
	BOOL result;
	InvokeHelper(0x3b6, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetHasRoutingSlip(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x3b6, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Workbook::GetIsAddin()
{
	BOOL result;
	InvokeHelper(0x5a5, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetIsAddin(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x5a5, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

VARIANT _Workbook::LinkInfo(LPCTSTR Name, long LinkInfo, const VARIANT& Type, const VARIANT& EditionRef)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_BSTR VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x327, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		Name, LinkInfo, &Type, &EditionRef);
	return result;
}

VARIANT _Workbook::LinkSources(const VARIANT& Type)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x328, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&Type);
	return result;
}

LPDISPATCH _Workbook::GetMailer()
{
	LPDISPATCH result;
	InvokeHelper(0x3d3, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Workbook::MergeWorkbook(const VARIANT& Filename)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x5a6, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Filename);
}

BOOL _Workbook::GetMultiUserEditing()
{
	BOOL result;
	InvokeHelper(0x491, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

CString _Workbook::GetName()
{
	CString result;
	InvokeHelper(0x6e, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::GetNames()
{
	LPDISPATCH result;
	InvokeHelper(0x1ba, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::NewWindow()
{
	LPDISPATCH result;
	InvokeHelper(0x118, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Workbook::OpenLinks(LPCTSTR Name, const VARIANT& ReadOnly, const VARIANT& Type)
{
	static BYTE parms[] =
		VTS_BSTR VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x323, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Name, &ReadOnly, &Type);
}

CString _Workbook::GetPath()
{
	CString result;
	InvokeHelper(0x123, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetPersonalViewListSettings()
{
	BOOL result;
	InvokeHelper(0x5a7, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetPersonalViewListSettings(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x5a7, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Workbook::GetPersonalViewPrintSettings()
{
	BOOL result;
	InvokeHelper(0x5a8, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetPersonalViewPrintSettings(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x5a8, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

LPDISPATCH _Workbook::PivotCaches()
{
	LPDISPATCH result;
	InvokeHelper(0x5a9, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Workbook::Post(const VARIANT& DestName)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x48e, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &DestName);
}

BOOL _Workbook::GetPrecisionAsDisplayed()
{
	BOOL result;
	InvokeHelper(0x195, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetPrecisionAsDisplayed(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x195, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Workbook::PrintOut(const VARIANT& From, const VARIANT& To, const VARIANT& Copies, const VARIANT& Preview, const VARIANT& ActivePrinter, const VARIANT& PrintToFile, const VARIANT& Collate)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x389, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &From, &To, &Copies, &Preview, &ActivePrinter, &PrintToFile, &Collate);
}

void _Workbook::PrintPreview(const VARIANT& EnableChanges)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x119, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &EnableChanges);
}

void _Workbook::Protect(const VARIANT& Password, const VARIANT& Structure, const VARIANT& Windows)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x11a, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Password, &Structure, &Windows);
}

void _Workbook::ProtectSharing(const VARIANT& Filename, const VARIANT& Password, const VARIANT& WriteResPassword, const VARIANT& ReadOnlyRecommended, const VARIANT& CreateBackup, const VARIANT& SharingPassword)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x5aa, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Filename, &Password, &WriteResPassword, &ReadOnlyRecommended, &CreateBackup, &SharingPassword);
}

BOOL _Workbook::GetProtectStructure()
{
	BOOL result;
	InvokeHelper(0x24c, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetProtectWindows()
{
	BOOL result;
	InvokeHelper(0x127, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetReadOnly()
{
	BOOL result;
	InvokeHelper(0x128, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetReadOnlyRecommended()
{
	BOOL result;
	InvokeHelper(0x129, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::RefreshAll()
{
	InvokeHelper(0x5ac, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void _Workbook::Reply()
{
	InvokeHelper(0x3d1, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void _Workbook::ReplyAll()
{
	InvokeHelper(0x3d2, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void _Workbook::RemoveUser(long Index)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x5ad, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Index);
}

long _Workbook::GetRevisionNumber()
{
	long result;
	InvokeHelper(0x494, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Workbook::Route()
{
	InvokeHelper(0x3b2, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

BOOL _Workbook::GetRouted()
{
	BOOL result;
	InvokeHelper(0x3b7, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::GetRoutingSlip()
{
	LPDISPATCH result;
	InvokeHelper(0x3b5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Workbook::RunAutoMacros(long Which)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x27a, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Which);
}

void _Workbook::Save()
{
	InvokeHelper(0x11b, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void _Workbook::SaveAs(const VARIANT& Filename, const VARIANT& FileFormat, const VARIANT& Password, const VARIANT& WriteResPassword, const VARIANT& ReadOnlyRecommended, const VARIANT& CreateBackup, long AccessMode, const VARIANT& ConflictResolution,
		const VARIANT& AddToMru, const VARIANT& TextCodepage, const VARIANT& TextVisualLayout)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x11c, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Filename, &FileFormat, &Password, &WriteResPassword, &ReadOnlyRecommended, &CreateBackup, AccessMode, &ConflictResolution, &AddToMru, &TextCodepage, &TextVisualLayout);
}

void _Workbook::SaveCopyAs(const VARIANT& Filename)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xaf, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Filename);
}

BOOL _Workbook::GetSaved()
{
	BOOL result;
	InvokeHelper(0x12a, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetSaved(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x12a, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Workbook::GetSaveLinkValues()
{
	BOOL result;
	InvokeHelper(0x196, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetSaveLinkValues(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x196, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Workbook::SendMail(const VARIANT& Recipients, const VARIANT& Subject, const VARIANT& ReturnReceipt)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x3b3, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Recipients, &Subject, &ReturnReceipt);
}

void _Workbook::SendMailer(const VARIANT& FileFormat, long Priority)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_I4;
	InvokeHelper(0x3d4, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &FileFormat, Priority);
}

void _Workbook::SetLinkOnData(LPCTSTR Name, const VARIANT& Procedure)
{
	static BYTE parms[] =
		VTS_BSTR VTS_VARIANT;
	InvokeHelper(0x329, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Name, &Procedure);
}

LPDISPATCH _Workbook::GetSheets()
{
	LPDISPATCH result;
	InvokeHelper(0x1e5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetShowConflictHistory()
{
	BOOL result;
	InvokeHelper(0x493, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetShowConflictHistory(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x493, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

LPDISPATCH _Workbook::GetStyles()
{
	LPDISPATCH result;
	InvokeHelper(0x1ed, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Workbook::Unprotect(const VARIANT& Password)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x11d, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Password);
}

void _Workbook::UnprotectSharing(const VARIANT& SharingPassword)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x5af, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &SharingPassword);
}

void _Workbook::UpdateFromFile()
{
	InvokeHelper(0x3e3, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void _Workbook::UpdateLink(const VARIANT& Name, const VARIANT& Type)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x324, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Name, &Type);
}

BOOL _Workbook::GetUpdateRemoteReferences()
{
	BOOL result;
	InvokeHelper(0x19b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetUpdateRemoteReferences(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x19b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

VARIANT _Workbook::GetUserStatus()
{
	VARIANT result;
	InvokeHelper(0x495, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::GetCustomViews()
{
	LPDISPATCH result;
	InvokeHelper(0x5b0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::GetWindows()
{
	LPDISPATCH result;
	InvokeHelper(0x1ae, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::GetWorksheets()
{
	LPDISPATCH result;
	InvokeHelper(0x1ee, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetWriteReserved()
{
	BOOL result;
	InvokeHelper(0x12b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

CString _Workbook::GetWriteReservedBy()
{
	CString result;
	InvokeHelper(0x12c, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::GetExcel4IntlMacroSheets()
{
	LPDISPATCH result;
	InvokeHelper(0x245, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Workbook::GetExcel4MacroSheets()
{
	LPDISPATCH result;
	InvokeHelper(0x243, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

BOOL _Workbook::GetTemplateRemoveExtData()
{
	BOOL result;
	InvokeHelper(0x5b1, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetTemplateRemoveExtData(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x5b1, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Workbook::HighlightChangesOptions(const VARIANT& When, const VARIANT& Who, const VARIANT& Where)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x5b2, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &When, &Who, &Where);
}

BOOL _Workbook::GetHighlightChangesOnScreen()
{
	BOOL result;
	InvokeHelper(0x5b5, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetHighlightChangesOnScreen(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x5b5, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Workbook::GetKeepChangeHistory()
{
	BOOL result;
	InvokeHelper(0x5b6, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetKeepChangeHistory(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x5b6, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Workbook::GetListChangesOnNewSheet()
{
	BOOL result;
	InvokeHelper(0x5b7, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Workbook::SetListChangesOnNewSheet(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x5b7, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Workbook::PurgeChangeHistoryNow(long Days, const VARIANT& SharingPassword)
{
	static BYTE parms[] =
		VTS_I4 VTS_VARIANT;
	InvokeHelper(0x5b8, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Days, &SharingPassword);
}

void _Workbook::AcceptAllChanges(const VARIANT& When, const VARIANT& Who, const VARIANT& Where)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x5ba, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &When, &Who, &Where);
}

void _Workbook::RejectAllChanges(const VARIANT& When, const VARIANT& Who, const VARIANT& Where)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x5bb, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &When, &Who, &Where);
}

void _Workbook::ResetColors()
{
	InvokeHelper(0x5bc, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH _Workbook::GetVBProject()
{
	LPDISPATCH result;
	InvokeHelper(0x5bd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Workbook::FollowHyperlink(LPCTSTR Address, const VARIANT& SubAddress, const VARIANT& NewWindow, const VARIANT& AddHistory, const VARIANT& ExtraInfo, const VARIANT& Method, const VARIANT& HeaderInfo)
{
	static BYTE parms[] =
		VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x5be, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Address, &SubAddress, &NewWindow, &AddHistory, &ExtraInfo, &Method, &HeaderInfo);
}

void _Workbook::AddToFavorites()
{
	InvokeHelper(0x5c4, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

BOOL _Workbook::GetIsInplace()
{
	BOOL result;
	InvokeHelper(0x6f4, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}


/////////////////////////////////////////////////////////////////////////////
// _Worksheet properties

/////////////////////////////////////////////////////////////////////////////
// _Worksheet operations

LPDISPATCH _Worksheet::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long _Worksheet::GetCreator()
{
	long result;
	InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Worksheet::Activate()
{
	InvokeHelper(0x130, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void _Worksheet::Copy(const VARIANT& Before, const VARIANT& After)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x227, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Before, &After);
}

void _Worksheet::Delete()
{
	InvokeHelper(0x75, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

CString _Worksheet::GetCodeName()
{
	CString result;
	InvokeHelper(0x55d, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

CString _Worksheet::Get_CodeName()
{
	CString result;
	InvokeHelper(0x80010000, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Worksheet::Set_CodeName(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x80010000, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

long _Worksheet::GetIndex()
{
	long result;
	InvokeHelper(0x1e6, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Worksheet::Move(const VARIANT& Before, const VARIANT& After)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x27d, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Before, &After);
}

CString _Worksheet::GetName()
{
	CString result;
	InvokeHelper(0x6e, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetName(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x6e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

LPDISPATCH _Worksheet::GetNext()
{
	LPDISPATCH result;
	InvokeHelper(0x1f6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::GetPageSetup()
{
	LPDISPATCH result;
	InvokeHelper(0x3e6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::GetPrevious()
{
	LPDISPATCH result;
	InvokeHelper(0x1f7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Worksheet::PrintOut(const VARIANT& From, const VARIANT& To, const VARIANT& Copies, const VARIANT& Preview, const VARIANT& ActivePrinter, const VARIANT& PrintToFile, const VARIANT& Collate)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x389, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &From, &To, &Copies, &Preview, &ActivePrinter, &PrintToFile, &Collate);
}

void _Worksheet::PrintPreview(const VARIANT& EnableChanges)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x119, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &EnableChanges);
}

void _Worksheet::Protect(const VARIANT& Password, const VARIANT& DrawingObjects, const VARIANT& Contents, const VARIANT& Scenarios, const VARIANT& UserInterfaceOnly)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x11a, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Password, &DrawingObjects, &Contents, &Scenarios, &UserInterfaceOnly);
}

BOOL _Worksheet::GetProtectContents()
{
	BOOL result;
	InvokeHelper(0x124, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

BOOL _Worksheet::GetProtectDrawingObjects()
{
	BOOL result;
	InvokeHelper(0x125, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

BOOL _Worksheet::GetProtectionMode()
{
	BOOL result;
	InvokeHelper(0x487, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

BOOL _Worksheet::GetProtectScenarios()
{
	BOOL result;
	InvokeHelper(0x126, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SaveAs(LPCTSTR Filename, const VARIANT& FileFormat, const VARIANT& Password, const VARIANT& WriteResPassword, const VARIANT& ReadOnlyRecommended, const VARIANT& CreateBackup, const VARIANT& AddToMru, const VARIANT& TextCodepage,
		const VARIANT& TextVisualLayout)
{
	static BYTE parms[] =
		VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x11c, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Filename, &FileFormat, &Password, &WriteResPassword, &ReadOnlyRecommended, &CreateBackup, &AddToMru, &TextCodepage, &TextVisualLayout);
}

void _Worksheet::Select(const VARIANT& Replace)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xeb, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Replace);
}

void _Worksheet::Unprotect(const VARIANT& Password)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x11d, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Password);
}

long _Worksheet::GetVisible()
{
	long result;
	InvokeHelper(0x22e, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetVisible(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x22e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

LPDISPATCH _Worksheet::GetShapes()
{
	LPDISPATCH result;
	InvokeHelper(0x561, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

BOOL _Worksheet::GetTransitionExpEval()
{
	BOOL result;
	InvokeHelper(0x191, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetTransitionExpEval(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x191, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Worksheet::GetAutoFilterMode()
{
	BOOL result;
	InvokeHelper(0x318, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetAutoFilterMode(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x318, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

void _Worksheet::SetBackgroundPicture(LPCTSTR Filename)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x4a4, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Filename);
}

void _Worksheet::Calculate()
{
	InvokeHelper(0x117, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

BOOL _Worksheet::GetEnableCalculation()
{
	BOOL result;
	InvokeHelper(0x590, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetEnableCalculation(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x590, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

LPDISPATCH _Worksheet::GetCells()
{
	LPDISPATCH result;
	InvokeHelper(0xee, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::ChartObjects(const VARIANT& Index)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x424, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&Index);
	return result;
}

void _Worksheet::CheckSpelling(const VARIANT& CustomDictionary, const VARIANT& IgnoreUppercase, const VARIANT& AlwaysSuggest, const VARIANT& IgnoreInitialAlefHamza, const VARIANT& IgnoreFinalYaa, const VARIANT& SpellScript)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1f9, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &CustomDictionary, &IgnoreUppercase, &AlwaysSuggest, &IgnoreInitialAlefHamza, &IgnoreFinalYaa, &SpellScript);
}

LPDISPATCH _Worksheet::GetCircularReference()
{
	LPDISPATCH result;
	InvokeHelper(0x42d, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Worksheet::ClearArrows()
{
	InvokeHelper(0x3ca, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH _Worksheet::GetColumns()
{
	LPDISPATCH result;
	InvokeHelper(0xf1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long _Worksheet::GetConsolidationFunction()
{
	long result;
	InvokeHelper(0x315, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

VARIANT _Worksheet::GetConsolidationOptions()
{
	VARIANT result;
	InvokeHelper(0x316, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

VARIANT _Worksheet::GetConsolidationSources()
{
	VARIANT result;
	InvokeHelper(0x317, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

BOOL _Worksheet::GetEnableAutoFilter()
{
	BOOL result;
	InvokeHelper(0x484, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetEnableAutoFilter(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x484, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

long _Worksheet::GetEnableSelection()
{
	long result;
	InvokeHelper(0x591, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetEnableSelection(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x591, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

BOOL _Worksheet::GetEnableOutlining()
{
	BOOL result;
	InvokeHelper(0x485, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetEnableOutlining(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x485, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

BOOL _Worksheet::GetEnablePivotTable()
{
	BOOL result;
	InvokeHelper(0x486, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetEnablePivotTable(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x486, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

VARIANT _Worksheet::Evaluate(const VARIANT& Name)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x1, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&Name);
	return result;
}

VARIANT _Worksheet::_Evaluate(const VARIANT& Name)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xfffffffb, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&Name);
	return result;
}

BOOL _Worksheet::GetFilterMode()
{
	BOOL result;
	InvokeHelper(0x320, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Worksheet::ResetAllPageBreaks()
{
	InvokeHelper(0x592, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH _Worksheet::GetNames()
{
	LPDISPATCH result;
	InvokeHelper(0x1ba, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::OLEObjects(const VARIANT& Index)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x31f, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&Index);
	return result;
}

LPDISPATCH _Worksheet::GetOutline()
{
	LPDISPATCH result;
	InvokeHelper(0x66, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Worksheet::Paste(const VARIANT& Destination, const VARIANT& Link)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xd3, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Destination, &Link);
}

void _Worksheet::PasteSpecial(const VARIANT& Format, const VARIANT& Link, const VARIANT& DisplayAsIcon, const VARIANT& IconFileName, const VARIANT& IconIndex, const VARIANT& IconLabel)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x403, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Format, &Link, &DisplayAsIcon, &IconFileName, &IconIndex, &IconLabel);
}

LPDISPATCH _Worksheet::PivotTables(const VARIANT& Index)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x2b2, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&Index);
	return result;
}

LPDISPATCH _Worksheet::PivotTableWizard(const VARIANT& SourceType, const VARIANT& SourceData, const VARIANT& TableDestination, const VARIANT& TableName, const VARIANT& RowGrand, const VARIANT& ColumnGrand, const VARIANT& SaveData,
		const VARIANT& HasAutoFormat, const VARIANT& AutoPage, const VARIANT& Reserved, const VARIANT& BackgroundQuery, const VARIANT& OptimizeCache, const VARIANT& PageFieldOrder, const VARIANT& PageFieldWrapCount, const VARIANT& ReadData,
		const VARIANT& Connection)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x2ac, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&SourceType, &SourceData, &TableDestination, &TableName, &RowGrand, &ColumnGrand, &SaveData, &HasAutoFormat, &AutoPage, &Reserved, &BackgroundQuery, &OptimizeCache, &PageFieldOrder, &PageFieldWrapCount, &ReadData, &Connection);
	return result;
}

LPDISPATCH _Worksheet::GetRange(const VARIANT& Cell1, const VARIANT& Cell2)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xc5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		&Cell1, &Cell2);
	return result;
}

LPDISPATCH _Worksheet::GetRows()
{
	LPDISPATCH result;
	InvokeHelper(0x102, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::Scenarios(const VARIANT& Index)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x38c, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&Index);
	return result;
}

CString _Worksheet::GetScrollArea()
{
	CString result;
	InvokeHelper(0x599, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetScrollArea(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x599, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 lpszNewValue);
}

void _Worksheet::ShowAllData()
{
	InvokeHelper(0x31a, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void _Worksheet::ShowDataForm()
{
	InvokeHelper(0x199, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

double _Worksheet::GetStandardHeight()
{
	double result;
	InvokeHelper(0x197, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, nullptr);
	return result;
}

double _Worksheet::GetStandardWidth()
{
	double result;
	InvokeHelper(0x198, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetStandardWidth(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x198, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 newValue);
}

BOOL _Worksheet::GetTransitionFormEntry()
{
	BOOL result;
	InvokeHelper(0x192, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetTransitionFormEntry(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x192, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

long _Worksheet::GetType()
{
	long result;
	InvokeHelper(0x6c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::GetUsedRange()
{
	LPDISPATCH result;
	InvokeHelper(0x19c, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::GetHPageBreaks()
{
	LPDISPATCH result;
	InvokeHelper(0x58a, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::GetVPageBreaks()
{
	LPDISPATCH result;
	InvokeHelper(0x58b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::GetQueryTables()
{
	LPDISPATCH result;
	InvokeHelper(0x59a, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

BOOL _Worksheet::GetDisplayPageBreaks()
{
	BOOL result;
	InvokeHelper(0x59b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, nullptr);
	return result;
}

void _Worksheet::SetDisplayPageBreaks(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x59b, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 bNewValue);
}

LPDISPATCH _Worksheet::GetComments()
{
	LPDISPATCH result;
	InvokeHelper(0x23f, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH _Worksheet::GetHyperlinks()
{
	LPDISPATCH result;
	InvokeHelper(0x571, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void _Worksheet::ClearCircles()
{
	InvokeHelper(0x59c, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void _Worksheet::CircleInvalid()
{
	InvokeHelper(0x59d, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH _Worksheet::GetAutoFilter()
{
	LPDISPATCH result;
	InvokeHelper(0x319, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}


/////////////////////////////////////////////////////////////////////////////
// Range properties

/////////////////////////////////////////////////////////////////////////////
// Range operations

LPDISPATCH Range::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long Range::GetCreator()
{
	long result;
	InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void Range::Activate()
{
	InvokeHelper(0x130, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

VARIANT Range::GetAddIndent()
{
	VARIANT result;
	InvokeHelper(0x427, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetAddIndent(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x427, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

CString Range::GetAddress(const VARIANT& RowAbsolute, const VARIANT& ColumnAbsolute, long ReferenceStyle, const VARIANT& External, const VARIANT& RelativeTo)
{
	CString result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xec, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, parms,
		&RowAbsolute, &ColumnAbsolute, ReferenceStyle, &External, &RelativeTo);
	return result;
}

CString Range::GetAddressLocal(const VARIANT& RowAbsolute, const VARIANT& ColumnAbsolute, long ReferenceStyle, const VARIANT& External, const VARIANT& RelativeTo)
{
	CString result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1b5, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, parms,
		&RowAbsolute, &ColumnAbsolute, ReferenceStyle, &External, &RelativeTo);
	return result;
}

void Range::AdvancedFilter(long Action, const VARIANT& CriteriaRange, const VARIANT& CopyToRange, const VARIANT& Unique)
{
	static BYTE parms[] =
		VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x36c, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Action, &CriteriaRange, &CopyToRange, &Unique);
}

void Range::ApplyNames(const VARIANT& Names, const VARIANT& IgnoreRelativeAbsolute, const VARIANT& UseRowColumnNames, const VARIANT& OmitColumn, const VARIANT& OmitRow, long Order, const VARIANT& AppendLast)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_I4 VTS_VARIANT;
	InvokeHelper(0x1b9, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Names, &IgnoreRelativeAbsolute, &UseRowColumnNames, &OmitColumn, &OmitRow, Order, &AppendLast);
}

void Range::ApplyOutlineStyles()
{
	InvokeHelper(0x1c0, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH Range::GetAreas()
{
	LPDISPATCH result;
	InvokeHelper(0x238, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

CString Range::AutoComplete(LPCTSTR String)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x4a1, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		String);
	return result;
}

void Range::AutoFill(LPDISPATCH Destination, long Type)
{
	static BYTE parms[] =
		VTS_DISPATCH VTS_I4;
	InvokeHelper(0x1c1, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Destination, Type);
}

void Range::AutoFilter(const VARIANT& Field, const VARIANT& Criteria1, long Operator, const VARIANT& Criteria2, const VARIANT& VisibleDropDown)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x319, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Field, &Criteria1, Operator, &Criteria2, &VisibleDropDown);
}

void Range::AutoFit()
{
	InvokeHelper(0xed, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::AutoFormat(long Format, const VARIANT& Number, const VARIANT& Font, const VARIANT& Alignment, const VARIANT& Border, const VARIANT& Pattern, const VARIANT& Width)
{
	static BYTE parms[] =
		VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x72, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Format, &Number, &Font, &Alignment, &Border, &Pattern, &Width);
}

void Range::AutoOutline()
{
	InvokeHelper(0x40c, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::BorderAround(const VARIANT& LineStyle, long Weight, long ColorIndex, const VARIANT& Color)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_I4 VTS_I4 VTS_VARIANT;
	InvokeHelper(0x42b, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &LineStyle, Weight, ColorIndex, &Color);
}

LPDISPATCH Range::GetBorders()
{
	LPDISPATCH result;
	InvokeHelper(0x1b3, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void Range::Calculate()
{
	InvokeHelper(0x117, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH Range::GetCells()
{
	LPDISPATCH result;
	InvokeHelper(0xee, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetCharacters(const VARIANT& Start, const VARIANT& Length)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x25b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		&Start, &Length);
	return result;
}

void Range::CheckSpelling(const VARIANT& CustomDictionary, const VARIANT& IgnoreUppercase, const VARIANT& AlwaysSuggest, const VARIANT& IgnoreInitialAlefHamza, const VARIANT& IgnoreFinalYaa, const VARIANT& SpellScript)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1f9, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &CustomDictionary, &IgnoreUppercase, &AlwaysSuggest, &IgnoreInitialAlefHamza, &IgnoreFinalYaa, &SpellScript);
}

void Range::Clear()
{
	InvokeHelper(0x6f, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::ClearContents()
{
	InvokeHelper(0x71, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::ClearFormats()
{
	InvokeHelper(0x70, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::ClearNotes()
{
	InvokeHelper(0xef, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::ClearOutline()
{
	InvokeHelper(0x40d, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

long Range::GetColumn()
{
	long result;
	InvokeHelper(0xf0, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::ColumnDifferences(const VARIANT& Comparison)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x1fe, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&Comparison);
	return result;
}

LPDISPATCH Range::GetColumns()
{
	LPDISPATCH result;
	InvokeHelper(0xf1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetColumnWidth()
{
	VARIANT result;
	InvokeHelper(0xf2, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetColumnWidth(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xf2, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

void Range::Consolidate(const VARIANT& Sources, const VARIANT& Function, const VARIANT& TopRow, const VARIANT& LeftColumn, const VARIANT& CreateLinks)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1e2, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Sources, &Function, &TopRow, &LeftColumn, &CreateLinks);
}

void Range::Copy(const VARIANT& Destination)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x227, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Destination);
}

long Range::CopyFromRecordset(LPUNKNOWN Data, const VARIANT& MaxRows, const VARIANT& MaxColumns)
{
	long result;
	static BYTE parms[] =
		VTS_UNKNOWN VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x480, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		Data, &MaxRows, &MaxColumns);
	return result;
}

void Range::CopyPicture(long Appearance, long Format)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0xd5, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Appearance, Format);
}

long Range::GetCount()
{
	long result;
	InvokeHelper(0x76, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void Range::CreateNames(const VARIANT& Top, const VARIANT& Left, const VARIANT& Bottom, const VARIANT& Right)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1c9, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Top, &Left, &Bottom, &Right);
}

void Range::CreatePublisher(const VARIANT& Edition, long Appearance, const VARIANT& ContainsPICT, const VARIANT& ContainsBIFF, const VARIANT& ContainsRTF, const VARIANT& ContainsVALU)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1ca, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Edition, Appearance, &ContainsPICT, &ContainsBIFF, &ContainsRTF, &ContainsVALU);
}

LPDISPATCH Range::GetCurrentArray()
{
	LPDISPATCH result;
	InvokeHelper(0x1f5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetCurrentRegion()
{
	LPDISPATCH result;
	InvokeHelper(0xf3, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void Range::Cut(const VARIANT& Destination)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x235, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Destination);
}

void Range::DataSeries(const VARIANT& Rowcol, long Type, long Date, const VARIANT& Step, const VARIANT& Stop, const VARIANT& Trend)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1d0, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Rowcol, Type, Date, &Step, &Stop, &Trend);
}

VARIANT Range::Get_Default(const VARIANT& RowIndex, const VARIANT& ColumnIndex)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, parms,
		&RowIndex, &ColumnIndex);
	return result;
}

void Range::Set_Default(const VARIANT& RowIndex, const VARIANT& ColumnIndex, const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x0, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &RowIndex, &ColumnIndex, &newValue);
}

void Range::Delete(const VARIANT& Shift)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x75, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Shift);
}

LPDISPATCH Range::GetDependents()
{
	LPDISPATCH result;
	InvokeHelper(0x21f, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Range::DialogBox_()
{
	VARIANT result;
	InvokeHelper(0xf5, DISPATCH_METHOD, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetDirectDependents()
{
	LPDISPATCH result;
	InvokeHelper(0x221, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetDirectPrecedents()
{
	LPDISPATCH result;
	InvokeHelper(0x222, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Range::EditionOptions(long Type, long Option, const VARIANT& Name, const VARIANT& Reference, long Appearance, long ChartSize, const VARIANT& Format)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT VTS_I4 VTS_I4 VTS_VARIANT;
	InvokeHelper(0x46b, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		Type, Option, &Name, &Reference, Appearance, ChartSize, &Format);
	return result;
}

LPDISPATCH Range::GetEnd(long Direction)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1f4, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		Direction);
	return result;
}

LPDISPATCH Range::GetEntireColumn()
{
	LPDISPATCH result;
	InvokeHelper(0xf6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetEntireRow()
{
	LPDISPATCH result;
	InvokeHelper(0xf7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void Range::FillDown()
{
	InvokeHelper(0xf8, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::FillLeft()
{
	InvokeHelper(0xf9, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::FillRight()
{
	InvokeHelper(0xfa, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::FillUp()
{
	InvokeHelper(0xfb, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH Range::Find(const VARIANT& What, const VARIANT& After, const VARIANT& LookIn, const VARIANT& LookAt, const VARIANT& SearchOrder, long SearchDirection, const VARIANT& MatchCase, const VARIANT& MatchByte,
		const VARIANT& MatchControlCharacters, const VARIANT& MatchDiacritics, const VARIANT& MatchKashida, const VARIANT& MatchAlefHamza)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x18e, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&What, &After, &LookIn, &LookAt, &SearchOrder, SearchDirection, &MatchCase, &MatchByte, &MatchControlCharacters, &MatchDiacritics, &MatchKashida, &MatchAlefHamza);
	return result;
}

LPDISPATCH Range::FindNext(const VARIANT& After)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x18f, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&After);
	return result;
}

LPDISPATCH Range::FindPrevious(const VARIANT& After)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x190, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&After);
	return result;
}

LPDISPATCH Range::GetFont()
{
	LPDISPATCH result;
	InvokeHelper(0x92, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetFormula()
{
	VARIANT result;
	InvokeHelper(0x105, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetFormula(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x105, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetFormulaArray()
{
	VARIANT result;
	InvokeHelper(0x24a, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetFormulaArray(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x24a, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

long Range::GetFormulaLabel()
{
	long result;
	InvokeHelper(0x564, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void Range::SetFormulaLabel(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x564, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

VARIANT Range::GetFormulaHidden()
{
	VARIANT result;
	InvokeHelper(0x106, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetFormulaHidden(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x106, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetFormulaLocal()
{
	VARIANT result;
	InvokeHelper(0x107, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetFormulaLocal(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x107, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetFormulaR1C1()
{
	VARIANT result;
	InvokeHelper(0x108, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetFormulaR1C1(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x108, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetFormulaR1C1Local()
{
	VARIANT result;
	InvokeHelper(0x109, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetFormulaR1C1Local(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x109, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

void Range::FunctionWizard()
{
	InvokeHelper(0x23b, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

BOOL Range::GoalSeek(const VARIANT& Goal, LPDISPATCH ChangingCell)
{
	BOOL result;
	static BYTE parms[] =
		VTS_VARIANT VTS_DISPATCH;
	InvokeHelper(0x1d8, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		&Goal, ChangingCell);
	return result;
}

VARIANT Range::Group(const VARIANT& Start, const VARIANT& End, const VARIANT& By, const VARIANT& Periods)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x2e, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&Start, &End, &By, &Periods);
	return result;
}

VARIANT Range::GetHasArray()
{
	VARIANT result;
	InvokeHelper(0x10a, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetHasFormula()
{
	VARIANT result;
	InvokeHelper(0x10b, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetHeight()
{
	VARIANT result;
	InvokeHelper(0x7b, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetHidden()
{
	VARIANT result;
	InvokeHelper(0x10c, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetHidden(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x10c, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetHorizontalAlignment()
{
	VARIANT result;
	InvokeHelper(0x88, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetHorizontalAlignment(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x88, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetIndentLevel()
{
	VARIANT result;
	InvokeHelper(0xc9, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetIndentLevel(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xc9, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

void Range::InsertIndent(long InsertAmount)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x565, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 InsertAmount);
}

void Range::Insert(const VARIANT& Shift)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xfc, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Shift);
}

LPDISPATCH Range::GetInterior()
{
	LPDISPATCH result;
	InvokeHelper(0x81, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetItem(const VARIANT& RowIndex, const VARIANT& ColumnIndex)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xaa, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, parms,
		&RowIndex, &ColumnIndex);
	return result;
}

void Range::SetItem(const VARIANT& RowIndex, const VARIANT& ColumnIndex, const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xaa, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &RowIndex, &ColumnIndex, &newValue);
}

void Range::Justify()
{
	InvokeHelper(0x1ef, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

VARIANT Range::GetLeft()
{
	VARIANT result;
	InvokeHelper(0x7f, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

long Range::GetListHeaderRows()
{
	long result;
	InvokeHelper(0x4a3, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void Range::ListNames()
{
	InvokeHelper(0xfd, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

long Range::GetLocationInTable()
{
	long result;
	InvokeHelper(0x2b3, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetLocked()
{
	VARIANT result;
	InvokeHelper(0x10d, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetLocked(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x10d, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

void Range::Merge(const VARIANT& Across)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x234, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Across);
}

void Range::UnMerge()
{
	InvokeHelper(0x568, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH Range::GetMergeArea()
{
	LPDISPATCH result;
	InvokeHelper(0x569, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetMergeCells()
{
	VARIANT result;
	InvokeHelper(0xd0, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetMergeCells(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xd0, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetName()
{
	VARIANT result;
	InvokeHelper(0x6e, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetName(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x6e, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

void Range::NavigateArrow(const VARIANT& TowardPrecedent, const VARIANT& ArrowNumber, const VARIANT& LinkNumber)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x408, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &TowardPrecedent, &ArrowNumber, &LinkNumber);
}

LPUNKNOWN Range::Get_NewEnum()
{
	LPUNKNOWN result;
	InvokeHelper(0xfffffffc, DISPATCH_PROPERTYGET, VT_UNKNOWN, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetNext()
{
	LPDISPATCH result;
	InvokeHelper(0x1f6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

CString Range::NoteText(const VARIANT& Text, const VARIANT& Start, const VARIANT& Length)
{
	CString result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x467, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		&Text, &Start, &Length);
	return result;
}

VARIANT Range::GetNumberFormat()
{
	VARIANT result;
	InvokeHelper(0xc1, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetNumberFormat(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xc1, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetNumberFormatLocal()
{
	VARIANT result;
	InvokeHelper(0x449, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetNumberFormatLocal(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x449, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

LPDISPATCH Range::GetOffset(const VARIANT& RowOffset, const VARIANT& ColumnOffset)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xfe, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		&RowOffset, &ColumnOffset);
	return result;
}

VARIANT Range::GetOrientation()
{
	VARIANT result;
	InvokeHelper(0x86, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetOrientation(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x86, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetOutlineLevel()
{
	VARIANT result;
	InvokeHelper(0x10f, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetOutlineLevel(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x10f, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

long Range::GetPageBreak()
{
	long result;
	InvokeHelper(0xff, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void Range::SetPageBreak(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xff, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

void Range::Parse(const VARIANT& ParseLine, const VARIANT& Destination)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1dd, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &ParseLine, &Destination);
}

void Range::PasteSpecial(long Paste, long Operation, const VARIANT& SkipBlanks, const VARIANT& Transpose)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x403, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Paste, Operation, &SkipBlanks, &Transpose);
}

LPDISPATCH Range::GetPivotField()
{
	LPDISPATCH result;
	InvokeHelper(0x2db, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetPivotItem()
{
	LPDISPATCH result;
	InvokeHelper(0x2e4, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetPivotTable()
{
	LPDISPATCH result;
	InvokeHelper(0x2cc, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetPrecedents()
{
	LPDISPATCH result;
	InvokeHelper(0x220, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetPrefixCharacter()
{
	VARIANT result;
	InvokeHelper(0x1f8, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetPrevious()
{
	LPDISPATCH result;
	InvokeHelper(0x1f7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void Range::PrintOut(const VARIANT& From, const VARIANT& To, const VARIANT& Copies, const VARIANT& Preview, const VARIANT& ActivePrinter, const VARIANT& PrintToFile, const VARIANT& Collate)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x389, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &From, &To, &Copies, &Preview, &ActivePrinter, &PrintToFile, &Collate);
}

void Range::PrintPreview(const VARIANT& EnableChanges)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x119, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &EnableChanges);
}

LPDISPATCH Range::GetQueryTable()
{
	LPDISPATCH result;
	InvokeHelper(0x56a, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetRange(const VARIANT& Cell1, const VARIANT& Cell2)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xc5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		&Cell1, &Cell2);
	return result;
}

void Range::RemoveSubtotal()
{
	InvokeHelper(0x373, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

BOOL Range::Replace(const VARIANT& What, const VARIANT& Replacement, const VARIANT& LookAt, const VARIANT& SearchOrder, const VARIANT& MatchCase, const VARIANT& MatchByte, const VARIANT& MatchControlCharacters, const VARIANT& MatchDiacritics,
		const VARIANT& MatchKashida, const VARIANT& MatchAlefHamza)
{
	BOOL result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xe2, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		&What, &Replacement, &LookAt, &SearchOrder, &MatchCase, &MatchByte, &MatchControlCharacters, &MatchDiacritics, &MatchKashida, &MatchAlefHamza);
	return result;
}

LPDISPATCH Range::GetResize(const VARIANT& RowSize, const VARIANT& ColumnSize)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x100, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		&RowSize, &ColumnSize);
	return result;
}

long Range::GetRow()
{
	long result;
	InvokeHelper(0x101, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::RowDifferences(const VARIANT& Comparison)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x1ff, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&Comparison);
	return result;
}

VARIANT Range::GetRowHeight()
{
	VARIANT result;
	InvokeHelper(0x110, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetRowHeight(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x110, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

LPDISPATCH Range::GetRows()
{
	LPDISPATCH result;
	InvokeHelper(0x102, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Range::Run(const VARIANT& Arg1, const VARIANT& Arg2, const VARIANT& Arg3, const VARIANT& Arg4, const VARIANT& Arg5, const VARIANT& Arg6, const VARIANT& Arg7, const VARIANT& Arg8, const VARIANT& Arg9, const VARIANT& Arg10,
		const VARIANT& Arg11, const VARIANT& Arg12, const VARIANT& Arg13, const VARIANT& Arg14, const VARIANT& Arg15, const VARIANT& Arg16, const VARIANT& Arg17, const VARIANT& Arg18, const VARIANT& Arg19, const VARIANT& Arg20,
		const VARIANT& Arg21, const VARIANT& Arg22, const VARIANT& Arg23, const VARIANT& Arg24, const VARIANT& Arg25, const VARIANT& Arg26, const VARIANT& Arg27, const VARIANT& Arg28, const VARIANT& Arg29, const VARIANT& Arg30)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x103, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		&Arg1, &Arg2, &Arg3, &Arg4, &Arg5, &Arg6, &Arg7, &Arg8, &Arg9, &Arg10, &Arg11, &Arg12, &Arg13, &Arg14, &Arg15, &Arg16, &Arg17, &Arg18, &Arg19, &Arg20, &Arg21, &Arg22, &Arg23, &Arg24, &Arg25, &Arg26, &Arg27, &Arg28, &Arg29, &Arg30);
	return result;
}

void Range::Select()
{
	InvokeHelper(0xeb, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::Show()
{
	InvokeHelper(0x1f0, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::ShowDependents(const VARIANT& Remove)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x36d, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Remove);
}

VARIANT Range::GetShowDetail()
{
	VARIANT result;
	InvokeHelper(0x249, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetShowDetail(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x249, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

void Range::ShowErrors()
{
	InvokeHelper(0x36e, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

void Range::ShowPrecedents(const VARIANT& Remove)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x36f, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Remove);
}

VARIANT Range::GetShrinkToFit()
{
	VARIANT result;
	InvokeHelper(0xd1, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetShrinkToFit(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xd1, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

void Range::Sort(const VARIANT& Key1, long Order1, const VARIANT& Key2, const VARIANT& Type, long Order2, const VARIANT& Key3, long Order3, long Header, const VARIANT& OrderCustom, const VARIANT& MatchCase, long Orientation, long SortMethod,
		const VARIANT& IgnoreControlCharacters, const VARIANT& IgnoreDiacritics, const VARIANT& IgnoreKashida)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT VTS_I4 VTS_VARIANT VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x370, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Key1, Order1, &Key2, &Type, Order2, &Key3, Order3, Header, &OrderCustom, &MatchCase, Orientation, SortMethod, &IgnoreControlCharacters, &IgnoreDiacritics, &IgnoreKashida);
}

void Range::SortSpecial(long SortMethod, const VARIANT& Key1, long Order1, const VARIANT& Type, const VARIANT& Key2, long Order2, const VARIANT& Key3, long Order3, long Header, const VARIANT& OrderCustom, const VARIANT& MatchCase, long Orientation)
{
	static BYTE parms[] =
		VTS_I4 VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT VTS_I4 VTS_VARIANT VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT VTS_I4;
	InvokeHelper(0x371, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 SortMethod, &Key1, Order1, &Type, &Key2, Order2, &Key3, Order3, Header, &OrderCustom, &MatchCase, Orientation);
}

LPDISPATCH Range::GetSoundNote()
{
	LPDISPATCH result;
	InvokeHelper(0x394, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::SpecialCells(long Type, const VARIANT& Value)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_I4 VTS_VARIANT;
	InvokeHelper(0x19a, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		Type, &Value);
	return result;
}

VARIANT Range::GetStyle()
{
	VARIANT result;
	InvokeHelper(0x104, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetStyle(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x104, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

void Range::SubscribeTo(LPCTSTR Edition, long Format)
{
	static BYTE parms[] =
		VTS_BSTR VTS_I4;
	InvokeHelper(0x1e1, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 Edition, Format);
}

void Range::Subtotal(long GroupBy, long Function, const VARIANT& TotalList, const VARIANT& Replace, const VARIANT& PageBreaks, long SummaryBelowData)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_I4;
	InvokeHelper(0x372, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 GroupBy, Function, &TotalList, &Replace, &PageBreaks, SummaryBelowData);
}

VARIANT Range::GetSummary()
{
	VARIANT result;
	InvokeHelper(0x111, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::Table(const VARIANT& RowInput, const VARIANT& ColumnInput)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x1f1, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &RowInput, &ColumnInput);
}

VARIANT Range::GetText()
{
	VARIANT result;
	InvokeHelper(0x8a, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::TextToColumns(const VARIANT& Destination, long DataType, long TextQualifier, const VARIANT& ConsecutiveDelimiter, const VARIANT& Tab, const VARIANT& Semicolon, const VARIANT& Comma, const VARIANT& Space, const VARIANT& Other,
		const VARIANT& OtherChar, const VARIANT& FieldInfo)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x410, DISPATCH_METHOD, VT_EMPTY, nullptr, parms,
		 &Destination, DataType, TextQualifier, &ConsecutiveDelimiter, &Tab, &Semicolon, &Comma, &Space, &Other, &OtherChar, &FieldInfo);
}

VARIANT Range::GetTop()
{
	VARIANT result;
	InvokeHelper(0x7e, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::Ungroup()
{
	InvokeHelper(0xf4, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

VARIANT Range::GetUseStandardHeight()
{
	VARIANT result;
	InvokeHelper(0x112, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetUseStandardHeight(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x112, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetUseStandardWidth()
{
	VARIANT result;
	InvokeHelper(0x113, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetUseStandardWidth(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x113, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

LPDISPATCH Range::GetValidation()
{
	LPDISPATCH result;
	InvokeHelper(0x56b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetValue()
{
	VARIANT result;
	InvokeHelper(0x6, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetValue(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x6, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetValue2()
{
	VARIANT result;
	InvokeHelper(0x56c, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetValue2(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x56c, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetVerticalAlignment()
{
	VARIANT result;
	InvokeHelper(0x89, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetVerticalAlignment(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x89, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Range::GetWidth()
{
	VARIANT result;
	InvokeHelper(0x7a, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetWorksheet()
{
	LPDISPATCH result;
	InvokeHelper(0x15c, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Range::GetWrapText()
{
	VARIANT result;
	InvokeHelper(0x114, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Range::SetWrapText(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x114, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

LPDISPATCH Range::AddComment(const VARIANT& Text)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x56d, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms,
		&Text);
	return result;
}

LPDISPATCH Range::GetComment()
{
	LPDISPATCH result;
	InvokeHelper(0x38e, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

void Range::ClearComments()
{
	InvokeHelper(0x56e, DISPATCH_METHOD, VT_EMPTY, nullptr, nullptr);
}

LPDISPATCH Range::GetPhonetic()
{
	LPDISPATCH result;
	InvokeHelper(0x56f, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Range::GetFormatConditions()
{
	LPDISPATCH result;
	InvokeHelper(0x570, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long Range::GetReadingOrder()
{
	long result;
	InvokeHelper(0x3cf, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

void Range::SetReadingOrder(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x3cf, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 nNewValue);
}

LPDISPATCH Range::GetHyperlinks()
{
	LPDISPATCH result;
	InvokeHelper(0x571, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}


/////////////////////////////////////////////////////////////////////////////
// Border properties

/////////////////////////////////////////////////////////////////////////////
// Border operations

LPDISPATCH Border::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long Border::GetCreator()
{
	long result;
	InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Border::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Border::GetColor()
{
	VARIANT result;
	InvokeHelper(0x63, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Border::SetColor(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x63, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Border::GetColorIndex()
{
	VARIANT result;
	InvokeHelper(0x61, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Border::SetColorIndex(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x61, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Border::GetLineStyle()
{
	VARIANT result;
	InvokeHelper(0x77, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Border::SetLineStyle(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x77, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Border::GetWeight()
{
	VARIANT result;
	InvokeHelper(0x78, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Border::SetWeight(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x78, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}


/////////////////////////////////////////////////////////////////////////////
// Borders properties

/////////////////////////////////////////////////////////////////////////////
// Borders operations

LPDISPATCH Borders::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long Borders::GetCreator()
{
	long result;
	InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Borders::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Borders::GetColor()
{
	VARIANT result;
	InvokeHelper(0x63, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Borders::SetColor(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x63, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Borders::GetColorIndex()
{
	VARIANT result;
	InvokeHelper(0x61, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Borders::SetColorIndex(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x61, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

long Borders::GetCount()
{
	long result;
	InvokeHelper(0x76, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Borders::GetItem(long Index)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xaa, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		Index);
	return result;
}

VARIANT Borders::GetLineStyle()
{
	VARIANT result;
	InvokeHelper(0x77, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Borders::SetLineStyle(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x77, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

LPUNKNOWN Borders::Get_NewEnum()
{
	LPUNKNOWN result;
	InvokeHelper(0xfffffffc, DISPATCH_PROPERTYGET, VT_UNKNOWN, (void*)&result, nullptr);
	return result;
}

VARIANT Borders::GetValue()
{
	VARIANT result;
	InvokeHelper(0x6, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Borders::SetValue(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x6, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Borders::GetWeight()
{
	VARIANT result;
	InvokeHelper(0x78, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Borders::SetWeight(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x78, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

LPDISPATCH Borders::Get_Default(long Index)
{
	LPDISPATCH result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, parms,
		Index);
	return result;
}


/////////////////////////////////////////////////////////////////////////////
// Interior properties

/////////////////////////////////////////////////////////////////////////////
// Interior operations

LPDISPATCH Interior::GetApplication()
{
	LPDISPATCH result;
	InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

long Interior::GetCreator()
{
	long result;
	InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, nullptr);
	return result;
}

LPDISPATCH Interior::GetParent()
{
	LPDISPATCH result;
	InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, nullptr);
	return result;
}

VARIANT Interior::GetColor()
{
	VARIANT result;
	InvokeHelper(0x63, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Interior::SetColor(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x63, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Interior::GetColorIndex()
{
	VARIANT result;
	InvokeHelper(0x61, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Interior::SetColorIndex(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x61, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Interior::GetInvertIfNegative()
{
	VARIANT result;
	InvokeHelper(0x84, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Interior::SetInvertIfNegative(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x84, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Interior::GetPattern()
{
	VARIANT result;
	InvokeHelper(0x5f, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Interior::SetPattern(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x5f, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Interior::GetPatternColor()
{
	VARIANT result;
	InvokeHelper(0x64, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Interior::SetPatternColor(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x64, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}

VARIANT Interior::GetPatternColorIndex()
{
	VARIANT result;
	InvokeHelper(0x62, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, nullptr);
	return result;
}

void Interior::SetPatternColorIndex(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x62, DISPATCH_PROPERTYPUT, VT_EMPTY, nullptr, parms,
		 &newValue);
}
