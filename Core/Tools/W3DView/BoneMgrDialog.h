/*
**	Command & Conquer Renegade(tm)
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

#pragma once

// BoneMgrDialog.h : header file
//

#include "resource.h"
#include "Vector.h"

// Forward declarations
class HModelClass;
class RenderObjClass;


/////////////////////////////////////////////////////////////////////////////
//
// BoneMgrDialogClass
//
class BoneMgrDialogClass : public CDialog
{
// Construction
public:
	BoneMgrDialogClass (RenderObjClass *prender_obj, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(BoneMgrDialogClass)
	enum { IDD = IDD_BONE_MANAGEMENT };
	CComboBox	m_ObjectCombo;
	CTreeCtrl	m_BoneTree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BoneMgrDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(BoneMgrDialogClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedBoneTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeObjectCombo();
	afx_msg void OnDestroy();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAttachButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		///////////////////////////////////////////////////////////////////
		//
		//	Public methods
		//

	protected:

		///////////////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		void						Fill_Bone_Item (HTREEITEM hbone_item, int bone_index);
		bool						Is_Object_In_List (const char *passet_name, DynamicVectorClass <RenderObjClass *> &node_list);
		bool						Is_Render_Obj_Already_Attached (const CString &name);
		void						Update_Controls (HTREEITEM selected_item);
		HTREEITEM				Get_Current_Bone_Item (void);
		void						Remove_Object_From_Bone (HTREEITEM bone_item, const CString &name);

	private:

		///////////////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		RenderObjClass *		m_pBaseModel;
		RenderObjClass *		m_pBackupModel;
		bool						m_bAttach;
		CString					m_BoneName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
