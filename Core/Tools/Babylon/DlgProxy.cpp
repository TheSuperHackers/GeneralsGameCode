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

// DlgProxy.cpp : implementation file
//

#include "StdAfx.h"
#include "Babylon.h"
#include "DlgProxy.h"
#include "BabylonDlg.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBabylonDlgAutoProxy

IMPLEMENT_DYNCREATE(CBabylonDlgAutoProxy, CCmdTarget)

CBabylonDlgAutoProxy::CBabylonDlgAutoProxy()
{
	EnableAutomation();

	// To keep the application running as long as an automation
	//	object is active, the constructor calls AfxOleLockApp.
	AfxOleLockApp();

	// Get access to the dialog through the application's
	//  main window pointer.  Set the proxy's internal pointer
	//  to point to the dialog, and set the dialog's back pointer to
	//  this proxy.
	ASSERT (AfxGetApp()->m_pMainWnd != nullptr);
	ASSERT_VALID (AfxGetApp()->m_pMainWnd);
	ASSERT_KINDOF(CBabylonDlg, AfxGetApp()->m_pMainWnd);
	m_pDialog = (CBabylonDlg*) AfxGetApp()->m_pMainWnd;
	m_pDialog->m_pAutoProxy = this;
}

CBabylonDlgAutoProxy::~CBabylonDlgAutoProxy()
{
	// To terminate the application when all objects created with
	// 	with automation, the destructor calls AfxOleUnlockApp.
	//  Among other things, this will destroy the main dialog
	if (m_pDialog != nullptr)
		m_pDialog->m_pAutoProxy = nullptr;
	AfxOleUnlockApp();
}

void CBabylonDlgAutoProxy::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CBabylonDlgAutoProxy, CCmdTarget)
	//{{AFX_MSG_MAP(CBabylonDlgAutoProxy)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CBabylonDlgAutoProxy, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CBabylonDlgAutoProxy)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IBabylon to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the
//  dispinterface in the .ODL file.

// {2BF31248-3BA1-11D3-B9DA-006097B90D93}
static const IID IID_IBabylon =
{ 0x2bf31248, 0x3ba1, 0x11d3, { 0xb9, 0xda, 0x0, 0x60, 0x97, 0xb9, 0xd, 0x93 } };

BEGIN_INTERFACE_MAP(CBabylonDlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CBabylonDlgAutoProxy, IID_IBabylon, Dispatch)
END_INTERFACE_MAP()

// The IMPLEMENT_OLECREATE2 macro is defined in StdAfx.h of this project
// {2BF31246-3BA1-11D3-B9DA-006097B90D93}
IMPLEMENT_OLECREATE2(CBabylonDlgAutoProxy, "Babylon.Application", 0x2bf31246, 0x3ba1, 0x11d3, 0xb9, 0xda, 0x0, 0x60, 0x97, 0xb9, 0xd, 0x93)

/////////////////////////////////////////////////////////////////////////////
// CBabylonDlgAutoProxy message handlers
