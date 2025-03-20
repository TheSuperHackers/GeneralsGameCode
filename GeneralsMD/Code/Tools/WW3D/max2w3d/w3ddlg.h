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

/* $Header: /Commando/Code/Tools/max2w3d/w3ddlg.h 9     2/10/00 5:45p Jason_a $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - W3D export                                  * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3ddlg.h                       $* 
 *                                                                                             * 
 *                      $Author:: Jason_a                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 2/09/00 9:50a                                               $* 
 *                                                                                             * 
 *                    $Revision:: 9                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef W3DDLG_H
#define W3DDLG_H

#include "always.h"
#include <max.h>
#include "w3dutil.h"


class W3dOptionsDialogClass
{
public:

	W3dOptionsDialogClass(Interface * maxinterface,ExpInterface * exportinterface);
	~W3dOptionsDialogClass();
	
	bool Get_Export_Options(W3dExportOptionsStruct * options);
	bool Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM);

public:

	HWND								Hwnd;

private:

	void Dialog_Init();
	BOOL Dialog_Ok();
	void Enable_WHT_Export();
	void Enable_WHT_Load();
	void Disable_WHT_Export();
	void Enable_WHA_Export();
	void Disable_WHA_Export();
	void Enable_WTM_Export();
	void Disable_WTM_Export();
	
	void Enable_ReduceAnimationOptions_Export();
	void Disable_ReduceAnimationOptions_Export();
	void Enable_CompressAnimationOptions_Export();
	void Disable_CompressAnimationOptions_Export();
	
	void WHT_Export_Radio_Changed();
	void WHA_Export_Radio_Changed();
	void WTM_Export_Radio_Changed();

	void WHA_Compress_Animation_Check_Changed();
	void WHA_Reduce_Animation_Check_Changed();

	void WHA_Compression_Flavor_Changed();

private:

	W3dExportOptionsStruct *	Options;
	bool								GotHierarchyFilename;
	Interface *						MaxInterface;
	ExpInterface *		 			ExportInterface;

	ISpinnerControl *				RangeLowSpin;
	ISpinnerControl *				RangeHighSpin;
	
	HWND								HwndReduce;
	HWND								HwndFlavor;
	HWND								HwndTError;
	HWND								HwndRError;
  
	int								UnitsType;
	float								UnitsScale;
};


#endif