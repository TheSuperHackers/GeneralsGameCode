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

/* $Header: /Commando/Code/Tools/max2w3d/bpick.cpp 7     1/04/01 11:12a Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - WWSkin                                      * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/bpick.cpp                      $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/04/01 11:12a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 7                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   BonePickerClass::Filter -- determine whether the passed node is suitable                  * 
 *   BonePickerClass::HitTest -- MAX HitTest method                                            * 
 *   BonePickerClass::Pick -- MAX Pick method                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "bpick.h"
#include "dllmain.h"
#include "resource.h"


/*
** Global instance of a bone picker :-)
*/ 
BonePickerClass TheBonePicker;


/*********************************************************************************************** 
 * BonePickerClass::Filter -- determine whether the passed node is suitable                    * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
BOOL BonePickerClass::Filter(INode *node)
{
	if (BoneList == NULL) {
		ObjectState os  = node->EvalWorldState(0);
		if (os.obj) {
			return TRUE;
		}

	} else {
		for (int i=0; i<BoneList->Count(); i++) {
			if ((*BoneList)[i] == node) return TRUE;
		}
	}

	return FALSE;
}

/*********************************************************************************************** 
 * BonePickerClass::HitTest -- MAX HitTest method                                              * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
BOOL BonePickerClass::HitTest(IObjParam *ip,HWND hwnd,ViewExp *vpt,IPoint2 m,int flags)
{
	if (ip->PickNode(hwnd,m,GetFilter())) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*********************************************************************************************** 
 * BonePickerClass::Pick -- MAX Pick method                                                    * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
BOOL BonePickerClass::Pick(IObjParam *ip,ViewExp *vpt)
{
	INode *node = vpt->GetClosestHit();
	
	if (node) {

		/*
		** Tell the "owning" skin modifier about the
		** bone which was picked.
		*/
		assert(User);
		User->User_Picked_Bone(node);
		User = NULL;
		BoneList = NULL;
	}

	return TRUE;
}

BOOL BonePickerClass::filter(INode * inode)
{
	return Filter(inode);
}

void BonePickerClass::proc(INodeTab & nodetab)
{
	assert(User != NULL);
	User->User_Picked_Bones(nodetab);	
	User = NULL;
	BoneList = NULL;
}

TCHAR * BonePickerClass::dialogTitle(void) 
{ 
	return Get_String(IDS_PICK_BONE_DIALOG_TITLE); 
}

TCHAR * BonePickerClass::buttonText(void) 
{ 
	return Get_String(IDS_PICK_BONE_BUTTON_TEXT); 
}
