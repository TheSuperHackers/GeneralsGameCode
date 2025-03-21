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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/SceneSetupDlg.h                $*
 *                                                                                             *
 *                      $Author:: Andre_a                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/15/99 4:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef SCENESETUPDLG_H
#define SCENESETUPDLG_H

// SceneSetupDlg.h : header file
//

#include "dllmain.h"
#include "resource.h"


class Interface;

/////////////////////////////////////////////////////////////////////////////
// SceneSetupDlg dialog

class SceneSetupDlg
{
public:

	// Construction
	SceneSetupDlg(Interface *max_interface);

	// Methods
	int DoModal (void);

	// DialogProc
	BOOL CALLBACK DialogProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Dialog data associated with GUI components.
	enum	{ IDD = IDD_SCENE_SETUP };
	int	m_DamageCount;
	float	m_DamageOffset;
	int	m_LodCount;
	float	m_LodOffset;
	int	m_LodProc;
	int	m_DamageProc;

	// Dialog Data
	HWND			m_hWnd;

protected:

	// Message Handlers
	void OnInitDialog (void);
	BOOL OnOK (void);		// TRUE if ok to close dialog

	// Protected Methods
	void  SetEditInt   (int control_id, int value);
	void  SetEditFloat (int control_id, float value);
	int   GetEditInt   (int control_id);
	float GetEditFloat (int control_id);
	bool  ValidateEditFloat (int control_id);

	// Protected Data
	Interface	*m_MaxInterface;
};

#endif
