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
 *                 Project name : Buccaneer Bay                                                *
 *                                                                                             *
 *                    File name : AlphaModifier.h                                              *
 *                                                                                             *
 *                   Programmer : Mike Lytle                                                   *
 *                                                                                             *
 *                   Start date : 11/1/1999                                                    *
 *                                                                                             *
 *                  Last update : 11/1/1999                                                    *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <max.h>
#include "iparamm2.h"
#include "istdplug.h"
#include "meshadj.h"
#include "modstack.h"
#include "macrorec.h"
#include "resource.h"
#include "dllmain.h"


#define	ALPHA_MODIFIER_CLASSID		Class_ID(0x518970b3, 0x37d73373)


extern ClassDesc* Get_Alpha_Desc();

#define ALPHA_VERTEX_CHANNEL 10

class AlphaModifierClass : public Modifier
{
	public:

		// Global parameter block
		IParamBlock2	*pblock;


		//Constructor/Destructor
		AlphaModifierClass();
		~AlphaModifierClass() {}
		void DeleteThis() { delete this;}


		// Plugin identification
		void GetClassName(TSTR& s) { s= TSTR(Get_String(IDS_ALPHA_MODIFIER_CLASS));}
		virtual Class_ID ClassID() { return ALPHA_MODIFIER_CLASSID;}
		TCHAR *GetObjectName() { return Get_String(IDS_ALPHA_MODIFIER_CLASS);}


		// Defines the behavior for this modifier
		// This is currently setup to be basic geometry
		// modification of deformable objects
		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE;}
		ChannelMask ChannelsChanged() {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE;}
		Class_ID InputType() { return triObjectClassID;}
		BOOL ChangeTopology() {return FALSE;}


		// Calculate the local validity from the parameters
		Interval LocalValidity(TimeValue t);
		Interval GetValidity(TimeValue t);


		// Object modification and notification of change
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);


		// Reference support
		int NumRefs() { return 1;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message);

		// SubAnim support
		int NumSubs() { return 0;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);

		// Direct paramblock access
		int	NumParamBlocks() {return 1;}
		IParamBlock2* GetParamBlock(int i) { return pblock;}
		IParamBlock2* GetParamBlockByID(BlockID id) {return (pblock->ID() == id) ? pblock : NULL;}
		int GetParamBlockIndex(int id) {return id;}

		// Does not use createmouse callbacks
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		// Load and unload our UI
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);
		void InvalidateUI();

		// Message saved from window messages.
		int			Message;

		// Selected vertices.
		BitArray	SelectedVertices;
};


/*===========================================================================*\
 |	Dialog Processor
\*===========================================================================*/

class AlphaModDlgProc : public ParamMap2UserDlgProc
{
	public:
		AlphaModifierClass *AlphaModifier;

		AlphaModDlgProc() {}
		AlphaModDlgProc(AlphaModifierClass *alpha_m) {AlphaModifier = alpha_m;}

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() {}

		void SetThing(ReferenceTarget *m) {AlphaModifier = (AlphaModifierClass*)m;}
};
