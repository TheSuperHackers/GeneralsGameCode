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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GameMtlPassDlg.h               $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/17/98 1:32p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */




#ifndef GAMEMTLPASSDLG_H
#define GAMEMTLPASSDLG_H

#include <max.h>

class GameMtl;
class GameMtlFormClass;

/*
** The GameMtlPassDlg will contain a Tab Control which switches between
** editing the VertexMaterial parameters, the Shader parameters and the
** Texture parameters.
*/
class GameMtlPassDlg: public ParamDlg
{
public:

	GameMtlPassDlg(HWND hwMtlEdit, IMtlParams *imp, GameMtl *m,int pass);
	~GameMtlPassDlg();

	BOOL					DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	void					Invalidate();
	void					UpdateMtlDisplay()						{ IParams->MtlChanged(); }

	void					ReloadDialog();
	Class_ID				ClassID();
	void					SetThing(ReferenceTarget* target);
	ReferenceTarget *	GetThing()									{ return (ReferenceTarget *)TheMtl; }
	void					DeleteThis()								{ delete this;  }
	void					SetTime(TimeValue t);
	void					ActivateDlg(BOOL onOff);

	enum { PAGE_COUNT = 3 };

	////////////////////////////////////////////////////////////////////////
	// Material dialog interface
	////////////////////////////////////////////////////////////////////////
	IMtlParams *		IParams;			// interface to the material editor
	GameMtl *			TheMtl;			// current mtl being edited.

	////////////////////////////////////////////////////////////////////////
	// Windows handles
	////////////////////////////////////////////////////////////////////////
	HWND					HwndEdit;		// window handle of the materials editor dialog
	HWND					HwndPanel;		// Rollup parameters panel

	////////////////////////////////////////////////////////////////////////
	// Variables
	////////////////////////////////////////////////////////////////////////
	int					PassIndex;
	int					CurPage;
	BOOL					Valid;

	GameMtlFormClass*	Page[PAGE_COUNT];
};

#endif
