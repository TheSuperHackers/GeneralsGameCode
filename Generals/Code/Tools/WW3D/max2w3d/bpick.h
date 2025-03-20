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

/* $Header: /Commando/Code/Tools/max2w3d/bpick.h 6     10/28/97 6:08p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - WWSkin                                      * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/bpick.h                        $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/21/97 2:05p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 6                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef BPICK_H
#define BPICK_H

#include "max.h"
//#include "dllmain.h"
//#include "resource.h"


/*
**	To use the Bone picking class, you should inherit from this class
** and implement the User_Picked... functions.  
*/
class BonePickerUserClass
{
public:
	virtual void User_Picked_Bone(INode * node) = 0;
	virtual void User_Picked_Bones(INodeTab & nodetab) = 0;
};


/*
** BonePickerClass
**	Uses Max's interface to let the user pick bones out of the scene
** or by using a dialog box to pick by name.
*/
class BonePickerClass : public PickNodeCallback, public PickModeCallback, public HitByNameDlgCallback
{
public:
	
	BonePickerClass(void) : User(NULL), BoneList(NULL), SinglePick(FALSE) {}

	/*
	** Tell this class who is using it and optionally the list
	** of bones to allow the user to select from.
	** Call this before giving this class to MAX...
	*/
	void Set_User(BonePickerUserClass * user,int singlepick = FALSE, INodeTab * bonelist = NULL) { User = user; SinglePick = singlepick; BoneList = bonelist; }

	/*
	** From BonePickNodeCallback:
	*/
	BOOL Filter(INode *node);

	/*
	** From BonePickModeCallback:
	*/
	BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
	BOOL Pick(IObjParam *ip,ViewExp *vpt);
		
	void EnterMode(IObjParam *ip) { }
	void ExitMode(IObjParam *ip) { }

	PickNodeCallback * GetFilter() {return this;}
	BOOL RightClick(IObjParam *ip,ViewExp *vpt) { return TRUE; }
	
	/*
	** From HitByNameDlgCallback
	*/
	virtual TCHAR * dialogTitle(void);
	virtual TCHAR * buttonText(void);
	virtual BOOL singleSelect(void) { return SinglePick; }
	virtual BOOL useFilter(void) { return TRUE; }
	virtual BOOL useProc(void) { return TRUE; }
	virtual BOOL doCustomHilite(void) { return FALSE; }
	virtual BOOL filter(INode * inode);
	virtual void proc(INodeTab & nodeTab);

protected:

	/*
	** The bone picker will pass the bones on to the "user" of
	** the class.  
	*/
	BonePickerUserClass * User;

	/*
	** List of bones that the user is being allowed to pick from.
	** If this is NULL, then the user can pick any bone
	*/
	INodeTab * BoneList;

	/*
	** Flag for whether to allow multiple selection or not
	*/
	int SinglePick;
};

extern BonePickerClass TheBonePicker;


#endif
