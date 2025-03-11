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

// propedit.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "../include/propedit.h"

/////////////////////////////////////////////////////////////////////////////
// PropEdit dialog

#define NO_RESTRICT_KEYS

PropEdit::PropEdit(AsciiString* key, Dict::DataType* type, AsciiString* value, Bool valueOnly, CWnd *parent)
	: CDialog(PropEdit::IDD, parent), m_key(key), m_type(type), m_value(value), m_updating(0), m_valueOnly(valueOnly)
{
	//{{AFX_DATA_INIT(PropEdit)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void PropEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PropEdit)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PropEdit, CDialog)
	//{{AFX_MSG_MAP(PropEdit)
	ON_EN_CHANGE(IDC_KEYNAME, OnChangeKeyname)
	ON_CBN_EDITCHANGE(IDC_KEYTYPE, OnEditchangeKeytype)
	ON_CBN_CLOSEUP(IDC_KEYTYPE, OnCloseupKeytype)
	ON_CBN_SELCHANGE(IDC_KEYTYPE, OnSelchangeKeytype)
	ON_EN_CHANGE(IDC_VALUE, OnChangeValue)
	ON_BN_CLICKED(IDC_PROPBOOL, OnPropbool)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void PropEdit::validate()
{
	if (m_updating > 0)
		return;

	Bool enabOK = true;
	CWnd *ok = GetDlgItem(IDOK);
	CWnd *keyname = GetDlgItem(IDC_KEYNAME);
	CWnd *valuename = GetDlgItem(IDC_VALUE);
	CComboBox *keytype = (CComboBox*)GetDlgItem(IDC_KEYTYPE);
	CButton *valuebool = (CButton*)GetDlgItem(IDC_PROPBOOL);
	*m_type = (Dict::DataType)keytype->GetCurSel();

	valuename->ShowWindow(*m_type != Dict::DICT_BOOL ? SW_SHOW : SW_HIDE);
	valuebool->ShowWindow(*m_type == Dict::DICT_BOOL ? SW_SHOW : SW_HIDE);

	keyname->EnableWindow(!m_valueOnly);
	keytype->EnableWindow(!m_valueOnly);

	CString tmp;

	keyname->GetWindowText(tmp);
#ifdef RESTRICT_KEYS
	tmp.MakeLower();	// we force user-entered keys to all-lowercase, to prevent case issues
#endif
	*m_key = (LPCTSTR)tmp;

	if (!m_valueOnly)
	{
		int len = tmp.GetLength();
		if (len <= 0)
			enabOK = false;

#ifdef RESTRICT_KEYS
		// only letters, numbers, and underline allowed
		for (int i = 0; i < len; i++)
			if (!isalnum(tmp[i]) && tmp[i] != '_')
				enabOK = false;
#endif
	}

	valuename->GetWindowText(tmp);
	*m_value = (LPCTSTR)tmp;
	switch (*m_type)
	{
		case Dict::DICT_BOOL:
			*m_value = valuebool->GetCheck() ? "true" : "false";
			break;
		case Dict::DICT_INT:
			break;
		case Dict::DICT_REAL:
			break;
		case Dict::DICT_ASCIISTRING:
			break;
		case Dict::DICT_UNICODESTRING:
			break;
		default:
			enabOK = false;
			break;
	}

	ok->EnableWindow(enabOK);
}

/////////////////////////////////////////////////////////////////////////////
// PropEdit message handlers

void PropEdit::OnChangeKeyname() 
{
	validate();
}

void PropEdit::OnEditchangeKeytype() 
{
	validate();
}

void PropEdit::OnCloseupKeytype() 
{
	validate();
}

void PropEdit::OnSelchangeKeytype() 
{
	validate();
}

void PropEdit::OnChangeValue() 
{
	validate();
}


BOOL PropEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd *keyname = GetDlgItem(IDC_KEYNAME);
	CWnd *valuename = GetDlgItem(IDC_VALUE);
	CComboBox *keytype = (CComboBox*)GetDlgItem(IDC_KEYTYPE);
	CButton *valuebool = (CButton*)GetDlgItem(IDC_PROPBOOL);

	++m_updating;
	keytype->SetCurSel((Int)*m_type);	
	keyname->SetWindowText(m_key->str());
	valuename->SetWindowText(m_value->str());
	valuebool->SetCheck(strcmp(m_value->str(), "true")==0 ? 1 : 0);
	--m_updating;
	validate();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void PropEdit::OnPropbool() 
{
	validate();	
}
