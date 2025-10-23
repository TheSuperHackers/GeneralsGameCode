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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/TextureMgrDialog.h     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/01/01 3:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "Vector.H"
#include "RendObj.H"
#include "Utils.H"
#include "Resource.H"
#include "DialogToolbar.H"
#include "texture.h"

// Forward declarations
class MeshClass;

#ifdef WW3D_DX8

/////////////////////////////////////////////////////////////////////////////
//
// Constants
//
const int ICON_MESH				= 0;
const int ICON_DEF_TEXTURE		= 1;
#define TEXTURE_NODE_LIST		DynamicVectorClass <class TextureListNodeClass *>


/////////////////////////////////////////////////////////////////////////////
//
// TextureListNodeClass
//

class TextureListNodeClass
{
	public:

		typedef enum
		{
			TYPE_MESH		= 0,
			TYPE_TEXTURE,
			TYPE_COUNT
		} NODE_TYPE;

		////////////////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//
		TextureListNodeClass (LPCTSTR name = NULL)
			: m_pTexture (NULL),
			  m_Type (TYPE_MESH),
			  m_pParent (NULL),
			  m_Name (name),
			  m_TextureIndex (0),
			  m_IconIndex (ICON_MESH) {}

		TextureListNodeClass (TextureClass *ptexture, LPCTSTR name = NULL)
			: m_pTexture (NULL),
			  m_Type (TYPE_TEXTURE),
			  m_pParent (NULL),
			  m_Name (name),
			  m_TextureIndex (0),
			  m_IconIndex (ICON_DEF_TEXTURE) { MEMBER_ADD (m_pTexture, ptexture); }

		~TextureListNodeClass (void) { MEMBER_RELEASE (m_pTexture); Free_Subobj_List (); }


		////////////////////////////////////////////////////////////
		//
		//	Public methods
		//
		void									Set_Name (LPCTSTR name)		{ m_Name = name; }
		LPCTSTR								Get_Name (void) const		{ return m_Name; }

		NODE_TYPE							Get_Type (void) const		{ return m_Type; }
		void									Set_Type (NODE_TYPE type)	{ m_Type = type; }

		TextureClass *						Peek_Texture (void) const	{ return m_pTexture; }
		void									Set_Texture (TextureClass *ptex) { MEMBER_ADD (m_pTexture, ptex); }

		TEXTURE_NODE_LIST	&				Get_Subobj_List (void)		{ return m_SubObjectList; }

		void									Set_Parent (TextureListNodeClass *pparent)	{ m_pParent = pparent; }
		TextureListNodeClass *			Get_Parent (void) const		{ return m_pParent; }

		void									Add_Subobj (TextureListNodeClass *pchild)	{ m_SubObjectList.Add (pchild); }

		int									Get_Icon_Index (void) const	{ return m_IconIndex; }
		void									Set_Icon_Index (int index)		{ m_IconIndex = index; }

		int									Get_Texture_Index (void) const	{ return m_TextureIndex; }
		void									Set_Texture_Index (int index)		{ m_TextureIndex = index; }

	protected:

		////////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		void									Free_Subobj_List (void);

	private:

		////////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		NODE_TYPE							m_Type;
		CString								m_Name;
		TEXTURE_NODE_LIST					m_SubObjectList;
		TextureClass *						m_pTexture;
		TextureListNodeClass *			m_pParent;
		int									m_IconIndex;
		int									m_TextureIndex;
};


/////////////////////////////////////////////////////////////////////////////
//
// Free_Subobj_List
//
__inline void
TextureListNodeClass::Free_Subobj_List (void)
{
	// Loop through all the subobject entries and free thier pointers
	for (int index = 0; index < m_SubObjectList.Count (); index ++) {
		SAFE_DELETE (m_SubObjectList[index]);
	}

	m_SubObjectList.Delete_All ();
	return ;
}



/////////////////////////////////////////////////////////////////////////////
//
// TextureMgrDialogClass dialog
//
class TextureMgrDialogClass : public CDialog
{

// Construction
public:
	TextureMgrDialogClass (RenderObjClass *pbase_model, CWnd *pParent = NULL);

// Dialog Data
	//{{AFX_DATA(TextureMgrDialogClass)
	enum { IDD = IDD_TEXTURE_MANAGMENT };
	CListCtrl	m_ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TextureMgrDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TextureMgrDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDblclkMeshTextureListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownMeshTextureListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnBack();
	afx_msg void OnDetails();
	afx_msg void OnLarge();
	afx_msg void OnList();
	afx_msg void OnSmall();
	afx_msg void OnPropagate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

	protected:

		////////////////////////////////////////////////////////////////
		//
		//	Protected methosd
		//
		void						Add_Subobjs_To_List (RenderObjClass *prender_obj);
		void						Add_Textures_To_Node (MeshClass *pmesh, TextureListNodeClass *pmesh_node);
		void						Fill_List_Ctrl_With_Meshes (void);
		void						Fill_List_Ctrl_With_Textures (TextureListNodeClass &pparent);
		int						Find_Texture_Thumbnail (LPCTSTR name);
		int						Get_Thumbnail (TextureClass *ptexture);
		void						Insert_Texture_Details (TextureListNodeClass *pnode, int index);

	private:

		////////////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		RenderObjClass *					m_pBaseModel;
		TEXTURE_NODE_LIST					m_NodeList;
		bool									m_bContainsMeshes;
		CImageList *						m_pImageList;
		CImageList *						m_pImageListSmall;
		CImageList *						m_pTextureImageList;
		CImageList *						m_pTextureImageListSmall;
		DynamicVectorClass <CString>	m_TextureNames;
		DialogToolbarClass				m_Toolbar;
};

#endif //WW3D_DX8

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
